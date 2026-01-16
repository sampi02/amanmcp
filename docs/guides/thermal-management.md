# Thermal Management Guide

This guide explains how to configure amanmcp for sustained GPU workloads, particularly on Apple Silicon machines that may experience thermal throttling during long indexing operations.

---

## Overview

When indexing large codebases, GPUs can heat up significantly. This can cause:

1. **Thermal Throttling**: GPU reduces performance to prevent overheating
2. **Timeout Failures**: Embedding requests that normally take 2s might take 10s+
3. **99% Completion Failure**: Indexing fails near the end when GPU is hottest

amanmcp includes thermal-aware indexing features that help prevent these issues.

---

## Default Behavior

**Most users don't need to configure anything.** The defaults work well for:

- Desktop machines with good cooling
- Short indexing operations (< 5000 chunks)
- Moderate ambient temperatures

The defaults are:

| Setting | Default | Effect |
|---------|---------|--------|
| Inter-batch delay | 0 (disabled) | No pause between batches |
| Timeout progression | 1.0 (disabled) | Fixed timeout throughout |
| Retry timeout multiplier | 1.0 (disabled) | No timeout increase on retry |

---

## When to Configure

Consider enabling thermal management if you experience:

1. **Indexing fails at high percentages** (80%+) with "context deadline exceeded"
2. **Laptop fans running at maximum** during indexing
3. **Apple Silicon Mac** (M1/M2/M3) running for extended periods
4. **Large codebase** (10,000+ chunks)

### Should I Enable Thermal Management?

```mermaid
flowchart TD
    Start([Starting Indexing]) --> DeviceType{What device<br/>are you using?}

    DeviceType -->|Desktop with<br/>active cooling| Desktop[Good cooling available]
    DeviceType -->|Laptop| Laptop[Limited cooling]

    Desktop --> DesktopCodebase{Codebase size?}
    Laptop --> LaptopCodebase{Codebase size?}

    DesktopCodebase -->|< 5,000 chunks| DesktopSmall[Use Defaults]
    DesktopCodebase -->|5,000-20,000 chunks| DesktopMedium{Previous indexing<br/>issues?}
    DesktopCodebase -->|> 20,000 chunks| DesktopLarge[Enable Minimal Preset]

    LaptopCodebase -->|< 5,000 chunks| LaptopSmall{Apple Silicon<br/>M1/M2/M3?}
    LaptopCodebase -->|5,000-20,000 chunks| LaptopMedium[Enable Moderate Preset]
    LaptopCodebase -->|> 20,000 chunks| LaptopLarge[Enable Aggressive Preset]

    DesktopMedium -->|No| DesktopMediumOK[Use Defaults]
    DesktopMedium -->|Yes| DesktopMediumIssue[Enable Minimal Preset]

    LaptopSmall -->|Yes| LaptopSmallApple[Enable Minimal Preset]
    LaptopSmall -->|No| LaptopSmallIntel[Use Defaults]

    DesktopSmall --> Monitor1[Monitor first run]
    DesktopMediumOK --> Monitor2[Monitor for issues]
    LaptopSmallIntel --> Monitor3[Monitor for issues]

    DesktopMediumIssue --> Apply1[inter_batch_delay: 100ms<br/>timeout_progression: 1.2]
    DesktopLarge --> Apply2[inter_batch_delay: 100ms<br/>timeout_progression: 1.2]
    LaptopSmallApple --> Apply3[inter_batch_delay: 100ms<br/>timeout_progression: 1.2]
    LaptopMedium --> Apply4[inter_batch_delay: 200ms<br/>timeout_progression: 1.5<br/>retry_timeout_multiplier: 1.5]
    LaptopLarge --> Apply5[inter_batch_delay: 500ms<br/>timeout_progression: 2.0<br/>retry_timeout_multiplier: 2.0]

    Monitor1 --> Done1([Indexing Complete])
    Monitor2 --> Done2([Indexing Complete])
    Monitor3 --> Done3([Indexing Complete])
    Apply1 --> Done4([Indexing Complete])
    Apply2 --> Done5([Indexing Complete])
    Apply3 --> Done6([Indexing Complete])
    Apply4 --> Done7([Indexing Complete])
    Apply5 --> Done8([Indexing Complete])

    style Start fill:#e1f5ff
    style Done1 fill:#c8e6c9
    style Done2 fill:#c8e6c9
    style Done3 fill:#c8e6c9
    style Done4 fill:#c8e6c9
    style Done5 fill:#c8e6c9
    style Done6 fill:#c8e6c9
    style Done7 fill:#c8e6c9
    style Done8 fill:#c8e6c9
    style DesktopSmall fill:#c8e6c9
    style DesktopMediumOK fill:#c8e6c9
    style LaptopSmallIntel fill:#c8e6c9
    style DesktopMediumIssue fill:#fff9c4
    style DesktopLarge fill:#fff9c4
    style LaptopSmallApple fill:#fff9c4
    style LaptopMedium fill:#ffe0b2
    style LaptopLarge fill:#ffab91
```

---

## Configuration Options

### Via User Config (Recommended)

Thermal settings are machine-specific and should go in your **user configuration** (applies to all projects on this machine):

```bash
# Create user config if it doesn't exist
amanmcp config init
```

Then edit `~/.config/amanmcp/config.yaml`:

```yaml
version: 1

embeddings:
  provider: ollama
  model: qwen3-embedding:8b
  ollama_host: http://localhost:11434

  # Thermal management settings (Apple Silicon / GPU throttling)
  inter_batch_delay: 200ms       # Pause between embedding batches
  timeout_progression: 1.5       # 50% timeout increase over indexing
  retry_timeout_multiplier: 1.5  # 50% timeout increase per retry
```

### Via Project Config (Per-Project Override)

If you need different thermal settings for a specific project, add to `.amanmcp.yaml`:

```yaml
embeddings:
  # Thermal management settings (overrides user config)
  inter_batch_delay: 500ms       # More aggressive for large project
```

### Via Environment Variables

Override settings temporarily without modifying config:

```bash
# For Apple Silicon with thermal throttling
AMANMCP_INTER_BATCH_DELAY=200ms \
AMANMCP_TIMEOUT_PROGRESSION=1.5 \
AMANMCP_RETRY_TIMEOUT_MULTIPLIER=1.5 \
amanmcp init --force
```

### Configuration Precedence

Settings are applied in order (later overrides earlier):

1. **Defaults** (hardcoded in binary)
2. **User config** (`~/.config/amanmcp/config.yaml`)
3. **Project config** (`.amanmcp.yaml` in project root)
4. **Environment variables** (`AMANMCP_*`)

```mermaid
flowchart TB
    Start([Thermal Setting Lookup]) --> Defaults[Defaults<br/>inter_batch_delay: 0<br/>timeout_progression: 1.0<br/>retry_timeout_multiplier: 1.0]

    Defaults --> UserConfig{User Config<br/>Exists?}
    UserConfig -->|Yes| ApplyUser[Apply User Config<br/>~/.config/amanmcp/config.yaml]
    UserConfig -->|No| ProjectConfig

    ApplyUser --> ProjectConfig{Project Config<br/>Exists?}
    ProjectConfig -->|Yes| ApplyProject[Apply Project Config<br/>.amanmcp.yaml]
    ProjectConfig -->|No| EnvVars

    ApplyProject --> EnvVars{Environment<br/>Variables Set?}
    EnvVars -->|Yes| ApplyEnv[Apply Environment Variables<br/>AMANMCP_INTER_BATCH_DELAY<br/>AMANMCP_TIMEOUT_PROGRESSION<br/>AMANMCP_RETRY_TIMEOUT_MULTIPLIER]
    EnvVars -->|No| Final

    ApplyEnv --> Final([Final Configuration])

    style Start fill:#e1f5ff
    style Final fill:#c8e6c9
    style Defaults fill:#fff9c4
    style ApplyUser fill:#ffe0b2
    style ApplyProject fill:#ffccbc
    style ApplyEnv fill:#ffab91
```

---

## Setting Descriptions

### Inter-Batch Delay

**What it does**: Pauses between embedding batches to let GPU cool.

**Format**: Duration string (e.g., "100ms", "500ms", "1s")

**Range**: 0 (disabled) to 5s (maximum)

**Recommendation**:

- Start with `200ms` if experiencing issues
- Increase to `500ms` for severe throttling
- Trade-off: Longer delay = slower indexing, but more reliable

```yaml
embeddings:
  inter_batch_delay: 200ms
```

### Timeout Progression

**What it does**: Gradually increases timeout as indexing progresses.

**Formula**: `effective_timeout = base_timeout * (1 + progress * (progression - 1))`

**Range**: 1.0 (disabled) to 3.0 (maximum)

**Example with progression=1.5**:

- At 0%: 60s timeout (base)
- At 50%: 75s timeout (1.25x)
- At 100%: 90s timeout (1.5x)

**Recommendation**:

- Start with `1.5` for moderate issues
- Use `2.0` for severe throttling
- Maximum is `3.0` (3x base timeout at end)

```yaml
embeddings:
  timeout_progression: 1.5
```

### Retry Timeout Multiplier

**What it does**: Increases timeout on each retry attempt.

**Formula**: `retry_timeout = base_timeout * (multiplier ^ attempt)`

**Range**: 1.0 (disabled) to 2.0 (maximum)

**Example with multiplier=1.5 and 3 retries**:

- Attempt 1: 60s
- Attempt 2: 90s (1.5x)
- Attempt 3: 135s (2.25x, capped at 2.0x)

**Recommendation**:

- Start with `1.5`
- Helps when occasional batch failures occur due to temporary throttling

```yaml
embeddings:
  retry_timeout_multiplier: 1.5
```

---

## Recommended Presets

### Minimal (Light throttling)

```yaml
embeddings:
  inter_batch_delay: 100ms
  timeout_progression: 1.2
  retry_timeout_multiplier: 1.2
```

### Moderate (Typical Apple Silicon laptop)

```yaml
embeddings:
  inter_batch_delay: 200ms
  timeout_progression: 1.5
  retry_timeout_multiplier: 1.5
```

### Aggressive (Severe throttling, hot environment)

```yaml
embeddings:
  inter_batch_delay: 500ms
  timeout_progression: 2.0
  retry_timeout_multiplier: 2.0
```

---

## Monitoring

### Check GPU Temperature (macOS)

```bash
# Install powermetrics if needed
sudo powermetrics --samplers gpu_power -i 1000 -n 1
```

### Watch Indexing Progress

```bash
# Monitor for timeout warnings
amanmcp init --force 2>&1 | grep -i timeout
```

### Check Ollama Model Status

```bash
# See if model is still loaded
ollama ps

# Check API status
curl http://localhost:11434/api/ps | jq
```

---

## Troubleshooting

### Diagnostic Flowchart

```mermaid
flowchart TD
    Start([Indexing Issue]) --> Symptom{What's happening?}

    Symptom -->|Fails at 80%+<br/>with timeout| Timeout[Timeout at High Progress]
    Symptom -->|Fans maxed out,<br/>slow progress| Overheating[GPU Overheating]
    Symptom -->|Indexing too slow<br/>but working| Slow[Performance Too Slow]
    Symptom -->|Random failures<br/>throughout| Random[Intermittent Failures]

    Timeout --> TimeoutCheck{Current timeout<br/>progression?}
    TimeoutCheck -->|Not set or < 1.5| TimeoutFix1[Set timeout_progression: 2.0<br/>retry_timeout_multiplier: 1.5]
    TimeoutCheck -->|>= 1.5| TimeoutFix2[Check GPU temperature<br/>May need aggressive preset]

    Overheating --> DelayCheck{Current inter_batch_delay?}
    DelayCheck -->|Not set or 0| DelayFix1[Set inter_batch_delay: 200ms]
    DelayCheck -->|< 200ms| DelayFix2[Increase to 500ms]
    DelayCheck -->|>= 500ms| DelayFix3[Check cooling:<br/>- Not on soft surface?<br/>- Clean air vents?<br/>- Cooler environment?]

    Slow --> SlowCheck{What delay set?}
    SlowCheck -->|> 200ms| SlowFix1[Reduce inter_batch_delay<br/>Try 100ms or 50ms]
    SlowCheck -->|<= 200ms| SlowFix2{Can tolerate<br/>slower indexing?}
    SlowFix2 -->|Yes| SlowAccept[Keep current settings<br/>Reliability > Speed]
    SlowFix2 -->|No| SlowModel[Switch to smaller model:<br/>nomic-embed-text]

    Random --> RandomCheck{Pattern in failures?}
    RandomCheck -->|Same chunks fail| RandomChunk[Problematic chunk:<br/>Check chunk size limits]
    RandomCheck -->|Different each time| RandomRetry[Increase retry_timeout_multiplier<br/>to 1.5 or 2.0]

    TimeoutFix1 --> Test1[Test with amanmcp init --force]
    TimeoutFix2 --> CheckTemp[sudo powermetrics<br/>--samplers gpu_power -i 1000 -n 1]
    DelayFix1 --> Test2[Test with amanmcp init --force]
    DelayFix2 --> Test3[Test with amanmcp init --force]
    DelayFix3 --> Physical[Physical environment check]
    SlowFix1 --> Test4[Test with amanmcp init --force]
    SlowAccept --> Done1([Accept Current Performance])
    SlowModel --> Test5[Test with smaller model]
    RandomChunk --> Investigate[Review chunk in .amanmcp/chunks.db]
    RandomRetry --> Test6[Test with amanmcp init --force]

    CheckTemp --> TempResult{Temperature<br/>> 100°C?}
    TempResult -->|Yes| UseAggressive[Use Aggressive Preset:<br/>inter_batch_delay: 500ms<br/>timeout_progression: 2.0]
    TempResult -->|No| CheckOllama[Check Ollama status:<br/>ollama ps]

    Physical --> PhysicalResult{Cooling improved?}
    PhysicalResult -->|Yes| Test7[Test with amanmcp init --force]
    PhysicalResult -->|No| UseAggressive2[Must use Aggressive Preset]

    CheckOllama --> OllamaResult{Ollama<br/>responsive?}
    OllamaResult -->|Yes| Other[Check other processes<br/>using GPU]
    OllamaResult -->|No| RestartOllama[Restart Ollama service]

    Test1 --> Success1{Works?}
    Test2 --> Success2{Works?}
    Test3 --> Success3{Works?}
    Test4 --> Success4{Works?}
    Test5 --> Success5{Works?}
    Test6 --> Success6{Works?}
    Test7 --> Success7{Works?}
    UseAggressive --> Test8[Test with amanmcp init --force]
    Test8 --> Success8{Works?}
    UseAggressive2 --> Done2([Use Aggressive Settings])
    RestartOllama --> Test9[Test with amanmcp init --force]
    Test9 --> Success9{Works?}
    Other --> Done3([Advanced Debugging])
    Investigate --> Done4([Check Chunk Processing])

    Success1 -->|Yes| Done5([Issue Resolved])
    Success1 -->|No| Escalate1[Try Aggressive Preset]
    Success2 -->|Yes| Done6([Issue Resolved])
    Success2 -->|No| Escalate2[Increase to 500ms]
    Success3 -->|Yes| Done7([Issue Resolved])
    Success3 -->|No| Escalate3[Check physical cooling]
    Success4 -->|Yes| Done8([Issue Resolved])
    Success4 -->|No| Escalate4[Balance needed]
    Success5 -->|Yes| Done9([Issue Resolved])
    Success5 -->|No| Escalate5[Consider batch indexing]
    Success6 -->|Yes| Done10([Issue Resolved])
    Success6 -->|No| Escalate6[Check chunk sizes]
    Success7 -->|Yes| Done11([Issue Resolved])
    Success7 -->|No| Escalate7[Last resort: Aggressive preset]
    Success8 -->|Yes| Done12([Issue Resolved])
    Success8 -->|No| LastResort[Last resort options:<br/>1. Smaller model<br/>2. Batch indexing<br/>3. External cooling<br/>4. Index during cooler times]
    Success9 -->|Yes| Done13([Issue Resolved])
    Success9 -->|No| LastResort

    style Start fill:#e1f5ff
    style Done1 fill:#c8e6c9
    style Done2 fill:#c8e6c9
    style Done3 fill:#c8e6c9
    style Done4 fill:#c8e6c9
    style Done5 fill:#c8e6c9
    style Done6 fill:#c8e6c9
    style Done7 fill:#c8e6c9
    style Done8 fill:#c8e6c9
    style Done9 fill:#c8e6c9
    style Done10 fill:#c8e6c9
    style Done11 fill:#c8e6c9
    style Done12 fill:#c8e6c9
    style Done13 fill:#c8e6c9
    style TimeoutFix1 fill:#fff9c4
    style DelayFix1 fill:#fff9c4
    style DelayFix2 fill:#ffe0b2
    style SlowFix1 fill:#c8e6c9
    style RandomRetry fill:#fff9c4
    style UseAggressive fill:#ffab91
    style UseAggressive2 fill:#ffab91
    style LastResort fill:#ffcdd2
```

### "context deadline exceeded" at 99%

**Problem**: GPU severely throttled at end of long indexing.

**Solution**:

```yaml
embeddings:
  timeout_progression: 2.0
  retry_timeout_multiplier: 1.5
```

### Indexing is too slow

**Problem**: Thermal settings make indexing much slower.

**Solution**: Reduce inter-batch delay:

```yaml
embeddings:
  inter_batch_delay: 50ms  # Minimal cooling pause
```

### Still failing with thermal settings

**Problem**: Even maximum settings don't help.

**Solutions**:

1. Use a smaller embedding model: `model: nomic-embed-text`
2. Index in smaller batches with `--resume`
3. Ensure adequate laptop cooling (not on soft surface)
4. Consider indexing during cooler times of day

---

## Background

### Why GPUs Throttle

Modern GPUs protect themselves by reducing performance when temperature exceeds safe limits (typically 100-107°C for Apple Silicon). This is normal and expected behavior.

### Research

This feature was informed by:

- [Apple M4 Thermal Throttling Analysis](https://hostbor.com/m4-macbook-air-review/)
- [Dynamic Shifting Prevents Thermal Throttling (arXiv)](https://arxiv.org/abs/2206.10849)
- [Ollama Performance Tuning Guide](https://collabnix.com/ollama-performance-tuning-gpu-optimization-techniques-for-production/)
- [llama.cpp Performance Degradation Issue](https://github.com/ggml-org/llama.cpp/issues/832)
