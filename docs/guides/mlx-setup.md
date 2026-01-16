# MLX Server Setup (Apple Silicon)

The bundled MLX embedding server provides **~1.7x faster** embedding throughput than Ollama on Apple Silicon Macs.

---

## Quick Start

```bash
# One-command setup: venv + dependencies + model download
make install-mlx

# Start the server
make start-mlx

# Index with MLX
amanmcp index --backend=mlx .
```

### Setup Workflow

```mermaid
flowchart TD
    Start([MLX Setup]) --> Check{Have Apple<br/>Silicon?}
    Check -->|No| NotSupported([Not Supported<br/>Use Ollama])
    Check -->|Yes| Install[make install-mlx]

    Install --> CreateVenv[Create Python venv]
    CreateVenv --> InstallDeps[Install Dependencies<br/>mlx, mlx-lm, fastapi]
    InstallDeps --> DownloadModel[Download Model<br/>small: ~900MB]

    DownloadModel --> Success{Install<br/>OK?}
    Success -->|No| Troubleshoot[See Troubleshooting]
    Success -->|Yes| StartServer[make start-mlx]

    StartServer --> VerifyHealth[curl localhost:9659/health]
    VerifyHealth --> Healthy{200 OK?}
    Healthy -->|No| Troubleshoot
    Healthy -->|Yes| Index[amanmcp index --backend=mlx .]

    Index --> Monitor[Monitor Performance]
    Monitor --> Ready([Ready to Use])

    style Start fill:#e1f5ff,stroke:#01579b,stroke-width:2px
    style Ready fill:#c8e6c9,stroke:#2e7d32,stroke-width:2px
    style NotSupported fill:#ffccbc,stroke:#d84315,stroke-width:2px
    style Success fill:#fff9c4,stroke:#f57f17,stroke-width:2px
    style Healthy fill:#fff9c4,stroke:#f57f17,stroke-width:2px
    style Check fill:#fff9c4,stroke:#f57f17,stroke-width:2px
    style Troubleshoot fill:#ffe0b2,stroke:#e65100,stroke-width:2px
```

---

## When to Use MLX

| Use MLX When | Use Ollama When |
|--------------|-----------------|
| You have Apple Silicon (M1/M2/M3/M4) | Cross-platform needed |
| Speed is priority | Simpler setup preferred |
| You have 8GB+ RAM available | Lower RAM usage needed |

**Performance comparison:**

| Backend | Speed | Memory | Setup |
|---------|-------|--------|-------|
| MLX | ~1.7x faster | ~3GB | Requires Python |
| Ollama | Baseline | ~3-6GB | One binary |

### Decision Matrix: Should You Use MLX?

```mermaid
flowchart TD
    Start[Choose Embedding Backend] --> Silicon{Apple Silicon<br/>M1/M2/M3/M4?}

    Silicon -->|No| OllamaChoice[Use Ollama]
    Silicon -->|Yes| RAM{8GB+ RAM<br/>Available?}

    RAM -->|No| OllamaChoice
    RAM -->|Yes| Priority{What's Your<br/>Priority?}

    Priority -->|Speed| MLXSpeed[✅ Use MLX<br/>~1.7x faster]
    Priority -->|Simplicity| SimpleCheck{Comfortable with<br/>Python setup?}
    Priority -->|Cross-platform| OllamaChoice

    SimpleCheck -->|Yes| MLXSpeed
    SimpleCheck -->|No| OllamaChoice

    MLXSpeed --> ModelSize{RAM Budget?}
    ModelSize -->|<2GB| MLXSmall[MLX Small<br/>1024d, ~900MB]
    ModelSize -->|2-4GB| MLXMedium[MLX Medium<br/>2560d, ~2.5GB]
    ModelSize -->|4GB+| MLXLarge[MLX Large<br/>4096d, ~4.5GB]

    OllamaChoice --> OllamaModel[Ollama<br/>nomic-embed-text<br/>~3-6GB]

    style MLXSpeed fill:#34a853
    style MLXSmall fill:#34a853
    style MLXMedium fill:#34a853
    style MLXLarge fill:#34a853
    style OllamaChoice fill:#4285f4
    style OllamaModel fill:#4285f4
```

---

## Installation

### Prerequisites

- Apple Silicon Mac (M1/M2/M3/M4)
- Python 3.9+
- 8GB+ RAM available

### Automated Setup

```bash
# From project root
make install-mlx
```

This command:

1. Creates Python virtual environment in `mlx-server/.venv`
2. Installs dependencies (mlx, mlx-lm, fastapi)
3. Downloads the default embedding model (~400MB)

### Manual Setup

```bash
cd mlx-server

# Create virtual environment
python3 -m venv .venv
source .venv/bin/activate

# Install dependencies
pip install -r requirements.txt

# Start server (downloads model on first run)
python server.py
```

---

## Running the Server

### Start/Stop

```bash
# Start (foreground)
make start-mlx

# Or run directly
cd mlx-server && source .venv/bin/activate && python server.py

# Check if running
curl http://localhost:9659/health
```

### Auto-Start on Login (Optional)

Create `~/Library/LaunchAgents/com.amanmcp.mlx-server.plist`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>com.amanmcp.mlx-server</string>
    <key>ProgramArguments</key>
    <array>
        <string>/path/to/amanmcp/mlx-server/.venv/bin/python</string>
        <string>/path/to/amanmcp/mlx-server/server.py</string>
    </array>
    <key>WorkingDirectory</key>
    <string>/path/to/amanmcp/mlx-server</string>
    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>
</dict>
</plist>
```

Enable with:

```bash
launchctl load ~/Library/LaunchAgents/com.amanmcp.mlx-server.plist
```

---

## Model Selection

MLX supports three model sizes:

| Model | Dimensions | Memory | Speed | Quality |
|-------|------------|--------|-------|---------|
| `small` | 1024 | ~900MB | Fastest | Good |
| `medium` | 2560 | ~2.5GB | Medium | Better |
| `large` | 4096 | ~4.5GB | Slower | Best |

### Model Comparison

```mermaid
graph TD
    classDef native fill:#e8f0fe,stroke:#4285f4,stroke-width:2px,color:#1967d2;
    classDef legacy fill:#f1f3f4,stroke:#5f6368,stroke-width:2px,color:#3c4043;
    classDef highlight fill:#e6fffa,stroke:#00b894,stroke-width:2px,color:#004d40;

    subgraph MLX [MLX Native Models]
        direction LR
        Small["⚡ <b>Small</b><br/>Fastest • 900MB RAM"]:::highlight
        Medium["⚖️ <b>Medium</b><br/>Balanced • 2.5GB RAM"]:::native
        Large["🧠 <b>Large</b><br/>Best Quality • 4.5GB RAM"]:::native
    end

    Ollama["🐢 <b>Ollama</b><br/>External • 3-6GB RAM"]:::legacy

    Small -->|"1.7x Faster"| Ollama
    Medium -->|"Similar Speed"| Ollama
    Large -->|"Higher Quality"| Ollama
```

**Recommendations:**

- **8GB RAM Mac**: Use `small` (leaves 7GB for other apps)
- **16GB RAM Mac**: Use `medium` (best quality/speed balance)
- **32GB+ RAM Mac**: Use `large` (maximum quality)
- **Codebase >100K files**: Use `small` (lower memory pressure)

### Changing Models

```bash
# Set model via environment variable
MODEL_NAME=small python server.py

# Or in AmanMCP
AMANMCP_MLX_MODEL=small amanmcp index .
```

**Note:** Changing model size requires reindexing (`amanmcp index --force .`).

---

## Configuration

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `PORT` | `9659` | Server port |
| `MODEL_NAME` | `small` | Model: `small`, `medium`, `large` |
| `LOG_LEVEL` | `INFO` | Logging level |
| `AMANMCP_MLX_MODELS_DIR` | `~/.amanmcp/models/mlx` | Model cache directory |

### AmanMCP Integration

```bash
# Force MLX backend for indexing
amanmcp index --backend=mlx .

# Or set environment variable
export AMANMCP_EMBEDDER=mlx
amanmcp index .
```

---

## Troubleshooting

### Troubleshooting Decision Tree

```mermaid
flowchart TD
    Start[MLX Issue?] --> Type{What's Wrong?}

    Type -->|Server won't start| ServerIssue{Check Error Type}
    Type -->|Model download fails| DownloadIssue{Check Disk Space}
    Type -->|MLX not detected| DetectIssue{Is server running?}
    Type -->|Falls back to Ollama| FallbackIssue{Backend configured?}
    Type -->|Memory errors| MemoryIssue{RAM available?}
    Type -->|Slow performance| PerfIssue{Check model size}

    ServerIssue -->|Port in use| PortFix["lsof -i :9659<br/>kill process or change PORT"]
    ServerIssue -->|Python version| PyFix["python3 --version<br/>Need 3.9+<br/>brew install python@3.11"]
    ServerIssue -->|Missing deps| DepFix["cd mlx-server<br/>source .venv/bin/activate<br/>pip install -r requirements.txt"]

    DownloadIssue -->|<1GB free| DiskFix["df -h ~<br/>Free up space<br/>small needs 500MB<br/>large needs 5GB"]
    DownloadIssue -->|Network error| NetFix["Check internet<br/>Retry with:<br/>python server.py"]
    DownloadIssue -->|Corrupt cache| CacheFix["rm -rf ~/.amanmcp/models/mlx/hub<br/>python server.py"]

    DetectIssue -->|Not running| StartFix["make start-mlx<br/>OR<br/>cd mlx-server && python server.py"]
    DetectIssue -->|Running| HealthFix["curl http://localhost:9659/health<br/>Should return 200 OK"]

    FallbackIssue -->|Not set| ConfigFix["AMANMCP_EMBEDDER=mlx amanmcp index .<br/>OR<br/>amanmcp index --backend=mlx ."]
    FallbackIssue -->|MLX unhealthy| RestartFix["pkill -f 'python.*server.py'<br/>make start-mlx"]

    MemoryIssue -->|<8GB total| RAMFix["Use Ollama instead<br/>OR<br/>MODEL_NAME=small python server.py"]
    MemoryIssue -->|Model too large| ModelFix["Switch to smaller model:<br/>MODEL_NAME=small python server.py<br/>amanmcp index --force ."]

    PerfIssue -->|Using large| SizeFix["Switch to medium/small:<br/>MODEL_NAME=medium python server.py<br/>amanmcp index --force ."]
    PerfIssue -->|Thermal throttle| CoolFix["Check Activity Monitor<br/>Close other apps<br/>Use cooling pad"]

    style PortFix fill:#fbbc04
    style PyFix fill:#fbbc04
    style DepFix fill:#fbbc04
    style DiskFix fill:#fbbc04
    style NetFix fill:#fbbc04
    style CacheFix fill:#fbbc04
    style StartFix fill:#34a853
    style HealthFix fill:#34a853
    style ConfigFix fill:#34a853
    style RestartFix fill:#fbbc04
    style RAMFix fill:#ea4335
    style ModelFix fill:#34a853
    style SizeFix fill:#34a853
    style CoolFix fill:#4285f4
```

### Server Won't Start

```bash
# Check Python version (need 3.9+)
python3 --version

# Check port availability
lsof -i :9659

# Check logs
amanmcp-logs --source mlx -f
```

### Model Download Fails

```bash
# Check disk space (need ~500MB for small, ~5GB for large)
df -h ~

# Clear cache and retry
rm -rf ~/.amanmcp/models/mlx/hub
python server.py
```

### MLX Not Detected

```bash
# Verify MLX server is running
curl http://localhost:9659/health

# Check AmanMCP sees it
amanmcp setup --check
```

### Falls Back to Ollama

If AmanMCP falls back to Ollama when you want MLX:

```bash
# Explicitly force MLX
AMANMCP_EMBEDDER=mlx amanmcp index .

# Or use --backend flag
amanmcp index --backend=mlx .
```

---

## Monitoring

### View Logs

```bash
# Real-time MLX logs
amanmcp-logs --source mlx -f

# All logs (Go + MLX) merged
amanmcp-logs --source all -f
```

### Performance Monitoring

```bash
# Terminal 1: MLX server with logging
cd mlx-server && python server.py

# Terminal 2: Watch GPU/ANE usage
sudo asitop
```

---

## Uninstalling

```bash
# Remove MLX server
rm -rf mlx-server/.venv

# Remove cached models
rm -rf ~/.amanmcp/models/mlx

# Remove LaunchAgent (if configured)
launchctl unload ~/Library/LaunchAgents/com.amanmcp.mlx-server.plist
rm ~/Library/LaunchAgents/com.amanmcp.mlx-server.plist
```

---

## See Also

- [Backend Switching Guide](backend-switching.md) - Managing multiple backends
- [Command Reference](../reference/commands.md) - All CLI commands
- [Configuration Reference](../reference/configuration.md) - All config options
- [MLX Server README](../../mlx-server/README.md) - Detailed MLX documentation
