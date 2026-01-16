# Research & Technical Decisions

This section documents the research and analysis behind AmanMCP's technical choices. These are in-depth explorations that show what alternatives we evaluated and why we made specific decisions.

**Audience**: Developers, researchers, and contributors who want to understand the "why" behind technical choices.

---

## Available Research

| Document | Question | Key Finding | Date |
|----------|----------|-------------|------|
| **Synthesis & Series** | | | |
| [Search Quality Improvement Series](search-quality-improvement-series.md) | How to solve vocabulary mismatch comprehensively? | **Synthesis**: Contextual retrieval (index-time) for vectors + query expansion (query-time) for BM25. Pass rate improved 60% to 92%. | 2026-01-16 |
| **Search & Retrieval** | | | |
| [Contextual Retrieval](contextual-retrieval-decision.md) | How to bridge vocabulary mismatch between queries and code? | Prepend LLM-generated context to chunks before embedding. Pattern fallback ensures zero-config. Based on Anthropic's research. | 2026-01-16 |
| [Query Expansion Asymmetry](query-expansion-asymmetric.md) | Should we expand queries for all search backends? | Expand for BM25, use original for vectors. Expansion helps BM25 but dilutes embeddings. | 2026-01-16 |
| [RRF Fusion Rationale](rrf-fusion-rationale.md) | How to combine BM25 and vector search results? | Reciprocal Rank Fusion (k=60) provides simple, effective combination without training. | 2026-01-16 |
| [Vocabulary Mismatch Analysis](vocabulary-mismatch-analysis.md) | Why does semantic search fail for code? | Users say "search function", code says `func Search`. Root cause of 40% of search failures. | 2026-01-16 |
| [Dogfooding Methodology](dogfooding-methodology.md) | How to validate RAG search quality? | Tiered query system with 5 Whys root cause analysis. Unit tests don't catch semantic gaps. | 2026-01-16 |
| [Contextual Retrieval Regression](contextual-retrieval-regression.md) | How can enhancements cause regressions? | Small embedding models + contextual prefixes can cluster in embedding space. Test components in isolation. | 2026-01-16 |
| **Embeddings & Models** | | | |
| [Embedding Models](embedding-models.md) | Which embedding model for code search? | qwen3-0.6b balances quality and resource usage. Code-specialized models like nomic-embed-code can improve retrieval 7-8%. | 2026-01-14 |
| [Embedding Backend Evolution](embedding-backend-evolution.md) | Which embedding backend by default? | Ollama default (lower RAM), MLX opt-in (16x faster). RAM matters more than speed for development. | 2026-01-16 |
| [Embedding Optimization](embedding-optimization.md) | How to optimize embedding performance? | MLX vs TEI benchmarking. Batch size tuning, GPU utilization patterns. | 2026-01-16 |
| [Embedding Model Evolution](embedding-model-evolution.md) | How did our embedding choice evolve? | nomic → Hugot → Qwen3. Each transition taught new lessons. | 2026-01-16 |
| **Infrastructure & Storage** | | | |
| [SQLite FTS5 vs Bleve](sqlite-vs-bleve.md) | Which BM25 backend for concurrent access? | SQLite FTS5 enables concurrent access (WAL mode) solving multi-process issues. Pure Go, production-proven. | 2026-01-14 |
| [Vector Database Selection](vector-database-selection.md) | Which vector database for local-first? | USearch (historical) → coder/hnsw. Pure Go, scales to 300K+ vectors. | 2026-01-16 |
| [Specialization vs Generalization](specialization-vs-generalization.md) | Should we use specialized or general models? | Specialized models excel in their domain but general models provide better fallback for varied content. | 2026-01-14 |
| **Indexing & Parsing** | | | |
| [Tree-sitter Chunking](tree-sitter-chunking.md) | How to chunk code intelligently? | AST-aware boundaries preserve semantic units. CGO required but worth it. | 2026-01-16 |
| [MLX Migration Case Study](mlx-migration-case-study.md) | How to plan and execute performance migrations? | Validate before implementing, always have fallback, prefer auto-detection. MLX delivered 16x indexing speedup. | 2026-01-16 |
| **Observability** | | | |
| [Observability for RAG](observability-for-rag.md) | How to observe RAG systems? | RAG vs agents distinction. Structured logging, metric design for search quality. | 2026-01-16 |

---

## How to Use These

- **Deciding on alternatives?** Check if we've already evaluated it
- **Proposing changes?** Reference these docs in your proposal
- **Learning?** See how we approach technical decisions
- **Junior engineers**: Study the decision-making process and tradeoff analysis

---

## What Makes Good Research Documentation?

Our research docs follow this structure:

1. **Problem Statement** - What question are we answering?
2. **Requirements** - What constraints do we have?
3. **Alternatives Evaluated** - What options did we consider?
4. **Analysis** - Data, benchmarks, comparisons
5. **Decision** - What we chose and why
6. **Results** - Did it work as expected?

---

## Contributing Research

Have an idea for improvement? Check these docs first, then:

1. **File an issue** referencing the relevant research
2. **Explain what's changed** (new models, new data, new requirements)
3. **Propose updated analysis** if needed
4. **Run experiments** and share results

We welcome research contributions that:
- Validate or challenge existing decisions
- Explore new alternatives
- Provide better data or benchmarks
- Consider new use cases or constraints

---

## Related Documentation

- [Concepts](../concepts/) - How systems work (explanatory)
- [Articles](../articles/) - Insights and thought leadership (narrative)
- [Architecture](../reference/architecture/) - System design (technical specs)
