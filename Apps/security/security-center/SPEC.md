<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Security Centre

By Odin Loch

| | |
|---|---|
| ID | `SE01` |
| Bundle | `org.pbsd.security-center` |
| Category | security |
| Priority | P0 |
| Backing repos | SENTINEL (statistics, inference, provenance and export code to be ported off Qt), FORGE (ForgeScript, keystore, signing), FUSE (UI), ParanoidBSD (veriexec, hardening sysctls, audit) |
| Status | Planned. Nothing built. |

## 1. Purpose

Security Centre answers one question at a glance: is this machine in a known-good state, and if not, what changed and what should be done. It shows boot, integrity, hardening and detection state, keeps an incident timeline, runs scans, holds the quarantine and writes reports. It is the FUSE front end for the SENTINEL engine after that engine is ported off Qt and pointed at host security events.

### 1.1 What SENTINEL is today (Verified, read 12 Nov 2026)

SENTINEL is **not** a host security monitor today. It is a crime-analytics desktop program.

| Fact | Source |
|---|---|
| "Crime analytics and investigative decision support — C++23 / Qt 6 desktop application". It ingests public crime data, runs statistical models and ranks investigative leads with provenance. | `SENTINEL:README.md` |
| Layers: ingest → NLP → temporal features → nine statistical models → inference → benchmarking → UI, persisted in SQLite. | `SENTINEL:docs/ARCHITECTURE.md` |
| It has no code for boot state, file integrity, hardening state, process or network monitoring, malware scanning or file quarantine. Its "quarantine" is a data-quality gate on imported records. | `SENTINEL:src/ingest/DataQualityScorer.h`, whole `src/` tree read by file list |
| It reaches the network: UK Police API, Open-Meteo, and a GitHub release check. | `SENTINEL:src/ingest/UKPoliceSource.cpp`, `src/ingest/WeatherSource.cpp`, `src/core/UpdateChecker.cpp:73` |
| It serves a read-only REST API on a local TCP port. | `SENTINEL:src/api/LocalApiServer.cpp:104-225` |
| 495 test files; 405 of them use QtTest. The README claims 495 passing tests; not run here. | `SENTINEL:tests/`, `SENTINEL:README.md` |
| Targets are Windows (NSIS, portable ZIP) and Linux (TGZ, DEB). No FreeBSD or HardenedBSD target. | `SENTINEL:packaging/`, `SENTINEL:installer/`, `SENTINEL:cmake/CPackConfig.cmake` |

What is reusable is the domain-neutral mathematics: anomaly scoring, rate baselines, self-exciting burst models, clustering, evidence weighting, ranked leads with contradiction checks, calibration, the provenance log and the exporters. Section 6.4 maps every SENTINEL component to a Security Centre view or marks it not carried over.

### 1.2 Qt dependencies in SENTINEL to be replaced (Verified)

Every one of the 107 files under `SENTINEL:src/` includes Qt headers. Nothing in the tree is Qt-free today.

| # | Qt dependency | Where (paths) | Replacement |
|---|---|---|---|
| Q1 | Build: `CMAKE_AUTOMOC/AUTORCC/AUTOUIC`, `find_package(Qt6 … Core Widgets Network Charts Sql Test)`, `Qt6::*` link lines, optional `Qt6::HttpServer`, `qt_add_executable`, `windeployqt` | `CMakeLists.txt:9-11, 14, 102-109, 152, 163-169, 208-234`; `tests/CMakeLists.txt` (190 Qt lines); `cmake/CPackConfig.cmake:38` | Plain CMake targets, C++23 standard library, FORGE and FUSE libraries; no `moc`, `rcc`, `uic` |
| Q2 | QtWidgets UI: `QMainWindow`, `QStackedWidget`, `QTableWidget`, `QTableView`, `QSortFilterProxyModel`, `QTabWidget`, `QSplitter`, `QFileDialog`, `QMessageBox`, `QInputDialog`, `QToolBar`, `QMenuBar`, `QStatusBar`, layouts | all 28 files in `src/ui/` (`MainWindow`, `DashboardWidget`, `EventsTableWidget`, `AnalyticsWidget`, `CaseWorkspaceWidget`, `CoOffendingGraphWidget`, `LeadsWidget`, `AuditLogWidget`, `SettingsWidget`, `DebugConsoleWidget`, `MapWidget`, `GeoProfileHeatmapWidget`, `TemporalHeatmapWidget`, `CalibrationDashboardWidget`; `.h` and `.cpp` each) | `.fuseui` documents and ForgeScript view models (section 5); the whole directory is deleted, not ported |
| Q3 | Custom painting with `QPainter`, `QPaintEvent`, `QWheelEvent`, `QMouseEvent`, `QFontMetrics` | `src/ui/MapWidget.cpp`, `CoOffendingGraphWidget.cpp`, `GeoProfileHeatmapWidget.cpp`, `TemporalHeatmapWidget.cpp`, `CalibrationDashboardWidget.cpp`, `AnalyticsWidget.cpp` | FUSE `chart`, `nodeGraph`, and a heat-map chart type (SE01-GAP-1) |
| Q4 | QtCharts (`QBarSet`, `QBarCategoryAxis`, `QValueAxis`) | `src/ui/AnalyticsWidget.cpp:7-9` | FUSE `chart` |
| Q5 | Style sheet and resource system | `resources/styles/dark.qss` (534 lines), `resources/sentinel.qrc`, PNG icons in `resources/icons/` | System tokens (DESIGN_SYSTEM.md section 2), bundle `style/*.css`, bundle `assets/` |
| Q6 | QtSql (`QSqlDatabase`, `QSqlQuery`, `QSqlRecord`, `QSqlError`) | `src/core/Database.cpp`, `src/core/Database.h` | SQLite C API behind a small C++23 wrapper; `fuse_ui_sql::SqliteModel` for views (PLATFORM.md V8) |
| Q7 | QtNetwork (`QNetworkAccessManager`, `QNetworkReply`, `QNetworkRequest`, `QEventLoop`, `QTcpServer`, `QTcpSocket`, `QHttpServer`) | `src/ingest/UKPoliceSource.*`, `src/ingest/WeatherSource.*`, `src/core/UpdateChecker.*`, `src/api/LocalApiServer.*` | Removed. The engine has no sockets. Updates come from `svc.update`; the API becomes a session-bus interface |
| Q8 | `QSettings`, `QStandardPaths` | `src/core/AppConfig.cpp`, `src/core/AppConfig.h`, `src/main.cpp` | `svc.settings`; data paths from PLATFORM.md section 10 |
| Q9 | `QObject`, signals and slots, `QTimer`, `QThread`, `QMutex`, `QAtomicInt`, `QLoggingCategory` | `src/core/SentinelLogger.*`, `src/core/UpdateChecker.*`, `src/ingest/DataSource.h`, `src/audit/ProvenanceLog.*`, `src/ui/*` | `std::mutex`, `std::atomic`, `std::jthread`, bus signals, a plain ring-buffer logger |
| Q10 | QtCore value types in the mathematics: `QString`, `QVector`, `QMap`, `QSet`, `QPair`, `QDateTime`, `QDate`, `QTimeZone`, `QUuid`, `QtMath`, `QRegularExpression`, `QJsonObject/Array/Document`, `QFile`, `QTextStream`, `QByteArray` | all of `src/models/` (20 files), `src/inference/` (14), `src/benchmark/` (6), `src/nlp/` (4), `src/audit/` (2), `src/core/CrimeEvent.h`, `src/core/DataExporter.*`, `src/ingest/CsvImporter.*`, `DataQualityScorer.*`, `IngestEnricher.*` | `std::string`, `std::vector`, `std::flat_map`, `std::chrono`, FORGE text and JSON facilities, own CSV reader. This is the bulk of the port |
| Q11 | QtTest harness and `QT_QPA_PLATFORM=offscreen` UI tests | `tests/*.cpp` (405 of 495 files use QtTest), `tests/CMakeLists.txt` | Plain C++ test runner for the engine; `.fuseui` golden and a11y-driven UI tests for the app |
| Q12 | CI and packaging that install Qt | `.github/workflows/ci.yml:49, 123-131`, `.github/workflows/release.yml`, `packaging/linux/build_release.sh`, `packaging/linux/build_wsl.sh`, `packaging/windows/*.ps1`, `packaging/windows/build_msvc.bat`, `installer/sentinel_installer.nsi`, `packaging/windows/sentinel.nsi`, `packaging/linux/sentinel.desktop` | PBSD package build, offline, on HardenedBSD |
| Q13 | `QApplication` entry point | `src/main.cpp` | The engine becomes a helper service; the app starts in the shared app host |

## 2. Scope

In scope:

- Overall posture score and status, with the reasons behind it.
- Verified and measured boot state, boot history, attestation quotes.
- Integrity checks: executables (veriexec), packages, app bundles, system configuration, ZFS health.
- Hardening profile state and drift from the chosen profile.
- Detection: rule-based and statistical anomaly detection over audit, process, capability and network events.
- Alerts, incidents, timeline, evidence chain, response actions.
- Scans: quick, full, custom, removable media, scheduled; signature and rule sets.
- Quarantine store.
- Recommendations, security policy, reports.

Out of scope:

| Item | Owner |
|---|---|
| Editing firewall rules, blocklists, DNS policy | SE02 (SE01 shows state and links) |
| Anonymous routing and VPN | SE03 |
| Passwords, keys, tokens, certificate store | SE04 |
| File encryption, secure erase, metadata cleaning | SE05 |
| Granting and revoking app capabilities, lineage tree | AD05 (SE01 links to the app's page) |
| Raw log browsing and crash reports | AD04 |
| Package updates, boot environments, SBOM view | SY05 |
| Process list and resource graphs | SY04 |
| Disk encryption set-up | AD02, AD01 |
| USB and device policy editing | VM05 |
| Packet capture and dissection | DV05 |
| Crime analytics (SENTINEL's present use) | Not in the suite. If wanted, it needs its own catalogue entry; see section 13 |

## 3. Users and top tasks

Personas:

- **Everyday user**: wants a green tick or one clear thing to fix.
- **Security-minded user**: checks boot measurements, reads alerts, tunes detection.
- **Administrator**: sets a policy for several machines, exports reports, proves compliance.
- **Incident responder**: needs a timeline, evidence with hashes, and isolation actions.
- **Assistive-technology user**: needs every status in words, not colour.

| # | Task | Target |
|---|---|---|
| T1 | See whether the machine is safe | Open the app (0 steps); also in the panel indicator |
| T2 | Fix the top recommendation | 2 clicks: recommendation → "Fix" |
| T3 | Check that the last boot was verified | 1 click: Boot |
| T4 | Run a quick scan | 1 click or `Ctrl+Shift+Q` |
| T5 | Scan a file or folder from Files | Context action in SY02, 2 steps |
| T6 | Open an alert and see why it fired | 2 steps: alert → evidence tab |
| T7 | Isolate an app named in an alert | 2 steps: alert → "Suspend app" |
| T8 | Restore a quarantined file | 3 steps: Quarantine → select → "Restore" |
| T9 | Compare today's integrity state with last week | 3 steps: Integrity → baseline → "Compare" |
| T10 | Switch hardening profile | 3 steps: Hardening → profile → "Apply" (reboot prompt if needed) |
| T11 | Export an incident report | 3 steps: incident → Export → format |
| T12 | Mark an alert as a false positive and stop repeats | 2 steps: alert → "Not a threat" |
| T13 | Find every event for one app in one hour | 3 steps: Events → filter app → time range |
| T14 | Verify the audit log has not been altered | 1 click: Evidence → "Verify chain" |

## 4. Features

### 4.1 Overview and posture

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE01-F001 | Overall status | One of Protected, Attention, At risk, Unknown. Icon, word and colour. Derived from the worst open item, never from the score alone | M | 1 |
| SE01-F002 | Posture score | 0–100, weighted sum of domain scores (boot 25, integrity 20, hardening 20, detection 15, updates 10, network 10). Weights shown and editable in policy | S | 1 |
| SE01-F003 | Domain tiles | Eight tiles: Boot, Integrity, Hardening, Detection, Network, Updates, Encryption, Accounts. Each shows state, one line of reason, last check time | M | 1 |
| SE01-F004 | "Why" drill-down | Every score links to the checks that produced it, with pass, fail, unknown counts | M | 1 |
| SE01-F005 | Score history | 90-day line chart with markers for boots, updates, profile changes | S | 2 |
| SE01-F006 | Unknown is not green | A check that cannot run counts as Unknown and lowers the score; shown separately | M | 1 |
| SE01-F007 | Top recommendations strip | The three highest-ranked actions with a "Fix" button each | M | 1 |
| SE01-F008 | Recent alerts list | Last 10 open alerts with severity, source, age | M | 1 |
| SE01-F009 | Activity summary | Counts for the last 24 h: capability denials, first contacts, blocked connections, failed logins | S | 1 |
| SE01-F010 | Panel indicator feed | Publishes status to SH02 so quick settings shows a shield state | M | 1 |
| SE01-F011 | Other security apps row | Live state cards for SE02, SE03, SE04, SE05, AD05 with one-click open | S | 1 |
| SE01-F012 | Refresh | `F5` re-runs cheap checks (under 2 s); expensive ones are queued | M | 1 |
| SE01-F013 | Forecast band | Optional: expected alert rate for the next 7 days with an uncertainty band (ported `RiskForecaster`, `GPRegression`) | C | 3 |

### 4.2 Boot

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE01-F014 | Boot chain view | Stages firmware → loader → kernel → modules → root dataset → init, each Verified, Measured only, Not checked or Failed | M | 1 |
| SE01-F015 | Secure Boot state | Enabled, set-up mode, enrolled keys (PK, KEK, db, dbx summary) | M | 1 |
| SE01-F016 | Measured boot | TPM 2.0 PCR values 0–15 (SHA-256 bank) compared with the expected set for this boot environment | M | 2 |
| SE01-F017 | Event log replay | Replays the TCG event log and shows which event changed which PCR | S | 2 |
| SE01-F018 | Expected-value management | Record current PCRs as expected after a signed update; SY05 supplies predicted values before reboot | M | 2 |
| SE01-F019 | Boot history | Table of the last 200 boots: time, boot environment, kernel hash, verdict, changed PCRs | S | 2 |
| SE01-F020 | Loader and kernel signatures | Signer identity, key fingerprint, validity for loader, kernel and each loaded module | M | 1 |
| SE01-F021 | Module lock state | Shows whether module loading after boot is forbidden (`hardening.forbid_kmod`) and lists modules loaded | M | 1 |
| SE01-F022 | Local attestation quote | Produces a TPM quote over a nonce and verifies it locally; saved as evidence | S | 3 |
| SE01-F023 | Remote attestation export | Writes a quote file for an external verifier. No network use by this app | C | 3 |
| SE01-F024 | Failed-boot fallback notice | Shows when the loader fell back to a previous kernel or boot environment | M | 2 |
| SE01-F025 | Disk unlock binding | States whether the disk key is sealed to PCRs, passphrase only, or both (read from AD02) | S | 2 |
| SE01-F026 | No-TPM mode | Machines without a TPM show "Measured boot not available" and what that means; never a pass | M | 1 |

### 4.3 Integrity

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE01-F027 | Veriexec state | Loaded, active, enforcing, locked flags; manifest signer; count of entries | M | 1 |
| SE01-F028 | Veriexec violations | List of refused executions and fingerprint mismatches from the audit stream | M | 1 |
| SE01-F029 | Package file check | Hash every file owned by a package against the signed package manifest; report changed, missing, extra | M | 1 |
| SE01-F030 | App bundle check | Re-verify each bundle's `SIGNATURES` tree hash (PLATFORM.md section 3) | M | 1 |
| SE01-F031 | Configuration baseline | Hash and diff of watched configuration paths against a named baseline snapshot | M | 2 |
| SE01-F032 | Baselines | Create, name, sign, compare and delete baselines; a baseline is a signed manifest of path, size, mode, owner, BLAKE3 | M | 2 |
| SE01-F033 | Diff view | Side-by-side text diff for changed text files; metadata diff for others | S | 2 |
| SE01-F034 | Accept change | Mark a change as expected (records who and why); it joins the next baseline | M | 2 |
| SE01-F035 | Revert from snapshot | Offer the ZFS snapshot copy of a changed file; hands off to SY02 snapshot browser | S | 2 |
| SE01-F036 | ZFS health | Pool state, checksum errors, last scrub, dataset encryption state, read-only root state | M | 1 |
| SE01-F037 | Scheduled integrity runs | Daily light run, weekly full run by default; on battery policy | M | 2 |
| SE01-F038 | Set-uid and world-writable audit | Lists unexpected set-uid files and world-writable directories on system datasets | S | 2 |
| SE01-F039 | Trust store drift | New or removed root certificates since baseline (data from SE04) | S | 2 |
| SE01-F040 | Reproducibility link | For a changed package file, link to SY05's reproducibility and SBOM page | C | 3 |

### 4.4 Hardening

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE01-F041 | Profiles | Standard, Strict, Paranoid, Custom. A profile is a signed list of settings with expected values | M | 1 |
| SE01-F042 | Kernel hardening table | Every `hardening.*` sysctl with compiled default, current value, profile value, verdict. The source tree has 59 names, 20 with compiled defaults (`docs/ROADMAP.md` section 15) | M | 1 |
| SE01-F043 | PaX feature state | ASLR, PAGEEXEC, MPROTECT, SEGVGUARD, SHM hardening, MAP32 state, system-wide | M | 1 |
| SE01-F044 | Per-app exemptions | Apps or binaries excused from a PaX feature, who granted it, when; flag each as a risk | M | 2 |
| SE01-F045 | W^X check | Reports any process that holds a writable-executable mapping (should be none, SI-3) | M | 2 |
| SE01-F046 | Securelevel and module policy | Current securelevel, `forbid_kmod`, `harden_rtld`, `harden_tty` | M | 1 |
| SE01-F047 | Drift detection | Alert when a value moves away from the active profile | M | 1 |
| SE01-F048 | Apply profile | Preview changes, apply through the privileged policy service, show which need a reboot | M | 2 |
| SE01-F049 | Roll back | Previous settings kept for 30 days; one-click revert | M | 2 |
| SE01-F050 | Service exposure | Listening sockets and the service behind each; unexpected listeners flagged | M | 1 |
| SE01-F051 | Account hygiene | Accounts without a lock timeout, with empty passwords, with administrator rights, stale accounts | S | 2 |
| SE01-F052 | Session hygiene | Screen lock delay, auto-login state, remote login state | S | 1 |
| SE01-F053 | Encryption state | Root, home, swap and removable media encryption; unencrypted items flagged | M | 1 |
| SE01-F054 | Explain setting | `F1` on a row opens the handbook entry: what it does, what it breaks | S | 2 |
| SE01-F055 | Profile compare | Table of two profiles side by side | C | 3 |
| SE01-F056 | Profile import and export | Signed `.pbsd-profile` JSON; unsigned profiles are shown as untrusted | S | 3 |

### 4.5 Detection

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE01-F057 | Event intake | Normalises records from `svc.audit`, kernel audit (BSM), veriexec, pf log summaries (through SE02), login records, package and update events | M | 1 |
| SE01-F058 | Rule engine | Declarative rules: match on fields, sequence within a window, threshold, absence. Rules are signed files | M | 1 |
| SE01-F059 | Built-in rule pack | Capability-denial bursts, repeated auth failure, new listener, new set-uid file, W^X attempt, exec from a writable path, PaX exemption added, audit gap | M | 1 |
| SE01-F060 | Rate baselines | Expected event rate per source × event type × hour (ported `PoissonBaseline` with overdispersion) | S | 2 |
| SE01-F061 | Anomaly scoring | Isolation score, local density ratio and z-scores per event (ported `AnomalyDetector`), with the reasons list | S | 2 |
| SE01-F062 | Burst detection | Self-exciting model flags clusters of related events (ported `HawkesProcess`) | S | 2 |
| SE01-F063 | Repeat-target detection | Same asset hit again within a window (ported `NearRepeatVictimisation`) | C | 3 |
| SE01-F064 | Event grouping | Links related alerts into one incident by time, entity and behaviour overlap (ported `SeriesDetector`: DBSCAN and Jaccard) | S | 2 |
| SE01-F065 | Behaviour similarity | Compares an incident's behaviour terms with past resolved incidents (ported `MOAnalyser`, TF-IDF cosine) | C | 3 |
| SE01-F066 | Combined score with uncertainty | Weighted ensemble; shows model disagreement separately from noise (ported `EnsemblePredictor`) | S | 2 |
| SE01-F067 | Confidence by evidence | Likelihood-ratio update per piece of evidence (ported `EvidenceScorer`), each contribution listed | S | 2 |
| SE01-F068 | Entity graph | Processes, apps, files, endpoints and handles as nodes; centrality and communities (ported `CoOffendingAnalyser`: PageRank, betweenness) | S | 3 |
| SE01-F069 | Sensor quality gate | Scores each event source for completeness and time precision; low-quality sources are marked and down-weighted (ported `DataQualityScorer`) | S | 2 |
| SE01-F070 | Learning period | New installs learn for 7 days; statistical alerts are labelled "learning" and do not notify | M | 2 |
| SE01-F071 | Detector tuning | Per-detector sensitivity, enable, quiet hours; preview of how many alerts last week's data would have produced | S | 2 |
| SE01-F072 | Detector quality | Precision at k, calibration curve, expected calibration error from user verdicts (ported `BenchmarkMetrics`, `CalibrationAnalyser`) | C | 3 |
| SE01-F073 | False-positive disparity | Shows whether some apps draw far more false alerts than others (ported `BiasAuditor`, re-keyed to apps) | C | 3 |
| SE01-F074 | Custom rules | Rule editor with syntax check, test against stored events, sign with a local key from SE04 | S | 3 |
| SE01-F075 | Audit-gap detector | Alerts when the append-only audit chain has a gap or a sequence break | M | 1 |
| SE01-F076 | No cloud lookups | Detection never sends hashes or events anywhere | M | 1 |

### 4.6 Alerts, incidents and timeline

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE01-F077 | Alert list | Virtualised table: severity, title, entity, detector, first seen, count, state | M | 1 |
| SE01-F078 | Severity | Info, Low, Medium, High, Critical; set by rule or by score band | M | 1 |
| SE01-F079 | Alert states | Open, Acknowledged, In progress, Resolved, Not a threat, Suppressed | M | 1 |
| SE01-F080 | Alert detail | What happened, why it fired (rule text or score parts), entities, evidence, suggested actions | M | 1 |
| SE01-F081 | Deduplication | Repeats fold into one alert with a counter and a sparkline | M | 1 |
| SE01-F082 | Suppression rules | "Not a threat" can create a scoped rule: this app, this path, this detector, with an expiry | M | 2 |
| SE01-F083 | Incidents | A named group of alerts with owner note, status, severity, timeline | M | 2 |
| SE01-F084 | Merge and split | Merge incidents, move alerts between them; the grouping engine respects overrides (as SENTINEL's case overrides do, `src/ui/CaseWorkspaceWidget.cpp`) | S | 2 |
| SE01-F085 | Timeline | Zoomable time axis with lanes per source; boots, updates, alerts and user actions as marks | M | 2 |
| SE01-F086 | Evidence chain | Each alert carries the provenance records that led to it: source, stage, action, detail, data hash (ported `ProvenanceLog`) | M | 1 |
| SE01-F087 | Chain verification | Verifies the hash chain of evidence and of `svc.audit`; reports the first broken link | M | 2 |
| SE01-F088 | Notes | Markdown notes on alerts and incidents, timestamped | S | 2 |
| SE01-F089 | Notifications | High and Critical notify at once; lower ones go to a daily digest. Honours do-not-disturb except Critical | M | 1 |
| SE01-F090 | Contradiction flag | Marks alerts whose evidence conflicts (ported `HintEngine` contradiction pairs) | C | 3 |
| SE01-F091 | Events table | All normalised events with filters, saved filters, column chooser | M | 1 |
| SE01-F092 | Pivot | From any entity (app, file, endpoint, user) open all events, alerts and incidents that name it | S | 2 |
| SE01-F093 | Local summary | Optional plain-language summary of an incident by `svc.model`; marked as generated; never used for scoring | C | 3 |

### 4.7 Response

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE01-F094 | Suspend app | Stops the app instance and its helpers; windows show "Suspended by Security Centre" | M | 2 |
| SE01-F095 | Revoke capabilities | Opens AD05 at the app with the capabilities named in the alert preselected | M | 2 |
| SE01-F096 | Cut network for an app | Asks SE02 to block the app; shown as a temporary rule with expiry | M | 2 |
| SE01-F097 | Network isolation | Asks SE02 to engage the kill switch | S | 2 |
| SE01-F098 | Quarantine file | Moves the file into the quarantine store | M | 2 |
| SE01-F099 | Snapshot now | Asks `svc.backup` for a snapshot to preserve state | S | 2 |
| SE01-F100 | Lock session | Calls SH03 | S | 2 |
| SE01-F101 | Roll back update | Opens SY05 at boot environments | S | 2 |
| SE01-F102 | Response playbooks | Ordered lists of the actions above, run with one confirmation; each step logged | C | 3 |
| SE01-F103 | Every action audited | Response actions are written to `svc.audit` with the alert ID | M | 2 |

### 4.8 Scans

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE01-F104 | Quick scan | Autostart items, running binaries, app bundles, downloads folder (by portal grant), temp paths | M | 2 |
| SE01-F105 | Full scan | Every dataset the scan service may read; resumable; throttled | M | 2 |
| SE01-F106 | Custom scan | Files or folders picked through the portal or sent by SY02's context action | M | 2 |
| SE01-F107 | Removable media scan | Offered when media is mounted (event from VM05); never automatic execution | S | 2 |
| SE01-F108 | Scheduled scans | Daily quick, weekly full by default; pause on battery, on full-screen, on load | S | 2 |
| SE01-F109 | Detection methods | Known-bad hash sets, byte-pattern rules, structural checks (packed or malformed executables), script heuristics | M | 2 |
| SE01-F110 | Signature sets | Signed sets delivered as packages by `svc.update` or imported from a file; version and age shown | M | 2 |
| SE01-F111 | Rule import | Reads a documented subset of YARA rule syntax and ClamAV hash lists; own matcher, no foreign engine | S | 3 |
| SE01-F112 | Archive scanning | Unpacks in the archive helper sandbox with depth, size and ratio limits | S | 2 |
| SE01-F113 | Scan results | Per item: verdict, rule, hash, path, action taken; export | M | 2 |
| SE01-F114 | Exclusions | Paths and hashes; each exclusion is listed as a posture finding | M | 2 |
| SE01-F115 | On-exec check | Optional check of a binary's hash on first execution from a user-writable dataset; result cached by hash | C | 3 |
| SE01-F116 | Scan progress | Files per second, current path, time left; survives closing the window | M | 2 |
| SE01-F117 | Open in disposable sandbox | For a suspicious document, hand off to the viewer in a disposable sandbox | C | 3 |

### 4.9 Quarantine

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE01-F118 | Quarantine store | Encrypted, non-executable dataset; items stored with original path, owner, mode, hashes, reason | M | 2 |
| SE01-F119 | List and detail | Table with reason, rule, date, size, origin; detail shows metadata and a hex preview of the first 4 KiB | M | 2 |
| SE01-F120 | Restore | To the original path or a chosen folder; needs confirmation; adds an exclusion only if asked | M | 2 |
| SE01-F121 | Delete for ever | Removes the item and its key | M | 2 |
| SE01-F122 | Retention | Auto-delete after 30 days by default | S | 2 |
| SE01-F123 | Export sample | Writes an encrypted, password-protected archive for analysis elsewhere | C | 3 |
| SE01-F124 | Send to analysis tools | Open in DV04 or DV07 read-only | C | 3 |
| SE01-F125 | Rescan after update | Items are rescanned when signature sets change; cleared verdicts are offered for restore | S | 3 |

### 4.10 Recommendations, policy and reports

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE01-F126 | Ranked recommendations | Each has a title, the risk it removes, effort, the score gain, and the evidence (ranking ported from `HintEngine`) | M | 1 |
| SE01-F127 | One-click fix | Where a fix is a setting, apply it through the owning service; otherwise deep-link to the owning app | M | 1 |
| SE01-F128 | Dismiss with reason | Dismissed items stay listed under "Accepted risks" with the reason and date | M | 1 |
| SE01-F129 | Policy document | One signed file: profile, score weights, detector settings, scan schedule, retention, notification rules | M | 2 |
| SE01-F130 | Policy lock | An administrator can lock policy; users see it read-only | S | 2 |
| SE01-F131 | Policy import and export | Signed `.pbsd-secpolicy`; diff before apply | S | 3 |
| SE01-F132 | Posture report | Current state of every check; Markdown, HTML, JSON, CSV, PDF (exporters ported from `DataExporter`, `LeadReportGenerator`) | M | 2 |
| SE01-F133 | Incident report | Timeline, evidence chain with hashes, actions taken, notes | M | 2 |
| SE01-F134 | Scheduled reports | Weekly posture report saved to a chosen folder | C | 3 |
| SE01-F135 | Signed reports | Reports can be signed with a key from SE04; the signature covers the JSON form | S | 3 |
| SE01-F136 | Benchmark mapping | Tags checks with the IDs of public hardening benchmarks where a mapping exists | C | 3 |
| SE01-F137 | Print | Any report through `svc.portal.print` | S | 2 |

### 4.11 App basics, accessibility, recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE01-F138 | Global search | `Ctrl+K` over checks, alerts, incidents, settings, help | M | 1 |
| SE01-F139 | Saved filters | Named filters for events and alerts | S | 2 |
| SE01-F140 | Diagnostics panel | The engine's ring-buffer log with category filter (replaces SENTINEL's debug console) | S | 2 |
| SE01-F141 | CSV log import | Import external logs for offline analysis with column auto-detect (ported `CsvImporter`, re-targeted) | C | 3 |
| SE01-F142 | Status in words | Every tile has a text state for screen readers and high contrast | M | 1 |
| SE01-F143 | Read-only mode | Without administrator authentication the app shows everything and changes nothing | M | 1 |
| SE01-F144 | Store recovery | Event store is checked at start; on damage, the last snapshot is offered and the damage is itself an alert | M | 2 |
| SE01-F145 | Service-down state | If the monitor service is not running the status is Unknown, with a restart button | M | 1 |
| SE01-F146 | Command-line twin | `secctl status|scan|report` for scripts and remote shells; same service, same policy | S | 2 |

## 5. GUI

Layout pattern: Dashboard, with sidebar + content for the sections (DESIGN_SYSTEM.md section 4).

### 5.1 Window map

```
Main window
├─ Sidebar: Overview · Boot · Integrity · Hardening · Detection · Alerts · Incidents · Timeline ·
│           Events · Scans · Quarantine · Recommendations · Policy · Reports
├─ Content (one page per sidebar item)
│   ├─ Overview (dashboard)
│   ├─ Boot ── tabs: Chain · Measurements · History · Attestation
│   ├─ Integrity ── tabs: Executables · Packages and apps · Configuration · Storage · Baselines
│   ├─ Hardening ── tabs: Profile · Kernel · Apps and exemptions · Services · Accounts
│   ├─ Detection ── tabs: Detectors · Rules · Anomalies · Entity graph · Quality
│   ├─ Alerts ── list + detail (three-pane)
│   ├─ Incidents ── list + workspace (Summary · Alerts · Timeline · Evidence · Notes)
│   ├─ Timeline
│   ├─ Events
│   ├─ Scans ── tabs: Run · Results · Schedule · Signatures · Exclusions
│   ├─ Quarantine
│   ├─ Recommendations ── tabs: Open · Accepted risks · Done
│   ├─ Policy
│   └─ Reports
├─ Inspector (F4): properties of the selected check, alert, event or file
├─ Dialogs: Apply profile · New baseline · Compare baselines · Custom scan · Restore item ·
│           Suppress alert · Export report · Rule editor · Import policy · Verify chain result
├─ Sheets: Response actions · Accept change · Dismiss recommendation
├─ Popovers: Score breakdown · PCR detail · Detector preview · Filter builder
└─ Panels: Diagnostics (bottom dock) · Job queue (status bar popover)
Shell surfaces (drawn by SH02/SH04, not by this app): shield indicator, notifications, administrator prompt
```

### 5.2 Main window

Overview at 1280×800.

```
+------------------------------------------------------------------------------------------+
| [=] Security Centre                    [ Search checks, alerts…  Ctrl+K ]     [▁][▢][✕] |
+--------------+---------------------------------------------------------------------------+
| > Overview   | (1) ATTENTION — 2 things need you            Score 82 /100 (2) [Why?]     |
|   Boot       |     Last full check 09:14 today                         [Check now F5](3) |
|   Integrity  +---------------------------------------------------------------------------+
|   Hardening  | (4)                                                                       |
|   Detection  | +--Boot-----------+ +--Integrity------+ +--Hardening-----+ +--Detection--+|
|   Alerts  3  | | ✔ Verified      | | ! 2 changed     | | ✔ Strict        | | ✔ Running   ||
|   Incidents  | | PCRs match      | | config files    | | 0 drifted       | | 3 open      ||
|   Timeline   | | 08:02 today     | | 09:14 today     | | 09:14 today     | | learning 2d ||
|   Events     | +-----------------+ +-----------------+ +-----------------+ +-------------+|
|   Scans      | +--Network--------+ +--Updates--------+ +--Encryption----+ +--Accounts---+|
|   Quarantine | | ✔ Firewall on   | | ! 4 pending     | | ✔ All disks     | | ✔ Lock 5 min||
|   Recommend. | | kill switch off | | 1 security      | | swap encrypted  | | no auto-login|
|   Policy     | +-----------------+ +-----------------+ +-----------------+ +-------------+|
|   Reports    +--------------------------------------+------------------------------------+
|              | (5) Recommendations                  | (6) Open alerts                    |
|              | 1 Install 1 security update  [Fix]   | High  New listener :8443  notes  2m|
|              | 2 Review 2 changed files    [Review] | Med   14 denials  org.x.viewer  1h |
|              | 3 Run first full scan        [Scan]  | Low   Login failed ×3          3h  |
|              |                         [See all 7]  |                      [See all 3]   |
|              +--------------------------------------+------------------------------------+
|              | (7) Score, 90 days   ▁▂▃▅▅▆▆▇▇▇▆▇▇█  | (8) Last 24 h: 31 denials ·        |
|              |      ▲boot ▲update ▲profile          |  6 first contacts · 112 blocked    |
+--------------+---------------------------------------------------------------------------+
| (9) Monitor running · Events 14 203 today · Audit chain verified 09:14 · Scan idle       |
+------------------------------------------------------------------------------------------+
```

Callouts:

1. Status banner: `panel` with icon, word, one-line reason. Click opens the worst item.
2. Score: `label` plus a `link` that opens the score-breakdown popover.
3. Primary action of the view: `button`.
4. Domain tiles: `gridView` of eight `panel` cards, each a button. Reflows to 2 columns under 1100 px, 1 column under 640 px.
5. Recommendations: `listView`, three rows, each with one action `button`.
6. Open alerts: `modelListView`; double-click opens the alert.
7. Score history: `chart` (line) with event markers.
8. Activity summary: `label` row; each number is a `link` to a filtered Events view.
9. Status bar: service state, event count, chain state, job queue popover.

States: loading (tiles show a skeleton and "Checking…"), monitor down (banner Unknown, tiles greyed, "Start monitor" button), read-only (action buttons replaced by "Authenticate to change"), narrow (sidebar overlay, lists stacked).

### 5.3 Other views and dialogs

**Boot › Chain and Measurements** (1280×800)

```
+---------------------------------------------------------------------------------------+
| Boot   [Chain] [Measurements] [History] [Attestation]                                 |
+---------------------------------------------------------------------------------------+
| Last boot 08:02 today · boot environment "default-2026-11-10" · ✔ Verified            |
|                                                                                       |
| (1) Firmware ──▶ Loader ──▶ Kernel ──▶ Modules (14) ──▶ Root dataset ──▶ Init         |
|     ✔ measured   ✔ signed    ✔ signed   ✔ all signed    ✔ verified       ✔ veriexec   |
|                                                                                       |
| (2) PCR | Expected          | Actual            | State   | Changed by                |
|     0   | 3f9a…c21e         | 3f9a…c21e         | ✔ match | firmware code             |
|     4   | 81d0…77ab         | 81d0…77ab         | ✔ match | loader                    |
|   > 7   | a4e2…0b13         | a4e2…0b13         | ✔ match | Secure Boot policy        |
|     8   | 55c1…9e0f         | 1b7d…44a2         | ✖ differ| kernel command line       |
|                                                                                       |
| (3) Event log for PCR 8:  #41 kernel command line "…"   #42 loader.conf hash          |
|                                    [Record current as expected…] [Export quote…]      |
+---------------------------------------------------------------------------------------+
```

1. Chain: custom row of `panel` stages joined by arrows; each stage is focusable and opens its detail in the inspector.
2. `tableView`, monospace hash columns, copy on `Ctrl+C`.
3. `listView` of TCG events for the selected PCR.

States: no TPM ("Measured boot not available on this machine"), mismatch (banner in `--danger`, "What changed" list first), learning (no expected values yet; offers to record).

**Integrity › Configuration**

```
+---------------------------------------------------------------------------------------+
| Integrity  [Executables] [Packages and apps] [Configuration] [Storage] [Baselines]    |
+---------------------------------------------------------------------------------------+
| Baseline [ after-install-2026-11-01 v]   Compared 09:14   [Run now] [New baseline…]   |
+--------------------------------------------+------------------------------------------+
| State    Path                     Changed  | /etc/pf.conf                             |
| ! mod    /etc/pf.conf             Tue 14:02| size 1 204 → 1 310   mode 0644 = 0644    |
| ! mod    /etc/rc.conf             Tue 14:02| BLAKE3 9c1e…  →  2d07…                   |
| + new    /usr/local/etc/x.conf    Mon 09:40| changed by: org.pbsd.firewall (audit #88)|
| − gone   /etc/motd.template       Sun 11:15|------------------------------------------|
|                                            | - block in all                           |
|                                            | + block in log all                       |
|                                            |                                          |
|                                            | [Accept change…] [Restore from snapshot] |
+--------------------------------------------+------------------------------------------+
```

`splitter` with `tableView` left; right is a metadata `propertyGrid` over a diff view (`codeEdit` read-only in diff mode; a real diff widget is FUSE-WP-38).

**Hardening › Kernel**

```
+---------------------------------------------------------------------------------------+
| Hardening  [Profile] [Kernel] [Apps and exemptions] [Services] [Accounts]             |
+---------------------------------------------------------------------------------------+
| Profile: Strict (signed, system)     18 of 20 match     [Change profile…] [Roll back] |
| [ Filter…            ]  ( ) All  (•) Drifted  ( ) Needs reboot                        |
| Setting                          | Default | Profile | Current | State                |
| hardening.pax.aslr.status        | 2       | 3       | 3       | ✔                    |
| hardening.pax.mprotect.status    | 2       | 3       | 3       | ✔                    |
| hardening.pax.tpe.status         | 1       | 2       | 1       | ! drift   [Fix]      |
| hardening.forbid_kmod            | 0       | 1       | 0       | ! needs reboot       |
+---------------------------------------------------------------------------------------+
```

**Alerts** (three-pane)

```
+------------------+------------------------------+-------------------------------------+
| Filters          | Sev  Title            Age    | New listener on port 8443           |
| (•) Open       3 | High New listener     2m  >  | High · Open · rule net.new-listener |
| ( ) Acknowledged | Med  14 denials       1h     | Entity: org.pbsd.notes (pid 4121)   |
| ( ) Resolved     | Low  Login failed ×3  3h     | [Summary] [Evidence] [Events] [Notes]|
| Severity         |                              | Why: process bound a TCP socket on  |
| [x] Critical     |                              | 0.0.0.0:8443; its manifest has      |
| [x] High         |                              | net.none.                           |
| [x] Medium       |                              | Confidence 0.91 (3 pieces of        |
| [ ] Low  [ ] Info|                              | evidence, 0 contradictions)         |
| Detector [All v] |                              | Suggested: Suspend app · Revoke net |
| Entity [      ]  |                              | [Suspend app] [Not a threat…] [⋯]   |
+------------------+------------------------------+-------------------------------------+
```

**Incident workspace › Timeline**

```
+---------------------------------------------------------------------------------------+
| Incident #12 "Notes app opened a listener"   High · In progress   [Export…] [Resolve] |
| [Summary] [Alerts 3] [Timeline] [Evidence 9] [Notes]                                  |
+---------------------------------------------------------------------------------------+
|            14:00        14:05        14:10        14:15        14:20                  |
| audit      ·  ·  ▲deny ▲deny ▲deny            ▲grant                                  |
| process          ■ exec helper                       ■ exit                           |
| network                     ◆ bind :8443   ◆ first contact 203.0.113.9                |
| actions                                         ● Suspend app (you)                   |
| [- zoom +]  [Fit]  lanes: [x] audit [x] process [x] network [x] actions [ ] integrity |
+---------------------------------------------------------------------------------------+
```

`timeline` widget with lanes; marks are focusable; `Enter` opens the event in the inspector.

**Scans › Run**

```
+---------------------------------------------------------------------------------------+
| Scans  [Run] [Results] [Schedule] [Signatures] [Exclusions]                           |
+---------------------------------------------------------------------------------------+
|  [Quick scan]   [Full scan]   [Custom scan…]        Signatures 2026-11-11 · 1 day old |
|                                                                                       |
|  Full scan — running 00:14:22                                         [Pause] [Stop]  |
|  [███████████░░░░░░░░░░░░░░]  43%   812 files/s   about 19 min left                   |
|  /usr/local/share/doc/…/manual.pdf                                                    |
|  Scanned 702 114 · Skipped 12 (excluded) · Found 1                                    |
|  ! Suspicious  ~/Downloads/setup.bin   rule pe.packed.upx-mod   [Quarantine] [Ignore] |
+---------------------------------------------------------------------------------------+
```

**Apply profile dialog**

```
+------------------------------------------------------------+
| Apply the "Paranoid" profile?                              |
| 7 settings change. 2 need a reboot. 1 may break apps:      |
|   hardening.pax.mprotect.status 2 → 3 (no exemptions)      |
| [x] Take a snapshot first    [x] Keep roll-back for 30 days|
| [Show all changes]                  [Cancel] [Apply profile]|
+------------------------------------------------------------+
```

### 5.4 Menus

```
File      New Baseline… Ctrl+N · Import Policy… · Import Signatures… · Import Log (CSV)… ·
          Export Report… Ctrl+E · Print… Ctrl+P · Close Ctrl+W · Quit Ctrl+Q
Edit      Copy Ctrl+C · Copy as JSON Ctrl+Shift+C · Select All Ctrl+A · Find Ctrl+F · Preferences Ctrl+,
View      Overview Ctrl+1 · Boot Ctrl+2 · Integrity Ctrl+3 · Hardening Ctrl+4 · Detection Ctrl+5 ·
          Alerts Ctrl+6 · Incidents Ctrl+7 · Timeline Ctrl+8 · Events Ctrl+9 · Scans Ctrl+0 ·
          Sidebar F9 · Inspector F4 · Diagnostics Ctrl+Shift+D · Density ▸ · Refresh F5 · Full Screen F11
Check     Check Now F5 · Verify Boot Measurements · Run Integrity Check Ctrl+I ·
          Verify Audit Chain Ctrl+Shift+V · Compare Baselines…
Scan      Quick Scan Ctrl+Shift+Q · Full Scan Ctrl+Shift+F · Custom Scan… Ctrl+Shift+O ·
          Pause Ctrl+Shift+P · Stop · Schedule… · Exclusions…
Alert     Acknowledge A · Resolve R · Not a Threat… N · Add to Incident… I · New Incident from Selection
          Ctrl+Shift+N · Suspend App · Cut Network for App · Quarantine File · Add Note Ctrl+M
Policy    Change Hardening Profile… · Edit Policy · Lock Policy… · Export Policy… · Detector Tuning…
Help      Security Centre Help F1 · Explain Selected Check Shift+F1 · Keyboard Shortcuts · About
```

### 5.5 Keyboard and pointer

- App shortcuts beyond the reserved set: the `Ctrl+1…0` page keys, `A`/`R`/`N`/`I` on a focused alert, `J`/`K` next and previous row, `Space` toggles the inspector preview, `Ctrl+Shift+V` verify chain.
- Context menu, check row: Run this check, Explain, Copy value, Open owning app, Accept as risk.
- Context menu, alert: Acknowledge, Resolve, Not a threat…, Add to incident, Pivot on entity, Copy as JSON.
- Context menu, event: Show in timeline, Filter by this value, Exclude this value, Copy.
- Context menu, quarantine item: Restore…, Delete for ever, Open in Hex Editor (DV04), Copy hash.
- Drag and drop: drop files or folders on Scans to start a custom scan (handles arrive from the shell's drop portal); drag alerts onto an incident; drag a report row out as a file (Markdown).
- Timeline: wheel zooms about the pointer, drag pans, `+`/`-` zoom, `Home` fits.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | New install: "No checks have run yet" with "Check now". Alerts: "Nothing needs you". Quarantine: "Nothing is quarantined" |
| Loading | Skeleton rows; cheap checks fill in as they return; the banner says "Checking…" and is never green until done |
| Error | The failed check shows Unknown with the error text and "Retry"; errors never read as pass |
| Offline | No change: the app never needs the network. Signature age is shown |
| Locked | Policy locked by an administrator: controls are read-only with a lock icon and who locked it |
| Read-only | Without authentication: view everything, change nothing |
| Service down | Banner "Monitor is not running"; history is still readable from the store |
| Narrow | Under 640 px: sidebar overlay, tiles one column, three-pane becomes list → detail navigation |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar | `listView` with badges |
| Page tabs | `tabBar` / `tabWidget` |
| Tiles | `gridView` of `panel` |
| Tables (events, alerts, PCRs, sysctls, quarantine) | `tableView` over `SqliteModel` |
| Score history, rates, calibration curve | `chart` |
| Timeline | `timeline` |
| Entity graph | `nodeGraph` (read-only layout mode) |
| Diff | `codeEdit` read-only now; diff view FUSE-WP-38 |
| Notes, help, report preview | `markdownView` |
| Inspector | `propertyGrid` |
| Scan progress | `progressBar` |
| Dialogs, sheets, popovers | `dialog`, `popupMenu`; sheets and popovers need FUSE-WP-30 |

Missing:

- **SE01-GAP-1**: heat-map chart type (hour × weekday rate grid). SENTINEL draws this by hand in `src/ui/TemporalHeatmapWidget.cpp`; FUSE `chart` needs a heat-map series (FUSE-WP-46).
- **SE01-GAP-2**: `nodeGraph` automatic layout (force-directed) for a read-only graph of up to 2 000 nodes (FUSE-WP-50).
- **SE01-GAP-3**: `timeline` with labelled lanes and point marks rather than clips (FUSE-WP-50).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | View models, navigation, filters, report assembly, calls to the monitor service |
| UI | FUSE UI | app host | `.fuseui` pages, CSS |
| Security monitor service | C++23 | system service `svc.security` (one per machine, starts at boot) | Owns the event store, runs checks on schedule, runs detectors, raises alerts, publishes status |
| Detection engine | C++23 (SENTINEL `models/`, `inference/`, `benchmark/`, `audit/` ported off Qt) | library inside the monitor service | Baselines, anomaly scores, grouping, evidence scoring, ranking, calibration, provenance |
| Boot state reader | C++23 | helper of the service, capability mode, holds only the TPM and EFI variable handles | Reads PCRs, event log, Secure Boot variables; makes quotes |
| Integrity checker | C++23 | helper, read-only handles to system datasets | Hashes files, reads veriexec state, compares with manifests |
| Hardening checker | C++23 | helper, read-only sysctl handle | Reads `hardening.*`, securelevel, listeners, account settings |
| Policy applier | C++23 | privileged helper, started only for an authenticated change | Writes sysctls and configuration for a profile; records roll-back data |
| Scan service | C++23 | helper, read-only handles to the datasets being scanned | Walks files, calls the content matcher |
| Content matcher | C++23 | helper per scan worker, capability mode, no file system, no network | Parses hostile bytes: hashes, pattern rules, structural checks |
| Quarantine store | C++23 | helper, sole owner of the quarantine dataset handle | Move in, restore, delete |
| Report renderer | ForgeScript | app host | Markdown, HTML, JSON, CSV; PDF through the print pipeline (G-16) |
| `secctl` | ForgeScript | command-line client | Same bus interface as the app |

```
 svc.audit ─┐                         ┌─ boot state reader (TPM, EFI vars)
 kernel BSM ─┤   ┌──────────────────┐ ├─ integrity checker (ro datasets, veriexec)
 SE02 pf sum.┼──▶│ security monitor │─┼─ hardening checker (sysctl ro)
 SY05 events ┤   │ service          │ ├─ scan service ─▶ content matcher (no fs, no net)
 login recs ─┘   │  + detection eng.│ ├─ quarantine store
                 │  + event store   │ └─ policy applier (privileged, on demand)
                 └───────┬──────────┘
        session bus      │ org.pbsd.security.v1 (status, checks, alerts, scans, policy)
   ┌─────────────────────┼─────────────────┬───────────────┐
   ▼                     ▼                 ▼               ▼
 Security Centre app   SH02 indicator   secctl        SE02 / AD05 / SY05 (actions)
 (ForgeScript+FUSE UI)
```

### 6.2 Data model

Store: SQLite at `/var/db/pbsd/security/events.db` (owned by the service), WAL mode, `schema_version` table. The app's own data (`~/.local/share/org.pbsd.security-center/`) holds saved filters and window state only. SENTINEL's store has tables `events`, `leads`, `audit_log`, `zone_risk_scores` and `schema_version` (`SENTINEL:src/core/Database.cpp:98-223`); the new schema keeps the versioning approach and replaces the crime fields.

| Entity | Fields |
|---|---|
| `event` | `id` u64, `ts` i64 ns UTC, `source` enum(audit, bsm, veriexec, pf, login, pkg, boot, scan, self), `kind` text, `severity` u8, `subject_app` text, `subject_pid` u32, `subject_uid` u32, `object_kind` enum(file, endpoint, handle, device, setting, key), `object` text, `outcome` enum(allow, deny, fail, info), `attrs` JSON, `src_seq` u64, `src_hash` blob32, `quality` f32 |
| `check` | `id` text (`boot.pcr.match`), `domain` enum, `title_key` text, `weight` f32, `interval_s` u32, `cost` enum(cheap, medium, heavy) |
| `check_result` | `check_id`, `ts`, `state` enum(pass, fail, unknown, accepted), `value` text, `expected` text, `detail` JSON |
| `alert` | `id` u64, `first_ts`, `last_ts`, `count` u32, `severity` u8, `state` enum, `detector` text, `rule_id` text, `title` text, `entity` text, `score` f32, `confidence` f32, `epistemic` f32, `aleatoric` f32, `incident_id` u64 null |
| `alert_event` | `alert_id`, `event_id` |
| `evidence` | `id` u64, `alert_id`, `ts`, `source` text, `stage` text, `action` text, `detail` text, `data_hash` blob32, `prev_hash` blob32 (fields follow `SENTINEL:src/audit/ProvenanceLog.h`, plus the chain link) |
| `incident` | `id`, `title`, `state`, `severity`, `created_ts`, `resolved_ts`, `summary_md` |
| `note` | `id`, `target_kind`, `target_id`, `ts`, `author_uid`, `body_md` |
| `suppression` | `id`, `detector`, `scope_kind`, `scope_value`, `reason`, `created_ts`, `expires_ts`, `created_by` |
| `baseline` | `id`, `name`, `created_ts`, `signer_fpr`, `manifest_path`, `entry_count` |
| `boot_record` | `boot_id` blob16, `ts`, `boot_env` text, `kernel_hash` blob32, `pcr` blob (16×32), `verdict` enum, `fallback` bool |
| `scan_job` | `id`, `kind` enum(quick, full, custom, media), `started_ts`, `ended_ts`, `state`, `files_scanned` u64, `bytes_scanned` u64, `findings` u32, `resume_token` blob |
| `finding` | `id`, `scan_id`, `path`, `blake3` blob32, `sha256` blob32, `verdict` enum(known_bad, suspicious, clean), `rule` text, `action` enum(none, quarantined, ignored) |
| `quarantine_item` | `id`, `orig_path`, `orig_uid`, `orig_mode`, `size`, `blake3`, `reason`, `rule`, `ts`, `expires_ts`, `wrapped_key` blob |
| `recommendation` | `id`, `check_id`, `rank` f32, `gain` f32, `effort` enum, `state` enum(open, accepted_risk, done), `reason` text |
| `model_state` | `detector` text, `version` u32, `fitted_ts`, `params` blob (baselines, means, deviations) |

Retention: events 90 days by default, alerts and incidents until deleted, boot records 200 boots. Migration is forward only with a copy of the previous file kept. Policy and profiles are signed JSON files under `/etc/pbsd/security/`.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Session-bus records (`org.pbsd.security.v1`) | yes | yes | Schema compiler (G-08) | app host; service |
| BSM audit records | yes | no | OpenBSM in base (`hbsd/src/contrib/openbsm`, BSD licence) | intake helper inside the service sandbox |
| TCG event log, TPM 2.0 structures | yes | quote files | Own C++23 reader | boot state reader |
| EFI variables (Secure Boot) | yes | no | Own code | boot state reader |
| Veriexec manifests | yes | no | Own reader of the manifest format used by `hbsd/src/sbin/veriexec` | integrity checker |
| Package manifests, bundle `SIGNATURES` | yes | no | FORGE signing code (Ed25519 over BLAKE3, PLATFORM.md V14) | integrity checker |
| Baseline manifest (`.pbsd-baseline`, JSON lines + detached signature) | yes | yes | Own code | integrity checker |
| Profile and policy (`.pbsd-profile`, `.pbsd-secpolicy`, signed JSON) | yes | yes | Own code | app host (verify) and policy applier (verify again) |
| Detection rules (`.pbsd-rule`, declarative text) | yes | yes | Own parser | service |
| YARA subset, ClamAV `.hdb` / `.hsb` hash lists | yes | no | Own parser and matcher; no foreign engine | content matcher |
| ELF, PE, Mach-O, scripts, archives (scan targets) | yes | no | Own structural checkers; archives through UT01's archive helper | content matcher; archive helper |
| Reports: Markdown, HTML, JSON, CSV | no | yes | Own (ported from `SENTINEL:src/core/DataExporter.cpp`) | app host |
| PDF report | no | yes | Print pipeline (G-16) | print service |
| CSV log import | yes | no | Own reader (logic from `SENTINEL:src/ingest/CsvImporter.cpp`) | import helper, no fs beyond the passed handle |

### 6.4 Integration

Intents: handles `scan` (files, folders), `open` for `.pbsd-profile`, `.pbsd-secpolicy`, `.pbsd-baseline`, `.pbsd-rule`; URL scheme `security:` (`security:alert/41`, `security:check/boot.pcr.match`). Emits `open` to SE02, SE03, SE04, SE05, AD05, AD04, SY05, SY02, DV04, DV07; `share` and `print` for reports.

Services used: `svc.audit` (read), `svc.notify`, `svc.settings`, `svc.update` (state, signature packages), `svc.backup` (snapshot), `svc.search` (alerts and checks as documents), `svc.keyring` (signing key reference), `svc.model` (optional summaries), `svc.portal.file`, `svc.portal.print`, `svc.power` (pause scans on battery).

Interface offered: `org.pbsd.security.v1` with `Status()`, `ListChecks()`, `RunCheck(id)`, `ListAlerts(filter)`, `SetAlertState()`, `StartScan(kind, handles)`, `ScanFile(handle) → verdict` (used by IN03 and SY02), `Quarantine(handle)`, `GetPolicy()`, `ProposePolicy()`, and signals `StatusChanged`, `AlertRaised`.

**SENTINEL component map.** Left column is Verified (`SENTINEL:` path). Right columns are Proposed.

| SENTINEL component (path under `src/`) | What it does today | Security Centre use | View |
|---|---|---|---|
| `inference/AnomalyDetector` | Isolation score, local density, temporal and spatial z-scores over crime events | Same scores over host events; feature vector becomes time, hour, source, kind, entity | Detection › Anomalies |
| `models/PoissonBaseline` | Zone × time × type rate baseline with Negative Binomial overdispersion | Source × hour × kind rate baseline | Detection › Detectors; Overview activity |
| `models/HawkesProcess` | Self-exciting near-repeat model | Burst detection | Detection › Anomalies; Timeline bursts |
| `models/NearRepeatVictimisation` | Knox-ratio clustering, decay-scored alerts | Repeat-target detection | Alerts |
| `models/SeriesDetector` | DBSCAN in space and time plus MO Jaccard linkage | Grouping alerts into incidents | Incidents |
| `models/KDEHotspot` | 2D Gaussian kernel density hotspots | Density over hour × weekday | Detection › Anomalies heat map |
| `models/GPRegression` | Gaussian-process regression with posterior | Trend line with band | Overview forecast |
| `models/BayesianHierarchical` | Gamma–Poisson zone hierarchy | Per-app rates shrunk towards the system rate (helps apps with little data) | Detection › Detectors |
| `models/RiskForecaster` | Multi-day zone risk | Expected alert rate, 7 days | Overview forecast |
| `models/EnsemblePredictor` | Weighted ensemble, epistemic and aleatoric split | Combined alert score with uncertainty | Alert detail |
| `models/TemporalFeatures` | Cyclical time encoding, lunar phase, payday, night and weekend flags | Cyclical time, night and weekend flags only; lunar and payday dropped | (internal) |
| `inference/EvidenceScorer` | Bayesian likelihood-ratio updating | Alert confidence | Alert detail › Evidence |
| `inference/HintEngine` | Ranked leads, contradiction detection, quality weighting | Ranked recommendations; contradiction flag on alerts | Recommendations; Alerts |
| `inference/MOAnalyser` | TF-IDF cosine similarity to resolved cases | Similar past incidents | Incident › Summary |
| `inference/CoOffendingAnalyser` | PageRank, betweenness, communities on person–incident graphs | Same on app–file–endpoint graphs | Detection › Entity graph |
| `inference/LeadReportGenerator` | Markdown and HTML reports with provenance chains | Incident report | Reports |
| `inference/GeographicProfiler` | Rossmo geographic profile | Not carried over: no geographic problem here | none |
| `nlp/MOExtractor`, `nlp/CrimeClassifier` | Keyword extraction and TF-IDF crime classification | Not carried over as is; the TF-IDF tokeniser is reused by behaviour similarity | none |
| `ingest/DataQualityScorer` | Completeness, precision, reliability, quarantine threshold for records | Sensor quality gate | Detection › Detectors (source quality column) |
| `ingest/CsvImporter` | Column auto-detect for crime CSV | External log import | File › Import Log |
| `ingest/UKPoliceSource`, `ingest/WeatherSource`, `ingest/IngestEnricher` | Network data sources and weather enrichment | Not carried over | none |
| `benchmark/BenchmarkMetrics`, `CalibrationAnalyser` | PAI, AUC, Brier, MRR, precision at k; ECE, reliability diagram | Detector quality from user verdicts | Detection › Quality |
| `benchmark/BiasAuditor` | Disparate impact, feedback-loop risk | False-positive disparity across apps | Detection › Quality |
| `audit/ProvenanceLog` | Per-event provenance entries with data hash; JSON, CSV, HTML export | Evidence chain, with a hash link added | Alert › Evidence; Incident › Evidence |
| `core/Database` | SQLite store, schema version, WAL | Event store (new schema) | Events |
| `core/DataExporter` | Markdown, JSON, CSV, HTML export | Report exporters | Reports |
| `core/AppConfig` | `QSettings` thresholds | Policy and `svc.settings` | Policy |
| `core/SentinelLogger` | Ring-buffer logger with categories | Diagnostics log | Diagnostics panel |
| `core/UpdateChecker` | GitHub release check | Removed (`svc.update`) | none |
| `api/LocalApiServer` | Read-only REST on a local port | Replaced by the session-bus interface | none |
| `ui/DashboardWidget` | Map and summary metrics | Overview | Overview |
| `ui/EventsTableWidget` | Filterable table | Events | Events |
| `ui/AnalyticsWidget`, `CalibrationDashboardWidget`, `TemporalHeatmapWidget` | Charts, reliability diagram, heat map | Detection › Quality and Anomalies | Detection |
| `ui/CaseWorkspaceWidget` | Case filter, merge and split, lead history, export | Incident workspace | Incidents |
| `ui/CoOffendingGraphWidget` | Graph with zoom and pan | Entity graph | Detection › Entity graph |
| `ui/LeadsWidget` | Ranked leads | Recommendations and Alerts | Recommendations; Alerts |
| `ui/AuditLogWidget` | Provenance viewer | Evidence tab | Alerts; Incidents |
| `ui/SettingsWidget` | Thresholds, theme | Policy; Preferences | Policy |
| `ui/DebugConsoleWidget` | Live log | Diagnostics | Diagnostics panel |
| `ui/MapWidget`, `GeoProfileHeatmapWidget` | Custom map and profile heat map | Not carried over | none |

Boot, integrity, hardening, scanning and quarantine have **no** SENTINEL code behind them. They are new work (section 12).

## 7. Security and privacy

Capabilities requested by the app:

| Capability | Reason |
|---|---|
| `fs.app` | Saved filters, window state |
| `fs.portal` | Pick files to scan, save reports, import policies |
| `net.none` | The app and the monitor service have no sockets |
| `svc.security` | Talk to the monitor service (new service, see section 13) |
| `svc.audit` | Read the audit trail for the Events view |
| `svc.notify`, `svc.settings`, `svc.search` | Alerts, preferences, search results |
| `svc.update`, `svc.backup` | Update state, snapshot before a profile change |
| `svc.keyring` | Reference to the report-signing key; the key never leaves the keyring |
| `svc.portal.file`, `svc.portal.print` | Reports |
| `ai.local` | Optional incident summary; off by default |
| `bg.run` | Not requested: the service runs in the background, the app does not |

The app has no elevated rights. Changes go through the service, which asks the shell for administrator authentication (SH03) per change.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| Truth of the status | Malware with user rights | Fake a green status or hide alerts | Status comes from a system service the user's apps cannot write to; the panel indicator reads the service, not the app; Unknown is never green |
| Event store and evidence | Attacker with user or app rights | Delete or edit history | Store owned by the service; evidence and audit are hash-chained; gaps alert (F075); snapshots |
| Monitor service | Hostile file content | Exploit the scanner's parsers | Content matcher runs with no file system and no network, one process per worker, memory limits, fuzzed |
| Quarantined files | Any app | Execute or read quarantined content | Dataset is `noexec`, encrypted per item, reachable only by the quarantine helper |
| Policy and profiles | Attacker with user rights | Load a weak profile | Profiles are signed; unsigned shown as untrusted; apply needs authentication; drift alerts |
| Detection rules | Supply chain | Malicious rule pack that suppresses alerts | Rule packs are signed packages; suppressions are listed as posture findings |
| Privileged policy applier | Local attacker | Confused deputy: get it to write arbitrary sysctls | It accepts only keys present in a signed profile and verifies the signature itself |
| Boot verdict | Attacker with kernel control | Lie about PCRs | Stated limit: a compromised kernel can lie to user space. Quotes are signed by the TPM and can be checked off the machine (F023) |
| The user | Alert fatigue | Too many alerts so the real one is missed | Learning period, deduplication, digest for low severity, calibration view |

Stored: events, alerts, boot records, baselines, quarantine, all on local system datasets. Leaves the machine: nothing. Reports leave only when the user saves or shares them.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `notify.min_severity` | enum | high | Lowest severity that notifies at once |
| `notify.digest_time` | time | 18:00 | Daily digest of lower alerts |
| `notify.break_dnd_critical` | bool | true | Critical alerts ignore do-not-disturb |
| `events.retention_days` | int | 90 | Event retention |
| `detect.learning_days` | int | 7 | Learning period |
| `detect.sensitivity` | enum | balanced | quiet, balanced, strict |
| `detect.contamination` | float | 0.05 | Expected anomaly share (same meaning as `AnomalyDetector`'s constructor argument, `SENTINEL:src/inference/AnomalyDetector.h`) |
| `integrity.light_interval_h` | int | 24 | Light run interval |
| `integrity.watch_paths` | list | `/etc`, `/usr/local/etc`, `/boot` | Configuration baseline paths |
| `scan.quick_daily` | bool | true | Daily quick scan |
| `scan.full_weekly` | bool | true | Weekly full scan |
| `scan.on_battery` | bool | false | Scan on battery |
| `scan.max_cpu_percent` | int | 30 | Throttle |
| `scan.archive_depth` | int | 4 | Nested archive limit |
| `scan.archive_ratio_limit` | int | 200 | Decompression ratio limit |
| `quarantine.retention_days` | int | 30 | Auto-delete age |
| `hardening.profile` | enum | standard | Active profile |
| `hardening.rollback_days` | int | 30 | Roll-back retention |
| `report.default_format` | enum | markdown | Report format |
| `ai.summaries` | bool | false | Local incident summaries |

## 9. Accessibility and languages

- Every state has an icon and a word; the banner is a live region and announces changes ("Status changed to Attention: two items").
- The timeline and the entity graph each have a table twin with the same data, reachable with `Ctrl+T`.
- Hashes are read in groups of four characters; a "copy" action sits beside each.
- Charts carry a text summary (minimum, maximum, trend) for screen readers.
- Check titles and explanations come from string tables keyed by check ID; rule packs carry their own tables.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| First frame | under 400 ms cold; Overview filled from cached results at once |
| Cheap check set (F5) | under 2 s |
| Monitor service idle | under 0.5% of one core, under 60 MiB resident |
| Event intake | 5 000 events/s sustained without loss; back-pressure counted and shown |
| Events table | 10 million rows, scroll at 60 frames/s (virtualised, indexed queries under 50 ms) |
| Full scan | at least 150 MiB/s on NVMe at the default 30% CPU cap |
| Timeline | 50 000 marks, pan and zoom under 16 ms a frame |

## 11. Testing and proof

Test plan (targets; nothing has run):

- **Unit**: every ported model against the numeric expectations in SENTINEL's own tests (`SENTINEL:tests/test_anomaly_*.cpp`, `test_poisson_*.cpp`, `test_hawkes_*.cpp`, `test_evidence_scorer*.cpp` and the rest), rewritten off QtTest. The port is accepted when old and new give the same numbers on the same inputs.
- **Differential**: build SENTINEL with Qt once on a reference machine, dump model outputs for fixed inputs as JSON, compare with the port.
- **Golden**: `.fuseui` renders of every page in dark, light and high contrast at 100% and 200%.
- **Scripted UI**: the 14 top tasks through the a11y tree.
- **Fuzz**: BSM reader, TCG event log reader, veriexec manifest reader, rule parser, YARA-subset parser, every structural checker, CSV import, policy and profile JSON.
- **Detection replay**: recorded event sets with labelled attacks; measure precision, recall and alert volume per detector.
- **Fault injection**: kill the service, corrupt the store, break the audit chain, remove the TPM; the status must go Unknown or alert, never green.
- **Boot matrix**: Secure Boot on and off, TPM present and absent, changed kernel command line, fallback boot.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Evidence hash chain | Any edit or deletion is detected | T1 | Model in Lean 4; ESBMC on the append and verify functions | none known |
| Status aggregation | Status is never Protected while any check is fail or unknown | T1 | Small pure function; proof by PRISM | none known |
| Policy applier | Writes only keys present in a verified signed profile | T2 | Contracts on the write path; ESBMC | Kernel sysctl interface is outside the proof |
| Content matcher | No out-of-bounds access, no unbounded allocation on any input | T2 | Safe subset, contracts, bounded proof, fuzzing | Size of the structural checkers |
| Rule engine | Evaluation terminates; memory bounded by the window | T2 | Bounded model checking | Rule language still to be fixed |
| Detection engine (ported models) | Numeric agreement with SENTINEL; no runtime errors | T3 | Differential tests, sanitizers, fuzzing | Floating-point code; statistical claims are empirical |
| Boot state reader | Parser safety on hostile event logs | T2 | Contracts, fuzzing | TPM driver is trusted |
| Quarantine store | An item is never left both in place and in the store, or in neither | T2 | State-machine model, crash-point tests | File system semantics assumed |

## 12. Build plan

**Phase 1 — usable core**

| ID | Work item | Depends on |
|---|---|---|
| SE01-W01 | Define `org.pbsd.security.v1` schema and the `svc.security` service skeleton | G-08, G-09, FORGE-WP-25, FORGE-WP-26 |
| SE01-W02 | Event store schema, intake from `svc.audit`, retention | SE01-W01, FORGE-WP-11 |
| SE01-W03 | Port `audit/ProvenanceLog`, `core/Database` wrapper, `core/SentinelLogger` off Qt (Q6, Q9, Q10) | SE01-W02 |
| SE01-W04 | Hardening checker: `hardening.*` table, PaX state, securelevel, listeners, encryption state | SE01-W01, FORGE-WP-03 |
| SE01-W05 | Integrity checker: veriexec state, package and bundle verification, ZFS health | SE01-W01, FORGE-WP-09, PBSD veriexec decision (`docs/ROADMAP.md` section 14) |
| SE01-W06 | Boot chain view from loader and kernel signature data; Secure Boot variables | SE01-W01, PBSD loader signing |
| SE01-W07 | Rule engine and the built-in rule pack; alert life cycle; notifications | SE01-W02 |
| SE01-W08 | Status aggregation, score, recommendations (rule-ranked), panel indicator feed | SE01-W04..W07, SH02 |
| SE01-W09 | App shell: Overview, Boot, Integrity, Hardening, Alerts, Events pages | G-01, G-02, G-07, FUSE-WP-12, FUSE-WP-13, FUSE-WP-28 |
| SE01-W10 | Read-only mode and administrator authentication path | SH03, SH04 |

**Phase 2 — full feature**

| ID | Work item | Depends on |
|---|---|---|
| SE01-W11 | Port `models/` and `inference/` off Qt types (Q10); differential harness against SENTINEL | SE01-W03 |
| SE01-W12 | Wire baselines, anomaly, burst, ensemble, evidence scoring into alerts; learning period | SE01-W11 |
| SE01-W13 | Incident grouping (`SeriesDetector`), incidents workspace, merge and split, notes | SE01-W12 |
| SE01-W14 | Timeline page | SE01-GAP-3, FUSE-WP-50 |
| SE01-W15 | Measured boot: PCR read, event log replay, expected values, boot history | TPM driver on PBSD, SY05 predicted values |
| SE01-W16 | Baselines, configuration diff, accept change, snapshot restore hand-off | SE01-W05, SY02, FUSE-WP-38 |
| SE01-W17 | Profiles: signed format, apply, roll-back, policy applier | SE01-W04, SE04 signing keys |
| SE01-W18 | Scan service, content matcher, hash sets, pattern rules, archive scanning | FORGE-WP-27, FORGE-WP-28, UT01 archive helper |
| SE01-W19 | Quarantine store and page | SE01-W18 |
| SE01-W20 | Response actions with SE02, AD05, SH03, `svc.backup` | SE02, AD05 |
| SE01-W21 | Reports (ported exporters), chain verification, print | SE01-W03, G-16 |
| SE01-W22 | Policy document, lock, `secctl` | SE01-W17 |
| SE01-W23 | Delete SENTINEL `src/ui/`, `resources/`, Qt build, Qt CI and Qt packaging (Q1–Q5, Q7, Q11–Q13) from the PBSD branch of SENTINEL | SE01-W11 |

**Phase 3 — advanced**

| ID | Work item | Depends on |
|---|---|---|
| SE01-W24 | Entity graph and centrality | SE01-GAP-2, SE01-W11 |
| SE01-W25 | Detector quality, calibration, disparity views; heat map | SE01-GAP-1, FUSE-WP-46 |
| SE01-W26 | Custom rule editor, YARA-subset and ClamAV hash-list import | SE01-W18 |
| SE01-W27 | Attestation quotes, remote export | SE01-W15 |
| SE01-W28 | Forecast band, behaviour similarity, repeat-target detector | SE01-W12 |
| SE01-W29 | Playbooks, scheduled and signed reports, benchmark mapping | SE01-W20, SE01-W21 |
| SE01-W30 | Local incident summaries | G-15, FORGE-WP-29 |
| SE01-W31 | On-exec check | PBSD kernel exec hook; SE01-W18 |

## 13. Open questions and risks

1. **SENTINEL is a crime-analytics program, not a security monitor** (section 1.1). The catalogue calls SE01 "front end for SENTINEL". This spec reuses SENTINEL's mathematics and builds the host sensors new. The owner should confirm this reading, or say whether a different SENTINEL code base exists. If the crime-analytics product is still wanted on PBSD it needs its own app entry.
2. The Qt port touches every source file (Q10). Estimated at roughly 9 000 lines of non-UI code to convert and 28 UI files to delete. The 405 QtTest files must be rewritten or the numeric expectations extracted.
3. Statistical detectors tuned for crime data may be poor on host events. The differential tests prove the port is faithful, not that the detectors are useful. Phase 2 ships them labelled "experimental" until replay data shows value.
4. New service `svc.security` is not in PLATFORM.md section 8. Needs adding (new platform gap).
5. Verified boot on PBSD is undecided: `MAC_VERIEXEC` is in the tree but not enabled, and no loader signing was found (`docs/ROADMAP.md` section 14). Until then Boot shows "Not checked" for those stages.
6. TPM access on PBSD: `hbsd/src/sys/dev/tpm` exists; a TCG event-log interface for user space is Unverified.
7. Per-process W^X and PaX exemption listing needs a kernel interface that is not specified yet.
8. A signature-set supply does not exist. Someone must build and sign hash and rule sets; without them scans find only structural oddities.
9. The score can mislead. It is secondary to the status word and can be hidden.
10. Scan service reads many user files. Its handle set and audit record need a design review with AD05.
