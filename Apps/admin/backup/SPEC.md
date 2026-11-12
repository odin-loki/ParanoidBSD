<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Backup

By Odin Loch

| | |
|---|---|
| ID | `AD03` (from CATALOGUE.md) |
| Bundle | `org.pbsd.backup` |
| Category | admin |
| Priority | P1 |
| Backing repos | ParanoidBSD (OpenZFS send and receive, OpenSSH in `hbsd/src/crypto/openssh`); FUSE (UI); FORGE (ForgeScript, crypto, network) |
| Status | Planned. Nothing built. |

## 1. Purpose

Keeps copies of the user's data and system so that a deleted file, a failed disk, a stolen laptop or a bad update can be undone. It sets snapshot schedules, replicates to other disks and machines, keeps encrypted copies off site, and restores anything from one file to the whole machine. It is for every user; the defaults work without knowing ZFS. It replaces hand-written `zfs send` scripts and the Plasma-era backup tools (PLATFORM.md V20).

## 2. Scope

In scope:

- Snapshot schedules and retention per dataset.
- Replication with ZFS send and receive to local disks, removable disks and remote PBSD machines.
- Encrypted, deduplicated off-site repositories on SFTP, WebDAV and S3-compatible storage.
- Per-app include and exclude rules through `svc.backup`.
- Restore browser with a timeline; restore of files, folders, app data, datasets and the whole system (bare metal).
- Verification, test restores, health of every copy.
- Bandwidth, schedule and power rules.
- Key creation, rotation and recovery, with storage handed to SE04.

Out of scope:

| Item | Owner |
|---|---|
| Pool, vdev and dataset administration; manual snapshot housekeeping | AD02 Disks |
| Browsing snapshot versions of one file inside the file manager | SY02 Files (uses this app's index) |
| Boot environments and update rollback | SY05 Software |
| Storing keys, passphrases and remote credentials | SE04 Passwords and Keys |
| File sync between devices (two-way) | IN03 Transfers |
| Network route selection (VPN, Tor) policy | SE03 Anonymity |
| Recovery media and boot repair | AD01 Installer (repair mode) |
| VM image snapshots inside a VM | VM01 Virtual Machines |

## 3. Users and top tasks

Personas: **Asha**, who wants "it just backs up" with one external disk; **Bo**, with a home server running PBSD as a replication target; **Cal**, who keeps an off-site encrypted copy on rented storage and checks it; **Dee**, who has just lost a laptop.

| # | Task | Target |
|---|---|---|
| T1 | Turn on local snapshots with sensible retention | 1 click (or already on from first run) |
| T2 | Back up to an external disk for the first time | Plug in, 2 clicks, 1 passphrase |
| T3 | Restore a file deleted yesterday | 4 clicks (Restore ▸ pick day ▸ pick file ▸ Restore) |
| T4 | Restore an older version of a file beside the current one | 4 clicks |
| T5 | Add an off-site target (SFTP or S3-compatible) | 1 form, 5 fields, 1 test button |
| T6 | See whether every copy is current and verified | 0 clicks (Overview) |
| T7 | Exclude an app's cache or a folder | 3 clicks |
| T8 | Run a backup now | 1 click |
| T9 | Run a test restore and read the result | 2 clicks |
| T10 | Limit backup speed during work hours | 3 clicks |
| T11 | Restore the whole machine onto a new disk | Boot recovery media, 5 clicks, 1 key |
| T12 | Restore one app's data to last week | 4 clicks |
| T13 | Rotate the repository passphrase | 3 clicks, 2 fields |
| T14 | Find out why last night's backup failed | 1 click (failure line links to the log) |

## 4. Features

### 4.1 Overview and plans

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F001 | Protection overview | One status line ("Protected: 3 copies, newest 2 h ago") and a card per copy: local snapshots, each target; age of newest backup, size, last verification, next run. | M | 1 |
| AD03-F002 | 3-2-1 indicator | States how many copies, on how many media, how many off site; names what is missing. | S | 1 |
| AD03-F003 | Backup plan | A plan = sources + target + schedule + retention + rules. Several plans may share sources. | M | 1 |
| AD03-F004 | Default plan | "My data and settings": every user home dataset, system configuration, the app list, capability grants and boot environment metadata. | M | 1 |
| AD03-F005 | Plan wizard | Three steps: what, where, when. Each has a default so "Next, Next, Start" works. | M | 1 |
| AD03-F006 | Run now, pause, resume, cancel | Per plan and for all; pause survives reboot; resume continues from the last complete chunk or resume token. | M | 1 |
| AD03-F007 | Dry run | Shows what would be sent (size after dedup estimate, file count, duration estimate) without writing. | S | 2 |
| AD03-F008 | Plan duplicate, disable, delete | Deleting a plan asks what happens to its existing backups (keep, or delete with typed confirmation). | M | 1 |
| AD03-F009 | Activity log | Per run: start, end, bytes read, bytes sent, new and reused chunks, files changed, warnings, errors. Kept 1 year. | M | 1 |
| AD03-F010 | Notifications | Success is silent by default; failure, "no backup for N days", "target nearly full" and "verification failed" notify through `svc.notify`. | M | 1 |
| AD03-F011 | Status for other apps | SE01 reads protection state; SH02 shows a small indicator while a backup runs. | S | 2 |
| AD03-F012 | Plan export and import | Plans as JSON without secrets, for a second machine. | S | 2 |
| AD03-F013 | Missed-run catch-up | A run missed while the machine was off starts at the next allowed window. | M | 1 |
| AD03-F014 | Pre-update backup hook | SY05 can ask for a snapshot and, if configured, a backup run before a system update. | S | 2 |

### 4.2 Snapshot schedules and retention

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F015 | Schedules per dataset | Frequent (every 15 min), hourly, daily, weekly, monthly, yearly; each can be on or off per dataset, inherited down the tree. | M | 1 |
| AD03-F016 | Retention by count | Keep N of each class (defaults: 4 frequent, 24 hourly, 14 daily, 8 weekly, 12 monthly, 2 yearly). | M | 1 |
| AD03-F017 | Retention by age and space | Also "keep everything newer than X" and "delete oldest when the pool passes Y % full" (never the newest of each class). | S | 2 |
| AD03-F018 | Snapshot naming | `auto-<class>-<UTC time>`; user property `org.pbsd:creator=schedule` and `org.pbsd:plan=<id>` so AD02 and SY02 can label them. | M | 1 |
| AD03-F019 | Atomic recursive snapshots | One recursive snapshot per run so related datasets are consistent with each other. | M | 1 |
| AD03-F020 | Skip when unchanged | No new snapshot when `written` is zero since the last one (frequent and hourly classes). | S | 1 |
| AD03-F021 | Holds for backups | Snapshots that a target still needs as an incremental base are held (`zfs hold`) until the next successful send; bookmarks replace holds where possible to free space. | M | 1 |
| AD03-F022 | Pruning preview | Shows which snapshots the next prune will delete and how much space returns. | S | 2 |
| AD03-F023 | Protected snapshots | Pin a snapshot so retention never deletes it; pins have a note. | M | 1 |
| AD03-F024 | Event snapshots | Snapshot before: system update, app update (app data dataset), restore, large delete (asked by SY02), login of a guest. | S | 2 |
| AD03-F025 | Space report | Space held by snapshots per dataset and per class; "what would deleting these free". | S | 2 |
| AD03-F026 | Schedule runner in the service | Schedules run in `svc.backup`, not in this app, and need no window. | M | 1 |
| AD03-F027 | Laptop rules | Frequent snapshots continue on battery (cheap); replication and off-site do not, unless allowed. | M | 1 |
| AD03-F028 | Clock-jump safety | A clock moved backwards never causes deletion; retention uses snapshot creation transaction order as well as time. | M | 2 |

### 4.3 ZFS replication targets

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F029 | Target: local or removable disk | A pool on another disk; created by this app with a guided "prepare this disk for backups" (encrypted pool, one dataset per source machine). | M | 1 |
| AD03-F030 | Target: another PBSD machine | Send over an authenticated, encrypted channel to a peer running the receive service; pairing by showing a short code on both machines. | M | 2 |
| AD03-F031 | Target: any host with ZFS and SSH | `zfs receive` over SSH with a restricted key (OpenSSH is in tree: `hbsd/src/crypto/openssh`); the app prints the exact `authorized_keys` line and `zfs allow` permissions to set on the far side. | S | 2 |
| AD03-F032 | Incremental sends | Full first, then incrementals from the last common snapshot or bookmark (`zfs send -i`/`-I`, Verified flags in `hbsd/src/sys/contrib/openzfs/man/man8/zfs-send.8`). | M | 1 |
| AD03-F033 | Raw encrypted sends | For natively encrypted datasets send raw (`-w`): the target stores ciphertext and never needs the key. Shown as "target cannot read your data". | M | 1 |
| AD03-F034 | Unencrypted source to untrusted target | Refused for replication unless the target pool is encrypted by this machine's key; the app proposes the repository method (4.4) instead. | M | 2 |
| AD03-F035 | Resumable transfers | Use receive resume tokens so an interrupted send continues (`zfs send -t`). | M | 2 |
| AD03-F036 | Compressed and large-block streams | `-c`, `-L`, `-e` used when both ends support them. | S | 2 |
| AD03-F037 | Target retention | Separate retention on the target (usually longer); pruning runs on the target through the receive service or delegated permissions. | M | 2 |
| AD03-F038 | Receive safety | Received datasets are `readonly=on`, `canmount=off`, `exec=off`, `setuid=off`; received properties are not trusted (`-x mountpoint`, Verified option in `zfs-receive.8`). | M | 1 |
| AD03-F039 | Divergence handling | If the target was modified or the common snapshot is gone: explain, then offer "start a new full copy beside the old one"; never force-overwrite silently. | M | 2 |
| AD03-F040 | Removable disk rotation | Several disks as one logical target ("Disk A", "Disk B"); the app tracks each disk's state and asks for the one that is most out of date. | S | 2 |
| AD03-F041 | Auto-start on attach | A known backup disk being attached starts its plan after unlock; ejects when done if set. | M | 1 |
| AD03-F042 | Receive service | This machine can accept backups from other PBSD machines into a quota-limited dataset per peer; off by default; needs a firewall rule from SE02. | S | 3 |
| AD03-F043 | Stream check | Each stream is checked on arrival by ZFS checksums; a stream dump summary (`zstream`, Verified in tree: `hbsd/src/cddl/usr.bin/zstream`) is kept in the log for failures. | S | 2 |

### 4.4 Encrypted off-site repositories

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F044 | Repository format | Proposed own format: content-defined chunks (average 1 MiB), each compressed (zstd) then encrypted (AEAD, 256-bit key) and named by a keyed BLAKE3 hash; packs of about 64 MiB; index and snapshot manifests encrypted the same way. The store learns sizes and timing only. | M | 2 |
| AD03-F045 | Source consistency | Always reads from a ZFS snapshot, never from live files. | M | 2 |
| AD03-F046 | Deduplication | Across files, across time and across machines that share a repository key. | M | 2 |
| AD03-F047 | Target: SFTP | Host, port, user, path; key from SE04; host key pinned on first contact and shown. | M | 2 |
| AD03-F048 | Target: WebDAV | URL, user, password or token from SE04; TLS required; certificate pinning optional. | S | 2 |
| AD03-F049 | Target: S3-compatible | Endpoint, region, bucket, prefix, access key from SE04; path- or host-style addressing; works with self-hosted stores; no vendor SDK. | M | 2 |
| AD03-F050 | Target: local folder or mounted share | The same repository format on any directory handle (for USB disks with a foreign filesystem). | S | 2 |
| AD03-F051 | Append-only mode | Optional: the credentials used for backups can add but not delete; pruning then needs a second credential entered on demand. Protects against ransomware on this machine. | S | 3 |
| AD03-F052 | Object lock awareness | Detect bucket retention settings and warn when retention is shorter than the plan's or would block pruning. | C | 3 |
| AD03-F053 | Padding | Optional padding of pack sizes to hide exact data sizes. | C | 3 |
| AD03-F054 | Repository check | Structure check (index against packs present), full read check (download and decrypt everything, or a rotating N %), repair of the index from packs. | M | 2 |
| AD03-F055 | Prune and compact | Delete expired snapshots, then rewrite packs with more than 30 % dead chunks; cost estimate (download and upload bytes) shown first. | M | 2 |
| AD03-F056 | Connection test | A test button that writes, reads, lists and deletes a small object and reports each step. | M | 2 |
| AD03-F057 | Route per target | Clear, VPN or Tor per target (`net.route`); the route is shown beside the target and enforced by the service. | S | 3 |
| AD03-F058 | Cost and quota view | Stored bytes, object count, growth per month; optional price per GB-month for an estimate; warning near a quota. | C | 3 |
| AD03-F059 | Multi-machine repository | Several machines in one repository with separate snapshot lists and a shared chunk pool; each machine has its own write key record. | C | 3 |
| AD03-F060 | Stream-in-repository mode | Store a raw `zfs send` stream as chunks for exact dataset restore; chosen per plan for system datasets. | S | 3 |
| AD03-F061 | Local chunk cache | Index and recent manifests cached locally (encrypted at rest by the home dataset) so browsing is offline. | M | 2 |

### 4.5 What is backed up (per-app rules)

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F062 | Sources tree | Datasets and folders with tri-state checkboxes; size and last-change shown. | M | 1 |
| AD03-F063 | Per-app rules through `svc.backup` | Each app's config and data directories are included, cache and runtime excluded (PLATFORM.md section 10). An app may declare more: `exclude` paths, `large` items the user may skip, `sensitive` items. Shown as a row per app. | M | 2 |
| AD03-F064 | App toggle | Turn backup off for one app or only for its large items; the overview counts what is unprotected. | M | 2 |
| AD03-F065 | Quiesce protocol | Before a snapshot, `svc.backup` signals apps that registered (`prepare`, 2 s limit) so they can flush; after it, `done`. Apps that do not answer are snapshotted anyway (stores are crash-safe by rule). | S | 2 |
| AD03-F066 | Exclude patterns | Glob patterns, size limit, "has a `.nobackup` file", per plan; a tester shows which files a pattern hits. | M | 2 |
| AD03-F067 | Standard excludes | Caches, trash older than 30 days, build output directories marked by a tag file, VM disk images (asked once), downloaded models (re-downloadable). Each visible and switchable. | M | 1 |
| AD03-F068 | System state set | Package list with versions and signatures, system configuration, user records, grants and policy export from AD05, firewall rules, pool and dataset layout record for bare-metal restore. | M | 2 |
| AD03-F069 | Secrets set | The keyring and password store are backed up as their own encrypted files; never decrypted by this app. | M | 2 |
| AD03-F070 | Extended attributes and metadata | Permissions, ACLs, timestamps, extended attributes (provenance and tags), sparse regions and hard links preserved. | M | 2 |
| AD03-F071 | Large-file handling | Chunk-level change detection so a large changed file sends only changed chunks. | M | 2 |
| AD03-F072 | Unprotected-data report | Lists datasets, folders and apps not covered by any plan. | S | 2 |
| AD03-F073 | Removable and network sources | Back up an attached external disk or a mounted remote folder as a source (read through a granted handle). | C | 3 |
| AD03-F074 | VM-aware backup | Hand-off to VM01 for consistent VM snapshots before the run. | C | 3 |

### 4.6 Restore

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F075 | Restore browser | Left: timeline; middle: file tree at the chosen time; right: preview and versions of the selected file. Works the same over local snapshots, replicas and repositories. | M | 1 |
| AD03-F076 | Timeline | A zoomable strip from hours to years; each backup is a mark coloured by source (local, disk, off site) with icon; drag or arrow keys to move; calendar jump. | M | 1 |
| AD03-F077 | Version list | Every distinct version of the selected file with time, size, which copies hold it. | M | 1 |
| AD03-F078 | Changes view | "What changed in this folder between two times": added, removed, modified. | S | 2 |
| AD03-F079 | Search in backups | By name and path across all times (index kept locally); content search hands off to `svc.search` on a mounted snapshot. | S | 2 |
| AD03-F080 | Preview | Through `svc.thumbnail` and the system previewers in a sandbox; no app is launched on backup content directly. | S | 2 |
| AD03-F081 | Restore to original place | With conflict choices: keep both (suffix with the date), replace, skip; a pre-restore snapshot is taken first so restore itself can be undone. | M | 1 |
| AD03-F082 | Restore to another place | Destination chosen through the file portal. | M | 1 |
| AD03-F083 | Drag out | Drag files from the restore browser to SY02 or the desktop. | S | 2 |
| AD03-F084 | Restore app data | Pick an app and a time; the app is stopped, its data dataset or directories restored, then it may start; the previous state is kept as a snapshot. | M | 2 |
| AD03-F085 | Restore a dataset | Roll back (local), or receive from a replica into a new dataset, or rebuild from a repository; the old dataset is renamed, not destroyed, until the user confirms. | M | 2 |
| AD03-F086 | Mount a backup | Mount any backup read-only (`noexec`) and open it in SY02; repositories mount through a read-only user-space filesystem helper. | S | 2 |
| AD03-F087 | Restore from foreign machine | Open a repository or replica made by another machine using its key or recovery sheet; browse and copy out. | M | 2 |
| AD03-F088 | Selective system restore | Restore only the package list, settings, grants or firewall rules. | S | 3 |
| AD03-F089 | Restore job control | Progress, pause, cancel; partial restores are listed and can be resumed or removed. | M | 1 |
| AD03-F090 | Restore provenance | Restored files carry an attribute "restored from <copy> at <time>"; the audit trail records who restored what. | S | 2 |
| AD03-F091 | Deleted-file finder | "Show files that existed in this folder and no longer do" over a time range. | S | 2 |
| AD03-F092 | SY02 integration | SY02's "Versions" panel calls this app's index and restore operations through `svc.backup`. | M | 2 |

### 4.7 Bare-metal recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F093 | Recovery from installer media | The AD01 live media includes this app in "Restore this computer from a backup" mode. | M | 2 |
| AD03-F094 | Layout record | Each system backup stores disk sizes, partition table, pool topology, dataset tree and properties, encryption mode and boot chain settings. | M | 2 |
| AD03-F095 | Restore to same or different disks | Recreate the layout, adapt to a larger or smaller disk or a different count (asks when topology cannot be kept). | M | 2 |
| AD03-F096 | Full system from replica | Receive all datasets, set `bootfs`, reinstall the boot loader, re-seal the TPM (AD01 boot chain helper). | M | 2 |
| AD03-F097 | Full system from repository | Fresh install of the recorded package set from the media, then restore configuration, apps, grants and data. | M | 3 |
| AD03-F098 | New keys on restore | New disk passphrase and recovery key for the new disk; old ones do not carry over. | M | 2 |
| AD03-F099 | Recovery kit | A printable sheet per target: where the backup is, how to reach it, the repository id, which key is needed (never the key unless the user ticks "include recovery key"). | M | 2 |
| AD03-F100 | Recovery rehearsal | Restore the whole system into a VM (VM01) from a chosen backup and boot it, without touching the real disks. | S | 3 |
| AD03-F101 | Time-to-restore estimate | From sizes and measured target speed, shown on the overview per target. | C | 3 |

### 4.8 Verification and health

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F102 | Verify after backup | Replicas: target pool checksum state and snapshot GUID match. Repositories: new packs read back (or hashes confirmed by the store when it offers strong ones). | M | 1 |
| AD03-F103 | Scheduled verification | Weekly structure check; a rolling full read so that every chunk is read at least once per 90 days (setting). | M | 2 |
| AD03-F104 | Test restore | Restore a random sample (default 100 files or 1 GiB) to a scratch dataset, compare with hashes recorded at backup time, then delete. Result recorded. | M | 2 |
| AD03-F105 | Target scrub status | For replica targets, show last scrub and errors (data from the storage broker); offer to start one. | S | 2 |
| AD03-F106 | Staleness rules | A copy older than its plan's interval × 3 is "stale"; older than 30 days is "at risk". | M | 1 |
| AD03-F107 | Verification history | Table and chart of checks, coverage and failures per target. | S | 2 |
| AD03-F108 | Damage handling | A bad chunk is re-uploaded from local data if it still exists; otherwise the affected files and snapshots are listed. | M | 2 |
| AD03-F109 | Canary files | A known file is kept in every backup; an unexpected change in it (ransomware pattern) raises an alert before pruning. | S | 3 |
| AD03-F110 | Mass-change guard | If a run sees more than N % of files changed or deleted, retention pruning pauses and the user is asked. | S | 2 |
| AD03-F111 | Health export | Verification report as Markdown or JSON. | C | 3 |

### 4.9 Keys (hand-off to SE04)

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F112 | Repository master key | Random 256-bit key created per repository; never leaves `svc.backup` in clear. | M | 2 |
| AD03-F113 | Key wrapping | The master key is stored in the repository wrapped by (a) a passphrase (Argon2id or scrypt, parameters recorded) and (b) optional public keys. Several wraps may exist. | M | 2 |
| AD03-F114 | Store in SE04 | Passphrases, remote credentials and wrapped-key copies are items in SE04 owned by `svc.backup`; this app shows only item names and states. | M | 1 |
| AD03-F115 | Recovery sheet | Printable or saved to removable media: repository id, key wrap as words or QR; same guard rules as the AD01 recovery key page. | M | 2 |
| AD03-F116 | Passphrase change | Adds a new wrap and removes the old; no data is re-encrypted. | M | 2 |
| AD03-F117 | Master key rotation | New key for new data; optional re-encryption of old packs as a background job. | C | 3 |
| AD03-F118 | Hardware token wrap | Wrap to a key on a token held by SE04 (FIDO2 `hmac-secret` or an OpenPGP card). | C | 3 |
| AD03-F119 | Unattended unlock policy | Scheduled runs need the key without the user: kept in the session keyring while logged in, or sealed for the service; the choice is explicit per plan. | M | 2 |
| AD03-F120 | Dataset key escrow for raw sends | For raw replicas the dataset key itself is not on the target; the app checks that a wrapped copy exists in SE04 and on a recovery sheet, and warns if not. | M | 2 |
| AD03-F121 | Key check | "Test my recovery key": type it, the app proves it unwraps the master key, nothing else happens. | S | 2 |
| AD03-F122 | Revoke a machine | Remove one machine's write record from a shared repository. | C | 3 |

### 4.10 Schedule, bandwidth, power

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F123 | Schedule per plan | Hourly, daily at a time, weekly on days, on attach, on network available, manual only. | M | 1 |
| AD03-F124 | Time windows | Allowed hours per weekday (a 7×24 grid); runs outside the window pause. | S | 2 |
| AD03-F125 | Bandwidth limits | Upload and download limits per target; a second limit for chosen hours; live rate shown. | M | 2 |
| AD03-F126 | Network conditions | Only on chosen networks (by name), never on metered connections, never while a VPN is down if the target requires that route. | M | 2 |
| AD03-F127 | Power conditions | Not on battery below N %; optional "wake to back up" and "sleep when done"; holds a `svc.power` inhibit only while writing. | M | 1 |
| AD03-F128 | Load awareness | Lower I/O and CPU priority; pause when the user is busy if set ("only when idle for 5 min"). | S | 2 |
| AD03-F129 | Concurrency | One run per target; at most two targets at once by default. | S | 2 |
| AD03-F130 | Retry policy | Exponential back-off from 1 min to 6 h; network errors do not count as failures until the window ends. | M | 2 |
| AD03-F131 | Pause all | One switch (also in SH02 quick settings) pauses every plan for 1 h, until tomorrow or until resumed. | S | 2 |

### 4.11 Safety, privacy, settings

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F132 | Network badge and endpoints | The header `[net]` badge lists each target host in use; first contact with a new host is confirmed and logged by `svc.audit`. | M | 2 |
| AD03-F133 | Host key and certificate pinning | Change of a pinned SSH host key or TLS certificate stops the plan and asks. | M | 2 |
| AD03-F134 | Delete backups | Deleting snapshots, a machine's history or a whole repository needs a typed confirmation and admin approval; deletions are logged. | M | 1 |
| AD03-F135 | Ransomware resistance summary | States for each target whether a compromised session could delete it (yes for plain replicas with delegated destroy, no for append-only). | S | 3 |
| AD03-F136 | Audit records | Every run, restore, deletion, key change and target change is written to `svc.audit`. | M | 1 |
| AD03-F137 | No plaintext names off site | File names, sizes and tree shape are inside encrypted manifests. | M | 2 |
| AD03-F138 | Command line | A small `backup` command (ForgeScript entry) for status, run, list, restore; same service calls. | S | 3 |
| AD03-F139 | Local assistant help | With `ai.local`, "Explain this failure" summarises a run log; nothing is sent anywhere. | C | 3 |
| AD03-F140 | Import existing setups | Detect existing auto-snapshot properties and take over naming so two tools do not prune each other's snapshots. | C | 3 |

## 5. GUI

Layout pattern: Sidebar + content (Overview is a dashboard; Restore is three-pane master-detail).

### 5.1 Window map

```
Main window
├─ Sidebar: Overview · Restore · Plans · Snapshots · Targets · Apps and rules · Verification · Activity · Keys
├─ Content views (one per sidebar item)
├─ Inspector (F4): properties of the selected plan, target, snapshot or file version
├─ Status bar: running jobs · rate · next run · pause switch
├─ Dialogs
│   ├─ New plan (wizard: What ▸ Where ▸ When) · Edit plan
│   ├─ Add target (Disk · PBSD machine · SSH+ZFS · SFTP · WebDAV · S3-compatible · Folder)
│   ├─ Prepare backup disk · Pair with machine · Connection test result
│   ├─ Restore options (place, conflicts) · Restore app data · Restore dataset
│   ├─ Schedule and windows · Bandwidth rules · Exclude pattern tester
│   ├─ Create repository key · Change passphrase · Recovery sheet · Test recovery key
│   ├─ Delete backups (typed) · Prune preview · Divergence found
│   └─ Preferences
├─ Sheets: Run log · Verification report · Test restore report · Unprotected data
├─ Popovers: Job detail · Endpoint list ([net]) · Calendar jump · Copy holders of a version
└─ Recovery mode window (on AD01 media): Find backup ▸ Unlock ▸ Choose time ▸ Disks ▸ Restore ▸ Done
```

### 5.2 Main window

Shown at 1280×800 on Overview. States: empty, loading, error, offline, locked, read-only (5.6).

```
+----------------------------------------------------------------------------------------+
| [=] Backup                                 [search Ctrl+K]        (1)[net] [▁] [▢] [✕] |
+-----------------+----------------------------------------------------------------------+
| (2) Overview  > | (3) ✓ Protected — 3 copies, newest 2 hours ago        [Back up now]  |
|     Restore     |     2 media · 1 off site · last test restore 6 days ago, passed      |
|     Plans       +----------------------------------------------------------------------+
|     Snapshots   | (4) +--------------------+ +--------------------+ +----------------+ |
|     Targets     |     | Local snapshots  ✓ | | Disk "Backup A"  ! | | Off site S3  ✓ | |
|     Apps, rules |     | newest 12 min ago  | | newest 9 days ago  | | newest 2 h ago | |
|     Verification|     | 86 snapshots, 58G  | | stale — attach it  | | 412 GiB, 🔒    | |
|     Activity    |     | next: 14:15        | | 1.1 TiB, 🔒 raw    | | verified 3 Nov | |
|     Keys        |     | [Browse]           | | [Details]          | | [Browse]       | |
|                 |     +--------------------+ +--------------------+ +----------------+ |
|                 | (5) Not protected: VM images (210 GiB) · app "Studio" large assets   |
|                 |     [Review…]                                                        |
|                 | (6) Recent activity                                                  |
|                 |  12:00  Off site S3    ✓  1.2 GiB sent, 3,204 files changed   [Log]  |
|                 |  03:00  Verification   ✓  structure OK, 4% of data read       [Log]  |
|                 |  Tue    Disk "Backup A" ✕ disk not attached (missed 9 runs)   [Log]  |
+-----------------+----------------------------------------------------------------------+
| (7) Running: Off site S3 — idle   Next 13:00        ↑ limit 2 MB/s   [Pause all v]     |
+----------------------------------------------------------------------------------------+
```

1. Network badge in `--net` while any target connection is open; click lists hosts, routes and bytes.
2. Sidebar, 220 px, `listView`.
3. Status banner, `panel`: state icon and word, one sentence, the single primary button.
4. Copy cards, `gridView` of `panel` cards: name, state icon and word, newest backup age, size, lock and "raw" marks, one action each. Stale and failed cards sort first.
5. Unprotected-data line; opens the sheet.
6. Recent activity, `modelListView`, newest first; each row links to its run log.
7. Status bar: current job with `progressBar` when active, next run, active limit, pause menu (1 h, until tomorrow, until resumed).

### 5.3 Other views and dialogs

**Restore browser (content region, 1060×680)**
```
| Source [All copies v]   Folder /home/asha/Documents          [search in backups…]  |
| (1) |·····|···|·|··|·|·|····|···|·|··|·|■|·|·|··|  ◄ 10 Nov 2026 14:00 ►  [Today] |
|     Aug        Sep        Oct        Nov ▲ selected          zoom [day|week|month|yr]|
+--------------------------------------------+---------------------------------------+
| (2) Name            | Size  | Changed      | (3) tax-2026.ods                      |
|  v Documents        |       |              |  [preview]                            |
|    > tax-2026.ods   | 84 KB | 10 Nov 13:52 |  Versions                             |
|      letter.odt     | 21 KB | 2 Nov        |  > 10 Nov 13:52  84 KB  local, S3     |
|    ~ notes-old.md   | 3 KB  | deleted 9 Nov|    9 Nov 18:10   80 KB  local, disk,S3|
|  > Pictures         |       |              |    1 Nov 09:00   61 KB  disk, S3      |
|                     |       |              |  [Restore…] [Restore as copy] [Open]  |
+--------------------------------------------+---------------------------------------+
| 1 item selected · 84 KB · held by 2 copies        [Show changes since…] [Restore…] |
```
(1) Timeline: `timeline` widget in read-only mode with one track per copy (needs FUSE-WP-50 or `AD03-GAP-1`); the mark shape differs per copy so colour is not the only cue. (2) File tree at the selected time, `treeView` with columns; `~` rows are files that do not exist now (deleted since). (3) Preview (`image` or `markdownView` or text) and version `listView`. States: empty ("No backups yet" + [Create a plan]), loading (index fetch), locked (repository key needed, [Unlock]), offline (cached index browsable; restore disabled for remote-only versions with reason).

**New plan wizard (dialog, 720×520, step 2)**
```
| New backup plan                              1 What ▸ 2 Where ▸ 3 When        |
| Where should the copies go?                                                   |
| ( ) External disk "Backup A" (attached, 2 TiB free)                           |
| ( ) Another ParanoidBSD machine…        ( ) Server with ZFS over SSH…         |
| (•) Off-site storage:  ( ) SFTP  ( ) WebDAV  (•) S3-compatible  ( ) Folder    |
|   Endpoint [https://s3.example.net ]  Bucket [asha-backup ]  Prefix [laptop/] |
|   Access key [from Passwords and Keys: "s3 backup" v]   Route [clear v]       |
|   [Test connection]  ✓ write ✓ read ✓ list ✓ delete (0.8 s)                   |
|   Encryption: new repository key — you will get a recovery sheet next.        |
|                                                   [Cancel] [Back] [Next]      |
```

**Plans (content region)** — `tableView`: Name, Sources, Target, Schedule, Retention, Last run (state word), Next run, Enabled `toggleSwitch`; toolbar [New plan] [Run now] [Edit] [Dry run]; the inspector shows the full rule set.

**Snapshots: schedules and retention**
```
| Dataset           | 15 min | Hourly | Daily | Weekly | Monthly | Held | Space |
| v zroot/home      |  (inh) |  24    |  14   |   8    |   12    |      | 58 G  |
|     asha          |   4    |  24    |  14   |   8    |   12    |  2   | 51 G  |
|     guest         |  off   |  off   |   3   |  off   |  off    |      | 0.4 G |
| v zroot/ROOT      |  off   |  off   |   7   |   4    |  off    |      | 6 G   |
| [Edit retention…] [Prune preview…] [Pin snapshot…]   Pool 52% full · rule at 85%|
```
`treeView` with `spinBox` cells; "(inh)" marks inherited values.

**Apps and rules**
```
| [filter…        ]                         Standard excludes [Edit…]           |
| App              | Data   | Backed up        | Large items        | Excluded  |
| Mail             | 3.1 G  | [on ]            | –                  | cache     |
| Photos           | 48 G   | [on ]            | –                  | thumbnails|
| Virtual Machines | 210 G  | [on ] config only| disk images [off]  | –         |
| Studio           | 96 G   | [on ]            | asset cache [off]  | build out |
| Custom excludes:  *.iso · size > 20 GiB · folders with .nobackup   [Test…]    |
```

**Schedule and bandwidth (dialog, 680×480)**
```
| When may "Off site S3" run?                                                   |
| Every [1 hour v]   Only when: [x] on mains power [x] not metered [ ] idle     |
|      00  03  06  09  12  15  18  21                                           |
| Mon  ██████████░░░░░░░░░░░░██████████    █ full speed  ▒ limited  ░ paused    |
| Tue  ██████████▒▒▒▒▒▒▒▒▒▒▒▒██████████                                         |
| …                                                                             |
| Limited speed: ↑ [2 MB/s]  ↓ [10 MB/s]      Networks: [any v]                 |
|                                                        [Cancel] [Save]        |
```
The week grid is a `gridView` of toggle cells, paintable by drag and by keyboard (`Space` cycles a cell, `Shift+arrows` extend).

**Verification**
```
| Target        | Last structure | Data read (90 d) | Last test restore | State |
| Off site S3   | 3 Nov ✓        | 71% ██████░░     | 6 Nov ✓ 100 files | ✓ OK  |
| Disk Backup A | 1 Nov ✓        | scrub 1 Nov ✓    | never             | ! stale|
| [Verify now v] [Test restore…]                    (chart: coverage over time) |
```

**Keys**
```
| Repository "Off site S3"   id 7c1e…a90f                                       |
| Unlock methods:  ✓ Passphrase (stored in Passwords and Keys: "backup s3")     |
|                  ✓ Recovery sheet (printed 3 Nov)     – Hardware token        |
| [Change passphrase…] [New recovery sheet…] [Test recovery key…] [Add token…]  |
| Dataset keys for raw replicas: zroot/home/asha — wrapped copy in Passwords ✓  |
```

**Recovery mode (on installer media), step 3 of 6**
```
| Restore this computer                 Find ▸ Unlock ▸ 3 Time ▸ Disks ▸ Restore|
| Backup: laptop "pbsd-7f3a" in Off site S3 — 214 backups, 1 Aug to 11 Nov      |
| (timeline strip)                           Selected: 11 Nov 2026 12:00        |
| Contains: system state ✓  3 users ✓  41 apps ✓  412 GiB                       |
| Estimated time: 5 h 40 min at 20 MB/s                    [Back] [Next]        |
```

### 5.4 Menus

| Menu | Items (shortcut) |
|---|---|
| File | New plan… (`Ctrl+N`) · Add target… (`Ctrl+Shift+N`) · Open backup from another machine… (`Ctrl+O`) · Import plans… · Export plans… · Close (`Ctrl+W`) · Quit (`Ctrl+Q`) |
| Edit | Copy path (`Ctrl+C`) · Find in backups (`Ctrl+F`) · Select all (`Ctrl+A`) · Preferences (`Ctrl+,`) |
| View | Overview (`Ctrl+1`) · Restore (`Ctrl+2`) · Plans (`Ctrl+3`) · Snapshots (`Ctrl+4`) · Targets (`Ctrl+5`) · Apps and rules (`Ctrl+6`) · Verification (`Ctrl+7`) · Activity (`Ctrl+8`) · Keys (`Ctrl+9`) · Sidebar (`F9`) · Inspector (`F4`) · Refresh (`F5`) |
| Backup | Back up now (`Ctrl+B`) · Run selected plan (`Ctrl+Enter`) · Dry run (`Ctrl+Shift+D`) · Pause all (`Ctrl+Shift+P` is the palette; uses `Ctrl+.`) · Resume · Snapshot now (`Ctrl+T`) · Prune preview… |
| Restore | Restore… (`Ctrl+R`) · Restore as copy (`Ctrl+Shift+R`) · Restore app data… · Restore dataset… · Mount this backup · Show changes between… (`Ctrl+D`) · Earlier (`Alt+Left`) · Later (`Alt+Right`) · Jump to date… (`Ctrl+J`) |
| Verify | Verify now · Full read check · Test restore… (`Ctrl+Shift+T`) · Repair index… · Report… |
| Keys | Change passphrase… · Recovery sheet… · Test recovery key… · Rotate master key… |
| Help | Help (`F1`) · How restore works · Recovery checklist · About |

### 5.5 Keyboard and pointer

In the restore browser: `Left`/`Right` on the timeline move one backup; `Shift+Left`/`Right` one day; `Home`/`End` oldest and newest; `Ctrl+scroll` zooms the timeline; `Space` previews; `Enter` opens the folder; `Ctrl+R` restores. Type-ahead in the tree.

Context menus: file or folder (Restore…, Restore as copy, Restore to…, Show versions, Show changes since…, Exclude from backups, Copy path); plan (Run now, Dry run, Edit, Disable, Duplicate, Show log, Delete…); target card (Browse, Verify, Test restore, Edit, Eject disk, Remove…); snapshot (Browse, Pin, Unpin, Delete…, Open in Disks); activity row (Show log, Retry, Copy summary).

Drag and drop: files out of the restore browser to SY02 or the desktop (restore as copy); a folder from SY02 onto Plans (new plan with that source) or onto "Apps and rules" (new exclude); an attached disk from the Targets list onto a plan (change target).

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | No plan: Overview shows "Nothing is backed up yet" with [Turn on local snapshots] and [Create a backup plan]. |
| Loading | Cards render from the service's cached state at once; ages update when the service answers. |
| Error | Failed plan: card in `--danger` with the cause in words and one fix action (Attach disk, Re-enter key, Check connection). Service down: banner with [Retry]. |
| Offline | Remote targets show "waiting for network"; local snapshots and browsing of cached indexes work; nothing counts as failed until its window passes. |
| Locked | Repository key not available: lock on the card, [Unlock]; scheduled runs show "waiting for key". |
| Read-only | Without admin authority: plans and rules read-only; restoring one's own files still works. |
| Narrow (< 640) | Sidebar overlay; cards stack; restore browser shows timeline over tree, versions in a sheet. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar, activity, versions | `listView`, `modelListView` |
| Copy cards | `gridView` + `panel` |
| Plans, verification, apps tables | `tableView` |
| File tree, schedule tree, sources tree | `treeView` (tri-state `checkBox` cells) |
| Timeline | `timeline` (read-only, model-backed; FUSE-WP-50) |
| Calendar jump | `calendar`, `dateTimeEdit` |
| Week grid | `gridView` of toggle cells |
| Coverage and growth charts | `chart` |
| Progress | `progressBar` |
| Preview | `image`, `markdownView`, `codeEdit` (read-only) |
| Dialogs, forms | `dialog`, `lineEdit`, `comboBox`, `spinBox`, `radioButton`, `toggleSwitch` |

Missing: `AD03-GAP-1` a history timeline strip (marks on a zoomable time axis with several tracks, keyboard stepping); FUSE's `timeline` is a clip editor today (FUSE build plan, FUSE-WP-50), so a simpler read-only mode or a new widget is needed; shared with SY02 versions and AD04's event density strip. Secret field as `AD01-GAP-2`.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | View models, wizards, restore browser, rule editing |
| UI | FUSE UI | app host | All views |
| Backup service (`svc.backup`, PLATFORM.md section 8) | C++23 | system daemon, per-plan authority | Schedules, snapshot and retention engine, plan runner, job state, per-app rule registry, restore operations, local index |
| Replication worker | C++23 | child per run, capability mode; holds the send stream descriptor and one socket | `zfs send` stream pump, resume tokens, rate limiting |
| Repository engine | C++23 | child per run, capability mode; snapshot directory handle (read-only) + transport descriptor | Chunking, compression, encryption, pack and index writing, check, prune |
| Transport helpers (SFTP, WebDAV, S3) | C++23 | one process per target connection, `net.host` to that endpoint only | Object put, get, list, delete; TLS or SSH; no keys for backup data |
| Key agent (inside the service) | C++23 | service, locked memory | Holds unwrapped master keys; talks to `svc.keyring` |
| Restore mount helper | C++23 | sandbox | Read-only user-space view of a repository snapshot |
| Recovery mode | ForgeScript + FUSE UI | app host on AD01 media | Bare-metal flow; drives `svc.install` and `svc.storage` |

```
 Backup app ──bus──► svc.backup ──► svc.storage (snapshots, holds, send/receive handles)
                        │  key agent ◄── svc.keyring (SE04 items)
        ┌───────────────┼──────────────────────────────┐
        ▼               ▼                              ▼
 replication worker   repository engine ──pipe──► transport helper ──► remote store
 (stream fd ► socket) (chunks, AEAD)                (sees only ciphertext objects)
 apps ◄── prepare/done signals ── svc.backup (per-app rules registry)
```

The transport helper is the only process with a network socket and it never sees plaintext or keys. The repository engine has keys and data and no network.

### 6.2 Data model

| Entity | Fields |
|---|---|
| `Plan` | `id: uuid`, `name: string`, `enabled: bool`, `sources: SourceRule[]`, `target: TargetId`, `method: enum{snapshot_only,replicate,repository}`, `schedule: Schedule`, `retention: Retention`, `conditions: Conditions`, `limits: RateLimits`, `verify: VerifyPolicy`, `key_policy: enum{session,sealed,ask}` |
| `SourceRule` | `kind: enum{dataset,path,app,system_state,secrets}`, `ref: string`, `include: bool`, `patterns: string[]`, `max_file_bytes: u64?` |
| `Target` | `id: uuid`, `kind: enum{local_pool,removable_pool,pbsd_peer,ssh_zfs,sftp,webdav,s3,folder}`, `label: string`, `endpoint: string`, `credential_item: string` (SE04 item id), `pin: bytes?` (host key or certificate hash), `route: enum{clear,vpn,tor}`, `append_only: bool`, `repo_id: bytes16?` |
| `Schedule` | `kind: enum{interval,daily,weekly,on_attach,on_network,manual}`, `every_minutes: u32?`, `at: time?`, `days: set<weekday>`, `windows: bitset<168>` |
| `Retention` | `frequent: u16`, `hourly: u16`, `daily: u16`, `weekly: u16`, `monthly: u16`, `yearly: u16`, `min_age_hours: u32`, `pool_full_pct: u8?` |
| `Conditions` | `mains_only: bool`, `min_battery_pct: u8`, `unmetered_only: bool`, `networks: string[]`, `idle_minutes: u16?` |
| `RateLimits` | `up_bps: u64?`, `down_bps: u64?`, `limited_windows: bitset<168>`, `limited_up_bps: u64?` |
| `Run` | `id: u64`, `plan: uuid`, `started_ns: i64`, `ended_ns: i64`, `state: enum{running,ok,warning,failed,cancelled,paused}`, `bytes_read: u64`, `bytes_sent: u64`, `chunks_new: u64`, `chunks_reused: u64`, `files_changed: u64`, `snapshot: string`, `error: string?`, `log_ref: string` |
| `BackupPoint` | `id: bytes32`, `machine: string`, `time_ns: i64`, `source_snapshot_guid: u64`, `target: TargetId`, `root_manifest: bytes32`, `size: u64`, `kind: enum{snapshot,replica,repo}` |
| `AppRule` | `bundle_id: string`, `enabled: bool`, `exclude: string[]`, `large_items: {path: string, enabled: bool}[]`, `quiesce: bool` |
| `Verification` | `target: TargetId`, `time_ns: i64`, `kind: enum{structure,read,test_restore,scrub}`, `coverage_pct: f32`, `ok: bool`, `failures: string[]` |
| `KeyWrap` | `repo_id: bytes16`, `kind: enum{passphrase,pubkey,token}`, `kdf: string`, `params: record`, `wrapped: bytes`, `created_ns: i64`, `label: string` |

Repository on-store layout (Proposed): `config` (clear: format version, repo id, KDF parameters), `keys/<id>` (key wraps), `packs/<hh>/<pack-id>`, `index/<id>`, `snapshots/<machine>/<id>`, `locks/`. Format version 1; a reader refuses a newer major version.

Stores: plans, runs, rules in the service (`/var/db/backup/state.sqlite`, schema-versioned, itself in the system-state backup set); per-user file index cache in `~/.cache/org.pbsd.backup/index/`; app view state through `svc.settings`.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| ZFS send stream | passes through | passes through | kernel OpenZFS; never parsed by this app | kernel on receive |
| Repository format v1 (packs, index, manifests) | yes | yes | own code | repository engine (capability mode) |
| zstd | yes | yes | zstd (BSD licence, in base) | repository engine |
| AEAD (XChaCha20-Poly1305 or AES-256-GCM), BLAKE3, Argon2id | yes | yes | FORGE crypto (FORGE-WP-22); if Argon2id is absent, reference implementation (CC0/Apache-2.0) vendored | repository engine, key agent |
| SSH transport and SFTP | yes | yes | OpenSSH client in tree (`hbsd/src/crypto/openssh`, BSD licence) run as the transport helper with a fixed configuration; own SFTP client is Phase 3 | transport helper |
| HTTPS, WebDAV (RFC 4918 subset: PUT, GET, DELETE, PROPFIND, MKCOL) | yes | yes | FORGE network stack (FORGE-WP-21; coverage of TLS client Unverified); XML subset parser own code | transport helper |
| S3-compatible REST with SigV4 | yes | yes | own code (about 1 500 lines) on the same HTTPS client | transport helper |
| Peer protocol for PBSD-to-PBSD | yes | yes | session-bus-style framed records over a mutually authenticated channel (Noise-style handshake, own code on FORGE crypto) | replication worker |
| Plan export | yes | yes | JSON | app host |
| Recovery sheet | no | PDF, text, QR | own; PDF via G-16 | app host |

### 6.4 Integration

- Intents handled: `open` for a repository or backup reference (`pbsd-backup:` scheme) and for a dataset or path with a time ("show versions", from SY02). Emitted: `open` to SY02 (mounted backup), AD02 (pool and snapshot detail), SE04 (key items), AD01 (recovery media creation hint), VM01 (rehearsal), UT05.
- Services: `svc.backup`, `svc.notify`, `svc.settings`, `svc.keyring`, `svc.portal.file`, `svc.thumbnail`, `svc.search`, `svc.power`, `svc.audit`, `svc.storage` (Proposed; through `svc.backup` for changes, direct for read-only pool state).
- IPC offered by `svc.backup` to all apps: `register_rules(AppRule)`, `prepare`/`done` signals, `list_versions(handle)`, `restore_version(handle, time, destination_handle)`; an app can only list and restore versions of files it already holds a handle to.
- Other apps: SY02 (versions panel), SY05 (pre-update hook, package list for bare-metal), AD05 (policy export in system state; reverse query for "who holds files open"), SE01 (protection state), SE02 (rule for the receive service), SE03 (routes), AD01 (recovery mode on media, boot chain helper).
- Verified from the repo: `zfs send`/`receive` options cited in section 4, `zstream`, OpenSSH and SQLite present in tree. Proposed: everything else, including what `svc.backup` does beyond its one-line description in PLATFORM.md section 8.

## 7. Security and privacy

Capabilities requested:

| Capability | Reason |
|---|---|
| `fs.app` | View state and index cache |
| `fs.portal` | Restore destinations, plan import and export, recovery sheet to removable media |
| `net.none` | The app itself opens no socket; transfers happen in `svc.backup` helpers under per-target `net.host` grants shown to the user when a target is added |
| `svc.backup` | Everything the app does |
| `svc.storage` (Proposed) | Read pool and snapshot state for display |
| `svc.keyring` | Name and state of key items; creating items on the user's request |
| `svc.notify`, `svc.settings`, `svc.power`, `svc.audit` | Alerts; preferences; show power conditions; read own audit records for the activity view |
| `svc.thumbnail`, `svc.search` | Previews and search inside mounted backups |
| `svc.portal.print`, `dev.print` | Print recovery sheets |
| `dev.usb:storage` | Notice backup disks being attached |
| `bg.run` | Not requested: schedules run in the service |
| `ai.local` (optional, off) | "Explain this failure" |

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| Backed-up data at rest off site | Storage provider, thief of a backup disk | Read or alter data | Client-side AEAD; raw encrypted sends; keyed chunk ids; tamper is detected on read and by verification |
| Metadata (names, sizes, activity) | Storage provider, network observer | Traffic and object-size analysis | Names inside encrypted manifests; fixed-size-range packs; optional padding; optional Tor or VPN route; stated honestly as "sizes and timing are visible" |
| Backups themselves | Malware or an attacker in the user's session | Delete or encrypt backups (ransomware) | Append-only credentials; deletion needs admin approval; mass-change guard; canary; held snapshots; replicas the session cannot destroy |
| Keys | Other apps, memory disclosure | Steal the master key | Keys only in the service's key agent in locked memory; wrapped at rest; SE04 holds the wraps; app never receives key bytes |
| The local system | Malicious backup content (restoring from a hostile or tampered store) | Parser exploit, path traversal, privilege gain on restore | Manifests authenticated before parsing; restore writes only under the destination handle; names sanitised; no setuid, device nodes or ownership changes restored outside a full system restore; received ZFS datasets never auto-mount and have `exec=off` |
| Remote credentials | Phishing by a look-alike host | Host substitution | Pinned host keys and certificates; first contact confirmed |
| Other users' data | A local user | Restore someone else's files | The service checks the caller's handle to the original path or admin approval |
| Availability of restore | Lost passphrase | — | Recovery sheet, key test, several wraps, reminders in the overview |
| Kernel | Malicious send stream from a peer | `zfs receive` parsing in the kernel | Only accept streams from paired peers; receive into a dedicated pool or dataset with quotas; documented as T4 foreign code risk |

Stored: plans, run history and indexes locally (above). Leaves the machine: only ciphertext and protocol metadata, only to targets the user added, each listed under the `[net]` badge.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `backup.snapshots.enabled` | bool | `true` | Local snapshot schedules |
| `backup.snapshots.defaults` | record | `4/24/14/8/12/2` | Default retention counts (frequent/hourly/daily/weekly/monthly/yearly) |
| `backup.snapshots.pool_full_pct` | u8 | `85` | Start space-based pruning above this |
| `backup.notify.success` | bool | `false` | Notify on success |
| `backup.notify.stale_days` | u16 | `7` | Warn when the newest copy is older |
| `backup.verify.after_run` | bool | `true` | Verify new data after each run |
| `backup.verify.full_read_days` | u16 | `90` | Read every chunk at least this often |
| `backup.verify.test_restore_days` | u16 | `30` | Automatic test restore interval; 0 = off |
| `backup.verify.test_restore_sample` | record | `100 files / 1 GiB` | Sample size |
| `backup.guard.mass_change_pct` | u8 | `30` | Pause pruning when more files than this changed |
| `backup.power.min_battery_pct` | u8 | `100` (mains only) | Battery floor for replication and off-site runs |
| `backup.net.metered` | bool | `false` | Allow runs on metered networks |
| `backup.concurrency.targets` | u8 | `2` | Targets running at once |
| `backup.repo.chunk_avg_bytes` | u32 | `1048576` | Average chunk size for new repositories |
| `backup.repo.pack_bytes` | u32 | `67108864` | Pack size |
| `backup.repo.compression_level` | i8 | `3` | zstd level |
| `backup.restore.pre_snapshot` | bool | `true` | Snapshot before restoring in place |
| `backup.restore.conflict` | enum{ask,keep_both,replace,skip} | `ask` | Default conflict choice |
| `backup.disk.eject_when_done` | bool | `false` | Export and power off a backup disk after its run |
| `backup.log.keep_days` | u16 | `365` | Run log retention |
| `backup.ui.timeline_zoom` | enum{day,week,month,year} | `week` | Restore browser zoom |

## 9. Accessibility and languages

- The timeline has a list equivalent ("Backups" list with dates) and is fully keyboard driven; each mark is announced with date, time and which copies hold it.
- Copy state is always a word plus icon; ages are spoken in full ("nine days ago").
- The 7×24 schedule grid has a text alternative ("Monday 09:00 to 17:00 limited") and an editable list form.
- Recovery sheets are available as plain text for braille output; words-based key encoding avoids ambiguous characters.
- Dates and times follow the locale; snapshot names stay in UTC ISO form as identifiers.
- Long operations announce start, each 10 %, and the result.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| First frame | Under 150 ms (state from the service cache) |
| Restore browser: change of time | Under 100 ms for a folder of 10 000 entries from the local index |
| Index size | Under 200 bytes per file version on disk |
| Largest supported set | 50 million files, 20 TiB, 10 000 backup points per repository |
| Snapshot run | Under 1 s for 500 datasets |
| Repository engine throughput | At least 200 MiB/s per core for chunk + compress + encrypt of new data; unchanged files skipped by snapshot diff, not by re-reading |
| Service memory | Under 300 MiB during a run with a 10-million-chunk index (on-disk index with a bounded cache) |
| App memory | Under 150 MiB |
| Idle | The service wakes only at schedule times and on device or network events |

## 11. Testing and proof

Test plan (nothing has run):

- Unit: retention selection (which snapshots survive), schedule and window arithmetic including time-zone and clock jumps, rate limiter, rule matching, conflict naming, SigV4 signing against published test vectors, key wrap and unwrap.
- Golden renders: Overview in each state, restore browser, wizards, schedule grid; three themes, two scales.
- Scripted UI: T1–T14 in a VM with file-backed pools and local test servers for SFTP, WebDAV and an S3-compatible store.
- Round trip: back up a generated tree (odd names, sparse files, hard links, ACLs, extended attributes, 0-byte and multi-GiB files), restore, compare byte for byte and attribute for attribute.
- Fault injection: kill at every write boundary, drop the network, fill the target, corrupt a pack, remove an index, change the host key, revoke the credential mid-run. Every case must end in a consistent repository and a clear message.
- Fuzz: repository manifest, index and pack readers; WebDAV XML and S3 list responses; peer protocol frames; plan import.
- Differential: replication result against a manual `zfs send | zfs receive` of the same snapshots; retention against a reference implementation in a model.
- Bare-metal: restore into a VM from each target kind and boot.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Retention selector | Never deletes the newest of a class, a pinned or a held snapshot; result is independent of evaluation order | T1 | Lean model and proof; PRISM refinement | — |
| Schedule and window arithmetic | Total, no overflow, monotone under clock corrections | T2 | PRISM | — |
| Repository reader (manifest, index, pack) | No runtime error on any input; authentication checked before use | T2 | PRISM + fuzz | Engine not written |
| Chunk store | Every stored chunk decrypts to the data whose keyed hash names it | T1 (format model), T3 (code) | Proof over the format; tests | Third-party zstd in the path |
| Prune and compact | Never removes a chunk referenced by a kept snapshot | T1 | Lean proof of the mark-and-sweep on the model; PRISM refinement | Concurrency with other machines in a shared repository |
| Restore writer | Writes only under the destination handle; no path escape | T2 | Capability mode + PRISM on name handling | — |
| Transport helpers | Memory safe; cannot read keys (no handle) | T3 | Safe subset, sanitizers, fuzz; process isolation by construction | OpenSSH and TLS code are foreign |
| Key agent | Keys never written to disk or bus in clear | T3 | Review, tests with memory scanning in a harness | Side channels out of scope |
| OpenZFS send and receive | — | T4 | Upstream tests | Foreign kernel code |
| UI | Delete needs typed confirmation; state words match service state | T3 | Scripted UI tests | Not in proof scope |

## 12. Build plan

Phase 1 — usable core (local snapshots, disk replication, restore):

| ID | Work item | Depends on |
|---|---|---|
| AD03-W01 | `svc.backup` interface schema: plans, runs, rules, versions, restore | G-08, G-09, FORGE-WP-26 |
| AD03-W02 | Snapshot schedule and retention engine with holds and pins | AD02-W01 (`svc.storage`), AD03-W01 |
| AD03-W03 | Replication to a local or removable encrypted pool: full, incremental, raw; prepare-disk flow; auto-start on attach | AD02-W06, AD02-W08 |
| AD03-W04 | Local file index from snapshots; version listing | AD03-W02 |
| AD03-W05 | App shell: Overview, Plans, Snapshots, Targets, Activity | G-02, G-07, FUSE-WP-12, FUSE-WP-28, FUSE-WP-29 |
| AD03-W06 | Restore browser over local snapshots and replicas with timeline | AD03-GAP-1, AD03-W04 |
| AD03-W07 | Restore to original or other place with pre-restore snapshot | SH04 (file portal), AD03-W04 |
| AD03-W08 | Notifications, staleness rules, audit records, power conditions | `svc.notify`, `svc.power`, `svc.audit` |
| AD03-W09 | Key items in SE04 for backup disk passphrases | SE04, `svc.keyring` |

Phase 2 — full feature (off-site, per-app rules, verification, bare metal):

| ID | Work item | Depends on |
|---|---|---|
| AD03-W10 | Repository format v1 engine: chunk, pack, index, manifest, check, prune | FORGE-WP-22, FORGE-WP-27, FORGE-WP-28 |
| AD03-W11 | Transport helpers: SFTP (OpenSSH), WebDAV, S3-compatible; connection test; pinning | FORGE-WP-21, SE02 (per-target rule), SE04 |
| AD03-W12 | Key wraps, recovery sheet, passphrase change, key test, unattended policy | AD03-W10, AD01-GAP-2, AD01-GAP-3 |
| AD03-W13 | Per-app rules registry and quiesce protocol in `svc.backup`; Apps and rules view | FORGE-WP-24 (service clients), every app's manifest |
| AD03-W14 | Verification: after-run, scheduled, rolling read, test restore; Verification view | AD03-W10 |
| AD03-W15 | Remote replication: SSH+ZFS and PBSD peer with pairing; resume tokens; target retention | AD03-W03, SE02 |
| AD03-W16 | Schedule windows, bandwidth limits, network conditions, retry policy | AD03-W11 |
| AD03-W17 | System state set and layout record; recovery mode on AD01 media; restore from replica | AD01-W05, AD01-W19, AD05 (policy export), SY05 (package list) |
| AD03-W18 | Restore app data, restore dataset, mount backup, search in backups, SY02 versions API | SY02, `svc.search`, restore mount helper |
| AD03-W19 | Mass-change guard, divergence handling, disk rotation, event snapshots, SY05 pre-update hook | SY05 |

Phase 3 — advanced:

| ID | Work item | Depends on |
|---|---|---|
| AD03-W20 | Append-only mode, object-lock awareness, canary files, ransomware resistance summary | AD03-W11 |
| AD03-W21 | Full system restore from repository; recovery rehearsal in a VM | VM01, AD03-W17 |
| AD03-W22 | Receive service for other machines; multi-machine repositories; revoke a machine | SE02, AD03-W15 |
| AD03-W23 | Routes per target (VPN, Tor), padding, cost view, token wrap, master key rotation | SE03, SE04 |
| AD03-W24 | Command-line entry, assistant explain, own SFTP client, stream-in-repository mode | G-15 |

## 13. Open questions and risks

1. **`svc.backup` is one line in PLATFORM.md.** This spec gives it schedules, a plan runner, a key agent and an app-rule registry. That scope needs agreement, or part of it moves into a separate service.
2. **Own repository format versus an existing one.** An own format avoids a large third-party dependency and fits the proof targets, but restore then depends on PBSD tools. A format specification and a small stand-alone reader must ship with it.
3. **TLS and SSH client availability.** Whether FORGE-WP-21 delivers a TLS client suitable for S3 and WebDAV was not read in detail. Using the in-tree OpenSSH and OpenSSL means foreign C code in the transport helper (kept away from keys by design).
4. **Raw sends and key loss.** A raw replica is useless without the dataset key. The app must make the key escrow check hard to ignore.
5. **Unattended key availability** conflicts with "keys only while the user is present". The per-plan choice makes the trade explicit; the default (session keyring) means no off-site backup runs while logged out.
6. **Receiving streams from other machines** exposes the kernel's receive parser (T4). The receive service stays off by default and Phase 3.
7. **Per-app rules need every app to declare its large and cache data.** A manifest field for this is not in PLATFORM.md section 12 (`AD03-GAP-2`).
8. **Bare-metal restore from a repository** depends on the same package versions being available on the media or mirror; version drift needs a policy (restore data onto the newer system).
9. **Timeline widget** (`AD03-GAP-1`) does not exist in the needed form.
10. **Snapshot-space pressure** on small laptops: frequent snapshots can fill a pool; space-based pruning must be on by default.
