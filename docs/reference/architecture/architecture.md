# AmanMCP Architecture Design

**Version:** 1.0.0-draft | **Go:** 1.25.5+ | **MCP Spec:** 2025-11-25

---

## Quick Overview

**New here?** This section gives you the essentials in 2 minutes.

### How AmanMCP Works

1. **Auto-Discovery** — Detects project type (Go, Node, Python) and source directories automatically
2. **AST-Based Chunking** — Uses tree-sitter to split code at function/class boundaries (not arbitrary lines)
3. **Hybrid Search** — Combines BM25 keyword search + semantic vector search for best results
4. **Incremental Updates** — Only re-indexes changed files

### Key Components

| Component | Technology | Purpose |
|-----------|------------|---------|
| MCP Server | Official Go SDK | Claude Code integration |
| Code Parsing | tree-sitter | AST-aware chunking |
| Keyword Search | SQLite FTS5 BM25 | Exact term matching |
| Vector Search | coder/hnsw | Semantic similarity |
| Embeddings | Ollama (or Static768 fallback) | Text → vectors |
| Metadata | SQLite | File and chunk info |

### Multi-Project Support

Each VS Code/Cursor instance spawns its own isolated AmanMCP server. No manual switching needed.

```
VS Code #1 (api/)     VS Code #2 (web/)
       │                     │
       ▼                     ▼
  AmanMCP #1            AmanMCP #2
  (indexes api/)        (indexes web/)
```

**For detailed design, see the sections below.**

---

## 1. Architectural Overview

### 1.1 Design Philosophy

AmanMCP follows the **"It Just Works"** philosophy, inspired by Apple's design principles:

1. **Simplicity First** - Zero configuration required
2. **Sensible Defaults** - Convention over configuration
3. **Progressive Disclosure** - Advanced options available but hidden
4. **Local-First** - Privacy by design, no cloud dependencies
5. **Single Binary** - No runtime dependencies for users

### 1.2 High-Level Architecture

```mermaid
flowchart TB
    subgraph Clients["CLIENT LAYER"]
        CC[Claude Code]
        Cursor[Cursor]
        Other[Other MCP Clients]
    end

    subgraph MCP["MCP SERVER LAYER"]
        Protocol[Protocol Handler]
        Tools[Tool Router]
        Resources[Resource Provider]
        Notifier[Event Notifier]
    end

    subgraph Core["CORE ENGINE"]
        subgraph Search["SEARCH ENGINE"]
            Parser[Query Parser]
            Router[Query Router]
            BM25[BM25 Searcher<br/>Keyword]
            Vector[Vector Searcher<br/>Semantic]
            RRF[Score Fusion<br/>RRF k=60]
            Aggregator[Result Aggregator]
        end

        subgraph Index["INDEXER"]
            Scanner[Scanner<br/>Discovery]
            Chunker[Chunker<br/>tree-sitter]
            Embedder[Embedder<br/>Ollama/MLX]
            Persister[Persister<br/>Storage]
        end

        subgraph Watch["WATCHER"]
            FSNotify[fsnotify]
            Debouncer[Debouncer]
            DiffQueue[Diff Queue]
        end
    end

    subgraph Storage["STORAGE LAYER"]
        HNSW[(coder/hnsw<br/>HNSW Vectors)]
        BM25Index[(BM25 Index<br/>Inverted)]
        Metadata[(SQLite<br/>Metadata)]
    end

    Clients -->|MCP Protocol<br/>stdio/SSE| MCP
    MCP --> Core

    Parser --> Router
    Router --> BM25
    Router --> Vector
    BM25 --> RRF
    Vector --> RRF
    RRF --> Aggregator

    Scanner --> Chunker --> Embedder --> Persister

    FSNotify --> Debouncer --> DiffQueue --> Scanner

    Persister --> Storage
    Search --> Storage

    style Clients fill:#3498db,color:#fff
    style MCP fill:#9b59b6,color:#fff
    style Search fill:#27ae60,color:#fff
    style Index fill:#e67e22,color:#fff
    style Watch fill:#f39c12,color:#fff
    style Storage fill:#34495e,color:#fff
```

---

## 2. Component Design

### 2.1 Project Structure

```
amanmcp/
├── cmd/
│   └── amanmcp/
│       └── main.go              # CLI entry point
├── internal/
│   ├── config/
│   │   ├── config.go            # Configuration management
│   │   ├── defaults.go          # Default values
│   │   └── detection.go         # Project type detection
│   ├── mcp/
│   │   ├── server.go            # MCP server implementation
│   │   ├── tools.go             # Tool definitions
│   │   ├── resources.go         # Resource providers
│   │   └── transport.go         # stdio/SSE transports
│   ├── search/
│   │   ├── engine.go            # Hybrid search coordinator
│   │   ├── bm25.go              # BM25 implementation
│   │   ├── vector.go            # Vector search wrapper
│   │   ├── fusion.go            # Score fusion (RRF)
│   │   └── classifier.go        # Query classification
│   ├── index/
│   │   ├── indexer.go           # Main indexer
│   │   ├── scanner.go           # File discovery
│   │   ├── chunker.go           # Chunking coordinator
│   │   └── watcher.go           # File system watcher
│   ├── chunk/
│   │   ├── code.go              # AST-based code chunker
│   │   ├── markdown.go          # Markdown chunker
│   │   ├── text.go              # Plain text chunker
│   │   └── treesitter.go        # tree-sitter integration
│   ├── embed/
│   │   ├── types.go             # Embedder interface
│   │   ├── ollama.go            # OllamaEmbedder (recommended, uses Ollama API)
│   │   ├── static768.go         # Static768 (768-dim fallback, default)
│   │   └── static.go            # Static256 (legacy fallback)
│   ├── store/
│   │   ├── store.go             # Storage coordinator
│   │   ├── hnsw.go              # coder/hnsw wrapper
│   │   ├── bm25.go              # BM25 index storage
│   │   └── metadata.go          # SQLite metadata
│   └── models/
│       ├── chunk.go             # Chunk model
│       ├── project.go           # Project model
│       └── result.go            # Search result model
├── pkg/
│   └── version/
│       └── version.go           # Version info
├── testdata/
│   ├── projects/                # Test projects
│   ├── fixtures/                # Test fixtures
│   └── golden/                  # Golden files
├── scripts/
│   ├── build.sh                 # Build script
│   └── release.sh               # Release script
├── .goreleaser.yaml             # GoReleaser config
├── Makefile                     # Development commands
├── go.mod
├── go.sum
├── LICENSE                      # Apache 2.0 License
└── README.md                    # User documentation
```

### 2.2 Component Interactions

```mermaid
sequenceDiagram
    participant Client as Claude Code
    participant MCP as MCP Server
    participant Engine as Search Engine
    participant BM25 as BM25 Index
    participant Vec as Vector Index
    participant Embed as Embedder

    Client->>MCP: search("authentication middleware")
    MCP->>Engine: Search(query, opts)

    par Parallel Search
        Engine->>BM25: Search(tokens)
        BM25-->>Engine: keyword_results[]
    and
        Engine->>Embed: Embed(query)
        Embed-->>Engine: query_vector
        Engine->>Vec: Search(query_vector, k=50)
        Vec-->>Engine: semantic_results[]
    end

    Engine->>Engine: RRF Fusion(keyword, semantic)
    Engine-->>MCP: ranked_results[]
    MCP-->>Client: SearchResult JSON
```

### 2.3 Indexing Pipeline

```mermaid
flowchart LR
    subgraph Discovery["1. DISCOVERY"]
        Files[Source Files]
        Gitignore[.gitignore filter]
        Scanner[File Scanner]
    end

    subgraph Parsing["2. PARSING"]
        TS[tree-sitter]
        AST[AST Analysis]
        Chunks[Code Chunks]
    end

    subgraph Embedding["3. EMBEDDING"]
        Batch[Batch Processing]
        Ollama[Ollama API]
        Vectors[768-dim Vectors]
    end

    subgraph Storage["4. STORAGE"]
        HNSW[(HNSW Index)]
        BM25[(BM25 Index)]
        Meta[(SQLite)]
    end

    Files --> Gitignore --> Scanner
    Scanner --> TS --> AST --> Chunks
    Chunks --> Batch --> Ollama --> Vectors
    Vectors --> HNSW
    Chunks --> BM25
    Chunks --> Meta

    style Discovery fill:#3498db,color:#fff
    style Parsing fill:#9b59b6,color:#fff
    style Embedding fill:#e67e22,color:#fff
    style Storage fill:#27ae60,color:#fff
```

---

## 3. Key Algorithms

### 3.1 Query Classification

```mermaid
flowchart TB
    Query["Query: 'useEffect cleanup function'"]

    subgraph Step1["Step 1: Pattern Detection"]
        P1{Has camelCase?}
        P2{Has error code?}
        P3{Natural language?}
        P4{Special chars?}
    end

    subgraph Step2["Step 2: Weight Assignment"]
        W1[Technical term → boost BM25]
        W2[Natural language → keep semantic]
        W3[Result: BM25=0.5, Semantic=0.5]
    end

    subgraph Step3["Step 3: Classification Output"]
        Output["{ type: 'mixed',<br/>bm25_weight: 0.5,<br/>semantic_weight: 0.5,<br/>boost_terms: ['useEffect'] }"]
    end

    Query --> Step1
    P1 -->|YES| W1
    P2 -->|NO| W2
    P3 -->|MIXED| W2
    P4 -->|NO| W3
    Step2 --> Output

    style Query fill:#3498db,color:#fff
    style Step1 fill:#f39c12,color:#fff
    style Step2 fill:#9b59b6,color:#fff
    style Output fill:#27ae60,color:#fff
```

```mermaid
flowchart LR
    subgraph Classification["Query Classification Flow"]
        Q[Query] --> Classify{Classify}

        Classify -->|Error Code| EC["BM25: 0.8<br/>Vector: 0.2"]
        Classify -->|camelCase| CC["BM25: 0.7<br/>Vector: 0.3"]
        Classify -->|Natural Lang| NL["BM25: 0.25<br/>Vector: 0.75"]
        Classify -->|Mixed| MX["BM25: 0.5<br/>Vector: 0.5"]
        Classify -->|Quoted| QT["BM25: 0.9<br/>Vector: 0.1"]
    end

    EC --> Search
    CC --> Search
    NL --> Search
    MX --> Search
    QT --> Search

    Search[Hybrid Search]

    style EC fill:#e74c3c,color:#fff
    style CC fill:#e67e22,color:#fff
    style NL fill:#9b59b6,color:#fff
    style MX fill:#3498db,color:#fff
    style QT fill:#27ae60,color:#fff
```

**Classification Rules:**

| Query Pattern | BM25 Weight | Semantic Weight | Reason |
|---------------|-------------|-----------------|--------|
| Error codes (ERR_*, E0001) | 0.8 | 0.2 | Exact match critical |
| camelCase/snake_case only | 0.7 | 0.3 | Technical identifier |
| Natural language | 0.25 | 0.75 | Conceptual understanding |
| Mixed (technical + NL) | 0.5 | 0.5 | Balanced approach |
| Quoted "exact phrase" | 0.9 | 0.1 | User wants exact |

### 3.2 Reciprocal Rank Fusion (RRF)

```mermaid
flowchart TB
    subgraph BM25["BM25 Results (weight: 0.35)"]
        B1["1. chunk_A"]
        B2["2. chunk_B"]
        B3["3. chunk_C"]
        B4["4. chunk_D"]
    end

    subgraph Vector["Vector Results (weight: 0.65)"]
        V1["1. chunk_C"]
        V2["2. chunk_A"]
        V3["3. chunk_D"]
        V4["4. chunk_B"]
    end

    subgraph RRF["RRF Formula: score(d) = Σ weight / (k + rank)"]
        Formula["k = 60 (smoothing constant)"]
    end

    subgraph Scores["Score Calculation"]
        SA["chunk_A: 0.00574 + 0.01048 = 0.01622"]
        SC["chunk_C: 0.00556 + 0.01066 = 0.01622"]
        SB["chunk_B: 0.00565 + 0.01016 = 0.01581"]
        SD["chunk_D: 0.00547 + 0.01032 = 0.01579"]
    end

    subgraph Final["Final Ranking"]
        R1["1. chunk_A ≈ chunk_C"]
        R2["2. chunk_B"]
        R3["3. chunk_D"]
    end

    BM25 --> RRF
    Vector --> RRF
    RRF --> Scores --> Final

    style BM25 fill:#3498db,color:#fff
    style Vector fill:#9b59b6,color:#fff
    style RRF fill:#f39c12,color:#fff
    style Final fill:#27ae60,color:#fff
```

```mermaid
sequenceDiagram
    participant Q as Query
    participant BM25 as BM25 Search
    participant Vec as Vector Search
    participant RRF as RRF Fusion
    participant Out as Output

    Q->>BM25: "auth middleware"
    Q->>Vec: embed("auth middleware")

    par Parallel Execution
        BM25-->>RRF: [A:r1, B:r2, C:r3, D:r4]
    and
        Vec-->>RRF: [C:r1, A:r2, D:r3, B:r4]
    end

    Note over RRF: score = Σ w/(60+rank)
    RRF->>RRF: Calculate combined scores
    RRF-->>Out: [A, C, B, D] ranked
```

### 3.3 AST-Based Chunking (cAST Algorithm)

```mermaid
flowchart TB
    subgraph Input["Input: Go Source File"]
        Code["package main<br/>import 'fmt'<br/>type User struct {...}<br/>func (u *User) Greet() string {...}<br/>func main() {...}"]
    end

    subgraph Step1["Step 1: Parse into AST"]
        AST["source_file"]
        AST --> Pkg[package_clause]
        AST --> Imp[import_declaration]
        AST --> Type[type_declaration<br/>User struct]
        AST --> Method[method_declaration<br/>User.Greet]
        AST --> Func[function_declaration<br/>main]
    end

    subgraph Step2["Step 2: Extract Chunks with Context"]
        C1["Chunk 1: User struct<br/>+ package, imports<br/>Symbols: [User: struct]"]
        C2["Chunk 2: User.Greet<br/>+ package, imports, User ref<br/>Symbols: [Greet: method]"]
        C3["Chunk 3: main<br/>+ package, imports<br/>Symbols: [main: function]"]
    end

    subgraph Step3["Step 3: Size Check"]
        Check{Each chunk<br/>< 1500 tokens?}
        Check -->|Yes| Keep[Keep as-is]
        Check -->|No| Split[Recursive split]
    end

    subgraph Step4["Step 4: Merge Small"]
        Merge{Adjacent chunks<br/>< 500 tokens?}
        Merge -->|Yes| Combine[Merge chunks]
        Merge -->|No| Final[Final chunks]
    end

    Input --> Step1
    Type --> C1
    Method --> C2
    Func --> C3
    Step2 --> Step3 --> Step4

    style Input fill:#3498db,color:#fff
    style Step1 fill:#9b59b6,color:#fff
    style Step2 fill:#e67e22,color:#fff
    style Step3 fill:#f39c12,color:#fff
    style Step4 fill:#27ae60,color:#fff
```

```mermaid
graph TD
    subgraph AST["AST Structure"]
        Root[source_file]
        Root --> P[package main]
        Root --> I[import 'fmt']
        Root --> T[type User struct]
        Root --> M[func Greet]
        Root --> F[func main]

        T --> TF1[Name string]
        T --> TF2[Age int]

        M --> MR[receiver: *User]
        M --> MB[body: return ...]

        F --> FB[body: user := ...]
    end

    style Root fill:#e74c3c,color:#fff
    style T fill:#9b59b6,color:#fff
    style M fill:#3498db,color:#fff
    style F fill:#27ae60,color:#fff
```

---

## 4. Storage Design

### 4.1 Storage Directory Structure

```
.amanmcp/
├── vectors.hnsw             # coder/hnsw HNSW index (GOB encoded)
├── vectors.hnsw.meta        # Vector ID mappings (GOB encoded)
├── bm25.db                  # SQLite FTS5 BM25 index
├── metadata.db              # SQLite metadata (chunks, files, symbols)
└── config.yaml              # Project-specific config (if any)
```

### 4.2 Metadata Schema (SQLite)

```sql
-- Projects table
CREATE TABLE projects (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    root_path TEXT NOT NULL UNIQUE,
    project_type TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Files table (for incremental indexing)
CREATE TABLE files (
    id TEXT PRIMARY KEY,
    project_id TEXT NOT NULL,
    path TEXT NOT NULL,
    content_hash TEXT NOT NULL,
    size INTEGER,
    mod_time TIMESTAMP,
    indexed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (project_id) REFERENCES projects(id),
    UNIQUE(project_id, path)
);

-- Chunks table (metadata only, content in vector store)
CREATE TABLE chunks (
    id TEXT PRIMARY KEY,
    file_id TEXT NOT NULL,
    content_type TEXT NOT NULL,
    language TEXT,
    start_line INTEGER,
    end_line INTEGER,
    symbols TEXT,  -- JSON array
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (file_id) REFERENCES files(id)
);

-- Indexes for fast lookups
CREATE INDEX idx_files_project ON files(project_id);
CREATE INDEX idx_files_path ON files(path);
CREATE INDEX idx_chunks_file ON chunks(file_id);
CREATE INDEX idx_chunks_type ON chunks(content_type);
```

### 4.3 Vector Store Design (coder/hnsw)

AmanMCP uses [coder/hnsw](https://github.com/coder/hnsw) — a pure Go HNSW implementation
designed for simplicity and portability. Replaced USearch (CGO) in v0.1.38.

**Why coder/hnsw?**

- **Pure Go** — no CGO, portable binary distribution
- **HNSW algorithm** — scales logarithmically O(log n), not linearly
- **Simple persistence** — GOB encoding for fast save/load
- **Production-tested** — used by Coder for AI features
- **No external deps** — simplifies build and deployment

**Performance Benchmarks** (HNSW with typical embeddings):

| Documents | Query Time | Notes |
|-----------|------------|-------|
| 10,000 | < 1ms | ⚡ Instant |
| 100,000 | ~2-5ms | ⚡ Excellent |
| **300,000** | **~5-10ms** | ✅ **Target scale** |
| 1,000,000 | ~10-20ms | ✅ Scales logarithmically |
| 10,000,000+ | ~20-50ms | ✅ Memory-mapped for large indexes |

```go
// Collection structure
type VectorCollection struct {
    Name       string                 // "chunks"
    Metadata   map[string]interface{} // Collection metadata
    Embeddings []EmbeddingRecord      // Vector records
}

type EmbeddingRecord struct {
    ID        string            // Chunk ID
    Embedding []float32         // 768-dim vector
    Content   string            // Original text (for retrieval)
    Metadata  map[string]string // file_path, language, etc.
}

// Storage format: GOB-encoded for fast serialization
// Persistence: Write on shutdown, load on startup
// Memory: Keep full index in memory (typical: 50-200MB for 100K files)
```

#### 4.3.1 coder/hnsw Integration

```go
import (
    "github.com/coder/hnsw"
)

// Initialize HNSW graph for vector storage
func NewVectorIndex(dimensions int) (*hnsw.Graph[uint64], error) {
    graph := hnsw.NewGraph[uint64]()
    return graph, nil
}

// Add vector to the graph
func (s *HNSWStore) Add(id string, vector []float32) error {
    key := s.getOrCreateKey(id)
    node := hnsw.MakeNode(key, vector)
    s.graph.Add(node)
    return nil
}

// Search for similar vectors
func (s *HNSWStore) Search(vector []float32, k int) ([]VectorResult, error) {
    neighbors := s.graph.Search(vector, k)
    // Convert to VectorResult...
    return results, nil
}

// Persistence via GOB encoding
func (s *HNSWStore) Save(path string) error {
    return s.graph.Export(path)
}

func (s *HNSWStore) Load(path string) error {
    return s.graph.Import(path)
}
```

#### 4.3.2 Vector Store Selection (v0.1.38)

**Why we switched from USearch to coder/hnsw:**

USearch required CGO with dynamic library linking, which caused distribution problems
(BUG-018: CLI binary hung when installed to `~/.local/bin/`). coder/hnsw is pure Go,
eliminating all CGO-related distribution issues.

| Solution | Status | Notes |
|----------|--------|-------|
| **[coder/hnsw](https://github.com/coder/hnsw)** | **Current (v0.1.38+)** | Pure Go, no CGO, portable |
| USearch | Removed (v0.1.38) | CGO issues, see ADR-022 |
| chromem-go | Not used | Linear scan, doesn't scale |

**AmanMCP Default:** coder/hnsw is the vector store since v0.1.38. With 300K+ documents as the target scale,
HNSW's logarithmic scaling ensures sub-10ms queries even at scale.

---

## 5. Performance Considerations

### 5.1 Memory Management

AmanMCP is designed for **typical developer hardware** (16-32GB RAM), not high-end workstations.

#### 5.1.1 Realistic Hardware Profiles

| Developer Setup | Total RAM | Available for AmanMCP | Recommended Settings |
|-----------------|-----------|----------------------|----------------------|
| MacBook Air M2 | 16 GB | ~4-6 GB | `GOGC=100 GOMEMLIMIT=4GiB` |
| **MacBook Pro M4** | **24 GB** | **~8-10 GB** | **`GOGC=100 GOMEMLIMIT=8GiB`** |
| MacBook Pro M4 Max | 32 GB | ~12-16 GB | `GOGC=50 GOMEMLIMIT=12GiB` |
| Linux Workstation | 64 GB+ | ~32+ GB | `GOGC=off GOMEMLIMIT=32GiB` |

**Why these limits?** Typical developer machines run:

- macOS + System: ~4-5 GB
- IDE (VS Code/Cursor): ~2-3 GB
- Browser: ~2-4 GB
- Ollama service: external process (memory varies by model)
- Other apps: ~2-4 GB

#### 5.1.2 Memory Budget (per 100K documents)

| Component | F32 (full) | F16 (default) | I8 (compact) |
|-----------|------------|---------------|--------------|
| Vector embeddings | ~300 MB | **~150 MB** | ~75 MB |
| BM25 inverted index | ~50 MB | ~50 MB | ~50 MB |
| Metadata (SQLite) | ~10 MB | ~10 MB | ~10 MB |
| Tree-sitter parsers | ~20 MB | ~20 MB | ~20 MB |
| Runtime overhead | ~50 MB | ~50 MB | ~50 MB |
| **Total** | **~430 MB** | **~280 MB** | **~205 MB** |

**Default:** F16 quantization — half the memory of F32, negligible quality loss.

#### 5.1.3 Memory-Mapped Index (Critical for Scale)

For 300K+ documents, memory-mapped mode is **essential**:

```go
// Memory-mapped loading — OS manages paging, not Go heap
func LoadIndexView(path string) (*usearch.Index, error) {
    index, err := usearch.NewIndex(usearch.DefaultConfig(768))
    if err != nil {
        return nil, err
    }
    // View() memory-maps the file instead of loading into RAM
    return index, index.View(path)
}
```

| Mode | 300K docs | 24GB System | Notes |
|------|-----------|-------------|-------|
| Load into RAM | ~450 MB heap | ⚠️ Tight | Faster queries |
| **Memory-mapped** | **~50 MB heap** | ✅ **Recommended** | OS pages on demand |

#### 5.1.4 Go Runtime Tuning

```bash
# For 24GB M4 Pro (recommended)
GOGC=100 GOMEMLIMIT=8GiB ./amanmcp serve

# For 16GB systems (conservative)
GOGC=100 GOMEMLIMIT=4GiB ./amanmcp serve

# For 32GB+ systems (aggressive)
GOGC=50 GOMEMLIMIT=12GiB ./amanmcp serve
```

Or programmatically with auto-detection:

```go
import (
    "runtime/debug"
    "github.com/pbnjay/memory"
)

func ConfigureMemory() {
    totalMem := memory.TotalMemory()

    // Reserve 60% for other apps, use 40% for AmanMCP
    limit := int64(float64(totalMem) * 0.4)

    // Cap at 16GB even on large systems
    if limit > 16*1024*1024*1024 {
        limit = 16 * 1024 * 1024 * 1024
    }

    debug.SetMemoryLimit(limit)
    debug.SetGCPercent(100) // Or -1 for GOGC=off on large systems
}
```

### 5.2 Latency Optimization

```mermaid
gantt
    title Query Latency Breakdown: "authentication middleware"
    dateFormat X
    axisFormat %L ms

    section Parsing
    Query parsing     :done, 0, 1
    Classification    :done, 1, 3

    section Search
    Query embedding   :active, 3, 53
    BM25 search       :done, 3, 8
    Vector search     :done, 3, 13

    section Fusion
    Score fusion      :done, 53, 54
    Result format     :done, 54, 55
```

```mermaid
flowchart LR
    subgraph Cold["Cold Query (~70ms)"]
        C1[Parse<br/>1ms] --> C2[Classify<br/>2ms]
        C2 --> C3[Embed<br/>50ms]
        C3 --> C4[Search<br/>10ms]
        C4 --> C5[Fuse<br/>1ms]
        C5 --> C6[Format<br/>1ms]
    end

    subgraph Warm["Warm Query (~20ms)"]
        W1[Parse<br/>1ms] --> W2[Classify<br/>2ms]
        W2 --> W3[Cache Hit<br/>0ms]
        W3 --> W4[Search<br/>10ms]
        W4 --> W5[Fuse<br/>1ms]
        W5 --> W6[Format<br/>1ms]
    end

    Cache[(LRU Cache<br/>Embeddings)]
    C3 -.->|store| Cache
    Cache -.->|hit| W3

    style Cold fill:#e74c3c,color:#fff
    style Warm fill:#27ae60,color:#fff
    style Cache fill:#3498db,color:#fff
```

| Component | Target | Strategy |
|-----------|--------|----------|
| Query parsing | < 1ms | Simple string ops |
| Classification | < 2ms | Pattern matching |
| Query embedding | ~30-50ms | Ollama (cached) |
| BM25 search | < 5ms | Inverted index |
| Vector search | < 10ms | HNSW |
| Score fusion | < 1ms | Simple arithmetic |
| **Total (cold)** | **~70ms** | |
| **Total (warm)** | **~20ms** | Embedding cached |

### 5.3 Indexing Throughput

```mermaid
flowchart LR
    subgraph Pipeline["Indexing Pipeline Throughput"]
        D[File Discovery<br/>10,000+/sec]
        R[File Reading<br/>5,000+/sec]
        P[AST Parsing<br/>2,000+/sec]
        C[Chunking<br/>3,000+/sec]
        E[Embedding<br/>200-500/sec]
        S[Storage<br/>5,000+/sec]
    end

    D --> R --> P --> C --> E --> S

    E -.->|BOTTLENECK| Note["Ollama API inference<br/>bounds throughput"]

    style D fill:#27ae60,color:#fff
    style R fill:#27ae60,color:#fff
    style P fill:#27ae60,color:#fff
    style C fill:#27ae60,color:#fff
    style E fill:#e74c3c,color:#fff
    style S fill:#27ae60,color:#fff
    style Note fill:#f39c12,color:#fff
```

```mermaid
xychart-beta
    title "Indexing Time by Codebase Size"
    x-axis ["1K files", "10K files", "50K files", "100K files"]
    y-axis "Time (minutes)" 0 --> 25
    bar [0.2, 1.5, 8, 18]
```

| Pipeline Stage | Throughput | Strategy |
|----------------|------------|----------|
| File discovery | 10,000+/sec | Concurrent walk |
| File reading | 5,000+/sec | Buffered I/O |
| AST parsing | 2,000+/sec | tree-sitter (fast) |
| Chunking | 3,000+/sec | Simple splitting |
| **Embedding** | **200-500/sec** | **Batch + concurrent (bottleneck)** |
| Storage | 5,000+/sec | Batch writes |

**Effective throughput:** ~100-200 files/sec (embedding-bound)

**Mitigations:**
1. Progress bar with ETA
2. Incremental indexing (changed files only)
3. Background indexing (serve queries while indexing)
4. Priority indexing (frequently-accessed dirs first)

---

## 6. Error Handling & Resilience

### 6.1 Failure Modes

| Failure | Detection | Recovery |
|---------|-----------|----------|
| Model download fails | Network error | Use static embeddings |
| Corrupted index | Checksum mismatch | Rebuild from scratch |
| File read error | OS error | Skip file, log warning |
| Parse error | tree-sitter error | Fall back to line chunking |
| OOM during indexing | Memory threshold | Pause, flush, resume |
| Network timeout | HTTP timeout | Retry with backoff |

### 6.2 Graceful Degradation

```mermaid
flowchart TB
    subgraph Embedding["Embedding Fallback Chain"]
        E1[OllamaEmbedder<br/>Qwen3-0.6B] -->|fail| E2[Static768<br/>Hash-based]
        E2 -->|works| EC[Continue with<br/>reduced quality]

        style E1 fill:#27ae60,color:#fff
        style E2 fill:#f39c12,color:#fff
    end

    subgraph Parsing["Code Parsing Fallback Chain"]
        P1[tree-sitter<br/>AST parsing] -->|fail| P2[Regex-based<br/>extraction]
        P2 -->|fail| P3[Line-based<br/>chunking]
        P3 -->|works| PC[Continue]

        style P1 fill:#27ae60,color:#fff
        style P2 fill:#f39c12,color:#fff
        style P3 fill:#e74c3c,color:#fff
    end

    subgraph Search["Search Fallback Chain"]
        S1[Hybrid Search<br/>BM25 + Vector] -->|vector fail| S2[BM25 Only<br/>Keyword search]
        S2 -->|works| SC[Continue with<br/>keyword results]

        style S1 fill:#27ae60,color:#fff
        style S2 fill:#f39c12,color:#fff
    end

    Principle["Principle: Always return something useful"]
    EC --> Principle
    PC --> Principle
    SC --> Principle

    style Principle fill:#3498db,color:#fff
```

```mermaid
flowchart LR
    subgraph Quality["Quality Levels"]
        Q1["Full Quality<br/>Hybrid + Neural"]
        Q2["Degraded<br/>BM25 + Static"]
        Q3["Minimal<br/>BM25 Only"]
    end

    Q1 -->|Ollama down| Q2
    Q2 -->|Embedding fail| Q3

    style Q1 fill:#27ae60,color:#fff
    style Q2 fill:#f39c12,color:#fff
    style Q3 fill:#e74c3c,color:#fff
```

---

## 7. Security Model

### 7.1 Threat Model

```mermaid
flowchart TB
    subgraph Trusted["TRUSTED ZONE (User's Machine)"]
        Claude[Claude Code<br/>Client]
        AmanMCP[AmanMCP<br/>Server]
        FS[(File System<br/>Project)]
        Ollama[Ollama<br/>localhost:11434]

        Claude <-->|MCP Protocol| AmanMCP
        AmanMCP --> FS
        AmanMCP <-->|localhost only| Ollama
    end

    subgraph Untrusted["UNTRUSTED ZONE (External Network)"]
        Internet((Internet))
        Cloud[Cloud Services]
        Telemetry[Telemetry]
    end

    AmanMCP x--x|"No outbound"| Internet
    AmanMCP x--x|"No cloud sync"| Cloud
    AmanMCP x--x|"No telemetry"| Telemetry

    style Trusted fill:#d5f4e6,color:#000
    style Untrusted fill:#fadbd8,color:#000
    style Claude fill:#3498db,color:#fff
    style AmanMCP fill:#27ae60,color:#fff
    style FS fill:#9b59b6,color:#fff
    style Ollama fill:#e67e22,color:#fff
```

```mermaid
flowchart LR
    subgraph Privacy["Privacy Guarantees"]
        P1["100% Local<br/>No internet after install"]
        P2["No Telemetry<br/>Zero data collection"]
        P3["No Cloud<br/>Code stays on machine"]
    end

    style P1 fill:#27ae60,color:#fff
    style P2 fill:#27ae60,color:#fff
    style P3 fill:#27ae60,color:#fff
```

### 7.2 Sensitive File Handling

```go
// Default patterns to exclude from indexing
var SensitivePatterns = []string{
    // Environment and secrets
    ".env*",
    "*.pem",
    "*.key",
    "*.p12",
    "*.pfx",
    "*credentials*",
    "*secrets*",
    "*password*",

    // Cloud provider configs
    ".aws/*",
    ".gcp/*",
    ".azure/*",

    // SSH and auth
    ".ssh/*",
    ".netrc",
    ".npmrc",
    ".pypirc",

    // Database files
    "*.sqlite",
    "*.db",
    "*.sql",

    // IDE and local configs
    ".idea/*",
    ".vscode/*",
    "*.local.*",
}
```

---

## 8. Extensibility

### 8.1 Plugin Architecture (Future)

```mermaid
classDiagram
    class Chunker {
        <<interface>>
        +Name() string
        +Extensions() []string
        +Chunk(path, content) []Chunk, error
    }

    class Embedder {
        <<interface>>
        +Name() string
        +Dimensions() int
        +Embed(text) []float32, error
    }

    class Searcher {
        <<interface>>
        +Name() string
        +Search(query, opts) []Result, error
    }

    class PluginManager {
        +LoadPlugins(dir string)
        +RegisterChunker(c Chunker)
        +RegisterEmbedder(e Embedder)
        +RegisterSearcher(s Searcher)
    }

    PluginManager --> Chunker : loads
    PluginManager --> Embedder : loads
    PluginManager --> Searcher : loads
```

```mermaid
flowchart TB
    subgraph PluginSystem["Plugin System (v2.0)"]
        subgraph Types["Plugin Types"]
            Chunkers["Chunkers<br/>Add language support"]
            Embedders["Embedders<br/>Add embedding providers"]
            Searchers["Searchers<br/>Add search strategies"]
        end

        subgraph Discovery["Plugin Discovery"]
            Dir["~/.amanmcp/plugins/"]
            Startup["Auto-load at startup"]
            Format["Format: .so/.dylib or gRPC"]
        end
    end

    Types --> Discovery

    style Chunkers fill:#3498db,color:#fff
    style Embedders fill:#9b59b6,color:#fff
    style Searchers fill:#27ae60,color:#fff
```

### 8.2 Language Support Matrix

| Language | Phase 1 | Phase 2 | Method |
|----------|---------|---------|--------|
| Go | ✅ | - | tree-sitter |
| TypeScript | ✅ | - | tree-sitter |
| JavaScript | ✅ | - | tree-sitter |
| Python | ✅ | - | tree-sitter |
| Markdown | ✅ | - | Custom parser |
| HTML | - | ✅ | tree-sitter |
| CSS | - | ✅ | tree-sitter |
| React/JSX | ✅ | - | tree-sitter (tsx) |
| React Native | - | ✅ | tree-sitter (tsx) |
| Next.js | - | ✅ | tree-sitter + special handling |
| Vue | - | ✅ | tree-sitter |
| SQL | - | ✅ | tree-sitter |
| JSON | - | ✅ | Built-in |
| YAML | - | ✅ | Built-in |

---

## 9. Testing Strategy

### 9.1 Test Pyramid

```mermaid
%%{init: {'theme': 'base', 'themeVariables': { 'primaryColor': '#3498db'}}}%%
flowchart TB
    subgraph Pyramid["Test Pyramid"]
        E2E["E2E Tests<br/>MCP protocol compliance<br/>Full server tests"]
        Integration["Integration Tests<br/>Component interactions<br/>Database operations"]
        Unit["Unit Tests<br/>Chunkers, BM25, Fusion<br/>Edge cases"]
    end

    E2E --> Integration --> Unit

    style E2E fill:#e74c3c,color:#fff
    style Integration fill:#f39c12,color:#fff
    style Unit fill:#27ae60,color:#fff
```

```mermaid
pie showData
    title "Test Distribution Target"
    "Unit Tests" : 70
    "Integration Tests" : 20
    "E2E Tests" : 10
```

### 9.2 Benchmark Suite

```go
// Performance benchmarks
func BenchmarkSearch_SmallIndex(b *testing.B)    // 1K files
func BenchmarkSearch_MediumIndex(b *testing.B)   // 10K files
func BenchmarkSearch_LargeIndex(b *testing.B)    // 100K files

func BenchmarkIndexing_GoProject(b *testing.B)   // Go codebase
func BenchmarkIndexing_NodeProject(b *testing.B) // Node.js project

func BenchmarkChunking_LargeFile(b *testing.B)   // 10K line file
func BenchmarkEmbedding_Batch(b *testing.B)      // 100 texts
```

---

## 10. Deployment Considerations

### 10.1 Build Matrix

| OS | Arch | CGO | Status |
|----|------|-----|--------|
| macOS | amd64 | Yes | Primary |
| macOS | arm64 | Yes | Primary |
| Linux | amd64 | Yes | Primary |
| Linux | arm64 | Yes | Secondary |
| Windows | amd64 | Yes | Community |

### 10.2 Release Checklist

1. **Pre-release**
   - [ ] All tests pass
   - [ ] Benchmarks within targets
   - [ ] CHANGELOG updated
   - [ ] Version bumped

2. **Build**
   - [ ] GoReleaser build
   - [ ] Cross-platform binaries
   - [ ] Homebrew formula

3. **Publish**
   - [ ] GitHub Release
   - [ ] Homebrew tap update
   - [ ] Documentation update

4. **Verification**
   - [ ] macOS smoke test
   - [ ] Linux smoke test
   - [ ] MCP integration test

---

## Appendix A: Technology Validation

For comprehensive validation of all technology choices against 2025 industry best practices, see:

**[Technology Validation Report (2026)](./technology-validation-2026.md)**

This document validates each component choice with grounded research from 20+ industry sources, including:
- Embedding backend comparison (Ollama vs vLLM vs TEI)
- Vector store evaluation (Pure Go HNSW vs CGO alternatives)
- Hybrid search strategy validation (RRF vs linear combination)
- Code parsing approach (tree-sitter AST vs alternatives)

---

## Appendix B: Decision Log

| Decision | Options Considered | Choice | Rationale |
|----------|-------------------|--------|-----------|
| Vector DB | Qdrant, Milvus, chromem-go, USearch, coder/hnsw | [coder/hnsw](https://github.com/coder/hnsw) (v0.1.38+) | Pure Go, no CGO, portable binary (ADR-022) |
| Previous Vector DB | USearch | Removed in v0.1.38 | CGO caused distribution issues (BUG-018) |
| Embeddings | OpenAI, Ollama, Hugot, gollama.cpp | Ollama API (OllamaEmbedder) + Static768 fallback | HTTP API, Metal GPU via Ollama, dimension-compatible fallback |
| Embedding Model | Qwen3-embedding, nomic-embed-text | [Qwen3-embedding](https://huggingface.co/Qwen/Qwen3-Embedding-8B) (recommended) | #1 MTEB, 32K context, via Ollama |
| Code parsing | Regex, custom, tree-sitter | [tree-sitter](https://github.com/tree-sitter/go-tree-sitter) | Battle-tested, 40+ languages, official Go bindings |
| MCP SDK | Custom, mcp-go, official | [Official Go SDK](https://github.com/modelcontextprotocol/go-sdk) | Maintained by Google & Anthropic, stable since July 2025 |
| Search fusion | Linear, RRF, learned | RRF | Simple, effective, no training |
| Storage | JSON, SQLite, custom | SQLite + GOB | Best of both: queries + speed |
| CLI framework | flag, cobra, urfave | cobra | Widely used, good UX |

---

*Document maintained by AmanMCP Team. Last updated: 2026-01-03*
