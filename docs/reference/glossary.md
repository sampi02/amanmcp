# Glossary

Definitions of terms used throughout AmanMCP documentation and code.

```mermaid
graph TD
    subgraph "Search & Retrieval"
        S1[Hybrid Search]
        S2[BM25]
        S3[Vector/Embeddings]
        S4[RRF Fusion]
        S1 --> S2
        S1 --> S3
        S2 --> S4
        S3 --> S4
    end

    subgraph "Code Parsing"
        C1[Tree-sitter]
        C2[AST]
        C3[Chunks]
        C1 --> C2
        C2 --> C3
    end

    subgraph "MCP Protocol"
        M1[Server]
        M2[Tools]
        M3[Resources]
        M4[JSON-RPC/stdio]
        M1 --> M2
        M1 --> M3
        M1 --> M4
    end

    subgraph "ML Components"
        ML1[Models]
        ML2[Embeddings]
        ML3[Inference]
        ML1 --> ML2
        ML2 --> ML3
    end

    C3 --> S3
    ML3 --> S3

    style S1 fill:#e1f5ff
    style S4 fill:#c8e6c9
    style C1 fill:#e1f5ff
    style C3 fill:#c8e6c9
    style M1 fill:#e1f5ff
    style M4 fill:#c8e6c9
    style ML2 fill:#e1f5ff
    style ML3 fill:#c8e6c9
```

---

## Search & Retrieval

| Term | Definition |
|------|------------|
| **BM25** | Best Match 25. A ranking function for keyword search based on term frequency and inverse document frequency. |
| **Chunk** | A semantic unit of code (function, class, method) extracted for indexing. Smaller than a file, larger than a line. |
| **Cosine Similarity** | Measure of similarity between two vectors based on the angle between them. Range: -1 to 1. |
| **Embedding** | A dense vector representation of text that captures semantic meaning. Typically 384-1536 dimensions. |
| **HNSW** | Hierarchical Navigable Small World. Graph-based algorithm for approximate nearest neighbor search. O(log n) complexity. |
| **Hybrid Search** | Combining multiple search methods (e.g., BM25 + vector) for better results. |
| **IDF** | Inverse Document Frequency. Weights rare terms higher than common ones. |
| **RAG** | Retrieval-Augmented Generation. Pattern where AI retrieves relevant context before generating responses. |
| **RRF** | Reciprocal Rank Fusion. Algorithm to combine ranked results from multiple sources. Formula: 1/(k + rank). |
| **Semantic Search** | Finding documents by meaning rather than exact keyword match. Uses embeddings. |
| **TF** | Term Frequency. How often a term appears in a document. |
| **Vector Database** | Database optimized for storing and querying high-dimensional vectors. |

### Search Concepts Relationship Diagram

This diagram shows how the core search concepts interact in AmanMCP's hybrid search architecture:

```mermaid
graph TB
    subgraph Query["Query Processing"]
        Q[User Query:<br/>'authentication middleware']
        QC[Query Classifier]
    end

    subgraph BM25Path["BM25 Keyword Search Path"]
        BM25[BM25 Ranker]
        TF[Term Frequency<br/>TF calculation]
        IDF[Inverse Document<br/>Frequency IDF]
        InvertedIdx[(Inverted Index<br/>SQLite FTS5)]
    end

    subgraph VectorPath["Semantic Vector Search Path"]
        Embed[Embedding Model<br/>Text → Vector]
        Vec[768-dim Vector]
        HNSW[HNSW Index<br/>Approximate NN]
        Cosine[Cosine Similarity<br/>Distance metric]
    end

    subgraph Fusion["Result Fusion"]
        RRF["RRF Algorithm<br/>score = #931; w/(k+rank)"]
        Hybrid[Hybrid Results<br/>Best of both]
    end

    Q --> QC
    QC -->|Weight: 0.35| BM25
    QC -->|Weight: 0.65| Embed

    BM25 --> TF
    BM25 --> IDF
    TF --> InvertedIdx
    IDF --> InvertedIdx
    InvertedIdx -->|Ranked Results| RRF

    Embed --> Vec
    Vec --> HNSW
    HNSW --> Cosine
    Cosine -->|Ranked Results| RRF

    RRF --> Hybrid

    style Q fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style BM25 fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style TF fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style IDF fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style InvertedIdx fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style Embed fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style Vec fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style HNSW fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style Cosine fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style RRF fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style Hybrid fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
```

**Key Relationships:**

- **BM25** combines TF (how often terms appear) with IDF (how rare terms are)
- **Semantic Search** uses embeddings and HNSW for approximate nearest neighbor search
- **RRF** fuses both paths using reciprocal rank scoring
- **Hybrid Search** balances keyword precision with semantic understanding

---

## Code Parsing

| Term | Definition |
|------|------------|
| **AST** | Abstract Syntax Tree. Tree representation of code structure. |
| **CGO** | Go's foreign function interface for calling C code. Required for tree-sitter. |
| **Grammar** | Rules defining the syntax of a programming language. Used by parsers. |
| **Node** | Element in an AST representing a syntactic construct (function, class, statement). |
| **Parser** | Tool that converts source code text into an AST. |
| **Tree-sitter** | Incremental parsing library used for code understanding. Supports 100+ languages. |

### Code Parsing Flow Visualization

This diagram shows how AmanMCP processes source code from files to searchable chunks using tree-sitter:

```mermaid
flowchart TB
    subgraph Input["Input Stage"]
        File["Source File<br/>main.go"]
        Content["Raw Code Text<br/>func main..."]
    end

    subgraph Detection["Language Detection"]
        Detect{"Detect Language<br/>by Extension"}
        GoLang["Go Language"]
        TypeScript["TypeScript/JavaScript"]
        Python["Python Language"]
        Markdown["Markdown"]
    end

    subgraph Parsing["Tree-sitter Parsing (CGO)"]
        Grammar["Load Grammar<br/>language-specific"]
        Parser["tree-sitter Parser<br/>C library via CGO"]
        AST["Abstract Syntax Tree<br/>AST Nodes"]
    end

    subgraph Traversal["AST Traversal"]
        Root["Root Node<br/>source_file"]
        Functions["Function Nodes<br/>function_declaration"]
        Classes["Type/Class Nodes<br/>type_declaration"]
        Methods["Method Nodes<br/>method_declaration"]
    end

    subgraph Chunking["Chunk Extraction"]
        Extract["Extract Semantic Units"]
        Context["Add Context<br/>imports, package, types"]
        Symbols["Extract Symbols<br/>function names, types"]
        Lines["Record Line Numbers<br/>start, end"]
    end

    subgraph Output["Output Stage"]
        Chunks["Chunks Array<br/>[]Chunk"]
        Chunk1["Chunk 1:<br/>func Authenticate"]
        Chunk2["Chunk 2:<br/>type User struct"]
        Chunk3["Chunk 3:<br/>func main"]
    end

    File --> Content
    Content --> Detect

    Detect -->|".go"| GoLang
    Detect -->|".ts/.js"| TypeScript
    Detect -->|".py"| Python
    Detect -->|".md"| Markdown

    GoLang --> Grammar
    TypeScript --> Grammar
    Python --> Grammar
    Markdown -->|Skip tree-sitter| Extract

    Grammar --> Parser
    Parser --> AST
    AST --> Root

    Root --> Functions
    Root --> Classes
    Root --> Methods

    Functions --> Extract
    Classes --> Extract
    Methods --> Extract

    Extract --> Context
    Context --> Symbols
    Symbols --> Lines
    Lines --> Chunks

    Chunks --> Chunk1
    Chunks --> Chunk2
    Chunks --> Chunk3

    style File fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style Content fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style Detect fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style GoLang fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style TypeScript fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style Python fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style Markdown fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style Grammar fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style Parser fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style AST fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style Root fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style Functions fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style Classes fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style Methods fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style Extract fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style Context fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style Symbols fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style Lines fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style Chunks fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style Chunk1 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style Chunk2 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style Chunk3 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
```

**Processing Stages:**

1. **Input**: Read source file and raw code text
2. **Detection**: Identify language by file extension
3. **Parsing**: Use tree-sitter grammar to build AST (requires CGO)
4. **Traversal**: Walk AST nodes to find semantic units (functions, classes, methods)
5. **Chunking**: Extract chunks with context (imports, symbols, line numbers)
6. **Output**: Array of searchable chunks ready for embedding and indexing

**Why tree-sitter?** Language-aware chunking respects code structure, unlike naive line-based splitting.

---

## MCP (Model Context Protocol)

| Term | Definition |
|------|------------|
| **Client** | Application that connects to MCP servers (e.g., Claude Code, Cursor). |
| **JSON-RPC** | JSON Remote Procedure Call. Protocol used for MCP communication. |
| **MCP** | Model Context Protocol. Open protocol for AI-tool integration. |
| **Prompt** | Reusable template exposed by MCP server. |
| **Resource** | Data that an MCP server makes available (files, database entries). |
| **Server** | Application that exposes tools/resources via MCP (AmanMCP). |
| **stdio** | Standard input/output. Transport mechanism for MCP communication. |
| **Tool** | Function that an AI can call via MCP (e.g., search, lookup). |

### MCP Architecture Client-Server Diagram

This diagram illustrates the MCP protocol flow between clients and the AmanMCP server:

```mermaid
sequenceDiagram
    participant CC as "Client<br/>(Claude Code)"
    participant Stdio as "stdio Transport<br/>(JSON-RPC)"
    participant MCP as "MCP Server<br/>(AmanMCP)"
    participant Tools as "Tool Router"
    participant Engine as "Search Engine"

    Note over CC,Engine: 1. Initialization Phase
    CC->>Stdio: Initialize connection
    Stdio->>MCP: initialize request
    MCP->>MCP: Load capabilities
    MCP-->>Stdio: server info + tools list
    Stdio-->>CC: Available tools:<br/>search_code, search_docs, search

    Note over CC,Engine: 2. Tool Call Phase
    CC->>Stdio: call tool: search_code<br/>params: {query: "auth middleware"}
    Stdio->>MCP: JSON-RPC tool request
    MCP->>Tools: Route to search_code handler
    Tools->>Engine: Execute hybrid search
    Engine->>Engine: BM25 + Vector + RRF Fusion
    Engine-->>Tools: Ranked results [10]
    Tools-->>MCP: Format as MCP response
    MCP-->>Stdio: JSON-RPC result
    Stdio-->>CC: Search results with metadata

    Note over CC,Engine: 3. Resource Access Phase (Optional)
    CC->>Stdio: read resource: file://path/to/code.go
    Stdio->>MCP: resource request
    MCP->>MCP: Read file content
    MCP-->>Stdio: File content + metadata
    Stdio-->>CC: Full file context

    Note over CC,Engine: 4. Shutdown Phase
    CC->>Stdio: Close connection
    Stdio->>MCP: shutdown notification
    MCP->>MCP: Cleanup resources
    MCP-->>Stdio: shutdown complete
```

**Key Interactions:**

- **stdio Transport**: MCP uses standard input/output for process communication
- **JSON-RPC**: All messages are formatted as JSON-RPC 2.0 requests/responses
- **Tool Calls**: Client invokes server tools (search_code, search_docs, search)
- **Resources**: Server exposes file contents and project metadata
- **Stateful Connection**: Single process lifecycle from init to shutdown

---

## Machine Learning

| Term | Definition |
|------|------------|
| **Dimension** | Number of values in an embedding vector. Higher = more nuance, more memory. |
| **F16** | Half-precision floating point. 16 bits per value. Half memory of F32. |
| **F32** | Single-precision floating point. 32 bits per value. Standard precision. |
| **I8** | 8-bit integer quantization. Quarter memory of F32, some quality loss. |
| **Inference** | Running a trained model to get predictions/embeddings. |
| **Model** | Trained neural network that performs a specific task. |
| **Quantization** | Reducing precision of numbers to save memory (F32 → F16 → I8). |
| **Transformer** | Neural network architecture used in modern embedding models. |

### Embedding Model Inference Flow

This diagram shows how AmanMCP converts code text into vector embeddings for semantic search:

```mermaid
flowchart LR
    subgraph Input["Text Input"]
        Code["Code Chunk<br/>func authenticate(...)"]
    end

    subgraph Embedder["Embedding Backend"]
        Backend{Backend<br/>Selection}
        Ollama[Ollama API<br/>qwen3-embedding]
        MLX[MLX Server<br/>Apple Silicon]
        Static[Static768<br/>Fallback]
    end

    subgraph Model["Neural Network"]
        Tokenize[Tokenizer<br/>Text → Tokens]
        Transform[Transformer Layers<br/>BERT/GPT Architecture]
        Pool[Pooling Layer<br/>Reduce to fixed dims]
    end

    subgraph Output["Vector Output"]
        Vec768["768-dim F32 Vector<br/>[0.12, -0.45, 0.89, ...]"]
        VecQuantized["Quantized Vector<br/>F16 or I8"]
    end

    Code --> Backend
    Backend -->|Primary| Ollama
    Backend -->|Apple Silicon| MLX
    Backend -->|Fallback| Static

    Ollama --> Tokenize
    MLX --> Tokenize
    Static --> VecQuantized

    Tokenize --> Transform
    Transform --> Pool
    Pool --> Vec768
    Vec768 -->|Optional| VecQuantized

    style Code fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style Backend fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style Ollama fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style MLX fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style Static fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style Tokenize fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style Transform fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style Pool fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style Vec768 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style VecQuantized fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
```

**Inference Pipeline:**

- **Backend Selection**: Auto-detects best available embedder (MLX > Ollama > Static)
- **Tokenization**: Converts text into tokens (subword units)
- **Transformer**: Processes tokens through neural network layers
- **Pooling**: Reduces variable-length token embeddings to fixed dimensions
- **Quantization**: Optional precision reduction (F32 → F16 → I8) for memory savings

### Quantization Quality vs Memory Tradeoff

```mermaid
graph LR
    subgraph Precision["Precision Levels"]
        F32[F32<br/>32-bit float<br/>4 bytes/value]
        F16[F16<br/>16-bit float<br/>2 bytes/value]
        I8[I8<br/>8-bit integer<br/>1 byte/value]
    end

    subgraph Memory["Memory Usage (768-dim)"]
        M32["3072 bytes<br/>100% baseline"]
        M16["1536 bytes<br/>50% reduction"]
        M8["768 bytes<br/>75% reduction"]
    end

    subgraph Quality["Search Quality"]
        Q32["100% quality<br/>No loss"]
        Q16["~99% quality<br/>Minimal loss"]
        Q8["~95% quality<br/>Small degradation"]
    end

    F32 --> M32 --> Q32
    F16 --> M16 --> Q16
    I8 --> M8 --> Q8

    style F32 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style F16 fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style I8 fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style M32 fill:#ffccbc,stroke:#e74c3c,stroke-width:2px
    style M16 fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style M8 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style Q32 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style Q16 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style Q8 fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
```

**Tradeoff Analysis:**

- **F32 (Default)**: Maximum quality, highest memory, standard precision
- **F16 (Recommended)**: Near-identical quality, 50% less memory, negligible loss
- **I8 (Aggressive)**: Good quality, 75% less memory, small quality degradation

**AmanMCP Default**: F16 for optimal balance (see `performance.quantization` in config)

---

## Development Process

| Term | Definition |
|------|------------|
| **AC** | Acceptance Criteria. Testable conditions that define "done" for a feature. |
| **ADR** | Architecture Decision Record. Document explaining why a decision was made. |
| **CI** | Continuous Integration. Automated testing on code changes. |
| **Feature** | Discrete unit of functionality with ID (F01, F02, etc.). |
| **Phase** | Group of related features implemented together. |
| **RCA** | Root Cause Analysis. Post-mortem document for incidents. |
| **RFC** | Request for Comments. Proposal for significant changes. |
| **SSOT** | Single Source of Truth. One authoritative location for each piece of information. |
| **TDD** | Test-Driven Development. Write tests before implementation. |

### TDD Workflow (Red-Green-Refactor)

This diagram shows AmanMCP's test-driven development cycle:

```mermaid
flowchart LR
    subgraph Red["RED Phase"]
        R1[Write Failing Test]
        R2[Verify Test Fails<br/>for right reason]
        R1 --> R2
    end

    subgraph Green["GREEN Phase"]
        G1[Write Minimal Code<br/>to pass test]
        G2[Run Tests]
        G3{All Tests Pass?}
        G1 --> G2 --> G3
        G3 -->|No| G1
    end

    subgraph Refactor["REFACTOR Phase"]
        RF1[Improve Code Quality]
        RF2[Run Tests Again]
        RF3{Tests Still Pass?}
        RF4[make ci-check]
        RF5{CI Pass?}
        RF1 --> RF2 --> RF3
        RF3 -->|Yes| RF4 --> RF5
        RF3 -->|No| RF1
        RF5 -->|No| RF1
    end

    subgraph Done["Completion"]
        D1[Commit Changes]
        D2[Update Changelog]
    end

    R2 --> G1
    G3 -->|Yes| RF1
    RF5 -->|Yes| D1 --> D2

    D2 --> Next{More Features?}
    Next -->|Yes| R1
    Next -->|No| Complete([Done])

    style R1 fill:#ffccbc,stroke:#e74c3c,stroke-width:2px
    style R2 fill:#ffccbc,stroke:#e74c3c,stroke-width:2px
    style G1 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style G2 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style G3 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style RF1 fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style RF2 fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style RF3 fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style RF4 fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style RF5 fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style D1 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style D2 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style Complete fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
```

**TDD Principles:**

- **RED**: Write a test that fails because the feature doesn't exist yet
- **GREEN**: Write just enough code to make the test pass (no more, no less)
- **REFACTOR**: Clean up code while ensuring tests still pass

**AmanMCP Requirements:**

- All tests must pass with race detector (`go test -race`)
- `make ci-check` must pass before commit (linting, tests, coverage)
- Acceptance Criteria (AC) defines what tests verify
- SSOT for feature specs: `.aman-pm/product/features/`

### Documentation Hierarchy (SSOT)

```mermaid
graph TB
    subgraph Public["Public Documentation"]
        P1[README.md<br/>Quick start]
        P2[docs/guides/<br/>User guides]
        P3[docs/reference/<br/>API reference]
        P4[CHANGELOG.md<br/>Release history]
    end

    subgraph Internal["Internal Documentation"]
        I1[CLAUDE.md<br/>AI-native dev rules]
        I2[.aman-pm/product/<br/>Feature specs]
        I3[.aman-pm/sprints/<br/>Sprint planning]
        I4[.aman-pm/changelog/<br/>Unreleased changes]
    end

    subgraph SSOT["Single Source of Truth"]
        S1[Feature Spec<br/>.aman-pm/product/features/]
        S2[ADR<br/>docs/reference/decisions/]
        S3[PM Index<br/>.aman-pm/index.yaml]
        S4[Config Schema<br/>internal/config/]
    end

    I2 --> S1
    I4 --> P4
    I3 --> S3
    P3 --> S4
    I1 --> S1

    S1 -->|Defines| AC[Acceptance Criteria]
    S2 -->|Explains| Decisions[Architecture Decisions]
    S3 -->|Tracks| Progress[Sprint Progress]
    S4 -->|Validates| Config[Configuration]

    style P1 fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style P2 fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style P3 fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style P4 fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style I1 fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style I2 fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style I3 fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style I4 fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style S1 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style S2 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style S3 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style S4 fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
```

**SSOT Principles:**

- **One authoritative source** for each type of information
- **Public docs** generated/derived from internal sources
- **Feature specs** are the definitive source for requirements
- **ADRs** capture architectural decisions with rationale
- **No duplicate information** - link to SSOT instead

---

## Project-Specific

| Term | Definition |
|------|------------|
| **AmanMCP** | This project. Local-first RAG MCP server for developers. |
| **coder/hnsw** | Pure Go HNSW vector database. Primary vector store (300K+ scale). |
| **Ollama** | Local LLM runner. Default embedding provider for AmanMCP. |
| **MLX** | Apple's machine learning framework. Optional faster embeddings on Apple Silicon. |
| **qwen3-embedding** | Default embedding model (0.6b variant). Optimized for code and documentation. |

---

## Metrics & Performance

| Term | Definition |
|------|------------|
| **Latency** | Time from request to response. Target: <100ms P95. |
| **P95** | 95th percentile. 95% of requests complete faster than this. |
| **Recall** | Percentage of relevant results actually retrieved. |
| **Throughput** | Requests processed per second. |

---

## Adding Terms

When adding new terms:

1. Place in appropriate category
2. Keep definitions concise (1-2 sentences)
3. Link to related terms if helpful
4. Update alphabetically within category

---

*Shared vocabulary enables clear communication.*
