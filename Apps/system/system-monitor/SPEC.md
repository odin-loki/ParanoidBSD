<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# System Monitor

By Odin Loch

| | |
|---|---|
| ID | `SY04` (from CATALOGUE.md) |
| Bundle | `org.pbsd.system-monitor` |
| Category | system |
| Priority | P0 |
| Backing repos | FUSE (`chart`, item views), FORGE (ForgeScript), ParanoidBSD (`docs/specs/HANDLES.md`, `docs/specs/CAPSICUM_LINEAGE.md`) |
| Status | Planned. Nothing built. |

## 1. Purpose

System Monitor shows what the machine is doing now and what it did recently: processes, processor, memory, disks, network, graphics, sensors, power and services. It is used to find what is slow, what is using the network, and what a process is allowed to touch. Unlike a classic task manager it groups processes by capability domain, jail and VM, and it shows each process's capabilities and open handles. It replaces Plasma System Monitor from the current plan (PLATFORM.md V20).

## 2. Scope

In scope:

- Live and historical views of processes and system resources.
- Per-process detail: capabilities, open handles, threads, memory map summary, network endpoints, limits.
- Actions: end, kill, suspend, resume, change priority, set limits, CPU affinity.
- Services and startup apps: state, start, stop, restart, enable, disable.
- Alerts on thresholds.
- Hand-off to tracing tools.

Out of scope:

| Item | Owner |
|---|---|
| Granting or revoking capabilities, the full lineage tree | AD05 Permissions (this app shows read-only state and links) |
| Firewall rules, blocking a connection for good | SE02 Firewall |
| Packet contents | DV05 Packet Analyser |
| Log browsing, crash reports, boot analysis | AD04 Logs |
| Disk health tests, SMART self-tests, pool repair | AD02 Disks |
| Space breakdown by folder | UT04 Disk Usage |
| Intrusion detection and security posture | SE01 Security Centre |
| VM and jail management | VM01, VM02 |
| Writing DTrace scripts, flame graphs as a full tool | DV01 IDE and SY03 Terminal (hand-off with a prepared script) |
| Power settings | SY01 Settings |
| Device tree and drivers | VM05 Devices |

## 3. Users and top tasks

Personas:

- **Everyday user**: "what is making the fan spin?", "end the app that froze".
- **Developer**: watches one process's memory, threads, files and sockets.
- **Administrator**: services, limits, I/O per dataset, ARC size, history over a week.
- **Security-minded user**: which process talks to which host; which handles a sandboxed app really holds.

| # | Task | Target steps |
|---|---|---|
| T1 | See what uses the most processor time | 1 (open; sorted by CPU) |
| T2 | End a frozen app | 2 (select, `Delete`) |
| T3 | Find a process by name | 2 (`Ctrl+F`, type) |
| T4 | See which process uses the network now | 1 (`Ctrl+6`) |
| T5 | See what a process may access (capabilities, handles) | 2 (select, `F4`) |
| T6 | See memory, swap and ARC use | 1 (`Ctrl+4`) |
| T7 | Look at processor use for the last 24 hours | 2 |
| T8 | Restart a service | 3 |
| T9 | Lower a process's priority | 3 |
| T10 | Suspend a process and resume it later | 2 + 2 |
| T11 | Set an alert when free memory is low | 4 |
| T12 | Find which process holds a file open | 2 (`Ctrl+Shift+F`, type path) |
| T13 | See all processes of one app, jail or VM as a group | 1 (group mode) |
| T14 | Start a trace of a process's system calls | 3 |

## 4. Features

### 4.1 Processes

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY04-F001 | Process table | One row per process. Columns (choosable): name, PID, parent, user, domain, jail, state, CPU %, CPU time, threads, memory (resident, private, shared, virtual), swap, disk read/s, write/s, network down/s, up/s, GPU %, GPU memory, energy impact, priority, nice, start time, command line, executable path, app id, sandbox state, hardening flags. | M | 1 |
| SY04-F002 | Tree mode | Parent and child tree with summed values on collapsed parents. | M | 1 |
| SY04-F003 | Group by app | Rows grouped by app bundle: the app host, its helpers and children under one row with totals. | M | 1 |
| SY04-F004 | Group by capability domain | Rows grouped by capability domain (the unit the kernel confines). Shows the domain's name, owner app, and whether it is in capability mode. | M | 1 |
| SY04-F005 | Group by jail and VM | Jails and bhyve VMs as top-level groups; guest processes are not listed for VMs, only the VM's host processes and its totals. | M | 2 |
| SY04-F006 | Group by user, by service | Further grouping choices. | S | 2 |
| SY04-F007 | Filters | My processes, all, apps only, background, system, in capability mode, not sandboxed, with network, by user, by state. Saved filter chips. | M | 1 |
| SY04-F008 | Search | `Ctrl+F` over name, PID, command line, path, app id, user, domain. | M | 1 |
| SY04-F009 | Sort and freeze | Sort by any column; "freeze order" (`Space`) stops rows jumping while reading; update rate 0.5, 1, 2, 5 s or paused. | M | 1 |
| SY04-F010 | Not-responding detection | Apps whose UI thread missed the compositor's ping are marked "not responding". | M | 2 |
| SY04-F011 | Sparkline columns | Optional tiny history graph per row for CPU and memory (last 60 samples). | S | 2 |
| SY04-F012 | Heat colouring | Cell background shade by value, with numbers always shown. | S | 1 |
| SY04-F013 | Sandbox state column | Words: "capability mode", "jailed", "VM", "unconfined"; unconfined user processes get a `--warn` mark. | M | 1 |
| SY04-F014 | Hardening flags column | Per process: ASLR, PAGEEXEC, MPROTECT, SEGVGUARD in force or excused (from the process's PaX flags). Read-only. | S | 2 |
| SY04-F015 | Threads view | Threads of a process: TID, name, state, CPU %, priority, core, wait channel. | S | 2 |
| SY04-F016 | Zombie and orphan marks | Marked states with an explanation tooltip. | S | 2 |
| SY04-F017 | Compare two processes | Select two; side-by-side of resources and capabilities. | C | 3 |
| SY04-F018 | Copy and export | Copy selected rows as text or CSV; export the table. | S | 1 |

### 4.2 Per-process detail

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY04-F019 | Overview tab | Identity, app, signer and bundle signature state, executable digest, start time, parent chain, working directory, environment (hidden until revealed; needs authentication for other users). | M | 1 |
| SY04-F020 | Capabilities tab | Manifest capabilities of the owning app, which are granted, which are in use now; ForgeScript `Caps` state; link "Open in Permissions". | M | 1 |
| SY04-F021 | Handles tab | Open handles and descriptors: number, kind (file, directory, socket, pipe, shared memory, device, process, kqueue, event), target, rights (Capsicum rights as words), lineage parent, granted by, age. Filter and search. | M | 2 |
| SY04-F022 | Rights detail | Selecting a handle lists every right bit as present or absent, and the path by which it was received (portal, manifest, inherited). | S | 2 |
| SY04-F023 | Files tab | Open files and memory-mapped files with mode and offset. | M | 2 |
| SY04-F024 | Network tab | Sockets: protocol, local and remote address and port, state, bytes, route class (clear, VPN, anonymous), resolved name as recorded at first contact; link to SE02 for a rule. | M | 2 |
| SY04-F025 | Memory tab | Resident, private, shared, swap, wired; map summary by kind (code, heap, stack, mapped file, shared); writable-and-executable mappings count (expected 0, shown in `--danger` otherwise). | M | 2 |
| SY04-F026 | Limits tab | Resource limits (rlimit) and resource-control rules (rctl) with current use; edit (F045). | S | 2 |
| SY04-F027 | Graphs tab | CPU, memory, disk, network history for this process since it started (kept while the app or sampler runs). | M | 1 |
| SY04-F028 | Children and IPC tab | Child processes, helpers, bus names owned, services it talks to. | S | 3 |
| SY04-F029 | Modules tab | Loaded shared objects with path, digest and signature state. | S | 3 |
| SY04-F030 | Audit tab | Recent `svc.audit` entries for this app: grants, denials, first network contacts. | S | 2 |
| SY04-F031 | Open location | Show the executable in SY02; open the app's page in SY05. | S | 1 |
| SY04-F032 | Who holds this file | `Ctrl+Shift+F`: type or drop a path; lists processes with a handle on it or below it. | S | 2 |
| SY04-F033 | Who listens | List of listening sockets system-wide with owner process and exposure (loopback, local network, any). | M | 2 |

### 4.3 Actions on processes

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY04-F034 | End (ask to quit) | Sends the app a close request through the session bus, then TERM after 5 s if still present. | M | 1 |
| SY04-F035 | Force kill | KILL after a confirm that names the process and warns about unsaved data; no confirm setting for power users. | M | 1 |
| SY04-F036 | End group | End or kill a whole app, domain, tree or jail group. | M | 1 |
| SY04-F037 | Suspend and resume | STOP and CONT; suspended rows are dimmed and marked. | M | 1 |
| SY04-F038 | Send signal | Any signal from a menu. | S | 2 |
| SY04-F039 | Priority | Nice value −20…20 with a slider; real-time and idle classes (authenticated). | M | 1 |
| SY04-F040 | CPU affinity | Core check boxes; shows efficiency and performance cores where known. | S | 2 |
| SY04-F041 | Privilege for actions | Own processes need no prompt. Other users' or system processes go through the control broker with authentication; each is audited. | M | 1 |
| SY04-F042 | Protected processes | Compositor, session bus, init and security services are marked protected; kill is refused with the reason, restart is offered where the service manager supports it. | M | 1 |
| SY04-F043 | Restart app | End then start again through the shell. | S | 2 |
| SY04-F044 | Undo for suspend and priority | Toast with Undo restores the previous state. | S | 1 |
| SY04-F045 | Set limits | Add or change rctl rules for a process, app or jail: memory, CPU percentage, processes, open files, I/O rate; action (deny, throttle, signal, log). Rules can be temporary or saved for the app. | S | 3 |
| SY04-F046 | Revoke network now | One click to cut a process's sockets through SE02's interface (temporary block), with Undo. | S | 3 |
| SY04-F047 | Dump for debugging | Request a core dump of an own process to a chosen file (hands to DV01). | C | 3 |

### 4.4 Processor

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY04-F048 | Total and per-core graphs | Stacked user, system, interrupt, nice, idle; per-core grid or bars; hybrid cores labelled. | M | 1 |
| SY04-F049 | Frequency and temperature | Per-core frequency, package temperature, throttling state. | M | 2 |
| SY04-F050 | Load and scheduler | Load averages, run queue, context switches, interrupts per second; top interrupt sources. | S | 2 |
| SY04-F051 | Processor information | Model, cores, threads, caches, features, microcode version, vulnerability mitigations in force (read-only). | S | 2 |
| SY04-F052 | Top consumers strip | The five heaviest processes under each resource graph, clickable. | M | 1 |

### 4.5 Memory, swap and ZFS ARC

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY04-F053 | Memory bar and graph | Active, inactive, laundry, wired, free, with ARC drawn as its own segment inside wired. Plain summary: "in use by apps", "cache (given back when needed)", "free". | M | 1 |
| SY04-F054 | ARC detail | ARC size, target, min, max, MRU and MFU sizes, hit ratio, metadata share, L2ARC size and hit ratio, compressed ratio. | M | 2 |
| SY04-F055 | Swap | Devices, size, used, encrypted state, page-in and page-out rates. | M | 1 |
| SY04-F056 | Memory pressure | A pressure gauge from paging activity and free target; history. | S | 2 |
| SY04-F057 | Kernel memory | Top UMA zones and malloc types (administrator view). | C | 3 |
| SY04-F058 | Per-app memory ranking | Apps ranked by private memory with helpers included. | M | 1 |

### 4.6 Disks and I/O

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY04-F059 | Per-device I/O | Read and write bytes/s and operations/s, queue depth, latency, busy %, per disk and per partition. | M | 1 |
| SY04-F060 | Per-pool and per-vdev | ZFS pool and vdev throughput, latency histograms, scrub and resilver progress, pool state word. | M | 2 |
| SY04-F061 | Per-dataset I/O and space | Datasets ranked by I/O; used, available, snapshot space; link to AD02 and UT04. | S | 2 |
| SY04-F062 | Per-process I/O | Read and write rates and totals per process. | M | 1 |
| SY04-F063 | Device health summary | SMART overall state and temperature, read-only; "Open in Disks". | S | 2 |
| SY04-F064 | File system events rate | Sync writes, TRIM activity, transaction group times. | C | 3 |

### 4.7 Network

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY04-F065 | Per-interface graphs | Down and up rates, totals, errors, drops; link speed; address summary. | M | 1 |
| SY04-F066 | Per-process network | Rate and total per process and per app, from kernel socket accounting. | M | 1 |
| SY04-F067 | Connections table | All sockets: process, protocol, local, remote, state, rate, age, route class. Remote host names come only from the system's first-contact log; no reverse lookups are made. | M | 2 |
| SY04-F068 | By route class | Totals for clear, VPN and anonymous routes. | S | 2 |
| SY04-F069 | By remote endpoint | Group connections by remote address or name; which apps talk to it. | S | 2 |
| SY04-F070 | Firewall counters | Passed and blocked counts from SE02, read-only; link. | S | 2 |
| SY04-F071 | Wireless detail | Signal, noise, rate, channel for Wi-Fi links. | C | 2 |
| SY04-F072 | Capture hand-off | "Capture this process's traffic…" opens DV05 with a filter. | C | 3 |

### 4.8 Graphics, sensors and power

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY04-F073 | GPU use | Per GPU: use %, memory, temperature, power, fan, video encode and decode use, where the driver reports them. | S | 2 |
| SY04-F074 | Per-process GPU | GPU time and memory per process; marks processes holding `dev.gpu.compute`. | S | 2 |
| SY04-F075 | Sensors | Temperatures, fan speeds, voltages, with min, max and limits; grouped by chip. | M | 2 |
| SY04-F076 | Power draw | Battery charge, rate (W), time left, health; package power from the processor's energy counters where present. | M | 2 |
| SY04-F077 | Energy impact | Per-app estimate from CPU time, wake-ups, GPU and network use; ranking over the last hour and day. | S | 2 |
| SY04-F078 | Wake-ups | Timer wake-ups per second per process; idle apps with wake-ups are flagged (the platform bar is zero). | S | 2 |
| SY04-F079 | Sleep inhibitors | Who is holding `svc.power` inhibits. | S | 2 |

### 4.9 Services and startup

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY04-F080 | Services list | System services and session services: name, description, state (running, stopped, failed, starting), enabled at boot, PID, memory, CPU, restarts, last exit, domain. | M | 2 |
| SY04-F081 | Service actions | Start, stop, restart, reload, enable, disable through the service manager interface, authenticated and audited. | M | 2 |
| SY04-F082 | Service detail | Dependencies, command, user, capability set, recent log lines (link to AD04). | S | 2 |
| SY04-F083 | Failed services banner | A count on the tab and one line per failure with "Show log". | M | 2 |
| SY04-F084 | Startup apps | Apps with `bg.autostart`: enabled, start cost (time, memory measured at last login), switch off. Same data as SY01's page. | M | 2 |
| SY04-F085 | Background apps | Apps running with `bg.run` and no window; end them or revoke in AD05. | M | 2 |
| SY04-F086 | Scheduled jobs | Read-only list of timers and periodic jobs with next run. | C | 3 |
| SY04-F087 | Boot time breakdown | Read-only summary of last boot stages; detail belongs to AD04. | C | 3 |

### 4.10 History, alerts and overview

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY04-F088 | Overview dashboard | Tiles for CPU, memory, disk, network, GPU, temperature, power, each with a graph and top consumer; alert strip. | M | 1 |
| SY04-F089 | History store | A sampler service records system series at 5 s (kept 2 h), 1 min (7 days) and 15 min (1 year), and per-app series at 1 min (7 days). Fixed size ring files. Optional. | M | 2 |
| SY04-F090 | History browser | Zoomable time axis; pick a range; the process table shows the top consumers in that range. | M | 2 |
| SY04-F091 | Annotations | Marks on graphs for boots, suspends, updates, app installs, alerts. | S | 3 |
| SY04-F092 | Alerts | Rules: metric, comparison, threshold, duration, scope (system, app, process name), action (notify, log, run a signed ForgeScript action). Presets: memory low, disk nearly full, temperature high, service failed, process over N % CPU for M minutes. | M | 2 |
| SY04-F093 | Alert log | Past alerts with the graph at that time. | S | 2 |
| SY04-F094 | Panel indicator data | Offers current values to SH01/SH02 for a small panel graph. | S | 2 |
| SY04-F095 | Mini mode | A small always-on-top window with chosen graphs. | C | 3 |
| SY04-F096 | Export | Export a time range as CSV or JSON; export a graph as PNG or SVG. | S | 2 |
| SY04-F097 | Report | A text snapshot of the system state for bug reports, with a redaction step (hostnames, user names, paths). | S | 3 |
| SY04-F098 | Local explanation | Optional: "Explain what is using my machine" sends the current summary table (no command lines unless allowed) to `svc.model`. Off by default. | C | 3 |

### 4.11 Tracing hand-off and housekeeping

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY04-F099 | Probe hand-off | "Trace…" on a process offers prepared DTrace scripts: system calls by count, file opens, network sends, on-CPU stacks, I/O latency. It opens SY03 with the script shown and the command ready, in an administrator shell. Nothing runs until the user starts it. | S | 3 |
| SY04-F100 | Script library | The prepared scripts are plain files in the bundle; "Open in IDE" hands to DV01. eBPF is not offered: the platform's tracer is DTrace. | S | 3 |
| SY04-F101 | Tracing policy awareness | States plainly when tracing is unavailable (for example `hardening.prohibit_ptrace_syscall` set, or DTrace not loaded and `forbid_kmod` in force). | S | 3 |
| SY04-F102 | Low overhead rule | The app and its sampler together stay under 1% of one core at the default rate; the detail pane samples only the selected process. | M | 1 |
| SY04-F103 | Pause when hidden | No sampling for the UI when the window is not visible; the sampler service continues only if history is on. | M | 1 |
| SY04-F104 | Remote host view | Show another PBSD machine's sampler over SSH port forward (read-only). | C | 3 |
| SY04-F105 | Command palette | All actions, tabs, saved filters; "kill <name>" style commands with confirmation. | M | 1 |
| SY04-F106 | Units and formats | Binary or decimal units, per-core or whole-machine percentage (100% = one core or all), from preferences. | M | 1 |
| SY04-F107 | Session restore | Tab, sort, columns, filters, selected ranges restored. | S | 1 |
| SY04-F108 | Emergency open | A global shortcut (`Ctrl+Shift+Esc`, set in SY01) opens the app at high priority with a minimal view when the system is under pressure. | S | 2 |

## 5. GUI

### 5.1 Window map

```
System Monitor window (single instance, one main window + optional mini window)
├─ Header bar: menu, title, search, update-rate control, window buttons
├─ Tab bar: Overview · Processes · Processor · Memory · Disks · Network · Graphics
│           · Sensors and power · Services · History · Alerts
├─ Content (per tab)
│   ├─ Overview: tile grid + alert strip
│   ├─ Processes: toolbar (group mode, filter chips) + table/tree + summary footer
│   ├─ Resource tabs: graphs on top, detail table below, top-consumers strip
│   ├─ Services: list + detail
│   ├─ History: time axis + stacked graphs + top consumers for the range
│   └─ Alerts: rules list + alert log
├─ Inspector (F4): process detail tabs — Overview · Capabilities · Handles · Files · Network
│                  · Memory · Limits · Graphs · Children · Modules · Audit
├─ Status bar: totals (processes, threads, CPU, memory, network), sampler state
├─ Dialogs: End process · Force kill · Change priority · CPU affinity · Set limits
│           · New alert rule · Who holds this file · Send signal · Trace hand-off
│           · Export range · Columns
├─ Popovers: Handle rights · Graph value at cursor · Filter editor
└─ Mini window
```

### 5.2 Main window

Shown at 1280×760, Processes tab, grouped by capability domain, inspector open on Handles.

```
+------------------------------------------------------------------------------------------+
| [=] System Monitor            [ Find process…  Ctrl+F ]   Update [1 s v] [⏸]  [▁] [▢] [✕] | (1)
+------------------------------------------------------------------------------------------+
| [Overview][Processes][Processor][Memory][Disks][Network][Graphics][Sensors][Services] …  | (2)
+-------------------------------------------------------------+----------------------------+
| Group [Domain v]  [Mine ×] [With network ×] [+ filter]      | Document Viewer (pid 5120) | (3)
|-------------------------------------------------------------| [Ovw][Caps][Handles][Net]… |
| Name                 | PID  | CPU % | Memory | Net ↓  |Sandbox|---------------------------|
| v Web Browser (domain)|     | 14.2  | 1.2 GiB| 310 kB |capmode| 9 handles    [ filter… ]  | (5)
|     browser          | 4410 |  3.1  | 310 MiB|   0    |capmode| # Kind  Target      Rights|
|     site: example.org| 4415 | 10.8  | 620 MiB| 310 kB |capmode| 3 file  invoice.pdf  r    |
|     net-helper       | 4412 |  0.3  |  40 MiB| 310 kB |capmode| 4 dir   ~/.cache/…   r w  |
| > Document Viewer    | 5120 |  0.4  | 180 MiB|   0    |capmode| 5 sock  session bus  rw   |
| v Terminal (domain)  |      |  2.0  |  96 MiB|  12 kB |mixed  | 6 shm   frame buffer r w  |
|     terminal         | 4801 |  0.5  |  60 MiB|   0    |capmode| 7 pipe  pdf-helper   w    |
|     zsh              | 4820 |  0.0  |   6 MiB|   0    |scoped | …                         |
|     make  ⏸ suspended| 4990 |  0.0  |  30 MiB|   0    |scoped |---------------------------|
|   old-tool         ⚠ | 5301 | 22.0  |  12 MiB|  80 kB |none ⚠ | Handle 3: file, read only | (6)
|                                                             | Received from: file portal |
|                                                             | Lineage: portal → app → 3  |
|-------------------------------------------------------------| Rights: read ✓ seek ✓      |
| CPU ▂▃▅▃▂▂▃  38 %    Memory ▅▅▅▅▅▆▆  9.1 / 32 GiB           |   write ✕ mmap ✕ fstat ✓   | (7)
+-------------------------------------------------------------+ [Open in Permissions]      |
| 212 processes · 1,480 threads · CPU 38 % · Mem 28 % · ↓ 402 kB/s ↑ 31 kB/s · history on  | (8)
+------------------------------------------------------------------------------------------+
```

1. Header bar: search, update-rate drop-down and pause. No `[net]` badge: the app has `net.none`.
2. Tab bar (`tabBar`); tabs with a problem carry a mark (for example a failed service count).
3. Toolbar: group mode (None, Tree, App, Domain, Jail and VM, User, Service) and filter chips.
4. Process table (`treeView` in table mode). Group rows sum their children. `⚠` marks an unconfined user process. `⏸` marks a suspended one.
5. Inspector, 300 px, `tabWidget`. Handles tab lists every handle with kind, target and rights.
6. Handle detail: where it came from, its lineage chain and each right as present or absent.
7. Summary footer with small graphs; click opens the resource tab.
8. Status bar totals and sampler state ("history on / off").

States: loading (first sample; table appears within 200 ms), sampler unavailable (banner: live data only, no history), restricted (other users' process details show "needs authentication" with [Unlock]), paused (header shows "Paused" in `--warn`), process gone (inspector shows "This process has ended" with its last values).

### 5.3 Other views and dialogs

#### 5.3.1 Overview dashboard

```
+------------------------------------------------------------------------------------------+
| ⚠ 1 alert: "Memory low" fired 10:42 — Web Browser 6.1 GiB                    [Show][Mute]|
+----------------------------+----------------------------+--------------------------------+
| Processor            38 %  | Memory        9.1 / 32 GiB | Disk  zroot      ↓ 12 ↑ 3 MB/s |
| ▁▂▃▅▇▅▃▂▂▃▅▃▂▁▂▃▅▇▅▃       | ▅▅▅▅▅▅▆▆▆▆▆▆▆▆▆▆           | ▁▁▂▇▂▁▁▁▂▁▁▁▃▁▁                |
| 16 cores · 52 °C           | apps 6.0 · cache 3.1 (ARC) | busy 14 % · 412 GiB free       |
| Top: make 22 %             | Top: Web Browser 1.2 GiB   | Top: backup 9 MB/s             |
+----------------------------+----------------------------+--------------------------------+
| Network   ↓ 402 ↑ 31 kB/s  | Graphics            12 %   | Power    −14 W · 3 h 20 left   |
| ▁▁▂▃▂▁▁▂▅▂▁▁               | ▁▁▁▂▁▁▁▁▂▁▁                | ▇▇▇▆▆▆▆▅▅▅▅▅                   |
| wlan0 · VPN off            | 1.1 / 8 GiB · 48 °C        | Top: Web Browser (high)        |
| Top: Web Browser           | Top: compositor            |                                |
+----------------------------+----------------------------+--------------------------------+
| Services: 61 running · 0 failed      Startup apps: 4      Background apps: 3             |
+------------------------------------------------------------------------------------------+
```

Tiles are `panel`s with a `chart` each, in a responsive grid (3, 2 or 1 columns). Each tile is a button that opens its tab. Tiles can be reordered and hidden.

#### 5.3.2 Memory tab

```
+------------------------------------------------------------------------------------------+
| Memory  32 GiB                                                     Range [5 min v]       |
| [███████ apps 6.0 ███░░░ cache: ARC 3.1 ░░ wired 1.4 ░ inactive 2.2 ·········· free 19.3]|
| ▅▅▅▅▅▅▅▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▆▇▇▇▇▇▇▇▇▇▇▇▇▇   (stacked area)                           |
+---------------------------------+--------------------------------------------------------+
| ZFS ARC                         | Swap                                                   |
|  Size      3.1 GiB of max 16    |  ada0p3.eli  8 GiB · used 0 · encrypted ✓              |
|  Target    3.4 GiB              |  Page in 0/s · out 0/s                                 |
|  MRU / MFU 1.2 / 1.7 GiB        | Pressure                                               |
|  Hit ratio 97.4 %               |  [▂ low ───────────────]                               |
|  L2ARC     none                 |                                                        |
+---------------------------------+--------------------------------------------------------+
| Top by private memory: Web Browser 1.2 GiB · IDE 0.9 GiB · Assistant 0.8 GiB · …         |
+------------------------------------------------------------------------------------------+
```

The segmented bar always has a text legend under it. "Cache" is explained in a tooltip: "given back when apps need it".

#### 5.3.3 Network tab, connections

```
+------------------------------------------------------------------------------------------+
| Network   wlan0 ↓ 402 kB/s ↑ 31 kB/s     [Interfaces][Processes][Connections][Listening] |
| ▁▁▂▃▂▁▁▂▅▂▁▁▂▃▅▃▂▁                                                    Range [5 min v]    |
+------------------------------------------------------------------------------------------+
| Process        | Proto | Remote                  | Port | Route | ↓ kB/s | ↑ kB/s | Age  |
| Web Browser    | TCP   | example.org             | 443  | clear |  310   |   12   | 2 m  |
| Web Browser    | UDP   | 2001:db8::1             | 443  | clear |   80   |    9   | 40 s |
| Mail           | TCP   | imap.example.net        | 993  | vpn   |   12   |    1   | 1 h  |
| > old-tool   ⚠ | TCP   | 203.0.113.7 (no name)   | 8080 | clear |    0   |   80   | 9 m  |
+------------------------------------------------------------------------------------------+
| old-tool → 203.0.113.7:8080 · first contact 10:33 · 43 MB sent                           |
| [Show process] [Open in Firewall…] [Cut connection (Undo for 30 s)] [Capture in Analyser]|
+------------------------------------------------------------------------------------------+
```

Names are those recorded by the system at first contact; an address with no recorded name is shown as an address. The app does no lookups.

#### 5.3.4 Services tab

```
+------------------------------------------------------------------------------------------+
| Services   [System v]   [ filter… ]     61 running · 0 failed · 12 stopped               |
+----------------------------------------------+-------------------------------------------+
| Name              | State    | Boot | Memory | search-index                              |
|   settings-store  | running  | on   | 12 MiB | Indexes files for system search           |
|   session-bus     | running🔒| on   |  8 MiB | State    running since 08:02 (pid 912)    |
| > search-index    | running  | on   | 96 MiB | Restarts 0      Last exit —               |
|   print-spooler   | stopped  | off  |   —    | Domain   capability mode, 6 handles       |
|   model-service   | running  | on   | 1.9 GiB| Needs    settings-store, session-bus      |
|                                              | CPU ▁▁▂▁▁▁   Memory ▅▅▅▅▅▅                |
|                                              | Last log lines                            |
|                                              |  10:40 indexed 212 files                  |
|                                              | [Restart] [Stop] [Disable at boot] [Log…] |
+----------------------------------------------+-------------------------------------------+
```

A lock marks protected services that cannot be stopped from here.

#### 5.3.5 History browser

```
+------------------------------------------------------------------------------------------+
| History    [2 h][24 h][7 d][30 d][1 y]      ◀ 11 Nov 00:00 — 12 Nov 00:00 ▶    [Export…] |
+------------------------------------------------------------------------------------------+
| CPU   ▁▁▁▂▂▃▇▇▇▅▃▂▁▁▁▁▂▃▃▂▁▁▁▁   ▲ update 14:02        ▲ suspend 23:10                   |
| Mem   ▃▃▃▃▄▄▆▆▆▆▅▄▄▄▄▄▄▄▄▄▃▃▃▃                                                           |
| Disk  ▁▁▁▁▁▁▂▂▁▁▁▁▁▁▇▁▁▁▁▁▁▁▁▁                                                           |
| Net   ▁▁▂▂▁▁▅▅▅▂▁▁▁▁▁▁▁▂▁▁▁▁▁▁                                                           |
|             [======]  selected 09:10 — 10:40                                             |
+------------------------------------------------------------------------------------------+
| Top in selection:  make 61 % CPU avg · Web Browser 1.4 GiB peak · backup 2.1 GB written  |
+------------------------------------------------------------------------------------------+
```

`chart` widgets share one time axis; drag selects a range; the wheel zooms; annotations are triangles with tooltips.

#### 5.3.6 End and force-kill dialogs

```
+---------------------- Force "Writer" to quit? ----------------------+
| Writer (pid 6021) is not responding.                                |
| Unsaved changes in 1 document may be lost. Autosave: 2 minutes ago. |
| [x] Also end its 2 helper processes                                 |
|                              [Cancel]  [Wait]  [Force quit]         |
+---------------------------------------------------------------------+
```

"End" (ask to quit) has no dialog. Force quit confirms once; the button is outlined `--danger` and is not the default.

#### 5.3.7 New alert rule

```
+------------------------------ New alert ------------------------------+
| When     [Free memory        v]  [is below v]  [ 1.0 ] [GiB v]        |
| For      [ 2 ] minutes                                                |
| Scope    (•) System   ( ) App [________ v]   ( ) Process name [____]  |
| Then     [x] Notify   [x] Log   [ ] Run action [choose… ]             |
| Repeat   not more than once every [ 30 ] minutes                      |
| Preview: would have fired 2 times in the last 7 days                  |
|                                          [Cancel]  [Create alert]     |
+-----------------------------------------------------------------------+
```

The preview line is computed from the history store.

#### 5.3.8 Set limits dialog

```
+----------------------- Limits for "old-tool" (pid 5301) -----------------------+
| Resource        | Now      | Limit        | When reached                       |
| Memory          | 12 MiB   | [ 512 ] MiB  | [deny v]                           |
| CPU             | 22 %     | [ 50  ] %    | [throttle v]                       |
| Processes       | 1        | [ 16  ]      | [deny v]                           |
| Open files      | 14       | [ 256 ]      | [deny v]                           |
| Disk write rate | 0        | [ none ]     |                                    |
| Apply to  (•) this process  ( ) this app from now on                           |
| Needs authentication.                         [Cancel]  [Apply limits]         |
+--------------------------------------------------------------------------------+
```

### 5.4 Menus

```
[=] menu / F10
├─ File
│   ├─ Export table…                     Ctrl+E
│   ├─ Export time range…                Ctrl+Shift+E
│   ├─ Save system report…
│   ├─ Mini window                       Ctrl+M
│   └─ Quit                              Ctrl+Q
├─ Edit
│   ├─ Copy rows                         Ctrl+C
│   ├─ Find process                      Ctrl+F
│   ├─ Who holds this file…              Ctrl+Shift+F
│   ├─ Select all                        Ctrl+A
│   └─ Preferences                       Ctrl+,
├─ View
│   ├─ Overview … Alerts                 Ctrl+1 … Ctrl+9, Ctrl+Shift+H (History), Ctrl+Shift+A (Alerts)
│   ├─ Group by ▸ None / Tree / App / Domain / Jail and VM / User / Service   Alt+1 … Alt+7
│   ├─ Columns…                          Ctrl+Shift+C
│   ├─ Freeze order                      Space
│   ├─ Pause updates                     Ctrl+Shift+P
│   ├─ Update rate ▸ 0.5 s / 1 s / 2 s / 5 s
│   ├─ Expand all / Collapse all         Ctrl+Shift+Right / Ctrl+Shift+Left
│   ├─ Inspector                         F4
│   └─ Refresh now                       F5
├─ Process
│   ├─ End (ask to quit)                 Delete
│   ├─ Force quit…                       Shift+Delete
│   ├─ End group…
│   ├─ Suspend / Resume                  Ctrl+.
│   ├─ Priority…                         Ctrl+Shift+N
│   ├─ CPU affinity…
│   ├─ Set limits…
│   ├─ Send signal ▸
│   ├─ Restart app
│   ├─ Trace… (hand-off)                 Ctrl+T
│   ├─ Show in Files · Show in Software · Open in Permissions
│   └─ Properties                        Alt+Enter
├─ Service
│   ├─ Start / Stop / Restart / Reload
│   ├─ Enable at boot / Disable at boot
│   └─ Show log
├─ Alerts
│   ├─ New alert…                        Ctrl+Shift+L
│   ├─ Mute all for 1 hour
│   └─ Alert log
└─ Help
    ├─ System Monitor help               F1
    ├─ What the columns mean
    ├─ Keyboard shortcuts
    └─ About System Monitor
```

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| `Delete` / `Shift+Delete` | End / force quit (matches the reserved trash and delete keys) |
| `Ctrl+.` | Suspend or resume the selection (the menu's suspend item; `Ctrl+Shift+S` is reserved for save as and is not used) |
| `Space` | Freeze row order |
| `+` / `-` on a tree row | Expand, collapse |
| `Ctrl+Shift+Esc` (global, set in SY01) | Open System Monitor in emergency mode |
| Wheel on a graph | Zoom time; drag to select a range; double click resets |
| Hover on a graph | Value at cursor in a popover; arrow keys move the cursor when the graph has focus |

Context menu on a process row: End · Force quit… · Suspend/Resume · Priority ▸ (Very high, High, Normal, Low, Very low, Custom…) · CPU affinity… · Set limits… · Send signal ▸ · Trace… · Show handles · Show connections · Open in Permissions · Show in Files · Copy ▸ (PID, name, command line, row). On a column header: sort, hide, Columns…, show as (number, bar, sparkline). On a connection: Show process · Copy address · Open in Firewall… · Cut connection. On a graph: Range ▸ · Copy image · Export data….

Drag and drop: drop a file or folder on the window to run "Who holds this file"; drag a process row to the Alerts tab to start a rule for it; drag tiles to reorder the overview.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Filters that match nothing: "No processes match" + [Clear filters]. No GPU or no sensors: the tab says so and is hidden by default. |
| Loading | Tables fill within one sample; graphs draw as data arrives; history shows "loading range". |
| Error | Sampler or broker not reachable: banner with reason; live local data still shown where the app can read it. |
| Offline | Not applicable; the app never uses the network. |
| Locked | Other users' details and all privileged actions need authentication; shown as lock icons with [Unlock]. |
| Read-only | Guest session or policy: actions hidden, views work. |
| Under pressure | Emergency mode: process table only, 2 s rate, no graphs, large End button. |
| Narrow (< 640) | Tab bar becomes a drop-down; table shows name, CPU, memory; inspector as a sheet; overview one column. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Tabs | `tabBar`, `tabWidget` |
| Process table and tree | `treeView` in table mode (FUSE-WP-28 `treeView2`) |
| Other tables | `tableView` |
| Graphs | `chart` |
| Memory bar, gauges | `progressBar` (segmented variant), `panel` |
| Inspector | `tabWidget`, `propertyGrid`, `tableView` |
| Overview | `gridView` of `panel` tiles |
| Dialogs | `dialog`, `slider`, `spinBox`, `comboBox`, `checkBox` |
| Help text | `markdownView` |

Missing:

- `SY04-GAP-1` `chart` needs streaming time-series mode: ring-buffer source, stacked areas, shared time axis across charts, range selection, annotations, value-at-cursor, zero allocation per sample. Whether `chart` has these is Unverified (only its name was read in PLATFORM.md V3).
- `SY04-GAP-2` Sparkline and bar cell delegates for table cells.
- `SY04-GAP-3` Segmented bar with legend.
- `SY04-GAP-4` Per-core heat grid (many small cells; could be one custom-drawn `panel`).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Monitor app | ForgeScript | app host | Tabs, view models, grouping, filters, alerts editor, actions |
| UI documents | FUSE UI | app host | `.fuseui` for tabs, inspector, dialogs |
| Sampler service | C++23 | system service, read-only kernel access, Capsicum after start | Reads process table, per-process accounting, devstat, ZFS kstats, interface and socket counters, sensors, GPU counters; publishes snapshots in shared memory; writes history rings; evaluates alert rules |
| Handle inspector | C++23 | part of the sampler service, separate interface | Lists a process's descriptors, rights and lineage ids on request; returns only what the caller may see |
| Control broker | C++23 | system service, privileged (shared with SY01's configuration broker, Proposed) | Signals, priority, affinity, rctl rules, service actions for processes the user does not own; authenticates and audits |
| History reader | C++23 | helper, Capsicum, read handles on ring files | Decodes ring files and downsamples a range for the UI |
| Alert actions | ForgeScript | separate engine per action | Runs a signed user action when an alert fires |

```
 +-------------------------- app host: System Monitor --------------------------+
 | ForgeScript: models, grouping, filters, actions        FUSE UI: tables, charts|
 +-----+----------------------+--------------------+----------------------------+
       | shm snapshots (ro)   | bus requests       | helper
       v                      v                    v
  sampler service  ◄──── handle inspector     history reader ── ring files (ro)
   │  reads kernel counters (read-only)
   │  writes ring files, evaluates alerts ──► svc.notify, svc.audit
   v
  control broker (privileged): signal, priority, rctl, service manager
       ▲ authentication through SH03
```

The app reads; it does not hold kernel access. Own-process signals go through the bus to the broker as well, so there is one audited path; the broker skips authentication for the caller's own processes.

### 6.2 Data model

| Entity | Fields | Store |
|---|---|---|
| `ProcSample` | `pid: i32`, `ppid`, `pgid`, `sid`, `uid`, `jid: i32`, `domain_id: u64`, `app_id: string`, `name`, `state: u8`, `cpu_ns: u64`, `cpu_pct: f32`, `threads: u32`, `rss, private, shared, virt, swap: u64`, `io_read, io_write: u64`, `net_rx, net_tx: u64`, `gpu_ns: u64`, `gpu_mem: u64`, `wakeups: u32`, `nice: i8`, `prio_class: u8`, `start_ns: u64`, `flags: u32` (capability mode, suspended, protected, not responding), `pax_flags: u32` | shared-memory snapshot, fixed-size records |
| `Domain` | `domain_id`, `name`, `app_id`, `kind: app\|helper\|service\|jail\|vm\|shell_scope`, `cap_mode: bool`, `handle_count: u32` | snapshot |
| `HandleInfo` | `fd: i32`, `kind`, `target: string`, `rights: u64[2]`, `lineage_id: u64`, `lineage_parent: u64`, `granted_by: string`, `opened_ns` | on request from the handle inspector |
| `SocketInfo` | `pid`, `proto`, `local`, `remote`, `state`, `rx, tx: u64`, `route: clear\|vpn\|tor`, `first_contact_name`, `opened_ns` | snapshot |
| `SystemSample` | `ts_ns`, `cpu: {user, sys, intr, nice, idle}[ncpu]`, `freq_khz[]`, `mem: {active, inactive, laundry, wired, free, arc_size, arc_target, arc_mru, arc_mfu, arc_hits, arc_misses}`, `swap: {total, used, pgin, pgout}`, `disks: {id, rd_bytes, wr_bytes, rd_ops, wr_ops, busy_pct, lat_us}[]`, `pools[]`, `ifaces: {id, rx, tx, errs, drops}[]`, `gpus[]`, `sensors: {id, kind, value}[]`, `power: {batt_pct, rate_mw, pkg_mw}` | snapshot + rings |
| `HistoryRing` | header `{magic, version, series_id, step_s, capacity, head, tz}` + fixed-width records (min, avg, max per step) | `/var/db/pbsd/monitor/*.ring` (system), per-app series alongside |
| `AlertRule` | `id`, `metric`, `op`, `threshold: f64`, `unit`, `for_s`, `scope`, `actions[]`, `cooldown_s`, `enabled` | `svc.settings` namespace `monitor.alerts` |
| `AlertEvent` | `rule_id`, `at`, `value`, `top_consumers[]` | sampler's alert log (ring) and `svc.audit` |
| `ServiceInfo` | `name`, `description`, `state`, `enabled`, `pid`, `restarts`, `last_exit`, `deps[]`, `domain_id` | from the service manager interface |
| `UiState` | tab, group mode, sort, columns, filters, overview tile order | `~/.config/org.pbsd.system-monitor/ui.json` |

Ring files are versioned; a version change starts a new ring and keeps the old one readable for its retention period.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Kernel counters (sysctl, kvm-style process list, devstat, kstat) | yes | no | OS interfaces | sampler service |
| Shared-memory snapshot (versioned fixed records, sequence lock) | yes | no | own code | app host (bounds-checked reader generated from the schema) |
| History ring files | yes | no (service writes) | own code | history reader helper |
| CSV, JSON export | no | yes | own code | app host |
| PNG, SVG graph export | no | yes | FUSE draw list to image; own SVG writer | app host |
| rctl rule text | no | yes (through the broker) | own code | control broker |
| DTrace scripts (`.d`) | shipped text | no | not parsed; handed to SY03 | — |
| Session bus records | yes | yes | schema compiler (G-08) | app host |

### 6.4 Integration

- **Intents handled**: `open` with `monitor://process/<pid>`, `monitor://app/<id>`, `monitor://tab/<name>`. **Emitted**: `open` to AD05, SE02, AD04, AD02, UT04, SY02, SY05, SY03, DV05, DV01, VM01, VM02.
- **Services**: `svc.settings`, `svc.notify`, `svc.intent`, `svc.audit` (read), `svc.power` (read inhibitors), `svc.portal.file` (exports), `svc.model` (optional), `svc.a11y`, plus Proposed `svc.monitor` (sampler and handle inspector) and `svc.config` (control broker).
- **Offers**: `org.pbsd.monitor.summary` (current totals for the panel, F094).
- **Backing repo facts (Verified)**: every PBSD handle carries `(rights, LineageId)`; `grant`, `borrow` and `duplicate` make child lineage ids; `revoke(id)` invalidates the node and its descendants (`docs/specs/HANDLES.md`, `docs/specs/CAPSICUM_LINEAGE.md`). The lineage spec plans a bridge onto FreeBSD Capsicum rights in a later wave. `hardening.prohibit_ptrace_syscall` and `hardening.prohibit_ptrace_capsicum` exist with defaults enabled and opt-out (`tools/hardening_sysctls.py --kernconf HARDENEDBSD`).
- **Proposed**: the sampler, the handle inspector's kernel interface for reading another process's rights and lineage, the control broker, per-process network and GPU accounting sources.

## 7. Security and privacy

Capabilities requested:

| Capability | Reason |
|---|---|
| `fs.app` | UI state |
| `fs.portal` | Save exports and reports |
| `net.none` | Never opens a socket; does no name lookups |
| `proc.helper:history-reader` | Decode ring files in a sandbox |
| `svc.monitor` (new, Proposed) | Read snapshots, handle lists, history |
| `svc.config` (new, Proposed) | Privileged process and service actions |
| `svc.settings`, `svc.notify`, `svc.intent`, `svc.audit`, `svc.power`, `svc.portal.file`, `svc.a11y` | As in 6.4 |
| `ai.local`, `svc.model` | Only for F098, off by default |

Not requested: `proc.exec` (tracing is handed to SY03), `fs.path`, any `dev.*`, `bg.run` (the sampler is a system service, not this app).

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| Other users' data | A local user | Reads command lines, environments, file names, endpoints of other users | The sampler filters by caller: other users' rows show name and totals only; details need authentication. Environment is always hidden until revealed. |
| Process control | A compromised app | Uses the monitor's interface to kill security services | Only this bundle id may call the broker's process interface; protected processes refused; privileged actions authenticated by the shell. |
| The app | Hostile process names, command lines, paths | Control characters, very long strings, bidirectional text to confuse the table | Fixed-size fields; names rendered with visible escapes; the snapshot reader is bounds-checked. |
| Secrets in command lines | Shoulder surfing, exports, the local model | Tokens passed as arguments shown or exported | Command-line column off by default; exports and reports have a redaction step; F098 excludes command lines unless allowed. |
| The system | The sampler itself | A privileged always-on service is a target | Read-only kernel access; enters capability mode after opening its sources; no network; fixed-size outputs; T2 target. |
| History | Local attacker with file access | Learns usage patterns | Rings are system-owned, readable only through the service; per-user app series visible only to that user and administrators. History can be off. |
| Honesty of the view | Malware hiding | A process misnames itself | Rows show the executable's digest, signer and app id from the kernel and the bundle store, not from the process's own title. |
| Capability truth | Stale data | UI shows a handle that was revoked | Handle lists are fetched on demand with a sequence number; revoked handles appear struck through until refresh, never as valid. |

Stored: UI state, alert rules. History in the system store. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `monitor.update_ms` | enum(500,1000,2000,5000) | 1000 | UI update rate |
| `monitor.start_tab` | enum(`overview`,`processes`,`last`) | `overview` | Tab at start |
| `monitor.group_mode` | enum(`none`,`tree`,`app`,`domain`,`jail`,`user`,`service`) | `app` | Default grouping |
| `monitor.cpu_percent_base` | enum(`core`,`machine`) | `core` | 100 % means one core or all |
| `monitor.units` | enum(`binary`,`decimal`) | `binary` | GiB or GB |
| `monitor.show_command_line` | bool | false | Command-line column |
| `monitor.show_system_processes` | bool | false | Include kernel and system processes |
| `monitor.heat_colours` | bool | true | Shade cells by value |
| `monitor.confirm_end` | bool | false | Confirm "End" |
| `monitor.confirm_force` | bool | true | Confirm "Force quit" |
| `monitor.end_grace_s` | int 1–60 | 5 | Wait before TERM after a close request |
| `monitor.history.enabled` | bool | true | Keep history |
| `monitor.history.per_app` | bool | true | Keep per-app series |
| `monitor.history.retention` | enum(`7d`,`30d`,`1y`) | `1y` | Longest ring kept |
| `monitor.alerts.enabled` | bool | true | Evaluate alert rules |
| `monitor.alerts.presets` | list<string> | `memory_low`,`disk_full`,`temp_high`,`service_failed` | Enabled presets |
| `monitor.graph.smooth` | bool | false | Smooth lines |
| `monitor.graph.range_default` | enum(`1m`,`5m`,`1h`) | `5m` | Live graph range |
| `monitor.mini.metrics` | list<string> | `cpu`,`mem`,`net` | Mini window graphs |
| `monitor.ai.explain` | bool | false | Local explanation |
| `monitor.emergency.minimal` | bool | true | Minimal view under pressure |

## 9. Accessibility and languages

- Every graph has a text equivalent: current, minimum, maximum and average for the visible range, and a "read values" table (`Alt+V`).
- Graph series differ by line style and marker as well as colour; the colour set is safe for common colour-vision differences.
- Live tables do not announce every update. A screen reader hears a change only for the focused row, on demand (`Ctrl+Shift+Space` reads the row).
- Freeze order (`Space`) keeps focus stable; focus follows the process, not the row index, when sorting changes.
- States are words: "suspended", "not responding", "unconfined".
- Numbers use the locale's digits and separators; units are translated.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | 150 ms; first full table 300 ms |
| App CPU at 1 s rate, Processes tab, 500 processes | under 0.5% of one core |
| Sampler CPU at 5 s history rate | under 0.2% of one core |
| Sampler memory | 24 MiB fixed |
| App memory | 70 MiB with all tabs visited |
| Table update, 2,000 processes | 4 ms model update, one frame |
| Largest supported | 20,000 processes, 200,000 handles listed on demand (virtualised), 256 cores |
| History query, 1 year at 15 min | under 50 ms |
| Ring storage | under 200 MiB total at default retention |
| Hidden window | 0 frames, 0 wake-ups in the app |

## 11. Testing and proof

Test plan (targets; nothing has run):

- Unit: grouping and summing, filters, rate computation from counters (wrap and reset handling), alert rule evaluation, unit formatting.
- Golden renders: every tab with recorded snapshots, three themes, 100% and 200%.
- Scripted UI: the 14 top tasks against a recorded snapshot stream and a fake broker.
- Fuzz: snapshot reader, ring file reader, alert rule parser.
- Differential: values against `top`, `ps`, `vmstat`, `iostat`, `zpool iostat`, `netstat` and `procstat` on the same machine at the same instant, within stated tolerances.
- Fault injection: sampler restart, process exit during inspection, counter wrap, clock step, suspend and resume.
- Overhead test: measured app and sampler CPU on an idle and a loaded machine.
- Privacy tests: as a standard user, no field of another user's process beyond the allowed set is returned.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Snapshot reader | No out-of-bounds for any shared-memory content; torn reads detected by the sequence lock | T1 | PRISM proof over the generated reader | — |
| Sampler record writer | Fixed-size outputs, no runtime errors | T2 | PRISM, sanitisers | Kernel interfaces are assumed |
| Visibility filter | A caller never receives restricted fields of processes it does not own without an authentication token | T1 | Proof over the filter function; privacy tests | Token validity is assumed from the broker |
| Ring file codec | Total, bounded, round-trips | T2 | PRISM plus fuzz | — |
| Alert evaluator | Fires exactly when the condition held for the whole duration | T2 | Property tests against a reference model | Timing assumptions |
| Control broker requests | Protected processes are never signalled | T1 | Proof over the request validator (finite protected set) | — |
| Rate computation | No overflow; handles counter reset | T2 | PRISM | — |
| ForgeScript models | Language safety | T3 | Tests | Interpreter not proved |

## 12. Build plan

Phase 1, usable core:

| ID | Work item | Depends on |
|---|---|---|
| SY04-W01 | Sampler service: processes, CPU, memory, swap, disks, interfaces; shared-memory snapshot and schema | G-06, G-08, G-09 |
| SY04-W02 | App skeleton: tabs, update loop, pause, preferences | G-02, G-07 |
| SY04-W03 | Process table: columns, sort, freeze, search, filters, tree and app grouping | SY04-W01, FUSE-WP-28 |
| SY04-W04 | Domain grouping and sandbox state column | SY04-W01, kernel domain ids |
| SY04-W05 | Actions for own processes: end, force quit, suspend, resume, priority | control broker |
| SY04-W06 | Streaming charts; processor, memory, disk, network tabs with top-consumer strips | SY04-GAP-1 |
| SY04-W07 | Overview dashboard | SY04-W06 |
| SY04-W08 | Inspector: overview, capabilities (from the permission store), graphs | AD05 |
| SY04-W09 | Per-process I/O and network rates | SY04-W01 |
| SY04-W10 | Export and copy | SY04-W03 |

Phase 2, full feature:

| ID | Work item | Depends on |
|---|---|---|
| SY04-W11 | Handle inspector: descriptors, rights, lineage; handles, files, memory tabs | kernel lineage interface, AD05 |
| SY04-W12 | Connections table, listening sockets, route classes, first-contact names | SE02, `svc.audit` |
| SY04-W13 | History rings, history browser, annotations source | SY04-W01, SY04-W06 |
| SY04-W14 | Alerts: rules, evaluator in the sampler, notifications, log | SY04-W13 |
| SY04-W15 | Services tab and actions; startup and background apps | service manager interface, SY01 |
| SY04-W16 | ARC detail, pools and datasets I/O | SY04-W01 |
| SY04-W17 | GPU, sensors, power, energy impact, wake-ups | VM05 |
| SY04-W18 | Privileged actions with authentication; protected processes; signals; affinity | SH03 |
| SY04-W19 | Jail and VM grouping; not-responding detection | VM01, VM02, SH05 |
| SY04-W20 | Who holds this file; hardening flags column; emergency mode; panel summary | SY04-W11, SH01 |

Phase 3, advanced:

| ID | Work item | Depends on |
|---|---|---|
| SY04-W21 | Limits editor (rctl), revoke network now | SE02 |
| SY04-W22 | Trace hand-off with the script library | SY03 |
| SY04-W23 | Modules tab, children and IPC tab, compare two processes | SY04-W11 |
| SY04-W24 | Kernel memory, file system event rates, scheduled jobs, boot summary | AD04 |
| SY04-W25 | Mini window, system report with redaction, remote host view | SY03 |
| SY04-W26 | Local explanation | G-15 |

## 13. Open questions and risks

- The kernel interface for reading another process's handle rights and lineage ids is Proposed. `docs/specs/CAPSICUM_LINEAGE.md` describes the model; an inspection interface was not found in what was read.
- "Capability domain" needs one kernel-level identifier that the sampler can read per process. Its exact form is not established here.
- Per-process network accounting and per-process GPU accounting sources on HardenedBSD are Unverified.
- The control broker is shared with SY01; its design must be settled once (see SY01 section 13).
- The service manager interface (rc-based or a new supervisor) is not chosen in the documents read.
- Whether `chart` can do streaming stacked series without allocation is Unverified; if not, SY04-GAP-1 is a real FUSE work item.
- History is a privacy cost. Default on is proposed for usefulness; the first-run tour (AD01) should say so.
- DTrace availability under `forbid_kmod` and the ptrace hardening knobs decides whether F099 works on a Strict profile.
