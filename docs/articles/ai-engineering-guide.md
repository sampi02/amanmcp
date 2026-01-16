# The 80/20 AI Engineering Learning Guide

*Mastering AI Engineering through the Pareto Principle: 80% results with 20% effort*

---

> **Learning Objectives:**
>
> - Follow a practical path to AI engineering competency
> - Focus on the 20% that gives 80% of results
> - Build from foundations through production
>
> **Audience:** Software engineers transitioning to AI, students, self-learners

---

## TL;DR

You don't need a PhD in Math. Strong software engineering fundamentals combined with pragmatic understanding of how to *orchestrate* intelligence makes an AI Engineer. This guide distills the most high-impact resources available in 2025-2026, moving beyond generic "data science" roadmaps to focus specifically on building robust, scalable, and intelligent systems using modern AI models (LLMs/SLMs).

---

## The 80/20 Philosophy

> "To master AI Engineering, you don't need a PhD in Math. You need strong software engineering fundamentals combined with pragmatic understanding of how to *orchestrate* intelligence."

The Pareto Principle applies powerfully to AI engineering education. A small subset of concepts, tools, and resources will get you most of the way there. This guide identifies that critical 20%.

```
Phase I:   Foundation      → Speak the language of data and infrastructure
Phase II:  The Core        → Build applications that "think" using LLMs
Phase III: Agentic AI      → Build systems that DO things, not just talk
Phase IV:  Production      → Make it reliable, cheap, and safe
```

---

## The Roadmap Overview

| Phase | Focus Area | Key Concepts (The 20%) | Estimated Time |
|-------|------------|------------------------|----------------|
| **I** | **Foundation** | Python for AI, Git, Docker, Linear Algebra nuances | 2-4 Weeks |
| **II** | **The Core (LLMs)** | Prompt Engineering, RAG, Embeddings | 4-6 Weeks |
| **III** | **The System (Agents)** | Tool use, Planning, Multi-Agent Orchestration, MCP | 4-6 Weeks |
| **IV** | **Production (LLMOps)** | Evaluation (Evals), Fine-tuning, Deployment, Monitoring | Ongoing |

### Learning Path Timeline

```mermaid
%%{init: {'theme': 'base', 'themeVariables': { 'primaryColor': '#e1f5ff', 'primaryTextColor': '#1a1a1a', 'primaryBorderColor': '#0066cc', 'lineColor': '#0066cc', 'secondaryColor': '#fff4e6', 'tertiaryColor': '#f0f0f0'}}}%%
timeline
    title AI Engineering Learning Journey
    section Phase I Foundation
        Week 1-2 : Python for AI (numpy, pandas, pydantic) + Docker basics
        Week 2-3 : Neural Networks (3Blue1Brown series) + Vectors and Matrices
        Week 3-4 : AI Fundamentals (Andrew Ng course) + What AI can and cannot do
    section Phase II Core LLMs
        Week 4-6 : LLM Basics (Transformers architecture, Karpathy GPT video)
        Week 6-8 : Prompt Engineering (Chain-of-Thought, Structured prompting)
        Week 8-10 : RAG Systems (Vector databases, Embedding models, Context retrieval)
    section Phase III Agentic AI
        Week 10-12 : Tool Use (Function calling, API integration)
        Week 12-14 : Orchestration (LangGraph patterns, State management)
        Week 14-16 : Planning (ReAct loops, Multi-agent systems, MCP protocol)
    section Phase IV Production
        Week 16+ : Evaluation (Build eval datasets, Ragas, DeepEval)
        Ongoing : Monitoring (Tracing systems, LangSmith, Phoenix)
        Ongoing : Optimization (Cost reduction, Latency tuning, Semantic caching)
```

---

## Phase I: Foundation (2-4 Weeks)

*Goal: Speak the language of data and infrastructure.*

Before diving into LLMs and agents, you need a solid foundation. This phase is about understanding the primitives that power modern AI systems.

### Must-Read/Watch

- **[Video] 3Blue1Brown - Neural Networks**
  - The most intuitive visual explanation of the math that matters
  - *Why:* You need to understand vectors and matrices to understand Embeddings
  - *Time:* 4 videos, ~1 hour total

- **[Course] DeepLearning.AI - AI for Everyone (Andrew Ng)**
  - Breaking through the hype with clear fundamentals
  - *Why:* Foundations of what AI can and cannot do
  - *Time:* 6 hours

### The 20% Skills

| Skill | What to Focus On | Why It Matters |
|-------|------------------|----------------|
| **Python for AI** | `numpy`, `pandas`, `pydantic` | Pydantic is crucial for structured outputs from LLMs |
| **Containerization** | Docker basics | Non-negotiable for reproducible environments |
| **Version Control** | Git workflows | Every AI project needs experiment tracking |

**Key Insight:** Many tutorials jump straight into LLMs. Resist the urge. A week spent on numpy broadcasting and pydantic models will save you weeks of debugging later.

---

## Phase II: The Core (4-6 Weeks)

*Goal: Build applications that "think" using LLMs.*

This is where you learn to harness the power of Large Language Models. The focus is on practical application, not theory.

### Essential Resources

- **[Book] "The LLM Engineering Handbook"** (Paul Iusztin)
  - Practical guide to building and deploying LLM applications
  - Covers the full lifecycle from prototype to production

- **[Book] "Build a Large Language Model (from Scratch)"** (Sebastian Raschka)
  - If you want to understand the "magic" of Transformers
  - Deep dive into the architecture

- **[Video] Andrej Karpathy - Let's build GPT**
  - The definitive "Zero to Hero" video
  - *Action:* Coding along with this video is worth 100 hours of passive reading
  - Builds intuition that serves you for years

- **[Course] DeepLearning.AI - Generative AI with LLMs**
  - Covers the lifecycle of GenAI projects
  - Industry-standard practices

### The 20% Skills

| Skill | Focus Areas | Why It Matters |
|-------|-------------|----------------|
| **Prompt Engineering** | Chain-of-Thought, ReAct, structured prompting | Not just "asking nicely" - this is a core engineering skill |
| **RAG** | Vector Databases, Embedding models | The bridge between frozen model knowledge and your dynamic data |
| **Frameworks** | LangChain **or** LlamaIndex | Pick one and master it; understand they're wrappers around API calls |

**Key Insight:** RAG (Retrieval Augmented Generation) is the most practical pattern for enterprise AI. It lets you add your data to any LLM without fine-tuning.

### RAG Deep Dive

```mermaid
%%{init: {'theme': 'base', 'themeVariables': { 'primaryColor': '#e1f5ff', 'primaryTextColor': '#1a1a1a', 'primaryBorderColor': '#0066cc', 'lineColor': '#0066cc', 'secondaryColor': '#fff4e6', 'tertiaryColor': '#f0f0f0'}}}%%
flowchart LR
    Query["Query<br/><i>How do we...</i>"]
    Embed["Embed<br/>768-dim vector"]
    Search["Vector Search<br/>Top-k matches"]
    Retrieval["Context Retrieval<br/>Relevant chunks"]
    Generation["LLM Generation<br/>Grounded response"]

    Query --> Embed --> Search --> Retrieval --> Generation

    style Query fill:#4caf50,stroke:#2e7d32,stroke-width:2px
    style Embed fill:#e1f5ff,stroke:#0066cc,stroke-width:2px
    style Search fill:#fff4e6,stroke:#ff9800,stroke-width:2px
    style Retrieval fill:#f3e5f5,stroke:#9c27b0,stroke-width:2px
    style Generation fill:#2196f3,stroke:#1565c0,stroke-width:2px
```

#### RAG Architecture Sequence Diagram

```mermaid
%%{init: {'theme': 'base', 'themeVariables': { 'primaryColor': '#e1f5ff', 'primaryTextColor': '#1a1a1a', 'primaryBorderColor': '#0066cc', 'lineColor': '#0066cc', 'secondaryColor': '#fff4e6', 'tertiaryColor': '#f0f0f0', 'actorBorder': '#0066cc', 'actorBkg': '#e1f5ff', 'noteBkgColor': '#fff4e6', 'noteBorderColor': '#ff9800'}}}%%
sequenceDiagram
    autonumber
    participant User
    participant App as RAG Application
    participant Emb as Embedding Model
    participant VDB as Vector Database
    participant LLM as Language Model

    User->>App: Query: "How does retry logic work?"

    Note over App: Query Processing
    App->>Emb: Embed query text
    Emb-->>App: Query vector [768-dim]

    Note over App,VDB: Retrieval Phase
    App->>VDB: Vector similarity search
    VDB-->>App: Top-K similar chunks (k=5)

    Note over App: Rank & Filter
    App->>App: Re-rank by relevance
    App->>App: Construct context window

    Note over App,LLM: Augmentation & Generation
    App->>LLM: Prompt + Retrieved Context
    activate LLM
    LLM->>LLM: Process context
    LLM->>LLM: Generate response
    deactivate LLM
    LLM-->>App: Grounded answer with citations

    App-->>User: "Retry logic uses exponential backoff..."

    Note over User,LLM: Optional: Feedback Loop
    User->>App: Thumbs up/down
    App->>VDB: Update chunk metadata
```

Study:

- Vector Databases: Chroma, Weaviate, or pure Go solutions like HNSW
- Embedding Models: Understand the trade-offs (quality vs. speed)

---

## Phase III: Agentic AI (4-6 Weeks)

*Goal: Build systems that DO things, not just talk.*

**This is the frontier of 2025-2026.**

Agentic AI represents the shift from "AI as a chat interface" to "AI as an autonomous system that can take actions, use tools, and complete complex tasks."

### Essential Resources

- **[Specification] Model Context Protocol (MCP)**
  - The open standard for connecting AI models to data and tools
  - This is the future of AI connectivity
  - Study the protocol, build with it

- **[Video/Blog] Latent.Space - "The Rise of the AI Engineer"**
  - Understand the philosophy of this new role
  - Where the industry is heading

- **[Course] DeepLearning.AI - AI Agentic Design Patterns with AutoGen**
  - Multi-Agent patterns: Reflection, Tool Use
  - Practical implementation patterns

### The 20% Skills

| Skill | Focus Areas | Why It Matters |
|-------|-------------|----------------|
| **Tool Use (Function Calling)** | Teaching LLMs to execute code, call APIs | The core capability of agents |
| **Orchestration** | LangGraph or CrewAI | Managing state and loops in agent workflows |
| **Planning** | ReAct loops (Reason + Act) | How agents decide what to do next |

### Tool Use Pattern Flowchart

```mermaid
%%{init: {'theme': 'base', 'themeVariables': { 'primaryColor': '#e1f5ff', 'primaryTextColor': '#1a1a1a', 'primaryBorderColor': '#0066cc', 'lineColor': '#0066cc', 'secondaryColor': '#fff4e6', 'tertiaryColor': '#f0f0f0'}}}%%
flowchart TD
    Start([User Query Received]) --> Analyze{Can LLM answer<br/>with existing knowledge?}

    Analyze -->|Yes| DirectGen[Generate text response]
    Analyze -->|No| NeedTools{Does LLM need<br/>external data or actions?}

    DirectGen --> Return([Return response to user])

    NeedTools -->|Yes| ToolSelect[Select appropriate tool]
    NeedTools -->|No| Clarify[Request clarification]

    ToolSelect --> ToolDef{Tool definition<br/>in context?}
    ToolDef -->|No| Error[Return error:<br/>Tool not available]
    ToolDef -->|Yes| ParseArgs[Parse function arguments]

    ParseArgs --> Validate{Arguments valid?}
    Validate -->|No| ReAsk[Ask user for missing info]
    Validate -->|Yes| Execute[Execute tool/function]

    Execute --> Success{Execution<br/>successful?}
    Success -->|No| HandleError[Error handling:<br/>Retry or explain failure]
    Success -->|Yes| ProcessResult[Process tool output]

    ProcessResult --> NeedMore{Need more<br/>tool calls?}
    NeedMore -->|Yes| ToolSelect
    NeedMore -->|No| Synthesize[Synthesize final response<br/>using tool results]

    Synthesize --> Return
    HandleError --> Return
    Error --> Return
    Clarify --> Return
    ReAsk --> Return

    style Start fill:#4caf50,stroke:#2e7d32,stroke-width:2px
    style Return fill:#2196f3,stroke:#1565c0,stroke-width:2px
    style Execute fill:#ff9800,stroke:#e65100,stroke-width:2px
    style Error fill:#f44336,stroke:#c62828,stroke-width:2px
```

### The ReAct Pattern

```
Thought: I need to find the user's recent orders
Action:  query_database(user_id="123", table="orders")
Observation: [order_1, order_2, order_3]
Thought: Now I should format this for the user
Action:  format_response(orders=[...])
Result:  "Your 3 recent orders are..."
```

#### ReAct Pattern State Diagram

```mermaid
%%{init: {'theme': 'base', 'themeVariables': { 'primaryColor': '#e1f5ff', 'primaryTextColor': '#1a1a1a', 'primaryBorderColor': '#0066cc', 'lineColor': '#0066cc', 'secondaryColor': '#fff4e6', 'tertiaryColor': '#f0f0f0'}}}%%
stateDiagram-v2
    [*] --> Reasoning: User query received

    state Reasoning {
        [*] --> AnalyzeContext
        AnalyzeContext --> SelectTool
        SelectTool --> [*]
    }
    note right of Reasoning
        Thought - Analyze task and
        determine next action
        Example: "I need to find
        the user's recent orders"
    end note

    Reasoning --> Acting: Action selected

    state Acting {
        [*] --> ExecuteTool
        ExecuteTool --> CallAPI
        CallAPI --> [*]
    }
    note right of Acting
        Execute tool/function
        or call API/query DB
        Example: query_database(
        user_id="123",
        table="orders")
    end note

    Acting --> Observing: Get result

    state Observing {
        [*] --> ProcessResult
        ProcessResult --> UpdateContext
        UpdateContext --> [*]
    }
    note right of Observing
        Process observation and
        update context
        Example: Found 3 orders -
        [order_1, order_2, order_3]
    end note

    Observing --> Reasoning: Need more info?
    Observing --> Complete: Task finished

    state Complete {
        [*] --> FormatResponse
        FormatResponse --> [*]
    }
    note left of Complete
        Format final response
        Example: "Your 3 recent
        orders are X, Y, Z"
    end note

    Complete --> [*]: Return to user
```

**Key Insight:** The shift from "prompt engineering" to "agent orchestration" is the defining transition of 2025-2026 AI engineering.

---

## Phase IV: Production (Ongoing)

*Goal: Make it reliable, cheap, and safe.*

Production AI is a different beast from prototypes. This phase never ends - it's about continuous improvement.

### Essential Resources

- **[Book] "AI Engineering" / "Designing Machine Learning Systems"** (Chip Huyen)
  - The bible of production ML/AI
  - System design trade-offs that matter at scale

- **[Blog] Eugene Yan's Blog**
  - High-quality posts on applied AI engineering patterns
  - Practical insights on Evals, RAG, and production systems

### The 20% Skills

| Skill | Focus Areas | Why It Matters |
|-------|-------------|----------------|
| **Evals** | Ragas, DeepEval | The TDD of AI: "If you don't measure it, you can't improve it" |
| **Tracing & Monitoring** | Arize Phoenix, LangSmith | See *why* your agent failed, not just *that* it failed |
| **Cost/Latency Optimization** | Semantic caching, SLMs for easy tasks | Production costs can spiral without optimization |

### AI Engineering Skill Tree

```mermaid
%%{init: {'theme': 'base', 'themeVariables': { 'primaryColor': '#e1f5ff', 'primaryTextColor': '#1a1a1a', 'primaryBorderColor': '#0066cc', 'lineColor': '#0066cc', 'secondaryColor': '#fff4e6', 'tertiaryColor': '#f0f0f0'}}}%%
graph TB
    subgraph Foundation["Foundation Skills"]
        Python["Python for AI<br/>numpy, pandas, pydantic"]
        SE["Software Engineering<br/>Clean code, APIs, Testing"]
        Infra["Infrastructure<br/>Docker, Git, CI/CD"]
    end

    subgraph Core["Core LLM Skills"]
        Prompt["Prompt Engineering<br/>CoT, Few-shot, Structured"]
        Embed["Embeddings<br/>Vector representations"]
        RAG["RAG Architecture<br/>Retrieval + Generation"]
    end

    subgraph Agent["Agentic Systems"]
        Tools["Tool Use<br/>Function calling"]
        Orch["Orchestration<br/>State management"]
        Plan["Planning<br/>ReAct, Multi-agent"]
    end

    subgraph Production["Production Skills"]
        Eval["Evaluation<br/>Metrics, Datasets"]
        Monitor["Monitoring<br/>Tracing, Debugging"]
        Opt["Optimization<br/>Cost, Latency, Quality"]
    end

    subgraph Advanced["Advanced Capabilities"]
        FineTune["Fine-tuning<br/>Model adaptation"]
        Multi["Multimodal<br/>Vision, Audio, Text"]
        Scale["Scaling<br/>Distributed systems"]
    end

    Python --> Prompt
    SE --> Tools
    Infra --> Monitor

    Prompt --> RAG
    Embed --> RAG
    RAG --> Tools

    Tools --> Orch
    Orch --> Plan

    RAG --> Eval
    Plan --> Eval
    Eval --> Monitor
    Monitor --> Opt

    Opt --> FineTune
    RAG --> Multi
    Plan --> Scale

    classDef foundationStyle fill:#e1f5ff,stroke:#0066cc,stroke-width:2px
    classDef coreStyle fill:#fff4e6,stroke:#ff9800,stroke-width:2px
    classDef agentStyle fill:#e8f5e9,stroke:#4caf50,stroke-width:2px
    classDef prodStyle fill:#f3e5f5,stroke:#9c27b0,stroke-width:2px
    classDef advStyle fill:#fce4ec,stroke:#e91e63,stroke-width:2px

    class Python,SE,Infra foundationStyle
    class Prompt,Embed,RAG coreStyle
    class Tools,Orch,Plan agentStyle
    class Eval,Monitor,Opt prodStyle
    class FineTune,Multi,Scale advStyle
```

### The Eval Mindset

```mermaid
%%{init: {'theme': 'base', 'themeVariables': { 'primaryColor': '#e1f5ff', 'primaryTextColor': '#1a1a1a', 'primaryBorderColor': '#0066cc', 'lineColor': '#0066cc', 'secondaryColor': '#fff4e6', 'tertiaryColor': '#f0f0f0'}}}%%
flowchart TD
    Dataset["Test Dataset<br/>(Golden answers)"]

    Dataset --> ModelA["Model A"]
    Dataset --> ModelB["Model B"]
    Dataset --> ModelC["Model C"]

    ModelA --> EvalA["Evaluation<br/>Score: 0.82"]
    ModelB --> EvalB["Evaluation<br/>Score: 0.91"]
    ModelC --> EvalC["Evaluation<br/>Score: 0.77"]

    EvalB --> Deploy(["Deploy Model B"])

    style Dataset fill:#e1f5ff,stroke:#0066cc,stroke-width:2px
    style ModelB fill:#4caf50,stroke:#2e7d32,stroke-width:2px
    style EvalB fill:#4caf50,stroke:#2e7d32,stroke-width:2px
    style Deploy fill:#2196f3,stroke:#1565c0,stroke-width:3px
    style EvalA fill:#f0f0f0,stroke:#666,stroke-width:1px
    style EvalC fill:#f0f0f0,stroke:#666,stroke-width:1px
```

### Evaluation Framework Diagram

```mermaid
%%{init: {'theme': 'base', 'themeVariables': { 'primaryColor': '#e1f5ff', 'primaryTextColor': '#1a1a1a', 'primaryBorderColor': '#0066cc', 'lineColor': '#0066cc', 'secondaryColor': '#fff4e6', 'tertiaryColor': '#f0f0f0'}}}%%
graph TD
    subgraph Dataset["Test Dataset Creation"]
        Collect[Collect real queries]
        Golden[Create golden answers]
        Edge[Add edge cases]
    end

    subgraph Metrics["Quality Metrics"]
        Acc["Accuracy<br/>Correct vs Total"]
        Rel["Relevance<br/>Answer quality"]
        Faith["Faithfulness<br/>Grounded in context"]
        Lat["Latency<br/>Response time"]
        Cost["Cost<br/>Token usage"]
    end

    subgraph Eval["Evaluation Methods"]
        Auto["Automated Evals<br/>Ragas, DeepEval"]
        LLMJudge["LLM-as-Judge<br/>GPT-4 scoring"]
        Human["Human Review<br/>Spot checks"]
    end

    subgraph Analysis["Analysis & Action"]
        Compare[Compare models/prompts]
        Identify[Identify failure patterns]
        Iterate[Iterate on system]
    end

    subgraph Production["Production Monitoring"]
        Live[Live traffic sampling]
        AB[A/B testing]
        Feedback[User feedback loop]
    end

    Collect --> Golden
    Golden --> Edge
    Edge --> Auto

    Auto --> Acc
    Auto --> Rel
    Auto --> Faith
    Auto --> Lat
    Auto --> Cost

    LLMJudge --> Rel
    LLMJudge --> Faith
    Human --> Rel

    Acc --> Compare
    Rel --> Compare
    Faith --> Compare
    Lat --> Compare
    Cost --> Compare

    Compare --> Identify
    Identify --> Iterate
    Iterate --> Auto

    Iterate --> Live
    Live --> AB
    AB --> Feedback
    Feedback --> Collect

    classDef dataStyle fill:#e1f5ff,stroke:#0066cc,stroke-width:2px
    classDef metricStyle fill:#fff4e6,stroke:#ff9800,stroke-width:2px
    classDef evalStyle fill:#e8f5e9,stroke:#4caf50,stroke-width:2px
    classDef analyzeStyle fill:#f3e5f5,stroke:#9c27b0,stroke-width:2px
    classDef prodStyle fill:#fce4ec,stroke:#e91e63,stroke-width:2px

    class Collect,Golden,Edge dataStyle
    class Acc,Rel,Faith,Lat,Cost metricStyle
    class Auto,LLMJudge,Human evalStyle
    class Compare,Identify,Iterate analyzeStyle
    class Live,AB,Feedback prodStyle
```

---

## The 6-Week Curriculum

If you have 10 hours a week, follow this progression:

| Week | Focus | Actions |
|------|-------|---------|
| **1** | Setup + Foundations | Watch Karpathy's "Intro to LLMs" and 3Blue1Brown's Neural Networks. Install Python/Cursor. |
| **2** | Build RAG | Build a simple RAG app. Index a PDF, query it. Use LlamaIndex. |
| **3** | Prompt Engineering | DeepLearning.AI Short Courses: "ChatGPT Prompt Engineering for Developers" and "Building Systems with the ChatGPT API" |
| **4** | Agentic AI | Build a tool-using agent (e.g., weather bot). Read the MCP specification. |
| **5** | Production | Add Tracing (LangSmith) to your agent. Create a dataset of 50 questions and run an Eval. |
| **6** | System Design | Read Chip Huyen's book. Understand the system design trade-offs. |

### Week-by-Week Milestones

```
Week 1: "I understand how LLMs work at a high level"
Week 2: "I can build a RAG app that answers questions about my data"
Week 3: "I can craft prompts that reliably produce structured outputs"
Week 4: "I can build agents that use tools to accomplish tasks"
Week 5: "I can measure and improve my AI system's performance"
Week 6: "I understand the trade-offs in production AI systems"
```

---

## How AmanMCP Applies This

AmanMCP is a real-world demonstration of these principles. Here's how the project maps to the phases:

### Phase II: RAG with Hybrid Search

AmanMCP implements production-grade RAG:

- **BM25** for keyword matching (the baseline that works)
- **Vector search** with HNSW for semantic understanding
- **RRF (Reciprocal Rank Fusion)** to combine both approaches

```mermaid
%%{init: {'theme': 'base', 'themeVariables': { 'primaryColor': '#e1f5ff', 'primaryTextColor': '#1a1a1a', 'primaryBorderColor': '#0066cc', 'lineColor': '#0066cc', 'secondaryColor': '#fff4e6', 'tertiaryColor': '#f0f0f0'}}}%%
flowchart LR
    Query([User Query])

    Query --> BM25["BM25 Search<br/>(exact match)"]
    Query --> Vector["Vector Search<br/>(semantic)"]

    BM25 --> BM25Results["BM25 Results"]
    Vector --> VectorResults["Vector Results"]

    BM25Results --> RRF["RRF Fusion<br/>(combined)"]
    VectorResults --> RRF

    RRF --> Final["Ranked Results<br/>(best of both)"]

    style Query fill:#4caf50,stroke:#2e7d32,stroke-width:2px
    style BM25 fill:#fff4e6,stroke:#ff9800,stroke-width:2px
    style Vector fill:#e1f5ff,stroke:#0066cc,stroke-width:2px
    style RRF fill:#f3e5f5,stroke:#9c27b0,stroke-width:2px
    style Final fill:#2196f3,stroke:#1565c0,stroke-width:2px
```

### Phase III: MCP Server as Tool

AmanMCP is itself an MCP server that AI assistants use as a tool:

- Implements the Model Context Protocol specification
- Exposes code search as a tool for Claude, Cursor, and other AI assistants
- Demonstrates the "AI using tools" pattern from Phase III

### Phase IV: Observability and Evaluation

Production patterns in AmanMCP:

- **Telemetry** for understanding usage patterns
- **Benchmarking** for measuring search quality
- **Configuration-driven behavior** for tuning without code changes

---

## Quick Reference Links

### Core Resources

| Resource | Type | URL |
|----------|------|-----|
| Latent.Space | Newsletter/Podcast | [latent.space](https://www.latent.space/) |
| Andrej Karpathy | YouTube | [youtube.com/@AndrejKarpathy](https://www.youtube.com/@AndrejKarpathy) |
| DeepLearning.AI | Courses | [deeplearning.ai](https://www.deeplearning.ai/) |
| Made With ML | MLOps Guide | [madewithml.com](https://madewithml.com/) |

### Books

| Book | Author | Focus |
|------|--------|-------|
| AI Engineering | Chip Huyen | Production systems |
| The LLM Engineering Handbook | Paul Iusztin | End-to-end LLM apps |
| Build a Large Language Model (from Scratch) | Sebastian Raschka | Deep understanding |

### Tools

| Tool | Purpose | When to Use |
|------|---------|-------------|
| LangChain / LlamaIndex | Framework | Building RAG and agents |
| LangSmith / Arize Phoenix | Tracing | Debugging agent behavior |
| Ragas / DeepEval | Evaluation | Measuring quality |

---

## The Mental Model

```mermaid
%%{init: {'theme': 'base', 'themeVariables': { 'primaryColor': '#e1f5ff', 'primaryTextColor': '#1a1a1a', 'primaryBorderColor': '#0066cc', 'lineColor': '#0066cc', 'secondaryColor': '#fff4e6', 'tertiaryColor': '#f0f0f0'}}}%%
graph TB
    subgraph AIEng["AI ENGINEERING"]
        direction TB

        subgraph Foundation["Software Engineering<br/>(The foundation)"]
            SE1["- Clean code<br/>- APIs/Services<br/>- System design<br/>- Testing"]
        end

        subgraph Specialization["AI/ML Knowledge<br/>(The specialization)"]
            AI1["- LLM capabilities<br/>- Prompt engineering<br/>- Embeddings/RAG<br/>- Agent orchestration"]
        end

        Foundation -."+".-> Combine[" "]
        Specialization -."+".-> Combine

        Combine --> AIEngineer["AI ENGINEER<br/><br/>Builds systems that:<br/>- Understand context<br/>- Use tools<br/>- Learn from data<br/>- Scale reliably"]
    end

    style Foundation fill:#e1f5ff,stroke:#0066cc,stroke-width:2px
    style Specialization fill:#fff4e6,stroke:#ff9800,stroke-width:2px
    style AIEngineer fill:#4caf50,stroke:#2e7d32,stroke-width:3px
    style Combine fill:none,stroke:none
    style AIEng fill:#f8f9fa,stroke:#2c3e50,stroke-width:2px
```

---

## Common Pitfalls

### What to Avoid

| Pitfall | Why It's Wrong | What to Do Instead |
|---------|----------------|-------------------|
| Starting with fine-tuning | Expensive, often unnecessary | Start with prompting, then RAG, then fine-tune |
| Ignoring evals | You can't improve what you don't measure | Build eval datasets from day 1 |
| Over-engineering | Complex agents fail in novel ways | Start simple, add complexity as needed |
| Skipping foundations | LLMs abstract away, but you still need to debug | Understand embeddings, tokenization, context windows |

### The Progression Fallacy

```
Wrong: "I'll learn everything about LLMs, then build"
Right: "I'll build, then learn what I need to fix problems"
```

---

## See Also

For deeper dives into specific topics covered in this guide:

- [Observability for RAG](../research/observability-for-rag.md) - Production monitoring patterns
- [Contextual Retrieval Decision](../research/contextual-retrieval-decision.md) - RAG architecture trade-offs
- [Static Embeddings Explained](./static-embeddings-explained.md) - Understanding embedding fundamentals
- [Smaller Models, Better Search](./smaller-models-better-search.md) - Model selection strategies

---

## Summary

The path to AI Engineering competency is not about consuming every tutorial or mastering every framework. It's about:

1. **Building on solid foundations** (Phase I) - Python, Docker, basic ML concepts
2. **Learning to orchestrate LLMs** (Phase II) - Prompting, RAG, embeddings
3. **Creating systems that act** (Phase III) - Tools, agents, MCP
4. **Making it production-ready** (Phase IV) - Evals, monitoring, optimization

The 80/20 rule applies: focus on the resources and skills listed here, build projects, and iterate. The AI engineering landscape moves fast, but the fundamentals in this guide remain stable.

**Start building. Measure everything. Iterate constantly.**

---

**Original Source:** `archive/gem_ai_engineering_guide.md`
**Last Updated:** 2026-01-16
