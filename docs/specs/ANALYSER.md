# SPEC: PBSD Static Analyser

**Status:** Frozen for Wave 1+  
**Modules:** `pbsd.analyser` (Clang plugin)

## Layers

| Layer | Mechanism | Duty |
|-------|-----------|------|
| L1 | clang-tidy + compile flags | Ban raw `new`/`delete` in kernel TUs; prefer span |
| L2 | Clang plugin CFG | Ownership dataflow on closed handle set |
| L3 | `-Wthread-safety*` | Mutex annotations |
| L4 | Plugin + attributes | No sleep/block in ISR-tagged functions (SI-5) |

## L2 checks

- Use-after-move of `UniqueHandle`
- Double-release
- Exclusive borrow while aliased
- Rights widen attempts

## Plugin id

`pbsd-ownership`

## CI mandatory flags (all layers)

Every PBSD CI job (`.github/workflows/pbsd-ci.yml`) MUST enforce the following. No merge on failure.

| Layer | Mandatory flags / inputs | Gate |
|-------|--------------------------|------|
| **L1** | `.clang-tidy` with `WarningsAsErrors: '*'`; kernel TUs compile with `-fno-exceptions -fno-rtti` | `clang-tidy -p build` on `pbsd/` module sources |
| **L2** | Clang plugin `pbsd-ownership` loaded on handle-bearing TUs | Build `pbsd_analyser` when LLVM present; plugin pass on `pbsd/handles`, `pbsd/bifrost`, `pbsd/compositor` |
| **L3** | `-Wthread-safety -Wthread-safety-analysis -Wthread-safety-negative` (global in `pbsd/CMakeLists.txt`) | Compile must be warning-clean under these flags |
| **L4** | `[[pbsd::isr]]` / ISR attribute contract (plugin); SI-5 invariant | Plugin rejects blocking calls in ISR-tagged functions |

### Handle-import compile contract

Any target that `import`s or links `pbsd.handles` MUST propagate `${PBSD_FS_CXX}` (`-fno-exceptions -fno-rtti`) as **PUBLIC** compile options so BMI consumers match the handle module ABI.

### Local verification

```bash
./pbsd/_build_wave.sh
clang-tidy -p build pbsd/handles/pbsd.handles.cppm pbsd/bifrost/pbsd.bifrost.cppm
python3 tools/wave_purge_c_check.py --gate   # Wave 9 owned-C gate (PBSD 0.1)
```
