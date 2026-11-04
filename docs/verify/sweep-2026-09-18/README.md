# Sweep 2026-09-18 (WSL)

Whole-tree verify run on this box. Tree: `/home/odin/ParanoidBSD` at `85239aa94`.
Host: Ubuntu in WSL2, 32 jobs, 60 s per function, `--resume`.

**Start here:** [REPORT.md](REPORT.md)

| File | What it is |
|---|---|
| [REPORT.md](REPORT.md) | How many bugs, how well proven, what ran, what did not |
| [GLOSSARY.md](GLOSSARY.md) | `PROVED` is not “correct”; `FAILED` is not “a bug” |
| [TODO.md](TODO.md) | Work that is still to do |
| [numbers.json](numbers.json) | Machine-readable counts from this run |
| [queue/cbmc-read-these.json](queue/cbmc-read-these.json) | 435 CBMC failures a person still has to read |
| [evidence/](evidence/) | Irreplaceable JSONL, packed; digests beside them |

Raw result trees stay on the machine that ran them (`~/pbsd-sweep`, `~/cxx-sweep`).
The tarballs drop regenerable files (`universe.jsonl`, `matrix.jsonl`, `classes.json`).
Rebuild those with the recipe inside each pack (`HOW-TO-REBUILD.json`).
