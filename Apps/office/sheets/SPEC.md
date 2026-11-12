<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Sheets

By Odin Loch

| | |
|---|---|
| ID | `OF05` (from CATALOGUE.md) |
| Bundle | `org.pbsd.sheets` |
| Category | office |
| Priority | P1 |
| Backing repos | FUSE (UI, `tableView`, `chart`, `SqliteModel`), FORGE (ForgeScript, signing), MathScript (solver, statistics and finance kernels) |
| Status | Planned. Nothing built. |

## 1. Purpose

The spreadsheet for the suite. People use it for budgets, lists, analysis of large tables, charts and what-if models. It opens ODS natively, reads and writes XLSX and CSV, recalculates on all cores with the same result every time, and treats every imported file and every macro as hostile until shown otherwise. It replaces LibreOffice Calc from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- A grid of 1,048,576 rows by 16,384 columns per sheet (XLSX limits), virtualised, with up to 1,024 sheets per workbook.
- A formula language compatible with OpenFormula (ODF 1.3 part 4, "Large" group) plus the modern dynamic-array and lambda functions of XLSX.
- A native calculation engine: dependency graph, multi-threaded deterministic recalculation, binary64 by default and decimal128 as a workbook option.
- Tables, named ranges, structured references, array formulas with spill, lambdas.
- Pivot tables, charts, conditional formatting, data validation, filters, sorts, outlines.
- What-if tools: goal seek, solver, scenarios, data tables.
- Import: CSV/TSV with an import dialog, fixed-width text, JSON, SQLite, Parquet, XLSX, ODS, FODS. Export: ODS, XLSX, CSV/TSV, JSON, Parquet, PDF, HTML table.
- Macros and custom functions in ForgeScript, signed and sandboxed.
- Cell protection, comments, tracked changes, formula auditing, print layout.

Out of scope:

| Item | Owner |
|---|---|
| SQL queries, schema browsing, server databases, ER diagrams | OF09 Data Studio |
| Free-form computational documents, symbolic maths | AI04 Notebook, OF07 Calculator |
| Time-series model fitting beyond the FORECAST functions | AI03 Forecast |
| Drawing tools beyond simple shapes on a sheet (embedded drawings are OF08 objects) | OF08 Draw |
| Word-processing of long text in cells; mail merge target documents | OF04 Writer |
| VBA execution. VBA modules are preserved as inert bytes on round trip and never run | none |
| Real-time co-editing over a network | not planned |
| Plain-text editing of a CSV as text | OF01 Text Editor |

## 3. Users and top tasks

Personas:

- **Household user**: budgets, lists, simple sums, a chart, print to one page.
- **Analyst**: million-row CSV and Parquet files, pivots, lookups, filters, charts, reproducible numbers.
- **Accountant**: exact decimal arithmetic, protected templates, tracked changes, audit of every formula.
- **Engineer or scientist**: array formulas, solver, statistics, custom functions.
- **Recipient of untrusted files**: opens an XLSX from email and wants no code run and no network touched.

| # | Task | Target steps |
|---|---|---|
| T1 | Open a CSV from Files with correct delimiter, encoding and types | 2 (double click, Import) |
| T2 | Sum a column | 2 (select cell under column, `Alt+=`) |
| T3 | Turn a range into a table with filters | 1 (`Ctrl+T`) |
| T4 | Make a chart from the selection | 2 (`Alt+F1`, pick type) |
| T5 | Build a pivot table from a table | 4 |
| T6 | Find which cells feed a result | 1 (`Ctrl+[`) |
| T7 | Freeze the header row | 2 |
| T8 | Fill a series down 10,000 rows | 2 (type, double-click fill handle) |
| T9 | Fit the sheet to one printed page | 3 |
| T10 | Save as XLSX for someone else, with a report of what will not survive | 3 |
| T11 | Run goal seek on a cell | 4 |
| T12 | Write a custom function and use it in a cell | 5 |
| T13 | Compare two versions of a workbook | 3 |
| T14 | Open an untrusted workbook with macros disabled and external links cut | 1 (default behaviour) |
| T15 | Jump to a named range or cell address | 2 (`Ctrl+G`, type) |

## 4. Features

### 4.1 Grid and navigation

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF05-F001 | Virtualised grid | Paints only visible cells. 1,048,576 × 16,384 cells per sheet. Row and column sizes kept in a prefix-sum tree so scroll to any row is O(log n). | M | 1 |
| OF05-F002 | Sparse cell store | Cells live in column-major chunks of 1,024 rows in the calc engine; empty chunks cost nothing. Typed columns (all numbers) store 8 bytes per cell. | M | 1 |
| OF05-F003 | Cell types | Empty, number, text, logical, error, date-time (number with format), rich text (runs), array anchor, spill member, lambda value. | M | 1 |
| OF05-F004 | Selection | Single cell, range, multiple ranges (`Ctrl+click`), whole rows and columns, whole sheet. Active cell moves within the selection with `Tab`/`Enter`. | M | 1 |
| OF05-F005 | Keyboard navigation | Arrows, `Ctrl+Arrow` to data edge, `Home`, `Ctrl+Home`, `Ctrl+End`, `PageUp/Down`, `Alt+PageUp/Down` (screen left and right), `Ctrl+PageUp/Down` (sheet). | M | 1 |
| OF05-F006 | Name box and go to | Shows the active address or name. Typing an address, name, or `Sheet!A1` jumps. `Ctrl+G` opens Go To with recent targets. | M | 1 |
| OF05-F007 | Go to special | Select blanks, constants, formulas, errors, comments, conditional formats, validation, visible cells only, row or column differences, precedents, dependants, last cell. | S | 2 |
| OF05-F008 | Freeze panes | Freeze rows, columns or both at the active cell. Frozen areas are separate viewports sharing one model. | M | 1 |
| OF05-F009 | Split view | Up to four panes with independent scroll. | S | 2 |
| OF05-F010 | Sheets | Add, delete, rename, reorder, duplicate, colour tab, hide, very-hide, move or copy to another workbook. Up to 1,024 sheets. | M | 1 |
| OF05-F011 | Row and column operations | Insert, delete, hide, unhide, resize, auto-fit, set default size, move by drag with `Shift`. All references adjust. | M | 1 |
| OF05-F012 | Outline and grouping | Group rows or columns to 8 levels, collapse buttons in the margin, auto-outline from formulas, subtotals. | S | 2 |
| OF05-F013 | Zoom | 10% to 400%, per sheet, `Ctrl+wheel`, zoom to selection. | M | 1 |
| OF05-F014 | Multiple windows | Several windows on one workbook; each keeps its own sheet, scroll and zoom. | S | 2 |
| OF05-F015 | Navigator panel | Sidebar tree of sheets, tables, names, charts, pivots, comments, images. Click jumps. Filter box. | S | 1 |
| OF05-F016 | Status bar aggregates | Sum, average, count, numeric count, min, max, distinct count of the selection; computed in the engine off the UI thread; click copies. | M | 1 |
| OF05-F017 | Full-screen and focus mode | `F11` hides chrome; formula bar and tabs stay on hover. | C | 2 |
| OF05-F018 | Side-by-side workbooks | Two workbooks with synchronised scroll. | C | 3 |

### 4.2 Editing and data entry

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF05-F019 | In-cell and formula-bar editing | `F2` edits in cell, typing replaces, `Esc` cancels, `Enter`/`Tab` commit and move, `Ctrl+Enter` fills the selection, `Alt+Enter` inserts a line break. | M | 1 |
| OF05-F020 | Input recognition | Numbers, percentages, currency, fractions, scientific, dates and times in the locale; a leading `'` forces text. Recognition is shown in the status bar before commit. Nothing is converted silently on import. | M | 1 |
| OF05-F021 | Undo and redo | Unlimited per session, stored as engine command inverses, survives autosave. History panel lists steps; click to roll back. | M | 1 |
| OF05-F022 | Clipboard | Copy, cut, paste with offers: native range, ODS fragment, TSV, CSV, HTML table, PNG, plain text. Marching-ants border on the source. | M | 1 |
| OF05-F023 | Paste special | Values, formulas, formats, comments, validation, column widths, all except borders; operations add, subtract, multiply, divide; skip blanks; transpose; paste link. `Ctrl+Shift+V`. | M | 1 |
| OF05-F024 | Fill handle and series | Drag or double-click to fill. Linear, growth, date (day, weekday, month, year), text with numbers, custom lists. `Ctrl+D` down, `Ctrl+R` right. | M | 1 |
| OF05-F025 | Flash fill by example | Infers a text transform from examples in the neighbouring column using a deterministic program search (split, join, case, substring, date parts). No model needed. `Ctrl+E`. | S | 2 |
| OF05-F026 | Autocomplete from column | Suggests existing values in the column while typing; `Alt+Down` lists them. | S | 1 |
| OF05-F027 | Find and replace | In selection, sheet or workbook; in values, formulas, comments; match case, whole cell, regex (FORGE regex engine, linear time). Results list panel. | M | 1 |
| OF05-F028 | Spell check | Text cells checked through the dictionary service (G-17). Off for formula cells. | S | 2 |
| OF05-F029 | Hyperlinks | Link to a cell, a name, a file (stored as a portal token, not a path) or a URL. URLs open through `svc.intent` after a confirmation that shows the full target. `HYPERLINK()` never opens anything by itself. | S | 2 |
| OF05-F030 | Images and shapes on sheets | PNG, JPEG, SVG anchored to cells; simple shapes and text boxes; z-order; alt text. Complex drawings embed an OF08 object. | S | 2 |
| OF05-F031 | Text to columns | Split a column by delimiter, fixed width or regex, with a preview. | S | 2 |
| OF05-F032 | Remove duplicates, trim, clean | One-step tools on a range with a count of rows affected and Undo. | S | 2 |
| OF05-F033 | Insert function dialog | Searchable list by category, argument fields with live result and help text. `Shift+F3`. | M | 1 |
| OF05-F034 | Checkbox and dropdown cells | Cell controls bound to the cell value (logical; validation list). No free-floating form controls with code. | S | 2 |
| OF05-F035 | Units and currency in entry | `12 kg` or `USD 5` stays text unless a number format matches; no automatic conversion. | C | 3 |

### 4.3 Formula language

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF05-F036 | Syntax | OpenFormula grammar: operators `+ - * / ^ & = <> < <= > >= %`, unary minus, range `:`, union `~`, intersection `!`/space, array constants `{1,2;3,4}`. Display syntax follows the formula flavour row below. | M | 1 |
| OF05-F037 | References | A1 and R1C1 styles; relative, absolute, mixed (`F4` cycles); sheet-qualified; 3D references `Sheet1:Sheet3!A1`; whole row and column; external workbook references (see External links in 4.9). | M | 1 |
| OF05-F038 | Compatibility target | Evaluator conformance to ODF 1.3 OpenFormula "Large" group. Each function's semantics follow OpenFormula where it defines them; where XLSX differs, the XLSX behaviour is selectable per workbook and recorded in the file. | M | 1 |
| OF05-F039 | Formula flavour | Display and entry as OpenFormula (`[.A1]`; `;` separators) is not shown to users; the UI uses A1 with locale separators. Files store OpenFormula (ODS) or OOXML syntax (XLSX). | M | 1 |
| OF05-F040 | Error values | `#NULL! #DIV/0! #VALUE! #REF! #NAME? #NUM! #N/A #SPILL! #CALC! #BLOCKED! #CIRC!`. `#BLOCKED!` is returned by any function that needs a capability the workbook does not hold. | M | 1 |
| OF05-F041 | Implicit intersection and `@` | Legacy formulas keep implicit intersection; new formulas use dynamic arrays; the `@` operator is shown where it applies. | M | 2 |
| OF05-F042 | Dynamic arrays and spill | A formula returning an array spills into neighbours; `A1#` refers to the spill range; blocked spill gives `#SPILL!` with the blocking cell highlighted. | M | 1 |
| OF05-F043 | Legacy array formulas | `Ctrl+Shift+Enter` array formulas over a fixed range, round-tripped to ODS matrix formulas and XLSX CSE formulas. | M | 2 |
| OF05-F044 | `LET` and `LAMBDA` | Local names; anonymous functions; named lambdas stored as workbook names; recursion depth limit 1,024 (setting). | S | 2 |
| OF05-F045 | Lambda helpers | `MAP REDUCE SCAN MAKEARRAY BYROW BYCOL ISOMITTED`. | S | 2 |
| OF05-F046 | Named ranges and expressions | Workbook and sheet scope; constants and formulas; relative names; name manager with filter, usage count and "go to". | M | 1 |
| OF05-F047 | Tables | A range with a header row, typed columns, banding, total row, auto-expansion, calculated columns, a unique name. `Ctrl+T`. | M | 1 |
| OF05-F048 | Structured references | `Table[Column]`, `Table[@Column]`, `Table[[#Headers],[A]:[C]]`, `[#Totals]`, `[#All]`, `[#Data]`; rename-safe. Stored as plain ranges plus a table definition in ODS (database range) with an extension attribute. | M | 2 |
| OF05-F049 | Formula editor aids | Autocomplete of functions, names, tables and columns; argument tooltip with the current argument bold; coloured range highlights that can be dragged; matching brackets; multi-line formulas with indentation; `F9` evaluates the selected sub-expression in a tooltip (does not replace it). | M | 1 |
| OF05-F050 | Iterative calculation | Circular references give `#CIRC!` unless iteration is on (max iterations 100, max change 0.001, per workbook). Order of iteration is fixed (sheet order, then row-major) so results are reproducible. | S | 2 |
| OF05-F051 | Volatile functions | `NOW TODAY RAND RANDBETWEEN RANDARRAY OFFSET INDIRECT INFO CELL`. Marked in the function list. Recalculated on edit or `F9`, never on a timer. Option to freeze volatile values at a stated instant and seed. | M | 1 |
| OF05-F052 | Deterministic random | `RAND` family uses a counter-based generator keyed by (workbook seed, cell address, recalculation counter). A fixed seed gives the same sheet on every machine and thread count. | M | 1 |
| OF05-F053 | Locale-independent storage | Files store formulas and numbers in the invariant form; display uses the locale (decimal comma, `;` separators, function names stay English by default, localised names optional). | M | 1 |

### 4.4 Function library (plan)

Target: every function of OpenFormula "Large" (ODF 1.3 part 4), plus the listed XLSX additions. Counts are planned totals. Statistics, distributions and financial kernels may call MathScript library code inside the calc engine (Verified to exist: `MathScript:include/ms/stats/stats.hpp`, `ms/prob/prob.hpp`, `ms/special/special.hpp`, `ms/finance/finance.hpp`, listed in `MathScript:docs/API.md`); the mapping per function is Proposed.

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF05-F054 | Maths and trigonometry (about 80) | `ABS ACOS ACOSH ACOT ACOTH ASIN ASINH ATAN ATAN2 ATANH BASE CEILING COMBIN COMBINA COS COSH COT COTH CSC CSCH DECIMAL DEGREES EVEN EXP FACT FACTDOUBLE FLOOR GCD INT LCM LN LOG LOG10 MOD MROUND MULTINOMIAL ODD PI POWER PRODUCT QUOTIENT RADIANS RAND RANDBETWEEN RANDARRAY ROMAN ARABIC ROUND ROUNDDOWN ROUNDUP SEC SECH SERIESSUM SIGN SIN SINH SQRT SQRTPI SUBTOTAL AGGREGATE SUM SUMIF SUMIFS SUMPRODUCT SUMSQ SUMX2MY2 SUMX2PY2 SUMXMY2 TAN TANH TRUNC`. | M | 1 |
| OF05-F055 | Logical (12) | `AND OR NOT XOR IF IFS IFERROR IFNA SWITCH TRUE FALSE CHOOSE`. | M | 1 |
| OF05-F056 | Text (about 50) | `ASC CHAR CLEAN CODE CONCAT CONCATENATE DOLLAR EXACT FIND FIXED LEFT LEN LOWER MID NUMBERVALUE PROPER REPLACE REPT RIGHT SEARCH SUBSTITUTE T TEXT TEXTJOIN TEXTSPLIT TEXTBEFORE TEXTAFTER TRIM UNICHAR UNICODE UPPER VALUE VALUETOTEXT ARRAYTOTEXT REGEX REGEXTEST REGEXEXTRACT REGEXREPLACE` and the byte variants `FINDB LEFTB LENB MIDB REPLACEB RIGHTB SEARCHB`. Lengths count grapheme clusters by option, code points by default (OpenFormula). | M | 1 |
| OF05-F057 | Date and time (about 30) | `DATE DATEDIF DATEVALUE DAY DAYS DAYS360 EDATE EOMONTH HOUR ISOWEEKNUM MINUTE MONTH NETWORKDAYS NETWORKDAYS.INTL NOW SECOND TIME TIMEVALUE TODAY WEEKDAY WEEKNUM WORKDAY WORKDAY.INTL YEAR YEARFRAC`. Epochs 1899-12-30, 1900 (with the 1900 leap-year bug only in XLSX mode) and 1904. | M | 1 |
| OF05-F058 | Lookup and reference (about 35) | `ADDRESS AREAS CHOOSE COLUMN COLUMNS FORMULATEXT GETPIVOTDATA HLOOKUP INDEX INDIRECT LOOKUP MATCH OFFSET ROW ROWS SHEET SHEETS TRANSPOSE VLOOKUP XLOOKUP XMATCH FILTER SORT SORTBY UNIQUE SEQUENCE TAKE DROP CHOOSEROWS CHOOSECOLS HSTACK VSTACK TOROW TOCOL WRAPROWS WRAPCOLS EXPAND`. Exact-match lookups build a hash index per column on first use. | M | 1 |
| OF05-F059 | Statistical (about 110) | Averages, counts, `COUNTIF(S) AVERAGEIF(S) MAXIFS MINIFS`, `LARGE SMALL RANK.EQ RANK.AVG PERCENTILE.INC/.EXC QUARTILE.INC/.EXC MEDIAN MODE.SNGL MODE.MULT`, `STDEV.S/.P VAR.S/.P`, `CORREL COVARIANCE.S/.P PEARSON RSQ SLOPE INTERCEPT STEYX LINEST LOGEST TREND GROWTH FORECAST.LINEAR FORECAST.ETS`, `SKEW KURT DEVSQ AVEDEV GEOMEAN HARMEAN TRIMMEAN STANDARDIZE FISHER FISHERINV FREQUENCY PROB PERMUT PERMUTATIONA`, and the legacy names. | M | 2 |
| OF05-F060 | Distributions and tests (about 60) | `NORM.DIST/.INV/.S.DIST/.S.INV T.DIST(.2T/.RT) T.INV(.2T) CHISQ.DIST(.RT) CHISQ.INV(.RT) F.DIST(.RT) F.INV(.RT) BINOM.DIST BINOM.INV NEGBINOM.DIST POISSON.DIST EXPON.DIST GAMMA.DIST GAMMA.INV GAMMA GAMMALN BETA.DIST BETA.INV WEIBULL.DIST LOGNORM.DIST LOGNORM.INV HYPGEOM.DIST CONFIDENCE.NORM/.T T.TEST Z.TEST F.TEST CHISQ.TEST PHI GAUSS`. | S | 2 |
| OF05-F061 | Financial (about 55) | `PV FV PMT NPER RATE IPMT PPMT CUMIPMT CUMPRINC NPV XNPV IRR XIRR MIRR SLN SYD DB DDB VDB EFFECT NOMINAL DOLLARDE DOLLARFR PRICE YIELD DURATION MDURATION ACCRINT ACCRINTM COUPDAYBS COUPDAYS COUPDAYSNC COUPNCD COUPNUM COUPPCD DISC INTRATE RECEIVED PRICEDISC PRICEMAT YIELDDISC YIELDMAT TBILLEQ TBILLPRICE TBILLYIELD ODDFPRICE ODDFYIELD ODDLPRICE ODDLYIELD FVSCHEDULE ISPMT PDURATION RRI`. Day-count bases 0–4. | S | 2 |
| OF05-F062 | Information (about 25) | `CELL ERROR.TYPE INFO ISBLANK ISERR ISERROR ISEVEN ISODD ISFORMULA ISLOGICAL ISNA ISNONTEXT ISNUMBER ISREF ISTEXT N NA TYPE SHEET SHEETS`. `INFO` returns only non-identifying values (no user name, no path, no OS version). | M | 1 |
| OF05-F063 | Database (12) | `DAVERAGE DCOUNT DCOUNTA DGET DMAX DMIN DPRODUCT DSTDEV DSTDEVP DSUM DVAR DVARP`. | S | 2 |
| OF05-F064 | Engineering (about 55) | Base conversion `BIN2DEC … OCT2HEX`, bit functions `BITAND BITOR BITXOR BITLSHIFT BITRSHIFT`, complex `COMPLEX IMABS IMAGINARY IMARGUMENT IMCONJUGATE IMCOS IMDIV IMEXP IMLN IMLOG10 IMLOG2 IMPOWER IMPRODUCT IMREAL IMSIN IMSQRT IMSUB IMSUM`, `BESSELI/J/K/Y CONVERT DELTA ERF ERFC GESTEP`. | S | 2 |
| OF05-F065 | Matrix (10) | `MDETERM MINVERSE MMULT MUNIT TRANSPOSE SUMPRODUCT` plus `MSOLVE EIGEN SVD` as extensions under the `PBSD.` prefix. | S | 2 |
| OF05-F066 | External and web (5) | `WEBSERVICE FILTERXML ENCODEURL RTD DDE`. `ENCODEURL` and `FILTERXML` work offline. `WEBSERVICE` returns `#BLOCKED!` unless the workbook holds an external-data grant for that host. `RTD` and `DDE` always return `#BLOCKED!`. | M | 2 |
| OF05-F067 | Extension namespace | Functions not in OpenFormula or XLSX carry the prefix `PBSD.` and are written to ODS with the namespace `org.pbsd.sheets`. Other apps show `#NAME?` rather than a wrong value. | M | 2 |
| OF05-F068 | Function conformance table | In-app help lists every function with status (implemented, differs, not implemented) generated from the test corpus, so the claim is checkable. | M | 1 |

### 4.5 Calculation engine

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF05-F069 | Dependency graph | Cell and range nodes; ranges stored once and shared (range-tree index), so `SUM(A1:A1000000)` is one edge. Incremental: an edit dirties only its transitive dependants. | M | 1 |
| OF05-F070 | Multi-threaded recalculation | Dirty set is levelled by topological depth; each level runs across a fixed thread pool in the calc engine. No cell reads a cell at the same level. | M | 1 |
| OF05-F071 | Determinism | Same inputs, same options, same bytes out on any thread count. Rules: fixed evaluation order inside a cell; pairwise summation with a fixed tree shape for range aggregates; no fused multiply-add; no fast-math; own `libm`-independent implementations of `exp log pow sin cos tan` with stated error bounds. | M | 1 |
| OF05-F072 | Calculation modes | Automatic, automatic except data tables, manual (`F9` workbook, `Shift+F9` sheet, `Ctrl+Alt+F9` full rebuild). The mode is stored in the file and shown in the status bar. | M | 1 |
| OF05-F073 | Background and cancellable | Recalculation never blocks the UI. Stale cells render dimmed with a clock mark. A progress pill in the status bar offers Cancel; cancelling restores the last consistent state. | M | 1 |
| OF05-F074 | Number model: binary64 | IEEE 754 double, round-half-even in arithmetic, display rounding per format. 15 significant digits shown; full 17 on request. | M | 1 |
| OF05-F075 | Number model: decimal128 | Workbook option. All numbers and arithmetic use IEEE 754-2008 decimal128 (34 digits). `0.1+0.2=0.3` holds. Transcendental functions compute in decimal with a stated bound. Stored in ODS with an extension attribute; exported to XLSX as doubles with a warning. | S | 2 |
| OF05-F076 | Precision as displayed | Option to round stored results to the display format; shown as a permanent warning badge because it changes values. | C | 3 |
| OF05-F077 | Reproducibility report | Hash (BLAKE3) over all cell values after a full recalculation, with engine version, number model, seed and frozen clock. Two machines can compare one line. | S | 2 |
| OF05-F078 | Recalculation cost view | Per-sheet and per-cell time for the last recalculation; top 50 slowest formulas; volatile count. | S | 3 |
| OF05-F079 | Resource limits | Per-workbook caps: memory (default 4 GiB), cells materialised by spill (default 50 million), recursion depth, regex steps, string length (32,767 by default, 1 MiB max). Exceeding a cap gives `#CALC!` in that cell, not a crash. | M | 1 |
| OF05-F080 | Lazy evaluation | `IF IFS CHOOSE SWITCH IFERROR AND OR` evaluate only the branches needed. | M | 1 |
| OF05-F081 | Columnar fast paths | Aggregates, comparisons and arithmetic over typed column chunks run vectorised loops. | S | 2 |

### 4.6 Formula auditing

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF05-F082 | Trace precedents and dependants | Arrows drawn on the grid, several levels, across sheets (dashed arrow with a sheet icon). `Ctrl+[` and `Ctrl+]` jump. | M | 2 |
| OF05-F083 | Evaluate formula | Step dialog: evaluate the underlined term, step into a reference or lambda, step out. | M | 2 |
| OF05-F084 | Show formulas | Toggle to show formulas in cells (`` Ctrl+` ``). | M | 1 |
| OF05-F085 | Error checking | Background rules: inconsistent formula in a row or column, number stored as text, formula omits adjacent cells, unlocked formula cell, reference to empty cell, two-digit year text. Each rule can be turned off; a cell can be marked "ignore". | S | 2 |
| OF05-F086 | Watch window | Dockable list of watched cells with value, formula and sheet. | S | 2 |
| OF05-F087 | Dependency map | Sheet-level graph of which sheets feed which, drawn with `nodeGraph`; click an edge to list the cells. | C | 3 |
| OF05-F088 | Workbook inspection report | Counts of formulas, unique formulas, constants in formulas, errors, hidden sheets and rows, external links, very-hidden sheets, macros, cells with white-on-white text. | S | 2 |
| OF05-F089 | Explain formula | Plain-language description of the selected formula from a rule-based describer; an optional longer explanation through `svc.model` when `ai.local` is granted. The model receives the formula text and referenced headers only, never cell values unless the user ticks "include values". | C | 3 |

### 4.7 Formatting

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF05-F090 | Number formats | General, number, currency, accounting, date, time, percentage, fraction, scientific, text, custom format codes (ODF data styles and XLSX codes, four sections, colours, conditions, `?` alignment). | M | 1 |
| OF05-F091 | Font and alignment | Family, size, bold, italic, underline (single, double, accounting), strike, colour, super/subscript; horizontal and vertical alignment, indent, wrap, shrink to fit, rotation −90…90°, stacked text, text direction. | M | 1 |
| OF05-F092 | Borders and fill | Per-edge borders with 13 line styles, diagonal borders, solid and pattern fills, gradient fills. | M | 1 |
| OF05-F093 | Merge cells | Merge, merge across, unmerge, centre across selection (no merge). | M | 1 |
| OF05-F094 | Cell styles | Named styles with inheritance (ODF cell styles); style panel; changing a style restyles every cell that uses it. | M | 2 |
| OF05-F095 | Themes | Colour and font themes shared with OF04 and OF06; XLSX theme colours and tints mapped. | S | 2 |
| OF05-F096 | Format painter | Copy formats once or locked for several targets. | S | 1 |
| OF05-F097 | Rich text in cells | Mixed runs inside one text cell. | S | 2 |
| OF05-F098 | Conditional formatting: rules | Cell value comparisons, between, text contains, dates occurring, duplicates and unique, top and bottom N or percent, above and below average, blanks, errors, custom formula. Ordered rules with "stop if true". | M | 2 |
| OF05-F099 | Conditional formatting: visuals | Data bars (solid, gradient, negative axis), 2- and 3-colour scales, icon sets (20 sets), with colour-blind-safe defaults and an icon beside every colour state. | M | 2 |
| OF05-F100 | Conditional formatting manager | List of rules per sheet or selection, reorder, edit range, preview. Rules are evaluated only for visible cells plus a cache. | M | 2 |
| OF05-F101 | Sparklines | Line, column, win-loss in a cell, with axis and marker options. | S | 3 |
| OF05-F102 | Autofit and default sizes | Row height follows wrapped text; column autofit measures only the visible window plus a sample of 10,000 cells for large columns. | M | 1 |

### 4.8 Data tools

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF05-F103 | Sort | Up to 64 keys; by value, cell colour, font colour, icon; custom lists; case and locale collation options; natural sort; stable. Runs in the engine; 1 million rows target under 1 s. | M | 1 |
| OF05-F104 | AutoFilter | Per-column dropdown: value checklist with search, text, number and date conditions, top N, colour, blanks. Filtered row count in the status bar. | M | 1 |
| OF05-F105 | Advanced filter | Criteria range, copy results elsewhere, unique records only. | S | 2 |
| OF05-F106 | Slicers and timeline | Button panels bound to a table or pivot field; a date range bar. | S | 3 |
| OF05-F107 | Data validation | Whole number, decimal, list (range or literal), date, time, text length, custom formula; input message; error alert (stop, warn, inform); circle invalid data. | M | 2 |
| OF05-F108 | Subtotals | Insert grouped subtotals with outline levels. | S | 2 |
| OF05-F109 | Consolidate | Combine ranges by position or labels with a chosen function. | C | 3 |
| OF05-F110 | Pivot tables | Row, column, value and filter fields; functions sum, count, average, min, max, product, stdev, var, distinct count, median; show as (% of row, column, total, parent, running total, difference, rank); grouping of dates and numbers; calculated fields and items; sort and filter per field; layouts compact, outline, tabular; grand totals and subtotals; drill-down to a detail sheet; refresh. | M | 2 |
| OF05-F111 | Pivot sources | A range, a table, a named range, several ranges, or a query result imported from SQLite or Parquet (data stays columnar; not expanded into cells). | S | 2 |
| OF05-F112 | Pivot charts | A chart bound to a pivot; field buttons on the chart. | S | 3 |
| OF05-F113 | Goal seek | One target cell, one changing cell; bracketing plus Brent's method; shows iterations and the residual; Undo restores. | M | 2 |
| OF05-F114 | Solver | Objective (max, min, value), changing cells, constraints (`<= = >=`, integer, binary, all-different). Methods: simplex for linear models, branch and bound for integer, quasi-Newton and Nelder–Mead for smooth and non-smooth non-linear. Uses MathScript `optim` kernels where they fit (Verified to exist: `simplex_solver`, `nelder_mead`, `bfgs`, `lbfgs`, `minimize_with_constraints` in `MathScript:include/ms/optim/optim.hpp` per `MathScript:docs/API.md`). Reports: answer, sensitivity, limits. Deterministic for a fixed seed. | S | 3 |
| OF05-F115 | Scenarios | Named sets of input cell values; switch from a dropdown; scenario summary sheet; stored in ODS scenario tables. | S | 2 |
| OF05-F116 | Data tables | One- and two-variable what-if tables (`MULTIPLE.OPERATIONS` in OpenFormula, `TABLE()` in XLSX). | S | 3 |
| OF05-F117 | Descriptive statistics tool | A dialog that writes a block of summary statistics, histogram bins, correlation matrix, regression output or moving average as live formulas, not static numbers. | C | 3 |

### 4.9 Import, export and external data

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF05-F118 | CSV and TSV import dialog | Detects encoding (BOM, UTF-8 validity, then statistical guess), delimiter, quote and escape style, header row, line endings. The user can override each. Per-column type: text, number, date (with format), logical, skip. Preview of the first 1,000 rows and a sampled tail. **Default column type is "detect, but never alter text that only looks like a number"**: leading zeros, long digit strings (over 15 digits), gene names and codes such as `MARCH1` or `1E5` stay text unless the user picks a type. | M | 1 |
| OF05-F119 | Import profile | The chosen options are saved per file hash and as named profiles; re-importing asks nothing. | S | 2 |
| OF05-F120 | Malformed CSV handling | Ragged rows, unbalanced quotes, embedded NULs, mixed line endings and over-long fields are reported with row numbers in an import log sheet; the user chooses pad, truncate or stop. | M | 1 |
| OF05-F121 | Streaming import | Files larger than memory budget load in the helper as columnar chunks; the grid is usable when the first chunk arrives. Over 1,048,576 rows: offer split across sheets, load as a data source for pivots only, or hand off to OF09. | M | 2 |
| OF05-F122 | Fixed-width text import | Column breaks set by clicking a ruler over the preview. | S | 2 |
| OF05-F123 | JSON import | Array of objects to rows; nested objects to dotted column names; JSON Lines; a path picker for the array to load; depth limit 64. | S | 2 |
| OF05-F124 | SQLite import | Pick a table or view, or type a read-only `SELECT`; the file is opened read-only with defensive settings in the import helper; result arrives as columnar chunks. Link kept for refresh. | S | 2 |
| OF05-F125 | Parquet import and export | Read: column selection, row-group pruning, types mapped (INT96 and timestamps to date-time, decimals to decimal128 or double per number model). Write: one table or range. Compression: uncompressed, Snappy, Zstandard, gzip. | S | 3 |
| OF05-F126 | ODS native | ODF 1.3 spreadsheet, extended conformance; also Flat ODS (`.fods`) and templates (`.ots`). Package signatures (ODF XML-DSig) verified and shown; encryption read (AES-256-GCM/Argon2id of ODF 1.3 wholesome-encryption drafts are Unverified; AES-256-CBC/PBKDF2 of ODF 1.2 is the baseline) and written. | M | 1 |
| OF05-F127 | XLSX import and export | SpreadsheetML (ISO/IEC 29500 transitional and strict), `.xlsx .xlsm .xltx`. Cells, formulas, shared and array formulas, styles, themes, names, tables, validation, conditional formats, charts (DrawingML charts), pivots (cache rebuilt, not trusted), comments and threaded comments, images, freeze panes, print settings, protection flags. | M | 1 |
| OF05-F128 | Round-trip preservation | Parts the app does not understand (custom XML, unknown extensions, VBA binary `vbaProject.bin`) are carried through unchanged as opaque blobs when saving to the same format, and listed in the compatibility report. | S | 2 |
| OF05-F129 | Compatibility report | Before export to XLSX or CSV: a list of features that will be lost or changed (decimal128, `PBSD.` functions, macros, tracked changes detail), each with a "go to" link. | M | 1 |
| OF05-F130 | External links | Links to other workbooks are stored as portal tokens. On open they are **not** refreshed; cached values are used and a bar offers "Update links" per source file. | M | 2 |
| OF05-F131 | External data over the network | Off by default. A workbook connection names a host, a port and a path; the first fetch raises the shell capability prompt for that endpoint; grants are per workbook and listed in a Connections panel; fetches run in the network fetch helper, which has no file handles. Refresh is manual or on open after approval, never on a timer by default. | S | 3 |
| OF05-F132 | CSV export | Encoding, delimiter, quote rule, line ending, BOM, date and number format, "values as shown" or "raw values". | M | 1 |
| OF05-F133 | Formula-injection defence on CSV export | Option on by default: cells whose text starts with `= + - @`, tab or carriage return are prefixed with `'` (or wrapped and prefixed, per OWASP guidance) so another spreadsheet will not run them. A count is shown. | M | 1 |
| OF05-F134 | Formula-injection defence on CSV import | Nothing in a CSV is ever interpreted as a formula. Fields starting with `= + - @` load as text with a marker; "Convert to formula" is an explicit per-selection action that shows the formulas first. | M | 1 |
| OF05-F135 | Other exports | PDF (through the print pipeline), HTML table, Markdown table, JSON, PNG or SVG of a range or chart. | S | 2 |
| OF05-F136 | Legacy binary formats | `.xls` (BIFF8) import only, in the import helper, C priority; `.xlsb` not planned. | C | 3 |

### 4.10 Charts

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF05-F137 | Chart types | Column and bar (clustered, stacked, 100%), line, area (plain, stacked, 100%), pie, doughnut, scatter (XY), bubble, radar, stock (HLC, OHLC, with volume), candlestick, histogram, Pareto, box and whisker, waterfall, funnel, treemap, sunburst, heat map, surface and contour, combination (mixed types, secondary axis), Gantt (stacked bar preset). 3D variants limited to bar, pie, surface. | M | 2 |
| OF05-F138 | Chart elements | Title, axis titles, axes (linear, log, date, category, reversed, secondary), gridlines, legend, data labels, data table, error bars, trend lines (linear, polynomial 2–6, exponential, log, power, moving average) with equation and R², drop lines, high-low lines, up-down bars. | M | 2 |
| OF05-F139 | Chart editing | Select an element on the chart, edit in the inspector; drag to move and resize; drag coloured source ranges on the grid. | M | 2 |
| OF05-F140 | Recommended charts | Rule-based suggestion from the shape of the selection (one text column plus numbers, dates, two numeric columns). | S | 2 |
| OF05-F141 | Chart sheets and placement | Embedded on a sheet or on its own chart sheet. | S | 2 |
| OF05-F142 | Chart styles and templates | Theme-linked palettes, colour-blind-safe set, save a chart as a template. | S | 3 |
| OF05-F143 | Large data charts | Series over 100,000 points are decimated (min-max per pixel column) in the engine. | S | 2 |
| OF05-F144 | Accessible charts | Every chart has alt text, a generated text summary (type, series, min, max, trend) and a data-table view reachable from the keyboard. | M | 2 |
| OF05-F145 | Chart export and reuse | Copy as SVG, PNG or as a live object into OF04 and OF06 (linked or embedded with a snapshot of the data). | S | 2 |

### 4.11 Macros and custom functions

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF05-F146 | Custom functions in ForgeScript | A module exports pure functions with typed parameters (`number`, `text`, `logical`, `range`, `any`). They run in the macro host with **no** capabilities: no files, no network, no clock, no random source except the engine's seeded one. Pure functions take part in multi-threaded recalculation. | S | 2 |
| OF05-F147 | Macros in ForgeScript | Procedures that act on the workbook through a narrow object API (`workbook`, `sheet`, `range`, `selection`, `chart`, `ui.prompt`). Every change goes through the command layer, so one macro run is one Undo step. | S | 3 |
| OF05-F148 | Signing | Modules are signed (Ed25519 over BLAKE3, V14). Default policy: unsigned or unknown-signer code does not run. The trust bar shows signer, key fingerprint and what the module asks for. | M | 2 |
| OF05-F149 | Macro capabilities | A macro may request `fs.portal` (one picked file per call) or nothing. It never gets `net.*`, `proc.*` or `dev.*`. Requests are declared in the module header and shown before first run. | M | 3 |
| OF05-F150 | Macro limits | Instruction budget, memory cap 256 MiB, wall-clock cap with a Stop button, max cells written per run (setting). | M | 2 |
| OF05-F151 | Macro recorder | Records UI commands as readable ForgeScript calling the object API. | C | 3 |
| OF05-F152 | Script editor panel | `codeEdit` panel with completion from the object API, run, step, console, unit-test runner for custom functions. Deeper editing hands off to DV01. | S | 3 |
| OF05-F153 | Event hooks | `on_open`, `on_change(range)`, `on_before_save` hooks exist only for signed modules and are off until the user enables them per workbook. | C | 3 |
| OF05-F154 | VBA and other foreign code | Never executed. Presence is reported; source text can be viewed read-only when the storage can be parsed in the import helper. | M | 2 |

### 4.12 Review, protection and security

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF05-F155 | Comments | Threaded comments on cells with author, time, resolve; legacy notes; comments panel; print comments at end or in place. | M | 2 |
| OF05-F156 | Track changes | Record cell edits, row and column inserts and deletes, moves, sheet operations with author and time; accept or reject one or all; filter by author, date, range. ODS tracked-changes elements written; XLSX legacy shared-workbook revisions import only. | S | 3 |
| OF05-F157 | Compare workbooks | Cell-level diff of two files: values, formulas, formats, structure; side-by-side with change list; merge chosen changes. | S | 3 |
| OF05-F158 | Cell protection | Locked and hidden-formula flags per cell; protect sheet with a list of allowed actions; protect workbook structure; editable ranges. Stated plainly in the UI: this stops accidents, it is not encryption. | M | 2 |
| OF05-F159 | File encryption | Password-protected ODS and XLSX (ECMA-376 agile encryption, AES-256) read and write; key derivation in the I/O helper; passwords never stored unless the user saves them to `svc.keyring`. | S | 2 |
| OF05-F160 | Document signatures | Sign and verify ODF and OOXML package signatures with keys from SE04. Signature state in the trust badge. | S | 3 |
| OF05-F161 | Trust bar on open | For files from outside (provenance tag from `svc.portal.file`): states origin, macros present (disabled), external links (not updated), data connections (cut), hidden sheets, signatures. Each has an explicit enable. | M | 1 |
| OF05-F162 | Disposable open | "Open in a disposable sandbox" opens the file in an app instance with no `fs.portal` grant memory and no network, read-only, discarded on close. | S | 2 |
| OF05-F163 | Metadata control | Document properties panel; "Remove personal data on save" (author, last editor, revision times, printer name, hidden names, comments authors); export warns about hidden sheets and rows. | M | 2 |
| OF05-F164 | Version history | Local versions on each explicit save (kept under the app data directory, deduplicated), browsable; ZFS snapshots of the document shown where the dataset has them. | S | 2 |
| OF05-F165 | Redact before share | A share check that lists hidden content, comments, tracked changes, external links and personal metadata with one-click removal in a copy. | S | 3 |

### 4.13 Print, view and system integration

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF05-F166 | Page layout | Paper size, orientation, margins, scaling (percent, fit to N pages wide by M tall), print area, print titles (repeat rows and columns), gridlines, headings, page order, centre on page, first page number. | M | 2 |
| OF05-F167 | Headers and footers | Left, centre, right parts with fields (page, pages, date, time, file name, sheet name), different first and odd/even pages. | M | 2 |
| OF05-F168 | Page break preview | Drag page breaks on the grid; manual breaks; page numbers watermark. | M | 2 |
| OF05-F169 | Print preview and print | Preview pages exactly as the PDF spool will be; print through `svc.portal.print` (G-16). | M | 2 |
| OF05-F170 | Autosave and recovery | Journal of engine commands flushed every 2 s to the app data directory; after a crash the workbook reopens at the last command. Never writes to the user's file without Save unless "autosave in place" is on. | M | 1 |
| OF05-F171 | Templates | New from template (budget, invoice, timesheet, gradebook, loan schedule); user templates folder. | S | 2 |
| OF05-F172 | Search integration | Sheet names, table names, headers and text cells (first 64 KiB) offered to `svc.search` for workbooks in indexed folders. | S | 2 |
| OF05-F173 | Thumbnails | A thumbnailer entry renders the first sheet's top-left area for `svc.thumbnail`, inside the I/O helper. | S | 2 |
| OF05-F174 | Command palette and shortcut editor | Every action in the palette; shortcuts rebindable; optional keymaps matching common spreadsheet programs. | M | 1 |
| OF05-F175 | Embedding | A range or chart can be embedded in OF04 and OF06 as an ODF object with a rendered SVG fallback; "Edit in Sheets" opens it through the `edit` intent. | S | 3 |
| OF05-F176 | Help | Function reference, formula tutorial and import guide in `help/*.md`; `F1` on a function name opens its page. | M | 1 |

## 5. GUI

Layout pattern: document canvas + inspector (DESIGN_SYSTEM.md section 4). Default density: compact.

### 5.1 Window map

```
Sheets
├─ Workbook window (one per workbook view; tabs for several workbooks)
│  ├─ Header bar: menu, title, search / command palette, trust badge, net badge
│  ├─ Toolbar (ribbon-less, one row, overflow menu)
│  ├─ Formula bar: name box, function button, formula editor (expandable)
│  ├─ Sidebar (F9): Navigator | Find results | Comments | Changes | History | Connections
│  ├─ Grid (frozen panes, splits, row and column headers, outline margin)
│  ├─ Inspector (F4): Cell | Table | Chart | Pivot | Image | Sheet | Conditional rules | Validation
│  ├─ Bottom dock: Watch window | Script editor | Import log | Recalculation cost
│  ├─ Sheet tab bar
│  └─ Status bar
├─ Dialogs
│  ├─ Text import (CSV, TSV, fixed width)        ├─ Format cells
│  ├─ Data source import (JSON, SQLite, Parquet) ├─ Conditional formatting manager
│  ├─ Insert function                            ├─ Data validation
│  ├─ Name manager                               ├─ Sort
│  ├─ Create table                               ├─ Advanced filter
│  ├─ Create pivot table                         ├─ Goal seek
│  ├─ Insert chart                               ├─ Solver
│  ├─ Paste special                              ├─ Scenario manager
│  ├─ Go to / Go to special                      ├─ Evaluate formula
│  ├─ Page setup                                 ├─ Protect sheet / workbook
│  ├─ Print preview (full-window view)           ├─ Compare workbooks
│  ├─ Export compatibility report                ├─ Document properties and metadata
│  ├─ CSV export options                         ├─ Macro trust and signatures
│  └─ Preferences                                └─ Version history
├─ Popovers: filter dropdown, validation list, colour and border pickers, function tooltip,
│            comment bubble, status aggregate chooser, zoom, sheet list, error-check menu
├─ Sheets (inline bars): trust bar, external links bar, recalculation pending bar, read-only bar
└─ Start view (no workbook): recent, templates, import
```

### 5.2 Main window

Shown at 1280×800, wide layout, sidebar closed, inspector open.

```
+----------------------------------------------------------------------------------------+
| [=] Sheets — budget-2027.ods                 [search  Ctrl+K]      (1)[ok] [▁] [▢] [✕]  |
+----------------------------------------------------------------------------------------+
| (2) [Undo][Redo] | [Paint] | [Font v][11 v] [B][I][U] [A v][Fill v] | [≡L][≡C][≡R][Wrap] |
|     [Merge v] | [$][%][.0][.00][Format v] | [Σ v][Sort v][Filter][Table][Chart][Pivot] … |
+--------+---+----------------------------------------------------------------------------+
| (3) C7 v |fx | (4) =SUMIFS(Costs[Amount], Costs[Month], C$1, Costs[Kind], $A7)        [v] |
+---+----+---+--------+----------+----------+----------+----------+------+-----------------+
|(5)|    A        |    B     |    C     |    D     |    E     |  F   | (9) Inspector   |
+---+-------------+----------+----------+----------+----------+------+ [Cell][Sheet]    |
| 1 | Kind        | Budget   | Jan      | Feb      | Mar      |      |                 |
| 2 |-------------+----------+----------+----------+----------+------| Number          |
| 3 | Rent        | 1,400.00 | 1,400.00 | 1,400.00 | 1,400.00 |      | [Currency    v] |
| 4 | Power       |   180.00 |   201.15 |   176.40 |   169.02 |      | Decimals [2]    |
| 5 | Food        |   650.00 |   702.88 |   611.30 |   640.11 |      | Symbol [$ v]    |
| 6 | Transport   |   120.00 |    98.00 |   131.50 |   115.00 |      |                 |
| 7 | Other       |   200.00 |>[ 240.07]|   188.20 |   251.90 |      | Alignment       |
| 8 | Total       | 2,550.00 | 2,642.10 | 2,507.40 | 2,576.03 |      | [L][C][R] [T][M][B]|
| 9 |             |          |          |          |          |      | [x] Wrap text   |
|10 | (6) frozen row 1 and column A                           |      |                 |
|11 |             |          |          |          |          |      | Protection      |
|12 |             |          |          |          |          |      | [x] Locked      |
|   |             |          |          |          |          |      | [ ] Hide formula|
+---+-------------+----------+----------+----------+----------+------+-----------------+
| (7) [+] [≡]  < Budget | Costs | Chart1 | Notes >                              [◀ ▮▮▮ ▶] |
+----------------------------------------------------------------------------------------+
| (8) Ready · Auto calc · binary64      Sum 240.07  Avg 240.07  Count 1      [-] 100% [+] |
+----------------------------------------------------------------------------------------+
```

1. Trust badge: verified signature, unsigned, external origin, or tampered. Click opens the trust panel. The `[net]` badge appears only while a data connection is open.
2. Toolbar, 2 rows at compact density when the window is wide, 1 row with overflow `…` under 1100 px. Every button is an action from the shared action table.
3. Name box (`comboBox`, editable, 120 px): address, name or table. Dropdown lists names.
4. Formula editor: one line, grows to 8 lines with `Ctrl+Shift+U`. Syntax colours match range highlights on the grid.
5. Select-all corner, column headers (24 px high), row headers (48 px wide, grows with digits). Outline margin appears left of the row headers when groups exist.
6. Grid: the spreadsheet grid widget (OF05-GAP-1). Frozen panes have a 2 px `--line` edge. Active cell has a 2 px `--accent` border and a fill handle.
7. Sheet tab bar: add sheet, sheet list popover, scrollable tabs, horizontal scroll bar sharing the row.
8. Status bar: engine state (Ready, Calculating n%, Manual with "Calculate" link), calculation mode, number model, selection aggregates, zoom slider.
9. Inspector, 280 px, tabs change with the selection type.

States: empty workbook (grid with a hint line in A1 until first key), loading (skeleton grid, progress in status bar, usable when first chunk arrives), error (inline bar with Details), offline (no difference; connections show "not refreshed"), locked (encrypted file: password sheet before any content is parsed), read-only (bar "Read-only — Save a copy"; editing keys start nothing).

### 5.3 Other views and dialogs

**Text import dialog** (900×640). States: detecting, ready, error (unreadable encoding), huge file (sampled preview).

```
+-- Import text: sales-export.csv (412 MB, about 3.1 million rows) ------------------------+
| Encoding  [UTF-8 (detected) v]   Line ends [LF (detected) v]   Start at row [1  ]        |
| Separator (•) Comma ( ) Semicolon ( ) Tab ( ) Space ( ) Pipe ( ) Other [_]  ( ) Fixed    |
| Quote [" v]  Escape [doubled quote v]  [x] First row is header  [ ] Merge repeated seps  |
| Numbers: decimal [. v] thousands [, v]   Dates [detect: YYYY-MM-DD v]   Locale [en-AU v] |
| (1) [x] Keep anything ambiguous as text (leading zeros, long digits, codes like 1E5)     |
+------------------------------------------------------------------------------------------+
| (2)  [Text v]     [Date v]      [Number v]   [Text v]       [Skip v]    [Number v]      |
|      order_id     date          amount       sku            notes       qty             |
|      000418227    2026-03-01    1,204.50     MARCH1         =cmd|…      4               |
|      000418228    2026-03-01      88.00      1E5            ok          1               |
|      …            …             …            …              …           …               |
+------------------------------------------------------------------------------------------+
| (3) ⚠ 2 fields start with "=" and will load as text.  ⚠ 14 rows have 7 fields, not 6.    |
| (4) Rows exceed one sheet: (•) Load as data source  ( ) Split across 3 sheets  ( ) Open  |
|     in Data Studio                         Profile [none v] [Save profile…]              |
|                                                            [Cancel]  [Import 3.1 M rows] |
+------------------------------------------------------------------------------------------+
```

1. The safe-typing switch, on by default. 2. Preview `tableView` with a type `comboBox` in each header; the first 1,000 rows plus 200 sampled from the tail. 3. Warnings list, each clickable to the row. 4. Over-limit choice, shown only when the row count exceeds the sheet.

**Insert function** (720×520).

```
+-- Insert function ------------------------------------------------------------------+
| [search: lookup________]   Category [All v]                                           |
+----------------------------+----------------------------------------------------------+
| > XLOOKUP                  | XLOOKUP(lookup, lookup_range, return_range,              |
|   VLOOKUP                  |         [if_not_found], [match_mode], [search_mode])     |
|   HLOOKUP                  | Finds a value in one range and returns the matching      |
|   LOOKUP                   | item from another.            Conformance: XLSX addition |
|   XMATCH                   +----------------------------------------------------------+
|   MATCH                    | lookup        [$A7_______] [▦]   = "Other"               |
|   INDEX                    | lookup_range  [Costs[Kind]] [▦]   = {"Rent";"Power";…}    |
|   …                        | return_range  [Costs[Amount]][▦]  = {1400;201.15;…}       |
|                            | if_not_found  [0_________] [▦]   = 0                     |
+----------------------------+----------------------------------------------------------+
| Result: 240.07                                   [Help F1]   [Cancel]  [Insert]         |
+---------------------------------------------------------------------------------------+
```

`modelListView` on the left, `markdownView` help, `lineEdit` per argument with a range-pick button `[▦]` that shrinks the dialog to a bar while the user selects on the grid.

**Create pivot table and pivot inspector** (inspector tab, 280 px, shown beside the grid).

```
+-- Pivot: SalesByRegion --------+
| Source  Sales (table) [Change] |
| Fields  [search______]         |
| [x] region   [x] month         |
| [ ] sku      [x] amount        |
| [ ] rep      [ ] qty           |
+--------------------------------+
| Filters    | Columns           |
| [rep   v]  | [month (by qtr) v]|
+------------+-------------------+
| Rows       | Values            |
| [region v] | [Sum of amount v] |
|            | [% of row total v]|
+------------+-------------------+
| Layout [Compact v] [x] Totals  |
| [ ] Defer update    [Refresh]  |
+--------------------------------+
```

Fields drag between the four `listView` wells; `Alt+Arrow` moves the focused field between wells from the keyboard.

**Insert chart** (860×560): left `listView` of types with icons, centre live preview (`chart` widget, OF05-GAP-2), right options (`propertyGrid`): data range, series in rows or columns, first row and column as labels, title, axes, legend. A "Recommended" group heads the type list.

```
+-- Insert chart -------------------------------------------------------------------------+
| Recommended     |                                              | Data range [A1:E8___][▦] |
| > Clustered col |      Budget against actual                   | Series in (•)cols ( )rows|
|   Line          |  1500 ┤ ██                                   | [x] First row as labels  |
| All types       |  1000 ┤ ██ ▒▒                                | [x] First col as labels  |
|   Column  Bar   |   500 ┤ ██ ▒▒ ██ ▒▒ ██ ▒▒                    | Title [Budget against…_] |
|   Line    Area  |     0 ┼─Rent──Power─Food──…                  | Legend [Bottom v]        |
|   Pie     XY    |        ■ Budget ▒ Jan                        | Axis Y [Linear v]        |
|   Stock   Radar |                                              | [x] Alt text generated   |
|   Box  Histogram|                                              | Place (•)This sheet ( )New|
+-----------------+----------------------------------------------+--------------------------+
|                                                              [Cancel]  [Insert chart]    |
+------------------------------------------------------------------------------------------+
```

**Trust bar and macro trust panel** (inline bar under the toolbar, then a 560×420 panel).

```
+------------------------------------------------------------------------------------------+
| ⚠ From outside this computer (mail attachment, 12 Nov). Macros: 2 modules, unsigned,     |
|   disabled. External links: 3, not updated. Connections: 1, cut.   [Review…]  [Dismiss]  |
+------------------------------------------------------------------------------------------+
+-- Review: forecast.xlsm ------------------------------------------+
| Origin        Mail (CO01), received 12 Nov 2026 09:14              |
| Signature     none                                                 |
| Macros        vbaProject.bin (VBA, 41 KB) — never runs  [View text]|
|               helpers.fs (ForgeScript, unsigned)        [View] [ ] |
| External links  ..\rates.xlsx (3 cells)        [Pick file…] [Keep] |
| Connections   https://example.org/rates.csv    [Allow…]   [Remove] |
| Hidden        1 very-hidden sheet "cfg"                   [Show]   |
| Metadata      author, last saved by, printer              [Strip]  |
+--------------------------------------------------------------------+
| [Open in disposable sandbox]              [Close]  [Apply choices] |
+--------------------------------------------------------------------+
```

**Evaluate formula** (640×360): the formula in a `codeEdit` (read-only) with the next term underlined; buttons `[Evaluate]`, `[Step in]`, `[Step out]`, `[Restart]`; a `treeView` of the call stack on the right.

**Solver** (680×560): objective `lineEdit` with range picker, `( ) Max ( ) Min ( ) Value of [__]`, changing cells, constraints `tableView` with Add, Change, Delete, method `comboBox`, options (tolerance, iterations, seed, time limit), `[Solve]`, and a results sheet offering Keep, Restore, and the three reports.

**Page break preview** (full grid mode): the grid at reduced zoom with blue page borders that can be dragged, page numbers drawn large behind cells, a right inspector tab "Page" with size, orientation, scaling and print titles.

**Conditional formatting manager** (760×480): scope `comboBox` (selection, this sheet), `tableView` of rules (preview swatch, rule text, applies-to range, stop-if-true check box), `[New rule…] [Edit…] [Delete] [▲][▼]`.

**Compare workbooks** (full window): two grids side by side with a centre change list (`treeView` grouped by sheet: values, formulas, formats, structure), filter chips, `[Take left] [Take right]`.

### 5.4 Menus

```
File    New (Ctrl+N) · New from template… · Open… (Ctrl+O) · Open recent ▸ · Open in disposable sandbox…
        Import ▸ Text/CSV… · JSON… · SQLite… · Parquet… · Fixed width…
        Save (Ctrl+S) · Save as… (Ctrl+Shift+S) · Save a copy… · Export ▸ XLSX… · CSV… · PDF… · HTML… · JSON… · Parquet…
        Versions… · Properties… · Check before sharing… · Page setup… · Print preview (Ctrl+F2) · Print… (Ctrl+P)
        Close (Ctrl+W) · Quit (Ctrl+Q)
Edit    Undo (Ctrl+Z) · Redo (Ctrl+Shift+Z) · History…
        Cut · Copy · Paste · Paste special… (Ctrl+Shift+V) · Paste values (Ctrl+Alt+V)
        Fill ▸ Down (Ctrl+D) · Right (Ctrl+R) · Series… · Flash fill (Ctrl+E)
        Clear ▸ Contents (Delete) · Formats · Comments · All
        Delete cells… (Ctrl+-) · Find… (Ctrl+F) · Replace… (Ctrl+H) · Go to… (Ctrl+G) · Go to special…
        Select ▸ All (Ctrl+A) · Row (Shift+Space) · Column (Ctrl+Space) · Current region (Ctrl+Shift+*) · Visible cells (Alt+;)
        Preferences… (Ctrl+,)
View    Normal · Page break preview · Show formulas (Ctrl+`) · Gridlines · Headings · Formula bar
        Freeze ▸ Rows and columns at cell · Top row · First column · Unfreeze
        Split · New window · Side by side · Zoom ▸ · Full screen (F11)
        Sidebar (F9) ▸ Navigator · Comments · Changes · History · Connections · Inspector (F4) · Watch window · Script editor
Insert  Cells… (Ctrl++) · Rows above · Columns left · Sheet (Shift+F11)
        Table… (Ctrl+T) · Pivot table… · Chart… (Alt+F1) · Sparkline… · Function… (Shift+F3) · Name ▸ Define… (Ctrl+F3 opens manager)
        Comment (Ctrl+Alt+M) · Link… (Ctrl+L) · Image… · Shape ▸ · Drawing (OF08)… · Checkbox · Dropdown · Date and time ▸ (Ctrl+; / Ctrl+Shift+;)
Format  Cells… (Ctrl+1) · Number ▸ · Bold (Ctrl+B) · Italic (Ctrl+I) · Underline (Ctrl+U)
        Row ▸ Height… · Autofit · Hide · Show    Column ▸ Width… · Autofit · Hide · Show
        Merge ▸ · Styles… · Theme ▸ · Conditional formatting ▸ · Clear direct formatting (Ctrl+M)
Data    Sort… · Filter (Ctrl+Shift+L) · Advanced filter… · Clear filters · Slicer…
        Validation… · Text to columns… · Remove duplicates… · Trim spaces · Subtotals… · Consolidate…
        Group (Alt+Shift+Right) · Ungroup (Alt+Shift+Left) · Auto outline
        Goal seek… · Solver… · Scenarios… · Data table… · Statistics…
        Connections… · Refresh ▸ This source · All (asks per source)
Formulas  AutoSum (Alt+=) · Calculate now (F9) · Calculate sheet (Shift+F9) · Rebuild all (Ctrl+Alt+F9) · Mode ▸
        Trace precedents (Ctrl+[) · Trace dependants (Ctrl+]) · Remove arrows · Evaluate… · Error checking… · Watch cell
        Name manager (Ctrl+F3) · Create names from selection (Ctrl+Shift+F3) · Reproducibility report… · Recalculation cost
Review  Spelling (F7) · Comments ▸ · Track changes ▸ Record · Accept or reject… · Compare with…
        Protect sheet… · Protect workbook… · Editable ranges… · Encrypt with password… · Signatures…
Tools   Macros ▸ Run… (Alt+F8) · Record · Script editor (Alt+F11) · Trust and signatures…
        Workbook inspection… · Explain formula
Help    Sheets help (F1) · Function reference · Conformance table · Keyboard shortcuts · About
```

### 5.5 Keyboard and pointer

App-specific keys beyond section 6 of DESIGN_SYSTEM.md: `F2` edit cell (the reserved "rename" meaning, applied to a cell); `F4` while editing a formula cycles reference type (inspector toggle applies only outside edit mode); `Ctrl+Enter` fill selection; `Ctrl+Shift+Enter` legacy array; `Alt+Down` column value list or filter dropdown; `Ctrl+Space`/`Shift+Space` column and row; `Ctrl+Shift+Arrow` extend to edge; `Ctrl+.` next corner of selection; `Ctrl+Backspace` scroll to active cell; `Ctrl+9`/`Ctrl+0` conflict with zoom reset, so hide row and column are `Ctrl+Alt+9` and `Ctrl+Alt+0`; `Ctrl+Shift+1…6` number format presets; `Ctrl+;` today, `Ctrl+Shift+;` now; `Alt+Enter` line break in cell; `Ctrl+PageUp/PageDown` previous and next sheet.

Context menus: cell (cut, copy, paste, paste special, insert, delete, clear, filter by value, sort, comment, format cells, define name, link, trace); row and column header (insert, delete, hide, size, group, freeze here); sheet tab (insert, delete, rename, move or copy, colour, protect, hide, select all sheets); chart element; pivot cell (field settings, show as, sort, expand, drill down); status bar (aggregates to show).

Drag and drop: drag a selection border to move, with `Ctrl` to copy, with `Shift` to insert; drag the fill handle; drag column and row edges; drag sheet tabs to reorder or out to a new window; drop files on the window (CSV, ODS, XLSX, JSON, Parquet, SQLite open the right import path); drop an image onto a cell; drag a range out as TSV, HTML or PNG; drag pivot fields between wells.

Pointer: wheel scrolls rows, `Shift+wheel` columns, `Ctrl+wheel` zoom; touchpad pinch zoom; double-click a border to jump to the data edge; double-click a column edge to autofit; middle-drag pans.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Start view: recent workbooks (`gridView` of thumbnails), templates, "Import data…", "Blank workbook". |
| Loading | Skeleton grid, determinate bar; first chunk interactive; Cancel closes cleanly. |
| Calculating | Stale cells dimmed; status pill with percent and Cancel; editing stays possible and queues. |
| Error | Parse failure: a report dialog with the part and offset, offers "Recover what can be read" (runs in the helper, lists what was dropped). Engine crash: the engine restarts and replays the journal; a toast says so. |
| Offline | Normal. Connections show last refresh time. |
| Locked | Password sheet for encrypted files; three wrong tries add a 2 s delay each; nothing parsed before the key verifies. |
| Read-only | Bar with reason (no write right on the handle, protected, disposable, snapshot version). |
| Narrow (<640) | Toolbar collapses to overflow; inspector becomes a bottom sheet; formula bar stays; sheet tabs become a popover list. |

### 5.7 Widgets

| Region | FUSE widget | Notes |
|---|---|---|
| Header, toolbar | `panel`, `toolButton`, `comboBox`, `menuBar`, `popupMenu` | Chrome kit from FUSE-WP-29 when it exists |
| Name box | `comboBox` (editable) | |
| Formula editor | `codeEdit` | Needs a custom grammar and inline completion popup (limits listed in FUSE build plan 2.2) |
| Grid | **OF05-GAP-1** | Not `tableView`: that widget has single-row selection, no cell selection, no inline editors, no frozen panes, no merged cells and a `u32` row count (Verified, `FUSE:Source/FUSE/UI/Widgets/include/fuse/ui/widgets/widgets.hpp`, summarised in FUSE build plan 2.2) |
| Sheet tabs | `tabBar` | Needs inline rename and colour stripe |
| Inspector | `tabWidget`, `propertyGrid`, `colorPicker`, `fontPicker`, `spinBox`, `checkBox` | |
| Sidebar navigator | `treeView` | Needs model-backed trees beyond 64 collapsible rows (FUSE-WP-28) |
| Import preview, rule lists | `tableView` | |
| Charts | **OF05-GAP-2** | `chart` today draws line, bar, area and scatter over a shared index with `f32` values; no real axes, legend, pie or stacking (Verified, same header) |
| Dependency map | `nodeGraph` | read-only use |
| Script editor | `codeEdit` | |
| Help | `markdownView` | Needs tables and links (FUSE-WP-28 era work) |
| Dialogs, sheets, toasts | `dialog`, overlays of FUSE-WP-30 | |

Gaps:

- **OF05-GAP-1 Spreadsheet grid widget.** A virtualised 2D cell view with: 64-bit row index; variable row and column sizes with O(log n) offset lookup; frozen and split viewports; cell, range and multi-range selection; merged cells; text overflow into empty neighbours; in-place editor hosting; fill handle; overlay layers (range highlights, trace arrows, page breaks, floating objects); header rendering; pull model that asks the app for a rectangle of display cells and never holds the data. Related to G-12 only for floating objects.
- **OF05-GAP-2 Chart widget v2.** Axes (linear, log, date, category), legends, stacking, pie and polar, box, heat map, labels, hit testing of elements, SVG and PDF output through the draw-list path API (FUSE-WP-23), a text alternative for a11y.
- **OF05-GAP-3 Range-pick mode for dialogs.** A dialog can collapse to a bar and receive a selection from another widget in the same window.
- G-10 for rich text in cells and comments; G-16 for print; G-17 for spelling; G-03 for complex scripts in cells.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Commands, view models, dialogs, selection state, undo stack front, settings, intents |
| UI documents and styles | FUSE UI (`.fuseui`, CSS) | app host | Windows, panels, dialogs |
| Grid adapter | ForgeScript + typed binding | app host | Answers the grid widget's "give me display cells for this rectangle" from a cache filled by the engine |
| Calc engine | C++23 | helper `calc-engine` | Workbook model, formula parser and compiler to a tree, dependency graph, recalculation, sort, filter, pivot cache, aggregates, command journal. Justification: speed (PLATFORM.md 2b). No file or network handles; receives chunks over IPC and shared memory |
| Sheet I/O helper | C++23 | helper `sheet-io` | Parses and writes ODS, FODS, XLSX, XLS, CSV, JSON, Parquet, SQLite; thumbnails; PDF page layout. Justification: untrusted bytes (2a). Gets only the file handles passed for one job |
| Macro host | C++23 embedding `forge::script::engine` | helper `macro-host` | Runs signed custom functions and macros with an empty capability set and budgets |
| Network fetch helper | C++23 | helper `data-fetch` | Fetches a granted URL over TLS and streams bytes to `sheet-io`. Started only when a connection is refreshed. No file handles |
| Chart layout | ForgeScript (layout) + FUSE draw list | app host | Turns a chart definition plus decimated series into draw commands |
| Solver kernels | C++23 (MathScript `optim`, own branch and bound) | inside `calc-engine` | Goal seek and solver |

```
                 +-------------------- app host ---------------------+
 user input ---> | FUSE UI  <->  ForgeScript app logic  <->  grid     |
                 |                  |   ^                    adapter  |
                 +------------------|---|------------------------^----+
                     commands (IPC) |   | change sets, cell windows (shared memory)
                                    v   |
                              +---------------+   pure calls   +------------+
                              |  calc-engine  | <------------> | macro-host |
                              +---------------+                +------------+
                                 ^ columnar chunks (shared memory)
                                 |
   file handle (portal) --> +----------+ <--- bytes --- +------------+ <--- TLS --- granted host
                            | sheet-io |                | data-fetch |
                            +----------+                +------------+
```

### 6.2 Data model

Held in the calc engine. The on-disk native format is ODS; this is the in-memory model and the journal schema.

| Entity | Fields |
|---|---|
| `Workbook` | `id: uuid`, `sheets: [SheetId]`, `names: [DefinedName]`, `styles: StyleTable`, `theme: Theme`, `calc: CalcSettings`, `number_model: enum{binary64, decimal128}`, `seed: u64`, `frozen_clock: i64?`, `date_epoch: enum{1899_12_30, 1900, 1904}`, `connections: [Connection]`, `modules: [ScriptModule]`, `opaque_parts: [Blob]`, `revision: u64` |
| `Sheet` | `id: u32`, `name: text`, `kind: enum{grid, chart}`, `visibility: enum{visible, hidden, very_hidden}`, `columns: [ColumnChunkSet]`, `row_heights: SizeTree`, `col_widths: SizeTree`, `merges: IntervalSet`, `freeze: (row, col)`, `tables: [Table]`, `cond_formats: [CondRule]`, `validations: [Validation]`, `filters: FilterState`, `outline: OutlineTree`, `objects: [FloatingObject]`, `protection: SheetProtection`, `page: PageSetup` |
| `ColumnChunk` | `col: u16`, `first_row: u32`, `kind: enum{empty, f64, d128, text_ref, bool, mixed}`, `values: bytes`, `formula_ids: [u32]?`, `style_runs: [(row_count, style_id)]` |
| `Formula` | `id: u32`, `ast: bytes` (compact tree), `origin: CellRef`, `shared_count: u32`, `volatile: bool`, `flavour_text: text?` |
| `DepNode` | `id: u64`, `kind: enum{cell, range, name, table_col}`, `level: u32`, `dirty: bool`, `out_edges: [u64]` |
| `DefinedName` | `name: text`, `scope: SheetId?`, `expr: FormulaId`, `comment: text`, `hidden: bool` |
| `Table` | `name: text`, `range: Range`, `columns: [TableColumn{name, total_fn, calc_formula}]`, `style: text`, `has_header: bool`, `has_totals: bool` |
| `PivotTable` | `name`, `source: PivotSource`, `cache_id: u32`, `rows/cols/values/filters: [PivotField]`, `layout`, `location: CellRef` |
| `Chart` | `id`, `type`, `series: [Series{name_ref, x_ref, y_ref, type, axis}]`, `axes`, `title`, `legend`, `alt_text`, `anchor` |
| `CondRule` | `ranges: [Range]`, `priority: u32`, `kind`, `params`, `style_id`, `stop_if_true: bool` |
| `Comment` | `id: uuid`, `cell: CellRef`, `author: text`, `time: i64`, `body: RichText`, `parent: uuid?`, `resolved: bool` |
| `Change` | `seq: u64`, `author`, `time`, `command: bytes`, `inverse: bytes`, `accepted: enum{pending, accepted, rejected}` |
| `ScriptModule` | `name`, `source: text`, `sig: bytes?`, `signer_fpr: text?`, `requests: [cap]`, `exports: [FnSig]` |
| `Connection` | `id`, `kind: enum{file, sqlite, parquet, url}`, `target: portal_token or (host, port, path)`, `last_refresh: i64`, `sha256: bytes`, `refresh: enum{manual, ask_on_open}` |

Stores:

| Store | Path | Format |
|---|---|---|
| Recovery journal | `~/.local/share/org.pbsd.sheets/recovery/<workbook-id>.journal` | Length-prefixed command records, BLAKE3 chain, schema version in the header |
| Local versions | `~/.local/share/org.pbsd.sheets/versions/` | Content-addressed ODS packages plus `versions.sqlite` (`id, doc_id, time, size, blake3, note`) |
| Import profiles, recent list, custom lists | `~/.local/share/org.pbsd.sheets/app.sqlite` | SQLite, `schema_version` table, forward-only migration with a copy kept |
| Trusted signers | `svc.keyring` (public keys) and `app.sqlite` (`fingerprint, name, added, scope`) | |
| Settings | `svc.settings`, namespace `org.pbsd.sheets` | |

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| ODS, OTS, FODS (ODF 1.3) | yes | yes | Own ZIP reader and writer, own strict XML reader (no DTD, no external entities, depth and size limits); zlib (zlib licence) for inflate and deflate | `sheet-io` |
| OpenFormula text | yes | yes | Own parser | `calc-engine` (input already length-limited) |
| XLSX, XLSM, XLTX (ISO/IEC 29500) | yes | yes | Same ZIP and XML code; own SpreadsheetML mapping | `sheet-io` |
| XLS (BIFF8, CFB container) | yes (C) | no | Own code | `sheet-io` |
| CSV, TSV, fixed width | yes | yes | Own code (RFC 4180 plus dialects) | `sheet-io` |
| JSON, JSON Lines | yes | yes | Own code, depth limit 64 | `sheet-io` |
| SQLite database | yes | no | SQLite (public domain), opened read-only with defensive config and no extension loading | `sheet-io` |
| Parquet | yes | yes | Own reader and writer for the Thrift compact footer and the encodings (plain, dictionary, RLE, delta); Zstandard (BSD-3-Clause) and own Snappy and gzip codecs. Apache Arrow C++ is not used: too large to vendor and audit | `sheet-io` |
| PDF (export) | no | yes | Platform print pipeline (G-16) | `sheet-io` writes page descriptions; spooler renders |
| PNG, JPEG, SVG (images on sheets) | yes | yes (PNG, SVG) | Platform image pipeline (FUSE-WP-27) | thumbnail or image helper of the platform |
| ODF and OOXML package signatures | verify | sign | Platform crypto through SE04 key handles | `sheet-io` (verify), SE04 (sign) |
| ODF and ECMA-376 encryption | yes | yes | Platform crypto (FORGE OS layer keystore primitives; exact API Unverified) | `sheet-io` |
| HTTPS (external data) | yes | no | Platform TLS (library choice is a platform decision; Unverified) | `data-fetch` |
| Engine IPC | — | — | Session bus schema `org.pbsd.sheets.engine/1` (G-08) | — |

### 6.4 Integration

- Intents handled: `open`, `edit`, `view`, `new`, `print`. Emitted: `share`, `print`, `open` (links, hand-off to OF09 for over-limit data, OF08 for drawings, DV01 for scripts).
- MIME view/edit: `application/vnd.oasis.opendocument.spreadsheet`, `…spreadsheet-template`, `…spreadsheet-flat-xml`. Import/export: `application/vnd.openxmlformats-officedocument.spreadsheetml.sheet`, `application/vnd.ms-excel.sheet.macroEnabled.12`, `text/csv`, `text/tab-separated-values`, `application/json`. Import only: `application/vnd.ms-excel`, `application/vnd.sqlite3`, `application/vnd.apache.parquet`. Export only: `application/pdf`, `text/html`, `text/markdown`.
- Services: `svc.settings`, `svc.notify` (long imports and solver runs), `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.search`, `svc.clipboard`, `svc.thumbnail`, `svc.keyring`, `svc.audit`, `svc.a11y`, `svc.backup`, `svc.model` (optional).
- IPC offered: `org.pbsd.sheets.embed/1` — render a range or chart of a workbook handle to SVG for OF04 and OF06 (Proposed).
- Other apps: OF04 and OF06 embed ranges and charts; OF08 supplies drawings; OF09 takes over-limit data and SQL; OF07 shares nothing at run time; SE04 holds signing keys; AD05 lists per-workbook network grants; UT06 is behind the print portal.
- Backing repos. Verified: FUSE `tableView`, `chart`, `codeEdit`, `SqliteModel` exist with the limits quoted in 5.7 (`FUSE:docs/ui/README.md`). Verified: ForgeScript signing (PLATFORM.md V14) and that `jit_threshold` compiles to bytecode only (`FORGE:script/src/runtime.cpp:729-730`). Verified: MathScript has in-tree statistics, distributions, special functions, finance and optimisation kernels with `Result<T>` errors and no external BLAS (`MathScript:README.md`, `MathScript:docs/API.md`). Proposed: everything in this spec that uses them.

## 7. Security and privacy

Capabilities requested:

| Capability | Why |
|---|---|
| `fs.app` | Journal, versions, profiles |
| `fs.portal` | Open and save workbooks, imports, linked files |
| `net.none` | Default state. The app host never opens a socket |
| `net.user` (optional, off) | Only for the `data-fetch` helper when the user approves a workbook connection; each endpoint is prompted and logged |
| `proc.helper:calc-engine` | Calculation |
| `proc.helper:sheet-io` | Parsing and writing files |
| `proc.helper:macro-host` | Running signed ForgeScript modules |
| `proc.helper:data-fetch` | External data |
| `dev.print` | Printing |
| `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.search`, `svc.clipboard`, `svc.thumbnail`, `svc.keyring`, `svc.audit`, `svc.a11y`, `svc.backup` | As in 6.4 |
| `ai.local`, `svc.model` (optional, off) | Explain formula |

Assets: workbook contents (often financial or personal), file passwords, signing trust list, the user's other files and network (which a hostile workbook wants to reach).

Adversaries: a crafted file (ZIP bomb, XML entity expansion, malformed Parquet, corrupt SQLite page); a workbook with code (macros, DDE, `WEBSERVICE`, external links to exfiltrate data through URLs); a CSV built to run formulas in the next program; a hostile custom function trying to read other cells' data and leak it; a remote data source returning hostile bytes.

Attack surface: the I/O helper's parsers; the formula parser; the macro host; the fetch helper; clipboard and drag payloads; the embed IPC.

Mitigations:

| Threat | Mitigation |
|---|---|
| Parser memory corruption | Parsers only in `sheet-io`, C++23 safe subset, capability mode, one job per process, no network, output is a validated columnar stream checked again by the engine |
| Decompression and expansion bombs | Limits: 4 GiB expanded per package, ratio 200:1 per entry, 10 million shared strings, XML depth 256; no DTD processing at all |
| Code in files | VBA, DDE, OLE, ActiveX, Excel 4 macros never execute; `RTD`, `DDE` return `#BLOCKED!`; ForgeScript modules need a trusted signature |
| Data exfiltration by formula | No function reaches the network without a per-workbook, per-host grant drawn by the shell; `HYPERLINK` needs a click and shows the full URL; images are embedded, never fetched from URLs on open |
| Exfiltration by macro | Macro host has no `net`, `proc`, `env`; custom functions see only their arguments; macros see the workbook they belong to |
| CSV formula injection | Import never makes formulas; export escapes by default |
| Tampered journal or version | BLAKE3 chain; mismatch offers the last good point |
| Fake protection | UI states that sheet protection is not encryption |
| Metadata leaks | Strip-on-save option, share check |
| Denial of service by huge formulas | Budgets with `#CALC!`; engine restart keeps the journal |

Stored: journal, versions, profiles, recent list (portal tokens, not paths) under the app directories. Passwords only in `svc.keyring` and only on request. Leaves the machine: nothing by default; granted connection requests only (URL and standard headers, no identifiers), listed in `svc.audit`.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `calc.mode` | enum | `auto` | Default mode for new workbooks |
| `calc.threads` | int | `0` | 0 means all cores less one |
| `calc.number_model` | enum | `binary64` | For new workbooks |
| `calc.memory_cap_mib` | int | `4096` | Engine memory limit per workbook |
| `calc.iteration.enabled` | bool | `false` | |
| `calc.iteration.max` / `.epsilon` | int / float | `100` / `0.001` | |
| `calc.lambda_depth` | int | `1024` | |
| `formula.r1c1` | bool | `false` | Reference style |
| `formula.localised_names` | bool | `false` | Show function names in the UI language |
| `formula.xlsx_semantics` | bool | `false` | Default for new workbooks; imported XLSX files set it on |
| `edit.enter_moves` | enum | `down` | `down`, `right`, `none` |
| `edit.autocomplete` | bool | `true` | |
| `edit.recognise_dates` | bool | `true` | Typed input only; never imports |
| `import.csv.safe_typing` | bool | `true` | Keep ambiguous values as text |
| `import.csv.max_preview_rows` | int | `1000` | |
| `export.csv.escape_formulas` | bool | `true` | Formula-injection defence |
| `export.csv.encoding` / `.bom` | enum / bool | `utf-8` / `false` | |
| `export.default_format` | enum | `ods` | Never a proprietary default |
| `security.macros` | enum | `signed_trusted` | `never`, `signed_trusted`, `ask_signed` |
| `security.links_on_open` | enum | `never` | `never`, `ask` |
| `security.strip_metadata_on_save` | bool | `false` | |
| `recovery.journal_interval_ms` | int | `2000` | |
| `versions.keep` | int | `50` | Local versions per document |
| `view.density` | enum | `compact` | |
| `view.gridlines` / `.zero_values` / `.formula_bar` | bool | `true` | |
| `ai.explain_formula` | bool | `false` | Needs `ai.local` |
| `keys.profile` | enum | `pbsd` | Optional alternative keymaps |

## 9. Accessibility and languages

- The grid exposes a table role with row and column headers; the screen reader hears address, value, then "has formula", "has comment", "invalid", "filtered" as states. Needs table and cell roles in the a11y model (FUSE build plan 2.2 lists them as missing).
- Every pointer action has a key route: range picking by typing or arrows, moving pivot fields, resizing columns (`Alt+O` style menus plus a "Column width…" dialog), dragging chart elements (arrow keys with the element selected).
- Conditional colours always pair with an icon or data bar option; default palettes are colour-blind safe.
- Charts carry generated text summaries and a data table.
- Right-to-left sheets (column A on the right), per-cell text direction, bidi in cells (G-03). Locale-aware number, date and list separators; function names English unless the setting says otherwise.
- Announcements: recalculation finished, filter applied with row count, import warnings count.

## 10. Performance budgets

| Measure | Target |
|---|---|
| Cold start to empty grid | 400 ms |
| Open 10 MB ODS or XLSX | 2 s to first interactive chunk |
| CSV import throughput | 150 MB/s parse in the helper |
| Largest workbook | 100 million non-empty cells within an 8 GiB cap; larger data stays as a columnar source |
| Scroll frame time | under 8 ms for a 4K viewport of text cells |
| Edit one cell with 10,000 dependants | under 50 ms to consistent display |
| Full recalculation, 1 million simple formulas | under 500 ms on 8 cores |
| Sort 1 million rows by 2 keys | under 1 s |
| Idle | zero frames, zero wake-ups; engine threads parked |
| App host memory, empty workbook | under 80 MiB; engine under 40 MiB |

## 11. Testing and proof

Test plan (targets; nothing has run):

- **Unit**: every function against a table of inputs and expected outputs; parser round trip `parse(print(ast)) == ast`; size-tree and interval-set structures.
- **Conformance corpus**: OpenFormula test cases from the ODF specification text; a function conformance table generated from the run.
- **Differential**: the same workbooks evaluated by Sheets and by LibreOffice Calc (run outside the product, as a test oracle only) with stated tolerances; mismatches triaged into "we differ on purpose" (documented) or bugs.
- **Determinism**: every corpus workbook recalculated with 1, 2, 7 and 64 threads and on two CPU families; the value hash must match.
- **Round trip**: ODS → model → ODS and XLSX → model → XLSX compared at the XML level after normalisation; ODS ↔ XLSX loss list checked against the compatibility report.
- **Fuzz**: ZIP, XML, SpreadsheetML, ODS content, CSV, JSON, Parquet, BIFF, formula text, journal records, engine IPC messages. Structure-aware fuzzers for formulas.
- **Golden renders**: grid with every border and format, each chart type, each dialog `.fuseui`, at 100% and 200%, light, dark and high contrast.
- **Scripted UI**: the 15 top tasks through the a11y tree.
- **Security tests**: bomb corpus; a workbook set that tries each exfiltration route and must produce `#BLOCKED!` and an audit line; CSV injection corpus in both directions.
- **Performance**: the budgets of section 10 as benchmarks with fixed data sets.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Size tree, interval set, range tree | Functional correctness against a simple model | T1 | PRISM proof of the invariants and of lookup results | Nothing known; small pure code |
| Dependency levelling | No cell is evaluated before its precedents; acyclic schedule | T1 | Proof over the levelling algorithm | Iterative mode is outside the proof |
| Recalculation scheduler | Result independent of thread count | T2 plus tests | Absence of data races by construction (level isolation) checked by PRISM; determinism by the test matrix | Floating-point library behaviour is tested, not proved |
| Decimal128 arithmetic | Correct rounding of `+ − × ÷` | T1 | Proof against the IEEE 754-2008 definition | Transcendentals stay T3 |
| Formula parser | No runtime errors on any input | T2 | PRISM absence-of-runtime-error job plus fuzzing | — |
| Function library | Matches specification | T3 | Conformance corpus, differential tests | Size; numeric specifications are informal |
| ZIP, XML, CSV, JSON readers | No runtime errors | T2 | PRISM plus fuzzing | — |
| Parquet, BIFF, SpreadsheetML mapping | Memory safe | T3 | Safe subset, sanitisers, fuzzing | Format complexity |
| SQLite (third party) | — | T4 | Confinement only | Not our code |
| Macro host boundary | A module cannot obtain a capability it was not granted | T2 | Proof over the binding table (no binding reachable without the cap flag) plus tests | Depends on FORGE engine proofs |
| App logic (ForgeScript) | Memory safe by the language | T3 | Unit and UI tests | Interpreter is the trusted base |

## 12. Build plan

Phase 1 — usable core:

| ID | Work item | Depends on |
|---|---|---|
| OF05-W01 | Engine skeleton: workbook model, chunk store, command journal, IPC schema | G-07, G-08 |
| OF05-W02 | Formula parser, tree evaluator, error values, references, names | OF05-W01 |
| OF05-W03 | Dependency graph, levelled multi-threaded recalculation, determinism rules | OF05-W02 |
| OF05-W04 | Function groups: maths, logical, text, date, lookup, information (4.4 first six rows) | OF05-W02 |
| OF05-W05 | Spreadsheet grid widget (OF05-GAP-1) and grid adapter | G-01, G-02, FUSE-WP-12, FUSE-WP-13, FUSE-WP-23 |
| OF05-W06 | Main window, formula bar with completion, sheet tabs, status aggregates | OF05-W05, FUSE-WP-14 |
| OF05-W07 | Editing: clipboard, paste special, fill, undo, find and replace, sort, AutoFilter | OF05-W03, OF05-W06 |
| OF05-W08 | ZIP and XML readers; ODS read and write | G-06 |
| OF05-W09 | XLSX read and write (cells, formulas, styles, names); compatibility report | OF05-W08 |
| OF05-W10 | CSV and TSV import dialog, export with injection defence | OF05-W08 |
| OF05-W11 | Number formats, cell formatting, merge, freeze | OF05-W05 |
| OF05-W12 | Tables and dynamic arrays with spill | OF05-W03 |
| OF05-W13 | Trust bar, provenance, autosave and recovery, conformance table in help | OF05-W08, G-09 |

Phase 2 — full feature:

| ID | Work item | Depends on |
|---|---|---|
| OF05-W14 | Remaining function groups: statistical, distributions, financial, engineering, database, matrix | OF05-W04, MathScript build on HardenedBSD (Unverified) |
| OF05-W15 | Structured references, `LET`, `LAMBDA`, lambda helpers, legacy arrays, iteration | OF05-W12 |
| OF05-W16 | Chart widget v2 (OF05-GAP-2) and chart feature set | FUSE-WP-23 |
| OF05-W17 | Pivot tables with columnar cache | OF05-W03 |
| OF05-W18 | Conditional formatting, validation, styles, themes | OF05-W11 |
| OF05-W19 | Auditing: trace, evaluate, error checks, watch, inspection report | OF05-W03 |
| OF05-W20 | Page layout, page break preview, print, PDF | G-16 |
| OF05-W21 | Decimal128 number model and reproducibility report | OF05-W03 |
| OF05-W22 | JSON, SQLite, fixed-width import; streaming import; external file links | OF05-W10 |
| OF05-W23 | Macro host, signing checks, custom functions | FORGE signing (V14), G-05 |
| OF05-W24 | Comments, protection, encryption, metadata tools, local versions, disposable open | G-09, SE04 |
| OF05-W25 | Goal seek, scenarios, outline, subtotals, flash fill | OF05-W07 |
| OF05-W26 | Search and thumbnail integration, templates | G-09 |

Phase 3 — advanced:

| ID | Work item | Depends on |
|---|---|---|
| OF05-W27 | Solver, data tables, statistics tool | OF05-W14 |
| OF05-W28 | Parquet read and write; XLS import | OF05-W22 |
| OF05-W29 | Network data connections with the fetch helper | G-09, platform TLS decision, AD05 |
| OF05-W30 | Macros with object API, recorder, script editor panel | OF05-W23 |
| OF05-W31 | Track changes, compare and merge workbooks | OF05-W24 |
| OF05-W32 | Slicers, pivot charts, sparklines, chart templates | OF05-W16, OF05-W17 |
| OF05-W33 | Embedding service for OF04 and OF06; signatures | OF04, OF06, SE04 |
| OF05-W34 | Explain formula with `svc.model` | G-15, AI02 |

## 13. Open questions and risks

- **Grid widget ownership.** OF05-GAP-1 is large. It should be a FUSE widget (the FUSE build plan lists "spreadsheet" among missing widgets) rather than app code, so OF09 can reuse it for result grids. Not yet assigned a FUSE work package in the part of the plan that was readable.
- **Determinism against speed.** Forbidding fused multiply-add and fixing summation order costs throughput. The budget in section 10 assumes it; unmeasured.
- **Own transcendental functions.** Needed for cross-machine identical results. Error bounds must be published per function; this is a sizeable numeric task.
- **XLSX semantics switch.** Two behaviours for one function doubles the test load. The list of functions that differ must be kept short and explicit.
- **Decimal128 and file exchange.** No other spreadsheet reads the extension attribute. Export always degrades to doubles with a warning.
- **Parquet scope.** Nested types (lists, maps, structs) are flattened or refused; which is undecided.
- **`net.user` breadth.** The platform table has no "per-document, per-host run-time grant". This spec assumes the shell can prompt per endpoint under `net.user`; if not, a narrower capability is needed (shared need with OF09).
- **MathScript on HardenedBSD.** Its CI covers Windows, Linux and macOS (`MathScript:README.md`); a FreeBSD or HardenedBSD build is Unverified.
- **ODF 1.3 encryption details and the platform TLS and crypto APIs** were not read; marked Unverified in 6.3.
- **Pivot and chart fidelity with XLSX** is a long tail; the compatibility report must be honest about it.
