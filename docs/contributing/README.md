# Contributing to AmanMCP

Welcome! We're excited that you want to contribute. This section contains detailed guides for developers.

**Start here**: [CONTRIBUTING.md](../../CONTRIBUTING.md) - Main contribution guide

---

## Developer Documentation

| Guide | What You'll Learn | Read This If... |
|-------|-------------------|-----------------|
| [Code Conventions](code-conventions.md) | Go patterns, naming, structure | You're writing Go code for AmanMCP |
| [TDD Rationale](tdd-rationale.md) | Why test-first, the TDD cycle | You want to understand our testing philosophy |
| [Testing Guide](testing-guide.md) | How to run tests, validation framework | You're adding features or fixing bugs |

---

## Quick Links for Contributors

### Getting Started
1. **Fork & Clone** - [CONTRIBUTING.md](../../CONTRIBUTING.md#setup)
2. **Development Setup** - Install dependencies, build locally
3. **Run Tests** - `make test` and `make ci-check`
4. **Code Conventions** - [code-conventions.md](code-conventions.md)

### Making Changes
1. **Create Branch** - `git checkout -b feature/your-feature`
2. **Write Tests First** - TDD approach (see [testing-guide.md](testing-guide.md))
3. **Follow Conventions** - [code-conventions.md](code-conventions.md)
4. **Run CI Locally** - `make ci-check` (must pass)
5. **Add Changelog Entry** - `.aman-pm/changelog/unreleased.md`

### Submitting
1. **Commit Messages** - Use [Conventional Commits](https://www.conventionalcommits.org/)
2. **Push & PR** - See [CONTRIBUTING.md](../../CONTRIBUTING.md#pull-requests)
3. **CI Must Pass** - Tests, linting, coverage all green

---

## Contribution Areas

Looking for where to help? Check:
- [Open Issues](https://github.com/Aman-CERP/amanmcp/issues) - Bug fixes and features
- [CONTRIBUTING.md](../../CONTRIBUTING.md#priority-areas) - High-priority areas
- [Research](../research/) - Technical decisions (challenge or improve them!)

---

## Development Commands

```bash
# Build
make build

# Run tests
make test

# Run tests with race detector
make test-race

# Run linter
make lint

# Full CI check (run before PR)
make ci-check

# Install locally
make install-local
```

---

## Code Quality Standards

✅ **All tests pass** - `make test`
✅ **No race conditions** - `make test-race`
✅ **Linter clean** - `make lint`
✅ **Coverage ≥ 25%** - `make coverage`
✅ **CI passes** - `make ci-check`
✅ **Changelog entry** - `.aman-pm/changelog/unreleased.md`

---

## Questions?

- **General**: File an issue or ask in discussions
- **Code Review**: Tag maintainers in your PR
- **Architecture**: See [Architecture](../reference/architecture/architecture.md) or [Research](../research/)

---

## Related Documentation

- [CONTRIBUTING.md](../../CONTRIBUTING.md) - Main contribution guide
- [Architecture](../reference/architecture/architecture.md) - System design
- [Research](../research/) - Technical decisions
- [Concepts](../concepts/) - How systems work
