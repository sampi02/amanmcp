# Embedding Model Evolution: From nomic-embed to Qwen3

> **Historical Document**
> This documents the original nomic-embed-text decision (December 2025).
> The embedding strategy evolved through four phases to the current Ollama-based Qwen3.
>
> **Current Implementation:** See [Embedding Models](./embedding-models.md)

> **Learning Objectives:**
> - Understand criteria for selecting embedding models
> - Learn why model choices evolve over time
> - See trade-offs between quality, speed, integration complexity, and resource usage
>
> **Audience:** ML engineers, RAG developers, anyone choosing embedding models

## TL;DR

AmanMCP's embedding strategy evolved through four phases:
1. **nomic-embed-text** (via Ollama) - High quality, but Ollama dependency too heavy
2. **Hugot (MiniLM)** - Pure Go, zero deps, but quality loss (~9%)
3. **MLX (Qwen3-8B)** - Fastest (55x), highest quality, but RAM-hungry
4. **Ollama (Qwen3)** - Balanced default with MLX as opt-in for speed

Each transition was driven by different priorities: first quality, then simplicity, then speed, and finally resource efficiency. The lesson: there is no "best" model - only best for your constraints.

## The Evolution Timeline

```
Phase 1 (Dec 2025): nomic-embed-text via Ollama
  │  81% MTEB, 768 dims, 8K context
  │  Problem: Ollama dependency violates "It Just Works"
  ↓
Phase 2 (Late Dec 2025): Hugot (all-MiniLM-L6-v2)
  │  70% MTEB, 384 dims, 512 context
  │  Problem: Quality loss noticeable in code search
  ↓
Phase 3 (Jan 2026): MLX with Qwen3-8B
  │  80%+ MTEB, 4096 dims, 55x faster than Ollama
  │  Problem: High RAM usage during long sessions
  ↓
Phase 4 (Jan 2026): Ollama default, MLX opt-in
  │  Best balance for typical development workflows
  └── Current state
```

## Phase 1: nomic-embed-text (Original Decision)

**Date:** December 28, 2025
**Status:** Superseded by ADR-005 (Hugot)

### Requirements

The initial embedding model selection was driven by these constraints:

1. **Local-first** - Privacy is paramount; no API calls to external services
2. **Code + Natural Language** - Must understand both programming constructs and documentation
3. **High Quality** - Search relevance directly impacts developer productivity
4. **Accessible Hardware** - Should work on typical developer laptops

### Why nomic-embed-text

| Criterion | nomic-embed-text Score |
|-----------|------------------------|
| MTEB Score | ~81% (near state-of-art for open models) |
| Languages | 100+ supported |
| Architecture | MoE (Mixture of Experts) - efficiency optimized |
| Context Window | 8,192 tokens |
| Dimensions | 768 |

**Alternatives Rejected:**
- **OpenAI Embeddings**: Best quality but requires API key, not local, costs money
- **sentence-transformers**: Requires Python runtime, complex setup
- **Static hash embeddings**: Zero semantic understanding

### Trade-offs Accepted

```
+ High-quality semantic search without API costs
+ Privacy preserved - embeddings never leave user's machine
+ Consistent results (no API variability)

- Requires Ollama installation (additional setup step)
- ~200MB model download required
- Slower than API (depends on local hardware)
- May not work on all machines (GPU recommended)
```

### What We Learned

The quality was excellent, but the setup friction was unacceptable:

```
User installs amanmcp
    ↓
"Ollama not running" error
    ↓
User must:
  1. Install Ollama (brew install ollama)
  2. Start Ollama service (ollama serve)
  3. Wait for model download (~274MB)
    ↓
Finally works
```

**Lesson 1:** Quality means nothing if users can't get started.

## Phase 2: Hugot (Pure Go Embedding)

**Date:** December 30, 2025
**Status:** Superseded by MLX integration

### Why the Change

The "It Just Works" philosophy demanded zero external dependencies. Hugot provided:

- **Pure Go backend** - No CGO required, works everywhere
- **Auto-download** - Downloads models from HuggingFace automatically
- **Production-tested** - Used by Knights Analytics in production
- **Higher-level API** - Handles tokenization, batching, model loading

### Model Comparison

| Provider | Model | MTEB Score | Dimensions | Context |
|----------|-------|------------|------------|---------|
| Ollama | nomic-embed-text-v2-moe | ~81% | 768 | 8192 |
| **Hugot** | all-MiniLM-L6-v2 | ~70% | 384 | 512 |
| Static | hash-based | N/A | 256 | infinite |

### The Quality vs Simplicity Trade-off

Our hybrid search architecture (BM25 + Semantic with RRF fusion) mitigated the quality loss:

```
final_score = RRF(0.35 * BM25_score, 0.65 * Semantic_score)
```

| Configuration | Estimated Accuracy | Delta from Ollama |
|---------------|-------------------|-------------------|
| Ollama Hybrid | 77.2% | baseline |
| **Hugot Hybrid** | 70.0% | -9.3% relative |
| Static Hybrid | 47.3% | -38.7% relative |

**Key Insight:** The 9.3% quality loss was deemed acceptable for the massive UX improvement (zero-config operation).

### What Worked

```
+ True "It Just Works" - Download amanmcp, run it, done
+ No external dependencies
+ Smaller footprint - 22MB model vs 274MB
+ Faster first run - 5 seconds download vs 30+ seconds
+ Cross-platform - Pure Go works on Windows without issues
```

### What Didn't Work

The quality loss became noticeable in real-world code search:
- Shorter context (512 tokens) truncated large functions
- Lower dimensions (384 vs 768) reduced semantic expressiveness
- Code-specific queries often returned less relevant results

**Lesson 2:** For code search, embedding quality matters more than for general text.

## Phase 3: MLX (Apple Silicon Optimization)

**Date:** January 8, 2026
**Status:** Superseded as default by ADR-037 (still available as opt-in)

### Why the Change

Embedding generation consumed 80% of indexing time. For 6,500 chunks:
- **Ollama:** ~48 minutes
- **MLX:** ~3 minutes (16x faster)

This slow iteration hurt developer experience when tuning search quality.

### The Performance Breakthrough

| Backend | Qwen3-8B Status | Batch (32 texts) | Speed vs Ollama |
|---------|-----------------|------------------|-----------------|
| **MLX** | Works perfectly | ~60ms | **55x faster** |
| TEI | Crashes on warm-up | N/A | N/A |
| Ollama | Works | ~3300ms | Baseline |

MLX achieved this through:
- Native Apple Silicon optimization
- Parallel batch processing
- Efficient Metal GPU utilization

### The Qwen3 Advantage

| Benchmark | Qwen3-8B | nomic-embed | Gap |
|-----------|----------|-------------|-----|
| MTEB Code | ~80.68% | ~77.2% | +3.5% |
| Multilingual | ~70.58% | ~62.10% | +8.5% |
| Dimensions | 4096 | 768 | 5x more |

### What We Gained

```
+ Index time: 48 min → 3 min (16x faster)
+ Same or better embedding quality (Qwen3-8B)
+ Rapid iteration enabled
+ Higher dimensional embeddings (4096 dims)
```

### What We Discovered

During extended development sessions, a critical issue emerged:

```
MLX server + Ollama (for LLM) = Combined RAM pressure
  ↓
System becomes sluggish
  ↓
24GB RAM users hit limits during indexing + search
```

**Lesson 3:** Speed optimization can't ignore resource constraints.

## Phase 4: Ollama Default, MLX Opt-In (Current)

**Date:** January 14, 2026
**Status:** Current

### Why the Change

Real-world usage revealed the true priority order:

| Use Case | Typical Frequency | MLX Benefit |
|----------|-------------------|-------------|
| Initial indexing | Once per project | Significant (16x faster) |
| Incremental reindex | Rare (file watcher) | Minimal |
| Search queries | Very frequent | None (same latency) |
| Development sessions | Hours/day | RAM overhead is net negative |

**Key Insight:** For typical development workflows, search latency matters more than indexing speed, and both have similar search latency.

### Current Configuration

```yaml
# Default (Ollama) - recommended for most users
embeddings:
  provider: ollama

# Opt-in (MLX) - for speed-critical operations
embeddings:
  provider: mlx
```

### Usage Recommendations

| Scenario | Recommended Backend |
|----------|---------------------|
| Day-to-day development | Ollama (default) |
| Large initial indexing (>10k files) | MLX (then switch back) |
| RAM-constrained system (<16GB) | Ollama |
| Speed-critical batch operations | MLX |

## Lessons for Model Selection

### 1. Requirements Drive Changes

Each phase had fundamentally different priorities:

| Phase | Primary Priority | Secondary |
|-------|-----------------|-----------|
| 1 (nomic) | Quality | Local-first |
| 2 (Hugot) | Zero-config | Quality |
| 3 (MLX) | Speed | Quality |
| 4 (Ollama+MLX) | Balance | Flexibility |

**Lesson:** Your priorities will change. Build for flexibility.

### 2. Quality vs Complexity Trade-off

Each choice occupied a different point on the trade-off curve:

```
Quality
  ↑
  │     * Qwen3-8B (MLX/Ollama)
  │   * nomic-embed-text
  │
  │  * MiniLM (Hugot)
  │
  │* Static
  └────────────────────→ Simplicity
```

**Lesson:** There's no universally "best" model - only best for your constraints.

### 3. Backend Matters as Much as Model

Same Qwen3-8B model, dramatically different performance:

| Backend | Speed | RAM Usage | Setup |
|---------|-------|-----------|-------|
| Ollama | Baseline | Low | Easy |
| MLX | 55x faster | High | Moderate |

**Lesson:** Optimization work multiplies model capability.

### 4. Code-Specific Models for Code Search

General models struggle with code vocabulary:

```python
# What a general model sees:
"func (e *Engine) Search" → tokens: [func, (, e, *, Engine, ), Search]

# What a code model understands:
"func (e *Engine) Search" → Go method receiver pattern on Engine type
```

**Lesson:** Specialized models exist for a reason. Use them.

### 5. User Experience Trumps Benchmarks

81% MTEB with 5-minute setup vs 70% MTEB with instant setup: users chose instant.

**Lesson:** A working solution beats a perfect one that's hard to use.

## Model Selection Decision Framework

Use this framework when choosing embedding models:

### Step 1: Define Constraints

```
[ ] Maximum RAM available: ______ GB
[ ] Maximum setup time acceptable: ______ minutes
[ ] Quality threshold (MTEB): ______ %
[ ] Context window needed: ______ tokens
[ ] Platform support: [ ] macOS [ ] Linux [ ] Windows
```

### Step 2: Match to Tier

| Tier | When to Use | Example Models |
|------|-------------|----------------|
| Code-Specialized | Code search is primary use case | nomic-embed-code, jina-v2-base-code |
| High Quality General | Mixed content, quality critical | Qwen3-8B, nomic-embed-text |
| Balanced | Day-to-day development | qwen3-0.6b via Ollama |
| Lightweight | RAM-constrained, speed-critical | all-MiniLM, Hugot |
| Static | Offline-only, zero dependencies | Hash-based |

### Step 3: Consider Evolution Path

Build in flexibility from day one:

```go
// Good: Provider abstraction allows swapping
type Embedder interface {
    Embed(ctx context.Context, texts []string) ([][]float32, error)
}

// Bad: Hardcoded to specific model
func embed(text string) []float32 {
    return ollama.Embed("nomic-embed-text", text)
}
```

## Applying This to Your Project

### For New Projects

1. Start with Ollama + qwen3-embedding (balanced default)
2. Build abstraction layer for future swapping
3. Measure actual quality before optimizing
4. Only add complexity when measured need exists

### For Existing Projects

1. Benchmark current embedding quality
2. Identify the bottleneck (quality? speed? setup?)
3. Evaluate one tier up/down based on bottleneck
4. Test with real queries, not just benchmarks

### Migration Checklist

When switching embedding models:

- [ ] Dimensions match (or plan for reindex)
- [ ] Context window sufficient for your chunks
- [ ] Fallback path exists if new model fails
- [ ] Documentation updated with new requirements
- [ ] Performance measured on representative workload

## See Also

- [Embedding Models](./embedding-models.md) - Current model comparison and recommendations
- [Specialization vs Generalization](./specialization-vs-generalization.md) - Trade-offs in model selection
- [Query Expansion](./query-expansion-asymmetric.md) - Improving search quality at query time

---

**Original Source:** `archive/docs-v1/decisions-superseded/ADR-002-embedding-model-nomic.md`
**Related ADRs:** ADR-002, ADR-005, ADR-023, ADR-035, ADR-037
**Status:** Historical (documents evolution through January 2026)
**Last Updated:** 2026-01-16
