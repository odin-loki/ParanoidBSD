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
