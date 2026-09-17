# Running the whole verification harness on your own machine

This is the operator's page. It assumes nothing about what is installed
and tells you, at each step, what you will and will not have checked.

The one thing to keep hold of while reading it: **a scope nobody checked
reports zero findings and looks exactly like one that is clean.** Every
design decision below exists to keep those two apart, and the output is
deliberately shaped so the last thing on your screen after a long run is
what did *not* happen.

---

## 0. The three numbers

The harness does not produce "a percentage". It produces three, and they
multiply:

```
    CLASS COVERAGE    of the kinds of defect that exist, how many does
                      some instrument know how to look for?
                      python3 tools/verify/taxonomy.py --available

  x CODE COVERAGE     of the functions that exist, how many has some
                      instrument returned a verdict on?
                      python3 tools/verify/matrix.py --matrix ... --report

  x RESOLUTION        of the verdicts returned, how many has somebody
                      read and settled?
                      python3 tools/verify/confidence.py --scope ...
```

Quoting the first one alone is the easiest way to mislead someone with
this harness. An instrument set that sees every class of defect, run
over 5% of the tree, has found 5% of what is there.

---

## 1. What to install

Check what you already have:

```sh
python3 tools/verify/taxonomy.py --available   # the score with what is here
python3 tools/verify/taxonomy.py --missing     # what is absent and what it buys
python3 tools/verify/sweep_all.py --list       # per stage: ok / NOTRUN / missing
```

### FreeBSD

```sh
pkg install cbmc llvm cppcheck coccinelle gcc python3
# Qt and KDE build deps, only needed for the KDE C++ tier:
pkg install qt6-base qt6-declarative kf6-extra-cmake-modules cmake ninja
```

### Debian / Ubuntu

```sh
apt install cbmc clang clang-tidy cppcheck coccinelle gcc python3
# KDE C++ tier:
apt install qt6-base-dev extra-cmake-modules cmake ninja-build
```

### The two that have no package

**ESBMC** — the only route here to a verdict with *no unwind bound at
all* (k-induction). Not in any distribution's package set. Take a release
binary from `github.com/esbmc/esbmc` and put it on `PATH`. Then, before
trusting a single verdict:

```sh
python3 tools/verify/esbmc_driver.py --selftest
```

That probes every flag the driver can emit against the binary's own
`--help` and prints which spellings it could not confirm. The driver has
never met a real ESBMC; `--selftest` is how it does.

**CodeQL** — free for open source, from `github.com/github/codeql-cli-binaries`.
It is the only instrument in the table that does **taint tracking**, and
taint is the one defect class nothing else here covers: a value from an
ioctl, a sysctl, a `copyin` length or a syscall argument reaching an
index, a size or a divisor. `taxonomy.py --available` currently reports
that as the single `GAP`. No driver for it exists in this tree yet.

---

## 2. Profile first, on something small

Never start a tree-wide run without a measured rate, because an estimate
you did not measure is worse than no estimate at all on a job this long.

```sh
python3 tools/verify/sweep_all.py \
    --scope lib/libutil --scope lib/libsysdecode \
    --out ~/pbsd-sweep --jobs $(nproc) --profile
```

That is 54 translation units and writes `~/pbsd-sweep/rates.json`.
Measured here at 8 jobs:

| stage | seconds per translation unit |
|---|---:|
| `classify` | 0.135 |
| `cbmc` | 3.763 |
| `fusebmc` | 0.500 |
| `analyze` | 0.896 |
| `universe` | 11.4 s **fixed**, not per unit |

Then ask what the real thing costs:

```sh
python3 tools/verify/sweep_all.py \
    --scope sys --scope lib --scope bin --scope sbin --scope usr.bin \
    --scope usr.sbin --scope stand --scope contrib --scope crypto \
    --scope cddl --scope secure --scope libexec \
    --out ~/pbsd-sweep --jobs $(nproc) --dry-run
```

22,851 translation units. From the 54-unit profile:

| jobs | estimated total |
|---:|---:|
| 8 | 33.6 h |
| 16 | 16.8 h |
| 32 | 8.4 h |
| 64 | 4.2 h |

Three things about those numbers, all of which the tool prints too:

1. They are a **floor**. Scaling a rate measured at 8 jobs to 32 assumes
   perfect parallelism. The model checkers come close; nothing bounded by
   disk does.
2. They are extrapolated from `lib/libutil` and `lib/libsysdecode`, which
   are scalar-heavy userland. `sys/` has larger functions and will be
   slower per unit. Re-profile on a `sys/` subtree if you want a `sys/`
   estimate.
3. **They exclude every instrument that could not run.** Installing
   ESBMC, cppcheck, Coccinelle and the C++ tier makes the run longer
   *and* the coverage larger, and neither of those is in the total.

---

## 3. The real run

```sh
python3 tools/verify/sweep_all.py \
    --scope sys --scope lib --scope bin --scope sbin --scope usr.bin \
    --scope usr.sbin --scope stand --scope contrib --scope crypto \
    --scope cddl --scope secure --scope libexec \
    --out ~/pbsd-sweep --jobs $(nproc) --timeout 60
```

It is resumable. State lives in `~/pbsd-sweep/state.json`; `--resume`
skips stages already `ok` and passes `--resume` down to the drivers that
support it, so a killed run restarts from the driver's own partial file
rather than from the top:

```sh
python3 tools/verify/sweep_all.py ... --resume
```

Useful narrowings:

```sh
--stage cbmc            # one stage only, repeatable
--skip fusebmc          # everything but, repeatable
--kind cxx              # only the C++ tier
--timeout 300           # longer per function: fewer TIMEOUT, slower
```

### What the end of the run looks like

```
== what ran, and what did not
   scope: lib/libutil lib/libsysdecode   units: 54

  ok
    universe        11.4s
    classify         7.3s
    cbmc           203.2s
    fusebmc         27.0s
    analyze         48.4s

  NOTRUN
    esbmc           esbmc not on PATH  (no package - see taxonomy.py --missing)

  missing
    tidy            tools/verify/clang_tidy_driver.py is not in this tree yet
    ...

  7 of 12 instruments produced NO data for this scope.
  Every function they would have covered is UNTOUCHED, not clean.
```

That block is the point of the whole program. Read it before you read any
finding.

---

## 4. Reading the result

### Which functions nobody looked at

```sh
python3 tools/verify/matrix.py --matrix ~/pbsd-sweep/matrix.jsonl \
        --report --by-scope
```

`UNTOUCHED` splits two ways and they need different work:

- **never attempted** — needs a *run*. An instrument was not pointed at it.
- **attempted, no answer** — needs a *build fixed* or a *bound raised*. A
  `TU-ERROR` means the file did not compile; a `TIMEOUT` means the model
  checker ran out of time. Neither is a statement about the code.

The work queue itself:

```sh
python3 tools/verify/matrix.py --matrix ~/pbsd-sweep/matrix.jsonl \
        --untouched --scope hbsd/sys/kern --reason attempted-no-answer
```

### What the findings mean

```sh
python3 tools/verify/report.py ~/pbsd-sweep/cbmc.jsonl \
        --analyze ~/pbsd-sweep/analyze.jsonl
```

A `FAILED` is a counterexample, which is **not** the same as a defect:
a model checker treats every parameter as unconstrained, so an exported
function whose caller guarantees a range reports a failure that is a
missing precondition rather than a bug. `report.py` buckets them by which
of those it is.

### How much of it is settled

```sh
python3 tools/verify/confidence.py --scope sys/kern
```

---

## 5. The KDE and C++ half

`hbsd/src` holds ~5,779 C++ files and `kde/` holds 1,392 more across
`frameworks`, `kwin` and `plasma-desktop`. Two things to know before
spending time on it:

1. **No bounded model checker here handles it.** CBMC and ESBMC do not
   parse the C++ that KDE is written in. The C++ tier is clang-based
   only, which means it gets `FINDS` and never `PROVES` — its silence is
   not evidence. `taxonomy.py --table` marks every `CXX-` row accordingly.
2. **Without Qt headers nothing compiles**, and a file that does not
   compile produces zero findings. If you run the C++ tier without the Qt
   development packages above, you will get a clean-looking report over a
   tree nothing read. The matrix will say `TU-ERROR` for every function
   in it; the findings list will say nothing at all.

The build-flag source matters and is recorded per translation unit. A
`compile_commands.json` from a real cmake configure is authoritative:

```sh
cmake -S kde/frameworks/kcoreaddons -B /tmp/b \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -GNinja
```

A finding derived from guessed flags is weaker evidence and the record
carries that fact.

---

## 6. What this still does not check

Named here rather than left for you to discover. `taxonomy.py --table`
prints the same list with the reasoning.

| | |
|---|---|
| **data races** | TSan is the instrument and it needs the code to *run* under a workload that exercises the interleaving. For a kernel that means a booted system under load, not a static sweep. |
| **TOCTOU, atomicity** | same reason. Nothing available finds these. |
| **functional correctness** | the largest class there is. CBMC proving a function has no undefined behaviour says nothing about whether it computes the right answer. A driver that returns the wrong value for every input scores a clean sweep. Closing this needs a *specification per function* and this project has none. |
| **lock order inversion** | the kernel's own WITNESS finds these at runtime, so the coverage is exactly the paths a boot exercises. |
| **uninitialised struct padding copied to userspace** | no instrument here models padding. Every instance found in this tree was found by reading. |
| **taint from a trust boundary** | the most important class in the taxonomy and the worst covered. CodeQL answers it; see §1. |
| **code behind an `#ifdef` nobody compiles** | every instrument sees one preprocessor configuration per translation unit. This tree has already found a wrong `-include` order compiling out every ZFS assertion. |

A `CONFIDENCE` of 1.0 does not mean correct. It means the checked
properties hold, for the code the instruments could see, within the
bounds they were given.
