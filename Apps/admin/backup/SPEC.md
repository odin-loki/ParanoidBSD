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
| AD03-F### | Protection overview | One status line ("Protected: 3 copies, newest 2 h ago") and a card per copy: local snapshots, each target; age of newest backup, size, last verification, next run. | M | 1 |
| AD03-F### | 3-2-1 indicator | States how many copies, on how many media, how many off site; names what is missing. | S | 1 |
| AD03-F### | Backup plan | A plan = sources + target + schedule + retention + rules. Several plans may share sources. | M | 1 |
| AD03-F### | Default plan | "My data and settings": every user home dataset, system configuration, the app list, capability grants and boot environment metadata. | M | 1 |
| AD03-F### | Plan wizard | Three steps: what, where, when. Each has a default so "Next, Next, Start" works. | M | 1 |
| AD03-F### | Run now, pause, resume, cancel | Per plan and for all; pause survives reboot; resume continues from the last complete chunk or resume token. | M | 1 |
| AD03-F### | Dry run | Shows what would be sent (size after dedup estimate, file count, duration estimate) without writing. | S | 2 |
| AD03-F### | Plan duplicate, disable, delete | Deleting a plan asks what happens to its existing backups (keep, or delete with typed confirmation). | M | 1 |
| AD03-F### | Activity log | Per run: start, end, bytes read, bytes sent, new and reused chunks, files changed, warnings, errors. Kept 1 year. | M | 1 |
| AD03-F### | Notifications | Success is silent by default; failure, "no backup for N days", "target nearly full" and "verification failed" notify through `svc.notify`. | M | 1 |
| AD03-F### | Status for other apps | SE01 reads protection state; SH02 shows a small indicator while a backup runs. | S | 2 |
| AD03-F### | Plan export and import | Plans as JSON without secrets, for a second machine. | S | 2 |
| AD03-F### | Missed-run catch-up | A run missed while the machine was off starts at the next allowed window. | M | 1 |
| AD03-F### | Pre-update backup hook | SY05 can ask for a snapshot and, if configured, a backup run before a system update. | S | 2 |

### 4.2 Snapshot schedules and retention

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F### | Schedules per dataset | Frequent (every 15 min), hourly, daily, weekly, monthly, yearly; each can be on or off per dataset, inherited down the tree. | M | 1 |
| AD03-F### | Retention by count | Keep N of each class (defaults: 4 frequent, 24 hourly, 14 daily, 8 weekly, 12 monthly, 2 yearly). | M | 1 |
| AD03-F### | Retention by age and space | Also "keep everything newer than X" and "delete oldest when the pool passes Y % full" (never the newest of each class). | S | 2 |
| AD03-F### | Snapshot naming | `auto-<class>-<UTC time>`; user property `org.pbsd:creator=schedule` and `org.pbsd:plan=<id>` so AD02 and SY02 can label them. | M | 1 |
| AD03-F### | Atomic recursive snapshots | One recursive snapshot per run so related datasets are consistent with each other. | M | 1 |
| AD03-F### | Skip when unchanged | No new snapshot when `written` is zero since the last one (frequent and hourly classes). | S | 1 |
| AD03-F### | Holds for backups | Snapshots that a target still needs as an incremental base are held (`zfs hold`) until the next successful send; bookmarks replace holds where possible to free space. | M | 1 |
| AD03-F### | Pruning preview | Shows which snapshots the next prune will delete and how much space returns. | S | 2 |
| AD03-F### | Protected snapshots | Pin a snapshot so retention never deletes it; pins have a note. | M | 1 |
| AD03-F### | Event snapshots | Snapshot before: system update, app update (app data dataset), restore, large delete (asked by SY02), login of a guest. | S | 2 |
| AD03-F### | Space report | Space held by snapshots per dataset and per class; "what would deleting these free". | S | 2 |
| AD03-F### | Schedule runner in the service | Schedules run in `svc.backup`, not in this app, and need no window. | M | 1 |
| AD03-F### | Laptop rules | Frequent snapshots continue on battery (cheap); replication and off-site do not, unless allowed. | M | 1 |
| AD03-F### | Clock-jump safety | A clock moved backwards never causes deletion; retention uses snapshot creation transaction order as well as time. | M | 2 |

### 4.3 ZFS replication targets

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F### | Target: local or removable disk | A pool on another disk; created by this app with a guided "prepare this disk for backups" (encrypted pool, one dataset per source machine). | M | 1 |
| AD03-F### | Target: another PBSD machine | Send over an authenticated, encrypted channel to a peer running the receive service; pairing by showing a short code on both machines. | M | 2 |
| AD03-F### | Target: any host with ZFS and SSH | `zfs receive` over SSH with a restricted key (OpenSSH is in tree: `hbsd/src/crypto/openssh`); the app prints the exact `authorized_keys` line and `zfs allow` permissions to set on the far side. | S | 2 |
| AD03-F### | Incremental sends | Full first, then incrementals from the last common snapshot or bookmark (`zfs send -i`/`-I`, Verified flags in `hbsd/src/sys/contrib/openzfs/man/man8/zfs-send.8`). | M | 1 |
| AD03-F### | Raw encrypted sends | For natively encrypted datasets send raw (`-w`): the target stores ciphertext and never needs the key. Shown as "target cannot read your data". | M | 1 |
| AD03-F### | Unencrypted source to untrusted target | Refused for replication unless the target pool is encrypted by this machine's key; the app proposes the repository method (4.4) instead. | M | 2 |
| AD03-F### | Resumable transfers | Use receive resume tokens so an interrupted send continues (`zfs send -t`). | M | 2 |
| AD03-F### | Compressed and large-block streams | `-c`, `-L`, `-e` used when both ends support them. | S | 2 |
| AD03-F### | Target retention | Separate retention on the target (usually longer); pruning runs on the target through the receive service or delegated permissions. | M | 2 |
| AD03-F### | Receive safety | Received datasets are `readonly=on`, `canmount=off`, `exec=off`, `setuid=off`; received properties are not trusted (`-x mountpoint`, Verified option in `zfs-receive.8`). | M | 1 |
| AD03-F### | Divergence handling | If the target was modified or the common snapshot is gone: explain, then offer "start a new full copy beside the old one"; never force-overwrite silently. | M | 2 |
| AD03-F### | Removable disk rotation | Several disks as one logical target ("Disk A", "Disk B"); the app tracks each disk's state and asks for the one that is most out of date. | S | 2 |
| AD03-F### | Auto-start on attach | A known backup disk being attached starts its plan after unlock; ejects when done if set. | M | 1 |
| AD03-F### | Receive service | This machine can accept backups from other PBSD machines into a quota-limited dataset per peer; off by default; needs a firewall rule from SE02. | S | 3 |
| AD03-F### | Stream check | Each stream is checked on arrival by ZFS checksums; a stream dump summary (`zstream`, Verified in tree: `hbsd/src/cddl/usr.bin/zstream`) is kept in the log for failures. | S | 2 |

### 4.4 Encrypted off-site repositories

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F### | Repository format | Proposed own format: content-defined chunks (average 1 MiB), each compressed (zstd) then encrypted (AEAD, 256-bit key) and named by a keyed BLAKE3 hash; packs of about 64 MiB; index and snapshot manifests encrypted the same way. The store learns sizes and timing only. | M | 2 |
| AD03-F### | Source consistency | Always reads from a ZFS snapshot, never from live files. | M | 2 |
| AD03-F### | Deduplication | Across files, across time and across machines that share a repository key. | M | 2 |
| AD03-F### | Target: SFTP | Host, port, user, path; key from SE04; host key pinned on first contact and shown. | M | 2 |
| AD03-F### | Target: WebDAV | URL, user, password or token from SE04; TLS required; certificate pinning optional. | S | 2 |
| AD03-F### | Target: S3-compatible | Endpoint, region, bucket, prefix, access key from SE04; path- or host-style addressing; works with self-hosted stores; no vendor SDK. | M | 2 |
| AD03-F### | Target: local folder or mounted share | The same repository format on any directory handle (for USB disks with a foreign filesystem). | S | 2 |
| AD03-F### | Append-only mode | Optional: the credentials used for backups can add but not delete; pruning then needs a second credential entered on demand. Protects against ransomware on this machine. | S | 3 |
| AD03-F### | Object lock awareness | Detect bucket retention settings and warn when retention is shorter than the plan's or would block pruning. | C | 3 |
| AD03-F### | Padding | Optional padding of pack sizes to hide exact data sizes. | C | 3 |
| AD03-F### | Repository check | Structure check (index against packs present), full read check (download and decrypt everything, or a rotating N %), repair of the index from packs. | M | 2 |
| AD03-F### | Prune and compact | Delete expired snapshots, then rewrite packs with more than 30 % dead chunks; cost estimate (download and upload bytes) shown first. | M | 2 |
| AD03-F### | Connection test | A test button that writes, reads, lists and deletes a small object and reports each step. | M | 2 |
| AD03-F### | Route per target | Clear, VPN or Tor per target (`net.route`); the route is shown beside the target and enforced by the service. | S | 3 |
| AD03-F### | Cost and quota view | Stored bytes, object count, growth per month; optional price per GB-month for an estimate; warning near a quota. | C | 3 |
| AD03-F### | Multi-machine repository | Several machines in one repository with separate snapshot lists and a shared chunk pool; each machine has its own write key record. | C | 3 |
| AD03-F### | Stream-in-repository mode | Store a raw `zfs send` stream as chunks for exact dataset restore; chosen per plan for system datasets. | S | 3 |
| AD03-F### | Local chunk cache | Index and recent manifests cached locally (encrypted at rest by the home dataset) so browsing is offline. | M | 2 |

### 4.5 What is backed up (per-app rules)

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F### | Sources tree | Datasets and folders with tri-state checkboxes; size and last-change shown. | M | 1 |
| AD03-F### | Per-app rules through `svc.backup` | Each app's config and data directories are included, cache and runtime excluded (PLATFORM.md section 10). An app may declare more: `exclude` paths, `large` items the user may skip, `sensitive` items. Shown as a row per app. | M | 2 |
| AD03-F### | App toggle | Turn backup off for one app or only for its large items; the overview counts what is unprotected. | M | 2 |
| AD03-F### | Quiesce protocol | Before a snapshot, `svc.backup` signals apps that registered (`prepare`, 2 s limit) so they can flush; after it, `done`. Apps that do not answer are snapshotted anyway (stores are crash-safe by rule). | S | 2 |
| AD03-F### | Exclude patterns | Glob patterns, size limit, "has a `.nobackup` file", per plan; a tester shows which files a pattern hits. | M | 2 |
| AD03-F### | Standard excludes | Caches, trash older than 30 days, build output directories marked by a tag file, VM disk images (asked once), downloaded models (re-downloadable). Each visible and switchable. | M | 1 |
| AD03-F### | System state set | Package list with versions and signatures, system configuration, user records, grants and policy export from AD05, firewall rules, pool and dataset layout record for bare-metal restore. | M | 2 |
| AD03-F### | Secrets set | The keyring and password store are backed up as their own encrypted files; never decrypted by this app. | M | 2 |
| AD03-F### | Extended attributes and metadata | Permissions, ACLs, timestamps, extended attributes (provenance and tags), sparse regions and hard links preserved. | M | 2 |
| AD03-F### | Large-file handling | Chunk-level change detection so a large changed file sends only changed chunks. | M | 2 |
| AD03-F### | Unprotected-data report | Lists datasets, folders and apps not covered by any plan. | S | 2 |
| AD03-F### | Removable and network sources | Back up an attached external disk or a mounted remote folder as a source (read through a granted handle). | C | 3 |
| AD03-F### | VM-aware backup | Hand-off to VM01 for consistent VM snapshots before the run. | C | 3 |

### 4.6 Restore

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F### | Restore browser | Left: timeline; middle: file tree at the chosen time; right: preview and versions of the selected file. Works the same over local snapshots, replicas and repositories. | M | 1 |
| AD03-F### | Timeline | A zoomable strip from hours to years; each backup is a mark coloured by source (local, disk, off site) with icon; drag or arrow keys to move; calendar jump. | M | 1 |
| AD03-F### | Version list | Every distinct version of the selected file with time, size, which copies hold it. | M | 1 |
| AD03-F### | Changes view | "What changed in this folder between two times": added, removed, modified. | S | 2 |
| AD03-F### | Search in backups | By name and path across all times (index kept locally); content search hands off to `svc.search` on a mounted snapshot. | S | 2 |
| AD03-F### | Preview | Through `svc.thumbnail` and the system previewers in a sandbox; no app is launched on backup content directly. | S | 2 |
| AD03-F### | Restore to original place | With conflict choices: keep both (suffix with the date), replace, skip; a pre-restore snapshot is taken first so restore itself can be undone. | M | 1 |
| AD03-F### | Restore to another place | Destination chosen through the file portal. | M | 1 |
| AD03-F### | Drag out | Drag files from the restore browser to SY02 or the desktop. | S | 2 |
| AD03-F### | Restore app data | Pick an app and a time; the app is stopped, its data dataset or directories restored, then it may start; the previous state is kept as a snapshot. | M | 2 |
| AD03-F### | Restore a dataset | Roll back (local), or receive from a replica into a new dataset, or rebuild from a repository; the old dataset is renamed, not destroyed, until the user confirms. | M | 2 |
| AD03-F### | Mount a backup | Mount any backup read-only (`noexec`) and open it in SY02; repositories mount through a read-only user-space filesystem helper. | S | 2 |
| AD03-F### | Restore from foreign machine | Open a repository or replica made by another machine using its key or recovery sheet; browse and copy out. | M | 2 |
| AD03-F### | Selective system restore | Restore only the package list, settings, grants or firewall rules. | S | 3 |
| AD03-F### | Restore job control | Progress, pause, cancel; partial restores are listed and can be resumed or removed. | M | 1 |
| AD03-F### | Restore provenance | Restored files carry an attribute "restored from <copy> at <time>"; the audit trail records who restored what. | S | 2 |
| AD03-F### | Deleted-file finder | "Show files that existed in this folder and no longer do" over a time range. | S | 2 |
| AD03-F### | SY02 integration | SY02's "Versions" panel calls this app's index and restore operations through `svc.backup`. | M | 2 |

### 4.7 Bare-metal recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F### | Recovery from installer media | The AD01 live media includes this app in "Restore this computer from a backup" mode. | M | 2 |
| AD03-F### | Layout record | Each system backup stores disk sizes, partition table, pool topology, dataset tree and properties, encryption mode and boot chain settings. | M | 2 |
| AD03-F### | Restore to same or different disks | Recreate the layout, adapt to a larger or smaller disk or a different count (asks when topology cannot be kept). | M | 2 |
| AD03-F### | Full system from replica | Receive all datasets, set `bootfs`, reinstall the boot loader, re-seal the TPM (AD01 boot chain helper). | M | 2 |
| AD03-F### | Full system from repository | Fresh install of the recorded package set from the media, then restore configuration, apps, grants and data. | M | 3 |
| AD03-F### | New keys on restore | New disk passphrase and recovery key for the new disk; old ones do not carry over. | M | 2 |
| AD03-F### | Recovery kit | A printable sheet per target: where the backup is, how to reach it, the repository id, which key is needed (never the key unless the user ticks "include recovery key"). | M | 2 |
| AD03-F### | Recovery rehearsal | Restore the whole system into a VM (VM01) from a chosen backup and boot it, without touching the real disks. | S | 3 |
| AD03-F### | Time-to-restore estimate | From sizes and measured target speed, shown on the overview per target. | C | 3 |

### 4.8 Verification and health

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F### | Verify after backup | Replicas: target pool checksum state and snapshot GUID match. Repositories: new packs read back (or hashes confirmed by the store when it offers strong ones). | M | 1 |
| AD03-F### | Scheduled verification | Weekly structure check; a rolling full read so that every chunk is read at least once per 90 days (setting). | M | 2 |
| AD03-F### | Test restore | Restore a random sample (default 100 files or 1 GiB) to a scratch dataset, compare with hashes recorded at backup time, then delete. Result recorded. | M | 2 |
| AD03-F### | Target scrub status | For replica targets, show last scrub and errors (data from the storage broker); offer to start one. | S | 2 |
| AD03-F### | Staleness rules | A copy older than its plan's interval × 3 is "stale"; older than 30 days is "at risk". | M | 1 |
| AD03-F### | Verification history | Table and chart of checks, coverage and failures per target. | S | 2 |
| AD03-F### | Damage handling | A bad chunk is re-uploaded from local data if it still exists; otherwise the affected files and snapshots are listed. | M | 2 |
| AD03-F### | Canary files | A known file is kept in every backup; an unexpected change in it (ransomware pattern) raises an alert before pruning. | S | 3 |
| AD03-F### | Mass-change guard | If a run sees more than N % of files changed or deleted, retention pruning pauses and the user is asked. | S | 2 |
| AD03-F### | Health export | Verification report as Markdown or JSON. | C | 3 |

### 4.9 Keys (hand-off to SE04)

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F### | Repository master key | Random 256-bit key created per repository; never leaves `svc.backup` in clear. | M | 2 |
| AD03-F### | Key wrapping | The master key is stored in the repository wrapped by (a) a passphrase (Argon2id or scrypt, parameters recorded) and (b) optional public keys. Several wraps may exist. | M | 2 |
| AD03-F### | Store in SE04 | Passphrases, remote credentials and wrapped-key copies are items in SE04 owned by `svc.backup`; this app shows only item names and states. | M | 1 |
| AD03-F### | Recovery sheet | Printable or saved to removable media: repository id, key wrap as words or QR; same guard rules as the AD01 recovery key page. | M | 2 |
| AD03-F### | Passphrase change | Adds a new wrap and removes the old; no data is re-encrypted. | M | 2 |
| AD03-F### | Master key rotation | New key for new data; optional re-encryption of old packs as a background job. | C | 3 |
| AD03-F### | Hardware token wrap | Wrap to a key on a token held by SE04 (FIDO2 `hmac-secret` or an OpenPGP card). | C | 3 |
| AD03-F### | Unattended unlock policy | Scheduled runs need the key without the user: kept in the session keyring while logged in, or sealed for the service; the choice is explicit per plan. | M | 2 |
| AD03-F### | Dataset key escrow for raw sends | For raw replicas the dataset key itself is not on the target; the app checks that a wrapped copy exists in SE04 and on a recovery sheet, and warns if not. | M | 2 |
| AD03-F### | Key check | "Test my recovery key": type it, the app proves it unwraps the master key, nothing else happens. | S | 2 |
| AD03-F### | Revoke a machine | Remove one machine's write record from a shared repository. | C | 3 |

### 4.10 Schedule, bandwidth, power

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F### | Schedule per plan | Hourly, daily at a time, weekly on days, on attach, on network available, manual only. | M | 1 |
| AD03-F### | Time windows | Allowed hours per weekday (a 7×24 grid); runs outside the window pause. | S | 2 |
| AD03-F### | Bandwidth limits | Upload and download limits per target; a second limit for chosen hours; live rate shown. | M | 2 |
| AD03-F### | Network conditions | Only on chosen networks (by name), never on metered connections, never while a VPN is down if the target requires that route. | M | 2 |
| AD03-F### | Power conditions | Not on battery below N %; optional "wake to back up" and "sleep when done"; holds a `svc.power` inhibit only while writing. | M | 1 |
| AD03-F### | Load awareness | Lower I/O and CPU priority; pause when the user is busy if set ("only when idle for 5 min"). | S | 2 |
| AD03-F### | Concurrency | One run per target; at most two targets at once by default. | S | 2 |
| AD03-F### | Retry policy | Exponential back-off from 1 min to 6 h; network errors do not count as failures until the window ends. | M | 2 |
| AD03-F### | Pause all | One switch (also in SH02 quick settings) pauses every plan for 1 h, until tomorrow or until resumed. | S | 2 |

### 4.11 Safety, privacy, settings

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD03-F### | Network badge and endpoints | The header `[net]` badge lists each target host in use; first contact with a new host is confirmed and logged by `svc.audit`. | M | 2 |
| AD03-F### | Host key and certificate pinning | Change of a pinned SSH host key or TLS certificate stops the plan and asks. | M | 2 |
| AD03-F### | Delete backups | Deleting snapshots, a machine's history or a whole repository needs a typed confirmation and admin approval; deletions are logged. | M | 1 |
| AD03-F### | Ransomware resistance summary | States for each target whether a compromised session could delete it (yes for plain replicas with delegated destroy, no for append-only). | S | 3 |
| AD03-F### | Audit records | Every run, restore, deletion, key change and target change is written to `svc.audit`. | M | 1 |
| AD03-F### | No plaintext names off site | File names, sizes and tree shape are inside encrypted manifests. | M | 2 |
| AD03-F### | Command line | A small `backup` command (ForgeScript entry) for status, run, list, restore; same service calls. | S | 3 |
| AD03-F### | Local assistant help | With `ai.local`, "Explain this failure" summarises a run log; nothing is sent anywhere. | C | 3 |
| AD03-F### | Import existing setups | Detect existing auto-snapshot properties and take over naming so two tools do not prune each other's snapshots. | C | 3 |
