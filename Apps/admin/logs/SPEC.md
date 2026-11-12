<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Logs

By Odin Loch

| | |
|---|---|
| ID | `AD04` (from CATALOGUE.md) |
| Bundle | `org.pbsd.logs` |
| Category | admin |
| Priority | P1 |
| Backing repos | ParanoidBSD (`syslogd`, OpenBSM audit, kernel message buffer, crash tools in `hbsd/src`; `pbsd/kernel/audit`); FORGE (audit stream, FORGE-WP-11); FUSE (UI) |
| Status | Planned. Nothing built. |

## 1. Purpose

Shows what the system, its services and its apps did and when: system and service logs, kernel messages, the security audit trail (capability grants and denials, first network contacts), crash reports and the boot timeline. It is for users answering "what just happened?" and for administrators and researchers investigating a fault or an intrusion. Logs never leave the machine unless the user exports them, after scrubbing. It replaces reading files under `/var/log` by hand, `dmesg`, `praudit` and the Plasma-era log viewer (PLATFORM.md V20).

## 2. Scope

In scope:

- One viewer over every log source with common time, severity and origin fields.
- The audit trail from `svc.audit` and the kernel audit subsystem.
- Crash reports, stored and analysed locally only.
- Boot timeline and boot comparison.
- Filters, a query language, saved queries, live tail, correlation across sources.
- Integrity: hash-chained records, tamper evidence, verification.
- Export with privacy scrubbing; import of exported bundles and plain log files for reading.
- Retention and size settings for the log store.

Out of scope:

| Item | Owner |
|---|---|
| Security posture, intrusion detection rules and alerts | SE01 Security Centre (consumes the same store) |
| Changing or revoking grants (this app links to the grant) | AD05 Permissions |
| Live process, CPU, memory and I/O views | SY04 System Monitor |
| Firewall rule editing and live connections | SE02 Firewall |
| Packet capture and dissection | DV05 Packet Analyser |
| Debugging a crashed program (debugger, symbols workbench) | DV01 IDE, DV07 Reverse Engineering |
| Update history and boot environment rollback | SY05 Software |
| Opening an arbitrary text file for editing | OF01 Text Editor |

## 3. Users and top tasks

Personas: **Asha**, a user whose app just closed by itself; **Bo**, an administrator checking why a service failed at boot; **Cal**, a security researcher reviewing denials and first contacts; **Dee**, a developer who must send a scrubbed log to a maintainer.

| # | Task | Target |
|---|---|---|
| T1 | See errors from the last hour | 1 click ("Errors" preset) |
| T2 | See what one app logged around the time it crashed | 2 clicks from the crash notification |
| T3 | List every capability denial today | 1 click (Audit ▸ Denials preset) |
| T4 | List every host contacted for the first time this week, per app | 2 clicks |
| T5 | Follow a service live while reproducing a fault | 2 clicks (select service, Live) |
| T6 | Find why boot was slow and what changed since the last boot | 2 clicks (Boot ▸ Compare) |
| T7 | Search all sources for a string or pattern | `Ctrl+F`, type, `Enter` |
| T8 | Save a query and pin it to the sidebar | 2 clicks |
| T9 | Verify that logs have not been altered | 1 click (Integrity ▸ Verify) |
| T10 | Export a time range, scrubbed, for someone else | 4 clicks with a preview of what is removed |
| T11 | From a denial, jump to the grant or app in Permissions | 1 click |
| T12 | See everything related to one event (same process, same handle, same second) | 1 click ("Correlate") |
| T13 | Read a crash report in plain words | 1 click on the report |

## 4. Features

### 4.1 Sources and ingestion

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F001 | Log store service | A system service (`svc.logs`, Proposed) owns one append-only structured store. Apps never read `/var/log` files directly. | M | 1 |
| AD04-F002 | Source: syslog | Messages from the base `syslogd` (Verified in tree: `hbsd/src/usr.sbin/syslogd/`, with Capsicum support files `syslogd_cap*.c` and RFC 5424 output mode per `syslogd.8`) forwarded to the store with facility, severity, host, tag, PID. | M | 1 |
| AD04-F003 | Source: kernel messages | The kernel message buffer (as `dmesg` reads it, `hbsd/src/sbin/dmesg`), with monotonic timestamps mapped to wall time and boot id. | M | 1 |
| AD04-F004 | Source: services | Standard output and error of every supervised service and helper, captured by the service supervisor with service name and restart count. | M | 1 |
| AD04-F005 | Source: apps | Output of app hosts and their helpers, tagged with bundle id and instance; an app can read only its own stream without admin approval. | M | 1 |
| AD04-F006 | Source: `svc.audit` | Grants, denials, revocations, first network contacts, prompts shown and answers. Record fields per FORGE-WP-11.4: time, bundle id, policy hash, control, decision, target. | M | 1 |
| AD04-F007 | Source: kernel audit (BSM) | OpenBSM trail records (Verified in tree: `hbsd/src/contrib/openbsm`, `hbsd/src/sys/security/audit`; PBSD hooks scaffold `pbsd/kernel/audit/pbsd.kernel.audit.cppm`) decoded into the common model: event, subject, object, return. | M | 2 |
| AD04-F008 | Source: crash reports | Kernel dumps summarised by `crashinfo` (Verified: `hbsd/src/usr.sbin/crashinfo`, `hbsd/src/sbin/savecore`) and user-space crash records written by the app host (FORGE-WP-11.1). | M | 1 |
| AD04-F009 | Source: boot timeline | Loader, kernel and service start timestamps. Kernel side from the boot tracing buffer (Verified in tree: `hbsd/src/sys/kern/kern_tslog.c`, `sys/sys/tslog.h`; whether it is compiled in is not established). | M | 2 |
| AD04-F010 | Source: firewall | pf log entries (blocks, per-app decisions) from SE02's logging path. | S | 2 |
| AD04-F011 | Source: updates and installer | SY05 transactions and the AD01 install log. | S | 2 |
| AD04-F012 | Source: storage and backup | Pool events and backup runs (from the AD02 and AD03 services). | S | 2 |
| AD04-F013 | Source: authentication | Logins, lock and unlock, failed attempts, admin approvals (SH03). | M | 1 |
| AD04-F014 | Source: hardware | Device attach and detach, USB policy decisions (VM05), thermal and power events. | S | 2 |
| AD04-F015 | Source: jails, VMs, compatibility layers | Per-guest streams from VM01, VM02, VM03, each its own origin. | C | 3 |
| AD04-F016 | Common record | Every source maps to one record shape (section 6.2) and keeps its raw form. | M | 1 |
| AD04-F017 | Structured fields | Key-value fields are indexed; JSON and logfmt payloads are recognised and split. | S | 2 |
| AD04-F018 | Rate limits | Per-origin ingestion limits with a "N messages dropped" marker record, so one noisy app cannot evict others. | M | 1 |
| AD04-F019 | Time handling | Wall time (UTC stored, local shown), monotonic time and boot id on every record; clock steps are records themselves. | M | 1 |
| AD04-F020 | Open plain files | Open a log file from the portal (text, gzip, zstd, xz) as a temporary source with format detection (syslog, RFC 5424, JSON lines, logfmt). | S | 2 |
| AD04-F021 | Open BSM trail files | Open an audit trail file from the portal and view it decoded. | C | 3 |
| AD04-F022 | Remote logs | Not collected. Receiving syslog from the network is off (matches the `bsdinstall` hardening option "disable syslogd network socket", `hbsd/src/usr.sbin/bsdinstall/scripts/hardening`). | M | 1 |

### 4.2 Viewing

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F023 | Unified list | Virtualised table: time, severity, origin, message; newest at bottom; smooth over 100 million records. | M | 1 |
| AD04-F024 | Source tree | Sidebar: All, System, Kernel, Services ▸ each, Apps ▸ each, Audit, Security, Crashes, Boot, Saved queries. Counts of new errors per node since last view. | M | 1 |
| AD04-F025 | Severity display | Emergency to debug as icon + word + colour token; debug hidden by default. | M | 1 |
| AD04-F026 | Detail inspector | All fields of the selected record, raw form, chain position and hash, related links (process, app, grant, unit). | M | 1 |
| AD04-F027 | Multi-line and long messages | Collapsed to one row with expand; stack traces grouped. | M | 1 |
| AD04-F028 | Wrap and columns | Toggle wrap; column chooser (PID, thread, host, boot, facility, any structured field); column order and width remembered per source. | M | 1 |
| AD04-F029 | Time formats | Local, UTC, relative ("3 min ago"), since boot, delta to previous or to a marked row. | M | 1 |
| AD04-F030 | Density strip | A histogram above the list showing record counts over time by severity; click or drag to jump and to select a range. | M | 2 |
| AD04-F031 | Boot selector | Choose a boot (this boot, previous, by date); list of boots with duration and shutdown kind (clean, crash, power loss). | M | 1 |
| AD04-F032 | Bookmarks and notes | Mark records, add a note; marks listed in a panel and included in exports. | S | 2 |
| AD04-F033 | Highlight rules | Colour or badge rows matching a pattern; rules are per user. | S | 2 |
| AD04-F034 | Collapse repeats | Consecutive identical messages collapse to one row with a count. | M | 1 |
| AD04-F035 | Message templates | Group messages by template (variable parts replaced) to show "top message kinds" for a range. | S | 3 |
| AD04-F036 | Context view | From a filtered result, show N records before and after in the unfiltered stream. | M | 1 |
| AD04-F037 | Split view | Two panes over different sources or times with optional time lock. | S | 2 |
| AD04-F038 | Raw view | Show the original bytes of a record or file (hex for non-text). | S | 2 |
| AD04-F039 | Copy | Copy rows as text, JSON lines or Markdown table; "copy scrubbed" applies the scrub rules. | M | 1 |
| AD04-F040 | Open related app | From a record: open the origin in SY04 (process), AD05 (app permissions), SE02 (connection), AD02 (device), SY05 (package). | M | 2 |

### 4.3 Search, filters, saved queries

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F041 | Filter bar | Chips for time range, severity, origin, boot, plus free text; chips are added from the keyboard or by "filter by this value" on any cell. | M | 1 |
| AD04-F042 | Quick time ranges | Last 15 min, hour, day, week, this boot, previous boot, custom from–to, "around this record ± N". | M | 1 |
| AD04-F043 | Full-text search | Indexed substring and token search over messages across all sources. | M | 1 |
| AD04-F044 | Regular expressions | Regex search (linear-time engine, no backtracking) on message or any field; a time limit per query. | M | 2 |
| AD04-F045 | Query language | Text form: `severity>=warning origin:app:org.pbsd.mail "timeout" -field.host=local since:-2h`. Operators: `AND`, `OR`, `NOT`, parentheses, comparison on numeric fields, `~` regex. | M | 2 |
| AD04-F046 | Query builder | Form equivalent of the query language; edits in one update the other. | S | 2 |
| AD04-F047 | Autocomplete | Field names, known values (origins, units, event kinds) and recent queries. | S | 2 |
| AD04-F048 | Saved queries | Name, query, columns, time rule; pinned to the sidebar; optional badge with the count of new matches. | M | 1 |
| AD04-F049 | Built-in presets | Errors, Denials, First contacts, Failed logins, Admin approvals, Crashes, Service restarts, Kernel warnings, Storage problems, Updates. | M | 1 |
| AD04-F050 | Aggregation | Count by field over a range (by origin, severity, event kind, host) as a table and bar chart. | S | 2 |
| AD04-F051 | Rate over time | Chart of matches per interval for the current query. | S | 2 |
| AD04-F052 | Find in results | `Ctrl+F` find next and previous within the current result without changing the filter. | M | 1 |
| AD04-F053 | Query history | Last 100 queries per user, local, clearable. | S | 2 |
| AD04-F054 | Share a query | Copy a query as text or as a `pbsd-logs:` link that opens it in this app on this machine. | S | 2 |
| AD04-F055 | Alert on match | A saved query can raise a `svc.notify` notification when it matches (evaluated by `svc.logs`, at most once per interval). Security rules proper belong to SE01. | S | 3 |
| AD04-F056 | Natural-language query | With `ai.local`: "errors from mail yesterday evening" is turned into a query the user sees and confirms before it runs. | C | 3 |

### 4.4 Live tail

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F057 | Follow | Live mode appends new matches and keeps the view pinned to the end; scrolling up pauses; a "N new" pill resumes. | M | 1 |
| AD04-F058 | Pause and buffer | While paused, up to 100 000 new records are buffered and counted. | M | 1 |
| AD04-F059 | Live with filters | Any query can be followed; evaluation happens in the service so the app receives only matches. | M | 1 |
| AD04-F060 | Rate guard | Above 2 000 records/s the view samples and says so; the store still has everything. | M | 2 |
| AD04-F061 | Marker line | Insert a marker ("--- started test ---") into the view (and optionally into the store as a user record). | S | 2 |
| AD04-F062 | Tail several origins | Tabs or split panes each following a different query. | S | 2 |
| AD04-F063 | Temporary verbosity | Ask a service or app to raise its log level for N minutes (only those that offer the control; needs admin approval for system services); reverts by itself. | S | 3 |
| AD04-F064 | Sound or flash on match | Optional cue when a followed query matches at error level. | C | 3 |

### 4.5 Audit trail

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F065 | Audit view | Table: time, app, decision (granted, denied, revoked, prompted, expired), capability, resource, duration (once, session, always), who decided (user, policy, profile). | M | 1 |
| AD04-F066 | Decision filters | One-click filters for each decision kind; denials grouped by app and capability with counts. | M | 1 |
| AD04-F067 | First contacts | Table of network first contacts: time, app, host, port, route (clear, VPN, Tor), resolved address family, whether allowed; grouped by app or by host. | M | 1 |
| AD04-F068 | Grant life view | For one grant: created, each use (sampled count per hour), narrowed, revoked; with the lineage id where the platform exposes it (SI-6, `docs/specs/CAPSICUM_LINEAGE.md`). | S | 2 |
| AD04-F069 | Jump to Permissions | Every audit row links to the app or grant in AD05; "Revoke" itself is done there. | M | 1 |
| AD04-F070 | Device use | Camera, microphone, screen, location, USB sessions with start, end and app. | M | 2 |
| AD04-F071 | Admin approvals | Every privileged approval (who, what action, which service) from SH03. | M | 1 |
| AD04-F072 | File-access audit (opt-in) | For chosen folders or datasets, record opens through the file portal and (when kernel audit is on) by path class; off by default; volume estimate shown. | S | 3 |
| AD04-F073 | Kernel audit classes | Choose which BSM classes are recorded (login, exec, administrative, network, file write) with size estimates. | S | 3 |
| AD04-F074 | Policy-change history | Every change to profiles, defaults and per-app policy with before and after. | M | 2 |
| AD04-F075 | Audit summary | Per day or week: counts by decision, new apps seen, new hosts, most-denied capability; as a short report. | S | 2 |
| AD04-F076 | Unusual-activity hints | Simple local statistics (first time an app used a capability, a burst of denials); labelled as hints; real detection is SE01. | C | 3 |
| AD04-F077 | Audit completeness state | Shows whether the audit sink had gaps (service restarts, dropped records) for the selected range. | M | 2 |

### 4.6 Crash reports

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F078 | Crash list | Time, program or app, kind (signal, script error, sandbox violation, kernel panic, watchdog), count of similar crashes. | M | 1 |
| AD04-F079 | Local only | Reports are never sent. The view says so; there is no "send" button. Sharing is an explicit export. | M | 1 |
| AD04-F080 | Report content | Signal or error, faulting thread, ForgeScript stack (from the app host report), native stack with module offsets, loaded bundle version and hash, capability set at the time, last 200 log lines of the app. No memory contents by default. | M | 1 |
| AD04-F081 | Plain-words summary | A template-based sentence: "Mail closed because its helper `mime-helper` tried to use memory it did not own." | M | 2 |
| AD04-F082 | Symbolisation | Resolve addresses with local debug files if installed (from SY05); done by a sandboxed helper. | S | 2 |
| AD04-F083 | Grouping | Reports with the same top frames and bundle version are grouped; trend per version. | S | 2 |
| AD04-F084 | Kernel panics | Panic message, backtrace and the tail of the kernel message buffer from the `crashinfo` text summary; the raw dump stays admin-only. | M | 2 |
| AD04-F085 | Core dumps | Off by default. When enabled per app, dumps are stored encrypted, size-limited and deleted after N days; this app lists and deletes them. | S | 3 |
| AD04-F086 | Open in tools | Hand a report (and dump, if any) to DV01 or DV07. | C | 3 |
| AD04-F087 | Related logs | One click shows all sources ± 60 s around the crash for that process tree. | M | 1 |
| AD04-F088 | Retention | Keep the newest 50 reports per app and 90 days; pinned reports are kept. | M | 1 |
| AD04-F089 | Hardening-related kills | Processes killed by a mitigation (W^X violation, ASLR or PaX policy) are shown as their own kind with the rule named (SI-3). | S | 2 |

### 4.7 Boot timeline

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F090 | Timeline view | Horizontal bars: firmware (if reported), loader, kernel init, each device probe group, root mount, each service start, login ready, session ready. | M | 2 |
| AD04-F091 | Critical chain | Highlights the chain of starts that determined the total time. | S | 2 |
| AD04-F092 | Slowest items | Sorted table of durations. | M | 2 |
| AD04-F093 | Compare boots | Two boots side by side with deltas; new, removed and slower items marked. | S | 2 |
| AD04-F094 | Boot history | Chart of total boot time across boots; marks for updates and boot environment changes. | S | 3 |
| AD04-F095 | Failed and degraded units | Services that failed or timed out at boot with their log lines. | M | 2 |
| AD04-F096 | Boot chain events | Verified-boot results: loader and kernel signature checks, veriexec state, TPM unseal outcome (data from SE01's attestation log). | S | 3 |
| AD04-F097 | Shutdown timeline | The same for shutdown; what delayed power-off. | C | 3 |
| AD04-F098 | Unclean shutdown detection | A boot with no shutdown record before it is flagged. | M | 2 |

### 4.8 Correlation

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F099 | Correlate from a record | Builds a query for the same process tree, app instance, session, handle or grant id, and time window; opens it in a new tab. | M | 2 |
| AD04-F100 | Common identifiers | Records carry `boot_id`, `session_id`, `app_instance`, `pid`, `unit`, and where relevant `grant_id` and `request_id` so joins are exact, not guessed. | M | 1 |
| AD04-F101 | Request tracing | A bus request id propagated through services lets one user action be followed across the app, portals and services (needs bus support, G-08). | S | 3 |
| AD04-F102 | Interleaved view | Selected origins merged in strict time order with an origin column and lane colouring. | M | 1 |
| AD04-F103 | Lane view | Swim lanes per origin on a shared time axis; denials, crashes and restarts as markers. | S | 3 |
| AD04-F104 | Cause hints | Rule-based links: service restart ← crash record; app exit ← denial 50 ms earlier; mount failure ← device detach. Shown as "possibly related". | S | 3 |
| AD04-F105 | Incident folder | Collect records, crash reports, notes and a boot timeline into a named local incident that can be exported as one bundle. | S | 3 |
| AD04-F106 | Local assistant summary | With `ai.local`: summarise the selected range; the prompt is built from scrubbed text and stays on the machine. | C | 3 |

### 4.9 Integrity

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F107 | Hash chain | Each record stores `hash = BLAKE3(prev_hash ‖ canonical record)`; chains are per segment and segments are chained. | M | 1 |
| AD04-F108 | Sealing | Every N minutes or M records the service signs the chain head with a key it holds (Ed25519); the seal is itself a record. | M | 2 |
| AD04-F109 | Forward security | The sealing key is evolved one-way after each seal, so a later compromise cannot forge earlier seals. | S | 3 |
| AD04-F110 | TPM anchoring | Optionally extend a PCR or NV counter with chain heads so truncation of the tail is detectable after reboot. | C | 3 |
| AD04-F111 | Verify | Verify a range or the whole store: chain continuity, seal signatures, segment order, gaps. Result: verified, gaps found (where), tampered (which segment). | M | 1 |
| AD04-F112 | Continuous check | The service verifies new segments on rotation and at boot; failure raises a SE01 alert. | M | 2 |
| AD04-F113 | Tamper evidence in the list | Records in a failed segment carry a "tampered or damaged" badge; missing ranges show a gap row. | M | 1 |
| AD04-F114 | Immutable storage | Segments live on a dataset where only the service can append; closed segments are set read-only and snapshotted. | M | 2 |
| AD04-F115 | Deletion is recorded | Retention deletes whole segments only and leaves a tombstone record with the removed range and its head hash. | M | 1 |
| AD04-F116 | External witness | Export the current chain head as a short string or QR to keep elsewhere; later verification can check against it. | S | 3 |
| AD04-F117 | Export proofs | Exported bundles include the chain hashes and seals so a recipient can verify the excerpt's integrity relative to seal points. | S | 2 |

### 4.10 Export, scrubbing, import

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F118 | Export selection or query | Formats: plain text, JSON lines, CSV, Markdown, and a bundle (`.pbsdlog`: zstd-compressed JSON lines + manifest + chain proofs). | M | 1 |
| AD04-F119 | Scrub rules | Remove or replace: user and host names, home paths, IP and MAC addresses, e-mail addresses, serial numbers and hardware ids, UUIDs, URLs' query strings, tokens and keys (pattern and entropy based), geographic coordinates. | M | 2 |
| AD04-F120 | Consistent pseudonyms | Each removed value becomes a stable placeholder (`<user-1>`, `<ip-3>`) within one export so the log still reads; the mapping stays local. | M | 2 |
| AD04-F121 | Scrub preview | Side-by-side preview with every replacement highlighted and a count per rule; nothing is written until confirmed. | M | 2 |
| AD04-F122 | Custom scrub terms | User-supplied words and patterns to remove; saved per user. | S | 2 |
| AD04-F123 | Scrub by default | Export dialogs start with scrubbing on; turning it off shows what will be exposed. | M | 2 |
| AD04-F124 | Support bundle | Guided export for a named app: its logs, crash reports, version and capability list, system version; scrubbed; a manifest lists every file. | S | 2 |
| AD04-F125 | Share | Hand the exported file to the share sheet (SH04); this app never uploads. | M | 2 |
| AD04-F126 | Encrypt export | Encrypt to a recipient key from SE04 (age-style) or with a passphrase. | S | 3 |
| AD04-F127 | Import bundle | Open a `.pbsdlog` bundle read-only as a separate source; verify its proofs; never merged into the local store. | S | 2 |
| AD04-F128 | Print | Print the selection or a report through the print portal. | C | 3 |
| AD04-F129 | Redaction check | After scrubbing, a second pass scans for anything matching the rules and reports leftovers. | S | 2 |

### 4.11 Retention, privacy, settings

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F130 | Retention per class | Defaults: system and services 30 days or 1 GiB; apps 14 days or 512 MiB; audit 1 year or 2 GiB; crashes 90 days; debug level 3 days. | M | 1 |
| AD04-F131 | Storage view | Size per class and per origin; projected days until the cap; top talkers. | M | 2 |
| AD04-F132 | Per-app log privacy | An app sees only its own records. A user sees their own session's app and audit records. System, kernel and other users' records need admin approval. | M | 1 |
| AD04-F133 | Sensitive-field marking | Sources can mark fields sensitive; these are hidden until revealed and always scrubbed on export. | S | 2 |
| AD04-F134 | Clear logs | Clearing a class is an admin action, recorded with a tombstone; audit records cannot be cleared before their minimum retention (default 30 days) in the Strict profile. | M | 2 |
| AD04-F135 | Persistent or volatile | Option to keep non-audit logs in memory only (lost at reboot) for high-privacy setups. | S | 3 |
| AD04-F136 | Encrypted at rest | The store sits on the encrypted pool; with per-user encrypted homes, per-user app logs live inside the user's dataset. | M | 2 |
| AD04-F137 | Read-only mode for viewers | Without admin approval the app works on the user's own records with a banner saying what is hidden. | M | 1 |
| AD04-F138 | Viewing is audited | Reading system or other users' logs under admin approval is itself an audit record. | S | 2 |
| AD04-F139 | Command line | `logs` command: query, follow, verify, export with the same query language. | S | 2 |

## 5. GUI

Layout pattern: Three-pane (sidebar, list, inspector) with tabs. Default density compact; message text in the monospace face.

### 5.1 Window map

```
Main window
├─ Sidebar: sources tree · saved queries · bookmarks
├─ Tabs (each tab = one query or view)
│   ├─ Log list view (filter bar · density strip · list)
│   ├─ Audit view (Decisions · First contacts · Devices · Approvals · Policy changes · Summary)
│   ├─ Crashes view (list · report)
│   ├─ Boot view (Timeline · Slowest · Compare · History)
│   ├─ Integrity view
│   └─ Storage view
├─ Inspector (F4): record fields · raw · chain · related
├─ Status bar: match count · live state · integrity state · scope (mine / system)
├─ Dialogs
│   ├─ Export (format · scrub rules · preview) · Support bundle
│   ├─ Save query · Manage saved queries · Highlight rules · Query builder
│   ├─ Time range · Go to time · Verify result · Clear logs (typed) · Retention
│   └─ Preferences · Open file or bundle
├─ Sheets: Context (± N records) · Scrub preview · Aggregation result · Crash related logs
└─ Popovers: Column chooser · Field value actions · Boot selector · New-records pill
```

### 5.2 Main window

Shown at 1280×800, log list view on "Apps ▸ Mail". States: empty, loading, error, locked, read-only (5.6).

```
+----------------------------------------------------------------------------------------+
| [=] Logs — Mail                         [search Ctrl+K]                 [▁] [▢] [✕]    |
+------------------+---------------------------------------------------+-----------------+
| (1) All          | (2) [Mail ×] [Denials today ×] [+]                | (7) RECORD      |
|  System      3!  +---------------------------------------------------+ Time 14:02:11.4 |
|  Kernel          | (3) [This boot v][≥ warning v][app:mail ×] [timeout       ] [Save]  |
|  v Services      | (4) ▁▁▂▁▁▁▃▁▁▁▁█▂▁▁▁▁▁▁▁▂▁▁▁  12:00 ─────────── 14:05   [● Live]    |
|     network      +------+----------+------+--------------------------+ Severity error  |
|     svc.backup   | (5)  | Time     | Sev  | Message                  | Origin          |
|  v Apps          |      | 14:01:58 | info | sync started, 3 folders  |  app:org.pbsd.. |
|   > Mail     2!  |      | 14:02:09 | warn | imap: slow response 9.8 s|  helper net     |
|     Browser      |    ✕ | 14:02:11 | DENY | net.host mail.ex.org:993 | PID 4412        |
|  Audit       5   |    > | 14:02:11 | err  | connect failed: denied   | Boot 41         |
|  Security        |      | 14:02:11 | info | retry in 30 s            | Grant g-7f21    |
|  Crashes     1   |      | (×12)    | dbg  | idle                     |  [Open in       |
|  Boot            |      | 14:04:40 | CRASH| mime-helper SIGSEGV      |   Permissions]  |
| SAVED            |      |          |      |                          | Chain ✓ #88213  |
|  ★ Denials today |      |          |      |                          | [Correlate]     |
|  ★ First contacts| (6)  7 of 1,204 shown in range · 2 hidden (debug)  | [Context ±20]   |
+------------------+---------------------------------------------------+-----------------+
| (8) 1,204 matches   Live: off   Integrity: ✓ verified to 14:00   Scope: my session     |
+----------------------------------------------------------------------------------------+
```

1. Sidebar, 220 px, `treeView`; counts are new warnings or worse since last viewed; saved queries below.
2. Tab bar, `tabBar`; each tab keeps its own query, columns and scroll position.
3. Filter bar: chips (`comboBox`-backed), free-text `lineEdit` (accepts the query language), [Save] to pin as a saved query.
4. Density strip, `chart` (stacked bars by severity) with a draggable range; [● Live] toggles follow.
5. Record list, `tableView` (virtualised, model fed by the service). Audit and crash records appear inline with a distinct badge word (DENY, GRANT, CRASH). `>` is the selection.
6. Result line: counts, what is hidden and why.
7. Inspector, 280 px, `propertyGrid` plus action buttons; grant and app values are links.
8. Status bar: total matches, live state, integrity state (click opens Integrity), scope (click asks for admin approval to widen).

### 5.3 Other views and dialogs

**Audit ▸ Decisions (content region, 1040×640)**
```
| [Today v] [All apps v]  (•) All ( ) Granted ( ) Denied ( ) Revoked ( ) Prompted   |
| Time     | App       | Decision    | Capability        | Resource           | By  |
| 14:02:11 | Mail      | ✕ denied    | net.host          | mail.ex.org:993    | policy |
| 13:40:02 | Photos    | ✓ granted   | fs.portal (rw)    | ~/Pictures/Trip    | user, always |
| 13:39:58 | Photos    | ? prompted  | fs.portal         | folder picker      | – |
| 11:15:20 | Browser   | ⟲ revoked   | dev.camera        | camera0            | user (Permissions) |
| (1) Denied by app: Mail 14 · Feeds 3 · Browser 1          [Group by v] [Export…] |
```
`tableView`; (1) aggregation line, each count a filter link. Row action: [Open in Permissions].

**Audit ▸ First contacts**
```
| [This week v]  Group by (•) App ( ) Host                                          |
| v Browser (41 new hosts)                                                          |
|     example.org:443        Mon 09:12   clear   allowed                            |
|     cdn.example.net:443    Mon 09:12   clear   allowed                            |
| v Mail (2 new hosts)                                                              |
|     mail.ex.org:993        Tue 14:02   clear   ✕ denied                           |
| > Feeds (7 new hosts)                                                             |
```
`treeView` with columns; host rows link to SE02.

**Crashes**
```
| Time      | Program            | Kind            | Similar |  (2) mime-helper (Mail 0.3.1) |
| 14:04:40 >| Mail ▸ mime-helper | SIGSEGV         | 3       |  Closed because it read memory|
| Mon 22:10 | Feeds              | script error    | 1       |  it did not own.              |
| 3 Nov     | kernel             | panic           | 1       |  Thread 2 · bundle hash 91ac… |
|           |                    |                 |         |  #0 parse_part +0x1c4         |
| (1) Reports stay on this computer.                         |  #1 walk_tree  +0x88          |
| [Related logs] [Export scrubbed…] [Pin] [Delete]           |  Capabilities at crash: none  |
```
List `tableView`, report pane `markdownView` + `codeEdit` (read-only) for stacks. (1) fixed privacy line. States: empty ("No crashes recorded").

**Boot ▸ Timeline**
```
| Boot 41 · 12 Nov 08:01 · 11.8 s to login · previous shutdown: clean   [Compare with: Boot 40 v] |
| 0 s        2          4          6          8          10         12                |
| loader  ██                                                                          |
| kernel    ████████                                                                  |
| devices        █████  (nvme 0.4 · usb 1.9 ! +1.2 s vs boot 40)                      |
| root mount          █  (unlock 0.3)                                                 |
| services             ██████████████   network ▓▓▓▓  svc.keyring ▓  compositor ▓▓▓   |
| login ready                         ▲ 11.8                                          |
| Slowest: usb probe 1.9 s · network 1.6 s · compositor 1.2 s          [Show logs]    |
```
Bars drawn in a `panel` (custom draw) or `chart` in Gantt mode (`AD04-GAP-2`); every bar is also a row in the "Slowest" `tableView`.

**Integrity**
```
| Log integrity                                        [Verify now] [Export head…]  |
| ✓ Chain verified from 12 Nov 2025 to 12 Nov 2026 14:00 · 8,412 segments           |
| Last seal 14:00:00 · signed by log service key 5e1c…                              |
| ! 1 gap: 3 Nov 02:14–02:19 (power loss, segment 7,921 closed without seal)        |
| Segment | From        | To          | Records | Seal | State                      |
| 8,412   | 13:00       | 14:00       | 14,220  | ✓    | verified                   |
| 7,921   | 3 Nov 02:00 | 3 Nov 02:14 | 3,310   | –    | unsealed tail (gap follows)|
```

**Export with scrub preview (dialog, 900×600)**
```
| Export 1,204 records                       Format [Bundle (.pbsdlog) v]           |
| Remove: [x] user names [x] host name [x] home paths [x] IP/MAC [x] e-mail         |
|         [x] serials/UUIDs [x] URL queries [x] tokens/keys [ ] app names  [+ term] |
| Original                              | Scrubbed                                  |
| asha@pbsd-7f3a imap login ok          | <user-1>@<host-1> imap login ok           |
| GET /?token=9f2c…e1 from 203.0.113.9  | GET /?<removed> from <ip-1>               |
| open /home/asha/Mail/inbox.db         | open <home-1>/Mail/inbox.db               |
| 212 replacements · 0 leftovers found            [Cancel] [Share…] [Save as…]      |
```
Two synchronised `codeEdit` panes (read-only) with highlighted replacements; counts per rule in a tooltip.

**Query builder (dialog)** — rows of `[field v] [operator v] [value]` joined by AND/OR, a live text form below, a match count, [Run] [Save].

**Storage view** — per-class bars (size against cap, days kept), top origins table, [Retention…], [Clear…].

### 5.4 Menus

| Menu | Items (shortcut) |
|---|---|
| File | New tab (`Ctrl+T`) · New window (`Ctrl+N`) · Open log file or bundle… (`Ctrl+O`) · Export… (`Ctrl+E`) · Support bundle… · Print… (`Ctrl+P`) · Close tab (`Ctrl+W`) · Quit (`Ctrl+Q`) |
| Edit | Copy (`Ctrl+C`) · Copy scrubbed (`Ctrl+Shift+C`) · Copy as JSON · Select all (`Ctrl+A`) · Find (`Ctrl+F`) · Find next / previous (`Ctrl+G` / `Ctrl+Shift+G`) · Preferences (`Ctrl+,`) |
| View | Sidebar (`F9`) · Inspector (`F4`) · Density strip (`Ctrl+Shift+H`) · Wrap lines (`Alt+Z`) · Columns… · Time format ▸ Local / UTC / Relative / Since boot / Delta · Show debug (`Ctrl+D`) · Collapse repeats · Split view (`Ctrl+\`) · Refresh (`F5`) |
| Go | Live / follow (`Ctrl+L`) · End (`Ctrl+End`) · Start (`Ctrl+Home`) · Go to time… (`Ctrl+J`) · Next error (`F8`) · Previous error (`Shift+F8`) · Next bookmark (`F2` is rename; uses `Ctrl+.`) · Previous boot (`Alt+Up`) · Next boot (`Alt+Down`) · Back / Forward (`Alt+Left` / `Alt+Right`) |
| Query | Filter by selection (`Ctrl+I`) · Exclude selection (`Ctrl+Shift+I`) · Clear filters (`Ctrl+Backspace`) · Query builder… (`Ctrl+B`) · Save query… (`Ctrl+S`) · Manage saved queries… · Aggregate by… · Correlate (`Ctrl+R`) · Context ± N (`Ctrl+Shift+X`) |
| Record | Bookmark (`Ctrl+M`) · Add note… · Mark as time zero (`Ctrl+0` is zoom reset; uses `Ctrl+Shift+0`) · Open origin in System Monitor · Open in Permissions · Open in Firewall · Show raw |
| Tools | Verify integrity · Storage and retention… · Highlight rules… · Insert marker (`Ctrl+Shift+M`) · Raise verbosity… · Clear logs… |
| Help | Help (`F1`) · Query language reference · Keyboard shortcuts · About |

### 5.5 Keyboard and pointer

List navigation: `Up`/`Down`, `PgUp`/`PgDn`; `Space` expands a multi-line record; `Enter` opens the inspector on it; `e`/`w`/`i`/`d` are not bound (no single-letter shortcuts, to keep type-ahead free). `Ctrl+scroll` zooms the density strip; dragging on it selects a time range; double-click on a cell filters by that value; `Alt+double-click` excludes it.

Context menus: record (Filter by this value, Exclude this value, Context ± 20, Correlate, Bookmark, Copy, Copy scrubbed, Open origin…); source in sidebar (Open in new tab, Follow, Export…, Retention…, Storage); saved query (Open, Rename, Edit, Duplicate, Badge on/off, Delete); crash report (Related logs, Export scrubbed, Pin, Delete, Open in IDE); boot bar (Show logs for this unit, Compare with previous boot).

Drag and drop: a log file or `.pbsdlog` bundle dropped on the window opens as a temporary source; a tab dragged to an edge splits the view; a record dragged out gives text and JSON offers; a filter chip dragged to the sidebar saves a query.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | A query with no match: "No records match" plus the two most likely fixes (widen the time range, include debug). A fresh system: sources listed with zero counts. |
| Loading | First page appears as soon as the service returns it; the density strip fills progressively; a query running over 2 s shows progress and [Cancel]. |
| Error | Service unreachable: banner with [Retry]; the app can still open files and bundles. Bad query: the failing token underlined with a message. |
| Offline | Not applicable; no network use. |
| Locked | Records of a locked user dataset are shown as a gap row "locked: user guest". |
| Read-only / limited scope | Banner: "Showing your session only. [Show system logs…]" which asks for admin approval. |
| Tampered | A red banner on any view that includes a failed segment, with [Details] to Integrity. |
| Narrow (< 640) | Sidebar overlay; inspector as a sheet; list shows time, severity icon, message only. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sources tree, first contacts | `treeView` |
| Record list, audit, crashes, segments | `tableView` (virtualised; needs variable row height for expanded records, FUSE-WP-28) |
| Tabs | `tabBar`, `tabWidget`, `splitter` |
| Filter chips | `comboBox`, `lineEdit`, `toolButton` |
| Density strip, rates, aggregation, boot history | `chart` (FUSE-WP-46) |
| Inspector | `propertyGrid`, `link` |
| Crash report, help | `markdownView` |
| Raw view, stacks, scrub preview | `codeEdit` (read-only, with highlights) |
| Time pickers | `dateTimeEdit`, `calendar` |
| Dialogs | `dialog`, `checkBox`, `radioButton` |

Missing: `AD04-GAP-1` a log list able to hold 100 million rows with a service-backed windowed model, variable row height and stable scroll while rows are appended at the end (a `tableView` extension; overlaps FUSE-WP-28 and FUSE-WP-19); `AD04-GAP-2` Gantt-style bar chart mode for the boot timeline; `AD04-GAP-3` filter-chip input (token field). The history strip need is shared with `AD03-GAP-1`.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Tabs, query text ↔ builder, view models, export flow, saved queries |
| UI | FUSE UI | app host | All views |
| Log store service (`svc.logs`, Proposed) | C++23 | system daemon; append rights to the log dataset; no network | Ingestion, normalisation, hash chain, sealing, index, query engine, live subscriptions, retention, access control |
| Collectors | C++23 | one small process per source class, capability mode | syslog socket reader, kernel message reader, BSM trail reader, supervisor output reader; each can only append to the service |
| `svc.audit` (PLATFORM.md section 8) | C++23 | system daemon | Receives grant, denial and first-contact records from app hosts and services; forwards into the store as a protected class |
| Query engine (inside the service) | C++23 | service | Parse and plan the query language; linear-time regex; aggregation |
| Decode helper | C++23 | app-side helper, capability mode | Parse user-opened files and bundles (text formats, compression, BSM) outside the app host |
| Symbol helper | C++23 | app-side helper, capability mode, read-only handles to debug files | Symbolise crash stacks |
| Scrubber | ForgeScript rules + C++23 matcher in the decode helper | helper | Apply scrub rules, pseudonym map, leftover check |
| Command-line client | ForgeScript | app host without UI | `logs` command |

```
 syslogd ─┐                         ┌─► segments (append-only dataset, hash chain, seals)
 kernel  ─┤ collectors ─► svc.logs ─┼─► index (per segment)
 services─┤   (cap mode)    ▲   │   └─► live subscriptions ─► Logs app (ForgeScript + FUSE UI)
 BSM     ─┘                 │   └─► SE01 (rules), SY04, AD05 (history of use)
 app hosts ─► svc.audit ────┘        decode helper ◄─ opened files / bundles (untrusted)
```

### 6.2 Data model

| Entity | Fields |
|---|---|
| `Record` | `seq: u64`, `time_utc_ns: i64`, `mono_ns: u64`, `boot_id: uuid`, `class: enum{system,kernel,service,app,audit,auth,crash,boot,firewall,storage,update,user}`, `severity: u8` (0–7, syslog scale), `origin: Origin`, `pid: u32?`, `tid: u32?`, `uid: u32?`, `session_id: uuid?`, `app_instance: u64?`, `message: string`, `fields: map<string, Value>`, `raw: bytes?`, `sensitive_keys: string[]`, `prev_hash: bytes32`, `hash: bytes32` |
| `Origin` | `kind: enum{kernel,unit,app,helper,user,collector}`, `name: string` (unit name or bundle id), `helper: string?` |
| `AuditRecord` (class `audit`, typed `fields`) | `bundle_id: string`, `policy_hash: bytes32`, `control: string` (capability), `decision: enum{granted,denied,revoked,prompted,expired,narrowed}`, `target: string`, `duration: enum{once,session,always}?`, `decided_by: enum{user,policy,profile,service}`, `grant_id: string?`, `lineage_id: u64?`, `route: enum{clear,vpn,tor}?` |
| `CrashReport` | `id: uuid`, `time_utc_ns: i64`, `boot_id: uuid`, `program: string`, `bundle_id: string?`, `bundle_hash: bytes32?`, `kind: enum{signal,script_error,sandbox_violation,mitigation_kill,panic,watchdog}`, `signal: u8?`, `threads: StackTrace[]`, `script_stack: string[]`, `capabilities: string[]`, `log_tail_ref: {seq_from: u64, seq_to: u64}`, `group_key: bytes16`, `pinned: bool`, `dump_ref: string?` |
| `BootRecord` | `boot_id: uuid`, `started_utc_ns: i64`, `be_name: string`, `kernel_version: string`, `spans: BootSpan[]`, `login_ready_ms: u32`, `previous_shutdown: enum{clean,crash,power_loss,unknown}` |
| `BootSpan` | `name: string`, `phase: enum{firmware,loader,kernel,device,mount,service,session}`, `start_ms: u32`, `duration_ms: u32`, `parent: string?`, `result: enum{ok,failed,timeout}` |
| `Segment` | `id: u64`, `class`, `first_seq: u64`, `last_seq: u64`, `from_utc_ns: i64`, `to_utc_ns: i64`, `head_hash: bytes32`, `prev_head: bytes32`, `seal: Seal?`, `state: enum{open,closed,sealed,deleted}` |
| `Seal` | `segment: u64`, `head_hash: bytes32`, `time_utc_ns: i64`, `key_epoch: u64`, `signature: bytes64` |
| `Tombstone` | `segment: u64`, `range: {first_seq, last_seq}`, `head_hash: bytes32`, `reason: enum{retention,cleared}`, `actor: string` |
| `SavedQuery` | `id: uuid`, `name: string`, `query: string`, `columns: string[]`, `time_rule: string`, `badge: bool`, `notify: bool` |
| `ScrubProfile` | `rules: set<enum{user,host,home,ip,mac,email,serial,uuid,url_query,token,geo,app}>`, `terms: string[]`, `patterns: string[]` |
| `Bookmark` | `seq: u64`, `note: string`, `created_ns: i64` |

Stores: segments and indexes under `/var/log/store/<class>/` on a dedicated dataset (`exec=off`, `setuid=off`), owned by the service; segment file format is own, versioned, length-prefixed records with the hash chain (version 1). Per-user items (saved queries, bookmarks, highlight rules, scrub profile, pseudonym maps) in `~/.local/share/org.pbsd.logs/user.sqlite` (schema v1). Canonical record encoding for hashing is fixed in the format specification so verification is deterministic.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Segment format v1, seals, tombstones | yes | yes | own code | `svc.logs` (writer); verifier also in the decode helper for bundles |
| syslog RFC 3164 and RFC 5424 lines | yes | no | own parser | syslog collector (capability mode) |
| Kernel message buffer | yes | no | own reader | kernel collector |
| BSM audit trail | yes | no | own decoder written against the format headers in `hbsd/src/contrib/openbsm` (BSD licence); not linking the C library | BSM collector; decode helper for opened files |
| `svc.audit` records | yes | no | schema-compiled (PLATFORM.md section 7) | `svc.logs` |
| Crash record from the app host | yes | no | schema-compiled | `svc.logs` |
| `crashinfo` text summary | yes | no | own tolerant text parser | crash collector |
| JSON lines, logfmt, CSV | yes | yes (export) | own | decode helper (read), app host (write) |
| gzip, zstd, xz for opened files | yes | zstd | zlib, zstd, liblzma from base | decode helper |
| `.pbsdlog` bundle (zstd JSON lines + `manifest.json` + `proofs.bin`) | yes | yes | own | decode helper (read) |
| Query language | yes | — | own parser, grammar published in help | `svc.logs` and app host (for the builder) |
| Regular expressions | — | — | own linear-time matcher or FORGE regex if it guarantees linear time (Unverified) | `svc.logs`, decode helper |
| Debug information (DWARF subset) | yes | no | own reader, line tables and symbols only | symbol helper |

### 6.4 Integration

- Intents handled: `open` and `view` for `text/x-log`, `application/vnd.pbsd.log-bundle`, `application/x-bsm-audit`; `search` with a query; URL scheme `pbsd-logs:` (query links, "show records around time T for origin O"). Emitted: `open` to AD05 (app or grant), SY04 (process), SE02 (host or connection), AD02 (device), SY05 (package), DV01 and DV07 (crash), UT05 (help); `share` for exports.
- Services: `svc.logs` (Proposed), `svc.audit` (read), `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.keyring` (export encryption keys via SE04), `svc.model` (optional).
- Interfaces offered by `svc.logs` to others: `append(Record)` for collectors and services; `query`, `subscribe`, `verify`; `app_log(bundle_id)` scoped to the caller. SE01 subscribes for its rules; AD05 reads "history of use" for a grant; SH02 links crash notifications to `pbsd-logs:` URLs; SY04 shows a service's recent lines.
- Verified from the repo: `syslogd` with Capsicum support files, `newsyslog`, OpenBSM, `praudit` and `auditreduce` build directories, kernel audit under `hbsd/src/sys/security/audit`, the PBSD audit module scaffold, `crashinfo`, `savecore`, `dmesg`, the kernel boot trace buffer source, and the audit record field list in FORGE-WP-11.4. Proposed: `svc.logs`, the segment format, the hash chain and seals (PLATFORM.md section 8 says only that `svc.audit` is append-only), all views.

## 7. Security and privacy

Capabilities requested:

| Capability | Reason |
|---|---|
| `fs.app` | Saved queries, bookmarks, scrub profiles |
| `fs.portal` | Open log files and bundles; save exports |
| `net.none` | The app never uses the network |
| `svc.logs` (Proposed) | Query, follow, verify, retention |
| `svc.audit` | Read the audit class (also served through `svc.logs`) |
| `svc.settings`, `svc.notify` | Preferences; saved-query alerts |
| `svc.keyring` | Recipient keys for encrypted export |
| `svc.portal.print`, `dev.print` | Print a selection or report |
| `proc.helper:decode-helper`, `proc.helper:symbol-helper` | Parse untrusted files and debug data outside the app host |
| `ai.local` (optional, off) | Natural-language query and range summary |

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| Evidence of what happened | An intruder with user or even service-level access | Edit or delete records to hide activity | Append-only dataset; hash chain; periodic seals; forward-secure key evolution; tombstones; optional TPM anchor and external witness; SE01 alert on verification failure |
| Private data inside logs (paths, hosts, names, tokens) | Other local users, other apps | Read someone else's logs | Per-app and per-session scoping in the service; admin approval for system scope; viewing under approval is audited; sensitive fields hidden |
| Private data inside logs | Recipient of an export, anyone it is forwarded to | Learn identity and environment | Scrub rules on by default, consistent pseudonyms, preview, leftover check, support-bundle manifest |
| The viewer itself | A hostile app or remote party that controls log content | Terminal-escape style or markup injection, huge lines, parser bugs via crafted messages | Messages are shown as plain text with control characters escaped; no markup interpretation; length caps; collectors and decoders in capability mode; fuzzed |
| The store's availability | A noisy or hostile app | Flood logs to evict audit records | Per-origin rate limits and per-class caps; audit class has its own budget and cannot be displaced by other classes |
| The audit stream | A compromised app host | Suppress its own denials | A stuck audit sink does not unblock a denial (FORGE-WP-11 test, FSS AUD-3); denials are recorded by the enforcing side, not by the app |
| Crash reports | Local snooper | Read memory contents of another app | No memory in reports by default; dumps off by default, encrypted and admin-only |
| Query engine | A user's own pathological regex | CPU exhaustion in a system service | Linear-time matcher, per-query time and memory limits, queries run at low priority |

Stored: see 6.2. Leaves the machine: nothing, unless the user exports a file and shares it.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `logs.view.time_format` | enum{local,utc,relative,since_boot} | `local` | Time column format |
| `logs.view.show_debug` | bool | `false` | Show debug severity |
| `logs.view.wrap` | bool | `false` | Wrap long messages |
| `logs.view.collapse_repeats` | bool | `true` | Collapse identical consecutive records |
| `logs.view.density_strip` | bool | `true` | Show the histogram |
| `logs.view.default_range` | enum{boot,1h,24h,7d} | `boot` | Range for new tabs |
| `logs.live.buffer` | u32 | `100000` | Records buffered while paused |
| `logs.live.max_rate` | u32 | `2000` | Records per second before sampling |
| `logs.query.timeout_ms` | u32 | `10000` | Per-query time limit |
| `logs.retention.system_days` / `_bytes` | u16 / u64 | `30` / `1 GiB` | System and service classes |
| `logs.retention.app_days` / `_bytes` | u16 / u64 | `14` / `512 MiB` | App class |
| `logs.retention.audit_days` / `_bytes` | u16 / u64 | `365` / `2 GiB` | Audit class |
| `logs.retention.audit_min_days` | u16 | `30` | Minimum before audit records may be cleared |
| `logs.retention.debug_days` | u16 | `3` | Debug-severity records |
| `logs.retention.crash_days` / `_per_app` | u16 / u16 | `90` / `50` | Crash reports |
| `logs.volatile` | bool | `false` | Keep non-audit logs in memory only |
| `logs.integrity.seal_minutes` | u16 | `60` | Seal interval |
| `logs.integrity.tpm_anchor` | bool | `false` | Anchor chain heads in the TPM |
| `logs.export.scrub_default` | set | all except `app` | Scrub rules ticked by default |
| `logs.export.format` | enum | `bundle` | Default export format |
| `logs.crash.core_dumps` | enum{off,per_app} | `off` | Core dump policy |
| `logs.audit.kernel_classes` | set | `{login, admin}` | BSM classes recorded |
| `logs.rate_limit.per_origin` | u32 | `1000` | Records per second accepted per origin |

## 9. Accessibility and languages

- Severity and decision are words in their own column, readable by a screen reader without colour or icon.
- Live tail does not flood speech: while following, the reader announces counts ("12 new records, 1 error") at a set interval; each record is read only when focused.
- The density strip and boot timeline have table equivalents; the strip's selected range is adjustable with arrow keys and announced as times.
- Log messages are not translated; the UI, field names' descriptions, crash plain-words summaries and decision words are.
- Bidirectional text: messages are shown left-to-right with isolation so mixed-direction content cannot reorder field boundaries (also a spoofing defence).
- Timestamps follow the locale; sub-second digits are always shown in tabular figures.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| First frame | Under 150 ms; first page of records under 300 ms |
| Scroll | 60 fps over any store size; page fetch under 30 ms p95 |
| Indexed query (text token over 30 days, 50 million records) | First results under 500 ms; full count under 3 s |
| Regex query | At least 200 MiB/s of message text per core |
| Ingestion | 50 000 records/s sustained; chain hashing under 1 µs per record on top of write cost |
| Live tail latency | Under 100 ms from append to display |
| Verify | At least 1 GiB/s of segment data |
| Store overhead | Index under 25 % of compressed segment size |
| App memory | Under 200 MiB with 10 tabs; no growth while following |
| Service idle | No wake-ups when nothing is logged |
| Largest opened file | 8 GiB compressed, streamed |

## 11. Testing and proof

Test plan (nothing has run):

- Unit: each source parser against recorded samples; query parser and planner; time-range arithmetic across clock steps and time zones; scrub rules with a labelled corpus (must-remove and must-keep cases); pseudonym stability; boot-span tree building.
- Golden renders: list in each density and theme, audit views, crash report, boot timeline, scrub preview; right-to-left UI with left-to-right messages.
- Scripted UI through the a11y tree: T1–T13.
- Integrity: flip, insert, delete, reorder and truncate records and segments in a copy of a store; the verifier must locate each change; power-cut during append and during sealing.
- Fuzz: syslog, RFC 5424, BSM, JSON lines, logfmt, bundle, segment and query parsers; regex compiler.
- Differential: BSM decoding against `praudit` output on the same trail; syslog parsing against what `syslogd` wrote; query results against a brute-force scan.
- Load: 100 million records; a flooding origin against rate limits; 20 concurrent followers.
- Privacy: export a seeded store with known secrets and identifiers; assert none survive with default rules.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Hash chain and verifier | Any change to a sealed range is detected; verifier is total | T1 | Lean model of the chain and seal scheme; PRISM refinement of the verifier | Hash and signature primitives assumed correct |
| Segment writer | Append-only: no code path rewrites a closed segment | T2 | PRISM invariant; dataset permissions as a second barrier | Service not written |
| Source parsers and decoders | No runtime error on any input | T2 | PRISM + fuzz | — |
| Regex matcher | Linear time and bounded memory | T1 (automaton construction), T2 (code) | Proof of the simulation bound; PRISM | — |
| Query planner | Result equals brute-force scan semantics | T3 | Differential tests | Proof effort not justified yet |
| Access scoping | A caller never receives a record outside its scope | T2 | PRISM contract on the query entry points; model of the scope rule | Depends on bus peer identity (G-08) |
| Scrubber | Every match of an enabled rule is replaced; mapping is injective per export | T2 (replacement), T3 (rule coverage) | PRISM on the replacer; corpus tests for rules | Rules are heuristics; no proof of "no private data remains" is possible |
| Rate limiter | Audit class budget cannot be consumed by other classes | T2 | PRISM invariant | — |
| UI escaping | Control characters never reach the renderer unescaped | T3 | Unit and fuzz tests on the display path | UI not in proof scope |
| syslogd, OpenBSM kernel side | — | T4 | Upstream tests | Foreign C code |

## 12. Build plan

Phase 1 — usable core:

| ID | Work item | Depends on |
|---|---|---|
| AD04-W01 | `svc.logs` schema: record, append, query, subscribe, verify | G-08, FORGE-WP-26 |
| AD04-W02 | Segment format v1 with hash chain, tombstones, retention by segment | AD04-W01 |
| AD04-W03 | Collectors: syslog, kernel messages, service and app output | G-07 (app host output), service supervisor (G-09) |
| AD04-W04 | `svc.audit` forwarding into the audit class | FORGE-WP-11, G-09 |
| AD04-W05 | Token index and basic query (time, severity, origin, text) with scoping | AD04-W02, SH03 (admin approval) |
| AD04-W06 | App shell: sidebar, tabs, list, inspector, filter chips, presets, saved queries | G-02, FUSE-WP-12, FUSE-WP-28, FUSE-WP-29, AD04-GAP-1, AD04-GAP-3 |
| AD04-W07 | Live tail with service-side filtering | AD04-W05 |
| AD04-W08 | Audit views: Decisions, First contacts, Approvals; links to AD05 | AD05 (URL scheme) |
| AD04-W09 | Crash list and report from app-host crash records; related logs | FORGE-WP-11.1 |
| AD04-W10 | Verify command and Integrity view (chain only) | AD04-W02 |
| AD04-W11 | Plain export (text, JSON lines, CSV) and copy | SH04 |

Phase 2 — full feature:

| ID | Work item | Depends on |
|---|---|---|
| AD04-W12 | Query language, builder, regex, aggregation, rate charts, density strip | FUSE-WP-46 |
| AD04-W13 | Seals, continuous verification, read-only closed segments, SE01 alert | SE01, AD02 (dataset setup) |
| AD04-W14 | Scrubber, preview, pseudonyms, support bundle, `.pbsdlog` bundle and import | decode helper, FORGE-WP-27, FORGE-WP-28 |
| AD04-W15 | BSM collector and decoder; device use; policy-change history; completeness state | kernel audit enabled in the PBSD kernel configuration |
| AD04-W16 | Boot timeline, slowest, compare, failed units, unclean shutdown | boot trace data from loader, kernel and supervisor; AD04-GAP-2 |
| AD04-W17 | Kernel panic summaries, symbolisation, grouping, mitigation kills | SY05 (debug files), symbol helper |
| AD04-W18 | Correlate, context, split view, bookmarks, highlight rules, opened plain files | AD04-W12 |
| AD04-W19 | Storage view, retention settings, clear with tombstones, sensitive fields, command-line client | AD04-W02 |
| AD04-W20 | Sources: firewall, updates, installer, storage, backup, hardware | SE02, SY05, AD01, AD02, AD03, VM05 |

Phase 3 — advanced:

| ID | Work item | Depends on |
|---|---|---|
| AD04-W21 | Forward-secure sealing, TPM anchoring, external witness | AD01-GAP-4 (TPM userland) |
| AD04-W22 | Request tracing, lane view, cause hints, incident folders | G-08 (request ids) |
| AD04-W23 | Message templates, alert on match, temporary verbosity, boot history | SE01 |
| AD04-W24 | File-access audit, kernel audit class chooser, core dumps, guest sources | VM01, VM02, VM03 |
| AD04-W25 | Natural-language query and range summary | G-15, FORGE-WP-29 |

## 13. Open questions and risks

1. **`svc.logs` is not in PLATFORM.md section 8.** Either `svc.audit` grows into the general log store or a second service is added. This spec assumes two services sharing one store format.
2. **Replace or keep `syslogd`.** Keeping it means two hops and text parsing; replacing it means owning a socket every legacy program writes to. Phase 1 keeps it.
3. **Kernel audit is not enabled or tested in this tree** as far as was read; the PBSD audit module is a scaffold. BSM-based features are Phase 2 at the earliest.
4. **Boot timing data sources** (loader timestamps, the kernel trace buffer being compiled in, supervisor timings) are not established.
5. **Tamper evidence is not tamper proof.** An attacker with the service's privileges can stop logging or truncate the unsealed tail. Seals, the TPM anchor and the external witness narrow the window; the UI must say "verified up to" a time, not "secure".
6. **Scrubbing is heuristic.** The preview and leftover check reduce risk; the export dialog must not promise anonymity.
7. **Log volume on small disks.** Caps are per class; the defaults total under 4 GiB.
8. **Who may read what** on a single-user machine: asking for admin approval to read system logs may feel heavy. The Balanced profile could allow system (not other users') logs to the administrator without a prompt.
9. **The 100-million-row list** (`AD04-GAP-1`) needs a windowed model the current `tableView` may not support.
