<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Archives

By Odin Loch

| | |
|---|---|
| ID | `UT01` (from CATALOGUE.md) |
| Bundle | `org.pbsd.archives` |
| Category | utils |
| Priority | P1 |
| Backing repos | HardenedBSD base: `hbsd/src/contrib/libarchive` (libarchive 3.8.7, BSD 2-clause, Verified); FORGE (crypto, hashing: Proposed use); FUSE (UI) |
| Status | Planned. Nothing built. |

## 1. Purpose

Archives opens, creates, tests and extracts archive files. Every archive is hostile data: all parsing runs in a helper with no file-system or network reach, and extraction can write only inside the folder the user chose. It replaces Ark from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- Browse, preview, extract, create, update, test and compare archives.
- Formats: zip, 7z, tar with gzip/bzip2/xz/zstd/lz4, rar (extract only), cpio, ar, ISO 9660, UDF, cab, deb, rpm, FreeBSD pkg, xar, lha, single-file compressed streams.
- Encrypted archives: AES zip, 7z AES-256, age-wrapped archives.
- Safe extraction with defined limits; quotas; provenance tags on extracted files.
- Zstandard dictionaries: train, store, use.
- Split volumes, checksums, batch jobs, compression profiles.
- Read-only "mount" of an archive as a location in Files (SY02).
- The `archive-helper` service used by other apps through the session bus.

Out of scope:

| Item | Owner |
|---|---|
| File management outside an archive, the folder picker | SY02 Files, SH04 Portals |
| Encrypted containers, secure erase, general file encryption and signing | SE05 Vault |
| Key and passphrase storage, age identities, OpenPGP keys | SE04 Passwords and Keys |
| Installing deb, rpm or pkg packages (this app only looks inside them) | SY05 Software |
| Writing ISO images to media, disk images of partitions | AD02 Disks |
| Backups, ZFS send streams | AD03 Backup |
| Running Windows installers found in cab or exe archives | VM03 Compatibility |
| Hex view of a damaged archive | DV04 Hex Editor |
| Downloading archives | IN03 Transfers |

## 3. Users and top tasks

Personas:

- **Everyday user**: receives a zip, wants the files out, and wants to send a folder as one file.
- **Admin**: handles tarballs, packages, cpio initramfs images, ISO files; needs permissions and ownership shown exactly.
- **Careful user**: opens archives from unknown senders; wants to look before anything touches the disk.
- **Developer**: makes reproducible release tarballs, compares two builds, checks hashes.

| # | Task | Target steps |
|---|---|---|
| T1 | Extract a zip from Files to a folder beside it | 1 (context menu "Extract Here") |
| T2 | Look inside an archive without extracting | 1 (double click) |
| T3 | Preview one file inside an archive | 2 (open, `Space`) |
| T4 | Extract three chosen files to a chosen folder | 3 |
| T5 | Compress a folder to `.tar.zst` | 2 (context menu, Create) |
| T6 | Create an encrypted archive with a passphrase from the keyring | 4 |
| T7 | Add files to an existing zip by dropping them in | 1 (drop) |
| T8 | Test an archive and see which entries are damaged | 2 |
| T9 | Compare two archives and see changed files | 3 |
| T10 | Verify a downloaded archive against a published SHA-256 | 2 (open, paste hash) |
| T11 | Extract 40 archives, each to its own folder | 3 |
| T12 | Join and extract a split archive | 1 (open the first volume) |
| T13 | Open an untrusted archive in a disposable sandbox | 2 |

## 4. Features

### 4.1 Opening and browsing

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT01-F001 | Open by handle | Archives arrive as read handles from `svc.intent` `open`/`view`, from the file portal, or by drop. The app never sees a path outside its grants. | M | 1 |
| UT01-F002 | Format detection by content | The helper identifies format and filters from magic bytes, never from the extension. A mismatch between name and content shows a `--warn` banner ("named .zip, is a tar"). | M | 1 |
| UT01-F003 | Entry listing | Streamed listing; first rows appear before the scan ends. Columns: name, size, packed size, ratio, modified, type, mode, owner, group, CRC or hash, encrypted, method, comment. | M | 1 |
| UT01-F004 | Tree and flat views | Folder tree with a breadcrumb, or one flat list of all entries. Toggle `Ctrl+1` / `Ctrl+2`. | M | 1 |
| UT01-F005 | Sorting and column chooser | Click headers to sort; choose columns; settings kept per format family. | M | 1 |
| UT01-F006 | Filter and search | `Ctrl+F` filters by name (substring, glob, regex). Filters by type, size range, date range, "encrypted only", "flagged only". | M | 1 |
| UT01-F007 | Nested archives | Opening an archive inside an archive opens it in a new tab, streamed through the helper into memory or the app cache (limit `nested.max_bytes`, depth limit 8). | S | 2 |
| UT01-F008 | Tabs | One archive per tab; tabs can be dragged out to windows. | S | 1 |
| UT01-F009 | Archive properties | Inspector shows format, filters, solid or not, volumes, entry count, total and packed size, ratio, comment, creation tool where recorded, hashes of the archive file. | M | 1 |
| UT01-F010 | Entry properties | Inspector shows all header fields for the selection, including extended attributes, ACLs, file flags, sparse map, link target, device numbers, Unix and DOS times. | S | 2 |
| UT01-F011 | Risk flags | Each entry is checked at listing time and flagged: absolute path, `..` component, symlink or hard link leaving the root, device node, setuid/setgid, executable, name with control or bidirectional override characters, name clash on a case-insensitive target, duplicate name, ratio above `limits.max_ratio`. | M | 1 |
| UT01-F012 | Risk summary | Status bar trust badge: "No risks found", "n entries flagged", or "Unsafe: will be skipped on extract". Click opens the flagged filter. | M | 1 |
| UT01-F013 | Name encoding | Entry names decoded as UTF-8 when valid, else by the zip language flag or a chosen code page (CP437, CP932, CP936, CP949, CP1251, CP1252, others from FORGE text). Change it in the status bar; listing reloads. | M | 2 |
| UT01-F014 | Large archives | Listing is virtualised and backed by an index in the cache. Target: one million entries. | S | 2 |
| UT01-F015 | Recent archives | Recent list in the sidebar and start view, held by `svc.portal.file`. Clearable. | S | 1 |
| UT01-F016 | Package views | For deb, rpm and pkg: a "Package" tab shows name, version, dependencies, maintainer scripts (text only, never run) and the payload tree. "Install" hands off to SY05. | S | 2 |
| UT01-F017 | Disc image views | For ISO 9660 and UDF: volume label, Joliet and Rock Ridge selection, El Torito boot entries listed as entries. | S | 2 |
| UT01-F018 | Single-file streams | `.gz`, `.bz2`, `.xz`, `.zst`, `.lz4`, `.Z` with no container show as one entry with the stored name. | M | 1 |
| UT01-F019 | Mtree and manifest view | Read mtree specs and show them as a listing with no payload; used by Compare. | C | 3 |

### 4.2 Preview

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT01-F020 | Quick preview | `Space` on an entry streams it from the helper to memory and shows it in the preview pane. Nothing is written to disk. Limit `preview.max_bytes` (default 64 MiB). | M | 1 |
| UT01-F021 | Text preview | Plain text with encoding detection; first 2 MiB then "Load more". | M | 1 |
| UT01-F022 | Image, PDF and media preview | Through `svc.thumbnail` and the system previewers; the bytes pass as a sealed shared-memory handle. | S | 2 |
| UT01-F023 | Hex preview | Fallback for unknown types: first 64 KiB as hex and ASCII. "Open in Hex Editor" hands off to DV04. | S | 2 |
| UT01-F024 | Open with | Extracts the entry to a private temporary directory, passes a read-only handle through `svc.intent`, and deletes it when the receiving app closes it. | M | 1 |
| UT01-F025 | Edit in place | "Open for editing" passes a read-write handle to a temporary copy; on save the app offers "Update archive" (F056). | S | 2 |
| UT01-F026 | Thumbnails in grid view | Optional icon grid for image-heavy archives (comics, photo sets). | C | 3 |

### 4.3 Extraction

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT01-F030 | Extract all | Destination picked in the folder portal. The helper gets one directory handle with create and write rights beneath it, nothing else. | M | 1 |
| UT01-F031 | Extract selection | Selected entries or folders only; paths kept relative to the current folder or to the archive root (option). | M | 1 |
| UT01-F032 | Extract Here, Extract to "name/" | Offered through SY02's context menu by intent. "Extract Here" makes a subfolder when the archive has more than one top-level entry (tarbomb guard). | M | 1 |
| UT01-F033 | Path containment | Every output path is resolved beneath the destination handle with `openat` and the beneath flag. `..`, absolute paths and drive letters are rejected before any write. Verified engine flags: `ARCHIVE_EXTRACT_SECURE_NODOTDOT`, `SECURE_SYMLINKS`, `SECURE_NOABSOLUTEPATHS` (`hbsd/src/contrib/libarchive/libarchive/archive.h:723-742`). The helper's own check comes first; the flags are a second layer. | M | 1 |
| UT01-F034 | Symlink policy | Default: symlinks are created only when the target stays inside the extraction root; others are skipped and listed. Writes never follow a symlink created earlier in the same job. Options: skip all, keep safe, materialise as files. | M | 1 |
| UT01-F035 | Hard link policy | Hard links to entries inside the archive are honoured; links to paths outside are refused. | M | 1 |
| UT01-F036 | Special files | Device nodes, FIFOs and sockets are never created. They are listed in the report. | M | 1 |
| UT01-F037 | Permission policy | Default: mode masked by umask, setuid/setgid/sticky stripped, owner is the user. "Preserve permissions" is an option; preserving ownership needs AD-level rights and is refused in the app. | M | 1 |
| UT01-F038 | Decompression limits | Per job: max total bytes (`limits.max_total`, default 4× free space cap or 100 GiB), max entries (5 million), max ratio per entry (1000:1 above 16 MiB), max path length (1023 bytes), max depth (128). Crossing a limit pauses the job and asks. | M | 1 |
| UT01-F039 | Quota check | Before writing, the sum of declared sizes is compared with free space and dataset quota from `svc.portal.file`. Sizes are also counted as written because headers lie. | M | 1 |
| UT01-F040 | Overwrite policy | Ask, skip, overwrite, overwrite if newer, keep both (suffix ` (2)`). "Apply to all". Default ask. | M | 1 |
| UT01-F041 | Atomic extract | Files are written to `.partial` names and renamed on completion. A cancelled or failed job removes its partial files. | M | 1 |
| UT01-F042 | Undo extract | The job journal goes to `svc.undo-trash`; Undo moves created files to trash. | S | 2 |
| UT01-F043 | Provenance tags | Extracted files inherit the archive's provenance tag (for example "downloaded from host X") plus "extracted from <archive hash>". Executable bits on files from untrusted archives are cleared unless the user opts in. | M | 2 |
| UT01-F044 | Extraction report | End-of-job sheet: counts written, skipped, renamed, refused, with reasons. Exportable as text or JSON. | M | 1 |
| UT01-F045 | Extract to disposable sandbox | Extracts into a throwaway dataset and opens a Files window scoped to it. Nothing is kept unless the user copies it out. | S | 3 |
| UT01-F046 | Timestamps and attributes | Restore mtime always; atime, birth time, xattrs, ACLs and file flags by option. | S | 2 |
| UT01-F047 | Sparse files | Sparse maps are honoured so holes stay holes. | C | 2 |
| UT01-F048 | Name repair on extract | Names illegal on the target (NUL, `/` inside a component) are refused; names with control characters are escaped; long names are shortened with a hash suffix. All are listed in the report. | M | 1 |
| UT01-F049 | Drag out | Dragging entries to Files or the desktop extracts them to the drop target through a portal-granted directory handle. | M | 1 |

### 4.4 Creating and updating

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT01-F050 | Create archive | From the app, from SY02's "Compress…" intent, or by dropping files on the empty window. Sources arrive as read handles. | M | 1 |
| UT01-F051 | Write formats | zip, 7z, tar (pax default, ustar, GNU), tar + gzip/bzip2/xz/zstd/lz4, cpio (newc, odc), ar, xar, ISO 9660, single-file gz/bz2/xz/zst/lz4. No RAR creation: there is no free RAR compressor. | M | 1 |
| UT01-F052 | Default format | `.tar.zst` for folders with Unix metadata, `.zip` when "for other systems" is ticked. Set in preferences. | M | 1 |
| UT01-F053 | Compression level and method | Level slider per method with an estimate of time and size from a sample of the input. Threads, window or dictionary size, solid mode (7z), long-distance matching (zstd). | M | 1 |
| UT01-F054 | Compression profiles | Named presets: Fast, Balanced, Small, Store, Reproducible, For other systems. Users add their own. Stored in `svc.settings`. | S | 2 |
| UT01-F055 | Add and remove entries | Drop files into an open archive, or Edit › Add Files. Delete removes entries. For formats that cannot be changed in place the archive is rewritten to a temporary file and swapped by rename. | M | 2 |
| UT01-F056 | Update in place | Zip: append and rewrite the central directory. Others: stream-rewrite. The old file is kept until the new one is synced; a snapshot hint goes to `svc.backup`. | S | 2 |
| UT01-F057 | Rename and move inside | `F2` renames an entry; drag moves it between folders in the archive. | S | 2 |
| UT01-F058 | Exclude rules | Glob list per job and global defaults (`.DS_Store`, `Thumbs.db`, `.git/` optional). "Respect ignore files" option. | S | 2 |
| UT01-F059 | Metadata control | Choose what is stored: owner names or numeric ids or none, permissions, timestamps, xattrs, ACLs. | S | 2 |
| UT01-F060 | Reproducible mode | Sorted entry order, fixed mtime (chosen or `SOURCE_DATE_EPOCH` value typed in), uid/gid 0, no owner names, fixed gzip header. Two runs give identical bytes. | S | 2 |
| UT01-F061 | Split volumes | Split output into parts of a given size (zip split, 7z volumes, or plain `.001` byte split). Presets: 25 MB, 100 MB, 700 MB, 4 GiB minus 1. | S | 2 |
| UT01-F062 | Join volumes | Opening any part finds its siblings in the same granted folder (the app asks for folder access if it only has one file). Missing parts are named. | S | 2 |
| UT01-F063 | Convert | Re-pack an archive into another format in one step, streaming entry by entry; encrypted entries need the passphrase. | S | 2 |
| UT01-F064 | Archive comment | View and edit zip and rar comments (rar read only). | C | 2 |
| UT01-F065 | Follow links option | Store symlinks as links (default) or the files they point to. Links that leave the source set are warned about. | S | 2 |
| UT01-F066 | Delete sources after | Optional "Move sources to trash when done", only after a successful test of the new archive. | C | 3 |
| UT01-F067 | Self-check after create | Every created archive is tested (F080) before the job is reported done. On by default. | M | 1 |

### 4.5 Encryption and signing

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT01-F070 | Read AES zip | WinZip AES-128/192/256 entries. Legacy ZipCrypto is read with a `--warn` "weak encryption" badge. | M | 1 |
| UT01-F071 | Write AES-256 zip | AES-256 only. ZipCrypto is never written. A note says zip does not hide file names. | M | 2 |
| UT01-F072 | Read and write 7z AES-256 | Including encrypted headers (hidden names). The engine in the tree does not decrypt 7z (see 6.3); this is `UT01-GAP-1`. | S | 2 |
| UT01-F073 | age-wrapped archives | Create `.tar.zst.age`: the archive stream is encrypted with age (X25519 recipients or scrypt passphrase). Recipients come from SE04. Names and sizes are hidden. This is the recommended encrypted format. | S | 2 |
| UT01-F074 | Passphrase entry | Secret field drawn per DESIGN_SYSTEM.md section 5. The passphrase goes to the helper over the bus and is never written to disk or logs. | M | 1 |
| UT01-F075 | Keyring use | "Remember for this archive" stores the passphrase in `svc.keyring` under the archive hash. "Generate" asks SE04 for a passphrase. | S | 2 |
| UT01-F076 | Strength hint | Entropy estimate under the field; no minimum is forced. | C | 2 |
| UT01-F077 | Encrypted badge | Lock icon in the header bar and per entry; inspector says cipher and whether names are hidden. | M | 1 |
| UT01-F078 | Detached signatures | Verify a detached signature (Ed25519 over BLAKE3 as in PLATFORM.md V14, or OpenPGP through SE04) found beside the archive. Sign on create with a key from SE04. | S | 3 |
| UT01-F079 | RAR encryption | Encrypted RAR 3/4 entries: read if the engine supports it. RAR5 encryption: not supported by the engine read here (no AES code in `archive_read_support_format_rar5.c`); reported plainly as unsupported. | C | 3 |

### 4.6 Integrity, checksums, repair, compare

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT01-F080 | Test archive | Decompress every entry to nowhere and check CRCs and structure. Results per entry: OK, bad CRC, truncated, unsupported method. | M | 1 |
| UT01-F081 | Archive hashes | SHA-256, SHA-512, BLAKE3 and (labelled legacy) SHA-1, MD5, CRC32 of the archive file. Computed in the helper. | M | 1 |
| UT01-F082 | Verify against a pasted hash | Paste a hash or drop a `SHA256SUMS`-style file; the matching algorithm is chosen by length; result is a trust badge. | M | 1 |
| UT01-F083 | Per-entry hashes | Compute hashes of selected entries without extracting; copy or export as a sums file. | S | 2 |
| UT01-F084 | Repair zip | Rebuild a central directory by scanning local headers; output is always a new file. | S | 3 |
| UT01-F085 | Salvage | Extract what can be read from a truncated or damaged archive; damaged entries get a `.damaged` suffix and appear in the report. | S | 2 |
| UT01-F086 | Recovery records | Read and use RAR recovery records: not supported (stated). Create PAR2-style parity for own archives: could, own code. | C | 3 |
| UT01-F087 | Compare two archives | Side-by-side tree: only in A, only in B, different content (by hash), different metadata only. Filter by state. | S | 2 |
| UT01-F088 | Compare with a folder | Same view between an archive and a granted folder; shows what changed since the archive was made. | S | 3 |
| UT01-F089 | Diff an entry | For a changed text entry, hand both versions to OF01's diff view; binary to DV04. | C | 3 |
| UT01-F090 | Export listing | Listing as CSV, JSON or mtree text. | S | 2 |

### 4.7 Batch jobs and automation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT01-F100 | Job queue | All long work runs as jobs with progress, speed, time left, pause, resume and cancel. Jobs survive closing the window (`bg.run`) and report through `svc.notify`. | M | 1 |
| UT01-F101 | Batch extract | Many archives at once: each to its own folder, or all to one. One report. | S | 2 |
| UT01-F102 | Batch create | One archive per selected item, or one archive of all. | S | 2 |
| UT01-F103 | Batch test and convert | Test or convert a set; failures listed. | C | 3 |
| UT01-F104 | Concurrency limit | Jobs run one at a time per destination device by default; setting `jobs.parallel`. | S | 2 |
| UT01-F105 | Power awareness | Holds a sleep inhibitor from `svc.power` during a job; lowers thread count on battery. | S | 2 |
| UT01-F106 | Command palette actions | Every action is in the shared action table (PLATFORM.md section 5). | M | 1 |
| UT01-F107 | Bus interface | `org.pbsd.archives.v1`: `list`, `extract`, `create`, `test`, `hash` for other apps, each call bounded by the caller's own handles. | S | 2 |
| UT01-F108 | User scripts | ForgeScript job recipes ("extract, then rename by pattern") run in a nested zero-capability engine with only the job API (FORGE-WP-10.6). | C | 3 |

### 4.8 Zstandard dictionaries

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT01-F110 | Train a dictionary | Train from a sample set of files (min 100 samples suggested); choose size (default 112 KiB). | C | 3 |
| UT01-F111 | Dictionary library | Dictionaries stored in app data with id, name, hash, sample source note. Import and export `.dict`. | C | 3 |
| UT01-F112 | Use on create and extract | Pick a dictionary when creating many small `.zst` files; on extract the dictionary id in the frame selects it, or the app asks for it. | C | 3 |

### 4.9 Mount and Files integration

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT01-F120 | Read-only location in Files | "Browse in Files" registers the archive as a virtual read-only location served by the helper through SY02's location-provider interface. No kernel mount, no setuid. | S | 3 |
| UT01-F121 | Unmount and lifetime | The location ends when the user ejects it, the archive handle is revoked, or the session ends. | S | 3 |
| UT01-F122 | Files context actions | Intents registered: Extract Here, Extract to…, Compress…, Test, Open in disposable sandbox. | M | 1 |
| UT01-F123 | Search contribution | Optional: entry names of archives in indexed folders are offered to `svc.search`. Off by default. | C | 3 |

### 4.10 Settings, accessibility, recovery

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT01-F130 | Preferences | Section 8. | M | 1 |
| UT01-F131 | Format support page | Help › Formats lists each format, read/write/encrypt support, the engine version and licence. | M | 1 |
| UT01-F132 | Job recovery | The job journal lets an interrupted job resume or roll back after a crash or power cut. | S | 2 |
| UT01-F133 | Helper crash handling | A helper crash fails only its job; the entry being parsed is named in the report. | M | 1 |
| UT01-F134 | Screen-reader progress | Job progress is a polite live region at 10% steps; risk flags are read as words. | M | 1 |
| UT01-F135 | Audit trail | Extraction of flagged archives, limit overrides and passphrase failures go to `svc.audit`. | S | 2 |
| UT01-F136 | Print listing | Print the listing through `svc.portal.print`. | C | 3 |

## 5. GUI

### 5.1 Window map

```
Archives
├─ Main window (tabs: one archive each)
│  ├─ Sidebar: Recent, Jobs, Dictionaries (phase 3)
│  ├─ Listing (tree | flat | grid)
│  ├─ Preview pane (bottom or right)
│  ├─ Inspector (archive / entry properties, risk list)
│  └─ Status bar (selection, job, trust badge, name encoding)
├─ Start view (no archive open)
├─ Extract sheet
├─ Create dialog
├─ Passphrase prompt
├─ Overwrite prompt
├─ Limit reached prompt
├─ Extraction report sheet
├─ Test results sheet
├─ Compare window
├─ Jobs popover
├─ Batch dialog
├─ Hash verify popover
├─ Dictionary manager (phase 3)
├─ Format support page (help)
└─ Preferences
```

### 5.2 Main window

Shown at 1100×680, one archive open, entry selected.

```
+------------------------------------------------------------------------------------------+
| [=] Archives — report-2026.zip          [Search entries Ctrl+F]         (1)[lock] [_][o][x]|
+------------------------------------------------------------------------------------------+
| [report-2026.zip x] [src.tar.zst x] [+]                                              (2) |
+-----------+----------------------------------------------------------+-------------------+
| RECENT (3)| [Extract v] [Add] [Test] [Preview] | [Tree|Flat] [Filter v] (4)| ARCHIVE   (7)|
| report-…  | / > docs >                                           (5) | Format  zip       |
| src.tar…  +----------------------------------------------------------+ Entries 1 284     |
| photos.7z | Name                 Size    Packed  Modified     Flags  | Size    48.2 MiB  |
|           | > figures/           -       -       2026-10-02          | Packed  12.9 MiB  |
| JOBS      |   notes.md           4.1 K   1.6 K   2026-10-30          | Ratio   27%       |
| Extract   | > summary.pdf        2.3 M   2.1 M   2026-11-01          | Encrypted AES-256 |
|  62% ▓▓▓░ |   run.sh             812     401     2026-09-14   [exec] | Names hidden: no  |
|           |   ../../etc/passwd   1.9 K   700     2026-09-14  [unsafe]| SHA-256 9f2c…a71 |
|           |   link -> /tmp/x     0       0       2026-09-14  [unsafe]|  [Copy] [Verify…] |
|           |                                                      (6) +-------------------+
|           +----------------------------------------------------------+ ENTRY             |
|           | Preview: summary.pdf — page 1 of 12                  (8) | summary.pdf       |
|           |  ┌──────────────┐                                        | Mode   0644       |
|           |  │              │                                        | CRC32  7be1 04aa  |
|           |  └──────────────┘                                        | Method deflate    |
+-----------+----------------------------------------------------------+-------------------+
| 1 of 1 284 selected · 2.3 MiB     Extract 62% [||]   (9)[! 2 unsafe entries]  UTF-8 [v] |
+------------------------------------------------------------------------------------------+
```

1. Lock badge: shown when any entry is encrypted. Click shows cipher and whether a passphrase is held for this session.
2. Tab bar (`tabBar`). Drag a tab out to make a window.
3. Sidebar (`listView`): recent archives and running jobs with inline `progressBar`.
4. Toolbar (`toolButton`s). Extract is the one primary action; its dropdown has "Extract All…", "Extract Selection…", "Extract Here", "To disposable sandbox".
5. Breadcrumb (`link` row) for the folder inside the archive.
6. Listing (`treeView` in tree mode, `tableView` in flat mode, `gridView` in grid mode). Flags column shows words with icons: `exec`, `unsafe`, `enc`, `link`, `device`.
7. Inspector (`propertyGrid`, read-only): archive block and entry block.
8. Preview pane (`splitter` below the listing; `markdownView`, `image`, text view or hex by type). Hidden until `Space`.
9. Status bar: trust badge. `--ok` "No risks found", `--warn` "n flagged", `--danger` "n unsafe entries will be skipped".

States: empty (start view), loading (rows stream in; "Listing… 12 400 entries"), error (banner with the entry offset and "Try salvage"), locked (encrypted headers: passphrase prompt replaces the listing), read-only (format cannot be updated: Add disabled with tooltip), narrow (sidebar overlay, inspector as sheet, preview full-pane).

### 5.3 Other views and dialogs

**Start view** (800×520, empty state)

```
+--------------------------------------------------------------------------------+
| [=] Archives                                                        [_][o][x] |
+--------------------------------------------------------------------------------+
|                                                                                |
|                 Drop an archive to open it, or files to compress them.         |
|                                                                                |
|                      [Open Archive…]      [New Archive…]                       |
|                                                                                |
|   Recent                                                                       |
|   report-2026.zip        ~/Documents          yesterday                        |
|   src.tar.zst            ~/Projects           3 days ago                       |
|                                                                [Clear recent]  |
+--------------------------------------------------------------------------------+
```

**Extract sheet** (attached to the main window, 620×420)

```
+-- Extract 1 284 entries (48.2 MiB) ------------------------------------------+
| To        [ ~/Documents/report-2026/            ] [Choose…]             (1) |
| Free space 212 GiB · needs about 48.2 MiB                               (2) |
|                                                                             |
| If a file exists   (•) Ask  ( ) Skip  ( ) Overwrite  ( ) Keep both          |
| Links              [Keep links that stay inside v]                      (3) |
| Permissions        [x] Strip setuid/setgid   [ ] Preserve modes             |
| [x] Clear executable bit (archive is untrusted: downloaded)             (4) |
| [x] Create a folder for the contents                                        |
|                                                                             |
| ! 2 unsafe entries will be skipped.  [Show]                             (5) |
|                                               [Cancel]  [Extract 1 282]     |
+-----------------------------------------------------------------------------+
```

1. `lineEdit` (read-only display of the portal grant) plus the folder portal button. 2. From the portal. 3. `comboBox`. 4. Shown only when the provenance tag says untrusted. 5. Inline warning; the primary button states the real count.

**Create dialog** (680×560)

```
+-- New archive ---------------------------------------------------------------+
| Sources (3 items, 1.2 GiB)                                [Add…] [Remove]   |
|   ~/Projects/site/        folder   1.1 GiB                                  |
|   ~/notes.md              file     4 KiB                                    |
| Save as  [ site-2026-11-12        ] [.tar.zst v]   in [~/Archives] [Choose…]|
| Profile  [Balanced v]   Level [----o-----] 9   Threads [auto v]             |
| Estimate  about 310 MiB, about 40 s                                     (1) |
| [ ] For other systems (zip, no Unix metadata)                               |
| [ ] Reproducible (sorted, fixed times, no owners)   Time [1970-01-01 00:00] |
| Encryption [None v]  (None | Passphrase (age) | Recipients (age) | AES zip) |
|   Passphrase [••••••••••••••] (hold to reveal)  [Generate] [x] Keyring  (2) |
| Split      [No v]          Exclude [.git/, *.o        ] [Defaults]          |
| [x] Test the archive when done                                              |
|                                                    [Cancel]  [Create]       |
+-----------------------------------------------------------------------------+
```

1. Estimate from compressing a 16 MiB sample in the helper. 2. Secret field; shown only when encryption is chosen.

**Passphrase prompt** (420×220): title "Passphrase for report-2026.zip", secret field, `[x] Remember in keyring`, `[Cancel] [Unlock]`. Wrong passphrase shows an inline error; after 5 failures a 2 s delay per try.

**Limit reached prompt** (480×240)

```
+-- This archive is larger than it claims -------------------------------------+
| "data.bin" has expanded to 16 GiB from 2 MiB (ratio 8000:1).                |
| The limit is 1000:1. This pattern is typical of a decompression bomb.       |
|                    [Stop and remove partial files]  [Skip this entry]       |
|                                           [Continue up to 32 GiB]           |
+-----------------------------------------------------------------------------+
```

Default button is Stop. Continue is outlined `--danger` and logs to `svc.audit`.

**Extraction report sheet** (640×440)

```
+-- Extracted to ~/Documents/report-2026/ --------------------------------------+
| 1 280 written · 2 renamed · 2 refused · 0 failed                [Open folder]|
| Filter [All v]                                                               |
|  refused   ../../etc/passwd        path leaves the destination               |
|  refused   link -> /tmp/x          link target leaves the destination        |
|  renamed   Readme.md → Readme (2).md   name clash with README.md             |
|  renamed   a\x1b[2J.txt → a_x1b_2J.txt control characters in name            |
|                                              [Export…] [Undo extract] [Done] |
+------------------------------------------------------------------------------+
```

**Compare window** (1000×640)

```
+------------------------------------------------------------------------------------------+
| [=] Compare — build-a.tar.zst ↔ build-b.tar.zst                               [_][o][x] |
+------------------------------------------------------------------------------------------+
| Show [x] Only in A  [x] Only in B  [x] Changed  [ ] Metadata only  [ ] Same   [Export…] |
+--------------------------------------------+---------------------------------------------+
| A: build-a.tar.zst (2 104 entries)         | B: build-b.tar.zst (2 106 entries)          |
| bin/tool          812 K  3fa1…  changed    | bin/tool          816 K  77c0…  changed     |
| lib/old.so        120 K  only in A         |                                             |
|                                            | lib/new.so        131 K  only in B          |
| share/doc/x.md    2 K    mtime differs     | share/doc/x.md    2 K    mtime differs      |
+--------------------------------------------+---------------------------------------------+
| 3 changed · 1 only in A · 2 only in B · 1 metadata only · 2 098 same                     |
+------------------------------------------------------------------------------------------+
```

Two synchronised `tableView`s in a `splitter`. Enter on a changed text entry opens the diff in OF01.

**Jobs popover** (360 wide, from the status bar): `listView` of jobs, each with name, `progressBar`, speed, time left, `[||]` `[x]`; finished jobs keep a "Report" link for the session.

**Test results sheet**: `tableView` of entries with state OK / bad CRC / truncated / unsupported, a summary line, `[Salvage…]` when anything failed.

### 5.4 Menus

```
File    New Archive… Ctrl+N · Open… Ctrl+O · Open Recent ▸ · Open in Disposable Sandbox
        Extract All… Ctrl+E · Extract Selection… Ctrl+Shift+E · Extract Here Ctrl+Alt+E
        Convert… · Split… · Properties Alt+Enter · Print Listing… Ctrl+P · Close Tab Ctrl+W · Quit Ctrl+Q
Edit    Undo Ctrl+Z · Redo Ctrl+Shift+Z · Add Files… Ctrl+Shift+A · Add Folder… · New Folder Ctrl+Shift+N
        Rename F2 · Delete from Archive Delete · Select All Ctrl+A · Invert Selection Ctrl+I
        Copy Name Ctrl+C · Copy Path Ctrl+Shift+C · Edit Comment… · Preferences Ctrl+,
View    Tree Ctrl+1 · Flat Ctrl+2 · Grid Ctrl+3 · Preview Space · Preview Pane F3
        Inspector F4 · Sidebar F9 · Columns… · Show Only Flagged Ctrl+Shift+F · Name Encoding ▸ · Refresh F5
Archive Test Ctrl+T · Verify Hash… Ctrl+Shift+H · Hashes of Selection · Compare With… Ctrl+D
        Repair… · Salvage… · Set Passphrase… · Forget Passphrase · Browse in Files
Jobs    Show Jobs Ctrl+J · Pause All · Resume All · Batch Extract… · Batch Create… · Last Report
Tools   Dictionaries… · Compression Profiles… · Export Listing…
Help    Archives Help F1 · Supported Formats · Safe Extraction Explained · About
```

The preview pane toggle is `F3`; `Ctrl+Shift+P` stays the command palette.

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| `Enter` | Open folder or nested archive; open file with default app |
| `Space` | Quick preview |
| `Backspace`, `Alt+Up` | Parent folder |
| `Ctrl+E` / `Ctrl+Shift+E` | Extract all / selection |
| `Ctrl+T` | Test |
| `Ctrl+D` | Compare with… |
| `Ctrl+J` | Jobs popover |
| `F3` | Preview pane |

Context menu on an entry: Preview, Open, Open With ▸, Extract…, Extract Here, Copy Name, Copy Path, Hashes, Rename, Delete from Archive, Properties. On empty space: Add Files…, New Folder, Paste, Select All.

Drag and drop:

| From | To | Result |
|---|---|---|
| SY02 Files (archive) | window | Open in a new tab |
| SY02 Files (other files) | start view | Create dialog with those sources |
| SY02 Files (files) | open, writable archive | Add to the current folder (F055) |
| Listing entries | SY02 Files, desktop | Extract to the drop target (F049) |
| Listing entries | another app | Offer as file handles to temporary copies, plus `text/uri-list` of names |
| Listing entries | folder in the same archive | Move inside the archive |

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Start view. |
| Loading | Streamed rows, spinner in the tab, Cancel in the status bar. |
| Error | Banner: what failed, at which offset, and "Test", "Salvage", "Open in Hex Editor". |
| Offline | No effect; the app has no network. |
| Locked | Encrypted headers: prompt. Encrypted entries only: listing shown, content actions prompt. |
| Read-only | rar, cab, deb, rpm, ISO opened from read-only media, or a read-only handle: editing actions disabled with a reason tooltip. |
| Unsupported | Format known but a method is not supported: listing shown, affected entries flagged "unsupported". |
| Narrow (<640) | Sidebar overlay, inspector sheet, columns reduce to Name, Size, Flags. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Tabs | `tabBar` |
| Sidebar | `listView`, `progressBar` |
| Toolbar | `toolButton`, `comboBox`, `popupMenu` |
| Listing | `treeView`, `tableView` with `SortFilterProxy`, `gridView` |
| Breadcrumb | `link` row in a `panel` |
| Preview | `splitter`, `scrollArea`, `image`, `markdownView`, text view (`codeEdit` read-only) |
| Inspector | `propertyGrid` (read-only) |
| Dialogs | `dialog`, `lineEdit`, `slider`, `spinBox`, `checkBox`, `radioButton` |
| Jobs | `listView`, `progressBar` |

Missing: sheets and popovers (FUSE-WP-30); drag and drop between apps (FUSE-WP-09, G-01); hex view for the preview fallback (FUSE-WP-41); a secret field variant of `lineEdit` (`UT01-GAP-2`, expected to be shared with SE04).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic (`main.fs`, `src/*.fs`) | ForgeScript | App host | View models, job queue, policy decisions, risk flag presentation, settings |
| UI (`ui/*.fuseui`, CSS) | FUSE UI | App host | All windows and dialogs |
| `archive-helper` | C++23 wrapping libarchive (C) | One process per job, capability mode | List, read, extract, create, test, hash. Holds only: the archive handle, optionally one destination directory handle, the bus pipe |
| `crypto-helper` | C++23 | One process per encrypted job | age encrypt/decrypt stream, key derivation; holds the passphrase or key handle; no file handles except pipes |
| `location-provider` | C++23 | One process per mounted archive | Serves read-only directory and file requests to SY02 over the bus |
| Index store | SQLite through `SqliteModel` | App host | Entry index cache for large archives |

```
SY02 Files / svc.intent ──handles──► App host (ForgeScript + FUSE UI)
                                        │ bus: job spec, progress, entry records
                 ┌──────────────────────┼─────────────────────────┐
                 ▼                      ▼                         ▼
        archive-helper           crypto-helper            location-provider
   fds: archive (ro),         fds: pipe in, pipe out   fds: archive (ro)
        dest dir (create/write)    secret via bus            serves SY02
        no net, no exec        no net, no fs             no net, no write
```

The helper is the place libarchive runs. libarchive is C, not the safe C++23 subset, so the containment is the sandbox, not the language: the helper is started already limited, enters capability mode before reading the first byte of the archive, and is single-use.

Path decisions are made twice: in the C++23 wrapper (own code, the path validator) before libarchive's disk writer is called, and by the kernel through the beneath-only directory handle.

### 6.2 Data model

Stores: `~/.local/share/org.pbsd.archives/jobs.sqlite` (journal and reports), `~/.cache/org.pbsd.archives/index/<blake3>.sqlite` (listing caches), settings in `svc.settings`.

| Entity | Fields |
|---|---|
| `ArchiveInfo` | `hash_blake3: bytes32`, `format: enum`, `filters: list<enum>`, `entry_count: u64`, `size: u64`, `unpacked_size: u64`, `solid: bool`, `volumes: u16`, `encrypted: enum{none, data, data_and_names}`, `comment: string`, `name_encoding: string` |
| `Entry` | `id: u64`, `path: bytes` (raw), `path_display: string`, `kind: enum{file, dir, symlink, hardlink, chardev, blockdev, fifo, socket}`, `size: u64`, `packed_size: u64`, `mtime: i64 ns`, `mode: u16`, `uid: u32`, `gid: u32`, `uname: string`, `gname: string`, `link_target: bytes`, `crc32: u32?`, `method: string`, `encrypted: bool`, `flags: RiskFlags` |
| `RiskFlags` (bitset) | `absolute`, `dotdot`, `link_escapes`, `device`, `setid`, `exec`, `ctrl_chars`, `bidi_override`, `case_clash`, `duplicate`, `high_ratio`, `long_path` |
| `Job` | `id: u64`, `kind: enum{list, extract, create, test, convert, hash, compare}`, `state: enum{queued, running, paused, done, failed, cancelled}`, `archive_hash`, `dest_token: string` (portal grant id), `policy: ExtractPolicy`, `bytes_done: u64`, `bytes_total: u64`, `started: i64`, `ended: i64` |
| `ExtractPolicy` | `overwrite: enum`, `links: enum`, `strip_setid: bool`, `preserve_mode: bool`, `clear_exec: bool`, `limits: Limits` |
| `Limits` | `max_total: u64`, `max_entries: u64`, `max_ratio: u32`, `max_depth: u16`, `max_path: u16` |
| `ReportLine` | `job_id`, `entry_path`, `action: enum{written, skipped, renamed, refused, failed}`, `reason: string`, `new_name: string?` |
| `Profile` | `name`, `format`, `filter`, `level: i8`, `threads: u8`, `solid: bool`, `reproducible: bool`, `metadata: bitset` |
| `Dictionary` | `id: u32`, `name`, `hash_blake3`, `size: u32`, `note` |

Schema version is a `meta.schema_version` row in each store. Index caches are disposable and rebuilt on version change. `jobs.sqlite` migrates forward with a copy kept.

### 6.3 File formats and protocols

Engine facts are read from `hbsd/src/contrib/libarchive/libarchive/` (file names `archive_read_support_format_*.c`, `archive_write_set_format_*.c`, `archive_read_support_filter_*.c`, `archive_write_add_filter_*.c`).

| Format | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| zip, zip64 | yes | yes | libarchive (`…format_zip.c`) | archive-helper |
| zip AES (WinZip) | yes | yes (AES-256) | libarchive; AES code is present in the zip reader and writer (Verified by reading the sources for AES references; behaviour not run) | archive-helper |
| 7z | yes | yes | libarchive (`…format_7zip.c`) | archive-helper |
| 7z AES-256 | no in engine: the reader only reports encrypted entries (`archive_read_support_format_7zip.c:392-516`); the writer has no AES code | no | `UT01-GAP-1`: own C++23 7z crypto layer (AES-256-CBC, SHA-256 key stretching) using FORGE crypto (FORGE-WP-22) | archive-helper + crypto-helper |
| tar (pax, ustar, GNU, v7) | yes | yes | libarchive | archive-helper |
| gzip, bzip2, xz, zstd, lz4, compress, lzop, lrzip, grzip filters | yes | yes | libarchive with base `zlib`, `bzip2`, `xz`, `zstd` (in `hbsd/src/contrib` and `hbsd/src/sys/contrib`); lz4 availability in base Unverified | archive-helper |
| rar (v3/v4), rar5 | yes | no | libarchive's own readers (`…format_rar.c`, `…format_rar5.c`), BSD-licensed. No unrar code is used. RAR compression is proprietary and has no free writer, so creation is not offered. RAR5 encryption unsupported. | archive-helper |
| cpio (newc, odc, binary) | yes | yes | libarchive | archive-helper |
| ar (BSD, GNU) | yes | yes | libarchive | archive-helper |
| ISO 9660 (+ Joliet, Rock Ridge) | yes | yes | libarchive | archive-helper |
| UDF | yes | no | Not in the engine read here. Own C++23 reader (Proposed), UDF 1.02–2.60 read only | archive-helper |
| cab | yes | no | libarchive (`…format_cab.c`) | archive-helper |
| deb | yes | no | ar + tar through libarchive, plus own control-file reader | archive-helper |
| rpm | yes | no | libarchive rpm filter (`…filter_rpm.c`) + cpio; own header reader for metadata | archive-helper |
| FreeBSD pkg | yes | no | tar + zstd/xz through libarchive; own manifest (UCL/JSON) reader | archive-helper |
| xar, lha, warc, mtree | yes | xar, warc, mtree | libarchive | archive-helper |
| age v1 | yes | yes | Own C++23 on FORGE crypto (X25519, ChaCha20-Poly1305, scrypt) (Proposed; FORGE-WP-22) | crypto-helper |
| zstd dictionary | yes | yes (train) | zstd library in base (`hbsd/src/sys/contrib/zstd`); dictionary builder availability Unverified | archive-helper |
| Sums files (`SHA256SUMS`, BSD style) | yes | yes | own ForgeScript | App host (text only) |
| Bus `org.pbsd.archives.v1` | — | — | schema compiler (G-08) | App host |

Third-party library summary: libarchive 3.8.7, BSD 2-clause (`hbsd/src/contrib/libarchive/COPYING`), already in the base tree, builds offline with the system. It is used because writing and fuzzing fifteen container parsers from nothing is not justified when a BSD engine is in tree and can be fully contained. `archive_read_support_filter_program.c` and `archive_write_add_filter_program.c` (external program filters) are compiled out of the helper build.

### 6.4 Integration

- Intents handled: `open`, `view` (archive MIME types), `new` ("compress these"), custom actions `extract-here`, `extract-to`, `test`.
- Intents emitted: `open` and `view` (for entries), `share` (a created archive).
- MIME: `application/zip`, `application/x-7z-compressed`, `application/x-tar`, `application/gzip`, `application/x-bzip2`, `application/x-xz`, `application/zstd`, `application/x-lz4`, `application/vnd.rar`, `application/x-cpio`, `application/x-archive`, `application/x-iso9660-image`, `application/vnd.ms-cab-compressed`, `application/vnd.debian.binary-package`, `application/x-rpm`, `application/x-xar`, `application/x-lzh-compressed`, `application/x-age-archive` (Proposed name).
- Services: `svc.portal.file`, `svc.intent`, `svc.notify`, `svc.keyring`, `svc.thumbnail`, `svc.power`, `svc.undo-trash`, `svc.audit`, `svc.settings`, `svc.backup` (snapshot hint), `svc.search` (optional).
- Offers `org.pbsd.archives.v1` (F107). SY02 Files, CO01 Mail (attachments), IN03 Transfers and SY05 Software are expected callers. PLATFORM.md section 12's example manifest already shows an `archive-helper` under Files; one helper should be built here and shared.
- Links: SY02 (context actions, drag and drop, location provider), SE04 (passphrases, age recipients, signing keys), SE05 (owner of general file encryption), OF01 (text diff), DV04 (hex), SY05 (package install hand-off), AD05 (grants shown and revocable).

## 7. Security and privacy

Capabilities:

| Capability | Why |
|---|---|
| `fs.app` | Job journal, index cache, profiles, dictionaries |
| `fs.portal` | Read archives and write to destinations the user picked |
| `net.none` | No network use at all |
| `proc.helper:archive-helper` | Contain all archive parsing and writing |
| `proc.helper:crypto-helper` | Hold secrets away from the parser |
| `proc.helper:location-provider` | Serve an archive to SY02 read-only |
| `svc.portal.file`, `svc.intent`, `svc.notify`, `svc.settings` | Core operation |
| `svc.keyring` | Optional stored passphrases (own items only) |
| `svc.thumbnail` | Previews |
| `svc.power` | Sleep inhibitor during jobs |
| `svc.undo-trash` | Undo extract, trash of sources |
| `svc.audit` | Record limit overrides and refused entries |
| `svc.backup` | Snapshot hint before an in-place update |
| `svc.portal.print` (`dev.print`) | Print a listing |
| `bg.run` | Finish jobs after the window closes |

Not requested: any `net.*` other than `net.none`, `proc.exec`, `fs.path`, `ai.local`, devices.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| Files outside the destination | Crafted archive | `../`, absolute paths, symlink then write-through, hard link to a sensitive file | Own path validator; beneath-only directory handle; no following of links made in the job (F033–F035) |
| Disk space, memory, CPU | Decompression bomb, quine archive, deep nesting | Ratio, total, entry, depth and nesting limits; counting bytes as written (F038, F039, F007) |
| Process integrity | Malformed headers hitting a parser bug in C code | Single-use helper in capability mode with two or three descriptors; no network, no exec; crash fails only the job; fuzzing (section 11) |
| System integrity | Device nodes, setuid files, files with hostile xattrs or flags | Never created or stripped by default (F036, F037) |
| User judgement | Names with bidi overrides, control characters, double extensions, look-alike case clashes | Flagged and shown escaped; executable bit cleared for untrusted provenance (F011, F043, F048) |
| Passphrases | Other apps, logs, swap | Secret field, crypto-helper only, locked memory, never logged; keyring items scoped to this app |
| Terminal and UI | Escape sequences in names or comments | All archive strings are data: rendered as text with control characters shown as escapes |
| Other apps calling the bus interface | Confused-deputy extraction | Calls carry the caller's handles; the app adds none of its own |

Residual risk: a kernel or Capsicum flaw combined with a libarchive bug. Reduced by PaX/W^X on the helper and by keeping the helper's descriptor set minimal.

Stored: recent list (in the portal service), job reports (entry names and reasons), listing caches (entry names), profiles. "Private mode" setting stores none of these. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `create.default_format` | enum | `tar.zst` | Format offered first |
| `create.default_profile` | string | `Balanced` | Profile offered first |
| `create.test_after` | bool | true | Test each new archive |
| `create.excludes` | list<string> | `.DS_Store, Thumbs.db` | Default exclude globs |
| `extract.overwrite` | enum | `ask` | ask, skip, overwrite, newer, keep_both |
| `extract.links` | enum | `keep_safe` | skip, keep_safe, materialise |
| `extract.strip_setid` | bool | true | Strip setuid, setgid, sticky |
| `extract.preserve_mode` | bool | false | Keep stored modes (still masked) |
| `extract.clear_exec_untrusted` | bool | true | Clear exec bit for untrusted provenance |
| `extract.make_folder` | enum | `auto` | auto, always, never |
| `limits.max_total` | bytes | 100 GiB | Per-job output cap |
| `limits.max_entries` | u64 | 5 000 000 | Per-job entry cap |
| `limits.max_ratio` | u32 | 1000 | Per-entry ratio cap above 16 MiB |
| `nested.max_bytes` | bytes | 1 GiB | Largest nested archive opened in place |
| `preview.max_bytes` | bytes | 64 MiB | Largest entry previewed in memory |
| `jobs.parallel` | u8 | 1 | Jobs per destination device |
| `jobs.threads_on_battery` | u8 | 2 | Thread cap on battery |
| `view.default` | enum | `tree` | tree, flat, grid |
| `view.name_encoding_fallback` | string | `CP437` | For zips with no UTF-8 flag |
| `privacy.private_mode` | bool | false | Keep no recent list, reports or caches |
| `search.index_names` | bool | false | Offer entry names to `svc.search` |

## 9. Accessibility and languages

- Risk flags are words plus icons, never colour alone, and are part of each row's accessible name ("etc passwd, unsafe: path leaves the destination").
- The extraction report is a table with a caption and can be read row by row.
- Entry names in any script render with the FULL text profile (G-03). Names are shown in logical order with bidi control characters made visible.
- Byte sizes use binary units with the locale's number format; a setting switches to decimal units.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | 150 ms |
| First rows of a listing | 100 ms after open for zip (central directory), 300 ms for streamed formats |
| Listing 1 000 000 entries | Scroll at 60 fps; index build under 20 s on the reference machine |
| App host memory, idle with one archive of 100 000 entries | 60 MiB |
| Helper memory | Engine window plus 64 MiB; hard `rlimit` set from the method's stated needs |
| Extract throughput | Within 10% of `bsdtar` on the same archive and disk |
| Largest archive | No limit from the app; tested to 1 TiB and 4 GiB+ entries (zip64) |
| Idle | No wake-ups with no job |

## 11. Testing and proof

- **Unit**: path validator, risk flagging, overwrite logic, limits, sums-file parser, profile serialisation.
- **Golden**: `.fuseui` renders of the main window, extract sheet, create dialog, report, compare, in dark, light, high contrast, 200%.
- **Scripted UI**: T1–T13 through the a11y tree.
- **Hostile corpus**: a kept set of archives: traversal names, symlink chains, hard-link escapes, 42.zip-style bombs, overlapping zip entries, quines, case clashes, device nodes, 64 KiB names, zip/tar polyglots, truncated files, wrong sizes in headers. Each has an expected report.
- **Fuzz**: the helper's list and extract paths per format, with the destination on a memory file system; the path validator; the age reader; the UDF reader; the deb/rpm/pkg metadata readers.
- **Differential**: listings and extracted trees against `bsdtar`, `bsdcpio` and `bsdunzip` from the same tree (`hbsd/src/contrib/libarchive/tar`, `cpio`, `unzip`); reproducible mode checked by byte equality of two runs.
- **Sandbox test**: the helper is run with a test that tries to open a path, a socket and exec; all must fail (needs G-06).

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Path validator (C++23) | No accepted path resolves outside the root; no accepted link target leaves the root | T1 | PRISM proof over a model of path resolution | Model of `openat` semantics must be agreed |
| Limits accounting | Counters never overflow; job stops at or before each limit | T2 | PRISM absence-of-runtime-error | — |
| Helper wrapper (C++23) | Memory safety of own code | T2 | PRISM, sanitizers, fuzzing | — |
| libarchive (C) | None claimed | T4 | Fuzzing and sandbox only | Third-party C; not in the safe subset |
| age reader/writer | Format conformance; no secret in logs | T3 | Test vectors, fuzzing | Crypto primitives' tier comes from FORGE |
| UDF reader | Memory safety | T3 | Safe subset, fuzzing | Size of the format |
| App logic (ForgeScript) | Memory-safe by language | T3 | Unit and UI tests | Interpreter tier (FORGE) |

Nothing here has been run.

## 12. Build plan

Phase 1: usable core.

| ID | Work item | Depends on |
|---|---|---|
| UT01-W01 | Helper skeleton: start, capability mode, bus protocol, job spec | G-06, G-07, G-08, FORGE-WP-27, FORGE-WP-28 |
| UT01-W02 | Build libarchive from the base tree into the helper, program filters removed | UT01-W01 |
| UT01-W03 | Path validator and extraction policy engine | UT01-W01 |
| UT01-W04 | Listing with risk flags; tree and flat views | G-02, UT01-W02 |
| UT01-W05 | Extract all/selection, limits, overwrite, report | UT01-W03, G-09 (`svc.portal.file`) |
| UT01-W06 | Create: zip, tar + filters, 7z; test after create | UT01-W02 |
| UT01-W07 | Test, hashes, hash verify | UT01-W02 |
| UT01-W08 | Preview (text, image), open with | UT01-W04, `svc.thumbnail` |
| UT01-W09 | Job queue, notifications, background run | G-09 |
| UT01-W10 | SY02 intents: Extract Here, Compress… | SY02, SH04 |
| UT01-W11 | AES zip read; passphrase prompt | UT01-W02 |
| UT01-W12 | Hostile corpus and fuzz harnesses | UT01-W03, FORGE-WP-35 |

Phase 2: full feature.

| ID | Work item | Depends on |
|---|---|---|
| UT01-W13 | Update in place, add/remove/rename, convert | UT01-W06 |
| UT01-W14 | crypto-helper; age archives; AES zip write; keyring | FORGE-WP-22, SE04 |
| UT01-W15 | 7z AES layer (`UT01-GAP-1`) | UT01-W14 |
| UT01-W16 | Split and join volumes | UT01-W06 |
| UT01-W17 | Compare two archives; export listing | UT01-W07 |
| UT01-W18 | Package and disc views (deb, rpm, pkg, ISO); UDF reader | UT01-W04 |
| UT01-W19 | Batch jobs, profiles, reproducible mode | UT01-W09 |
| UT01-W20 | Provenance tags, undo extract, job recovery | SY02, `svc.undo-trash` |
| UT01-W21 | Bus interface `org.pbsd.archives.v1` | G-08 |
| UT01-W22 | Name encodings, nested archives, large-archive index | UT01-W04 |

Phase 3: advanced.

| ID | Work item | Depends on |
|---|---|---|
| UT01-W23 | Location provider for Files | SY02 location-provider interface |
| UT01-W24 | Disposable sandbox extract | SY02, AD05 |
| UT01-W25 | Repair, parity, signatures | SE04 |
| UT01-W26 | Zstandard dictionaries | UT01-W06 |
| UT01-W27 | User job recipes | FORGE-WP-10.6 |
| UT01-W28 | Compare with folder, entry diff | OF01, DV04 |

## 13. Open questions and risks

- libarchive is C. Is a contained C engine acceptable for a P1 app, or must the most common formats (zip, tar, gzip, zstd framing) get own C++23 readers first, with libarchive kept for the long tail? Recommendation: ship with libarchive contained, then replace zip and tar readers with proved own code.
- 7z AES and RAR5 encryption are not in the engine. 7z AES is planned as own code; RAR5 encryption is left unsupported.
- lz4 and the zstd dictionary builder in the base build are Unverified; if absent they must be vendored (both BSD 2-clause).
- The location-provider interface in SY02 is assumed, not read. If SY02 has none, F120 moves to a FUSE-file-system-free design or is dropped.
- Exact behaviour of the libarchive secure flags was read from the header only; behaviour is to be confirmed by the hostile corpus.
- Who owns the shared `archive-helper` binary: this bundle or a platform package used by Files, Mail and Software? Proposed: a platform package, built from this spec.
- Provenance tags depend on a system-wide tag format not yet specified (SY02, IN03).
