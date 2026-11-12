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
| AD04-F### | Log store service | A system service (`svc.logs`, Proposed) owns one append-only structured store. Apps never read `/var/log` files directly. | M | 1 |
| AD04-F### | Source: syslog | Messages from the base `syslogd` (Verified in tree: `hbsd/src/usr.sbin/syslogd/`, with Capsicum support files `syslogd_cap*.c` and RFC 5424 output mode per `syslogd.8`) forwarded to the store with facility, severity, host, tag, PID. | M | 1 |
| AD04-F### | Source: kernel messages | The kernel message buffer (as `dmesg` reads it, `hbsd/src/sbin/dmesg`), with monotonic timestamps mapped to wall time and boot id. | M | 1 |
| AD04-F### | Source: services | Standard output and error of every supervised service and helper, captured by the service supervisor with service name and restart count. | M | 1 |
| AD04-F### | Source: apps | Output of app hosts and their helpers, tagged with bundle id and instance; an app can read only its own stream without admin approval. | M | 1 |
| AD04-F### | Source: `svc.audit` | Grants, denials, revocations, first network contacts, prompts shown and answers. Record fields per FORGE-WP-11.4: time, bundle id, policy hash, control, decision, target. | M | 1 |
| AD04-F### | Source: kernel audit (BSM) | OpenBSM trail records (Verified in tree: `hbsd/src/contrib/openbsm`, `hbsd/src/sys/security/audit`; PBSD hooks scaffold `pbsd/kernel/audit/pbsd.kernel.audit.cppm`) decoded into the common model: event, subject, object, return. | M | 2 |
| AD04-F### | Source: crash reports | Kernel dumps summarised by `crashinfo` (Verified: `hbsd/src/usr.sbin/crashinfo`, `hbsd/src/sbin/savecore`) and user-space crash records written by the app host (FORGE-WP-11.1). | M | 1 |
| AD04-F### | Source: boot timeline | Loader, kernel and service start timestamps. Kernel side from the boot tracing buffer (Verified in tree: `hbsd/src/sys/kern/kern_tslog.c`, `sys/sys/tslog.h`; whether it is compiled in is not established). | M | 2 |
| AD04-F### | Source: firewall | pf log entries (blocks, per-app decisions) from SE02's logging path. | S | 2 |
| AD04-F### | Source: updates and installer | SY05 transactions and the AD01 install log. | S | 2 |
| AD04-F### | Source: storage and backup | Pool events and backup runs (from the AD02 and AD03 services). | S | 2 |
| AD04-F### | Source: authentication | Logins, lock and unlock, failed attempts, admin approvals (SH03). | M | 1 |
| AD04-F### | Source: hardware | Device attach and detach, USB policy decisions (VM05), thermal and power events. | S | 2 |
| AD04-F### | Source: jails, VMs, compatibility layers | Per-guest streams from VM01, VM02, VM03, each its own origin. | C | 3 |
| AD04-F### | Common record | Every source maps to one record shape (section 6.2) and keeps its raw form. | M | 1 |
| AD04-F### | Structured fields | Key-value fields are indexed; JSON and logfmt payloads are recognised and split. | S | 2 |
| AD04-F### | Rate limits | Per-origin ingestion limits with a "N messages dropped" marker record, so one noisy app cannot evict others. | M | 1 |
| AD04-F### | Time handling | Wall time (UTC stored, local shown), monotonic time and boot id on every record; clock steps are records themselves. | M | 1 |
| AD04-F### | Open plain files | Open a log file from the portal (text, gzip, zstd, xz) as a temporary source with format detection (syslog, RFC 5424, JSON lines, logfmt). | S | 2 |
| AD04-F### | Open BSM trail files | Open an audit trail file from the portal and view it decoded. | C | 3 |
| AD04-F### | Remote logs | Not collected. Receiving syslog from the network is off (matches the `bsdinstall` hardening option "disable syslogd network socket", `hbsd/src/usr.sbin/bsdinstall/scripts/hardening`). | M | 1 |

### 4.2 Viewing

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F### | Unified list | Virtualised table: time, severity, origin, message; newest at bottom; smooth over 100 million records. | M | 1 |
| AD04-F### | Source tree | Sidebar: All, System, Kernel, Services ▸ each, Apps ▸ each, Audit, Security, Crashes, Boot, Saved queries. Counts of new errors per node since last view. | M | 1 |
| AD04-F### | Severity display | Emergency to debug as icon + word + colour token; debug hidden by default. | M | 1 |
| AD04-F### | Detail inspector | All fields of the selected record, raw form, chain position and hash, related links (process, app, grant, unit). | M | 1 |
| AD04-F### | Multi-line and long messages | Collapsed to one row with expand; stack traces grouped. | M | 1 |
| AD04-F### | Wrap and columns | Toggle wrap; column chooser (PID, thread, host, boot, facility, any structured field); column order and width remembered per source. | M | 1 |
| AD04-F### | Time formats | Local, UTC, relative ("3 min ago"), since boot, delta to previous or to a marked row. | M | 1 |
| AD04-F### | Density strip | A histogram above the list showing record counts over time by severity; click or drag to jump and to select a range. | M | 2 |
| AD04-F### | Boot selector | Choose a boot (this boot, previous, by date); list of boots with duration and shutdown kind (clean, crash, power loss). | M | 1 |
| AD04-F### | Bookmarks and notes | Mark records, add a note; marks listed in a panel and included in exports. | S | 2 |
| AD04-F### | Highlight rules | Colour or badge rows matching a pattern; rules are per user. | S | 2 |
| AD04-F### | Collapse repeats | Consecutive identical messages collapse to one row with a count. | M | 1 |
| AD04-F### | Message templates | Group messages by template (variable parts replaced) to show "top message kinds" for a range. | S | 3 |
| AD04-F### | Context view | From a filtered result, show N records before and after in the unfiltered stream. | M | 1 |
| AD04-F### | Split view | Two panes over different sources or times with optional time lock. | S | 2 |
| AD04-F### | Raw view | Show the original bytes of a record or file (hex for non-text). | S | 2 |
| AD04-F### | Copy | Copy rows as text, JSON lines or Markdown table; "copy scrubbed" applies the scrub rules. | M | 1 |
| AD04-F### | Open related app | From a record: open the origin in SY04 (process), AD05 (app permissions), SE02 (connection), AD02 (device), SY05 (package). | M | 2 |

### 4.3 Search, filters, saved queries

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F### | Filter bar | Chips for time range, severity, origin, boot, plus free text; chips are added from the keyboard or by "filter by this value" on any cell. | M | 1 |
| AD04-F### | Quick time ranges | Last 15 min, hour, day, week, this boot, previous boot, custom from–to, "around this record ± N". | M | 1 |
| AD04-F### | Full-text search | Indexed substring and token search over messages across all sources. | M | 1 |
| AD04-F### | Regular expressions | Regex search (linear-time engine, no backtracking) on message or any field; a time limit per query. | M | 2 |
| AD04-F### | Query language | Text form: `severity>=warning origin:app:org.pbsd.mail "timeout" -field.host=local since:-2h`. Operators: `AND`, `OR`, `NOT`, parentheses, comparison on numeric fields, `~` regex. | M | 2 |
| AD04-F### | Query builder | Form equivalent of the query language; edits in one update the other. | S | 2 |
| AD04-F### | Autocomplete | Field names, known values (origins, units, event kinds) and recent queries. | S | 2 |
| AD04-F### | Saved queries | Name, query, columns, time rule; pinned to the sidebar; optional badge with the count of new matches. | M | 1 |
| AD04-F### | Built-in presets | Errors, Denials, First contacts, Failed logins, Admin approvals, Crashes, Service restarts, Kernel warnings, Storage problems, Updates. | M | 1 |
| AD04-F### | Aggregation | Count by field over a range (by origin, severity, event kind, host) as a table and bar chart. | S | 2 |
| AD04-F### | Rate over time | Chart of matches per interval for the current query. | S | 2 |
| AD04-F### | Find in results | `Ctrl+F` find next and previous within the current result without changing the filter. | M | 1 |
| AD04-F### | Query history | Last 100 queries per user, local, clearable. | S | 2 |
| AD04-F### | Share a query | Copy a query as text or as a `pbsd-logs:` link that opens it in this app on this machine. | S | 2 |
| AD04-F### | Alert on match | A saved query can raise a `svc.notify` notification when it matches (evaluated by `svc.logs`, at most once per interval). Security rules proper belong to SE01. | S | 3 |
| AD04-F### | Natural-language query | With `ai.local`: "errors from mail yesterday evening" is turned into a query the user sees and confirms before it runs. | C | 3 |

### 4.4 Live tail

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F### | Follow | Live mode appends new matches and keeps the view pinned to the end; scrolling up pauses; a "N new" pill resumes. | M | 1 |
| AD04-F### | Pause and buffer | While paused, up to 100 000 new records are buffered and counted. | M | 1 |
| AD04-F### | Live with filters | Any query can be followed; evaluation happens in the service so the app receives only matches. | M | 1 |
| AD04-F### | Rate guard | Above 2 000 records/s the view samples and says so; the store still has everything. | M | 2 |
| AD04-F### | Marker line | Insert a marker ("--- started test ---") into the view (and optionally into the store as a user record). | S | 2 |
| AD04-F### | Tail several origins | Tabs or split panes each following a different query. | S | 2 |
| AD04-F### | Temporary verbosity | Ask a service or app to raise its log level for N minutes (only those that offer the control; needs admin approval for system services); reverts by itself. | S | 3 |
| AD04-F### | Sound or flash on match | Optional cue when a followed query matches at error level. | C | 3 |

### 4.5 Audit trail

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F### | Audit view | Table: time, app, decision (granted, denied, revoked, prompted, expired), capability, resource, duration (once, session, always), who decided (user, policy, profile). | M | 1 |
| AD04-F### | Decision filters | One-click filters for each decision kind; denials grouped by app and capability with counts. | M | 1 |
| AD04-F### | First contacts | Table of network first contacts: time, app, host, port, route (clear, VPN, Tor), resolved address family, whether allowed; grouped by app or by host. | M | 1 |
| AD04-F### | Grant life view | For one grant: created, each use (sampled count per hour), narrowed, revoked; with the lineage id where the platform exposes it (SI-6, `docs/specs/CAPSICUM_LINEAGE.md`). | S | 2 |
| AD04-F### | Jump to Permissions | Every audit row links to the app or grant in AD05; "Revoke" itself is done there. | M | 1 |
| AD04-F### | Device use | Camera, microphone, screen, location, USB sessions with start, end and app. | M | 2 |
| AD04-F### | Admin approvals | Every privileged approval (who, what action, which service) from SH03. | M | 1 |
| AD04-F### | File-access audit (opt-in) | For chosen folders or datasets, record opens through the file portal and (when kernel audit is on) by path class; off by default; volume estimate shown. | S | 3 |
| AD04-F### | Kernel audit classes | Choose which BSM classes are recorded (login, exec, administrative, network, file write) with size estimates. | S | 3 |
| AD04-F### | Policy-change history | Every change to profiles, defaults and per-app policy with before and after. | M | 2 |
| AD04-F### | Audit summary | Per day or week: counts by decision, new apps seen, new hosts, most-denied capability; as a short report. | S | 2 |
| AD04-F### | Unusual-activity hints | Simple local statistics (first time an app used a capability, a burst of denials); labelled as hints; real detection is SE01. | C | 3 |
| AD04-F### | Audit completeness state | Shows whether the audit sink had gaps (service restarts, dropped records) for the selected range. | M | 2 |

### 4.6 Crash reports

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F### | Crash list | Time, program or app, kind (signal, script error, sandbox violation, kernel panic, watchdog), count of similar crashes. | M | 1 |
| AD04-F### | Local only | Reports are never sent. The view says so; there is no "send" button. Sharing is an explicit export. | M | 1 |
| AD04-F### | Report content | Signal or error, faulting thread, ForgeScript stack (from the app host report), native stack with module offsets, loaded bundle version and hash, capability set at the time, last 200 log lines of the app. No memory contents by default. | M | 1 |
| AD04-F### | Plain-words summary | A template-based sentence: "Mail closed because its helper `mime-helper` tried to use memory it did not own." | M | 2 |
| AD04-F### | Symbolisation | Resolve addresses with local debug files if installed (from SY05); done by a sandboxed helper. | S | 2 |
| AD04-F### | Grouping | Reports with the same top frames and bundle version are grouped; trend per version. | S | 2 |
| AD04-F### | Kernel panics | Panic message, backtrace and the tail of the kernel message buffer from the `crashinfo` text summary; the raw dump stays admin-only. | M | 2 |
| AD04-F### | Core dumps | Off by default. When enabled per app, dumps are stored encrypted, size-limited and deleted after N days; this app lists and deletes them. | S | 3 |
| AD04-F### | Open in tools | Hand a report (and dump, if any) to DV01 or DV07. | C | 3 |
| AD04-F### | Related logs | One click shows all sources ± 60 s around the crash for that process tree. | M | 1 |
| AD04-F### | Retention | Keep the newest 50 reports per app and 90 days; pinned reports are kept. | M | 1 |
| AD04-F### | Hardening-related kills | Processes killed by a mitigation (W^X violation, ASLR or PaX policy) are shown as their own kind with the rule named (SI-3). | S | 2 |

### 4.7 Boot timeline

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F### | Timeline view | Horizontal bars: firmware (if reported), loader, kernel init, each device probe group, root mount, each service start, login ready, session ready. | M | 2 |
| AD04-F### | Critical chain | Highlights the chain of starts that determined the total time. | S | 2 |
| AD04-F### | Slowest items | Sorted table of durations. | M | 2 |
| AD04-F### | Compare boots | Two boots side by side with deltas; new, removed and slower items marked. | S | 2 |
| AD04-F### | Boot history | Chart of total boot time across boots; marks for updates and boot environment changes. | S | 3 |
| AD04-F### | Failed and degraded units | Services that failed or timed out at boot with their log lines. | M | 2 |
| AD04-F### | Boot chain events | Verified-boot results: loader and kernel signature checks, veriexec state, TPM unseal outcome (data from SE01's attestation log). | S | 3 |
| AD04-F### | Shutdown timeline | The same for shutdown; what delayed power-off. | C | 3 |
| AD04-F### | Unclean shutdown detection | A boot with no shutdown record before it is flagged. | M | 2 |

### 4.8 Correlation

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F### | Correlate from a record | Builds a query for the same process tree, app instance, session, handle or grant id, and time window; opens it in a new tab. | M | 2 |
| AD04-F### | Common identifiers | Records carry `boot_id`, `session_id`, `app_instance`, `pid`, `unit`, and where relevant `grant_id` and `request_id` so joins are exact, not guessed. | M | 1 |
| AD04-F### | Request tracing | A bus request id propagated through services lets one user action be followed across the app, portals and services (needs bus support, G-08). | S | 3 |
| AD04-F### | Interleaved view | Selected origins merged in strict time order with an origin column and lane colouring. | M | 1 |
| AD04-F### | Lane view | Swim lanes per origin on a shared time axis; denials, crashes and restarts as markers. | S | 3 |
| AD04-F### | Cause hints | Rule-based links: service restart ← crash record; app exit ← denial 50 ms earlier; mount failure ← device detach. Shown as "possibly related". | S | 3 |
| AD04-F### | Incident folder | Collect records, crash reports, notes and a boot timeline into a named local incident that can be exported as one bundle. | S | 3 |
| AD04-F### | Local assistant summary | With `ai.local`: summarise the selected range; the prompt is built from scrubbed text and stays on the machine. | C | 3 |

### 4.9 Integrity

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F### | Hash chain | Each record stores `hash = BLAKE3(prev_hash ‖ canonical record)`; chains are per segment and segments are chained. | M | 1 |
| AD04-F### | Sealing | Every N minutes or M records the service signs the chain head with a key it holds (Ed25519); the seal is itself a record. | M | 2 |
| AD04-F### | Forward security | The sealing key is evolved one-way after each seal, so a later compromise cannot forge earlier seals. | S | 3 |
| AD04-F### | TPM anchoring | Optionally extend a PCR or NV counter with chain heads so truncation of the tail is detectable after reboot. | C | 3 |
| AD04-F### | Verify | Verify a range or the whole store: chain continuity, seal signatures, segment order, gaps. Result: verified, gaps found (where), tampered (which segment). | M | 1 |
| AD04-F### | Continuous check | The service verifies new segments on rotation and at boot; failure raises a SE01 alert. | M | 2 |
| AD04-F### | Tamper evidence in the list | Records in a failed segment carry a "tampered or damaged" badge; missing ranges show a gap row. | M | 1 |
| AD04-F### | Immutable storage | Segments live on a dataset where only the service can append; closed segments are set read-only and snapshotted. | M | 2 |
| AD04-F### | Deletion is recorded | Retention deletes whole segments only and leaves a tombstone record with the removed range and its head hash. | M | 1 |
| AD04-F### | External witness | Export the current chain head as a short string or QR to keep elsewhere; later verification can check against it. | S | 3 |
| AD04-F### | Export proofs | Exported bundles include the chain hashes and seals so a recipient can verify the excerpt's integrity relative to seal points. | S | 2 |

### 4.10 Export, scrubbing, import

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F### | Export selection or query | Formats: plain text, JSON lines, CSV, Markdown, and a bundle (`.pbsdlog`: zstd-compressed JSON lines + manifest + chain proofs). | M | 1 |
| AD04-F### | Scrub rules | Remove or replace: user and host names, home paths, IP and MAC addresses, e-mail addresses, serial numbers and hardware ids, UUIDs, URLs' query strings, tokens and keys (pattern and entropy based), geographic coordinates. | M | 2 |
| AD04-F### | Consistent pseudonyms | Each removed value becomes a stable placeholder (`<user-1>`, `<ip-3>`) within one export so the log still reads; the mapping stays local. | M | 2 |
| AD04-F### | Scrub preview | Side-by-side preview with every replacement highlighted and a count per rule; nothing is written until confirmed. | M | 2 |
| AD04-F### | Custom scrub terms | User-supplied words and patterns to remove; saved per user. | S | 2 |
| AD04-F### | Scrub by default | Export dialogs start with scrubbing on; turning it off shows what will be exposed. | M | 2 |
| AD04-F### | Support bundle | Guided export for a named app: its logs, crash reports, version and capability list, system version; scrubbed; a manifest lists every file. | S | 2 |
| AD04-F### | Share | Hand the exported file to the share sheet (SH04); this app never uploads. | M | 2 |
| AD04-F### | Encrypt export | Encrypt to a recipient key from SE04 (age-style) or with a passphrase. | S | 3 |
| AD04-F### | Import bundle | Open a `.pbsdlog` bundle read-only as a separate source; verify its proofs; never merged into the local store. | S | 2 |
| AD04-F### | Print | Print the selection or a report through the print portal. | C | 3 |
| AD04-F### | Redaction check | After scrubbing, a second pass scans for anything matching the rules and reports leftovers. | S | 2 |

### 4.11 Retention, privacy, settings

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD04-F### | Retention per class | Defaults: system and services 30 days or 1 GiB; apps 14 days or 512 MiB; audit 1 year or 2 GiB; crashes 90 days; debug level 3 days. | M | 1 |
| AD04-F### | Storage view | Size per class and per origin; projected days until the cap; top talkers. | M | 2 |
| AD04-F### | Per-app log privacy | An app sees only its own records. A user sees their own session's app and audit records. System, kernel and other users' records need admin approval. | M | 1 |
| AD04-F### | Sensitive-field marking | Sources can mark fields sensitive; these are hidden until revealed and always scrubbed on export. | S | 2 |
| AD04-F### | Clear logs | Clearing a class is an admin action, recorded with a tombstone; audit records cannot be cleared before their minimum retention (default 30 days) in the Strict profile. | M | 2 |
| AD04-F### | Persistent or volatile | Option to keep non-audit logs in memory only (lost at reboot) for high-privacy setups. | S | 3 |
| AD04-F### | Encrypted at rest | The store sits on the encrypted pool; with per-user encrypted homes, per-user app logs live inside the user's dataset. | M | 2 |
| AD04-F### | Read-only mode for viewers | Without admin approval the app works on the user's own records with a banner saying what is hidden. | M | 1 |
| AD04-F### | Viewing is audited | Reading system or other users' logs under admin approval is itself an audit record. | S | 2 |
| AD04-F### | Command line | `logs` command: query, follow, verify, export with the same query language. | S | 2 |
