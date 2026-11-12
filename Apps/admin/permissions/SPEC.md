<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Permissions

By Odin Loch

| | |
|---|---|
| ID | `AD05` (from CATALOGUE.md) |
| Bundle | `org.pbsd.permissions` |
| Category | admin |
| Priority | P0 |
| Backing repos | ParanoidBSD (`pbsd/handles`, `pbsd/rights`, `pbsd/lineage`, `pbsd/kernel/capsicum_bridge.cppm`); FORGE (grant table, FORGE-WP-06; manifest-to-policy compiler, FORGE-WP-08); FUSE (UI) |
| Status | Planned. Nothing built. |

## 1. Purpose

The one place where a person sees and controls what every app can reach: its declared capabilities, each file, folder, device and host it was granted, the tree of handles derived from those grants, and the history of their use. Any grant can be narrowed or revoked here and the effect is immediate. It is for every user ("why can this app see my photos?") and for administrators and researchers who need the full handle lineage. It replaces nothing in the Plasma plan; no earlier desktop exposes this model.

## 2. Scope

In scope:

- Per-app capability lists with state (declared, granted, in use, denied, revoked).
- Per-resource grants: files, folders, devices, hosts, services.
- The handle lineage tree with revocation of any node and its descendants.
- History of use per grant, per app, per resource.
- Temporary grants (time-limited, session, once) and their expiry.
- Profiles (Strict, Balanced) and per-app overrides.
- Bulk revoke; reverse queries ("what can reach this file, device or host"); simulation ("what if I revoke").
- Export and import of policy; policy comparison.

Out of scope:

| Item | Owner |
|---|---|
| Drawing the capability prompt and the file, device and share pickers | SH04 Portals and Prompts |
| Firewall rules, blocklists, DNS policy, live connection table | SE02 Firewall (this app shows and revokes `net.*` grants; SE02 owns packet rules) |
| Routes and identity separation | SE03 Anonymity |
| Security posture score and alerts | SE01 Security Centre |
| Full audit log viewing, export and integrity | AD04 Logs (this app shows per-grant history from the same store) |
| Installing and removing apps; bundle signatures | SY05 Software |
| USB device class policy, driver (UDA) grants (SI-7) | VM05 Devices |
| VM guest capabilities (SI-8) | VM01 Virtual Machines |
| Secrets inside the keyring | SE04 Passwords and Keys |
| Process list and resource use | SY04 System Monitor |
| Unix file modes and ACLs on files | SY02 Files (shown here read-only in reverse queries) |

## 3. Users and top tasks

Personas: **Asha**, who wants to check and tidy what apps can reach; **Bo**, an administrator setting a strict policy for a shared machine; **Cal**, a security researcher tracing how a handle reached a helper; **Dee**, a developer checking that their app asks for no more than it needs.

| # | Task | Target |
|---|---|---|
| T1 | See everything one app can reach | 1 click (select the app) |
| T2 | Revoke one grant | 2 clicks (select, Revoke); undo offered for 10 s |
| T3 | Revoke everything an app holds, now | 2 clicks ("Revoke all") |
| T4 | Find which apps can reach a given folder | Drop the folder on the window, or 2 clicks + picker |
| T5 | Find which apps can use the camera or microphone | 2 clicks (Resources ▸ Devices) |
| T6 | Find which apps can contact a given host | 2 clicks + type the host |
| T7 | See what would stop working before revoking | 1 click ("What if…") |
| T8 | Turn an "always" grant into "ask every time" | 2 clicks |
| T9 | Give an app access to a folder for one hour | 3 clicks |
| T10 | Switch the whole system to the Strict profile | 2 clicks + admin approval; a diff is shown first |
| T11 | See when and how often a grant was used | 1 click (History tab on the grant) |
| T12 | Follow a handle from the user's choice down to the helper that holds it | 1 click (Lineage on the grant) |
| T13 | Revoke all network access for all apps at once | 2 clicks (Bulk ▸ Network) |
| T14 | Export the policy to review or to apply on another machine | 2 clicks |
| T15 | Review grants unused for 90 days and remove them | 2 clicks ("Unused" filter, Revoke all listed) |

## 4. Features

### 4.1 The model shown to the user

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F001 | Three levels, one view | Declared capabilities (from the manifest, PLATFORM.md section 6), grants (a capability bound to a resource and a duration), handles (live objects with rights and a lineage id). Each level links to the next. | M | 1 |
| AD05-F002 | Rights shown as words | The closed rights set is Read, Write, Execute, Grant, Duplicate, Destroy, Map, Irq (Verified: `pbsd/rights/pbsd.rights.cppm`). Shown as words with a short meaning; "Grant" and "Duplicate" are explained as "may pass this on". | M | 1 |
| AD05-F003 | Rights only narrow | Every edit control can remove rights or shorten duration, never add (SI-1, `docs/security/INVARIANTS.md`). Widening is done by the app asking again through a prompt. | M | 1 |
| AD05-F004 | No ambient authority statement | Each app page begins with what the app can reach with no grants at all: its own data, its windows, the session bus (SI-2; PLATFORM.md section 6 rule 1). | M | 1 |
| AD05-F005 | Handle kinds | Unique, Borrowed, Shared and VM handles are labelled as such (closed type set, `docs/specs/HANDLES.md`); Shared handles carry a note because the spec treats them as rare and audited. | S | 2 |
| AD05-F006 | Grant origin | Each grant shows how it arose: manifest (install), portal (user picked), service (a system service returned it), helper return, profile default, administrator. Matches the `origin` field planned in FORGE-WP-06. | M | 1 |
| AD05-F007 | Duration | Once, this session, until a time, always; with the time left for temporary ones. | M | 1 |
| AD05-F008 | State vocabulary | Declared (never asked), Granted, In use now, Denied, Revoked, Expired; one icon and word each; used everywhere. | M | 1 |
| AD05-F009 | Plain and expert modes | Plain mode shows apps, resources and grants. Expert mode adds handles, lineage ids, rights bits, policy hashes and process ids. | M | 1 |
| AD05-F010 | Explanations | Each capability has a one-line meaning, a risk note and an example of what breaks without it; from a local table, translated. | M | 1 |

### 4.2 Per-app view

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F011 | App list | Every installed bundle, system service and helper-owning app: icon, name, state badges (network, devices in use), count of grants, last active. Sort and filter. | M | 1 |
| AD05-F012 | Capability table | Rows: each declared capability with state, resource count, last use; grouped as Files, Network, Devices, Services, Processes, Background, AI. | M | 1 |
| AD05-F013 | Grants under a capability | Expand a capability to its grants: resource, rights, duration, origin, granted on, last used, use count. | M | 1 |
| AD05-F014 | Revoke a grant | One action; takes effect at once for the app and every handle derived from it (SI-6). A toast offers Undo for 10 s; undo creates a fresh grant with the same terms where the origin allows it (portal and profile grants), otherwise explains that the app must ask again. | M | 1 |
| AD05-F015 | Revoke all for an app | Revokes every non-essential grant; `fs.app` stays. The app is told and can keep running with less. | M | 1 |
| AD05-F016 | Narrow a grant | Remove Write from a read-write folder grant; restrict a folder grant to a subfolder; shorten duration. Implemented as revoke plus a derived narrower grant so lineage stays truthful. | M | 2 |
| AD05-F017 | Change prompt policy | Per capability per app: Allow, Ask every time, Ask once per session, Deny. | M | 1 |
| AD05-F018 | Deny in advance | Mark a declared capability Denied so the app's request fails without a prompt. | M | 1 |
| AD05-F019 | Helpers | Each helper of the app with the capabilities passed to it (never more than the app's, PLATFORM.md section 6 rule 2) and its live handles. | M | 2 |
| AD05-F020 | Manifest view | The manifest's capability list as shipped, with the bundle signature state and a diff against the previous installed version. | S | 2 |
| AD05-F021 | Capability change on update | When an update adds a capability the app is flagged "asks for more than before" until the user has seen it. | M | 2 |
| AD05-F022 | Running instances | Instances and their process tree; per instance: live handles count, [Revoke all], [Stop app] (hand-off to SY04). | S | 2 |
| AD05-F023 | Background permission | `bg.run` and `bg.autostart` state with what the app did in the background in the last 7 days. | M | 2 |
| AD05-F024 | Per-app network summary | Hosts granted (`net.host`), user-typed endpoints seen (`net.user` first contacts), route allowed; link to SE02 for rules. | M | 1 |
| AD05-F025 | Per-app data summary | Size of the app's own data, config and cache; [Clear cache], [Reset app data…] (hands to SY05 or SY02). | C | 3 |
| AD05-F026 | Least-privilege hint | "Declared but never used in 90 days" list per app; offers to set those to Deny. | S | 2 |
| AD05-F027 | Compare two apps | Side-by-side capability and grant comparison. | C | 3 |
| AD05-F028 | App reset | Return an app to its just-installed state: all grants and prompt answers forgotten. | M | 1 |
| AD05-F029 | System services | Services are listed in their own group with the handles they hold; essential ones cannot be revoked here and say why. | M | 2 |
| AD05-F030 | Non-bundle programs | Programs started through `proc.exec` (Terminal, IDE) and compatibility-layer apps (VM03) appear under their launcher with the confinement they run in. | S | 3 |

### 4.3 Per-resource view

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F031 | Resource classes | Files and folders, Devices, Hosts, Services, Other apps (intents), Keys (item names only). | M | 1 |
| AD05-F032 | Files and folders | Tree of every path that has a grant, with the apps under each; inherited reach is shown (a grant on a folder covers its children). | M | 1 |
| AD05-F033 | Fixed paths | `fs.path:` grants from manifests listed separately with a note that they were shown at install. | M | 1 |
| AD05-F034 | Devices | Camera, microphone, screen, location, USB classes, printers, GPU compute: who may use, who is using now (live indicator), last use. | M | 1 |
| AD05-F035 | Hosts | Every `net.host` endpoint and every first-contacted host under `net.user`, with apps, ports, route, first and last contact. | M | 1 |
| AD05-F036 | Services | For each `svc.*`: which apps may talk to it and which calls they made (counts). | S | 2 |
| AD05-F037 | Per-resource revoke | Revoke one app's grant on the resource, or all grants on it. | M | 1 |
| AD05-F038 | Protect a resource | Mark a folder, device or host "always ask" or "never share": future prompts for it are restricted or refused regardless of the app's remembered answers. | S | 2 |
| AD05-F039 | Sensitive folders preset | Keys, password store, mail store, browser profiles are pre-marked "never share by picker default; ask with a warning". | S | 2 |
| AD05-F040 | Removable and remote resources | Grants on a removable disk or network location are shown as dormant when the resource is absent and can be set to expire on removal. | S | 2 |
| AD05-F041 | Recent documents list | Remembered recent files per app (`svc.portal.file`) with forget one, forget all. | M | 2 |
| AD05-F042 | Clipboard and intents | Which apps may read clipboard history (`svc.clipboard`) and which intents each app may send to which app. | S | 3 |
| AD05-F043 | Dataset view | With AD02 data: per ZFS dataset, which apps hold handles into it (used by AD02 for export and auto-lock). | S | 2 |

### 4.4 Handle lineage tree

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F044 | Lineage tree view | A tree per root grant. Every `grant`, `borrow` and `duplicate` makes a child node (Verified model: `docs/specs/CAPSICUM_LINEAGE.md`, `docs/specs/HANDLES.md`). Node: holder (app, helper, service), resource, rights, handle kind, age, state. | M | 1 |
| AD05-F045 | Rights narrowing along a path | Each child shows which rights it lost relative to its parent; a child can never show more (SI-1). A violation would be drawn as an integrity error and reported to SE01. | M | 1 |
| AD05-F046 | Revoke a node | Revoking a node invalidates it and all descendants atomically from the holders' view (SI-6; reference implementation `LineageTree::revoke_subtree` in `pbsd/lineage/pbsd.cap.lineage.cppm`). The count of affected handles and holders is shown before confirming. | M | 1 |
| AD05-F047 | Revoked state | Revoked nodes stay visible, greyed, with time and actor, until pruned; use after revoke returns `Status::Revoked` (per `CAPSICUM_LINEAGE.md`) and each such attempt is counted on the node. | M | 2 |
| AD05-F048 | Roots | Roots are grouped by source: portal picks, manifest grants, service-issued handles, device sessions. | M | 1 |
| AD05-F049 | Cross-process edges | When a handle is passed over the bus or to a helper, the edge is labelled with the transfer (which interface, when). | M | 2 |
| AD05-F050 | Graph view | Optional graph layout for wide trees; the same data. | S | 2 |
| AD05-F051 | Find in lineage | Search by holder, resource, lineage id, rights. | M | 2 |
| AD05-F052 | Path to root | From any handle: the chain of derivations back to the user's decision or manifest line that started it. | M | 1 |
| AD05-F053 | Live updates | Nodes appear and disappear as handles are created and released; churn is rate-limited in the view (batched every 250 ms). | M | 2 |
| AD05-F054 | Depth and fan-out limits | Shows how close a tree is to platform limits. The reference tree is a fixed table of 4 096 nodes (Verified: `kMaxNodes` in `pbsd.cap.lineage.cppm`); the production limit is not set yet. | S | 2 |
| AD05-F055 | Borrow lifetime | Borrowed handles show their lender; a borrow that outlives a released lender is flagged (the spec forbids it, `HANDLES.md` closed type set table). | S | 3 |
| AD05-F056 | Capsicum mapping detail | Expert mode shows the Capsicum rights a handle maps to. The current bridge maps Read, Write, Execute and Map only (Verified: `pbsd/kernel/capsicum_bridge.cppm`); the other four rights are PBSD-only. | S | 3 |
| AD05-F057 | Snapshot of the tree | Save the current tree as a file for later comparison or for a report. | S | 3 |
| AD05-F058 | Kernel and service lineage | Lineage of handles held by system services and drivers (read-only for essential ones); UDA and VM handle trees link to VM05 and VM01 (SI-7, SI-8). | C | 3 |

### 4.5 History of use

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F059 | Per-grant history | Timeline of: granted, each use (bucketed per hour with counts), narrowed, denied attempts, revoked or expired; from `svc.audit` records (schema fields in FORGE-WP-11.4). | M | 1 |
| AD05-F060 | Per-app history | All decisions and uses for one app; filters by capability and decision. | M | 1 |
| AD05-F061 | Per-resource history | Who touched this folder, device or host, and when. | M | 2 |
| AD05-F062 | Prompt history | Every prompt shown: what was asked, what the user chose, how long the prompt was open. | M | 2 |
| AD05-F063 | Denials list | Recent denials with the reason (not declared, denied by policy, denied by user, revoked, expired); "allow next time" is offered only for policy and user denials. | M | 1 |
| AD05-F064 | Device session log | Camera, microphone, screen and location sessions with duration. | M | 2 |
| AD05-F065 | Usage heat map | Calendar heat map of a grant's or app's use per day. | S | 3 |
| AD05-F066 | Unused grants | Filter: not used in 30, 90, 365 days; bulk revoke from the list. | M | 2 |
| AD05-F067 | Unusual use hints | First use of a capability by an app, use at an unusual hour, sudden volume; labelled hints, detection proper is SE01. | C | 3 |
| AD05-F068 | Open in Logs | Any history row opens the matching record range in AD04 (`pbsd-logs:` link). | M | 1 |
| AD05-F069 | History retention | Follows the audit class retention in AD04; this app keeps no second copy. | M | 1 |
| AD05-F070 | Weekly digest | Optional notification: new grants, new hosts, devices used, grants unused for 90 days. Local only. | S | 3 |

### 4.6 Temporary grants and expiry

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F071 | Grant for a time | From here or from the prompt (SH04): 5 min, 1 h, until logout, until a date and time. Uses the `expires_ns` field planned in FORGE-WP-06. | M | 2 |
| AD05-F072 | Active temporary grants list | All grants with an expiry, sorted by time left; [Extend] (asks), [End now]. | M | 2 |
| AD05-F073 | Expiry is revocation | At expiry the grant's lineage node is revoked exactly as a manual revoke (SI-6); the app is notified. | M | 2 |
| AD05-F074 | Grant from here | Give an app a folder, device or host from this app without waiting for it to ask: pick app, resource, rights, duration. The resource is chosen through the portal so the user's own authority is the root. | S | 2 |
| AD05-F075 | Session grants end at lock (option) | Device and network session grants can end at screen lock, not only at logout. | S | 2 |
| AD05-F076 | Idle expiry | "Forget this grant if unused for N days" per grant or as a profile default. | S | 2 |
| AD05-F077 | One-shot tokens | A grant valid for one operation (open one file once), shown as "once" and expiring on first release. | M | 2 |
| AD05-F078 | Expiry notice | Optional notification 5 min before a temporary grant ends for an app in use. | C | 3 |
| AD05-F079 | Emergency mode | One switch: revoke all device and network grants for all apps now, and deny new ones until switched off. Also reachable from SH02 quick settings. | S | 2 |

### 4.7 Profiles and policy

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F080 | Profiles | Strict and Balanced (chosen at install, AD01). A profile is a table: capability → default prompt policy, allowed durations, idle expiry, whether "always" may be remembered. | M | 1 |
| AD05-F081 | Balanced defaults | `fs.portal` remembered grants allowed; devices ask on first use then per session; `net.host` from manifest allowed and shown; `net.user` allowed with first-contact logging; `bg.*` ask. | M | 1 |
| AD05-F082 | Strict defaults | No "always" for devices or folders (session at most); `net.*` asks per app at first use even when declared; `proc.exec`, `bg.autostart`, `ai.local` ask every session; idle expiry 30 days; fixed paths (`fs.path`) need confirmation after each update. | M | 1 |
| AD05-F083 | Profile diff before switching | Lists every existing grant that the new profile would revoke or shorten; nothing changes until confirmed; needs admin approval. | M | 1 |
| AD05-F084 | Per-app overrides | An app may be stricter or (with admin approval) looser than the profile for named capabilities; overrides are listed and countable. | M | 2 |
| AD05-F085 | Custom profiles | Copy a profile, edit the table, name it; assign per user. | S | 3 |
| AD05-F086 | Per-user policy | On multi-user machines each user has their own grants; the administrator can set a ceiling profile no user may exceed. | S | 2 |
| AD05-F087 | Category rules | Rules by app category or signer: "apps not signed by the PBSD key never get `proc.exec`". | S | 3 |
| AD05-F088 | Policy lock | An administrator can lock named rules so the user cannot loosen them (shared or managed machines). | S | 3 |
| AD05-F089 | Policy hash | The effective policy for each app has a hash; the same hash appears in audit records (`policy hash` in FORGE-WP-11.4) so a decision can be tied to the policy in force. | M | 2 |
| AD05-F090 | Policy source view | For any effective setting: which layer set it (manifest, profile, category rule, per-app override, administrator lock). | M | 2 |
| AD05-F091 | Restore defaults | Reset one app, one capability across apps, or everything to the profile. | M | 1 |
| AD05-F092 | Policy history | Every policy change with before and after, actor and time; undo of the last change. | S | 2 |

### 4.8 Bulk actions, reverse queries, simulation

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F093 | Bulk revoke by filter | Select grants by any filter (capability, resource class, app, origin, age, unused) and revoke the set; the confirmation lists counts per app. | M | 1 |
| AD05-F094 | Bulk presets | All network; all devices; all remembered folders; everything for apps not opened in 90 days; everything from one signer. | M | 2 |
| AD05-F095 | Bulk change of prompt policy | Set a capability to Ask or Deny across selected apps. | S | 2 |
| AD05-F096 | Reverse query: file or folder | "What can reach this?" Result: apps with a grant on it or on an ancestor, the rights, via which grant, live handles now, plus services that reach it by design (backup, search index, thumbnailer) listed separately. | M | 1 |
| AD05-F097 | Reverse query: device | Apps allowed, apps using now, and the path through the portal service. | M | 1 |
| AD05-F098 | Reverse query: host | Apps with `net.host` for it, apps with `net.user` that have contacted it, routes used; "could contact" (any `net.user` app) is listed as potential, separate from actual. | M | 1 |
| AD05-F099 | Reverse query: service | Apps that may call a service and what the service can reach on their behalf (confused-deputy view). | S | 3 |
| AD05-F100 | Transitive reach | Follows chains: app → helper, app → service that acts for it, app → other app by intent with a passed handle. Each hop is shown; depth limit 6. | M | 2 |
| AD05-F101 | Reverse query from other apps | SY02 ("Who can access this?" on a file), VM05 (device), SE02 (host) open this app with the query filled in. | M | 2 |
| AD05-F102 | Saved queries | Keep a reverse query and re-run it; optional alert when its answer grows. | S | 3 |
| AD05-F103 | Simulation: what if I revoke | For a grant, a lineage node, a set or a profile switch: lists handles that would die, holders affected, operations in progress (open files with unsaved state where the app reports it), features of the app that depend on it (from the manifest's reasons). No change is made. | M | 2 |
| AD05-F104 | Simulation: what if I grant | Shows what new reach a proposed grant creates, including transitive reach through helpers and services. | S | 3 |
| AD05-F105 | Simulation: policy import | Dry run of an imported policy: adds, removals, conflicts. | S | 2 |
| AD05-F106 | Impact warnings | Revoking something an app is using right now warns "Photos has 3 files open here; unsaved edits may be lost" and offers "ask the app to close them first" (a polite request with a 5 s limit, then revoke). | M | 2 |
| AD05-F107 | Undo window | Bulk and single revokes have a 10 s Undo where re-granting is within the user's authority; after that the app must ask again. | M | 1 |

### 4.9 Export, import, reporting

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F108 | Export policy | JSON file: profile, rules, per-app overrides, remembered grants (paths optional), schema version, optional Ed25519 signature. | M | 2 |
| AD05-F109 | Export as report | Human-readable Markdown or PDF: per app, what it can reach and why; for review or records. | S | 2 |
| AD05-F110 | Scrubbed export | Replace user names and home paths with placeholders (same rules as AD04). | S | 2 |
| AD05-F111 | Import policy | With dry-run simulation, signature check, admin approval; imported grants never exceed what the importing user could grant themselves. | M | 2 |
| AD05-F112 | Diff two policies | Current against a file, or two files. | S | 3 |
| AD05-F113 | Backup integration | The policy export is part of AD03's system state set; restore goes through the same import path. | M | 2 |
| AD05-F114 | Lineage export | A lineage tree as JSON or Graphviz DOT text. | C | 3 |
| AD05-F115 | Machine-readable query | A `permissions` command: list, query, revoke, simulate, export; same service calls and approvals. | S | 3 |
| AD05-F116 | Print | Print the report through the print portal. | C | 3 |

### 4.10 Safety, integration, settings

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F117 | Own authority is minimal | This app holds no handle to any user file, device or host. It sends requests to the policy service; revocation is done by the service and the kernel. | M | 1 |
| AD05-F118 | Trusted identity | The window carries the shell's trusted-app mark so a look-alike app cannot pose as Permissions; changes that loosen policy need an SH03 confirmation. | M | 1 |
| AD05-F119 | Tightening needs no approval | A user may always revoke or narrow their own grants without admin approval; loosening locked rules needs it. | M | 1 |
| AD05-F120 | Essential grants | Grants the session needs to function (compositor, shell, portals) are shown, locked, with the reason; they cannot be revoked from the GUI. | M | 1 |
| AD05-F121 | Self-lockout guard | Revoking this app's own `svc.policy` access or the shell's portal access is refused; recovery is in AD01 repair mode ("reset capability grants"). | M | 1 |
| AD05-F122 | Every change audited | Each revoke, narrow, policy change, import and query under admin scope is written to `svc.audit`. | M | 1 |
| AD05-F123 | Deep links | `pbsd-permissions:` URLs select an app, a grant, a resource or a query; used by prompts ("Manage…"), AD04, SE01, SE02, SY02, SY04, SY05. | M | 1 |
| AD05-F124 | Indicator hand-off | Clicking the header `[net]` badge or a device indicator in any app can open that app's page here. | S | 2 |
| AD05-F125 | Notifications | Optional: "an app was granted a new capability", "a temporary grant ended", "an update asks for more". | S | 2 |
| AD05-F126 | Search | One search box over apps, capabilities, resources, hosts and lineage ids. | M | 1 |
| AD05-F127 | Local assistant help | With `ai.local`: "Explain what this app can do" produces a plain summary from the grant list; never changes policy. | C | 3 |
| AD05-F128 | Developer view | For an app under development (from DV01): requested-versus-used capabilities over a test run, to trim the manifest. | C | 3 |
| AD05-F129 | First-run summary | After install, a one-page summary of the chosen profile and the handful of grants that exist (links from AD01 first run). | S | 2 |

## 5. GUI

Layout pattern: Three-pane master-detail (sidebar of sections, list, detail) with an inspector.

### 5.1 Window map

```
Main window
├─ Sidebar: Apps · Resources (Files · Devices · Hosts · Services) · Lineage · History ·
│           Temporary · Profiles · Queries · Emergency switch
├─ Content
│   ├─ Apps: app list ▸ app page (Capabilities · Grants · Helpers · History · Manifest)
│   ├─ Resources: class list ▸ resource tree ▸ who can reach
│   ├─ Lineage: roots list ▸ tree or graph
│   ├─ History: decisions · prompts · device sessions · denials · unused
│   ├─ Temporary: active time-limited grants
│   ├─ Profiles: profile table · overrides · rules · policy history
│   └─ Queries: reverse query form · results · saved queries
├─ Inspector (F4): selected grant or handle (rights, origin, duration, lineage path, last use)
├─ Status bar: totals · live handle count · profile name · scope (me / all users)
├─ Dialogs
│   ├─ Revoke (impact summary) · Revoke all for app · Bulk revoke (counts) · Narrow grant
│   ├─ What if… (simulation result) · Grant from here · Extend temporary grant
│   ├─ Switch profile (diff) · Edit rule · Per-app override
│   ├─ Export policy · Import policy (dry run) · Compare policies
│   └─ Preferences
├─ Sheets: Capability explanation · Path to root · Manifest diff · Transitive reach
└─ Popovers: Rights legend · State legend · Duration picker · Undo toast
```

### 5.2 Main window

Shown at 1280×800, Apps ▸ Photos ▸ Capabilities. States: empty, loading, error, read-only, locked (5.6).

```
+----------------------------------------------------------------------------------------+
| [=] Permissions — Photos                [search Ctrl+K]            ★trusted [▁][▢][✕]  |
+--------------+-----------------------+-------------------------------------+-----------+
| (1) Apps   > | (2) [filter…       ]  | (3) Photos 0.4.2  ✓ signed · running| (6) GRANT |
|  Resources   |  Assistant        3   |  Can always reach: its own data.    | ~/Pictures|
|   Files      |  Backup          12   |  [Revoke all] [Reset app] [What if…]| Rights    |
|   Devices    |  Browser    ⬤net 41   | [Capabilities][Grants][Helpers][History][Manifest]| | Read ✓ |
|   Hosts      |  Files            –   +-------------------------------------+  Write ✓  |
|   Services   |  Mail       ⬤net  6   | (4) FILES                           |  Grant ✓  |
|  Lineage     | > Photos    ⬤cam  5   |  v fs.portal        Granted · 3     | Origin    |
|  History     |  Terminal         2   |    > ~/Pictures     rw  always  2h  |  portal,  |
|  Temporary 2 |  Text Editor      1   |      ~/Downloads/a.jpg r  once  3d  |  you chose|
|  Profiles    |  Video            2   |      /media/sd-card  r  session  –  |  3 Aug    |
|  Queries     |  …                    |  DEVICES                            | Duration  |
|              |                       |    dev.camera       In use now ⬤    |  always   |
| ------------ |  Show: (•) Apps       |    dev.location     Denied          | Used 412× |
| (7) [Emergency  |     ( ) Services   |  SERVICES                           | Handles 7 |
|   stop: off] |       ( ) All         |    svc.thumbnail    Granted         | [Lineage] |
|              |                       |  AI   ai.local      Declared, never asked        |
|              |                       | (5) [Revoke] [Narrow…] [Ask every time v] [Deny] |
+--------------+-----------------------+-------------------------------------+-----------+
| (8) 142 grants · 61 apps · 318 live handles     Profile: Balanced     Scope: me        |
+----------------------------------------------------------------------------------------+
```

1. Sidebar, 180 px, `listView` with nested Resources entries; "Temporary" shows the count of active time-limited grants.
2. App list, 220 px, `modelListView`: name, live badges (network, camera, microphone, screen, location; each an icon plus a word in the tooltip and a11y name), grant count. Radio group switches between apps, services, all.
3. App header, `panel`: version, signature state, running state, the fixed "can always reach" line, three app-level actions. "Revoke all" is outlined `--danger`, never the default.
4. Capability table, `treeView` with columns State, Count or Rights, Duration, Last used; grouped by class; expanding a capability lists its grants. Live use shows a dot and the words "In use now".
5. Action row for the selected row; controls only narrow (no "add rights").
6. Inspector, 240 px, `propertyGrid`: rights as ticked words, origin sentence, duration, use count, live handle count, [Lineage] jumps to the tree at this root.
7. Emergency stop `toggleSwitch` (asks once, then acts).
8. Status bar: totals, profile (click opens Profiles), scope (click asks for admin approval to see all users).

### 5.3 Other views and dialogs

**Lineage tree (content region, 1060×660)**
```
| Root: ~/Pictures  (portal pick by you, 3 Aug)        View (•) Tree ( ) Graph  [Find…] |
| Holder                   | Kind     | Rights            | Lost vs parent | Age  | State |
| v Photos (app host)      | Unique   | Read Write Grant  | –              | 101d | live  |
|   v Photos (engine)      | Borrowed | Read Write        | Grant          | 2h   | live  |
|       thumbnail service  | Borrowed | Read              | Write          | 2h   | live  |
|   v raw-helper           | Unique   | Read              | Write Grant    | 14m  | live  |
|     > raw-helper (dup)   | Unique   | Read              | –              | 14m  | live  |
|   ~ export-helper        | Unique   | Read Write        | Grant          | 3d   | revoked 9 Nov by you |
| (1) Path to root: you ▸ file portal ▸ Photos ▸ raw-helper ▸ raw-helper (dup)          |
| (2) [Revoke this and 0 below] [Revoke from parent (2 handles)] [What if…] [Copy id]   |
```
`treeView` with columns. (1) breadcrumb of the selected node's derivation. (2) revoke buttons state the number of handles affected. Expert mode adds columns Lineage id, PID, Capsicum rights. States: empty ("No live handles; this grant is not in use"), revoked nodes greyed with `~`.

**Reverse query (content region)**
```
| What can reach…  (•) File or folder ( ) Device ( ) Host ( ) Service                   |
| [/home/asha/Documents/tax        ] [Choose…]   [x] include parents [x] transitive     |
| CAN REACH NOW (grants)                                                                |
|  Writer        Read Write  via grant on ~/Documents (always)   2 live handles [Revoke]|
|  Backup        Read        via svc.backup (system service, by design)                 |
|  Search index  Read        via svc.search (by design)          [Exclude folder…]      |
| COULD REACH IF YOU PICK IT (will prompt): 14 apps with fs.portal            [List]    |
| CANNOT REACH: 44 apps                                                                 |
| HAS REACHED (last 90 days): Writer 212×, Document Viewer 3× (grant expired)  [History]|
| [Save query] [Protect this folder…] [Export result]                                   |
```
Results in a grouped `treeView`. The three groups (can, could, has) are always shown so "no grant now" is not mistaken for "never accessed".

**What if… (simulation dialog, 720×480)**
```
| If you revoke Photos ▸ ~/Pictures (read, write, always)                               |
| These stop at once:                                                                   |
|   7 handles in 3 processes: Photos (4), raw-helper (2), thumbnail service (1)         |
|   3 files open for writing — Photos reports 1 unsaved edit   ! may be lost            |
| These features of Photos need it (from its manifest): library, import, edit           |
| Photos can ask again; you will be prompted.                                           |
| Nothing has been changed.     [Close] [Ask Photos to close files, then revoke] [Revoke now] |
```

**Bulk revoke (dialog)**
```
| Revoke 37 grants?                                                                     |
| Filter: capability = net.*  ·  all apps                                               |
| Browser 12 · Mail 6 · Feeds 9 · Messenger 4 · Weather 3 · Maps 3                      |
| 5 apps are using the network now and will lose their connections.                     |
| [ ] Also set these to "Ask every time"                  [Cancel] [What if…] [Revoke 37]|
```

**Profiles**
```
| Active: Balanced   [Switch to Strict…]  [Duplicate…]                 Overrides: 4     |
| Capability      | Balanced            | Strict                | This system           |
| fs.portal       | remember allowed    | session at most       | Balanced              |
| dev.camera      | ask, per session    | ask every time        | Balanced              |
| net.user        | allow, log first    | ask per app           | stricter: ask (yours) |
| proc.exec       | ask                 | ask every session     | Balanced              |
| bg.autostart    | ask                 | ask every session     | Balanced              |
| idle expiry     | 180 days            | 30 days               | 90 days (yours)       |
```
`tableView`; the last column is editable (`comboBox` cells) and shows the source of each value. "Switch to Strict…" opens the diff dialog: grants revoked (count per app), grants shortened, prompts added.

**History ▸ per-grant**
```
| Photos ▸ ~/Pictures                                   [30 days v] [Open in Logs]      |
| (heat strip: uses per day)  ▁▂▁▁▅█▂▁▁▁▃▂▁▁▁▁▁▂▇▂▁▁                                    |
| 12 Nov 14:02  used 38× (read 30, write 8)                                             |
| 11 Nov        used 114×                                                               |
| 9 Nov  10:15  derived handle revoked: export-helper (by you)                          |
| 3 Aug  09:40  granted — you chose the folder in the picker, "always"                  |
```

**Temporary grants** — `tableView`: App, Capability, Resource, Rights, Ends in (countdown), Origin; row actions [End now] [Extend…]; a toolbar toggle "end session grants at lock".

**Grant from here (dialog)** — App `comboBox`; Resource [Choose…] (file portal, device list or host field); Rights `checkBox` set limited to what the user holds; Duration (`radioButton`: once, 5 min, 1 h, session, until `dateTimeEdit`, always if the profile allows); [Grant].

**Import policy (dialog)** — file, signature state, dry-run table (Adds, Removes, Conflicts, Ignored: beyond your authority), [Cancel] [Apply].

### 5.4 Menus

| Menu | Items (shortcut) |
|---|---|
| File | New window (`Ctrl+N`) · Export policy… (`Ctrl+E`) · Export report… (`Ctrl+Shift+E`) · Import policy… (`Ctrl+O`) · Compare policies… · Print report… (`Ctrl+P`) · Close (`Ctrl+W`) · Quit (`Ctrl+Q`) |
| Edit | Undo last change (`Ctrl+Z`) · Copy (`Ctrl+C`) · Copy as JSON · Select all (`Ctrl+A`) · Find (`Ctrl+F`) · Preferences (`Ctrl+,`) |
| View | Apps (`Ctrl+1`) · Resources (`Ctrl+2`) · Lineage (`Ctrl+3`) · History (`Ctrl+4`) · Temporary (`Ctrl+5`) · Profiles (`Ctrl+6`) · Queries (`Ctrl+7`) · Expert mode (`Ctrl+Shift+X`) · Show services · Show revoked · Sidebar (`F9`) · Inspector (`F4`) · Refresh (`F5`) |
| Grant | Revoke (`Delete`) · Revoke with descendants from parent (`Shift+Delete`) · Narrow… (`Ctrl+R`) · Change duration… (`Ctrl+D`) · Ask every time · Deny · Grant from here… (`Ctrl+G` is find-next; uses `Ctrl+Shift+G`) · What if… (`Ctrl+I`) · Show lineage (`Ctrl+L`) · Show history (`Ctrl+H`) |
| App | Revoke all… (`Ctrl+Shift+Delete`) · Reset app… · Show manifest · Show helpers · Open in System Monitor · Open in Software · Open in Firewall |
| Query | What can reach a file… (`Ctrl+Shift+F`) · …a device · …a host · …a service · Save query · Unused grants (`Ctrl+U`) |
| Tools | Bulk revoke… (`Ctrl+B`) · Emergency stop (`Ctrl+Shift+Esc`) · Switch profile… · Restore defaults… · Policy history |
| Help | Help (`F1`) · How capabilities work · Rights legend · Keyboard shortcuts · About |

### 5.5 Keyboard and pointer

`Delete` revokes the selected grant (with Undo toast); `Enter` opens it in the inspector; `Right`/`Left` expand and collapse tree nodes; `Alt+Up` goes to the parent node in lineage; `Ctrl+Enter` runs a query; `Space` toggles a rights tick (only to off).

Context menus: app (Revoke all…, Reset app…, What if…, Open in System Monitor, Copy summary); capability (Ask every time, Ask per session, Deny, Restore default, Explain); grant (Revoke, Narrow…, Change duration…, What if…, Show lineage, Show history, Open resource in Files, Copy); lineage node (Revoke this and below, Revoke from parent, Path to root, Find holder in System Monitor, Copy lineage id); resource (Who can reach, Protect…, Revoke all on this, Open in Files / Devices / Firewall); history row (Open in Logs, Filter by this app, Filter by this resource).

Drag and drop: a file or folder dropped from SY02 runs "what can reach this" (the drop passes a reference for the query only and conveys no access to this app); an app icon dropped on the window selects the app; a grant row dragged to the "Temporary" sidebar item opens Change duration. No drop ever creates a grant without the Grant dialog.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | New system: "No app has been granted anything yet." Per app with no grants: the "can always reach" line and the declared list in state Declared. |
| Loading | App list from the bundle registry at once; grant counts fill in; lineage shows a skeleton until the service answers (budget 300 ms). |
| Error | Policy service unreachable: banner, read-only cached view marked stale, [Retry]. A failed revoke is shown in `--danger` on the row with the reason and is never reported as done. |
| Offline | Not applicable. |
| Locked | Rules locked by an administrator show a lock and who locked them. |
| Read-only | Another user's grants (admin scope) are read-only unless approved for change. |
| Integrity error | A lineage node whose rights exceed its parent, or a live handle with no known root: banner in `--danger`, the node is marked, SE01 is notified. |
| Narrow (< 640) | Sidebar overlay; app list and app page become a stack (back with `Alt+Left`); inspector as a sheet. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar, app list | `listView`, `modelListView` |
| Capability table, lineage tree, reverse-query results, resource tree | `treeView` with columns (FUSE-WP-28) |
| Profiles, temporary grants, history tables | `tableView` |
| Inspector | `propertyGrid`, `link` |
| App page tabs | `tabWidget` |
| Lineage graph view | `nodeGraph` (read-only; FUSE-WP-50) |
| Heat strip and usage charts | `chart` (FUSE-WP-46) |
| Duration picker | `radioButton`, `dateTimeEdit` |
| Emergency stop, policy toggles | `toggleSwitch`, `comboBox` |
| Explanations, help | `markdownView` |
| Dialogs, undo | `dialog`, toast (FUSE-WP-30) |

Missing: `AD05-GAP-1` automatic tree and layered layout for `nodeGraph` (shared with `AD02-GAP-2`); `AD05-GAP-2` calendar heat-map chart type; `AD05-GAP-3` the shell's "trusted app" window mark (SH05 / compositor feature, not a widget).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | View models, filters, simulation presentation, export formatting |
| UI | FUSE UI | app host | All views |
| Policy service (`svc.policy`, Proposed) | C++23 | system daemon (per-user instance for user grants, system instance for ceilings and locks) | Policy store; remembered grants; profile evaluation; answers "may app X have Y" for the host and portals; revoke and narrow requests; lineage metadata index; reverse and transitive queries; simulation |
| Lineage index (inside the policy service) | C++23 | service | Side table: `LineageId` → holder, resource, rights, parent, origin, times. Fed by app hosts (FORGE-WP-06.4 lineage export), portals, services and the kernel bridge |
| App host grant table | C++23 (FORGE, Proposed in FORGE-WP-06) | each app host | Per-host `grant_table`: derive, check, revoke; receives revoke messages from the policy service |
| Kernel lineage and Capsicum bridge | C++23 kernel modules | kernel | `pbsd.cap.lineage` tree and `capsicum_bridge` (Verified reference code; kernel integration is Wave 4 per `CAPSICUM_LINEAGE.md`) |
| Portals (SH04) | — | shell | Create root grants when the user picks; draw prompts using policy answers |
| `svc.audit` | C++23 | system daemon | Records decisions and uses; source of history |
| Report helper | ForgeScript | app host | Markdown and JSON report; PDF via the print pipeline |

```
 Permissions app ──bus──► svc.policy ──revoke/narrow──► app hosts (grant tables) ──► handles die
        ▲   queries          │  ▲                        portals (SH04), services
        │                    │  └── lineage export, grant events ──┘
        │ history            ▼
     svc.audit ◄──── decisions, uses          kernel: LineageTree.revoke(id) via capsicum bridge
```

Revocation path: the app sends `revoke(grant_id)`; the policy service marks the policy record, sends the revoke to the owning host's grant table (which closes descriptors and fails pending operations, FORGE-WP-06.3) and to the kernel lineage node (which invalidates derived descriptors in other processes). The service reports success only when both layers have acknowledged; a partial result is shown as an error and raised to SE01.

### 6.2 Data model

| Entity | Fields |
|---|---|
| `AppRecord` | `bundle_id: string`, `name: string`, `version: string`, `signer: string`, `signature_ok: bool`, `kind: enum{app,service,helper,launcher}`, `declared: DeclaredCap[]`, `policy_hash: bytes32`, `running: bool`, `instances: u32` |
| `DeclaredCap` | `capability: string` (e.g. `fs.portal`, `net.host:mail.ex.org:993`), `reason: string` (from the manifest), `added_in: string` (version) |
| `Grant` | `id: GrantId {slot: u32, generation: u32}` (shape from FORGE-WP-06), `bundle_id: string`, `capability: string`, `resource: ResourceRef`, `rights: RightsMask`, `origin: enum{manifest,portal,service,helper_return,profile,admin}`, `duration: enum{once,session,until,always}`, `expires_ns: i64?`, `created_ns: i64`, `last_used_ns: i64?`, `use_count: u64`, `state: enum{active,dormant,revoked,expired}`, `lineage_root: LineageId`, `decided_by: string` |
| `RightsMask` | `u32` bit set: `Read=1`, `Write=2`, `Execute=4`, `Grant=8`, `Duplicate=16`, `Destroy=32`, `Map=64`, `Irq=128` (Verified values: `pbsd/rights/pbsd.rights.cppm`) |
| `ResourceRef` | `class: enum{file,folder,device,host,service,app,key}`, `id: string` (stable object id for files, device id, `host:port`, service name), `display: string`, `dataset: string?` |
| `LineageNode` | `id: LineageId` (`u64`, 0 invalid, Verified type), `parent: LineageId`, `grant: GrantId`, `holder: HolderRef`, `handle_kind: enum{unique,borrowed,shared,vm}`, `rights: RightsMask`, `created_ns: i64`, `via: string?` (interface of transfer), `revoked: bool`, `revoked_ns: i64?`, `revoked_by: string?`, `post_revoke_attempts: u32` |
| `HolderRef` | `bundle_id: string`, `helper: string?`, `pid: u32`, `instance: u64` |
| `PolicyRule` | `scope: enum{profile,category,signer,app,user_ceiling}`, `match: string`, `capability: string`, `prompt: enum{allow,ask_once,ask_session,ask_always,deny}`, `max_duration: enum`, `idle_expiry_days: u16?`, `locked: bool`, `set_by: string`, `set_ns: i64` |
| `Profile` | `name: string`, `base: enum{strict,balanced,custom}`, `rules: PolicyRule[]`, `version: u32` |
| `Protection` | `resource: ResourceRef`, `mode: enum{always_ask,never_share,warn}`, `note: string` |
| `SimulationResult` | `revoked_nodes: LineageId[]`, `holders: HolderRef[]`, `open_writes: u32`, `unsaved_reported: u32`, `features_affected: string[]`, `policy_changes: PolicyDiff[]` |
| `ReachAnswer` | `resource: ResourceRef`, `can: ReachEntry[]`, `could: string[]` (bundle ids), `by_design: ReachEntry[]`, `has: {bundle_id, count, last_ns}[]` |
| `ReachEntry` | `bundle_id: string`, `rights: RightsMask`, `via: GrantId[]` (hop chain), `live_handles: u32` |
| `PolicyExport` | `schema: u32`, `created_ns: i64`, `machine: string?`, `profile: Profile`, `overrides: PolicyRule[]`, `protections: Protection[]`, `grants: Grant[]?`, `signature: bytes64?` |

Stores: the policy service keeps `policy.sqlite` per user under the user's config dataset and a system store for ceilings and locks (both schema-versioned, both in AD03's system state set). Live lineage is in memory in the service, rebuilt at session start from host reports; revoked nodes are kept for 24 h then live on only as audit records. This app stores only view state and saved queries in `~/.local/share/org.pbsd.permissions/`.

File identity: grants on files refer to stable object identities (dataset GUID + object number) with the path kept for display, so a rename does not silently move or lose a grant (Proposed; needs portal support).

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| `svc.policy` bus interface (schema-compiled records) | yes | yes | PLATFORM.md section 7 schema compiler (FORGE-WP-26) | app host and service |
| App manifest (`manifest.json`) | yes | no | FORGE JSON; read by the service after the host verified the bundle signature | policy service |
| Policy export (`.pbsd-policy.json` + optional `.sig`) | yes | yes | FORGE JSON, Ed25519 over BLAKE3 (V14 scheme) | app host for display; policy service validates before applying |
| Lineage export from app hosts | yes | no | schema-compiled (FORGE-WP-06.4) | policy service |
| Kernel lineage introspection | yes | revoke only | Proposed kernel interface; none exists (see section 13) | policy service |
| `svc.audit` records | yes | append own | schema-compiled | via `svc.logs` / `svc.audit` |
| Report | no | Markdown, JSON, PDF (G-16), DOT text | own | app host |
| `pbsd-permissions:` URLs | yes | yes | own parser (strict grammar) | app host |

### 6.4 Integration

- Intents handled: `open` and `view` for `application/vnd.pbsd.policy+json`; `search`; URL scheme `pbsd-permissions:` (`app/<bundle>`, `grant/<id>`, `reach?file=…|device=…|host=…`, `lineage/<id>`). Emitted: `open` to AD04 (history in Logs), SE02 (host rules), SY04 (process), SY05 (app page), SY02 (resource), VM05 (device), VM01 (VM handles), UT05; `share`, `print` for reports.
- Services: `svc.policy` (Proposed), `svc.audit` (read history, write own actions), `svc.settings`, `svc.notify`, `svc.portal.file` (pick a resource for a query or a grant-from-here), `svc.portal.print`, `svc.model` (optional).
- Used by: SH04 prompts ("Manage…" link, and policy answers come from the same service); SH02 (emergency stop, indicators); AD01 (profile choice at install, repair-mode reset); AD02 (who holds handles into a dataset; revoke before auto-lock); AD03 (policy export and restore); SE01 (posture inputs: over-broad grants, integrity errors); SY02 ("Who can access this?").
- Verified from the repo: the closed handle type set and operations (`docs/specs/HANDLES.md`), the rights bit set (`pbsd/rights/pbsd.rights.cppm`), the lineage tree API `create_root`, `create_child`, `revoke`, `valid`, `is_revoked`, `parent`, `is_ancestor_of` and its fixed capacity (`pbsd/lineage/pbsd.cap.lineage.cppm`), the `grant`/`duplicate`/`narrow`/`borrow` methods on `UniqueHandle` (`pbsd/handles/pbsd.handles.cppm`), the Capsicum bridge's four-right mapping (`pbsd/kernel/capsicum_bridge.cppm`), the SI harness (`pbsd/tests/si_harness.cpp`), invariants SI-1, SI-2, SI-6, SI-7, SI-8 (`docs/security/INVARIANTS.md`), and the revoke-race priority surface (`docs/security/THREAT_MODEL.md`). Planned elsewhere, not built: FORGE-WP-06 grant table and lineage export, FORGE-WP-08 manifest-to-policy compiler, FORGE-WP-11 audit stream. Proposed here: `svc.policy`, the lineage index, reverse queries, simulation, profiles, every view.

## 7. Security and privacy

Capabilities requested:

| Capability | Reason |
|---|---|
| `fs.app` | View state and saved queries |
| `fs.portal` | Choose a file or folder for a reverse query or a grant-from-here; save exports. Picked items are used as references; the app does not read their content |
| `net.none` | Never uses the network |
| `svc.policy` (Proposed) | All reads and changes of policy and grants |
| `svc.audit` | Per-grant and per-app history |
| `svc.settings`, `svc.notify` | Preferences; optional digests and notices |
| `svc.portal.print`, `dev.print` | Print reports |
| `ai.local` (optional, off) | "Explain what this app can do" |

The app requests no `proc.*`, no `dev.*` beyond printing, and no helper. It holds the Read right on nothing but its own data.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| The policy itself | A malicious app | Call `svc.policy` to grant itself more | The service accepts loosening calls only from the Permissions bundle identity (verified by the host signature and bus peer credentials) and only with an SH03 confirmation; any app may ask the service to tighten or revoke its own grants |
| The user's trust in this window | A look-alike app | Fake "Permissions" UI that tricks the user into approving | Shell-drawn trusted mark (`AD05-GAP-3`); real prompts and confirmations are drawn by the shell, not by apps (PLATFORM.md section 6 rule 3) |
| Completeness of revocation | Race between revoke and a concurrent grant or duplicate | A derived handle escapes the revoked subtree | Named priority surface in `docs/security/THREAT_MODEL.md` ("subtree invalidation vs concurrent `grant`"); the kernel bridge must hold the same locks as descriptor-table mutation (`CAPSICUM_LINEAGE.md` threat notes); the service confirms both layers; SI-6 is a T1 proof target |
| Truthfulness of the display | A compromised app host | Under-report its handles in the lineage export | Kernel-side lineage is the authority for descriptors; host reports are cross-checked against it; mismatches raise an integrity error. Until kernel introspection exists the display is "as reported by hosts" and says so in expert mode |
| Privacy of what the user reaches | Other local users | Read another user's grant list (paths, hosts) | Per-user policy stores; admin scope is approved and audited |
| Exported policy | Recipient of an export | Learn folder names and hosts | Paths optional; scrubbed export |
| Availability | User error | Revoke something the session needs and lock oneself out | Essential grants locked; self-lockout guard; Undo window; repair-mode reset in AD01 |
| Imported policy | A hostile policy file | Grant broad access on import | Signature shown; dry run; imported grants capped by the importer's own authority (SI-1); locked rules cannot be overridden |
| Service confusion | An app that uses a service as a deputy | Reach a resource through `svc.search` or `svc.thumbnail` it could not reach itself | Transitive-reach view makes deputies visible; services must pass narrowed, per-request handles; reported per service |

Stored: policy and grants in the service's stores; nothing about file contents. Leaves the machine: nothing; exports are files the user saves.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `permissions.mode` | enum{plain,expert} | `plain` | Level of detail |
| `permissions.show_services` | bool | `false` | List system services with apps |
| `permissions.show_revoked` | bool | `true` | Keep revoked nodes visible (24 h) |
| `permissions.undo_seconds` | u8 | `10` | Undo window after a revoke; 0 disables |
| `permissions.confirm.revoke_in_use` | bool | `true` | Warn when revoking something in use |
| `permissions.polite_close_seconds` | u8 | `5` | Time given to an app to close files before a revoke |
| `permissions.profile` | string | from install | Active profile name |
| `permissions.idle_expiry_days` | u16 | `180` (Balanced), `30` (Strict) | Forget grants unused this long; 0 = never |
| `permissions.session_grants_end_at_lock` | bool | `false` (Balanced), `true` (Strict) | End session grants at screen lock |
| `permissions.unused_threshold_days` | u16 | `90` | "Unused" filter threshold |
| `permissions.notify.new_capability_on_update` | bool | `true` | Notify when an update asks for more |
| `permissions.notify.temporary_end` | bool | `false` | Notify before a temporary grant ends |
| `permissions.digest` | enum{off,weekly,monthly} | `off` | Local summary notification |
| `permissions.transitive_depth` | u8 | `6` | Hop limit for transitive reach |
| `permissions.lineage.batch_ms` | u16 | `250` | Live update batching |
| `permissions.export.include_paths` | bool | `false` | Include remembered paths in exports |
| `permissions.emergency.scope` | set{network,devices,files} | `{network,devices}` | What the emergency stop revokes |

## 9. Accessibility and languages

- Rights, states and durations are words. Icons and colour only repeat them.
- The lineage tree is a real tree in the a11y tree; each node is announced as "holder, kind, rights, lost rights, state", and the path to root is available as one sentence.
- The graph view is optional and never the only way to reach information.
- Revocation announces its result: "Revoked. 7 handles closed in 3 processes. Undo available for 10 seconds." The Undo is reachable by one key (`Ctrl+Z`).
- Capability identifiers (`fs.portal`) stay as identifiers; their names and explanations are translated. Paths and host names are never translated or reordered; they are shown with left-to-right isolation in right-to-left layouts.
- Live "in use now" changes are polite live-region updates, at most one announcement per 5 s.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| First frame | Under 150 ms; app list complete under 300 ms |
| Select an app | Capability table under 50 ms for 500 grants |
| Revoke | User action to all handles dead: under 50 ms for a 1 000-node subtree on one machine (FORGE-WP-06 budgets revoke of 1 000 nodes under 1 ms inside one host; the rest is two bus round trips and the kernel call) |
| Reverse query | Under 100 ms for a path with 10 000 grants in the store; transitive, depth 6: under 500 ms |
| Simulation | Under 300 ms for a profile switch over 2 000 grants |
| Lineage view | 100 000 live nodes system-wide; the visible tree virtualised; live batching keeps redraws at or under 4 per second |
| Memory | App under 120 MiB; service under 64 MiB plus about 160 bytes per live lineage node |
| Idle | No wake-ups in the app when the window is hidden; the service is event-driven |

## 11. Testing and proof

Test plan (nothing has run):

- Unit: policy evaluation (layer precedence, locks, ceilings), duration and expiry arithmetic, rights narrowing, reach computation on synthetic graphs, URL parsing, export and import round trip, policy diff.
- Model-based tests: random sequences of grant, borrow, duplicate, narrow, transfer, revoke and expire against a reference model; after every step the service's view, each host's table and the kernel tree must agree.
- Concurrency: revoke racing with grant, duplicate, transfer over the bus, process exit and expiry (the priority surface in `docs/security/THREAT_MODEL.md`); run under a thread sanitizer and with schedule fuzzing.
- Golden renders: every view in 5.2–5.3, plain and expert, three themes, two scales, right-to-left.
- Scripted UI through the a11y tree: T1–T15 with fixture apps that hold real handles; each revoke is verified by the fixture's next operation failing with a revoked error.
- Fuzz: policy import, manifest reader, lineage export decoder, URL handler, the service's request decoder.
- Differential: the reference `LineageTree` (`pbsd/lineage/pbsd.cap.lineage.cppm`) and the SI harness cases (`pbsd/tests/si_harness.cpp`) against the service's index on the same operation traces.
- Negative: look-alike bundle calling loosening interfaces; import of a policy exceeding the user's authority; self-lockout attempts; host that under-reports.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Lineage tree (kernel reference) | SI-6: after `revoke(n)`, `valid(m)` is false for `n` and every descendant; no other node changes | T1 | Lean model and proof; PRISM refinement of `pbsd.cap.lineage` | C++ modules are outside the checker-friendly subset named in `docs/plan/PBSD_PLAN.md`; the module form may need a header twin |
| Rights algebra | SI-1: `grant`, `narrow`, `borrow`, `duplicate` never yield rights outside the parent's | T1 | Proof over `rights_subset`, `check_grant`, `check_duplicate` | — |
| Host grant table | Derived ⊆ parent; revoke kills the subtree (FORGE-WP-06 states T1 model, T2 refinement) | T1 / T2 | As planned in FORGE-WP-06 | Not built |
| Policy evaluator | Deterministic; a locked rule is never overridden; result never looser than the user ceiling | T1 | Lean model of layer precedence; PRISM refinement | Service not written |
| Policy service request gate | Loosening requires the Permissions identity and a valid confirmation token | T2 | PRISM contract; model check of the token state machine | Depends on bus peer identity (G-08) and SH03 |
| Reach computation | Sound: every app that holds a live handle to the resource appears in "can reach" | T2 | Proof against the lineage index model; differential tests | Soundness depends on complete lineage reporting (kernel introspection gap) |
| Simulation | Predicted revoked set equals the set actually revoked | T2 | Same function computes both; property test | — |
| Import | Applied policy ⊆ importer's authority | T2 | PRISM contract on the apply path | — |
| Revoke across layers | No window in which the service reports success while a derived descriptor is usable | T3 | Concurrency tests, schedule fuzzing | Kernel bridge is Wave 4 scaffold; proof needs the real locking design |
| UI | No control can add rights; destructive default never focused | T3 | Scripted UI tests | UI not in proof scope |

## 12. Build plan

Phase 1 — usable core (per-app and per-resource view, revoke, profiles):

| ID | Work item | Depends on |
|---|---|---|
| AD05-W01 | `svc.policy` interface schema: apps, grants, rules, revoke, queries, events | G-08, FORGE-WP-26 |
| AD05-W02 | Policy store, profiles Strict and Balanced as data, layer evaluation; answers for app hosts and portals | FORGE-WP-08 (manifest-to-policy), G-07, G-09 |
| AD05-W03 | Revoke path to app-host grant tables with acknowledgement | FORGE-WP-06 (06.1–06.3), FORGE-WP-25 |
| AD05-W04 | Lineage index fed by host lineage export and portal events | FORGE-WP-06.4, SH04 |
| AD05-W05 | App shell: Apps view, capability table, grants, inspector, search | G-02, FUSE-WP-12, FUSE-WP-28, FUSE-WP-29 |
| AD05-W06 | Resources view: files, devices, hosts; per-resource revoke | AD05-W02 |
| AD05-W07 | Lineage tree view with revoke of a node and path to root | AD05-W04 |
| AD05-W08 | Per-grant and per-app history from `svc.audit`; denials; link to AD04 | FORGE-WP-11.4, AD04-W04 |
| AD05-W09 | Reverse queries (file, device, host) without transitive hops | AD05-W04 |
| AD05-W10 | Bulk revoke by filter; Undo window; essential-grant and self-lockout guards | AD05-W03 |
| AD05-W11 | Profile switch with diff; restore defaults; prompt-policy edits | SH03 (approval), AD01-W08 |
| AD05-W12 | `pbsd-permissions:` deep links; trusted window mark | SH04, SH05, AD05-GAP-3 |

Phase 2 — full feature:

| ID | Work item | Depends on |
|---|---|---|
| AD05-W13 | Kernel lineage integration: revoke through the Capsicum bridge; introspection for cross-checking | PBSD Wave 4 kernel bridge (`CAPSICUM_LINEAGE.md`), `AD05-GAP-4`, G-06 |
| AD05-W14 | Temporary grants, expiry as revocation, grant-from-here, idle expiry, one-shot, emergency stop | FORGE-WP-06 (`expires_ns`), SH04, SH02 |
| AD05-W15 | Narrow a grant (revoke + derived grant); stable file identity for grants | SH04 portal support |
| AD05-W16 | Simulation for grants, nodes, sets, profile switch; impact warnings; polite close | apps reporting unsaved state through the app run-time (FORGE-WP-10) |
| AD05-W17 | Transitive reach; services and helpers views; cross-process edge labels | AD05-W13 |
| AD05-W18 | Policy export, import with dry run, signature, report, scrubbed export; AD03 hook | FORGE-WP-22, AD03-W17, G-16 |
| AD05-W19 | Per-app overrides, per-user ceilings, policy hash and source view, policy history | AD05-W02 |
| AD05-W20 | Update capability-change flag, manifest diff, unused grants, protect a resource, recent documents | SY05 |
| AD05-W21 | Prompt history, device sessions, notifications, reverse queries from SY02, VM05, SE02 | SY02, VM05, SE02 |

Phase 3 — advanced:

| ID | Work item | Depends on |
|---|---|---|
| AD05-W22 | Graph view, heat maps, lineage snapshot and export, borrow-lifetime checks, Capsicum detail | AD05-GAP-1, AD05-GAP-2 |
| AD05-W23 | Custom profiles, category and signer rules, policy lock, compare policies, saved queries with alerts | SE01 |
| AD05-W24 | Kernel, driver and VM lineage views; non-bundle programs; clipboard and intent permissions | VM05 (SI-7), VM01 (SI-8), VM03, AX05 |
| AD05-W25 | Command-line client, developer view, assistant explain, weekly digest | DV01, G-15 |

## 13. Open questions and risks

1. **No kernel revoke primitive was found.** `docs/specs/CAPSICUM_LINEAGE.md` maps `revoke(id)` to "`cap_revoke` + invalidation of derived fds"; a search of `hbsd/src/sys/kern` and `hbsd/src/sys/sys` found no `cap_revoke`. Revoking descriptors already held by another process is new kernel work (`AD05-GAP-4`), and SI-6 for descriptors depends on it. Until then revocation is enforced by app hosts and services, which a compromised host could ignore.
2. **The lineage tree carries no metadata and cannot be enumerated.** `LineageTree` has `parent` and `is_ancestor_of` but no child listing, holder or resource fields, and a fixed 4 096-node table. The lineage index in `svc.policy` is therefore necessary, and its truth depends on reporting. A kernel introspection interface is needed for cross-checking (`AD05-GAP-4`).
3. **Three enforcement layers must agree** (ForgeScript caps and FSS scopes, Capsicum rights, PBSD handle rights; PLATFORM.md section 6). The Capsicum bridge maps only four of the eight rights. The app must show what each layer enforces, not one merged claim.
4. **`svc.policy` is not in PLATFORM.md section 8.** Today "who remembers grants and answers prompts" is implied between the app host, the portals and `svc.audit`. This spec names one owner. If the platform instead keeps policy in the portal service, this app's interface target changes but not its design.
5. **Revoke races** are a named priority surface. The T1 proof target covers the tree; the cross-process path is T3 until the kernel locking design exists.
6. **Undo after revoke** is a re-grant, not a true undo: the app's old handles are gone. The toast text must say "Allow again", and for `once` and helper-return origins undo is impossible.
7. **Unsaved-state reporting** for impact warnings needs every app to tell the run-time about dirty documents; otherwise the warning is "may have unsaved work".
8. **Stable file identity** for grants across rename and move needs portal and filesystem support; without it grants are path-based and weaker.
9. **"Could reach" lists can alarm.** Any `fs.portal` app could reach any file the user picks. The view separates "can now", "could if you choose" and "has" to keep this honest without noise.
10. **Scale.** A busy session may hold far more than 4 096 live handles; the production lineage structure and its memory cost are undecided.
