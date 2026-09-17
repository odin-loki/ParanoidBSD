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
apt install cbmc clang clang-tidy gcc python3
apt install cppcheck coccinelle          # see the warning below
# KDE C++ tier:
apt install qt6-base-dev extra-cmake-modules cmake ninja-build
```

> **If apt refuses cppcheck or coccinelle**, it is almost certainly a
> broken `libz3-dev` pin, and **`apt --fix-broken install` would remove
> `libz3-dev`, which `cbmc` needs.** Do not run it. This works instead:
>
> ```sh
> cd /tmp
> apt-get download cppcheck libtinyxml2-10
> dpkg -i --force-depends ./libtinyxml2-10_*.deb ./cppcheck_*.deb
>
> apt-get download coccinelle libparmap-ocaml ocaml-findlib \
>                  ocaml-base ocaml-base-nox libpcre-ocaml
> dpkg -i --force-depends ./coccinelle_*.deb ./libparmap-ocaml_*.deb \
>         ./ocaml-findlib_*.deb ./ocaml-base_*.deb \
>         ./ocaml-base-nox_*.deb ./libpcre-ocaml_*.deb
> ```
>
> `cppcheck` ends up "unconfigured" over `python3-pygments` and `spatch`
> has unsatisfied OCaml library deps. Both binaries work; `spatch` is
> native and does not need them at runtime.

### The KDE tier needs newer Qt than Debian stable ships

`kdecoration`, `kwayland` and `kwin` declare **`QT_MIN_VERSION 6.10.0`**
and **`KF6_MIN_VERSION 6.22.0`**. If your distribution is older than
that, the C++ tier will report every KDE translation unit as `ERROR` and
tell you so — which is correct, and is not a bug to work around.
FreeBSD ports is the better host for this half.

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

## 1a. The very first command, on a fresh clone

`docs/port_plan.json` is **generated and gitignored** — the markdown is
what is committed. Nothing works without it, so build it first:

```sh
python3 tools/port_plan.py          # ~1 minute, writes docs/port_plan.json
```

Every tool refuses with that exact instruction if you skip it. None of
them degrades into a number: an empty ledger would make every coverage
fraction read 1.00 and every time estimate read 0.00 h.

## 2. Profile first, on something small

Never start a tree-wide run without a measured rate, because an estimate
you did not measure is worse than no estimate at all on a job this long.

```sh
python3 tools/verify/sweep_all.py \
    --scope lib/libutil --scope lib/libsysdecode \
    --out ~/pbsd-sweep --jobs $(nproc) --profile
```

That is 54 translation units and writes `~/pbsd-sweep/rates.json`,
which **takes precedence over the reference profile shipped in
`tools/verify/rates-reference.json`**. That reference exists so a first
run gets a number instead of nothing; every line it produces is marked
`*** SHIPPED REFERENCE, measured on another machine ***`, because an
estimate from the wrong hardware with no provenance is worse than none.

The reference, all eleven stages, 54 units at 8 jobs on a 4-core box:

| stage | seconds per translation unit |
|---|---:|
| `cbmc` | 2.411 |
| `coccinelle` | 0.352 |
| `analyze` | 0.291 |
| `tidy` | 0.246 |
| `fusebmc` | 0.224 |
| `cppcheck` | 0.170 |
| `classify` | 0.146 |
| `warnings` | 0.107 |
| `universe` | 11.9 s **fixed**, not per unit |

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
| 8 | 25.1 h |
| 16 | 12.5 h |
| 32 | 6.3 h |
| 64 | 3.1 h |

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

### The C and C++ halves take different scopes

This trips people and the tool now says so rather than failing:

- the **C** stages take paths relative to `hbsd/src` — `sys/kern`,
  `lib/libc`, `usr.bin`;
- the **C++** stages take those *and* repo-root-relative paths, which is
  how you reach `kde/frameworks/kcoreaddons`.

A stage whose kind is absent from the scope is **`skipped`**, not
`failed` — "there is nothing here of that kind" is a stronger answer
than "its input failed", and a failure list with noise in it is one you
learn to skim. The report counts those separately and says they are not
a coverage gap.

So the two halves are two runs:

```sh
# the C tree
python3 tools/verify/sweep_all.py --scope sys --scope lib ... --out ~/c-sweep

# the C++ tree
python3 tools/verify/sweep_all.py --scope kde --out ~/cxx-sweep --kind cxx
```

The port ledger names **no `kde/` path at all** (0 records of 35,050),
so a KDE scope's unit count comes from the filesystem instead, and the
tool prints which source it used. A disk count and a ledger count do not
mean the same thing.

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

### What only a person or a model can decide

Some work is not waiting on a better checker. A counterexample an engine
produced but cannot judge — because judging it means reading the
callers, the comments and the intent — is a *reading* job, and it is now
a named disposition rather than a residue.

```sh
python3 tools/verify/taxonomy.py --available --needs-model   # which CLASSES
python3 tools/verify/matrix.py --matrix ... --needs-model    # which FUNCTIONS
python3 tools/verify/report.py  ...                          # which FINDINGS
```

On the last full run that was **463 findings** an engine produced and
could not decide. The class list is seven in scope — taint from a trust
boundary, a contract enforced nowhere, lock order, the padding leak, and
three more.

Three rules the tooling enforces, and you should hold to as well:

1. **Every model finding is a hypothesis until it is checked against the
   source.** During this harness's own construction a model reported
   that adding `core.BitwiseShift` to the analyser would cover the
   `1 << 31` class. Half of that was true. A three-line test showed the
   other half was not — the checker reports zero on 151 units, and only
   cppcheck sees `1 << 31`.
2. **A model saying nothing about a function means nothing.** Its silence
   is not evidence, which is why a read is recorded as `REVIEWED` beside
   the shallow scans and never beside a proof.
3. **`never-attempted` is not a model's job — it is a run's.** Point an
   engine at it first. The expensive instrument goes last, and where the
   proof is not.

---

## 4a. Bringing the results back

A whole-tree run leaves about **92 MB**, and roughly 90 of that is
regenerable in fifteen seconds — `universe.jsonl` and `matrix.jsonl` are
rebuilt from the port ledger and the stage results, which is what
`matrix.py` is *for*. What cannot be regenerated is the evidence: the
per-stage JSONL each instrument produced, and `state.json`, which says
what ran and what did not.

```sh
python3 tools/verify/pack_results.py ~/pbsd-sweep \
        --pack ~/sweep.tar.gz --digest ~/sweep-digest.json
```

Measured on a real run: **92 MB → 0.9 MB packed, 49 KB digest.**

| | | |
|---|---:|---|
| `--pack` | ~1 MB | everything irreplaceable. Attach it, or push it to a branch. The rebuild commands travel inside as `HOW-TO-REBUILD.json`. |
| `--digest` | ~50 KB | paste-able. Counts, the stages that produced nothing, the untriaged failures, the reading queue. |

The digest carries the **absences first**, deliberately. A summary that
lists findings and omits the four instruments that never ran is the same
lie in a smaller file — and the smaller file is the one people read.
It also says of itself that it is a summary and cannot be re-analysed;
only the pack can be fed back to `matrix.py`.

Verified round-trip: unpacking the 0.9 MB and re-running `matrix.py`
reproduces the original matrix exactly — 159,885 TOUCHED of 335,853,
the same numbers to the row.

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
