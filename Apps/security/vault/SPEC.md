<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Vault

By Odin Loch

| | |
|---|---|
| ID | `SE05` |
| Bundle | `org.pbsd.vault` |
| Category | security |
| Priority | P1 |
| Backing repos | ParanoidBSD (ZFS native encryption, GELI, memory disks and the user-space file system interface in the `hbsd/` reference tree), FORGE (signing, ForgeScript), FUSE (UI) |
| Status | Planned. Nothing built. |

## 1. Purpose

Vault protects files. It makes encrypted folders and portable encrypted containers, encrypts, decrypts, signs and verifies single files, checks the hashes and signatures of downloads, removes hidden metadata, and erases data with an honest account of what erasing can and cannot do on this system. It takes every key from Passwords and Keys (SE04) and adds its actions to Files (SY02). It replaces a container tool, a file-encryption tool, a checksum tool and a metadata cleaner.

Ground truth (Verified in this repo, 12 Nov 2026):

| Fact | Source |
|---|---|
| ZFS is imported with its native-encryption code. | `hbsd/src/sys/contrib/openzfs/module/zfs/dsl_crypt.c`; `docs/plan/PBSD_PLAN.md` (VFS package: OpenZFS "direct import, wrapped") |
| GELI block-level encryption is in the reference tree. | `hbsd/src/sys/geom/eli/`, `hbsd/src/lib/geom/eli/` |
| File-backed memory disks can be configured. | `hbsd/src/sbin/mdconfig/` |
| A kernel interface for user-space file systems is in the reference tree and is listed for import. | `hbsd/src/sys/fs/fuse/`; `docs/plan/PBSD_PLAN.md` (VFS package) |
| An archive library is in the reference tree. | `hbsd/src/contrib/libarchive/` |
| FORGE signs files with Ed25519 over BLAKE3, with detached `.sig` files bound to the file name, and has `forge keygen/sign/verify`. | PLATFORM.md V14; `FORGE:docs/security.md` |
| Storage on PBSD is copy-on-write ZFS with checksums on by default. | `docs/plan/PBSD_PLAN.md` ("Storage integrity") |

Note on names: "FUSE" in this suite is the GUI toolkit. The kernel's user-space file system interface is written out in full in this document to avoid confusion.

Everything else here is Proposed.

## 2. Scope

In scope:

- Encrypted folders (ZFS encrypted datasets) and portable containers (single file).
- Per-file and per-folder encrypt and decrypt; sign and verify.
- Hash calculation and verification; checksum files; download verification with provenance.
- Metadata inspection and cleaning for images, documents, audio, video and archives.
- Secure erase: crypto-erase, snapshot-aware delete, free-space actions, with stated limits.
- Optional steganography indicators.
- A discussion of hidden volumes and deniability, with honest limits.
- Key hand-off to SE04; context actions in SY02.

Out of scope:

| Item | Owner |
|---|---|
| Key generation, storage, tokens, trust store | SE04 (Vault holds references only) |
| Whole-disk and boot-disk encryption, pools, partitions, disk wipe | AD02 |
| Backups and off-site copies of containers | AD03 |
| Encrypted archives as an archive format (7z, zip AES) | UT01 (Vault hands off) |
| Mail encryption | CO01 |
| Signing app bundles and packages | DV01, SY05 with `forge sign` through SE04 |
| Malware scanning of files | SE01 (Vault offers "Scan" hand-off) |
| Redacting document content | OF03 |
| Editing image content | CR01 |
| Snapshot browsing | SY02 |

## 3. Users and top tasks

Personas:

- **Everyday user**: wants a locked folder for private papers and a way to send one file safely.
- **Traveller or journalist**: carries a portable container on removable media; cleans photos before sharing.
- **Developer or administrator**: verifies downloads, signs releases, checks checksum lists.
- **Records keeper**: needs proof a file has not changed (signature and timestamped hash record).
- **High-risk user**: wants to understand exactly what "erased" means on this machine.

| # | Task | Target |
|---|---|---|
| T1 | Unlock an encrypted folder | 2 steps: click, authenticate |
| T2 | Lock everything | 1 click or `Ctrl+Shift+Delete` |
| T3 | Create an encrypted folder | 3 steps |
| T4 | Encrypt a file for a person | In Files: context action → pick recipient → done (3 steps) |
| T5 | Decrypt a received file | Double-click, confirm key use (2 steps) |
| T6 | Verify a download against a published hash | Drop file, paste hash (2 steps) |
| T7 | Verify a signature | Drop file and its `.sig` (1 step) |
| T8 | Sign a file | Context action → pick key → confirm (3 steps) |
| T9 | Remove location data from 30 photos | Drop, review, "Clean copies" (3 steps) |
| T10 | Erase a file for good and learn what remains | Context action → read the summary → confirm (3 steps) |
| T11 | Create a portable container on a USB stick | Wizard, 4 steps |
| T12 | Check a whole folder against a `SHA256SUMS` file | Open the sums file (1 step) |
| T13 | Destroy a container beyond recovery | 3 steps with typed confirmation |
| T14 | See what metadata a file carries | Drop the file (1 step) |

## 4. Features

### 4.1 Encrypted folders (ZFS datasets)

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE05-F001 | Create encrypted folder | A ZFS dataset with native encryption (AES-256-GCM), own key, mounted at a chosen place under the user's home | M | 1 |
| SE05-F002 | Key sources | Passphrase; key held in SE04 (released on confirmation); hardware token through SE04; combinations by policy | M | 1 |
| SE05-F003 | Unlock and lock | Load key and mount; unmount and unload key. Lock fails safe: lists apps holding files open and offers to close them | M | 1 |
| SE05-F004 | Auto-lock | On idle, session lock, suspend, token removal, schedule | M | 1 |
| SE05-F005 | Auto-unlock at login | Optional per folder; off by default | S | 2 |
| SE05-F006 | Properties | Quota, compression, case sensitivity, snapshot policy, backup inclusion | S | 2 |
| SE05-F007 | Snapshots inside | Snapshots are encrypted with the same key; count and space shown; policy editable | S | 2 |
| SE05-F008 | Change key | Change passphrase or wrapping key without rewriting data | M | 1 |
| SE05-F009 | Send as encrypted stream | Export the dataset as a raw encrypted stream to a file or to AD03; the receiver cannot read it without the key | S | 3 |
| SE05-F010 | Per-app access | Apps reach an unlocked folder only through portal grants like any other location; a folder can be marked "ask every time" | M | 2 |
| SE05-F011 | Destroy (crypto-erase) | Destroys the dataset, its snapshots and its wrapped key; typed confirmation; summary of what is left (backups, clones) | M | 1 |
| SE05-F012 | Unlocked indicator | An open-lock badge in the panel while any folder is unlocked; list in quick settings | M | 1 |
| SE05-F013 | Search exclusion | Contents are not indexed by `svc.search` unless switched on per folder; index entries are dropped at lock | M | 2 |
| SE05-F014 | Thumbnail and recent-files policy | Thumbnails and recents for files inside are kept inside the folder or not at all | M | 2 |

### 4.2 Portable containers

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE05-F015 | Portable container file | A single `.pbsdbox` file in a documented chunked format (section 6.2); grows as needed; safe to copy and sync | M | 2 |
| SE05-F016 | Open as a location | Appears in Files as a place; apps reach it through the file portal; contents never land on disk in the clear | M | 2 |
| SE05-F017 | Mount as a file system | Optional mount through the kernel's user-space file system interface for tools that need paths | S | 3 |
| SE05-F018 | Key slots | Passphrase, key file, token, recipients' public keys (several people can open one container) | M | 2 |
| SE05-F019 | Read-only open | For media that may be untrusted or write-protected | S | 2 |
| SE05-F020 | Integrity | Every chunk is authenticated; a damaged chunk is reported with the affected files; the rest stays readable | M | 2 |
| SE05-F021 | Compaction | Reclaims space after deletes; runs on request | S | 3 |
| SE05-F022 | Size padding | Optional padding of the file to size buckets so its size tells less | C | 3 |
| SE05-F023 | Rewrap and rotate | Change slots without rewriting data; full re-key rewrites all chunks | S | 3 |
| SE05-F024 | Container on removable media wizard | Picks format, size policy, key slots, prints a recovery sheet | S | 2 |
| SE05-F025 | Block-style container (GELI) | A fixed-size file attached as a block device with GELI and a file system inside; for compatibility with tools that need a real disk | C | 3 |
| SE05-F026 | Foreign containers | Open VeraCrypt and LUKS2 volumes read-only | C | 3 |
| SE05-F027 | Reader for other systems | Publish the format and a small command-line reader so containers are not locked to this OS | S | 3 |
| SE05-F028 | Container list | Known containers with location, state, last opened, size, slots | M | 2 |

### 4.3 File encryption

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE05-F029 | Encrypt file or folder | Default format age v1 (`.age`); folders are packed to a tar stream first | M | 1 |
| SE05-F030 | To recipients | Pick people or own keys from SE04; several recipients at once | M | 1 |
| SE05-F031 | With a passphrase | Generated by default (SE04 generator), typed if wanted; shown once | M | 1 |
| SE05-F032 | OpenPGP output | Optional `.pgp` / `.asc` when the recipient needs it; done by the SE04 key service | S | 2 |
| SE05-F033 | Decrypt | Detects the format; asks SE04 for the key; writes beside the source or to a chosen place | M | 1 |
| SE05-F034 | Decrypt to a disposable view | Open the plaintext in a viewer inside a disposable sandbox without writing it to the user's files | S | 2 |
| SE05-F035 | Sign while encrypting | Encrypt-then-sign with a detached signature, or OpenPGP sign-and-encrypt | S | 2 |
| SE05-F036 | Armour | Text-armoured output for pasting | S | 2 |
| SE05-F037 | Streaming | Constant memory for any size; progress, pause, cancel; partial output removed on cancel | M | 1 |
| SE05-F038 | Batch | Many files in one job; one output per input or one packed output | M | 1 |
| SE05-F039 | Name handling | Option to hide the original name (random output name, real name stored inside) | S | 2 |
| SE05-F040 | After-encrypt action | Keep the original, move it to trash, or erase it (with the erase summary) | M | 1 |
| SE05-F041 | Hybrid post-quantum recipients | X25519 with ML-KEM-768 recipient type when SE04 offers such keys; marked experimental | C | 3 |
| SE05-F042 | Text and clipboard | Encrypt or decrypt a pasted text block | S | 2 |
| SE05-F043 | Self-contained share | A passphrase-encrypted file plus a short instruction text for a receiver who has only a standard age tool | S | 2 |
| SE05-F044 | Encrypted archives hand-off | "Encrypt as 7z/zip" opens UT01 | C | 2 |

### 4.4 Sign and verify

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE05-F045 | Sign file | Detached signature with a key from SE04; default FORGE format (Ed25519 over BLAKE3, `.sig`, bound to the file name) | M | 1 |
| SE05-F046 | Other signature formats (write) | OpenPGP detached (`.sig`, `.asc`), SSH signatures (`sshsig`) | S | 2 |
| SE05-F047 | Verify | Auto-detect: FORGE, OpenPGP, SSH signature, minisign, signify, X.509 CMS detached | M | 1 |
| SE05-F048 | Verification result | Valid or not; who signed (name from SE04 and the verified mark); key fingerprint; time; what exactly was covered | M | 1 |
| SE05-F049 | Unknown signer flow | Shows the fingerprint, offers "Import key into Passwords and Keys" and never says "valid" for an untrusted key without saying so | M | 1 |
| SE05-F050 | Signed checksum lists | Verify `SHA256SUMS` plus `SHA256SUMS.sig` then check the files | M | 2 |
| SE05-F051 | Folder manifest | Sign a manifest of path, size, BLAKE3 for a whole folder; verify later and list modified, missing, extra files (same model as FORGE's signed bundles) | S | 2 |
| SE05-F052 | Counter-signature | Add a second signature to an existing manifest | C | 3 |
| SE05-F053 | Timestamp record | Local record of hash and time in the audit log; export as a signed statement. No external timestamp service by default | S | 3 |
| SE05-F054 | Batch verify | A folder of files and signatures in one pass with a result table | S | 2 |
| SE05-F055 | Signature badge for Files | SY02 can show a verified badge from a cached result keyed by content hash | C | 3 |

### 4.5 Hashes and download verification

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE05-F056 | Compute hashes | BLAKE3, SHA-256, SHA-512, SHA3-256; legacy SHA-1 and MD5 shown with a "broken, for comparison only" mark | M | 1 |
| SE05-F057 | Compare with expected | Paste or type a hash; algorithm inferred from length and prefix; case and spaces ignored; match or mismatch in words | M | 1 |
| SE05-F058 | Clipboard watch (in window) | When the window is focused and the clipboard holds a hash-shaped string, offer to compare | S | 1 |
| SE05-F059 | Checksum files | Read and write `SHA256SUMS`-style, BSD-style (`SHA256 (file) = …`), `.sha256`, `.b3` | M | 1 |
| SE05-F060 | Verify a folder against a list | Table of OK, mismatch, missing, extra | M | 2 |
| SE05-F061 | Download verification flow | From IN03 or IN01: "Verify…" opens Vault with the file, its source address and any signature or checksum fetched beside it | S | 2 |
| SE05-F062 | Provenance record | Stores source address, time, hash and verification result as file metadata that Files can show | S | 2 |
| SE05-F063 | Speed | Multi-threaded BLAKE3; streaming for the others; progress for large files | M | 1 |
| SE05-F064 | Compare two files | Byte-identical or not, by hash; hand-off to DV04 for a diff | C | 2 |
| SE05-F065 | Hash of text | Hash a pasted string, with encoding stated | C | 2 |
| SE05-F066 | Image and disc verification | Verify an install image, then hand to AD02 to write | S | 2 |

### 4.6 Metadata cleaner

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE05-F067 | Inspect | Show every metadata field found, grouped: location, device, author, software, dates, identifiers, thumbnails, history | M | 2 |
| SE05-F068 | Risk marks | Fields that identify a person, place or device are marked | M | 2 |
| SE05-F069 | Images | JPEG, PNG, WebP, TIFF, HEIF, AVIF, GIF: EXIF, XMP, IPTC, ICC description, comments, embedded thumbnails, maker notes | M | 2 |
| SE05-F070 | PDF | Info dictionary, XMP, document IDs, embedded file metadata, incremental-save history (rewrites the file) | M | 2 |
| SE05-F071 | Office documents | ODF and OOXML: author, company, revision data, comments, tracked changes (warn, optional removal), custom properties, embedded previews | M | 2 |
| SE05-F072 | Audio | ID3, Vorbis comments, FLAC blocks, MP4 atoms, cover art | S | 2 |
| SE05-F073 | Video | MP4 and MOV atoms, Matroska tags, location and device tags | S | 3 |
| SE05-F074 | Archives | zip, tar: owner names, timestamps, extra fields, comments | S | 3 |
| SE05-F075 | Other | SVG metadata and editor data, torrent files (creator, comment), plain EPUB metadata | C | 3 |
| SE05-F076 | Clean copy by default | Writes `<name>.clean.<ext>`; the original is untouched | M | 2 |
| SE05-F077 | Clean in place | Only on request; the old version may remain in snapshots, said clearly | S | 2 |
| SE05-F078 | Presets | Remove everything; keep colour profile and orientation; keep copyright and author; custom | M | 2 |
| SE05-F079 | Before and after | Side-by-side list of fields removed and kept; re-inspect the result to prove it | M | 2 |
| SE05-F080 | Lossless | Image pixels and audio samples are never re-encoded; where a format cannot be cleaned without re-encoding, the tool says so and offers the choice | M | 2 |
| SE05-F081 | Batch | Folders and many files; summary table; failures listed | M | 2 |
| SE05-F082 | Unknown formats | Refuses to claim "clean" for formats it does not understand | M | 2 |
| SE05-F083 | Content warnings | Notes things metadata cleaning cannot fix: visible faces, text in the picture, document content, file names | S | 2 |
| SE05-F084 | File-system metadata | Option to reset times and drop extended attributes and provenance records on the copy | S | 2 |
| SE05-F085 | Share-sheet hook | "Clean before sharing" toggle offered in the shell share sheet (SH04) for supported types | S | 3 |
| SE05-F086 | Edit a field | Change single fields (author, title) instead of removing | C | 3 |

### 4.7 Secure erase

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE05-F087 | Erase summary | Before any erase: where copies of this data may still exist (snapshots, clones, boot environments, backups, trash, thumbnails, search index, swap), found by checking, not guessed | M | 1 |
| SE05-F088 | Honest method statement | States that overwriting a file in place does not destroy data on a copy-on-write file system or on flash storage, and that Vault does not pretend it does | M | 1 |
| SE05-F089 | Crypto-erase | For data inside an encrypted folder or container: destroy the key. The strongest method available here | M | 1 |
| SE05-F090 | Snapshot-aware delete | Delete the file and, on request, every snapshot that still holds it (lists them, with what else those snapshots hold) | M | 2 |
| SE05-F091 | Trace cleanup | Remove thumbnails, recent-file entries, search index entries, provenance records, trash copies for the file | M | 2 |
| SE05-F092 | Free-space trim | Ask the pool to trim freed blocks on flash; explains it is a request to the device, not proof | S | 2 |
| SE05-F093 | Free-space overwrite | Fill free space of a dataset with random data, then release; slow; limits stated (reserved space, spare blocks) | C | 3 |
| SE05-F094 | Erase removable media | Hand-off to AD02 for device-level secure erase or full overwrite | S | 2 |
| SE05-F095 | Erase on unencrypted spinning disks | Single-pass overwrite offered only where the file system is not copy-on-write (foreign media); multi-pass schemes are not offered, with the reason | C | 3 |
| SE05-F096 | Advice to prevent the problem | Suggests keeping sensitive files in an encrypted folder so that erase is always crypto-erase | M | 1 |
| SE05-F097 | Erase report | What was done, what could not be done, what remains; exportable | S | 2 |
| SE05-F098 | Swap and memory note | States whether swap is encrypted (from SE01) | S | 2 |
| SE05-F099 | Scheduled purge | Per encrypted folder: delete files older than N days in a "temporary" subfolder | C | 3 |

### 4.8 Steganography indicators (optional)

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE05-F100 | Appended data check | Bytes after the end marker of JPEG, PNG, GIF, PDF, zip | S | 3 |
| SE05-F101 | Statistical tests | Chi-square and sample-pair tests on least-significant bits of lossless images; reported as a score, not a verdict | C | 3 |
| SE05-F102 | Structure oddities | Unused chunks, oversized comment blocks, mismatched declared and actual sizes, polyglot files | S | 3 |
| SE05-F103 | Entropy map | Entropy per block as a strip chart; hand-off to DV04 | C | 3 |
| SE05-F104 | Clear wording | Results read "indicator found" or "no indicator found"; never "contains hidden data" or "clean" | M | 3 |
| SE05-F105 | No embedding | Vault does not hide data in files | M | 3 |

### 4.9 Hidden volumes and deniability (discussion, honest limits)

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE05-F106 | Explainer page | A plain page on what deniable storage claims and why the claim is weak on this system (section 7.1) | M | 2 |
| SE05-F107 | No hidden volumes in phases 1–2 | Not offered. An honest absence is safer than a feature people over-trust | M | 2 |
| SE05-F108 | Experimental hidden region | Phase 3 at most, portable containers only, off by default, behind the explainer; only if the leak audit in section 11 can be passed | C | 3 |
| SE05-F109 | Plausible alternatives | Points to what does help: travel mode in SE04, containers kept off the machine, crypto-erase | S | 2 |

### 4.10 Keys, integration, basics

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE05-F110 | Keys by reference | Vault never holds private keys. It asks the SE04 key service to wrap, unwrap, sign and verify; SE04 shows the confirmation | M | 1 |
| SE05-F111 | Recipient picker | The SE04 key picker (intent `pick`) with people, own keys, verified marks | M | 1 |
| SE05-F112 | Passphrase storage | Offer to save a container or file passphrase as an item in SE04 | M | 1 |
| SE05-F113 | Recovery sheet | Printable sheet per folder or container: what it is, format version, key slot hints, optional share (made by SE04) | S | 2 |
| SE05-F114 | Files context actions | Encrypt…, Decrypt, Sign…, Verify, Checksum…, Inspect metadata, Clean metadata, Erase…, Add to container, registered with SY02 | M | 1 |
| SE05-F115 | Open-with handlers | `.age`, `.pbsdbox`, `.sig`, `.minisig`, `.asc` and checksum files open here | M | 1 |
| SE05-F116 | Job queue | All long work runs as jobs with progress, pause, cancel; survives closing the window; results kept until dismissed | M | 1 |
| SE05-F117 | Drop zone | Drop anything on the window; Vault proposes the likely actions for that type | M | 1 |
| SE05-F118 | Audit records | Container create, unlock, lock, destroy; key use; erase; written to `svc.audit` without file contents | M | 1 |
| SE05-F119 | Command-line twin | `vaultctl encrypt|decrypt|sign|verify|hash|clean|lock` | S | 2 |
| SE05-F120 | Activity history | Local list of past jobs; can be turned off; cleared on request | S | 2 |
| SE05-F121 | Lock all | One shortcut locks every folder and container | M | 1 |
| SE05-F122 | Scan hand-off | "Scan with Security Centre" for decrypted or downloaded files | C | 2 |
| SE05-F123 | Print | Recovery sheets, verification and erase reports | S | 2 |
| SE05-F124 | Crash safety | Outputs are written to a temporary name and renamed when complete; container writes are transactional | M | 1 |

## 5. GUI

Layout pattern: sidebar + content, with a drop zone and a job strip.

### 5.1 Window map

```
Main window
├─ Sidebar: Home · Encrypted folders · Containers · Encrypt and decrypt · Sign and verify ·
│           Checksums · Metadata · Erase · Indicators · Activity
├─ Content
│   ├─ Home (drop zone, locked/unlocked summary, recent results)
│   ├─ Encrypted folders (list + detail)
│   ├─ Containers (list + detail)
│   ├─ Encrypt and decrypt (job form + results)
│   ├─ Sign and verify (job form + result card)
│   ├─ Checksums (single file · list verify)
│   ├─ Metadata (file list + field table + before/after)
│   ├─ Erase (targets + erase summary)
│   ├─ Indicators (optional page)
│   └─ Activity (job history)
├─ Inspector (F4): file, container or job properties
├─ Dialogs: New encrypted folder · New container (wizard) · Unlock · Encrypt options · Pick recipients
│           (SE04) · Decrypt destination · Verify result · Erase confirmation · Destroy container ·
│           Clean options · Change key · Recovery sheet
├─ Sheets: What erase can and cannot do · About hidden volumes · Format details
└─ Popovers: Hash detail · Slot detail · Job detail
Shell surfaces: key-use confirmation and passphrase entry (SH04/SE04), Files context menu (SY02),
                unlocked indicator (SH02), share-sheet toggle (SH04)
```

### 5.2 Main window

Home at 1180×760.

```
+------------------------------------------------------------------------------------------+
| [=] Vault                             [ Search…  Ctrl+K ]              🔓2 (1) [▁][▢][✕] |
+----------------+-------------------------------------------------------------------------+
| > Home         | (2) +---------------------------------------------------------------+  |
|   Encrypted    |     |          Drop files here, or  [Choose files…]                 |  |
|     folders  3 |     |  Vault will suggest: encrypt · decrypt · verify · clean · hash|  |
|   Containers 2 |     +---------------------------------------------------------------+  |
|   Encrypt and  +-------------------------------------+-----------------------------------+
|     decrypt    | (3) Unlocked now                    | (4) Recent results                |
|   Sign and     | 🔓 Papers      ~/Private/Papers     | ✔ pbsd-15.iso  SHA-256 matches    |
|     verify     |    locks in 12 min       [Lock]     |   and signature valid (PBSD rel.) |
|   Checksums    | 🔓 travel.pbsdbox  /media/usb       | ✔ 30 photos cleaned (location,    |
|   Metadata     |    read-write            [Lock]     |   device removed)                 |
|   Erase        | 🔒 Finance     ~/Private/Finance    | ✖ report.pdf.sig  NOT valid       |
|   Indicators   |                          [Unlock…]  | ✔ notes.txt → notes.txt.age (Sam) |
|   Activity     |                    [Lock all ⌃⇧⌫]   |                       [Activity…] |
|                +-------------------------------------+-----------------------------------+
|                | (5) Jobs                                                                |
|                | Encrypt  holiday-video.mkv → .age   [██████░░░░] 61%  1.2 GiB/s  [⏸][✕]|
|                | Hash     backup.tar  BLAKE3         [██████████] done   7c1e…   [Copy]  |
+----------------+-------------------------------------------------------------------------+
| (6) 2 unlocked · 2 jobs · keys from Passwords and Keys (unlocked) · search indexing: off |
+------------------------------------------------------------------------------------------+
```

Callouts:

1. Header lock badge: number unlocked; click opens the list with "Lock all".
2. Drop zone: `panel` with drop target; after a drop it becomes an action chooser (`listView` of proposed actions with reasons).
3. Unlocked and locked items: `listView` with state icon and word, countdown, one `button` each.
4. Recent results: `listView`; each row opens its result card.
5. Job strip: `listView` of jobs with `progressBar`, pause and cancel `toolButton`s.
6. Status bar.

States: first run (drop zone plus "Create your first encrypted folder"), SE04 locked (banner "Passwords and Keys is locked; key actions will ask you to unlock"), no jobs (strip collapses), narrow (sidebar overlay, panels stack).

### 5.3 Other views and dialogs

**Encrypted folders**

```
+-------------------------------+-------------------------------------------------------+
| Encrypted folders  [+ New…]   | Papers                                 🔓 unlocked    |
|-------------------------------| ~/Private/Papers · 4.2 GiB of 20 GiB · AES-256-GCM    |
| > 🔓 Papers        4.2 GiB    | Key      passphrase AND security key (policy: both)   |
|   🔒 Finance       1.1 GiB    | Locks    after 15 min idle · on session lock · on     |
|   🔒 Archive 2024  38 GiB     |          token removal                  [Change…]     |
|                               | Snapshots 14 (0.6 GiB) · hourly, kept 2 days [Policy…]|
|                               | Indexing  off      Thumbnails  kept inside            |
|                               | Backup    included, sent encrypted (raw stream)       |
|                               | Open files: Writer (1), Files (window)                |
|                               | [Lock] [Open in Files] [Change key…] [Recovery sheet…]|
|                               |                                    [Destroy…]         |
+-------------------------------+-------------------------------------------------------+
```

**New container wizard (step 3 of 4, keys)**

```
+---------------------------------------------------------------------+
| New portable container — who can open it?               step 3 of 4 |
| [x] Passphrase     (•) Generate  ( ) Type my own                    |
|       correct-horse-battery-staple-orbit-lamp      [👁] [⟳]         |
|       [x] Save in Passwords and Keys → Personal                     |
| [ ] Key file       [Choose…]                                        |
| [x] Security key   "Security key (USB)"   [Enrol second key…]       |
| [x] People         Sam (verified) · Me (age)      [Add…]            |
| Need:  (•) any one of the above   ( ) passphrase AND security key   |
| ! Only one security key is enrolled. Add another or keep the        |
|   passphrase so you cannot be locked out.                           |
|                                          [Back] [Cancel] [Next]     |
+---------------------------------------------------------------------+
```

**Sign and verify › result card**

```
+---------------------------------------------------------------------------------------+
| Verify                                                                                |
| File       pbsd-15.0-amd64.iso   (3.1 GiB)                                            |
| Signature  pbsd-15.0-amd64.iso.sig   (FORGE format, Ed25519 over BLAKE3)              |
|                                                                                       |
|   ✔ VALID — signed by "PBSD Release Key 2026"                                         |
|     Key   ed25519:9c4f…1ab2   trusted: in the system trust store                      |
|     Covers the file's bytes and its name                                              |
|     BLAKE3 7c1e…d09a                                             [Copy]               |
|   Checked 12 Nov 2026 14:31 · result saved as provenance on the file                  |
|                                         [Verify another] [Write to disk in Disks…]    |
+---------------------------------------------------------------------------------------+
```

Variants: `✖ NOT VALID` (file changed or wrong signature; `--danger`), `! VALID BUT SIGNER UNKNOWN` (fingerprint shown, "Import key…"), `? CANNOT CHECK` (format not understood).

**Checksums**

```
+---------------------------------------------------------------------------------------+
| Checksums   [One file] [Verify a list]                                                |
+---------------------------------------------------------------------------------------+
| File  backup.tar (41 GiB)                                         [Choose…]           |
| BLAKE3    7c1e 4a90 …  d09a                                       [Copy]              |
| SHA-256   3b7f 22c1 …  91e4                                       [Copy]              |
| SHA-512   (not computed)                                          [Compute]           |
| Expected  [ 3b7f22c1…91e4                                   ]  → ✔ matches SHA-256    |
|                                           [Save checksum file…] [Sign checksum file…] |
+---------------------------------------------------------------------------------------+
```

**Metadata**

```
+--------------------------+------------------------------------------------------------+
| Files (30)   [Add…]      | IMG_2041.jpg                           preset [Remove all v]|
|--------------------------| Group      Field              Value                  Action |
| > IMG_2041.jpg  ! 9      | ! Location GPS position       51.5007 N, 0.1246 W    remove |
|   IMG_2042.jpg  ! 9      | ! Location GPS altitude       12 m                   remove |
|   IMG_2043.jpg  ! 7      | ! Device   Camera serial      0481…                  remove |
|   scan.pdf      ! 4      |   Device   Make, model        …                      remove |
|   notes.odt     ! 6      |   Dates    Taken              2026-11-02 14:11       remove |
|                          |   Image    Orientation        rotate 90              keep   |
|                          |   Image    Colour profile     Display P3             keep   |
|                          | ! Preview  Embedded thumbnail 160×120                remove |
|                          |------------------------------------------------------------|
|                          | Not fixable here: 2 faces visible; file name has a date    |
|                          | Output (•) clean copies  ( ) in place     [Clean 30 files] |
+--------------------------+------------------------------------------------------------+
```

**Erase summary and confirmation**

```
+---------------------------------------------------------------------+
| Erase "tax-2025.pdf" for good?                                      |
| What Vault can do here                                              |
|  ✔ Delete the file and its trash copy                               |
|  ✔ Remove 3 snapshots that still hold it (they also hold 41 other   |
|    changed files)                                   [Show]          |
|  ✔ Remove thumbnail, recent-files entry, search entry               |
|  ✔ Ask the disk to trim the freed blocks                            |
| What may remain                                                     |
|  ! 2 backups made on 3 Nov and 10 Nov (Backup) hold a copy          |
|  ! Flash storage may keep old blocks until it reuses them; this     |
|    cannot be forced from software                                   |
| This folder is not encrypted. Inside an encrypted folder, erase     |
| would be certain (the key is destroyed).        [What this means…]  |
| [x] Remove the 3 snapshots     Type ERASE to confirm [_______]      |
|                                         [Cancel] [Erase for good]   |
+---------------------------------------------------------------------+
```

**Encrypt options dialog**

```
+---------------------------------------------------------------------+
| Encrypt 3 items                                                     |
| For   (•) People or my keys   [Sam ✔] [Me] [+]                      |
|       ( ) A passphrase                                              |
| Format [ age v] ( .age )     [ ] Text armour   [ ] Hide file names  |
| Output (•) one file per item  ( ) one packed file                   |
| Also sign with  [ none v]                                           |
| Originals  (•) keep  ( ) move to trash  ( ) erase…                  |
|                                          [Cancel] [Encrypt]         |
+---------------------------------------------------------------------+
```

### 5.4 Menus

```
File      New Encrypted Folder… Ctrl+N · New Container… Ctrl+Shift+N · Open Container… Ctrl+O ·
          Add Files… Ctrl+Shift+O · Print ▸ (Recovery Sheet, Report) Ctrl+P · Lock All Ctrl+Shift+Delete ·
          Close Ctrl+W · Quit Ctrl+Q
Edit      Copy Ctrl+C · Copy Hash Ctrl+Shift+C · Paste Expected Hash Ctrl+Shift+V · Find Ctrl+F · Preferences Ctrl+,
Actions   Encrypt… Ctrl+E · Decrypt Ctrl+D · Sign… Ctrl+Shift+S · Verify Ctrl+Shift+Y ·
          Compute Checksums Ctrl+M · Verify Checksum List… · Inspect Metadata Ctrl+I ·
          Clean Metadata… Ctrl+Shift+I · Erase… Ctrl+Shift+E · Check for Indicators
Container Unlock… Ctrl+U · Lock Ctrl+L · Open in Files Ctrl+Return · Change Key… · Add Key Slot… ·
          Compact · Recovery Sheet… · Destroy…
View      Home Ctrl+1 · Encrypted Folders Ctrl+2 · Containers Ctrl+3 · Encrypt and Decrypt Ctrl+4 ·
          Sign and Verify Ctrl+5 · Checksums Ctrl+6 · Metadata Ctrl+7 · Erase Ctrl+8 · Activity Ctrl+9 ·
          Sidebar F9 · Inspector F4 · Jobs Ctrl+J · Refresh F5
Help      Vault Help F1 · What Erase Can and Cannot Do · About Hidden Volumes · Container Format ·
          Keyboard Shortcuts · About
```

### 5.5 Keyboard and pointer

- App shortcuts beyond the reserved set: `Ctrl+E` encrypt, `Ctrl+D` decrypt, `Ctrl+Shift+S` sign, `Ctrl+Shift+Y` verify, `Ctrl+M` checksums, `Ctrl+I` inspect metadata, `Ctrl+Shift+E` erase, `Ctrl+U` unlock, `Ctrl+L` lock, `Ctrl+Shift+Delete` lock all, `Ctrl+J` jobs.
- Context menu, folder or container: Unlock/Lock, Open in Files, Change key…, Add key slot…, Properties, Recovery sheet…, Destroy….
- Context menu, job or result: Show output in Files, Copy hash, Copy report, Run again, Remove from activity.
- Context menu, metadata field: Keep, Remove, Copy value, Apply to all files.
- Context menu in SY02 (registered by Vault): Encrypt…, Decrypt, Sign…, Verify, Checksum…, Inspect metadata, Clean metadata…, Erase…, Add to container ▸.
- Drag and drop: drop files anywhere to get proposed actions; drop a file plus its signature to verify at once; drop a checksum list to verify its folder; drag files onto a container to add them; drag a result's output file out to another app. Dropping onto a locked container asks to unlock first.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Home shows the drop zone and "Create your first encrypted folder"; lists show one line and one button |
| Loading | Jobs show determinate progress; hashing shows bytes per second; unlock shows key derivation progress |
| Error | Wrong key: inline, with which slots were tried. Damaged container: list of affected files, "Open read-only" |
| Offline | No effect; nothing needs the network |
| Locked | Folder and container detail shows only non-secret properties; contents are not listed |
| Read-only | Containers on write-protected media: banner, write actions disabled |
| Busy at lock | "Writer has 1 file open": Close apps and lock, Lock when free, Cancel |
| Keys locked | SE04 locked: key actions queue behind its unlock prompt |
| Media removed | Container vanished while open: writes fail safe, the container is marked "needs check" on next open |
| Narrow | Sidebar overlay; list → detail navigation; job strip becomes a status-bar popover |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar | `listView` |
| Drop zone and cards | `panel` |
| Lists (folders, containers, files, jobs, results) | `listView`, `modelListView` |
| Tables (metadata fields, checksum list results, slots) | `tableView` |
| Forms | `lineEdit`, `comboBox`, `checkBox`, `radioButton`, `spinBox`, `toggleSwitch` |
| Progress | `progressBar` |
| Wizards and confirmations | `dialog` |
| Explainers | `markdownView` |
| Entropy strip | `chart` |
| Recipient picker | provided by SE04 through the `pick` intent |
| Passphrase entry | design-system secret field (SE04-GAP-1) |

Missing:

- **SE05-GAP-1**: drop target that accepts portal handles for many files and folders at once with a typed offer list (FUSE-WP-09, FUSE-WP-33).
- **SE05-GAP-2**: before-and-after two-column field diff (can be built from `tableView`; a diff widget, FUSE-WP-38, would be better).
- Shares SE04-GAP-1 (secure text field).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Action proposals, job orchestration, view models, reports |
| UI | FUSE UI | app host | Pages, dialogs |
| Storage helper | C++23 | system helper of `svc.storage` scope, privileged for dataset operations only | Create, mount, unmount, re-key and destroy the user's encrypted datasets under a delegated parent; snapshot listing and removal for erase; trim requests |
| Container service | C++23 | per-user helper; holds unwrapped container keys (PLATFORM.md section 2 rule c) | Reads and writes `.pbsdbox`; serves contents as a portal location; optional mount through the kernel user-space file system interface |
| Crypto worker | C++23 | helper per job, capability mode, handles for input and output only | Streaming age encryption and decryption, hashing; receives the file key from SE04's key service, never a private key |
| Signature checker | C++23 | helper, capability mode | Parses signature files (FORGE, minisign, signify, SSH signature); OpenPGP and X.509 checks are delegated to SE04's key service |
| Metadata helpers | C++23 | one helper per format family (image, PDF, office, audio-video, archive), capability mode, input and output handles only | Parse hostile files, list fields, write cleaned copies |
| Indicator helper | C++23 | helper, capability mode | Appended-data, structure and statistical checks |
| Erase planner | ForgeScript + storage helper queries | app host | Finds snapshots, backups, traces; builds the summary; runs the steps |
| `vaultctl` | ForgeScript | command line | Same job interface |

```
 Files (SY02) context action / drop / open-with
        │ handles
        ▼
 Vault app (ForgeScript + FUSE UI) ──jobs──▶ crypto worker ─────────┐ file key (wrapped/unwrapped)
        │            │           └──────▶ signature checker ────────┤
        │            ├──────────────────▶ metadata helpers          ▼
        │            └──────────────────▶ indicator helper     SE04 key service
        │                                                     (confirm on trusted path)
        ├── org.pbsd.vault-storage.v1 ──▶ storage helper ──▶ ZFS encrypted datasets, snapshots, trim
        └── container service ──▶ .pbsdbox file          ──▶ file portal location / optional mount
 svc.audit ◀── container and erase events          svc.backup, svc.search, svc.thumbnail ◀── trace cleanup
```

### 6.2 Data model

App store: `~/.local/share/org.pbsd.vault/vault.db` (SQLite; no secrets).

| Entity | Fields |
|---|---|
| `enc_folder` | `id` UUID, `name`, `dataset` text, `mountpoint`, `key_policy` JSON (slot references into SE04), `auto_lock` JSON {`idle_min`, `on_session_lock`, `on_suspend`, `on_token_removed`}, `auto_unlock_login` bool, `index` bool, `thumbnails` enum(inside, none), `ask_every_time` bool, `created_ts` |
| `container_ref` | `id` UUID (from the file header), `label`, `last_path`, `last_opened_ts`, `read_only_default` bool, `size_bytes`, `slot_summary` text |
| `job` | `id`, `kind` enum(encrypt, decrypt, sign, verify, hash, verify_list, inspect, clean, erase, indicator, compact), `state` enum(queued, running, paused, done, failed, cancelled), `inputs` JSON (portal tokens, names), `outputs` JSON, `started_ts`, `ended_ts`, `bytes_total`, `bytes_done`, `result` JSON |
| `verify_result` | `job_id`, `file_blake3` blob32, `format` enum(forge, openpgp, sshsig, minisign, signify, cms), `valid` bool, `signer_fpr`, `signer_name`, `trust` enum(system, user_verified, known, unknown), `covered` text, `ts` |
| `provenance` (written as an extended attribute on the file, schema shared with IN03 and SY02) | `source_url`, `fetched_ts`, `hash_algo`, `hash`, `verified` enum(none, hash, signature), `signer_fpr` |
| `clean_preset` | `name`, `rules` JSON (group → keep or remove) |
| `erase_report` | `job_id`, `target`, `steps[]` {`action`, `outcome`}, `remaining[]` text |

**Portable container `.pbsdbox`, format version 1:**

| Part | Content |
|---|---|
| Header (plain, authenticated) | magic `PBSDBOX1`, `format_version` u16, `container_id` UUID, `chunk_size` u32 (default 64 KiB), `cipher` xchacha20-poly1305, `kdf` params (Argon2id), `key_slots[]` (same slot kinds and policy groups as `.pbsdvault` in SE04), `header_mac` |
| Chunk area | Fixed-size sealed chunks; nonce derived from chunk index and a per-write counter; a chunk is never rewritten in place (new version appended, map updated) |
| Chunk map | Sealed B-tree of logical chunk → file offset, with a root pointer written last (two root slots, alternate, with sequence numbers) so a torn write falls back to the previous root |
| Directory tree | Sealed records: `inode` u64, `parent` u64, `name`, `kind` enum(file, dir, symlink), `size`, `mode`, `mtime`, `xattrs`, `chunks[]` |
| Free list | For compaction |

File encryption output is standard age v1; Vault adds nothing to the format. Signatures default to FORGE's detached `.sig`. Checksum files follow the two common text layouts. Schema versions are in every store; forward-only migration with a copy kept.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| age v1 (binary and armoured) | yes | yes | Own; primitives from FORGE-WP-22 | crypto worker (stream); key wrapping in SE04 key service |
| OpenPGP messages and detached signatures | yes | yes | SE04 key service (its implementation choice) | SE04 parse helper and key service |
| FORGE `.sig` and signed manifests | yes | yes | FORGE signing code (PLATFORM.md V14) | signature checker; signing in SE04 key service |
| minisign, signify | yes | no | Own (both are small Ed25519 formats) | signature checker |
| SSH signatures (`sshsig`) | yes | yes | Own framing; key operations in SE04 | signature checker |
| CMS detached signatures (X.509) | yes | no | SE04 | SE04 parse helper |
| BLAKE3, SHA-2, SHA-3, SHA-1, MD5 | — | — | FORGE-WP-22 primitives; BLAKE3 as in FORGE | crypto worker |
| Checksum lists (GNU and BSD layouts) | yes | yes | Own | app host (plain text, bounded line length) |
| `.pbsdbox` | yes | yes | Own | container service |
| ZFS encrypted datasets, raw send streams | — | — | OpenZFS through the storage helper | kernel; storage helper |
| GELI file-backed containers | yes | yes | GELI and memory-disk tools in the reference tree | storage helper |
| VeraCrypt, LUKS2 (read-only, Phase 3) | yes | no | Own header parsers; standard ciphers | container service |
| JPEG, PNG, WebP, TIFF, HEIF, AVIF, GIF metadata (EXIF, XMP, IPTC, ICC) | yes | yes | Own container walkers (no pixel decode needed) | image metadata helper |
| PDF structure | yes | yes | Own PDF object parser for metadata and rewrite; shared with OF03's parser if that exists first | PDF metadata helper |
| ODF, OOXML (zip + XML) | yes | yes | Own zip and XML readers | office metadata helper |
| ID3, Vorbis comments, FLAC, MP4 atoms, Matroska tags | yes | yes | Own | audio-video metadata helper |
| zip, tar headers | yes | yes | Own for headers; `libarchive` (BSD-2-Clause, in the reference tree) only if own code slips | archive metadata helper |
| tar stream for folder encryption | yes | yes | Own minimal writer and reader (ustar + pax) | crypto worker |

### 6.4 Integration

- Intents: handles `open` (`.age`, `.pbsdbox`, `.sig`, `.minisig`, `.asc`, `.pgp`, checksum lists), `encrypt`, `decrypt`, `sign`, `verify`, `hash`, `clean-metadata`, `erase` (new intent names registered by this app and offered in the SY02 context menu and the share sheet). Emits `pick` (keys, to SE04), `open` (outputs in SY02, hand-offs to UT01, AD02, DV04, SE01), `print`, `share`.
- MIME: `application/age`, `application/vnd.pbsd.box`, `application/pgp-encrypted`, `application/pgp-signature`, `application/vnd.forge.signature`, `text/x-checksum`.
- Services used: `svc.keyring` (through SE04's `org.pbsd.keys.v1`), `svc.portal.file`, `svc.audit`, `svc.notify`, `svc.settings`, `svc.backup` (list backups holding a file; inclusion rules), `svc.search` and `svc.thumbnail` (trace cleanup, exclusion), `svc.undo-trash`, `svc.power` (inhibit sleep during long jobs), `svc.portal.print`.
- Interfaces offered: `org.pbsd.vault-jobs.v1` (`Submit(kind, handles, options)`, `Status`, `Cancel`) used by SY02, IN03, IN01 and `vaultctl`; a portal provider for open containers.
- Other apps: SE04 (all keys and confirmations, recipient picker, passphrase storage, shares for recovery sheets); SY02 (context actions, provenance display, snapshot browser); IN03 and IN01 (download verification hand-off); AD02 (device erase, image writing); AD03 (backup awareness, raw encrypted sends); UT01 (encrypted archives); SE01 (scan hand-off, swap encryption state); SH02 (unlocked indicator); SH04 (share-sheet "clean before sharing"); OF03 and ME01 (may call `clean-metadata` on export).

## 7. Security and privacy

Capabilities requested by the app:

| Capability | Reason |
|---|---|
| `fs.app` | Job history and settings |
| `fs.portal` | Every file the user acts on arrives as a portal handle |
| `net.none` | No network. Verification material comes with the file or from the user |
| `svc.keyring` | Key operations by reference through SE04's key service |
| `svc.audit`, `svc.notify`, `svc.settings` | Records, job notices, preferences |
| `svc.backup`, `svc.search`, `svc.thumbnail`, `svc.undo-trash` | Erase summary and trace cleanup |
| `svc.power` | Keep the machine awake during long jobs |
| `svc.portal.file`, `svc.portal.print` | Files, printed sheets |
| `svc.storage` | Ask the storage helper for dataset operations (new service scope, section 13) |
| `proc.helper:crypto-worker`, `proc.helper:signature-checker`, `proc.helper:metadata-image`, `proc.helper:metadata-pdf`, `proc.helper:metadata-office`, `proc.helper:metadata-av`, `proc.helper:metadata-archive`, `proc.helper:indicator`, `proc.helper:container-service` | Hostile-input parsing and key-holding work outside the app |
| `bg.run` | Jobs and unlocked containers outlive the window |

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| Data at rest in a locked folder or container | Thief, forensic examiner | Read the disk or the file | AES-256-GCM (ZFS) or XChaCha20-Poly1305 (container); keys wrapped by SE04 slots; no plaintext names in containers |
| Data while unlocked | Malicious app | Read files of an unlocked folder | Apps still need portal grants; "ask every time" folders; auto-lock; visible unlocked indicator |
| Container keys in memory | Malware with user rights | Read the container service | Separate process, capability mode, no debug attach, zero on lock |
| The user's keys | Vault itself being compromised | Steal private keys | Vault never has them; SE04 confirms each use on the trusted path and shows what is being decrypted or signed |
| Vault's parsers | Hostile files (images, PDFs, signatures, containers) | Exploit a parser | One sandboxed helper per format family with only input and output handles; fuzzing; size and depth limits |
| The user's belief about erasure | The system's own design (copy-on-write, flash, snapshots, backups) | Data survives a "secure delete" | The erase summary checks for surviving copies and says what remains; crypto-erase is recommended; no fake overwrite passes |
| Verification verdicts | Attacker supplying file, hash and signature together | "Valid" for an attacker's key; hash from the same compromised site | Verdict separates "signature is mathematically valid" from "signer is trusted"; a hash from the same source as the file is labelled as integrity only, not authenticity |
| Metadata cleaning claims | Unknown or new metadata fields | Residual identifying data | Re-inspection after cleaning; refuses to say "clean" for unknown structures; content warnings |
| Portable container on untrusted media | Tampering between uses | Roll back to an old state, corrupt chunks | Per-chunk authentication; root sequence numbers; last-seen sequence stored in SE04; a lower one warns |
| Traces of sensitive files | Local examiner | Thumbnails, recents, index, swap | Policies per encrypted folder; trace cleanup on erase; swap state shown |
| Coercion | Someone who can compel the user | Demand the passphrase | Not solved. Section 7.1 explains why hidden volumes are not offered as a fix |

### 7.1 Hidden volumes: honest limits

A hidden volume claims that a second secret area cannot be shown to exist. On this system the claim is weak, for reasons that software inside the container cannot fix:

1. **The outer system remembers.** Recent-file lists, thumbnails, search indexes, audit records, application histories and snapshots outside the container can name files that are not in the visible part.
2. **Copy-on-write and snapshots keep old versions** of the container file. Comparing versions shows writes to areas the visible volume says are free.
3. **Flash storage keeps stale blocks** with the same effect, below the file system.
4. **Backups** multiply the versions available for comparison.
5. **The tool is the tell.** A system that ships a hidden-volume feature gives every container the suspicion; an adversary who can compel one passphrase can keep asking.
6. **Safe use needs discipline** (never mount the outer volume read-write without the hidden key, never open hidden files with ordinary apps) that people do not keep.

Decision: no hidden volumes in phases 1 and 2. A Phase 3 experiment is allowed only for portable containers on removable media, off by default, shown after the explainer, and only if the leak audit in section 11 finds no trace in the outer system during scripted use. What Vault offers instead: crypto-erase, containers kept off the machine, and travel mode in SE04.

Stored: job history without contents (can be turned off), container references, presets. Leaves the machine: nothing. Vault has no network capability.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `folders.parent_dataset` | text | per-user delegated dataset | Where encrypted folders are created |
| `folders.auto_lock_idle_min` | int | 15 | Default idle lock for new folders |
| `folders.lock_on_session_lock` | bool | true | Default |
| `folders.lock_on_suspend` | bool | true | Default |
| `folders.index_by_default` | bool | false | Search indexing of contents |
| `folders.thumbnails` | enum | inside | inside, none |
| `container.chunk_kib` | int | 64 | Chunk size for new containers |
| `container.kdf_mem_kib` | int | 262144 | Argon2id memory for passphrase slots |
| `container.pad_sizes` | bool | false | Pad container size to buckets |
| `container.mount_fs` | bool | false | Also mount through the kernel user-space file system interface |
| `encrypt.format` | enum | age | age, openpgp |
| `encrypt.armour` | bool | false | Text armour |
| `encrypt.hide_names` | bool | false | Random output names |
| `encrypt.after` | enum | keep | keep, trash, erase |
| `sign.format` | enum | forge | forge, openpgp, sshsig |
| `sign.default_key` | key ref | none | Default signing key in SE04 |
| `hash.default` | list | blake3, sha256 | Algorithms computed by default |
| `hash.show_legacy` | bool | false | Offer SHA-1 and MD5 |
| `hash.clipboard_offer` | bool | true | Offer compare when the clipboard holds a hash (window focused only) |
| `verify.save_provenance` | bool | true | Write the result as provenance on the file |
| `clean.preset` | text | remove_all | Default cleaning preset |
| `clean.output` | enum | copy | copy, in_place |
| `clean.reset_times` | bool | false | Reset file times on cleaned copies |
| `erase.remove_snapshots_default` | bool | false | Pre-tick snapshot removal |
| `erase.trim` | bool | true | Request trim after erase |
| `erase.require_typed_confirm` | bool | true | Type ERASE to confirm |
| `indicators.enabled` | bool | false | Show the Indicators page |
| `activity.keep_history` | bool | true | Keep the job history |
| `activity.retention_days` | int | 30 | History retention |
| `jobs.threads` | int | 0 | 0 means automatic |

## 9. Accessibility and languages

- Verification verdicts are words first ("Valid", "Not valid", "Valid but signer unknown", "Cannot check"), announced in a live region with the signer's name.
- Hashes are read in groups of four; "matches" and "does not match" are spoken; a difference view marks the first differing group for screen readers as text.
- The erase summary is a structured list (can do, may remain), not a paragraph, so it reads cleanly item by item.
- The typed confirmation word is localised and shown in the prompt; it is never case-sensitive.
- Metadata field names are translated; raw tag names are available with `Alt` held or in the inspector.
- Recovery sheets print in a large monospace face with per-line check values.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| First frame | under 300 ms cold |
| BLAKE3 hashing | at least 2 GiB/s on the reference machine for files in the page cache; storage-bound otherwise |
| SHA-256 hashing | at least 500 MiB/s per core with hardware support |
| age encryption and decryption | at least 800 MiB/s per stream; constant memory under 64 MiB per job |
| Container read and write | within 20% of the raw file-copy rate on the same medium |
| Folder unlock | under 1.5 s including key derivation at default cost |
| Metadata inspect | 1 000 photos in under 5 s |
| Metadata clean | 1 000 photos in under 30 s (no re-encode) |
| Erase summary | under 3 s for a file with up to 500 snapshots to check |
| Largest input | Single files up to 16 EiB by design (64-bit sizes); tested to 1 TiB. Containers tested to 1 TiB and 10 million entries |
| Memory | App under 100 MiB; each helper under 128 MiB |

## 11. Testing and proof

Test plan (targets; nothing has run):

- **Unit**: age against its published test vectors; signature formats against files made by their reference tools; checksum-list parsing for both layouts; slot policy logic.
- **Differential**: age files exchanged both ways with an independent implementation; minisign and signify verification against reference outputs; metadata fields listed by Vault compared with an independent metadata reader over a corpus; cleaned files re-read by that reader must show none of the removed fields.
- **Lossless check**: for cleaned images and audio, decoded pixels or samples are bit-identical to the original's.
- **Golden**: `.fuseui` renders of every page and of each verdict variant.
- **Scripted UI**: the 14 top tasks through the a11y tree, including SY02 context actions.
- **Fuzz**: every metadata helper, the signature checker, the `.pbsdbox` reader, the tar reader, checksum lists, foreign container headers.
- **Crash and tamper tests on containers**: power cut at every write step (previous root must load); flipped bits in any chunk (reported, rest readable); roll-back to an older file (warned).
- **Erase tests**: after each erase path, search the pool with a known marker pattern on a test pool and record honestly where the marker is still found; the summary shown to the user must list every place it was found.
- **Leak audit (for any hidden-region experiment)**: scripted use, then a search of the outer system (recents, thumbnails, index, audit log, snapshots) for any trace; the feature ships only with zero traces.
- **Lock tests**: lock with open files, on suspend, on token removal; no plaintext remains mounted.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Container root update | After any crash the container opens at the last complete root; no chunk is rewritten in place | T1 | TLA+ model (Apalache) of the two-root protocol; ESBMC on the commit function | Storage device honouring flush is assumed |
| Key-slot policy | The container key is released only when a full policy group is met | T1 | Shared proof with SE04's slot policy | none known |
| Verdict logic | "Valid and trusted" is shown only when the signature checks and the signer's trust is system or user-verified | T1 | Pure function over check results; PRISM | Correctness of the underlying checks is separate |
| Erase planner | Every surviving copy found by the queries appears in the summary | T2 | Contracts between query results and summary items; tests with planted copies | Completeness of the queries cannot be proved (unknown copies) |
| Crypto worker | No runtime errors; output is removed on failure; constant memory | T2 | Contracts, bounded proof, fuzzing | Primitive correctness is by vectors (T3) |
| Metadata helpers | No runtime errors on any input; pixel and sample data unchanged | T2 for memory safety, T3 for lossless | Safe subset, fuzzing, bit-identity tests | Format breadth |
| Signature checker | No runtime errors; minisign and signify verification correct | T2 | Contracts, reference vectors | none known |
| Storage helper | Acts only on datasets under the user's delegated parent | T2 | Contracts on every dataset name; ESBMC | ZFS tooling trusted |
| Indicator helper | Memory safety | T3 | Fuzzing, sanitizers | Statistical claims are heuristics |
| App logic | Safe subset | T3 | ForgeScript checker, unit tests | Interpreter not verified |

## 12. Build plan

**Phase 1 — usable core**

| ID | Work item | Depends on |
|---|---|---|
| SE05-W01 | Job interface `org.pbsd.vault-jobs.v1`, job store, crypto worker skeleton | G-08, FORGE-WP-25, FORGE-WP-26, FORGE-WP-27, FORGE-WP-28 |
| SE05-W02 | Hashing and checksum files; compare with expected | FORGE-WP-22 |
| SE05-W03 | age encryption and decryption with keys by reference | SE04-W17 (age in the key service), SE04-W09 |
| SE05-W04 | Sign and verify: FORGE format, minisign, signify; verdict logic | SE04-W18, FORGE-WP-33 |
| SE05-W05 | Storage helper and encrypted folders: create, unlock, lock, change key, destroy | ZFS native encryption on PBSD, delegated datasets, new `svc.storage` scope (with AD02) |
| SE05-W06 | Erase summary, crypto-erase, basic delete with trace list | SE05-W05, `svc.backup`, `svc.undo-trash` |
| SE05-W07 | App: Home with drop zone, folders, encrypt/decrypt, sign/verify, checksums, jobs | G-01, G-02, G-07, FUSE-WP-12, FUSE-WP-13, SE05-GAP-1 |
| SE05-W08 | SY02 context actions and open-with handlers | SY02, SH04 |
| SE05-W09 | Unlocked indicator, lock all, audit records | SH02, FORGE-WP-11 |

**Phase 2 — full feature**

| ID | Work item | Depends on |
|---|---|---|
| SE05-W10 | `.pbsdbox` format specification, container service, portal provider | SE05-W01, SE04 slot code |
| SE05-W11 | Container pages and wizard, recovery sheets | SE05-W10, G-16 |
| SE05-W12 | Metadata helpers: images, PDF, office, audio; inspect and clean; presets; before and after | FORGE-WP-28 |
| SE05-W13 | Snapshot-aware delete, trace cleanup with `svc.search` and `svc.thumbnail`, trim, erase report | SE05-W06 |
| SE05-W14 | OpenPGP and SSH signature formats; signed checksum lists; folder manifests; batch verify | SE04-W16 |
| SE05-W15 | Download verification hand-off and provenance attribute | IN03, IN01, SY02 |
| SE05-W16 | Per-folder policies: indexing, thumbnails, ask every time, snapshots, backup | SE05-W05, AD03 |
| SE05-W17 | Decrypt to a disposable view; text and clipboard; self-contained share | SH04 disposable sandbox, SE05-W03 |
| SE05-W18 | Hidden-volume explainer; `vaultctl`; activity page | SE05-W07 |

**Phase 3 — advanced**

| ID | Work item | Depends on |
|---|---|---|
| SE05-W19 | Container mount through the kernel user-space file system interface; compaction; padding; re-key | SE05-W10, that interface on PBSD |
| SE05-W20 | Metadata: video, archives, other; share-sheet hook; field editing | SE05-W12 |
| SE05-W21 | Indicators page and helper | SE05-W12 |
| SE05-W22 | GELI file-backed containers; foreign containers read-only | SE05-W05, AD02 |
| SE05-W23 | Published format documents and the stand-alone reader | SE05-W10 |
| SE05-W24 | Hybrid post-quantum recipients | SE04-W26 |
| SE05-W25 | Raw encrypted send; timestamp statements; counter-signatures; Files badge | AD03, SE05-W14 |
| SE05-W26 | Hidden-region experiment, only if the leak audit passes | SE05-W10, section 11 leak audit |
| SE05-W27 | Free-space overwrite; scheduled purge | SE05-W13 |

## 13. Open questions and risks

1. **Unprivileged encrypted datasets.** Creating, mounting and keying ZFS datasets from a user app needs a privileged helper and delegated permissions. The service scope (`svc.storage`) is not in PLATFORM.md section 8 and overlaps AD02; one owner must be chosen.
2. **Secure erase cannot be made certain** on copy-on-write and flash storage outside encrypted folders. The design tells the truth instead; some users will want a "shred" button anyway. The answer stays no.
3. Completeness of the erase summary: Vault can only list copies it can discover. Copies made by the user by hand or by other tools are unknown. The wording says "found", never "all".
4. The portable container format is new. A custom format is a risk to data longevity; mitigation is a published specification, a stand-alone reader and test vectors (SE05-W23). The alternative, a block container with a foreign file system, loses per-chunk authentication and sync-friendliness.
5. Serving a container as a portal location needs the file portal to accept third-party providers; whether `svc.portal.file` allows that is undefined (possible new platform gap).
6. The kernel user-space file system interface is "direct import" in the plan, not yet present in PBSD; mounting is Phase 3 for that reason.
7. PDF and office metadata cleaning is deep; incomplete cleaning is worse than none if it is called clean. The "refuse to claim clean" rule and re-inspection are the guard.
8. Provenance extended attributes are a privacy cost themselves (they record where a file came from). They are dropped by the cleaner and can be turned off.
9. OpenPGP support depends on SE04's undecided implementation choice.
10. Steganography indicators have high false-positive and false-negative rates. The page is off by default and worded as indicators only.
11. Hidden volumes: the leak audit may never pass on a desktop with search, thumbnails and snapshots. If so, SE05-W26 is dropped and the explainer stays.
12. New intent names (`encrypt`, `decrypt`, `sign`, `verify`, `hash`, `clean-metadata`, `erase`) are not in PLATFORM.md section 9; they need registering.
