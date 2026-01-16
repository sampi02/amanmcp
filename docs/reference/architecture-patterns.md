# Architecture Patterns Reference

> **Learning Objectives:**
> - Understand the high-level architecture of hybrid search systems
> - Learn key design patterns for RAG applications
> - Know module boundaries and interface designs
>
> **Prerequisites:**
> - [Hybrid Search Concepts](../concepts/hybrid-search.md)
>
> **Audience:** Contributors, architects, developers building similar systems

---

## TL;DR

AmanMCP uses a modular architecture with clear interface boundaries. The core pattern is: Client -> MCP Handler -> Search Engine -> (BM25 + Vector parallel) -> RRF Fusion -> Results. Each component is swappable via interfaces, enabling testing, experimentation, and graceful degradation.

---

## Data Flow

```
Client -> MCP Handler -> Search Engine -> (BM25 + Vector parallel) -> RRF Fusion -> Results
```

### Stage-by-Stage Breakdown

| Stage | Component | Responsibility |
|-------|-----------|----------------|
| 1. **Client** | Claude Code, Cursor | Send MCP tool calls (`search_code`, `search_docs`) |
| 2. **MCP Handler** | `internal/mcp/` | Parse MCP protocol, route to appropriate tool handler |
| 3. **Search Engine** | `internal/search/engine.go` | Coordinate hybrid search, classify queries |
| 4. **BM25 Search** | `internal/search/bm25.go` | Keyword-based retrieval (SQLite FTS5) |
| 5. **Vector Search** | `internal/search/vector.go` | Semantic retrieval (HNSW) |
| 6. **RRF Fusion** | `internal/search/fusion.go` | Combine ranked results |
| 7. **Results** | `internal/models/result.go` | Formatted search results |

---

## Key Design Decisions

### ADR-001: Hybrid Search (BM25 + Semantic)

**Decision:** Use both BM25 and semantic search, fused with RRF.

**Why neither alone is sufficient:**

| Query Type | BM25 Strength | Vector Strength |
|------------|---------------|-----------------|
| `ERR_CONNECTION_REFUSED` | Exact match | May miss exact identifiers |
| "how does auth work" | Too literal | Finds conceptual matches |
| `handleUserLogin` | Exact identifier | Understands intent |
| "security best practices" | Needs exact terms | Conceptual understanding |

**Weights:** BM25 0.35, Semantic 0.65 (tuned empirically for code search)

**See Also:** [RRF Fusion Rationale](../research/rrf-fusion-rationale.md)

---

### ADR-002: Chunk as Primitive

**Decision:** `Chunk` is the core data type flowing through the system.

**Why:** A single primitive enables composition. All indexers, searchers, and embedders operate on Chunks, making the system pluggable at every layer.

```go
type Chunk struct {
    ID       string      // Unique identifier (hash of content + path)
    FilePath string      // Absolute path to source file
    Content  string      // The actual code/documentation content
    Type     ChunkType   // Code, Documentation, Comment
    Language string      // go, typescript, python, markdown
    Lines    LineRange   // Start and end line numbers
    Symbols  []string    // Function names, class names, etc.
}
```

**Benefits:** Testability, composability, cacheability, debuggability.

**Pipeline:**

```
Source File -> TreeSitter -> []Chunk -> Embedder -> HNSW Index
                                     -> BM25 Index
                                     -> SQLite Metadata
```

---

### ADR-003: Graceful Degradation

**Decision:** If any component fails, fall back to reduced functionality rather than crash.

**Why:** User experience matters more than theoretical purity. BM25-only results are infinitely better than an error message.

**Fallback Chains:**

| Component | Primary | Fallback 1 | Fallback 2 |
|-----------|---------|------------|------------|
| Embedder | Ollama (Qwen3) | Static768 (hash-based) | BM25-only mode |
| Code Parsing | tree-sitter AST | Regex extraction | Line chunking |
| Search | Hybrid (BM25 + Vector) | BM25-only | File listing |
| BM25 Index | SQLite FTS5 | Auto-rebuild | Memory fallback |

**Principle:** Always return something useful.

---

## Module Boundaries (Black Box Design)

The architecture follows [Eskil Steenberg's Black Box Design](../articles/black-box-architecture-case-study.md) principles: modules are replaceable black boxes with clean interfaces.

### Core Interfaces

| Module | Interface | Purpose |
|--------|-----------|---------|
| **Embedder** | `Embed(text string) ([]float32, error)` | Convert text to vector |
| **Indexer** | `Index(chunks []Chunk) error` | Add chunks to storage |
| **Searcher** | `Search(query Query) ([]Result, error)` | Retrieve matching chunks |
| **Chunker** | `Chunk(path, content string) ([]Chunk, error)` | Split content into chunks |

### Implementations

```
internal/embed/
  |- types.go       # Embedder interface definition
  |- ollama.go      # OllamaEmbedder - primary
  |- static768.go   # Static768Embedder - fallback

internal/search/
  |- engine.go      # Search engine coordinator
  |- bm25.go        # BM25 (SQLite FTS5)
  |- vector.go      # Vector search (coder/hnsw)
  |- fusion.go      # RRF score fusion

internal/chunk/
  |- code.go        # AST-based code chunker
  |- treesitter.go  # tree-sitter integration
  |- markdown.go    # Markdown chunker
```

### Why Black Box Design?

| Benefit | Example |
|---------|---------|
| **Testability** | Mock `Embedder` interface for unit tests |
| **Experimentation** | Benchmark Tantivy vs SQLite FTS5 by swapping implementations |
| **Resilience** | Fallback from Ollama to Static768 without code changes |
| **Evolution** | Replace HNSW library without touching search logic |

---

## Patterns in Use

| Pattern | Where | Purpose |
|---------|-------|---------|
| **Interface-based modules** | `internal/embed/`, `internal/search/` | Swap implementations |
| **Graceful degradation** | BM25 auto-recovery, embedder fallback | Never crash on failure |
| **Progressive disclosure** | CLAUDE.md -> Skills | Load context on demand |
| **Parallel execution** | Search engine dual-path | Concurrent BM25 + Vector |
| **Composition over inheritance** | HybridIndexer wraps components | Combine without hierarchies |
| **Single primitive** | Chunk as universal data type | Unified data model |

### Pattern: Composition over Inheritance

```go
// BAD: Monolithic engine that knows everything
type MonolithicEngine struct {
    bm25Store     *SQLiteStore
    vectorStore   *HNSWStore
    ollamaClient  *http.Client
}

// GOOD: Composed from interfaces
type HybridEngine struct {
    bm25     Searcher  // Could be SQLite, Tantivy, or mock
    vector   Searcher  // Could be HNSW, Milvus, or mock
    embedder Embedder  // Could be Ollama, MLX, or static
    fusion   Fusioner  // Could be RRF, linear, or learned
}
```

### Pattern: Query Classification

Dynamically adjust weights based on query characteristics:

| Query Pattern | BM25 Weight | Vector Weight | Reason |
|---------------|-------------|---------------|--------|
| Error codes (ERR_*) | 0.8 | 0.2 | Exact match critical |
| camelCase/snake_case | 0.7 | 0.3 | Technical identifier |
| Natural language | 0.25 | 0.75 | Conceptual understanding |
| "exact phrase" | 0.9 | 0.1 | User wants exact |
| Default | 0.35 | 0.65 | Balanced approach |

---

## Known Gotchas

### 1. Dimension Mismatch

**Problem:** Switching embedding backends requires reindexing.

**Why:** Ollama Qwen3 produces 768-dim vectors. A 384-dim model makes the HNSW index invalid.

**Solution:** Store embedder metadata in index. Detect mismatch on startup and trigger reindex.

### 2. Go Method Syntax

**Problem:** Semantic search struggles with `func (s *Service)` receiver syntax.

**Why:** Embedding models are trained on natural language. Go's receiver syntax is less common in training data.

**Mitigation:** Query classifier boosts BM25 weight for camelCase/PascalCase patterns.

### 3. Large Files

**Problem:** Files > 10MB can cause memory issues during chunking.

**Solution:** Skip files exceeding threshold. Log warning for visibility.

### 4. CGO and Binary Distribution

**Problem:** CGO dependencies complicate binary distribution.

**Solution:**
- tree-sitter: Required CGO, stable and well-tested
- Vector store: Switched from USearch (CGO) to coder/hnsw (pure Go) in v0.1.38

### 5. Cold Start Latency

**Problem:** First query is slow (~70ms vs ~20ms warm).

**Why:** Embedding requires Ollama inference on first call.

**Mitigation:** Query embedding cache. Common queries pre-warmed at startup.

---

## Architecture Evolution

| Version | Change | Rationale |
|---------|--------|-----------|
| v0.1.0 | Initial hybrid search | Combine BM25 precision with semantic recall |
| v0.1.20 | Interface extraction | Enable testing and experimentation |
| v0.1.38 | USearch -> coder/hnsw | Eliminate CGO distribution issues |
| v0.2.0 | Query classification | Optimize weights per query type |
| v0.4.0 | SQLite FTS5 BM25 | Battle-tested FTS5 implementation |

---

## Configuration Principles

Architecture decisions should not be hardcoded. Tunable values live in config:

```yaml
# .amanmcp.yaml
search:
  bm25_weight: 0.35
  semantic_weight: 0.65
  rrf_k: 60

indexing:
  max_file_size: 10485760  # 10MB
  batch_size: 100

embedding:
  provider: ollama
  model: qwen3-embedding
```

**Principle:** Data-driven behavior, not hardcoded values.

---

## Performance Targets

| Metric | Target | Measured |
|--------|--------|----------|
| Query latency (warm) | < 20ms | ~15ms |
| Query latency (cold) | < 100ms | ~70ms |
| Memory usage | < 300MB | ~280MB @ 100K docs |
| Startup time | < 2s | ~1.2s |
| Index throughput | > 100 files/sec | ~150 files/sec |

---

## See Also

- [Black Box Architecture Case Study](../articles/black-box-architecture-case-study.md) - Detailed refactoring example
- [RRF Fusion Rationale](../research/rrf-fusion-rationale.md) - Why RRF over alternatives
- [Hybrid Search Concepts](../concepts/hybrid-search.md) - How BM25 and vector search work
- [Technology Validation 2026](architecture/technology-validation-2026.md) - Component validation

---

**Original Source:** `.aman-pm/memory-bank/system-patterns.md` (internal)
**Last Updated:** 2026-01-16
