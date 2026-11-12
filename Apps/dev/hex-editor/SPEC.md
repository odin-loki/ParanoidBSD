<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Hex Editor

By Odin Loch

| | |
|---|---|
| ID | `DV04` (from CATALOGUE.md) |
| Bundle | `org.pbsd.hex-editor` |
| Category | dev |
| Priority | P2 |
| Backing repos | none (FUSE and FORGE as platform; disassembly through DV07's engine) |
| Status | Planned. Nothing built. |

## 1. Purpose

A binary editor. Developers, reverse engineers, forensic analysts and administrators use it to inspect and change bytes in files of any size, in block devices and in process memory, to decode structures with templates, to compare binaries and to carve embedded files. It replaces Okteta from the Plasma plan (PLATFORM.md V20).

Everything it opens is hostile data. Decoding runs in sandboxed helpers. Devices and process memory are reached only through explicit grants and are read-only unless the user unlocks writing.

## 2. Scope

In scope: huge files and block devices; hex, decimal, octal, binary and text views; data inspector; structure templates in a declarative binary grammar language defined here; bookmarks and annotations; search and replace; insert and overwrite editing with undo; binary diff; entropy and byte-histogram views; checksums and hashes; strings extraction; disassembly pane; process memory view; patch files; ForgeScript scripting; file carving.

Out of scope:

| Item | Owner |
|---|---|
| Decompilation, control-flow graphs, cross references, symbol recovery | DV07 Reverse Engineering |
| Text editing of files that turn out to be text | OF01 Text Editor |
| Partitioning, formatting, imaging, SMART, pool management | AD02 Disks |
| Debugging a process (breakpoints, stepping) | DV01 IDE |
| Packet dissection (this app shows payload bytes handed over) | DV05 Packet Analyser |
| Archive browsing and extraction | UT01 Archives |
| Secure erase, file encryption | SE05 Vault |
| Malware scanning and verdicts | SE01 Security Centre |

## 3. Users and top tasks

Personas:

- **Developer**: checks a file format the app writes, patches a constant, compares two builds.
- **Reverse engineer**: maps an unknown format with a template, marks regions, hands code to DV07.
- **Forensic analyst**: reads a disk image or device read-only, carves files, hashes ranges, keeps notes.
- **Administrator**: inspects a boot sector or a corrupt file header.

| # | Task | Target steps |
|---|---|---|
| T1 | Open a 100 GiB file and see the first screen | 1 |
| T2 | Go to an offset | 2 (`Ctrl+L`, type) |
| T3 | Read the value under the caret as every common type | 0 (inspector is live) |
| T4 | Find a byte pattern with wildcards | 2 (`Ctrl+F`, type `DE AD ?? EF`) |
| T5 | Overwrite bytes and save | type, `Ctrl+S` |
| T6 | Apply a structure template and click through its fields | 2 |
| T7 | Compare two binaries and step through differences | 2 (open compare, `F7`) |
| T8 | Hash a selection with SHA-256 | 2 |
| T9 | See where the compressed or encrypted regions are | 1 (entropy view) |
| T10 | Extract all strings and jump to one | 2 |
| T11 | Carve embedded PNG and ZIP files out of a firmware image | 3 |
| T12 | Save changes as a patch file instead of writing the original | 2 |
| T13 | View a process's memory map and read a region | 3 (pick process, approve prompt, pick region) |
| T14 | Send a code region to DV07 | 1 (context menu) |

## 4. Features

Priority: M must, S should, C could. Phase 1, 2, 3 as in section 12.

### 4.1 Sources: files, devices, memory

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV04-F001 | Open file | `Ctrl+O` through the file portal; `open` intent for any type (`application/octet-stream` and "open as binary" from other apps). Read-only handle by default; "Enable editing" asks the portal for a write handle to that one file. | M | 1 |
| DV04-F002 | Huge files | Up to 2^63 − 1 bytes. The file is never read whole. First screen appears before any scan. All offsets are 64-bit. | M | 1 |
| DV04-F003 | Piece-tree buffer | Edits are a tree of pieces over the original file and an append-only add buffer, in the byte engine helper. Insert, delete and overwrite cost O(log pieces), independent of file size. | M | 1 |
| DV04-F004 | New buffer | `Ctrl+N` creates an empty buffer; "New from clipboard", "New from selection", "New filled" (size and fill pattern). | M | 1 |
| DV04-F005 | Block devices | "Open Device…" asks the device portal (section 13, DV04-GAP-3) for a device the user picks. The handle is read-only. The title bar shows device name, size, sector size and a `DEVICE · READ-ONLY` chip. | S | 2 |
| DV04-F006 | Device write unlock | Writing to a device needs a second, separate grant with a shell prompt that names the device and says the change cannot be undone; mounted devices and the boot pool are refused. Writes go through in whole sectors with read-back verify. | C | 3 |
| DV04-F007 | Device details | Partition table (MBR, GPT) decoded by a built-in template with jump links; sector navigation (`Alt+PageUp/Down` by sector); bad-read sectors shown hatched with the error. | S | 2 |
| DV04-F008 | Disk images | Raw images open as files. Sparse regions are shown as zero-fill with a "hole" tint. | S | 2 |
| DV04-F009 | Process memory | "Open Process Memory…" asks the debug broker (DV04-GAP-4) for a process the user picks; the shell shows the prompt. Read-only by default. Only processes of the same user that are not marked non-inspectable. | S | 3 |
| DV04-F010 | Memory map | Regions with start, end, size, protection (r, w, x), backing object; unmapped gaps are skipped, not shown as zeros. Jump to region; filter by protection. | S | 3 |
| DV04-F011 | Memory refresh | Manual (`F5`) or timed (0.5 to 10 s); changed bytes since the last refresh are highlighted; "freeze snapshot" copies a region to a file buffer. | S | 3 |
| DV04-F012 | Memory write | Off unless the broker grants write for that process (a second prompt). Each write is logged to `svc.audit`. | C | 3 |
| DV04-F013 | Bytes from other apps | Accepts a byte range by intent from DV05 (packet payload), DV07 (section), DV01 (debugger memory), UT01 (archive member), as a read-only shared-memory buffer with a source label. | M | 2 |
| DV04-F014 | Recent | Last 50 sources with kind (file, device, process) and a "missing" flag. Devices and processes are listed by name only and always re-prompt. | M | 1 |
| DV04-F015 | External change | The file handle is watched. If the file changes under an unmodified buffer it reloads; under a modified buffer the app freezes it read-only and offers "Save my edits as a patch". | M | 1 |
| DV04-F016 | Session restore | Open sources (files only), caret, selection, view settings, bookmarks, applied templates and unsaved edit journals are restored. | M | 1 |
| DV04-F017 | Provenance and disposable open | Files tagged "untrusted" open read-only with a banner; helpers already have no rights, so this only guards against accidental writes and marks exports with the same tag. | S | 2 |

### 4.2 Views

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV04-F020 | Hex view | Offset column, byte columns, text column. Bytes per row: fixed 8, 16, 24, 32, 48, 64, any number, or fit to width. Group size 1, 2, 4, 8, 16 with spacing. | M | 1 |
| DV04-F021 | Number bases | Byte column as hex (upper or lower case), decimal, octal or binary (8 bits per byte). | M | 1 |
| DV04-F022 | Value views | Treat the data as a grid of fixed-width values: u8 to u64, i8 to i64, f16, f32, f64, with endianness; useful for tables of numbers. | S | 2 |
| DV04-F023 | Text column encodings | ASCII, Latin-1, UTF-8 (multi-byte sequences drawn over their bytes), UTF-16LE/BE, UTF-32, EBCDIC (code pages 037, 500, 1047), windows-125x, KOI8, Shift_JIS, EUC, GBK, Big5 from the platform text library; non-printable shown as `.` or a chosen glyph. | M | 1 |
| DV04-F024 | Offset column | Hex, decimal or octal; absolute, relative to a mark, or as sector:offset; for memory, virtual addresses. | M | 1 |
| DV04-F025 | Split views | Up to 4 panes of the same buffer with independent position and settings; synchronised scrolling optional. | M | 1 |
| DV04-F026 | Linked selection | Selecting in the hex column selects in the text column and the reverse; the inspector, template tree and disassembly follow. | M | 1 |
| DV04-F027 | Byte colouring | Schemes: none, by class (zero, ASCII printable, whitespace, control, high), by value gradient, by annotation, by diff state. Always with a legend. | S | 1 |
| DV04-F028 | Row and column highlight | Current row and column crosshair; hovered byte highlights its pair in the other column. | S | 1 |
| DV04-F029 | Bitmap view | Draw the data as pixels: width, bits per pixel (1, 2, 4, 8, 16, 24, 32), channel order, row padding; click a pixel to jump to its byte. Helps find images and tables. | S | 3 |
| DV04-F030 | Minimap | A strip showing the whole source coloured by entropy or byte class, with the viewport, bookmarks, search hits and edits marked. | S | 2 |
| DV04-F031 | Zoom and font | `Ctrl++/-/0`; monospace face and size settings; tabular spacing. | M | 1 |
| DV04-F032 | Go to | `Ctrl+L`: absolute (`0x1F00`, `7936`), relative (`+0x40`, `-16`), from end (`$-512`), percent (`50%`), sector (`s:63`), expression (`0x400 + 4*12`), value at caret as pointer (with endianness and width). | M | 1 |
| DV04-F033 | Follow pointer | `Ctrl+Enter` on a selected 2, 4 or 8 byte value jumps to that offset (optionally plus a base); `Alt+Left` returns. | S | 2 |
| DV04-F034 | Navigation history | Back and forward through jumps (last 200). | M | 1 |
| DV04-F035 | Selection | By drag, `Shift+arrows`, `Ctrl+A`, "Select range…" (start plus length or end), "Select to mark", block from template field, multiple disjoint selections with `Ctrl+drag`. | M | 1 |
| DV04-F036 | Selection info | Status bar: start, end, length in hex and decimal, and length in sectors for devices. | M | 1 |
| DV04-F037 | Column mode | Rectangular selection across rows for tabular data. | C | 3 |

### 4.3 Data inspector

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV04-F040 | Live decode | At the caret or for the selection: binary, octal, u8, i8, u16, i16, u24, u32, i32, u64, i64, f16, f32, f64, each in little and big endian (toggle or both columns). | M | 1 |
| DV04-F041 | Variable-length integers | LEB128 unsigned and signed, protobuf varint, UTF-8 code point, with the consumed byte count. | S | 2 |
| DV04-F042 | Time values | Unix seconds (32 and 64), Unix milliseconds, Windows FILETIME, DOS date and time, HFS and Mac absolute time, OLE date, NTP timestamp, shown in UTC and local time. | S | 2 |
| DV04-F043 | Identifiers and addresses | GUID and UUID (both byte orders), IPv4, IPv6, MAC address, 4-character code, RGB and RGBA colour swatch. | S | 2 |
| DV04-F044 | Text decode | The next bytes as ASCII, UTF-8, UTF-16LE/BE strings up to a terminator. | M | 1 |
| DV04-F045 | Edit through the inspector | Typing a value in any row encodes it and overwrites the bytes (one undo step); out-of-range values are refused with the valid range. | M | 1 |
| DV04-F046 | Custom rows | Add, remove, reorder rows; save inspector layouts. | S | 2 |
| DV04-F047 | Copy value | Copy as shown, as hex, as a language literal (C array, ForgeScript list). | S | 1 |
| DV04-F048 | Bit view | The selected bytes as individual bits with bit numbers; click toggles a bit (in edit mode); bit-field extraction by start and width. | S | 2 |
| DV04-F049 | Instruction at caret | One-line disassembly of the bytes at the caret for the chosen architecture (from the disassembly engine). | S | 3 |

### 4.4 Structure templates

The language is defined in section 6.5.

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV04-F050 | Apply template | Pick a template and a start offset; the template engine parses the data and returns a tree of fields with offset, size, type and decoded value. | M | 2 |
| DV04-F051 | Template tree | Tree panel: name, value, offset, size, type, comment. Selecting a field selects its bytes; selecting bytes reveals the innermost field. | M | 2 |
| DV04-F052 | Field colouring | Fields colour the hex view by depth or by type, with field names as hover cards. | M | 2 |
| DV04-F053 | Edit through fields | Typing a value in the tree encodes it by the field's type and endianness and overwrites the bytes; the template is re-evaluated incrementally. | S | 2 |
| DV04-F054 | Automatic template choice | Templates declare `magic` patterns; on open the app lists matching templates in a banner ("Looks like PNG — Apply template"). Nothing is applied without a click. | S | 2 |
| DV04-F055 | Shipped templates | ELF, PE/COFF, Mach-O, PNG, JPEG, GIF, BMP, WebP container, ZIP, gzip, tar, zstd frame, xz, 7z header, PDF header and xref, SQLite header and page, MBR, GPT, FAT boot sector, UFS and ZFS labels (uberblock, label nvlist header), ext superblock, ISO 9660, pcap, pcapng, WAV, FLAC, MP4 boxes, Ogg page, TrueType and OpenType tables, DER/ASN.1, protobuf wire, CBOR, MessagePack, BSON, `.fpk` header, cooked `.fuseui` header, `.fsc` cache header. Each is a plain template file. | S | 2 |
| DV04-F056 | Template editor | A split with the template source (shared code editor, grammar, completion, diagnostics) and the live result; errors show the template line and the data offset. | M | 2 |
| DV04-F057 | Template library | User templates in the app data directory; project templates beside the data; import and export; each shows its signer if signed. Unsigned templates run all the same: a template has no authority (section 7). | S | 2 |
| DV04-F058 | Multiple templates | Several templates applied at different offsets at once; each is a root in the tree. | S | 2 |
| DV04-F059 | Arrays at scale | Arrays of millions of elements are evaluated lazily: the tree shows ranges (`[0..999]`) that expand on demand. | M | 2 |
| DV04-F060 | Errors are local | A failed assertion or out-of-range read marks that field as an error and parsing continues with the next sibling where the size is known. | M | 2 |
| DV04-F061 | Resource limits | Fuel (default 50 million steps), depth (256), node count (10 million, lazily materialised), time (5 s per evaluation slice); over the limit the engine stops and reports where. | M | 2 |
| DV04-F062 | Export decoded tree | As JSON, CSV or Markdown. | S | 3 |
| DV04-F063 | Template from selection | Mark fields by selecting bytes and choosing a type; the app writes the template text. | S | 3 |
| DV04-F064 | Infer structure (local AI) | Opt-in: asks `svc.model` to propose a template for the first kilobytes plus the user's notes; the result opens in the template editor as a draft, checked by the template compiler. | C | 3 |
| DV04-F065 | Checksum fields | A template can declare that a field holds a checksum of a range; the tree shows valid or invalid and offers "Fix checksum" after edits. | S | 3 |

### 4.5 Bookmarks and annotations

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV04-F070 | Bookmarks | `Ctrl+F2` toggles at the caret; `F2`/`Shift+F2` next and previous while the hex view has focus; named, with colour. | M | 1 |
| DV04-F071 | Annotations | A range with a name, colour, type hint and a Markdown note. Shown as a coloured span with a hover card; overlapping annotations nest. | M | 1 |
| DV04-F072 | Annotation list | Table: name, start, length, note, sortable and searchable; double click jumps. | M | 1 |
| DV04-F073 | Sidecar storage | Annotations and bookmarks are stored in the app store keyed by the source's content hash (first and last MiB plus size) and file identity, and can be saved beside the file as `<name>.hexnotes` (JSON) on request. Never written into the file. | M | 1 |
| DV04-F074 | Offsets follow edits | Inserting or deleting bytes before an annotation moves it; editing inside one grows or shrinks it. | M | 2 |
| DV04-F075 | Import and export | `.hexnotes` JSON; export as CSV and Markdown report; import of symbol or section lists from DV07 as annotations. | S | 2 |
| DV04-F076 | Marks | A single "mark" (`Ctrl+M`) for relative offsets and select-to-mark. | S | 1 |
| DV04-F077 | Chain of custody note | For forensic use: a per-source log of who opened it, when, source hash at open and at close, and every export made, exportable as a signed text file (signature through SE04). | C | 3 |

### 4.6 Search and replace

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV04-F080 | Byte search | Hex pattern with nibble wildcards (`DE AD ?? E?`), byte ranges (`[00-1F]`), alternatives (`(00|FF)`), repeats (`{4}`), and jumps (`.{0,16}`). | M | 1 |
| DV04-F081 | Text search | Text in a chosen encoding or "all common encodings at once" (ASCII, UTF-8, UTF-16LE/BE); case-insensitive option using Unicode folding. | M | 1 |
| DV04-F082 | Regex search | Byte-oriented regular expressions (no backreferences; linear-time automaton so a pattern cannot hang the search). | M | 2 |
| DV04-F083 | Numeric search | A value as a type: integer of a width and endianness, float with tolerance, a range (`100..200` as u16le), varints. | M | 2 |
| DV04-F084 | Scope and direction | Whole source, selection, from caret, an annotation, a memory region set; forward and backward; wrap. | M | 1 |
| DV04-F085 | Alignment | Only at offsets that are a multiple of N (plus a remainder). | S | 2 |
| DV04-F086 | Find all | Results table with offset and context bytes; streaming; cancel; results mark the minimap; "Annotate all". | M | 1 |
| DV04-F087 | Speed on huge sources | Streaming search in the byte engine with SIMD byte scanning where available; progress with rate and time left; never loads the file. | M | 1 |
| DV04-F088 | Replace | Replace current or all with bytes, text or a fill pattern. Overwrite mode needs equal length or pads; insert mode allows any length. One undo step for Replace All. | M | 2 |
| DV04-F089 | Search history and saved searches | Last 50; named searches. | S | 2 |
| DV04-F090 | Find differences from a value | "Next non-zero", "next byte not equal to this", "next change of byte class": fast skips over padding. | S | 2 |
| DV04-F091 | Cross-source search | Search all open sources. | C | 3 |
| DV04-F092 | Pointer search | Find values that point to the selected offset (given width, endianness and base). | S | 3 |

### 4.7 Editing and undo

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV04-F100 | Overwrite and insert modes | `Insert` toggles; shown as `OVR`/`INS` in the status bar. Overwrite is the default. Insert mode is off for devices and process memory. | M | 1 |
| DV04-F101 | Type in either column | Hex digits in the byte column (nibble by nibble), characters in the text column (encoded in the column's encoding). | M | 1 |
| DV04-F102 | Delete, cut | In insert mode removes bytes; in overwrite mode zero-fills (option) or is refused. | M | 1 |
| DV04-F103 | Paste | As bytes; "Paste Special": interpret clipboard text as hex, decimal list, Base64, C array, escaped string; insert or overwrite; repeat N times. | M | 1 |
| DV04-F104 | Copy as | Raw bytes, hex text (spaced or packed), C array, ForgeScript list, Base64, escaped string, hex dump with offsets, decimal list. | M | 1 |
| DV04-F105 | Fill | Selection with a byte, a pattern, zeros, random bytes (system generator), an incrementing counter. | M | 1 |
| DV04-F106 | Operations on selection | AND, OR, XOR with a key (repeating), NOT, shift and rotate (bit and byte), add and subtract with width and endianness, byte swap (2, 4, 8), reverse bytes, reverse bits. | S | 2 |
| DV04-F107 | Transform selection | Base64, hex text, URL encoding, encode and decode; decompress (zlib, gzip, zstd, xz, lz4, bzip2) into a new buffer with an output cap (default 1 GiB) through the archive helper of UT01. | S | 2 |
| DV04-F108 | Insert file, export selection | Insert another file's bytes at the caret; save the selection as a new file through the portal. | M | 1 |
| DV04-F109 | Resize | Truncate or extend to a size; extend with a fill byte. | M | 2 |
| DV04-F110 | Undo and redo | Unlimited, per buffer, persisted with the session journal; each step has a name; an undo history panel allows jumping. | M | 1 |
| DV04-F111 | Change tracking | Modified bytes are coloured (changed, inserted) and deletions leave a marker; `F7`/`Shift+F7` next and previous change; "Revert change here". | M | 1 |
| DV04-F112 | Changes panel | List of edit regions with offset, old and new length; revert any; export as a patch (4.11). | S | 2 |
| DV04-F113 | Save | In place when only overwrites were made (writes only changed ranges, with `fsync`). With inserts or deletes: writes a new file beside the original and renames, needing free space equal to the new size; the original is kept as `.bak` if the option is on. Progress and cancel. | M | 1 |
| DV04-F114 | Save as, save copy | Through the portal. | M | 1 |
| DV04-F115 | Read-only lock | Per buffer; the default for devices, memory, snapshots, untrusted files and handed-over ranges. | M | 1 |
| DV04-F116 | Write confirmation for devices and memory | Each save to a device or process lists the ranges to be written and needs a typed confirmation of the device or process name. | M | 3 |
| DV04-F117 | Snapshot before save | For files on ZFS, optional checkpoint through `svc.backup` before an in-place save. | C | 3 |

### 4.8 Compare

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV04-F120 | Two-way binary diff | Two sources side by side with linked scrolling; differing bytes coloured; only-in-left and only-in-right ranges shown as gaps. | M | 2 |
| DV04-F121 | Diff modes | **Positional** (byte at the same offset; instant, any size) and **aligned** (finds inserted and deleted ranges by content-defined anchors and a diff over block hashes; for sources up to the engine's limit, default 4 GiB each). | M | 2 |
| DV04-F122 | Difference list | Table of ranges: kind (changed, inserted, deleted), offsets in each side, length; `F7`/`Shift+F7` to step. | M | 2 |
| DV04-F123 | Summary | Bytes equal, changed, inserted, deleted; percentage; first difference. | M | 2 |
| DV04-F124 | Copy across | Copy a differing range from one side to the other (when that side is editable). | S | 2 |
| DV04-F125 | Structured compare | With the same template applied to both sides, the tree shows which fields differ. | S | 3 |
| DV04-F126 | Three-way | A base and two variants, to see which side changed a range. | C | 3 |
| DV04-F127 | Offset rebasing | Start comparison at different offsets in each source; compare selections only. | S | 2 |
| DV04-F128 | Export diff | As a patch file (4.11), as a text report, or as annotations on either side. | S | 2 |
| DV04-F129 | Compare by intent | DV03 (binary file diff) and DV01 (reproducible build check) open this view with two handles. | M | 2 |

### 4.9 Analysis: entropy, histogram, hashes, strings

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV04-F130 | Entropy view | Shannon entropy per block (block size 256 bytes to 1 MiB, auto by source size) as a graph over the whole source or selection; click jumps; thresholds mark "likely compressed or encrypted" (over 7.5 bits per byte) and "likely padding" (under 0.5). | M | 2 |
| DV04-F131 | Byte histogram | Counts of each of the 256 values for the source or selection, linear or log scale, sorted by value or count; click a bar to search that byte. | M | 2 |
| DV04-F132 | Digram plot | 256×256 plot of adjacent byte pairs, to recognise text, code and compressed data by shape. | C | 3 |
| DV04-F133 | Statistics | Size, zero count, printable ratio, mean, chi-square against uniform, longest run. | S | 2 |
| DV04-F134 | Checksums and hashes | For the source, selection or each annotation: CRC-8, CRC-16 (common variants), CRC-32, CRC-32C, CRC-64, Adler-32, Fletcher, sum8/16/32, XOR8; MD5 and SHA-1 (labelled "not for security"), SHA-224/256/384/512, SHA3-256/512, BLAKE2b, BLAKE3. Streamed with progress. | M | 1 |
| DV04-F135 | Verify against a value | Paste an expected digest; the matching algorithm is found by length and compared; green or red with a word. | M | 1 |
| DV04-F136 | Hash table export | A table of hashes per fixed-size block or per annotation as CSV. | S | 3 |
| DV04-F137 | Custom CRC | Width, polynomial, initial value, reflect in and out, final XOR. | S | 3 |
| DV04-F138 | Strings extraction | Runs of printable characters of a minimum length (default 4) in ASCII, UTF-8, UTF-16LE/BE; table with offset, length, encoding, text; filter and regex; jump; export. Streaming. | M | 2 |
| DV04-F139 | String classes | Tag strings that look like paths, URLs, email addresses, IPv4 and IPv6 addresses, GUIDs, version strings, Base64 blobs, key headers. Local pattern matching only. | S | 3 |
| DV04-F140 | Type detection | Magic-number scan at offset 0 and "scan for embedded signatures" over the whole source (the carving signature set). | M | 2 |

### 4.10 Disassembly pane and hand-off

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV04-F150 | Disassembly pane | Linear disassembly from the caret or selection: address, bytes, mnemonic, operands. Architectures as the engine provides (target list: x86-64, x86-32, AArch64, ARM and Thumb, RISC-V 64 and 32); base address and mode settable. | S | 3 |
| DV04-F151 | Engine | Decoding is done by DV07's disassembly engine as a helper service (section 6.4). This app holds no decoder of its own. If DV07 is not installed the pane says so and offers nothing else. | S | 3 |
| DV04-F152 | Linked navigation | Selecting an instruction selects its bytes; branch targets inside the source are links. | S | 3 |
| DV04-F153 | Open in Reverse Engineering | Sends the source handle and the selected range, base address and architecture to DV07 (`open` intent). Annotations can be sent as labels. | M | 2 |
| DV04-F154 | Executable awareness | With an ELF, PE or Mach-O template applied, section names label the address column and "disassemble section" uses the right base address. | S | 3 |
| DV04-F155 | Assemble (patch instruction) | Not provided here; DV07 owns assembling. A patch made there returns as a byte range edit by intent. | C | 3 |

### 4.11 Patch files

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV04-F160 | Create patch from edits | Save the buffer's changes as a patch instead of, or as well as, writing the file. | M | 2 |
| DV04-F161 | Patch formats | Own open format `.bpatch` (section 6.2; text, with source and target BLAKE3, ranges, old and new bytes); IPS; BPS; VCDIFF (RFC 3284) read and write; `bsdiff` read. | S | 2 |
| DV04-F162 | Apply patch | Preview: each range with old bytes expected and found; mismatches are listed; apply all, selected, or none; result is ordinary undoable edits. Source hash is checked first. | M | 2 |
| DV04-F163 | Reverse a patch | Apply in reverse when the format stores old bytes. | S | 3 |
| DV04-F164 | Patch from compare | From the diff view, "Create patch: left → right". | S | 2 |
| DV04-F165 | Patch report | A human-readable Markdown listing of what a patch changes, with annotation names for the ranges. | C | 3 |

### 4.12 File carving

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV04-F170 | Signature scan | Scans a source or range for headers (and footers where the format has one) from a signature set: JPEG, PNG, GIF, BMP, WebP, PDF, ZIP and its family, gzip, bzip2, xz, zstd, 7z, RAR, tar, ELF, PE, Mach-O, SQLite, MP3, MP4, Ogg, FLAC, WAV, OpenPGP and SSH key blocks, X.509 DER, squashfs, UBI, cpio, device tree blobs, U-Boot images. | S | 3 |
| DV04-F171 | Size determination | By footer, by header length field (through the format's template), or by maximum size; each hit shows how its end was decided and a confidence. | S | 3 |
| DV04-F172 | Results | Table: type, offset, size, confidence, validity (template parses or not); preview of the header; select bytes; annotate all. | S | 3 |
| DV04-F173 | Extract | Save chosen hits to a folder picked through the portal; names are `offset_type.ext`; a manifest CSV with offsets and hashes is written; extracted files get an "untrusted, carved" provenance tag. | S | 3 |
| DV04-F174 | Custom signatures | User signature file (JSON): name, header pattern, footer pattern, maximum size, template name. | C | 3 |
| DV04-F175 | Aligned scan | Only at sector or cluster boundaries for speed on disk images. | C | 3 |
| DV04-F176 | Open hit in place | Open a hit as a sub-buffer (a window onto the parent) with offsets relative to it; edits write through to the parent. | S | 3 |

### 4.13 Scripting

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV04-F180 | Script console | A panel running ForgeScript in a separate engine with the `hex::` interface: `hex::size()`, `hex::read(offset, n)`, `hex::u16le(offset)` and the other typed readers, `hex::find(pattern, from)`, `hex::selection()`, `hex::select(offset, n)`, `hex::annotate(offset, n, name, colour)`, `hex::bookmark`, `hex::write(offset, bytes)`, `hex::insert`, `hex::erase`, `hex::apply_template(name, offset)`, `hex::hash(alg, offset, n)`, `hex::print`. | S | 3 |
| DV04-F181 | Script sandbox | Scripts get no file, network, process or environment capability. They see only the buffer through `hex::`. Step, heap and time limits apply (engine options). Writes are ordinary undoable edits and obey the read-only lock. | S | 3 |
| DV04-F182 | Script library | User scripts in the app data directory, run from the command palette; bind to shortcuts. Signed scripts show their signer. | S | 3 |
| DV04-F183 | Record macro | Records edits and navigation as a ForgeScript script. | C | 3 |
| DV04-F184 | Batch run | Run a script over a set of files picked through the portal, with a results table; read-only unless ticked. | C | 3 |
| DV04-F185 | Byte type | Depends on a `bytes` value type and views in ForgeScript (FORGE-R-041); until then `hex::read` returns a list of integers and is slow for large ranges. | S | 3 |

### 4.14 Workbench, export, settings, recovery

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV04-F190 | Tabs and docks | One tab per source; dock panels: Inspector, Template, Annotations, Search results, Strings, Hashes, Entropy, Histogram, Changes, Disassembly, Console. Layouts saved. | M | 1 |
| DV04-F191 | Command palette | `Ctrl+K`: actions, go-to expressions (typing `0x…` jumps), templates, scripts, annotations by name. | M | 1 |
| DV04-F192 | Status bar | Caret offset (hex and decimal), selection, mode (OVR, INS, read-only), endianness, encoding, source size, source kind chip, modified marker, background jobs. | M | 1 |
| DV04-F193 | Export view | Hex dump as text (classic layout), HTML-free Markdown, or PDF through print; with annotations as a legend. | S | 2 |
| DV04-F194 | Print | Selection or range as a paginated dump through `svc.portal.print`. | C | 3 |
| DV04-F195 | Crash recovery | The edit journal (ranges and new bytes) is flushed every 2 s; after a crash the buffer reopens with edits and undo history. | M | 1 |
| DV04-F196 | Background jobs | Search, hash, entropy, strings, diff and carve run in the byte engine with progress, pause and cancel; several at once; they survive closing the panel. | M | 1 |
| DV04-F197 | Open in other apps | Text Editor (OF01), Archives (UT01), Reverse Engineering (DV07), Disks (AD02), for the whole source or a selection saved to a temporary handle. | S | 2 |
| DV04-F198 | Calculator strip | A small expression evaluator for offsets (hex, decimal, binary, bit operations) in the go-to field and the inspector. | S | 2 |
| DV04-F199 | Explain bytes (local AI) | Opt-in: sends the selected bytes (up to 4 KiB) and inspector values to `svc.model` for a guess at what they are. Output is labelled as a guess. | C | 3 |
