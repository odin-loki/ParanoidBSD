<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Files

By Odin Loch

| | |
|---|---|
| ID | `SY02` (from CATALOGUE.md) |
| Bundle | `org.pbsd.files` |
| Category | system |
| Priority | P0 |
| Backing repos | FUSE (item views, `fileDialog`), FORGE (ForgeScript, OS layer), ParanoidBSD (`docs/specs/HANDLES.md`, `pbsd/zfs`) |
| Status | Planned. Nothing built. |

## 1. Purpose

Files is the file manager. People use it to browse, find, preview, organise, move and share files on local disks, removable media and remote servers. On this system it also shows what no ordinary file manager can: where a file came from, which apps may reach a folder, what a folder looked like in any ZFS snapshot, and it can open anything untrusted in a disposable sandbox. It replaces Dolphin from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- Browsing with five views, tabs, dual pane, split, tree sidebar and preview pane.
- File operations with a journal, undo, conflict handling and a queue.
- Search by name, content, metadata and tags through `svc.search`, plus live folder filtering.
- Tags, stars, notes, bulk rename, checksums, signature verification.
- ZFS snapshot time-travel for any folder or file.
- Remote locations: SFTP, WebDAV, SMB, NFS.
- Provenance, capabilities per folder, ACLs, extended attributes.
- ForgeScript actions.

Out of scope:

| Item | Owner |
|---|---|
| Creating, browsing inside and extracting archives (Files shows them as files and hands off) | UT01 Archives |
| Partitioning, pools, datasets, mounting encrypted disks | AD02 Disks |
| Snapshot schedules, send and receive, off-site backup | AD03 Backup |
| Encrypted containers, file encryption, secure erase, metadata cleaning | SE05 Vault |
| The grant store, lineage tree, global revocation | AD05 Permissions |
| Space treemap, duplicate finder | UT04 Disk Usage |
| Queued downloads, BitTorrent, rsync jobs | IN03 Transfers |
| The file chooser dialog used by other apps | SH04 Portals (shares the view components of this app) |
| Editing file contents | the app for that type (OF01, ME01, …) |
| SSH key management | SE04 Passwords and Keys |
| Photo library features | ME01 Photos |

## 3. Users and top tasks

Personas:

- **Everyday user**: finds a document, moves photos off a camera, empties the trash.
- **Power user**: dual pane, keyboard only, bulk rename, scripts.
- **Administrator**: ACLs, ownership, remote servers, snapshots.
- **Cautious user**: checks where a download came from, verifies its signature, opens it in a sandbox.

| # | Task | Target steps |
|---|---|---|
| T1 | Open a file in the Documents folder | 2 |
| T2 | Find a file by part of its name anywhere | 2 (`Ctrl+F`, type) |
| T3 | Copy selected files to the other pane | 1 (`Alt+F5`) |
| T4 | Undo an accidental move | 1 (`Ctrl+Z`) |
| T5 | Restore yesterday's version of a file | 3 (select, `Alt+T`, Restore) |
| T6 | Rename 200 photos by date | 4 |
| T7 | Connect to an SFTP server | 3 |
| T8 | Verify a download against a published SHA-256 | 3 |
| T9 | Open an untrusted PDF in a disposable sandbox | 2 (context menu, click) |
| T10 | See which apps can read a folder and revoke one | 3 |
| T11 | Tag files and list everything with that tag | 2 + 1 |
| T12 | Open a terminal in the current folder | 1 (`Ctrl+Alt+T`) |
| T13 | Recover a file from the trash | 2 |
| T14 | Compare two folders | 3 |

## 4. Features

### 4.1 Views and navigation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY02-F001 | Icon view | Grid of thumbnails, 32–512 px, zoom with `Ctrl+wheel`; labels 1–3 lines. | M | 1 |
| SY02-F002 | List (details) view | Columns: name, size, type, modified, created, accessed, owner, group, mode, tags, origin, trust, dataset, path. Column chooser, reorder, sort by several columns. | M | 1 |
| SY02-F003 | Column (Miller) view | One column per level; last column is a preview. | S | 2 |
| SY02-F004 | Tree view | Expand folders in place in the list view. | M | 1 |
| SY02-F005 | Gallery view | Large preview on top, film strip below; arrow keys step. | S | 2 |
| SY02-F006 | Compact view | Dense multi-column name list. | C | 2 |
| SY02-F007 | Per-folder view memory | View, sort, columns and zoom kept per folder, per folder type template, or globally. | M | 1 |
| SY02-F008 | Tabs | New, close, reopen closed (`Ctrl+Shift+T`), duplicate, pin, drag between windows, drop files on a tab. | M | 1 |
| SY02-F009 | Dual pane | Two panes side by side or stacked (`F3`); active pane marked; copy and move to the other pane; swap and equalise. | M | 1 |
| SY02-F010 | Path bar | Breadcrumb with drop-down siblings per segment; click blank area or `Ctrl+L` to type; completion; accepts URLs. | M | 1 |
| SY02-F011 | History | Back, forward, up, per tab, with a history menu. | M | 1 |
| SY02-F012 | Sidebar places | Home, Desktop, Documents, Downloads, Pictures, Music, Videos, Recent, Starred, Trash, bookmarks, devices, remote, tags, snapshots. Reorder, hide, add by drag. | M | 1 |
| SY02-F013 | Sidebar folder tree | Optional tree under the places list. | S | 1 |
| SY02-F014 | Devices | Mounted and unmounted volumes with free-space bars, eject, safely remove, unlock (hands to AD02 for GELI). | M | 1 |
| SY02-F015 | Recent | Files opened through the portal recently; can be turned off (SY01). | S | 1 |
| SY02-F016 | Type-ahead and filter | Typing jumps to a name; `Ctrl+I` or `/` filters the listing live (substring, glob, regex). | M | 1 |
| SY02-F017 | Sorting and grouping | Sort by any column, folders first, natural number order, locale collation; group by type, date, size class, tag, origin. | M | 1 |
| SY02-F018 | Hidden and system files | Toggle `Ctrl+H`; honours `.hidden`; backup files rule. | M | 1 |
| SY02-F019 | Selection tools | Rubber band, Shift and Ctrl, invert, select by pattern, selection survives sort; selection count and size in the status bar. | M | 1 |
| SY02-F020 | Go to | `Ctrl+G` fuzzy jump over bookmarks, recent folders and indexed folder names. | S | 2 |
| SY02-F021 | Large folders | A folder with 1,000,000 entries lists progressively and stays responsive. | M | 1 |
| SY02-F022 | Folder sizes | Optional recursive size column, computed in the background, cached per dataset transaction group. | S | 2 |
| SY02-F023 | Symbolic and hard links | Emblem on links; target shown; broken links marked; hard-link count column. | M | 1 |
| SY02-F024 | Compare folders | Side-by-side diff of two folders by name, size, time or digest; sync actions. | C | 3 |

### 4.2 File operations

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY02-F025 | Copy, move, link | Copy, move, symbolic link, hard link; ZFS block cloning for same-pool copies where available (instant). | M | 1 |
| SY02-F026 | Operation queue | Jobs run in the file operations service; per-device serial queues; pause, resume, cancel, reorder; survive closing the window. | M | 1 |
| SY02-F027 | Conflict dialog | Replace, skip, keep both, merge folders, compare (size, time, digest, thumbnail); apply to all; rules ("replace if newer"). | M | 1 |
| SY02-F028 | Verify after copy | Optional BLAKE3 compare of source and destination. Default on for removable and remote targets. | M | 2 |
| SY02-F029 | Undo journal | Every operation is recorded in `svc.undo-trash`; `Ctrl+Z` reverses copy, move, rename, trash, tag, attribute and permission changes, across restarts. | M | 1 |
| SY02-F030 | Journal viewer | List of past operations with status; undo any reversible entry, not only the last. | S | 2 |
| SY02-F031 | Trash | Per-dataset trash; restore to origin; shows deleted date and origin; auto-empty after N days; trash on removable media. | M | 1 |
| SY02-F032 | Delete for ever | `Shift+Delete` with a confirm that names the count; states plainly that snapshots may still hold the data. | M | 1 |
| SY02-F033 | Secure erase hand-off | "Erase securely…" opens SE05, which explains copy-on-write limits. | S | 2 |
| SY02-F034 | New items | New folder, new file from a template (`svc.intent` `new`), new folder from selection. | M | 1 |
| SY02-F035 | Rename | Inline `F2`; selects the stem, not the extension; `Tab` renames the next file; warns on extension change and on names that are confusable or contain control or bidirectional override characters. | M | 1 |
| SY02-F036 | Bulk rename | Rule stack: find and replace (text or regex), case, insert, remove, numbering, date from metadata, extension, transliterate; live preview with conflict marks; undoable; save rule sets. | M | 2 |
| SY02-F037 | Duplicate | `Ctrl+D` makes "name (copy)". | S | 1 |
| SY02-F038 | Paste special | Paste clipboard image or text as a new file; paste as link. | S | 2 |
| SY02-F039 | Resume and retry | Interrupted copies resume from the last verified block; transient remote errors retry with back-off. | S | 2 |
| SY02-F040 | Sparse, attributes and times | Preserve sparse regions, extended attributes, ACLs, times and flags on copy where the target supports them; report what was lost. | M | 2 |
| SY02-F041 | Open and open with | Default app through `svc.intent`; open-with list; set default per type; open in new tab or window. | M | 1 |
| SY02-F042 | Executable files | Double click never runs a file. A file with the execute bit offers "Run in Terminal…" and "Run in a sandbox…" after a prompt that shows origin and signature. | M | 1 |
| SY02-F043 | Send to and share | `share` intent: Mail, Messenger, Transfers, nearby device, Bluetooth. | S | 2 |
| SY02-F044 | Archives hand-off | Compress…, Extract here, Extract to… call UT01; progress appears in the same queue. | M | 1 |
| SY02-F045 | Split and join | Split a file into parts and join them, with digests. | C | 3 |

### 4.3 Search, tags and metadata

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY02-F046 | Indexed search | `svc.search` query by name, content, type, date, size, tag, origin, owner. Scope: this folder, home, everywhere indexed. | M | 1 |
| SY02-F047 | Query syntax | `name:report type:pdf modified:>2026-10 size:>5M tag:tax origin:web has:signature`; a filter bar builds the same query with chips. | M | 2 |
| SY02-F048 | Unindexed search | Walks folders not in the index (remote, removable) with a progress line; content search optional. | M | 2 |
| SY02-F049 | Saved searches | Save a query as a virtual folder in the sidebar. | S | 2 |
| SY02-F050 | Regex and content search | Regex on names; content search with line preview and hit count for text types. | S | 2 |
| SY02-F051 | Tags | Free-text tags with colours, stored in the extended attribute `user.pbsd.tags` and mirrored in the index; tag browser in the sidebar; tag by drag onto a tag. | M | 2 |
| SY02-F052 | Stars and notes | Star a file; a short note stored in `user.pbsd.note`. | S | 2 |
| SY02-F053 | Emblems | Small marks for link, shared, read-only, encrypted, tagged, untrusted origin, signature state, snapshot-only. | M | 1 |
| SY02-F054 | Properties inspector | `F4`: general, permissions, provenance, capabilities, metadata (EXIF, audio tags, document info via the thumbnail sandbox), checksums, versions. Multi-selection shows totals and common values. | M | 1 |
| SY02-F055 | Local semantic search | Optional: find by description ("the invoice from the plumber") using `svc.model` embeddings computed locally by the index. Off by default. | C | 3 |

### 4.4 Preview and thumbnails

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY02-F056 | Thumbnails | From `svc.thumbnail`, each generated in a sandbox per MIME type; cached by content digest; off for remote and for untrusted-origin files unless allowed. | M | 1 |
| SY02-F057 | Preview pane | `F7` toggles a right pane: image, text with highlighting, PDF first pages, audio and video with scrub, fonts, Markdown, hex for unknown. | M | 1 |
| SY02-F058 | Quick look | `Space` opens a borderless preview; arrows step through the folder; `Enter` opens the app. | M | 2 |
| SY02-F059 | Preview trust rule | Previews of untrusted-origin files are rendered only by sandboxed thumbnailers and never fetch anything. | M | 1 |
| SY02-F060 | Text peek | First 64 KiB of text files with encoding detection. | S | 1 |

### 4.5 ZFS snapshots and versions

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY02-F061 | Time-travel slider | `Alt+T` shows a timeline of snapshots of the current dataset; moving it shows the folder as it was, read-only, with a coloured banner. | M | 2 |
| SY02-F062 | Versions of a file | Inspector tab listing each snapshot where the file differs (by size, time, block birth); preview, restore, restore a copy, compare. | M | 2 |
| SY02-F063 | Diff against a snapshot | Marks added, removed and changed entries between now and a snapshot (from `zfs diff` data given by the snapshot service). | S | 2 |
| SY02-F064 | Restore | Copy items out of a snapshot into the live folder; conflict dialog applies; undoable. | M | 2 |
| SY02-F065 | Snapshot now | "Snapshot this folder's dataset" with a name; needs the snapshot right; hands scheduling to AD03. | S | 2 |
| SY02-F066 | Deleted files finder | Lists files that exist in snapshots but not now, for the current folder. | S | 3 |
| SY02-F067 | Dataset information | Inspector shows dataset, used, quota, compression ratio, encryption state, record size. Changing these belongs to AD02. | S | 2 |
| SY02-F068 | Clone browse | Open a snapshot as a writable clone in a scratch location (hands to AD02 for the clone). | C | 3 |

### 4.6 Capabilities, provenance and trust

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY02-F069 | Capabilities per folder | Inspector tab: apps holding a grant on this folder or a parent, with rights (read, write), since when, how granted (picker, remembered, manifest path), last use. Revoke in place; "Open in Permissions" for the lineage tree. | M | 2 |
| SY02-F070 | Grant from Files | "Allow an app to use this folder…" creates a remembered portal grant; the shell draws the confirmation. | S | 2 |
| SY02-F071 | Provenance record | Each file may carry `user.pbsd.origin`: source kind (web, mail, chat, removable, network share, created here, extracted), source URL or device id, referrer, the app that wrote it, time, and the digest at arrival. Written by the portal when an app saves. | M | 2 |
| SY02-F072 | Provenance display | Origin column, emblem, inspector tab with the full chain (downloaded → extracted → copied). | M | 2 |
| SY02-F073 | Provenance propagation | Copy, move, extract and rename carry the record forward and append a step. | M | 2 |
| SY02-F074 | Untrusted mark | Files from web, mail, chat, removable media and remote shares are "untrusted" until the user clears the mark. Opening one offers the sandbox first. | M | 2 |
| SY02-F075 | Open in disposable sandbox | Opens the file with its app in a fresh domain: read-only handle to that file only, `net.none`, throw-away data directory, destroyed on close. For types with no safe app, opens a disposable VM through VM01. | M | 2 |
| SY02-F076 | Checksums | BLAKE3, SHA-256, SHA-512, SHA-1 and MD5 (legacy, labelled weak); compute for selection; copy; compare with pasted value. | M | 1 |
| SY02-F077 | Checksum files | Verify `SHA256SUMS`-style and `.b3`/`.sha256` files against a folder; create them. | S | 2 |
| SY02-F078 | Signature verify | Detached Ed25519 `.sig` (as V14), minisign, signify, OpenPGP `.asc`/`.sig`; keys come from SE04; result shown as verified by whom, unknown key, or bad. | M | 2 |
| SY02-F079 | Trust column | Combined state: signed and verified, digest matches origin, unverified, tampered (digest differs from the arrival digest). | S | 2 |
| SY02-F080 | Sign and encrypt hand-off | "Sign…", "Encrypt…" open SE05 with the selection. | S | 2 |
| SY02-F081 | Metadata clean hand-off | "Remove metadata…" opens SE05. | C | 3 |
| SY02-F082 | Activity for a file | From `svc.audit`: which apps opened or changed this file (when audit of file access is on). | C | 3 |

### 4.7 Permissions, ACLs and attributes

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY02-F083 | POSIX mode | Owner, group, mode as check boxes and octal; apply to enclosed items with a preview count. | M | 1 |
| SY02-F084 | NFSv4 ACL editor | ZFS NFSv4 ACL entries: principal, allow or deny, 14 permission bits, inheritance flags; effective-access calculator for a chosen user. | S | 2 |
| SY02-F085 | File flags | `uchg`, `schg`, `uappnd`, `sappnd`, `nodump`, `hidden` shown; change where permitted. | S | 2 |
| SY02-F086 | Extended attributes | Table of namespace, name, size, value (text or hex); add, edit, delete in the `user` namespace; `system` read-only. | S | 2 |
| SY02-F087 | Ownership change | Through the configuration broker with authentication when the user does not own the file. | S | 2 |
| SY02-F088 | Admin mode off by design | Files never runs as root. Privileged actions are single brokered requests, each authenticated and audited. | M | 1 |

### 4.8 Remote locations

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY02-F089 | Connect to server | Dialog: protocol, host, port, path, user, authentication; saved as a bookmark; secrets in `svc.keyring`. | M | 2 |
| SY02-F090 | SFTP | SSH v2; key, agent (SE04) or password; host key shown on first contact and pinned; resume; symbolic links. | M | 2 |
| SY02-F091 | WebDAV | HTTPS only by default; TLS certificate shown; locks; quota. | S | 2 |
| SY02-F092 | SMB | SMB 3 with signing and encryption required by default; SMB 1 refused; share browse by host only. | S | 3 |
| SY02-F093 | NFS | NFSv4.1+ mounts through the mount broker; Kerberos optional; never auto-mounts. | S | 3 |
| SY02-F094 | Network badge | `[net]` badge lists each endpoint in use; remote tabs carry the `--net` colour. | M | 2 |
| SY02-F095 | Route choice | Per bookmark: clear, VPN or anonymous route (SE03). | S | 3 |
| SY02-F096 | Offline behaviour | Remote tabs show "disconnected" with Reconnect; queued operations pause, not fail. | M | 2 |
| SY02-F097 | Remote thumbnails and previews | Off by default; opt in per bookmark. | S | 2 |
| SY02-F098 | Local discovery | Off by default. When on, lists services found by mDNS, per network. | C | 3 |

### 4.9 Automation and integration

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY02-F099 | Terminal here | `Ctrl+Alt+T` opens SY03 in the current folder with a handle to that folder only. | M | 1 |
| SY02-F100 | Embedded terminal panel | `F12` shows a terminal panel under the listing that follows the folder (uses the widget of G-13). | C | 3 |
| SY02-F101 | ForgeScript actions | Signed `.fs` actions in `~/.config/org.pbsd.files/actions/` appear in the context menu for matching types. Each declares the capabilities it needs; it runs in its own engine with handles to the selection only. | S | 2 |
| SY02-F102 | Action editor | List, enable, test on a sample, view declared capabilities; "Edit" opens OF01. | S | 3 |
| SY02-F103 | Folder rules | Per folder: "when a file arrives, run action X" (for example sort Downloads by type). Runs through a background agent only if `bg.run` is granted. | C | 3 |
| SY02-F104 | Templates | New-file templates from `~/Templates` and from apps' `new` intents. | S | 2 |
| SY02-F105 | Desktop integration | SH01 uses the same views for the desktop surface; SH04 uses them for the file chooser. | M | 1 |
| SY02-F106 | Command palette | Every action, bookmark and recent folder. | M | 1 |
| SY02-F107 | Copy path and copy as | Copy path, name, URL, or a handle reference for scripts. | S | 1 |
| SY02-F108 | Print listing | Print or export the current listing as text, CSV or PDF. | C | 3 |
| SY02-F109 | Disk usage hand-off | "Analyse space" opens UT04 at the folder. | S | 2 |
| SY02-F110 | Write image hand-off | An `.iso`/`.img` offers "Write to device…" in AD02. | C | 3 |

### 4.10 Safety, recovery and housekeeping

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY02-F111 | Session restore | Windows, tabs, panes, selection and scroll restored after crash or login. | M | 1 |
| SY02-F112 | Removable media policy | Nothing runs or opens on insert; a toast offers "Open". Files from it are marked untrusted. | M | 1 |
| SY02-F113 | Name safety | Shows hidden extensions always for untrusted files; marks double extensions and right-to-left override tricks. | M | 1 |
| SY02-F114 | Free-space guard | Warns before an operation that cannot fit; accounts for snapshots and quotas. | M | 1 |
| SY02-F115 | Read-only places | Snapshots, read-only mounts and verified system datasets are shown with a banner; write actions are disabled with the reason. | M | 1 |
| SY02-F116 | Slow-device care | Listing and thumbnails never block the UI; a spinner per pane; cancel with `Esc`. | M | 1 |
| SY02-F117 | Audit | Privileged requests, grants made from Files, sandbox opens and signature failures go to `svc.audit`. | M | 2 |
| SY02-F118 | Preferences | See section 8. | M | 1 |

## 5. GUI

### 5.1 Window map

```
Files window (many windows, one app instance)
├─ Header bar: menu, back/forward/up, path bar, search, [net] badge, window buttons
├─ Tab bar (tabWidget)
├─ Sidebar: Places · Devices · Remote · Tags · Saved searches · (folder tree)
├─ Toolbar: new, view switch, sort, group, filter chips, zoom slider, pane toggle
├─ Content: one or two panes, each: banner (snapshot / read-only / untrusted / disconnected)
│           + view (icon / list / columns / tree / gallery / compact) + filter bar
├─ Time-travel bar (snapshot timeline)
├─ Preview pane · Inspector (tabs: General, Permissions, Provenance, Capabilities, Metadata,
│                            Checksums, Versions, Attributes)
├─ Terminal panel (optional)
├─ Status bar: selection, free space, jobs, trust
├─ Dialogs: Conflict · Bulk rename · Connect to server · Host key · Checksums and verify
│           · Delete for ever · ACL editor · Open in sandbox · Select by pattern
│           · Compare folders · Empty trash · Run file prompt · New from template
├─ Popovers: Jobs queue · Path segment siblings · Tag picker · Column chooser · View options
├─ Quick look window
├─ Journal viewer window
└─ Preferences window
```

### 5.2 Main window

Shown at 1280×760, list view, inspector open on Provenance.

```
+------------------------------------------------------------------------------------------+
| [=] [<][>][^]  Home › Downloads ›                     [ Search…  Ctrl+F ]   [▁] [▢] [✕]   | (1)
+------------------------------------------------------------------------------------------+
| [Downloads ×] [Projects ×] [sftp: build01 ×]  [+]                                         | (2)
+--------------+--------------------------------------------------+------------------------+
| PLACES       | [New v] [⊞ ≡ ⫴ ▤ ▣]  Sort [Modified v]  [Filter]  | invoice-0412.pdf       | (3)
| > Home       +--------------------------------------------------+ [General][Perm][Prov.] |
|   Documents  | Name                  | Size   | Modified | Origin| [Caps][Meta][Sums][Ver]|
|   Downloads  | > invoice-0412.pdf  ⚠ | 212 kB | 10:42    | web   |------------------------|
|   Pictures   |   pbsd-0.1.img.xz   ✓ | 1.4 GB | 09:15    | web   | Origin   web download  | (5)
|   Recent     |   notes.md            | 3 kB   | Tue      | here  | From     https://exam… |
|   Starred    |   photos/             | 48 items| Mon     | usb   | Via      Web Browser   |
|   Trash      |   setup.sh          ⚠ | 1 kB   | Mon      | mail  | Arrived  12 Nov 10:42  |
| DEVICES      |                                                  | Digest   b3:9f2c…41 ✓  |
|   System 61% |                                                  | Trust    Untrusted ⚠   |
|   USB stick ⏏|                                                  | Chain    1 step        |
| REMOTE       |                                                  |                        |
|   build01    |                                                  | [Open in sandbox]      |
| TAGS         |                                                  | [Verify signature…]    |
|   ● tax      |                                                  | [Mark as trusted]      |
|   ● work     +--------------------------------------------------+                        |
|              | ◷ Now ─────●────────●───●──●─●●●●  [Snapshots v]  |                        | (6)
+--------------+--------------------------------------------------+------------------------+
| 1 of 5 selected · 212 kB        Free 412 GB        ⟳ 1 job  43%           Unverified ⚠   | (7)
+------------------------------------------------------------------------------------------+
```

1. Header bar with navigation buttons and the breadcrumb path bar. Each segment is a drop target and opens a sibling list.
2. Tab bar (`tabWidget`). Remote tabs are tinted `--net`. Dragging a tab to the window edge makes a second pane.
3. Toolbar: New menu, view switch (icons, list, columns, gallery, compact), sort, filter chips.
4. Content pane: `tableView` in list view (tree mode when expanded), `gridView` in icon view. Emblems: `⚠` untrusted origin, `✓` signature or digest verified.
5. Inspector, 280 px, `tabWidget`. Provenance tab shows the origin record and the three trust actions.
6. Time-travel bar (hidden until `Alt+T`): each mark is a snapshot; drag or arrow keys move through them.
7. Status bar: selection summary, free space of the current dataset, job indicator (opens the jobs popover), trust state of the selection.

States: empty folder ("This folder is empty" + [New folder]), loading (progressive rows and a pane spinner), error (banner with reason and Retry), offline (remote tab banner), read-only (banner, write actions disabled), snapshot (amber banner "Viewing 11 Nov 2026 03:00 — read-only [Back to now]").

### 5.3 Other views and dialogs

#### 5.3.1 Dual pane

```
+------------------------------------------------------------------------------------------+
| [=] [<][>][^]  Home › Projects                        [ Search… ]           [▁] [▢] [✕]   |
+--------------------------------------------+---------------------------------------------+
| Home › Projects                    (active)| sftp://build01/srv/out                [net] |
| Name              | Size   | Modified      | Name              | Size   | Modified       |
| > src/            | —      | 10:02         |   release/        | —      | yesterday      |
|   build.fs        | 4 kB   | 09:58         |   log.txt         | 88 kB  | 10:01          |
|   README.md       | 2 kB   | Mon           |                                             |
+--------------------------------------------+---------------------------------------------+
| Alt+F5 Copy → · Alt+F6 Move → · F2 Rename · F8 Trash · Tab switch pane · Ctrl+U swap     |
+------------------------------------------------------------------------------------------+
```

`splitter` with two pane components. The active pane has an `--accent` top line. The key hint row can be hidden.

#### 5.3.2 Conflict dialog

```
+------------------------ 3 files already exist in "Reports" ------------------------+
|                 In destination                 Incoming                            |
| report.pdf      1.2 MB · 10 Nov 14:02          1.3 MB · 12 Nov 09:40  newer        |
|                 [thumbnail]                    [thumbnail]             different   |
| For this file:  ( ) Replace  ( ) Skip  (•) Keep both ("report (2).pdf") [Compare…] |
| [x] Do the same for the remaining 2 conflicts                                      |
| Rule: [Replace only if newer v]                                                    |
|                                              [Cancel all]  [Skip]  [Continue]      |
+------------------------------------------------------------------------------------+
```

"different" or "identical" comes from a digest compare started when the dialog opens.

#### 5.3.3 Bulk rename

```
+------------------------------- Rename 212 items --------------------------------+
| Rules                                    | Preview                              |
| 1 [Date from metadata v] [yyyy-MM-dd  ]  | IMG_0412.JPG → 2026-10-03_001.jpg    |
| 2 [Insert text       v] ["_" at end   ]  | IMG_0413.JPG → 2026-10-03_002.jpg    |
| 3 [Numbering         v] [start 1 pad 3]  | IMG_0414.JPG → 2026-10-04_001.jpg    |
| 4 [Extension         v] [lower case   ]  | DSC_9.JPG    → 2026-10-04_001.jpg ✕  |
| [+ Add rule]  [Load set v] [Save set…]   |   1 conflict · 0 unchanged           |
|-----------------------------------------------------------------------------------|
| [ ] Regular expressions   Number per [folder v]         [Cancel]  [Rename 211]   |
+-----------------------------------------------------------------------------------+
```

Preview is a virtualised `tableView`; conflicts block only the rows involved. The whole rename is one journal entry.

#### 5.3.4 Time-travel view

```
+------------------------------------------------------------------------------------------+
| ⏱ Viewing "Documents" as of Tue 10 Nov 2026 03:00 (snapshot auto-daily) · read-only      |
|                                              [Compare with now] [Restore…] [Back to now] |
+------------------------------------------------------------------------------------------+
| Name                 | Size    | Modified     | Compared with now                        |
|   budget.ods         | 48 kB   | 9 Nov        | changed since                            |
| > thesis-draft.odt   | 2.1 MB  | 9 Nov        | deleted since                            |
|   notes/             | 12 items| 8 Nov        | same                                     |
+------------------------------------------------------------------------------------------+
| ◷ Oct ──●──●──●──●──[●]──●──●──●─ Now      hourly ● daily ● manual ◆     ← → step        |
+------------------------------------------------------------------------------------------+
```

The timeline is a `timeline` widget with one track per snapshot class. Restore copies the selection to the live folder through the normal queue.

#### 5.3.5 Connect to server

```
+------------------------------ Connect to server ------------------------------+
| Protocol   [SFTP (SSH)  v]        Name  [build01            ]                 |
| Host       [build01.lan           ]  Port [22  ]                              |
| Folder     [/srv/out              ]                                           |
| User       [odin        ]                                                     |
| Sign in    (•) Key from Passwords and Keys [ed25519 laptop v]                 |
|            ( ) Password  [________]  [ ] remember in keyring                  |
| Route      [Clear v]      [ ] Thumbnails and previews on this server          |
| This connects to build01.lan:22 only.                 [Cancel]  [Connect]     |
+-------------------------------------------------------------------------------+
```

Then, on first contact, the host-key dialog shows the key type and fingerprint (SHA-256 and a word list), with "Trust and pin" or "Cancel". A changed key is a `--danger` dialog that cannot be accepted with one click.

#### 5.3.6 Checksums and verify

```
+------------------------- Verify "pbsd-0.1.img.xz" --------------------------+
| BLAKE3    9f2c 81aa … 41d0                               [Copy]             |
| SHA-256   3b7e 02c4 … 7f19                               [Copy]             |
| SHA-512   [Compute]                                                         |
| Compare with  [3b7e02c4…7f19                    ]  ✓ matches SHA-256        |
|---------------------------------------------------------------------------- |
| Signature   pbsd-0.1.img.xz.sig  (Ed25519)                                  |
|             ✓ Good signature by "PBSD release key 2026" (trusted in Keys)   |
| Arrival digest   ✓ unchanged since download                                 |
|                                   [Save checksum file…]  [Close]            |
+-----------------------------------------------------------------------------+
```

#### 5.3.7 Capabilities tab of the inspector

```
+------------------ Documents — Capabilities ------------------+
| Apps that can reach this folder                              |
| App          | Rights | Granted          | Last used |       |
| Writer       | r w    | picker, remembered| today    | [Revoke]|
| Backup       | r      | manifest path    | 03:00     |  🔒    |
| Text Editor  | r w    | this session     | now  ●    | [Revoke]|
| Inherited from: Home (1 more app)             [Show]         |
| [Allow an app…]                      [Open in Permissions]   |
+--------------------------------------------------------------+
```

A lock marks grants fixed by the app's manifest (`fs.path`), which only AD05 or removal of the app can change.

#### 5.3.8 Open in sandbox prompt

```
+---------------- Open "invoice-0412.pdf" in a disposable sandbox? ----------------+
| Opens with Document Viewer in a fresh sandbox:                                    |
|   • this file only, read-only        • no network        • nothing is kept        |
| Origin: web download from https://example.org/…   Signature: none                 |
| [x] Always do this for untrusted PDF files                                        |
|                                     [Cancel]  [Open normally]  [Open in sandbox]  |
+-----------------------------------------------------------------------------------+
```

### 5.4 Menus

```
[=] menu / F10
├─ File
│   ├─ New window                        Ctrl+N
│   ├─ New tab                           Ctrl+T
│   ├─ New folder                        Ctrl+Shift+N
│   ├─ New file ▸ (templates)
│   ├─ Open                              Enter
│   ├─ Open with ▸
│   ├─ Open in disposable sandbox        Ctrl+Shift+Enter
│   ├─ Open in new tab                   Ctrl+Enter
│   ├─ Connect to server…                Ctrl+Shift+K
│   ├─ Properties                        Alt+Enter
│   ├─ Close tab                         Ctrl+W
│   └─ Quit                              Ctrl+Q
├─ Edit
│   ├─ Undo / Redo                       Ctrl+Z / Ctrl+Shift+Z
│   ├─ Cut / Copy / Paste                Ctrl+X / Ctrl+C / Ctrl+V
│   ├─ Paste as link                     Ctrl+Shift+V
│   ├─ Duplicate                         Ctrl+D
│   ├─ Rename                            F2
│   ├─ Bulk rename…                      Ctrl+F2
│   ├─ Move to trash                     Delete
│   ├─ Delete for ever…                  Shift+Delete
│   ├─ Select all / Invert               Ctrl+A / Ctrl+Shift+I
│   ├─ Select by pattern…                Ctrl+Shift+A
│   ├─ Copy path                         Ctrl+Shift+C
│   └─ Preferences                       Ctrl+,
├─ View
│   ├─ Icons / List / Columns / Gallery / Compact     Ctrl+1 … Ctrl+5
│   ├─ Expand as tree                    Ctrl+Shift+E
│   ├─ Dual pane                         F3
│   ├─ Preview pane                      F7
│   ├─ Inspector                         F4
│   ├─ Sidebar                           F9
│   ├─ Terminal panel                    F12
│   ├─ Hidden files                      Ctrl+H
│   ├─ Filter                            Ctrl+I
│   ├─ Sort by ▸ · Group by ▸ · Columns…
│   ├─ Zoom in / out / reset             Ctrl++ / Ctrl+- / Ctrl+0
│   └─ Refresh                           F5
├─ Go
│   ├─ Back / Forward / Up               Alt+Left / Alt+Right / Alt+Up
│   ├─ Home                              Alt+Home
│   ├─ Location…                         Ctrl+L
│   ├─ Go to…                            Ctrl+G
│   ├─ Time travel                       Alt+T
│   ├─ Trash · Recent · Starred
│   └─ Bookmarks ▸  (Add bookmark  Ctrl+B)
├─ Tools
│   ├─ Search                            Ctrl+F
│   ├─ Checksums and verify…             Ctrl+Shift+H
│   ├─ Compare folders…
│   ├─ Compress… / Extract here / Extract to…
│   ├─ Sign… / Encrypt… / Erase securely…
│   ├─ Open terminal here                Ctrl+Alt+T
│   ├─ Analyse space
│   ├─ Snapshot this dataset…
│   ├─ Operation journal…                Ctrl+J
│   └─ Actions ▸ (ForgeScript actions) · Manage actions…
└─ Help
    ├─ Files help                        F1
    ├─ Keyboard shortcuts                Ctrl+/
    └─ About Files
```

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| `Space` | Quick look |
| `Tab` (dual pane) | Switch pane |
| `Alt+F5` / `Alt+F6` | Copy / move selection to the other pane |
| `F8` | Move to trash (same as `Delete`) |
| `Ctrl+U` | Swap panes |
| `Ctrl+Shift+T` | Reopen closed tab |
| `Ctrl+.` | Star |
| `Ctrl+Shift+G` | Tag picker |
| `Alt+T`, then `Left`/`Right` | Step through snapshots; `Esc` returns to now |
| `Backspace` | Up one level |
| `Ctrl+Shift+Enter` | Open in disposable sandbox |

Context menu on a file: Open · Open with ▸ · Open in sandbox · Cut · Copy · Copy path · Rename · Move to trash · Compress… · Tags ▸ · Star · Checksums and verify… · Versions… · Share ▸ · Actions ▸ · Properties. On a folder, also: Open in new tab · Open in other pane · Open terminal here · Add bookmark · Allow an app… · Analyse space. On blank space: New ▸ · Paste · Sort ▸ · Group ▸ · Select by pattern… · Open terminal here · Properties. On a snapshot item: Restore · Restore a copy… · Compare with now · Copy.

Drag and drop: move within a dataset, copy across datasets and to remote (shown by the cursor); `Ctrl` forces copy, `Shift` move, `Alt` link; right-drag opens a menu. Drops onto tabs, breadcrumb segments, sidebar places, tags (to tag) and the other pane. Dragging out offers a file handle list plus `text/uri-list`; another app receives handles only after the shell's drop confirmation for sandboxed targets. Touchpad: pinch zooms icons, three-finger swipe is back and forward.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | One line and [New folder]; empty search says what was searched and where. |
| Loading | Progressive listing; spinner in the pane header; sort applies as rows arrive. |
| Error | Banner in the pane (no permission, device gone, I/O error) with Retry and Details. |
| Offline | Remote panes: "Disconnected from build01" [Reconnect]; jobs paused. |
| Locked | Encrypted, locked dataset: a lock page with [Unlock in Disks]. |
| Read-only | Banner; cut, paste, rename, delete disabled with tooltips. |
| Snapshot | Amber banner; only copy-out actions enabled. |
| Narrow (< 640) | Sidebar overlay, inspector as a sheet, single pane only, list view becomes two-line rows. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Tabs | `tabBar` / `tabWidget` |
| Sidebar | `treeView` |
| List and tree view | `tableView`, `treeView` (FUSE-WP-28 `tableView2`, `treeView2`) |
| Icon, gallery strip, compact | `gridView`, `listView` |
| Panes | `splitter` |
| Inspector | `tabWidget`, `propertyGrid` |
| Preview | `image`, `markdownView`, `codeEdit` (read-only), `videoWidget` (G-11) |
| Time-travel bar | `timeline` |
| Jobs | `progressBar` in a `popupMenu`-style popover |
| Bulk rename, ACL, attributes | `dialog`, `tableView`, `comboBox`, `lineEdit`, `checkBox` |
| Terminal panel | terminal grid (G-13, FUSE-WP-39) |

Missing:

- `SY02-GAP-1` Breadcrumb path bar widget (segments, overflow, edit mode, per-segment drop targets).
- `SY02-GAP-2` Miller column view (chained `listView`s with horizontal scrolling and a preview column).
- `SY02-GAP-3` Chip filter bar (typed tokens with completion).
- `SY02-GAP-4` Rubber-band selection and emblem overlays in `gridView` (planned in FUSE-WP-28).
- G-11 for media preview; G-13 for the terminal panel; G-01 for drag and drop between apps.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Files app | ForgeScript | app host | Windows, tabs, panes, view models, selection, actions, preferences |
| Views and dialogs | FUSE UI | app host | `.fuseui` documents; pane component shared with SH01 and SH04 |
| Directory lister | C++23 | helper, Capsicum, directory handles only | Fast enumeration, stat batches, change notification (kqueue), sorting keys; streams rows over shared memory |
| File operations service | C++23 | system service (`svc.undo-trash`) | Copy, move, link, trash, journal, undo, verify; runs jobs with the handles passed to it. Proposed as part of G-09. |
| Remote helper (one per connection) | C++23 | helper, Capsicum, one socket to one host | SFTP, WebDAV, SMB clients; exposes a directory and file interface to the lister and the operations service |
| Hash and signature helper | C++23 | helper, Capsicum, read handles | BLAKE3, SHA-2, legacy digests; signature checks with public keys passed in |
| Snapshot interface | C++23 | system service (snapshot and dataset service, Proposed) | List snapshots, give read-only directory handles into them, `zfs diff` records, create a snapshot |
| Mount broker | C++23 | system service, privileged (Proposed) | NFS mounts, removable media mount and eject |
| Action runner | ForgeScript | separate app-host engine per action | Runs a signed user action with only the selection handles |
| Metadata reader | C++23 | `svc.thumbnail` sandboxes | EXIF, tags, document info |

```
 +------------------------------- app host: Files -------------------------------+
 | ForgeScript: tabs, panes, models, actions      FUSE UI: views, inspector      |
 +---+-----------+------------+-------------+-------------+-----------+----------+
     | shm rows  | bus        | bus         | bus         | helper    | engine
     v           v            v             v             v           v
 directory   file operations  snapshot    svc.search   hash/signature  action runner
 lister      service          service     svc.thumbnail  helper        (selection
 (dir        (svc.undo-trash) (read-only  svc.intent                    handles only)
  handles)        |            handles)   svc.audit
     ^            v
     +------ remote helper (one host, one socket) ── network
```

The app itself never opens a file's contents. It holds directory handles for places the user has (home by manifest grant, see section 7) and passes narrowed handles to helpers and other apps.

### 6.2 Data model

| Entity | Fields | Store |
|---|---|---|
| `Entry` (in memory) | `name: string`, `kind: file\|dir\|link\|device\|socket\|fifo`, `size: u64`, `mtime, ctime, btime, atime: i64 ns`, `uid, gid: u32`, `mode: u16`, `flags: u32`, `nlink: u32`, `mime: string`, `link_target`, `tags: string[]`, `origin_kind: u8`, `trust: u8`, `emblems: u16` | lister rows in shared memory |
| `Provenance` | `version: u8`, `steps: {kind: web\|mail\|chat\|removable\|share\|created\|extracted\|copied\|restored, source: string, referrer: string, app_id: string, at: i64, digest_b3: bytes32}[]`, `trusted_by_user: bool` | extended attribute `user.pbsd.origin` (CBOR, at most 4 KiB; older steps are dropped first) |
| `Tags` | `string[]` | extended attribute `user.pbsd.tags` (UTF-8, newline-separated); mirrored in `svc.search` |
| `Note`, `Star` | `string`, `bool` | `user.pbsd.note`, `user.pbsd.star` |
| `Bookmark` | `id`, `name`, `uri`, `icon`, `route`, `secret_ref`, `host_key_pin`, `previews: bool` | `~/.config/org.pbsd.files/bookmarks.json` |
| `FolderViewState` | `path_or_dataset_id`, `view`, `sort: {col, dir}[]`, `columns: {id,width}[]`, `zoom`, `group_by`, `show_hidden` | `~/.local/share/org.pbsd.files/views.sqlite` |
| `Job` | `id: u64`, `kind`, `sources[]`, `dest`, `state: queued\|running\|paused\|done\|failed\|undone`, `bytes_done`, `bytes_total`, `errors[]`, `started_at`, `verify: bool` | file operations service journal |
| `JournalEntry` | `id`, `job_id`, `op`, `before`, `after`, `reversible: bool`, `expires_at` | `svc.undo-trash` |
| `RenameRuleSet` | `name`, `rules: {type, params}[]` | `~/.config/org.pbsd.files/rename-sets.json` |
| `Action` | `id`, `title`, `file: path`, `mime_filter[]`, `caps[]`, `signature_ok` | `~/.config/org.pbsd.files/actions/*.fs` + `.sig` |
| `SavedSearch` | `id`, `name`, `query`, `scope` | `~/.config/org.pbsd.files/searches.json` |
| `SessionState` | windows, tabs, panes, paths, selection, scroll | `~/.local/share/org.pbsd.files/session.json` |

Stores carry `schema_version`; migration is forward only with a copy of the previous file (PLATFORM.md section 10). Extended-attribute records carry their own `version` byte and unknown versions are shown as "unreadable origin record", never guessed.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Directory entries, stat, ACL, extattr | yes | yes | OS interfaces through handles | directory lister; writes in the file operations service |
| Provenance record (CBOR) | yes | yes | own code | directory lister (read), operations service (write) |
| SFTP (SSH v2) | yes | yes | Proposed: libssh2 (BSD-3-Clause) or the base system's SSH code; decision open | remote helper |
| WebDAV over HTTPS | yes | yes | own code on the FORGE network layer; XML subset parser own code | remote helper |
| SMB 3 | yes | yes | Proposed: libsmb2 (LGPL-2.1) in its own process; licence review needed | remote helper |
| NFSv4 | yes | yes | kernel client | kernel, through the mount broker |
| Checksum files (`SHA256SUMS`, BSD style, `.b3`) | yes | yes | own code | hash helper |
| Ed25519 detached `.sig` (V14), minisign, signify | yes | no | own code, FORGE crypto | hash and signature helper |
| OpenPGP signatures | yes | no | handed to SE04's key helper | SE04 helper |
| Images, PDF, media, fonts for preview | yes | no | `svc.thumbnail` | thumbnail sandboxes |
| ZFS diff records | yes | no | snapshot service | snapshot service |
| `text/uri-list`, handle lists | yes | yes | own code | app host |

### 6.4 Integration

- **Intents handled**: `open` (folders, `file:`, `sftp:`, `davs:`, `smb:`, `nfs:` URLs), `pick` (as the engine behind SH04), `search`. **Emitted**: `open`, `edit`, `view`, `share`, `print`, `new`, `compose`.
- **MIME**: `inode/directory` (view). Everything else is opened by other apps.
- **Services**: `svc.portal.file`, `svc.intent`, `svc.search`, `svc.thumbnail`, `svc.undo-trash`, `svc.notify`, `svc.keyring`, `svc.audit`, `svc.settings`, `svc.clipboard`, `svc.model` (optional), `svc.backup` (snapshot list, Proposed extension), `svc.a11y`.
- **Offers**: the pane component and the lister to SH01 (desktop surface) and SH04 (file chooser) as a shared library of `.fuseui` and ForgeScript modules.
- **Other apps**: UT01 (archives), SE05 (sign, encrypt, erase), SE04 (keys), AD05 (grants), AD02 (unlock, datasets, write image), AD03 (schedules), UT04 (space), SY03 (terminal here), VM01 (disposable VM), IN03 (large remote transfers), OF01 (edit action).
- **Backing repo facts (Verified)**: PBSD handles carry rights that only narrow and can be revoked in full (`docs/security/INVARIANTS.md` SI-1, SI-6, via PLATFORM.md V19). FUSE has `tableView`, `treeView`, `gridView`, `fileDialog` and `timeline` widgets (PLATFORM.md V3).
- **Proposed**: provenance attributes, the snapshot interface, the mount broker, disposable sandbox launch through the app host.

## 7. Security and privacy

Capabilities requested:

| Capability | Reason |
|---|---|
| `fs.app` | Own config, view-state database, session |
| `fs.portal` | Default route to user files |
| `fs.path:~:rw` | A file manager must list the user's home without a prompt per folder. Shown at install. Handles are directory handles; contents are read only by helpers and other apps. |
| `fs.path:/media:rw`, `fs.path:/:r` | Removable media; read-only browsing of the system tree |
| `net.user` | Servers the user types; each first contact is logged |
| `net.route:clear`, `net.route:vpn`, `net.route:tor` | Per-bookmark route choice |
| `proc.helper:dir-lister`, `proc.helper:remote-helper`, `proc.helper:hash-helper` | Helpers of section 6.1 |
| `svc.*` as in 6.4 | Services |
| `ai.local` | Only for semantic search (F055), off by default |

Not requested: `proc.exec` (running a file goes through SY03 or the sandbox launcher), `dev.*`, `bg.autostart`. `bg.run` is requested only if folder rules (F103) are enabled.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| The user's files | Malicious file names or metadata | Control characters, bidirectional overrides, very long names, crafted extattrs | Names are rendered with visible escapes; attribute parsers run in the lister sandbox with size caps. |
| The session | Hostile file content | Bugs in preview decoders | All decoding in `svc.thumbnail` sandboxes; untrusted previews never fetch; no in-process parsing of file bodies. |
| The user | Disguised executables | `invoice.pdf.sh`, launchers, auto-run media | Nothing executes on double click; extensions always shown for untrusted files; no auto-run. |
| Credentials | Hostile or spoofed server | SSH downgrade, key change, WebDAV redirect to another host | Host keys pinned; a changed key blocks; the remote helper has a socket right to one host only; redirects off-host refused. |
| Other files | Compromised remote helper | Reads local files | It has no file handles; data flows through the operations service. |
| Integrity | Archive or copy tricks | Symbolic-link races, path traversal during copy | Operations use handle-relative calls (`openat` with no-follow beneath) only. |
| Privacy | Provenance records | Leaking source URLs when a file is shared | Share and copy-to-removable strip `user.pbsd.*` attributes by default, with a visible switch. |
| The system | User actions (ForgeScript) | A downloaded action abuses access | Actions must be signed by a key the user trusts; they declare capabilities; they get only the selection handles. |
| Trash and snapshots | False sense of deletion | User believes data is gone | "Delete for ever" states that snapshots may keep it and links to SE05 and AD03. |

Stored: bookmarks, view state, session, rule sets (no secrets). Leaves the machine: only what the user sends to a server they added.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `files.view.default` | enum(`icons`,`list`,`columns`,`gallery`,`compact`) | `list` | Default view |
| `files.view.remember` | enum(`per_folder`,`per_type`,`global`) | `per_folder` | View memory |
| `files.view.folders_first` | bool | true | Sort folders first |
| `files.view.show_hidden` | bool | false | Hidden files |
| `files.view.density` | enum(`compact`,`normal`,`touch`) | `compact` | Row density |
| `files.click.single_opens` | bool | false | Single click opens |
| `files.dualpane.start` | bool | false | Start with two panes |
| `files.thumbnails.local` | bool | true | Thumbnails for local files |
| `files.thumbnails.untrusted` | bool | false | Thumbnails for untrusted-origin files |
| `files.thumbnails.max_size_mb` | int | 64 | Largest file to thumbnail |
| `files.ops.verify` | enum(`never`,`removable_remote`,`always`) | `removable_remote` | Verify after copy |
| `files.ops.confirm_trash` | bool | false | Confirm moving to trash |
| `files.ops.strip_origin_on_share` | bool | true | Remove provenance when sharing |
| `files.trash.auto_empty_days` | int 0–365 | 30 | 0 means never |
| `files.untrusted.default_open` | enum(`ask`,`sandbox`,`normal`) | `ask` | What double click does for untrusted files |
| `files.search.content` | bool | true | Search contents by default |
| `files.search.semantic` | bool | false | Local semantic search |
| `files.remote.previews` | bool | false | Previews on remote locations |
| `files.remote.discovery` | bool | false | mDNS listing |
| `files.sizes.folders` | bool | false | Compute folder sizes |
| `files.session.restore` | bool | true | Restore windows and tabs |
| `files.checksum.default` | enum(`blake3`,`sha256`,`sha512`) | `blake3` | Default digest |
| `files.terminal.panel_follows` | bool | true | Terminal panel follows the folder |

## 9. Accessibility and languages

- Icon view is exposed as a grid with row and column counts; the list as a table with sortable headers.
- Emblems have names ("untrusted origin", "signature verified") spoken after the file name.
- Drag and drop has a keyboard equivalent everywhere: cut, copy, paste, and "Move to…" / "Copy to…" dialogs.
- The time-travel bar is a slider with value text ("10 November 03:00, daily snapshot, 7 of 31").
- Names sort by the locale's collation; mixed-direction names are isolated so a right-to-left name cannot reorder the extension.
- Sizes follow the locale's unit style; binary or decimal units are a system setting.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | 150 ms; home listing visible in 250 ms |
| List 10,000 entries | first rows 50 ms, complete 300 ms on local ZFS |
| List 1,000,000 entries | first rows 100 ms, scroll at 60 frames per second while loading, memory under 400 MiB |
| Sort 100,000 rows | 150 ms |
| Scroll | 16.6 ms frame budget, zero allocation in steady state |
| Copy throughput | within 5% of `cp` on the same devices; same-pool copies use block cloning |
| Memory, one window, small folders | 80 MiB |
| Idle | 0 frames; change notification by kqueue, no polling |
| Snapshot step | under 100 ms to show a folder in another snapshot |

## 11. Testing and proof

Test plan (targets; nothing has run):

- Unit: rename rules, query parser, conflict rules, provenance append, natural sort, name-safety checks.
- Golden renders: all views, banners, dialogs, three themes, right-to-left.
- Scripted UI: the 14 top tasks on a ZFS test pool with snapshots and a local SFTP server.
- Fuzz: provenance CBOR, checksum files, WebDAV XML, SFTP and SMB packet parsers, extattr values, URI parser.
- Differential: operations against `cp -a`, `mv`, `rsync -aHAX` results (content, attributes, ACLs, sparse layout); digests against reference tools; ACL effective-access against the kernel's answer.
- Fault injection: device removal mid-copy, full disk, power cut during move (journal replay), server drop.
- Adversarial names corpus: control characters, bidirectional overrides, 255-byte names, invalid UTF-8.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Operation journal | Undo of a completed job restores the prior tree; replay after a crash is idempotent | T1 | Model of the journal state machine proved; crash-point tests | File system semantics are assumed |
| Handle-relative path resolution | No operation escapes the directory handle it was given | T1 | PRISM proof over the path walker | — |
| Provenance codec | Total, bounded, round-trips | T2 | PRISM plus fuzz | Format spec for T1 |
| Rename rule engine | Output names are unique or flagged; no data loss | T2 | Property tests, PRISM on the conflict detector | — |
| Remote protocol parsers | No runtime errors on any input | T2 | PRISM plus fuzz | Third-party code, if used, stays T3 |
| Hash helper | Digests equal reference vectors | T3 | Known-answer tests | Crypto proofs out of scope here |
| Directory lister | Bounded memory per entry, no out-of-bounds | T2 | PRISM plus sanitisers | — |
| ForgeScript app logic | Language safety | T3 | Unit and UI tests | Interpreter not proved |

## 12. Build plan

Phase 1, usable core:

| ID | Work item | Depends on |
|---|---|---|
| SY02-W01 | Directory lister helper with shared-memory row stream and change notification | G-06, G-07 |
| SY02-W02 | Pane component: list, tree and icon views, selection, type-ahead, sort, group | G-02, FUSE-WP-28 |
| SY02-W03 | Window: tabs, dual pane, sidebar places and devices, path bar | SY02-GAP-1 |
| SY02-W04 | File operations through `svc.undo-trash`: copy, move, link, trash, queue, conflict dialog, undo | G-09 |
| SY02-W05 | Open and open-with through `svc.intent`; executable-file rule | SH04 |
| SY02-W06 | Inspector: general, permissions (POSIX), checksums | SY02-W01 |
| SY02-W07 | Thumbnails, preview pane, text peek | G-09 (`svc.thumbnail`) |
| SY02-W08 | Indexed name search and live filter | G-09 (`svc.search`) |
| SY02-W09 | Session restore, preferences, per-folder view state | SY02-W03 |
| SY02-W10 | Share the pane component with SH01 and SH04 | SH01, SH04 |
| SY02-W11 | Terminal here; archives hand-off | SY03, UT01 |

Phase 2, full feature:

| ID | Work item | Depends on |
|---|---|---|
| SY02-W12 | Snapshot interface; time-travel bar, versions tab, restore, diff | AD03, snapshot service |
| SY02-W13 | Provenance attribute: portal writes, display, propagation, untrusted mark | SH04, IN01, CO01 |
| SY02-W14 | Open in disposable sandbox | G-07, AD05 |
| SY02-W15 | Capabilities tab and grant from Files | AD05 |
| SY02-W16 | Remote helper: SFTP and WebDAV; connect dialog, host-key pinning, bookmarks | SE04, SE02 |
| SY02-W17 | Tags, stars, notes, saved searches, query syntax, unindexed search | SY02-W08, SY02-GAP-3 |
| SY02-W18 | Bulk rename | SY02-W04 |
| SY02-W19 | Signature verification and checksum files | SE04 |
| SY02-W20 | NFSv4 ACL editor, flags, extended attributes, brokered ownership change | SY01 broker |
| SY02-W21 | Column view, gallery view, quick look | SY02-GAP-2, G-11 |
| SY02-W22 | ForgeScript actions and the action runner | G-07 |
| SY02-W23 | Verify after copy, resume, attribute preservation report, journal viewer | SY02-W04 |

Phase 3, advanced:

| ID | Work item | Depends on |
|---|---|---|
| SY02-W24 | SMB and NFS locations; route choice; discovery | SE03, mount broker |
| SY02-W25 | Compare folders; deleted-files finder; clone browse | SY02-W12, AD02 |
| SY02-W26 | Embedded terminal panel | G-13 |
| SY02-W27 | Folder rules agent; action editor | SY02-W22 |
| SY02-W28 | Local semantic search | G-15, AI02 |
| SY02-W29 | File activity from audit; print listing; split and join | AD04 |

## 13. Open questions and risks

- `fs.path:~:rw` makes Files the most trusted user app. The alternative is a privileged lister service with the app holding nothing; this needs a decision with AD05.
- Provenance only works if every writer (portal, browser, mail, archives) records it. It needs a platform-level rule, not an app feature.
- Extended attributes do not survive every file system or transfer (FAT, some remote protocols). The index mirror keeps tags locally, but provenance is lost there.
- The snapshot and dataset service is not in PLATFORM.md section 8. `svc.backup` may be the right home.
- SFTP and SMB client code: third-party libraries or own code is undecided; SMB is the weaker case and may be dropped.
- Block cloning availability on the HardenedBSD ZFS version in use is Unverified.
- Per-file access audit (F082) may be too costly to keep on by default.
- Per-dataset trash on encrypted or quota-limited datasets needs rules for what counts against quota.
