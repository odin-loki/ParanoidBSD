<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Disk Usage

By Odin Loch

| | |
|---|---|
| ID | `UT04` (from CATALOGUE.md) |
| Bundle | `org.pbsd.disk-usage` |
| Category | utils |
| Priority | P2 |
| Backing repos | FUSE (UI; treemap and sunburst planned as FUSE-WP-47); OpenZFS in the base tree at `hbsd/src/sys/contrib/openzfs` (Verified present; accounting semantics taken from ZFS properties, not from reading that source) |
| Status | Planned. Nothing built. |

## 1. Purpose

Disk Usage answers "where did the space go, and what would I get back if I deleted this?". On ZFS the naive answer is often wrong: snapshots, clones, compression, deduplication and reservations change what a delete frees. This app shows both the plain file sizes and the real effect on free space. It replaces Filelight from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- Scanning folders, datasets, whole pools and remote locations the user grants.
- Treemap, sunburst and list views of one scan.
- ZFS-aware accounting: datasets, snapshots, clones, compression, dedup, reservations, quotas.
- Finders: duplicates, large files, old files, by type, by owner.
- Per-app data and cache sizes; clearing caches through app-declared cache directories.
- Trash size; clean-up suggestions; every clean-up undoable where the platform allows.
- Scan history and growth over time; export.

Out of scope:

| Item | Owner |
|---|---|
| General file management (copy, move, rename) | SY02 Files |
| Creating, destroying, resizing pools and datasets; changing quotas and reservations | AD02 Disks |
| Snapshot schedules, retention policy, off-site copies | AD03 Backup (this app links to it for snapshot deletion) |
| Removing packages and apps, pruning boot environments | SY05 Software (this app shows their size and links) |
| Live I/O and per-process disk activity | SY04 System Monitor |
| Secure erase | SE05 Vault |
| Photo-specific near-duplicate detection | ME01 Photos |
| Log rotation and crash-report clean-up policy | AD04 Logs |

## 3. Users and top tasks

Personas:

- **Everyday user**: the disk is nearly full and something must go.
- **Admin**: needs to know which dataset or snapshot holds the space, and what a destroy would reclaim.
- **Tidy user**: finds duplicates and forgotten downloads.
- **Developer**: build caches and VM images grow; wants a regular look at growth.

| # | Task | Target steps |
|---|---|---|
| T1 | See what fills the home folder | 1 (open; home is scanned or loaded from the last scan) |
| T2 | Drill into the biggest folder | 1 click per level |
| T3 | Find the ten largest files | 2 |
| T4 | Learn how much space deleting a folder would really free | 1 (select it; inspector says) |
| T5 | See how much space snapshots hold and which ones | 2 |
| T6 | Clear all app caches | 3 |
| T7 | Empty the trash | 2 |
| T8 | Find duplicates and keep one of each | 4 |
| T9 | Move a large file to trash and undo it | 1 + 1 |
| T10 | Compare with last month's scan | 2 |
| T11 | Scan a network share | 3 |
| T12 | Export a report | 2 |

## 4. Features

### 4.1 Scanning

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT04-F001 | Scan targets | Home, any folder granted through the folder portal, a dataset, a pool, a mounted volume, a remote location mounted by SY02. | M | 1 |
| UT04-F002 | Scan helper | A native helper walks the tree with a read-and-list-only directory handle and streams records to the app. It cannot open files for writing, cannot read file contents during a size scan, and has no network. | M | 1 |
| UT04-F003 | Progressive results | The views fill while the scan runs; totals are marked "so far". | M | 1 |
| UT04-F004 | Sizes collected | Apparent size (length), allocated size (blocks), link count, type, owner, mode, mtime, atime (if kept), birth time, inode and device ids. | M | 1 |
| UT04-F005 | Hard links counted once | Files with several links are charged once inside a scan root; the row shows "n links, m outside this folder". | M | 1 |
| UT04-F006 | Do not cross boundaries | By default a scan stays on one dataset or file system; mount points and child datasets appear as separate nodes with their own totals. Option to descend. | M | 1 |
| UT04-F007 | Symlinks not followed | Links are listed with their own size only. | M | 1 |
| UT04-F008 | Unreadable items | Folders the handle cannot read are shown as "no access (n)" with the space that cannot be accounted for, derived from the parent's totals where the file system gives them. | M | 1 |
| UT04-F009 | Elevated scan | "Scan with administrator rights" asks SH04 for a one-time read-and-list handle to the root of a dataset. The grant is logged in `svc.audit` and ends with the scan. | S | 2 |
| UT04-F010 | Exclusions | Glob and path excludes per target and global. | S | 2 |
| UT04-F011 | Pause, resume, cancel | A cancelled scan keeps partial results marked incomplete. | M | 1 |
| UT04-F012 | Rescan a subtree | `F5` on a node rescans only that folder. | M | 1 |
| UT04-F013 | Incremental rescan | A rescan reuses the stored tree and revisits only directories whose mtime changed; on ZFS it can use the dataset's "written since snapshot" figure to skip unchanged datasets. | S | 2 |
| UT04-F014 | Live updates | After a clean-up in this app the tree is adjusted at once without a rescan. | M | 1 |
| UT04-F015 | Scan speed controls | I/O priority low by default; "fast" option. Stops on battery below 20% unless told otherwise. | S | 2 |
| UT04-F016 | Remote scans | Network locations are scanned through the mounted handle; a note says sizes come from the server and block sizes may be unknown. A request-rate cap protects slow links. | S | 3 |
| UT04-F017 | Sparse and compressed files | Shown with both sizes; a badge when allocated is under half of apparent (sparse or well compressed) or above it (small files, padding). | M | 1 |
| UT04-F018 | Scheduled scans | Optional weekly scan of chosen targets in the background for history (`bg.run`). Off by default. | C | 3 |
| UT04-F019 | Sandboxed app data | Each app's data, config and cache directories are measured through a platform call that returns sizes only, not through a handle to the directories. | M | 2 |
| UT04-F020 | Device overview | Start view lists pools, datasets and volumes with used, free, and a "what is free really" figure (4.3). | M | 1 |

### 4.2 Views

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT04-F030 | Treemap | Squarified treemap of the current folder to a chosen depth (1–6). Cushion shading off by default (flat, calm). Labels where they fit. | M | 1 |
| UT04-F031 | Sunburst | Rings for depth levels around the current folder; centre goes up. | S | 2 |
| UT04-F032 | List | Tree table: name, size bar, size, allocated, share of parent, items, newest change, owner. Sorted by size by default. | M | 1 |
| UT04-F033 | Linked selection | Selecting in the map selects in the list and the reverse; both are always visible in the default layout. | M | 1 |
| UT04-F034 | Navigate | Double click or `Enter` zooms into a folder; `Backspace` up; breadcrumb; back and forward. | M | 1 |
| UT04-F035 | Size metric switch | Apparent, allocated (default), or "would free" (4.3) drives all views. | M | 1 |
| UT04-F036 | Colour by | Type category (default), age, owner, depth, compression ratio, "in snapshots only". A legend names every colour; patterns are added in high contrast. | M | 1 |
| UT04-F037 | Small-item grouping | Items under a threshold are merged into "n smaller items" so the map stays readable. | M | 1 |
| UT04-F038 | Free space tile | Option to show free space (and reserved space) as tiles so the map is the whole dataset. | S | 2 |
| UT04-F039 | Tooltip | Path, sizes, share, items, modified, type. | M | 1 |
| UT04-F040 | Type summary | Table and bar by category (images, video, audio, documents, archives, code, disk images, app data, other) and by extension. Click filters the views. | S | 2 |
| UT04-F041 | Age summary | Histogram of bytes by last change (week, month, year, older). | S | 2 |
| UT04-F042 | Owner summary | Bytes by owner on shared machines (needs elevated scan for other users' files). | C | 3 |
| UT04-F043 | Filter | By name, type, size at least, age at least, owner; the map dims what does not match. | S | 2 |
| UT04-F044 | Find in scan | `Ctrl+F` searches names in the scan store. | M | 1 |
| UT04-F045 | Compare view | Two scans of the same target: per folder, the change in bytes, with growth in `--warn` and shrink in `--ok` plus arrows. | S | 2 |
| UT04-F046 | Save view as image | PNG or SVG of the map with legend. | C | 3 |

### 4.3 ZFS-aware accounting

The numbers come from dataset properties supplied by a storage accounting interface (6.4). Names below are the ZFS property names.

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT04-F050 | Dataset breakdown | For each dataset: `used` split into `usedbydataset`, `usedbysnapshots`, `usedbychildren`, `usedbyrefreservation`, shown as one stacked bar with words. | M | 2 |
| UT04-F051 | Pool view | Pool size, allocated, free, fragmentation, capacity; the gap between pool free and dataset `available` explained (parity, slop space, reservations). | M | 2 |
| UT04-F052 | Snapshot space | Per snapshot: `used` (unique to it), `referenced`, `written`, creation time, holds, clones depending on it. A note explains that space shared between several snapshots is in none of their `used` figures. | M | 2 |
| UT04-F053 | Snapshot range reclaim | Select a contiguous range of snapshots and get the space a destroy would free, from the service's dry-run destroy estimate. This is the only honest figure for more than one snapshot. | M | 2 |
| UT04-F054 | "Would free" for files | For a selected file or folder: bytes freed now if deleted. If the blocks are held by a snapshot, a clone or dedup, the figure is lower than the size, and the inspector says why and what holds them ("still held by 3 snapshots, newest 2026-11-10"). | M | 2 |
| UT04-F055 | Held-by-snapshot estimate | Method: a file older than the newest snapshot of its dataset, and unchanged since, is treated as held; the result is labelled "estimate" unless the service can give an exact per-file answer. | M | 2 |
| UT04-F056 | Compression | Dataset `compressratio`, `logicalused` against `used`; per file, apparent against allocated. "Saved by compression: n GiB". | S | 2 |
| UT04-F057 | Deduplication | Pool dedup ratio and table size in memory when dedup is on; a warning that deleting a deduplicated file may free little. | S | 3 |
| UT04-F058 | Block cloning | Files that share blocks through clone-on-copy are flagged where the service can tell; "would free" accounts for it or is marked unknown. | C | 3 |
| UT04-F059 | Reservations and quotas | `reservation`, `refreservation`, `quota`, `refquota`, user and group quotas shown per dataset with how full each is. Changing them hands off to AD02. | S | 2 |
| UT04-F060 | Clones and origins | Clone datasets show their origin snapshot; an origin snapshot shows that it cannot be destroyed while clones exist. | S | 2 |
| UT04-F061 | Boot environments | Listed with unique size; "Manage" hands off to SY05. | S | 2 |
| UT04-F062 | Zvols | Volumes (VM disks) show `volsize`, `used`, `refreservation`; owner app (VM01) named where known. | S | 3 |
| UT04-F063 | Explain panel | Plain-language panel for the selected dataset: "You have 40 GiB of files here. 65 GiB is used: 22 GiB of it is old versions kept by 14 snapshots." | M | 2 |
| UT04-F064 | Non-ZFS volumes | FAT, exFAT, UFS and others get the plain view; ZFS panels are hidden. | M | 1 |
| UT04-F065 | Encrypted, locked datasets | Shown with their `used` total and a lock; contents cannot be scanned until unlocked (hand-off to AD02 or SE05). | S | 2 |

### 4.4 Finders

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT04-F070 | Largest files | Top n (default 100) across the scan, with path, size, age. | M | 1 |
| UT04-F071 | Old and unused | Files not changed (and, where atime exists, not read) for more than n months, ranked by size. | S | 2 |
| UT04-F072 | Duplicates | Candidates by equal size, then a hash of the first and last 64 KiB, then full BLAKE3. Hashing is a separate job in the hash helper with read-only file handles. | S | 2 |
| UT04-F073 | Duplicate groups | Groups ranked by reclaimable bytes. Hard links and block clones are recognised and not reported as duplicates. | S | 2 |
| UT04-F074 | Keep rules | Keep newest, oldest, shortest path, or the copy in a preferred folder; preview before acting. Nothing is selected for removal by default. | S | 2 |
| UT04-F075 | Replace duplicates by links | Option to replace copies with hard links, or with block clones where the file system supports them, instead of deleting. | C | 3 |
| UT04-F076 | Empty folders and files | Listed; removable. | C | 2 |
| UT04-F077 | Big folders of small files | Folders with very high item counts (build trees, caches) ranked by count. | S | 2 |
| UT04-F078 | Known space hogs | Rule list for well-known patterns: build output folders, package caches, VM images, disk images, old installers, core dumps, thumbnails. Each rule states what it is and whether it is safe to remove. | S | 2 |
| UT04-F079 | Temporary and partial files | `.partial`, `.tmp`, editor swap files older than a week. | C | 2 |
| UT04-F080 | Broken links | Symlinks whose target is missing. | C | 3 |

### 4.5 Apps, caches, trash

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT04-F090 | Per-app sizes | Table: app, bundle size, data, config, cache, total, last used. Data from PLATFORM.md section 10 layout through the platform size call (F019). | M | 2 |
| UT04-F091 | Clear an app's cache | Uses the cache directory the app declares (`~/.cache/<id>/`, which "may be deleted at any time" per PLATFORM.md section 10). The request goes to the platform, which clears it while the app is told; this app never gets a handle to another app's files. | M | 2 |
| UT04-F092 | Clear all caches | One action with a per-app list and total; apps can be unticked. | M | 2 |
| UT04-F093 | App data is not cache | Data and config are shown but not offered for clean-up; "Remove app and data" hands off to SY05. | M | 2 |
| UT04-F094 | Trash size | Per volume, with item count and oldest item; "Empty trash" and "Empty items older than 30 days" through `svc.undo-trash`. | M | 1 |
| UT04-F095 | Thumbnails and system caches | Sizes of thumbnail, font and search-index caches with clear actions offered by their owning services. | S | 2 |
| UT04-F096 | Logs and crash reports | Size shown; "Manage" hands off to AD04. | S | 2 |
| UT04-F097 | Packages and updates | Size of package cache and old boot environments; hands off to SY05. | S | 2 |
| UT04-F098 | Model files | Size of installed `.fpk` models; hands off to AI02. | C | 3 |
| UT04-F099 | VM and container images | Sizes; hand off to VM01 and VM02. | C | 3 |

### 4.6 Clean-up

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT04-F110 | Move to trash | `Delete` on any item in any view; goes through `svc.undo-trash`; toast with Undo. Needs a write grant for that folder: the portal asks once per scan root. | M | 1 |
| UT04-F111 | Delete for ever | `Shift+Delete` with a confirmation naming the count and the bytes; not undoable; logged. | M | 1 |
| UT04-F112 | Basket | Mark items across the scan into a basket; review totals (size and "would free") and act once. | S | 2 |
| UT04-F113 | Suggestions | A ranked list built from finders and rules: caches, trash, duplicates, old downloads, big old files, snapshot ranges. Each has what, why safe or not, how much it frees, and what undo exists. | S | 2 |
| UT04-F114 | Undo coverage shown | Each action states its undo: trash (undo), cache clear (no undo, data is re-creatable), snapshot destroy (no undo), delete for ever (no undo). | M | 1 |
| UT04-F115 | Trash does not free space yet | After moving to trash the app says so and shows the pending total with "Empty trash". | M | 1 |
| UT04-F116 | Snapshots hold deleted data | After a delete on a dataset with snapshots, the app says how much stays held and until when (next retention expiry, from AD03 if available). | M | 2 |
| UT04-F117 | Snapshot destroy hand-off | Destroying snapshots is done by AD03; this app passes the selected names and shows the result. | S | 2 |
| UT04-F118 | Safety snapshot | Before a batch clean-up over a threshold (default 1 GiB or 1 000 items) the app offers a snapshot through `svc.backup`, and explains that the snapshot itself keeps the space until removed. | C | 3 |
| UT04-F119 | Protected paths | System datasets, app bundles and other apps' data cannot be removed here; the row says who can. | M | 1 |
| UT04-F120 | Open and reveal | Open the item with its default app; "Show in Files" (SY02); "Open Terminal here" (SY03). | M | 1 |
| UT04-F121 | Compress instead | "Compress to archive" hands the selection to UT01. | C | 3 |
| UT04-F122 | Low-space assistant | When `svc.notify` raises a low-space notice (from the platform), its action opens this app on Suggestions for that volume. | S | 2 |

### 4.7 History and export

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT04-F130 | Scan history | Each completed scan is stored (compressed) with date, target, totals. Retention by count and age. | S | 2 |
| UT04-F131 | Growth chart | Used space over time per target, from stored scans and from dataset figures sampled daily when the background option is on. | S | 2 |
| UT04-F132 | Top growers | Folders that grew most between two scans. | S | 2 |
| UT04-F133 | Forecast | Straight-line estimate of days until full, with the fit shown; no model needed. Labelled an estimate. | C | 3 |
| UT04-F134 | Export report | CSV or JSON of the tree to a depth; a Markdown or PDF summary (totals, top folders, top files, type split, map image). | S | 2 |
| UT04-F135 | Export finder results | Duplicates, large files and suggestions as CSV. | S | 2 |
| UT04-F136 | Print | Summary report through `svc.portal.print`. | C | 3 |
| UT04-F137 | Open a saved scan | Load a stored scan read-only; clean-up actions are disabled until a rescan confirms the items. | S | 2 |
| UT04-F138 | Privacy of history | History holds file names. A setting keeps names only to a depth, or keeps no history. Stores are in `fs.app` and can be cleared. | M | 2 |

### 4.8 Settings, accessibility, recovery

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT04-F140 | Preferences | Section 8. | M | 1 |
| UT04-F141 | Units | Binary (GiB) by default, decimal optional; always labelled. | M | 1 |
| UT04-F142 | Accessible map | The list is the full equivalent of the map; the map itself is navigable by arrow keys with each tile named and sized. | M | 1 |
| UT04-F143 | Scan resume after crash | A scan in progress is checkpointed every 10 s and can resume. | S | 2 |
| UT04-F144 | Stale warning | A loaded scan older than a day shows its age; items changed since are re-checked before any action. | M | 1 |
| UT04-F145 | Action journal | Every clean-up is journaled (what, size, undo token) and viewable. | S | 2 |

## 5. GUI

### 5.1 Window map

```
Disk Usage
├─ Main window
│  ├─ Sidebar: Overview, targets (Home, pools, datasets, volumes, remote), Apps and caches,
│  │           Trash, Finders (Largest, Old, Duplicates, Hogs), Suggestions, History
│  ├─ Scan view (map + list, linked)
│  ├─ Inspector: selection details, "would free", ZFS explain panel
│  └─ Status bar: scan progress, metric, basket
├─ Overview (start)
├─ Dataset and snapshot view
├─ Apps and caches view
├─ Duplicates view
├─ Suggestions view
├─ History and compare view
├─ Basket sheet
├─ Scan options popover
├─ Export dialog
├─ Delete-for-ever confirmation
└─ Preferences
```

### 5.2 Main window

Shown at 1280×760, scan of Home finished, a folder selected, treemap + list.

```
+------------------------------------------------------------------------------------------+
| [=] Disk Usage — Home                    [Find in scan Ctrl+F]                [_][o][x] |
+-----------+----------------------------------------------------------+-------------------+
| Overview  | [Rescan] [Map v] Size [Allocated v] Colour [Type v] Depth[3] (1)| SELECTION (6)|
| TARGETS   | Home > Projects >                                    (2) | build/            |
| > Home    +----------------------------------------------------------+ ~/Projects/site    |
|   zroot   | +---------------------------+--------------+-----------+ | Size      18.4 GiB|
|   data    | | Videos            41 GiB  | Projects     | Downloads | | Allocated 11.2 GiB|
|   USB 32G | |                           | +----------+ | 9.1 GiB   | | Items     212 480 |
| ─────     | |                           | |[build]   | |           | | Changed   today   |
| Apps      | |                           | | 11.2 GiB | +-----------+ |                   |
| Trash 3.2G| +-------------+-------------+ +----------+ | Pictures  | | WOULD FREE    (7) |
| FINDERS   | | VMs 22 GiB  | .cache 6 GiB| | src      | | 7.4 GiB   | | 2.1 GiB now       |
|  Largest  | +-------------+-------------+-+----------+-+-----------+ | 9.1 GiB is still  |
|  Old      |                                                      (3) | held by 3 snapshots|
|  Duplicates+---------------------------------------------------------+ (newest 10 Nov).  |
|  Hogs     | Name          Size bar        Alloc     %    Items  Changed| [Which snapshots?]|
| Suggestions| > Videos     ▓▓▓▓▓▓▓▓▓▓▓░░   41.0 GiB  38%  1 204  Oct   |                   |
| History   | v Projects    ▓▓▓▓▓░░░░░░░░   19.6 GiB  18%  240 k  today | [Move to Trash]   |
|           |   > build     ▓▓▓▓▓▓▓░░░░░░   11.2 GiB  57%  212 k  today | [Add to basket]   |
|           |   > src       ▓▓░░░░░░░░░░░    3.1 GiB  16%  18 k   today | [Show in Files]   |
|           | > VMs         ▓▓▓▓▓░░░░░░░░   22.0 GiB  20%  6      Sep (4)|                  |
+-----------+----------------------------------------------------------+-------------------+
| Scan complete 14:02 · 1.9 M items · 108 GiB allocated (131 GiB apparent)  (5) Basket 0   |
+------------------------------------------------------------------------------------------+
```

1. Toolbar: Rescan `toolButton`; view `comboBox` (Map, Sunburst, List only, Map only); metric `comboBox` (Apparent, Allocated, Would free); colour `comboBox`; depth `spinBox`.
2. Breadcrumb.
3. Map: treemap widget (FUSE-WP-47; until then `UT04-GAP-1`). Selected tile has the focus ring.
4. List: `treeView` with a size-bar column painted by a cell delegate. Shares one selection model with the map.
5. Status: scan state and totals, both metrics; basket counter (click opens the basket sheet).
6. Inspector top: plain facts (`propertyGrid`, read-only).
7. Inspector "would free" block: the figure, the reason in words, and a link to the snapshot view. On non-ZFS volumes it shows only "Frees 11.2 GiB (after the trash is emptied)".

States: empty (Overview), loading (map fills in; totals say "so far"; a `progressBar` with items per second), error (target vanished or grant revoked: banner with "Choose again"), stale (banner "Scanned 9 days ago" with Rescan), read-only (saved scan or protected path: actions disabled with reasons), narrow (sidebar overlay; map above list becomes a tab switch; inspector sheet).

### 5.3 Other views and dialogs

**Overview** (1280×760, start)

```
+------------------------------------------------------------------------------------------+
| Storage                                                                                  |
|  zroot (pool, 476 GiB)   ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░  352 GiB used · 124 GiB free     [Details] |
|    Home                  ▓▓▓▓▓▓░░░░░░░░░░░░░░  131 GiB   files 108 · snapshots 23 [Scan] |
|    System                ▓▓░░░░░░░░░░░░░░░░░░   38 GiB   2 boot environments   [Details] |
|    Apps and their data   ▓▓▓░░░░░░░░░░░░░░░░░   54 GiB   caches 9.3 GiB        [Review]  |
|    Virtual machines      ▓▓▓▓░░░░░░░░░░░░░░░░   88 GiB   reserved 20 GiB       [Details] |
|    Other datasets        ▓▓░░░░░░░░░░░░░░░░░░   41 GiB                         [Details] |
|  USB 32G (exFAT)         ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░   27 GiB used · 2.4 GiB free     [Scan]    |
|                                                                                          |
|  Quick wins                                                              (1)             |
|   Empty trash                       3.2 GiB   undo: none once emptied      [Review]      |
|   Clear app caches                  9.3 GiB   re-created as needed         [Review]      |
|   12 snapshots older than 90 days   up to 14 GiB (estimate)                [Review]      |
|                                                         [Scan a folder…] [All suggestions]|
+------------------------------------------------------------------------------------------+
```

Rows are a `listView` of `panel`s with `progressBar`s. 1. Top three suggestions.

**Dataset and snapshot view** (for a ZFS dataset)

```
+------------------------------------------------------------------------------------------+
| zroot/home/odin                                             [Manage in Disks] [Backup…] |
| Used 131 GiB = files 108 ▓▓▓▓▓▓▓▓▓▓▓ + snapshots 23 ▒▒▒ + children 0 + reservation 0 (1)|
| Logical 164 GiB · compression 1.25× (saved 33 GiB) · quota none · available 124 GiB      |
+------------------------------------------------------------------------------------------+
| "You have 108 GiB of current files. Another 23 GiB is old versions and deleted files     |
|  kept by 31 snapshots. Deleting a file frees its space only when no snapshot holds it."  |
+------------------------------------------------------------------------------------------+
| [ ] Snapshot                  Created       Unique   Written  Refers   Holds  Clones     |
| [x] auto-2026-08-01           1 Aug 2026    1.2 GiB  3.4 GiB  96 GiB   -      -          |
| [x] auto-2026-08-08           8 Aug 2026    0.3 GiB  2.1 GiB  97 GiB   -      -          |
| [x] auto-2026-08-15           15 Aug 2026   0.9 GiB  2.8 GiB  99 GiB   -      -          |
| [ ] before-upgrade            2 Sep 2026    4.0 GiB  6.2 GiB  101 GiB  keep   be-2026-09 |
| …                                                                                        |
+------------------------------------------------------------------------------------------+
| 3 selected (contiguous) · sum of unique 2.4 GiB · destroying all three frees 7.9 GiB (2) |
|                                                     [Send to Backup to destroy…]         |
+------------------------------------------------------------------------------------------+
```

1. Stacked bar with words. 2. The range figure from the dry-run estimate; when the selection is not contiguous the line says "Select a contiguous range for an exact figure". `tableView` with `checkBox` column.

**Apps and caches view**

```
+------------------------------------------------------------------------------------------+
| [Clear selected caches (7.9 GiB)]                       Sort [Total v]   [x] Show system |
+------------------------------------------------------------------------------------------+
| [ ] App               Bundle    Data      Config   Cache     Total     Last used         |
| [x] Web Browser       210 MiB   1.1 GiB   2 MiB    4.2 GiB   5.5 GiB   today             |
| [x] Photos            90 MiB    38 GiB    1 MiB    2.9 GiB   41 GiB    yesterday         |
| [x] Maps              60 MiB    6.0 GiB   0        0.8 GiB   6.9 GiB   3 weeks ago       |
| [ ] Assistant         40 MiB    12 MiB    1 MiB    0         53 MiB    today             |
+------------------------------------------------------------------------------------------+
| Caches are safe to clear: apps re-create them. Data and settings are never touched here. |
+------------------------------------------------------------------------------------------+
```

**Duplicates view**

```
+------------------------------------------------------------------------------------------+
| Scope [Home v]  Min size [1 MiB v]  [Find]      Keep rule [None v]   Reclaim: 0 of 6.1 GiB|
+------------------------------------------------------------------------------------------+
| v 3 copies · 1.4 GiB each · reclaim up to 2.8 GiB          holiday.mp4                   |
|   [ ] ~/Videos/holiday.mp4                 2026-07-02                                    |
|   [ ] ~/Downloads/holiday.mp4              2026-07-02                                    |
|   [ ] ~/Desktop/holiday (copy).mp4         2026-07-03                                    |
| v 2 copies · 700 MiB each · reclaim up to 700 MiB          installer.iso                 |
|   [ ] ~/Downloads/installer.iso                                                          |
|   [ ] ~/Downloads/installer (1).iso                                                      |
+------------------------------------------------------------------------------------------+
| Hashing 62% (BLAKE3) · 14 groups so far      [Move ticked to Trash]  [Replace by links…] |
+------------------------------------------------------------------------------------------+
```

`treeView` with tick boxes. The app refuses to tick every copy in a group; the last one stays.

**Suggestions view**: `listView` of cards, each: title, bytes, "would free" bytes, risk word (Safe, Check first, Cannot undo), reason, `[Review]`. Review opens the matching view with the items preselected into the basket.

**History and compare view**

```
+------------------------------------------------------------------------------------------+
| Target [Home v]   From [12 Oct 2026 v]  To [12 Nov 2026 v]                               |
|  GiB                                                                                     |
|  140 |                                             ___....----                           |
|  120 |                 ___....-----''''''''''''''''                                      |
|  100 |____....----'''''                                                              (1) |
|      +----------------------------------------------------------------------            |
|       Jun      Jul      Aug      Sep      Oct      Nov                                   |
| Top growers since 12 Oct                                                                 |
|  ~/Projects/site/build     +8.9 GiB ↑      ~/VMs                 +4.0 GiB ↑              |
|  ~/.cache                  +2.2 GiB ↑      ~/Downloads           -1.1 GiB ↓              |
+------------------------------------------------------------------------------------------+
```

1. `chart` (line). The growers list is a `tableView`.

**Basket sheet**: `tableView` of marked items with size and "would free"; totals; per-row remove; `[Move all to Trash] [Delete for ever…] [Compress to archive…] [Export list…]`.

**Delete-for-ever confirmation** (`dialog`): title "Delete 212 480 items for ever?"; body: bytes, the "would free" figure, "This cannot be undone." and, on ZFS with snapshots, "Snapshots will still hold 9.1 GiB of it."; buttons `[Keep]` (default) `[Delete 212 480 items]` (outlined `--danger`).

### 5.4 Menus

```
File    Scan Folder… Ctrl+O · Scan Home Ctrl+Shift+H · Scan with Administrator Rights…
        Open Saved Scan… · Export Report… Ctrl+E · Save Map as Image… · Print… Ctrl+P
        Close Ctrl+W · Quit Ctrl+Q
Edit    Undo Ctrl+Z · Redo Ctrl+Shift+Z · Copy Path Ctrl+Shift+C · Copy Size
        Move to Trash Delete · Delete for Ever Shift+Delete · Add to Basket B · Find Ctrl+F
        Preferences Ctrl+,
View    Map and List Ctrl+1 · Map Only Ctrl+2 · List Only Ctrl+3 · Treemap / Sunburst Ctrl+M
        Size: Apparent / Allocated / Would Free (Alt+1…3) · Colour By ▸ · Depth + / -
        Show Free Space · Type Summary · Age Summary · Inspector F4 · Sidebar F9
Go      Up Backspace · Back Alt+Left · Forward Alt+Right · Scan Root Home · Overview Ctrl+Home
Scan    Rescan F5 · Rescan Selection Shift+F5 · Pause · Cancel Esc · Exclusions… · Scan Options…
Tools   Largest Files · Old Files · Duplicates… Ctrl+D · Space Hogs · Suggestions Ctrl+G
        Apps and Caches · Trash · Snapshots · Basket Ctrl+B · Action Journal
Help    Disk Usage Help F1 · How ZFS Counts Space · About
```

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| `Enter` / `Backspace` | Zoom into / out of a folder |
| Arrow keys (map) | Move to the neighbouring tile in that direction |
| `+` / `-` | Depth more / less |
| `B` | Add selection to the basket |
| `Space` | Quick look (system previewer) |
| `Alt+1…3` | Size metric |
| `Ctrl+M` | Treemap or sunburst |
| `Shift+F5` | Rescan the selected folder |

Context menu on an item: Open, Show in Files, Open Terminal Here, Zoom In, Rescan This Folder, Copy Path, Add to Basket, Move to Trash, Delete for Ever, Exclude from Scans, Compress to Archive…, Properties.

Drag and drop: a folder from SY02 onto the window starts a scan (the drop carries a read-and-list handle); items from the list onto the Trash sidebar row; items onto the basket counter. Wheel zooms depth with `Ctrl`; middle click goes up.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Overview with "Scan a folder…". A scan of an empty folder says "Nothing here". |
| Loading | Progressive map; "so far" totals; estimated completion when the dataset's `used` gives a total. |
| Error | Permission denied on the root: explain and offer the portal or the elevated scan. I/O errors: counted and listed, scan continues. |
| Offline | Local work unaffected; a remote target shows "not reachable" with its last scan. |
| Locked | Encrypted dataset not unlocked: total only, lock icon, hand-off. |
| Read-only | Read-only media, saved scans, snapshots browsed as folders: no clean-up actions. |
| Narrow (<640) | Map and list as tabs; inspector as a bottom sheet. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar | `treeView` |
| Toolbar | `toolButton`, `comboBox`, `spinBox` |
| Map | treemap and sunburst (FUSE-WP-47) |
| List | `treeView` with cell delegate (size bar) |
| Tables | `tableView`, `SortFilterProxy` |
| Bars | `progressBar` (stacked variant needed) |
| Charts | `chart` (line, bar) |
| Inspector | `propertyGrid`, `label`, `button` |
| Dialogs | `dialog`, `checkBox`, `comboBox` |

Missing: treemap and sunburst (`UT04-GAP-1` = FUSE-WP-47; fallback is the list alone); a stacked, labelled bar (`UT04-GAP-2`, small, could be a `chart` kind in FUSE-WP-46); sheets and popovers (FUSE-WP-30).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | App host | Views, metric maths, finders' ranking, suggestions, clean-up orchestration |
| UI | FUSE UI | App host | All views |
| `scan-helper` | C++23 | One per scan; capability mode | Walk a tree by directory handle (`openat` beneath, `fstatat`, directory reads). Rights: lookup, list, stat. Emits batched records over shared memory |
| `hash-helper` | C++23 | One per duplicates job; capability mode | Receives read-only file handles for candidate files, returns BLAKE3 digests. No directory handles |
| Scan store | SQLite | App host | Current tree, history, journal |
| Storage accounting client | ForgeScript | App host | Read-only queries for pool, dataset and snapshot figures and dry-run destroy estimates |
| Background sampler | ForgeScript, headless | Own process when enabled | Daily dataset totals, optional scheduled scans |

```
 folder portal ──dir handle (lookup/list/stat)──► App host ──► scan-helper (cap mode, no net)
                                                    │  ▲ records (shm)
                                                    │  └────────────────────────────
                                                    ├──► hash-helper (ro file handles)
                                                    ├──► storage accounting service (read-only; platform/AD02)
                                                    ├──► svc.undo-trash (trash, journal, undo)
                                                    └──► platform app-data call (sizes, clear cache)
```

Native helpers are justified by speed (PLATFORM.md section 2, case b): millions of `stat` calls through the interpreter are too slow. The helpers hold nothing that can delete: all destructive actions go through `svc.undo-trash` or the owning service, from the app host, on explicit user action.

### 6.2 Data model

Stores: `~/.local/share/org.pbsd.disk-usage/scans.sqlite` (index of scans, journal, history samples), `~/.local/share/org.pbsd.disk-usage/scans/<id>.scan` (one tree per scan, own columnar format, zstd-compressed), `~/.cache/org.pbsd.disk-usage/` (working tree of the open scan, duplicate hash cache keyed by device, inode, size, mtime).

| Entity | Fields |
|---|---|
| `Scan` | `id: u64`, `target_kind: enum{folder, dataset, pool, volume, remote}`, `target_label: string`, `grant_token: string`, `dataset: string?`, `started: i64`, `ended: i64`, `complete: bool`, `items: u64`, `apparent: u64`, `allocated: u64`, `errors: u32`, `elevated: bool`, `tool_version: u16` |
| `Node` | `id: u64`, `parent: u64`, `name: bytes`, `kind: enum{dir, file, symlink, other, mount, no_access, grouped}`, `apparent: u64`, `allocated: u64`, `items: u64` (subtree), `mtime: i64`, `newest_mtime: i64` (subtree), `atime: i64?`, `uid: u32`, `dev: u64`, `ino: u64`, `nlink: u32`, `type_cat: u8`, `flags: bitset{sparse, compressed, hardlinked, cloned, held_est}` |
| `DatasetInfo` | `name`, `used`, `usedbydataset`, `usedbysnapshots`, `usedbychildren`, `usedbyrefreservation`, `referenced`, `logicalused`, `compressratio: f32`, `available`, `quota`, `refquota`, `reservation`, `refreservation`, `origin: string?`, `encrypted: bool`, `locked: bool`, `mounted_at: string` |
| `SnapshotInfo` | `name`, `dataset`, `created: i64`, `used`, `referenced`, `written`, `holds: list<string>`, `clones: list<string>` |
| `PoolInfo` | `name`, `size`, `allocated`, `free`, `fragmentation: u8`, `dedup_ratio: f32`, `health: string` |
| `AppUsage` | `app_id`, `bundle: u64`, `data: u64`, `config: u64`, `cache: u64`, `last_used: i64` |
| `DupGroup` | `id`, `size: u64`, `hash: bytes32`, `members: list<node_id>` |
| `Suggestion` | `id`, `kind: enum`, `title`, `bytes: u64`, `would_free: u64`, `exact: bool`, `risk: enum{safe, check, no_undo}`, `items_ref` |
| `JournalEntry` | `id`, `when: i64`, `action: enum{trash, delete, clear_cache, empty_trash, link_dups}`, `count: u64`, `bytes: u64`, `would_free: u64`, `undo_token: string?` |
| `HistorySample` | `target`, `when: i64`, `used: u64`, `by_snapshots: u64?` |

"Would free" for a node = `allocated` minus bytes estimated as held (F055) minus bytes shared by links, clones or dedup outside the selection. It is stored as a figure plus `exact: bool`.

Schema: `meta.schema_version` in `scans.sqlite`; `.scan` files carry a format version and are never migrated (old ones open read-only or are dropped).

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Directory trees (through handles) | yes | no | own C++23 | scan-helper |
| `.scan` tree store | yes | yes | own; zstd from base (`hbsd/src/sys/contrib/zstd`) | App host (own files only) |
| ZFS accounting records | yes | no | bus schema from the storage accounting service | App host (typed records) |
| File contents for hashing | yes | no | BLAKE3 (FORGE crypto, Proposed) | hash-helper |
| CSV, JSON, Markdown reports | no | yes | own | App host |
| PDF report | no | yes | print-to-PDF pipeline (G-16, UT06) | print service |
| PNG, SVG map image | no | yes | own writers | App host |
| Hog rules (`rules.tbl`, bundle data) | yes | no | own | App host |

No third-party library is added by this app.

### 6.4 Integration

- Intents handled: `open` and `view` for `inode/directory` ("Analyse disk usage" in SY02's context menu), custom `disk-usage.low-space`.
- Intents emitted: `open` (items, SY02 reveal, SY03 terminal), `new` to UT01 (compress), hand-offs to AD02, AD03, SY05, AD04, AI02, VM01.
- Services: `svc.portal.file`, `svc.undo-trash`, `svc.notify`, `svc.settings`, `svc.backup`, `svc.audit`, `svc.power`, `svc.intent`, `svc.portal.print`, `svc.thumbnail` (quick look).
- Needs two platform interfaces that PLATFORM.md does not list:
  - **Storage accounting (read-only)**: pool, dataset and snapshot properties, user and group space, dry-run destroy estimates. Proposed as part of the storage service that AD02 Disks fronts. New gap, reported as `UT04-GAP-3`.
  - **App data sizes and cache clearing**: sizes of each app's section 10 directories and a "clear cache of app X" call that coordinates with the app. Proposed in the app host platform (G-07) or `svc.backup`'s per-app rules. New gap, `UT04-GAP-4`.
- Links: SY02 (context action, reveal, trash view), AD02, AD03 (snapshot destroy), SY05 (boot environments, packages), SY04 (shares the pool figures), SH02 (low-space notice).

## 7. Security and privacy

| Capability | Why |
|---|---|
| `fs.app` | Scan store, history, caches |
| `fs.portal` | Folders the user chooses to scan; write grant only when the user cleans up |
| `net.none` | No network |
| `proc.helper:scan-helper` | Fast, contained tree walk |
| `proc.helper:hash-helper` | Contained content hashing |
| `svc.portal.file` | Folder grants |
| `svc.undo-trash` | Trash, undo, journal |
| `svc.notify` | Scan finished, low-space action |
| `svc.settings` | Preferences |
| `svc.backup` | Safety snapshot; retention facts |
| `svc.audit` | Elevated scans, permanent deletes |
| `svc.power` | Inhibit sleep during a scan; battery state |
| `svc.intent` | Hand-offs |
| `svc.portal.print` (`dev.print`) | Print report |
| `svc.thumbnail` | Quick look |
| `svc.storage` (Proposed name, `UT04-GAP-3`) | Read-only ZFS accounting |
| `bg.run` | Optional scheduled scans and daily sampling (off by default) |

Home is not granted by default. On first run the app asks through the portal for a read-and-list grant to the home folder ("always" is offered). A scan grant never includes write; the first clean-up in a scan root asks for it.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| User files | Bug in this app, or a confused user | Mass deletion | Trash by default with undo; counts and bytes in every confirmation; protected paths; no helper holds delete rights; re-check before acting on stale scans |
| File names and layout (sensitive metadata) | Other apps, backups | Read the scan history | Stores in `fs.app`; history depth and retention settings; "no history" mode; `.scan` files excluded from backup by default |
| Files outside the granted folder | Crafted directory tree | Symlink or mount tricks to make the scan or a delete leave the root | Beneath-only handles; links never followed; boundaries not crossed; deletes go by handle-relative path and are re-verified by device and inode |
| Other users' data | The elevated scan | Over-broad read | Elevated grant is list and stat only (no file read), one scan, logged; contents hashing is refused under an elevated grant |
| Other apps' private data | This app | Reading or deleting it while measuring | Sizes come from a platform call; cache clearing is done by the platform; no handles to app directories |
| The scan helper | Hostile names (control characters, overlong, invalid UTF-8) | Parser or UI confusion | Names are bytes end to end; display escapes; length-prefixed records |
| System availability | Huge trees | Memory exhaustion | Out-of-core store; grouping of small items; item cap with a warning (default 200 million) |
| Truthfulness | — | Wrong "would free" leads to deleting more than needed | Estimates are labelled; exact figures only from the service |

Stored: scans (names, sizes, times, owners), journal, history samples, duplicate hash cache. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `scan.cross_boundaries` | bool | false | Descend into other datasets and mounts |
| `scan.priority` | enum | `low` | low, normal, fast |
| `scan.exclude` | list<string> | empty | Global exclude globs |
| `scan.pause_on_battery_pct` | u8 | 20 | Pause scans below this charge |
| `scan.item_cap` | u64 | 200 000 000 | Warn and stop above |
| `view.layout` | enum | `map_and_list` | map_and_list, map, list |
| `view.map` | enum | `treemap` | treemap, sunburst |
| `view.metric` | enum | `allocated` | apparent, allocated, would_free |
| `view.colour_by` | enum | `type` | type, age, owner, depth, compression, snapshot_only |
| `view.depth` | u8 | 3 | Map depth |
| `view.group_below_pct` | f32 | 0.5 | Merge items under this share |
| `view.units` | enum | `binary` | binary, decimal |
| `find.large_count` | u16 | 100 | Largest files listed |
| `find.old_months` | u16 | 12 | "Old" threshold |
| `find.dup_min_size` | bytes | 1 MiB | Smallest file considered |
| `cleanup.confirm_trash_over` | bytes | 10 GiB | Confirm trashing above this |
| `cleanup.offer_snapshot_over` | bytes | 1 GiB | Offer a safety snapshot above this |
| `history.keep_scans` | u16 | 12 | Scans kept per target |
| `history.name_depth` | u8 | 0 (all) | Keep names only to this depth in history |
| `history.enabled` | bool | true | Keep completed scans |
| `background.daily_sample` | bool | false | Sample dataset totals daily |
| `background.weekly_scan` | list | empty | Targets scanned weekly |

## 9. Accessibility and languages

- The list view carries everything the map shows. The map is a single focusable region with arrow-key navigation; each tile announces name, size and share.
- Colour-by modes have a legend with words; high contrast adds patterns; nothing is conveyed by hue alone (growth and shrink use arrows).
- Sizes are read as numbers with units ("eleven point two gibibytes"), using locale number formats.
- File names in any script and direction; invalid byte sequences shown escaped.
- The ZFS explain panel is plain sentences from string tables, with plural forms.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | 200 ms (Overview from cached figures) |
| Scan rate, local SSD, warm cache | 500 000 items per second in the helper |
| First usable map | Within 1 s of scan start |
| Map layout | 16 ms for 5 000 visible tiles |
| Largest tree | 200 million items out of core |
| Host memory | 150 MiB for a 5 million item scan (rest on disk) |
| Helper memory | 64 MiB |
| Duplicate hashing | Disk-bound; BLAKE3 at 1 GiB/s per core or faster |
| Stored scan size | Under 12 bytes per item after compression (target) |
| Idle | No wake-ups without a scan or the sampler |

## 11. Testing and proof

- **Unit**: aggregation, hard-link accounting, grouping, metric maths, "would free" rules, keep rules, suggestion ranking, growth fit.
- **Fixture trees**: generated trees with known totals (hard links, sparse files, deep paths, 1 million small files, odd names, loops through links, mount boundaries). On ZFS test pools: snapshots, clones, compression, dedup, reservations, with expected figures taken from the pool itself.
- **Reclaim truth test**: on a test pool, record the predicted "would free", perform the delete, and compare with the real change in free space. Exact figures must match; estimates must be within a stated error and never claim more than is freed.
- **Golden**: all views, themes, scales.
- **Scripted UI**: T1–T12.
- **Differential**: totals against `du -A` / `du` and dataset properties for the same tree.
- **Fuzz**: `.scan` reader, record stream from the helper, names.
- **Sandbox**: scan-helper cannot open files for write, unlink, or leave the root (G-06).

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Aggregation | Subtree totals equal the sum of children; hard links counted once per root | T1 | PRISM proof on the fold over the tree model | — |
| Scan walker (C++23) | Never leaves the root handle; no runtime errors | T2 | PRISM; fixture trees; sandbox test | Kernel semantics are trusted, not proved |
| "Would free" (exact path) | Never greater than `allocated`; equals the service figure | T2 | PRISM plus reclaim truth test | — |
| "Would free" (estimate) | Labelled; conservative on test pools | T3 | Reclaim truth test | Depends on ZFS internals |
| Duplicate finder | Reported members are byte-identical (given the hash) and not the same inode | T2 | PRISM on grouping; tests | Hash collision assumption |
| Treemap layout | Tiles tile the rectangle, areas proportional within rounding | T2 | PRISM on the layout function (FUSE side) | Owned by FUSE-WP-47 |
| App logic | Memory-safe by language | T3 | Tests | — |

Nothing here has been run.

## 12. Build plan

Phase 1: usable core.

| ID | Work item | Depends on |
|---|---|---|
| UT04-W01 | scan-helper with record stream and limits | G-06, G-07, G-08, FORGE-WP-27, FORGE-WP-28 |
| UT04-W02 | Scan store (working tree, out of core) and aggregation | FORGE-WP-20 |
| UT04-W03 | List view, navigation, inspector, find | G-02 |
| UT04-W04 | Treemap view linked to the list | FUSE-WP-47 (`UT04-GAP-1`) |
| UT04-W05 | Overview with volumes (plain figures) and trash size | G-09 (`svc.portal.file`, `svc.undo-trash`) |
| UT04-W06 | Move to trash, delete for ever, undo, live tree update | UT04-W02 |
| UT04-W07 | Largest files; metric switch apparent/allocated | UT04-W02 |
| UT04-W08 | Fixture trees and differential tests | UT04-W01 |

Phase 2: full feature.

| ID | Work item | Depends on |
|---|---|---|
| UT04-W09 | Storage accounting client; dataset, pool, snapshot views | `UT04-GAP-3`, AD02 |
| UT04-W10 | "Would free" metric, explain panel, snapshot range estimate | UT04-W09 |
| UT04-W11 | Apps and caches view, clear cache | `UT04-GAP-4`, G-07 |
| UT04-W12 | hash-helper and duplicates | FORGE-WP-22 |
| UT04-W13 | Old files, hogs, small-file folders, suggestions, basket | UT04-W07 |
| UT04-W14 | Sunburst, type and age summaries, filters, colour modes | FUSE-WP-47 |
| UT04-W15 | History, compare, growth chart, export | UT04-W02 |
| UT04-W16 | Incremental rescan, crash resume, elevated scan | SH04, AD05 |
| UT04-W17 | Hand-offs: AD03 snapshot destroy, SY05, AD04; low-space entry | AD03, SY05, SH02 |
| UT04-W18 | Reclaim truth test rig on ZFS test pools | UT04-W10 |

Phase 3: advanced.

| ID | Work item | Depends on |
|---|---|---|
| UT04-W19 | Remote scans | SY02 remote locations |
| UT04-W20 | Dedup, block cloning, zvols, owner summary | UT04-W09 |
| UT04-W21 | Replace duplicates by links or clones | UT04-W12 |
| UT04-W22 | Background sampler, scheduled scans, forecast | FORGE-WP-10 (headless host) |
| UT04-W23 | Safety snapshot, compress hand-off, map image export, print | UT01, G-16 |

## 13. Open questions and risks

- There is no storage accounting service in PLATFORM.md section 8. Without it the app cannot show any ZFS figure, because it has no `proc.exec` and must not run `zfs` itself. This is the main dependency (`UT04-GAP-3`).
- Exact per-file "held by snapshot" answers are expensive on ZFS. The plan uses a labelled estimate and exact figures only for snapshot ranges and whole datasets. If the service can diff a file against the newest snapshot cheaply, the estimate improves.
- How apps declare cache directories beyond the fixed `~/.cache/<id>/` (PLATFORM.md section 10) is not defined; a manifest field for extra re-creatable directories would help (`UT04-GAP-4`).
- Whether a read-and-list grant to all of home is acceptable as an "always" grant for this app is a policy question for AD05.
- atime is often off on ZFS; "not read for n months" then falls back to mtime and says so.
- The treemap widget is planned (FUSE-WP-47) but not built; phase 1 can ship with the list only.
- ZFS property semantics were written from knowledge of ZFS, not from reading `hbsd/src/sys/contrib/openzfs`; the fixture pools are the check.
