# Vector Search Concepts

Learn how vector search enables semantic understanding in AmanMCP.

---

## Overview

Vector search finds documents by meaning, not just keywords. It converts text into high-dimensional vectors (embeddings) and finds similar vectors using mathematical distance.

**Why we use it**: A user searching for "authentication" should find code about "login", "auth", "credentials" even if those exact words aren't in the query.

---

## Core Concepts

### What is an Embedding?

An embedding is a list of numbers (vector) that captures the meaning of text:

```
"authentication" → [0.12, -0.34, 0.56, 0.02, ..., 0.11]  (768 numbers)
"login system"   → [0.11, -0.32, 0.58, 0.03, ..., 0.10]  (similar!)
"weather report" → [0.89, 0.12, -0.45, 0.78, ..., -0.22] (different)
```

Similar meanings → Similar vectors → Close in vector space

### Dimensionality

Modern embeddings use 384-1536 dimensions:

| Model | Dimensions | Quality | Speed |
|-------|------------|---------|-------|
| EmbeddingGemma-300M | 768 | Very Good (code) | Fast |
| all-MiniLM-L6-v2 | 384 | Good | Very Fast |
| Static (hash-based) | 256 | Basic | Instant |

More dimensions = more nuance, but more memory and slower search.

### Vector Space

Imagine a 3D space where each point is a document:

```mermaid
flowchart TB
    subgraph AuthCluster["Auth Cluster"]
        login["login.go"]
        auth["auth.go"]
        session["session.go"]
    end

    subgraph DataCluster["Data Cluster"]
        database["database.go"]
        cache["cache.go"]
    end

    subgraph TimeCluster["Time Cluster"]
        weather["weather.go"]
        calendar["calendar.go"]
    end

    login <-.-> auth
    auth <-.-> session

    database <-.-> cache

    weather <-.-> calendar

    AuthCluster x--x TimeCluster

    style AuthCluster fill:#27ae60,stroke-width:2px
    style DataCluster fill:#3498db,stroke-width:2px
    style TimeCluster fill:#9b59b6,stroke-width:2px
```

Similar documents cluster together.

---

## How Embeddings Work

### Embedding Pipeline

The complete flow from text to stored vector:

```mermaid
flowchart TB
    Input["Input Text<br/>'handle user login'"]

    subgraph Tokenization["1. Tokenization"]
        Token["Split into tokens<br/>['handle', 'user', 'login']<br/>↓<br/>Token IDs: [3847, 1029, 8273]"]
    end

    subgraph Model["2. Neural Network (Transformer)"]
        Attention["Attention Mechanism<br/>Find word relationships"]
        Context["Context Building<br/>'login' relates to 'user'"]
        Transform["Transform to embedding space"]

        Attention --> Context --> Transform
    end

    subgraph PostProcess["3. Post-Processing"]
        Raw["Raw Vector<br/>[0.12, -0.34, 0.56, ..., 0.11]<br/>(768 dimensions)"]
        Norm["Normalize<br/>Scale to unit length<br/>√(Σ vi²) = 1.0"]
        Final["Final Embedding<br/>[0.11, -0.31, 0.51, ..., 0.10]"]

        Raw --> Norm --> Final
    end

    subgraph Storage["4. Storage"]
        Index["Vector Index (HNSW)<br/>Store with ID for retrieval"]
    end

    Input --> Tokenization --> Model --> PostProcess --> Storage

    style Input fill:#3498db,stroke-width:2px
    style Tokenization fill:#9b59b6,stroke-width:2px
    style Model fill:#e67e22,stroke-width:2px
    style PostProcess fill:#f39c12,stroke-width:2px
    style Storage fill:#27ae60,stroke-width:2px
```

### Neural Network Magic

Text → Tokenize → Neural Network → Vector

```
Input: "handle user login"

Tokenization:
["handle", "user", "login"] → [3847, 1029, 8273]

Neural Network (Transformer):
- Attention mechanisms find relationships
- "login" relates to "user"
- Context builds meaning

Output: [0.12, -0.34, 0.56, ...]
```

### The Training Process

Embedding models learn from vast text:

```
Training data:
- "The cat sat on the mat"
- "A dog rested on the rug"
- ... billions of examples

Model learns:
- "cat" and "dog" are similar (both pets)
- "sat" and "rested" are similar (both verbs of position)
- "mat" and "rug" are similar (both floor coverings)
```

### Why 768 Dimensions?

Each dimension captures a different aspect:

- Dimension 1: formality (casual ↔ formal)
- Dimension 2: domain (technical ↔ everyday)
- Dimension 3: sentiment (negative ↔ positive)
- ... (768 such aspects, learned automatically)

---

## Similarity Measures

### Cosine Similarity

Measures the angle between vectors (most common):

```
           A (query)
          ╱│
         ╱ │
        ╱  │
       ╱ θ │
      ╱────┘
     B (document)

similarity = cos(θ)

θ = 0°   → cos = 1.0  (identical)
θ = 90°  → cos = 0.0  (unrelated)
θ = 180° → cos = -1.0 (opposite)
```

Formula:

```
cosine_similarity(A, B) = (A · B) / (|A| × |B|)
```

### Euclidean Distance

Measures straight-line distance:

```
distance = √(Σ(ai - bi)²)
```

Smaller distance = more similar.

### When to Use Which

| Metric | Best For | Notes |
|--------|----------|-------|
| Cosine | Text embeddings | Ignores magnitude |
| Euclidean | When magnitude matters | Sensitive to length |
| Dot Product | Normalized vectors | Fastest |

AmanMCP uses **cosine similarity** (standard for text).

---

## The Search Problem

### Brute Force is Slow

To find nearest neighbors naively:

```
For query Q, find top-10 similar from 100K documents:

1. Compute similarity(Q, doc1)   # 768 multiplications
2. Compute similarity(Q, doc2)   # 768 multiplications
3. ...
4. Compute similarity(Q, doc100000)
5. Sort all 100K scores
6. Return top 10

Total: 76.8 million multiplications + sort
Time: ~100ms per query (too slow)
```

### Brute Force vs Approximate Search

The fundamental trade-off in vector search:

```mermaid
flowchart TB
    subgraph BruteForce["Brute Force Search (Exact)"]
        direction TB
        BF_Input["Query Vector"]
        BF_Comp["Compare with ALL vectors<br/>O(n) complexity"]
        BF_Sort["Sort all results"]
        BF_Top["Return top-k"]
        BF_Result["Results"]

        BF_Input --> BF_Comp --> BF_Sort --> BF_Top --> BF_Result

        BF_Metrics["<br/>100K docs: 100,000 comparisons<br/>1M docs: 1,000,000 comparisons<br/><br/>Accuracy: 100%<br/>Speed: Slow (100ms+)<br/>Memory: Low"]
    end

    subgraph ApproxSearch["Approximate Search (HNSW)"]
        direction TB
        AS_Input["Query Vector"]
        AS_Nav["Navigate graph layers<br/>O(log n) complexity"]
        AS_Local["Local exhaustive search<br/>in candidate set"]
        AS_Top["Return top-k"]
        AS_Result["Results"]

        AS_Input --> AS_Nav --> AS_Local --> AS_Top --> AS_Result

        AS_Metrics["<br/>100K docs: ~60 comparisons<br/>1M docs: ~70 comparisons<br/><br/>Accuracy: 95-99%<br/>Speed: Fast (1-5ms)<br/>Memory: Higher (graph structure)"]
    end

    Comparison["<b>Key Insight:</b><br/>Miss 1-5% of perfect results<br/>Gain 20-100x speedup<br/><br/>For code search: acceptable trade-off"]

    BruteForce -.->|"vs"| ApproxSearch
    BruteForce --> Comparison
    ApproxSearch --> Comparison

    style BruteForce fill:#e74c3c,stroke-width:2px
    style BruteForce color:#FFFFFF
    style ApproxSearch fill:#27ae60,stroke-width:2px
    style ApproxSearch color:#FFFFFF
    style Comparison fill:#3498db,stroke-width:2px
    style Comparison color:#FFFFFF
    style BF_Comp fill:#c0392b,stroke-width:2px
    style BF_Comp color:#FFFFFF
    style AS_Nav fill:#229954,stroke-width:2px
    style AS_Nav color:#FFFFFF
```

### We Need Approximate Search

Trade accuracy for speed:

- Instead of checking all docs, check a smart subset
- Miss some good results, but much faster
- For 99% of queries, the "best" we find is good enough

---

## HNSW: How AmanMCP Searches

### Hierarchical Navigable Small Worlds

HNSW builds a multi-layer graph for O(log n) search:

```mermaid
flowchart TB
    subgraph L2["Layer 2 (Sparse - Long jumps)"]
        A2[A] ---- D2[D]
    end

    subgraph L1["Layer 1 (Medium)"]
        A1[A] --- B1[B] --- C1[C] --- D1[D]
    end

    subgraph L0["Layer 0 (Dense - All nodes)"]
        A0[A] --- B0[B] --- E0[E] --- F0[F] --- C0[C] --- D0[D]
        B0 --- G0[G]
        E0 --- H0[H]
        F0 --- I0[I]
        C0 --- J0[J]
    end

    A2 --> A1
    D2 --> D1
    A1 --> A0
    B1 --> B0
    C1 --> C0
    D1 --> D0

    style L2 fill:#e74c3c,stroke-width:2px
    style L1 fill:#f39c12,stroke-width:2px
    style L0 fill:#27ae60,stroke-width:2px
    
    style L2 color:#FFFFFF
    style L1 color:#FFFFFF
    style L0 color:#FFFFFF
```

### Search Algorithm

```mermaid
sequenceDiagram
    participant Q as Query
    participant L2 as Layer 2
    participant L1 as Layer 1
    participant L0 as Layer 0
    participant R as Result

    Note over Q: Find nearest to query vector

    Q->>L2: Start at entry (A)
    L2->>L2: Check neighbors
    L2-->>Q: D is closer, move to D

    Q->>L1: Drop down from D
    L1->>L1: Check D's neighbors
    L1-->>Q: C is closest

    Q->>L0: Drop to bottom
    L0->>L0: Exhaustive local search
    L0-->>Q: I is nearest

    Q->>R: Return I
```

### Detailed HNSW Search Navigation

Step-by-step visualization of how HNSW navigates layers to find nearest neighbors:

```mermaid
---
config:
  layout: elk
---
flowchart TB
    Start([Query Vector Q]) --> EntryPoint["Layer 2 (Top)<br/>Entry point: Node A"]

    EntryPoint --> L2Check["Check A's neighbors in Layer 2<br/>Neighbors: [D]<br/>Distance(Q,A)=0.8<br/>Distance(Q,D)=0.3"]

    L2Check --> L2Better{Found<br/>closer node?}
    L2Better -->|Yes: D is closer| L2Move["Move to D<br/>New best: D (0.3)"]
    L2Move --> L2Recheck["Check D's neighbors<br/>No closer nodes found"]

    L2Recheck --> L2Done["Layer 2 complete<br/>Best node: D"]

    L2Done --> DropL1["Drop to Layer 1<br/>Start from D"]

    DropL1 --> L1Check["Check D's neighbors in Layer 1<br/>Neighbors: [A, B, C]<br/>Distance(Q,A)=0.8<br/>Distance(Q,B)=0.5<br/>Distance(Q,C)=0.2"]

    L1Check --> L1Better{Found<br/>closer node?}
    L1Better -->|Yes: C is closer| L1Move["Move to C<br/>New best: C (0.2)"]
    L1Move --> L1Recheck["Check C's neighbors<br/>No closer nodes found"]

    L1Recheck --> L1Done["Layer 1 complete<br/>Best node: C"]

    L1Done --> DropL0["Drop to Layer 0<br/>Start from C"]

    DropL0 --> L0Search["Exhaustive local search in Layer 0<br/>Check C's neighbors: [B, D, F, J]<br/>Distance(Q,B)=0.5<br/>Distance(Q,D)=0.3<br/>Distance(Q,F)=0.15<br/>Distance(Q,J)=0.4"]

    L0Search --> L0Better{Found<br/>closer node?}
    L0Better -->|Yes: F is closer| L0Move["Move to F<br/>New best: F (0.15)"]

    L0Move --> L0Expand["Check F's neighbors: [C, E, I]<br/>Distance(Q,C)=0.2<br/>Distance(Q,E)=0.25<br/>Distance(Q,I)=0.08"]

    L0Expand --> L0Best{Found<br/>closer node?}
    L0Best -->|Yes: I is closer| L0Final["Move to I<br/>New best: I (0.08)"]

    L0Final --> L0Check["Check I's neighbors<br/>No closer nodes found"]

    L0Check --> Complete["Search complete<br/>Nearest neighbor: I<br/>Distance: 0.08"]

    Complete --> Stats["
    Performance:
    • Layer 2: 2 comparisons
    • Layer 1: 4 comparisons
    • Layer 0: 8 comparisons
    • Total: 14 comparisons
    • vs Brute force: 10,000 comparisons
    • Speedup: 714x
    "]

    style Start fill:#3498db,stroke-width:2px
    style EntryPoint fill:#e74c3c,stroke-width:2px
    style L2Done fill:#f39c12,stroke-width:2px
    style L1Done fill:#f39c12,stroke-width:2px
    style L0Final fill:#27ae60,stroke-width:2px
    style Complete fill:#27ae60,stroke-width:2px
    style Stats fill:#e1f5ff
    style L2Move fill:#ffe0b2
    style L1Move fill:#ffe0b2
    style L0Move fill:#c8e6c9
    style L2Better fill:#fff9c4
    style L1Better fill:#fff9c4
    style L0Better fill:#fff9c4
    style L0Best fill:#fff9c4
```

**Key Algorithm Steps:**

1. **Start at top layer** (Layer 2): Fewest nodes, longest jumps
2. **Greedy search**: Move to closer neighbors until stuck
3. **Drop down**: Descend to next layer at current best node
4. **Repeat**: Greedy search at each layer
5. **Bottom layer**: Exhaustive local search for precision
6. **Result**: Near-optimal with logarithmic comparisons

### Why It's Fast

| Documents | Brute Force | HNSW |
|-----------|-------------|------|
| 10K | 10,000 checks | ~50 checks |
| 100K | 100,000 checks | ~60 checks |
| 1M | 1,000,000 checks | ~70 checks |

Logarithmic scaling!

---

## coder/hnsw: Our Vector Database

> **Note:** AmanMCP originally used USearch (CGO). Replaced with coder/hnsw in v0.1.38 for simpler distribution (pure Go, no CGO dependency for vectors).

### Why coder/hnsw?

| Feature | coder/hnsw | Alternatives |
|---------|------------|--------------|
| Pure Go | Yes - no CGO | USearch requires CGO |
| Portability | True single binary | Library dependencies |
| Scale | 300K+ vectors | Similar |
| Memory | Efficient | Similar |

### Key Features

1. **Pure Go**: No CGO dependency, simple distribution
2. **HNSW Algorithm**: Same fast approximate nearest neighbor search
3. **Persistence**: Save/load from disk
4. **Incremental**: Add/remove vectors without full rebuild

### In AmanMCP

```go
import "github.com/coder/hnsw"

// Create index
g := hnsw.NewGraph[uint64]()

// Add vectors
g.Add(hnsw.MakeNode(key, embedding))

// Search
neighbors := g.Search(queryEmbedding, 10)
```

---

## Quantization

### The Memory Problem

```
768 dimensions × 4 bytes (float32) = 3,072 bytes per vector
100,000 documents = 307 MB just for vectors
```

### Solution: Use Less Precision

| Type | Bytes/dim | Memory (100K) | Quality |
|------|-----------|---------------|---------|
| F32 | 4 | 307 MB | 100% |
| F16 | 2 | 154 MB | ~99% |
| I8 | 1 | 77 MB | ~95% |

AmanMCP uses **F16** (half precision):

- Half the memory
- Negligible quality loss for code search

---

## Embeddings in AmanMCP

### Hugot Provider (Default)

AmanMCP uses Hugot with EmbeddingGemma for local embeddings:

```go
type HugotEmbedder struct {
    session  *hugot.Session
    pipeline *pipelines.FeatureExtractionPipeline
}

func (h *HugotEmbedder) Embed(ctx context.Context, text string) ([]float32, error) {
    result, err := h.pipeline.Run([]string{text})
    if err != nil {
        return nil, fmt.Errorf("embedding failed: %w", err)
    }
    return normalizeVector(result.Embeddings[0]), nil
}
```

### Model Choice: EmbeddingGemma

Why this model:

| Feature | EmbeddingGemma | OpenAI ada-002 |
|---------|----------------|----------------|
| Privacy | 100% local | API call |
| Cost | Free | $0.0001/1K tokens |
| Speed | ~50ms | ~200ms (+ network) |
| Quality | 68% MTEB code | Slightly better |
| Offline | Yes (after download) | No |
| Context | 2048 tokens | 8191 tokens |
| Dimensions | 768 | 1536 |

### Static Fallback

When Hugot model download fails, we use a simple fallback:

```go
func StaticEmbed(text string) []float32 {
    // Simple bag-of-words hashing
    // Not great, but functional
    vec := make([]float32, 768)
    words := tokenize(text)
    for i, word := range words {
        h := hash(word)
        idx := h % 768
        vec[idx] += float32(1.0 / float64(i+1))
    }
    normalize(vec)
    return vec
}
```

---

## Indexing Pipeline

### Document → Vector

```mermaid
flowchart TB
    Source["Source File"]
    Chunker["Chunker<br/>tree-sitter splits<br/>into functions/types"]
    Embedder["Embedder<br/>Convert chunks to vectors"]
    Index[(Vector Index<br/>Store for retrieval)]

    Source --> Chunker --> Embedder --> Index

    style Source fill:#3498db,stroke-width:2px,color:#fff
    style Chunker fill:#9b59b6,stroke-width:2px,color:#fff
    style Embedder fill:#e67e22,stroke-width:2px,color:#fff
    style Index fill:#27ae60,stroke-width:2px,color:#fff
```

### Batching for Speed

```go
func (i *Indexer) IndexFiles(files []File) error {
    // Chunk all files
    var allChunks []Chunk
    for _, f := range files {
        allChunks = append(allChunks, i.chunker.Chunk(f)...)
    }

    // Batch embed (much faster than one-by-one)
    texts := make([]string, len(allChunks))
    for i, c := range allChunks {
        texts[i] = c.Content
    }
    embeddings := i.embedder.EmbedBatch(texts)

    // Add to index
    for i, emb := range embeddings {
        i.vectorIndex.Add(allChunks[i].ID, emb)
    }

    return nil
}
```

---

## Search Flow

### Query → Results

```mermaid
flowchart TB
    Query["Query: 'authentication middleware'"]

    Embed["Embed Query<br/>[0.12, -0.34, ...]"]
    HNSW["HNSW Search<br/>Find 20 nearest neighbors"]
    Rerank["Rerank<br/>Reorder by exact similarity"]
    Return["Return<br/>Top 10 chunks with scores"]

    Query --> Embed --> HNSW --> Rerank --> Return

    style Query fill:#3498db,stroke-width:2px
    style Embed fill:#9b59b6,stroke-width:2px
    style HNSW fill:#e67e22,stroke-width:2px
    style Rerank fill:#f39c12,stroke-width:2px
    style Return fill:#27ae60,stroke-width:2px
```

### Code Example

```go
func (e *VectorEngine) Search(query string, limit int) ([]Result, error) {
    // 1. Embed query
    queryVec, err := e.embedder.Embed(query)
    if err != nil {
        return nil, fmt.Errorf("embed query: %w", err)
    }

    // 2. Search index
    keys, distances, err := e.index.Search(queryVec, limit*2)
    if err != nil {
        return nil, fmt.Errorf("search index: %w", err)
    }

    // 3. Convert to results
    results := make([]Result, len(keys))
    for i, key := range keys {
        chunk := e.getChunk(key)
        results[i] = Result{
            Chunk: chunk,
            Score: 1 - distances[i],  // Convert distance to similarity
        }
    }

    return results[:limit], nil
}
```

---

## Common Mistakes

### 1. Not Normalizing Vectors

```go
// BAD: Raw vectors
index.Add(key, embedding)

// GOOD: Normalize for cosine similarity
normalize(embedding)
index.Add(key, embedding)
```

### 2. Wrong Chunk Size

```
Too small (10 tokens):
  "func main" → Not enough context

Too large (5000 tokens):
  Entire file → Too diluted, hard to match

Just right (200-500 tokens):
  Complete function → Good semantic unit
```

### 3. Ignoring Empty Results

```go
// BAD: Assume results exist
results, _ := engine.Search(query, 10)
fmt.Println(results[0])  // Panic if empty

// GOOD: Check length
if len(results) == 0 {
    return nil, ErrNoResults
}
```

---

## Performance Tips

### 1. Pre-compute and Cache

```go
// Cache frequently used embeddings
type EmbedCache struct {
    cache map[string][]float32
    mu    sync.RWMutex
}

func (c *EmbedCache) Get(text string) ([]float32, bool) {
    c.mu.RLock()
    defer c.mu.RUnlock()
    v, ok := c.cache[text]
    return v, ok
}
```

### 2. Batch Embedding

```go
// BAD: One at a time (slow)
for _, text := range texts {
    emb := embedder.Embed(text)  // HTTP call each time
}

// GOOD: Batch (fast)
embeddings := embedder.EmbedBatch(texts)  // One HTTP call
```

### 3. Use Appropriate Precision

```go
// coder/hnsw uses float32 vectors
// For memory optimization, consider dimensionality reduction
// or using smaller embedding models

// AmanMCP uses float32 with 768-dimensional vectors
type Vector = []float32
```

---

## Further Reading

- [Sentence Transformers](https://www.sbert.net/) - Embedding models
- [HNSW Paper](https://arxiv.org/abs/1603.09320) - Original algorithm
- [coder/hnsw](https://github.com/coder/hnsw) - Pure Go HNSW implementation
- [What are Word Embeddings?](https://jalammar.github.io/illustrated-word2vec/)

---

*Vector search finds meaning, not just words. Use it to understand intent.*
