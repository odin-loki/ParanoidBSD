<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Disks

By Odin Loch

| | |
|---|---|
| ID | `AD02` (from CATALOGUE.md) |
| Bundle | `org.pbsd.disks` |
| Category | admin |
| Priority | P1 |
| Backing repos | ParanoidBSD (OpenZFS, GEOM, GELI, CAM and NVMe tools in `hbsd/src`); FUSE (UI); FORGE (ForgeScript) |
| Status | Planned. Nothing built. |

## 1. Purpose

One place to see and manage every storage device: physical disks, GPT partitions, ZFS pools, vdevs and datasets, encryption, health and removable media. It is for the owner of a workstation or laptop who needs to add a disk, replace a failing one, check health, write an image or erase a device safely. It replaces the command-line set `gpart`, `zpool`, `zfs`, `geli`, `camcontrol`, `nvmecontrol`, `dd` for common work, and the KDE partition tool from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- Disks and GPT partitions: create, delete, resize, move, type, label, attributes; MBR read and convert.
- ZFS: pools, vdevs, datasets, zvols, properties, quotas, reservations, scrub, resilver, trim, import, export, upgrade, checkpoint.
- Snapshots, clones, bookmarks, holds, rollback, diff (management view).
- Boot environments view (read and jump to SY05).
- Encryption: GELI providers and ZFS native encryption; keys load, unload, change.
- Health: SMART, NVMe logs, self-tests, pool errors, history and alerts.
- Benchmarks, image write, restore and verify, removable media, secure erase, mount options, filesystem check.
- Topology diagram.

Out of scope:

| Item | Owner |
|---|---|
| Snapshot schedules, retention, send and receive as backup, restore browsing | AD03 Backup |
| Creating, activating, deleting boot environments; updates | SY05 Software |
| Browsing files and per-file snapshot versions | SY02 Files |
| Space analysis by folder, duplicates, cleanup | UT04 Disk Usage |
| Encrypted containers and file-level encryption | SE05 Vault |
| Storing passphrases and key files | SE04 Passwords and Keys |
| USB device policy, firmware updates, IOMMU | VM05 Devices |
| VM disk images inside a VM manager | VM01 Virtual Machines |
| First install partitioning flow (shares this app's engine) | AD01 Installer |
| Live I/O charts per process | SY04 System Monitor |

## 3. Users and top tasks

Personas: **Asha**, laptop owner with one encrypted disk and USB sticks; **Bo**, workstation owner with a mirrored pool; **Cal**, a careful user who checks health and erases disks before disposal; **Dee**, a tinkerer who writes OS images.

| # | Task | Target |
|---|---|---|
| T1 | See whether every disk and pool is healthy | 0 clicks (overview on open) |
| T2 | Format a USB stick (exFAT or FAT32 or encrypted ZFS) | 3 clicks |
| T3 | Write an image file to a USB stick and verify it | 4 clicks |
| T4 | Replace a failing disk in a mirror | 3 clicks after the new disk is attached |
| T5 | Add a mirror half to a single-disk pool | 3 clicks |
| T6 | Start a scrub and see progress | 2 clicks |
| T7 | Create a dataset with a quota and its own encryption key | 4 clicks, 2 fields |
| T8 | Import a pool from a disk moved from another machine | 2 clicks + passphrase |
| T9 | Safely remove an external disk | 1 click |
| T10 | Securely erase a disk before disposal | 3 clicks + typed disk name |
| T11 | Run a SMART self-test and read the result | 2 clicks |
| T12 | Change a dataset's mount options (exec, setuid) | 3 clicks |
| T13 | Roll a dataset back to a snapshot | 3 clicks (confirmation lists what is lost) |
| T14 | Find out why a pool is degraded | 1 click (status line links to the faulted device) |

## 4. Features

### 4.1 Overview and navigation

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD02-F001 | Device tree sidebar | Sections: Pools, Disks, Removable, Images (attached memory disks), Other (swap, dumps). Each row has name, size, health icon and word. | M | 1 |
| AD02-F002 | Overview page | Cards per pool (capacity, health, last scrub, errors) and per disk (health, temperature, wear). One line at top: "All healthy" or the worst problem. | M | 1 |
| AD02-F003 | Hot-plug updates | Device arrival and removal update the tree within 1 s from kernel device events; no polling. | M | 1 |
| AD02-F004 | Stable identity | Devices keyed by serial, WWN or GPT UUID, never by kernel name, so a renumbered disk keeps its history. | M | 1 |
| AD02-F005 | Search and filter | Filter the tree by name, serial, label, pool, dataset path. | S | 1 |
| AD02-F006 | Pending-changes model | Partition edits queue as operations shown in a list; "Apply" runs them in order; "Revert" discards. ZFS actions run at once after their own confirmation. | M | 1 |
| AD02-F007 | Admin authorisation | Every changing action asks the storage broker, which shows an SH03 authentication prompt stating the exact action and device. Read-only viewing needs no prompt. | M | 1 |
| AD02-F008 | Command preview | Every dialog has "Show equivalent commands" listing the `zpool`, `zfs`, `gpart`, `geli` commands that would do the same, for learning and audit. | S | 2 |
| AD02-F009 | Job list | Long jobs (scrub, resilver, image write, erase, benchmark) appear in the status bar and as `svc.notify` progress; they continue when the window closes. | M | 1 |
| AD02-F010 | Operation history | Per-device and per-pool history: the pool's own log (`zpool history`) merged with this app's journal and `svc.audit` entries. | S | 2 |
| AD02-F011 | Protection of the running system | The boot pool, mounted system datasets, swap in use and the install medium are marked; destructive actions on them are disabled with a reason. | M | 1 |

### 4.2 Disks and partitions

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD02-F012 | Disk detail | Model, firmware, serial (hidden until "Show identifiers"), bus, link speed, capacity, logical and physical sector size, rotation rate, TRIM support, write cache, power-on hours. | M | 1 |
| AD02-F013 | Partition bar and table | Proportional bar (log scale option for tiny partitions) and table: index, type, label, start, size, filesystem, mount point, flags. | M | 1 |
| AD02-F014 | Create GPT | New GPT on an empty or erased disk; optional protective layout presets (single data, ESP + data). | M | 1 |
| AD02-F015 | Create partition | Type list (`freebsd-zfs`, `freebsd-swap`, `efi`, `freebsd-ufs`, `ms-basic-data`, `linux-data`, raw GUID), size, 1 MiB alignment, label. | M | 1 |
| AD02-F016 | Delete partition | To the pending list; refused when mounted or in a pool. | M | 1 |
| AD02-F017 | Resize and move | Grow or shrink the partition entry; moving copies data with a journal so power loss is recoverable. Grow of contained ZFS (`zpool online -e`), UFS (`growfs`) follows automatically; shrink only for empty or FAT partitions. | M | 2 |
| AD02-F018 | Edit type, label, attributes | Change GUID type, GPT label, attributes (`bootme`, `bootonce`, `bootfailed`). | S | 1 |
| AD02-F019 | MBR and other schemes | Read MBR, BSD label, APM (kernel classes exist in tree: `pbsd/geom/pbsd.geom.part_mbr.cppm`, `part_bsd`, `part_apm`); offer "convert MBR to GPT" when layout allows. | S | 2 |
| AD02-F020 | Backup and restore partition table | Save the table to a file (`gpart backup` format) and restore it; automatic backup before every apply, kept in app data. | M | 1 |
| AD02-F021 | Recover GPT | Detect a corrupt primary or secondary header and offer `recover`. | S | 2 |
| AD02-F022 | Format | FAT32, exFAT (if a writer is available), UFS2, ZFS single-disk pool, encrypted ZFS; options per type (label, cluster size, ashift). | M | 1 |
| AD02-F023 | Foreign filesystem recognition | Identify ext2/3/4, NTFS, exFAT, ISO 9660, UDF, HFS+, APFS, btrfs, XFS, LUKS, BitLocker by signature (reference: `hbsd/src/usr.sbin/fstyp`); mount those the kernel supports. | M | 1 |
| AD02-F024 | Swap management | List swap devices, add and remove, encrypted swap with one-time keys (`geli onetime`, Verified in `hbsd/src/lib/geom/eli/geli.8`). | S | 2 |
| AD02-F025 | Free-space actions | Click free space in the bar to create; drag a boundary to resize. | S | 2 |
| AD02-F026 | Disk identify | Blink the locate light through enclosure services where present. | C | 3 |
| AD02-F027 | Software RAID read-only view | Show existing GEOM mirror, stripe, concat, RAID3 sets (classes in `hbsd/src/lib/geom/`); no creation, with a note to use ZFS. | C | 3 |
| AD02-F028 | Write cache and power settings | Toggle write cache, standby timer and APM level where the device allows. | C | 3 |

### 4.3 ZFS pools and vdevs

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD02-F029 | Pool list and status | Name, size, allocated, free, fragmentation, capacity %, dedup ratio, health state, last scrub, error counts; the status text and action advice from the pool. | M | 1 |
| AD02-F030 | Create pool wizard | Pick disks or partitions, topology (stripe, mirror, RAID-Z1/2/3, dRAID), `ashift`, name, root dataset properties (compression, encryption, atime), mount point. Live usable size and tolerance. | M | 1 |
| AD02-F031 | Add vdev | Add data, log, cache, special, dedup or spare vdevs; warns in `--danger` when the redundancy of the new vdev is lower than existing ones, and that a top-level RAID-Z vdev cannot be removed. | M | 1 |
| AD02-F032 | Attach and detach | Turn a single disk into a mirror, add a mirror side, detach a side; RAID-Z expansion (attach to a raidz vdev) when the pool feature is enabled. | M | 1 |
| AD02-F033 | Replace device | Guided replace of a faulted or ageing device; shows resilver estimate; keeps the old device until resilver ends when both are present. | M | 1 |
| AD02-F034 | Online, offline, clear | Take a device offline (temporary option), bring it online, clear error counters with a note of what was cleared. | M | 1 |
| AD02-F035 | Remove vdev | Device removal for mirrors and single-disk top-level vdevs; shows the remapping memory cost before starting. | S | 2 |
| AD02-F036 | Hot spares | Add and remove spares; show which spare is in use; rely on `zfsd` for automatic activation (Verified in tree: `hbsd/src/cddl/usr.sbin/zfsd`). | S | 2 |
| AD02-F037 | Scrub | Start, pause, resume, stop; progress, rate, time left, repaired bytes; errors listed with affected files. | M | 1 |
| AD02-F038 | Scrub schedule | Monthly by default per pool; day and hour editable; skipped on battery; the schedule lives in the storage broker, not in this app. | M | 1 |
| AD02-F039 | Resilver view | Progress per device, rate, estimate; deferred resilvers listed; "restart resilver" under Advanced. | M | 1 |
| AD02-F040 | Trim | Manual trim per pool or device with progress; `autotrim` toggle. | S | 1 |
| AD02-F041 | Initialise | `zpool initialize` for new devices, with progress. | C | 3 |
| AD02-F042 | Import | List importable pools (including destroyed ones under Advanced), show their devices and state, import with options: read-only, alternate root, new name, do not mount, force (with the "last used by host X" warning). | M | 1 |
| AD02-F043 | Export | Export with a check for open files; names the apps holding them (from the handle tree, via AD05's reverse query). | M | 1 |
| AD02-F044 | Pool properties | Table of all properties with source and description; editable ones edit in place: `autoexpand`, `autoreplace`, `autotrim`, `failmode`, `comment`, `cachefile`, `bootfs`. | M | 1 |
| AD02-F045 | Feature flags and upgrade | List feature flags (disabled, enabled, active); upgrade with a warning that older systems and the boot loader may not read the pool; refuses on the boot pool unless the loader is known to support the features. | M | 2 |
| AD02-F046 | Checkpoint | Create, discard, and rewind to a pool checkpoint (rewind only from import). | S | 2 |
| AD02-F047 | Split | Split a mirrored pool into a new pool. | C | 3 |
| AD02-F048 | Destroy pool | Typed pool name required; lists datasets and snapshot count that will be lost; label clearing offered for the member disks. | M | 1 |
| AD02-F049 | Error detail | Read, write and checksum counters per device; list of files with permanent errors; link to restore from AD03. | M | 1 |
| AD02-F050 | Pool events | Live view of pool events (`zpool events`) with filters. | S | 2 |
| AD02-F051 | I/O statistics | Per-pool and per-vdev operations, bandwidth and latency histograms (`zpool iostat` data), 1 s sampling while visible only. | S | 2 |
| AD02-F052 | ARC summary | Cache size, target, hit ratio, L2ARC and special-device use; limit setting hands off to SY01. | C | 3 |
| AD02-F053 | Capacity warnings | Warn at 80 % and 90 % full and when fragmentation passes 50 %; thresholds editable. | M | 1 |

### 4.4 Datasets, properties, quotas

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD02-F054 | Dataset tree | Tree with columns: used, available, referenced, compression ratio, mount point, encryption state, snapshot count. Sortable, column chooser. | M | 1 |
| AD02-F055 | Create dataset | Name, mount point, compression, record size preset (general, database, media, VM), quota, reservation, encryption (inherit, own key), `exec`, `setuid`, `devices`, `atime`, case sensitivity. | M | 1 |
| AD02-F056 | Create volume (zvol) | Size, block size, sparse option; for VM01 and for foreign filesystems. | S | 2 |
| AD02-F057 | Rename and move | Rename within the pool; shows mount point changes that follow. | M | 1 |
| AD02-F058 | Destroy dataset | Lists children, snapshots and clones that go with it; typed name for anything with children; refuses when a hold exists. | M | 1 |
| AD02-F059 | Property editor | `propertyGrid` of every property: value, source (local, inherited from, default, received), description; "Inherit" resets a local value. | M | 1 |
| AD02-F060 | Quotas and reservations | `quota`, `refquota`, `reservation`, `refreservation` with a bar showing each against use. | M | 1 |
| AD02-F061 | User and group quotas | Table of `userused@`, `userquota@`, `groupquota@` with edit. | S | 2 |
| AD02-F062 | Mount and unmount | Mount, unmount, `canmount` setting; names the apps holding files open when unmount fails. | M | 1 |
| AD02-F063 | Mount options | `exec`, `setuid`, `devices`, `readonly`, `atime`, `relatime`, `nbmand`, `xattr`; a preset "untrusted data" sets `exec=off`, `setuid=off`, `devices=off`. | M | 1 |
| AD02-F064 | Space breakdown | Used by dataset, snapshots, children, reservation (`usedby*` properties) as a stacked bar. | M | 1 |
| AD02-F065 | Compression and dedup advice | Shows the achieved ratio; dedup can be enabled only after a memory estimate dialog. | S | 2 |
| AD02-F066 | Delegated administration | View and edit `zfs allow` permissions per user on a dataset. | C | 3 |
| AD02-F067 | Jail and VM attachment view | Shows datasets attached to jails (`zfs jail`) or used by VMs; links to VM02 and VM01. | C | 3 |
| AD02-F068 | Promote clone | Promote a clone so its origin can be removed. | S | 2 |
| AD02-F069 | Dataset templates | Saved property sets ("Downloads: noexec", "Database", "Media") applied on create. | C | 3 |
| AD02-F070 | Per-app data datasets | Shows which datasets hold which app's data when apps have their own datasets; link to AD05 for the app. | C | 3 |

### 4.5 Snapshots, clones, boot environments

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD02-F071 | Snapshot list | Per dataset: name, time, used, referenced, holds, origin of clones, creator (manual, schedule, update, install). Filter by creator and age. | M | 1 |
| AD02-F072 | Take snapshot | Name with a date default; recursive option; atomic across children. | M | 1 |
| AD02-F073 | Rollback | Confirmation lists the newer snapshots and clones that will be destroyed and the amount of changed data; offers "clone instead". | M | 1 |
| AD02-F074 | Clone | Create a writable clone at a chosen mount point. | M | 1 |
| AD02-F075 | Destroy snapshots | Single, range (from–to) or by filter; shows space that will be reclaimed (dry run first). | M | 1 |
| AD02-F076 | Holds | Add and release holds with a tag; held snapshots show a pin. | S | 2 |
| AD02-F077 | Bookmarks | Create and list bookmarks for incremental send sources. | S | 2 |
| AD02-F078 | Snapshot diff | Summary of files added, removed, modified, renamed between two snapshots (`zfs diff`); open either side in SY02. | S | 2 |
| AD02-F079 | Browse snapshot | Open the snapshot's `.zfs/snapshot/<name>` directory in SY02, read-only. | M | 1 |
| AD02-F080 | Snapshot space chart | Chart of space held uniquely by each snapshot over time. | C | 3 |
| AD02-F081 | Schedules link | A dataset's schedule and retention are shown read-only with "Edit in Backup" (AD03). | M | 1 |
| AD02-F082 | Boot environments view | List BEs with active-now and active-on-reboot flags, creation time, size, origin (reference: `hbsd/src/sbin/bectl`, `hbsd/src/lib/libbe`). Actions (activate, create, delete) open SY05. Mount read-only is offered here. | M | 1 |

### 4.6 Encryption

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD02-F083 | Encryption state everywhere | A lock icon in `--secret` and a word (encrypted, unlocked, locked, not encrypted) on every disk, partition and dataset row. | M | 1 |
| AD02-F084 | GELI: initialise | Cipher (AES-XTS 256 default), sector size 4096, optional data authentication (HMAC/SHA256), passphrase and or key file, PBKDF2 iterations calibrated (Verified options: `hbsd/src/lib/geom/eli/geli.8`). | M | 1 |
| AD02-F085 | GELI: attach and detach | Unlock with passphrase, key file or both; detach; "detach on last close" option. | M | 1 |
| AD02-F086 | GELI: keys | Two key slots (index 0 and 1, Verified in `geli.8`): set, change, delete a slot; shows which slots are in use. | M | 2 |
| AD02-F087 | GELI: metadata backup and restore | `backup` and `restore` of the provider metadata to a file chosen through the portal; reminder that the backup plus passphrase opens the disk. | M | 2 |
| AD02-F088 | GELI: kill and suspend | Destroy all key copies (instant crypto-erase) with typed confirmation; suspend and resume a provider. | S | 2 |
| AD02-F089 | ZFS: create encrypted dataset | `aes-256-gcm`, key format passphrase, raw or hex; key location prompt or a file handle from SE04. | M | 1 |
| AD02-F090 | ZFS: load and unload key | Per dataset or recursively; unmounts first on unload; key status column. | M | 1 |
| AD02-F091 | ZFS: change key | New passphrase or new wrapping key; "inherit key from parent" (Verified command: `zfs-change-key.8`). | M | 2 |
| AD02-F092 | Unlock at login or on demand | Per dataset: unlock at boot, at the owner's login (SH03), or only when asked. | S | 2 |
| AD02-F093 | Auto-lock | Lock chosen datasets and removable GELI providers on screen lock, suspend, or after idle minutes; apps holding handles into them have those handles revoked first (SI-6). | S | 2 |
| AD02-F094 | Key hand-off to SE04 | Passphrases and key files can be saved to or fetched from the keyring item for that volume; this app never stores them. | M | 1 |
| AD02-F095 | Encryption audit | A table of every dataset and device with cipher, key source and whether swap and dump devices are encrypted; unencrypted data locations flagged. | S | 2 |
| AD02-F096 | Encrypt existing data | Guided copy of an unencrypted dataset into a new encrypted one (send and receive), then swap names; needs free space; original destroyed only after verification. | S | 3 |

### 4.7 Health and SMART

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD02-F097 | Health summary | One word per disk: OK, Watch, Failing, Unknown, with the attribute that caused it. | M | 1 |
| AD02-F098 | ATA SMART attributes | Table: id, name, value, worst, threshold, raw, type (pre-fail, old age), trend arrow. Read through CAM pass-through (reference tool: `hbsd/src/sbin/camcontrol`). | M | 1 |
| AD02-F099 | NVMe health log | Critical warning bits, temperature, available spare, percentage used, data units, power cycles, unsafe shutdowns, media errors (reference: `nvmecontrol logpage`, `hbsd/src/sbin/nvmecontrol`). | M | 1 |
| AD02-F100 | SCSI and SAS logs | Grown defects, error counter log pages, temperature. | C | 3 |
| AD02-F101 | Self-tests | Start short, extended, conveyance (ATA) or NVMe device self-test; progress; abort; result log with LBA of first error. | M | 2 |
| AD02-F102 | Error logs | ATA error log and NVMe error log entries in a readable table. | S | 2 |
| AD02-F103 | History | Attributes sampled daily by the storage broker and kept for 2 years; charts for temperature, reallocated sectors, wear, pending sectors. | S | 2 |
| AD02-F104 | Alerts | Rules: new reallocated or pending sectors, spare below threshold, temperature over limit, self-test failed, pool error count rose. Sent through `svc.notify` and visible in SE01. | M | 2 |
| AD02-F105 | Failure-likelihood note | Plain statement of which attributes correlate with failure; no numeric prediction unless a local model is added later. | C | 3 |
| AD02-F106 | Temperature | Current, min, max; warning thresholds per device type. | S | 1 |
| AD02-F107 | Scheduled self-tests | Short weekly, extended monthly; off on battery. | S | 2 |
| AD02-F108 | USB bridge handling | Detect bridges that block pass-through and say "health not available through this adapter". | M | 2 |
| AD02-F109 | Health report | Export one device's data as Markdown or JSON; identifiers removed unless ticked. | S | 2 |

### 4.8 Benchmarks

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD02-F110 | Read benchmark | Non-destructive sequential read at sampled offsets across the device and random 4 KiB reads; throughput and access-time charts. | S | 2 |
| AD02-F111 | Write benchmark | Only on an empty device or inside a temporary dataset; never raw on a device with data; typed confirmation for raw. | S | 2 |
| AD02-F112 | Dataset benchmark | Sequential and random I/O on a temporary file with chosen block sizes, queue depth, sync on or off. | S | 2 |
| AD02-F113 | Latency percentiles | p50, p95, p99, max per test. | S | 2 |
| AD02-F114 | Result history and compare | Results stored per device identity; overlay two runs. | C | 3 |
| AD02-F115 | Counterfeit capacity test | Fill-and-verify test for removable media that report a false size. | S | 3 |
| AD02-F116 | Thermal guard | Pause when the device passes its warning temperature. | S | 2 |

### 4.9 Images and removable media

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD02-F117 | Write image to device | Source: raw, ISO, or compressed (`.xz`, `.zst`, `.gz`) image chosen through the portal; target: a removable device by default (internal disks need "show all"). Unmounts, writes with direct I/O, syncs. | M | 1 |
| AD02-F118 | Verify after write | Read back and compare BLAKE3 of the written range; on by default. | M | 1 |
| AD02-F119 | Image checksum and signature | Compute SHA-256, SHA-512, BLAKE3 of the source; compare with a pasted value or a checksum file; verify a detached signature with keys from SE04. | M | 1 |
| AD02-F120 | Create image from device | Whole disk or one partition to raw or `zstd`-compressed file; sparse output; read-error map on bad sectors with retries setting. | M | 2 |
| AD02-F121 | Restore image to device | As write, with a size check and the option to grow the last partition. | M | 2 |
| AD02-F122 | Attach image | Attach an image file as a memory disk (`md`), read-only by default, so its partitions appear under Images; detach. | S | 2 |
| AD02-F123 | Create blank image | Sparse file of given size, optionally partitioned and formatted. | C | 3 |
| AD02-F124 | Removable media list | USB, SD and optical devices with one-click Mount, Unmount, Safely remove (unmount, export, flush cache, power off the port where supported). | M | 1 |
| AD02-F125 | Mount policy for removable media | Default `noexec,nosuid` and no auto-open; auto-mount off in the Strict profile; a device seen for the first time asks before mounting. | M | 1 |
| AD02-F126 | Read-only attach | Attach a device read-only at the block layer for forensics or suspect media. | S | 2 |
| AD02-F127 | Optical media | Read data discs, create an image from a disc; blank and burn are phase 3. | C | 3 |
| AD02-F128 | Multi-target write | Write one image to several sticks in parallel with per-target verification. | C | 3 |
| AD02-F129 | Bootable-media check | After writing, report whether the image has an ESP, a hybrid MBR, and which firmware types it boots. | C | 3 |

### 4.10 Erase, check, repair

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD02-F130 | Quick erase | Clear partition table, ZFS labels (`zpool labelclear`) and filesystem signatures at both ends of the device. | M | 1 |
| AD02-F131 | Secure erase methods | Offered by device ability, best first: crypto-erase (GELI kill or NVMe format with crypto erase), NVMe sanitize, ATA sanitize or security erase (reference: `camcontrol sanitize`, `camcontrol security` in `hbsd/src/sbin/camcontrol/camcontrol.8`), whole-device TRIM, overwrite with random data (1 pass). Each states what it does and does not guarantee on flash. | M | 2 |
| AD02-F132 | Erase verification | Sample or full read-back after overwrite; a report with device identity, method, time and result. | S | 2 |
| AD02-F133 | Erase certificate | Signed (user key from SE04) erase report as PDF and JSON. | C | 3 |
| AD02-F134 | Frozen-state handling | Detect ATA "security frozen" and explain the suspend-resume workaround. | S | 2 |
| AD02-F135 | Free-space wipe | Not offered on ZFS; the page explains why and points to snapshots and crypto-erase. | S | 2 |
| AD02-F136 | Filesystem check | `fsck` for UFS and FAT (reference: `hbsd/src/sbin/fsck_ffs`, `fsck_msdosfs`) with a dry-run report first; for ZFS the action is "Scrub". | M | 2 |
| AD02-F137 | Pool recovery import | Guided recovery options in order: read-only import, rewind to an earlier transaction (`-F`, dry run `-n` first), checkpoint rewind. | S | 3 |
| AD02-F138 | Bad-sector surface scan | Read-only scan with a block map. | C | 3 |
| AD02-F139 | Data rescue hand-off | When a device has read errors, offer "create image with error map" before any repair. | S | 2 |

### 4.11 Topology, reports, settings, safety

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD02-F140 | Topology diagram | Graph: controller ▸ disk ▸ partition ▸ encryption layer ▸ vdev ▸ pool ▸ dataset. Nodes coloured by health with icon and word; click selects the same item in the tree. | M | 2 |
| AD02-F141 | Diagram layouts | Physical (by controller and port) and logical (by pool); export as SVG and PNG. | S | 2 |
| AD02-F142 | Storage report | One document of all devices, pools, datasets, encryption and health for records; Markdown, JSON, PDF. | S | 2 |
| AD02-F143 | Typed confirmations | Destroy, erase and kill actions need the device or pool name typed; the default button is never the destructive one. | M | 1 |
| AD02-F144 | Automatic safety snapshot | Before rollback, rename, property changes that affect data layout, and destroy of a dataset without children, offer or take `@before-<action>-<time>`. | S | 2 |
| AD02-F145 | Dry run | Where ZFS supports `-n`, the dialog shows the dry-run result before the real run. | M | 1 |
| AD02-F146 | Audit records | Every changing action is written to `svc.audit` with actor, device identity, action and result. | M | 1 |
| AD02-F147 | Explain mode | Each property and state has a one-line explanation and a link to the handbook page (UT05). | S | 2 |
| AD02-F148 | Local assistant help | With `ai.local` granted, "Explain this status" sends the pool status text to `svc.model` for a plain-language summary; never changes anything. | C | 3 |
| AD02-F149 | Units | Binary units (GiB) by default with a decimal option; exact bytes on hover. | S | 1 |
| AD02-F150 | Open from other apps | `open` intent with a device or dataset reference selects it (used by SY02, UT04, SE01, AD03). | M | 1 |
| AD02-F151 | Read-only mode | Without admin authority the app is a complete viewer; changing actions are disabled with the reason. | M | 1 |

## 5. GUI

Layout pattern: Sidebar + content, with an inspector (DESIGN_SYSTEM.md sections 3–4). Default density compact.

### 5.1 Window map

```
Main window
├─ Sidebar: device tree (Pools · Disks · Removable · Images · Other)
├─ Content tabs (depend on selection)
│   ├─ Overview (nothing selected)
│   ├─ Disk: Partitions · Health · Benchmark · History
│   ├─ Pool: Status · Datasets · Snapshots · Properties · Statistics · Events · History
│   ├─ Dataset: Summary · Snapshots · Properties · Quotas
│   └─ Topology (whole system)
├─ Inspector (F4): properties of the selected row
├─ Status bar: jobs · pending changes · authority state
├─ Dialogs
│   ├─ Create pool (wizard, 4 steps) · Add vdev · Replace device · Import pool · Export · Destroy pool
│   ├─ Create dataset · Create volume · Destroy dataset · Rollback · Clone · Destroy snapshots
│   ├─ New partition · Resize / move · Format · Apply pending changes
│   ├─ Encrypt (GELI init) · Unlock · Change key · Metadata backup / restore
│   ├─ Write image · Create image · Verify checksum · Secure erase · Self-test
│   └─ Preferences · Equivalent commands · Error detail
├─ Sheets: Scrub detail · Resilver detail · SMART attribute history · Job log
└─ Popovers: Job list · Health reason · Property description · Capacity breakdown
```

### 5.2 Main window

Shown at 1280×800 with a pool selected, Status tab. States: empty, loading, error, locked, read-only (see 5.6).

```
+----------------------------------------------------------------------------------------+
| [=] Disks — zroot                          [search Ctrl+K]            [▁] [▢] [✕]      |
+--------------------+---------------------------------------------------+---------------+
| (1) POOLS          | (3) [Status][Datasets][Snapshots][Properties][Stats][Events]      |
| > zroot   1.8T ✓   +---------------------------------------------------+ (6) INSPECTOR |
|   tank    7.2T ! D | (4) zroot   ONLINE ✓   mirror · 2 devices         | ada0p3.eli    |
| DISKS              |  [██████████░░░░░░░░░░] 52% of 1.8 TiB  frag 11%  | State ONLINE  |
|   nda0 Samsung ✓   |  Last scrub 3 Nov, 0 errors · next 3 Dec          | Read err  0   |
|   ada0 WDC Red ✓   |  [Scrub now] [Add device…] [Export] [More v]      | Write err 0   |
|   ada1 WDC Red ! W +---------------------------------------------------+ Cksum err 0   |
| REMOVABLE          | (5) Device            | State   | Read|Write|Cksum| Size 1.8 TiB  |
|   da1 Kingston 🔒  |  v zroot              | ONLINE  |  0  |  0  |  0  | Encrypted:    |
| IMAGES             |    v mirror-0         | ONLINE  |  0  |  0  |  0  |  GELI AES-XTS |
|   md0 debian.iso   |      > ada0p3.eli     | ONLINE  |  0  |  0  |  0  | Disk: ada0    |
| OTHER              |        ada1p3.eli     | ONLINE  |  0  |  0  |  2  |  WDC Red 4TB  |
|   swap  8G 🔒      |    v logs             |         |     |     |     |  Health ✓ OK  |
|                    |        nda0p4         | ONLINE  |  0  |  0  |  0  |  34 °C        |
| [+ Pool] [Topology]|  [Replace…] [Offline] [Online] [Clear errors] [Detach]            |
+--------------------+---------------------------------------------------+---------------+
| (7) Jobs: scrub tank 41% · 2 h left   Pending: 0      (8) Admin: ask each time  GiB    |
+----------------------------------------------------------------------------------------+
```

1. Sidebar, 220 px, `treeView`. Each row: name, size, health icon plus a letter (✓ OK, W watch, D degraded, F failed), lock for encrypted.
2. (Header) search filters the tree; `Ctrl+K` opens the command palette.
3. Tab bar, `tabBar`; the tab set follows the selection type.
4. Summary card, `panel`: state word and icon, capacity `progressBar`, scrub line, primary actions. One primary button only (Scrub now, or Replace… when degraded).
5. Vdev tree, `treeView` with numeric columns in tabular figures; non-zero error counts in `--warn` or `--danger` with a word in the tooltip; action row below acts on the selected device.
6. Inspector, 280 px, `propertyGrid`, read-only values with copy buttons; identifiers hidden until revealed.
7. Status bar left: running jobs (click opens the job list popover), pending partition changes with [Apply] [Revert].
8. Status bar right: admin authority state (ask each time, or "unlocked for 5 min"), unit switch.

### 5.3 Other views and dialogs

**Disk ▸ Partitions (content region, 820×480)**
```
| nda0  Samsung SSD 990 PRO 1TB   NVMe · 931 GiB · 4K · ✓ OK 38 °C   |
| (1) [ESP][swap 8G][ zfs  zroot  (encrypted)                 ][free]|
| # | Type         | Label   | Size   | Contains          | Mounted  |
| 1 | efi          | efiboot | 512M   | FAT32             | /boot/efi|
| 2 | freebsd-swap | swap0   | 8G     | swap 🔒 one-time  | in use   |
| 3 | freebsd-zfs  | zfs0    | 921G   | GELI ▸ zroot      | pool     |
| – | free         |         | 1.5G   |                   |          |
| [+ New] [Delete] [Resize…] [Format…] [Encrypt…]   Table: [Backup…] |
| (2) Pending: none                                                  |
```
(1) bar drawn in a `panel`; segments are focusable and draggable at their edges. (2) pending list appears here as numbered rows with [Apply 3 changes] [Revert]. States: empty disk ("No partition table" + [Create GPT]), read-only, system disk (destructive buttons disabled with reason).

**Pool ▸ Datasets**
```
| [+ Dataset] [+ Volume] [Snapshot] [filter…        ]   Columns v    |
| Name               | Used  | Avail | Refer | Ratio | Mount   | Key |
| v zroot            | 940G  | 860G  | 96K   | 1.42x | none    | –   |
|   v ROOT           | 21G   |       |       |       |         |     |
|       default  ★   | 21G   | 860G  | 18G   | 1.61x | /       | –   |
|   v home           | 610G  |       |       |       | /home   |     |
|     > asha      🔓 | 598G  | 860G  | 540G  | 1.12x | /home/… | own |
|       guest     🔒 | 12G   | –     | –     | –     | locked  | own |
| (1) [dataset 540G][snapshots 58G][children 0][reservation 0]       |
```
`treeView` with columns; (1) space breakdown bar for the selected dataset. ★ marks the active boot environment.

**Create pool (dialog wizard, 760×520, step 2 of 4)**
```
| Create pool                                   1 Disks ▸ 2 Layout ▸ 3 Options ▸ 4 Review |
| Layout (•) Mirror ( ) RAID-Z1 ( ) RAID-Z2 ( ) Stripe ( ) Custom…    |
| Usable 3.6 TiB of 7.3 TiB · survives 1 disk failure                 |
| (1) tank ─ mirror-0 ─┬ ada0 WDC Red 4TB                             |
|                      └ ada1 WDC Red 4TB                             |
| ! ada1 has 2 checksum errors recorded. [Health…]                    |
| [x] Encrypt (GELI under ZFS)  ashift [12 v]                         |
|                                        [Cancel] [Back] [Next]       |
```
(1) `nodeGraph` read-only; in Custom, disks drag between vdev boxes.

**Disk ▸ Health**
```
| ada1 WDC Red 4TB      Health: WATCH — 8 reallocated sectors (was 0) |
| Temp 36 °C (max 47)  Power-on 21,400 h  Start/stop 312              |
| [Short test] [Extended test] [Abort]   Last: extended, 1 Nov, passed|
| ID  | Attribute              | Value | Worst | Thresh | Raw  | Trend|
|   5 | Reallocated sectors    | 199   | 199   | 140    | 8    |  ↑ ! |
| 197 | Current pending        | 200   | 200   | 0      | 0    |  →   |
| 194 | Temperature            | 114   | 103   | 0      | 36   |  →   |
| (1) [chart: reallocated sectors, 24 months]        [Export report…] |
```
`tableView` + `chart` (line). NVMe devices show the health log as a `propertyGrid` instead of the attribute table.

**Write image (dialog, 640×420)**
```
| Write an image to a device                                          |
| Image  [debian-13.iso            ] [Choose…]   3.9 GiB              |
|   SHA-256 [paste expected value…                 ]  ✓ matches       |
|   Signature: ✓ signed by "Debian CD signing key" (from Passwords)   |
| Target [da1 Kingston 32 GB (removable) v]   [ ] Show internal disks |
| [x] Verify after writing     Everything on da1 will be erased.      |
| ████████░░░░ 62% · 41 MB/s · 0:38 left        [Cancel] [Write image]|
```

**Secure erase (dialog, 640×460)**
```
| Erase ada1 WDC Red 4TB (…K7LD)                                      |
| Method                         | Time    | What it does             |
| ( ) Quick (labels only)        | seconds | Data stays recoverable   |
| (•) ATA sanitize (overwrite)   | ~7 h    | Drive overwrites all LBAs|
| ( ) Overwrite with random data | ~8 h    | One pass from the host   |
| ! Drive is "frozen": suspend and resume to unlock.   [Suspend now]  |
| [x] Verify afterwards  [x] Save erase report                        |
| Type the disk name to confirm: [ada1____]     [Cancel] [Erase disk] |
```

**Rollback (dialog)**
```
| Roll zroot/home/asha back to @daily-2026-11-10?                     |
| This destroys 2 newer snapshots:  @daily-2026-11-11, @manual-tax    |
| and discards 3.1 GiB written since then (412 files changed) [List…] |
| ( ) Take a snapshot of the current state first — not possible:      |
|     rollback removes newer snapshots.  (•) Clone the old state instead |
|                              [Cancel] [Create clone] [Roll back]    |
```

**Import pool (dialog)** — table of importable pools (name, id, state, last host, devices), options row (`[ ] read-only`, `[ ] do not mount`, `alt root [____]`, `new name [____]`), unlock field when encrypted, [Import].

**Topology (content region)**
```
| View (•) Logical ( ) Physical        [Fit] [Export SVG]   zoom 100% |
|  [ahci0]──[ada0 ✓]──[p3]──[GELI 🔒]──┐                              |
|  [ahci0]──[ada1 W]──[p3]──[GELI 🔒]──┴─[mirror-0]──[zroot ✓]──[ROOT]|
|  [nvme0]──[nda0 ✓]──[p4]─────────────[log]───────────┘      ├─[home]|
|  [xhci0]──[da1  ✓]──[p1]──[GELI 🔒 locked]                  └─[var] |
```
`nodeGraph`, read-only, with pan and zoom; arrow keys move between nodes; `Enter` selects the item in the tree.

**Unlock (dialog)** — volume name, passphrase secret field, optional key file [Choose…] or [Use item from Passwords and Keys], `[ ] lock again on screen lock`, [Unlock].

### 5.4 Menus

| Menu | Items (shortcut) |
|---|---|
| File | Attach image… (`Ctrl+O`) · Write image to device… (`Ctrl+W` is reserved for close; uses `Ctrl+Shift+I`) · Create image from device… · Verify checksum… · Export storage report… (`Ctrl+Shift+S`) · Close (`Ctrl+W`) · Quit (`Ctrl+Q`) |
| Edit | Undo pending change (`Ctrl+Z`) · Redo (`Ctrl+Shift+Z`) · Copy value (`Ctrl+C`) · Copy as command · Find (`Ctrl+F`) · Preferences (`Ctrl+,`) |
| View | Overview (`Ctrl+1`) · Topology (`Ctrl+2`) · Sidebar (`F9`) · Inspector (`F4`) · Job list (`Ctrl+J`) · Show identifiers · Units ▸ Binary / Decimal · Refresh (`F5`) |
| Disk | New partition table… · New partition… (`Ctrl+N`) · Delete (`Delete`) · Resize / move… (`Ctrl+R`) · Format… · Encrypt… · Backup table… · Restore table… · Self-test ▸ Short / Extended · Benchmark… · Secure erase… · Safely remove (`Ctrl+E`) |
| Pool | Create pool… (`Ctrl+Shift+N`) · Import… (`Ctrl+I`) · Export · Scrub now (`Ctrl+B`) · Pause scrub · Trim · Add device… · Replace device… · Clear errors · Checkpoint ▸ Create / Discard · Upgrade features… · Destroy… |
| Dataset | New dataset… (`Ctrl+D`) · New volume… · Rename (`F2`) · Mount / Unmount (`Ctrl+M`) · Load key / Unload key (`Ctrl+U`) · Change key… · Snapshot now (`Ctrl+T`) · Rollback… · Clone… · Promote · Destroy… |
| Tools | Apply pending changes (`Ctrl+Enter`) · Revert pending · Equivalent commands · Open in Files · Open in Backup · Open in Disk Usage · Open terminal here |
| Help | Help (`F1`) · ZFS concepts · Keyboard shortcuts · About |

### 5.5 Keyboard and pointer

App shortcuts are in 5.4. Also: `Alt+1`…`Alt+7` switch content tabs; `Space` expands a tree node; `Ctrl+Shift+C` copies the selected row as text.

Context menus:

| Target | Items |
|---|---|
| Disk | New partition table, Self-test, Benchmark, Create image, Write image, Secure erase, Safely remove, Show in topology, Copy identity |
| Partition | Format, Resize / move, Delete, Edit label and type, Encrypt, Mount / Unmount, Open in Files |
| Pool | Scrub, Add device, Export, Properties, Snapshot recursively, Show history, Destroy |
| Vdev member | Replace, Offline / Online, Detach, Clear errors, Show disk health |
| Dataset | New child, Snapshot, Mount / Unmount, Load / Unload key, Properties, Open in Files, Open in Backup, Destroy |
| Snapshot | Browse, Rollback, Clone, Hold / Release, Diff with…, Destroy |

Drag and drop: an image file dropped on a removable device opens Write image prefilled; a disk dragged onto a pool opens Add device; disks drag between vdev boxes in Create pool ▸ Custom; partition edges drag to resize (queued). Nothing destructive happens on drop without its dialog.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | No pool: Overview shows the disks and one button "Create a pool". No removable media: section header only. |
| Loading | Tree appears from cached identity data at once; rows show a spinner until the broker answers; health cells say "reading". |
| Error | Broker unreachable: banner "Storage service is not running" with [Retry] and [Details]; the app stays open with cached read-only data marked stale. |
| Offline | Not applicable; the app never uses the network. |
| Locked | Locked volumes show 🔒 and one action, Unlock. Their children are hidden, not guessed. |
| Read-only | No admin authority: all changing controls disabled with the tooltip "Needs administrator approval". |
| Degraded pool | The pool row and Overview show the problem line and the single recommended action. |
| Narrow (< 640) | Sidebar becomes an overlay; tabs collapse to a dropdown; the inspector becomes a sheet; tables keep name, state and size columns. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Device tree, vdev tree, dataset tree | `treeView` |
| Partition, SMART, snapshot, property-source tables | `tableView` (virtualised) |
| Property editors, inspector | `propertyGrid` |
| Tabs | `tabBar`, `tabWidget` |
| Capacity, jobs | `progressBar` |
| History, I/O, benchmark charts | `chart` (line, bar, histogram; FUSE-WP-46) |
| Topology, create-pool layout | `nodeGraph` (read-only and drag modes; FUSE-WP-50) |
| Partition bar, space breakdown bar | `panel` + custom draw (FUSE-WP-23) |
| Dialogs, wizard | `dialog`, `radioButton`, `checkBox`, `comboBox`, `spinBox`, `lineEdit`, `slider` |
| Command preview | `codeEdit` (read-only) |
| Help text | `markdownView` |

Missing: `AD02-GAP-1` segmented proportional bar widget with draggable boundaries (partition bar; shared with AD01, UT04); `AD02-GAP-2` automatic layered graph layout for `nodeGraph` (the widget places nodes by hand today, per FUSE-WP-50 design notes); secret field as `AD01-GAP-2`.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic (`main.fs`, `src/*.fs`) | ForgeScript | app host | View models, pending-change list, dialogs, validation, command preview text |
| UI (`ui/*.fuseui`, `style/*.css`) | FUSE UI | app host | All views |
| Storage broker (`svc.storage`, Proposed) | C++23 | privileged system daemon | The only holder of raw disk and pool-admin handles; typed operations; authorisation; job engine; schedules for scrub and self-tests; health sampling |
| Storage worker | C++23 | child of the broker per job, capability mode, handles for the named devices only | Image write and read, erase, benchmark, verify |
| Image helper | C++23 | app-side helper, capability mode | Decompress and hash image files chosen through the portal; no device access |
| Health store | C++23 (inside the broker) | broker | SQLite history of SMART and pool samples |
| Device event source | C++23 (inside the broker) | broker | Kernel device and pool events → bus signals |

```
 Disks app (ForgeScript + FUSE UI) ──session bus──► svc.storage (privileged)
      │  read: tree, status, health, jobs             │ authorise via SH03 prompt
      │  write: typed requests + confirmation         ├─► libzfs_core / ioctl ─► OpenZFS
      │                                               ├─► GEOM control (part, eli, label)
 image helper (hash, decompress) ──fd──► broker ──►   ├─► CAM / NVMe pass-through (health)
                                                      └─► storage worker (one per job, only its device handles)
```

The app holds no device handle at any time. Image data moves as a passed descriptor from the image helper to the worker (PLATFORM.md section 7).

### 6.2 Data model

| Entity | Fields |
|---|---|
| `Device` | `id: DeviceId` (serial or WWN hash), `kernel_name: string`, `model: string`, `serial: string`, `bus: enum{nvme,sata,sas,usb,sd,virtio,md}`, `bytes: u64`, `sector_logical: u32`, `sector_physical: u32`, `rotational: bool`, `removable: bool`, `trim: bool`, `health: HealthState`, `temperature_c: i16?`, `flags: set{system,install_media,in_pool,encrypted}` |
| `Partition` | `device: DeviceId`, `index: u32`, `type_guid: uuid`, `uuid: uuid`, `label: string`, `start_lba: u64`, `sectors: u64`, `attributes: u64`, `content: ContentKind`, `mountpoint: string?` |
| `CryptoLayer` | `provider: string`, `kind: enum{geli,zfs_native}`, `cipher: string`, `key_bits: u16`, `state: enum{locked,unlocked}`, `slots_used: u8`, `auth: bool`, `keyring_item: string?` |
| `Pool` | `guid: u64`, `name: string`, `state: enum{online,degraded,faulted,offline,unavail,removed}`, `size: u64`, `alloc: u64`, `free: u64`, `frag_pct: u8`, `scan: ScanState`, `errors: ErrorCounts`, `features: map<string,enum{disabled,enabled,active}>`, `properties: Property[]`, `root_vdev: Vdev` |
| `Vdev` | `guid: u64`, `kind: enum{root,mirror,raidz1,raidz2,raidz3,draid,disk,file,log,cache,special,dedup,spare,replacing}`, `state`, `read_err: u64`, `write_err: u64`, `cksum_err: u64`, `children: Vdev[]`, `device: DeviceId?`, `resilver: ScanState?` |
| `Dataset` | `guid: u64`, `name: string`, `kind: enum{filesystem,volume,snapshot,bookmark}`, `used: u64`, `avail: u64`, `refer: u64`, `mountpoint: string?`, `mounted: bool`, `key_status: enum{none,available,unavailable}`, `encryption_root: string?`, `origin: string?`, `properties: Property[]` |
| `Property` | `name: string`, `value: string`, `source: enum{local,inherited,default,received,temporary}`, `inherited_from: string?`, `editable: bool` |
| `Snapshot` | `Dataset` + `created_ns: i64`, `used_unique: u64`, `holds: string[]`, `creator: enum{manual,schedule,update,install,app}` (from a user property `org.pbsd:creator`) |
| `HealthSample` | `device: DeviceId`, `time_ns: i64`, `kind: enum{ata,nvme,scsi}`, `attributes: map<u16, AttrValue>`, `summary: HealthState` |
| `Job` | `id: u64`, `kind: enum{scrub,resilver,trim,image_write,image_read,verify,erase,benchmark,selftest,move}`, `target: string`, `state: enum{queued,running,paused,done,failed,cancelled}`, `done: u64`, `total: u64`, `rate: u64`, `started_ns: i64`, `message: string` |
| `PendingOp` | `seq: u32`, `kind: enum{create_table,create_part,delete_part,resize,move,set_type,set_label,format}`, `device: DeviceId`, `args: record`, `undo_hint: string` |
| `BenchmarkResult` | `device: DeviceId`, `time_ns: i64`, `kind: string`, `block_size: u32`, `mbps: f64`, `iops: f64`, `p50_us: u32`, `p95_us: u32`, `p99_us: u32` |

Stores: the app keeps only view state, partition-table backups (`~/.local/share/org.pbsd.disks/table-backups/<device-id>/<time>.gpt`) and benchmark results (`bench.sqlite`, schema v1). Health history lives in the broker (`/var/db/storage/health.sqlite`). Truth about pools and datasets is always read from the kernel through the broker; nothing is cached across restarts except identity for fast first paint.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| GPT, MBR, BSD label, APM | yes | GPT (MBR for removable media) | kernel `geom_part` classes (`pbsd/geom/pbsd.geom.part_*.cppm`) | kernel via broker |
| ZFS pool configuration, properties, events, history | yes | yes | `libzfs_core` and ZFS ioctls from `hbsd/src/sys/contrib/openzfs` | broker |
| GELI metadata | yes | yes | `hbsd/src/lib/geom/eli` | broker |
| ATA SMART, NVMe log pages, SCSI log pages | yes | commands only | own decoders over CAM and NVMe pass-through | broker (decoder is T2 target) |
| Filesystem signatures (ext, NTFS, exFAT, ISO, UDF, HFS+, APFS, btrfs, XFS, LUKS, BitLocker) | detect | no | `fstyp`-style readers plus own | storage worker, read-only handle |
| FAT32, UFS2 | yes | format | base `newfs_msdos`, `newfs` | broker |
| exFAT | mount if kernel support exists | Proposed | no in-tree writer found; own formatter, about 1 200 lines | storage worker |
| Raw images, ISO 9660 | yes | yes | own code | storage worker |
| `.xz`, `.zst`, `.gz` image compression | yes | `.zst` | zstd (BSD, in base), liblzma (0BSD/public domain, in base), zlib (in base) | image helper |
| SHA-256, SHA-512, BLAKE3 | yes | yes | FORGE crypto (FORGE-WP-22) | image helper |
| Detached signatures (Ed25519, OpenPGP) | verify | no | SE04 signing service | SE04 |
| Storage report | no | Markdown, JSON, PDF | own; PDF via G-16 | app host |
| Topology export | no | SVG, PNG | FUSE draw list export | app host |

### 6.4 Integration

- Intents handled: `open` for a device, pool or dataset reference (`pbsd-storage:` URL scheme), and for image MIME types (`application/x-iso9660-image`, `application/x-raw-disk-image`) which opens Write image. Emitted: `open` to SY02 (browse), AD03 (schedules, restore), UT04 (usage), SY05 (boot environments), SE04 (keys), UT05 (help), SY03 (terminal).
- Services: `svc.storage` (Proposed), `svc.notify`, `svc.settings`, `svc.portal.file`, `svc.keyring`, `svc.audit`, `svc.power` (inhibit sleep during jobs), `svc.portal.usb`, `svc.model` (optional).
- Shared components: partition editor, vdev editor and dataset table are a ForgeScript module used by AD01. AD03 uses the broker's snapshot and send/receive operations. SE01 reads health and encryption state from the broker. AD05 answers "which apps hold handles into this dataset" for export and unmount.
- Verified from the repo: the tools and libraries cited by path in section 4, the ZFS commands under `hbsd/src/sys/contrib/openzfs/man/man8/`, GEOM classes under `pbsd/geom/` and ZFS modules under `pbsd/zfs/` (ported file sets; their build and test state was not checked). Proposed: the broker, its interface, every view.

## 7. Security and privacy

Capabilities requested:

| Capability | Reason |
|---|---|
| `fs.app` | View state, partition-table backups, benchmark results |
| `fs.portal` | Choose image files, key files, report destinations |
| `net.none` | The app never uses the network |
| `svc.storage` (Proposed) | All storage reads and changes |
| `svc.notify` | Job progress, health alerts |
| `svc.settings` | Preferences |
| `svc.keyring` | Fetch or save a volume passphrase item, with the user's consent each time |
| `svc.audit` | Record changing actions |
| `svc.power` | Inhibit sleep during image writes and erases |
| `svc.portal.usb`, `dev.usb:storage` | See removable storage arrive; safe removal |
| `proc.helper:image-helper` | Decompress and hash untrusted image files outside the app host |
| `ai.local` (optional, off) | "Explain this status" |

Threat model:

| Asset | Adversary | Attack surface | Mitigation |
|---|---|---|---|
| All data on all disks | A compromised or buggy app using the broker | Broker interface | Broker checks the caller's bundle identity and capability; every changing call needs a fresh SH03 confirmation that names the action and device; typed, closed set of operations, no command strings |
| The running system | User error | Destructive dialogs | System devices protected; typed confirmations; dry runs; pending list; automatic table backup |
| Encryption keys and passphrases | Other apps, logs, crash dumps | Passphrase entry, key files | Secret entry goes straight from the SH03 or app secret field to the broker in a zeroised buffer; never logged; never in command preview; key files passed as handles |
| The kernel | Malicious removable media | Partition tables, filesystem metadata, ZFS labels | First-seen devices are not mounted without consent; signature probing in a worker with a read-only handle; kernel mounts with `noexec,nosuid`; pool import of foreign media asks first and defaults to read-only |
| The image helper | Malicious image file | Decompressors | Capability mode, no device or network handle, output only through a pipe to the worker; fuzzed |
| Device identity (serial numbers) | Anyone given a report or screenshot | Reports, inspector | Identifiers hidden by default and stripped from exports unless ticked |
| Health history | Local tampering to hide a failing disk | Health store | Store owned by the broker; app cannot write it |
| Erased disks | A later finder of the disk | Incomplete erase | Method table states guarantees; verification pass; report |

Stored: see 6.2. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `disks.units` | enum{binary,decimal} | `binary` | Size units |
| `disks.show_identifiers` | bool | `false` | Show serial numbers and WWNs |
| `disks.confirm.typed_name` | bool | `true` | Require typed names for destructive actions (cannot be turned off in Strict profile) |
| `disks.authority.cache_seconds` | u32 | `0` | How long an admin approval is reused; 0 = ask every time; max 300 |
| `disks.image.verify_after_write` | bool | `true` | Read back after writing |
| `disks.image.show_internal_targets` | bool | `false` | List internal disks as image targets |
| `disks.removable.automount` | enum{never,ask,known} | `ask` | Mount policy for removable media |
| `disks.removable.options` | string | `noexec,nosuid` | Mount options for removable media |
| `disks.scrub.interval_days` | u16 | `30` | Default scrub interval per pool |
| `disks.scrub.on_battery` | bool | `false` | Allow scheduled scrubs on battery |
| `disks.selftest.short_days` | u16 | `7` | Short self-test interval; 0 = off |
| `disks.selftest.extended_days` | u16 | `30` | Extended self-test interval; 0 = off |
| `disks.health.sample_hours` | u16 | `24` | Health sampling interval |
| `disks.health.temp_warn_c` | i16 | `55` (HDD), `70` (NVMe) | Temperature warning |
| `disks.capacity.warn_pct` | u8 | `80` | Pool capacity warning |
| `disks.capacity.critical_pct` | u8 | `90` | Pool capacity critical |
| `disks.safety.snapshot_before_change` | bool | `true` | Offer a snapshot before rollback and similar |
| `disks.commands.preview` | bool | `false` | Show equivalent commands in every dialog |
| `disks.topology.layout` | enum{logical,physical} | `logical` | Default diagram layout |

## 9. Accessibility and languages

- The partition bar and the topology graph each have an equivalent table; the graph is also a tree in the a11y tree (controller ▸ disk ▸ … ▸ dataset) so a screen reader never needs the picture.
- Health and pool states are spoken as words ("degraded, one device faulted"), never as colours or glyphs.
- Job progress is a live region announced at 10 % steps; completion and failure are always announced.
- Sizes are read with units ("nine hundred and thirty-one gibibytes"); exact bytes on request.
- ZFS property names stay in English as identifiers; their descriptions are translated.
- Typed confirmations accept the name regardless of input method composition and are not case-folded.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| First frame | Under 150 ms from cached identities; live data within 500 ms |
| Tree refresh on hot-plug | Under 1 s |
| Dataset tree | 50 000 datasets and 1 000 000 snapshots listed lazily; scrolling at 60 fps; first page of snapshots in under 300 ms |
| Memory | Under 120 MiB with 10 000 datasets shown |
| Idle | Zero wake-ups when no job runs and the window is not visible; statistics sampling stops when its tab is hidden |
| Image write | Within 5 % of the device's sequential rate |
| Hashing | At least 1 GiB/s BLAKE3 on one core |
| Broker call latency (read) | Under 20 ms p95 |

## 11. Testing and proof

Test plan (nothing has run):

- Unit: pending-change list (ordering, undo, conflict detection), size and alignment arithmetic, usable-capacity calculator for each topology, SMART and NVMe decoders against recorded log pages, property-source resolution.
- Golden renders: every view in 5.2–5.3 in three themes and two scales; partition bar with 1, 4 and 128 partitions.
- Scripted UI through the a11y tree: T1–T14 against file-backed and virtual disks in a VM; fault injection with `zinject` (Verified in tree: `hbsd/src/cddl/usr.bin/zinject`) for degraded, faulted and checksum-error states.
- Fuzz: partition-table readers, filesystem signature readers, SMART and NVMe decoders, image decompression, the broker's request decoder.
- Differential: app's view against `zpool status`, `zfs list`, `gpart show`, `geli list` output on the same system; usable-size estimates against real pools.
- Power-cut tests: partition move and image write interrupted at random points must leave a recoverable state.
- Negative: wrong passphrase, device removed mid-job, pool with unsupported features, counterfeit-capacity stick, frozen ATA security state.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Size and alignment arithmetic | No overflow; partitions never overlap; all starts aligned | T1 | Lean proof of the allocator; PRISM refinement of the C++ | — |
| Pending-change planner | Applying then reverting yields the original table | T1 | Model of GPT edits | ForgeScript code not in PRISM scope; planner would move to the broker |
| Broker request decoder | No runtime error on any input | T2 | PRISM + fuzz | Broker not written |
| Broker authorisation gate | No changing operation runs without a matching, unexpired confirmation token | T2 | PRISM contract; model check of the token state machine | SH03 prompt path is outside the proof |
| System-device guard | No destructive operation targets a device flagged `system` | T2 | PRISM invariant on the dispatch table | Flag correctness depends on kernel mount state |
| SMART and NVMe decoders | Memory safe, total | T2 | PRISM + recorded-log corpus | — |
| Image helper decompressors | Memory safe | T3 | Safe subset where own code; sanitizers and fuzz for zstd, lzma, zlib | Third-party C code |
| Storage worker (write, verify, erase) | Writes only inside the device range handed to it | T2 | Capability mode + PRISM on offset arithmetic | — |
| OpenZFS, GEOM, GELI | — | T4 | Existing upstream tests (`tests/sys/cddl` exists and is unrun per `docs/ROADMAP.md` section 20) | Large foreign C code |
| UI | Destructive button is never default; typed name required | T3 | Scripted UI tests | UI not in proof scope |

## 12. Build plan

Phase 1 — usable core:

| ID | Work item | Depends on |
|---|---|---|
| AD02-W01 | `svc.storage` interface schema: read model, typed operations, jobs, signals | G-08, FORGE-WP-26 |
| AD02-W02 | Broker read side: devices, partitions, pools, vdevs, datasets, snapshots, events | AD02-W01, G-09 |
| AD02-W03 | Authorisation path with SH03 prompt and confirmation tokens | SH03, AD01-GAP-6 |
| AD02-W04 | App shell: tree, Overview, Disk ▸ Partitions, Pool ▸ Status, Datasets, Snapshots, inspector | G-02, G-07, FUSE-WP-12, FUSE-WP-28, FUSE-WP-29 |
| AD02-W05 | Partition editing with pending list, table backup, format (FAT32, UFS, ZFS) | AD02-W02, AD02-GAP-1 |
| AD02-W06 | Pool operations: create, add, attach, detach, replace, online, offline, clear, scrub, trim, import, export, destroy | AD02-W03 |
| AD02-W07 | Dataset operations: create, rename, destroy, properties, quotas, mount, snapshots, rollback, clone | AD02-W03 |
| AD02-W08 | Encryption: GELI init, attach, detach; ZFS encrypted dataset, load and unload key; SE04 hand-off | SE04, AD01-GAP-2 |
| AD02-W09 | Health summary, SMART and NVMe read | AD02-W02 |
| AD02-W10 | Image write with verify and checksum; removable media mount and safe removal | image helper, FORGE-WP-27, FORGE-WP-28 |
| AD02-W11 | Job engine, status bar, notifications, audit records | `svc.notify`, `svc.audit` |
| AD02-W12 | Boot environments read-only view | `svc.update` / SY05 |

Phase 2 — full feature:

| ID | Work item | Depends on |
|---|---|---|
| AD02-W13 | Resize and move with journal; MBR convert; GPT recover | AD02-W05 |
| AD02-W14 | Self-tests, health history, alerts, scheduled tests | AD02-W09, SE01 |
| AD02-W15 | Secure erase methods with verification and report | storage worker |
| AD02-W16 | Topology diagram | AD02-GAP-2, FUSE-WP-50 |
| AD02-W17 | Benchmarks | FUSE-WP-46 |
| AD02-W18 | GELI key slots, metadata backup; ZFS change key; auto-lock with handle revocation | AD05 (revocation API), SH03 |
| AD02-W19 | Create and restore images, attach images, read-only attach | AD02-W10 |
| AD02-W20 | Pool statistics, events, history, feature flags, checkpoint, vdev removal, holds, bookmarks, diff | AD02-W06 |
| AD02-W21 | Filesystem check, command preview, storage report | G-16 for PDF |

Phase 3 — advanced:

| ID | Work item | Depends on |
|---|---|---|
| AD02-W22 | Encrypt existing data, pool recovery import, split, initialise | AD03 (send and receive engine) |
| AD02-W23 | exFAT formatter, optical media, multi-target write, counterfeit test | — |
| AD02-W24 | Delegation, templates, ARC view, erase certificate, assistant explain | SE04, G-15 |

## 13. Open questions and risks

1. **`svc.storage` is a large privileged surface.** It must be a small typed interface, not a command runner. Its authorisation rule is a platform decision (`AD01-GAP-6`).
2. **GEOM may be replaced.** `docs/plan/PBSD_PLAN.md` P2-02 plans a block multiqueue layer with an adapter. The broker interface uses device identities, not GEOM names, so the back end can change.
3. **ZFS and GELI in this tree are untested** (`docs/ROADMAP.md` section 20: "ZFS is on and untested"). The app's tests depend on that work.
4. **SMART through USB bridges** is unreliable; the app must say "unknown", not "OK".
5. **Secure erase on flash** cannot be verified from the host for remapped blocks. The method table must not overstate.
6. **Moving partitions** is the riskiest operation; it may stay out of phase 2 if the journal design is not proven.
7. **exFAT and NTFS write support** was not found in the tree; removable-media interchange may be limited to FAT32 at first.
8. **Feature-flag upgrades can make a pool unreadable by the loader.** The app needs a table of loader-supported features, which does not exist yet.
9. **Authority caching** (`disks.authority.cache_seconds`) trades safety for convenience; default stays 0.
