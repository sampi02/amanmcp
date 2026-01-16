# Debugging the MCP Protocol: When Stdout Breaks Everything

*A real-world case study in protocol debugging, root cause analysis, and the dangers of logging in the wrong place*

---

> **Learning Objectives:**
> - Understand JSON-RPC protocol strictness over stdio
> - Learn systematic debugging with "5 Whys" root cause analysis
> - See how subtle logging bugs break distributed systems
>
> **Prerequisites:**
> - Basic understanding of stdio (stdin/stdout/stderr)
> - Familiarity with JSON-RPC (helpful but not required)
>
> **Audience:** Backend engineers, developers building MCP servers, anyone working with JSON-RPC protocols

---

## TL;DR

When your MCP server fails to connect with no useful error message, check if anything writes to stdout before the JSON-RPC handshake. Even a single status message, emoji, or warning printed to stdout will corrupt the protocol stream. The fix is architectural: create a logging mode that guarantees file-only output during protocol operation.

---

## The Mystery

It was supposed to be a routine bug fix. Two minor issues had been resolved, the tests were passing, CI was green. Time to test the changes in Claude Code.

```
/mcp list
amanmcp: /Users/nirajkumar/.local/bin/amanmcp serve --debug - Failed to connect
```

Failed to connect. No stack trace. No error details. Just... failed.

The developer restarted Claude Code. Same result. Rebuilt the binary. Same result. Checked the MCP tool registration code - working fine. Checked the server initialization - no obvious issues. Two hours of debugging later, frustration was mounting.

"Let me take a 10,000 foot view," the developer muttered, stepping back from the code.

---

## The Investigation

### First Hypothesis: Tool Registration

The initial investigation focused on MCP tool registration. Perhaps the tools were not being registered correctly? The code looked fine:

```go
// MCP tools registered successfully
func (s *Server) registerTools() {
    s.tools["search"] = searchTool
    s.tools["search_code"] = searchCodeTool
    // ... other tools
}
```

Tests confirmed the tools registered correctly. Dead end.

### Second Hypothesis: Server Initialization

Maybe the server was failing during startup? Adding debug logging showed the server initializing perfectly... when run directly from the terminal. But Claude Code still reported "Failed to connect."

This was the key observation: **it worked in isolation but failed when connected to the client**.

### The Breakthrough

A web search for "MCP server debugging" led to the [MCP Debugging Best Practices](https://modelcontextprotocol.io/legacy/tools/debugging) guide. One paragraph stopped the investigation cold:

> "Local MCP servers should NOT log messages to stdout - this will interfere with protocol operation."

Wait. What was the server printing before it started?

```go
// In cmd/amanmcp/cmd/root.go
out.Newline()
printConfigInstructions(out, root)  // Prints JSON config example
out.Status("Rocket", "Starting MCP server on stdio...")
return runServe(ctx, "stdio", 0)
```

There it was. The "smart default" mode was printing helpful configuration instructions and a rocket emoji to stdout *before* starting the JSON-RPC server.

### The Contamination

When `amanmcp` started in smart default mode, stdout contained:

```
Add to your AI assistant config:
{
  "mcpServers": {
    "amanmcp": {
      ...
    }
  }
}
Starting MCP server on stdio...

{"jsonrpc":"2.0","method":"..."}  <-- JSON-RPC starts here
```

Claude Code received this stream and tried to parse the first bytes as JSON-RPC. The text "Add to your AI assistant config" is not valid JSON. Connection failed.

But there was more. A second contamination source lurked in the embedder factory:

```go
// In internal/embed/factory.go
fmt.Fprintf(os.Stderr, "OllamaEmbedder unavailable: %v, using static fallback\n", err)
```

Even stderr was being captured by the MCP client. Any output to either stdout or stderr before the protocol handshake would corrupt the stream.

---

## Root Cause: The "5 Whys"

Root cause analysis using the "5 Whys" technique revealed the true source of the bug:

**1. Why** did Claude Code show "Failed to connect" for amanmcp?
- Because the JSON-RPC handshake failed due to non-JSON data in the stream

**2. Why** was there non-JSON data in the stdout stream?
- Because the "smart default" mode wrote status messages to stdout before starting MCP

**3. Why** did status messages go to stdout?
- Because `output.Writer` writes to stdout by default, and we called it before `runServe()`

**4. Why** wasn't this caught during development?
- Because tests ran individual components but didn't verify end-to-end MCP protocol compliance

**5. Why** was MCP protocol compliance not enforced at the architecture level?
- **Because logging was ad-hoc (`fmt.Fprintf`, `output.Writer`) rather than centralized with MCP-awareness**

The root cause was not a bug in one line of code. It was an architectural gap: the logging system had no concept of "MCP mode" where stdout and stderr must remain pristine.

---

## The Fix: MCP-Safe Logging

### Step 1: Create an MCP-Safe Logging Mode

```go
// internal/logging/mcp.go

// SetupMCPMode initializes logging for MCP server mode.
// - Logs ONLY to file (never stdout/stderr)
// - Uses JSON format for structured logs
// - Always enables debug level for complete diagnostics
func SetupMCPMode() (func(), error) {
    cfg := Config{
        Level:         "debug",
        FilePath:      DefaultLogPath(), // ~/.amanmcp/logs/server.log
        MaxSizeMB:     10,
        MaxFiles:      5,
        WriteToStderr: false, // CRITICAL: Never write to stderr in MCP mode
    }
    return Setup(cfg)
}
```

The key insight: MCP mode requires a **guarantee** that no code path can write to stdout or stderr. This must be enforced at the logging architecture level, not by hunting down individual `fmt.Print` statements.

### Step 2: Initialize Logging First

```go
// cmd/amanmcp/cmd/serve.go

func runServe(ctx context.Context, transport string, port int) error {
    // Initialize MCP-safe logging FIRST, before ANYTHING else.
    mcpLogCleanup, logErr := logging.SetupMCPMode()
    if logErr != nil {
        return fmt.Errorf("failed to setup MCP logging: %w", logErr)
    }
    defer mcpLogCleanup()

    // Now the server can initialize safely
    // ...
}
```

### Step 3: Remove Stdout Writes from Entry Points

```go
// cmd/amanmcp/cmd/root.go

func runSmartDefault(ctx context.Context, root string) error {
    // MCP protocol requires stdout to be used EXCLUSIVELY for JSON-RPC.
    // We must NOT write ANY output to stdout before starting the MCP server.
    // All status output is suppressed in favor of file logging.

    // Silent operations with slog logging...
    slog.Debug("smart default mode", "root", root)

    // Start MCP server directly - NO stdout output before this point
    return runServe(ctx, "stdio", 0)
}
```

### Step 4: Replace All fmt.Fprintf with Structured Logging

```go
// BEFORE (dangerous)
fmt.Fprintf(os.Stderr, "OllamaEmbedder unavailable: %v, using static fallback\n", err)

// AFTER (safe)
slog.Warn("OllamaEmbedder unavailable, using static fallback",
    slog.String("error", err.Error()),
    slog.String("fallback", "static768"))
```

### The Result: Clean Protocol Stream

After the fix:

```
{"jsonrpc":"2.0","method":"..."}  <-- First byte is JSON-RPC
```

All diagnostic information now goes to `~/.amanmcp/logs/server.log`:

```json
{"time":"2026-01-04T20:52:11","level":"INFO","msg":"MCP mode logging initialized","log_file":"/Users/dev/.amanmcp/logs/server.log","stderr_disabled":true}
{"time":"2026-01-04T20:52:11","level":"INFO","msg":"=== AmanMCP Server Startup ===","version":"0.4.0","transport":"stdio"}
{"time":"2026-01-04T20:52:11","level":"DEBUG","msg":"Found project root","root":"/path/to/project"}
{"time":"2026-01-04T20:52:11","level":"INFO","msg":"MCP tools registered","count":4}
{"time":"2026-01-04T20:52:11","level":"INFO","msg":"MCP server ready","transport":"stdio"}
```

---

## Lessons for Protocol Implementers

### 1. Protocol Strictness is Non-Negotiable

JSON-RPC over stdio has zero tolerance for non-protocol bytes. There is no "mostly valid" - a single extra character before the first `{` breaks everything.

This applies to many protocols:
- **HTTP**: Headers must follow strict formatting rules
- **gRPC**: Framing bytes must be precise
- **WebSocket**: Handshake must be exact
- **LSP (Language Server Protocol)**: Same stdio constraints as MCP

**Rule**: If a protocol specifies a wire format, every byte matters. "It looks right" is not good enough.

### 2. "It Works in Tests" Does Not Mean "It Works in Production"

The unit tests passed because they tested components in isolation:
- Tool registration: tested in isolation (passed)
- Server initialization: tested in isolation (passed)
- MCP protocol handling: tested in isolation (passed)

But the bug only manifested when running the full flow with a real client (Claude Code). The tests never verified that stdout was clean at process startup.

**Rule**: End-to-end integration tests are not optional. For protocol code, write tests that verify the actual wire format.

### 3. Read Protocol Documentation First

The MCP debugging guide explicitly warned about stdout contamination. Reading the documentation before implementation would have prevented this bug entirely.

**Rule**: Before implementing a protocol, read the debugging and troubleshooting guides, not just the specification. They often contain hard-won wisdom about common pitfalls.

### 4. Centralized Logging Architecture

Ad-hoc logging is dangerous in protocol-sensitive code. When different parts of the codebase use:
- `fmt.Printf()` (writes to stdout)
- `fmt.Fprintf(os.Stderr, ...)` (writes to stderr)
- `log.Print()` (writes to stderr by default)
- Custom output writers

...it becomes impossible to guarantee protocol compliance.

**Rule**: For protocol servers, implement a centralized logging mode that:
1. Redirects all output to files
2. Provides a clear API (`slog`, `zerolog`, etc.)
3. Makes stdout/stderr writes impossible during protocol operation
4. Fails loudly if misconfigured

```go
// Good: Centralized logging with mode awareness
logging.SetupMCPMode()  // Guarantees no stdout/stderr

// Bad: Scattered logging calls
fmt.Printf("Debug: %v\n", value)  // Who knows where this goes?
```

---

## Debugging Checklist for Protocol Issues

When your protocol server fails to connect, work through this checklist:

### Immediate Checks

- [ ] **Check stdout/stderr contamination**: Run your server and capture all output before protocol starts
  ```bash
  your-server 2>&1 | head -1 | xxd | head
  ```
  The first bytes should be valid protocol data.

- [ ] **Verify with a minimal client**: Use a simple test client to rule out client-side issues
  ```bash
  echo '{"jsonrpc":"2.0","method":"initialize","id":1}' | your-server
  ```

- [ ] **Check for startup warnings**: Fallback handlers, missing config, deprecation warnings often print to stderr

### Architecture Checks

- [ ] **Audit all fmt.Print* calls**: Search your codebase
  ```bash
  grep -r "fmt\.Print\|fmt\.Fprint\|os\.Stdout\|os\.Stderr" --include="*.go"
  ```

- [ ] **Review initialization order**: Is logging configured before any other initialization?

- [ ] **Check third-party libraries**: Do any dependencies write to stdout/stderr?

### Prevention Checks

- [ ] **Add integration test for clean stdout**:
  ```go
  func TestMCPStartupHasCleanStdout(t *testing.T) {
      cmd := exec.Command("amanmcp", "serve", "--timeout=1s")
      stdout, _ := cmd.StdoutPipe()
      cmd.Start()

      first := make([]byte, 1)
      stdout.Read(first)

      if first[0] != '{' {
          t.Errorf("stdout contaminated: first byte is %q, want '{'", first[0])
      }
  }
  ```

- [ ] **Consider a linter rule**: Block direct stdout/stderr writes in protocol-related packages

- [ ] **Document the constraint**: Add a comment in your entry point:
  ```go
  // WARNING: MCP protocol requires stdout for JSON-RPC ONLY.
  // Do NOT add any Print/Println/Fprintf(os.Stdout) calls before runServe().
  ```

---

## Summary

Protocol debugging requires a different mindset than application debugging. The wire format is sacred - every byte must conform to specification. What appears to be a "failed to connect" error with no details often points to contamination of the communication channel.

The fix is not to hunt down individual bad log statements. The fix is architectural:
1. Create a protocol-safe logging mode
2. Initialize it before anything else
3. Make unsafe output impossible by design

When in doubt, capture and inspect the raw bytes your server sends. The answer is usually in the first few bytes of the stream.

---

## See Also

- [MCP Protocol Documentation](https://modelcontextprotocol.io/)
- [MCP Debugging Best Practices](https://modelcontextprotocol.io/legacy/tools/debugging)
- [JSON-RPC 2.0 Specification](https://www.jsonrpc.org/specification)
- [Static Embeddings Explained](./static-embeddings-explained.md) - Another deep-dive article on AmanMCP internals

---

**Based on:** Real production incident, January 2026
**Original Analysis:** `.aman-pm/postmortems/RCA-007` (internal)
**Last Updated:** 2026-01-16
