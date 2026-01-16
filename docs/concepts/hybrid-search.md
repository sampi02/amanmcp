# Hybrid Search Concepts

**Version:** 1.0.0  
**Last Updated:** 2025-12-28

Learn how AmanMCP combines keyword and semantic search for best results.

---

## Overview

Hybrid search combines two complementary approaches:

- **BM25 (Keyword)**: Finds exact term matches
- **Vector (Semantic)**: Finds meaning-based matches

Neither alone is sufficient. Together, they're powerful.

```mermaid
flowchart LR
    Query[Query] --> BM25[BM25<br/>Keywords]
    Query --> Vector[Vector<br/>Semantic]

    BM25 --> RRF[RRF Fusion]
    Vector --> RRF

    RRF --> Results[Best Results]

    style BM25 fill:#3498db,stroke:#2980b9,stroke-width:2px
    style Vector fill:#9b59b6,stroke:#8e44ad,stroke-width:2px
    style RRF fill:#27ae60,stroke:#229954,stroke-width:2px
```

---

## Why Hybrid?

### The Keyword Search Problem

BM25 is great for:

- Error codes: `ERR_CONNECTION_REFUSED`
- Function names: `handleUserLogin`
- Exact phrases: `"authentication middleware"`

But fails for:

- Synonyms: "auth" vs "authentication"
- Concepts: "how to validate user input"
- Typos: "autentication"

### The Semantic Search Problem

Vector search is great for:

- Natural language: "how does user login work"
- Concepts: "security best practices"
- Similar meaning: finds "authentication" when searching "login"

But fails for:

- Exact identifiers: `ERR_001` might not match
- Rare terms: domain-specific vocabulary
- Code: variable names don't have "meaning"

### The Solution: Hybrid

Combine both and get best of each:

```mermaid
flowchart TB
    Query["Query: 'useEffect cleanup function'"]

    subgraph BM25Result["BM25 Finds"]
        B1["Exact 'useEffect' matches"]
        B2["File: hooks.ts:45"]
    end

    subgraph VectorResult["Vector Finds"]
        V1["Conceptually similar hook patterns"]
        V2["cleanup patterns, effect handlers"]
    end

    Query --> BM25Result
    Query --> VectorResult

    BM25Result --> Fusion[RRF Fusion]
    VectorResult --> Fusion

    Fusion --> Best["Best of both, ranked together"]

    style BM25Result fill:#3498db,stroke:#2980b9,stroke-width:2px
    style VectorResult fill:#9b59b6,stroke:#8e44ad,stroke-width:2px
    style Fusion fill:#27ae60,stroke:#229954,stroke-width:2px
```

---

## How BM25 Works

BM25 (Best Match 25) is a ranking function based on term frequency.

### Core Idea

```
Score = Σ IDF(term) × TF(term, doc) × normalization

Where:
- IDF = Inverse Document Frequency (rare terms score higher)
- TF = Term Frequency (more occurrences = higher score)
- normalization = adjusts for document length
```

### Example

```
Documents:
1. "authentication middleware for API"
2. "auth helper functions"
3. "user profile page"

Query: "authentication"

Scores:
1. High (exact match, technical doc)
2. Medium (partial match "auth")
3. Zero (no match)
```

### BM25 Algorithm Step-by-Step

```mermaid
---
config:
  layout: elk
---
%%{init: {'theme': 'base', 'themeVariables': { 'primaryColor': '#e1f5ff', 'primaryTextColor': '#1a1a1a', 'primaryBorderColor': '#3498db', 'lineColor': '#3498db', 'secondaryColor': '#c8e6c9', 'tertiaryColor': '#fff4e6'}}}%%
flowchart TB
    Start(["Query: authentication"]) --> Tokenize[Tokenize Query]
    Tokenize --> Tokens["Tokens: authentication"]

    Tokens --> ForEachDoc{For Each<br/>Document}

    ForEachDoc --> CalcTF["Calculate Term Frequency<br/>TF = count in doc"]
    CalcTF --> GetIDF["Get Inverse Document Frequency<br/>IDF = log of N divided by df"]
    GetIDF --> GetDocLen["Get Document Length<br/>DL = token count"]

    GetDocLen --> CalcNorm["Calculate Length Normalization<br/>norm = 1 - b + b times DL divided by avgDL"]

    CalcNorm --> CalcBM25Score["Calculate BM25 Score<br/>score = IDF times TF times k1+1<br/>divided by TF + k1 times norm"]

    CalcBM25Score --> AddToTotal["Add to Document Score<br/>docScore += score"]

    AddToTotal --> MoreTerms{More Query<br/>Terms?}
    MoreTerms -->|Yes| CalcTF
    MoreTerms -->|No| MoreDocs{More<br/>Documents?}

    MoreDocs -->|Yes| ForEachDoc
    MoreDocs -->|No| SortResults[Sort Documents by Score]

    SortResults --> Return([Return Ranked Results])

    style Start fill:#3498db,stroke:#2980b9,stroke-width:2px
    style Tokens fill:#f39c12,stroke:#d68910,stroke-width:2px
    style CalcTF fill:#9b59b6,stroke:#8e44ad,stroke-width:2px
    style GetIDF fill:#9b59b6,stroke:#8e44ad,stroke-width:2px
    style CalcNorm fill:#9b59b6,stroke:#8e44ad,stroke-width:2px
    style CalcBM25Score fill:#e74c3c,stroke:#c0392b,stroke-width:2px
    style SortResults fill:#27ae60,stroke:#229954,stroke-width:2px
    style Return fill:#27ae60,stroke:#229954,stroke-width:2px
```

**Key Parameters:**
- `k1 = 1.2` - Term frequency saturation parameter
- `b = 0.75` - Document length normalization strength
- `N` - Total number of documents
- `df` - Number of documents containing term
- `avgDL` - Average document length

### In AmanMCP

```go
type BM25Index struct {
    documents map[string][]string  // docID -> tokens
    idf       map[string]float64   // term -> IDF score
    avgLen    float64              // average doc length
}

func (b *BM25Index) Search(query string) []ScoredResult {
    tokens := tokenize(query)
    scores := make(map[string]float64)

    for _, token := range tokens {
        for docID, docTokens := range b.documents {
            tf := countOccurrences(token, docTokens)
            idf := b.idf[token]
            scores[docID] += b.score(tf, idf, len(docTokens))
        }
    }

    return rankByScore(scores)
}
```

---

## How Vector Search Works

Vector search finds semantically similar documents using embeddings.

### Core Idea

```mermaid
flowchart LR
    subgraph Index["Indexing Time"]
        T1[Text] --> E1[Embed] --> V1[Vector]
        V1 --> Store[(Vector Index)]
    end

    subgraph Query["Query Time"]
        Q[Query] --> E2[Embed] --> QV[Query Vector]
        QV --> Search{Find Nearest<br/>Neighbors}
        Store --> Search
        Search --> Results[Similar Docs]
    end

    style Index fill:#e67e22,stroke:#d35400,stroke-width:2px
    style Query fill:#27ae60,stroke:#229954,stroke-width:2px
```

### Embeddings

An embedding is a dense vector (e.g., 768 dimensions) that captures meaning:

```
"authentication" → [0.12, -0.34, 0.56, ...]  (768 numbers)
"login"          → [0.11, -0.32, 0.58, ...]  (similar vector)
"weather"        → [0.89, 0.12, -0.45, ...]  (different vector)
```

Similar meanings → Similar vectors → Close in vector space

```mermaid
flowchart TB
    subgraph VectorSpace["Vector Space Visualization"]
        Auth["'authentication'"]
        Login["'login'"]
        Cred["'credentials'"]
        Weather["'weather'"]

        Auth <-.->|"close"| Login
        Auth <-.->|"close"| Cred
        Login <-.->|"close"| Cred

        Weather x--x|"far apart"| Auth
    end

    style Auth fill:#27ae60,stroke:#229954,stroke-width:2px
    style Login fill:#27ae60,stroke:#229954,stroke-width:2px
    style Cred fill:#27ae60,stroke:#229954,stroke-width:2px
    style Weather fill:#e74c3c,stroke:#c0392b,stroke-width:2px
```

### HNSW (Hierarchical Navigable Small Worlds)

Finding nearest neighbors in 768 dimensions could be slow. HNSW solves this:

```
Brute force: O(n) - check every document
HNSW:        O(log n) - navigate graph structure
```

For 100K documents:

- Brute force: 100,000 comparisons
- HNSW: ~17 comparisons

### In AmanMCP

```go
type VectorIndex struct {
    graph    *hnsw.Graph[uint64]
    embedder Embedder
}

func (v *VectorIndex) Search(query string) []ScoredResult {
    // Convert query to vector
    embedding, _ := v.embedder.Embed(query)

    // Find nearest neighbors using HNSW
    neighbors := v.graph.Search(embedding, 20)

    // Convert to results
    return toResults(neighbors)
}
```

---

## Reciprocal Rank Fusion (RRF)

RRF combines results from multiple sources.

### The Problem

BM25 gives: [A, B, C, D]
Vector gives: [C, A, D, B]

How to combine? Can't just average scores (different scales).

### The Solution: RRF

Score based on rank, not absolute score:

```
RRF(doc) = Σ weight_i / (k + rank_i)

Where:
- k = 60 (smoothing constant)
- rank_i = position in source list
- weight_i = source weight
```

### Example

```
BM25 results:      Vector results:
1. chunk_A         1. chunk_C
2. chunk_B         2. chunk_A
3. chunk_C         3. chunk_D
4. chunk_D         4. chunk_B

Weights: BM25 = 0.35, Vector = 0.65

chunk_A:
  BM25: 0.35 / (60 + 1) = 0.00574
  Vec:  0.65 / (60 + 2) = 0.01048
  Total: 0.01622

chunk_C:
  BM25: 0.35 / (60 + 3) = 0.00556
  Vec:  0.65 / (60 + 1) = 0.01066
  Total: 0.01622

Final ranking: A ≈ C > B > D
```

### Why k=60?

The constant k prevents extreme values:

- Low k: Top ranks dominate too much
- High k: Rankings matter less
- k=60: Good balance, empirically validated

### RRF Algorithm Detailed Example

Step-by-step calculation of RRF fusion with real scores:

```mermaid
flowchart TB
    Start([Query: 'authentication']) --> Sources

    subgraph Sources["Input: Two Ranked Lists"]
        direction TB
        BM25List["BM25 Results<br/>1. chunk_A (score: 2.5)<br/>2. chunk_B (score: 1.8)<br/>3. chunk_C (score: 1.2)<br/>4. chunk_D (score: 0.9)"]
        VectorList["Vector Results<br/>1. chunk_C (score: 0.92)<br/>2. chunk_A (score: 0.87)<br/>3. chunk_D (score: 0.81)<br/>4. chunk_B (score: 0.75)"]
    end

    Sources --> RRFCalc

    subgraph RRFCalc["RRF Score Calculation (k=60, weights: BM25=0.35, Vector=0.65)"]
        direction TB

        ChunkA["chunk_A:<br/>BM25 rank: 1 → 0.35/(60+1) = 0.00574<br/>Vector rank: 2 → 0.65/(60+2) = 0.01048<br/>RRF score: 0.01622"]

        ChunkB["chunk_B:<br/>BM25 rank: 2 → 0.35/(60+2) = 0.00565<br/>Vector rank: 4 → 0.65/(60+4) = 0.01016<br/>RRF score: 0.01581"]

        ChunkC["chunk_C:<br/>BM25 rank: 3 → 0.35/(60+3) = 0.00556<br/>Vector rank: 1 → 0.65/(60+1) = 0.01066<br/>RRF score: 0.01622"]

        ChunkD["chunk_D:<br/>BM25 rank: 4 → 0.35/(60+4) = 0.00547<br/>Vector rank: 3 → 0.65/(60+3) = 0.01032<br/>RRF score: 0.01579"]
    end

    RRFCalc --> Sort

    subgraph Sort["Sort by RRF Score"]
        direction TB
        Sorted["Final Ranking:<br/>1. chunk_A (0.01622) ← Best BM25 + good Vector<br/>2. chunk_C (0.01622) ← Best Vector + good BM25<br/>3. chunk_D (0.01579)<br/>4. chunk_B (0.01581)"]
    end

    Sort --> Insight

    subgraph Insight["Why This Works"]
        direction TB
        Explain["
        • chunk_A: #1 in BM25, #2 in Vector → Strong overall
        • chunk_C: #1 in Vector, #3 in BM25 → Strong overall
        • Both get boosted by appearing high in both lists

        • chunk_B: #2 in BM25, but #4 in Vector → Mixed signals
        • chunk_D: #4 in BM25, #3 in Vector → Mixed signals

        Key insight: RRF rewards consistency across sources
        Documents that rank well in BOTH get highest scores
        "]
    end

    subgraph KEffect["Effect of k Parameter"]
        direction TB
        KTable["
        If k=10 (low):
        • Rank 1: 1/(10+1) = 0.091 (very high)
        • Rank 10: 1/(10+10) = 0.050 (still significant)
        • Top ranks dominate too much

        If k=60 (balanced):
        • Rank 1: 1/(60+1) = 0.016
        • Rank 10: 1/(60+10) = 0.014
        • Smooth decay, all ranks contribute

        If k=200 (high):
        • Rank 1: 1/(200+1) = 0.005
        • Rank 10: 1/(200+10) = 0.005
        • Ranks matter less, almost uniform
        "]
    end

    Insight --> KEffect

    style Start fill:#3498db,stroke:#2980b9,stroke-width:2px
    style Sources fill:#e1f5ff
    style RRFCalc fill:#fff9c4
    style ChunkA fill:#c8e6c9
    style ChunkC fill:#c8e6c9
    style ChunkB fill:#ffe0b2
    style ChunkD fill:#ffe0b2
    style Sort fill:#27ae60,stroke:#229954,stroke-width:2px
    style Sorted fill:#27ae60,stroke:#229954,stroke-width:2px
    style Insight fill:#e1f5ff
    style KEffect fill:#f3e5f5
```

**Formula:**

```
RRF_score(doc) = Σ weight_i / (k + rank_i(doc))

where:
  i ∈ {BM25, Vector}
  weight_BM25 = 0.35
  weight_Vector = 0.65
  k = 60
  rank_i(doc) = position of doc in source i (1-indexed)
```

**Properties:**
- **Scale-free**: Works regardless of original score ranges
- **Rank-based**: Only position matters, not absolute scores
- **Weighted**: Different importance for different sources
- **Smooth**: Constant k smooths rank differences

---

## Query Classification

Different queries need different weights:

```
Query Type          BM25    Vector
─────────────────────────────────
Error codes         0.8     0.2     (need exact match)
Identifiers         0.7     0.3     (technical terms)
Mixed               0.5     0.5     (balanced)
Natural language    0.25    0.75    (need meaning)
```

### Query Classification Decision Tree

```mermaid
flowchart TB
    Start([Incoming Query]) --> CheckQuoted{Quoted<br/>Phrase?}

    CheckQuoted -->|Yes: exact match| Quoted["BM25: 0.9<br/>Vector: 0.1"]
    CheckQuoted -->|No| CheckError{Error Code<br/>Pattern?}

    CheckError -->|Yes: error codes| ErrorCode["BM25: 0.8<br/>Vector: 0.2"]
    CheckError -->|No| CheckIdentifier{Code<br/>Identifier?}

    CheckIdentifier -->|CamelCase| CamelCase["BM25: 0.7<br/>Vector: 0.3"]
    CheckIdentifier -->|snake_case| SnakeCase["BM25: 0.7<br/>Vector: 0.3"]
    CheckIdentifier -->|SCREAMING_SNAKE| Constant["BM25: 0.75<br/>Vector: 0.25"]
    CheckIdentifier -->|No| CheckNatural{Natural<br/>Language?}

    CheckNatural -->|Contains: how, what, why| Question["BM25: 0.25<br/>Vector: 0.75"]
    CheckNatural -->|Word count > 5| Sentence["BM25: 0.3<br/>Vector: 0.7"]
    CheckNatural -->|No| CheckMixed{Mixed<br/>Content?}

    CheckMixed -->|Has code + text| Mixed["BM25: 0.5<br/>Vector: 0.5"]
    CheckMixed -->|Pure technical| Technical["BM25: 0.6<br/>Vector: 0.4"]
    CheckMixed -->|Default| Default["BM25: 0.5<br/>Vector: 0.5"]

    Quoted --> ApplyWeights[Apply Weights to Search]
    ErrorCode --> ApplyWeights
    CamelCase --> ApplyWeights
    SnakeCase --> ApplyWeights
    Constant --> ApplyWeights
    Question --> ApplyWeights
    Sentence --> ApplyWeights
    Mixed --> ApplyWeights
    Technical --> ApplyWeights
    Default --> ApplyWeights

    ApplyWeights --> Execute[Execute Hybrid Search]
    Execute --> Results([Weighted Results])

    style Start fill:#3498db,stroke:#2980b9,stroke-width:2px
    style Quoted fill:#e74c3c,stroke:#c0392b,stroke-width:2px
    style ErrorCode fill:#e74c3c,stroke:#c0392b,stroke-width:2px
    style CamelCase fill:#9b59b6,stroke:#8e44ad,stroke-width:2px
    style SnakeCase fill:#9b59b6,stroke:#8e44ad,stroke-width:2px
    style Constant fill:#9b59b6,stroke:#8e44ad,stroke-width:2px
    style Question fill:#27ae60,stroke:#229954,stroke-width:2px
    style Sentence fill:#27ae60,stroke:#229954,stroke-width:2px
    style Mixed fill:#f39c12,stroke:#d68910,stroke-width:2px
    style Technical fill:#f39c12,stroke:#d68910,stroke-width:2px
    style Default fill:#95a5a6,stroke:#7f8c8d,stroke-width:2px
    style ApplyWeights fill:#3498db,stroke:#2980b9,stroke-width:2px
    style Execute fill:#3498db,stroke:#2980b9,stroke-width:2px
    style Results fill:#27ae60,stroke:#229954,stroke-width:2px
```

**Classification Priority (top to bottom):**
1. Quoted phrases → Exact match (BM25-heavy)
2. Error codes → Technical precision (BM25-heavy)
3. Code identifiers → Structural match (BM25-heavy)
4. Natural language → Semantic understanding (Vector-heavy)
5. Mixed content → Balanced approach
6. Default → Balanced (0.5/0.5)

### Classification Patterns

```go
func classifyQuery(query string) (bm25Weight, vecWeight float64) {
    // Error code pattern
    if isErrorCode(query) {
        return 0.8, 0.2
    }

    // Technical identifier
    if isCamelCase(query) || isSnakeCase(query) {
        return 0.7, 0.3
    }

    // Quoted exact phrase
    if strings.HasPrefix(query, "\"") {
        return 0.9, 0.1
    }

    // Natural language
    if isNaturalLanguage(query) {
        return 0.25, 0.75
    }

    // Default: balanced
    return 0.5, 0.5
}
```

---

## In AmanMCP

### Full Search Flow

```mermaid
flowchart TB
    Query["Query: 'authentication middleware'"]

    Query --> Classifier["Query Classifier"]
    Classifier -->|Weights: 0.5, 0.5| Split

    Split --> BM25["BM25 Search<br/>Keyword"]
    Split --> Vector["Vector Search<br/>Semantic"]

    BM25 --> RRF["RRF Fusion<br/>Combine results"]
    Vector --> RRF

    RRF --> Final["Final Results"]

    style Query fill:#3498db,stroke:#2980b9,stroke-width:2px
    style Classifier fill:#f39c12,stroke:#d68910,stroke-width:2px
    style BM25 fill:#9b59b6,stroke:#8e44ad,stroke-width:2px
    style Vector fill:#9b59b6,stroke:#8e44ad,stroke-width:2px
    style RRF fill:#27ae60,stroke:#229954,stroke-width:2px
    style Final fill:#27ae60,stroke:#229954,stroke-width:2px
```

```mermaid
sequenceDiagram
    participant Q as Query
    participant C as Classifier
    participant B as BM25
    participant V as Vector
    participant R as RRF

    Q->>C: "authentication middleware"
    C->>C: Classify → mixed
    C-->>B: weights: 0.5
    C-->>V: weights: 0.5

    par Parallel Search
        B->>B: Search inverted index
        B-->>R: [chunk_A, chunk_B, ...]
    and
        V->>V: Embed → HNSW search
        V-->>R: [chunk_C, chunk_A, ...]
    end

    R->>R: RRF fusion (k=60)
    R-->>Q: Final ranked results
```

### Code Structure

```
internal/search/
├── engine.go      # Coordinates hybrid search
├── bm25.go        # BM25 implementation
├── vector.go      # Vector search wrapper
├── fusion.go      # RRF implementation
└── classifier.go  # Query classification
```

---

## Common Mistakes

### 1. Using Only One Method

```go
// BAD: BM25 only
results := bm25.Search(query)

// BAD: Vector only
results := vector.Search(query)

// GOOD: Hybrid
results := engine.HybridSearch(query)
```

### 2. Equal Weights for All Queries

```go
// BAD: Always 50/50
return bm25.Search(query, 0.5), vector.Search(query, 0.5)

// GOOD: Query-dependent weights
weights := classifier.Classify(query)
return fusion.Combine(bm25, vector, weights)
```

### 3. Sequential Execution

```go
// BAD: Sequential (slow)
bm25Results := bm25.Search(query)
vecResults := vector.Search(query)

// GOOD: Parallel (fast)
var wg sync.WaitGroup
wg.Add(2)
go func() { bm25Results = bm25.Search(query); wg.Done() }()
go func() { vecResults = vector.Search(query); wg.Done() }()
wg.Wait()
```

---

## Performance

### Latency Targets

| Component | Target |
|-----------|--------|
| Query classification | < 2ms |
| BM25 search | < 5ms |
| Vector search | < 10ms |
| RRF fusion | < 1ms |
| **Total** | **< 20ms** |

### Scaling

| Documents | BM25 | Vector (HNSW) |
|-----------|------|---------------|
| 10K | < 2ms | < 1ms |
| 100K | < 10ms | < 5ms |
| 300K | < 20ms | < 10ms |

---

## Further Reading

- [BM25 Wikipedia](https://en.wikipedia.org/wiki/Okapi_BM25)
- [HNSW Paper](https://arxiv.org/abs/1603.09320)
- [RRF Paper](https://plg.uwaterloo.ca/~gvcormac/cormacksigir09-rrf.pdf)
- [EmbeddingGemma](https://huggingface.co/onnx-community/embeddinggemma-300m-ONNX)

---

*Hybrid search gives you precision AND recall. Use both.*
