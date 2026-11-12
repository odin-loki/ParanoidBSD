<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Forecast

By Odin Loch

| | |
|---|---|
| ID | `AI03` |
| Bundle | `org.pbsd.forecast` |
| Category | ai |
| Priority | P2 |
| Backing repos | Cypha (`native/` forecast module, classifier and regressors, `cypha_rest`), FORGE (plugin session kind, `.fpk` container) |
| Status | Planned. Nothing built. |

## 1. Purpose

Forecast is a studio for time-series forecasting on the user's own data: import a table, clean it, look at it, fit several models, backtest them honestly, and produce forecasts with prediction intervals that can be scheduled, compared and exported. It is for analysts, small businesses, researchers and anyone with a spreadsheet of dated numbers. It replaces spreadsheet trend lines and cloud forecasting services. All computation is local.

## 2. Scope

### 2.1 What Cypha's forecasting module is today (Verified)

The catalogue calls this app a "time-series forecasting studio on Cypha's forecasting module". The module was read. It is not a general time-series library.

| Fact | Source |
|---|---|
| The module is **event forecasting** for armed-conflict data: dispute, conflict, crisis and news-event records, per-theatre models, a scenario tree, and scoring against fatality observations. | `CYPHA:native/include/cypha/forecast/*.hpp`, `CYPHA:docs/MODULES.md` (row `forecast`) |
| Cypha's own status: "forecasting is a scaffold under repair" with no results yet. | `CYPHA:README.md:259`, `CYPHA:MODEL_CARD.md:58`, `CYPHA:docs/RESEARCH_STATUS.md:25` |
| Inputs are CSV files in a directory: MID-style dyad-years (`DisputeRecord`), UCDP-style onsets (`ConflictRecord`), ICB-style crises (`CrisisRecord`), GDELT CAMEO events (`GdeltEvent`), VIEWS country-month fatalities (`ViewsObservation`). | `CYPHA:native/include/cypha/forecast/dispute_data.hpp`, `views_scoring.hpp` |
| Parts: `NodeEstimator` (a `cypha::Cypha` classifier on dispute features), `EventVocabulary` (theatre × escalation × CAMEO root tokens), a CyphaLM sequence model over event tokens, `generate_rollout_tree` (MCTS-style scenario tree, defaults depth 8, 200 rollouts, top-3 expansion), `extract_interpretable_paths`, `TheaterModelBank` (per-theatre models with EWC and replay), `GdeltMonitor` (drift and anomaly alarm over a window of 128), `GdeltCsvTail` (poll a growing CSV), fatality-bin baselines and a leaderboard (`conflictology`, `observed_markov`, `negbin_glmm`), `crps` and `ignorance_score`. | `CYPHA:native/include/cypha/forecast/` (`node_estimator.hpp`, `event_vocabulary.hpp`, `rollout_tree.hpp`, `interpretability.hpp`, `theater_models.hpp`, `gdelt_monitor.hpp`, `gdelt_stream.hpp`, `views_baselines.hpp`, `views_scoring.hpp`) |
| Orchestrator: `ForecastPipeline::run(data_dir)` trains everything from the CSVs on each call and returns node accuracy, sequence BPC, the tree, annotated paths, a VIEWS validation result and a drift-alarm count. Seeds are fixed at 42. | `CYPHA:native/include/cypha/forecast/forecast_pipeline.hpp`, `CYPHA:native/src/forecast/forecast_pipeline.cpp` |
| Command-line tools: `cypha_forecast_run [--data DIR]` (prints `node_eval_acc`, `sequence_eval_bpc`, `tree_nodes`, `views_crps`), `forecast_smoke`, `forecast_drift_backtest`, `forecast_gdelt_poll`, `views_leaderboard_bench`. | `CYPHA:native/tools/forecast/` |
| REST: `cypha_rest` has `POST /forecast/run`, `POST /forecast/ingest`, `POST /forecast/monitor/reset`. `/forecast/run` takes a server-side `data_dir` path from the request body, retrains the whole pipeline on every call, and returns summary numbers, not a forecast. `/forecast/ingest` takes `drift` and `anomaly` from the caller; the server scores nothing itself. | `CYPHA:native/src/serve/cyphalm_rest_routes.cpp:393-479`, `CYPHA:docs/reports/FC0_FORECAST_AUDIT.md:199-207` |
| Known defects from Cypha's own audit (FC0): 6 of 7 committed CSVs are UTF-16 and the loaders read none of their rows; the sequence model throws on readable events; the escalation estimator uses same-year inputs and a random split; the "holdout" forecast reads the held-out value; the CRPS function is wrong (100 where the exact value is 470); tree uncertainty is a constant 0; no time steps, no horizons, no intervals, no saved model, no fixtures. | `CYPHA:docs/FORECAST_PLAN.md` section 1, `CYPHA:docs/reports/FC0_FORECAST_AUDIT.md` |
| Cypha has a repair plan (items FC2–FC16: strict loaders, real data with a manifest, a `History` type that enforces the origin rule, exact sample CRPS, IGN, MIS, Brier, ECE, PIT, a VIEWS-protocol forecaster with 1 000 draws, time-stepped tree with intervals, a saved forecast bundle, `/forecast/load`, `/forecast/predict`, `/forecast/ingest`). Only FC0 and FC1 are done. | `CYPHA:docs/FORECAST_PLAN.md` sections 2 and 6 |
| No ARIMA, exponential smoothing, seasonal decomposition or autocorrelation code exists in the module. | read of `CYPHA:native/src/forecast/` and `native/include/cypha/forecast/` |

Other Cypha parts a time-series app can build on (Verified to exist, not verified for forecasting quality):

| Part | Source |
|---|---|
| `cypha::Cypha` online classifier and regressors with calibrated uncertainty: mixture-of-experts scalar prediction `y = Σ p·μ`, `unc = √(Σ p·var)`; RLS and two-stage ridge fits. | `CYPHA:native/include/cypha/classify/regression.hpp` |
| A quantile variant with a committed training fixture. | `CYPHA:fixtures/quantile_dif_train/` |
| Per-prediction `uncertainty`, `anomaly_score`, `is_ood`; `drift_score()`, `active_query_score()`; save and load of `.cypha` files. | `CYPHA:native/include/cypha/cypha.hpp:46-48`, `:140`, `:236-238` |
| `TimeSeriesEncoder`: sliding windows (default 50) to statistical plus rFFT features (default 10 coefficients). It lives in the benchmark code, not in a public module. | `CYPHA:native/include/cypha/eval/bench_encoder_timeseries.hpp` |
| UTF-8 CSV parsing and dense loading. | `CYPHA:native/include/cypha/core/csv_ingest.hpp` |
| A Qt 6 shell and a web UI served by `cypha_rest` exist; neither is used here. | `CYPHA:native/qt/README.md`, `CYPHA:native/tools/serve/static/` |

Note: "M4" in `regression.hpp` and `fixtures/regression_m4/` is a port milestone number, not the M4 forecasting competition.

### 2.2 `cypha_rest` (Verified) and how this app talks to Cypha

- `cypha_rest` is an HTTP server built on the vendored cpp-httplib 0.18.3 (`CYPHA:native/third_party/httplib.h:11`). Default listen is `127.0.0.1:8099` (`CYPHA:native/tools/serve/cypha_rest.cpp:1695-1696`); `--listen host:port` accepts any address.
- No authentication was found in `cypha_rest.cpp` or the route files (searched for `Authorization`, `Bearer`, `Access-Control`). `/forecast/run` runs without the server's mutex (`FC0_FORECAST_AUDIT.md:269`).
- **Problem to fix.** The default is loopback, not a public address, but on this OS loopback TCP without authentication is ambient authority: any process that may open a loopback socket can retrain models, read results, and make the server read a directory of its choosing. `--listen 0.0.0.0` would expose the same to the network. This conflicts with SI-2 (PLATFORM.md V19).
- **Plan.** Phase 1 does not use `cypha_rest`. The forecast engine helper links `cypha_core` in-process (the Qt shell README names linking `cypha_core` directly as a supported path). If a REST path is ever wanted (Phase 3, to reuse Cypha's contract tests), it runs only on a Unix-domain socket created by the app host and passed to the server as an inherited descriptor, with peer credentials checked; TCP is compiled out; `/forecast/run` is removed (Cypha's own FC12 plans this); the static web UI is not built. The vendored httplib has `AF_UNIX` code paths (`httplib.h:3413`); whether `cypha_rest` can be started on an inherited descriptor is Unverified and is work item AI03-W27.

### 2.3 In scope (all Proposed)

- Projects holding datasets, series, models, backtests, forecasts, scenarios, monitors and reports.
- Import from CSV, TSV, SQLite, Parquet, and hand-off from OF05 and OF09.
- Cleaning, resampling, exploration, decomposition, autocorrelation.
- Classical forecasting methods written new in the engine helper, and learned forecasters on Cypha regressors.
- Backtesting with proper scores and baselines beside every number; prediction intervals; scenario comparison.
- Anomaly detection; scheduled re-forecasts; reports; an interface for other apps; reproducibility records; explanation views.
- An event-forecasting workspace on Cypha's module once its FC track delivers a loadable bundle.

### 2.4 Out of scope

| Item | Owner |
|---|---|
| General spreadsheet editing | OF05 |
| Database browsing and SQL authoring | OF09 |
| Notebook-style free computation | AI04 |
| Weather forecasts | IN05 |
| Model signing, trust policy, per-app model grants | AI02 |
| Downloading datasets from the network | IN03 (Forecast has no network capability) |
| Repairing Cypha's forecast module (FC2–FC16) | the Cypha repository |
| System metrics history and alerting | SY04 |

## 3. Users and top tasks

Personas: **Tess**, a shop owner forecasting weekly sales from a spreadsheet; **Dana**, an analyst who must defend the error bars; **Raj**, an engineer watching sensor data for anomalies; **Kim**, a researcher who needs every run to be reproducible; **Ari**, who wants a monthly re-forecast to run by itself and tell him when the data drifted.

| # | Task | Target steps |
|---|---|---|
| T1 | Import a CSV and see the series plotted | 3 (drop file, confirm columns, Finish) |
| T2 | Get a first forecast with intervals | 2 (pick horizon, Forecast) |
| T3 | Fix gaps and outliers | 3 |
| T4 | See seasonality and trend | 1 (Explore tab) |
| T5 | Compare five models by backtest | 3 |
| T6 | Read which model is best and by how much, with uncertainty | 1 |
| T7 | Make a "what if" scenario and compare | 4 |
| T8 | Flag anomalies in the last month | 2 |
| T9 | Schedule a monthly re-forecast | 3 |
| T10 | Export forecast to CSV or back to Sheets | 2 |
| T11 | Produce a PDF report | 3 |
| T12 | Re-run last year's forecast exactly | 2 |
| T13 | Explain why the forecast went up | 1–2 |
| T14 | Forecast 200 series at once | 4 |

## 4. Features

Priority: M must, S should, C could. Phase: 1 usable core, 2 full feature, 3 advanced.

### 4.1 Projects and data import

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI03-F001 | Project file | One `.fcproj` file (SQLite, documented schema, section 6.2) holds everything. Default save format. | M | 1 |
| AI03-F002 | New, open, save, save as | Reserved shortcuts; autosave every 30 s to a journal beside the file. | M | 1 |
| AI03-F003 | Recent projects | From the portal's recent list. | M | 1 |
| AI03-F004 | Import CSV and TSV | Detects delimiter, quote, decimal mark, header row, encoding (UTF-8, UTF-16 with BOM, Latin-1). A non-UTF-8 file is converted with a visible notice, never read as garbage. | M | 1 |
| AI03-F005 | Column mapping | Pick the time column, one or more value columns, optional group columns (for many series) and regressor columns. Preview of the first 200 rows. | M | 1 |
| AI03-F006 | Time parsing | ISO 8601, common day-first and month-first forms (the user confirms which), Unix seconds and milliseconds, year-week, year-month, fiscal periods. Time zone choice; daylight-saving handling stated. | M | 1 |
| AI03-F007 | Fail closed | A missing column, an unparseable cell or a wrong encoding is an error that names file, row and column. Nothing is silently defaulted. (The rule Cypha's plan sets for its own loaders, `CYPHA:docs/FORECAST_PLAN.md` section 3 rule 6.) | M | 1 |
| AI03-F008 | Import SQLite | Pick a table or view, or type a read-only `SELECT`; the file is opened read-only in the import helper. | S | 2 |
| AI03-F009 | Import Parquet | Flat schemas with integer, float, string, date and timestamp columns; plain and dictionary encodings; uncompressed, Snappy and Zstandard pages. Nested columns are refused by name. | S | 2 |
| AI03-F010 | Hand-off from Sheets | OF05 shares a selected range through intent `share` with `text/csv` plus cell types; Forecast opens the mapping step. | M | 2 |
| AI03-F011 | Hand-off from Data Studio | OF09 shares a query result the same way. | S | 2 |
| AI03-F012 | Paste | Tabular text from the clipboard goes through the same mapping step. | S | 1 |
| AI03-F013 | Long and wide layouts | Wide (one column per series) and long (group column) tables both map to series. | M | 1 |
| AI03-F014 | Linked or embedded data | Embedded: rows copied into the project. Linked: the project keeps a portal grant and a content hash, and re-reads on Refresh. | S | 2 |
| AI03-F015 | Refresh from source | Re-import with the saved mapping; shows added, changed and removed rows before applying. | S | 2 |
| AI03-F016 | Data provenance | Each dataset records source name, size, BLAKE3 and SHA-256, import options, date. | M | 1 |
| AI03-F017 | Sample data | Three small synthetic series shipped in the bundle, named `synthetic_*`, for the tour. | C | 1 |
| AI03-F018 | Size limits | 50 million rows and 10 000 series per project by default; streaming import with a row counter and cancel. | M | 2 |

### 4.2 Cleaning and preparation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI03-F019 | Frequency detection | Infers the step (second to year), reports irregular gaps and duplicate timestamps. | M | 1 |
| AI03-F020 | Resample | To a coarser or finer regular step with a chosen aggregate (sum, mean, last, first, min, max, count) or interpolation. | M | 1 |
| AI03-F021 | Missing values | Per series: leave, forward fill, linear, seasonal interpolation, model-based fill; filled points are marked and never count as observations in scores. | M | 1 |
| AI03-F022 | Duplicates | Keep first, last, sum or mean; the choice is recorded. | M | 1 |
| AI03-F023 | Outlier marking | Rules: robust z-score on the remainder after decomposition, interquartile fence, manual brush. Marked points can be kept, down-weighted, or replaced. | S | 2 |
| AI03-F024 | Transforms | Log, Box–Cox (lambda fitted or set), square root, difference, seasonal difference, scale by a column (per capita), inflation adjust with a user-supplied index series. Forecasts are back-transformed with bias adjustment where it applies. | S | 2 |
| AI03-F025 | Calendar | Trading-day and holiday calendars: user tables only (ICS import through the portal); none fetched. | S | 2 |
| AI03-F026 | Regressors | Mark columns as known-in-advance (price, promotion, holiday) or observed-only; the engine refuses a forecast that needs future values that are absent. | S | 2 |
| AI03-F027 | Train, validation and test ranges | Set by date on the chart; always blocks of time, never shuffled. | M | 1 |
| AI03-F028 | Preparation recipe | The ordered list of steps is a visible, editable pipeline; each step can be disabled; the recipe is hashed. | M | 1 |
| AI03-F029 | Before and after view | Chart overlays raw and prepared series. | M | 1 |
| AI03-F030 | Hierarchies | Group series into a hierarchy (region › shop › product) for aggregation and reconciliation (F064). | C | 3 |
| AI03-F031 | Data table | Virtualised `tableView` of the series with marks for filled, outlier, excluded; inline edit with undo. | M | 1 |
| AI03-F032 | Data quality summary | Rows, span, step, gaps, zeros, negatives, constant runs, share filled; warnings such as "one class" or "constant column" block a fit. | M | 1 |
| AI03-F033 | Units and labels | Unit, display name, decimal places and colour per series. | S | 1 |
| AI03-F034 | Leakage check | Flags a regressor whose correlation with the target at lag 0 is near 1, and any feature computed from the target at the same period. | S | 2 |

### 4.3 Exploration

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI03-F035 | Time plot | Zoom, pan, range brush, cross-hair read-out, several series, secondary axis, log scale. | M | 1 |
| AI03-F036 | Seasonal plots | Values by position in the season (month of year, day of week, hour of day), one line per cycle. | S | 1 |
| AI03-F037 | Subseries plot | One small panel per season position with its mean. | C | 2 |
| AI03-F038 | Decomposition | STL (seasonal-trend by loess) with window settings; classical additive and multiplicative; multiple seasonal periods (MSTL) in Phase 2. Panels: observed, trend, seasonal, remainder. | M | 1 |
| AI03-F039 | Autocorrelation | ACF and PACF bar charts with a confidence band; lag slider. | M | 1 |
| AI03-F040 | Periodogram | Spectrum with dominant periods listed. | S | 2 |
| AI03-F041 | Stationarity tests | ADF and KPSS statistics with plain-language reading and the caution that tests are hints. | S | 2 |
| AI03-F042 | Distribution | Histogram, quantiles, rolling mean and variance. | S | 1 |
| AI03-F043 | Lag plots and cross-correlation | Scatter of y(t) against y(t−k); cross-correlation with a regressor. | C | 2 |
| AI03-F044 | Strength measures | Trend strength and seasonal strength from the decomposition, intermittency (share of zeros, mean interval), spectral entropy. | S | 2 |
| AI03-F045 | Many-series overview | A grid of small charts (`gridView`), sortable by length, strength, intermittency, last value. | S | 2 |
| AI03-F046 | Change points | Marks level and trend shifts found by a penalised cost search; the user confirms or rejects each. | C | 3 |
| AI03-F047 | Annotations | Notes pinned to dates ("store closed"); shown on every chart and in reports. | S | 1 |
| AI03-F048 | Copy chart | As PNG and SVG to the clipboard or a file. | S | 1 |

### 4.4 Models

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI03-F049 | Baselines, always fitted | Naive (last value), seasonal naive, drift, mean, last-window mean. Every table shows them. Own code. | M | 1 |
| AI03-F050 | Exponential smoothing | Simple, Holt, damped Holt, Holt–Winters additive and multiplicative; ETS state-space form with selection by AICc; analytic or simulated intervals. Own code. | M | 1 |
| AI03-F051 | Theta | Standard and optimised Theta. Own code. | S | 1 |
| AI03-F052 | ARIMA | Non-seasonal and seasonal ARIMA with regressors; order search with stepwise AICc; fixed orders allowed. Own code. | S | 2 |
| AI03-F053 | Intermittent demand | Croston, SBA, TSB. Own code. | S | 2 |
| AI03-F054 | Regression with calendar terms | Linear model with trend, Fourier seasonal terms, holidays and regressors. Own code. | S | 2 |
| AI03-F055 | Cypha lag regressor | Features: lags, rolling statistics and, optionally, `TimeSeriesEncoder`-style window features (statistical plus rFFT). Model: a `cypha::Cypha` regressor (mixture of experts) giving a mean and an uncertainty per step. Direct or recursive multi-step. Requires the encoder to move out of Cypha's benchmark code. | S | 2 |
| AI03-F056 | Cypha quantile forecaster | Quantile outputs (for example 5, 25, 50, 75, 95%) from Cypha's quantile variant, giving intervals without a distribution assumption. | C | 3 |
| AI03-F057 | Online updating | A Cypha model can learn from each new observation without a full refit; the app records every update and can roll back to a saved state. | C | 3 |
| AI03-F058 | Ensemble | Simple average, median, or weights from backtest error; the members are listed; intervals from pooled simulation. | S | 2 |
| AI03-F059 | Automatic choice | "Auto" fits the allowed families and picks by backtest score, not in-sample fit. The table still shows all candidates. | M | 1 |
| AI03-F060 | Model configuration | A `propertyGrid` per family with defaults, ranges and a reset; an advanced toggle. | M | 1 |
| AI03-F061 | Constraints | Non-negative, integer, bounded forecasts; applied to draws, not by clipping means. | S | 2 |
| AI03-F062 | Global models | One Cypha model across many series with a series id feature. | C | 3 |
| AI03-F063 | Cold start | For a short series: fall back to baselines and say so; borrow seasonality from a chosen parent series. | S | 2 |
| AI03-F064 | Reconciliation | Bottom-up, top-down and MinT-style reconciliation over a hierarchy so totals add up. | C | 3 |
| AI03-F065 | Model library | Saved model specifications reusable across projects (a spec is settings, not fitted state). | S | 2 |
| AI03-F066 | Save fitted model | Fitted state saved in the project; Cypha models as `.cypha` blobs with a hash. | M | 2 |
| AI03-F067 | Register for other apps | A fitted Cypha model can be exported as a signed `.fpk` (sections as in `FORGE:examples/integrations/cypha_classifier.cpp`) and registered in AI02 as class `plugin`, to be served by `svc.model` (FORGE-WP-31.9). | C | 3 |
| AI03-F068 | Honest naming | A row is called "Cypha" only when a Cypha model produced it; heuristics are called baselines (Cypha plan rule 8). | M | 1 |

### 4.5 Training and backtesting

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI03-F069 | Fit job | Runs in the engine helper with progress, cancel, log; several series and models in parallel up to a thread limit. | M | 1 |
| AI03-F070 | Origin rule | A forecaster receives only data up to the forecast origin. The engine's interface has no call that takes both history and future truth. | M | 1 |
| AI03-F071 | Rolling-origin backtest | Expanding or sliding window; number of folds, step between origins, horizon. Folds are drawn on the chart. | M | 1 |
| AI03-F072 | Point metrics | MAE, RMSE, median absolute error, MASE and RMSSE (scaled by in-sample seasonal naive), sMAPE and MAPE (hidden when values are near zero, with the reason), bias. | M | 1 |
| AI03-F073 | Probabilistic metrics | Exact sample CRPS, pinball loss per quantile, interval coverage at 50, 80, 95%, mean interval score, log score where a density exists, PIT histogram. | M | 1 |
| AI03-F074 | Skill scores | Each metric as skill against seasonal naive, with a bootstrap confidence interval on the difference. | M | 1 |
| AI03-F075 | Per-horizon results | Error by step ahead, as table and chart. | M | 1 |
| AI03-F076 | Leaderboard | Sortable table of models × metrics with baselines pinned; ties within the interval are marked "not distinguishable". | M | 1 |
| AI03-F077 | Significance | Diebold–Mariano test between two models on backtest errors, with its assumptions stated. | C | 3 |
| AI03-F078 | Residual diagnostics | Residual plot, ACF of residuals, Ljung–Box, normal quantile plot. | S | 2 |
| AI03-F079 | Calibration view | Reliability curve of nominal against observed coverage. | S | 2 |
| AI03-F080 | Interval calibration | Conformal adjustment of interval width from backtest residuals; reported as "calibrated on N folds". | S | 2 |
| AI03-F081 | Test set read once | The final test range is locked; opening its scores is a deliberate action that is recorded with a date; later refits show "test already seen". | S | 2 |
| AI03-F082 | No in-sample headline | In-sample fit is shown only in diagnostics, never as the headline accuracy. | M | 1 |
| AI03-F083 | Degenerate input fails | Zero rows, a constant series, fewer observations than parameters: the fit fails with a reason instead of returning numbers. | M | 1 |
| AI03-F084 | Batch backtest | Run a model set over many series; aggregate scores with a distribution, not only a mean. | S | 2 |
| AI03-F085 | Resource limits | Thread count, memory cap and wall-clock cap per job; defers on battery if set. | S | 2 |
| AI03-F086 | Hyper-parameter search | Grid or random search on the validation range only; the number of configurations tried is reported beside the winning score. | C | 3 |

### 4.6 Forecasts and intervals

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI03-F087 | Forecast | Horizon in steps or to a date; produces a mean, median, quantiles and stored sample paths (default 1 000 draws). | M | 1 |
| AI03-F088 | Fan chart | History, forecast median, 50/80/95% bands; toggles per band. | M | 1 |
| AI03-F089 | Interval method shown | Each forecast states how its intervals were made: analytic, simulation, quantile model, conformal. | M | 1 |
| AI03-F090 | Forecast table | Period, point, lower and upper per level; copy and export. | M | 1 |
| AI03-F091 | Cumulative and aggregate forecasts | Totals over the horizon or by quarter and year, with intervals computed from the sample paths (not by adding interval ends). | S | 2 |
| AI03-F092 | Probability queries | "Chance that the value exceeds X in period P" and "chance the total exceeds Y", answered from the draws with a Monte Carlo error. | S | 2 |
| AI03-F093 | Forecast versions | Every forecast is kept with its origin date; overlay past forecasts on actuals. | M | 1 |
| AI03-F094 | Actuals tracking | When new data arrives, past forecasts are scored and the running accuracy is plotted. | S | 2 |
| AI03-F095 | Manual adjustment | An override layer with a required note; the unadjusted forecast is kept and both are scored later. | C | 3 |
| AI03-F096 | Prospective register | A forecast for a period that has not happened can be sealed: hashed, time-stamped and optionally signed, then scored once when truth arrives (Cypha plan rule 9). | C | 3 |

### 4.7 Scenarios

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI03-F097 | Scenario | A named set of future regressor values or structural assumptions (growth rate, level shift, event on a date). | S | 2 |
| AI03-F098 | Scenario editor | Edit future regressors in a table or by dragging points on a chart (`curveEditor`). | S | 2 |
| AI03-F099 | Compare scenarios | Overlaid fan charts and a table of differences against the base case with intervals on the difference. | S | 2 |
| AI03-F100 | Shock | Apply a temporary or permanent shock of a given size and see its decay under the model. | C | 3 |
| AI03-F101 | Sensitivity | One-at-a-time sweep of a regressor; tornado-style bar chart. | C | 3 |
| AI03-F102 | Scenario labels | Scenarios are labelled "assumption", never "forecast", in charts and reports. | M | 2 |
| AI03-F103 | Goal seek | "What regressor value gives a median of X?" by search over the model. | C | 3 |

### 4.8 Anomaly detection and monitoring

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI03-F104 | Forecast-based anomalies | A point outside the one-step interval at a chosen level is flagged, with its tail probability. | M | 2 |
| AI03-F105 | Decomposition-based anomalies | Robust score on the STL remainder. | S | 2 |
| AI03-F106 | Cypha anomaly score | For Cypha models: `anomaly_score` and `is_ood` per observation, and `drift_score()` over time, plotted beside the series. | S | 3 |
| AI03-F107 | Thresholds from history | The threshold is set to a target false-alarm rate measured on a validation range, not typed blind. | S | 2 |
| AI03-F108 | Anomaly list | Table of flagged points with score, method, note; confirm, dismiss, or mark as a known event. | M | 2 |
| AI03-F109 | Monitor | A saved check on a linked dataset: re-read, score new points, notify on anomaly or drift. | S | 2 |
| AI03-F110 | Drift report | Shift of level, variance and seasonal pattern against the training window (population stability index and a two-sample test). | S | 3 |
| AI03-F111 | Model staleness | Warns when recent error exceeds the backtest error by a set factor; offers a refit. | S | 2 |
| AI03-F112 | Notifications | Through `svc.notify`: anomaly found, forecast refreshed, job failed, drift alarm. Content shows series name and date, not values, on the lock screen. | S | 2 |

### 4.9 Scheduling

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI03-F113 | Scheduled re-forecast | Per project: daily, weekly, monthly or on data change; steps: refresh linked data, refit or update, forecast, score past forecasts, export. | S | 2 |
| AI03-F114 | Background run | Uses `bg.run`; a small agent runs the engine helper without a window. The capability is requested when the first schedule is created. | S | 2 |
| AI03-F115 | Start at login | `bg.autostart`, requested only if a schedule exists; shown in AD05. | S | 2 |
| AI03-F116 | Run history | Each run: time, duration, data hash, result, errors; compare with the previous run. | S | 2 |
| AI03-F117 | Output targets | Write CSV to a granted folder, update a report, notify. No network targets. | S | 2 |
| AI03-F118 | Power and load rules | Run only on mains, only when idle, with a thread limit. | C | 2 |
| AI03-F119 | Missed runs | A run missed while the machine was off runs once at next login, marked "late". | S | 2 |
| AI03-F120 | Pause all | One switch stops every schedule; visible in the status bar. | M | 2 |

### 4.10 Reports, export, interface for other apps

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI03-F121 | Export forecast | CSV (default), JSON, Parquet; choice of point, quantiles, sample paths. | M | 1 |
| AI03-F122 | Send to Sheets | Intent `share` to OF05 with CSV; columns typed. | S | 2 |
| AI03-F123 | Export charts | PNG and SVG at a chosen size and theme. | M | 1 |
| AI03-F124 | Report builder | Blocks: title, data summary, charts, leaderboard, forecast table, scenarios, method notes, limits, reproducibility record. Output: PDF (G-16), Markdown, HTML (static). | S | 2 |
| AI03-F125 | Method notes | Generated plain text stating model, settings, backtest design and limits; fixed templates, no language model needed. | S | 2 |
| AI03-F126 | Optional summary by a local model | A paragraph written by a text model through `svc.model` from the numbers table; marked "generated text", off by default; the numbers table is always printed beside it. | C | 3 |
| AI03-F127 | Print | Charts and reports through `svc.portal.print`. | C | 3 |
| AI03-F128 | Forecast interface | IPC interface `org.pbsd.forecast.Forecast/1`: `forecast(series_handle, horizon, options) → quantiles`; another app passes a CSV handle and receives a result file handle. The first call from each app is prompted by the shell. | S | 3 |
| AI03-F129 | Intent `new` with a forecast request | `application/vnd.pbsd.forecast.request+json` opens a prepared project so the user sees and confirms the run. | S | 3 |
| AI03-F130 | ForgeScript module | A `forecast::` client module for app authors and for AI04 notebooks, wrapping F128. | C | 3 |
| AI03-F131 | Project export | A self-contained archive (project, embedded data, results) with a manifest of hashes; optional detached signature. | S | 2 |

### 4.11 Reproducibility and explanation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI03-F132 | Run record | Every fit, backtest and forecast stores: data hash, recipe hash, model spec hash, seed, engine version, Cypha commit, thread count, start and end time, result hash. | M | 1 |
| AI03-F133 | Deterministic mode | Fixed seed and single-thread reductions so a re-run gives the same bytes; the default for final forecasts. | M | 1 |
| AI03-F134 | Re-run and verify | Re-executes a record and compares the result hash; reports "identical" or shows the first difference. | S | 2 |
| AI03-F135 | Lineage view | A `nodeGraph` from dataset through recipe, model, backtest to forecast and report. | C | 3 |
| AI03-F136 | Sign a result | Detached Ed25519 signature over the run record and result hash with a key from SE04. | C | 3 |
| AI03-F137 | Component view | Forecast split into level, trend, seasonal, regressor and holiday contributions per period. | S | 2 |
| AI03-F138 | Change explanation | Between two forecast versions: how much of the change came from new data, a refit, a changed setting. | S | 3 |
| AI03-F139 | Feature importance | For regression and Cypha models: permutation importance on the validation range, with intervals. | C | 3 |
| AI03-F140 | Nearest history | For a Cypha model: the past windows most similar to the current one, shown on the chart. | C | 3 |
| AI03-F141 | Plain-language panel | Fixed-template sentences ("The forecast for March is 1 240, most likely between 1 100 and 1 390. The model beat seasonal naive by 12% ± 5% in backtests."). | S | 2 |
| AI03-F142 | Limits panel | Always shown with a forecast: length of history, number of folds, what the model cannot know (future events, regime changes). | M | 1 |

### 4.12 Event forecasting workspace (on Cypha's module)

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI03-F143 | Status gate | The workspace is hidden until Cypha's module delivers a loadable forecast bundle (its item FC11). Before that the page shows the status from `CYPHA:docs/FORECAST_PLAN.md` and nothing else. | M | 3 |
| AI03-F144 | Load bundle | Open a `<name>.forecast/` bundle (manifest, components, calibration, monitor state) after hash checks; signed bundles preferred. | C | 3 |
| AI03-F145 | Fatality distributions | Country-month draws as fan charts and bin tables with CRPS, IGN and MIS against the published baselines. | C | 3 |
| AI03-F146 | Event probabilities | P(level ≥ k within h weeks) per theatre with intervals. | C | 3 |
| AI03-F147 | Scenario tree view | The rollout tree in a `nodeGraph`: edge probability, path probability as the product of edges, uncertainty; top paths listed. Labelled "statistical continuations, not causes" (Cypha plan section 10). | C | 3 |
| AI03-F148 | Event data import | The module's CSV schemas, with the strict loader rules; data comes from files the user provides. | C | 3 |
| AI03-F149 | Monitor replay | Replay an event file through the drift monitor and list alarms. | C | 3 |
| AI03-F150 | Honest status line | Every view in this workspace carries the module's own status and the date it was read. | M | 3 |

### 4.13 Security, recovery, housekeeping

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI03-F151 | Sandboxed parsing | CSV, SQLite and Parquet files are parsed in the import helper with only the file handle. | M | 1 |
| AI03-F152 | Encrypted project option | A project can be sealed with a passphrase or a keyring key; charts and exports of a sealed project carry a lock mark. | S | 2 |
| AI03-F153 | Crash recovery | Journal replay on open; a running job's partial results are discarded, completed ones kept. | M | 1 |
| AI03-F154 | Undo | Unlimited undo for data edits, recipe and settings changes within a session. | M | 1 |
| AI03-F155 | Snapshots | Named project checkpoints; compare two checkpoints. | S | 2 |
| AI03-F156 | Jobs panel | All running and finished jobs with cancel and logs. | M | 1 |
| AI03-F157 | Tour and help | A five-step tour on sample data; help pages on each metric with its formula. | S | 1 |

## 5. GUI

### 5.1 Window map

```
Forecast
├─ Main window (one per project)
│  ├─ Header bar: menu, project name, search / palette, lock mark (sealed project)
│  ├─ Sidebar (F9): project tree — Data, Series, Models, Backtests, Forecasts, Scenarios,
│  │                Monitors, Schedules, Reports, Jobs
│  ├─ Stage bar: Data › Prepare › Explore › Model › Backtest › Forecast › Report
│  ├─ Content: the stage's view
│  │  ├─ Data: table and source list
│  │  ├─ Prepare: recipe pipeline and before/after chart
│  │  ├─ Explore: time plot, seasonal, decomposition, ACF/PACF, spectrum, overview grid
│  │  ├─ Model: model set and configuration
│  │  ├─ Backtest: folds chart, leaderboard, per-horizon, diagnostics, calibration
│  │  ├─ Forecast: fan chart, table, versions, scenarios, components, explanation
│  │  └─ Report: block editor and preview
│  ├─ Inspector (F4): properties of the selection, run record, limits panel
│  └─ Status bar: series count, selection range, jobs, schedules state, deterministic mark
├─ Anomalies view (from Monitors)
├─ Event workspace (Phase 3, gated)
├─ Wizards: Import data (Source → Columns → Time → Check → Finish)
├─ Dialogs
│  ├─ Resample · Missing values · Transform · Outlier rule
│  ├─ New backtest
│  ├─ New forecast
│  ├─ Scenario editor
│  ├─ Schedule
│  ├─ Export forecast · Export chart · Export project
│  ├─ Open test scores (confirmation)
│  └─ Preferences (Ctrl+,)
├─ Popovers: point read-out, metric definition, run record, annotation editor
└─ Shell-drawn: file and folder portals, print, background-run and autostart prompts
```

### 5.2 Main window

Shown at 1360×840 on the Forecast stage, one series selected.

```
+------------------------------------------------------------------------------------------+
| [=] Forecast — shop-sales.fcproj        [ Search / commands  Ctrl+K ]          [▁][▢][✕] |(1)
+----------------+--------------------------------------------------------+----------------+
| DATA           | Data › Prepare › Explore › Model › Backtest ›[Forecast]›… (3)| Forecast #7 (8)|
|   sales.csv    +--------------------------------------------------------+ origin 31 Oct  |
| SERIES    (12) | [v Weekly sales — Shop A] [v ETS(A,Ad,A) ▾] H [ 13 ] wk | horizon 13 wk  |
| > Shop A       | [Forecast]  [Compare versions] [Scenarios ▾] [Export ▾] | ETS(A,Ad,A)    |
|   Shop B       +--------------------------------------------------------+ intervals      |
|   Shop C  …    |  1600 ┤                                    ░░░░░░░░    |  simulation,   |
| MODELS         |       │                                ░░▒▒▒▒▒▒▒▒░░░   |  1 000 draws,  |
|   Auto set     |  1200 ┤   ╭╮    ╭╮    ╭╮    ╭╮      ░▒▒▓▓▓▓▓▓▓▓▒▒░░    |  conformal     |
|   ETS          |       │╭╮╭╯╰╮╭╮╭╯╰╮╭╮╭╯╰╮╭╮╭╯╰╮╭╮  ▒▓▓━━━━━━━━▓▓▒▒      |  adjusted      |
|   Cypha lags   |   800 ┤╯╰╯   ╰╯╰╯   ╰╯╰╯   ╰╯╰╯╰╮╭━━━━            (4)   |----------------|
| BACKTESTS      |       │                         ╰╯ │origin             | Backtest       |
|   BT-3 ✔       |   400 ┼──────┬──────┬──────┬──────┬┴─────┬──────       |  MASE 0.81     |
| FORECASTS      |        2023   2024   2025   2026    Nov    Jan         |  ±0.06         |
| > #7 31 Oct    |  ━ median  ▓ 50%  ▒ 80%  ░ 95%   ● anomaly  ▽ note     |  skill vs      |
|   #6 30 Sep    +--------------------------------------------------------+  seasonal naive|
| SCENARIOS      | Period     Median    80% low   80% high   95% low  high|  +19% ±6%      |
|   Base         | 2026-W45    1 184     1 102     1 266      1 058  1 310|  coverage 80%: |
|   Promo +10%   | 2026-W46    1 207     1 109     1 305      1 057  1 357|  78% (6 folds) |
| MONITORS       | 2026-W47    1 240     1 124     1 356      1 062  1 418|----------------|
| SCHEDULES      | …                                                   (5)| Limits         |
| REPORTS        +--------------------------------------------------------+ 3.8 years of   |
| JOBS       (2) | "Week 47 is most likely between 1 124 and 1 356."   (6)| data · cannot  |
|                |                                                        | see new events |
+----------------+--------------------------------------------------------+----------------+
| 12 series · sel. 2023-W01…2026-W44 · 0 jobs · schedules: 1 active · ⚙ deterministic   (7)|
+------------------------------------------------------------------------------------------+
```

The figures in this wireframe are placeholders that show layout, not results.

1. **Header bar.** Menu, project name with an unsaved dot, command palette. No network badge exists in this app; it has no network capability.
2. **Sidebar, 220 px.** `treeView` of project objects with state marks (✔ done, ⟳ running, ⚠ stale because the data changed after the run).
3. **Stage bar.** `tabBar` styled as steps. Stages are not locked in order, but a stage with unmet inputs shows why.
4. **Fan chart.** The time-series chart (AI03-GAP-1): history line, forecast median, three bands, origin marker, anomaly points, annotation flags; zoom with the wheel, pan by drag, range brush with `Shift`+drag.
5. **Forecast table.** `tableView`, tabular figures, copy and export.
6. **Plain-language line.** Fixed-template text (F141).
7. **Status bar.** Counts, selection range, jobs (click opens Jobs), schedules, deterministic mark.
8. **Inspector, 280 px.** Run record summary, backtest evidence for the model in use, and the limits panel that cannot be hidden while a forecast is shown.

States: empty project ("Import a table of dated numbers to begin." and an Import button), importing (row counter), series too short (forecast allowed with baselines only, reason shown), fitting (progress on the chart area, Cancel), stale (banner "Data changed since this forecast" with Refresh), failed fit (the engine's reason and "Open log"), sealed project locked (unlock prompt), read-only (linked source missing or project opened from a read-only location), narrow (sidebar overlay; table moves under the chart; inspector as a sheet).

### 5.3 Other views and dialogs

#### Import wizard, Columns step (820×600)

```
+------------------------------------------------------------------------------------+
| Import data                         Source › Columns › Time › Check › Finish       |
+------------------------------------------------------------------------------------+
| sales.csv · 18 204 rows · UTF-8 · delimiter ","  decimal "."         [Change…]     |
| Layout  (•) Long: one row per date and group   ( ) Wide: one column per series     |
+------------------------------------------------------------------------------------+
| Column     Sample values                 Use as              Type                  |
| date       2023-01-02, 2023-01-09 …      [v Time        ]    date (ISO)       ✔    |
| shop       A, B, C …                     [v Group       ]    text, 12 values       |
| units      1 020, 987, 1 105 …           [v Value       ]    number                |
| price      4.50, 4.50, 3.99 …            [v Regressor: known in advance ]  number  |
| note       "", "closed", "" …            [v Ignore      ]    text                  |
+------------------------------------------------------------------------------------+
| Preview (first 200 rows)                                                           |
| 2023-01-02 | A | 1020 | 4.50 |                                                     |
| 2023-01-09 | A |  987 | 4.50 |                                                     |
| ⚠ Row 4 411, column "units": "n/a" is not a number.  [Treat "n/a" as missing]      |
|                                                    [Back] [Cancel] [Next]          |
+------------------------------------------------------------------------------------+
```

`tableView` with `comboBox` cells; the preview is a second `tableView`. Problems are listed with file, row and column; Next is disabled until each is resolved by an explicit choice. States: detecting, encoding converted (notice), no time column found, duplicate timestamps (choice required on the Check step).

#### Prepare stage

```
+----------------------------------------------------------------------------------------+
| Recipe (hash 7c1e…)        [+ Step ▾]   | Before / after                    [v Shop A] |
| 1 [x] Resample → weekly, sum            |  ╭╮  ╭╮   raw (grey)                         |
| 2 [x] Fill gaps → seasonal interp. (3)  | ╭╯╰╮╭╯╰╮╭─ prepared (accent)                 |
| 3 [x] Outliers → mark, down-weight (2)  | ╯  ╰╯  ╰╯   ○ filled   ✕ outlier            |
| 4 [ ] Transform → log                   |                                              |
| 5 [x] Ranges → train …2025-W52,         | Quality: 201 weeks · 3 filled · 2 outliers ·  |
|        validation 2026-W01…W26,         | no zeros · step regular after step 1          |
|        test 2026-W27…W44 (locked)       | ⚠ price: correlation with units at lag 0 = 0.97|
+-----------------------------------------+----------------------------------------------+
```

Left: reorderable `listView` with `checkBox` per step; right: chart and a quality `panel`. Dragging a step reorders the recipe; every change re-hashes it and marks downstream results stale.

#### Explore stage, decomposition and autocorrelation

```
+----------------------------------------------------------------------------------------+
| [Time plot] [Seasonal] [>Decomposition<] [ACF/PACF] [Spectrum] [Distribution] [Overview]|
| Method [v STL]  Period [v 52 (detected)]  Seasonal window [ 13 ]  [x] Robust           |
+----------------------------------------------------------------------------------------+
| observed  ╭╮╭╯╰╮╭╮╭╯╰╮╭╮╭╯╰╮╭╮╭╯╰╮                                                     |
| trend     ───────────╱‾‾‾‾‾‾‾‾‾╲──────                                                 |
| seasonal  ╱╲╱╲╱╲╱╲╱╲╱╲╱╲╱╲╱╲╱╲╱╲╱╲                                                     |
| remainder ·.·'·.·.'·.·'·.·.●·.'·.·                                                     |
+----------------------------------------------------------------------------------------+
| Trend strength 0.62 · seasonal strength 0.88 · dominant periods 52, 26                 |
| ACF  ▇▅▃▂▁ ▁▂▃▅▇ …  (band = 95% under white noise)   lag [====|-----] 104              |
+----------------------------------------------------------------------------------------+
```

Four stacked time-series charts sharing one x axis; a `chart` (Bar) for ACF. States: series too short for the period (says how many cycles are needed), no seasonality detected.

#### Backtest stage, leaderboard

```
+----------------------------------------------------------------------------------------+
| Backtest BT-3: expanding window · 6 folds · horizon 13 · step 13      [New backtest…]  |
| Folds  |████████████░|  |█████████████░|  |██████████████░| …   █ train  ░ test        |
+----------------------------------------------------------------------------------------+
| Model            MASE        CRPS       Cover 80%   Skill vs s-naive     Time          |
| ETS(A,Ad,A)      0.81 ±0.06  41.2 ±3.1    78%        +19% ±6%   ★        0.4 s         |
| Cypha lags       0.84 ±0.07  42.9 ±3.4    81%        +16% ±7%   ≈ ★      3.1 s         |
| ARIMA(1,1,1)(0,1,1) 0.90 ±0.08 46.0 ±3.9  74%        +10% ±8%            2.2 s         |
| Theta            0.93 ±0.07  47.5 ±3.6    83%         +7% ±7%   ≈ base   0.1 s         |
| — Seasonal naive 1.00        51.3 ±4.0    80%          0%       baseline 0.0 s         |
| — Naive          1.42 ±0.11  73.8 ±5.2    69%        −42% ±11%  baseline 0.0 s         |
+----------------------------------------------------------------------------------------+
| ★ best · ≈ not distinguishable from the row it names · ± bootstrap 95% over folds      |
| [Per horizon] [Residuals] [Calibration] [Compare two…]     Test range: locked [Open…]  |
+----------------------------------------------------------------------------------------+
```

`tableView` with pinned baseline rows; clicking a metric header opens its definition. "Open…" on the locked test range shows a confirmation ("This is recorded. Later changes will be marked 'test already seen'.").

#### Scenario comparison

```
+----------------------------------------------------------------------------------------+
| Scenarios: [x] Base  [x] Promo +10% price cut  [ ] Store closed wk 48    [Edit…] [New…]|
+----------------------------------------------------------------------------------------+
|  ╭ base median ━━   promo median ╍╍   bands 80%                                        |
|  │            ╍╍╍╍╍╍╍╍╍╍                                                               |
|  │        ━━━━━━━━━━━━━━━                                                              |
+----------------------------------------------------------------------------------------+
| Period    Base    Promo   Difference   80% interval of difference                      |
| W45       1 184   1 262     +78          +31 … +124                                    |
| W46       1 207   1 290     +83          +29 … +139                                    |
| Total 13w 15 610  16 540   +930         +310 … +1 560                                  |
| Scenarios are assumptions about the future, not forecasts of it.                       |
+----------------------------------------------------------------------------------------+
```

#### Schedule dialog (560×440)

```
+------------------------------------------------------------+
| Schedule a re-forecast                                     |
+------------------------------------------------------------+
| Run     [v Monthly] on day [ 1 ] at [ 06:00 ]              |
| Steps   [x] Refresh linked data (sales.csv)                |
|         (•) Refit  ( ) Update only                         |
|         [x] Score earlier forecasts against new actuals    |
|         [x] Write forecast CSV to  ~/Documents/Forecasts   |
|         [x] Notify me when done   [x] Notify on anomalies  |
| Rules   [x] Only on mains power   Threads [ 4 ]            |
| ⚠ This lets Forecast run without a window. You will be     |
|   asked once by the system. It appears in Permissions.     |
|                                   [Cancel] [Save schedule] |
+------------------------------------------------------------+
```

#### Anomalies view

```
+----------------------------------------------------------------------------------------+
| Monitor: Shop A weekly · method: one-step interval 99% · false-alarm target 1 per year |
+----------------------------------------------------------------------------------------+
|  ╭╮╭╯╰╮╭╮╭╯╰╮╭●╭╯╰╮╭╮╭╯╰╮   ● flagged   ░ expected band                                |
+----------------------------------------------------------------------------------------+
| Date       Value   Expected (99%)     Tail prob.  Method     State        Note         |
| 2026-W31    412     980 … 1 310        <0.001     interval   confirmed    store closed |
| 2026-W40   1 702    1 010 … 1 420       0.004     interval   open         —            |
+----------------------------------------------------------------------------------------+
| [Confirm] [Dismiss] [Mark as known event…] [Exclude from training]                     |
+----------------------------------------------------------------------------------------+
```

### 5.4 Menus

Items are separated by `·`; `—` marks a separator.

```
File      New Project Ctrl+N · Open… Ctrl+O · Open Recent ▸ · Save Ctrl+S · Save As… Ctrl+Shift+S
          — Import Data… Ctrl+I · Refresh Linked Data F5
          — Export Forecast… Ctrl+E · Export Chart… Ctrl+Shift+E · Export Project… · Print… Ctrl+P
          — Seal Project… · Close Ctrl+W · Quit Ctrl+Q
Edit      Undo Ctrl+Z · Redo Ctrl+Shift+Z — Cut Ctrl+X · Copy Ctrl+C · Paste Ctrl+V · Select All Ctrl+A
          — Find Series Ctrl+F · Add Annotation… Ctrl+Shift+N · Preferences Ctrl+,
View      Sidebar F9 · Inspector F4 · Stage: Data … Report Ctrl+1 … Ctrl+7
          — Zoom In Ctrl++ · Zoom Out Ctrl+- · Fit All Ctrl+0 · Zoom to Selection Ctrl+Shift+0
          — Log Scale Ctrl+L · Show Bands ▸ (50%, 80%, 95%) · Show Annotations · Chart as Table Ctrl+Alt+T
          — Full Screen F11
Data      Resample… · Missing Values… · Duplicates… · Outliers… · Transform… · Regressors…
          — Set Train / Validation / Test Ranges… · Data Quality Summary · Leakage Check
Model     Add Model ▸ (Baselines, ETS, Theta, ARIMA, Intermittent, Regression, Cypha Lags, Ensemble)
          — Auto Select Ctrl+Shift+A · Configure… Ctrl+M · Fit Ctrl+Return · Save to Model Library…
          — Register in Model Manager…
Backtest  New Backtest… Ctrl+B · Run Ctrl+Shift+Return · Compare Two Models… · Residuals · Calibration
          — Open Test Scores…
Forecast  New Forecast… Ctrl+Shift+F · Compare Versions · Scenarios ▸ (New…, Edit…, Compare)
          — Probability Query… · Components · Explain Change… · Seal Prospective Forecast…
Monitor   New Monitor… · Anomalies Ctrl+Shift+M · Drift Report — Schedules… · Pause All Schedules
Report    New Report… · Add Block ▸ · Export PDF… · Export Markdown… · Export HTML…
Help      Forecast Help F1 · Metric Definitions · Tour · About Reproducibility · Keyboard Shortcuts · About
```

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| `Ctrl+1`…`Ctrl+7` | Go to stage |
| `Left` / `Right` (chart focused) | Move the cross-hair one period; `Shift` extends the range |
| `+` / `-` / `0` (chart focused) | Zoom in, out, fit |
| `[` / `]` | Previous / next series |
| `Ctrl+Return` | Fit the selected model |
| `Ctrl+Shift+Return` | Run the backtest |
| `M` (point focused) | Mark or unmark as outlier |
| `N` (point focused) | Add an annotation at this date |
| `Ctrl+Alt+T` | Show the focused chart as a table |

Context menus: series (Open, Rename, Duplicate, Group, Set Unit, Export, Remove); chart point (Mark Outlier, Exclude, Add Annotation, Set as Origin, Copy Value); chart area (Copy as PNG, Copy as SVG, Export, Log Scale, Bands); model (Configure, Fit, Duplicate, Save to Library, Remove); leaderboard row (Use for Forecast, Compare With…, Residuals, Show Run Record); forecast (Compare With Version, Export, Send to Sheets, Seal, Delete); recipe step (Edit, Disable, Move Up, Move Down, Remove).

Drag and drop: a CSV, TSV, SQLite or Parquet file onto the window starts the import wizard; a series onto a chart overlays it; a model onto a series fits it; recipe steps reorder by drag; a forecast row dragged out offers `text/csv`; a chart dragged out offers PNG and SVG. In the scenario editor, points on the regressor curve drag vertically.

Pointer: wheel zooms time at the pointer; drag pans; `Shift`+drag selects a range; double-click a band toggles it; pinch zoom on touch.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | One line and Import; the tour is offered once. |
| Loading | Project opens with its tree first; charts fill in as series load. |
| Job running | Stage content stays usable; affected objects show ⟳; progress in the status bar. |
| Stale | Objects computed from older data or an older recipe show ⚠ with "Recompute". |
| Error | The engine's message with the failing series and step; never a partial result table. |
| Offline | No difference; the app has no network use. |
| Locked | Sealed project: content hidden until unlocked. |
| Read-only | Linked source missing: results stay viewable; Refresh is disabled with the reason. |
| Engine unavailable | The helper failed to start: data views work, model actions are disabled with the reason. |
| Narrow (<640) | Stage bar becomes a `comboBox`; sidebar overlay; table under chart; inspector sheet. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Project tree | `treeView` |
| Stage bar, view tabs | `tabBar`, `tabWidget` |
| Tables (data, forecast, leaderboard, anomalies) | `tableView` (with `SqliteModel` over the project file) |
| Simple charts (ACF, histogram, bars) | `chart` (Bar, Line, Area, Scatter) |
| Time-series and fan charts | missing: AI03-GAP-1 |
| Recipe | `listView`, `checkBox` |
| Model configuration, inspector | `propertyGrid`, `spinBox`, `slider`, `comboBox` |
| Scenario regressor editing | `curveEditor` |
| Lineage, scenario tree | `nodeGraph` |
| Overview grid | `gridView` |
| Dates | `dateTimeEdit`, `calendar` |
| Reports | `markdownView` preview; PDF through G-16 |
| Dialogs, wizards | `dialog`, `lineEdit`, `radioButton`, `progressBar` |

Missing:

- `AI03-GAP-1`: a time-series chart. `chart` takes plain value arrays per series (`chartSetSeries`, `FUSE:Source/FUSE/UI/Widgets/include/fuse/ui/widgets/widgets.hpp:244-246`); it has no time axis, bands, zoom, pan, brush, markers, shared axes or secondary axis. Wanted by SY04, OF05 and AI04 as well.
- `AI02-GAP-1` (editable cells in `tableView`) is also needed here for column mapping.
- G-16 for PDF reports; G-10 only if reports gain rich text editing.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic (`main.fs`, `src/*.fs`) | ForgeScript | app host | Project model, stages, recipes, job orchestration, run records, report templates |
| UI (`ui/*.fuseui`, `style/*.css`) | FUSE UI | app host | Windows, stages, dialogs, charts |
| Import helper `import-helper` | C++23 | disposable helper per import, capability mode | Parses CSV, TSV, SQLite and Parquet from a file handle; writes typed columns to a shared-memory handle (PLATFORM.md section 2 reason a) |
| Forecast engine helper `forecast-engine` | C++23; links Cypha `cypha_core` | helper, capability mode, no network | Preparation steps, decomposition, classical models, Cypha regressors, backtests, scoring, simulation (reasons b and d) |
| Schedule agent `forecast-agent` | ForgeScript on the app host, no window | app host instance under `bg.run` | Runs schedules and monitors; starts the engine helper; posts notifications |
| Project store | SQLite | app host (through `SqliteModel`), engine helper for bulk writes via a passed handle | The `.fcproj` file |
| Optional: Cypha REST adapter (Phase 3) | C++23 (`cypha_rest`, modified) | helper on an inherited Unix socket only | Reuse of Cypha's REST contract; never TCP (section 2.2) |

```
 Sheets / Data Studio ──share (CSV handle)──┐        other apps ──Forecast/1──┐
                                            ▼                                 ▼
 ┌──────────────────── app host: org.pbsd.forecast ───────────────────────────────┐
 │ FUSE UI ◄─ view models ─ ForgeScript: project, recipe, jobs, records, reports   │
 └──────┬──────────────────────┬───────────────────────────┬─────────────────────┘
        │ file handle          │ shm columns + job spec    │ SQLite handle
   import-helper          forecast-engine ────────────────►│ .fcproj (project store)
   (hostile bytes)        ├ own code: prep, STL, ETS, ARIMA, scores
                          └ cypha_core: Cypha regressors, anomaly, drift
 forecast-agent (bg.run) ─ same engine, no window ─ svc.notify
 svc.model (optional): registered plugin models (FORGE-WP-31.9), report summary text
```

The engine's interface enforces the origin rule by shape: `fit(history) → model`, `predict(model, history, horizon, regressors_future) → draws`, `score(draws, truth) → metrics`. `history` is a view cut at the origin by the caller; no engine call accepts both history and truth.

### 6.2 Data model

Project file `*.fcproj`: SQLite, `application_id` set, `schema_version` 1, forward-only migrations with a copy kept. Large arrays are stored as little-endian f64 blobs with a length and a BLAKE3.

| Entity | Fields |
|---|---|
| `dataset` | `id` uuid, `name` text, `source_kind` enum(csv, tsv, sqlite, parquet, share, paste), `source_name` text, `source_size` i64, `blake3` blob(32), `sha256` blob(32), `import_options` json, `linked` bool, `grant_token` text?, `imported_at` i64, `row_count` i64 |
| `column` | `id`, `dataset_id`, `name`, `role` enum(time, value, group, regressor_known, regressor_observed, ignore), `dtype` enum(i64, f64, text, date, timestamp), `unit` text? |
| `series` | `id`, `dataset_id`, `key` json (group values), `display_name`, `unit`, `frequency` text (ISO 8601 duration), `tz` text, `start` i64, `end` i64, `n_obs` i64, `values_ref` blob id, `flags_ref` blob id (bit per point: filled, outlier, excluded), `data_hash` blob(32) |
| `recipe` | `id`, `series_scope` json, `steps` json [{`kind`, `params`, `enabled`}], `hash` blob(32) |
| `range_split` | `recipe_id`, `train_end` i64, `validation_end` i64, `test_end` i64, `test_opened_at` i64? |
| `annotation` | `id`, `series_id`?, `at` i64, `text`, `kind` enum(note, known_event) |
| `model_spec` | `id`, `family` enum(naive, snaive, drift, mean, ets, theta, arima, croston, regression, cypha_lags, cypha_quantile, ensemble), `params` json, `constraints` json, `hash` blob(32) |
| `fit` | `id`, `model_spec_id`, `series_id`, `recipe_hash`, `origin` i64, `state_ref` blob id (fitted parameters or a `.cypha` blob), `state_hash`, `info` json (AICc, iterations, warnings), `run_id` |
| `backtest` | `id`, `name`, `window` enum(expanding, sliding), `folds` i32, `step` i32, `horizon` i32, `run_id` |
| `fold_result` | `backtest_id`, `model_spec_id`, `series_id`, `fold` i32, `origin` i64, `errors_ref` blob id, `draws_summary_ref` blob id |
| `metric` | `backtest_id`, `model_spec_id`, `series_id`?, `name` text, `horizon` i32?, `value` f64, `ci_low` f64, `ci_high` f64, `n` i32 |
| `forecast` | `id`, `fit_id`, `origin` i64, `horizon` i32, `interval_method` enum(analytic, simulation, quantile, conformal), `n_draws` i32, `quantiles_ref` blob id, `draws_ref` blob id?, `scenario_id`?, `sealed_hash` blob?, `run_id` |
| `scenario` | `id`, `name`, `assumptions` json, `regressors_future_ref` blob id |
| `monitor` | `id`, `series_id`, `method` enum(interval, remainder, cypha), `level` f64, `false_alarm_target` f64, `threshold` f64, `enabled` bool |
| `anomaly` | `id`, `monitor_id`, `at` i64, `value` f64, `expected_low` f64, `expected_high` f64, `score` f64, `state` enum(open, confirmed, dismissed, known_event), `note` |
| `schedule` | `id`, `rule` text (RFC 5545 RRULE), `steps` json, `output_grant` text?, `rules` json (mains, threads), `enabled` bool, `last_run_id`? |
| `run` | `id`, `kind` enum(import, prepare, fit, backtest, forecast, monitor, report), `seed` u64, `threads` i32, `deterministic` bool, `engine_version` text, `cypha_commit` text, `inputs` json (hashes), `result_hash` blob(32), `started_at`, `ended_at`, `status`, `log_ref` |
| `report` | `id`, `title`, `blocks` json, `last_export_hash` blob? |

Application data outside projects (`~/.local/share/org.pbsd.forecast/`): model library (`model_spec` rows), calendars, schedule registry (project grant tokens and next run times).

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| `.fcproj` (SQLite, own schema) | yes | yes | SQLite (public domain; platform store engine, V8) | app host for own files; a project from an untrusted source is first checked in import-helper (integrity check, schema check) |
| CSV, TSV | yes | yes | own strict reader (fails closed) | import-helper |
| SQLite database (as a data source) | yes | — | SQLite, opened read-only, no extensions | import-helper |
| Parquet (flat subset) | yes | yes | own reader and writer; Zstandard (BSD licence, in the FreeBSD base system) and own Snappy decoder | import-helper |
| ICS (holiday calendars) | yes | — | own | import-helper |
| `.cypha` model blob | yes | yes | Cypha `Cypha::save` / `load` | forecast-engine |
| Cypha forecast bundle `<name>.forecast/` (planned by Cypha FC11; does not exist yet) | yes | — | Cypha | forecast-engine |
| `.fpk` with Cypha sections (export for `svc.model`) | — | yes | FORGE `modelfile::writer` | forecast-engine |
| Forecast request and result JSON (`Forecast/1`) | yes | yes | FORGE JSON | app host (bounded size, schema-checked) |
| PNG, SVG | — | yes | FUSE draw-list export (Proposed) or own SVG writer | app host |
| PDF report | — | yes | print pipeline (G-16) | print service |
| Markdown, static HTML | — | yes | own | app host |
| Session bus records | yes | yes | schema compiler (FORGE-WP-26) | app host |

Third-party code that comes with Cypha: cpp-httplib and nlohmann/json are vendored in `CYPHA:native/third_party/` (both MIT upstream; licences to be confirmed from the vendored files before packaging). httplib is needed only by `cypha_rest`, so the Phase 1 engine does not link it. Cypha fetches a SQLite amalgamation from the network at configure time when `CYPHA_BUILD_EXPERIMENT_DB` is on (`CYPHA:native/CMakeLists.txt:1081-1087`); that option stays off, and the offline build uses the system SQLite. Cypha's licence is AGPL-3.0-or-later or commercial (`CYPHA:LICENSE`, `CYPHA:COMMERCIAL-LICENCE.md`).

### 6.4 Integration

- **Intents handled:** `open` and `edit` for `.fcproj`; `open` for CSV, TSV, Parquet (as "Import into Forecast"); `share` from OF05 and OF09 (`text/csv`); `new` with `application/vnd.pbsd.forecast.request+json`.
- **Intents emitted:** `share` (CSV to OF05, charts), `print`, `open` (source file in Files), `new` (key in SE04 for signing).
- **Services:** `svc.settings`, `svc.notify`, `svc.intent`, `svc.portal.file`, `svc.portal.print`, `svc.power`, `svc.backup`, `svc.keyring` (sealed projects, result signing), `svc.thumbnail` (project thumbnails: the last chart), `svc.model` (optional, Phase 3).
- **Interface offered:** `org.pbsd.forecast.Forecast/1` — `forecast(series: handle, horizon: i32, options) → result handle`, `models() → list`, `status(job) → state`. Callers pass data by handle; Forecast returns a file handle with quantiles. First use per calling app is prompted.
- **Other apps:** OF05 (range hand-off both ways), OF09 (query results), AI04 (`forecast::` module), AI02 (plugin model registration), SE04 (signing keys), AD05 (background and autostart grants), SY02 (open with).
- **Cypha, Verified:** everything in section 2.1. **Proposed:** linking `cypha_core` into `forecast-engine`; moving `TimeSeriesEncoder` into a public Cypha module; the `Forecaster` and `History` interface from Cypha's plan (FC5, FC7, FC15) as the common shape for learned forecasters; the event workspace on FC11's bundle.
- **FORGE, Verified:** the plugin session kind and the Cypha example (`FORGE:examples/integrations/cypha_classifier.cpp`), `.fpk` named sections. **Proposed:** FORGE-WP-31.9 service wrapper so a registered forecast model can be called through `svc.model`.

## 7. Security and privacy

### 7.1 Capabilities

| Capability | Why |
|---|---|
| `fs.app` | Model library, calendars, schedule registry, caches |
| `fs.portal` | Open data files and projects; save projects, exports and reports; remembered grants for linked sources and schedule output folders |
| `net.none` | The app never uses the network. No other `net.*` is requested |
| `proc.helper:import-helper`, `proc.helper:forecast-engine` | Section 6.1 |
| `svc.settings`, `svc.notify`, `svc.intent`, `svc.portal.file`, `svc.portal.print`, `svc.power`, `svc.backup`, `svc.thumbnail` | Section 6.4 |
| `svc.keyring` | Sealed projects; signing results |
| `bg.run` | Scheduled re-forecasts and monitors; requested when the first schedule is saved |
| `bg.autostart` | So schedules survive a reboot; requested with `bg.run`; both removed when the last schedule is deleted |
| `ai.local`, `svc.model` | Optional, Phase 3: report summary text and serving registered models; off by default |

Not requested: any `net.*` beyond `net.none`, any device, `proc.exec`, `fs.path`, `svc.search` (project contents are not indexed), `svc.accounts`.

### 7.2 Threat model

Assets: the user's data (often commercially sensitive), fitted models (which encode that data), forecasts that drive decisions, the integrity of scores and run records, remembered folder grants.

| Adversary | Attack | Mitigation |
|---|---|---|
| Malicious data file | Parser exploit in CSV, SQLite or Parquet; decompression bomb | import-helper in capability mode with only the input handle and an output memory handle; size, row and expansion-ratio limits; SQLite opened read-only with extensions and triggers off; fuzzed parsers |
| Malicious project file | Crafted SQLite project that exploits the app host on open | Integrity and schema check in import-helper before the app host opens it; fixed queries only, no SQL from the file is executed |
| Poisoned data | Rows crafted to bend a forecast or hide an anomaly | Data hashes and Refresh diffs make changes visible; outlier and leakage checks; robust methods offered; backtests show sensitivity. This risk cannot be removed, only made visible |
| Other local process | Calls a loopback REST server to retrain, read results or read a directory | No TCP server is ever started; engine is in-process in a helper; optional REST adapter only on an inherited Unix socket (section 2.2) |
| Another app using `Forecast/1` | Floods jobs, or reads another app's results | Shell prompt on first use; per-caller quotas; results returned only to the caller's handle; no listing of other callers' jobs |
| Background agent misuse | Schedule writes to unexpected places, or runs for ever | Output only to folders granted when the schedule was made; wall-clock and thread caps; "Pause all"; grants visible in AD05 |
| Result tampering | Someone edits a forecast table after the fact | Result hashes in run records; re-run and verify (F134); optional signatures (F136); sealed prospective forecasts (F096) |
| The user's own optimism | Tuning on the test set, reading in-sample fit as accuracy | Test range locked and opening recorded (F081); no in-sample headline (F082); baselines beside every number |
| Shoulder surfer | Reads values in notifications | Notifications carry names and dates, not values, on the lock screen |

Stored: projects where the user saves them; app data under `fs.app`. Nothing leaves the machine. There is no telemetry and no network code in the app host or the Phase 1 helpers.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `import.day_first` | enum(ask, yes, no) | ask | Ambiguous dates |
| `import.embed_data` | bool | true | Copy rows into the project instead of linking |
| `import.max_rows` | int | 50 000 000 | Row limit per import |
| `prepare.default_fill` | enum(none, forward, linear, seasonal) | none | Missing-value default |
| `model.auto_families` | list | baselines, ets, theta, arima | Families "Auto" may fit |
| `model.include_cypha_in_auto` | bool | false | Add Cypha lag regressors to "Auto" |
| `backtest.folds` | int | 6 | Default number of folds |
| `backtest.window` | enum(expanding, sliding) | expanding | Default window |
| `forecast.draws` | int | 1 000 | Sample paths per forecast |
| `forecast.levels` | list of float | 0.5, 0.8, 0.95 | Interval levels shown |
| `forecast.conformal` | bool | true | Calibrate intervals from backtest residuals |
| `forecast.keep_draws` | bool | false | Store full sample paths in the project |
| `run.deterministic_final` | bool | true | Final forecasts use deterministic mode |
| `run.default_seed` | u64 | 1 | Seed when none is set |
| `run.threads` | int | 0 | 0 = all performance cores |
| `run.memory_cap_mb` | int | 4 096 | Engine helper memory cap |
| `schedule.only_on_mains` | bool | true | Default power rule |
| `schedule.catch_up_missed` | bool | true | Run a missed schedule at next login |
| `notify.values_on_lock_screen` | bool | false | Show numbers in notifications when locked |
| `report.include_limits` | bool | true | Limits block in every report (cannot be turned off for PDF) |

## 9. Accessibility and languages

- Every chart has a table view (`Ctrl+Alt+T`) and a text summary in the a11y tree: range, last value, trend direction, forecast median and interval for the focused period.
- The cross-hair moves by keyboard and announces date and values; bands are named by level, not colour. Series use shape and dash as well as colour; the palette is checked for colour-vision deficiencies.
- Optional sonification of a series (pitch for value) through `svc.media` is a Phase 3 idea, not a commitment.
- Numbers, dates, week numbering (ISO or locale) and first day of week follow the locale; metric names stay in their standard abbreviations with a translated description.
- Formulae in Help are text with a spoken form.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | under 400 ms |
| CSV import | 1 million rows in under 2 s; 50 million rows streamed without loading the file into memory |
| Chart pan and zoom | under 8 ms per frame with 1 million points (min–max decimation per pixel column) |
| STL on 10 000 points | under 100 ms |
| ETS and Theta fit, 1 000 points | under 50 ms each |
| ARIMA stepwise search, 1 000 points | under 3 s |
| Backtest, 6 folds × 6 models × 1 series | under 10 s |
| Batch: 1 000 series, Auto, 6 folds | under 10 min on 8 cores |
| Forecast with 1 000 draws, horizon 104 | under 200 ms |
| App host memory | under 200 MB; engine helper within `run.memory_cap_mb` |
| Idle, and agent between runs | zero wake-ups; the agent sleeps until the next schedule time |

Cypha model timings are not budgeted until measured; the engine reports them per run.

## 11. Testing and proof

- **Unit:** time parsing, frequency detection, resampling and fill rules, recipe hashing, split logic, run-record building, report templates.
- **Reference values:** every metric against values computed offline with a published implementation, to 1e-9 (the method Cypha's FC6 sets for itself); every classical model against a reference implementation on public series, with tolerances stated per model.
- **Property tests:** the origin rule (a model never reads beyond its origin: checked by poisoning future values with NaN and requiring unchanged output); quantiles are monotone; intervals nest; scores of a perfect forecast are zero; deterministic mode gives identical bytes on re-run.
- **Planted tests:** a planted seasonal series must be recovered; a planted level shift must be flagged; a planted leak must trip the leakage check; a constant series must fail the fit.
- **Golden renders:** all stages and dialogs, three themes, 200% scale, narrow.
- **Scripted UI:** tasks T1–T14 on the sample data.
- **Fuzz:** CSV, Parquet, ICS readers; the project integrity checker; the `Forecast/1` request decoder.
- **Differential:** Cypha regressors called through the engine against Cypha's own golden fixtures (`CYPHA:fixtures/`, CTest `native_*`).
- **Fault:** kill the engine mid-backtest (no partial rows committed); remove a linked source; fill the disk during save.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Split and origin logic | No fit or predict call receives data after its origin | T1 | PRISM proof over the engine interface types | — |
| Scoring library | CRPS, pinball, coverage formulas match their definitions; no runtime errors | T2 (T1 for sample CRPS on sorted input) | PRISM plus reference fixtures | Floating-point reasoning |
| CSV reader | Memory safety; every error names row and column | T2 | Absence of runtime errors; fuzzing | — |
| Parquet reader | Memory safety | T3 | Safe subset, sanitisers, fuzzing | Format size |
| Recipe engine | Deterministic: same input and recipe hash give the same output hash | T2 | Proof of no hidden state; replay tests | — |
| Classical models | Numerical agreement with references | T3 | Reference tests | Iterative optimisers |
| Cypha core | Parity with its fixtures | T3 at best | Cypha's CTest suite; safe-subset status of Cypha is Unverified | Large existing C++ code base not written to the safe subset |
| Schedule agent | Writes only to granted handles | T2 | PRISM on the agent's handle use | — |
| Forecast accuracy | — | T4 | Not provable; measured by backtest | Statistical |

Nothing above has been run.

## 12. Build plan

### Phase 1 — import, look, baseline and smoothing forecasts, honest backtest

| ID | Work item | Depends on |
|---|---|---|
| AI03-W01 | Project store schema, run records, hashing | G-07, G-08 |
| AI03-W02 | `import-helper`: strict CSV and TSV, time parsing, mapping wizard | G-06, FORGE-WP-27, FORGE-WP-28, AI02-GAP-1 |
| AI03-W03 | Time-series chart widget with bands, zoom, brush, markers | AI03-GAP-1 (FUSE work), G-01, G-02 |
| AI03-W04 | Main window, project tree, stage bar, data table | W01, W03, FORGE-WP-13 |
| AI03-W05 | Preparation: frequency, resample, fill, duplicates, ranges, recipe | W01 |
| AI03-W06 | Exploration: time plot, seasonal plot, STL, ACF and PACF, distribution | W03, W05 |
| AI03-W07 | `forecast-engine` skeleton with the origin-safe interface; baselines, ETS, Theta | W05 |
| AI03-W08 | Scoring library with reference fixtures; rolling-origin backtest; leaderboard | W07 |
| AI03-W09 | Forecast with simulated intervals, fan chart, table, versions, limits panel | W07, W08 |
| AI03-W10 | Export CSV, JSON, PNG, SVG; deterministic mode; undo; crash recovery; tour | W04 |

### Phase 2 — full classical set, Cypha regressors, scenarios, monitoring, schedules

| ID | Work item | Depends on |
|---|---|---|
| AI03-W11 | ARIMA, intermittent methods, regression with calendar terms, ensembles | W07 |
| AI03-W12 | Link `cypha_core` into the engine on HardenedBSD; offline build; Cypha lag regressor | G-18-style port check for Cypha (section 13), Cypha encoder move |
| AI03-W13 | Outliers, transforms, regressors, calendars, leakage check | W05 |
| AI03-W14 | Residual diagnostics, calibration, conformal intervals, test-range lock | W08 |
| AI03-W15 | Scenarios: editor, comparison, labels | W09 |
| AI03-W16 | Anomalies and monitors; drift and staleness warnings | W09 |
| AI03-W17 | Schedules and the background agent | `bg.run`, AD05, schedule service (section 13) |
| AI03-W18 | SQLite and Parquet import; hand-off from OF05 and OF09; send to Sheets | OF05, OF09, `svc.intent` |
| AI03-W19 | Report builder with PDF | G-16 |
| AI03-W20 | Many-series batch, overview grid, sealed projects, snapshots, project export | W08, `svc.keyring` |
| AI03-W21 | Component view, plain-language panel, re-run and verify | W09 |

### Phase 3 — advanced

| ID | Work item | Depends on |
|---|---|---|
| AI03-W22 | Cypha quantile forecaster, online updating, global models, Cypha anomaly and drift scores | W12 |
| AI03-W23 | Hierarchies and reconciliation; change points; hyper-parameter search | W11 |
| AI03-W24 | `Forecast/1` interface, `forecast::` ForgeScript module, intent `new` request | FORGE-WP-24, intent reply channel (section 13), AI04 |
| AI03-W25 | Export fitted Cypha models as signed `.fpk`; register in AI02; serve through `svc.model` | FORGE-WP-31.9, AI02 |
| AI03-W26 | Event workspace: bundle load, distributions, tree view, monitor replay | Cypha FC4–FC12 delivered |
| AI03-W27 | Optional REST adapter: `cypha_rest` on an inherited Unix socket, TCP compiled out, `/forecast/run` removed | Cypha change (Proposed); section 2.2 |
| AI03-W28 | Prospective register, result signing, lineage graph, explanation of change, feature importance | SE04, W21 |
| AI03-W29 | Local-model report summary | AI02, FORGE-WP-30 |

## 13. Open questions and risks

1. **The backing module is not what the catalogue line implies.** Cypha's forecast module is an event-forecasting scaffold under repair, not a time-series library. This spec therefore plans the classical methods as new code and uses Cypha where it has working parts (regressors, uncertainty, anomaly scores). The owner may prefer to rename the catalogue line or to grow Cypha's module toward general series; either way Phase 1 does not depend on Cypha.
2. **Cypha on HardenedBSD is Unverified.** Its documented builds are Linux, Windows and macOS (`CYPHA:README.md`). No FreeBSD build was read. A port check like G-18 is needed for Cypha before AI03-W12.
3. **Cypha and the safe C++ subset.** Cypha is a large existing code base; whether it passes the MathScript subset checker is unknown. It is confined to the engine helper under PLATFORM.md section 2 reason d.
4. **Quality of Cypha regressors for forecasting is unmeasured.** Cypha's own plan predicts its estimator will be close to logistic regression on lagged features (P-FC2 in `CYPHA:docs/FORECAST_PLAN.md`). The leaderboard will show whatever is true; "Auto" excludes Cypha models by default until backtests justify including them.
5. **`cypha_rest` exposure.** Loopback TCP without authentication, a request-supplied server path, and retraining per call (section 2.2) must not reach a desktop install. Until fixed upstream, the binary is not packaged.
6. **No time-series chart in FUSE.** AI03-GAP-1 is the largest dependency; without it the app cannot show a fan chart.
7. **New platform needs found here:** a scheduled-job service (timed background runs without each app holding `bg.autostart`); a reply channel on intents so a request can return a result handle; a change notice on linked file grants.
8. **Parquet scope.** A full Parquet reader is large. The flat subset may reject files users have; the error must say which feature is unsupported.
9. **Reference implementations for differential tests** are needed at build time only; they must not become run-time dependencies, and tests must run offline from recorded fixtures.
10. **Misuse.** A forecasting tool invites over-trust. The limits panel, baselines and locked test range are the design answer; they add friction on purpose.
