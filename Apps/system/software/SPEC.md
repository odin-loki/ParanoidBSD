<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Software

By Odin Loch

| | |
|---|---|
| ID | `SY05` (from CATALOGUE.md) |
| Bundle | `org.pbsd.software` |
| Category | system |
| Priority | P0 |
| Backing repos | ParanoidBSD (`pbsd/pkg/*.cppm`, `tools/pbsd_pkg/pbsd_pkg`, `docs/ROADMAP.md`), FORGE (signing: Ed25519 over BLAKE3), FUSE (widgets) |
| Status | Planned. Nothing built. The package modules it sits on are scaffolds (section 6.4). |

## 1. Purpose

Software installs, updates and removes apps and system packages. Every user uses it to get apps and to keep the system current; administrators use it to manage repositories, keys, boot environments and rollbacks. It shows what an app will be allowed to do before it is installed, applies every change as one atomic transaction that can be undone, and can show how each package was built. It replaces Discover from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- Catalogue browsing, search and app detail pages, all from locally stored, signed repository metadata.
- Install, update, remove, pin, hold; dependency resolution; transaction preview.
- Signed updates; staged, atomic application through ZFS boot environments; rollback.
- Repositories and their signing keys.
- Reproducible-build status and software bill of materials (SBOM) view per package.
- Vulnerability feed matched against installed packages.
- Offline and USB updates; exporting update bundles for other machines.
- Kernel and live patches.
- History of transactions and undo.

Out of scope:

| Item | Owner |
|---|---|
| Granting and revoking capabilities after install | AD05 Permissions (Software shows the manifest and links) |
| Device firmware updates | VM05 Devices (Software lists pending firmware and hands off) |
| Model files (`.fpk`) | AI02 Model Manager |
| Building packages from source; development toolchains | DV01 IDE and the build system |
| Windows, Linux and Android apps | VM03 Compatibility |
| Snapshot schedules and data backup | AD03 Backup |
| Dataset and pool operations | AD02 Disks |
| First installation of the system | AD01 Installer (shares the transaction engine) |
| Security posture summary | SE01 Security Centre (reads this app's vulnerability state) |
| Fonts, language packs as content | installed here, managed in UT03 and SY01 |
| The update policy switch | SY01 shows a summary; the setting lives here |

## 3. Users and top tasks

Personas:

- **Everyday user**: installs an app, presses "Update all", expects nothing to break.
- **Administrator**: holds packages, adds a repository, rolls back a bad update, updates an offline machine from USB.
- **Security-minded user**: reads the capability list before installing, checks signatures, reproducibility and known vulnerabilities.
- **Auditor**: exports the SBOM of the installed system.

| # | Task | Target steps |
|---|---|---|
| T1 | Find and install an app | 3 (search, open, Install) |
| T2 | See what an app can access before installing | 0 extra (shown on the detail page) |
| T3 | Install all updates | 2 (Updates, Update all) |
| T4 | Roll back the last update | 2 (History, Undo) or 1 at the boot menu |
| T5 | Remove an app and its data | 3 |
| T6 | See whether any installed package has a known vulnerability | 1 (Security tab) |
| T7 | Add a repository with its key | 4 |
| T8 | Update an offline machine from a USB stick | 3 |
| T9 | Check that a package was built reproducibly | 2 |
| T10 | Hold a package at its current version | 2 |
| T11 | Export the system SBOM | 2 |
| T12 | Boot into a previous boot environment once | 3 |
| T13 | See exactly what a pending update changes | 2 |
| T14 | Apply a live patch without restarting | 2 |

## 4. Features

### 4.1 Catalogue and discovery

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY05-F001 | Home | Categories (the 15 suite categories plus System, Development libraries, Fonts, Languages), recently updated, installed count, pending updates card. No promoted or paid placement. | M | 1 |
| SY05-F002 | Local catalogue | The catalogue is the signed repository index stored on disk. Browsing and search make no network request. | M | 1 |
| SY05-F003 | Search | By name, summary, description, category, MIME type handled, command name, capability ("apps that never use the network"), licence. Fuzzy, ranked; offline. | M | 1 |
| SY05-F004 | Filters | Installed, not installed, updatable, apps only, system packages, by licence family, by capability (no network, no background), by repository, reproducible only, signed by. | M | 1 |
| SY05-F005 | Sort | Name, last updated, size, install date. No popularity ranking (there is no telemetry to compute one). | M | 1 |
| SY05-F006 | App and package modes | "Apps" shows bundles a person launches; "All packages" adds libraries, system components, kernels. | M | 1 |
| SY05-F007 | Find by file type | "What can open `.odt`?" from the MIME registry in the index; used by `svc.intent` when no handler is installed. | S | 2 |
| SY05-F008 | Find by command | "Which package gives the command `rsync`?" | S | 2 |
| SY05-F009 | Collections | Named sets defined in repository metadata (for example "Office", "Development"); install a set in one transaction. | C | 3 |
| SY05-F010 | Screenshots and media | Shipped inside the signed metadata, size-capped, decoded by `svc.thumbnail`. Never fetched from third-party hosts. | S | 2 |
| SY05-F011 | Recently viewed | Local list; can be cleared; off switch. | C | 2 |

### 4.2 App detail

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY05-F012 | Detail page | Name, summary, description, version, size (download and installed), licence, homepage, source location, maintainer, repository, release date, changelog. | M | 1 |
| SY05-F013 | Capabilities before install | The app's manifest capabilities (PLATFORM.md section 6) in plain language, grouped: files, network, devices, services, background, AI. Each row says always, on first use (prompt), or off unless enabled. Fixed paths and named hosts are listed exactly. | M | 1 |
| SY05-F014 | Capability risk summary | One line on top: "No network. Reads only files you choose." or "Can reach any network address you type." Derived by fixed rules from the manifest, not by a score. | M | 1 |
| SY05-F015 | Capability change on update | An update that adds or widens a capability is flagged; the diff is shown; such updates are never applied silently. | M | 1 |
| SY05-F016 | Helpers | Lists native helpers in the bundle with language and each helper's capabilities. | S | 2 |
| SY05-F017 | Trust panel | Signature state (who signed, key fingerprint, when), repository, reproducible-build state, proof-tier summary where the package publishes one, known vulnerabilities. | M | 1 |
| SY05-F018 | Dependencies | Tree of required and optional dependencies with sizes; reverse dependencies ("needed by"). | M | 1 |
| SY05-F019 | Files | List of files the package installs with type, size and digest; search. | S | 2 |
| SY05-F020 | Versions | Available versions per repository; install a specific version; downgrade. | S | 2 |
| SY05-F021 | Changelog and diff | Changelog entries between installed and candidate; file-level diff summary (added, removed, changed counts). | S | 2 |
| SY05-F022 | Licence text | Full text offline; licence family badge. | M | 1 |
| SY05-F023 | Local notes | The user's own note on a package. No reviews or ratings (they would need accounts and a network service). | C | 3 |
| SY05-F024 | Launch and manage | After install: Open, Remove, Hold, Permissions (AD05), Storage used (app, data, cache). | M | 1 |
| SY05-F025 | Try in a disposable sandbox | Install into a temporary overlay and run once with a throw-away data directory; removed on exit. | C | 3 |

### 4.3 Install, remove and transactions

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY05-F026 | Transaction model | Every change is a transaction: resolve, download to a content-addressed cache, verify, stage, commit, or roll back. Mirrors `begin` / `stage` / `commit` / `rollback` of the package module (section 6.4). | M | 1 |
| SY05-F027 | Preview | Before commit: packages to install, update, remove; download and disk size; capability changes; whether a restart is needed; which boot environment will be made. | M | 1 |
| SY05-F028 | Resolver | Dependency closure with version constraints, conflicts and "provides"; explains failures in plain words ("A needs B 2.x; C needs B 3.x"). | M | 1 |
| SY05-F029 | Per-user and system installs | Apps install system-wide by default (authenticated); a standard user may install apps for themselves into their own verified dataset if policy allows. | S | 2 |
| SY05-F030 | App install without restart | App bundles live on their own datasets; installing or updating an app is an atomic dataset swap and needs no restart of the system. Running instances keep the old version until closed. | M | 1 |
| SY05-F031 | Remove | Remove an app; choose to keep or delete its data, config and cache (sizes shown); a snapshot of deleted data is kept for N days. | M | 1 |
| SY05-F032 | Autoremove | List packages that were installed only as dependencies and are no longer needed. | S | 2 |
| SY05-F033 | Hold and pin | Hold at the current version; pin to a repository or version range; held items show a reason field. | M | 2 |
| SY05-F034 | Queue | Several requests merge into one transaction where possible; progress in the status bar and `svc.notify`; survives closing the window. | M | 1 |
| SY05-F035 | Pause, resume, cancel | Downloads pause and resume by block; cancel is safe at any point before commit. | M | 1 |
| SY05-F036 | Power-loss safety | A cut during commit leaves the previous state bootable and intact (the PBSD 0.1 target in `pbsd/pkg/README.md`). | M | 1 |
| SY05-F037 | Disk-space check | Checks cache and target datasets, counting snapshot and boot-environment cost. | M | 1 |
| SY05-F038 | Install from file | Open a signed package or bundle file; shows signer and capabilities first. Unsigned files are refused; a developer mode can allow a locally trusted key. | M | 2 |
| SY05-F039 | Lockfile | Export the exact installed set (names, versions, digests) as a lockfile; apply a lockfile to reproduce it on another machine. | S | 2 |
| SY05-F040 | Dry run | Run resolve and verify without committing; report saved as text. | S | 2 |
| SY05-F041 | Post-install checks | After commit, verify installed files against manifest digests. | M | 1 |
| SY05-F042 | No install scripts | Packages carry no arbitrary install scripts. Declared, typed actions only (create user, register service, register MIME types), each shown in the preview. | M | 1 |
| SY05-F043 | Repair | Verify all installed files against their manifests; re-stage any that differ. | S | 2 |

### 4.4 Updates

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY05-F044 | Updates view | Pending updates grouped: security, system, apps, firmware (hand-off), live patches. Each with version change, size, restart need, capability change mark. | M | 1 |
| SY05-F045 | Check | Manual check, and automatic check on a schedule. A check fetches only signed index files from configured repositories. | M | 1 |
| SY05-F046 | Policy | Off; notify; download in the background; install apps automatically; install system updates at shutdown. Per class (security, system, apps). | M | 1 |
| SY05-F047 | Metered and battery rules | No background download on metered links or below a battery threshold (from `svc.power` and SY01's metered flag). | S | 2 |
| SY05-F048 | Staged system update | System updates are applied into a new boot environment cloned from the running one; the running system is untouched until restart. | M | 1 |
| SY05-F049 | Activate on restart | The new environment is marked for the next boot once ("try boot"); if it does not reach a healthy login it is not made permanent and the next boot returns to the old one. | M | 2 |
| SY05-F050 | Health check | After booting a new environment, a list of checks (services up, session reached, package verification) must pass before it becomes the default. | M | 2 |
| SY05-F051 | Restart prompt | "Restart to finish" with a list of what changes; restart now, tonight, or at next shutdown. Never forces a restart. | M | 1 |
| SY05-F052 | Selective update | Tick the updates to apply; the resolver adds what they need. | M | 1 |
| SY05-F053 | Skip a version | Hide one version of one package; it reappears with the next. | C | 2 |
| SY05-F054 | Release upgrade | Moving to a new OS release is the same mechanism with a larger transaction, a compatibility check and a longer health check. | S | 3 |
| SY05-F055 | Rollback protection | Index files carry a monotonic sequence number and expiry time; an older or expired index is refused (stops replay of old metadata). | M | 1 |
| SY05-F056 | Delta downloads | Because the store is content-addressed, only missing blocks are fetched. | S | 2 |
| SY05-F057 | Mirrors | Several mirrors per repository; integrity comes from signatures, so mirrors need not be trusted; automatic fail-over. | S | 2 |
| SY05-F058 | Route choice | Fetch through clear, VPN or anonymous route (SE03), per repository. | S | 2 |
| SY05-F059 | Update notes | Human-written notes per update set, shown before applying. | S | 2 |

### 4.5 Boot environments and rollback

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY05-F060 | Boot environment list | Name, created, reason (which transaction), active now, active on next boot, size unique to it, kernel version, hardening profile (from SY01), health state. | M | 1 |
| SY05-F061 | Activate | Make an environment the default, or boot it once. | M | 1 |
| SY05-F062 | Create | Manual environment with a name and note. | S | 2 |
| SY05-F063 | Rename, delete, protect | Protect from automatic cleanup; delete with size shown; the active one cannot be deleted. | M | 2 |
| SY05-F064 | Compare | Package and version differences between two environments; configuration file differences as a list. | S | 2 |
| SY05-F065 | Mount to inspect | Mount an environment read-only and open it in SY02. | C | 3 |
| SY05-F066 | Retention | Keep the last N environments and any younger than D days; protected ones are never removed. | M | 2 |
| SY05-F067 | Rollback | "Undo this update" activates the environment from before the transaction and asks to restart. App-only transactions roll back at once with no restart. | M | 1 |
| SY05-F068 | Boot menu entry | States what the boot menu shows and how to pick an older environment when the desktop does not start. | M | 1 |
| SY05-F069 | Data separation notice | Explains that user data lives outside boot environments and is not rolled back; links to AD03. | M | 1 |
| SY05-F070 | Verified boot state | Shows whether the environment's kernel and loader are covered by the verified-boot chain (state read from SE01). | S | 3 |

### 4.6 Repositories and keys

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY05-F071 | Repository list | Name, URL or path, flags (signed, mirror, local, immutable, as in `pbsd.pkg.repo` `RepoFlag`), key fingerprint, priority, enabled, last refresh, index sequence number. | M | 1 |
| SY05-F072 | Add repository | From a signed repository descriptor file or by URL plus fingerprint typed or scanned. The key is shown as words and hex and must be confirmed. Unsigned repositories cannot be added. | M | 2 |
| SY05-F073 | Scope limits | A third-party repository may be limited to named packages or name prefixes; it can never replace a system package unless explicitly allowed. | M | 2 |
| SY05-F074 | Key management | Keys per repository: fingerprint, created, expiry, rotation chain. Key rotation needs a statement signed by the old key (or by a quorum for the system repository). | M | 2 |
| SY05-F075 | Revocation | Revoked keys and revoked package digests are carried in signed metadata and enforced at verify time. | M | 2 |
| SY05-F076 | Threshold signing | The system repository index must carry k of n signatures (role keys: root, index, packages), so one stolen key is not enough. | S | 3 |
| SY05-F077 | Local repository | A folder or USB path as a repository (`Local` flag). | M | 2 |
| SY05-F078 | Priorities and pinning | Order between repositories; per-package origin pin. | S | 2 |
| SY05-F079 | Repository health | Last successful refresh, expiry countdown of metadata, mirror status. | S | 2 |
| SY05-F080 | Network transparency | Each repository's hosts are declared; the `[net]` badge lists them; first contact is logged. | M | 1 |

### 4.7 Provenance: reproducibility, SBOM, vulnerabilities

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY05-F081 | Reproducibility badge | Per package: "reproduced by N independent builders" (attestations signed by rebuilder keys), "not yet reproduced", or "not reproducible" with the differing files. | M | 2 |
| SY05-F082 | Build record | Source location and revision, source digest, toolchain versions, build options, build environment digest, builder identity, time. | M | 2 |
| SY05-F083 | Rebuilder trust | Choose which rebuilder keys count; require at least K matching attestations before install (policy). | S | 3 |
| SY05-F084 | SBOM view | Components of a package: name, version, licence, supplier, digest, relation (built from, bundled, linked). Search and filter. Formats: SPDX 2.3 JSON and CycloneDX 1.5 JSON, read and export. | M | 2 |
| SY05-F085 | System SBOM | One SBOM for the whole installed system or a boot environment; export; diff between two. | S | 2 |
| SY05-F086 | Licence report | Count of packages per licence; list those with given obligations; export. | S | 3 |
| SY05-F087 | Vulnerability feed | A signed feed (OSV-schema records, carried by the repository or a separate signed source) matched locally against installed versions and SBOM components. | M | 2 |
| SY05-F088 | Vulnerability list | Id, severity, package, installed and fixed versions, summary, whether a fix is available now, mitigation notes. | M | 2 |
| SY05-F089 | Exposure hint | Notes when the affected package belongs to an app with no network capability or when the hardening profile mitigates the class. Stated as context, not as "safe". | C | 3 |
| SY05-F090 | Vulnerability alerts | Notification for new high-severity matches; one click to the fixing update. | M | 2 |
| SY05-F091 | Feed freshness | Shows feed date; warns when older than N days; offline import (section 4.8). | M | 2 |
| SY05-F092 | Proof tier display | Where a package publishes PRISM results, show the tier per module and the report digest; link to DV06. | C | 3 |
| SY05-F093 | Verify now | Re-verify a package's signature, digests and attestations on demand and show each step's result. | S | 2 |

### 4.8 Offline, USB and enterprise

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY05-F094 | Offline bundle import | Apply an update bundle (`.pbsp` signed manifest plus content store) from a file or removable device; same verification as online. | M | 2 |
| SY05-F095 | Bundle export | On a connected machine, build a bundle for another machine from that machine's exported lockfile, so only what it lacks is included. | S | 2 |
| SY05-F096 | USB detection | When a device with a bundle is mounted, a toast offers "Review updates on this device". Nothing is applied automatically. | S | 2 |
| SY05-F097 | Feed and index import | Import vulnerability feed and index snapshots offline; expiry rules still apply, with a visible override for air-gapped policy. | S | 2 |
| SY05-F098 | Local mirror tool | Create and refresh a full local mirror on a disk. | C | 3 |
| SY05-F099 | Fleet lockfile | Apply a signed lockfile from an administrator to match a fleet baseline. | C | 3 |

### 4.9 Firmware, kernel and live patches

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY05-F100 | Firmware hand-off | Pending firmware updates appear as a group with "Open in Devices"; VM05 applies them. Firmware files are delivered through the same signed repositories. | M | 2 |
| SY05-F101 | Kernel packages | Kernels are packages; installed kernels listed per boot environment; kernel configuration name and hardening option set shown. | M | 2 |
| SY05-F102 | Kernel modules | Modules are packages tied to a kernel version; mismatches are blocked by the resolver. | M | 2 |
| SY05-F103 | Live patches | Signed live patches for the running kernel or for long-running services: list, apply, revert, state (applied, superseded by a full update). Patches are applied by a privileged loader after signature and target-digest checks. | S | 3 |
| SY05-F104 | Live patch honesty | Shows which fixes are active only in memory and will need the matching full update at next restart. States when policy (`hardening.forbid_kmod`, securelevel) forbids live patching. | S | 3 |
| SY05-F105 | Service restarts | After a library update, lists running services and apps still using the old version, with restart buttons. | S | 2 |

### 4.10 History, storage and housekeeping

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY05-F106 | History | Every transaction: time, who, what (installed, updated, removed with versions), source repository, boot environment before and after, result. Search and filter. | M | 1 |
| SY05-F107 | Undo a transaction | Reverse any past transaction where the old versions are still in the cache or a boot environment; shows what else would change. | M | 2 |
| SY05-F108 | Export history | Text or JSON. | C | 2 |
| SY05-F109 | Cache management | Content-addressed cache size, slot states (staged, ready, evicted, as in `pbsd.pkg.cache`), clean now, size limit, keep N previous versions. | S | 2 |
| SY05-F110 | Storage by app | Table of app size, data, cache; sort; open in UT04. | S | 2 |
| SY05-F111 | Audit | Every transaction, key change and repository change goes to `svc.audit`. | M | 1 |
| SY05-F112 | Command-line parity | Everything here is also available from the `pbsd-pkg` tool; the app shows the equivalent command for a transaction. | S | 2 |
| SY05-F113 | Automation | ForgeScript module `software::` for query and for requesting transactions; requests still need authentication. | C | 3 |
| SY05-F114 | Intents | `pbsd-app:<id>` links and "no app for this file type" hand-offs open the detail page. | M | 1 |
| SY05-F115 | Explain an update | Optional local summary of long changelogs through `svc.model`. Off by default. | C | 3 |
| SY05-F116 | Developer mode | Allow packages signed by a local developer key, marked clearly everywhere they appear. | S | 3 |
| SY05-F117 | Session restore and notifications | Reopen at the last view; progress and "restart needed" as persistent notifications. | S | 1 |

## 5. GUI

### 5.1 Window map

```
Software window (single instance)
├─ Header bar: menu, title, search, [net] badge (only while fetching), window buttons
├─ Sidebar: Explore · Installed · Updates (count) · Security (count) · Boot environments
│           · History · Repositories · Categories ▸ (list)
├─ Content
│   ├─ Explore: category grid, lists
│   ├─ List views: package rows with state and action button
│   ├─ Detail page: header, capability panel, trust panel, tabs (About · Capabilities
│   │               · Dependencies · Files · Versions · Build and SBOM · Vulnerabilities)
│   ├─ Updates: grouped list + summary bar
│   ├─ Security: vulnerability table + feed state
│   ├─ Boot environments: table + detail
│   ├─ History: transaction list + detail
│   └─ Repositories: list + keys
├─ Inspector (F4): selected package or transaction summary
├─ Status bar: transaction progress, restart-needed state, last check time
├─ Dialogs: Transaction preview · Capability change · Remove app · Add repository
│           · Confirm key · Import offline bundle · Export bundle · Restart to finish
│           · Compare boot environments · Resolver conflict · Verify now · Hold reason
├─ Popovers: Progress details · Signature details · Filter editor
└─ Preferences window: Updates · Repositories · Verification policy · Storage · Advanced
```

### 5.2 Main window

Shown at 1280×780 on an app detail page.

```
+------------------------------------------------------------------------------------------+
| [=] Software                 [ Search apps and packages…  Ctrl+F ]          [▁] [▢] [✕]   | (1)
+----------------+-------------------------------------------------------------------------+
| > Explore      | ‹ Back   Office › Sheets                                                |
|   Installed    | +----+  Sheets  0.4.1                                    [ Install ]    | (3)
|   Updates   12 | |icon|  Spreadsheet: formulas, charts, pivot tables     48 MB download  |
|   Security   1 | +----+  AGPL-3.0-or-later · pbsd-apps · 3 Nov 2026       131 MB on disk |
|   Boot envs    |-------------------------------------------------------------------------|
|   History      | WHAT THIS APP CAN DO                                                    | (4)
|   Repositories |  ✓ No network.                                                          |
| CATEGORIES     |  ✓ Files: only the ones you open or save.                               |
|   Office       |  • Print (asks first)     • Local AI (off unless you turn it on)        |
|   Internet     |  • 2 helpers: file-import (no capabilities), calc-engine (none)         |
|   Media        |                                              [All 9 capabilities…]      |
|   Development  |-------------------------------------------------------------------------|
|   System       | TRUST                                                                   | (5)
|   …            |  ✓ Signed by PBSD apps key 2026 (ed25519 9f3a…c1)                       |
|                |  ✓ Reproduced by 3 of 3 builders        ✓ No known vulnerabilities      |
|                |  SBOM: 41 components · 6 licences                 [Build and SBOM…]     |
|                |-------------------------------------------------------------------------|
|                | [About][Capabilities][Dependencies][Files][Versions][Build][Vulns]      | (6)
|                | Sheets opens ODS, XLSX and CSV. It works with large tables and …        |
+----------------+-------------------------------------------------------------------------+
| Idle · last checked today 08:02 · 12 updates · restart not needed                        | (7)
+------------------------------------------------------------------------------------------+
```

1. Header bar. The `[net]` badge appears only while a refresh or download runs and lists the repository hosts.
2. Sidebar, 220 px (`treeView`), with counts on Updates and Security.
3. Detail header: icon, name, version, summary, licence, repository, date, sizes, and the primary action (Install, Open, Update, or "Restart to finish").
4. Capability panel: the plain-language summary from the manifest (F013, F014). Always above the description. `✓` marks strong statements (no network); bullets mark things the app may do.
5. Trust panel: signature, reproducibility, vulnerabilities, SBOM size (F017).
6. Detail tabs (`tabWidget`).
7. Status bar: transaction state, last check, pending count, restart state.

States: loading (index open, under 100 ms; skeleton only on first run), no index yet ("No catalogue on this machine yet" + [Refresh] + [Import from a device]), offline (banner: "Offline. Showing the catalogue from 12 Nov 08:02." Install enabled only for content already in the cache or a local repository), error (repository unreachable or metadata expired, with exact reason), locked (install and system changes need authentication), read-only (guest: browse only).

### 5.3 Other views and dialogs

#### 5.3.1 Updates view

```
+------------------------------------------------------------------------------------------+
| Updates                                    [Check now]  [Update selected (12)]           |
| 12 updates · 310 MB to download · restart needed for 3                                   |
+------------------------------------------------------------------------------------------+
| v Security (2)                                                                           |
|  [x] libexample        1.2.3 → 1.2.4     PBSD-2026-0142 high      1.1 MB                 |
|  [x] kernel            15.0-p3 → 15.0-p4 PBSD-2026-0150 medium   38 MB   restart         |
| v System (4)                                                                             |
|  [x] base-userland     0.1.6 → 0.1.7                             62 MB   restart         |
|  [x] session-bus       0.3.0 → 0.3.1                              2 MB   restart         |
|  …                                                                                       |
| v Apps (5)                                                                               |
|  [x] Web Browser       0.9.2 → 0.9.3                             81 MB                   |
|  [x] Mail              0.5.0 → 0.6.0     ⚠ new capability: dev.print  22 MB  [Review]    |
| v Firmware (1)                                                    [Open in Devices]      |
| v Live patches (0)                                                                       |
+------------------------------------------------------------------------------------------+
| System updates go into a new boot environment. Your current system stays as it is        |
| until you restart, and you can go back.                                                  |
+------------------------------------------------------------------------------------------+
```

`treeView` with check boxes. An update with a capability change cannot be ticked into an automatic run; it needs [Review].

#### 5.3.2 Transaction preview

```
+----------------------------- Review changes -----------------------------+
| Install 1 · Update 11 · Remove 0                                         |
|  + Sheets 0.4.1                          48 MB                           |
|  ↑ kernel 15.0-p3 → 15.0-p4              38 MB                           |
|  ↑ base-userland 0.1.6 → 0.1.7           62 MB                           |
|  … 9 more                                [Show all]                      |
| Download 310 MB · Disk +120 MB · Cache has 40 MB already                 |
| Capability changes:  Mail gains "print"  [Review]                        |
| Declared actions:    register 2 MIME types · restart service "indexer"   |
| Boot environment:    new "update-2026-11-12" (current one kept)          |
| Restart:             needed to finish system updates                     |
| All 12 packages: signatures ✓ · index sequence 4412 ✓                    |
|                                    [Cancel]  [Dry run]  [Apply changes]  |
+--------------------------------------------------------------------------+
```

#### 5.3.3 Capability change dialog

```
+------------------- Mail 0.6.0 asks for more access -------------------+
| New in this version                                                   |
|   + Print  (dev.print) — asks each time before printing               |
| Unchanged                                                             |
|   Network: servers you add · Files: ones you choose · Keyring · …     |
| Removed                                                               |
|   − none                                                              |
| From the changelog: "Print messages and attachments."                 |
|                 [Keep 0.5.0 and hold]  [Cancel]  [Accept and update]  |
+-----------------------------------------------------------------------+
```

#### 5.3.4 Boot environments

```
+------------------------------------------------------------------------------------------+
| Boot environments                                   [Create…] [Compare…] [Clean up…]     |
+------------------------------------------------------------------------------------------+
| Name                  | Created      | Reason              | Size  | State               |
| > update-2026-11-12   | today 09:10  | 11 updates          | 0.4 G | next boot (try once)|
|   update-2026-11-03   | 3 Nov        | 4 updates           | 0.9 G | running now ✓       |
|   hardening-2026-10-28| 28 Oct       | Strict profile      | 0.2 G | healthy             |
|   install             | 2 Oct  🔒    | first install       | 1.8 G | healthy, protected  |
+------------------------------------------------------------------------------------------+
| update-2026-11-12 · kernel 15.0-p4 · hardening Standard · 11 packages differ from running|
| [Boot once] [Make default] [Rename] [Protect] [Open in Files] [Delete…]                  |
| Your documents and settings are not part of a boot environment and are not rolled back.  |
+------------------------------------------------------------------------------------------+
```

#### 5.3.5 Build and SBOM tab

```
+------------------------------------------------------------------------------------------+
| Build record                                                                             |
|  Source     git: forge.example/pbsd/sheets @ 4c1e9a2 (digest b3:71aa…)                   |
|  Toolchain  clang 21.1 · libc++ 21.1 · forge 0.9      Build env digest b3:02fe…          |
|  Built      3 Nov 2026 by builder "pbsd-ci-1"                                            |
| Reproducibility   ✓ 3 of 3 builders produced the same package digest                     |
|  pbsd-ci-1 ✓   rebuild-au ✓   rebuild-eu ✓                          [Attestations…]      |
+------------------------------------------------------------------------------------------+
| SBOM  41 components        [ filter… ]            [Export SPDX…] [Export CycloneDX…]     |
| Component        | Version | Licence        | Relation  | Digest      | Vulns            |
| sheets           | 0.4.1   | AGPL-3.0+      | this      | b3:9c…      | 0                |
| calc-engine      | 0.4.1   | AGPL-3.0+      | bundled   | b3:11…      | 0                |
| forge-text       | 0.9.0   | AGPL-3.0+      | linked    | b3:5d…      | 0                |
+------------------------------------------------------------------------------------------+
```

#### 5.3.6 Security (vulnerabilities) view

```
+------------------------------------------------------------------------------------------+
| Security        Feed: 12 Nov 2026 06:00 ✓ signed · 1 affects this system    [Refresh]    |
+------------------------------------------------------------------------------------------+
| Id              | Sev.  | Package    | Installed | Fixed in | Status                     |
| > PBSD-2026-0142| high  | libexample | 1.2.3     | 1.2.4    | fix available [Update now] |
|   PBSD-2026-0131| low   | old-tool   | 2.0       | —        | no fix yet                 |
+------------------------------------------------------------------------------------------+
| PBSD-2026-0142 — heap overflow in the example parser                                     |
| Used by: Web Browser, Mail (both have network access)                                    |
| Context: the hardening profile's MPROTECT and ASLR are active for these apps.            |
| References: (3)                                                                          |
+------------------------------------------------------------------------------------------+
```

#### 5.3.7 Add repository and confirm key

```
+----------------------------- Add a repository -----------------------------+
| From   (•) Descriptor file [extra.pbrepo      ] [Browse…]   ( ) URL [____] |
| Name   extra-tools            URL  https://pkg.example.org/pbsd            |
| Key    ed25519  4c9e 11a0 77b2 … e93f                                      |
|        words:   amber · canal · eleven · mosaic · river · tundra …         |
| Compare this with the fingerprint from a source you trust.                 |
| Limit to packages [extra-*              ]   [ ] May replace system packages|
| Route  [clear v]                                                           |
|                         [Cancel]  [I have checked the key — add]           |
+----------------------------------------------------------------------------+
```

#### 5.3.8 Import offline bundle

```
+------------------ Updates on "USB stick" (pbsd-2026-11.pbsp) ------------------+
| Signed by PBSD release key 2026 ✓   Index sequence 4412 (newer than 4398) ✓   |
| Built for: this machine's package set (lockfile digest matches) ✓             |
| Contains: 11 updates, vulnerability feed of 12 Nov                            |
|   [x] System (4)   [x] Security (2)   [x] Apps (5)   [x] Vulnerability feed   |
| 310 MB will be copied to the cache and verified.                              |
|                                         [Cancel]  [Review changes…]           |
+-------------------------------------------------------------------------------+
```

#### 5.3.9 History

```
+------------------------------------------------------------------------------------------+
| History                           [ filter… ]                         [Export…]          |
+------------------------------------------------------------------------------------------+
| When          | By     | Change                         | Boot env          | Result     |
| > today 09:10 | odin   | 11 updated, 1 installed        | update-2026-11-12 | staged     |
|   3 Nov 21:40 | auto   | 4 updated                      | update-2026-11-03 | ok         |
|   1 Nov 10:02 | odin   | removed "old-game" (+ data)    | —                 | ok         |
+------------------------------------------------------------------------------------------+
| today 09:10 · pbsd-base, pbsd-apps · [Show packages] [Undo this…] [Show command]         |
+------------------------------------------------------------------------------------------+
```

### 5.4 Menus

```
[=] menu / F10
├─ File
│   ├─ Install from file…                Ctrl+O
│   ├─ Import offline bundle…            Ctrl+Shift+O
│   ├─ Export update bundle…
│   ├─ Export installed set (lockfile)…  Ctrl+E
│   ├─ Apply lockfile…
│   ├─ Export system SBOM…               Ctrl+Shift+B
│   └─ Quit                              Ctrl+Q
├─ Edit
│   ├─ Find                              Ctrl+F
│   ├─ Copy package name / link
│   └─ Preferences                       Ctrl+,
├─ View
│   ├─ Explore                           Ctrl+1
│   ├─ Installed                         Ctrl+2
│   ├─ Updates                           Ctrl+3
│   ├─ Security                          Ctrl+4
│   ├─ Boot environments                 Ctrl+5
│   ├─ History                           Ctrl+6
│   ├─ Repositories                      Ctrl+7
│   ├─ Apps only / All packages          Ctrl+Shift+A
│   ├─ Filters…                          Ctrl+I
│   ├─ Inspector                         F4
│   └─ Refresh view                      F5
├─ Package
│   ├─ Install / Update                  Ctrl+Enter
│   ├─ Remove…                           Delete
│   ├─ Open app                          Enter
│   ├─ Hold / Release                    Ctrl+H
│   ├─ Install a specific version…
│   ├─ Verify now                        Ctrl+Shift+V
│   ├─ Show in Permissions
│   └─ Show files
├─ Updates
│   ├─ Check now                         Ctrl+R
│   ├─ Update all…                       Ctrl+U
│   ├─ Review pending transaction        Ctrl+T
│   ├─ Restart to finish…
│   ├─ Undo last transaction…            Ctrl+Z
│   └─ Repair installed packages…
├─ System
│   ├─ Create boot environment…
│   ├─ Compare boot environments…
│   ├─ Add repository…
│   ├─ Manage keys…
│   ├─ Clean cache…
│   └─ Autoremove unused…
└─ Help
    ├─ Software help                     F1
    ├─ How updates and rollback work
    ├─ Keyboard shortcuts
    └─ About Software
```

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| `Ctrl+Z` | Undo the last transaction (opens its preview; never silent). The menu's "Undo last transaction" uses this reserved key, not `Ctrl+Shift+Z`. |
| `Ctrl+Enter` | Install or update the selected package |
| `Space` | Tick or untick an update row |
| `Alt+Left` / `Alt+Right` | Back and forward between pages |
| `Ctrl+L` | Copy the `pbsd-app:` link of the open page |

Context menu on a package row: Open · Install / Update · Remove… · Hold · Install a specific version… · Verify now · Show in Permissions · Show files · Copy name · Copy link. On a boot environment: Boot once · Make default · Rename · Protect · Compare with running · Open in Files · Delete…. On a repository: Refresh · Disable · Edit limits… · Show key · Remove…. On a history row: Show packages · Undo this… · Copy as command.

Drag and drop: a package file, a `.pbrepo` descriptor, a lockfile or a `.pbsp` bundle dropped on the window opens the matching dialog. Nothing installs on drop.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Installed with a filter that matches nothing; no updates ("Everything is up to date · checked today 08:02"); no history. |
| Loading | Index queries are local and fast; only refresh and download show progress. |
| Error | Signature failure is a `--danger` block naming the package, the expected key and what was found; the transaction stops. Network errors offer Retry and another mirror. |
| Offline | Catalogue and detail pages work; Install works from cache or local repositories; a banner says when metadata will expire. |
| Locked | System changes need authentication; a standard user without rights sees "Ask an administrator" and can save the request as a lockfile. |
| Read-only | Running from a snapshot or live medium: browse only, with reason. |
| Restart pending | Status bar and a persistent notification; the Updates view shows the staged environment and [Restart to finish] and [Discard staged update]. |
| Transaction running | Other installs queue; boot-environment deletion is disabled. |
| Narrow (< 640) | Sidebar overlay; detail panels stack; tables become two-line rows; action button stays pinned at the bottom. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar | `treeView` |
| Category and app grids | `gridView` |
| Package lists, updates | `modelListView`, `treeView` with check boxes |
| Detail tabs | `tabWidget`, `markdownView` (description, changelog, licence) |
| Tables (SBOM, files, vulnerabilities, boot environments, history, repositories) | `tableView` (backed by `SqliteModel`, PLATFORM.md V8) |
| Dependency tree | `treeView` |
| Progress | `progressBar` |
| Screenshots | `image` in a `scrollArea` |
| Dialogs | `dialog`, `fileDialog` through the portal, `lineEdit`, `comboBox`, `checkBox` |

Missing:

- `SY05-GAP-1` Capability summary component: a shared, shell-owned rendering of a manifest's capabilities, so SY05, AD05 and the SH04 prompts word them identically.
- `SY05-GAP-2` Key fingerprint display (hex groups plus word list, with compare aid).
- `SY05-GAP-3` Version diff list cell (old → new with change marks). Small; a styled `label` pair may do.
- No web view is used. Descriptions are Markdown rendered by `markdownView` with remote images disabled.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Software app | ForgeScript | app host | Views, queries on the local catalogue database, transaction requests, policy editing |
| UI documents | FUSE UI | app host | `.fuseui` for views and dialogs |
| Update service (`svc.update`) | C++23 | system service, privileged | Owns transactions: resolve, stage, verify, commit, rollback; boot environments; history; the only writer of system datasets. Built on the `pbsd.pkg` modules. |
| Fetcher | C++23 | helper of the update service, Capsicum, sockets to repository hosts only, write handle on the cache staging area only | Downloads index files and content blocks. Holds no keys; cannot mark anything verified. |
| Verifier | C++23 | helper of the update service, Capsicum, read handles only, no network | Checks digests, signatures, sequence numbers, expiry, revocations, attestations |
| Metadata parser | C++23 | helper, Capsicum, no network | Parses index, manifests, SBOM, vulnerability feed, repository descriptors into the catalogue database |
| Catalogue database | SQLite | written by the update service, read-only handle given to the app | Packages, versions, capabilities, dependencies, files, SBOM, vulnerabilities, history |
| Live patch loader | C++23 | privileged, part of the update service | Applies and reverts verified live patches |
| `pbsd-pkg` command-line tool | C++23 | user command | Same interface to the update service as the app |

```
 +----------------------- app host: Software -----------------------+
 | ForgeScript: views, queries, requests     FUSE UI                |
 +-------+---------------------------+------------------------------+
         | read-only SQLite handle   | bus: svc.update (authenticated requests)
         v                           v
   catalogue database  ◄──────  update service (privileged)
                                 │        │            │
                     fetcher ◄───┘        │            └──► ZFS: boot environments,
                (net: repo hosts,     verifier              app datasets, cache
                 writes staging)   (no net, read-only)
                                          │
                                   metadata parser (no net) ──► catalogue database
```

The app is unprivileged and has no network. It cannot install anything by itself; it asks the service, which asks the shell to authenticate the user.

### 6.2 Data model

Catalogue database (`/var/db/pbsd/software/catalogue.sqlite`, `schema_version` in table `meta`):

| Table | Fields |
|---|---|
| `repo` | `id`, `name`, `url`, `flags: u32` (signed, mirror, local, immutable), `key_fp: text`, `priority`, `enabled`, `scope_glob`, `route`, `index_seq: u64`, `index_expires: i64`, `last_refresh: i64` |
| `package` | `id`, `name`, `kind: app\|lib\|system\|kernel\|module\|font\|lang\|firmware\|livepatch`, `summary`, `description_md`, `licence_spdx`, `homepage`, `category`, `app_id` |
| `version` | `id`, `package_id`, `repo_id`, `version`, `digest: text(64)`, `size_download`, `size_installed`, `released_at`, `changelog_md`, `manifest_digest`, `sig_key_fp`, `sig_alg` |
| `capability` | `version_id`, `cap: text`, `subject: app\|helper:<name>`, `mode: always\|prompt\|off_default` |
| `dependency` | `version_id`, `dep_name`, `constraint`, `optional: bool`, `kind: requires\|conflicts\|provides` |
| `file` | `version_id`, `path`, `type: file\|dir\|symlink\|device`, `digest`, `size`, `mode` |
| `installed` | `package_id`, `version_id`, `scope: system\|user:<uid>`, `installed_at`, `reason: user\|dependency`, `held: bool`, `hold_note`, `pin` |
| `attestation` | `version_id`, `builder_key_fp`, `result_digest`, `matches: bool`, `at` |
| `build_record` | `version_id`, `source_url`, `source_rev`, `source_digest`, `toolchain_json`, `env_digest`, `builder`, `built_at` |
| `sbom_component` | `version_id`, `name`, `version`, `licence_spdx`, `supplier`, `digest`, `relation` |
| `vuln` | `id: text`, `severity`, `summary`, `details_md`, `published`, `modified` |
| `vuln_affects` | `vuln_id`, `name`, `introduced`, `fixed`, `ecosystem` |
| `tx` | `id`, `started_at`, `finished_at`, `uid`, `origin: app\|cli\|auto\|offline`, `state: staging\|verified\|applied\|rolled_back\|failed`, `be_before`, `be_after`, `needs_restart` |
| `tx_item` | `tx_id`, `package_id`, `from_version`, `to_version`, `action: install\|update\|remove\|downgrade` |
| `boot_env` | `name`, `created_at`, `reason_tx`, `protected`, `health: unknown\|healthy\|failed`, `kernel_version`, `hardening_profile` |
| `key` | `fp`, `repo_id`, `role: root\|index\|packages\|rebuilder`, `not_before`, `not_after`, `revoked_at`, `rotated_from` |

On-disk formats (Proposed unless marked):

- Signed manifest `.pbsp`: named in `pbsd/pkg/README.md` (`pbsd-pkg apply manifest.pbsp`) and `pbsd.pkg.bootstrap` (`bootstrap.pbsp`) — Verified as a name only; its byte format is not defined in the repo. Proposed: canonical CBOR `{format, seq, expires, entries: [{type, path, digest, size}], packages: [...]}` with a detached signature block `{algo, key_id, sig}` matching the fields of `pbsd.pkg.signature::SignatureBlock`.
- Lockfile: header `{magic 'PBSD', version 1, entry_count, created_unix}` followed by index entries `{name, version, digest, kind}` — field names Verified from `pbsd.pkg.lockfile` and `pbsd.pkg.index`; serialisation Proposed.
- Content store: files named by digest under the cache dataset; slot states empty, staged, ready, evicted (Verified as an enum in `pbsd.pkg.cache`).
- Repository descriptor `.pbrepo`: JSON `{name, urls[], key_fp, roles, scope_glob}` plus signature. Proposed.

Migration: the catalogue is rebuilt from signed metadata if the schema changes; `installed`, `tx` and `boot_env` are migrated forward with a backup.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Repository index and package manifests (`.pbsp`) | yes | yes (bundle export) | own code on `pbsd.pkg.manifest`, `.index`, `.verify` | metadata parser; signatures in the verifier |
| Signature blocks (Ed25519; BLAKE3 or SHA-256 digests) | yes | no (signing is a release tool) | own code; FORGE's Ed25519-over-BLAKE3 scheme (PLATFORM.md V14) | verifier |
| Lockfile | yes | yes | own code on `pbsd.pkg.lockfile` | metadata parser |
| Repository descriptor `.pbrepo` | yes | yes | own code (JSON) | metadata parser |
| SPDX 2.3 JSON, CycloneDX 1.5 JSON | yes | yes | own code (JSON subset readers) | metadata parser |
| OSV-schema vulnerability records (JSON) | yes | no | own code | metadata parser |
| Rebuilder attestations (signed JSON statements) | yes | no | own code | verifier |
| HTTPS (fetch only, GET with ranges) | yes | no | FORGE network layer; TLS from the base system | fetcher |
| Markdown (descriptions, changelogs) | yes | no | FUSE `markdownView`, remote content disabled | app host |
| Screenshots (PNG) | yes | no | `svc.thumbnail` | thumbnail sandbox |
| Live patch container | yes | no | own code | verifier, then the loader |
| SQLite catalogue | yes (app), yes/write (service) | — | SQLite (public domain), already used by FUSE `fuse_ui_sql` | app host reads through `SqliteModel` |

### 6.4 Integration

- **Intents handled**: `open` for `pbsd-app:<id>` links, package files, `.pbrepo`, `.pbsp`, lockfiles; `search` ("find an app for this type") from `svc.intent`. **Emitted**: `open` to AD05, VM05, SY02, UT04, SE01, DV06, AD04, SY01 (`settings://security/hardening`).
- **Services**: `svc.update` (all changes), `svc.settings`, `svc.notify`, `svc.intent`, `svc.power` (battery and restart), `svc.audit`, `svc.thumbnail`, `svc.portal.file`, `svc.search` (contributes installed apps and catalogue entries), `svc.model` (optional), `svc.a11y`.
- **Used by**: AD01 (same transaction engine for first install), SY01 (boot environment for hardening changes, updates summary), SE01 (vulnerability state), SH02 (restart-needed indicator).

What exists in the repo (Verified, read on 12 Nov 2026):

| Fact | Source |
|---|---|
| The package system is described as "content-addressed package transactions for rollback-safe system updates" with the model `begin()`, `stage()`, `commit()`, `rollback()`; commit is to be a ZFS boot-environment swap "in full implementation". | `pbsd/pkg/README.md` |
| The stated 0.1 target: `pbsd-pkg apply manifest.pbsp` applies a signed manifest; power loss mid-commit rolls back to the last good boot environment. | `pbsd/pkg/README.md` |
| `AtomicUpdater` has `begin`, `stage`, `commit`, `rollback` over a `Transaction` with states Idle, Staging, Applied, RolledBack and at most 16 `PackageRef {name[64], digest_hex[65]}`. `commit` only sets the state; a comment says the real path snapshots datasets and swaps the boot environment. | `pbsd/pkg/pbsd.pkg.cppm` |
| A second transaction type has phases Idle, Staging, Verified, Applied, RolledBack and at most 32 staged packages. | `pbsd/pkg/pbsd.pkg.transaction.cppm` |
| Manifest entries have a type (File, Directory, Symlink, Device), `path[256]`, `digest_hex[65]`, `size`. | `pbsd/pkg/pbsd.pkg.manifest.cppm` |
| Repositories have `name`, `url`, `fingerprint_hex` and flags Signed, Mirror, Local, Immutable. The default is `pbsd-base` at `https://pkg.pbsd.local/base`, Signed and Immutable, with seed manifest `bootstrap.pbsp` and an `offline` switch. | `pbsd/pkg/pbsd.pkg.repo.cppm`, `pbsd.pkg.bootstrap.cppm` |
| Signature blocks name an algorithm (None, Sha256, Ed25519), `key_id[32]`, `sig_hex[129]`; the file calls itself "signature verification stubs". | `pbsd/pkg/pbsd.pkg.signature.cppm` |
| Index (256 entries of name, version, digest, kind), cache slots (Empty, Staged, Ready, Evicted; 64 slots), lockfile (magic `PBSD`, version 1), dependency list (package, version, optional; 64), resolver (closure, at most 32) and a verify module (checks Digest, Signature, Manifest; 64 entries) exist as fixed-size in-memory structures. | `pbsd/pkg/pbsd.pkg.index.cppm`, `.cache.cppm`, `.lockfile.cppm`, `.deps.cppm`, `.resolver.cppm`, `.verify.cppm` |
| The command-line tool is a Python placeholder with `status`, `apply`, `rollback`; `apply` and `rollback` print "not implemented" and exit 2. | `tools/pbsd_pkg/pbsd_pkg` |
| The roadmap records: "No `pkg` repository, no signing key, no update path." | `docs/ROADMAP.md` (line 588) |

So: the vocabulary and shape exist; no file I/O, network, cryptography, ZFS call or persistent format exists. Everything in this spec beyond those structures is Proposed, and the fixed limits (16, 32, 64, 256) must be lifted for a real catalogue.

## 7. Security and privacy

Capabilities requested:

| Capability | Reason |
|---|---|
| `fs.app` | UI state, local notes, recently viewed |
| `fs.portal` | Open package files, bundles, descriptors; save exports |
| `fs.path:/var/db/pbsd/software:r` | Read-only catalogue database |
| `net.none` | The app never opens a socket. Fetching is done by the update service's fetcher, whose hosts are the configured repositories. |
| `svc.update` | Request transactions and boot-environment actions |
| `svc.settings`, `svc.notify`, `svc.intent`, `svc.power`, `svc.audit`, `svc.thumbnail`, `svc.portal.file`, `svc.search`, `svc.a11y` | As in 6.4 |
| `ai.local`, `svc.model` | Changelog summary only, off by default |
| `bg.run` | Not requested by the app. Background checks belong to `svc.update`. |

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| The whole system | Network attacker or hostile mirror | Serves modified packages | Every index and package is signed; content is addressed by digest; mirrors are untrusted by design. |
| The whole system | Network attacker | Replays old, vulnerable metadata or freezes updates | Monotonic sequence numbers, expiry times, freshness warning (F055, F079). |
| The whole system | Stolen repository key | Signs malicious updates | Role separation and threshold signatures for the system repository; revocation in signed metadata; rotation rules; reproducibility attestations as a second check. |
| The whole system | Compromised build server | Ships a backdoored binary from clean source | Independent rebuilders; policy can require K matching attestations (F083). |
| The system | Malicious package | Install scripts run as root | No install scripts; typed declared actions only (F042). |
| The user | Malicious or over-reaching app | Asks for broad access unnoticed | Capabilities shown before install; capability growth on update never silent (F013, F015). |
| The system | Third-party repository | Replaces a system package | Scope limits and priority rules (F073). |
| The update service | Hostile metadata | Parser bugs in a privileged process | Parsing in an unprivileged helper; fetcher cannot verify; verifier cannot fetch; the privileged core handles only verified, digest-named content. |
| Availability | Bad update | System does not boot | Boot environments, try-once boot, health check, boot-menu fallback. |
| Privacy | Repository operator | Learns what is installed from requests | The whole index is fetched, not per-package queries; content blocks can be fetched over an anonymous route; no identifiers are sent; no telemetry. |
| Offline media | Tampered USB bundle | Applies hostile content | Same verification as online; nothing auto-applies. |
| Integrity of the view | Compromised app process | Shows false "verified" marks | Verification results come from the service into the read-only database; the commit path re-verifies regardless of what the UI showed. |

Stored: catalogue and history in the system database; app UI state per user. Leaves the machine: HTTPS requests from the fetcher to configured repository hosts for index and content. Nothing else.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `software.updates.check` | enum(`off`,`daily`,`weekly`) | `daily` | Automatic check |
| `software.updates.security` | enum(`notify`,`download`,`install`) | `download` | Policy for security updates |
| `software.updates.system` | enum(`notify`,`download`,`install_at_shutdown`) | `notify` | Policy for system updates |
| `software.updates.apps` | enum(`notify`,`download`,`install`) | `notify` | Policy for app updates |
| `software.updates.metered` | bool | false | Allow downloads on metered links |
| `software.updates.min_battery_pct` | int 0–100 | 40 | No background work below this on battery |
| `software.updates.auto_capability_growth` | bool (fixed) | false | Updates that add capabilities are never automatic; shown, not changeable |
| `software.be.keep_count` | int 2–50 | 5 | Boot environments kept |
| `software.be.keep_days` | int 0–365 | 30 | Also keep any younger than this |
| `software.be.try_once` | bool | true | New environments boot once before becoming default |
| `software.verify.require_reproduced` | int 0–5 | 0 | Minimum matching rebuilder attestations to install |
| `software.verify.metadata_max_age_days` | int 1–90 | 14 | Refuse older index files |
| `software.verify.airgap_override` | bool | false | Allow expired metadata from offline bundles, with a warning |
| `software.vuln.notify_min_severity` | enum(`low`,`medium`,`high`,`critical`) | `high` | Alert threshold |
| `software.vuln.feed_warn_days` | int | 7 | Warn when the feed is older |
| `software.cache.limit_gb` | int | 8 | Cache size limit |
| `software.cache.keep_versions` | int 0–5 | 1 | Previous versions kept for undo |
| `software.remove.keep_data_snapshot_days` | int 0–90 | 14 | Keep a snapshot of removed app data |
| `software.view.default_mode` | enum(`apps`,`all`) | `apps` | Apps only or all packages |
| `software.view.recent` | bool | true | Keep recently viewed |
| `software.developer_mode` | bool | false | Allow a local developer key |
| `software.user_installs` | bool | true | Standard users may install apps for themselves |
| `software.ai.summarise_changelogs` | bool | false | Local changelog summaries |

## 9. Accessibility and languages

- Trust states are words with icons: "signed and verified", "not reproduced yet", "signature failed". Never colour alone.
- The capability panel is a list with one item per capability, each a full sentence, so a screen reader reads the same text sighted users see.
- Key fingerprints are exposed in groups of four characters and as a word list, and can be read character by character.
- Long transactions announce stage changes (downloading, verifying, staging, ready) through a live region, not every percentage.
- Package descriptions and capability sentences are translated through the string tables; package metadata may carry per-language summaries, falling back to English.
- Version strings and digests are marked left-to-right inside right-to-left text.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | 150 ms; home filled in 250 ms |
| Catalogue size supported | 100,000 packages, 5,000,000 file rows |
| Search | under 30 ms per keystroke on the full catalogue |
| Detail page open | 50 ms |
| Index refresh (parse and load 100,000 packages) | under 20 s in the background, UI stays live |
| Verification throughput | at least 1 GiB/s BLAKE3 on one core, parallel across cores |
| App install (50 MB bundle, cached) | under 3 s to launchable |
| Boot environment creation | under 2 s (clone) |
| Memory | 90 MiB for the app; update service under 150 MiB during a transaction |
| Idle | 0 frames, 0 wake-ups; scheduled checks are timers in the service |

## 11. Testing and proof

Test plan (targets; nothing has run):

- Unit: resolver (conflicts, provides, optional), version comparison, capability diff, capability-to-sentence rules, policy logic, vulnerability matching.
- Golden renders: all views and dialogs, three themes, right-to-left.
- Scripted UI: the 14 top tasks against a local test repository.
- Fuzz: manifest, index, lockfile, descriptor, SPDX, CycloneDX, OSV, attestation and live-patch parsers.
- Differential: resolver results against a reference solver on generated dependency graphs; SBOM round-trip through external validators' schemas.
- Fault injection: power cut at every step of commit (in a VM), disk full, network drop, corrupted block, expired metadata, rollback of rollback.
- Attack tests: wrong key, valid key outside its scope, replayed old index, mixed-version index and packages, mirror serving another repository's files, revoked digest, capability growth hidden in an update.
- Upgrade tests: N to N+1 on a matrix of installed sets; try-once boot with a deliberately broken environment.
- Reproducibility: build the same package twice on separate builders and compare (a check on the build system, reported here).

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Transaction state machine | Only the transitions idle → staging → verified → applied or rolled back; commit impossible before verified; rollback restores the prior set | T1 | PRISM proof over `pbsd.pkg.transaction` once it is real | ZFS behaviour is assumed |
| Verifier | Accepts content only if digest matches and the signature chain, sequence and expiry checks all pass | T1 | Proof of the decision function over abstract crypto primitives | Cryptographic primitives are assumed correct |
| Metadata parsers | No runtime errors, bounded memory | T2 | PRISM plus fuzz | Grammar specifications |
| Resolver | Output satisfies every constraint or reports unsatisfiable | T2 | Checker that validates each solution; property tests | Optimality is not claimed |
| Capability diff | Reports every added or widened capability | T1 | Proof over the finite capability grammar | — |
| Fetcher | Writes only to the staging area; no runtime errors | T2 | Capsicum rights plus PRISM | Third-party TLS stays T3/T4 |
| Boot-environment switch | After a crash at any point, either the old or the new environment is the default and is complete | T1 | Model with crash points; VM power-cut tests | Loader and ZFS are assumed |
| Live patch loader | Applies only patches whose target digest equals the running image | T2 | PRISM on the checker | Kernel patching mechanism itself |
| ForgeScript app logic | Language safety | T3 | Tests | Interpreter not proved |

## 12. Build plan

Phase 1, usable core:

| ID | Work item | Depends on |
|---|---|---|
| SY05-W01 | Define on-disk formats: `.pbsp` manifest, index, signature block, lockfile, content store layout | `pbsd/pkg` owners |
| SY05-W02 | Make the `pbsd.pkg` modules real: persistent index, cache, verify with real digests and Ed25519, limits lifted | SY05-W01 |
| SY05-W03 | Update service: transactions, app-dataset swap, boot-environment create and activate, history | SY05-W02, G-09, ZFS boot environments |
| SY05-W04 | Fetcher, verifier and metadata parser helpers with Capsicum confinement | G-06 |
| SY05-W05 | Catalogue database and loader | SY05-W04 |
| SY05-W06 | App skeleton: sidebar, explore, lists, search, filters | G-02, G-07 |
| SY05-W07 | Detail page with capability panel and trust panel | SY05-GAP-1, AD05 |
| SY05-W08 | Install, remove, transaction preview, queue, progress, authentication | SH03, SY05-W03 |
| SY05-W09 | Updates view, check, policy, selective update, capability-change review, restart prompt | SY05-W08 |
| SY05-W10 | Boot environments view: list, activate, rollback; history view | SY05-W03 |
| SY05-W11 | Repository list (read-only), network badge, audit | SE02 |
| SY05-W12 | Real `pbsd-pkg` command-line tool on the same service (replaces the placeholder) | SY05-W03 |
| SY05-W13 | A signed system repository, release keys and a release process | project infrastructure |

Phase 2, full feature:

| ID | Work item | Depends on |
|---|---|---|
| SY05-W14 | Try-once boot, health check, retention, compare, protect | boot loader support |
| SY05-W15 | Repository add, scope limits, key management, rotation, revocation | SY05-GAP-2 |
| SY05-W16 | Build records, rebuilder attestations, reproducibility badge | reproducible builds of the suite |
| SY05-W17 | SBOM ingest, view, export; system SBOM | SY05-W05 |
| SY05-W18 | Vulnerability feed, matching, list, alerts | SE01 |
| SY05-W19 | Offline bundle import and export, USB detection, feed import | SY02 |
| SY05-W20 | Hold, pin, versions, downgrade, autoremove, repair, undo any transaction, lockfile apply | SY05-W08 |
| SY05-W21 | Firmware group and hand-off; kernel and module packages | VM05 |
| SY05-W22 | Mirrors, delta fetch, route choice, metered rules | SE03, SY01 |
| SY05-W23 | Per-user installs; install from file; find by type and command; cache management | SH04 |

Phase 3, advanced:

| ID | Work item | Depends on |
|---|---|---|
| SY05-W24 | Threshold signatures and role keys | SY05-W15 |
| SY05-W25 | Live patches | kernel support, SY01 hardening state |
| SY05-W26 | Release upgrade flow | SY05-W14 |
| SY05-W27 | Required-attestation policy, proof-tier display | DV06 |
| SY05-W28 | Try in a disposable sandbox, collections, developer mode, mirror tool, fleet lockfile | AD05 |
| SY05-W29 | Licence report, exposure hints, changelog summaries | G-15 |

## 13. Open questions and risks

- There is no repository, no signing key and no update path today (`docs/ROADMAP.md`). SY05-W13 is project infrastructure, not app work, and everything waits on it.
- Whether PBSD keeps its own package format or wraps FreeBSD `pkg` for third-party ports is undecided in the documents read. The roadmap notes the external toolchain already comes from a package set that is not tracked.
- Two transaction types exist in `pbsd/pkg` (`pbsd.pkg.cppm` and `pbsd.pkg.transaction.cppm`) with different states and limits. They need to become one.
- Digest algorithm: the scaffold comments say SHA-256 and the enum offers `Sha256` and `Ed25519`; FORGE signs Ed25519 over BLAKE3. One choice is needed.
- Boot-loader support for boot-once with automatic fallback on HardenedBSD is Unverified.
- Live patching conflicts with `HARDEN_KLD` / `hardening.forbid_kmod` and with W^X. It may be impossible on the Strict profile; F104 exists to say so.
- Reproducible builds of the whole suite are assumed, not shown.
- A vulnerability feed needs someone to produce it. Mapping upstream advisories to PBSD package names is ongoing work.
- App bundles on per-app datasets: the cost of thousands of small datasets is not measured.
- No ratings, reviews or popularity: a deliberate gap compared with other stores, because they need accounts and telemetry.
