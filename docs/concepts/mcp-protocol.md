# MCP Protocol Basics

**Version:** 1.0.0
**Last Updated:** 2025-12-28

Learn how Model Context Protocol enables AI-tool integration in AmanMCP.

---

## Overview

MCP (Model Context Protocol) is an open protocol that standardizes how AI applications connect to external tools and data sources. It's like USB for AI - a universal interface.

**Why we use it**: AmanMCP is an MCP server. Claude Code, Cursor, and other AI assistants connect to it as clients to search codebases.

---

## The Problem MCP Solves

### Before MCP

Every AI app needed custom integrations:

```mermaid
graph LR
    Claude[Claude]
    Cursor[Cursor]
    Codebase[Codebase]
    GitHub[GitHub]
    Docs[Docs]

    Claude -.Custom API.-> Codebase
    Cursor -.Custom API.-> GitHub
    Cursor -.Another API.-> Docs

    Note[Result: N clients × M tools = N×M integrations]

    style Claude fill:#f9f,stroke:#333,stroke-width:2px
    style Cursor fill:#f9f,stroke:#333,stroke-width:2px
    style Codebase fill:#bbf,stroke:#333,stroke-width:2px
    style GitHub fill:#bbf,stroke:#333,stroke-width:2px
    style Docs fill:#bbf,stroke:#333,stroke-width:2px
```

**Result:** N clients × M tools = N×M integrations

### After MCP

One protocol, universal compatibility:

```mermaid
graph TB
    subgraph Clients
        Claude[Claude]
        Cursor[Cursor]
        AnyAI[Any AI]
    end

    subgraph Servers
        MCPServer[MCP Server]
        Codebase[Codebase]
        AnyTool[Any Tool]
    end

    Claude <-->|MCP| MCPServer
    Cursor <-->|MCP| MCPServer
    AnyAI <-->|MCP| MCPServer

    MCPServer --> Codebase
    MCPServer --> AnyTool

    style Claude fill:#f9f,stroke:#333,stroke-width:2px
    style Cursor fill:#f9f,stroke:#333,stroke-width:2px
    style AnyAI fill:#f9f,stroke:#333,stroke-width:2px
    style MCPServer fill:#9f9,stroke:#333,stroke-width:3px
    style Codebase fill:#bbf,stroke:#333,stroke-width:2px
    style AnyTool fill:#bbf,stroke:#333,stroke-width:2px
```

**Result:** N + M integrations (linear!)

---

## Core Concepts

### Client-Server Model

```mermaid
graph LR
    Client["MCP CLIENT<br/>(Claude Code)<br/><br/>- Sends queries<br/>- Uses results"]
    Server["MCP SERVER<br/>(AmanMCP)<br/><br/>- Handles tools<br/>- Returns data"]

    Client <-->|JSON-RPC| Server

    style Client fill:#f9f,stroke:#333,stroke-width:2px
    style Server fill:#9f9,stroke:#333,stroke-width:2px
```

**Client**: AI application that uses tools (Claude Code, Cursor)  
**Server**: Tool provider that exposes functionality (AmanMCP)  
**Transport**: Communication layer (stdio, SSE, HTTP)

### The Three Primitives

MCP has three core concepts:

| Primitive | Purpose | Who Provides |
|-----------|---------|--------------|
| **Tools** | Actions the AI can take | Server |
| **Resources** | Data the AI can access | Server |
| **Prompts** | Reusable templates | Server |

### Tools

Functions the AI can call:

```json
{
  "name": "search",
  "description": "Search the codebase for relevant code",
  "inputSchema": {
    "type": "object",
    "properties": {
      "query": {
        "type": "string",
        "description": "Search query"
      },
      "limit": {
        "type": "number",
        "description": "Maximum results"
      }
    },
    "required": ["query"]
  }
}
```

### Resources

Data the AI can read:

```json
{
  "uri": "file:///src/main.go",
  "name": "Main Application",
  "mimeType": "text/x-go",
  "description": "Entry point for the application"
}
```

### Prompts

Pre-defined templates:

```json
{
  "name": "explain-function",
  "description": "Explain what a function does",
  "arguments": [
    {
      "name": "function_name",
      "required": true
    }
  ]
}
```

---

## Protocol Details

### JSON-RPC 2.0

MCP uses JSON-RPC for communication:

**Request**:

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "tools/call",
  "params": {
    "name": "search",
    "arguments": {
      "query": "authentication",
      "limit": 10
    }
  }
}
```

**Response**:

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "content": [
      {
        "type": "text",
        "text": "Found 5 results for 'authentication'..."
      }
    ]
  }
}
```

### Message Types

| Method | Direction | Purpose |
|--------|-----------|---------|
| `initialize` | Client→Server | Start session |
| `tools/list` | Client→Server | Get available tools |
| `tools/call` | Client→Server | Execute a tool |
| `resources/list` | Client→Server | Get available resources |
| `resources/read` | Client→Server | Read a resource |
| `prompts/list` | Client→Server | Get available prompts |
| `prompts/get` | Client→Server | Get a prompt template |

### Lifecycle

```mermaid
sequenceDiagram
    participant Client
    participant Server

    Client->>Server: initialize
    Server->>Client: initialize response

    Client->>Server: tools/list
    Server->>Client: available tools

    Client->>Server: tools/call (search)
    Server->>Client: search results

    Client->>Server: (more interactions)
```

### Complete MCP Lifecycle

A comprehensive view of the full MCP lifecycle from initialization to shutdown:

```mermaid
sequenceDiagram
    participant Client as MCP Client<br/>(Claude Code)
    participant Server as MCP Server<br/>(AmanMCP)
    participant Search as Search Engine

    Note over Client,Server: Initialization Phase
    Client->>Server: initialize (capabilities, version)
    activate Server
    Server->>Client: initialize response (server capabilities)
    deactivate Server

    Note over Client,Server: Discovery Phase
    Client->>Server: tools/list
    activate Server
    Server->>Client: [search, lookup, similar, index-status]
    deactivate Server

    Client->>Server: resources/list
    activate Server
    Server->>Client: [file://.., indexed files]
    deactivate Server

    Note over Client,Server: Request/Response Phase
    Client->>Server: tools/call: search("authentication", limit: 10)
    activate Server
    Server->>Search: Execute hybrid search
    activate Search
    Search->>Search: BM25 + Vector → RRF
    Search->>Server: Top 10 results
    deactivate Search
    Server->>Client: Formatted results with code chunks
    deactivate Server

    Client->>Server: resources/read (file://auth.go)
    activate Server
    Server->>Client: File contents
    deactivate Server

    Note over Client,Server: Optional: Progress Reporting
    Client->>Server: tools/call: reindex (with progress token)
    activate Server
    Server-->>Client: progress notification (10% complete)
    Server-->>Client: progress notification (50% complete)
    Server-->>Client: progress notification (100% complete)
    Server->>Client: Reindex complete
    deactivate Server

    Note over Client,Server: Shutdown Phase
    Client->>Server: shutdown
    activate Server
    Server->>Client: shutdown acknowledgment
    deactivate Server
```

---

## AmanMCP as MCP Server

### Tools We Expose

```go
var amanMCPTools = []mcp.Tool{
    {
        Name:        "search",
        Description: "Hybrid search over codebase (BM25 + semantic)",
        InputSchema: searchInputSchema,
    },
    {
        Name:        "lookup",
        Description: "Get specific code by file path and symbol",
        InputSchema: lookupInputSchema,
    },
    {
        Name:        "similar",
        Description: "Find code similar to a reference",
        InputSchema: similarInputSchema,
    },
    {
        Name:        "index-status",
        Description: "Check indexing status and statistics",
        InputSchema: nil,
    },
}
```

### Resources We Expose

```go
func (s *Server) ListResources() []mcp.Resource {
    var resources []mcp.Resource

    // Expose indexed files as resources
    for _, file := range s.index.Files() {
        resources = append(resources, mcp.Resource{
            URI:      "file://" + file.Path,
            Name:     filepath.Base(file.Path),
            MimeType: mimeTypeFor(file.Path),
        })
    }

    return resources
}
```

### Transport: stdio

AmanMCP uses stdio transport (simplest):

```mermaid
sequenceDiagram
    participant Claude Code
    participant AmanMCP

    Claude Code->>AmanMCP: stdin (JSON-RPC request)
    AmanMCP->>Claude Code: stdout (JSON-RPC response)
```

Configuration in Claude Code:

```json
{
  "mcpServers": {
    "amanmcp": {
      "command": "amanmcp",
      "args": ["serve"]
    }
  }
}
```

---

## Implementing MCP in Go

### Using the Official SDK

AmanMCP uses the official MCP Go SDK from Anthropic:

```go
import (
    "github.com/modelcontextprotocol/go-sdk/mcp"
    "github.com/modelcontextprotocol/go-sdk/server"
)

func main() {
    s := server.NewMCPServer(
        "AmanMCP",
        "0.1.0",
        server.WithToolCapabilities(true),
        server.WithResourceCapabilities(true, false),
    )

    // Register tools
    s.AddTool(mcp.NewTool("search",
        mcp.WithDescription("Search the codebase"),
        mcp.WithString("query", mcp.Required()),
        mcp.WithNumber("limit"),
    ), handleSearch)

    // Start server (stdio transport)
    server.ServeStdio(s)
}

func handleSearch(ctx context.Context, req mcp.CallToolRequest) (*mcp.CallToolResult, error) {
    query := req.Params.Arguments["query"].(string)
    limit := 10
    if l, ok := req.Params.Arguments["limit"]; ok {
        limit = int(l.(float64))
    }

    results, err := searchEngine.Search(query, limit)
    if err != nil {
        return nil, err
    }

    return mcp.NewToolResultText(formatResults(results)), nil
}
```

### Tool Handler Pattern

```go
type ToolHandler func(ctx context.Context, args map[string]any) (*mcp.CallToolResult, error)

func (s *Server) registerTools() {
    tools := map[string]ToolHandler{
        "search":       s.handleSearch,
        "lookup":       s.handleLookup,
        "similar":      s.handleSimilar,
        "index-status": s.handleStatus,
    }

    for name, handler := range tools {
        s.mcp.AddTool(s.toolDef(name), handler)
    }
}
```

### Error Handling

```go
func handleSearch(ctx context.Context, req mcp.CallToolRequest) (*mcp.CallToolResult, error) {
    query, ok := req.Params.Arguments["query"].(string)
    if !ok || query == "" {
        return nil, &mcp.Error{
            Code:    mcp.InvalidParams,
            Message: "query parameter is required",
        }
    }

    results, err := engine.Search(query)
    if err != nil {
        // Internal error - logged, generic message to client
        log.Printf("search error: %v", err)
        return nil, &mcp.Error{
            Code:    mcp.InternalError,
            Message: "search failed",
        }
    }

    return mcp.NewToolResultText(format(results)), nil
}
```

### MCP Error Handling Flow

How errors are handled and propagated in the MCP protocol:

```mermaid
flowchart TB
    Start([Client Request]) --> Validate{Validate<br/>Request}

    Validate -->|Invalid JSON-RPC| E1["Return Error<br/>Code: -32700<br/>Parse error"]
    Validate -->|Unknown method| E2["Return Error<br/>Code: -32601<br/>Method not found"]
    Validate -->|Valid| CheckParams{Validate<br/>Parameters}

    CheckParams -->|Missing required| E3["Return Error<br/>Code: -32602<br/>Invalid params<br/>Message: 'query required'"]
    CheckParams -->|Wrong type| E4["Return Error<br/>Code: -32602<br/>Invalid params<br/>Message: 'query must be string'"]
    CheckParams -->|Valid| Execute[Execute Tool]

    Execute --> ExecResult{Execution<br/>Result}

    ExecResult -->|Success| Success["Return Success<br/>Code: 200<br/>Result: {...}"]
    ExecResult -->|Business Error| E5["Return Error<br/>Code: -32603<br/>Internal error<br/>Log detailed error"]
    ExecResult -->|Timeout| E6["Return Error<br/>Code: -32000<br/>Server error<br/>Message: 'operation timeout'"]
    ExecResult -->|Cancelled| E7["Return Error<br/>Code: -32000<br/>Server error<br/>Message: 'operation cancelled'"]

    E1 --> Response([JSON-RPC Error Response])
    E2 --> Response
    E3 --> Response
    E4 --> Response
    E5 --> Response
    E6 --> Response
    E7 --> Response
    Success --> SuccessResp([JSON-RPC Success Response])

    subgraph ErrorCodes["MCP Error Codes"]
        direction TB
        Codes["
        Standard JSON-RPC:
        • -32700: Parse error
        • -32600: Invalid request
        • -32601: Method not found
        • -32602: Invalid params
        • -32603: Internal error

        MCP-specific:
        • -32000: Server error
        • -32001: Resource not found
        • -32002: Resource access denied
        "]
    end

    Response --> ErrorCodes
    SuccessResp --> ErrorCodes

    style Start fill:#3498db,stroke-width:2px
    style Validate fill:#fff9c4
    style CheckParams fill:#fff9c4
    style ExecResult fill:#fff9c4
    style E1 fill:#e74c3c,stroke-width:2px
    style E2 fill:#e74c3c,stroke-width:2px
    style E3 fill:#f39c12,stroke-width:2px
    style E4 fill:#f39c12,stroke-width:2px
    style E5 fill:#e74c3c,stroke-width:2px
    style E6 fill:#e74c3c,stroke-width:2px
    style E7 fill:#e74c3c,stroke-width:2px
    style Success fill:#27ae60,stroke-width:2px
    style Execute fill:#9b59b6,stroke-width:2px
    style Response fill:#ffccbc
    style SuccessResp fill:#c8e6c9
    style ErrorCodes fill:#e1f5ff
```

**Error Response Format:**

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "error": {
    "code": -32602,
    "message": "Invalid params",
    "data": {
      "parameter": "query",
      "issue": "required parameter missing"
    }
  }
}
```

**Best Practices:**
1. **Validate early**: Check parameters before expensive operations
2. **Use appropriate codes**: Standard codes for protocol errors, custom for business logic
3. **Log internally**: Detailed errors in logs, generic messages to clients
4. **Include context**: Use `data` field for debugging information

---

## AmanMCP Integration Flow

### Full Request Flow

```mermaid
graph TB
    User["User types in Claude<br/>'Where is authentication handled?'"]
    ClaudeCode["Claude Code<br/>(MCP Client)<br/>Decides to use AmanMCP search tool"]
    AmanMCP["AmanMCP<br/>(MCP Server)<br/>tools/call: search('authentication')"]
    SearchEngine["Hybrid Search Engine<br/>BM25 + Vector → RRF Fusion"]
    Results["Results (10 chunks)<br/>auth.go, session.go, middleware.go"]
    Response["Claude Response<br/>'Authentication is handled in auth.go...'"]

    User --> ClaudeCode
    ClaudeCode -->|JSON-RPC via stdio| AmanMCP
    AmanMCP --> SearchEngine
    SearchEngine --> Results
    Results --> Response

    style User fill:#ffe,stroke:#333,stroke-width:2px
    style ClaudeCode fill:#f9f,stroke:#333,stroke-width:2px
    style AmanMCP fill:#9f9,stroke:#333,stroke-width:2px
    style SearchEngine fill:#bbf,stroke:#333,stroke-width:2px
    style Results fill:#fdb,stroke:#333,stroke-width:2px
    style Response fill:#bfb,stroke:#333,stroke-width:2px
```

### Integration Architecture

How MCP integrates with AmanMCP's search engine components:

```mermaid
graph TB
    subgraph "MCP Client Layer"
        Client[Claude Code / Cursor]
    end

    subgraph "MCP Server Layer (AmanMCP)"
        MCPServer[MCP Server<br/>JSON-RPC Handler]
        ToolRouter[Tool Router]

        subgraph "Tool Handlers"
            SearchTool[search handler]
            LookupTool[lookup handler]
            SimilarTool[similar handler]
            StatusTool[index-status handler]
        end

        ResourceMgr[Resource Manager]
    end

    subgraph "Search Engine Layer"
        SearchEngine[Hybrid Search Engine]

        subgraph "Search Components"
            BM25[BM25 Index]
            Vector[Vector Store<br/>HNSW]
            RRF[RRF Fusion<br/>k=60]
        end
    end

    subgraph "Storage Layer"
        SQLite[(SQLite<br/>Metadata)]
        VectorDB[(Vector DB<br/>Embeddings)]
        FSWatcher[File System<br/>Watcher]
    end

    subgraph "Indexing Pipeline"
        Scanner[File Scanner]
        Chunker[tree-sitter<br/>Chunker]
        Embedder[Embedder<br/>Ollama/MLX]
    end

    Client <-->|JSON-RPC<br/>stdio| MCPServer
    MCPServer --> ToolRouter
    ToolRouter --> SearchTool
    ToolRouter --> LookupTool
    ToolRouter --> SimilarTool
    ToolRouter --> StatusTool

    SearchTool --> SearchEngine
    LookupTool --> SearchEngine
    SimilarTool --> SearchEngine
    StatusTool --> SearchEngine

    MCPServer --> ResourceMgr
    ResourceMgr --> SQLite

    SearchEngine --> BM25
    SearchEngine --> Vector
    BM25 --> RRF
    Vector --> RRF

    BM25 <--> SQLite
    Vector <--> VectorDB

    FSWatcher -->|file changes| Scanner
    Scanner --> Chunker
    Chunker --> Embedder
    Embedder --> BM25
    Embedder --> Vector

    style Client fill:#f9f,stroke:#333,stroke-width:2px
    style MCPServer fill:#9f9,stroke:#333,stroke-width:3px
    style SearchEngine fill:#bbf,stroke:#333,stroke-width:2px
    style RRF fill:#fdb,stroke:#333,stroke-width:2px
    style SQLite fill:#ddd,stroke:#333,stroke-width:2px
    style VectorDB fill:#ddd,stroke:#333,stroke-width:2px
```

This diagram shows:

- **MCP Client Layer**: AI assistants (Claude Code, Cursor) connect via JSON-RPC over stdio
- **MCP Server Layer**: Handles protocol, routes to tool handlers
- **Tool Handlers**: Implement search, lookup, similar, and status operations
- **Search Engine Layer**: Hybrid search with BM25 + Vector → RRF fusion
- **Storage Layer**: SQLite for metadata, Vector DB for embeddings
- **Indexing Pipeline**: File scanning → chunking → embedding → storage

### Tool Responses

What Claude sees from our tools:

```json
{
  "content": [
    {
      "type": "text",
      "text": "## Search Results for \"authentication\"\n\n### 1. internal/auth/handler.go (0.95)\n```go\nfunc AuthMiddleware(next http.Handler) http.Handler {\n    return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {\n        token := r.Header.Get(\"Authorization\")\n        // ...\n    })\n}\n```\n\n### 2. internal/auth/jwt.go (0.87)\n..."
    }
  ]
}
```

---

## MCP Specification

### Current Version

AmanMCP implements MCP spec version **2025-11-25**.

Key features:

- JSON-RPC 2.0 transport
- Tool calling with schema validation
- Resource access with URI scheme
- Prompt templates
- Progress notifications
- Cancellation support

### Capabilities

Servers declare what they support:

```go
server.NewMCPServer(
    "AmanMCP",
    "0.1.0",
    server.WithToolCapabilities(true),
    server.WithResourceCapabilities(
        true,  // subscribe supported
        true,  // listChanged supported
    ),
    server.WithPromptCapabilities(true),
)
```

---

## Configuration

### Claude Code Config

Create `.mcp.json` in your project root:

```json
{
  "mcpServers": {
    "amanmcp": {
      "command": "amanmcp",
      "args": ["serve"],
      "cwd": "/path/to/your/project",
      "env": {
        "AMANMCP_LOG_LEVEL": "info"
      }
    }
  }
}
```

> **Important:** The `cwd` parameter is required because Claude Code doesn't automatically set the working directory when spawning MCP servers.

### Cursor Config

Create `.cursor/mcp.json` in your project root:

```json
{
  "mcpServers": {
    "amanmcp": {
      "command": "amanmcp",
      "args": ["serve"],
      "cwd": "/path/to/your/project"
    }
  }
}
```

---

## Common Patterns

### Progress Reporting

For long operations:

```go
func (s *Server) handleReindex(ctx context.Context, req mcp.CallToolRequest) (*mcp.CallToolResult, error) {
    total := len(files)

    for i, file := range files {
        // Send progress notification
        s.mcp.SendProgress(mcp.Progress{
            Token:   req.ProgressToken,
            Current: float64(i),
            Total:   float64(total),
            Message: fmt.Sprintf("Indexing %s", file.Name),
        })

        if err := s.index(file); err != nil {
            return nil, err
        }
    }

    return mcp.NewToolResultText("Indexed " + strconv.Itoa(total) + " files"), nil
}
```

### Pagination

For large result sets:

```go
type SearchArgs struct {
    Query  string `json:"query"`
    Limit  int    `json:"limit"`
    Offset int    `json:"offset"`
}

func handleSearch(args SearchArgs) *mcp.CallToolResult {
    results := engine.Search(args.Query, args.Limit+1, args.Offset)

    hasMore := len(results) > args.Limit
    if hasMore {
        results = results[:args.Limit]
    }

    return formatWithPagination(results, args.Offset, hasMore)
}
```

---

## Testing MCP Servers

### Manual Testing with mcp-cli

```bash
# Install MCP inspector
npm install -g @modelcontextprotocol/inspector

# Test your server
mcp-inspector amanmcp serve
```

### Unit Testing Tools

```go
func TestSearchTool(t *testing.T) {
    server := NewTestServer(t)

    result, err := server.CallTool(context.Background(), mcp.CallToolRequest{
        Params: mcp.CallToolParams{
            Name: "search",
            Arguments: map[string]any{
                "query": "authentication",
                "limit": 5,
            },
        },
    })

    require.NoError(t, err)
    assert.NotEmpty(t, result.Content)

    // Verify result format
    text := result.Content[0].(mcp.TextContent).Text
    assert.Contains(t, text, "Search Results")
}
```

---

## Common Mistakes

### 1. Not Validating Input

```go
// BAD: Trusts all input
query := args["query"].(string)  // Might panic

// GOOD: Validate
query, ok := args["query"].(string)
if !ok || query == "" {
    return nil, &mcp.Error{Code: mcp.InvalidParams, Message: "query required"}
}
```

### 2. Blocking stdio

```go
// BAD: Blocks the event loop
time.Sleep(10 * time.Second)  // Blocks all MCP traffic

// GOOD: Use goroutines for long work
go func() {
    doLongWork()
    s.SendNotification(mcp.Notification{...})
}()
```

### 3. Not Handling Cancellation

```go
// BAD: Ignores context
for _, file := range files {
    index(file)  // Continues even if cancelled
}

// GOOD: Check context
for _, file := range files {
    select {
    case <-ctx.Done():
        return nil, ctx.Err()
    default:
        index(file)
    }
}
```

---

## Further Reading

- [MCP Specification](https://modelcontextprotocol.io/specification/2025-11-25)
- [Official MCP Go SDK](https://github.com/modelcontextprotocol/go-sdk)
- [MCP Servers Directory](https://github.com/modelcontextprotocol/servers)
- [Claude Code MCP Docs](https://docs.anthropic.com/claude/docs/claude-code)

---

*MCP connects AI to tools. Build the bridge, let AI cross it.*
