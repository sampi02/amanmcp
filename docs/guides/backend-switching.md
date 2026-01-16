# Backend Switching Guide

AmanMCP supports multiple embedding backends. This guide covers how to switch between them and when to use each.

---

## Available Backends

| Backend | Platform | Speed | Memory | Best For |
|---------|----------|-------|--------|----------|
| **Ollama** | Cross-platform | Good | ~3-6GB | Default, easy setup |
| **MLX** | Apple Silicon | ~1.7x faster | ~3GB | M1/M2/M3/M4 Macs |
| **Static** | Any | Instant | <100MB | Offline, CI/CD |

---

## Quick Commands

```bash
# Check current backend status
./scripts/switch-backend.sh status

# Switch to MLX (Apple Silicon)
make switch-backend-mlx
amanmcp index --force .

# Switch to Ollama
make switch-backend-ollama
amanmcp index --force .

# Use static (offline mode)
amanmcp index --backend=static .
```

**Important:** When switching backends with different dimensions, you must reindex with `--force`.

---

## Ollama (Default)

Ollama is the default backend on all platforms. It provides good performance with minimal setup.

### Setup

```bash
# Install and start Ollama
make install-ollama
make start-ollama

# Or manually
brew install ollama
ollama serve
ollama pull qwen3-embedding:0.6b
```

### Usage

```bash
# Index with Ollama (default)
amanmcp index .

# Explicitly use Ollama
amanmcp index --backend=ollama .
```

### Configuration

| Variable | Default | Description |
|----------|---------|-------------|
| `AMANMCP_OLLAMA_HOST` | `http://localhost:11434` | Ollama endpoint |
| `AMANMCP_OLLAMA_MODEL` | `qwen3-embedding:0.6b` | Embedding model |
| `AMANMCP_OLLAMA_TIMEOUT` | `5m` | Request timeout |

---

## MLX (Apple Silicon)

MLX provides ~1.7x faster embeddings on Apple Silicon. Requires Python and more RAM.

### Setup

```bash
make install-mlx
make start-mlx
```

### Usage

```bash
# Index with MLX
amanmcp index --backend=mlx .

# Or set environment variable
export AMANMCP_EMBEDDER=mlx
amanmcp index .
```

### Configuration

| Variable | Default | Description |
|----------|---------|-------------|
| `AMANMCP_MLX_ENDPOINT` | `http://localhost:9659` | MLX server endpoint |
| `AMANMCP_MLX_MODEL` | `small` | Model: `small`, `medium`, `large` |

For detailed MLX setup, see [MLX Setup Guide](mlx-setup.md).

---

## Static (Offline)

Static embeddings use hash-based vectors. No external dependencies, but lower search quality (~35% of neural).

### When to Use

- CI/CD pipelines without GPU
- Air-gapped environments
- Testing and development
- BM25-only search is sufficient

### Usage

```bash
# Initialize in offline mode
amanmcp init --offline

# Index with static backend
amanmcp index --backend=static .
```

**Note:** Static mode provides BM25 keyword search. Semantic search quality is significantly reduced.

---

## Switching Backends

### Check Current Status

```bash
# Show which backend is active
./scripts/switch-backend.sh status

# Or use amanmcp
amanmcp index info
```

### Switch Process

1. **Stop current backend** (if running)
2. **Start new backend**
3. **Reindex** (required if dimensions differ)

```bash
# Example: Ollama → MLX
make stop-ollama
make start-mlx
amanmcp index --force .
```

### Dimension Compatibility

Different backends may produce different embedding dimensions:

| Backend | Model | Dimensions |
|---------|-------|------------|
| Ollama | qwen3-embedding:0.6b | 1024 |
| MLX | small | 1024 |
| MLX | medium | 2560 |
| MLX | large | 4096 |
| Static | - | 768 |

**When dimensions change, you must reindex.** AmanMCP will warn you if there's a mismatch.

#### Dimension Compatibility Matrix

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'fontSize':'14px'}}}%%
graph TD
    subgraph Compatibility["Backend → Dimension Compatibility Matrix"]
        direction TB

        subgraph Ollama["Ollama"]
            O1[qwen3-embedding:0.6b<br/>✓ 1024 dimensions]
        end

        subgraph MLX["MLX (Apple Silicon)"]
            M1[small<br/>✓ 1024 dimensions]
            M2[medium<br/>✓ 2560 dimensions]
            M3[large<br/>✓ 4096 dimensions]
        end

        subgraph Static["Static (Offline)"]
            S1[hash-based<br/>✓ 768 dimensions]
        end

        subgraph Rules["Switching Rules"]
            R1["✓ Compatible: No reindex needed<br/>Ollama 1024 ↔ MLX small 1024"]
            R2["⚠ Incompatible: Reindex required<br/>Ollama 1024 → MLX medium 2560<br/>Ollama 1024 → Static 768<br/>MLX small → MLX medium/large"]
        end
    end

    O1 -.compatible.-> M1
    M1 -.compatible.-> O1

    O1 -.incompatible.-> M2
    O1 -.incompatible.-> M3
    O1 -.incompatible.-> S1
    M1 -.incompatible.-> M2
    M1 -.incompatible.-> M3
    M2 -.incompatible.-> M3

    style O1 fill:#bbdefb
    style M1 fill:#c8e6c9
    style M2 fill:#c8e6c9
    style M3 fill:#c8e6c9
    style S1 fill:#fff9c4
    style R1 fill:#e8f5e9
    style R2 fill:#ffebee
    style Compatibility fill:#fafafa
```

---

## Auto-Detection

By default (`AMANMCP_EMBEDDER=auto`), AmanMCP tries backends in order:

1. **MLX** - If server is running on localhost:9659
2. **Ollama** - If server is running on localhost:11434
3. **Error** - Prompts user to start a backend or use `--offline`

### Auto-Detection Decision Flow

```mermaid
flowchart TD
    Start([Start: AMANMCP_EMBEDDER=auto]) --> CheckMLX{MLX endpoint<br/>localhost:9659<br/>accessible?}
    CheckMLX -->|Yes| UseMLX[Use MLX Backend]
    CheckMLX -->|No| CheckOllama{Ollama endpoint<br/>localhost:11434<br/>accessible?}
    CheckOllama -->|Yes| UseOllama[Use Ollama Backend]
    CheckOllama -->|No| CheckOffline{--offline flag<br/>or AMANMCP_EMBEDDER=static?}
    CheckOffline -->|Yes| UseStatic[Use Static Backend]
    CheckOffline -->|No| Error[Error: No backend available<br/>Prompt to start Ollama/MLX<br/>or use --offline]

    UseMLX --> Proceed([Proceed with indexing])
    UseOllama --> Proceed
    UseStatic --> Proceed
    Error --> Exit([Exit with error])

    style Start fill:#e1f5ff
    style UseMLX fill:#c8e6c9
    style UseOllama fill:#c8e6c9
    style UseStatic fill:#fff9c4
    style Error fill:#ffcdd2
    style Proceed fill:#e1f5ff
    style Exit fill:#ffcdd2
```

### Backend Selection Priority

```mermaid
flowchart LR
    subgraph Priority["Backend Selection Priority (AMANMCP_EMBEDDER=auto)"]
        P1[1. MLX<br/>Apple Silicon only<br/>~1.7x faster]
        P2[2. Ollama<br/>Cross-platform<br/>Default choice]
        P3[3. Static<br/>Offline fallback<br/>Lower quality]
    end

    subgraph Factors["Decision Factors"]
        F1[Platform:<br/>Apple Silicon?]
        F2[Availability:<br/>Server running?]
        F3[Performance:<br/>Speed vs Memory]
        F4[Offline mode:<br/>--offline flag?]
    end

    P1 --> P2
    P2 --> P3

    F1 -.influences.-> P1
    F2 -.influences.-> P1
    F2 -.influences.-> P2
    F3 -.influences.-> P1
    F4 -.influences.-> P3

    style P1 fill:#c8e6c9
    style P2 fill:#bbdefb
    style P3 fill:#fff9c4
    style Priority fill:#f5f5f5
    style Factors fill:#fafafa
```

To force a specific backend:

```bash
# Force Ollama even if MLX is available
AMANMCP_EMBEDDER=ollama amanmcp index .

# Force MLX
AMANMCP_EMBEDDER=mlx amanmcp index .

# Force static
AMANMCP_EMBEDDER=static amanmcp index .
```

---

## Makefile Targets

| Target | Description |
|--------|-------------|
| `make install-ollama` | Install Ollama + pull model |
| `make start-ollama` | Start Ollama server |
| `make stop-ollama` | Stop Ollama server |
| `make install-mlx` | Install MLX server |
| `make start-mlx` | Start MLX server |
| `make switch-backend-mlx` | Switch to MLX |
| `make switch-backend-ollama` | Switch to Ollama |
| `make verify-install` | Verify installation |

---

## Troubleshooting

### "Dimension mismatch" Error

```bash
# Reindex with current backend
amanmcp index --force .
```

### Backend Not Detected

```bash
# Check what's running
./scripts/switch-backend.sh status

# Check endpoints
curl http://localhost:11434/api/tags  # Ollama
curl http://localhost:9659/health      # MLX
```

### Fallback Behavior

If the configured backend fails, AmanMCP will:

1. Log a warning
2. Fall back to BM25-only search (keywords still work)
3. **Not** silently switch to another neural backend

To explicitly allow fallback:

```bash
amanmcp init --offline  # Allows static fallback
```

---

## See Also

- [MLX Setup Guide](mlx-setup.md) - Detailed MLX configuration
- [Command Reference](../reference/commands.md) - All CLI commands
- [Configuration Reference](../reference/configuration.md) - All config options
