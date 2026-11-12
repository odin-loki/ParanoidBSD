<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Calculator

By Odin Loch

| | |
|---|---|
| ID | `OF07` (from CATALOGUE.md) |
| Bundle | `org.pbsd.calculator` |
| Category | office |
| Priority | P1 |
| Backing repos | MathScript (engine), FUSE (UI), FORGE (ForgeScript) |
| Status | Planned. Nothing built. |

## 1. Purpose

The calculator for everyone from a quick sum to exact and symbolic mathematics. It is keyboard first: the user types an expression and reads the answer. It has modes for scientific work, programmers, units, currency, dates, finance, statistics, graphing and symbolic maths, all on the MathScript engine running in a sandboxed helper. It works fully offline. It replaces KCalc from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- One expression line that accepts ordinary maths, units, dates, bases and simple phrases ("15% of 80", "3 ft in cm").
- Keypad layouts for basic, scientific and programmer use.
- Exact arithmetic (integers and rationals of any size), chosen-precision decimals, and binary64.
- Unit and currency conversion with tables stored on the machine.
- Date and time arithmetic, financial functions, statistics on lists.
- Graphing: 2D functions, parametric, polar, implicit, inequalities, data plots; 3D surfaces and parametric curves.
- Symbolic operations the engine supports: simplify, expand, differentiate, integrate, solve, limits, series, transforms.
- A history tape, variables, user functions, copy as LaTeX or MathML.
- A mini mode and an evaluation service for the launcher.

Out of scope:

| Item | Owner |
|---|---|
| Multi-cell documents with text, code and plots; long scripts; saved worksheets with narrative | AI04 Notebook |
| Tables of data, pivots, solver on ranges | OF05 Sheets |
| Time-series forecasting | AI03 Forecast |
| Hex dumps and binary file inspection | DV04 Hex Editor |
| World clocks, timers, alarms | UT02 Clocks |
| Live market data, portfolios | not planned |
| Programming beyond one-line user functions | DV01 IDE, AI04 Notebook |

## 3. Users and top tasks

Personas:

- **Anyone**: sums, percentages, a tip, a unit or currency conversion.
- **Student**: fractions kept exact, roots, trigonometry, graphs, derivative of a function, step display.
- **Engineer or scientist**: constants, units carried through formulas, complex numbers, matrices, statistics.
- **Programmer**: hex, binary, bit operations, two's complement at a word size, shifts, character codes.
- **Finance user**: loan payment, compound interest, NPV and IRR, date differences.

| # | Task | Target steps |
|---|---|---|
| T1 | Open and compute `1249.50 * 1.1` | type, read (result shows while typing); `Enter` to keep |
| T2 | Reuse the last answer | 0 (start with an operator) |
| T3 | Convert 5 miles to km | type `5 mi in km` |
| T4 | Convert 100 USD to AUD with the stored rate and see its date | type `100 USD in AUD` |
| T5 | See 255 in hex, octal and binary | type `255`; programmer panel shows all bases |
| T6 | Days between two dates | type `2027-03-01 - today` |
| T7 | Plot `sin(x)/x` | type `plot sin(x)/x` or `Ctrl+Shift+G` on the line |
| T8 | Differentiate an expression | type `d/dx x^3*sin(x)` |
| T9 | Monthly payment on a loan | Financial panel, 4 fields, read result |
| T10 | Copy a result as LaTeX | `Ctrl+Shift+L` |
| T11 | Define and use a function | `f(x) = x^2 + 1`, then `f(3)` |
| T12 | Call the calculator from anywhere | launcher: type `= 2^64`; or `Super+C` mini mode (binding set in SY01) |
| T13 | Recall an old calculation | `Up` in the line, or click the tape |
| T14 | Keep a fraction exact and see its decimal | type `1/3 + 1/6`; result `1/2` with `≈ 0.5` beside it |

## 4. Features

### 4.1 Expression line and evaluation

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF07-F001 | Single expression line | A text line with syntax colouring, bracket matching, and auto-closing brackets. Keypad buttons insert text into the same line; there is one input model, not a button state machine. | M | 1 |
| OF07-F002 | Live result | The result of the current line is shown as the user types (debounced 60 ms, evaluated in the helper with a 200 ms budget; longer jobs wait for `Enter`). Errors are not shown until the user pauses for 600 ms or presses `Enter`. | M | 1 |
| OF07-F003 | Operators | `+ - * / ^ !` (factorial), `%` (percent or modulo by context, see percent row), `mod`, `div`, unary minus, implicit multiplication (`2x`, `3(4+1)`, `2 pi`), comparison `= != < <= > >=`, logical `and or not xor`. | M | 1 |
| OF07-F004 | Precedence and clarity | Standard precedence; `-2^2 = -4`; `2^3^2` is right-associative; ambiguous input such as `1/2x` shows the parsed form (`1/(2x)` or `(1/2)x` by setting, default `(1/2)·x`) above the result so the user sees what was computed. | M | 1 |
| OF07-F005 | Pretty echo | Each entered line is echoed in typeset form (fractions, roots, powers) above its result. | M | 1 |
| OF07-F006 | Percent | `80 + 15%` = 92; `15% of 80`; `80 as % of 320`; `% change from 80 to 92`. The echo shows the meaning chosen. | M | 1 |
| OF07-F007 | Answer and history references | `ans`, `ans1`…, `#12` for tape line 12; starting a line with an operator prepends `ans`. | M | 1 |
| OF07-F008 | Number entry | Decimal point or comma by locale (both accepted when unambiguous), digit grouping with space or `_`, scientific `1.2e-3`, engineering suffixes (`k M G m µ n` only in engineering mode), fractions `3/4`, mixed numbers `2 1/2` (setting), repeating decimals `0.(3)`. | M | 1 |
| OF07-F009 | Number models | Exact (integers and rationals of any size), decimal with chosen precision (default 34 digits, up to 10,000), binary64. Default: exact when the input is exact, decimal otherwise. The status bar shows the model used for each result. | M | 1 |
| OF07-F010 | Exact results with approximations | `sqrt(8)` gives `2√2 ≈ 2.828…`; `1/3` stays `1/3 ≈ 0.333…`; `Ctrl+Enter` forces a decimal answer; `F8` toggles the form of a tape line. | M | 2 |
| OF07-F011 | Display formats | Auto, fixed (0–30 places), scientific, engineering, fraction, mixed fraction, sexagesimal (degrees-minutes-seconds, hours-minutes-seconds), with digit grouping per locale. | M | 1 |
| OF07-F012 | Rounding modes | Half-even (default), half-up, toward zero, up, down; shown when not default. | S | 2 |
| OF07-F013 | Angle units | Degrees, radians, gradians, turns; a toggle in the status bar; explicit suffix overrides (`sin(30°)`, `sin(1 rad)`). | M | 1 |
| OF07-F014 | Complex numbers | `i` (or `j` by setting), rectangular and polar entry (`3∠45°`), `re im abs arg conj`, complex results of roots and logs when complex mode is on; otherwise a clear "no real result" message. | S | 2 |
| OF07-F015 | Constants | `pi e phi tau`, physical constants (CODATA values with units: `c h hbar G k_B N_A R e_charge m_e m_p mu_0 eps_0 g_0` and more), searchable list with value, unit, uncertainty and source year. | M | 1 |
| OF07-F016 | Error messages | Say what and where: a caret under the position, a plain sentence ("`log` of a negative number needs complex mode"), and a fix where there is one. Never a bare "Error". | M | 1 |
| OF07-F017 | Long results | Results over the line width scroll, can be expanded in a viewer, and state their digit count (`2^4096` — 1,234 digits). | M | 1 |
| OF07-F018 | Cancel and limits | Any evaluation can be cancelled with `Esc`. Limits: 5 s default wall time (asks to continue), 512 MiB engine memory, 1,000,000 digits. The helper is killed and restarted on overrun; variables are restored from the session file. | M | 1 |
| OF07-F019 | Multi-line and continuation | `Shift+Enter` adds a line; `;` separates statements; the last value is shown. | S | 2 |
| OF07-F020 | Comments and labels | `# text` after an expression labels the tape line. | S | 1 |

### 4.2 Natural-language line

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF07-F021 | Phrase grammar | A fixed, documented grammar (not a language model): "`X in Y`", "`X to Y`", "`X as Y`", "`N% of X`", "`X plus/minus/times/divided by Y`", "`half/double/a third of X`", "`square root of X`", "`X squared/cubed`", "`sum/average/median of a, b, c`", "`days until DATE`", "`DATE plus 3 weeks`", "`what is …`" (ignored prefix). | M | 2 |
| OF07-F022 | Interpretation echo | The formal expression the phrase was turned into is always shown and can be edited. Nothing is computed from a guess without showing it. | M | 2 |
| OF07-F023 | Number words | "three hundred and five", "2 million", "1.5k" in the UI language (English first; tables per language). | C | 3 |
| OF07-F024 | Paste clean-up | Pasted text with currency signs, thousands separators, trailing units or surrounding prose is parsed where unambiguous; otherwise the line is kept as pasted with a hint. | S | 2 |
| OF07-F025 | Suggestions | While typing: functions, constants, units, variables and recent expressions in a completion popup (`Tab` accepts). | M | 1 |

### 4.3 Basic and scientific

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF07-F026 | Basic keypad | Digits, `+ − × ÷`, `=`, `±`, `%`, `√`, `x²`, `1/x`, brackets, backspace, clear entry, clear all, memory keys. Large targets (44 px in touch density). | M | 1 |
| OF07-F027 | Memory | `M+ M− MR MC MS` on the keypad map to variables `m0…m9`; a memory strip shows stored values; click to insert. | M | 1 |
| OF07-F028 | Scientific keypad | Trig and inverse trig, hyperbolic, `ln log log2 logb`, `exp 10^x 2^x x^y y√x`, `n! nCr nPr`, `abs floor ceil round sign`, `mod`, `EE`, constants, `2nd` layer, angle unit toggle. | M | 1 |
| OF07-F029 | Function library | Elementary functions; `gamma lgamma beta erf erfc`, Bessel and other special functions from the engine (Verified to exist: `MathScript:include/ms/special/special.hpp`, per `MathScript:docs/API.md`); `gcd lcm`; `min max sum prod` over lists. | M | 2 |
| OF07-F030 | Number theory | `isprime nextprime factor divisors phi mobius modinv modpow crt` (Verified to exist: `ms/numthy/numthy.hpp` lists `isprime`, `factor`, `divisors`, `euler_phi`, `mod_inv`, `mod_pow`, `crt`). | S | 2 |
| OF07-F031 | Combinatorics | Binomial, multinomial, permutations, Stirling, Bell, Catalan numbers (Verified to exist: `ms/combo/combo.hpp`). | S | 2 |
| OF07-F032 | Lists and vectors | `[1,2,3]` literals, element-wise arithmetic, `len sum mean sort reverse range(a,b,step)`, indexing, dot and cross products, norm. | S | 2 |
| OF07-F033 | Matrices | `[[1,2],[3,4]]` literals; `det inv transpose rank trace`, solve `A\b`, eigenvalues, LU, QR, SVD (Verified to exist: in-tree linear algebra, `MathScript:README.md`). Matrix editor grid for entry up to 12×12; larger by paste. | S | 2 |
| OF07-F034 | Numeric calculus | Numeric derivative at a point, definite integral (adaptive quadrature with an error estimate shown), sums and products with bounds, root finding (`solve f(x)=0 near x0`), minimise on an interval. | S | 2 |
| OF07-F035 | Random numbers | `rand randint randn choice shuffle` with a visible seed; "reseed" action; never used for anything security-related and labelled so. | C | 2 |
| OF07-F036 | Uncertainty arithmetic | `5.0 ± 0.1` values propagate through formulas (first-order); result shown with uncertainty. | C | 3 |
| OF07-F037 | Step display | For arithmetic with fractions, percentages and simple equations: show the steps (common denominator, cancel, isolate). Rule-based. | C | 3 |

### 4.4 Programmer

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF07-F038 | Bases | Binary, octal, decimal, hexadecimal entry (`0b 0o 0x` prefixes, or the mode's base); any base 2–36 with `base(n, b)` and `n_b` literals; results shown in all four bases at once. | M | 1 |
| OF07-F039 | Word size and signedness | 8, 16, 32, 64, 128 bits and "unlimited"; signed (two's complement) or unsigned; overflow wraps with a visible flag, or saturates, or errors, by setting. | M | 1 |
| OF07-F040 | Bitwise operators | `and or xor not nand nor`, `<< >>` (logical), `>>>` or `sar` (arithmetic), `rol ror`, with keypad keys; C-style `& | ^ ~` accepted in programmer mode only. | M | 1 |
| OF07-F041 | Bit panel | A grid of bits (grouped in nibbles, numbered) that toggles on click or with `Space`; bit ranges can be selected to extract a field. | M | 1 |
| OF07-F042 | Bit functions | `popcount clz ctz parity bswap bitreverse`, mask builders `mask(hi, lo)`, field extract and insert. | S | 2 |
| OF07-F043 | Hex keypad | `A–F` keys, `00`, `FF`, shift keys, word-size and sign selectors. | M | 1 |
| OF07-F044 | Character codes | Unicode code point, UTF-8, UTF-16 and UTF-32 bytes of a character or string (`'A'`, `"é"`), and back; name of the code point from the system Unicode table. | S | 2 |
| OF07-F045 | Floating-point inspector | Show sign, exponent and mantissa of a value as binary16, 32, 64, 128; next and previous representable value; exact decimal expansion of the stored value. | S | 2 |
| OF07-F046 | Integer formats | Byte order swap view, grouped bytes, C and Rust literal copy formats (`0xFFu8`, `0b1010`), signed and unsigned readings side by side. | S | 2 |
| OF07-F047 | Fixed point | Qm.n interpretation of a word. | C | 3 |
| OF07-F048 | Sizes and addresses | `KiB MiB GiB` and `kB MB GB` as units; page and alignment helpers `align_up(x, 4096)`; IPv4 and IPv6 address arithmetic and CIDR ranges (`10.0.0.0/22` → first, last, count). No network access is involved. | S | 2 |
| OF07-F049 | Time stamps | Unix seconds, milliseconds and nanoseconds to date-time and back, with the time zone shown. | S | 2 |
| OF07-F050 | Checksums of typed text | `crc32 sha256` of a typed string for quick checks, labelled "not for files" (files go to SE05). Uses engine digests (Verified to exist: `ms/crypto/crypto.hpp` has `sha256`). | C | 3 |

### 4.5 Units

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF07-F051 | Unit-aware arithmetic | Quantities carry dimensions: `3 m * 2 m = 6 m²`; `5 km / 20 min in km/h`; adding incompatible units is an error that names both dimensions. | M | 2 |
| OF07-F052 | Unit table | SI base and derived units with all prefixes; imperial and US customary (with the US/UK gallon, pint, ton distinctions named); CGS; nautical; typographic; astronomical; data sizes (decimal and binary); angles; cooking measures by region. About 600 units in a signed data file in the bundle. | M | 1 |
| OF07-F053 | Categories | Length, area, volume, mass, time, speed, acceleration, force, pressure, energy, power, temperature, angle, frequency, data, data rate, electric (charge, current, potential, resistance, capacitance, inductance), magnetic, luminous, radiation, amount, density, flow, torque, viscosity, fuel economy, typography, cooking. | M | 1 |
| OF07-F054 | Conversion syntax | `X unit in unit`, `to`, `as`, `->`; compound targets (`in ft and in`, `in h m s`); several targets at once (`in km, mi, nmi`). | M | 1 |
| OF07-F055 | Temperature | Absolute and difference handled apart: `20 °C in °F` = 68; `20 Δ°C in Δ°F` = 36. The echo states which was used. | M | 1 |
| OF07-F056 | Non-linear and reciprocal conversions | Fuel economy (L/100 km ↔ mpg), decibel ratios, pH helpers, photographic stops. | S | 2 |
| OF07-F057 | Converter panel | Two-field converter with a category and unit pickers for pointer users; favourites; swap; shows the factor used and its exactness (exact by definition or rounded). | M | 1 |
| OF07-F058 | Simplify and choose units | Results are shown in sensible units (`0.0012 m` → `1.2 mm`), SI by default or the locale's system; "base units" view. | S | 2 |
| OF07-F059 | User units | `1 smoot = 1.7018 m` defines a unit for the session or permanently. | C | 3 |
| OF07-F060 | Dimensional check of formulas | A user function can declare units for its parameters; a mismatch is reported at call time. | C | 3 |

### 4.6 Currency

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF07-F061 | Offline rate table | A table of reference rates for about 170 ISO 4217 currencies ships as signed data, with its date. Every currency result shows "rate of DD Mon YYYY, N days old". | M | 1 |
| OF07-F062 | Rate updates by system update | The rate table is a small data package that `svc.update` refreshes with other signed updates; the app itself makes no network request for this. | M | 2 |
| OF07-F063 | Optional direct update | Off by default. If the user turns it on, a fetch helper downloads the published daily reference rates from one fixed host over TLS (manifest: `net.host:www.ecb.europa.eu:443`), parses them in its sandbox, and stores them with the fetch time. Manual "Update now", or daily when the app is open. The `[net]` badge shows during the fetch. | S | 2 |
| OF07-F064 | Stale-rate warning | Rates older than 7 days (setting) show `--warn` with the word "old". | M | 1 |
| OF07-F065 | Manual rates | The user can set or override any pair (`1 EUR = 1.62 AUD`), marked "manual" in results. | M | 1 |
| OF07-F066 | Currency syntax | ISO codes and symbols (`$`, `€`, `£`, `¥`; `$` resolves to the locale's currency and the echo says which), `100 USD in AUD, EUR, GBP`. | M | 1 |
| OF07-F067 | Historical rates | The last 90 daily tables are kept when updates are on; `100 USD in AUD on 2026-10-01`. | C | 3 |
| OF07-F068 | Currency rounding | Results round to the currency's minor unit, with the unrounded value on hover. | S | 2 |
| OF07-F069 | Other rate sources | None built in. Cryptocurrency and commodity prices are not included; manual rates cover them. | C | 3 |

### 4.7 Date and time

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF07-F070 | Date literals | ISO `2027-03-01`, locale forms, `today now tomorrow yesterday`, weekday names (`next friday`), times `14:30`, zones `14:30 Sydney`, `UTC+10`. | M | 2 |
| OF07-F071 | Date arithmetic | Date ± duration (`today + 90 days`, `+ 3 months` with end-of-month rule shown), date − date in days, weeks, months and days, years months days; durations add and scale. | M | 2 |
| OF07-F072 | Business days | `workdays between A and B`, `A + 10 workdays`, with a weekend pattern and a holiday list the user maintains (no downloaded calendars). | S | 3 |
| OF07-F073 | Date facts | Weekday, ISO week number, day of year, days left in year, leap year, Julian day number, age from a birth date. | S | 2 |
| OF07-F074 | Time zones | Convert between zones using the system zone database; daylight-saving transitions named when a time is ambiguous or does not exist. | S | 2 |
| OF07-F075 | Time arithmetic | `1h 20min + 55min`, `7:45 to 16:10` as a duration, decimal hours, pace and speed (`10 km in 52 min` → min/km and km/h). | S | 2 |
| OF07-F076 | Date panel | Two date pickers (`dateTimeEdit`, `calendar`) with the difference shown in several forms; add or subtract fields. | M | 2 |

### 4.8 Financial

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF07-F077 | Time value of money | Solve any one of N, I/Y, PV, PMT, FV from the other four; payments per year, compounding per year, begin or end mode. Panel with five fields; the solved one is highlighted. | M | 2 |
| OF07-F078 | Loans | Payment, total interest, amortisation schedule (table, exportable as CSV), effect of extra payments, balance after n payments. | M | 2 |
| OF07-F079 | Interest conversions | Nominal ↔ effective, simple and compound growth, continuous compounding, rule-of-72 estimate with the exact figure. | S | 2 |
| OF07-F080 | Cash-flow analysis | NPV, IRR, MIRR, payback period from a list of flows (Verified to exist in the engine: NPV, IRR, PV, FV annuities in `ms/finance/finance.hpp` per `MathScript:docs/API.md`). | S | 2 |
| OF07-F081 | Depreciation | Straight line, declining balance, sum of years' digits. | C | 3 |
| OF07-F082 | Bonds | Price, yield to maturity, duration, convexity (Verified to exist in the engine: bond price, duration, convexity, YTM). | C | 3 |
| OF07-F083 | Everyday money | Tip and split, discount and sale price, tax inclusive and exclusive (rate set by the user), margin and markup, unit price comparison. | M | 1 |
| OF07-F084 | Decimal money arithmetic | Financial mode always computes in decimal, never binary64, and rounds only at display. | M | 2 |

### 4.9 Statistics

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF07-F085 | Data entry | A small list editor (one or two columns, with frequencies) or a pasted column; import from CSV through the file portal; up to 1,000,000 values. | M | 2 |
| OF07-F086 | One-variable summary | n, sum, mean, median, mode, min, max, range, quartiles, IQR, variance and standard deviation (sample and population), standard error, skewness, kurtosis, geometric and harmonic means. | M | 2 |
| OF07-F087 | Two-variable | Covariance, correlation, linear regression (slope, intercept, r², residuals), and fits: quadratic, exponential, logarithmic, power; predicted values. | S | 2 |
| OF07-F088 | Distributions | PDF, CDF and inverse for normal, t, chi-square, F, binomial, Poisson, exponential, gamma, beta, uniform, lognormal (Verified to exist: `ms/prob/prob.hpp`); a panel that shades the area under the curve. | S | 2 |
| OF07-F089 | Tests and intervals | z and t tests (one and two sample), confidence intervals, chi-square test, one-way ANOVA (Verified to exist: t/z tests and `one_way_anova` in `ms/stats/stats.hpp`). Output states the hypotheses and the p-value in a sentence. | C | 3 |
| OF07-F090 | Quick plots | Histogram, box plot, scatter with fit line from the data list. | S | 2 |
| OF07-F091 | Counting helpers | Permutations, combinations, with and without repetition, shown with the formula. | S | 2 |

### 4.10 Graphing

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF07-F092 | 2D function plots | `y = f(x)`, several at once, each with colour, line style and an on-off switch; domain restrictions `{x > 0}`. | M | 2 |
| OF07-F093 | Other 2D forms | Parametric `(x(t), y(t))`, polar `r(θ)`, implicit `F(x, y) = 0` (marching squares with interval refinement), inequalities (shaded), piecewise, point lists and data plots. | M | 2 |
| OF07-F094 | Adaptive sampling | Samples refine near high curvature and discontinuities; vertical asymptotes are not joined; undefined regions are left blank. | M | 2 |
| OF07-F095 | Navigation | Pan by drag, zoom by wheel or pinch about the pointer, box zoom, axis-only zoom with modifier keys, fit to data, equal axes, reset; all with keys. | M | 2 |
| OF07-F096 | Axes and grid | Linear and log axes, π-multiples on an axis, tick labels, grid, axis labels, legend. | M | 2 |
| OF07-F097 | Analysis tools | Trace along a curve with coordinates; roots, extrema, intersections, y-intercept found and labelled; tangent at a point; area between bounds; derivative curve overlay. | S | 2 |
| OF07-F098 | Parameters with sliders | An undefined symbol in a plotted expression becomes a slider with range and step; a play button animates it (honours reduced motion: steps on key press instead). | S | 2 |
| OF07-F099 | Table of values | A table for chosen x values or a step; export as CSV. | S | 2 |
| OF07-F100 | 3D surfaces | `z = f(x, y)`, parametric surfaces and space curves, implicit surfaces at low resolution; rotate, zoom, contour and colour-map options, wireframe. Rendered by own code into a `viewportWidget` image (CPU first; GPU when `dev.gpu.compute` is granted). | S | 3 |
| OF07-F101 | Contour and vector field plots | Contours of `f(x, y)`, direction fields for `dy/dx = f(x, y)` with solution curves through clicked points (uses engine ODE solvers; Verified to exist: `ode_rk45` in `ms/ode/ode.hpp`). | C | 3 |
| OF07-F102 | Complex-function plots | Domain colouring of `f(z)`. | C | 3 |
| OF07-F103 | Graph export | PNG, SVG, PDF at a chosen size; copy image; plot data as CSV. | M | 2 |
| OF07-F104 | Graph accessibility | A text description (domain, range in view, roots, extrema, trend), a data table view, and audio trace (pitch follows y) as an option. | S | 3 |

### 4.11 Symbolic and exact mathematics

The engine's symbolic core is small (its README calls it "a small symbolic CAS"). Rows say what is Verified in the engine and what would need engine work.

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF07-F105 | Exact integers and rationals | Any size (Verified to exist: `BigInt` and `Rational` in `MathScript:include/ms/bignum/bignum.hpp`). | M | 1 |
| OF07-F106 | Simplify, expand, collect, substitute | Verified to exist as REPL calls `sym_simplify`, `sym_expand`, `sym_collect`, `sym_substitute` (`MathScript:docs/API.md`). | M | 2 |
| OF07-F107 | Differentiate | `d/dx`, `diff(f, x, n)`, partial derivatives. Verified to exist: `sym_diff`. | M | 2 |
| OF07-F108 | Integrate | Indefinite and definite. Verified: `sym_integrate` covers the power rule, linearity and `sin`/`cos`, with unsupported forms documented. The app says "no closed form found by this engine" and offers a numeric value for definite integrals; it never shows a wrong form. Wider coverage needs engine work. | S | 2 |
| OF07-F109 | Solve equations | Linear systems symbolically (Verified: `sym_solve_linear`); polynomial roots numerically and, for degree ≤ 4, exactly (Proposed, needs engine work); general equations numerically with all roots in an interval. | S | 2 |
| OF07-F110 | Limits and series | Verified to exist: `sym_limit`, `sym_series` (Taylor to a given order). | S | 3 |
| OF07-F111 | Transforms | Laplace, inverse Laplace, Fourier, Z (Verified to exist as table-driven calls `sym_laplace`, `sym_ilaplace`, `sym_fourier`, `sym_ztransform`); unsupported inputs say so. | C | 3 |
| OF07-F112 | Differential equations | Separable first-order ODEs symbolically (Verified: `sym_dsolve`); others numerically with a plot. | C | 3 |
| OF07-F113 | Polynomial tools | Factor over the integers, GCD, division with remainder, partial fractions, discriminant, resultant (Verified to exist on coefficient vectors: GCD, division, resultant, discriminant, square-free in `ms/poly/poly.hpp`; factoring and partial fractions are Proposed). | S | 3 |
| OF07-F114 | Exact constants and radicals | Results keep `π`, `e`, `√n` and rational multiples where the engine can; otherwise decimal with the model named. Needs engine work beyond `Rational`; Proposed. | S | 2 |
| OF07-F115 | Assumptions | `assume x > 0`, `assume n integer` to guide simplification; Proposed, needs engine support. | C | 3 |

### 4.12 History, variables and functions

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF07-F116 | History tape | Every committed line with its echo, result, label and time. Scrolls up from the input. Click inserts the result; double-click inserts the expression; `Up/Down` in the line walks history; search with `Ctrl+F`. | M | 1 |
| OF07-F117 | Tape editing | Edit an old line and re-evaluate lines that depend on it (dependants are marked); delete lines; pin lines; insert a separator or note. | S | 2 |
| OF07-F118 | Sessions | The tape, variables and functions persist across restarts (crash-safe). Named sessions; "New session" clears; private session keeps nothing on disk. | M | 1 |
| OF07-F119 | Variables | `x = 5`, `rate = 7.5%`; a variables panel lists name, value, unit; rename, delete, pin. Names may be Unicode letters. | M | 1 |
| OF07-F120 | User functions | `f(x) = x^2 + 1`, `g(a, b) = sqrt(a^2 + b^2)`, piecewise definitions; listed in the panel; saved with the session or to "my functions". | M | 2 |
| OF07-F121 | Export and print tape | Plain text, Markdown, CSV, LaTeX document, PDF through the print portal; choose range. | S | 2 |
| OF07-F122 | Formula library | Built-in sheets of formulas by subject (geometry, physics, finance) with fields to fill; the user's own saved formulas. | C | 3 |

### 4.13 Copy, paste and exchange

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF07-F123 | Copy result | `Ctrl+C` with nothing selected copies the result as plain text in the display format; `Ctrl+Shift+C` copies full precision. | M | 1 |
| OF07-F124 | Copy as LaTeX | Expression, result or both (Verified to exist: `sym_latex` and `sym_export(…, "latex")`). `Ctrl+Shift+L`. | M | 2 |
| OF07-F125 | Copy as MathML | Presentation or content MathML (Verified to exist: notations `mathml` and `content-mathml` in `sym_export`). `Ctrl+Shift+M`. | M | 2 |
| OF07-F126 | Copy as code or other notations | `unicode ascii sympy mathematica c c++ python` (Verified to exist in `sym_export`). | S | 2 |
| OF07-F127 | Paste LaTeX | A LaTeX expression pasted into the line is parsed by the engine's strict subset reader, which rejects anything outside the subset with a position (Verified: `MathScript:docs/LATEX_SUBSET.md`, `include/ms/sym2/latex_parse.hpp`). | S | 2 |
| OF07-F128 | Copy as image | Typeset expression as PNG or SVG. | C | 3 |
| OF07-F129 | Clipboard offers | Results are offered as text plus MathML; graphs as PNG plus SVG; nothing is marked sensitive unless the session is private. | S | 2 |
| OF07-F130 | Send to other apps | "Open in Notebook" (AI04) with the session as cells; "Send table to Sheets" (OF05) as CSV through `svc.intent`. | C | 3 |

### 4.14 Window modes, system integration and settings

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF07-F131 | Mini mode | A 360×56 window with only the line and the result; optional keep-on-top; toggles to the full window with `Ctrl+Shift+F`; `Esc` hides. Opens in under 100 ms when the app is already running. | M | 1 |
| OF07-F132 | Mode switch | Basic, Scientific, Programmer, Units, Currency, Date, Financial, Statistics, Graph, Symbolic; `Alt+1…0`; the expression line and tape are shared by all modes. | M | 1 |
| OF07-F133 | Launcher evaluation service | The shell's command runner (SH01) can send an expression and get a result string through a bus interface, so `= 2^10` works in the launcher without opening a window. The service evaluates with no variables and a 100 ms budget. | S | 2 |
| OF07-F134 | Several windows | More than one calculator window, sharing or not sharing a session by choice. | S | 2 |
| OF07-F135 | Keypad visibility and layout | Keypad can be hidden (keyboard users), shown on the right or below; custom keypad pages with user-chosen keys. | S | 2 |
| OF07-F136 | Themes and digit font | Follows the system theme; large-digit option; tabular figures always. | M | 1 |
| OF07-F137 | Sounds and haptics | None by default; optional key click. | C | 3 |
| OF07-F138 | Privacy of history | "Private session" toggle; "Clear history" with a time range; history is never offered to `svc.search`. | M | 1 |
| OF07-F139 | Help | Syntax guide, function reference with examples, unit list, shortcuts; `F1` on a function name. | M | 1 |
| OF07-F140 | Engine information | About panel shows the engine version, build options (JIT off, GPU off) and the rate table date. | S | 1 |

## 5. GUI

Layout pattern: single-purpose utility. Default density: normal. The window is small by default (420×620) and grows for graphing.

### 5.1 Window map

```
Calculator
├─ Main window
│  ├─ Header bar: menu, mode switch, session name, [net] badge (only during a rate fetch)
│  ├─ Tape (history)
│  ├─ Expression line with live result
│  ├─ Mode panel (one of): Keypad basic | Keypad scientific | Programmer panel | Converter |
│  │                        Currency | Date | Financial | Statistics | Graph | Symbolic tools
│  ├─ Side panel (F9): Variables | Functions | Constants | Units | Formula library
│  ├─ Inspector (F4, graph mode only): plot list, style, axes, sliders
│  └─ Status bar: angle unit, number model, format, word size (programmer), rate date (currency)
├─ Mini window (line and result only)
├─ Dialogs
│  ├─ Matrix editor            ├─ Export tape
│  ├─ Data list editor         ├─ Export graph
│  ├─ Amortisation schedule    ├─ Manual currency rates
│  ├─ Long result viewer       ├─ Holidays and work week
│  ├─ Sessions                 └─ Preferences
├─ Popovers: completion list, constant picker, unit picker, format menu, base menu,
│            error detail, interpretation echo, date picker
└─ Service (no window): launcher evaluation
```

### 5.2 Main window

Shown at 420×620, scientific mode. Minimum 320×240 (keypad hides under 420 px height).

```
+----------------------------------------------------+
| [=] [Scientific v]   Session: default        [▁][✕]|  (1)
+----------------------------------------------------+
| 12  sqrt(8)                                        |  (2)
|                              2√2  ≈ 2.828 427 125  |
| 13  1/3 + 1/6            # share                   |
|                                    1/2  ≈ 0.5      |
| 14  5 mi in km                                     |
|                                    8.046 72 km     |
+----------------------------------------------------+
| (3) > ans + 2 km|                                  |
| (4)                              = 10.046 72 km    |
|     exact · shown to 7 significant digits          |
+----------------------------------------------------+
| (5) [2nd][ π ][ e ][ C ][ ⌫ ]   [sin][cos][tan]    |
|     [x² ][ √ ][ ^ ][ ( ][ ) ]   [ln ][log][ ! ]    |
|     [ 7 ][ 8 ][ 9 ][ ÷ ][ % ]   [EE ][abs][mod]    |
|     [ 4 ][ 5 ][ 6 ][ × ][1/x]   [ans][ x ][ = ]    |
|     [ 1 ][ 2 ][ 3 ][ − ][ ± ]   [M+ ][M− ][MR ]    |
|     [ 0 ][ . ][ , ][ + ][     Enter      ]         |
+----------------------------------------------------+
| (6) DEG · exact/decimal 34 · auto            [F9 ▸]|
+----------------------------------------------------+
```

1. Header: menu button, mode `comboBox`, session name (click opens Sessions). No search field at this width; `Ctrl+K` still opens the palette.
2. Tape: a virtualised list. Each entry has a line number, the typeset echo, an optional label and the right-aligned result. The newest entry is at the bottom.
3. Expression line: `lineEdit`-like editor with colouring and completion (see OF07-GAP-1).
4. Live result and message row. The message row states the number model and rounding of the shown value, or the error text with a caret position when the line is not valid. An invalid line never shows a guessed value.
5. Keypad: `gridView`-like fixed grid of `button`s; `2nd` swaps the function columns. Buttons are 56×40 at normal density.
6. Status bar: each item is a `toolButton` that opens a menu (angle unit, number model and precision, display format); `[F9 ▸]` opens the side panel.

States: empty (tape shows three example lines as hints, greyed, removed on first entry), evaluating (a thin progress line under the expression after 200 ms, `Esc` cancels), error (message row, caret mark), engine restarting (line disabled for up to 1 s, toast "engine restarted; variables restored"), offline (always; currency shows stored rate date), private session (header shows a mask icon and the word "Private").

### 5.3 Other views and dialogs

**Programmer mode** (520×620).

```
+----------------------------------------------------------------+
| [=] [Programmer v]   Session: default                    [▁][✕]|
+----------------------------------------------------------------+
|  7  0xFF00 and (1 << 12)                           0x1000      |
+----------------------------------------------------------------+
| > 0xDEADBEEF >> 4|                                             |
|   HEX  0DEA DBEE                DEC  233 495 534               |
|   OCT  1 572 555 756            BIN  see bits                  |
+----------------------------------------------------------------+
| (1) 31      24 23      16 15       8 7        0                |
|     0000 1101  1110 1010  1101 1011  1110 1110                 |
| (2) [8][16][>32][64][128][∞]  [>unsigned][signed]  [wrap v]    |
+----------------------------------------------------------------+
| [and][or ][xor][not]   [ D ][ E ][ F ][ ⌫ ][ C ]               |
| [<< ][>> ][sar][rol]   [ A ][ B ][ C ][ ( ][ ) ]               |
| [ror][pop][clz][ctz]   [ 7 ][ 8 ][ 9 ][ ÷ ][mod]               |
| [bsw][msk]['c'][flt]   [ 4 ][ 5 ][ 6 ][ × ][ − ]               |
| [hex][dec][oct][bin]   [ 1 ][ 2 ][ 3 ][ + ][ = ]               |
+----------------------------------------------------------------+
| 32-bit unsigned · wrap · input base HEX                        |
+----------------------------------------------------------------+
```

1. Bit panel: each bit is a toggle; arrow keys move, `Space` flips; drag selects a field and shows its value. 2. Word size and sign selectors (`radioButton` groups), overflow rule `comboBox`.

**Converter and currency panel** (420×620, lower half).

```
+----------------------------------------------------+
| Category [Length v]                  [★ Favourites]|
| [ 5______________ ]  [mile (mi)            v]      |
|          [⇅ swap]                                  |
| [ 8.046 72_______ ]  [kilometre (km)       v]      |
| 1 mi = 1.609 344 km (exact by definition)          |
+----------------------------------------------------+
| Currency                                           |
| [ 100____________ ]  [USD US dollar        v]      |
| [ 153.20_________ ]  [AUD Australian dollar v]     |
| Rate 1 USD = 1.5320 AUD · table of 3 Nov 2026      |
| ⚠ old: 9 days   [Update now]  [Set rate…]          |
+----------------------------------------------------+
```

The numbers in this drawing are placeholders for layout, not real rates. `[Update now]` is shown only when direct update is enabled; otherwise the row says "updates with system updates".

**Graph mode** (1100×720).

```
+------------------------------------------------------------------------------------------+
| [=] [Graph v]   Session: default                                        [▁] [▢] [✕]      |
+---------------------------+--------------------------------------------------------------+
| (1) Plots            [+]  | (3)                    y                                     |
| [x] ■ y = sin(x)/x        |                       1┤      ●(0, 1)                        |
| [x] ■ y = a·x² − 1        |              ╭──╮      │    ╭──╮                             |
| [ ] ■ r = 1 + cos(θ)      |    ╭──╮     ╱    ╲     │   ╱    ╲     ╭──╮                   |
| [x] ■ x² + y² = 4         |───╯────╲───╱──────╲────┼──╱──────╲───╱────╰──── x            |
+---------------------------+ −10     ╰─╯   −5   ╰───┼─╯   5    ╰─╯     10                 |
| (2) a  [──────o───] 0.40  |                      −1┤                                     |
|     min [-2] max [2] [▶]  |   (4) trace: x = 4.49  y = −0.217   root ○  min ▽  max △     |
+---------------------------+--------------------------------------------------------------+
| > plot a x^2 - 1|                                                     = added as plot 2  |
+------------------------------------------------------------------------------------------+
| RAD · view x[−10, 10] y[−1.5, 1.5]   [Fit] [1:1] [Table] [Export…]     [Tools v]        |
+------------------------------------------------------------------------------------------+
```

1. Plot list (`listView` with check box, colour swatch, editable expression). 2. Sliders for free parameters. 3. Plot area (`viewportWidget` painting an image made from the draw list; needs paths, see 5.7). 4. Trace read-out and markers for found points. Keys: arrows pan, `+ −` zoom, `T` trace, `R` roots, `I` intersections, `Home` reset.

**Financial mode: time value of money** (420×620, lower half).

```
+----------------------------------------------------+
| [TVM][Loan][Cash flows][Interest][Everyday]        |
| N    payments   [ 360_______ ]  ( ) solve          |
| I/Y  % a year   [ 6.25______ ]  ( ) solve          |
| PV   present    [ 450 000___ ]  ( ) solve          |
| PMT  payment    [ −2 770.73_ ]  (•) solve          |
| FV   future     [ 0_________ ]  ( ) solve          |
| Payments/yr [12]  Compounding/yr [12]  (•)End ( )Begin|
| Total paid 997 462.80 · interest 547 462.80        |
| [Schedule…]  [Copy as expression]                  |
+----------------------------------------------------+
```

"Copy as expression" puts the equivalent function call on the expression line, so panel results can be reproduced by typing.

**Matrix editor** (480×400): a grid of `lineEdit` cells with row and column `spinBox`es (1–12), `[Paste]`, `[Identity]`, `[Clear]`, a name field, `[Insert]`. Tab moves by cell; `Ctrl+Enter` inserts.

**Statistics mode**: left a two-column list editor (`tableView` with inline editing), right a summary (`propertyGrid`, read-only) and a small chart (`chart`: histogram or scatter), tabs `[Summary][Regression][Distributions][Tests]`.

**Symbolic tools panel**: buttons `[Simplify][Expand][Factor][d/dx][∫ dx][Solve for…][Limit][Series]` that wrap the current line in the matching call; a variable `comboBox`; results show exact form, decimal form and a `[Copy LaTeX]` button.

**Mini window** (360×56).

```
+----------------------------------------------+
| > 1249.50 * 1.1|              = 1 374.45  [⤢]|
+----------------------------------------------+
```

`Enter` commits to the shared tape; `Ctrl+C` copies; `[⤢]` opens the full window.

**Long result viewer** (640×480): a read-only `codeEdit` with the digits grouped, digit count, `[Copy]`, `[Save…]`, base selector.

### 5.4 Menus

```
File    New session (Ctrl+N) · New private session (Ctrl+Shift+N) · Sessions… (Ctrl+O) · New window
        Export tape… (Ctrl+Shift+S) · Export graph… · Print tape… (Ctrl+P) · Close (Ctrl+W) · Quit (Ctrl+Q)
Edit    Undo (Ctrl+Z) · Redo (Ctrl+Shift+Z) · Cut · Copy result (Ctrl+C) · Copy full precision (Ctrl+Shift+C)
        Copy as ▸ LaTeX (Ctrl+Shift+L) · MathML (Ctrl+Shift+M) · Unicode · ASCII · C · C++ · Python · Image
        Paste (Ctrl+V) · Paste and evaluate (Ctrl+Shift+V) · Find in tape (Ctrl+F)
        Clear line (Esc) · Clear tape (Ctrl+L) · Clear history… · Preferences… (Ctrl+,)
View    Mode ▸ Basic (Alt+1) · Scientific (Alt+2) · Programmer (Alt+3) · Units (Alt+4) · Currency (Alt+5)
               Date (Alt+6) · Financial (Alt+7) · Statistics (Alt+8) · Graph (Alt+9) · Symbolic (Alt+0)
        Mini mode (Ctrl+Shift+F) · Keep on top · Keypad (Ctrl+Shift+K) · Side panel (F9) · Inspector (F4)
        Zoom ▸ · Full screen (F11)
Number  Angle ▸ Degrees · Radians · Gradians · Turns (F6 cycles)
        Model ▸ Exact when possible · Decimal… · Binary64      Format ▸ Auto · Fixed… · Scientific · Engineering · Fraction · Sexagesimal (F7 cycles)
        Toggle exact / decimal (F8) · Rounding ▸ · Complex mode · Base ▸ (programmer) · Word size ▸ · Signed
Insert  Constant… (Ctrl+Shift+O) · Unit… (Ctrl+U) · Function… (Ctrl+Shift+I)
        Matrix… (Ctrl+M) · Data list… · Date… (Ctrl+D) · Previous answer (Ctrl+Shift+A)
Graph   Plot current line (Ctrl+Shift+G) · Add plot · Fit · Equal axes · Trace (T) · Roots (R) · Intersections (I)
        Table of values… · 3D view · Export…
Tools   Variables… · Functions… · Formula library… · Currency rates… · Update rates now · Holidays and work week…
        Engine information
Help    Calculator help (F1) · Syntax guide · Function reference · Unit list · Keyboard shortcuts · About
```

### 5.5 Keyboard and pointer

The app is usable with the keyboard alone and with the keypad alone.

| Keys | Action |
|---|---|
| `Enter` | Commit the line to the tape |
| `Ctrl+Enter` | Commit and force a decimal result |
| `Shift+Enter` | New line in the expression |
| `Up` / `Down` | Walk history (when the line has one row) |
| `Tab` | Accept completion |
| `Esc` | Cancel evaluation; then clear the line; then (mini mode) hide |
| `Ctrl+L` | Clear tape (with Undo toast) |
| `Alt+Up` | Move focus to the tape; arrows select entries; `Enter` inserts the result; `F2` edits the entry |
| `F6`, `F7`, `F8` | Angle unit, display format, exact or decimal |
| `Ctrl+B` | Cycle base of the last result (programmer) |
| `Ctrl+Shift+G` | Plot the current line |
| `Ctrl+R` | Insert `sqrt(` ; `Ctrl+I` insert `1/(`; `Ctrl+T` open TVM panel |
| typing an operator first | Prepends `ans` |

Context menus: tape entry (copy result, copy expression, copy as, insert, edit, label, pin, delete, plot); result (copy, format, show in bases, show exact and decimal, open in viewer); plot (style, hide, remove, table, trace); variable (rename, delete, pin, copy value); bit panel (set, clear, invert field).

Drag and drop: drag a tape result into the line or out to another app (text and MathML); drop a CSV on statistics mode to load a list; drop text on the line to paste and parse; drag a plot image out as PNG or SVG.

Pointer: wheel on a number field steps it; wheel on the plot zooms; long press on a keypad key shows its second function.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Hint entries in the tape; keypad visible. |
| Evaluating | Progress line after 200 ms; a "still working — keep going?" prompt at 5 s. |
| Error | Message row with caret position; the tape is not changed. |
| Engine crashed | Helper restarts; session variables reloaded; the failing line is kept in the editor with a note. Three crashes on the same line stop automatic retry. |
| Offline | The normal state. Currency shows the stored date. |
| Rates missing or unsigned | Currency mode says "no rate table" and allows manual rates only. |
| Locked / private | Private session: nothing written; closing discards. |
| Read-only | Not applicable to sessions; exported files only. |
| Narrow (<360 wide) | Keypad switches to the basic set; tape shows results only. |
| Short (<420 high) | Keypad hidden; line and tape remain (keyboard use). |

### 5.7 Widgets

| Region | FUSE widget | Notes |
|---|---|---|
| Tape | `modelListView` | Needs variable row height (FUSE-WP-28) and a typeset-maths cell delegate (OF07-GAP-2) |
| Expression line | **OF07-GAP-1** | `lineEdit` has no colouring or completion; `codeEdit` is multi-line with a gutter. Needed: a one-to-few-line code field with token colours, bracket match, inline completion popup |
| Keypad | `panel` grid of `button` / `toolButton` | Keys such as `+ − × ÷ . ,` and numpad keys must be bindable; today's `ui::Key` set lacks punctuation and numpad keys (FUSE build plan 2.2, Input model) |
| Mode switch, pickers | `comboBox`, `popupMenu`, `radioButton`, `toggleSwitch` | |
| Bit panel | `panel` of `toolButton` toggles | 128 toggles; one a11y group with per-bit names |
| Converter | `lineEdit`, `comboBox` (with search) | |
| Date panel | `calendar`, `dateTimeEdit` | |
| Statistics list, schedule, value table | `tableView` | Needs inline editors (FUSE-WP-28) |
| Plot area 2D and 3D | `viewportWidget` | Paints an image the app produces. Needs draw-list paths for crisp vector output (FUSE-WP-23); until then, lines are rasterised by app code |
| Quick statistics charts | `chart` | Histogram as bars; box plot needs chart v2 (OF05-GAP-2) |
| Variables, functions, constants | `treeView`, `listView`, `propertyGrid` | |
| Long result viewer | `codeEdit` (read-only) | |
| Help | `markdownView` | |

Gaps:

- **OF07-GAP-1 Code line field** (above).
- **OF07-GAP-2 Mathematics typesetting.** Layout of fractions, roots, powers, matrices, big operators from a tree (MathML-like) into draw-list glyphs and rules, with a mathematical font. Shared need with OF04, OF06 and AI04. No such facility was found in FUSE (`FUSE:docs/ui/README.md` lists none).
- **OF07-GAP-3 Always-on-top and small utility windows** for mini mode: a window-management hint the compositor honours (SH05), not available in `fuse_app` as read.
- G-02 (binding), G-07 (helper launch), G-01.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Line editing model, phrase grammar, mode panels, tape, sessions, settings, unit and currency tables lookup, plot scene description |
| UI documents and styles | FUSE UI | app host | Windows, panels, keypads |
| Front-end parser | ForgeScript | app host | Tokenises the line for colouring and completion; turns phrases, units, dates, bases and percent forms into a canonical expression tree sent to the engine |
| Maths engine helper | C++23 | helper `math-engine` | Links MathScript libraries. Evaluates trees: exact and decimal arithmetic, functions, matrices, statistics, finance, symbolic calls, notation export, sampling for plots. Capability mode, no handles except its IPC socket. Justification: speed and engine wrap (PLATFORM.md section 2 b; MathScript is the catalogue's named engine) |
| Plot sampler and 3D rasteriser | C++23 | inside `math-engine` | Adaptive sampling, marching squares, surface meshes, software 3D render to an image buffer in shared memory |
| Typesetter | ForgeScript layout over draw list (OF07-GAP-2) | app host | Echo and result display |
| Rates fetch helper | C++23 | helper `rates-fetch` | Only when direct update is on: one HTTPS GET to the fixed host, strict XML parse, range checks, writes a rate table through IPC. Justification: untrusted bytes and network isolation |
| Evaluation service | ForgeScript | app host (headless activation) | `org.pbsd.calculator.eval/1` for the launcher |

```
   keys, keypad                         +------------------+
        |                               |   math-engine    |
        v                               |  (MathScript:    |
 +-------------- app host -----------+  |   bignum, sym2,  |
 | FUSE UI <-> ForgeScript:          |  |   symbolic,      |
 |   front-end parser, tape, modes   |<-+   linalg, stats, |
 |   typesetter, plot scene          |->|   prob, finance, |
 +----^---------------------^--------+  |   special, ode)  |
      | eval/1 (bus)        | rates     +------------------+
   launcher (SH01)     +-------------+        no files, no network
                       | rates-fetch | --TLS--> one fixed host (optional, off)
                       +-------------+
   signed data in the bundle: units.tbl, constants.tbl, rates package (svc.update)
```

How MathScript is used (Proposed), on top of what exists (Verified):

- Verified: MathScript is a C++23 library of 35 static libraries with `Result<T>` errors and no external BLAS (`MathScript:README.md`). It ships a REPL interpreter class `ms::interp::Interpreter` (`MathScript:include/ms/interp/repl_engine.hpp`) and executables `mathscriptc`, `mathscript-repl`, `mathscript-server`.
- Verified: the Qt 6 GUI (`MathScript:src/gui/`, `MS_BUILD_GUI`), the LLVM ORC JIT (`MS_BUILD_JIT`), CUDA and MPI are CMake options that default to OFF (`MathScript:cmake/options.cmake`, `MathScript:THIRD_PARTY.md`). The helper is built with all four OFF. The JIT must stay off (no-JIT rule).
- Verified: vendored third-party code is GoogleTest (tests only) and xsimd (BSD-3-Clause) with checksums (`MathScript:THIRD_PARTY.md`), so an offline build is plausible; a HardenedBSD build is Unverified.
- Proposed: the helper calls library headers directly (`ms/bignum`, `ms/sym2`, `ms/symbolic`, `ms/linalg`, `ms/stats`, `ms/prob`, `ms/finance`, `ms/special`, `ms/numthy`, `ms/combo`, `ms/poly`, `ms/ode`) rather than feeding text to the REPL, so that no string evaluation path with file commands (`run_file`, `source`, save and load, listed in `MathScript:docs/USER_GUIDE.md`) is reachable from user input.
- Proposed additions the calculator needs and the engine does not show: a decimal floating type with chosen precision; run-time unit quantities (`ms/core/units.hpp` is a compile-time system; whether `ms/core/scalar.hpp` covers run-time conversion is Unverified); exact radicals and constants; wider integration.

### 6.2 Data model

| Entity | Fields |
|---|---|
| `Session` | `id: uuid`, `name: text`, `created: i64`, `private: bool`, `settings: SessionSettings{angle, model, precision, format, word_bits, signed, overflow, complex}`, `next_line: u32` |
| `TapeEntry` | `session_id`, `line: u32`, `input: text`, `canonical: bytes` (expression tree), `echo_mathml: text`, `result: Value`, `result_text: text`, `label: text?`, `time: i64`, `pinned: bool`, `depends_on: [u32]`, `elapsed_us: u32`, `error: text?` |
| `Value` | tagged union: `Integer{bytes}`, `Rational{num, den}`, `Decimal{coeff, exp, precision}`, `Float64`, `Complex{re, im}`, `Quantity{value, dims: [i8; 9], display_unit}`, `Money{value, currency}`, `Date`, `DateTime{instant, zone}`, `Duration`, `List`, `Matrix{rows, cols, cells}`, `Symbolic{tree}`, `Boolean`, `Text` |
| `Variable` | `session_id`, `name`, `value: Value`, `pinned: bool`, `defined_at: u32` |
| `UserFunction` | `scope: enum{session, library}`, `name`, `params: [name, unit?]`, `body: bytes`, `source: text` |
| `Plot` | `session_id`, `id`, `kind: enum{fx, parametric, polar, implicit, inequality, points, surface, curve3d, field}`, `expr: bytes`, `domain`, `style{colour, width, dash}`, `visible` |
| `GraphView` | `session_id`, `x_min, x_max, y_min, y_max`, `log_x, log_y`, `equal_axes`, `camera` (3D), `sliders: [Slider{name, min, max, step, value}]` |
| `UnitDef` | `symbol`, `names`, `dims: [i8; 9]` (7 SI base + angle + currency), `factor: Rational or Decimal`, `offset`, `exact: bool`, `system`, `category` |
| `RateTable` | `date: date`, `base: text`, `rates: {code → Decimal}`, `source: enum{bundle, system_update, direct, manual}`, `fetched: i64?`, `sig_ok: bool` |

Stores:

| Store | Path | Format |
|---|---|---|
| Sessions | `~/.local/share/org.pbsd.calculator/sessions.sqlite` | SQLite, WAL, tables `session`, `tape`, `variable`, `function`, `plot`, `graph_view`, `schema_version`; one transaction per committed line |
| User library | same database, tables `library_function`, `user_unit`, `manual_rate`, `holiday` | |
| Rate tables | `~/.local/share/org.pbsd.calculator/rates/<date>.tbl` plus the read-only system data package | key=value text with a detached signature for packaged tables |
| Unit and constant tables | bundle `assets/units.tbl`, `assets/constants.tbl` | key=value, signed with the bundle |
| Settings | `svc.settings` namespace `org.pbsd.calculator` | |

Schema versions migrate forward only; the previous database file is kept as `sessions.sqlite.v<n>`.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Expression text | yes | yes | Own front-end parser (ForgeScript); engine tree codec | app host (ForgeScript, memory safe); tree validated again in `math-engine` |
| LaTeX subset | yes | yes | MathScript `sym2` reader and printer (Verified) | `math-engine` |
| MathML (presentation, content) | no | yes | MathScript `sym_export` (Verified) | `math-engine` |
| CSV (data lists, schedules, tables) | yes | yes | Own code | app host for small pasted text; a file over 1 MiB is parsed in `math-engine` from a passed handle |
| PNG, SVG, PDF (graphs, tape) | no | yes | Platform image and print pipelines (FUSE-WP-27, G-16); SVG written by own code | app host |
| Rate table (`.tbl`) | yes | yes | Own code, signature check by platform crypto | app host |
| Daily reference rates XML over HTTPS | yes | no | Own strict XML reader; platform TLS (Unverified which library) | `rates-fetch` |
| Engine IPC `org.pbsd.calculator.engine/1` | — | — | Bus schema (G-08), shared memory for plot buffers | — |
| Launcher service `org.pbsd.calculator.eval/1` | — | — | Bus schema: `eval(text) → {ok, text, mathml}` | app host |

### 6.4 Integration

- Intents handled: `open` for the URL scheme `calc:` (an expression to evaluate, shown, never auto-committed if it defines variables), `view` for `text/x-pbsd-calc-session` exports. Emitted: `share` (tape, graph), `print`, `open` (send to AI04 or OF05).
- MIME: export `text/plain`, `text/markdown`, `text/csv`, `application/x-latex`, `application/mathml+xml`, `image/png`, `image/svg+xml`, `application/pdf`.
- Services: `svc.settings`, `svc.clipboard`, `svc.intent`, `svc.portal.file` (exports, CSV import), `svc.portal.print`, `svc.update` (reads the rate data package version), `svc.a11y`, `svc.audit` (direct rate fetches), `svc.notify` (not used unless a long job finishes with the window hidden).
- Interface offered: `org.pbsd.calculator.eval/1` to SH01.
- Other apps: SH01 (launcher), AI04 (shares the engine and the typesetter need), OF05 (table hand-off), SY01 (global shortcut for mini mode), SH05 (keep-on-top rule), SY05 (delivers the rate data package).
- Backing repo facts are in 6.1 and in the feature rows of 4.11.

## 7. Security and privacy

| Capability | Why |
|---|---|
| `fs.app` | Sessions, user library, rate tables |
| `fs.portal` | Export tape and graphs; import a CSV list. Only on user action |
| `net.none` | Default |
| `net.host:www.ecb.europa.eu:443` (optional, off) | Direct currency rate update, used only by `rates-fetch` |
| `proc.helper:math-engine` | The engine |
| `proc.helper:rates-fetch` | The optional updater |
| `dev.print` | Print the tape |
| `dev.gpu.compute` (optional, off) | Faster 3D plots |
| `svc.settings`, `svc.clipboard`, `svc.intent`, `svc.portal.file`, `svc.portal.print`, `svc.update`, `svc.a11y`, `svc.audit`, `svc.notify` | As in 6.4 |

Assets: the history (it can reveal salaries, debts, health figures), the correctness of results (a wrong answer is the main harm a calculator can do), the rate table.

Adversaries: a hostile expression arriving by `calc:` link, paste or the launcher; a hostile or wrong rate source; a resource-exhaustion input (`9^9^9^9`, a huge factorial, a 10⁶×10⁶ matrix); another app reading history.

Mitigations:

| Threat | Mitigation |
|---|---|
| Expression as an attack on the engine | The engine helper has no files, network or processes; the REPL's file commands are not linked into the call path; trees are size-limited (64 KiB, depth 256) and validated |
| Resource exhaustion | Time and memory limits; size estimates before big-number operations (digits of the result are bounded first, and the user is asked above 1,000,000); helper restart |
| Wrong answers | Exact arithmetic by default for exact input; the model is always shown; interpretation echo for phrases and ambiguous precedence; symbolic results the engine cannot justify are refused, not guessed; differential tests (section 11) |
| `calc:` links | The expression is placed on the line and shown, with its source app named; assignments and function definitions need `Enter` from the user |
| Rate tampering | Packaged tables are signed; direct fetch uses TLS to one fixed host, values are range-checked against the previous table (a change over 25% in a day is flagged, not hidden); the date and source are always shown |
| Network metadata | Direct update is off by default; when on, one request a day, no identifiers, visible badge, audit line |
| History exposure | Stored only in the app directory; never indexed by `svc.search`; private sessions; clear by range; clipboard copies are ordinary (not marked secret) unless private |
| Launcher service misuse | No variables, no definitions, 100 ms budget, output length cap 4 KiB |

Stored: sessions and library in `sessions.sqlite`. Leaves the machine: nothing by default; with direct update on, one HTTPS request to the fixed host.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `mode.start` | enum | `last` | Mode at start |
| `number.model` | enum | `exact_when_possible` | `exact_when_possible`, `decimal`, `binary64` |
| `number.decimal_precision` | int | `34` | Digits, 1–10000 |
| `number.format` | enum | `auto` | |
| `number.fixed_places` | int | `2` | |
| `number.grouping` | enum | `locale` | `locale`, `space`, `none` |
| `number.rounding` | enum | `half_even` | |
| `number.complex` | bool | `false` | |
| `number.imaginary_unit` | enum | `i` | `i`, `j` |
| `angle.unit` | enum | `degrees` | |
| `parse.implicit_mul_binds_tighter` | bool | `false` | `1/2x` reads as `(1/2)x` when false |
| `parse.mixed_numbers` | bool | `false` | `2 1/2` as two and a half |
| `parse.phrases` | bool | `true` | Natural-language grammar |
| `live.result` | bool | `true` | |
| `live.budget_ms` | int | `200` | |
| `limit.time_s` | int | `5` | Ask to continue after this |
| `limit.memory_mib` | int | `512` | |
| `programmer.word_bits` | enum | `64` | |
| `programmer.signed` | bool | `true` | |
| `programmer.overflow` | enum | `wrap` | `wrap`, `saturate`, `error` |
| `units.system` | enum | `locale` | `si`, `imperial`, `us` |
| `currency.home` | text | locale | ISO code |
| `currency.direct_update` | bool | `false` | Needs the `net.host` grant |
| `currency.stale_days` | int | `7` | |
| `date.week_start` / `date.weekend` | enum / set | locale | |
| `history.persist` | bool | `true` | |
| `history.max_entries` | int | `10000` | Per session |
| `keypad.visible` / `keypad.position` | bool / enum | `true` / `bottom` | |
| `mini.keep_on_top` | bool | `true` | |
| `graph.reduced_motion_sliders` | enum | `system` | |

## 9. Accessibility and languages

- Every result is exposed as text and as a spoken form ("two times the square root of two, approximately 2.828"); typeset maths has a linear text alternative generated from the same tree.
- The live result is a polite live region, announced only after typing pauses; errors are assertive only on `Enter`.
- Keypad keys have full names ("sine", "x squared", "memory add"); the bit panel announces "bit 12, set".
- Graphs have a text summary, a value table and optional audio trace.
- Decimal and list separators follow the locale; `.` and `,` are both accepted when unambiguous and the echo shows the reading. Digits in other scripts (Arabic-Indic, Devanagari) are accepted on input; output follows the locale setting.
- Function names are English by default with localised aliases in the string tables. Unit names are localised; symbols are not.
- Large-digit and high-contrast layouts; at 200% scale the keypad switches to the basic set to keep 44 px targets.

## 10. Performance budgets

| Measure | Target |
|---|---|
| Cold start to typing | 150 ms (engine helper starts in parallel; first result under 250 ms) |
| Mini mode show (app running) | 100 ms |
| Launcher evaluation | 30 ms typical, 100 ms cap |
| Simple expression | under 2 ms in the engine |
| `2^1000000` exact | under 1 s |
| 2D plot of 4 functions, re-sample on pan | under 16 ms per frame at 1920×1080 |
| Implicit plot | first pass under 100 ms, refined under 500 ms |
| 3D surface 200×200 on CPU | 30 fps while rotating |
| Statistics on 1,000,000 values | under 300 ms |
| Memory | app host under 60 MiB; engine under 40 MiB idle |
| Idle | zero frames, zero wake-ups; no timers (rates checked only at start when enabled) |

## 11. Testing and proof

Test plan (targets; nothing has run):

- **Unit**: front-end parser (precedence table, implicit multiplication, percent forms, bases, dates, units, phrases) with a large table of input → canonical tree; unit conversion table consistency (every unit converts to base and back exactly where marked exact); date arithmetic against known calendars; word-size arithmetic for every size and overflow rule.
- **Property tests**: `parse(print(tree)) == tree`; `parse_latex(to_latex(e)) == e` is the engine's own stated property (`MathScript:docs/LATEX_SUBSET.md`), re-run through the helper boundary; exact arithmetic identities (`(a/b)*b == a`); conversion round trips.
- **Differential**: numeric results against an arbitrary-precision reference (for example GNU `bc` and MPFR-based tools, used only as test oracles); symbolic results checked by numeric evaluation at random points (derivative against finite differences, integral against differentiation of the result).
- **Golden renders**: each keypad, each mode panel, typeset echoes of a formula corpus, plots of a function corpus at fixed views.
- **Scripted UI**: the 14 top tasks through the a11y tree, keyboard only.
- **Fuzz**: front-end parser, tree codec at the engine boundary, LaTeX reader through the helper, rate XML, rate table, CSV, `calc:` URLs, launcher service input.
- **Resource tests**: bomb expressions must be refused or cancelled within the limits and leave the session intact.
- **Engine regression**: MathScript's own suites are reported upstream as 374 CTest suites (`MathScript:README.md`, `MathScript:docs/STATUS.md`); they have not been run here or on HardenedBSD.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Front-end parser (ForgeScript) | Total; every accepted line has one canonical tree; echo matches tree | T3 | Table and property tests | ForgeScript code is outside PRISM's C++ scope unless a ForgeScript front end exists (Unverified) |
| Tree codec at the engine boundary | No runtime errors on any bytes | T2 | PRISM plus fuzzing | — |
| Fixed-width integer arithmetic (programmer mode) | Matches two's-complement semantics for every size and overflow rule | T1 | PRISM proof against bit-vector specifications | — |
| Unit conversion core | Dimension algebra is sound; exact factors stay exact | T1 | Proof over rational arithmetic and dimension vectors | Depends on the proof of `Rational` |
| `BigInt` / `Rational` (MathScript) | Functional correctness | T2 target, T1 desirable | MathScript has a `verification/` directory run under ESBMC and CBMC (`MathScript:docs/STATUS.md`); the harnesses present are `isa_gating.c` and `miller_rabin_witness.c`, none for `BigInt`. PRISM job proposed | Size of the multiplication and division code |
| Decimal type (new) | Correct rounding | T1 | Proof against the specification | Not written yet |
| Date arithmetic | No overflow; civil-date conversions are inverse | T1 | Proof over pure integer functions | Time-zone data is external |
| Symbolic engine | Results are equal to the input mathematically | T3 | Differential numeric checks | No formal semantics; the engine documents unsupported forms |
| Numeric functions | Stated error bounds | T3 | Reference comparison | Floating-point proofs are costly |
| Plot sampler and 3D rasteriser | Memory safe | T3 | Sanitisers, fuzzing of scene descriptions | — |
| Rates fetch helper | No runtime errors; only one host contacted | T2 | PRISM on the parser; capability confinement for the host | TLS library is third party (T4) |

## 12. Build plan

Phase 1 — usable core:

| ID | Work item | Depends on |
|---|---|---|
| OF07-W01 | Build MathScript for HardenedBSD with JIT, GUI, CUDA and MPI off; record the result | G-18 analogue for MathScript (Unverified), G-06 |
| OF07-W02 | `math-engine` helper: IPC, tree codec, exact and binary64 arithmetic, elementary functions, limits and cancel | OF07-W01, G-07, G-08 |
| OF07-W03 | Front-end parser: numbers, operators, functions, percent, bases, `ans`, variables | G-02 |
| OF07-W04 | Code line field (OF07-GAP-1), completion popup | FUSE-WP-12, FUSE-WP-14 |
| OF07-W05 | Main window: tape, live result, basic and scientific keypads, status menus | OF07-W03, OF07-W04 |
| OF07-W06 | Typesetter (OF07-GAP-2) for echo and results | G-03, FUSE-WP-23 |
| OF07-W07 | Sessions store, history, variables, private session, crash recovery | OF07-W02 |
| OF07-W08 | Programmer mode: word sizes, bit panel, bases | OF07-W05 |
| OF07-W09 | Unit table, conversion syntax, converter panel, temperature rules | OF07-W03 |
| OF07-W10 | Currency with the bundled signed table, manual rates, stale warning | OF07-W09 |
| OF07-W11 | Mini mode and everyday-money panel | OF07-GAP-3, SH05 |
| OF07-W12 | Help, palette, shortcuts, engine information | OF07-W05 |

Phase 2 — full feature:

| ID | Work item | Depends on |
|---|---|---|
| OF07-W13 | Decimal type with chosen precision; exact-with-approximation display | OF07-W02 (engine work in MathScript) |
| OF07-W14 | Unit-aware arithmetic (quantities through formulas) | OF07-W09, OF07-W13 |
| OF07-W15 | Phrase grammar and interpretation echo | OF07-W03 |
| OF07-W16 | Dates, durations, time zones, date panel | OF07-W03 |
| OF07-W17 | Financial panels and decimal money | OF07-W13 |
| OF07-W18 | Statistics mode, distributions, quick plots | OF07-W02 |
| OF07-W19 | 2D graphing: sampler, navigation, analysis tools, sliders, export | OF07-W02, FUSE-WP-23 |
| OF07-W20 | Symbolic tools: simplify, expand, differentiate, integrate (as the engine allows), solve linear; copy as LaTeX, MathML and code; paste LaTeX | OF07-W06 |
| OF07-W21 | Complex numbers, lists, matrices, numeric calculus, special functions, number theory | OF07-W02 |
| OF07-W22 | Launcher evaluation service | SH01, G-08 |
| OF07-W23 | Rate data package through `svc.update`; optional `rates-fetch` helper | SY05, G-09, platform TLS decision |
| OF07-W24 | User functions, tape editing with dependants, export and print | OF07-W07, G-16 |

Phase 3 — advanced:

| ID | Work item | Depends on |
|---|---|---|
| OF07-W25 | 3D plots, contours, direction fields, complex plots | OF07-W19 |
| OF07-W26 | Limits, series, transforms, ODEs, polynomial tools, assumptions | OF07-W20, engine work in MathScript |
| OF07-W27 | Business days, historical rates, user units, uncertainty arithmetic | OF07-W16, OF07-W14 |
| OF07-W28 | Step display, formula library, number words | OF07-W15 |
| OF07-W29 | Graph accessibility: descriptions and audio trace | OF07-W19, `svc.media` |
| OF07-W30 | Hand-off to AI04 and OF05 | AI04, OF05 |

## 13. Open questions and risks

- **Engine reach.** MathScript's symbolic part is small by its own description. The calculator must be honest about that in the UI; a stronger CAS is engine work outside this app's plan. Features in 4.11 marked Proposed depend on it.
- **Decimal and exact-radical types do not exist in the engine as read.** Until they do, "decimal" mode would fall back to rationals with late rounding, which is exact but can be slow for transcendental functions.
- **Calling libraries directly instead of the REPL** needs a stable C++ surface per module; the REPL registry (`src/interp/matrix_calls/<domain>/`) is the tested path upstream. If the helper must use the REPL, the file-related commands have to be compiled out or blocked, which needs an engine option that was not found.
- **MathScript on HardenedBSD** is Unverified (CI covers Windows, Linux, macOS). xsimd and AVX options need checking with the hardened toolchain.
- **Run-time units.** `ms/core/scalar.hpp` was not read in detail; the plan assumes the app owns the unit tables.
- **Rate source.** One fixed host gives euro reference rates for a limited set of currencies (about 30), fewer than the 170 the bundled table aims at. Either the bundled table comes from a broader open source at packaging time, or the count is lowered. Undecided.
- **Mathematics typesetting** (OF07-GAP-2) is a shared platform need with no owner yet; a mathematical font with an OpenType MATH table must ship with the system.
- **Global shortcut and keep-on-top** depend on SH05 and the compositor.
- **`calc:` URL scheme** is convenient and is also an input path from other apps; it could be dropped with little loss.
