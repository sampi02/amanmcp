# Auto-Reindexing in AmanMCP - Deep Dive

## Summary

AmanMCP uses a **real-time file watcher** combined with **startup reconciliation** to keep the index in sync with your codebase. Changes you make are detected and indexed automatically within ~200ms.

---

## How It Works: The Complete Flow

```mermaid
flowchart TD
    A["FILE SYSTEM CHANGE<br/>(create/modify/delete)"] --> B
    B["HYBRID WATCHER<br/>• Primary: fsnotify (real-time OS events)<br/>• Fallback: Polling (5s interval, Docker/NFS)<br/>Location: internal/watcher/hybrid.go:20-489"] --> C
    C[".gitignore FILTER<br/>• Respects .gitignore patterns (nested + root)<br/>• Excludes archive/, node_modules/, .git/<br/>Location: internal/watcher/hybrid.go:329-348"] --> D
    D["DEBOUNCER (200ms)<br/>• Coalesces rapid changes to prevent thrashing<br/>• Smart rules: CREATE+DELETE=nothing<br/>Location: internal/watcher/debouncer.go:79-124"] --> E
    E["COORDINATOR<br/>• Receives batched events, routes to handlers<br/>• OpCreate/OpModify → indexFile()<br/>• OpDelete → removeFile()<br/>Location: internal/index/coordinator.go:82-127"] --> F
    F["INDEX UPDATE<br/>1. Chunk file (tree-sitter/headers)<br/>2. Generate embeddings (Ollama/Static768)<br/>3. Update BM25 index<br/>4. Update HNSW vector store<br/>5. Update SQLite metadata<br/>Location: internal/index/coordinator.go:129-248"]

    style A fill:#e1f5ff,stroke:#01579b,stroke-width:2px
    style B fill:#fff3e0,stroke:#e65100,stroke-width:2px
    style C fill:#f3e5f5,stroke:#4a148c,stroke-width:2px
    style D fill:#e8f5e9,stroke:#1b5e20,stroke-width:2px
    style E fill:#fff9c4,stroke:#f57f17,stroke-width:2px
    style F fill:#fce4ec,stroke:#880e4f,stroke-width:2px
```

---

## Key Components

### 1. HybridWatcher (`internal/watcher/hybrid.go`)

**What it does:** Monitors your codebase for file changes in real-time.

**How it works:**
- **Primary method**: Uses `fsnotify` (lines 71-74) for OS-level file events
- **Fallback method**: Polling every 5 seconds (lines 101) for environments where fsnotify doesn't work (Docker, NFS, network drives)
- **Automatic detection**: Tries fsnotify first, falls back if it fails (line 76-78)

**Events detected:**
| Event | Code | Description |
|-------|------|-------------|
| `OpCreate` | 0 | New file created |
| `OpModify` | 1 | Existing file modified |
| `OpDelete` | 2 | File deleted |
| `OpRename` | 3 | File renamed (becomes MODIFY via debouncer) |
| `OpGitignoreChange` | 4 | .gitignore file changed |
| `OpConfigChange` | 5 | .amanmcp.yaml changed |

### 2. Debouncer (`internal/watcher/debouncer.go`)

**What it does:** Prevents index thrashing from rapid file changes (like saving multiple times quickly).

**How it works:**
- Collects events for 200ms (configurable)
- Coalesces related events using smart rules (lines 79-124):
  ```
  CREATE + MODIFY = CREATE (file still new)
  CREATE + DELETE = NOTHING (file never existed)
  MODIFY + DELETE = DELETE (file gone)
  DELETE + CREATE = MODIFY (file replaced)
  ```
- Emits batched events to the Coordinator

**State Machine:**

```mermaid
stateDiagram-v2
    [*] --> Idle: Debouncer started
    Idle --> Pending: First event received
    Pending --> Debouncing: Timer started (200ms)
    Debouncing --> Debouncing: More events received<br/>(reset timer)
    Debouncing --> Executing: Timer expired
    Executing --> Idle: Events sent to Coordinator

    note right of Pending
        Event added to buffer
        Timer not yet started
    end note

    note right of Debouncing
        Coalescing events:
        CREATE+DELETE=NOTHING
        CREATE+MODIFY=CREATE
        MODIFY+DELETE=DELETE
        DELETE+CREATE=MODIFY
    end note

    note right of Executing
        Batched events emitted
        Buffer cleared
    end note
```

### 3. Coordinator (`internal/index/coordinator.go`)

**What it does:** The brain of the indexing system. Routes events to appropriate handlers.

**Key methods:**
- `HandleEvents()` (lines 82-98): Entry point for batched events
- `indexFile()` (lines 129-248): Indexes new or modified files
- `removeFile()` (lines 251-291): Removes deleted files from index
- `handleGitignoreChange()` (lines 313-369): Smart .gitignore reconciliation

**indexFile() flow:**
1. Stat file, check size < 100MB
2. Skip symlinks and binary files
3. Detect language (Go, TypeScript, etc.)
4. Remove old chunks (for re-indexing modified files)
5. Parse with tree-sitter (code) or header-based (markdown)
6. Generate embeddings via Ollama/Static768
7. Add to BM25 + HNSW + SQLite

### 4. Server Integration (`cmd/amanmcp/cmd/serve.go`)

**Key point:** Watcher runs in **background goroutine** (line 285), doesn't block MCP handshake.

```go
// Line 309-319: Watcher configuration
opts := watcher.Options{
    DebounceWindow:  200 * time.Millisecond,
    PollInterval:    5 * time.Second,
    EventBufferSize: 1000,
}
```

---

## Startup Reconciliation

When the MCP server starts, it detects changes made while it was stopped:

### Phase 1: Gitignore Reconciliation (line 346)
**Location:** `coordinator.go:665-709`

- Computes SHA256 hash of all .gitignore files
- Compares against cached hash in SQLite
- If changed, runs smart reconciliation:
  - **Nested .gitignore**: Only scans affected subtree
  - **Root .gitignore + patterns added**: No filesystem scan, just filters indexed files
  - **Patterns removed**: Full scan to find newly-unignored files

### Phase 2: File Reconciliation (line 352)
**Location:** `coordinator.go:830-893`

- Gets all indexed files with their mtime + size
- Scans current filesystem
- Detects:
  - **Deleted**: In index but not on disk
  - **Modified**: mtime or size changed
  - **Added**: On disk but not in index
- Processes changes deterministically: deletions → modifications → additions

### Decision Tree: Startup Reconciliation

```mermaid
flowchart TD
    Start([Server Startup]) --> A{Compute .gitignore<br/>SHA256 hash}
    A --> B{Hash changed?}
    B -->|No| E[Skip gitignore reconciliation]
    B -->|Yes| C{Which .gitignore<br/>changed?}

    C -->|Nested| D1[Scan affected subtree only]
    C -->|Root + patterns added| D2[Filter indexed files<br/>No filesystem scan]
    C -->|Root + patterns removed| D3[Full filesystem scan<br/>Find newly-unignored files]

    D1 --> F
    D2 --> F
    D3 --> F
    E --> F

    F[Update cached gitignore hash] --> G{Get all indexed files<br/>with mtime + size}
    G --> H[Scan current filesystem]
    H --> I{Compare index vs disk}

    I --> J{File in index<br/>but not on disk?}
    J -->|Yes| K[Mark for deletion]
    J -->|No| L

    I --> L{mtime or size<br/>changed?}
    L -->|Yes| M[Mark for reindexing]
    L -->|No| N

    I --> N{File on disk<br/>but not in index?}
    N -->|Yes| O[Mark for addition]
    N -->|No| P

    K --> Q[Process deletions]
    M --> R[Process modifications]
    O --> S[Process additions]

    Q --> R
    R --> S
    S --> T[Start file watcher]
    P --> T
    T --> End([Ready for MCP requests])

    style Start fill:#e1f5ff,stroke:#01579b,stroke-width:3px
    style End fill:#c8e6c9,stroke:#2e7d32,stroke-width:3px
    style B fill:#fff9c4,stroke:#f57f17,stroke-width:2px
    style C fill:#fff9c4,stroke:#f57f17,stroke-width:2px
    style I fill:#fff9c4,stroke:#f57f17,stroke-width:2px
    style J fill:#ffe0b2,stroke:#e65100,stroke-width:2px
    style L fill:#ffe0b2,stroke:#e65100,stroke-width:2px
    style N fill:#ffe0b2,stroke:#e65100,stroke-width:2px
    style K fill:#ffcdd2,stroke:#c62828,stroke-width:2px
    style M fill:#fff3e0,stroke:#ef6c00,stroke-width:2px
    style O fill:#c8e6c9,stroke:#2e7d32,stroke-width:2px
```

---

## Tracking Methods

| Method | What it tracks | When used |
|--------|----------------|-----------|
| **mtime + size** | Quick change detection | Real-time watcher, startup reconciliation |
| **Content hash (SHA256)** | Exact content verification | Stored in metadata for validation |
| **Gitignore hash** | .gitignore file changes | Cached in SQLite, checked on startup |

---

## Key File Locations

| Component | File | Key Lines |
|-----------|------|-----------|
| Watcher Interface | `internal/watcher/watcher.go` | 8-138 |
| Hybrid Watcher | `internal/watcher/hybrid.go` | 20-489 |
| Polling Fallback | `internal/watcher/polling.go` | 15-185 |
| Debouncer | `internal/watcher/debouncer.go` | 9-184 |
| Coordinator | `internal/index/coordinator.go` | 60-1025 |
| Server Integration | `cmd/amanmcp/cmd/serve.go` | 305-439 |
| Metadata Store | `internal/store/metadata.go` | 114-645 |
| Search Engine | `internal/search/engine.go` | 156-203 |

---

## Why Are My Changes Being Reindexed?

Based on the architecture:

1. **The MCP server is running** with a background file watcher
2. **When you edit a file**, fsnotify detects the `Write` event
3. **After 200ms debounce**, the event is batched and sent to the Coordinator
4. **Coordinator calls `indexFile()`**, which:
   - Removes old chunks for that file
   - Re-chunks the file
   - Generates new embeddings
   - Updates BM25 + HNSW + SQLite
5. **Next search query** sees the updated content

**If the server was restarted** since the initial indexing:
- Startup reconciliation detects your file changes via mtime/size comparison
- Processes them before the watcher starts listening

### Sequence Diagram: File System Change Flow

```mermaid
sequenceDiagram
    participant FS as File System
    participant FSN as fsnotify
    participant GI as .gitignore Filter
    participant DB as Debouncer
    participant CO as Coordinator
    participant IDX as Index Stores<br/>(BM25/HNSW/SQLite)

    Note over FS: User saves file.go
    FS->>FSN: Write event
    Note over FSN: t=0ms
    FSN->>GI: Event: OpModify file.go

    GI->>GI: Check .gitignore patterns
    alt File ignored
        GI-->>FSN: Drop event
    else File not ignored
        GI->>DB: Forward event
    end

    Note over DB: t=0ms<br/>Start 200ms timer
    DB->>DB: Add to buffer

    Note over FS: User saves again
    FS->>FSN: Write event
    Note over FSN: t=50ms
    FSN->>GI: Event: OpModify file.go
    GI->>DB: Forward event

    Note over DB: t=50ms<br/>Reset timer to 200ms
    DB->>DB: Coalesce MODIFY+MODIFY=MODIFY

    Note over DB: t=250ms<br/>Timer expired
    DB->>CO: Batched events: [OpModify file.go]

    CO->>CO: Route to indexFile()
    CO->>IDX: Remove old chunks for file.go
    IDX-->>CO: OK

    CO->>CO: Parse file with tree-sitter
    CO->>CO: Generate embeddings (Ollama)

    CO->>IDX: Add chunks to BM25
    IDX-->>CO: OK
    CO->>IDX: Add vectors to HNSW
    IDX-->>CO: OK
    CO->>IDX: Update SQLite metadata
    IDX-->>CO: OK

    Note over CO,IDX: Total time: ~200-500ms<br/>from last file save
```

---

## Verification

To confirm auto-reindexing is working:

```bash
# Check index status (CLI)
amanmcp status

# Or via MCP tool (when server is running)
# Use index_status tool to see file counts, last indexed time
```

Changes should be reflected in search results within ~200-500ms of saving a file.

The "Last indexed" timestamp in `amanmcp status` is updated after every incremental update when the MCP server is running.

---

## Auto-Reindexing Operational Guide

```mermaid
flowchart TB
    subgraph Operation["Auto-Reindexing: What Happens When"]
        direction TB

        subgraph Startup["Server Startup (amanmcp serve)"]
            S1["1. Load existing index from disk"]
            S2["2. Reconcile .gitignore changes<br/>(compute SHA256 hash)"]
            S3["3. Reconcile file changes<br/>(mtime + size comparison)"]
            S4["4. Process deletions → modifications → additions"]
            S5["5. Start HybridWatcher<br/>(fsnotify + polling fallback)"]
            S6["6. MCP server ready"]
            S1 --> S2 --> S3 --> S4 --> S5 --> S6
        end

        subgraph Runtime["During Runtime (File Changes)"]
            R1["Developer saves file.go"]
            R2["fsnotify detects Write event<br/>(< 1ms)"]
            R3["Filter through .gitignore<br/>patterns"]
            R4["Debouncer collects events<br/>(200ms window)"]
            R5["Coordinator receives batch<br/>Routes to indexFile()"]
            R6["Re-chunk → Embed → Update indexes<br/>(~200-500ms total)"]
            R7["Next search sees updated content"]
            R1 --> R2 --> R3 --> R4 --> R5 --> R6 --> R7
        end

        subgraph Special["Special Cases"]
            SP1[".gitignore Modified:<br/>• Nested: Scan subtree only<br/>• Root + added patterns: Filter indexed files<br/>• Root + removed patterns: Full scan"]
            SP2["Rapid Changes (same file):<br/>• Multiple saves debounced<br/>• Final state indexed once"]
            SP3["File Renamed:<br/>• Treated as DELETE + CREATE<br/>• Debouncer coalesces to MODIFY"]
        end

        subgraph Fallback["Polling Fallback (Docker/NFS)"]
            F1["fsnotify unreliable?<br/>Auto-detect and switch"]
            F2["Poll every 5s:<br/>• Stat all tracked files<br/>• Compare mtime + size"]
            F3["Process changes same as fsnotify"]
            F1 --> F2 --> F3
        end
    end

    Startup -.-> Runtime
    Runtime -.-> Special
    Runtime -.-> Fallback

    style Operation fill:#f5f5f5,stroke:#757575,stroke-width:2px
    style Startup fill:#e1f5ff,stroke:#01579b,stroke-width:2px
    style Runtime fill:#c8e6c9,stroke:#2e7d32,stroke-width:2px
    style Special fill:#fff9c4,stroke:#f57f17,stroke-width:2px
    style Fallback fill:#ffe0b2,stroke:#e65100,stroke-width:2px
    style S6 fill:#c8e6c9
    style R7 fill:#c8e6c9
```
