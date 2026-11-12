<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Notebook

By Odin Loch

| | |
|---|---|
| ID | `AI04` (from CATALOGUE.md) |
| Bundle | `org.pbsd.notebook` |
| Category | ai |
| Priority | P2 |
| Backing repos | MathScript (numerical and symbolic engine, REPL interpreter), FORGE (ForgeScript engine, model service), FUSE (UI, `chart`, `viewportWidget`) |
| Status | Planned. Nothing built. |

## 1. Purpose

A computational notebook: a document of text, maths and code cells whose results (numbers, tables, formulae, plots) sit under the code that made them. Students, engineers, scientists and analysts use it to calculate, explore data and write up results. Code runs in MathScript or ForgeScript kernels, each in its own sandboxed process with the capabilities that one notebook was granted and no network by default. It replaces Jupyter-style tools and MathScript's optional Qt 6 IDE (`MATHSCRIPT:src/gui/`).

## 2. Scope

In scope:

- Cells: code (MathScript, ForgeScript), Markdown, maths, raw.
- Kernels as sandboxed processes; per-notebook capability sheet.
- Rich outputs: text, typeset maths, tables, 2D and 3D plots, images, interactive widgets.
- Numeric and symbolic maths through MathScript; units.
- Data import, variable explorer, plotting library.
- Reproducibility: pinned environment, deterministic re-run, hashes of inputs.
- An open, text-based notebook file format; Jupyter `.ipynb` import and export.
- Export to PDF, HTML, Markdown; diffs that work in version control.
- GPU compute by capability; local models through `svc.model`.
- Publishing a read-only signed notebook; teaching mode.

Out of scope:

| Item | Owner |
|---|---|
| Quick one-line calculation, unit conversion | OF07 Calculator |
| Spreadsheet grids, pivot tables | OF05 Sheets |
| Database connections and SQL workbench | OF09 Data Studio |
| Time-series forecasting studio | AI03 Forecast |
| General chat assistant | AI01 Assistant |
| Model import, signing, benchmarking | AI02 Model Manager |
| ForgeScript and C++ project development, debugging | DV01 IDE |
| Git history and merge UI | DV03 Version Control |
| Rich note-taking without code | OF02 Notes |
| Python, R or Julia kernels | none planned; foreign-language cells are kept as text on import |

## 3. Users and top tasks

Personas:

- **Student**: works through exercises; needs typeset maths and plots without set-up.
- **Engineer**: calculations with units that must be re-checkable later.
- **Scientist or analyst**: loads data files, fits, plots, exports a report.
- **Teacher**: writes lessons with locked text, exercises and hidden solutions.
- **Reviewer**: opens a signed, published notebook and checks that it re-runs to the same result.
- **Developer**: prototypes ForgeScript logic with instant feedback.

| # | Task | Target steps |
|---|---|---|
| T1 | New notebook, type an expression, see the result | 2 (`Ctrl+N`, type + `Shift+Enter`) |
| T2 | Plot a function | 1 cell |
| T3 | Differentiate an expression and see typeset output | 1 cell |
| T4 | Import a CSV and see it as a table | 3 |
| T5 | Change a parameter with a slider and watch the plot update | 2 |
| T6 | Re-run the whole notebook from a clean kernel | 1 (`Ctrl+Shift+F5`) |
| T7 | See which cells are stale after an edit | 0 (always marked) |
| T8 | Inspect a matrix variable | 1 click in the variable explorer |
| T9 | Export to PDF | 2 |
| T10 | Open an `.ipynb` and keep text, maths and outputs | 1 (open) |
| T11 | Review a change to a notebook in version control | plain text diff, no tool needed |
| T12 | Publish a read-only signed copy | 3 |
| T13 | Verify a received notebook and re-run it | 2 |
| T14 | Grant a notebook access to one data folder | 2 (prompt, pick) |
| T15 | Run a lesson in teaching mode and check an exercise | 1 per exercise |

## 4. Features

Priority: M must, S should, C could. Phase 1 usable core, 2 full feature, 3 advanced.

### 4.1 Document and cells
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI04-F001 | Cell types | `code` (language `mathscript` or `forgescript`), `markdown`, `math` (display formula), `raw` (kept verbatim, not run). | M | 1 |
| AI04-F002 | Add, delete, move, split, merge, duplicate | Command mode keys `A`, `B`, `D D`, `Alt+Up/Down`, `Ctrl+Shift+-`, `Shift+M`. Deleted cells go to a per-notebook bin with undo. | M | 1 |
| AI04-F003 | Change cell type | `Y` code, `M` Markdown, `E` maths, `R` raw. | M | 1 |
| AI04-F004 | Two modes | Command mode (cell selected) and edit mode (caret in cell). `Enter` and `Esc` switch. | M | 1 |
| AI04-F005 | Multi-cell selection | Shift and Ctrl selection; act on all: run, move, delete, collapse, convert, tag. | M | 1 |
| AI04-F006 | Collapse | Collapse input, output, or a whole section under a heading. | M | 1 |
| AI04-F007 | Sections and outline | Headings in Markdown cells form an outline with numbering. | M | 1 |
| AI04-F008 | Cell ids | Stable 8-character ids; never reused; survive reorder. | M | 1 |
| AI04-F009 | Cell tags and attributes | `hide-input`, `hide-output`, `locked`, `skip`, `slow`, `exercise`, `solution`, `parameters`; free tags. | S | 2 |
| AI04-F010 | Undo and redo | Structural and text changes in one history per notebook; survives restart. | M | 1 |
| AI04-F011 | Find and replace | Across cells, with regex, limited to inputs or including outputs. | M | 1 |
| AI04-F012 | Tabs and split view | Several notebooks; two views of one notebook side by side. | S | 2 |
| AI04-F013 | Templates | Blank, lab report, lecture, data analysis, engineering calculation sheet. | C | 2 |
| AI04-F014 | Cross-references | Label an equation, figure or table; `@eq:name` in Markdown resolves to its number. | S | 3 |
| AI04-F015 | Bibliography | BibTeX or CSL-JSON file attached; `[@key]` citations; one built-in numeric style and one author-year style. | C | 3 |
| AI04-F016 | Table of contents cell | Generated list of sections. | C | 2 |
| AI04-F017 | Line numbers, soft wrap, font zoom | Per notebook. | M | 1 |
| AI04-F018 | Spell check in Markdown cells | Through the system service (G-17). | S | 2 |

### 4.2 Code editing
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI04-F019 | Syntax highlighting | MathScript and ForgeScript grammars; Markdown; LaTeX subset inside maths. | M | 1 |
| AI04-F020 | Completion | Function names from the kernel's catalogue, variables in the session, file names in granted folders, ForgeScript members. `Tab` or `Ctrl+Space`. | M | 1 |
| AI04-F021 | Signature help and docs | Argument list and the entry from MathScript's API reference (`MATHSCRIPT:docs/API.md`) or ForgeScript docs. `Shift+Tab`. Offline. | M | 2 |
| AI04-F022 | Inline errors | Kernel error message under the cell with the line marked. MathScript returns `Result` errors as text (for example `mat_at: row index 2 is out of range (matrix has 2)`); shown verbatim. | M | 1 |
| AI04-F023 | MathScript multi-line cells | MathScript is one command per line with no block syntax (`MATHSCRIPT:docs/USER_GUIDE.md`). A cell is sent line by line; `#` comments and blank lines are skipped, as `run_file` does. | M | 1 |
| AI04-F024 | Stop on first error | Lines after a failing line in a cell do not run; the cell is marked failed at that line. | M | 1 |
| AI04-F025 | Bracket matching, auto-indent, comment toggle | `Ctrl+/`. | M | 1 |
| AI04-F026 | Matrix literal helper | Insert an n×m grid editor that writes `[a, b; c, d]`. | S | 2 |
| AI04-F027 | Symbol palette | Greek letters, operators, common functions; inserts text. | S | 2 |
| AI04-F028 | Snippets | Per language; user-defined. | S | 2 |
| AI04-F029 | Formatter | ForgeScript formatter (FORGE-WP-37); MathScript spacing normaliser. | C | 3 |
| AI04-F030 | Magic-free | No hidden line prefixes that change meaning. Notebook actions (import data, set capability, plot options) are UI actions or explicit library calls recorded in the file. | M | 1 |

### 4.3 Kernels and execution
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI04-F031 | One process per kernel | Each notebook language gets its own kernel helper process. A notebook with both languages has two. | M | 1 |
| AI04-F032 | MathScript kernel | Hosts `ms::interp::Interpreter` (`MATHSCRIPT:include/ms/interp/repl_engine.hpp`); `execute(line)` per line; interpreter path only. The optional LLVM ORC JIT (`MS_BUILD_JIT`) is not built: the OS forbids JIT. | M | 1 |
| AI04-F033 | ForgeScript kernel | Hosts `forge::script::engine` with an FSS policy generated from the notebook's capability sheet. Bytecode VM only. | M | 1 |
| AI04-F034 | Run controls | Run cell `Ctrl+Enter`; run and advance `Shift+Enter`; run and insert `Alt+Enter`; run all, above, below, selected, stale. | M | 1 |
| AI04-F035 | Interrupt | Cooperative stop request, then process kill after a grace period (default 3 s). State after a kill is "kernel restarted"; variables are gone and the UI says so. | M | 1 |
| AI04-F036 | Restart, restart and run all | Fresh process; execution counters reset. | M | 1 |
| AI04-F037 | Execution queue | Cells run in order per kernel; queued cells show their position. | M | 1 |
| AI04-F038 | Execution record | Per cell: counter, start time, duration, kernel session id, hash of the input text. | M | 1 |
| AI04-F039 | Stale tracking | A cell is stale when its text changed since it ran, or when a cell above it that assigns a name it reads has run since. Names are found by a static scan (assignments `x =`, `L, U =`, handle-creating calls; ForgeScript declarations). | M | 2 |
| AI04-F040 | Out-of-order warning | If cells ran in a different order than the document, a banner offers "Restart and run all". | M | 1 |
| AI04-F041 | Resource limits per kernel | Memory, CPU time per cell, output size, thread count; set in the capability sheet. | M | 1 |
| AI04-F042 | Output streaming | Text output appears while the cell runs; long outputs are truncated with "show all". | M | 1 |
| AI04-F043 | Cross-language values | Pass a scalar, matrix, string or table between kernels through an explicit `share("name")` / `take("name")` pair; copied as typed data, not as code. | S | 2 |
| AI04-F044 | Kernel status | Idle, busy, starting, dead; memory in use; session id; in the status bar. | M | 1 |
| AI04-F045 | Session save | MathScript session files (`save`/`load`, text format) can be written to the notebook's data folder; listed as attachments. | C | 2 |
| AI04-F046 | Background run | A long run continues when the window closes (`bg.run`); a notification reports completion. | S | 2 |
| AI04-F047 | Run on a cluster node | MathScript has an MPI compute node (`mathscript-server`). Not used in Phase 1–2; a remote kernel is an open question. | C | 3 |
| AI04-F048 | Timing | `%`-free timing: a toolbar toggle records wall time per line and shows a small bar. | S | 2 |
| AI04-F049 | Parameters cell | A cell tagged `parameters` defines inputs; "Run with parameters…" overrides them for one run without editing the file. | C | 3 |

### 4.4 Outputs
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI04-F050 | Typed outputs | Each output has a kind: `text`, `math`, `table`, `plot2d`, `plot3d`, `image`, `widget`, `error`, `file`. No output is executable. | M | 1 |
| AI04-F051 | Typeset maths | Symbolic results and maths cells render as formulae. Needs a maths layout engine (section 5.7, `AI04-GAP-1`). | M | 1 |
| AI04-F052 | Maths source forms | The same result as LaTeX, Presentation MathML, Unicode text or code, from MathScript's `sym_export` formats (`latex`, `mathml`, `content-mathml`, `unicode`, `ascii`, `sympy`, `mathematica`, `c`, `c++`, `python`). Copy as any of them. | M | 2 |
| AI04-F053 | Tables | Matrix and data-table outputs in a virtualised grid: sort, column width, number format, copy as CSV/TSV/Markdown, open in OF05. | M | 1 |
| AI04-F054 | Large value summarising | Above a size threshold, show shape, type, head and tail rows, min/max; full view on demand. | M | 1 |
| AI04-F055 | 2D plots | Line, scatter, bar, histogram, area, step, error bars, heat map, contour, sparsity (`spy`), box, polar. MathScript today gives Line, Bar, Scatter, Heatmap, Spy, Surface3D series (`PlotSeries::Kind`). | M | 1 |
| AI04-F056 | 3D plots | Surface, wireframe, 3D scatter, 3D line, in `viewportWidget`: orbit, pan, zoom, reset. | S | 2 |
| AI04-F057 | Plot interaction | Zoom box, pan, reset, crosshair read-out, legend toggle, pick a point to see its value. | M | 2 |
| AI04-F058 | Plot export | PNG, SVG, PDF; data as CSV. | M | 2 |
| AI04-F059 | Images | PNG and JPEG from granted files or produced by code (`imshow`); decoded in the image helper, not in the app host. | S | 2 |
| AI04-F060 | Widgets | Slider, range slider, number box, dropdown, checkbox, toggle, text box, button, colour, date. Bound to a variable; a change re-runs the cells that read it. | S | 2 |
| AI04-F061 | Widget throttle | Re-run after the pointer rests (default 80 ms) or on release for slow cells. | S | 2 |
| AI04-F062 | Animations | A plot with a frame parameter and a play control; frames computed by re-running the cell. | C | 3 |
| AI04-F063 | Output actions | Copy, save as file, clear, pin (keep visible while scrolling), collapse, "open in new window". | M | 1 |
| AI04-F064 | Error output | Message, line, kernel, and a "what ran" trace. No stack of host internals. | M | 1 |
| AI04-F065 | Stored outputs | Outputs are saved in a sidecar so a reader sees results without running. Each stored output records the input hash it came from. | M | 1 |
| AI04-F066 | Output freshness mark | A stored output whose input hash differs from the cell text is shown with "from an earlier version". | M | 1 |
| AI04-F067 | No active content | No HTML, script or remote resource is ever rendered from an output. | M | 1 |

### 4.5 Mathematics
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI04-F068 | Numeric library | Everything the MathScript REPL binds: linear algebra, FFT, statistics, ODE/PDE/FEM/CFD, optimisation, special functions, signal and image, number theory, graphs, geometry, control, finance and more (module list in `MATHSCRIPT:README.md`). | M | 1 |
| AI04-F069 | Symbolic commands | `sym_diff`, `sym_integrate`, `sym_simplify`, `sym_expand`, `sym_collect`, `sym_substitute`, `sym_limit`, `sym_series`, `sym_solve_linear`, `sym_dsolve`, Laplace, Fourier, Mellin, Hankel and Z transforms with inverses (`MATHSCRIPT:docs/API.md`). Expressions are passed as quoted strings today. | M | 1 |
| AI04-F070 | Maths cell input | A maths cell accepts the strict LaTeX subset MathScript parses (`MATHSCRIPT:docs/LATEX_SUBSET.md`); anything outside is rejected with a position. It can be display-only or bound to a name for use in code. | S | 2 |
| AI04-F071 | Formula editor | Structured editing of fractions, roots, sub- and superscripts, matrices with keyboard templates; writes the LaTeX subset. | C | 3 |
| AI04-F072 | Units | Quantities with SI units, conversion and dimension checks in results and tables. MathScript has a compile-time unit system (`core/units.hpp`) and a `Scalar` with units (`core/scalar.hpp`); REPL-level unit syntax was not found in the user guide, so run-time units in the kernel are Proposed. | S | 3 |
| AI04-F073 | Exact and big numbers | MathScript has `bignum` and `numthy` modules; show exact integers and rationals without rounding when the kernel returns them. | S | 2 |
| AI04-F074 | Number format | Significant digits, fixed, scientific, engineering; per notebook and per output. The REPL prints six decimals by default; the kernel protocol carries full `f64`. | M | 1 |
| AI04-F075 | Complex numbers and matrices | Typeset with `i`/`j` choice; matrix brackets style. | S | 2 |
| AI04-F076 | Assumptions shown | When a symbolic routine reports a condition or refuses (for example a negative base under a fractional power), the message is shown as written. | M | 1 |
| AI04-F077 | Function catalogue browser | Searchable list of bound functions with argument forms and examples from the API reference. | S | 2 |
| AI04-F078 | Seeded randomness | MathScript's numerical RNG has a fixed default seed and optional seed arguments; the seed in force is recorded per run. Cryptographic randomness is never reproducible and is flagged. | M | 2 |

### 4.6 Data
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI04-F079 | Capability sheet | Per notebook: folders (read or read/write), network endpoints, GPU, models, limits. Empty by default. Stored in the file; grants are asked from the user through the shell on first use on this machine. | M | 1 |
| AI04-F080 | Data import wizard | CSV/TSV (delimiter, header, types, encoding, decimal mark, date format, NA values), fixed width, JSON arrays, plain matrix text, MathScript session files. Preview of the first 200 rows. Writes an explicit import call into a cell. | M | 2 |
| AI04-F081 | Binary formats | NumPy `.npy`, Matrix Market, WAV, PNG as matrix; Parquet and HDF5 are Phase 3 and need a reader (own subset or a vetted library). | S | 3 |
| AI04-F082 | Attachments | Small data files stored beside the notebook in its data folder; listed with size and BLAKE3 hash. | M | 2 |
| AI04-F083 | Input hashing | Every file a run read is recorded with path-in-grant, size and BLAKE3. A later run flags changed inputs. | M | 2 |
| AI04-F084 | Data table type | A typed column table (number, text, date, boolean, category) in the ForgeScript kernel; conversion to MathScript matrices for numeric columns. | S | 2 |
| AI04-F085 | Table operations | Filter, sort, select, group and aggregate, join, pivot, describe; as library calls and as a point-and-click builder that writes the calls. | S | 3 |
| AI04-F086 | Export data | Variable to CSV, JSON, `.npy`, Matrix Market through the save portal. | M | 2 |
| AI04-F087 | Network data | Off by default. With a per-notebook grant to named hosts, a ForgeScript cell can fetch; each fetch is logged with URL, time, size and hash, and the response is stored as an input so re-runs need no network. | C | 3 |
| AI04-F088 | Hand-off | Open a table in OF05 Sheets or OF09 Data Studio; receive a table from them. | S | 2 |

### 4.7 Variable explorer
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI04-F089 | Variable list | Name, kind (scalar, matrix, table, string, handle, function), shape, size in memory, preview; per kernel. From MathScript's `vars` and `session_objects()` and the ForgeScript scope. | M | 1 |
| AI04-F090 | Matrix viewer | Grid with heat-map tint option, go to row/column, statistics row. | M | 2 |
| AI04-F091 | Quick plot | Plot a variable from the explorer without writing code; "insert as cell" writes the call. | S | 2 |
| AI04-F092 | Changed-since marks | Variables changed by the last run are marked. | S | 2 |
| AI04-F093 | Delete variable | Writes and runs a visible command; never a silent state change. | S | 2 |
| AI04-F094 | Session objects | MathScript handles (`bloom_new`, tensor decompositions and so on) listed with type. | S | 2 |
| AI04-F095 | Defined-in link | Jump to the cell that last assigned a variable. | S | 2 |

### 4.8 Plotting library
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI04-F096 | Plot specification | A plot is data plus a declarative spec (marks, axes, scales, legend, title, size, theme). The kernel emits the spec; the app renders it. Specs are stored, so plots re-render without re-running. | M | 1 |
| AI04-F097 | MathScript plot commands | `plot`, `scatter`, `hist`, `imshow`, `spy`, `surf` map to specs. The console REPL prints an ASCII preview; the kernel returns the series instead. | M | 1 |
| AI04-F098 | Axes and scales | Linear, log, symlog, date; limits, ticks, grid, secondary axis; equal aspect. | M | 2 |
| AI04-F099 | Multiple series and subplots | Overlay, grid of panels, shared axes. | M | 2 |
| AI04-F100 | Styling | Colour cycles from the design tokens, colour-blind-safe default palette, line styles and markers so series differ without colour. | M | 2 |
| AI04-F101 | Annotations | Text, arrows, reference lines, shaded ranges. | S | 2 |
| AI04-F102 | Colour maps | Perceptually uniform sequential and diverging maps; colour bar. | S | 2 |
| AI04-F103 | Function plotting | `fplot("sin(x)/x", a, b)` with adaptive sampling; implicit and parametric curves. | S | 2 |
| AI04-F104 | Statistical plots | Histogram with density, box, violin, QQ, error bars, regression line with band. | S | 3 |
| AI04-F105 | Vector fields and contours | Quiver, stream lines, filled contour. | C | 3 |
| AI04-F106 | 3D rendering | Lit surfaces with colour map, axes box, in `viewportWidget`; CPU fallback when no GPU. | S | 2 |
| AI04-F107 | Plot data table | Every plot exposes its data as a table for accessibility and export. | M | 2 |
| AI04-F108 | Vector export | SVG and PDF from the spec with text as text. | M | 2 |

### 4.9 Reproducibility
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI04-F109 | Environment pin | The lock file records kernel versions and commits, ISA path, thread count and seed. MathScript has `ms::runtime::capture()` returning version, commit, ISA path and seed (`MATHSCRIPT:src/runtime/repro.cpp`); the kernel reports it at start. | M | 2 |
| AI04-F110 | Deterministic re-run | "Verify" restarts kernels, runs all cells in order with the pinned thread count and seed, and compares each output hash with the stored one. | M | 2 |
| AI04-F111 | Honest comparison | MathScript documents that results can differ in the last bits across ISA paths and thread counts. Verify reports "identical", "numerically equal within tolerance t" or "different", and states the environment difference when there is one. | M | 2 |
| AI04-F112 | Run manifest | Per run: cell order, input hashes, data file hashes, environment, output hashes, duration. Stored in the lock file. | M | 2 |
| AI04-F113 | Reproducibility badge | Header shows: verified on this machine (date), not verified, or failed. | S | 2 |
| AI04-F114 | Time and randomness sources | Wall-clock reads and unseeded randomness in a cell are flagged as non-reproducible. | S | 3 |
| AI04-F115 | Pinned kernel versions | Several kernel versions can be installed; a notebook runs with the version it pins or says which is missing. | C | 3 |
| AI04-F116 | Checkpoints | Named checkpoints of the notebook (source, outputs, lock); restore or compare. ZFS snapshots give a second layer. | S | 2 |

### 4.10 File format, import, export, version control
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI04-F117 | Native format | `.pnb`: UTF-8 text, one cell after another with delimiter lines; defined in section 6.2. Outputs and lock live in sidecars, so the source file changes only when the author's text changes. | M | 1 |
| AI04-F118 | Packed form | `.pnbz`: one archive holding source, outputs, lock, attachments and optional signature; for sending and publishing. | M | 2 |
| AI04-F119 | Import `.ipynb` | Reads nbformat 4 JSON: Markdown and raw cells kept; code cells kept as text in a foreign-language cell (not runnable unless the language is MathScript or ForgeScript); outputs `text/plain`, `image/png`, `image/jpeg`, `text/latex`, `text/markdown` kept; `text/html`, JavaScript and widget state dropped and listed in an import report. | M | 2 |
| AI04-F120 | Export `.ipynb` | Writes nbformat 4 with kernel name `mathscript` or `forgescript`, outputs as `text/plain`, `text/latex`, `image/png`. A round-trip report lists what the target cannot hold (widgets, capability sheet, lock). | S | 2 |
| AI04-F121 | Export PDF | Paged layout with title block, numbered sections, equations, figures as vectors, code with wrapping, page numbers; through the print pipeline (G-16). | M | 2 |
| AI04-F122 | Export HTML | One static file: no script, no remote resources; maths as MathML with an image fallback; plots as inline SVG. | M | 2 |
| AI04-F123 | Export Markdown | Markdown with fenced code, LaTeX maths, figures as files in a folder. | M | 2 |
| AI04-F124 | Export script | All code cells of one language as a `.ms` or `.fs` file with cell markers as comments. | S | 2 |
| AI04-F125 | Export options | Include or hide inputs, outputs, cells by tag; paper size; theme. | S | 2 |
| AI04-F126 | Text diff friendly | Stable cell order, stable ids, one attribute per line in headers, no timestamps or counters in the source file, trailing newline, LF only. | M | 1 |
| AI04-F127 | Notebook diff view | Cell-aware diff of two versions: added, removed, moved, changed cells; text diff inside cells; output changes summarised (and plots side by side). | S | 2 |
| AI04-F128 | Merge helper | Three-way merge by cell id; conflicts shown per cell. Offered to DV03 as a merge tool for `.pnb`. | C | 3 |
| AI04-F129 | Strip outputs | One action removes the sidecars; the source is untouched. | M | 1 |
| AI04-F130 | Print | Same layout as PDF. | S | 2 |

### 4.11 GPU and local models
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI04-F131 | GPU compute by capability | A notebook that asks for GPU gets it only after a `dev.gpu.compute` grant; an indicator shows while a kernel holds the device. | S | 3 |
| AI04-F132 | GPU backend | MathScript's GPU backend today is CUDA (`MS_ENABLE_CUDA`, optional). A portable compute path for the OS is Proposed (`AI04-GAP-4`); until it exists GPU cells fall back to CPU and say so. | S | 3 |
| AI04-F133 | Device and memory read-out | Device name, memory used by the kernel, fallback reason. | S | 3 |
| AI04-F134 | Explain this cell | Local model explains code or an error; shown in a side panel marked as model output. | S | 2 |
| AI04-F135 | Draft a cell from a description | The draft is inserted as an unrun cell for the user to read and run. A model never runs code. | S | 2 |
| AI04-F136 | Model calls from code | ForgeScript cells call local models (generate, embed) through `svc.model` when the capability sheet lists `ai.local`; model name and hash go into the run manifest. Needs G-15. | C | 3 |
| AI04-F137 | Model output is not reproducible by default | Cells that call a model are flagged; a fixed seed and temperature 0 are recorded when set. | S | 3 |
| AI04-F138 | No cloud | No remote model endpoint exists in the app. | M | 1 |

### 4.12 Publishing and teaching
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI04-F139 | Publish read-only | Produces a `.pnbz` with all cells locked, outputs included, lock file included and a detached Ed25519 signature over a BLAKE3 tree hash, using a key from `svc.keyring`. | M | 2 |
| AI04-F140 | Verify on open | A signed notebook shows signer key fingerprint and state: verified, unknown signer, or tampered. Tampered notebooks open as plain text only. | M | 2 |
| AI04-F141 | Reader mode | Published notebooks open with no kernel. "Re-run in a sandbox" starts kernels with an empty capability sheet and asks for each grant the notebook lists. | M | 2 |
| AI04-F142 | Fork a published notebook | Makes an editable copy; the signature does not carry over; provenance records the original hash. | S | 2 |
| AI04-F143 | Teaching mode | Locked text cells, exercise cells the learner edits, hidden solution cells, hints revealed in steps. | S | 3 |
| AI04-F144 | Check cells | A teacher's check cell tests the learner's variables and returns pass or fail with a message; runs in the same sandbox. | S | 3 |
| AI04-F145 | Progress | Per-lesson progress stored locally in the learner's copy; nothing is sent anywhere. | S | 3 |
| AI04-F146 | Lesson pack | A folder or `.pnbz` set with an index page; export learner answers as a file to hand in. | C | 3 |
| AI04-F147 | Presentation mode | Full screen, one section per slide, large type, run cells live. | C | 3 |
| AI04-F148 | Solution stripping | Export a learner copy with solution cells removed. | S | 3 |

### 4.13 Settings, security, accessibility, recovery
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI04-F149 | Untrusted notebook rule | Opening a notebook never runs anything. Stored outputs are shown; code runs only on a user action. | M | 1 |
| AI04-F150 | Capability review on first run | Before the first run of a notebook from elsewhere, a sheet lists what it asks for. | M | 1 |
| AI04-F151 | Autosave and crash recovery | Source saved atomically on idle; a kernel crash loses variables only; the app host crash loses nothing saved. | M | 1 |
| AI04-F152 | Session restore | Open notebooks, scroll positions, collapsed state. Kernels are not restored; outputs are. | M | 1 |
| AI04-F153 | Keyboard complete | Both modes fully keyboard driven; shortcut sheet `Ctrl+/` in command mode. | M | 1 |
| AI04-F154 | Screen-reader maths | Formulae expose a spoken form and navigable structure from MathML. | S | 2 |
| AI04-F155 | Audit entries | Capability grants, network fetches, GPU use, publish and signature failures go to `svc.audit`. | M | 2 |
| AI04-F156 | Search integration | Titles and Markdown text of the user's notebooks go to `svc.search`; opt-out per notebook. | C | 2 |

## 5. GUI

Layout pattern: document canvas with sidebar and inspector. Default density: normal.

### 5.1 Window map

```
Notebook
├─ Start page (recent notebooks, new from template, open, import .ipynb)
├─ Notebook window
│  ├─ Header bar: menu, title, search, kernel chips, capability badges, signature badge
│  ├─ Toolbar: run controls, cell type, insert, kernel menu, verify, export
│  ├─ Sidebar (F9): Outline · Files and attachments · Function catalogue · Checkpoints
│  ├─ Cell canvas (centre): cells with input, output and gutter
│  ├─ Inspector (F4): Variables · Cell properties · Plot properties · Capability sheet · Run manifest
│  ├─ Bottom panel (Ctrl+J): Kernel log · Problems · Assistant
│  └─ Status bar: mode, kernel state, memory, stale count, reproducibility state
├─ Detached windows: plot window · table/matrix viewer · presentation
├─ Dialogs
│  ├─ New notebook (template, languages)
│  ├─ Capability sheet (review and grant)
│  ├─ Data import wizard
│  ├─ Insert widget · Insert matrix · Symbol palette
│  ├─ Verify (re-run and compare) report
│  ├─ Export (PDF, HTML, Markdown, .ipynb, script) options
│  ├─ Publish and sign
│  ├─ Import report (.ipynb)
│  ├─ Compare versions (notebook diff)
│  └─ Find and replace
├─ Popovers: completion, signature help, output actions, freshness note, signer details
└─ Preferences
```

### 5.2 Main window

Shown at 1280×820, edit mode in a MathScript cell.

```
+------------------------------------------------------------------------------------------+
| [=] Notebook — beam-deflection.pnb       [search Ctrl+K]   [MS ● idle] [FS ○]  [▁][▢][✕] | (1)
+------------------------------------------------------------------------------------------+
| [▶ Run] [▶▶ All] [■ Stop] [↻ Restart]  [Code: MathScript v] [+ Cell v]  [Verify] [Export]| (2)
+-------------+--------------------------------------------------------------+-------------+
| OUTLINE     |  # Beam deflection                                      (3)  | VARIABLES   | (8)
| 1 Beam defl.|  A simply supported beam with a point load at midspan.       | MS kernel   |
|  1.1 Inputs |                                                              | E   2.1e11  |
|  1.2 Model  | [3] ┃ E = 2.1e11                                         (4) | I   8.3e-6  |
|  1.3 Plot   |     ┃ I = 8.3e-6                                             | L   4       |
|             |     ┃ L = 4                                                  | P   12000 ● |
| FILES       |     ┃ P = 12000                    MathScript · 2 ms · ✓     | x   1x101   |
|  loads.csv  |                                                              | w   1x101 ● |
|  9f2c… 4 KB | [4] ┃ sym_diff("P*x*(3*L^2-4*x^2)/(48*E*I)", "x")        (5) |-------------|
|             |     ┃                                                        | CELL        |
| CATALOGUE   |       P (3 L² − 12 x²)                                       | id  c7f3a1b2|
|  [search__] |       ─────────────────                                      | lang MS     |
|  linalg     |            48 E I                         [copy v] [⋯]       | tags —      |
|  stats      |                                                              | ran 14:02   |
|  sym_*      | [5]*┃ plot(x, w)                                  stale (6)  | 3 ms        |
|             |      ┌──────────────────────────────────────────────┐        |             |
|             |      │ 0 ┤──╮                                  ╭──  │        |             |
|             |      │   │   ╰──╮                          ╭──╯     │   (7)  |             |
|             |      │-3 ┤      ╰────────────────────────╯   mm     │        |             |
|             |      └──────────────────────────────────────────────┘        |             |
+-------------+--------------------------------------------------------------+-------------+
| EDIT · MathScript 1.0.0 idle · 84 MB · 1 stale · no network · not verified               | (9)
+------------------------------------------------------------------------------------------+
```

Callouts:

1. **Header bar.** Kernel chips show language, state (● busy, ○ not started, idle) and open the kernel menu. Badges: `[net]` only when a notebook holds a network grant and a socket is open; GPU indicator while held; a signature badge on signed notebooks.
2. **Toolbar.** One primary action, Run. Cell type and language selector. Verify runs the reproducibility check.
3. **Markdown cell**, rendered. Double click or `Enter` edits.
4. **Code cell.** Left: execution counter `[3]`. A coloured bar shows state: idle, queued, running, done, failed. Footer line: language, duration, result mark.
5. **Maths output** from a symbolic command, typeset, with a copy menu (LaTeX, MathML, Unicode, code).
6. **Stale mark**: `*` after the counter and the word "stale". Hover says which upstream cell changed.
7. **Plot output** (`chart`) with hover toolbar: zoom, pan, reset, data, save.
8. **Inspector.** Variables for the active kernel (● changed by the last run); properties of the selected cell.
9. **Status bar.** Mode, kernel state and memory, stale count, network state in words, reproducibility state.

Sizes: sidebar 220 px, inspector 280 px, canvas content width capped at 900 px and centred, cell gutter 56 px. States: empty (one empty code cell with the hint "Type an expression and press Shift+Enter"), loading (source shows at once; outputs load after; a large output shows a placeholder with its size), error (kernel failed to start: banner with the reason and "Open kernel log"), offline (no effect unless the notebook has a network grant; then fetch cells fail with a clear message), locked (session locked: outputs hidden), read-only (published or read-only handle: no edit, "Fork to edit").

### 5.3 Other views and dialogs

**Capability sheet.**
```
+--------------------------------------------------------------------------+
| What may "beam-deflection.pnb" use?                                      |
| This notebook came from: Transfers (downloaded 12 Nov 2026)   unsigned   |
+--------------------------------------------------------------------------+
| Files      [x] its own data folder (3 files, read)                       |
|            [ ] ~/Documents/loads/   read          [Choose folder…]       |
| Network    ( ) none  (default)                                           |
|            ( ) these hosts only: [data.example.org:443        ] [+]      |
| GPU        [ ] compute device (dev.gpu.compute)                          |
| Models     [ ] local models through the model service                    |
| Limits     memory [2048] MB   time per cell [60] s   threads [4]         |
+--------------------------------------------------------------------------+
| Kernels start with exactly this and nothing else. You can change it      |
| later in the inspector. Grants are recorded in the audit log.            |
|                                      [Open without running]  [Allow]     |
+--------------------------------------------------------------------------+
```
States: unchanged since last grant (not shown again), notebook asks for more than before (differences highlighted), signed notebook (signer shown).

**Data import wizard.**
```
+------------------------------------------------------------------------------------------+
| Import data — loads.csv (4 KB, BLAKE3 9f2c…)                                             |
+---------------------------+--------------------------------------------------------------+
| Format     [CSV v]        | PREVIEW (first 200 rows)                                     |
| Delimiter  [, v]          | station   | time                | load_kN | ok               |
| Header     [x] first row  | text      | date                | number  | boolean          |
| Encoding   [UTF-8 v]      | A1        | 2026-03-01 09:00    | 11.8    | true             |
| Decimal    [. v]          | A1        | 2026-03-01 10:00    | 12.1    | true             |
| Dates      [ISO 8601 v]   | A2        | 2026-03-01 09:00    | NA      | false            |
| NA values  [NA, ""]       +--------------------------------------------------------------+
| Into       (•) table `loads` (ForgeScript)  ( ) matrix `loads` (MathScript, numeric cols)|
+---------------------------+--------------------------------------------------------------+
| Will insert:  loads = data::read_csv("loads.csv", {header: true, na: ["NA", ""]})        |
|                                                         [Cancel]  [Insert cell and run]  |
+------------------------------------------------------------------------------------------+
```
The wizard only writes a visible cell. States: parse error (row and column named), type conflict in a column (shown in the header with a count).

**Variable and matrix viewer** (detached or in the inspector).
```
+------------------------------------------------------------------------------------------+
| w   matrix 1×101  f64   808 B   defined in cell [4]      [Plot] [Export…] [Heat tint ☐] |
+------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
|      | 0         | 1         | 2         | 3         | 4         | 5         | …         |
| 0    | 0.000000  | -0.000344 | -0.000687 | -0.001029 | -0.001369 | -0.001707 | …         |
+------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
| min -0.002754   max 0   mean -0.001753   go to [row,col]   format [6 fixed v]            |
+------------------------------------------------------------------------------------------+
```

**Verify report.**
```
+------------------------------------------------------------------------------------------+
| Verify beam-deflection.pnb                                                               |
| Environment  pinned: MathScript 1.0.0 (a4adf28) avx2 4 threads seed default              |
|              now:    MathScript 1.0.0 (a4adf28) avx2 4 threads seed default     same ✓   |
| Inputs       loads.csv 9f2c… same ✓                                                      |
+------+---------------------------+-------------------------------------------------------+
| cell | result                    | detail                                                |
| c01  | identical                 |                                                       |
| c02  | identical                 |                                                       |
| c03  | equal within 1e-12        | max relative difference 3.1e-16                       |
| c04  | different                 | stored output came from an earlier version of the cell|
+------+---------------------------+-------------------------------------------------------+
| 3 of 4 reproduce.                         [Keep stored outputs] [Replace with this run]  |
+------------------------------------------------------------------------------------------+
```
States: environment differs (the differing fields are named and results are labelled "compared across environments"), kernel missing.

**Publish and sign.**
```
+----------------------------------------------------------------+
| Publish a read-only copy                                       |
| Include   [x] outputs  [x] lock file  [x] attachments (3, 9 KB)|
|           [ ] hide inputs tagged hide-input                    |
| Verify    [x] run Verify first (last: 3 of 4 reproduce)  ⚠     |
| Sign with [ Odin (ed25519 … 4f1c) v ]                          |
| Output    beam-deflection.pnbz                                 |
|                                  [Cancel]  [Publish…]          |
+----------------------------------------------------------------+
```

**Notebook diff.**
```
+------------------------------------------------------------------------------------------+
| Compare  a: checkpoint "before review"      b: working copy        3 changed · 1 added   |
+------------------------------------------+-----------------------------------------------+
| [3] E = 2.1e11                           | [3] E = 2.1e11                                |
|     I = 8.3e-6                           |     I = 8.3e-6                                |
| -   L = 4                                | +   L = 4.2                                   |
+------------------------------------------+-----------------------------------------------+
| (no cell)                                | + [6] markdown  "Deflection limit is L/250."  |
+------------------------------------------+-----------------------------------------------+
| [5] plot(x, w)   output: plot changed    [show plots side by side]                       |
+------------------------------------------------------------------------------------------+
```

**Teaching mode (learner view).**
```
+------------------------------------------------------------------------------------------+
| Lesson 3 — Derivatives                                   progress ███████░░░ 5 of 7      |
+------------------------------------------------------------------------------------------+
| [locked] Differentiate f(x) = x² sin x and store the result in `df`.                     |
| [ ] ┃ df = sym_diff("...", "x")                         ← your answer (editable)         |
|       [Check ✓]   [Hint 1 of 2]   [Show solution] (unlocks after 2 attempts)             |
|       ✖ Not yet: df should contain a cos term.                                           |
+------------------------------------------------------------------------------------------+
```

### 5.4 Menus

```
File      New  Ctrl+N · New From Template… · Open…  Ctrl+O · Open Recent ▸ · Import .ipynb…
          Save  Ctrl+S · Save As…  Ctrl+Shift+S · Save Checkpoint…  Ctrl+Alt+S · Revert to Checkpoint ▸
          Export ▸ (PDF…  Ctrl+Shift+E, HTML…, Markdown…, .ipynb…, Script…) · Publish and Sign…
          Strip Outputs · Print  Ctrl+P · Close  Ctrl+W · Quit  Ctrl+Q
Edit      Undo  Ctrl+Z · Redo  Ctrl+Shift+Z · Cut/Copy/Paste Cells  X / C / V (command mode)
          Delete Cell  D D · Split Cell  Ctrl+Shift+- · Merge With Below  Shift+M
          Move Up  Alt+Up · Move Down  Alt+Down · Find  Ctrl+F · Replace  Ctrl+H · Preferences  Ctrl+,
Insert    Code Cell Above  A · Below  B · Markdown Cell · Maths Cell · Widget… · Matrix… · Symbol…
          Data Import…  Ctrl+Shift+I · Table of Contents · Image…
Cell      Run  Ctrl+Enter · Run and Advance  Shift+Enter · Run and Insert  Alt+Enter
          Run All  Ctrl+F5 · Run Above · Run Below · Run Stale  F5* · Type ▸ (Code  Y, Markdown  M, Maths  E, Raw  R)
          Language ▸ (MathScript, ForgeScript) · Collapse Input · Collapse Output · Clear Output · Tags…
Kernel    Interrupt  I I · Restart  0 0 · Restart and Run All  Ctrl+Shift+F5 · Shut Down
          Capability Sheet… · Kernel Log · Environment…
View      Sidebar  F9 · Inspector  F4 · Bottom Panel  Ctrl+J · Outline · Variables · Line Numbers
          Hide All Inputs · Presentation  F11* · Zoom ▸
Tools     Verify (Re-run and Compare)  Ctrl+Shift+V · Compare Versions… · Function Catalogue
          Explain Cell  Ctrl+Shift+A · Draft Cell From Description… · Teaching Mode ▸
Help      Notebook Help  F1 · MathScript Reference · ForgeScript Reference · Keyboard Shortcuts  Ctrl+/ · About
```

`*` `F5` (reserved "refresh") runs stale cells, which is this app's refresh. `F11` presentation is full screen with slide paging.

### 5.5 Keyboard and pointer

Command mode single keys (no modifier): `A` `B` insert, `D D` delete, `Y` `M` `E` `R` type, `X` `C` `V` cut/copy/paste cell, `J`/`K` or arrows move, `Shift+J/K` extend selection, `O` toggle output, `H` toggle input, `L` line numbers, `I I` interrupt, `0 0` restart, `Z` undo cell delete.

Edit mode: `Tab` complete or indent, `Shift+Tab` signature help, `Ctrl+/` comment, `Ctrl+]`/`Ctrl+[` indent, `Esc` to command mode.

Context menus. Cell: Run, Run From Here, Cut, Copy, Paste Below, Duplicate, Move, Type ▸, Language ▸, Collapse, Tags, Copy Cell Link, Explain. Output: Copy ▸ (text, LaTeX, MathML, image, CSV), Save As, Open in Window, Open in Sheets, Clear, Pin. Variable: View, Plot, Insert Name, Export, Go to Definition. Plot: Zoom, Reset, Show Data, Edit Properties, Save As.

Drag and drop: cells by the gutter to reorder; a CSV or image file from Files onto the canvas starts the import wizard or inserts an image (the file is copied to attachments after a prompt); a variable from the explorer into a cell inserts its name; a plot out to another app as SVG or PNG; a table out as CSV.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | One empty cell and a hint line. |
| Loading | Source first, outputs lazily; counters shown from the lock file. |
| Running | Cell bar animates (reduced motion: static "running" label); queue positions shown; Stop enabled. |
| Kernel dead | Banner: "The MathScript kernel stopped (memory limit 2048 MB)". Outputs stay; variables panel empties; Restart offered. |
| Error | Cell marked failed at a line; later queued cells are cancelled and say so. |
| Offline | Only fetch cells are affected. |
| Read-only | Published or read-only file: banner with signer state; "Fork to edit". |
| Tampered | Signature check failed: opens as plain text, no outputs rendered, red banner. |
| Narrow (<640) | Sidebar and inspector become overlays; toolbar collapses to Run and a menu; plots fit width. |
| No GPU / no model | Cells that ask for them say which capability or device is missing. |

### 5.7 Widgets

| Region | FUSE widget | Notes |
|---|---|---|
| Cell input | `codeEdit` | One instance per visible cell, recycled; needs grammars, completion popup, multi-cursor (FUSE-WP-37) |
| Markdown cells (rendered) | `markdownView` | Needs emphasis, links, tables, images, inline maths hooks (FUSE-WP-42) |
| Cell canvas | `scrollArea` with virtualised cell list | Variable-height virtualisation (FUSE-WP-28); `AI04-GAP-2` |
| 2D plots | `chart` | Needs real axes, log and date scales, legends, heat map, contour, zoom and pan, data table (FUSE-WP-46) |
| 3D plots | `viewportWidget` | Small scene renderer for surfaces and scatter; CPU fallback (`AI04-GAP-3`) |
| Tables, matrix viewer, variables | `tableView`, `modelListView` | Cell selection, copy ranges (FUSE-WP-28, FUSE-WP-40 for large grids) |
| Outline, files, catalogue | `treeView`, `listView` | |
| Widgets in outputs | `slider`, `spinBox`, `comboBox`, `checkBox`, `toggleSwitch`, `lineEdit`, `button`, `colorPicker`, `dateTimeEdit` | |
| Inspector | `propertyGrid`, `tabWidget` | |
| Diff | diff view | FUSE-WP-38 |
| Dialogs and menus | `dialog`, `menuBar`, `popupMenu`, `toolButton`, `progressBar`, `tooltip` | |
| Typeset maths | none today | `AI04-GAP-1` |

**Maths renderer needed (`AI04-GAP-1`).** FUSE has no maths typesetting. The app needs a maths layout engine that takes Presentation MathML or the LaTeX subset MathScript emits and produces positioned glyphs and rules for the FUSE draw list: fractions, radicals, scripts, large operators with limits, stretchy delimiters, matrices, accents, spacing by atom class. It must use an OpenType font with a MATH table (for example STIX Two Math or Latin Modern Math, both under the SIL Open Font Licence 1.1) and read that table through the FULL text profile's HarfBuzz (G-03). Own code in C++23, shared by OF04 Writer, OF07 Calculator and UT05 Help. It also emits an a11y tree and a spoken form. No TeX engine and no web engine are used.

Other app gaps: `AI04-GAP-2` virtualised canvas of mixed-height cells with embedded editors. `AI04-GAP-3` 3D plot renderer. `AI04-GAP-4` portable GPU compute path for MathScript on the OS (its backend is CUDA). `AI04-GAP-5` a structured-value API on the MathScript interpreter (today `execute` returns formatted text). `AI04-GAP-6` MathScript built on HardenedBSD (Unverified; its CI covers Windows MSVC, Linux GCC 13, macOS AppleClang).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Document model, undo, execution queue, stale tracking, lock and manifest, export assembly, diff |
| UI | FUSE UI | app host | Canvas, cells, inspector, dialogs |
| MathScript kernel | C++23 (MathScript libraries + thin protocol shim) | helper `kernel-mathscript`, one per notebook, capability mode | Runs lines in `ms::interp::Interpreter`; returns typed values and plot series |
| ForgeScript kernel | C++23 host of `forge::script::engine` | helper `kernel-forgescript`, one per notebook, capability mode + FSS policy | Runs ForgeScript cells; data table library; model and network bindings when granted |
| Maths layout | C++23 | in the app host as a FUSE widget (trusted input: MathML made by our own printers) or helper for imported LaTeX | Typeset formulae |
| Import helper | C++23 | helper `import`, capability mode | Parses `.ipynb`, CSV, `.npy`, Matrix Market, images, `.pnbz` archives |
| Export helper | C++23 | helper `export`, capability mode | PDF and SVG writers, page layout |
| Plot renderer | FUSE UI + ForgeScript | app host | Spec → draw list; spec → SVG |
| Assistant | ForgeScript | app host → `svc.model` | Explain and draft |

```
+------------------------- app host (ForgeScript + FUSE UI) ------------------------+
| document model · queue · stale graph · lock/manifest · plot specs · maths layout  |
+------+-----------------+------------------+----------------+---------------+------+
       | kernel protocol | kernel protocol  | records        | records       | bus
       v                 v                  v                v               v
 kernel-mathscript   kernel-forgescript   import           export         svc.model
 (granted folders r, (FSS policy from     (one file r,     (scratch rw)   svc.keyring
  scratch rw,         capability sheet;    scratch rw)                    svc.audit
  no network)         net only if granted)
       |
       v  optional, after dev.gpu.compute grant
   GPU compute device
```

**Kernel protocol** (Proposed; session-bus record schema `org.pbsd.notebook.Kernel/1`): `hello(env)`, `execute(cell_id, seq, text) → stream of output records, then done(status, duration)`, `interrupt()`, `complete(text, pos)`, `inspect(name)`, `variables()`, `value(name, slice)`, `share(name)`, `take(name, value)`, `shutdown()`. Output records are typed (`text`, `math{mathml, latex}`, `table{columns, rows-handle}`, `plot{spec, data-handle}`, `image{handle}`, `error{line, message}`); large data travels as shared-memory handles. The app renders records; a kernel cannot make the app run anything.

**MathScript today** (Verified):

- **Language.** A line-oriented REPL command language: one command per line, no block syntax and no several statements per line. Scalars, matrix literals `[1, 2; 3, 4]`, assignments including multi-target (`L, U = lu(M)`), function calls, quoted formula strings for symbolic and ODE routines, session-object handles, plot commands, meta-commands (`vars`, `clear`, `save`, `load`, `run_file`/`source`, `help`). No user-defined functions, loops or conditionals were found in the user guide (`MATHSCRIPT:docs/USER_GUIDE.md`).
- **Engine.** A C++23 library of 35 static libraries under `src/` with public headers in `include/ms/`; in-tree BLAS/LAPACK; errors as `Result<T>`; symbolic modules `symbolic` and `sym2` (precedence-aware printers for LaTeX, MathML, Unicode and code; a strict LaTeX-subset parser). The interpreter is `ms::interp::Interpreter` with `execute(line) → Result<std::string>`, session save/load, and a `PlotSeries` state.
- **Executables.** `mathscript-repl`, `mathscriptc` (batch runner), `mathscript-server` (MPI compute node). Options: Qt 6 IDE (`MS_BUILD_GUI`, `src/gui/`, links Qt6 Widgets, OpenGLWidgets, OpenGL), CUDA, MPI, LLVM ORC JIT (`MS_BUILD_JIT`, LLVM 18), all off by default.
- **Clang plugin.** `src/plugin/` (`MS_BUILD_PLUGIN`, needs LLVM/Clang dev packages; built in a Linux CI job) enforces twenty rules of a restricted C++23 subset and records `MS_UNSAFE` sites to `ms-unsafe-audit.json`. It checks C++ source of the library and of OS helpers; it is not part of the notebook run time.
- **Status claimed by the repo.** Version 1.0.0; 374 CTest suites passing on Linux GCC 13 and Windows MSVC per its README. Not re-run for this spec.

### 6.2 Data model

**Native file `name.pnb`** (UTF-8, LF, text). Cell delimiter lines start with `%%%` in column 0; a body line that itself starts with `%%%` is written with a leading backslash, which the reader removes.

```
%%% notebook 1
%%% title: Beam deflection
%%% languages: mathscript, forgescript
%%% requires: fs.data:r
%%% cell c1a9e3f0 markdown
# Beam deflection
A simply supported beam with a point load at midspan.
%%% cell 7be210c4 code mathscript
E = 2.1e11
I = 8.3e-6
%%% cell c7f3a1b2 code mathscript
%%% tags: hide-input
sym_diff("P*x*(3*L^2-4*x^2)/(48*E*I)", "x")
%%% cell 0d44a7e1 math eq:deflection
w(x) = \frac{P x (3 L^{2} - 4 x^{2})}{48 E I}
```

Header keys (one per line, fixed order): `title`, `authors`, `languages`, `requires` (capability sheet entries: `fs.data:r|rw`, `fs.folder:<label>:r|rw`, `net.host:<host>:<port>`, `dev.gpu.compute`, `ai.local`), `limits` (`memory_mb`, `cell_seconds`, `threads`), `number_format`, `teaching`. Cell line: `%%% cell <id> <type> [<language>|<label>]`, optionally followed by `%%% tags:` and `%%% widget:` lines. No timestamps, counters or outputs in this file.

**Sidecars** beside the source:

| File | Content |
|---|---|
| `name.pnb.out` | JSON Lines, one record per stored output: `cell: text`, `input_hash: blake3`, `seq: u32`, `kind: enum`, `mime: text`, `inline: text?`, `blob: blake3?`, `spec: object?` |
| `name.pnb.lock` | JSON: `format: 1`, `kernels: [{language, version, commit, isa, threads, seed}]`, `inputs: [{label, rel_path, size, blake3}]`, `runs: [{id, started, order: [cell], cells: [{cell, input_hash, output_hashes, ms}]}]`, `verified: {at, result}` |
| `name.pnb.data/` | Attachments and output blobs, named by BLAKE3 |
| `name.pnb.sig` | Detached Ed25519 signature over the BLAKE3 tree hash of the four items above (published form) |

`.pnbz` is an uncompressed-or-deflate archive of exactly those items plus `manifest.json` (tree hash, format versions, signer key id).

In-memory entities: `Notebook{id, header, cells: [Cell], undo}`, `Cell{id: text(8), type: enum, language: enum?, label: text?, tags: [text], source: text, state: enum(idle, queued, running, done, failed, stale), exec_count: u32, input_hash: blake3, outputs: [Output]}`, `Output{kind, mime, data|blob, spec, from_input_hash}`, `KernelSession{language, pid_handle, session_id, env, state, memory_bytes}`, `Variable{name, kind, shape, bytes, preview, defined_in}`, `Grant{capability, resource, scope, granted_at}`, `Checkpoint{name, created, tree_hash}`.

App data: `~/.local/share/org.pbsd.notebook/` holds the recent list, grants per notebook hash, checkpoints index and teaching progress (SQLite, `schema_version`). Format versions are in every file; older versions are read and upgraded on save with a backup copy.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| `.pnb`, `.pnb.out`, `.pnb.lock` | yes | yes | own (line format, JSON) | app host for own files; `import` helper when the origin is untrusted |
| `.pnbz` | yes | yes | own archive reader/writer, BLAKE3 and Ed25519 from FORGE | `import` helper |
| Jupyter `.ipynb` (nbformat 4) | yes | yes | own JSON mapping | `import` helper |
| Markdown (CommonMark subset + maths spans) | yes | yes | FUSE `markdownView` path, own | app host |
| LaTeX maths subset | yes | yes | MathScript `sym2` parser and printer | `kernel-mathscript`; display via MathML |
| Presentation MathML subset | yes | yes | own maths layout | app host |
| MathScript commands, `.ms` scripts and sessions | yes | yes | MathScript interpreter | `kernel-mathscript` |
| ForgeScript `.fs` | yes | yes | FORGE engine | `kernel-forgescript` |
| CSV, TSV, fixed width, JSON data | yes | yes | own | `import` helper or kernel |
| `.npy`, Matrix Market, WAV | yes | yes | own | `import` helper |
| PNG, JPEG | yes | PNG | system image decoders through `svc.thumbnail`-class helper | image helper |
| SVG (subset), PDF | no | yes | own writers | `export` helper |
| HTML export | no | yes | own static writer | app host |
| BibTeX, CSL-JSON | yes | no | own | `import` helper |
| Kernel protocol | — | — | session-bus schema | both ends validate |

### 6.4 Integration

Intents handled: `open`, `edit`, `view`, `new` for `.pnb`/`.pnbz`; `open` for `.ipynb` (import); `open` for `.ms` and `.fs` (as a one-cell notebook). Emitted: `open` to OF05, OF09, OF03 (exported PDF), DV01 (script), AI02; `share`, `print`.
MIME: view/edit `application/x-pbsd-notebook`, view `application/x-pbsd-notebook-packed`; import `application/x-ipynb+json`, `text/csv`, `text/x-mathscript`, `text/x-forgescript`; export `application/pdf`, `text/html`, `text/markdown`, `application/x-ipynb+json`, `image/svg+xml`, `image/png`, `text/csv`.
Services: `svc.portal.file`, `svc.settings`, `svc.notify`, `svc.intent`, `svc.keyring`, `svc.model`, `svc.audit`, `svc.search`, `svc.portal.print`, `svc.clipboard`, `svc.backup`.
IPC offered: `org.pbsd.notebook.Merge/1` (three-way merge of `.pnb` for DV03), `org.pbsd.notebook.Render/1` (render a notebook to PDF headless for other apps).
Links: OF07 Calculator uses the same MathScript kernel helper and maths layout; DV03 shows `.pnb` as text and can call the merge helper; AI03 can export a forecast as a notebook.

Verified from MathScript: section 6.1 list. Verified from FORGE: ForgeScript engine, capability flags and FSS policy, signing (PLATFORM.md V10–V15); `jit_threshold` compiles to bytecode only (`FORGE:script/src/runtime.cpp:729-730`). Proposed: the kernels, protocol, file format, plotting, maths layout, data table library and everything else here.

## 7. Security and privacy

Capabilities requested:

| Capability | Reason |
|---|---|
| `fs.app` | Recent list, grants, checkpoints, progress |
| `fs.portal` | Notebooks and data folders the user picks; grants remembered per notebook |
| `proc.helper:kernel-mathscript`, `proc.helper:kernel-forgescript` | Run code out of process |
| `proc.helper:import`, `proc.helper:export` | Parse foreign files and write PDF outside the app host |
| `net.user` | Only for notebooks whose capability sheet names hosts and the user allowed them; off otherwise; each first contact logged |
| `dev.gpu.compute` | GPU cells, prompt on first use, indicator while in use |
| `ai.local`, `svc.model` | Explain, draft, model calls from code |
| `bg.run` | Long runs after the window closes |
| `svc.portal.file`, `svc.settings`, `svc.notify`, `svc.intent`, `svc.keyring`, `svc.audit`, `svc.search`, `svc.portal.print`, `svc.clipboard`, `svc.backup` | Section 6.4 |

Not requested: `proc.exec`, `fs.path`, `net.host` fixed endpoints, cameras, microphones, `bg.autostart`. A kernel never has more than its notebook's sheet, and a sheet never has more than the app.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| User files and network | Author of a shared notebook | Code that reads files or sends data out | Nothing runs on open; kernels start with an empty sheet; file access only through granted folder handles; no network without a named-host grant; all grants audited |
| App host | Malicious notebook file | Malformed `.pnbz`, `.ipynb`, images, huge outputs | Parsing in the `import` helper; size limits; no HTML or script outputs rendered; blobs addressed by hash |
| App host | Malicious kernel output | Escapes through output records | Typed records validated against the schema; plots are data plus a declarative spec; maths is MathML subset; no executable output kinds |
| Reader's trust | Forger | Altered published notebook or stale outputs passed off as results | Signature over the tree hash; outputs carry input hashes; Verify re-runs and compares; freshness marks |
| Other notebooks | A running notebook | Reading another notebook's variables or data | One kernel process per notebook; no shared state; `share`/`take` is explicit and within one notebook |
| Machine | Runaway code | Memory or CPU exhaustion | Per-kernel memory, time, thread and output limits; kill after grace period |
| Secrets | Notebook code | Reading the keyring | Kernels have no `svc.keyring`; signing happens in the app host on a user action |
| Learner data | Lesson author | Hidden collection of answers | No network by default; hand-in is a file the learner exports |
| Model channel | Text in cells or data | Prompt injection | Model drafts are inserted unrun; a model never triggers a run or a grant |

MathScript has cryptographic bindings (`crypto_*`); keys typed into cells are ordinary notebook text and are saved. The app warns when a cell matches a private-key pattern and offers to keep that cell's output out of the sidecar.

Stored: notebooks where the user saved them, sidecars beside them, grants and progress in app data. Leaves the machine: nothing, unless a notebook with a network grant fetches from its named hosts, or the user exports or sends a file.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `kernel.default_language` | enum | mathscript | Language of new code cells |
| `kernel.memory_mb` | int | 2048 | Default per-kernel memory limit |
| `kernel.cell_seconds` | int | 60 | Default time limit per cell (0 = none) |
| `kernel.threads` | int | 4 | Default thread count; pinned in the lock |
| `kernel.interrupt_grace_ms` | int | 3000 | Wait before killing on interrupt |
| `kernel.start_on_open` | bool | false | Start kernels when a notebook opens (never for untrusted origin) |
| `run.stop_on_error` | bool | true | Cancel queued cells after a failure |
| `run.warn_out_of_order` | bool | true | Banner on out-of-order execution |
| `output.max_inline_kb` | int | 256 | Larger outputs become blobs with a preview |
| `output.number_format` | string | `6g` | Default number format |
| `output.store` | bool | true | Keep outputs in the sidecar |
| `output.imaginary_unit` | enum | i | i or j |
| `plot.theme` | enum | system | system, light, dark, print |
| `plot.default_size` | size | 640×400 | Default plot size |
| `plot.palette` | enum | safe | Colour-blind-safe default |
| `widgets.rerun_delay_ms` | int | 80 | Rest time before re-run |
| `editor.completion` | bool | true | Completion popup |
| `editor.line_numbers` | bool | false | Line numbers in cells |
| `repro.tolerance` | float | 1e-12 | Relative tolerance for "numerically equal" |
| `repro.verify_before_publish` | bool | true | Run Verify in the publish dialog |
| `import.ipynb_keep_foreign_code` | bool | true | Keep non-runnable code cells as text |
| `assist.enabled` | bool | false | Explain and draft features |
| `assist.model` | string | empty | Model id in `svc.model` |
| `teaching.solution_after_attempts` | int | 2 | Attempts before a solution unlocks |
| `search.index_notebooks` | bool | true | Contribute titles and text to system search |

## 9. Accessibility and languages

- Maths has a spoken form ("fraction, numerator P times …") and can be walked part by part with arrow keys; Braille output needs Nemeth or UEB maths tables in AX01 (open).
- Every plot exposes its data table and a one-line summary (kind, series count, axis ranges).
- Cell state is announced: "cell 5, code, MathScript, stale, has plot output".
- Command-mode single keys are off while a screen reader's browse mode needs them; all have menu equivalents.
- Function names and keywords are not translated. Number formatting in outputs follows the notebook setting, not the locale, so files mean the same everywhere; the UI chrome follows the locale.
- Right-to-left text in Markdown cells is supported; code and maths stay left-to-right.

## 10. Performance budgets

| Item | Budget |
|---|---|
| Cold start to an empty notebook | 300 ms |
| Open a 500-cell notebook with stored outputs | 500 ms to first screen; scroll at 60 fps |
| MathScript kernel start | under 300 ms to `hello` |
| Round trip for `1 + 2` | under 20 ms from key press to output |
| Plot of 1 million points | first frame under 150 ms with decimation; pan and zoom 60 fps |
| Table of 10 million cells | virtualised; scroll 60 fps |
| Largest stored output | 256 MB per blob; notebook total limited by disk |
| Typeset a 200-symbol formula | under 5 ms |
| Export a 50-page PDF | under 5 s |
| Idle | zero frames, zero wake-ups with idle kernels |
| App host memory, 500-cell notebook | under 250 MB excluding kernels |

## 11. Testing and proof

Test plan:

- **Unit (ForgeScript)**: `.pnb` reader and writer round trip, delimiter escaping, stale tracking, queue, hashing, lock handling, diff and merge by cell id.
- **Format properties**: `read(write(n)) == n` for random notebooks; saving an unchanged notebook writes identical bytes.
- **Golden renders**: each output kind, each plot type, a corpus of formulae against reference images at two sizes.
- **Maths layout**: the round-trip corpus MathScript uses for `parse_latex(to_latex(e)) == e` rendered without overlap or missing glyphs; metric checks against the font's MATH constants.
- **Kernel conformance**: every command in MathScript's REPL corpus (`MATHSCRIPT:tests/repl_corpus/`) gives the same text through the kernel as through `mathscriptc`.
- **Differential**: plot data against the console preview's series; CSV import against a second reader on a public corpus of malformed CSV.
- **Scripted UI**: the 15 top tasks through the a11y tree.
- **Fuzz**: `.pnb`, `.pnbz`, `.ipynb`, CSV, `.npy`, Matrix Market, MathML subset, kernel protocol records, plot specs.
- **Sandbox**: a cell that tries to open a file outside its grants, open a socket, or start a process fails, and the denial is audited (needs G-06).
- **Reproducibility**: Verify on fixtures gives "identical" on the same environment; a forced thread-count change gives "within tolerance" or "different" and names the change.
- **Import**: an `.ipynb` corpus with HTML and script outputs imports with those dropped and reported.

Proof tiers (targets only; nothing has run):

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| `.pnb` reader and writer | Round trip; no out-of-bounds on any input | T2 | PRISM contracts and `pir` on the C++ reader used in the import helper; property tests for the ForgeScript one | Two implementations must agree |
| Signature and tree-hash check | A changed byte is detected | T2 | PRISM on the hashing walk; known-answer tests | FORGE crypto's own tier |
| Kernel protocol decoder | No memory errors; rejects records outside the schema | T3 | Safe subset, sanitizers, fuzzing | Schema compiler (G-08) |
| Capability sheet to policy mapping | Kernel policy ⊆ sheet ⊆ app manifest | T1 | Small Lean model of the narrowing relation with the mapping proved monotone; shared with FORGE-WP-08 | Depends on the FORGE policy model |
| Import helper (`.ipynb`, CSV, `.npy`) | No memory errors | T3 | Safe subset, fuzzing | Size |
| Maths layout | No memory errors; boxes do not overlap for the supported subset | T3 | Fuzzing, golden tests | Layout correctness is not specified formally |
| MathScript kernel (library code) | As stated by MathScript: restricted subset with approved `MS_UNSAFE` sites, sanitizers, fuzz targets | T3 | Its plugin and CI; PRISM bounded stages on the shim | Size of the numerical library; floating-point semantics |
| Numerical results | Not a proof target | T4 | MathScript's numerical test suites | — |
| UI | No tier | T4 | Golden and scripted tests | — |

## 12. Build plan

Phase 1, usable core:

| ID | Work item | Depends on |
|---|---|---|
| AI04-W01 | `.pnb` format reader and writer, sidecars, autosave | G-07 |
| AI04-W02 | Kernel protocol schema and launcher with per-notebook policy | G-06, G-08, FORGE-WP-08, FORGE-WP-27 |
| AI04-W03 | MathScript built for HardenedBSD (JIT, CUDA, MPI, Qt off); kernel shim with typed values | AI04-GAP-5, AI04-GAP-6 |
| AI04-W04 | ForgeScript kernel | FORGE-WP-10, FORGE-WP-17 |
| AI04-W05 | Cell canvas, cell editing, two modes, undo | G-02, AI04-GAP-2, FUSE-WP-37 |
| AI04-W06 | Markdown cells | FUSE-WP-42 |
| AI04-W07 | Execution queue, interrupt, restart, limits, status | AI04-W02 |
| AI04-W08 | Outputs: text, error, table, basic 2D plots; stored outputs with input hashes | FUSE-WP-46, FUSE-WP-28 |
| AI04-W09 | Maths layout engine v1 (fractions, scripts, radicals, matrices, delimiters) | AI04-GAP-1, G-03, FUSE-WP-16 |
| AI04-W10 | Capability sheet and first-run review | G-09, SH04 |
| AI04-W11 | Variable list, completion from the catalogue | AI04-W03 |

Phase 2, full feature:

| ID | Work item | Depends on |
|---|---|---|
| AI04-W12 | Plot specification, full 2D set, interaction, export | AI04-W08 |
| AI04-W13 | 3D plots | AI04-GAP-3, FUSE-WP-24 |
| AI04-W14 | Widgets and stale tracking | AI04-W07 |
| AI04-W15 | Data import wizard, attachments, input hashing, data table library | AI04-W04 |
| AI04-W16 | Variable and matrix viewer, quick plot | AI04-W11 |
| AI04-W17 | Lock file, run manifest, Verify | AI04-W03 |
| AI04-W18 | Import and export `.ipynb` | AI04-W01 |
| AI04-W19 | Export PDF, HTML, Markdown, script; print | G-16, AI04-W09 |
| AI04-W20 | `.pnbz`, publish, sign, verify on open, reader mode | FORGE-WP-22, FORGE-WP-33 |
| AI04-W21 | Notebook diff view, checkpoints | FUSE-WP-38 |
| AI04-W22 | Maths cells with LaTeX-subset input; copy as LaTeX/MathML | AI04-W09 |
| AI04-W23 | Explain and draft with the local model | G-15, FORGE-WP-29 |
| AI04-W24 | Signature help, catalogue browser, spell check | G-17 |

Phase 3, advanced:

| ID | Work item | Depends on |
|---|---|---|
| AI04-W25 | Run-time units | MathScript change |
| AI04-W26 | GPU compute path and indicators | AI04-GAP-4, FORGE-WP-32 |
| AI04-W27 | Model calls from ForgeScript cells | FORGE-WP-30 |
| AI04-W28 | Teaching mode, check cells, lesson packs, presentation | AI04-W14 |
| AI04-W29 | Merge helper service for DV03; parameters cell | DV03 |
| AI04-W30 | Network data with named hosts and cached responses | FORGE-WP-21, SE02 |
| AI04-W31 | Formula editor, cross-references, bibliography | AI04-W22 |
| AI04-W32 | Parquet/HDF5 readers, statistical and vector-field plots, animations | AI04-W12 |

## 13. Open questions and risks

1. **MathScript is a command language, not a programming language.** No loops, conditionals or user functions were found. Notebooks that need control flow must use ForgeScript cells or MathScript must grow a language. Owner decision.
2. **Structured values.** The interpreter returns formatted text. The kernel needs typed access to scalars, matrices, symbolic expressions and plot series; `SessionState` exposes matrices, scalars and one plot, but symbolic results are strings. This is a MathScript change (`AI04-GAP-5`).
3. **One plot per session.** `SessionState` holds a single `PlotSeries`. Multi-series plots and several plots per cell need the plot specification layer and interpreter changes.
4. **HardenedBSD build of MathScript is Unverified.** Its CI covers Windows, Linux and macOS.
5. **GPU.** The only GPU backend is CUDA. Whether CUDA can exist on the OS at all is open; a portable path is a large piece of work.
6. **Maths layout engine** is a new shared component with real typographic depth. Without it the app falls back to Unicode linear output (`sym_export(…, "unicode")`), which is usable but poor.
7. **Bit-level reproducibility across machines** is not promised by MathScript. The app reports the truth; some users will expect more.
8. **Symbolic strength.** The symbolic modules cover a defined set of operations. Users coming from large computer-algebra systems will hit limits; the app must show refusals plainly.
9. **`.ipynb` round trip is lossy** in both directions. The import and export reports must be read.
10. **Units at run time** do not exist in the REPL today.
11. **Signing keys and identity.** A fingerprint is not a name. How signer keys are introduced and trusted belongs to SE04.
12. **Remote kernels** (the MPI compute node) would need a network capability and a trust story; left out until asked for.
