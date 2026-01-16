# Reciprocal Rank Fusion: Combining BM25 and Vector Search

> **Learning Objectives:**
> - Understand why direct score combination doesn't work for hybrid search
> - Learn the Reciprocal Rank Fusion (RRF) algorithm
> - Know when to use RRF and how to tune its parameters
>
> **Prerequisites:**
> - [Hybrid Search Concepts](../concepts/hybrid-search.md)
> - Basic understanding of BM25 and vector search
>
> **Audience:** Search engineers, RAG developers, anyone building hybrid search

---

## TL;DR

BM25 scores (0-25) cannot combine directly with vector similarity scores (0-1). Reciprocal Rank Fusion solves this elegantly by using **ranks** instead of scores. A document ranked first in both lists gets a high combined score regardless of what the raw scores were. This makes RRF robust, simple, and proven across information retrieval research.

---

## The Problem: Incompatible Scores

When building hybrid search, you run two separate retrieval systems:

```
Query: "authentication middleware"

BM25 Result:
  chunk_A: score = 18.5
  chunk_B: score = 12.3
  chunk_C: score = 8.7

Vector Result:
  chunk_C: score = 0.92
  chunk_A: score = 0.87
  chunk_D: score = 0.71
```

How do you combine these? You cannot simply add or average them:

- 18.5 + 0.87 = 19.37 for chunk_A
- 8.7 + 0.92 = 9.62 for chunk_C

This ranks chunk_A higher, but is that correct? The BM25 score of 18.5 is in a completely different universe than the vector score of 0.87. These numbers are **incommensurable**---they measure different things on different scales.

### Why Direct Combination Fails

| Problem | Description |
|---------|-------------|
| **Different scales** | BM25: 0-25+, Vector: 0-1 |
| **Different distributions** | BM25 scores vary widely by query; vector scores cluster tightly |
| **No universal calibration** | A BM25 score of 10 means nothing without context |
| **Dominance effects** | The method with larger scores drowns out the other |

### What About Normalization?

You might think: "Just normalize both to 0-1 first."

```
BM25 normalized: chunk_A = 18.5/18.5 = 1.0
Vector normalized: chunk_C = 0.92/0.92 = 1.0
```

This helps, but introduces new problems:

- Requires knowing score distributions in advance
- Different queries produce different distributions
- Edge cases (single result, identical scores) break normalization
- Still assumes scores are linearly comparable

Normalization is brittle. We need something more robust.

---

## Alternatives Considered

| Method | Pros | Cons |
|--------|------|------|
| Score normalization | Simple concept | Needs score distribution knowledge, brittle |
| Linear combination | Easy to implement | Scores are fundamentally incomparable |
| **RRF (k=60)** | Rank-based, proven, simple | Loses fine-grained score information |
| CombMNZ | Good for multi-source fusion | Complex, needs additional tuning |
| Learning-to-Rank | Can learn optimal combination | Requires training data, complexity |

### Why RRF Won

RRF has a key insight: **ranks are universal**.

First place is first place, whether the score was 18.5 or 0.92. By converting scores to ranks, we eliminate the scale problem entirely.

---

## The RRF Algorithm

### Formula

```
RRF_score(d) = sum over all sources of: weight_i / (k + rank_i)
```

Where:
- `d` = document being scored
- `weight_i` = weight for search source i
- `k` = smoothing constant (typically 60)
- `rank_i` = position in ranked list from source i (1-indexed)

### Step-by-Step Example

```
BM25 results (sorted by score):     Vector results (sorted by score):
1. chunk_A (score=18.5)             1. chunk_C (score=0.92)
2. chunk_B (score=12.3)             2. chunk_A (score=0.87)
3. chunk_C (score=8.7)              3. chunk_D (score=0.71)

Weights: BM25 = 0.35, Semantic = 0.65
k = 60
```

Calculate RRF score for each document:

**chunk_A:**
```
BM25:   0.35 / (60 + 1) = 0.35 / 61 = 0.00574
Vector: 0.65 / (60 + 2) = 0.65 / 62 = 0.01048
Total:  0.01622
```

**chunk_B:**
```
BM25:   0.35 / (60 + 2) = 0.35 / 62 = 0.00565
Vector: not in list, use missing_rank = 4
        0.65 / (60 + 4) = 0.65 / 64 = 0.01016
Total:  0.01581
```

**chunk_C:**
```
BM25:   0.35 / (60 + 3) = 0.35 / 63 = 0.00556
Vector: 0.65 / (60 + 1) = 0.65 / 61 = 0.01066
Total:  0.01622
```

**chunk_D:**
```
BM25:   not in list, use missing_rank = 4
        0.35 / (60 + 4) = 0.35 / 64 = 0.00547
Vector: 0.65 / (60 + 3) = 0.65 / 63 = 0.01032
Total:  0.01579
```

**Final ranking:** chunk_A = chunk_C > chunk_B > chunk_D

Notice: chunk_A and chunk_C tie despite having very different raw scores. This is intentional---both were highly ranked by their respective systems.

### Why It Works

1. **Ranks are comparable:** First is first, regardless of raw scores
2. **High ranks contribute more:** 1/(60+1) > 1/(60+10)
3. **The denominator prevents extremes:** Even rank 1 only gets score 1/61, not infinity
4. **Missing documents are penalized:** Assigned rank beyond the list length

---

## The k=60 Sweet Spot

The constant `k` controls how much top ranks dominate:

```
Rank    k=10      k=60      k=100
1       1/11      1/61      1/101
2       1/12      1/62      1/102
10      1/20      1/70      1/110

Ratio (rank 1 / rank 10):
k=10:   11/20 = 1.82x
k=60:   61/70 = 1.15x
k=100:  101/110 = 1.09x
```

### k Trade-offs

| k Value | Behavior | Use Case |
|---------|----------|----------|
| Low (10-30) | Top ranks dominate heavily | When you trust top results strongly |
| **60 (default)** | Balanced; top ranks matter but lower ranks contribute | General purpose |
| High (100+) | Rankings flatten; positions matter less | When many results are relevant |

### Why 60 Specifically?

- **Empirically validated:** Used by Azure AI Search, OpenSearch, and academic research
- **Good balance:** Top result (rank 1) scores 1.6x higher than rank 10, not too extreme
- **Large enough:** Prevents numerical instability from tiny denominators
- **Small enough:** Rankings still matter; not just averaging everything

The original RRF paper by Cormack et al. (SIGIR 2009) established k=60 as a robust default that works across diverse retrieval scenarios.

---

## Our Implementation

```go
// internal/search/fusion.go
const DefaultRRFConstant = 60

type RRFFusion struct {
    K int // RRF smoothing constant (default: 60)
}

func (f *RRFFusion) Fuse(
    bm25 []*store.BM25Result,
    vec []*store.VectorResult,
    weights Weights,
) []*FusedResult {
    scores := make(map[string]*FusedResult)

    // Process BM25 results (1-indexed ranks)
    for rank, r := range bm25 {
        result := f.getOrCreate(scores, r.DocID)
        result.BM25Score = r.Score
        result.BM25Rank = rank + 1
        result.RRFScore += weights.BM25 / float64(f.K+rank+1)
    }

    // Process vector results (1-indexed ranks)
    for rank, r := range vec {
        result := f.getOrCreate(scores, r.ID)
        result.VecScore = float64(r.Score)
        result.VecRank = rank + 1
        result.RRFScore += weights.Semantic / float64(f.K+rank+1)

        // Mark if in both lists
        if result.BM25Rank > 0 {
            result.InBothLists = true
        }
    }

    // Handle documents in only one list
    missingRank := max(len(bm25), len(vec)) + 1
    for _, r := range scores {
        if r.BM25Rank == 0 {
            r.RRFScore += weights.BM25 / float64(f.K+missingRank)
        }
        if r.VecRank == 0 {
            r.RRFScore += weights.Semantic / float64(f.K+missingRank)
        }
    }

    return sortAndNormalize(scores)
}
```

### Weight Tuning

```yaml
# AmanMCP defaults (.amanmcp.yaml)
bm25_weight: 0.35
semantic_weight: 0.65
rrf_constant: 60
```

**Why 0.35/0.65?**

- Semantic search captures meaning better for natural language queries
- BM25 excels at exact matches (error codes, identifiers)
- The 65% semantic bias reflects that most queries are conceptual
- Weights can be adjusted dynamically based on query classification

### Tie-Breaking Strategy

When two documents have identical RRF scores, we need deterministic ordering:

```go
// Priority:
// 1. Higher RRF score
// 2. In both lists (documents found by both methods are more confident)
// 3. Higher BM25 score (exact match indicator)
// 4. Lexicographically smaller ChunkID (deterministic)
func (f *RRFFusion) compare(a, b *FusedResult) bool {
    if a.RRFScore != b.RRFScore {
        return a.RRFScore > b.RRFScore
    }
    if a.InBothLists != b.InBothLists {
        return a.InBothLists
    }
    if a.BM25Score != b.BM25Score {
        return a.BM25Score > b.BM25Score
    }
    return a.ChunkID < b.ChunkID
}
```

The `InBothLists` tie-breaker is particularly useful: if both BM25 and vector search ranked a document highly, we have higher confidence it's relevant.

---

## When to Use RRF

### Good Fits

- **Hybrid search:** Combining keyword and semantic retrieval
- **Multi-modal retrieval:** Fusing text and image search results
- **Federated search:** Combining results from multiple indexes
- **Ensemble ranking:** Merging outputs from different ranking models

### Decision Guide

```
Do you have multiple ranked lists to combine?
├── No → RRF not applicable
└── Yes → Are the scores directly comparable?
          ├── Yes (same scale, same meaning) → Simple averaging might work
          └── No (different scales or meanings) → Use RRF
```

### When NOT to Use RRF

- **Single source:** RRF is for fusion; one list needs no fusion
- **Comparable scores:** If scores are already calibrated (same scale, same meaning), weighted averaging might be simpler
- **Need fine-grained score info:** RRF discards score magnitudes; if you need to know "how much better" rank 1 was than rank 2, raw scores might matter

---

## Limitations

RRF is not perfect. Understanding its limitations helps you decide when to use alternatives.

### 1. Loses Score Magnitude Information

```
BM25 Result A: score = 25.0 (perfect match)
BM25 Result B: score = 24.9 (almost as good)
```

RRF treats these as rank 1 and rank 2. The fact that they were nearly identical is lost. If A scored 25.0 and B scored 5.0, they would still be rank 1 and rank 2.

**When this matters:** If you need to threshold results ("only return if confidence > X"), RRF scores are normalized and don't map to confidence.

### 2. Assumes Ranks Are Meaningful

If one retrieval system returns garbage in the top 10, RRF still treats rank 1 as rank 1. RRF cannot detect that a source is unreliable.

**Mitigation:** Use quality weights. If BM25 is unreliable for certain query types, reduce its weight.

### 3. Fixed k May Not Be Optimal

k=60 is a good default, but might not be optimal for your specific data distribution. Some domains might benefit from k=30 (trust top ranks more) or k=100 (spread out contributions).

**Mitigation:** Run experiments with your validation suite to tune k.

### 4. No Learning

RRF is a fixed formula. It cannot learn from user feedback or click data to improve fusion quality.

**When this matters:** If you have training data and need state-of-the-art quality, Learning-to-Rank models can outperform RRF.

---

## Practical Tips

### 1. Preserve Original Scores

Even though RRF uses ranks, preserve the original scores in your result structure:

```go
type FusedResult struct {
    ChunkID     string
    RRFScore    float64  // Combined score
    BM25Score   float64  // Preserved for debugging
    BM25Rank    int      // Preserved for analysis
    VecScore    float64  // Preserved for debugging
    VecRank     int      // Preserved for analysis
    InBothLists bool     // Confidence indicator
}
```

These help with:
- Debugging why a document ranked where it did
- Building validation suites
- User-facing explanations ("found by keyword match")

### 2. Handle Missing Documents

Documents appearing in only one list need special handling:

```go
// Option 1: Assign missing_rank = max(len1, len2) + 1
missingRank := max(len(bm25), len(vec)) + 1

// Option 2: Assign missing_rank = infinity (only score from present list)
// This penalizes documents that one system couldn't find at all
```

We use Option 1---it gives documents a chance from both sources while still penalizing absence.

### 3. Normalize Final Scores

Raw RRF scores are small numbers (0.01-0.03 range). Normalizing to 0-1 makes them more interpretable:

```go
func normalize(results []*FusedResult) {
    if len(results) == 0 {
        return
    }
    maxScore := results[0].RRFScore  // Results are sorted
    for _, r := range results {
        r.RRFScore = r.RRFScore / maxScore
    }
}
```

After normalization, the top result has score 1.0, making it easy to understand relative rankings.

---

## Further Reading

- [RRF Paper (Cormack et al., SIGIR 2009)](https://plg.uwaterloo.ca/~gvcormac/cormacksigir09-rrf.pdf) - Original research establishing RRF and k=60
- [Azure AI Search RRF Documentation](https://learn.microsoft.com/en-us/azure/search/hybrid-search-ranking) - Production implementation details
- [OpenSearch RRF](https://opensearch.org/docs/latest/search-plugins/search-relevance/reranking-search-results/) - Open source implementation

---

## See Also

- [Hybrid Search Concepts](../concepts/hybrid-search.md) - How BM25 and vector search combine
- [Query Expansion](./query-expansion-asymmetric.md) - Why BM25 needs expansion but vector search doesn't

---

**Original Source:** `.aman-pm/decisions/ADR-004` (internal)
**Last Updated:** 2026-01-16
