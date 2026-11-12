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
| AD05-F### | Three levels, one view | Declared capabilities (from the manifest, PLATFORM.md section 6), grants (a capability bound to a resource and a duration), handles (live objects with rights and a lineage id). Each level links to the next. | M | 1 |
| AD05-F### | Rights shown as words | The closed rights set is Read, Write, Execute, Grant, Duplicate, Destroy, Map, Irq (Verified: `pbsd/rights/pbsd.rights.cppm`). Shown as words with a short meaning; "Grant" and "Duplicate" are explained as "may pass this on". | M | 1 |
| AD05-F### | Rights only narrow | Every edit control can remove rights or shorten duration, never add (SI-1, `docs/security/INVARIANTS.md`). Widening is done by the app asking again through a prompt. | M | 1 |
| AD05-F### | No ambient authority statement | Each app page begins with what the app can reach with no grants at all: its own data, its windows, the session bus (SI-2; PLATFORM.md section 6 rule 1). | M | 1 |
| AD05-F### | Handle kinds | Unique, Borrowed, Shared and VM handles are labelled as such (closed type set, `docs/specs/HANDLES.md`); Shared handles carry a note because the spec treats them as rare and audited. | S | 2 |
| AD05-F### | Grant origin | Each grant shows how it arose: manifest (install), portal (user picked), service (a system service returned it), helper return, profile default, administrator. Matches the `origin` field planned in FORGE-WP-06. | M | 1 |
| AD05-F### | Duration | Once, this session, until a time, always; with the time left for temporary ones. | M | 1 |
| AD05-F### | State vocabulary | Declared (never asked), Granted, In use now, Denied, Revoked, Expired; one icon and word each; used everywhere. | M | 1 |
| AD05-F### | Plain and expert modes | Plain mode shows apps, resources and grants. Expert mode adds handles, lineage ids, rights bits, policy hashes and process ids. | M | 1 |
| AD05-F### | Explanations | Each capability has a one-line meaning, a risk note and an example of what breaks without it; from a local table, translated. | M | 1 |

### 4.2 Per-app view

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F### | App list | Every installed bundle, system service and helper-owning app: icon, name, state badges (network, devices in use), count of grants, last active. Sort and filter. | M | 1 |
| AD05-F### | Capability table | Rows: each declared capability with state, resource count, last use; grouped as Files, Network, Devices, Services, Processes, Background, AI. | M | 1 |
| AD05-F### | Grants under a capability | Expand a capability to its grants: resource, rights, duration, origin, granted on, last used, use count. | M | 1 |
| AD05-F### | Revoke a grant | One action; takes effect at once for the app and every handle derived from it (SI-6). A toast offers Undo for 10 s; undo creates a fresh grant with the same terms where the origin allows it (portal and profile grants), otherwise explains that the app must ask again. | M | 1 |
| AD05-F### | Revoke all for an app | Revokes every non-essential grant; `fs.app` stays. The app is told and can keep running with less. | M | 1 |
| AD05-F### | Narrow a grant | Remove Write from a read-write folder grant; restrict a folder grant to a subfolder; shorten duration. Implemented as revoke plus a derived narrower grant so lineage stays truthful. | M | 2 |
| AD05-F### | Change prompt policy | Per capability per app: Allow, Ask every time, Ask once per session, Deny. | M | 1 |
| AD05-F### | Deny in advance | Mark a declared capability Denied so the app's request fails without a prompt. | M | 1 |
| AD05-F### | Helpers | Each helper of the app with the capabilities passed to it (never more than the app's, PLATFORM.md section 6 rule 2) and its live handles. | M | 2 |
| AD05-F### | Manifest view | The manifest's capability list as shipped, with the bundle signature state and a diff against the previous installed version. | S | 2 |
| AD05-F### | Capability change on update | When an update adds a capability the app is flagged "asks for more than before" until the user has seen it. | M | 2 |
| AD05-F### | Running instances | Instances and their process tree; per instance: live handles count, [Revoke all], [Stop app] (hand-off to SY04). | S | 2 |
| AD05-F### | Background permission | `bg.run` and `bg.autostart` state with what the app did in the background in the last 7 days. | M | 2 |
| AD05-F### | Per-app network summary | Hosts granted (`net.host`), user-typed endpoints seen (`net.user` first contacts), route allowed; link to SE02 for rules. | M | 1 |
| AD05-F### | Per-app data summary | Size of the app's own data, config and cache; [Clear cache], [Reset app data…] (hands to SY05 or SY02). | C | 3 |
| AD05-F### | Least-privilege hint | "Declared but never used in 90 days" list per app; offers to set those to Deny. | S | 2 |
| AD05-F### | Compare two apps | Side-by-side capability and grant comparison. | C | 3 |
| AD05-F### | App reset | Return an app to its just-installed state: all grants and prompt answers forgotten. | M | 1 |
| AD05-F### | System services | Services are listed in their own group with the handles they hold; essential ones cannot be revoked here and say why. | M | 2 |
| AD05-F### | Non-bundle programs | Programs started through `proc.exec` (Terminal, IDE) and compatibility-layer apps (VM03) appear under their launcher with the confinement they run in. | S | 3 |

### 4.3 Per-resource view

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F### | Resource classes | Files and folders, Devices, Hosts, Services, Other apps (intents), Keys (item names only). | M | 1 |
| AD05-F### | Files and folders | Tree of every path that has a grant, with the apps under each; inherited reach is shown (a grant on a folder covers its children). | M | 1 |
| AD05-F### | Fixed paths | `fs.path:` grants from manifests listed separately with a note that they were shown at install. | M | 1 |
| AD05-F### | Devices | Camera, microphone, screen, location, USB classes, printers, GPU compute: who may use, who is using now (live indicator), last use. | M | 1 |
| AD05-F### | Hosts | Every `net.host` endpoint and every first-contacted host under `net.user`, with apps, ports, route, first and last contact. | M | 1 |
| AD05-F### | Services | For each `svc.*`: which apps may talk to it and which calls they made (counts). | S | 2 |
| AD05-F### | Per-resource revoke | Revoke one app's grant on the resource, or all grants on it. | M | 1 |
| AD05-F### | Protect a resource | Mark a folder, device or host "always ask" or "never share": future prompts for it are restricted or refused regardless of the app's remembered answers. | S | 2 |
| AD05-F### | Sensitive folders preset | Keys, password store, mail store, browser profiles are pre-marked "never share by picker default; ask with a warning". | S | 2 |
| AD05-F### | Removable and remote resources | Grants on a removable disk or network location are shown as dormant when the resource is absent and can be set to expire on removal. | S | 2 |
| AD05-F### | Recent documents list | Remembered recent files per app (`svc.portal.file`) with forget one, forget all. | M | 2 |
| AD05-F### | Clipboard and intents | Which apps may read clipboard history (`svc.clipboard`) and which intents each app may send to which app. | S | 3 |
| AD05-F### | Dataset view | With AD02 data: per ZFS dataset, which apps hold handles into it (used by AD02 for export and auto-lock). | S | 2 |

### 4.4 Handle lineage tree

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F### | Lineage tree view | A tree per root grant. Every `grant`, `borrow` and `duplicate` makes a child node (Verified model: `docs/specs/CAPSICUM_LINEAGE.md`, `docs/specs/HANDLES.md`). Node: holder (app, helper, service), resource, rights, handle kind, age, state. | M | 1 |
| AD05-F### | Rights narrowing along a path | Each child shows which rights it lost relative to its parent; a child can never show more (SI-1). A violation would be drawn as an integrity error and reported to SE01. | M | 1 |
| AD05-F### | Revoke a node | Revoking a node invalidates it and all descendants atomically from the holders' view (SI-6; reference implementation `LineageTree::revoke_subtree` in `pbsd/lineage/pbsd.cap.lineage.cppm`). The count of affected handles and holders is shown before confirming. | M | 1 |
| AD05-F### | Revoked state | Revoked nodes stay visible, greyed, with time and actor, until pruned; use after revoke returns `Status::Revoked` (per `CAPSICUM_LINEAGE.md`) and each such attempt is counted on the node. | M | 2 |
| AD05-F### | Roots | Roots are grouped by source: portal picks, manifest grants, service-issued handles, device sessions. | M | 1 |
| AD05-F### | Cross-process edges | When a handle is passed over the bus or to a helper, the edge is labelled with the transfer (which interface, when). | M | 2 |
| AD05-F### | Graph view | Optional graph layout for wide trees; the same data. | S | 2 |
| AD05-F### | Find in lineage | Search by holder, resource, lineage id, rights. | M | 2 |
| AD05-F### | Path to root | From any handle: the chain of derivations back to the user's decision or manifest line that started it. | M | 1 |
| AD05-F### | Live updates | Nodes appear and disappear as handles are created and released; churn is rate-limited in the view (batched every 250 ms). | M | 2 |
| AD05-F### | Depth and fan-out limits | Shows how close a tree is to platform limits. The reference tree is a fixed table of 4 096 nodes (Verified: `kMaxNodes` in `pbsd.cap.lineage.cppm`); the production limit is not set yet. | S | 2 |
| AD05-F### | Borrow lifetime | Borrowed handles show their lender; a borrow that outlives a released lender is flagged (the spec forbids it, `HANDLES.md` closed type set table). | S | 3 |
| AD05-F### | Capsicum mapping detail | Expert mode shows the Capsicum rights a handle maps to. The current bridge maps Read, Write, Execute and Map only (Verified: `pbsd/kernel/capsicum_bridge.cppm`); the other four rights are PBSD-only. | S | 3 |
| AD05-F### | Snapshot of the tree | Save the current tree as a file for later comparison or for a report. | S | 3 |
| AD05-F### | Kernel and service lineage | Lineage of handles held by system services and drivers (read-only for essential ones); UDA and VM handle trees link to VM05 and VM01 (SI-7, SI-8). | C | 3 |

### 4.5 History of use

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F### | Per-grant history | Timeline of: granted, each use (bucketed per hour with counts), narrowed, denied attempts, revoked or expired; from `svc.audit` records (schema fields in FORGE-WP-11.4). | M | 1 |
| AD05-F### | Per-app history | All decisions and uses for one app; filters by capability and decision. | M | 1 |
| AD05-F### | Per-resource history | Who touched this folder, device or host, and when. | M | 2 |
| AD05-F### | Prompt history | Every prompt shown: what was asked, what the user chose, how long the prompt was open. | M | 2 |
| AD05-F### | Denials list | Recent denials with the reason (not declared, denied by policy, denied by user, revoked, expired); "allow next time" is offered only for policy and user denials. | M | 1 |
| AD05-F### | Device session log | Camera, microphone, screen and location sessions with duration. | M | 2 |
| AD05-F### | Usage heat map | Calendar heat map of a grant's or app's use per day. | S | 3 |
| AD05-F### | Unused grants | Filter: not used in 30, 90, 365 days; bulk revoke from the list. | M | 2 |
| AD05-F### | Unusual use hints | First use of a capability by an app, use at an unusual hour, sudden volume; labelled hints, detection proper is SE01. | C | 3 |
| AD05-F### | Open in Logs | Any history row opens the matching record range in AD04 (`pbsd-logs:` link). | M | 1 |
| AD05-F### | History retention | Follows the audit class retention in AD04; this app keeps no second copy. | M | 1 |
| AD05-F### | Weekly digest | Optional notification: new grants, new hosts, devices used, grants unused for 90 days. Local only. | S | 3 |

### 4.6 Temporary grants and expiry

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F### | Grant for a time | From here or from the prompt (SH04): 5 min, 1 h, until logout, until a date and time. Uses the `expires_ns` field planned in FORGE-WP-06. | M | 2 |
| AD05-F### | Active temporary grants list | All grants with an expiry, sorted by time left; [Extend] (asks), [End now]. | M | 2 |
| AD05-F### | Expiry is revocation | At expiry the grant's lineage node is revoked exactly as a manual revoke (SI-6); the app is notified. | M | 2 |
| AD05-F### | Grant from here | Give an app a folder, device or host from this app without waiting for it to ask: pick app, resource, rights, duration. The resource is chosen through the portal so the user's own authority is the root. | S | 2 |
| AD05-F### | Session grants end at lock (option) | Device and network session grants can end at screen lock, not only at logout. | S | 2 |
| AD05-F### | Idle expiry | "Forget this grant if unused for N days" per grant or as a profile default. | S | 2 |
| AD05-F### | One-shot tokens | A grant valid for one operation (open one file once), shown as "once" and expiring on first release. | M | 2 |
| AD05-F### | Expiry notice | Optional notification 5 min before a temporary grant ends for an app in use. | C | 3 |
| AD05-F### | Emergency mode | One switch: revoke all device and network grants for all apps now, and deny new ones until switched off. Also reachable from SH02 quick settings. | S | 2 |

### 4.7 Profiles and policy

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F### | Profiles | Strict and Balanced (chosen at install, AD01). A profile is a table: capability → default prompt policy, allowed durations, idle expiry, whether "always" may be remembered. | M | 1 |
| AD05-F### | Balanced defaults | `fs.portal` remembered grants allowed; devices ask on first use then per session; `net.host` from manifest allowed and shown; `net.user` allowed with first-contact logging; `bg.*` ask. | M | 1 |
| AD05-F### | Strict defaults | No "always" for devices or folders (session at most); `net.*` asks per app at first use even when declared; `proc.exec`, `bg.autostart`, `ai.local` ask every session; idle expiry 30 days; fixed paths (`fs.path`) need confirmation after each update. | M | 1 |
| AD05-F### | Profile diff before switching | Lists every existing grant that the new profile would revoke or shorten; nothing changes until confirmed; needs admin approval. | M | 1 |
| AD05-F### | Per-app overrides | An app may be stricter or (with admin approval) looser than the profile for named capabilities; overrides are listed and countable. | M | 2 |
| AD05-F### | Custom profiles | Copy a profile, edit the table, name it; assign per user. | S | 3 |
| AD05-F### | Per-user policy | On multi-user machines each user has their own grants; the administrator can set a ceiling profile no user may exceed. | S | 2 |
| AD05-F### | Category rules | Rules by app category or signer: "apps not signed by the PBSD key never get `proc.exec`". | S | 3 |
| AD05-F### | Policy lock | An administrator can lock named rules so the user cannot loosen them (shared or managed machines). | S | 3 |
| AD05-F### | Policy hash | The effective policy for each app has a hash; the same hash appears in audit records (`policy hash` in FORGE-WP-11.4) so a decision can be tied to the policy in force. | M | 2 |
| AD05-F### | Policy source view | For any effective setting: which layer set it (manifest, profile, category rule, per-app override, administrator lock). | M | 2 |
| AD05-F### | Restore defaults | Reset one app, one capability across apps, or everything to the profile. | M | 1 |
| AD05-F### | Policy history | Every policy change with before and after, actor and time; undo of the last change. | S | 2 |

### 4.8 Bulk actions, reverse queries, simulation

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F### | Bulk revoke by filter | Select grants by any filter (capability, resource class, app, origin, age, unused) and revoke the set; the confirmation lists counts per app. | M | 1 |
| AD05-F### | Bulk presets | All network; all devices; all remembered folders; everything for apps not opened in 90 days; everything from one signer. | M | 2 |
| AD05-F### | Bulk change of prompt policy | Set a capability to Ask or Deny across selected apps. | S | 2 |
| AD05-F### | Reverse query: file or folder | "What can reach this?" Result: apps with a grant on it or on an ancestor, the rights, via which grant, live handles now, plus services that reach it by design (backup, search index, thumbnailer) listed separately. | M | 1 |
| AD05-F### | Reverse query: device | Apps allowed, apps using now, and the path through the portal service. | M | 1 |
| AD05-F### | Reverse query: host | Apps with `net.host` for it, apps with `net.user` that have contacted it, routes used; "could contact" (any `net.user` app) is listed as potential, separate from actual. | M | 1 |
| AD05-F### | Reverse query: service | Apps that may call a service and what the service can reach on their behalf (confused-deputy view). | S | 3 |
| AD05-F### | Transitive reach | Follows chains: app → helper, app → service that acts for it, app → other app by intent with a passed handle. Each hop is shown; depth limit 6. | M | 2 |
| AD05-F### | Reverse query from other apps | SY02 ("Who can access this?" on a file), VM05 (device), SE02 (host) open this app with the query filled in. | M | 2 |
| AD05-F### | Saved queries | Keep a reverse query and re-run it; optional alert when its answer grows. | S | 3 |
| AD05-F### | Simulation: what if I revoke | For a grant, a lineage node, a set or a profile switch: lists handles that would die, holders affected, operations in progress (open files with unsaved state where the app reports it), features of the app that depend on it (from the manifest's reasons). No change is made. | M | 2 |
| AD05-F### | Simulation: what if I grant | Shows what new reach a proposed grant creates, including transitive reach through helpers and services. | S | 3 |
| AD05-F### | Simulation: policy import | Dry run of an imported policy: adds, removals, conflicts. | S | 2 |
| AD05-F### | Impact warnings | Revoking something an app is using right now warns "Photos has 3 files open here; unsaved edits may be lost" and offers "ask the app to close them first" (a polite request with a 5 s limit, then revoke). | M | 2 |
| AD05-F### | Undo window | Bulk and single revokes have a 10 s Undo where re-granting is within the user's authority; after that the app must ask again. | M | 1 |

### 4.9 Export, import, reporting

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F### | Export policy | JSON file: profile, rules, per-app overrides, remembered grants (paths optional), schema version, optional Ed25519 signature. | M | 2 |
| AD05-F### | Export as report | Human-readable Markdown or PDF: per app, what it can reach and why; for review or records. | S | 2 |
| AD05-F### | Scrubbed export | Replace user names and home paths with placeholders (same rules as AD04). | S | 2 |
| AD05-F### | Import policy | With dry-run simulation, signature check, admin approval; imported grants never exceed what the importing user could grant themselves. | M | 2 |
| AD05-F### | Diff two policies | Current against a file, or two files. | S | 3 |
| AD05-F### | Backup integration | The policy export is part of AD03's system state set; restore goes through the same import path. | M | 2 |
| AD05-F### | Lineage export | A lineage tree as JSON or Graphviz DOT text. | C | 3 |
| AD05-F### | Machine-readable query | A `permissions` command: list, query, revoke, simulate, export; same service calls and approvals. | S | 3 |
| AD05-F### | Print | Print the report through the print portal. | C | 3 |

### 4.10 Safety, integration, settings

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD05-F### | Own authority is minimal | This app holds no handle to any user file, device or host. It sends requests to the policy service; revocation is done by the service and the kernel. | M | 1 |
| AD05-F### | Trusted identity | The window carries the shell's trusted-app mark so a look-alike app cannot pose as Permissions; changes that loosen policy need an SH03 confirmation. | M | 1 |
| AD05-F### | Tightening needs no approval | A user may always revoke or narrow their own grants without admin approval; loosening locked rules needs it. | M | 1 |
| AD05-F### | Essential grants | Grants the session needs to function (compositor, shell, portals) are shown, locked, with the reason; they cannot be revoked from the GUI. | M | 1 |
| AD05-F### | Self-lockout guard | Revoking this app's own `svc.policy` access or the shell's portal access is refused; recovery is in AD01 repair mode ("reset capability grants"). | M | 1 |
| AD05-F### | Every change audited | Each revoke, narrow, policy change, import and query under admin scope is written to `svc.audit`. | M | 1 |
| AD05-F### | Deep links | `pbsd-permissions:` URLs select an app, a grant, a resource or a query; used by prompts ("Manage…"), AD04, SE01, SE02, SY02, SY04, SY05. | M | 1 |
| AD05-F### | Indicator hand-off | Clicking the header `[net]` badge or a device indicator in any app can open that app's page here. | S | 2 |
| AD05-F### | Notifications | Optional: "an app was granted a new capability", "a temporary grant ended", "an update asks for more". | S | 2 |
| AD05-F### | Search | One search box over apps, capabilities, resources, hosts and lineage ids. | M | 1 |
| AD05-F### | Local assistant help | With `ai.local`: "Explain what this app can do" produces a plain summary from the grant list; never changes policy. | C | 3 |
| AD05-F### | Developer view | For an app under development (from DV01): requested-versus-used capabilities over a test run, to trim the manifest. | C | 3 |
| AD05-F### | First-run summary | After install, a one-page summary of the chosen profile and the handful of grants that exist (links from AD01 first run). | S | 2 |
