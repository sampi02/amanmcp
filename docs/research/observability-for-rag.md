# Observability for RAG Systems: When to Apply AI Best Practices

> **Learning Objectives:**
> - Understand the difference between agentic AI and RAG pipelines
> - Learn how to critically evaluate industry recommendations
> - Apply appropriate observability for different system types
>
> **Prerequisites:**
> - Basic understanding of RAG (Retrieval Augmented Generation)
> - Familiarity with logging and telemetry concepts
>
> **Based on:** [LangChain Blog: In AI, traces document the app](https://blog.langchain.com/in-software-the-code-documents-the-app-in-ai-the-traces-do/)
>
> **Audience:** AI engineers, RAG developers, anyone building search systems

---

## TL;DR

LangChain argues that execution traces are essential for AI systems because models make runtime decisions outside the codebase. However, RAG pipelines are fundamentally deterministic: given the same query and index, they produce predictable results. For RAG systems, traditional observability (structured logging, metrics, profiling) is sufficient and appropriate - full tracing infrastructure is often overkill.

---

## The Source: LangChain's Argument

### Main Thesis

> "In traditional software, code documents behavior. In AI agents, execution traces become the source of truth because the model makes runtime decisions outside the codebase."

LangChain's article makes several compelling points:

1. **Code as Scaffolding:** AI agent code orchestrates models and tools, but actual decisions happen at runtime in the model
2. **Traces as Documentation:** The sequence of steps an agent takes documents the real logic of your application
3. **Debugging Shift:** Traditional debugging (breakpoints, code review) doesn't work for agent behavior
4. **Essential Infrastructure:** Observability platforms are necessary for understanding AI systems

### Why This Argument is Persuasive

The article resonates because it identifies a real problem: when an LLM decides which tool to call, what arguments to pass, and when to stop, the code truly doesn't document behavior. You can't understand why a chatbot failed by reading the source code - you need to see what the model actually did.

---

## The Key Distinction: Agents vs RAG

### AI Agents: Non-Deterministic

Agentic systems have these characteristics:

| Property | Description | Example |
|----------|-------------|---------|
| **Runtime decisions** | Model chooses actions dynamically | "Should I search the web or use the calculator?" |
| **Multi-step reasoning** | Chains of decisions compound | Tool A result affects Tool B selection |
| **Non-reproducible** | Same input can yield different paths | Temperature > 0, prompt variation |
| **Branching logic** | Code paths depend on model output | If model says X, do Y; else do Z |

For these systems, traces truly document behavior that code cannot.

### RAG Pipelines: Deterministic

RAG search systems have fundamentally different properties:

| Property | Description | Example |
|----------|-------------|---------|
| **Fixed pipeline** | Same steps every query | BM25 -> Vector -> Fusion -> Results |
| **Reproducible** | Same input yields same output | Query "search function" always runs same code |
| **No runtime decisions** | Model doesn't choose actions | Pipeline is defined in code |
| **Predictable branching** | All paths known at compile time | If BM25 fails, return error |

```
User Query -> BM25 Search + Vector Search -> RRF Fusion -> Results
(deterministic at each step)
```

The "intelligence" in a RAG system comes from:
1. **Embedding models** - deterministic given weights (no temperature)
2. **BM25 scoring** - deterministic algorithm
3. **RRF fusion** - fixed formula (k=60)
4. **Reranking** - deterministic given weights

**This is traditional software.** The code *does* document the behavior.

---

## When Full Observability Makes Sense

Full tracing infrastructure is appropriate when:

### 1. Multi-Turn Conversations

```
User: "Find flights to Paris"
Agent: Searches flights API
User: "Actually, make it London"
Agent: Must remember context, search again
User: "Book the cheapest one"
Agent: Needs full conversation trace to understand "the cheapest one"
```

Here, traces document the conversational state machine that code cannot.

### 2. Tool-Calling Agents

```python
# Agent code - doesn't document what actually happens
agent = Agent(tools=[search, calculator, database])
result = agent.run("What's the population density of France?")

# What might happen (not in code):
# 1. Model calls database("France population") -> 67M
# 2. Model calls database("France area") -> 643,801 km^2
# 3. Model calls calculator("67000000 / 643801") -> 104.1
# 4. Model returns "104.1 people per km^2"
```

The execution path is invisible without traces.

### 3. Autonomous Loops

Systems that run without human intervention need traces to understand:
- Why did it decide to stop?
- What triggered the error recovery?
- How did it handle the edge case?

---

## When Traditional Telemetry Suffices

Standard observability works when your system has these properties:

### 1. Single-Pass Pipelines

```go
func (e *Engine) Search(query string) ([]Result, error) {
    bm25Results := e.bm25.Search(query)      // Step 1: always happens
    vecResults := e.vector.Search(query)      // Step 2: always happens
    return e.fusion.Combine(bm25Results, vecResults)  // Step 3: always happens
}
```

Reading this code tells you exactly what happens. A trace adds no information.

### 2. Stateless Request/Response

Each query is independent - no conversation history, no accumulated state, no context from previous requests. The request contains everything needed to understand behavior.

### 3. Debuggable with Breakpoints

You can step through the code with a debugger and understand why a query returned specific results. The logic is in the code, not in an LLM's "reasoning."

---

## What We Actually Need for RAG

| Concept | Article Recommends | RAG Reality | Our Approach |
|---------|-------------------|-------------|--------------|
| Full traces | Essential | Often overkill | Structured logging |
| Decision quality | Track it | Zero-result tracking | QueryMetrics |
| Latency monitoring | Yes | Yes | Latency histograms |
| Query patterns | Yes | Yes | Top terms, query types |
| Replay debugging | Recommended | Not needed | Deterministic pipeline |
| Eval-driven testing | Essential | Useful | Tier 1/2 validation |

### The QueryMetrics Approach

Instead of full traces, we track aggregate metrics:

```go
type QueryMetrics struct {
    queryTypes      map[QueryType]int64     // What kinds of searches
    topTerms        *lru.Cache[string, int64]  // Popular search terms
    zeroResults     *CircularBuffer[string]    // Failed queries
    latencies       map[LatencyBucket]int64    // Performance distribution
    totalQueries    int64                      // Volume
    zeroResultCount int64                      // Failure rate
}
```

This tells us:
- **What's being searched:** Query type distribution
- **What's failing:** Zero-result queries for improvement
- **How fast:** Latency percentiles
- **Volume trends:** Usage patterns over time

### Why This is Enough

For debugging a specific query, we don't need traces. We need:

1. **The query itself:** What did the user search for?
2. **The results:** What came back?
3. **The timing:** Was it slow?

All of this fits in a structured log line:

```json
{
    "level": "info",
    "query": "search function",
    "results_count": 5,
    "latency_ms": 45,
    "bm25_hits": 12,
    "vector_hits": 8,
    "timestamp": "2026-01-16T10:30:00Z"
}
```

To reproduce: run the same query. You'll get the same results. No trace needed.

---

## Lessons for Evaluating Best Practices

### 1. Consider the Source's Context

LangChain builds agent tooling. Their products include:
- LangSmith (trace platform)
- LangGraph (agent framework)
- Templates for agentic applications

Their perspective is calibrated for their use case: complex, multi-step, model-driven applications. When they say "traces are essential," they mean "traces are essential *for the systems we build*."

**Question to ask:** Does the recommendation come from someone who builds systems like mine, or systems fundamentally different from mine?

### 2. Classify Your System

Before adopting observability recommendations, categorize your system:

| Question | Agent Answer | RAG Answer |
|----------|--------------|------------|
| Does an LLM make decisions at runtime? | Yes | No |
| Is the execution path predictable? | No | Yes |
| Can you debug with breakpoints? | Often not | Yes |
| Does state accumulate across requests? | Usually | Rarely |

If your answers lean toward "RAG Answer," traditional observability likely suffices.

### 3. Match Observability to Complexity

```
System Complexity → Observability Complexity

Simple function:     Logging
Microservice:        Logging + Metrics
Distributed system:  Logging + Metrics + Basic Tracing
Agent system:        Full Observability Platform
```

Over-engineering observability has costs:
- **Integration overhead:** Platforms require setup, maintenance
- **Runtime overhead:** Traces add latency and storage
- **Cognitive overhead:** More data to interpret
- **Dependency risk:** External services can fail

### 4. Identify What You Actually Need to Know

For a RAG system, the key questions are:

| Question | How to Answer | Full Traces? |
|----------|---------------|--------------|
| "Why did this query return no results?" | Check BM25 terms, vector similarity | No |
| "Why is search slow?" | Latency histogram, profiling | No |
| "What are users searching for?" | Query term aggregation | No |
| "Is search quality improving?" | Tier 1/2 validation pass rate | No |

None of these require per-request execution traces.

---

## What We Chose NOT to Implement

### Full LangSmith/Langfuse Integration

**Why not:** AmanMCP is local-first and privacy-focused. Cloud observability platforms:
- Send query data to external servers
- Require internet connectivity
- Add external dependencies
- Contradict our privacy philosophy

### Complex Trace Hierarchies

**Why not:** Agent traces have hierarchies (sessions -> tasks -> turns -> spans) because agents have conversational state. RAG search is stateless request/response - one level is enough.

### OpenTelemetry Integration

**Why not:** OTEL is powerful but complex. For a deterministic pipeline:
- Integration cost exceeds benefit
- Standard logging achieves the same goals
- Profiling tools (pprof) handle performance

### Replay Debugging Infrastructure

**Why not:** Replay debugging captures inputs and outputs to reproduce agent behavior later. For RAG:
- Same query + same index = same results
- No special infrastructure needed
- Just run the query again

---

## The Mindset Shift: Empirical Validation

While full traces don't apply, one concept from the article does: **empirical validation**.

The article argues that you can't trust AI systems to work correctly just because the code compiles. You must verify with real queries and measure actual results.

This applies to RAG too:

```yaml
# Tier 1 Validation Queries (must pass)
validation:
  tier1:
    - query: "search function"
      expected_file: "internal/search/engine.go"
    - query: "BM25 implementation"
      expected_file: "internal/search/bm25.go"
    - query: "embedding model"
      expected_file: "internal/embed/ollama.go"
```

Running these queries against a fresh index validates that search quality meets expectations. This is the RAG equivalent of agent traces: empirical evidence that the system works.

---

## Summary: Right-Sizing Your Observability

| System Type | Observability Approach |
|-------------|----------------------|
| **AI Agents** | Full tracing (LangSmith, Langfuse) |
| **Multi-turn Chat** | Conversation traces |
| **RAG Search** | Metrics + Structured Logging |
| **Simple API** | Request logging |

For RAG systems:

1. **Do implement:** Query metrics, zero-result tracking, latency monitoring, structured logging
2. **Do consider:** Query trace export for debugging (optional)
3. **Don't implement:** Full trace platforms, complex hierarchies, cloud observability (for local-first systems)

The goal is observability that matches your system's complexity - not observability that matches someone else's system.

---

## See Also

- [Hybrid Search](../concepts/hybrid-search.md) - How RAG search pipelines work
- [Vector Search Concepts](../concepts/vector-search-concepts.md) - Understanding semantic search
- [Embedding Models](./embedding-models.md) - Model selection for RAG

---

**Original Analysis:** `archive/research/2026-01-11-ai-observability-analysis.md`
**Last Updated:** 2026-01-16
