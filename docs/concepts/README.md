# Concepts: Understanding How AmanMCP Works

This section explains the core concepts and architecture behind AmanMCP. Read these to understand **how** the system works internally.

**Audience**: Users who want to understand the "why" and "how", developers, and anyone curious about the internals.

```mermaid
graph TD
    START[New to AmanMCP] --> LEVEL{What do you<br/>want to know?}

    LEVEL -->|Just use it| G[Getting Started Guide]
    LEVEL -->|How search works| S[Search Concepts]
    LEVEL -->|Full architecture| A[Architecture Docs]

    S --> S1[Hybrid Search]
    S --> S2[Vector Search]
    S --> S3[Two-Stage Retrieval]

    A --> A1[Tree-sitter Chunking]
    A --> A2[MCP Protocol]
    A --> A3[Full System Design]

    S1 --> S2
    S2 --> S3

    A1 --> A2
    A2 --> A3

    style START fill:#e1f5ff
    style G fill:#c8e6c9
    style S fill:#e1f5ff
    style A fill:#ffe0b2
    style S1 fill:#c8e6c9
    style S2 fill:#c8e6c9
    style S3 fill:#c8e6c9
```

---

## Core Concepts

| Concept | What You'll Learn | Read This If... |
|---------|-------------------|-----------------|
| [Hybrid Search](hybrid-search.md) | How BM25 keyword search + semantic vector search work together | You want to understand why results are relevant |
| [Vector Search](vector-search-concepts.md) | Embeddings, HNSW index, semantic similarity | You're curious about "AI search" internals |
| [Two-Stage Retrieval](two-stage-retrieval.md) | Why we search twice (fast filter → precise ranking) | You want to optimize search performance |
| [Tree-sitter AST Chunking](tree-sitter-guide.md) | How we extract functions/classes from code | You're curious about code parsing |
| [MCP Protocol](mcp-protocol.md) | How AmanMCP talks to Claude | You want to understand the integration |

---

## Learning Path

### Beginner: "I just want to use it"
Start with [Getting Started](../getting-started/) - you don't need to understand concepts to use AmanMCP.

### Intermediate: "I want to understand how search works"
1. [Hybrid Search](hybrid-search.md) - The foundation
2. [Vector Search](vector-search-concepts.md) - How semantic search works
3. [Two-Stage Retrieval](two-stage-retrieval.md) - Why it's fast and accurate

### Advanced: "I want to understand the architecture"
1. [Tree-sitter AST Chunking](tree-sitter-guide.md) - Code parsing internals
2. [MCP Protocol](mcp-protocol.md) - Integration layer
3. [Architecture Overview](../reference/architecture/architecture.md) - Full system design

---

## Concepts vs Guides vs Research

| Section | Focus | Example |
|---------|-------|---------|
| **Concepts** (here) | How systems work | "How does hybrid search combine BM25 and vectors?" |
| [Guides](../guides/) | How to do tasks | "How do I switch to MLX embeddings?" |
| [Research](../research/) | Why we chose this | "Why SQLite FTS5 instead of Bleve?" |

```mermaid
%%{init: {'theme': 'base', 'themeVariables': { 'primaryColor': '#e1f5ff', 'primaryTextColor': '#1a1a1a', 'primaryBorderColor': '#3498db', 'lineColor': '#3498db', 'secondaryColor': '#c8e6c9', 'tertiaryColor': '#ffe0b2'}}}%%
graph TB
    subgraph Concepts["Concepts: HOW it works"]
        C1[Hybrid Search<br/>Combines BM25 + vectors]
        C2[Vector Search<br/>Embeddings & HNSW]
        C3[Two-Stage Retrieval<br/>Filter then rank]
        C4[Tree-sitter<br/>Code parsing]
        C5[MCP Protocol<br/>AI integration]
    end

    subgraph Guides["Guides: HOW to do tasks"]
        G1[Switch Embedders<br/>MLX vs Ollama]
        G2[Configure Search<br/>Weights & filters]
        G3[Optimize Performance<br/>Tuning parameters]
    end

    subgraph Research["Research: WHY we chose"]
        R1[SQLite vs Bleve<br/>Performance analysis]
        R2[HNSW vs Annoy<br/>Memory tradeoffs]
        R3[BM25 weights<br/>Empirical results]
    end

    Question[I have a question...] --> Q1{What kind?}

    Q1 -->|How does X work?| Concepts
    Q1 -->|How do I do Y?| Guides
    Q1 -->|Why not Z?| Research

    C1 --> R3
    C2 --> R2
    G1 --> C2
    G2 --> C1
    G3 --> C3

    style Concepts fill:#e1f5ff,stroke:#3498db,stroke-width:2px
    style Guides fill:#c8e6c9,stroke:#27ae60,stroke-width:2px
    style Research fill:#ffe0b2,stroke:#f39c12,stroke-width:2px
    style Question fill:#fff9c4,stroke:#f39c12,stroke-width:2px
```

---

## Visual Learning

Most concept docs include:
- 📊 **Mermaid diagrams** - Flowcharts and sequence diagrams
- 📈 **Performance charts** - Speed and quality comparisons
- 🎯 **Examples** - Real queries and results

---

## Contribute

Found a concept confusing? Want to add diagrams or examples?
1. File an issue with the concept name
2. Suggest improvements (PRs welcome!)
3. Ask questions - confusion indicates documentation gaps

---

## Related Documentation

- [Getting Started](../getting-started/) - Installation and first steps
- [Guides](../guides/) - Task-based how-tos
- [Architecture Reference](../reference/architecture/) - Technical specifications
- [Research](../research/) - Technical decisions and analysis
