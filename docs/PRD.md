# KickBass Assistant PRD

## Positioning

KickBass Assistant is a single-purpose low-end workflow plugin:

- Host track: `bass`
- Sidechain source: `kick`
- Goal: see and fix low-frequency conflict without chaining multiple utilities

## First-release scope

Included in `v0.1`:

- Low-band conflict meter
- Mono compatibility meter
- Kick-to-bass low-end correlation meter
- Low-band polarity inversion
- Bass timing offset with fixed plugin latency
- Split-band ducking driven by kick sidechain
- Real-time alignment and polarity hints
- macOS `VST3` build target

Explicitly excluded from `v0.1`:

- Multi-instance linking
- Machine learning inference
- Automatic phase rotation / all-pass alignment
- Bus-wide mix assistant modes
- Dynamic fundamental tracking
- Preset browser and onboarding system

## User promise

The plugin should not claim to "auto-fix" low end. It should:

- reveal overlap quickly
- provide a solid starting point
- make manual adjustment faster and more repeatable

## Core controls

- `Listen`: Bass / Kick / Both / Mono / Delta
- `Polarity`: low-band inversion
- `Offset`: bass timing shift in milliseconds
- `Crossover`: split point for low-band processing
- `Duck Amount`: strength of low-band ducking
- `Attack`: detector rise time
- `Release`: detector release time
- `Output Trim`: final gain stage

## DSP notes

- Fixed-latency offset model so positive and negative alignment moves are both possible
- Low-band detector built from sidechain low-pass energy
- Crossover split and recombine performed in real time
- Heuristic suggestion layer based on low-band timing correlation and signed polarity correlation
