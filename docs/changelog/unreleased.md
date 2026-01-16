# Unreleased Changes

Changes that will be included in the next release.

---

## Added

## Changed

## Fixed

## Removed

## Documentation

- **FEAT-OSS4**: Complete documentation audit for open-source release
  - Fix version references (0.2.4/0.1.43 → 0.10.2) in README.md, first-time-user-guide.md
  - Fix embedding model references (8b → 0.6b) in homebrew-setup-guide.md, first-time-user-guide.md, configuration.md
  - Fix non-existent CLI flags (--skip-check, --reindex → index --force) in SYSTEM_REQUIREMENTS.md, first-time-user-guide.md
  - Fix MCP protocol attribution (remove incorrect Google reference) in mcp-protocol.md
  - Update glossary with current technology stack (coder/hnsw, Ollama, qwen3-embedding)
  - Fix relative path references in introduction.md

- **README restructure**: Reduce cognitive load with hub-and-spoke architecture
  - Slim README from 542 lines → 168 lines (~70% reduction)
  - Add intent-based navigation ("I want to..." table)
  - Add roadmap section
  - Create docs/reference/commands.md - full command reference
  - Create docs/guides/mlx-setup.md - Apple Silicon guide
  - Create docs/guides/backend-switching.md - backend management
  - Move Development section to CONTRIBUTING.md

- **Mermaid diagrams**: Add visual documentation across key guides
  - **architecture.md**: Replace ASCII art with 15+ Mermaid diagrams
    - High-level architecture flowchart
    - Search request sequence diagram
    - Indexing pipeline visualization
    - Query classification flow
    - RRF fusion explanation
    - AST chunking algorithm
    - Latency breakdown (gantt chart)
    - Graceful degradation fallback chain
    - Security boundaries
    - Plugin architecture (class diagram)
    - Test pyramid
  - **README.md**: Add data flow diagram (ASCII → Mermaid)
  - **hybrid-search.md**: Add search flow, RRF fusion, and vector space diagrams
  - **vector-search-concepts.md**: Add HNSW layer visualization, search algorithm sequence
  - **two-stage-retrieval.md**: Add pipeline flowchart, speed comparison xychart
  - **tree-sitter-guide.md**: Add AST visualization, incremental parsing, memory management

---

## Notes

This file is reset after each version release.

Add changes here as you work. Use present tense ("Add feature" not "Added feature").

Categories:

- **Added**: New features
- **Changed**: Changes in existing functionality
- **Deprecated**: Soon-to-be removed features
- **Removed**: Removed features
- **Fixed**: Bug fixes
- **Security**: Security-related changes
- **Documentation**: Documentation-only changes
