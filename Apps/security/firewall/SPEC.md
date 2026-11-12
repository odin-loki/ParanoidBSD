<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Firewall

By Odin Loch

| | |
|---|---|
| ID | `SE02` |
| Bundle | `org.pbsd.firewall` |
| Category | security |
| Priority | P0 |
| Backing repos | ParanoidBSD (pf, `pfctl`, `libpfctl`, `pflogd`, unbound, Casper network service in the `hbsd/` reference tree), FORGE (network service planned in FORGE-WP-21), FUSE (UI) |
| Status | Planned. Nothing built. |

## 1. Purpose

Firewall decides what may use the network and shows what is using it. It manages pf rules in a visual editor and as text, sets the network policy of each app from its `net.*` capabilities, shows live connections and traffic per app, handles first-contact decisions, blocklists and DNS policy, and applies changes safely with automatic roll-back. It replaces a hand-edited `pf.conf` and the firewall pages of other desktops.

Ground truth (Verified in this repo, 12 Nov 2026):

| Fact | Source |
|---|---|
| The PBSD plan keeps pf: "socket layer with BSD and Linux APIs, pf firewall kept"; pf is to be ported as the packet filter on the new buffer type. | `docs/plan/PBSD_PLAN.md` (network package, lines 487–490) |
| The reference tree has the pf kernel code with ioctl and netlink interfaces, logging and state export. | `hbsd/src/sys/netpfil/pf/pf_ioctl.c`, `pf_nl.c`, `if_pflog.c`, `pflow.c`, `if_pfsync.c` |
| User-space pieces exist: `pfctl` (grammar in `parse.y`), `libpfctl`, `pflogd`, `pflowctl`, `blocklistd`, `ftp-proxy`. | `hbsd/src/sbin/pfctl/`, `hbsd/src/lib/libpfctl/`, `hbsd/src/sbin/pflogd/`, `hbsd/src/sbin/pflowctl/`, `hbsd/src/usr.sbin/blocklistd/` |
| A validating resolver is in the tree. | `hbsd/src/contrib/unbound/`, `hbsd/src/usr.sbin/unbound/` |
| Capsicum network and DNS broker services exist as a reference for brokered sockets. | `hbsd/src/lib/libcasper/services/cap_net/`, `cap_dns/` |
| The app platform plans a network service that checks rules, resolves on the app's route, connects, logs first contact and hands back a socket; the route mechanism is left to SE02, SE03 and the OS. | `Apps/00-foundation/build-plans/FORGE_BUILD_PLAN.md` FORGE-WP-21 |

Everything below is Proposed.

## 2. Scope

In scope:

- pf rule sets: visual editor, text editor, tables, anchors, NAT and redirection, queues (basic).
- Per-app network policy bound to manifest capabilities `net.none`, `net.host`, `net.user`, `net.route`.
- Live connections, per-app traffic, history.
- First-contact decisions and their prompts (drawn by SH04).
- Blocklists: address, domain, country (offline database), custom.
- DNS policy: local validating resolver, encrypted upstreams, filtering, per-profile resolvers.
- Network profiles (home, work, public, custom), chosen per network.
- Kill switch.
- Logging to `svc.audit`, log viewing for firewall events.
- Rule simulation, safe apply, roll-back.

Out of scope:

| Item | Owner |
|---|---|
| Tor, mix network and VPN tunnels, route definitions | SE03 (SE02 enforces the route an app is allowed) |
| Granting `net.*` capabilities to apps | AD05 (SE02 narrows inside what is granted and links there) |
| Wi-Fi, Ethernet, addresses, interfaces set-up | SY01 |
| Packet capture and protocol dissection | DV05 |
| Posture score, alerts and incidents | SE01 (SE02 feeds summaries) |
| Jail and VM network topology | VM02, VM01 (SE02 shows their anchors read-only, edits by request) |
| Browser-level tracker blocking | IN01 |
| Certificate trust | SE04 |

## 3. Users and top tasks

Personas:

- **Everyday user**: wants "on", wants to answer an occasional clear question, wants to block an app.
- **Privacy-minded user**: wants to see every endpoint an app talks to and to filter DNS.
- **Administrator**: writes pf rules as text, tests them, deploys a signed rule set.
- **Developer**: opens a port for a local server for one hour.
- **Traveller**: wants a stricter profile on public networks without thinking.

| # | Task | Target |
|---|---|---|
| T1 | See that the firewall is on and what profile is active | 0 steps (Overview) |
| T2 | Block one app from the network | 2 steps: app row → toggle |
| T3 | See what an app is connected to right now | 2 steps: Apps → app |
| T4 | Answer a first-contact prompt | 1 click, with "always" by default off |
| T5 | Allow an inbound port for one hour | 3 steps: New rule → port → duration |
| T6 | Engage the kill switch | 1 click or `Ctrl+Shift+K`; also in quick settings |
| T7 | Add a blocklist from a file | 3 steps |
| T8 | Turn on encrypted DNS with a chosen resolver | 3 steps |
| T9 | Edit pf rules as text and apply safely | Edit → `Ctrl+Return` → confirm within 30 s |
| T10 | Test whether a packet would pass | 3 steps: Simulate → fill → Run |
| T11 | Undo the last change | `Ctrl+Z` or History → "Restore" |
| T12 | Find why a connection was blocked | 2 steps: Log → row → "Explain" |
| T13 | Set the profile for a new network | 1 click in the prompt when joining |
| T14 | Export the rule set | 2 steps |

## 4. Features

### 4.1 Status and overview

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE02-F001 | Master state | On, Off (needs authentication and a reason), Kill switch engaged. Off shows a persistent warning in the panel | M | 1 |
| SE02-F002 | Active profile | Name, network it was chosen for, default inbound and outbound stance | M | 1 |
| SE02-F003 | Live counters | Packets and bytes passed and blocked, states, state-table fill | M | 1 |
| SE02-F004 | Throughput chart | Up and down rate, last 5 minutes, per second | S | 1 |
| SE02-F005 | Top talkers | Top 5 apps and top 5 remote endpoints by bytes, last hour | S | 1 |
| SE02-F006 | Recent blocks | Last 10 blocked attempts with app, endpoint, rule | M | 1 |
| SE02-F007 | Pending decisions | First-contact questions waiting for an answer | M | 1 |
| SE02-F008 | Health checks | pf enabled, rule set loaded and matches the stored hash, logger running, resolver running, tables loaded | M | 1 |
| SE02-F009 | Status feed | Publishes state to SE01 and to the SH02 quick-settings tile | M | 1 |

### 4.2 Rules: visual editor

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE02-F010 | Rule list | Ordered table grouped by section: System, Kill switch, Apps, Blocklists, User, Profile. System sections are read-only | M | 1 |
| SE02-F011 | Rule fields | Action (pass, block drop, block return), direction, interface or group, address family, protocol, source, source port, destination, destination port, state policy, log, quick, label, tag, schedule, expiry, comment | M | 1 |
| SE02-F012 | Rule builder | Form with plain-language summary: "Allow incoming TCP to port 8080 from the local network for 1 hour" | M | 1 |
| SE02-F013 | Presets | SSH in, web server, file sharing on the local network, printing, local discovery, game hosting, ICMP sets | S | 2 |
| SE02-F014 | Reorder | Drag or `Alt+Up/Down`; order matters and is shown by number | M | 1 |
| SE02-F015 | Enable and disable | Per rule without deleting | M | 1 |
| SE02-F016 | Temporary rules | Expiry by duration or time; removed automatically; survive reboot until expiry | M | 2 |
| SE02-F017 | Scheduled rules | Active on given weekdays and hours | C | 3 |
| SE02-F018 | Tables | Named address sets; edit, import, count, last change; used by rules and blocklists | M | 1 |
| SE02-F019 | Anchors | Tree of anchors with owner (app, VM, jail, user); owner-managed anchors are read-only here | M | 2 |
| SE02-F020 | Macros and lists | Named interfaces, hosts, ports reused across rules | S | 2 |
| SE02-F021 | NAT and redirection | `nat-to`, `rdr-to`, binat; port forwarding wizard for VMs and jails | S | 2 |
| SE02-F022 | Normalisation | Scrub options, fragment handling, antispoof per interface | S | 2 |
| SE02-F023 | Options | State limits, timeouts, optimisation, block policy, skip interfaces | S | 2 |
| SE02-F024 | Queues | Basic traffic shaping: per-app or per-rule rate limits | C | 3 |
| SE02-F025 | Rate and connection limits | `max-src-conn`, `max-src-conn-rate`, overload table, for inbound services | S | 2 |
| SE02-F026 | Hit counters | Evaluations, packets, bytes, states per rule; reset; "never matched" filter | M | 1 |
| SE02-F027 | Shadow and conflict detection | Marks rules that can never match or that contradict an earlier quick rule | S | 2 |
| SE02-F028 | Search and filter | By text, port, address, app, label, state (enabled, expired, unmatched) | M | 1 |
| SE02-F029 | IPv4 and IPv6 parity | Every visual rule covers both families unless narrowed; a warning when a rule covers only one | M | 1 |
| SE02-F030 | Rule notes and ownership | Who made it, when, why; link to the audit record | S | 2 |

### 4.3 Rules: text editor

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE02-F031 | Text view of the user section | pf.conf syntax in `codeEdit` with highlighting, folding by anchor, line numbers | M | 1 |
| SE02-F032 | Round trip | Visual and text views edit the same model; constructs the visual editor cannot show are kept verbatim and shown as "advanced rule" rows | M | 1 |
| SE02-F033 | Live syntax check | Parse on pause (300 ms) with the real pf grammar in the check helper; errors underlined with the parser message | M | 1 |
| SE02-F034 | Completion | Keywords, interfaces, tables, macros, service names, labels | S | 2 |
| SE02-F035 | Expanded view | Shows the rules after macro and list expansion, as pf will load them | S | 2 |
| SE02-F036 | Diff before apply | Old against new, as text and as a list of rule changes | M | 1 |
| SE02-F037 | Full-file mode | Administrators may edit the whole generated rule set; a banner warns that app sections are regenerated | C | 3 |
| SE02-F038 | Format | Normalise spacing and ordering of rule options | C | 2 |
| SE02-F039 | Import pf.conf | Read an existing file into the user section; unsupported lines are reported, never dropped silently | S | 2 |

### 4.4 Per-app network policy

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE02-F040 | App list | Every installed app with its declared `net.*` capabilities, granted state, current policy, live connection count, bytes today | M | 1 |
| SE02-F041 | Policy levels | Blocked, Declared hosts only (`net.host`), Ask on first contact, Allowed (`net.user`), Local network only. Never wider than the granted capability | M | 1 |
| SE02-F042 | Narrow only | Firewall can narrow a grant, never widen it; "needs capability" rows link to AD05 | M | 1 |
| SE02-F043 | Endpoint rules per app | Allow or block by host name, domain suffix, address, CIDR, port, protocol | M | 1 |
| SE02-F044 | Route binding | Shows and enforces `net.route:<clear|vpn|tor>`; a socket on the wrong route is refused | M | 2 |
| SE02-F045 | Local network access | Separate switch for private ranges, link-local and multicast; off by default for apps with `net.user` | M | 1 |
| SE02-F046 | Inbound per app | An app may listen only if its manifest declares it and the user allows it; shown with port and scope (loopback, local network, any) | M | 2 |
| SE02-F047 | Per-profile policy | A different level per network profile (e.g. blocked on public networks) | S | 2 |
| SE02-F048 | Metered behaviour | Block or limit background traffic on metered networks | C | 3 |
| SE02-F049 | Rate limit per app | Upload and download caps | C | 3 |
| SE02-F050 | Temporary allow | Allow for 5 minutes, 1 hour, until the app quits | M | 1 |
| SE02-F051 | Helper inheritance | Helpers get at most their app's policy; listed under the app | M | 1 |
| SE02-F052 | Non-app processes | System services, shells, VMs and jails are listed as their own subjects with their own rules | M | 2 |
| SE02-F053 | Policy template | Copy one app's policy to another; export and import as signed JSON | C | 3 |
| SE02-F054 | Explain | For any app: the chain capability → grant → policy → rule that decides a given endpoint | M | 2 |

### 4.5 Live connections and traffic

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE02-F055 | Connections table | App, process, protocol, local and remote address and port, host name, route, state, age, bytes each way, rule | M | 1 |
| SE02-F056 | Group by | App, remote host, domain, country (offline database), route, interface | M | 1 |
| SE02-F057 | Kill a connection | Removes the state; optional "and block this endpoint" | M | 1 |
| SE02-F058 | Per-app traffic chart | Up and down rate per app, stacked, selectable window (5 min to 30 days) | S | 2 |
| SE02-F059 | History | Per app and endpoint: first seen, last seen, total bytes, connection count; kept 30 days | S | 2 |
| SE02-F060 | Name provenance | The host name shown is the name the app asked the resolver for, not a reverse lookup | M | 1 |
| SE02-F061 | Listening sockets | Every listener, its app, its scope, whether a rule exposes it | M | 1 |
| SE02-F062 | Pause updates | Freeze the table to inspect; `Space` | S | 1 |
| SE02-F063 | Open in Packet Analyser | Start a capture filtered to this connection in DV05 (needs its own capability) | C | 3 |
| SE02-F064 | Endpoint detail | Address ranges, blocklist membership, apps that use it, history | S | 2 |
| SE02-F065 | Export | CSV or JSON of the current table or history | S | 2 |

### 4.6 First contact

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE02-F066 | First-contact log | Every first contact of an app with a new host is recorded (PLATFORM.md section 6) | M | 1 |
| SE02-F067 | Ask mode | The connection waits while the shell asks; answers: Allow once, Allow this session, Always allow, Block; scope: this host, this domain, any host on this port | M | 1 |
| SE02-F068 | Prompt content | App name and signer, host name, port and service name, route, blocklist hits, how often this app asked this hour | M | 1 |
| SE02-F069 | Time-out | No answer in 30 s means block; the request joins "Pending decisions" | M | 1 |
| SE02-F070 | Prompt limits | At most 3 prompts per app per minute; the rest are blocked and queued | M | 1 |
| SE02-F071 | Quiet mode | No prompts; unknown hosts are blocked and listed for later review | S | 1 |
| SE02-F072 | Learning mode | For a set time, allow and record, then show the list to approve in bulk | S | 2 |
| SE02-F073 | Review queue | Bulk approve or block queued hosts; group by domain | M | 2 |
| SE02-F074 | Suggest domain rules | "This app contacted 14 hosts under example.org; allow the domain?" | C | 3 |

### 4.7 Blocklists

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE02-F075 | List manager | Name, kind (address, domain), entries, source, last update, signature state, hits | M | 2 |
| SE02-F076 | Import from file | Plain lists, hosts-file format, CIDR lists, domain lists, RPZ zones | M | 2 |
| SE02-F077 | Subscribed lists | Optional fetch from a URL the user adds; needs `net.host` granted for that host; signature or pinned hash checked | S | 2 |
| SE02-F078 | Bundled lists | Bogon and reserved ranges ship with the system as a signed package | M | 2 |
| SE02-F079 | Country lists | Block or allow by country from an offline address database shipped as a package; accuracy limits stated | C | 3 |
| SE02-F080 | Custom list | User entries with comments and expiry | M | 2 |
| SE02-F081 | Allow list | Exceptions that override blocklists; each shown as a finding in SE01 | M | 2 |
| SE02-F082 | Scope | Apply a list to all apps, some apps, some profiles; inbound, outbound or both | S | 2 |
| SE02-F083 | Test entry | "Is this host or address blocked, and by which list?" | S | 2 |
| SE02-F084 | Dynamic block | Addresses that exceed inbound connection limits join an overload table for a set time (pf overload; `blocklistd` as reference) | S | 3 |
| SE02-F085 | Size limits | Lists up to 2 million entries; larger are refused with a message | M | 2 |

### 4.8 DNS policy

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE02-F086 | Local resolver | A validating, caching resolver on loopback (unbound, in base) is the only resolver apps reach | M | 1 |
| SE02-F087 | DNSSEC | Validation on by default; failures shown with the reason | M | 1 |
| SE02-F088 | Encrypted upstream | DNS over TLS to named resolvers with certificate pinning; several upstreams with fail-over | M | 2 |
| SE02-F089 | DNS over HTTPS and oblivious DoH | Through a small upstream helper; optional | C | 3 |
| SE02-F090 | Recursive mode | Resolve from the roots with no upstream; slower, no single observer | S | 2 |
| SE02-F091 | Resolver presets | A short editable list of public resolvers with their stated policies; none chosen by default | S | 2 |
| SE02-F092 | Plain DNS fall-back | Off by default; if on, a warning badge is shown while it is in use | M | 2 |
| SE02-F093 | DNS leak guard | Outbound port 53, 853 and known DoH endpoints are blocked for everything except the resolver | M | 1 |
| SE02-F094 | Filtering | Domain blocklists answer NXDOMAIN or a null address; per-list counters | M | 2 |
| SE02-F095 | Per-route resolution | Names for an app on the `vpn` or `tor` route resolve on that route only | M | 2 |
| SE02-F096 | Per-profile DNS | Different upstreams and filters per network profile | S | 2 |
| SE02-F097 | Local names | Static host entries; local zones; split DNS for a domain suffix | S | 2 |
| SE02-F098 | Query log | Off by default; when on: app, name, type, answer, blocked-by; retention 24 h | S | 2 |
| SE02-F099 | Cache control | Flush, minimum and maximum TTL, cache size | C | 2 |
| SE02-F100 | Captive portal handling | Detects a captive network; opens a disposable browser profile on the clear route with DNS filtering relaxed only for that sandbox | S | 3 |
| SE02-F101 | Query minimisation and padding | QNAME minimisation on; padding on encrypted transports | S | 2 |

### 4.9 Profiles and kill switch

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE02-F102 | Built-in profiles | Home (local network allowed), Work, Public (no inbound, no local network, no discovery), Offline (loopback only) | M | 1 |
| SE02-F103 | Custom profiles | Clone and edit; each holds inbound stance, local-network stance, app overrides, DNS set, blocklists | S | 2 |
| SE02-F104 | Per-network binding | Each known network (by identity from SY01) remembers its profile; unknown networks get Public | M | 1 |
| SE02-F105 | Join prompt | On a new network: "Treat as Home, Work or Public?"; default Public | M | 1 |
| SE02-F106 | Automatic switch | Profile changes when the active network changes; the change is atomic | M | 1 |
| SE02-F107 | Kill switch | Blocks all traffic except loopback at once; survives reboot until released; release needs authentication | M | 1 |
| SE02-F108 | Route kill switch | Blocks all traffic that is not on the chosen route when that route is down (used by SE03) | M | 2 |
| SE02-F109 | Lockdown allow list | Optional list of apps that keep access during kill switch (default empty) | C | 3 |
| SE02-F110 | Boot-time stance | A minimal default-deny rule set loads before any interface comes up | M | 1 |

### 4.10 Simulation, apply and history

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE02-F111 | Packet simulator | Enter app, direction, protocol, addresses, ports, interface; shows the verdict and the deciding rule, against the live or the edited rule set | M | 2 |
| SE02-F112 | Replay check | Before apply, replay the last hour of connection records against the new rule set and list what would change | S | 3 |
| SE02-F113 | Parse check | The new rule set is loaded with the "no-op" option first; errors stop the apply | M | 1 |
| SE02-F114 | Safe apply | Apply, then a 30 s countdown; "Keep" confirms, anything else restores the previous set. Protects remote sessions | M | 1 |
| SE02-F115 | Atomic load | Rule set and tables swap in one transaction | M | 1 |
| SE02-F116 | History | Every applied rule set kept with time, author, summary, hash; diff any two; restore | M | 1 |
| SE02-F117 | Undo | `Ctrl+Z` undoes edits; after apply, "Restore previous" is one click | M | 1 |
| SE02-F118 | Lock-out guard | Warns when a change would cut the session that is making it (remote desktop, SSH) | S | 2 |
| SE02-F119 | Signed rule sets | Export and import `.pbsd-fwrules` with a signature from SE04; unsigned imports are marked | S | 3 |
| SE02-F120 | Snapshot on apply | Configuration dataset snapshot through `svc.backup` | S | 2 |

### 4.11 Logging, automation, basics

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE02-F121 | Audit records | Rule changes, policy changes, kill switch, first-contact answers, blocks (summarised per minute per app and endpoint) go to `svc.audit` | M | 1 |
| SE02-F122 | Firewall log view | Blocked and logged packets with app, rule, direction; filters; live tail | M | 1 |
| SE02-F123 | Explain a block | From a log row: the rule, its section, who made it, a button to allow | M | 2 |
| SE02-F124 | Log retention | 7 days of detail by default, counters for 90 days | S | 2 |
| SE02-F125 | Notifications | Optional toast on a block for a foreground app; digest of new endpoints | S | 2 |
| SE02-F126 | Command-line twin | `fwctl status|apply|simulate|block-app|kill-switch` on the same service | S | 2 |
| SE02-F127 | Script hooks | ForgeScript automation: react to profile change, expiry, new network | C | 3 |
| SE02-F128 | Read-only mode | Without authentication everything is visible and nothing changes | M | 1 |
| SE02-F129 | Search | `Ctrl+K` over rules, apps, endpoints, lists, settings | M | 1 |
| SE02-F130 | Recovery | If the service cannot load its rule set, the boot-time deny set stays and the app says so | M | 1 |
| SE02-F131 | Print and export | Rule set as text, Markdown table or PDF | C | 3 |
| SE02-F132 | Help in place | `F1` on a field opens the handbook section for that pf option | S | 2 |

## 5. GUI

Layout pattern: sidebar + content with an inspector.

### 5.1 Window map

```
Main window
├─ Sidebar: Overview · Apps · Connections · Rules · Blocklists · DNS · Profiles · Log · History
├─ Content
│   ├─ Overview
│   ├─ Apps ── list + app detail (Policy · Endpoints · Traffic · History)
│   ├─ Connections ── tabs: Active · Listening · History
│   ├─ Rules ── tabs: Visual · Text · Tables · Anchors · NAT · Options
│   ├─ Blocklists ── tabs: Lists · Allow list · Test
│   ├─ DNS ── tabs: Resolver · Upstreams · Filtering · Local names · Query log
│   ├─ Profiles ── list + editor · Networks
│   ├─ Log
│   └─ History ── list + diff
├─ Inspector (F4): rule, connection, endpoint or app properties
├─ Dialogs: New rule · Edit rule · Port forward wizard · Import list · Add upstream ·
│           Simulate packet · Apply changes (diff) · Confirm keep (countdown) · Import rules ·
│           Turn firewall off · Release kill switch
├─ Sheets: Temporary allow · Explain decision · Review queue
└─ Popovers: Counter detail · Endpoint card · Filter builder
Shell surfaces (SH02/SH04): first-contact prompt, join-network prompt, quick-settings tile, kill-switch toggle
```

### 5.2 Main window

Apps page at 1280×800.

```
+------------------------------------------------------------------------------------------+
| [=] Firewall — Home profile       [ Search rules, apps, hosts…  Ctrl+K ]  [net][▁][▢][✕] |
+-------------+----------------------------------------------------------------------------+
|   Overview  | (1) [ Filter apps…     ] Show (•) All ( ) Active ( ) Blocked   [Kill switch]|
| > Apps      +-------------------------------+--------------------------------------------+
|   Connections| (2) App          Policy   Now | (3) Mail  org.pbsd.mail   ✔ signed         |
|   Rules     | > Mail          Declared   3  | Capability: net.user, net.route:clear      |
|   Blocklists|   Browser       Ask        41 | Policy  [ Declared hosts only        v] (4)|
|   DNS       |   Notes         Blocked    0  | [x] Block on Public networks               |
|   Profiles  |   Feeds         Allowed    2  | [ ] Local network access                   |
|   Log       |   Software      Declared   0  | Route   clear   [Change in Anonymity…]     |
|   History   |   Messenger     Ask        5  | [Policy] [Endpoints] [Traffic] [History]   |
|             |   Text Editor   No network —  +--------------------------------------------+
|             |   ─ System ─                  | (5) Endpoint            Port  Rule   Bytes |
|             |   resolver      System     6  | ✔ imap.example.org      993   allow  2.1 M |
|             |   time sync     System     1  | ✔ smtp.example.org      465   allow  40 K  |
|             |   ─ VMs and jails ─           | ✖ tracker.example.net   443   block  0     |
|             |   dev-jail      Anchor     0  | ? cdn.example.com       443   ask    —     |
|             |                               | [Add endpoint…] [Temporary allow…] [Explain]|
|             |                               | (6) ▁▂▅▇▅▂▁▁▂▃ up 12 KiB/s · down 340 KiB/s|
+-------------+-------------------------------+--------------------------------------------+
| (7) Firewall on · Home · 58 states · 2 pending decisions · DNS encrypted · unsaved: 0    |
+------------------------------------------------------------------------------------------+
```

Callouts:

1. Toolbar: `lineEdit` filter, `radioButton` group, kill switch `toggleSwitch` styled `--danger` when on.
2. App list: `tableView` with section rows (Apps, System, VMs and jails); the policy cell is an inline `comboBox`.
3. Header: `label`s with trust badge from the bundle signature.
4. Policy `comboBox`; options wider than the grant are disabled with the reason in a tooltip.
5. Endpoint rules: `tableView`; state icon and word; inline edit with `F2`.
6. Traffic sparkline: `chart`.
7. Status bar: master state, profile, states, pending decisions (click opens the review queue), DNS state, unsaved count.

States: no apps with network capability ("No app can use the network yet"), service down (banner, last known data greyed), read-only (lock icon on controls), kill switch on (red banner across the content, all rows show "blocked by kill switch"), narrow (list → detail navigation).

### 5.3 Other views and dialogs

**Overview**

```
+---------------------------------------------------------------------------------------+
| ✔ Firewall on        Profile: Home (network "house")        [Kill switch ( o)]        |
| Inbound: block · Outbound: per app · Local network: allowed · DNS: encrypted, filtered|
+--------------------------------------------+------------------------------------------+
| Throughput (5 min)                         | Top apps (1 h)        Top endpoints (1 h)|
|  ▁▂▃▅▇▇▅▃▂▂▃▅▇▅▃  down 1.2 MiB/s           | Browser   412 MiB     cdn.example.com    |
|  ▁▁▁▂▂▁▁▁▁▁▂▂▁▁▁  up   80 KiB/s            | Software  120 MiB     pkg.example.org    |
+--------------------------------------------+------------------------------------------+
| Recent blocks                              | Pending decisions (2)                    |
| 14:02 Notes → 203.0.113.9:8443  no net     | Feeds → news.example.org:443   [Review]  |
| 14:00 in  198.51.100.7 → :22    default    | Browser → 2001:db8::5:443      [Review]  |
+--------------------------------------------+------------------------------------------+
| Health: ✔ pf enabled ✔ rule set matches ✔ logger ✔ resolver ✔ 6 tables loaded         |
+---------------------------------------------------------------------------------------+
```

**Rules › Visual**

```
+---------------------------------------------------------------------------------------+
| Rules  [Visual] [Text] [Tables] [Anchors] [NAT] [Options]      [New rule…] [Simulate…]|
| [ Filter…         ]  [x] Show system sections   3 unsaved changes  [Discard] [Apply…] |
+---------------------------------------------------------------------------------------+
|  #  On  Action Dir Proto From          To            Port   Log  Hits   Label         |
| ▾ System (read-only)                                                                  |
|  1  ✔   block  any any   any           any           —      ✔    1 204  default-deny  |
|  2  ✔   pass   out any   resolver      upstreams     853    —    9 113  dns-upstream  |
| ▾ Kill switch (inactive)                                                              |
| ▾ Apps (generated, 14 rules)                                                          |
| ▾ User                                                                                |
|> 20 ✔   pass   in  tcp   <localnet>    self          8080   —    0      dev-server ⏱1h|
|  21 ✔   pass   in  icmp  any           self          echo   —    44     ping          |
|  22 ○   pass   in  tcp   any           self          22     ✔    0      ssh (disabled)|
+---------------------------------------------------------------------------------------+
```

`treeView` in table mode with section nodes; `⏱` marks a temporary rule; "advanced rule" rows show raw text.

**Rules › Text**

```
+---------------------------------------------------------------------------------------+
| Rules  [Visual] [Text] …                         [Expand] [Format] [Diff] [Apply… ⌃↵] |
+---------------------------------------------------------------------------------------+
|  1 | # User section — edited here; other sections are generated                       |
|  2 | localnet = "{ 192.168.0.0/16, fd00::/8 }"                                        |
|  3 | pass in on egress proto tcp from $localnet to (self) port 8080 label "dev-server"|
|  4 | pass in inet proto icmp icmp-type echoreq label "ping"                           |
|  5 | pass in proto tcp to port ssh keep stat                                          |
|    |                                 ~~~~~~~~~ syntax error: expected "state"         |
+---------------------------------------------------------------------------------------+
| ✖ 1 error · line 5 col 34                                     parser: pf check helper |
+---------------------------------------------------------------------------------------+
```

**Connections › Active**

```
+---------------------------------------------------------------------------------------+
| Connections [Active] [Listening] [History]   Group by [App v]  [Pause ␣]  [Export…]   |
+---------------------------------------------------------------------------------------+
| App / remote host           Proto  Remote            Route  Age    ↓        ↑   Rule  |
| ▾ Browser (41)                                                      88 MiB   2 MiB    |
|     cdn.example.com         tcp    203.0.113.20:443  clear  2m     60 MiB   1 MiB  14 |
|     video.example.org       udp    2001:db8::9:443   clear  40s    28 MiB   1 MiB  14 |
| ▾ Mail (3)                                                          2 MiB   40 KiB    |
|   > imap.example.org        tcp    198.51.100.3:993  clear  1h     2 MiB    30 KiB  9 |
| ▸ Messenger (5)                                              tor                      |
+---------------------------------------------------------------------------------------+
| Selected: Mail → imap.example.org:993     [Kill connection] [Block endpoint…] [Explain]|
+---------------------------------------------------------------------------------------+
```

**DNS › Upstreams**

```
+---------------------------------------------------------------------------------------+
| DNS  [Resolver] [Upstreams] [Filtering] [Local names] [Query log]                     |
+---------------------------------------------------------------------------------------+
| Mode  (•) Encrypted upstream (DNS over TLS)  ( ) Recursive from roots  ( ) Network's  |
| [x] Validate DNSSEC   [x] Query minimisation   [ ] Allow plain DNS fall-back          |
|                                                                                       |
| Upstream           Address          TLS name            Pin        State     Latency  |
| ✔ resolver-a       192.0.2.53       dns.a.example       sha256:…   ✔ up      18 ms    |
| ✔ resolver-b       2001:db8::53     dns.b.example       sha256:…   ✔ up      31 ms    |
| ○ resolver-c       198.51.100.53    dns.c.example       —          disabled  —        |
|                                         [Add upstream…] [Test all] [Flush cache]      |
+---------------------------------------------------------------------------------------+
```

**Simulate packet dialog**

```
+---------------------------------------------------------------------+
| Would this packet pass?                                             |
| Rule set (•) Live  ( ) Edited (3 unsaved changes)                   |
| App [ Mail            v]   Direction (•) Out ( ) In                 |
| Protocol [tcp v]  From [ self        ]  To [ imap.example.org ]     |
| Port [ 993 ]  Interface [ egress v]  Route [ clear v]               |
|                                                        [Run]        |
| ✔ PASS — rule 9 "app org.pbsd.mail declared hosts" (Apps section)   |
|   evaluated 9 of 31 rules · state would be created                  |
|                                                    [Close] [Copy]   |
+---------------------------------------------------------------------+
```

**Apply and confirm**

```
+---------------------------------------------------------------------+
| Apply 3 changes?                                                    |
| + pass in tcp <localnet> → self:8080  (expires in 1 h)              |
| ~ rule 21: log off → on                                             |
| − pass in tcp any → self:22                                         |
| Replay of the last hour: 0 current connections would be cut         |
| [x] Snapshot configuration first        [Show text diff]            |
|                                         [Cancel] [Apply and test]   |
+---------------------------------------------------------------------+
+---------------------------------------------------------------------+
| Keep the new rules?            Restoring the old rules in 24 s      |
| [███████████████░░░░░]                    [Restore now] [Keep]      |
+---------------------------------------------------------------------+
```

**First-contact prompt** (drawn by SH04 from data SE02 supplies)

```
+---------------------------------------------------------------------+
| Feeds wants to connect to news.example.org                          |
| Port 443 (HTTPS) · route: clear · not on any blocklist              |
| Feeds is signed by PBSD. It has asked once this hour.               |
| Scope (•) this host ( ) *.example.org ( ) any host on port 443      |
| [Block]      [Allow once] [Allow this session] [Always allow]       |
| Waiting 27 s, then block                                            |
+---------------------------------------------------------------------+
```

### 5.4 Menus

```
File      Import Rules… · Import Blocklist… · Export Rule Set… Ctrl+E · Export Connections… ·
          Print… Ctrl+P · Close Ctrl+W · Quit Ctrl+Q
Edit      Undo Ctrl+Z · Redo Ctrl+Shift+Z · Cut/Copy/Paste Rule Ctrl+X/C/V · Duplicate Ctrl+D ·
          Delete Delete · Find Ctrl+F · Replace Ctrl+H (text view) · Preferences Ctrl+,
View      Overview Ctrl+1 · Apps Ctrl+2 · Connections Ctrl+3 · Rules Ctrl+4 · Blocklists Ctrl+5 ·
          DNS Ctrl+6 · Profiles Ctrl+7 · Log Ctrl+8 · History Ctrl+9 · Visual/Text Ctrl+T ·
          Show System Sections · Sidebar F9 · Inspector F4 · Pause Updates Space · Refresh F5
Rule      New Rule… Ctrl+N · Edit Enter · Enable/Disable Ctrl+/ · Move Up Alt+Up · Move Down Alt+Down ·
          Make Temporary… · New Table… · Port Forward… · Reset Counters
Firewall  Apply… Ctrl+Return · Discard Changes · Simulate Packet… Ctrl+Shift+S ·
          Restore Previous Rule Set · Kill Switch Ctrl+Shift+K · Turn Firewall Off… · Switch Profile ▸
App       Block Ctrl+B · Allow Temporarily… Ctrl+Shift+T · Add Endpoint… · Explain Decision Ctrl+Shift+E ·
          Open in Permissions
DNS       Flush Cache · Test Upstreams · Test a Name… · Toggle Query Log
Help      Firewall Help F1 · pf Syntax Reference · Keyboard Shortcuts · About
```

### 5.5 Keyboard and pointer

- App shortcuts beyond the reserved set: `Ctrl+Return` apply, `Ctrl+Shift+K` kill switch, `Ctrl+/` enable or disable a rule, `Alt+Up/Down` reorder, `Ctrl+B` block app, `Ctrl+T` visual/text, `Space` pause updates, `Ctrl+Shift+S` simulate.
- Context menu, rule: Edit, Duplicate, Enable/Disable, Make temporary, Move to top/bottom, Show as text, Simulate against this rule, Copy as pf text, Delete.
- Context menu, connection: Kill, Kill and block endpoint, Allow endpoint always, Explain, Show app, Copy address, Capture in Packet Analyser.
- Context menu, app: Block, Allow temporarily, Reset policy to declared, Open in Permissions (AD05), Show connections, Show log.
- Context menu, log row: Explain, Allow this, Add to blocklist, Filter by app/host/rule.
- Drag and drop: drag rules to reorder; drop a list file on Blocklists to import; drop a `pf.conf` on Rules to import; drag a connection row onto Blocklists to block its endpoint; drag an app onto a profile to set its override.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Rules › User: "No rules of your own. The defaults block incoming connections." with "New rule". Blocklists: "No lists" with "Import list" |
| Loading | Counters and tables show the last known values with a "refreshing" mark |
| Error | Parse error: apply disabled, error list docked under the editor. Service error: banner with details and "Retry" |
| Offline | No interface up: Overview says "No network"; everything stays editable |
| Locked | Rule set locked by an administrator or managed anchors: rows show a lock and the owner |
| Read-only | No authentication: all edit controls disabled with a single "Unlock to change" button |
| Unsaved | Status bar shows the count; closing asks "Apply, keep as draft or discard"; drafts survive restart |
| Kill switch | Red banner, content dimmed, only "Release…" is primary |
| Narrow | Sidebar overlay; tables hide low-priority columns (hits, label), inspector becomes a sheet |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar | `listView` |
| App list, endpoints, connections, log, lists, upstreams | `tableView` (virtualised) |
| Rule list with sections | `treeView` with columns |
| Text rules | `codeEdit` with a pf grammar definition |
| Charts and sparklines | `chart` |
| Policy and option pickers | `comboBox`, `toggleSwitch`, `checkBox`, `radioButton`, `spinBox` |
| Rule dialog | `dialog` with `propertyGrid`-style form |
| Diff | `codeEdit` diff mode now; FUSE-WP-38 later |
| Countdown | `progressBar` |
| Anchors | `treeView` |

Missing:

- **SE02-GAP-1**: tree-table with inline editors and drag reorder inside sections (FUSE-WP-28).
- **SE02-GAP-2**: pf grammar for `codeEdit` highlighting and an error-squiggle API fed by an external parser (FUSE-WP-37).
- **SE02-GAP-3**: stacked area chart with a time axis and live append (FUSE-WP-46).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | View models, rule model editing, round trip between visual and text, undo |
| UI | FUSE UI | app host | Pages and dialogs |
| Firewall service | C++23 | system service `svc.firewall`, privileged, starts before the network | Sole holder of the pf control handle; compiles the rule model to pf rules; loads them atomically; safe-apply timer; history; kill switch |
| Rule compiler | C++23 | library in the firewall service | Model → pf text, per section and anchor; deterministic output |
| pf check helper | C++23 wrapping the pf grammar from `pfctl` | helper, capability mode, no pf handle | Parses candidate text, returns errors and the expanded rule list; used for live checks and for the simulator |
| Simulator | C++23 | library in the check helper | Evaluates a described packet against an expanded rule list |
| Connection monitor | C++23 | helper with a read-only pf state handle and the log interface | State table, counters, log records; joins sockets to apps using the network service's records |
| Network service (platform, FORGE-WP-21) | C++23 | system service | Creates every app socket; asks the firewall service for a verdict; resolves names on the app's route; records first contact |
| Resolver | unbound (BSD-3-Clause, in base) | own process, own user, loopback only | Validation, cache, encrypted upstream, filtering zones |
| Resolver controller | C++23 | helper | Writes resolver configuration from DNS policy, reloads, reads statistics |
| List importer | C++23 | helper, capability mode, input handle only | Parses blocklist files into tables and zones |
| List fetcher | ForgeScript | app host or scheduled job, `net.host` for user-added hosts only | Downloads subscribed lists; hands bytes to the importer |
| `fwctl` | ForgeScript | command line | Same bus interface |

```
 apps ──socket request──▶ network service ──verdict?──▶ firewall service ──▶ pf (kernel)
   ▲        (FORGE-WP-21)       │  ▲                       │   │  ▲             │
   │                            │  └── allow / ask / deny ─┘   │  │ state, log  │
   └──── connected socket ◀─────┘                              │  └── connection monitor
                                 resolver controller ◀─────────┘
                                        │                    pf check helper (parse, simulate)
                                     unbound (loopback)      list importer (sandboxed parse)
 Firewall app (ForgeScript + FUSE UI) ── session bus org.pbsd.firewall.v1 ── firewall service
 SH04 prompts ◀── first-contact question ── firewall service ──▶ svc.audit, SE01, SH02
```

Two enforcement layers, both required:

1. **Network service** (per app, by name): an app in capability mode cannot open a network socket itself. The service refuses or allows by app identity, host name, port and route.
2. **pf** (per packet): default deny, system rules, kill switch, blocklist tables, inbound rules, and per-app rules matched on a packet tag the kernel sets from the socket's owner label (platform gap, section 13).

### 6.2 Data model

Stores: `/var/db/pbsd/firewall/policy.db` (SQLite, service-owned), `/var/db/pbsd/firewall/history/` (applied rule sets, text), `/var/db/pbsd/firewall/traffic.db` (rolling history). App data holds drafts and window state only.

| Entity | Fields |
|---|---|
| `rule` | `id` u64, `section` enum(system, killswitch, apps, blocklists, user, profile), `anchor` text, `seq` u32, `enabled` bool, `action` enum(pass, block_drop, block_return, match), `dir` enum(in, out, any), `quick` bool, `iface` text, `af` enum(any, inet, inet6), `proto` text, `src` addr_spec, `src_port` port_spec, `dst` addr_spec, `dst_port` port_spec, `state_policy` enum, `log` bool, `label` text, `tag` text, `tagged` text, `max_src_conn` u32, `max_src_conn_rate` text, `overload_table` text, `schedule` text, `expires_ts` i64 null, `raw` text null (verbatim advanced rule), `comment` text, `created_by` u32, `created_ts` i64 |
| `table_def` | `name` text, `kind` enum(addr, domain), `persist` bool, `source` enum(user, list, dynamic, system), `entry_count` u32 |
| `table_entry` | `table` text, `value` text (CIDR or domain), `comment` text, `expires_ts` i64 null |
| `app_policy` | `app_id` text, `profile` text (or `*`), `level` enum(blocked, declared, ask, allowed, local_only), `local_net` bool, `inbound` enum(none, loopback, local, any), `route` enum(clear, vpn, tor), `rate_up` u32, `rate_down` u32 |
| `endpoint_rule` | `id`, `app_id`, `match_kind` enum(host, suffix, addr, cidr), `match` text, `port` port_spec, `proto` text, `verdict` enum(allow, block, ask), `scope` enum(once, session, always), `expires_ts`, `source` enum(manifest, user, prompt, learning) |
| `first_contact` | `id`, `app_id`, `host` text, `addr` text, `port` u16, `route`, `ts`, `answer` enum(pending, allow_once, allow_session, allow_always, block, timeout) |
| `profile` | `name`, `inbound_default`, `local_net` bool, `discovery` bool, `dns_set` text, `blocklists` list, `builtin` bool |
| `network_binding` | `network_id` blob (from SY01), `label` text, `profile` text, `first_seen_ts` |
| `blocklist` | `id`, `name`, `kind`, `source_url` text null, `pin_sha256` blob null, `signer_fpr` text null, `updated_ts`, `entries` u32, `scope` JSON, `enabled` bool |
| `dns_upstream` | `id`, `name`, `addr`, `port` u16, `transport` enum(dot, doh, odoh, plain), `tls_name`, `spki_pin` blob, `enabled`, `set` text |
| `dns_local` | `name`, `type`, `value`, `zone` text |
| `ruleset_version` | `id`, `ts`, `author_uid`, `summary`, `blake3` blob32, `path`, `confirmed` bool |
| `conn_history` | `app_id`, `host`, `addr`, `port`, `proto`, `route`, `first_ts`, `last_ts`, `bytes_in` u64, `bytes_out` u64, `conns` u32 |
| `kill_switch` | `engaged` bool, `mode` enum(all, route), `route` text, `since_ts`, `by_uid`, `allow_apps` list |

Schema version in every database; forward migration with a copy kept. Rule-set history is plain text so it can be read with no tool at all.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| pf.conf text | yes | yes | Grammar from `hbsd/src/sbin/pfctl/parse.y` (BSD licence) built into the check helper; writer is own code | pf check helper |
| pf control interface (ioctl or netlink) | yes | yes | `libpfctl` (BSD licence) | firewall service; connection monitor (read-only) |
| pf log records (`pflog` header + packet head) | yes | no | Own reader of the fixed header; packet bytes are not parsed beyond addresses and ports | connection monitor |
| Session bus `org.pbsd.firewall.v1` | yes | yes | Schema compiler (G-08) | app host; service |
| Signed rule set `.pbsd-fwrules` (JSON model + pf text + signature) | yes | yes | Own code; FORGE signing | app host verifies; service verifies again |
| Blocklists: plain address and CIDR lists, hosts-file format, domain lists, RPZ zone text | yes | export of custom lists | Own parsers | list importer |
| Country address database (CSV ranges, signed package) | yes | no | Own reader | list importer |
| unbound configuration and control | no | yes | Own writer; control through unbound's local control socket | resolver controller |
| DNS over TLS, DNSSEC | — | — | unbound | resolver process |
| DNS over HTTPS, oblivious DoH (optional) | — | — | Own small upstream helper over the platform TLS helper | upstream helper, `net.host` to chosen resolvers only |
| CSV and JSON export | no | yes | Own | app host |

### 6.4 Integration

- Intents: handles `open` for `.pbsd-fwrules`, `pf.conf`, list files; URL scheme `firewall:` (`firewall:app/org.pbsd.mail`, `firewall:rule/20`). Emits `open` to AD05, SE03, SE01, DV05, SY01, AD04.
- Services used: `svc.audit` (write and read own records), `svc.notify`, `svc.settings`, `svc.backup`, `svc.keyring` (signing key reference), `svc.search`, `svc.portal.file`, `svc.portal.print`.
- Interface offered `org.pbsd.firewall.v1`: `Status()`, `GetRules()`, `ProposeRules(model) → diff, errors`, `Apply(token)`, `Confirm(token)`, `Restore(version)`, `Simulate(packet, which)`, `SetAppPolicy()`, `Decide(app, host, port, route) → verdict` (called by the network service), `AnswerFirstContact()`, `KillSwitch(mode)`, `BlockApp(app, ttl)` (called by SE01), `RouteGuard(route, on)` (called by SE03), `RegisterAnchor(owner)` (VM01, VM02), signals `StatusChanged`, `FirstContactPending`, `Blocked`.
- Other apps: SH04 draws prompts; SH02 hosts the tile and kill-switch toggle; SY01 supplies network identity and the metered flag; SE03 owns routes and asks for route guards; SE01 reads state and asks for app blocks; AD05 shows the same policy from the capability side; VM01 and VM02 register anchors; DV05 takes capture hand-offs.

## 7. Security and privacy

Capabilities requested by the app:

| Capability | Reason |
|---|---|
| `fs.app` | Drafts, window state |
| `fs.portal` | Import and export rule sets and lists |
| `net.none` | Default. The app itself needs no network |
| `net.host:<host>:443` | Added only when the user subscribes to a list at that host; shown at grant time |
| `svc.firewall` | Talk to the firewall service (new service, section 13) |
| `svc.audit` | Read firewall records for the Log page |
| `svc.notify`, `svc.settings`, `svc.search`, `svc.backup` | Notices, preferences, search, snapshot on apply |
| `svc.keyring` | Reference to the signing key for exported rule sets |
| `svc.portal.file`, `svc.portal.print` | Import, export, print |
| `proc.helper:list-importer` | Parse hostile list files outside the app |

The app holds no privilege. Every change is a proposal to the service, which authenticates the user through SH03.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| The rule set | Malicious local app | Ask the service to open a hole | Service accepts changes only from an authenticated user session; apps cannot call `Apply`; all changes audited |
| The rule set | Hostile blocklist or rule file | Crash or subvert the parser | List importer and check helper run in capability mode with one input handle; fuzzed; size limits |
| Per-app policy | App trying to escape its policy | Use a helper, raw sockets, another route, own resolver | Capability mode forbids sockets; helpers inherit; pf tag rules catch anything that bypasses the service; DNS leak guard |
| First-contact prompt | App spoofing or spamming prompts | Trick the user into "Always allow" | Prompt drawn by the shell on the trusted path; rate limit; default answer is block; "always" is never the default button |
| DNS answers | Network attacker | Spoofing, downgrade to plain DNS | DNSSEC validation, pinned encrypted upstream, plain fall-back off by default |
| The user's connectivity | The user's own mistake | Lock-out by a bad rule | Parse check, simulation, replay check, 30 s confirm, history |
| Traffic metadata held by the app | Anyone with the user's files | Read connection history and DNS query log | Stored in service-owned databases; query log off by default; retention limits; clear-history action |
| Kill switch | Malware | Release it silently | Release needs authentication; state shown in the panel; audited |
| pf itself | Kernel bug | Bypass | Out of scope; the network service layer still refuses app sockets |

Stored: rules, policies, history, counters, optional query log, all local. Leaves the machine: nothing, except downloads of lists the user subscribed to and DNS queries to the upstreams the user chose.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `apply.confirm_seconds` | int | 30 | Safe-apply countdown; 0 turns it off (not advised) |
| `apply.snapshot` | bool | true | Snapshot configuration before apply |
| `apply.replay_check` | bool | true | Replay the last hour before apply |
| `default.unknown_network_profile` | enum | public | Profile for unknown networks |
| `default.app_level` | enum | declared | Level for a new app with `net.user`: declared, ask, allowed |
| `prompt.mode` | enum | ask | ask, quiet, learning |
| `prompt.timeout_s` | int | 30 | First-contact time-out |
| `prompt.max_per_app_per_min` | int | 3 | Prompt rate limit |
| `learning.hours` | int | 24 | Learning-mode length |
| `local_net.default` | bool | false | Local network access for new apps |
| `log.blocked` | bool | true | Log blocked packets |
| `log.passed` | bool | false | Log passed connections (new states only) |
| `log.retention_days` | int | 7 | Detail retention |
| `history.retention_days` | int | 30 | Connection history |
| `history.rulesets_kept` | int | 100 | Rule-set versions kept |
| `dns.mode` | enum | encrypted | encrypted, recursive, network |
| `dns.dnssec` | bool | true | Validate |
| `dns.plain_fallback` | bool | false | Allow plain DNS when encrypted fails |
| `dns.query_log` | bool | false | Keep the query log |
| `dns.cache_mib` | int | 32 | Cache size |
| `blocklist.update_hours` | int | 24 | Subscribed list refresh; 0 is manual |
| `killswitch.persist` | bool | true | Stay engaged across reboot |
| `notify.on_block_foreground` | bool | false | Toast when the foreground app is blocked |
| `ui.connections_refresh_ms` | int | 1000 | Table refresh period |
| `ui.show_system_sections` | bool | false | Show generated sections in Rules |
| `ui.resolve_names` | bool | true | Show names recorded at request time |

## 9. Accessibility and languages

- A rule is announced as one sentence ("Rule 20, enabled, allow incoming TCP from the local network to port 8080, expires in one hour").
- Verdicts use icon and word (Allowed, Blocked, Asks); never colour alone.
- The countdown dialog announces the remaining time every 5 s and can be extended once by 30 s with `E`.
- Reorder has keyboard equivalents and announces the new position.
- Addresses and ports are never mirrored in right-to-left layouts; column order mirrors, cell content does not.
- pf keywords in the text editor are not translated; the visual editor and summaries are.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| First frame | under 400 ms cold |
| Verdict for a socket request (`Decide`) | under 100 µs for a cached decision; under 1 ms uncached |
| Live parse check | under 50 ms for 2 000 rules |
| Apply (compile, check, atomic load) | under 500 ms for 2 000 rules and 500 000 table entries |
| Connections table | 20 000 states, refresh in under 16 ms on the UI thread |
| Blocklist import | 1 million entries in under 5 s |
| Service idle | zero wake-ups with no traffic events subscribed; under 30 MiB |
| App memory | under 120 MiB |
| Simulator | under 5 ms per packet on 2 000 rules |

## 11. Testing and proof

Test plan (targets; nothing has run):

- **Unit**: rule model ↔ pf text round trip for every field; rule compiler determinism; expiry; profile switch.
- **Differential**: simulator verdict against real pf for generated rule sets and packets in a test jail or VM; the parse check against `pfctl -n`.
- **Golden**: `.fuseui` renders of every page and dialog.
- **Scripted UI**: the 14 top tasks through the a11y tree, including the confirm countdown and the roll-back path.
- **Fuzz**: pf text parser in the helper, list importers (each format), `.pbsd-fwrules`, pf log reader, bus messages.
- **System**: a socket from an app with `net.none` is refused before any packet; a host outside `net.host` is refused; a `tor`-route app never emits on the clear interface (packet capture on a test network); DNS leak guard; kill switch within 100 ms of the call.
- **Fault injection**: kill the service mid-apply (the old rule set must still be loaded); unplug during confirm; corrupt policy database (boot-time deny set stays).

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| `Decide` rule matcher | A socket is allowed only if a rule allows that app, host, port and route | T1 | Shared with FORGE-WP-21's target; Lean model plus ESBMC on the matcher | none known |
| Narrow-only policy | An app policy is never wider than its granted capability | T1 | Pure function over capability and policy; PRISM | none known |
| Safe-apply state machine | After any crash or time-out the loaded rule set is either the old or the confirmed new one | T1 | TLA+ model (Apalache) and crash-point tests | pf atomic load assumed |
| Rule compiler | Output parses; section order is fixed; user rules cannot precede system deny or kill switch | T2 | Contracts, bounded proof, differential tests | pf grammar size |
| Simulator | Agrees with pf on the supported rule subset | T3 | Differential tests | pf semantics are not formalised |
| pf check helper and list importer | No runtime errors on any input | T2 for own code, T3 for the imported yacc grammar | Contracts, fuzzing, sanitizers | Imported C grammar is outside the safe subset |
| Kill switch | When engaged, no non-loopback packet passes | T2 | Rule-set property check over the generated rules; system test | Depends on pf |
| App logic | Safe subset | T3 | ForgeScript checker, unit tests | Interpreter not verified |

## 12. Build plan

**Phase 1 — usable core**

| ID | Work item | Depends on |
|---|---|---|
| SE02-W01 | `svc.firewall` skeleton, schema `org.pbsd.firewall.v1`, authentication path | G-08, G-09, FORGE-WP-25, FORGE-WP-26, SH03 |
| SE02-W02 | Rule model, rule compiler, sections and anchors, boot-time deny set | pf on PBSD (network package in `docs/plan/PBSD_PLAN.md`) |
| SE02-W03 | pf check helper from the `pfctl` grammar; atomic load through `libpfctl` | SE02-W02, FORGE-WP-27, FORGE-WP-28 |
| SE02-W04 | Safe apply, history, restore | SE02-W03 |
| SE02-W05 | `Decide` interface and app policy store; integration with the network service | FORGE-WP-21, FORGE-WP-08 |
| SE02-W06 | First-contact flow with SH04 prompt, time-out, rate limit, review queue | SE02-W05, SH04 |
| SE02-W07 | Connection monitor: states, counters, log; socket-to-app join | SE02-W05, SE02-GAP-P1 (packet tag from socket label) |
| SE02-W08 | Local resolver, DNSSEC, leak guard | unbound on PBSD |
| SE02-W09 | Profiles, per-network binding, kill switch, quick-settings tile | SY01, SH02 |
| SE02-W10 | App: Overview, Apps, Connections, Rules (visual and text), Log, History | G-01, G-02, G-07, FUSE-WP-12, FUSE-WP-28, FUSE-WP-37, SE02-GAP-1, SE02-GAP-2 |
| SE02-W11 | Audit records and SE01 status feed | FORGE-WP-11, SE01 |

**Phase 2 — full feature**

| ID | Work item | Depends on |
|---|---|---|
| SE02-W12 | Simulator and "Explain" | SE02-W03 |
| SE02-W13 | Blocklists: importer, tables, scopes, allow list, subscriptions | SE02-W03 |
| SE02-W14 | Encrypted upstreams, filtering zones, per-profile DNS, per-route resolution, query log | SE02-W08, SE03 |
| SE02-W15 | Temporary rules, presets, NAT and port-forward wizard, options, limits | SE02-W02 |
| SE02-W16 | Route binding and route kill switch | SE03, SE02-GAP-P1 |
| SE02-W17 | Traffic history and charts | SE02-W07, SE02-GAP-3 |
| SE02-W18 | Anchors for VMs and jails; non-app subjects | VM01, VM02 |
| SE02-W19 | `fwctl`, shadow and conflict detection, lock-out guard | SE02-W12 |

**Phase 3 — advanced**

| ID | Work item | Depends on |
|---|---|---|
| SE02-W20 | Replay check before apply | SE02-W12, SE02-W17 |
| SE02-W21 | Signed rule-set import and export | SE04 |
| SE02-W22 | Country lists, dynamic block, queues and rate limits | SE02-W13 |
| SE02-W23 | DNS over HTTPS and oblivious DoH upstream helper | FORGE-WP-21.2 |
| SE02-W24 | Captive portal flow | IN01, SH04 |
| SE02-W25 | Learning mode suggestions, policy templates, script hooks, scheduled rules | SE02-W06 |

## 13. Open questions and risks

1. **Socket-to-app attribution in pf (new platform gap, SE02-GAP-P1).** pf matches users, interfaces and tags, not apps. Per-app packet rules need the kernel to label each socket with its app identity and to expose that label to pf as a tag. Without it, per-app enforcement rests on the network service alone, and non-app processes can only be matched by user.
2. **Route mechanism is undecided** (per-app routing table, jail-like network domain or proxy); FORGE-WP-21 marks it Unverified. SE02-W16 depends on the choice, shared with SE03.
3. `svc.firewall` is not in PLATFORM.md section 8. Needs adding.
4. pf on PBSD is a port onto a new buffer type and does not exist yet; behaviour differences from the reference pf would break the simulator's differential tests.
5. The pf grammar is yacc-generated C. It runs only in the check helper, but it is outside the safe subset; rewriting it is large.
6. First-contact prompts can become noise. Default for new apps is "declared hosts only", which never prompts; "ask" is opt-in per app.
7. Host-name policy depends on the app using the system resolver through the network service. An app that connects to a literal address is judged by address only; this is shown as such in the prompt.
8. Encrypted DNS still shows the upstream which names are asked. The app says so and offers recursive mode and the SE03 routes.
9. Country databases are inexact and age quickly; the feature is marked "best effort".
10. unbound's control interface and its availability in the PBSD base build are assumed from the reference tree, not tested.
