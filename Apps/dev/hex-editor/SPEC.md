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

The language is defined in section 6.3.

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

## 5. GUI

### 5.1 Window map

```
Hex Editor
├─ Main window
│  ├─ Header bar, toolbar (mode, base, bytes per row, encoding, endianness, go to)
│  ├─ Sidebar: Sources · Bookmarks and Annotations · Templates · Scripts
│  ├─ Content: source tabs → hex view panes (splits) │ Compare view │ Memory map view
│  ├─ Inspector (right): Data inspector · Template tree · Selection hashes
│  └─ Bottom dock: Search results · Strings · Entropy · Histogram · Changes · Disassembly · Carve results · Console
├─ Dialogs: Go to, Find and Replace, Open Device, Open Process Memory, Apply Template, Compare, Fill,
│           Paste Special, Operations, Create Patch, Apply Patch, Carve, Write to Device, Preferences
├─ Popovers: annotation card, field card, bytes-per-row, encoding picker, checksum verify
└─ Sheets (narrow): inspector, search results
```

### 5.2 Main window

Shown at 1440×900 with a file open and a template applied.

```
+------------------------------------------------------------------------------------------+
| [=] Hex Editor — firmware.bin           [ Search or run a command  Ctrl+K ]    [▁][▢][✕] | (1)
+------------------------------------------------------------------------------------------+
| [OVR] [Hex v] [16/row v] [UTF-8 v] [LE|BE]  Go to [0x_________]  [Find] [Template v]     | (2)
+----------------+--------------------------------------------------+----------------------+
| SOURCES        | firmware.bin ×   compare: a.bin ↔ b.bin          | INSPECTOR       [F4] |
| > firmware.bin | Offset    00 01 02 03 04 05 06 07  08 09 0A 0B … |  u8      127         |
|   a.bin        | 00000000  7F 45 4C 46 02 01 01 00  00 00 00 00 … |  i8      127         |
| -------------- | 00000010  03 00 3E 00 01 00 00 00  40 10 00 00 … |  u16 LE  17791       |
| ANNOTATIONS    | 00000020 [40 00 00 00 00 00 00 00] 98 31 00 00 … |  u32 LE  1179403647  |
|  ■ ELF header  | 00000030  00 00 00 00 40 00 38 00  0D 00 40 00 … |  u64 LE  …           |
|  ■ .text       |    (4)                        (5)  .ELF........ |  f32 LE  1.3e4       |
|  ■ key blob ⚑  | 00000040  06 00 00 00 04 00 00 00  …         (6) |  UTF-8   "\x7FELF"   |
| -------------- |                                                ▐ |  Unix    —       (7) |
| TEMPLATES      +--------------------------------------------------+ -------------------- |
|  ELF ✓ applied | TEMPLATE  SEARCH  STRINGS  ENTROPY  HISTOGRAM    | TEMPLATE TREE        |
|  PNG           | 8.0┤      ▄▄▄▄████████▄                      (9) | v ElfHeader          |
|  ZIP       (3) | 4.0┤▂▃▄▅▆▇              ▇▅▃▂▁▁▁                  |   magic  7F 45 4C 46 |
|                | 0.0┼────────────────────────────── 0 … 4.2 MiB   | > e_phoff 0x40   (8) |
+----------------+--------------------------------------------------+----------------------+
| 0x00000020 (32) · sel 8 bytes · OVR · LE · UTF-8 · 4.2 MiB · file · modified ● · 1 job   | (10)
+------------------------------------------------------------------------------------------+
```

1. Header bar. The app has no network capability; no `[net]` badge.
2. Toolbar: mode `toolButton`, base, bytes-per-row and encoding `comboBox`es, endianness toggle, go-to `lineEdit`.
3. Sidebar (220 px): sources `listView`, annotations `listView` with colour chips, templates `listView`.
4. Offset column. 5. Byte columns with the selection `[…]` and field colouring. 6. Text column. All three are the hex view widget (FUSE-WP-41) reading a `ByteSource` served by the byte engine.
7. Data inspector (`tableView`, 280 px): editable value cells.
8. Template tree (`treeView`): the selected field matches the selected bytes.
9. Bottom dock (`tabWidget`, 200 px): entropy `chart` shown; a click jumps.
10. Status bar; each chip is a button (mode, endianness, encoding).

States: empty (no source: "Open File", "Open Device", "New", recent list); loading (first screen is immediate; background scans show in the status bar); error (read error on a range: hatched cells with the error in a hover card; source unreadable: banner with the reason); offline (not applicable); locked (session lock: process-memory and device sources close their handles and must be re-granted); read-only (lock chip in the tab and status bar; typing shows a toast "Read-only — Enable editing").

### 5.3 Other views and dialogs

#### Compare view (1440×900)

```
+------------------------------------------------------------------------------------------+
| Compare  a.bin (4.20 MiB)  ↔  b.bin (4.21 MiB)   [Positional|Aligned]  [◀ F7] [▶]  3/17  |
+--------------------------------------------+---------------------------------------------+
| 00001000  48 89 E5 41 57 41 56 41  H..AWAVA | 00001000  48 89 E5 41 57 41 56 41  H..AWAVA |
| 00001008 [90 90]53 48 83 EC 28 48  ..SH..(H | 00001008 [EB 12]53 48 83 EC 28 48  ..SH..(H |
| 00001010  ░░░░░░░░ only in right ░░░░░░░░   | 00001010 [DE AD BE EF 00 00 00 01] ........ |
| 00001010  8B 05 22 2F 00 00 …               | 00001018  8B 05 22 2F 00 00 …               |
+--------------------------------------------+---------------------------------------------+
| DIFFERENCES                                               equal 99.98% · changed 2 ·     |
| ~ changed   0x1008  0x1008   2 bytes                      inserted 8 · deleted 0         |
| + inserted  —       0x1010   8 bytes          [Copy → left] [Create patch…] [Annotate]   |
+------------------------------------------------------------------------------------------+
```

Two hex views with linked scrolling and a difference `tableView`. Changed, inserted and deleted ranges use colour plus a gutter sign (`~ + −`). States: computing (aligned mode shows progress; positional results are shown meanwhile), too large for aligned mode (message with the limit and "Compare selections").

#### Find and replace (docked bar, expanded)

```
+------------------------------------------------------------------------------------------+
| Find   (•) Bytes ( ) Text ( ) Regex ( ) Number      [DE AD ?? E? .{0,16} (00|FF)       ] |
| In     [Whole source v]   Align [1 ]   [x] Wrap   [ ] Backward       [Find all] [◀] [▶]  |
| Replace [__________________________] (•) Overwrite ( ) Insert   [Replace] [Replace all]  |
| ✓ pattern is 5+ bytes, variable length · 1 204 matches so far · 62% · 1.8 GiB/s [Cancel] |
+------------------------------------------------------------------------------------------+
```

Number mode replaces the field with value, type and endianness controls; Text mode adds encoding and case options. An invalid pattern shows the parser's message under the field.

#### Apply template and template editor (bottom dock, split)

```
+---------------------------------------------+--------------------------------------------+
| png.bintpl                        [Run F5]  | Result at 0x0                              |
|  1 template Png {                           | v Png                         0x0   4.1 KiB |
|  2   endian big;                            |   signature  89 50 4E 47 …    0x0   8   ✓  |
|  3   magic signature = 89 50 4E 47 0D 0A…;  |   v chunks [0..11]            0x8          |
|  4   Chunk chunks[] until eof;              |     v [0] Chunk  "IHDR"       0x8   25     |
|  5 }                                        |         length  13                         |
|  6 struct Chunk {                           |         type    "IHDR"                     |
|  7   u32 length;                            |       > data    Ihdr {…}                   |
|  8   char type[4];                          |         crc     0x5A3C… ✓ crc32(type,data) |
|  9   match type { "IHDR" => Ihdr data; …    |     > [1] Chunk  "IDAT"       0x21  8204   |
| ✕ line 9: unknown type 'Ihdrr'          (1) | fuel 1 204 / 50 000 000 · 0.4 ms       (2) |
+---------------------------------------------+--------------------------------------------+
```

1. Compiler diagnostics inline. 2. Resource use of the last evaluation.

#### Open process memory (dialog 640×480) and memory map

```
+----------------------------------------------------------------------+
| Open process memory (read-only)                                       |
| [filter________]                                                      |
| PID    Name            App                    User                    |
| 2291   notes           org.pbsd.notes         odin                    |
| 2304   archive-helper  org.pbsd.archives      odin                    |
| 1180   keyring         system                 —  not inspectable      |
| After you choose, the system asks you to confirm.   [Cancel] [Open…]  |
+----------------------------------------------------------------------+

 MEMORY MAP — notes (2291)                      [r v] [filter____] [⟳ 2 s v]
 Start              End                Size     Prot  Backing
 0x0000000001021000 0x0000000001043000 136 KiB  r-x   app host
 0x0000000801200000 0x0000000801a00000 8 MiB    rw-   heap             → open
 0x00007fffdfffc000 0x00007ffffffff000 512 MiB  rw-   stack (guard…)
```

The process list comes from the broker and already excludes other users' processes. The shell draws the confirmation. Map is a `tableView`; double click opens the region in the hex view with virtual addresses in the offset column.

#### Carve (dialog 760×520 → results panel)

```
+--------------------------------------------------------------------------+
| Carve files from firmware.bin                                             |
| Range   (•) Whole source ( ) Selection ( ) From caret                     |
| Types   [x] Images [x] Archives [x] Executables [ ] Media [x] Keys  [All] |
| Align   [1 v] bytes          Max size per file [256 MiB]                  |
|                                              [Cancel]  [Scan]             |
+--------------------------------------------------------------------------+
 CARVE RESULTS  41 hits                                   [Extract selected…]
 [x] PNG     0x0004A000   18.2 KiB   footer found      valid ✓
 [x] ZIP     0x00120000   1.4 MiB    end record found  valid ✓
 [ ] gzip    0x002F0010   ≤ 256 MiB  no length; max    unknown ?
 [x] SSH key 0x003A1F00   411 B      footer found      —
```

Validity uses the type's template. Extraction writes through a folder handle from the portal.

#### Write to device (confirmation dialog 560×320)

```
+------------------------------------------------------------+
| Write 2 ranges to ada1 (Samsung 500 GB)?                    |
| This cannot be undone.                                      |
|   sector 0      512 bytes   (MBR)                           |
|   sector 2048   4096 bytes                                  |
| Device is not mounted · not part of the boot pool           |
| Type the device name to confirm: [_______]                  |
|                         [Keep read-only]  [Write 2 ranges]  |
+------------------------------------------------------------+
```

The destructive button is outlined `--danger`, disabled until the name matches; Esc keeps read-only.

### 5.4 Menus

| Menu | Items |
|---|---|
| File | New (`Ctrl+N`) · Open File… (`Ctrl+O`) · Open Device… · Open Process Memory… · Open Recent ▸ · Enable Editing · Save (`Ctrl+S`) · Save As… (`Ctrl+Shift+S`) · Save Selection As… · Insert File… · Revert · Create Patch… · Apply Patch… · Export Hex Dump… · Print… (`Ctrl+P`) · Close (`Ctrl+W`) · Quit (`Ctrl+Q`) |
| Edit | Undo (`Ctrl+Z`) · Redo (`Ctrl+Shift+Z`) · Cut, Copy, Paste (`Ctrl+X/C/V`) · Copy As ▸ (`Ctrl+Shift+C`) · Paste Special… (`Ctrl+Shift+V`) · Delete · Select All (`Ctrl+A`) · Select Range… (`Ctrl+Shift+A`) · Fill… (`Ctrl+Shift+I`) · Operations ▸ XOR…, AND…, OR…, NOT, Shift…, Rotate…, Add…, Byte Swap, Reverse · Transform ▸ · Resize… · Insert/Overwrite (`Insert`) · Preferences (`Ctrl+,`) |
| Search | Find… (`Ctrl+F`) · Find Next, Previous (`Ctrl+G`, `Ctrl+Shift+G`) · Replace… (`Ctrl+H`) · Find All · Go To… (`Ctrl+L`) · Follow Pointer (`Ctrl+Enter`) · Back, Forward (`Alt+Left/Right`) · Next, Previous Change (`F7`, `Shift+F7`) · Next Non-Zero (`Ctrl+.`) · Find Pointers to Selection |
| View | Command Palette (`Ctrl+K`) · Base ▸ Hex, Decimal, Octal, Binary · Bytes per Row ▸ · Group Size ▸ · Text Encoding ▸ · Offset Format ▸ · Endianness (`Ctrl+E`) · Byte Colours ▸ · Split Right (`Ctrl+\`) · Split Down · Minimap · Bitmap View · Sidebar (`F9`) · Inspector (`F4`) · Panels ▸ · Refresh (`F5`) · Zoom (`Ctrl++/-/0`) · Full Screen (`F11`) |
| Marks | Toggle Bookmark (`Ctrl+F2`) · Next, Previous Bookmark (`F2`, `Shift+F2` in the hex view) · Set Mark (`Ctrl+M`) · Select to Mark · Annotate Selection… (`Ctrl+Shift+N`) · Annotations List · Import, Export Notes… |
| Analyse | Apply Template… (`Ctrl+T`) · Edit Templates · Checksums and Hashes… (`Ctrl+Shift+H`) · Verify Digest… · Entropy · Histogram · Statistics · Strings… (`Ctrl+Alt+S`) · Detect Type · Carve Files… · Compare… (`Ctrl+D`) · Disassemble Selection · Open in Reverse Engineering |
| Tools | Script Console (`` Ctrl+` ``) · Run Script ▸ · Record Macro · Batch Run… · Calculator |
| Help | Handbook (`F1`) · Template Language Reference · Keyboard Shortcuts · About |

### 5.5 Keyboard and pointer

- Hex view: arrows move by byte and row; `Ctrl+Left/Right` by group; `Home/End` row; `Ctrl+Home/End` source; `PageUp/Down`; `Tab` switches between byte and text columns; typing edits; `Backspace` in overwrite mode restores the original byte.
- Context menu on bytes: Cut, Copy, Copy As ▸, Paste, Paste Special, Fill, Operations ▸, Annotate, Bookmark, Follow Pointer, Select Field (template), Hash Selection, Search for Selection, Find Pointers Here, Disassemble, Open in Reverse Engineering, Save Selection As.
- Context menu on annotations: Go To, Select, Edit, Colour ▸, Hash, Export Bytes, Delete. On template fields: Select Bytes, Copy Value, Copy Path, Annotate, Follow (pointer fields), Export Subtree.
- Drag and drop: a file from Files onto the window (open) or onto a hex view with `Ctrl` (insert at the drop offset in insert mode); a selection dragged to Files (saves the bytes, through the portal); a selection onto another tab (paste); a `.bintpl` onto the window (add template); a `.bpatch` (apply preview); two files dropped together (compare).
- Pointer: double click selects the field or annotation under the pointer; `Alt+click` adds a disjoint selection; wheel scrolls rows, `Shift+wheel` pages, `Ctrl+wheel` zooms; dragging the minimap scrolls.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Open File, Open Device, New, recent list. |
| Loading | First screen at once; "indexing strings…", "computing entropy…" as jobs. |
| Error | Per-range read errors hatched; helper crash: "Byte engine restarted" banner, buffers reopen from the edit journal. |
| Offline | Not applicable. |
| Locked | On session lock, device and process handles are dropped; their tabs show "Grant again to continue". File buffers stay. |
| Read-only | Default for devices, memory, snapshots, untrusted and handed-over sources; all analysis works; edits refused with a toast. |
| Source changed underneath | Buffer frozen; banner: Reload, Save edits as patch, Compare with new content. |
| Narrow (< 640 px) | Bytes per row drops to fit (8 or 4); text column can hide; sidebar overlay; inspector and bottom dock become sheets. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Hex view, compare panes | hex view (FUSE-WP-41: `ByteSource`, 64-bit offsets, annotation layers, comparison) |
| Inspector, annotations, results, memory map, differences | `tableView` (FUSE-WP-28 for 64-bit row counts) |
| Template tree | `treeView` with lazy children |
| Template editor, script console | code editor (FUSE-WP-37) |
| Entropy, histogram | `chart` |
| Digram plot, bitmap view, minimap | `viewportWidget` with app drawing (DV04-GAP-1) |
| Disassembly | `tableView` with monospace delegate |
| Toolbar, dialogs | `toolButton`, `comboBox`, `lineEdit`, `spinBox`, `radioButton`, `checkBox`, `dialog`, `progressBar`, `colorPicker` (annotation colour) |
| Notes | `markdownView` |
| Docks | `dockArea`, `dockWidget`, `tabWidget`, `splitter` |

| ID | Missing | Note |
|---|---|---|
| DV04-GAP-1 | Raster plot surface for bitmap, digram and minimap views with pixel-exact zoom | App drawing on `viewportWidget` is enough; a shared widget is optional |
| DV04-GAP-2 | Hex view features beyond FUSE-WP-41's stated design: UTF-8 multi-byte overlay in the text column, disjoint selections, value-grid mode, per-byte change marks | Requirements on FUSE-WP-41 |
| DV04-GAP-3 | Device portal: pick a block device and get a rights-limited handle (read-only by default) | Platform; not in PLATFORM.md section 8. AD02 needs it too |
| DV04-GAP-4 | Debug broker: pick a process and get a read-only memory handle and its map | Platform; same service as DV01-GAP-2 |

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App shell and view models | ForgeScript + FUSE UI | app host | Windows, tabs, panels, commands, annotations store, session, settings |
| Byte engine `byte-engine` | C++23 | bundle helper; holds the source handles passed to it (file, device, memory); no network | Piece-tree buffers, edit journal, `ByteSource` service for the hex view, save, search (bytes, text, regex, numeric), hashes and checksums, entropy, histogram, statistics, strings, positional and aligned diff, signature scan |
| Template engine `template-engine` | C++23 | bundle helper; no capabilities; reads bytes only through a read window the byte engine serves | Compiles and evaluates structure templates under fuel, depth and memory limits; lazy tree |
| Patch codec | C++23, in `template-engine` process | same | Parses and writes `.bpatch`, IPS, BPS, VCDIFF, `bsdiff` |
| Script engines | ForgeScript | separate engine instances in the app host with an empty policy | User scripts and macros through `hex::` |
| Disassembly client | ForgeScript | app host | Talks to DV07's disassembly service over the session bus |
| Decompress hand-off | — | UT01's archive helper | Decompression of selections with output caps |

```
 user ─ FUSE UI ─ [ app host: shell · annotations · session · script engines · disassembly client ]
                      │ ByteSource (shared memory windows)      │ session bus
                      ▼                                         ▼
               [ byte-engine ] ── read window ──▶ [ template-engine + patch codec ]   DV07 disassembly service
                 file / device / memory handles        (no capabilities)             UT01 archive helper
                 edit journal in fs.app
```

The byte engine is the only process that holds source handles. The template engine, which interprets hostile data under a user-supplied grammar, holds none.

### 6.2 Data model

In the byte engine:

| Entity | Fields |
|---|---|
| `Source` | `id: u32`, `kind: file\|device\|memory\|shared`, `size: u64`, `sector: u32`, `writable: bool`, `identity: {dev, ino, mtime_ns}` or `{pid, start_time}` |
| `Piece` | `origin: original\|add`, `start: u64`, `length: u64` (nodes of a balanced tree with subtree length sums) |
| `EditRecord` | `seq: u64`, `kind: overwrite\|insert\|erase`, `offset: u64`, `old_len: u64`, `new_len: u64`, `add_start: u64`, `name: string` |
| `Job` | `id`, `kind`, `range`, `progress: u64`, `state` |

App store `~/.local/share/org.pbsd.hex-editor/hex.db` (SQLite, `user_version` = schema):

| Table | Fields |
|---|---|
| `source_key` | `id`, `content_key BLOB(32)` (BLAKE3 of size ‖ first MiB ‖ last MiB), `size INT`, `label TEXT`, `last_opened INT` |
| `annotation` | `id`, `source_id`, `start INT`, `length INT`, `name`, `colour`, `type_hint`, `note` |
| `bookmark` | `id`, `source_id`, `offset INT`, `name`, `colour` |
| `applied_template` | `source_id`, `template_name`, `offset INT` |
| `view_state` | `source_id`, `caret INT`, `sel_start`, `sel_len`, `bytes_per_row`, `base`, `encoding`, `endian` |
| `search_history` | `kind`, `pattern`, `options`, `ts` |
| `custody` | `source_id`, `ts`, `event`, `hash BLOB`, `detail` |

Journal `~/.local/share/org.pbsd.hex-editor/journal/<source>.ej`: append-only edit records plus the add buffer, checksummed per record. Templates: `templates/*.bintpl`. Scripts: `scripts/*.fs`. Signatures for carving: `signatures.json`.

Sidecar `<name>.hexnotes` (JSON, `schema: 1`): `{source: {size, content_key}, annotations[], bookmarks[], templates[]}`.

Patch format `.bpatch` (UTF-8 text, line-based):

```
bpatch 1
source size=4404224 blake3=9f2c…e1
target size=4404232 blake3=71aa…04
@ 0x1008 old=9090 new=EB12
@ 0x1010 old= new=DEADBEEF00000001
end
```

Each `@` line is offset in the source, old bytes (empty for an insert), new bytes (empty for a delete), in ascending offset order. Long runs may use `old=@file:offset:length` forms only in the binary variants (VCDIFF); `.bpatch` is for small, reviewable patches (limit 16 MiB of text).

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Any file, device or memory as bytes | yes | yes (by grant) | own | `byte-engine` (no interpretation) |
| Structure templates `.bintpl` | yes | yes | own compiler and evaluator | `template-engine` |
| Data under a template | yes | field edits | own | `template-engine` |
| `.hexnotes`, signatures, settings (JSON) | yes | yes | own strict JSON | app host (capped) |
| `.bpatch`, IPS, BPS, VCDIFF, `bsdiff` | yes | `.bpatch`, IPS, BPS, VCDIFF | own | `template-engine` process (patch codec) |
| Byte regex | yes | — | own linear-time automaton | `byte-engine` |
| Compressed streams (decompress selection) | yes | no | UT01 archive helper | that helper |
| Machine code | yes | no | DV07 disassembly engine | DV07's helper |
| Hash and checksum algorithms | — | — | platform crypto (FORGE) plus own CRC tables | `byte-engine` |
| Hex dump text, CSV, Markdown | no | yes | own | app host |

#### Structure template language

A declarative grammar for binary layouts. A template describes; it does not compute freely. There are no loops, no recursion without progress, no assignments and no I/O. Every array has a count, a byte size or an `until` condition, and every element must consume at least one byte, so evaluation terminates on any input; fuel and depth limits bound the cost.

Grammar (EBNF):

```
file        = { import | const | enum | bitfield | struct | union | template } ;
import      = "import" string ";" ;
const       = "const" ident "=" expr ";" ;
template    = "template" ident "{" { option } { member } "}" ;       (* a root: can be applied *)
struct      = "struct" ident [ "(" params ")" ] "{" { option } { member } "}" ;
union       = "union" ident "{" { member } "}" ;                      (* members overlap *)
enum        = "enum" ident ":" inttype "{" ident "=" expr { "," ident "=" expr } "}" ;
bitfield    = "bitfield" ident ":" inttype "{" { ident ":" number ";" } "}" ;   (* widths sum to the type *)
option      = "endian" ( "little" | "big" ) ";" | "align" expr ";" ;
member      = field | magic | cond | match | at | assert | let | pad ;
field       = type ident [ array ] [ attrs ] ";" ;
array       = "[" expr "]" | "[" "]" "until" ( "eof" | expr ) | "[" "]" "size" expr ;
magic       = "magic" ident "=" bytes ";" ;                          (* fixed bytes; also used for auto-detect *)
cond        = "if" "(" expr ")" "{" { member } "}" [ "else" "{" { member } "}" ] ;
match       = "match" expr "{" { pattern "=>" member } [ "_" "=>" member ] "}" ;
at          = "at" "(" expr ")" member ;                              (* parse at an absolute offset; cursor unchanged *)
assert      = "assert" expr [ ":" string ] ";" ;
let         = "let" ident "=" expr ";" ;                              (* named value, no storage *)
pad         = "pad" expr ";" | "align" expr ";" ;
type        = inttype | "f16" | "f32" | "f64" | "char" | "bool"
            | "cstring" [ "(" encoding ")" ] | "string" "(" expr [ "," encoding ] ")"
            | "uleb128" | "sleb128" | "bytes" "(" expr ")" | ident [ "(" args ")" ] ;
inttype     = ( "u" | "i" ) ( "8" | "16" | "24" | "32" | "64" ) [ "le" | "be" ] ;
attrs       = "@" attr { "," attr } ;
attr        = "hex" | "dec" | "bin" | "hidden" | "time" "(" ident ")" | "colour" "(" string ")"
            | "comment" "(" string ")" | "checksum" "(" ident "," range ")" | "max" "(" expr ")" ;
expr        = integer arithmetic, comparison, logic, bit operations, "?:", field paths (a.b[i].c),
              built-ins: sizeof(x), offsetof(x), here, base, eof, length(x), parent.x ;
```

Rules:

- Expressions are pure and use checked 64-bit integers; overflow or division by zero makes the field an error.
- A field path may refer only to fields already parsed (earlier in document order) or to `parent` fields. This keeps evaluation single-pass.
- `at` reads elsewhere without moving the cursor; the engine tracks visited `(type, offset)` pairs and refuses a repeat, which stops pointer cycles.
- `checksum(alg, range)` names a built-in algorithm (the list of F134); it is checked, never executed as code.
- Limits per evaluation: fuel, depth 256, array `max` (default 2^32 elements, lazily materialised), string length 16 MiB.
- Templates cannot name files other than `import` of other templates from the same library directory.

Example (ELF64 header, abridged):

```
enum ElfType : u16 { NONE = 0, REL = 1, EXEC = 2, DYN = 3, CORE = 4 }

template Elf64 {
  endian little;
  magic ident = 7F 45 4C 46;
  u8  class   @comment("1 = 32-bit, 2 = 64-bit");
  u8  data;
  assert class == 2 : "not ELF64";
  pad 10;
  ElfType e_type;
  u16 e_machine @hex;
  u32 e_version;
  u64 e_entry  @hex;
  u64 e_phoff  @hex;
  u64 e_shoff  @hex;
  u32 e_flags  @hex;
  u16 e_ehsize; u16 e_phentsize; u16 e_phnum; u16 e_shentsize; u16 e_shnum; u16 e_shstrndx;
  at (e_phoff) ProgramHeader phdr[e_phnum];
  at (e_shoff) SectionHeader shdr[e_shnum];
}
```

### 6.4 Integration

- Intents handled: `open`, `view`, `edit` for any type as bytes; `open` with a byte-range payload from DV05, DV07, DV01, UT01; `view` with two handles (compare) from DV03 and DV01. Emitted: `open` to DV07 (range, base, architecture), OF01, UT01, AD02; `share`; `print`.
- Services: `svc.settings`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.clipboard`, `svc.notify` (long jobs), `svc.audit` (device and memory access, writes), `svc.backup` (optional checkpoint), `svc.model` (optional).
- IPC offered: `org.pbsd.hex-editor.Templates/1` — evaluate a named template over a byte buffer and return the tree (used by DV05 for payload decoding and DV07 for headers).
- IPC used: DV07 disassembly service (Proposed; DV07's spec defines it). The RetDec fork was not read for this spec; which decoder it carries is Unverified.
- Backing repos: none. All Proposed. The hex view widget is FUSE-WP-41 (Verified as a planned work package in `FUSE_BUILD_PLAN.md`, not as code).

## 7. Security and privacy

| Capability | Reason |
|---|---|
| `fs.app` | Annotation store, edit journals, templates, scripts |
| `fs.portal` | Files the user opens or saves; folder for carved files |
| `proc.helper:byte-engine`, `proc.helper:template-engine` | Bundle helpers |
| `net.none` | No network use |
| `svc.settings`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.clipboard`, `svc.notify`, `svc.audit`, `svc.backup` | As in 6.4 |
| `svc.model`, `ai.local` | Optional local explanations and template drafts; off by default |

Not expressible in PLATFORM.md section 6 today, requested as gaps: a block-device handle from a device portal (DV04-GAP-3) and a process-memory handle from a debug broker (DV04-GAP-4). The app declares no `dev.*` or `fs.path` capability; without those portals the device and memory features are absent.

Assets: the bytes being examined (may be secrets: disk images, memory, keys); the integrity of devices and running processes; the user's other files.

Adversaries: a hostile file crafted to exploit a parser; a hostile template or script from another person; malware in a carved file; a user mistake that overwrites a disk; another process's secrets exposed through the memory view.

| Threat | Mitigation |
|---|---|
| Exploit in format decoding | The byte engine does not interpret formats. Templates are interpreted in `template-engine`, which has no capabilities, fuel and memory limits, and a read window only |
| Hostile template (exponential nesting, pointer cycles, huge arrays) | Language is total by construction; `at` cycle check; lazy arrays; fuel, depth and node limits |
| Hostile script | Own engine with an empty policy; only `hex::`; step, heap and time limits; writes obey the lock and are undoable |
| Regex denial of service | Linear-time automaton; no backreferences |
| Accidental write to a disk | Devices are read-only; write needs a separate grant, an unmounted non-boot device and a typed confirmation; writes are verified by read-back |
| Reading another process's secrets | Broker decides which processes are offered (same user, inspectable only); shell prompt per process; handle dropped on lock; every access in `svc.audit` |
| Carved malware runs | Extraction only writes files with an "untrusted, carved" provenance tag; nothing is opened or executed |
| Decompression bomb | Decompression in UT01's helper with an output cap |
| Secrets left behind | Edit journals and annotation notes may contain sensitive bytes; they live in `fs.app`, are removed when the buffer is closed without restore, and a "private session" keeps them in memory only |
| Clipboard exposure | "Copy" of more than 1 MiB asks first; a "sensitive" flag can be set so the clipboard service expires the entry |
| Annotation or patch path tricks | Sidecar and patch files carry no paths that are followed; `import` in templates is limited to the library directory |

Stored: section 6.2. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `view.bytes_per_row` | int or `fit` | 16 | Bytes per row |
| `view.group` | int | 1 | Group size |
| `view.base` | enum | `hex` | `hex`, `dec`, `oct`, `bin` |
| `view.hex_upper` | bool | true | Upper-case hex digits |
| `view.encoding` | string | `UTF-8` | Text column encoding |
| `view.offset_base` | enum | `hex` | Offset column base |
| `view.colour_scheme` | enum | `class` | Byte colouring |
| `view.minimap` | bool | true | Minimap strip |
| `edit.default_mode` | enum | `overwrite` | `overwrite`, `insert` |
| `edit.open_read_only` | bool | true | Files open read-only until "Enable editing" |
| `edit.keep_backup` | bool | false | Keep `.bak` on rewrite saves |
| `inspector.endian` | enum | `little` | Default endianness |
| `inspector.rows` | string[] | standard set | Rows shown |
| `search.max_results` | int | 1 000 000 | Find-all cap |
| `template.fuel` | int | 50 000 000 | Evaluation steps |
| `template.auto_suggest` | bool | true | Offer templates by magic |
| `diff.aligned_limit_bytes` | int | 4 GiB | Per side |
| `strings.min_length` | int | 4 | Minimum string length |
| `entropy.block` | int or `auto` | `auto` | Block size |
| `carve.max_size_bytes` | int | 256 MiB | Per carved file without a length |
| `memory.refresh_ms` | int | 0 | 0 = manual |
| `script.max_steps`, `script.heap_bytes`, `script.timeout_ms` | int | 10^9, 256 MiB, 60 000 | Script limits |
| `session.private` | bool | false | Keep journals in memory only |
| `clipboard.confirm_bytes` | int | 1 MiB | Ask before copying more |
| `ai.enabled` | bool | false | Local explanations |

## 9. Accessibility and languages

- The hex view exposes rows as a table in the a11y tree: offset, then each byte as a cell with its value read as hex or decimal (setting), then the text. A "read row" command and a "read selection as type" command (uses the inspector) exist.
- Changed bytes, diff states and annotations are announced by word ("changed", "inserted", "annotation: ELF header"), not only shown by colour; colour schemes have a high-contrast and a colour-blind-safe variant, and diff uses gutter signs.
- Entropy and histogram have a table alternative.
- Digits are always ASCII digits regardless of locale; thousands separators appear only in decimal sizes in the status bar. Offsets are left-to-right in right-to-left UIs.
- Every panel and dialog is keyboard reachable; the hex view never traps `Tab` (it switches columns once, then leaves on the second press; `Shift+Tab` reverses).

## 10. Performance budgets

| Metric | Budget |
|---|---|
| Cold start to first frame | ≤ 150 ms |
| Open any file to first screen | ≤ 50 ms, independent of size |
| Scroll | 60 Hz at any offset in a 1 TiB source; zero steady-state allocation in the view |
| Edit (insert one byte at offset 0 of a 100 GiB file) | ≤ 1 ms |
| Byte search (literal, NVMe, warm cache) | ≥ 1.5 GiB/s; with wildcards ≥ 500 MiB/s |
| SHA-256 / BLAKE3 | ≥ 400 MiB/s / ≥ 1.5 GiB/s on one core |
| Entropy of a 10 GiB file | ≤ 15 s; progressive display from the first second |
| Template: ELF with 5 000 section headers | ≤ 20 ms; 10 million array elements stay lazy with ≤ 50 MiB |
| Aligned diff of two 1 GiB files | ≤ 30 s |
| Memory | app host ≤ 150 MiB; byte engine ≤ 256 MiB plus page cache; template engine ≤ 512 MiB cap |
| Save, overwrite-only edits | time proportional to changed bytes |
| Idle | zero frames and wake-ups (memory auto-refresh excepted when on) |

## 11. Testing and proof

Test plan:

- Unit: piece-tree operations against a naive byte-vector model (property tests: random edit sequences give identical content and identical undo results); go-to expression parser; pattern compiler; inspector encoders and decoders with round trips for every type; `.bpatch` round trip.
- Template language: conformance suite (each construct), termination suite (adversarial templates and data), goldens of shipped templates against known files, differential against an independent decoder for ELF, PNG and ZIP on a corpus.
- Fuzz: template compiler; template evaluator over (template, data) pairs; patch parsers; byte regex compiler; `.hexnotes` loader; signature file loader; `ByteSource` protocol.
- Search and hash: results equal reference tools on a corpus; huge-source tests on sparse 1 TiB files.
- Diff: aligned diff output applied as a patch reproduces the other side (always).
- Device and memory: tests against a memory disk and a test process; write protection tests (every write path refused without the grant).
- Golden renders of the hex view in each base, encoding and colour scheme; scripted UI for the 14 top tasks.
- Crash: kill the byte engine mid-save and mid-edit; the original file is intact or fully updated, and the journal restores edits.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Piece tree | Content equals the model after any edit sequence; undo is exact; offsets never overflow | T1 | PRISM proof of the tree invariants (length sums, balance) and refinement to a sequence model | Proof effort; falls back to T2 |
| Save path | After a crash at any point the file is the old or the new content | T2 | PRISM for runtime errors; model of the rename protocol; fault-injection tests | Depends on file-system guarantees |
| Template evaluator | Terminates within fuel on all inputs; never reads outside the window; no runtime errors | T1 (termination and bounds), T2 (rest) | Lean statement of the fuel argument; PRISM on the code; fuzz | — |
| Template compiler | No runtime errors on any text | T2 | PRISM + fuzz | — |
| Byte regex | Linear time in input; no runtime errors | T2 | PRISM; complexity by construction and benchmark | — |
| Search, hash, entropy, strings, diff | Safe subset; results equal references | T3 | Subset plugin, sanitizers, differential tests | Volume of code |
| Patch codec | No runtime errors; apply(create(a, b), a) = b | T2 (safety), T3 (round trip by test) | PRISM + property tests | — |
| Device write gate | No write is issued without the write grant flag set by the broker | T2 | PRISM on the gate | Broker is outside the module |
| App ForgeScript | Engine safety only | T4 | Tests | — |

Nothing here has been run.

## 12. Build plan

Phase 1: usable core.

| ID | Work item | Depends on |
|---|---|---|
| DV04-W01 | Shell: tabs, docks, toolbar, status bar, command palette | G-01, G-02, G-07 |
| DV04-W02 | Byte engine: sources, piece tree, edit journal, `ByteSource` service | FORGE-WP-28.1, G-08 |
| DV04-W03 | Hex view integration: bases, rows, encodings, splits, selection | FUSE-WP-41, DV04-GAP-2 |
| DV04-W04 | Editing: overwrite, insert, paste special, copy as, fill, undo, change marks | DV04-W02, DV04-W03 |
| DV04-W05 | Save paths (in place, rewrite), external change detection, crash recovery | DV04-W02 |
| DV04-W06 | Data inspector with edit-through | DV04-W03 |
| DV04-W07 | Go to, navigation history, bookmarks, annotations and their store | DV04-W03 |
| DV04-W08 | Search: bytes with wildcards, text, find all | DV04-W02 |
| DV04-W09 | Checksums and hashes, verify | DV04-W02 |

Phase 2: full feature.

| ID | Work item | Depends on |
|---|---|---|
| DV04-W20 | Template language: compiler, evaluator, limits | FORGE-WP-28.1 |
| DV04-W21 | Template tree, field colouring, editor, edit-through, shipped templates (first 15) | DV04-W20, FUSE-WP-37 |
| DV04-W22 | Regex and numeric search, replace, alignment | DV04-W08 |
| DV04-W23 | Compare view: positional and aligned diff | DV04-W03 |
| DV04-W24 | Entropy, histogram, statistics, minimap | DV04-W02, DV04-GAP-1 |
| DV04-W25 | Strings extraction, type detection | DV04-W02 |
| DV04-W26 | Patch files: `.bpatch`, IPS, BPS, VCDIFF | DV04-W04 |
| DV04-W27 | Operations and transforms; decompress through UT01 | UT01 |
| DV04-W28 | Block devices read-only, partition templates | DV04-GAP-3, AD02 |
| DV04-W29 | Intents: byte ranges in, hand-off to DV07, compare by intent; Templates IPC | DV07, DV03, DV01, DV05 |

Phase 3: advanced.

| ID | Work item | Depends on |
|---|---|---|
| DV04-W40 | Process memory view and map | DV04-GAP-4 |
| DV04-W41 | Disassembly pane | DV07 disassembly service |
| DV04-W42 | File carving, custom signatures, sub-buffers | DV04-W20, DV04-W25 |
| DV04-W43 | Scripting console, library, macros, batch | FORGE-WP-16, FORGE-WP-20 (`bytes`) |
| DV04-W44 | Device write unlock and memory write | DV04-W28, DV04-W40 |
| DV04-W45 | Remaining shipped templates, checksum fields, structured compare, three-way | DV04-W21, DV04-W23 |
| DV04-W46 | Bitmap and digram views, pointer search, custody log | DV04-W24 |
| DV04-W47 | Local AI explain and template draft | G-15 |

## 13. Open questions and risks

1. **Device portal (DV04-GAP-3).** No portal in PLATFORM.md section 8 hands out block-device handles. AD02 Disks needs the same thing. It should be one platform service with read-only as the default right.
2. **Debug broker (DV04-GAP-4).** Process memory needs a platform decision on who may inspect what; HardenedBSD restricts this by default. Shared with DV01-GAP-2.
3. **Template language scope.** The language is deliberately not Turing-complete. Some formats (compressed streams, formats needing backward references or look-ahead) cannot be described fully; those fields stay `bytes` and the user decompresses separately. Whether to add a bounded `repeat … while` with mandatory progress is open.
4. **Disassembly engine.** Depends on DV07 offering a helper service. The RetDec fork was not read here; what it can decode is Unverified.
5. **`bytes` in ForgeScript.** Scripting large ranges is impractical until FORGE-R-041 lands.
6. **Aligned diff at scale.** A 4 GiB per side limit is a guess; content-defined anchoring may allow more, to be measured.
7. **Hex view requirements.** FUSE-WP-41's stated design covers grouped hex, text column, editing, annotation layers and comparison; multi-byte overlay, value grid and disjoint selection (DV04-GAP-2) are additions.
8. **Sensitive data in journals.** Crash recovery and secrecy pull in opposite directions; the private-session switch is the compromise. Encrypting journals with a per-session key from `svc.keyring` is an option.
