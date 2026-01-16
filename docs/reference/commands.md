# Command Reference

Complete reference for all AmanMCP CLI commands.

---

## Quick Reference

| Command | Description |
|---------|-------------|
| `amanmcp` | Smart default: auto-index + start server |
| `amanmcp init` | Initialize project |
| `amanmcp search "query"` | Search codebase |
| `amanmcp doctor` | Check system health |

---

## Getting Started

| Command | Description |
|---------|-------------|
| `amanmcp` | Smart default: auto-index + start server |
| `amanmcp init` | Initialize project (MCP config + indexing) |
| `amanmcp init --force` | Reinitialize, overwrite existing config |
| `amanmcp init --config-only` | Fix config without reindexing |
| `amanmcp init --offline` | Use static embeddings (no Ollama) |
| `amanmcp setup` | Check/configure embedding backend |
| `amanmcp setup --check` | Check status only, don't start/pull |
| `amanmcp setup --auto` | Non-interactive mode (for scripts) |
| `amanmcp setup --offline` | Configure for offline mode |

---

## Indexing

| Command | Description |
|---------|-------------|
| `amanmcp index` | Index current directory |
| `amanmcp index [path]` | Index specific directory |
| `amanmcp index --backend=ollama` | Force Ollama backend |
| `amanmcp index --backend=mlx` | Force MLX backend (Apple Silicon) |
| `amanmcp index --force` | Clear existing index and rebuild |
| `amanmcp index --resume` | Resume interrupted indexing |
| `amanmcp index --no-tui` | Plain text output (no TUI) |
| `amanmcp index info` | Show index configuration and stats |
| `amanmcp index info --json` | Index info as JSON |
| `amanmcp compact` | Optimize vector index |

---

## Search

| Command | Description |
|---------|-------------|
| `amanmcp search "query"` | Hybrid search across codebase |
| `amanmcp search -t code "query"` | Search code files only |
| `amanmcp search -t docs "query"` | Search documentation only |
| `amanmcp search -l go "query"` | Filter by language |
| `amanmcp search -n 20 "query"` | Limit results (default: 10) |
| `amanmcp search -f json "query"` | JSON output format |

### Search Examples

```bash
# Find authentication code
amanmcp search "authentication"

# Search only Go files
amanmcp search -l go "error handling"

# Get JSON output for scripting
amanmcp search -f json "database connection" | jq '.results[0]'
```

---

## Session Management

| Command | Description |
|---------|-------------|
| `amanmcp sessions` | List all sessions |
| `amanmcp sessions delete NAME` | Delete a session |
| `amanmcp sessions prune` | Remove sessions older than 30 days |
| `amanmcp resume NAME` | Resume a saved session |
| `amanmcp switch NAME` | Switch to different session |

---

## Server & Daemon

| Command | Description |
|---------|-------------|
| `amanmcp serve` | Start MCP server (stdio) |
| `amanmcp serve --transport sse --port 8765` | SSE transport on port |
| `amanmcp daemon start` | Start background daemon |
| `amanmcp daemon stop` | Stop daemon |
| `amanmcp daemon status` | Check daemon status |

---

## Configuration

| Command | Description |
|---------|-------------|
| `amanmcp config init` | Create user config from template |
| `amanmcp config init --force` | Upgrade config (preserves settings) |
| `amanmcp config path` | Show config file locations |
| `amanmcp config show` | Show effective configuration |

---

## Diagnostics

| Command | Description |
|---------|-------------|
| `amanmcp doctor` | Check system requirements |
| `amanmcp status` | Show index health |
| `amanmcp stats` | Show statistics |
| `amanmcp stats queries --days 7` | Query analytics |
| `amanmcp version` | Show version |
| `amanmcp version --json` | Version as JSON |

---

## Debugging

| Command | Description |
|---------|-------------|
| `amanmcp --debug <cmd>` | Enable file logging |
| `amanmcp-logs` | Show last 50 log lines |
| `amanmcp-logs -f` | Follow logs real-time |
| `amanmcp-logs --level error` | Filter by level |
| `amanmcp-logs --source mlx` | View MLX server logs |
| `amanmcp-logs --source all` | View all logs merged |

### Log Locations

| Source | File |
|--------|------|
| Go server | `~/.amanmcp/logs/server.log` |
| MLX server | `~/.amanmcp/logs/mlx-server.log` |

---

## Global Flags

These flags work with most commands:

| Flag | Description |
|------|-------------|
| `--debug` | Enable verbose logging to file |
| `--help` | Show help for command |
| `--version` | Show version |

---

## Environment Variables

For complete environment variable reference, see [Configuration Reference](configuration.md#environment-variables).

| Variable | Default | Description |
|----------|---------|-------------|
| `AMANMCP_EMBEDDER` | `auto` | Backend: `mlx`, `ollama`, `static` |
| `AMANMCP_OLLAMA_HOST` | `http://localhost:11434` | Ollama endpoint |
| `AMANMCP_MLX_ENDPOINT` | `http://localhost:9659` | MLX endpoint |
| `AMANMCP_LOG_LEVEL` | `info` | Log level |

---

## See Also

- [Configuration Reference](configuration.md) - All configuration options
- [First-Time User Guide](../getting-started/first-time-user-guide.md) - Step-by-step setup
- [MLX Setup Guide](../guides/mlx-setup.md) - Apple Silicon optimization
- [Backend Switching](../guides/backend-switching.md) - Managing embedding backends
