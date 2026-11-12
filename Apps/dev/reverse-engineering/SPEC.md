<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Reverse Engineering

By Odin Loch

| | |
|---|---|
| ID | `DV07` (from CATALOGUE.md) |
| Bundle | `org.pbsd.reverse-engineering` |
| Category | dev |
| Priority | P2 |
| Backing repos | RetDec-Decompiler (the RetDec fork: loaders, lifter, C back end, detectors, tools), FUSE (UI), FORGE (ForgeScript, model service) |
| Status | Planned. Nothing built. |

## 1. Purpose

A decompiler workbench for analysts, security researchers and developers who must understand a binary without its source. It loads a file, never runs it, and shows headers, disassembly, decompiled C, graphs, cross-references, types and strings in linked views, with a project database for names and comments. It replaces the fork's Qt 6 GUI (`RETDEC:src/gui/`, about 28,000 lines) and adds what that GUI does not have: a no-capability sandbox around every parser, an analysis database, a types editor, patching, binary diffing, rules and scripting.

## 2. Scope

In scope:

- Loading ELF, PE, Mach-O, COFF, raw images, `ar` archives, universal Mach-O, firmware containers, and the bytecode formats the fork reads (WebAssembly, JVM, DEX, .NET CIL, Python, Lua).
- Triage overview: headers, sections, imports, exports, strings, entropy, compiler and packer detection, signatures.
- Disassembly listing, decompiled C, IR view, all navigated in step.
- Control-flow graph and call graph.
- Cross-references, types and structures, renaming, comments, bookmarks, tags.
- Library function matching and signature creation.
- Patching, binary diffing, rules, scripting in ForgeScript.
- Static unpacking of known packers.
- Local AI help for names and summaries.
- Debugger hand-off by capability; hostile samples go to a VM.
- Export and collaboration through project files.

Out of scope:

| Item | Owner |
|---|---|
| General hex editing, structure templates, large file byte editing | DV04 Hex Editor |
| Network capture analysis of a sample's traffic | DV05 Packet Analyser |
| Running a hostile sample, VM snapshots, VM networking | VM01 Virtual Machines |
| Source-level debugging of the user's own programs | DV01 IDE |
| Verifying recovered C with a model checker | DV06 Proof Workbench |
| Archive browsing and extraction as a general task | UT01 Archives |
| Malware scanning of the whole system, quarantine | SE01 Security Centre |
| Managing model files | AI02 Model Manager |
| Hash verification, secure erase of samples | SE05 Vault |

## 3. Users and top tasks

Personas:

- **Malware analyst**: opens hostile samples daily. Must never execute one by accident. Wants triage in seconds.
- **Vulnerability researcher**: reads firmware and stripped binaries, diffs patched against unpatched.
- **Developer**: checks what a compiler produced, or recovers a lost tool's behaviour.
- **Student**: learns how C maps to machine code.
- **Auditor**: checks that a shipped binary matches the claimed source.

| # | Task | Target steps |
|---|---|---|
| T1 | Open a binary and see format, architecture, packer and risk flags | 1 (drop the file) |
| T2 | Decompile and read `main` | 2 (Analyse, double click) |
| T3 | Jump from a C line to its instructions | 1 (click; views follow) |
| T4 | Find every use of a string | 2 (Strings, `X`) |
| T5 | Rename a function and see the name everywhere | 2 (`N`, type) |
| T6 | Define a struct and apply it to a variable | 4 |
| T7 | Show the call graph two hops around a function | 2 |
| T8 | Diff two versions of a library and list changed functions | 3 |
| T9 | Patch a conditional jump and export the patched file | 4 |
| T10 | Match statically linked library code | 2 |
| T11 | Run a ForgeScript over all functions and tag results | 3 |
| T12 | Send a sample to a disposable VM for dynamic analysis | 3 |
| T13 | Ask for a summary of a function with the local model | 1 (`Ctrl+Shift+A`) |
| T14 | Export decompiled C and a report | 3 |
| T15 | Share a project without sharing the sample | 2 |

## 4. Features

Priority: M must, S should, C could. Phase 1 usable core, 2 full feature, 3 advanced.

### 4.1 Loading and sandbox
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV07-F001 | Open by portal or drop | The app receives a read-only handle. The file is never given the execute right and is never mapped executable. | M | 1 |
| DV07-F002 | Parser sandbox | Every byte of the sample is parsed in the `loader` helper: capability mode, no network, no file system, one read handle to the sample and one write handle to a scratch directory. | M | 1 |
| DV07-F003 | Never execute | No code path starts the sample. The fork's `--try-emulation` option and in-process unpacker emulation (`RETDEC:src/mini_emu`, `src/unpacker`) run only inside the sandbox as interpreters; native execution is out of the app. | M | 1 |
| DV07-F004 | Formats | ELF, PE, Mach-O, COFF, Intel HEX, raw. Bytecode: `.wasm`, `.class`/`.jar`, `.dex`/`.apk`, managed `.dll`, `.pyc`, `.luac` (as listed in `RETDEC:README.md`). | M | 1 |
| DV07-F005 | Raw image dialog | Architecture, endianness, bit size, base address, entry point; maps to `-a`, `-e`, `-b`, `--raw-section-vma`, `--raw-entry-point`. | M | 1 |
| DV07-F006 | Archives | `ar` static libraries: member list, open one member (`--ar-index`, `--ar-name`), or analyse all. Uses `retdec-ar-extractor` logic in the sandbox. | M | 1 |
| DV07-F007 | Universal Mach-O | Pick an architecture slice (`retdec-macho-extractor`). | S | 1 |
| DV07-F008 | Firmware containers | Scan for embedded images by magic and entropy; list candidates with offset and size; carve one into a child item. Formats in Phase 2: uImage, FIT, squashfs/cramfs headers (list only), Intel HEX, S-record, UEFI capsule sections. Proposed; not in the fork. | S | 2 |
| DV07-F009 | Debug info | DWARF and PDB (`-p`, `RETDEC:src/pdbparser`, `src/debugformat`) loaded when present; PDB chosen through the portal. | S | 1 |
| DV07-F010 | Size and resource limits | Memory cap per analysis (`--max-memory`), time limit (`--timeout`), job count (`--jobs`); shown and editable before a run. | M | 1 |
| DV07-F011 | Hash on open | SHA-256, BLAKE3, TLSH fuzzy hash (TLSH is vendored in the fork, `RETDEC:deps/tlsh`), imphash for PE. | M | 1 |
| DV07-F012 | Sample quarantine store | Optional copy of the sample into an app-private store with no-exec dataset flags and a renamed extension. | S | 2 |
| DV07-F013 | Hostile mode default | Files from Transfers, Mail or a removable device open with AI help, rules from the sample directory and debugger hand-off disabled until the user enables each. | M | 1 |
| DV07-F014 | Batch load | Queue several files; each analysed in its own sandbox; results per item. The fork's GUI has a batch dialog (`batch_decompile_dialog.cpp`). | S | 2 |
| DV07-F015 | Reopen with cache | Analysis results are keyed by sample hash and settings; reopening skips finished stages. | M | 1 |

### 4.2 Overview and triage
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV07-F016 | Triage banner | Format, architecture, OS, size, packer warning, with "Unpack first" and "Analyse anyway". The fork's `TriageBanner` does this. | M | 1 |
| DV07-F017 | Header tree | File, optional and section headers as a tree with raw value, decoded meaning and file offset. | M | 1 |
| DV07-F018 | Sections and segments | Name, VMA, size, raw size, flags, entropy, hash; W+X sections flagged. | M | 1 |
| DV07-F019 | Imports and exports | By library; ordinal resolution (the fork fetches MFC ordinal data at set-up); delay imports; demangled names (`RETDEC:src/demangler`). | M | 1 |
| DV07-F020 | Risk flags | Deterministic list: W+X sections, entry in last section, high-entropy sections, TLS callbacks, missing mitigations (PIE, RELRO, canary, NX, CFG, ASLR), suspicious imports, overlay data. Each flag states its rule. | M | 1 |
| DV07-F021 | Entropy plot | Entropy by offset as a `chart` with section bounds; click navigates. | M | 1 |
| DV07-F022 | Byte map | Coloured strip of the file: code, data, strings, zero, high entropy, unknown. | S | 2 |
| DV07-F023 | Compiler and packer detection | From `retdec-fileinfo` / `cpdetect`: tool, version, confidence, method (signature or heuristic). | M | 1 |
| DV07-F024 | Static unpack | `retdec-unpacker` plugins (UPX, MPRESS and others in `RETDEC:src/unpackertool`) run in the sandbox; output becomes a child item. Unknown packers: stated as not unpacked. | M | 2 |
| DV07-F025 | Certificates | Authenticode chain, signer, timestamps, validity against the file hash (`RETDEC:deps/authenticode-parser`); Mach-O code signature blob fields. Trust is displayed, not asserted. | S | 2 |
| DV07-F026 | Resources | PE resources tree (icons, version info, manifests, strings); icons shown through the image thumbnailer; nothing is executed or rendered as HTML. | S | 2 |
| DV07-F027 | Rich header, PDB path, build ids | Shown with decoded tool ids. | S | 2 |
| DV07-F028 | Symbols | Symbol table with type, binding, section; filter and export. | M | 1 |
| DV07-F029 | Relocations and dynamic entries | Tables with links into the listing. | S | 2 |
| DV07-F030 | Overlay and embedded files | Data after the last section; embedded PE/ELF/ZIP detection; open as child. | S | 2 |
| DV07-F031 | Summary report | One page: hashes, format, flags, top imports, interesting strings, detected algorithms. | M | 2 |

### 4.3 Strings and constants
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV07-F032 | Strings table | Address, encoding (ASCII, UTF-8, UTF-16LE/BE), length, section, category, reference count. | M | 1 |
| DV07-F033 | Categories | Plain, format string, URL, file path, regex, crypto constant (the fork's Strings Browser has these six), plus registry key, IP address, e-mail, command line. | M | 1 |
| DV07-F034 | Filter | Text or regex over all columns; minimum length; section; "referenced only". | M | 1 |
| DV07-F035 | Constants table | Address, size, hex, decimal, semantic label: magic numbers, crypto round constants, IEEE special values, port numbers (as in the fork's Constants tab). | S | 2 |
| DV07-F036 | Stack and built strings | Strings assembled at run time found by pattern over the IR; marked "reconstructed". | C | 3 |
| DV07-F037 | Decode helpers | XOR, ROT, Base64, hex, simple key search on a selection; output is a new comment, not a file change. | S | 2 |
| DV07-F038 | Indicators list | URLs, domains, IPs, paths, mutex-like names collected for export. Defanged on copy by default. | S | 2 |

### 4.4 Disassembly listing
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV07-F039 | Linear listing | Address, bytes, mnemonic, operands, comment; data shown typed; virtualised over the whole image. | M | 1 |
| DV07-F040 | Architectures | Those the fork lifts: x86, x86-64, ARM, Thumb, ARM64, MIPS, PIC32, PowerPC, RISC-V, SPARC, SystemZ, XCore (`RETDEC:README.md` maturity table). Disassembly through Capstone in the helper. | M | 1 |
| DV07-F041 | Syntax options | Intel or AT&T for x86; register aliases; number base per operand. | S | 1 |
| DV07-F042 | Code/data marking | Mark a range as code, data, string, array, alignment; undo; stored in the project. | M | 2 |
| DV07-F043 | Function boundaries | Create, delete, split, merge, change start and end. | M | 2 |
| DV07-F044 | Operand rendering | Show an immediate as hex, decimal, char, enum member, offset, or struct field. | M | 2 |
| DV07-F045 | Jump arrows | Local branch arrows in the gutter. | S | 2 |
| DV07-F046 | Stack view | Per-function frame layout with variable names and sizes. | S | 2 |
| DV07-F047 | Instruction help | Short description of the mnemonic and flags affected, offline. | C | 3 |
| DV07-F048 | Bytes column toggle, address formats | VMA, file offset, RVA, section+offset. | M | 1 |
| DV07-F049 | Select ranges to decode | Analyse only chosen ranges or functions (`--select-ranges`, `--select-functions`, `--select-decode-only`). | S | 1 |

### 4.5 Decompiled C and IR
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV07-F050 | Decompiled C view | Read-only C with highlighting, line numbers, folding. Output of the fork's C back end (`llvmir2hll`). Native binaries give C only; there is no C++ writer in the fork. | M | 1 |
| DV07-F051 | Synchronised navigation | Click in C, listing or IR; the other views highlight and scroll to the matching lines. Uses a line-mapping table; the fork's `TriPaneCodeView` keeps `(asm_line, ir_line, c_line)` triples. | M | 1 |
| DV07-F052 | Mapping quality stated | Where no mapping exists for a C line, the view says "no mapping" and does not guess. | M | 1 |
| DV07-F053 | IR view | LLVM IR / SSA text per function with block headers, phi nodes, use-def hover. | S | 1 |
| DV07-F054 | Rename in C | Functions, parameters, locals, globals, labels; applied through the project database and re-rendered. | M | 2 |
| DV07-F055 | Retype in C | Change a variable or return type; set a function prototype; calling convention. | M | 2 |
| DV07-F056 | Comments in C | Line and block comments stored by address, shown in every view. | M | 2 |
| DV07-F057 | Re-decompile one function | After a rename, retype or boundary change. Whole-program re-run only when needed. | M | 2 |
| DV07-F058 | Back-end options | Variable renamer, compound operators, keep brackets, keep library functions, no optimisations (`--backend-*` options). | S | 2 |
| DV07-F059 | Pass pipeline profile | Pick an LLVM pass list (`--llvm-passes-json`, `--profile`; the fork ships default and fast lists). | S | 2 |
| DV07-F060 | Semantic annotations | `// [RetDec]` comments and the config sidecar from the detectors (containers, algorithms, crypto, serialisation, concurrency, patterns) shown as badges with the detector name. The fork's README gives name-blind algorithm F1 of 0.056 on its corpus, so each badge reads "possible". | S | 2 |
| DV07-F061 | Buildable output | Show the `--buildable` sidecar and whether it passed a syntax-only compile in the sandbox. | S | 2 |
| DV07-F062 | Bytecode output | Python for `.pyc`, Lua for `.luac`, WAT for `.wasm`, Java-family for JVM/DEX, C#-family for CIL, as the fork emits. | S | 2 |
| DV07-F063 | Neural refinement off by default | The fork can refine output with a model through llama.cpp. In this app that path is replaced by `svc.model` and is opt-in; refined text is shown beside, never instead of, the deterministic output. | M | 2 |
| DV07-F064 | Find in view | Text and regex; find across all functions' C. | M | 1 |
| DV07-F065 | Highlight same symbol | Click a variable to highlight all uses in the function. | M | 1 |
| DV07-F066 | Copy with addresses | Copy C or listing with or without addresses. | S | 1 |

### 4.6 Graphs
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV07-F067 | Control-flow graph | Basic blocks as `nodeGraph` nodes with address header and instruction preview; edges coloured by kind (taken, not taken, unconditional, switch, exception). | M | 2 |
| DV07-F068 | Layered layout | Sugiyama layout computed in the app; long chains compressed above 200 nodes (the fork's `CFGScene` does both). | M | 2 |
| DV07-F069 | Loop regions | Loop bodies outlined; nesting depth shown. | S | 2 |
| DV07-F070 | Graph minimap | Overview with draggable viewport. | S | 2 |
| DV07-F071 | Graph navigation | Zoom, pan, fit, reset, follow edge, go to block in listing. | M | 2 |
| DV07-F072 | Block content modes | Disassembly, IR, or C lines per block. | S | 3 |
| DV07-F073 | Call graph | Functions as nodes sized by instruction count; edges weighted by call count; indirect calls dashed. | M | 2 |
| DV07-F074 | Call graph filters | Name glob, module, minimum call count, hide library functions, N-hop focus on callers or callees. | M | 2 |
| DV07-F075 | SCC collapse | Strongly connected components collapsed into one node with a count (Tarjan), expandable. | S | 2 |
| DV07-F076 | Module clusters | Communities shown as backdrops; from `RETDEC:src/module_cluster` output when run. | C | 3 |
| DV07-F077 | Graph export | SVG, PNG, DOT, JSON. | S | 2 |
| DV07-F078 | Xref graph | Graph of references to and from one symbol, bounded depth. | S | 3 |
| DV07-F079 | Type hierarchy | Class tree from RTTI and vtables with inheritance kind; vtable slots table (slot, name, address, pure). | S | 2 |

### 4.7 Cross-references and navigation
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV07-F080 | Xrefs to / from | For any address, symbol, string, import, struct field: list with kind (call, jump, read, write, address-of), function and preview. `X` opens it. | M | 1 |
| DV07-F081 | Go to | Address, symbol, expression (`base+0x40`), file offset. `G`. | M | 1 |
| DV07-F082 | History | Back and forward across all views. | M | 1 |
| DV07-F083 | Function list | Address, name, size, instructions, calling convention, pattern badges, tags, notes; sort, filter by glob, address range, pattern; CSV/JSON export (columns from the fork's `FunctionListPanel`). | M | 1 |
| DV07-F084 | Bookmarks | Named, with colour and note. | M | 2 |
| DV07-F085 | Tags | On functions and addresses; batch tag from a selection or script. | S | 2 |
| DV07-F086 | Global search | One box over names, strings, comments, bytes (hex pattern with wildcards), instruction text, constants. | M | 2 |
| DV07-F087 | Imports usage | For each import: every caller. | M | 1 |
| DV07-F088 | Entry points list | Program entry, exports, TLS callbacks, constructors, interrupt vectors for raw images. | S | 2 |

### 4.8 Types and structures
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV07-F089 | Type library | Built-in C types and OS type sets; the fork ships type information consumed by `ctypes`/`ctypesparser`. | M | 2 |
| DV07-F090 | Struct editor | Fields with offset, size, type, name, comment; padding shown; drag to reorder; array and bitfield support. | M | 2 |
| DV07-F091 | Union, enum, typedef, function pointer editors | Enum with flag mode for OR-ed values. | M | 2 |
| DV07-F092 | Import C declarations | Paste or open a header subset; parsed in the sandbox; errors by line. | S | 2 |
| DV07-F093 | Export types | As a C header or JSON. | S | 2 |
| DV07-F094 | Apply type | To data at an address, a local, a parameter, a return value; listing and C re-render. | M | 2 |
| DV07-F095 | Struct from usage | Propose a struct from offsets accessed through one pointer; shown as a proposal to accept or edit. | S | 3 |
| DV07-F096 | Type propagation preview | Before applying, list what will change. | C | 3 |
| DV07-F097 | Class recovery view | Vtable, RTTI name, methods grouped per class. | S | 3 |

### 4.9 Project database
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV07-F098 | Project file | One `.reproj` SQLite file: names, comments, types, bookmarks, tags, patches, settings, analysis cache index. The sample is referenced by hash, not embedded, unless the user asks. | M | 1 |
| DV07-F099 | Import fork projects | Reads the fork's `.retdec` JSON (`version`, `binary`, `arch`, `os`, `entryPoint`, `stages`, `annotations`, `decompiledOutput`; `RETDEC:include/retdec/gui/project_file.h`). | S | 2 |
| DV07-F100 | Undo and redo | Every database change is a journal entry; unlimited undo within a session; history survives restart. | M | 2 |
| DV07-F101 | Change log | Who (local user name), when, what; filter by kind. | S | 2 |
| DV07-F102 | Autosave and crash recovery | SQLite WAL; a crash loses at most the edit in flight. | M | 1 |
| DV07-F103 | Snapshots | Named snapshots of the database; compare two snapshots. | S | 3 |
| DV07-F104 | Multiple samples per project | A project can hold several binaries (a firmware image and its parts, two versions for a diff). | S | 2 |
| DV07-F105 | Notes | A Markdown notes page per project and per function. | S | 2 |

### 4.10 Signatures and library matching
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV07-F106 | Static code detection | Match statically linked library functions with the fork's signature files (`stacofin`, `--static-code-sigfile`); matched functions named and marked "library". | M | 2 |
| DV07-F107 | Signature creation | From an object or archive: extract members, `retdec-bin2pat` to patterns, `retdec-pat2yara` to rules; the fork's Signature Studio panel drives these steps. | S | 2 |
| DV07-F108 | Signature sets | Enable or disable sets per project; show which set named a function. | S | 2 |
| DV07-F109 | Fuzzy function match | Match functions against a local library of known functions by normalised instruction hash and by TLSH; show score. | C | 3 |
| DV07-F110 | Match confidence | Each match shows method and score; low scores are "candidates" and do not rename automatically. | M | 2 |
| DV07-F111 | Local symbol store | Keep names from earlier projects keyed by function hash; offer them in later projects. Local only. | C | 3 |

### 4.11 Hex, patching and diffing
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV07-F112 | Inline hex pane | Read-only hex and ASCII of the selection, following navigation. | M | 1 |
| DV07-F113 | Hand-off to Hex Editor | Sends the sample handle and offset to DV04 with a read-only flag; DV04 can send an offset back. | M | 2 |
| DV07-F114 | Patch bytes | Edit bytes at an address; stored as a patch record, the original file is never changed. | M | 2 |
| DV07-F115 | Patch instruction | Type an instruction; assembled in the helper; size mismatch shown with NOP padding offer. Assembler: own table-driven encoder for x86-64 and ARM64 first (the fork's Keystone is GPL-2.0 and tool-only per its NOTICE, so it is not used). | S | 3 |
| DV07-F116 | Patch helpers | NOP out, invert branch, force branch, return constant. | S | 2 |
| DV07-F117 | Patch list | Address, old bytes, new bytes, note, enabled; toggle and revert each. | M | 2 |
| DV07-F118 | Export patched file | Written through the save portal as a new file with no execute bit and a `.patched` suffix; also export a patch script or a binary diff. | M | 2 |
| DV07-F119 | Binary diff: function matching | Match functions of two binaries by name, hash, call-graph position and fuzzy hash; list unchanged, changed, added, removed with a similarity score. Proposed; the fork's Diff panel compares decompiled text before and after recovery passes, not two binaries. | M | 3 |
| DV07-F120 | Binary diff: side by side | Two listings or two C views with a line diff (Myers), synchronised scroll, next/previous change. | M | 3 |
| DV07-F121 | Binary diff: graph | CFG of both versions with matched blocks coloured. | C | 3 |
| DV07-F122 | Diff report | Changed functions with summary; export Markdown or HTML. | S | 3 |
| DV07-F123 | Pass diff | Before/after view of the decompiled text per recovery stage, with statistics; kept from the fork. | C | 3 |
| DV07-F124 | Port annotations | Copy names, comments and types from one version to the matched functions of the other, with a preview. | S | 3 |

### 4.12 Rules and scripting
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV07-F125 | Rule scan | YARA rules run in the sandbox against the file and sections (libyara and yaramod are vendored in the fork). Hits: rule, tags, meta, offsets. | M | 2 |
| DV07-F126 | Rule editor | Text editor with syntax colouring, compile errors by line, test against the open sample. | S | 2 |
| DV07-F127 | Rule sets | Sets stored in app data; signed sets can be marked trusted; rules from a sample's own folder are never auto-loaded. | M | 2 |
| DV07-F128 | Rule from selection | Generate a byte-pattern rule from selected instructions with operand wildcards. | S | 3 |
| DV07-F129 | Scripting console | ForgeScript REPL bound to the analysis API. | M | 2 |
| DV07-F130 | Script API | Read: binaries, sections, functions, blocks, instructions, xrefs, strings, types, C text. Write: names, comments, tags, types, bookmarks, patches. Every write goes through the undo journal. | M | 2 |
| DV07-F131 | Script capabilities | A script inherits no more than the app has and by default less: no `net`, no `proc`, `fs` limited to a per-script scratch directory. | M | 2 |
| DV07-F132 | Script library | Saved scripts with name, description, shortcut; signed scripts marked. | S | 2 |
| DV07-F133 | Batch scripts | Run a script over many samples headless in the queue. | C | 3 |
| DV07-F134 | Script output table | A script can return rows shown in a table with address links. | S | 2 |
| DV07-F135 | No native plugins | The fork loads shared-library plugins with `QPluginLoader` (`plugin_interface.h`). The app does not: extension is by signed ForgeScript only. | M | 1 |

### 4.13 Debugger and VM hand-off
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV07-F136 | Send to disposable VM | Hands the sample to VM01 with a profile: no network or simulated network, snapshot before, discard after. The app itself never runs it. | M | 3 |
| DV07-F137 | Debugger attach by capability | Connects to a debug stub (GDB remote protocol) inside a VM over a channel VM01 provides. Needs the user's grant per session. | S | 3 |
| DV07-F138 | Debug views | Registers, stack, memory, breakpoints, step, run to address; breakpoints set from listing, C or graph. | S | 3 |
| DV07-F139 | Rebase | Map run-time addresses to the static image when the load address differs. | S | 3 |
| DV07-F140 | Trace import | Import an execution trace (address list with counts) and colour covered blocks. | C | 3 |
| DV07-F141 | Memory dump import | Load a dump from the VM as a new sample in the project. | C | 3 |
| DV07-F142 | Trusted local debug | For the user's own non-hostile binaries only: debug in a jail with no network. Off by default; needs `proc.exec` granted per project. | C | 3 |

### 4.14 Local AI assistance
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV07-F143 | Suggest names | For a function, parameters and locals; shown as suggestions with accept/reject; accepted names are marked "model-suggested" in the database. | S | 2 |
| DV07-F144 | Summarise function | A short description stored as a comment tagged "model". | S | 2 |
| DV07-F145 | Explain selection | Explain the selected instructions or C. | S | 2 |
| DV07-F146 | Prompt hygiene | String literals from the sample are stripped or fenced before the prompt (the fork strips C string literals in `RETDEC:src/neural/prompts.cpp`); the model has no tools and cannot write to the database without the user's accept. | M | 2 |
| DV07-F147 | Local only | Through `svc.model`; no remote endpoint exists in the app. | M | 2 |
| DV07-F148 | Suggest struct layout and types | As proposals feeding the struct editor. | C | 3 |
| DV07-F149 | Chat about the binary | Side panel with the current function as context; answers cite addresses that link into the views. | C | 3 |
| DV07-F150 | Model labels everywhere | Any text that came from a model has a "model" mark in every view and export. | M | 2 |

### 4.15 Export and collaboration
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV07-F151 | Export C | One file or per function; with or without comments and addresses. | M | 1 |
| DV07-F152 | Export JSON | Functions, xrefs, strings, types, annotations in a documented schema; the fork's config JSON sidecar passed through. | M | 2 |
| DV07-F153 | Export listing | Text or HTML (static). | S | 2 |
| DV07-F154 | Export report | Markdown, HTML or PDF: overview, flags, indicators, notes, selected functions, graphs. | M | 2 |
| DV07-F155 | Export bundle | Archive of C, config, listing, IR and log (the fork's GUI writes a store-only ZIP of these, `export_bundle.cpp`). | S | 2 |
| DV07-F156 | Annotation exchange | Export or import names, comments and types as a text file keyed by address and function hash; merge with a conflict list. | M | 2 |
| DV07-F157 | Project sharing without the sample | The `.reproj` holds only hashes; a colleague opens it with their own copy and the hash is checked. | M | 2 |
| DV07-F158 | Three-way merge of projects | Merge two edited copies with a common ancestor; conflicts per item. | C | 3 |
| DV07-F159 | Signed project | Detached signature over the project file from `svc.keyring`. | C | 3 |
| DV07-F160 | Indicators export | CSV, JSON, STIX 2.1 bundle (objects only, no sharing service). | C | 3 |
| DV07-F161 | Print | Selected function or report through the print portal. | S | 2 |

### 4.16 Analysis control, settings, accessibility, recovery
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV07-F162 | Analysis queue and progress | Per-stage progress with times; cancel; the fork's `ProgressPanel` shows a stage list, a waterfall and throughput. | M | 1 |
| DV07-F163 | Live console | Helper log streamed while it runs; command log with arguments, exit code and time. | M | 1 |
| DV07-F164 | Problems list | Warnings and errors per stage with severity filter. | M | 1 |
| DV07-F165 | Analysis profiles | Fast, default, deep; maps to pass lists and detector toggles. | S | 2 |
| DV07-F166 | Layouts | Save and restore dock layouts: Triage, Reading, Graph, Diff, Debug. | S | 2 |
| DV07-F167 | Keyboard complete | Single-key commands in code views (`N`, `X`, `G`, `;`, `Y`, `Esc`) plus full menu access. | M | 1 |
| DV07-F168 | Graph alternatives | Every graph has an equivalent list view for screen readers. | M | 2 |
| DV07-F169 | Session restore | Open projects, views, positions. | M | 1 |
| DV07-F170 | Audit entries | Sample opened (hash), VM hand-off, patched file export, debugger attach go to `svc.audit`. | M | 2 |

## 5. GUI

Layout pattern: workbench with docks. Default density: compact. Monospace face in all code views.

### 5.1 Window map

```
Reverse Engineering
├─ Start page (recent projects, open binary, open project, hostile-mode notice)
├─ Project window
│  ├─ Header bar: menu, sample switcher, address/search box, capability badges
│  ├─ Triage banner (dismissible)
│  ├─ Left dock: Functions · Symbols · Imports/Exports · Types · Bookmarks
│  ├─ Centre tabs
│  │  ├─ Overview
│  │  ├─ Listing
│  │  ├─ Decompiled C
│  │  ├─ Synced (Listing | IR | C)
│  │  ├─ IR
│  │  ├─ Control-flow graph
│  │  ├─ Call graph
│  │  ├─ Strings and constants
│  │  ├─ Type editor
│  │  ├─ Diff
│  │  └─ Debug (only with a VM session)
│  ├─ Right dock (F4): Inspector · Xrefs · Hex pane · Notes · Assistant
│  ├─ Bottom dock: Console · Problems · Command log · Progress · Script · Rule hits · Patches
│  └─ Status bar: address, offset, section, function, analysis state, sandbox state
├─ Tool windows: Signature studio · Rule editor · Type hierarchy · Script library
├─ Dialogs
│  ├─ Open raw image · Open archive member · Pick Mach-O slice
│  ├─ Analysis options · Batch analyse
│  ├─ Go to · Rename · Set type · Edit function · Comment
│  ├─ Xrefs chooser
│  ├─ Patch instruction · Export patched file
│  ├─ Diff setup (pick two samples)
│  ├─ Send to VM · Attach debugger
│  ├─ Export (C, JSON, listing, report, bundle, annotations)
│  └─ Merge annotations (conflicts)
├─ Popovers: risk flag rule, match confidence, model suggestion, mapping quality
└─ Preferences
```

### 5.2 Main window

Shown at 1440×900, Synced tab, analysis finished.

```
+------------------------------------------------------------------------------------------+
| [=] Reverse Engineering — fw_update.elf   [0x00401a40  go/search Ctrl+K]    [▁] [▢] [✕] | (1)
+------------------------------------------------------------------------------------------+
| ⚠ ELF64 x86-64 · Linux · 412 KB · UPX packed (signature)  [Unpack first] [Dismiss]       | (2)
+------------------+------------------------------------------------------+----------------+
| FUNCTIONS  [filt]| Overview | Listing | C | Synced | CFG | Calls | Str |  | XREFS TO       | (7)
| addr     name    +------------------+----------------+------------------+ parse_hdr       |
| 401000  _start   | LISTING          | IR             | C                | call 401a9c main|
| 401a40 >parse_hdr| 401a40 push rbp  | entry:         | int parse_hdr(   | call 4021f0 upd |
| 401b10  crc32  L | 401a41 mov rbp,… |  %1 = alloca   |   uint8_t *buf,  | data 604120 tbl |
| 401c00  sub_401c | 401a44 sub rsp,… |  %2 = load …   |   size_t n) {    +----------------+
| 4021f0  update   |>401a48 mov eax,… |> %5 = icmp …   |>  if (n < 16) {  | INSPECTOR      |
| …           (3)  | 401a4d cmp eax,… |  br i1 %5 …    |     return -1;   | addr 0x401a48  |
+------------------+ 401a50 jb 401a90 |                |   }              | off  0x1a48    |
| SYMBOLS | IMPORTS| …            (4) | …          (5) |   …          (6) | sect .text r-x |
| TYPES | BOOKMARKS|                  |                |                  | func parse_hdr |
+------------------+------------------+----------------+------------------+ size 208       |
| Console | Problems | Command log | Progress | Script | Rule hits | Patches               | (8)
| [loader] parsed 14 sections · [lift] 312 functions · [c] done 6.2 s                      |
+------------------------------------------------------------------------------------------+
| 0x401a48 · .text+0xa48 · parse_hdr · analysis done · sandbox: no net, no fs, no exec     | (9)
+------------------------------------------------------------------------------------------+
```

Callouts:

1. **Header bar.** Sample switcher for multi-sample projects. The box accepts an address, a symbol or a search. Badges: model badge when the assistant is in use; `[net]` never appears in normal use because the app has no network capability.
2. **Triage banner** (`panel`): format line, packer warning, the two common next actions.
3. **Functions dock** (`tableView`): `L` marks library matches; badges for detector patterns; filter box.
4. **Listing pane** (`codeEdit`, read-only): address, bytes (optional), instruction, comment.
5. **IR pane** (`codeEdit`, read-only).
6. **C pane** (`codeEdit`, read-only text, in-place rename and comment through commands). `>` marks the linked line in all three panes.
7. **Right dock**: xrefs for the selection (`listView`) and inspector (`propertyGrid`).
8. **Bottom dock** (`dockWidget` + `tabBar`).
9. **Status bar**: address in three forms, function, analysis state, and the sandbox state in words.

Sizes: left dock 240 px, right dock 280 px, bottom dock 150 px, row 20 px in code views. States: empty (start page), loading (banner shows stage progress; views fill as stages finish: overview first, listing next, C last), error (helper crashed: banner with stage and "Retry with lower limits"), offline (no effect), locked (code views blanked while the session is locked), read-only (project opened without the sample: C and listing from cache only, patch and re-analysis disabled).

### 5.3 Other views and dialogs

**Overview.**
```
+------------------------------------------------------------------------------------------+
| fw_update.elf    SHA-256 9f2c…e1  BLAKE3 71aa…  TLSH T1A2…        [Copy hashes] [Report] |
+------------------------------+-----------------------------------------------------------+
| HEADERS                      | SECTIONS    vma       size   flags  entropy               |
| ▾ ELF header                 | .text       401000   3a2f0  r-x    6.1  ▆▆▆▆▆▆            |
|   class      ELF64           | .rodata     43c000    8f10  r--    4.9  ▅▅▅▅              |
|   machine    x86-64          | .data       604000    1200  rw-    2.2  ▂▂                |
|   entry      0x401000        | UPX1        700000   51000  rwx    7.9  ████████  ⚠ W+X   |
| ▸ Program headers (6)        +-----------------------------------------------------------+
| ▸ Dynamic (24)               | ENTROPY BY OFFSET                                   (1)   |
+------------------------------+  8 ┤            ▁▂▃▅▇████████████                          |
| RISK FLAGS               (2) |  4 ┤▃▄▅▅▅▅▅▆▆▅▅▃                                           |
| ⚠ W+X section UPX1           |  0 ┼──────────┬──────────┬──────────┬────                 |
| ⚠ entry point in last sect.  +-----------------------------------------------------------+
| ⚠ no RELRO, no canary        | IMPORTS (41)  libc.so.6: fopen fread memcpy system …      |
| ✓ PIE off (static address)   | DETECTED  compiler gcc 9.x (heuristic 62%) · packer UPX   |
+------------------------------+-----------------------------------------------------------+
```
1. `chart` with section bounds; click navigates the listing. 2. Each flag opens a popover stating the rule that raised it. States: no sections (raw image: shows the byte map instead), detection not run.

**Control-flow graph.**
```
+------------------------------------------------------------------------------------------+
| parse_hdr · 9 blocks · 2 loops    [Fit] [1:1] [Blocks: asm v] [Export v]      ┌───────┐ |
|                                                                               │minimap│ |
|                 ┌──────────────────────┐                                      │  ▫    │ |
|                 │ 401a40               │                                      │ ▫▫▫   │ |
|                 │ push rbp             │                                      └───────┘ |
|                 │ cmp eax, 0x10        │                                             (1)|
|                 └───────┬──────────┬───┘                                                |
|                  taken  │          │ not taken                                          |
|          ┌──────────────▼──┐   ┌───▼──────────────┐                                     |
|          │ 401a90          │   │ 401a56  ↻ loop 1 │ (2)                                  |
|          │ mov eax, -1     │   │ movzx ecx, [rdi] │                                     |
|          └────────┬────────┘   └───┬──────────────┘                                     |
|                   └──────┬─────────┘                                                    |
|                     ┌────▼─────┐                                                        |
|                     │ 401aa2 ret│                                                       |
+------------------------------------------------------------------------------------------+
```
`nodeGraph` with read-only nodes. 1. Minimap overlay. 2. Loop region outline. Keys: arrows move along edges, `Enter` opens the block in the listing, `Tab` cycles successors. States: too large (above 2,000 blocks: offers the list view or a subgraph), no function selected.

**Xrefs chooser** (`X`).
```
+----------------------------------------------------------------------+
| References to  crc32  (0x401b10)                     [filter____]    |
+--------+----------+-----------+--------------------------------------+
| kind   | from     | function  | preview                              |
| call   | 401a9c   | parse_hdr | call crc32                           |
| call   | 402233   | update    | call crc32                           |
| addr   | 604128   | (data)    | dq offset crc32                      |
+--------+----------+-----------+--------------------------------------+
|                                   [Open in new tab]  [Go]  [Close]   |
+----------------------------------------------------------------------+
```

**Type editor.**
```
+------------------------------------------------------------------------------------------+
| TYPES [filter]        | struct fw_header            size 0x20     [Apply to selection…] |
| ▾ Structs             +------+------+-------------+------------+-----------------------+
|   > fw_header         | off  | size | type        | name       | comment               |
|     chunk             | 0x00 | 4    | uint32_t    | magic      | 'FWUP'                |
| ▸ Enums               | 0x04 | 2    | uint16_t    | version    |                       |
| ▸ Typedefs            | 0x06 | 2    | uint16_t    | flags      | enum fw_flags         |
| ▸ Function types      | 0x08 | 8    | uint64_t    | length     |                       |
|                       | 0x10 | 4    | uint32_t    | crc        |                       |
| [New v] [Import C…]   | 0x14 | 12   | (padding)   |            |                       |
+-----------------------+------+------+-------------+------------+-----------------------+
| C: struct fw_header { uint32_t magic; uint16_t version; uint16_t flags; … };   [Copy]   |
+------------------------------------------------------------------------------------------+
```
Left `treeView`, right `tableView` with inline editors. States: type in use (shows use count before delete), parse error on import (line and message).

**Binary diff.**
```
+------------------------------------------------------------------------------------------+
| Diff  A: libfoo-1.2.so (9f2c…)    B: libfoo-1.3.so (c41b…)       matched 96% of 412     |
+----------------+-------+--------------------+--------------------+-----------------------+
| status         | score | A                  | B                  | matched by            |
| changed        | 0.81  | parse_hdr 401a40   | parse_hdr 401a80   | name + hash           |
| changed        | 0.64  | sub_402f00         | sub_403010         | call graph + fuzzy    |
| added          | —     |                    | check_len 4031c0   |                       |
| removed        | —     | old_copy 402a10    |                    |                       |
+----------------+-------+--------------------+--------------------+-----------------------+
| A  if (n < 16) {                    | B  if (n < 16 || n > MAX_HDR) {             (1)   |
|      return -1;                     |      return -1;                                    |
+-------------------------------------+----------------------------------------------------+
```
1. Side-by-side C or listing with changed lines tinted and marked `+`/`-`/`~` in the gutter. States: one side not analysed, architectures differ (matching by name only, stated).

**Send to VM.**
```
+---------------------------------------------------------------+
| Run fw_update.elf in a disposable VM                          |
| This app never runs the sample. Virtual Machines will.        |
| VM profile     [Linux analysis (disposable) v]                |
| Network        (•) none   ( ) simulated   ( ) Tor-only        |
| Snapshot       [x] before start    [x] discard on close       |
| Debug stub     [x] attach this workbench (GDB remote)         |
| Collect        [x] memory dump  [x] execution trace           |
|                                   [Cancel]  [Send to VM]      |
+---------------------------------------------------------------+
```

### 5.4 Menus

```
File      Open Binary…  Ctrl+O · Open Raw Image… · Open Project…  Ctrl+Shift+O · Open Recent ▸
          Add Sample to Project… · Save Project  Ctrl+S · Save Project As…  Ctrl+Shift+S
          Import ▸ (Annotations…, Fork Project (.retdec)…, C Types…, Trace…, Memory Dump…)
          Export ▸ (C…, JSON…, Listing…, Report…  Ctrl+Shift+R, Bundle…, Annotations…, Indicators…)
          Print  Ctrl+P · Close Tab  Ctrl+W · Quit  Ctrl+Q
Edit      Undo  Ctrl+Z · Redo  Ctrl+Shift+Z · Copy  Ctrl+C · Copy Address  Ctrl+Shift+C · Copy As ▸
          Rename  N · Set Type  Y · Comment  ; · Function Comment  Shift+; · Bookmark  Ctrl+B · Tag…  T
          Mark As ▸ (Code  C, Data  D, String  A, Array  *, Undefine  U) · Edit Function…  Alt+P
          Find  Ctrl+F · Find Next  Ctrl+G · Search Everything  Ctrl+Shift+F · Preferences  Ctrl+,
View      Overview  Alt+1 · Listing  Alt+2 · C  Alt+3 · Synced  Alt+4 · IR  Alt+5 · CFG  Alt+6
          Call Graph  Alt+7 · Strings  Alt+8 · Types  Alt+9 · Diff  Alt+0
          Sidebar  F9 · Inspector  F4 · Bottom Dock  Ctrl+J · Hex Pane  Ctrl+H* · Layouts ▸ · Full Screen  F11
Go        Go To…  G · Back  Alt+Left · Forward  Alt+Right · Xrefs To  X · Xrefs From  Shift+X
          Next Function  Ctrl+Down · Previous  Ctrl+Up · Entry Point · Follow  Enter · Return  Esc
Analysis  Analyse  F6 · Options…  Shift+F6 · Stop  Ctrl+. · Re-decompile Function  F7
          Unpack… · Match Library Code · Run Rules  Ctrl+Shift+Y · Batch Analyse…
Patch     Patch Bytes…  Ctrl+Shift+B · Patch Instruction…  Ctrl+Shift+I · NOP Out · Invert Branch
          Patch List · Export Patched File…
Compare   Diff Two Samples…  Ctrl+D · Port Annotations… · Pass Diff
Tools     Signature Studio · Rule Editor · Type Hierarchy · Script Console  Ctrl+` · Script Library
          Open in Hex Editor  Ctrl+Shift+H · Send to VM… · Attach Debugger…
Assist    Suggest Names  Ctrl+Shift+N · Summarise Function  Ctrl+Shift+A · Explain Selection · Chat
Help      Help  F1 · Keyboard Shortcuts · Architecture Reference · About
```

`*` `Ctrl+H` is reserved for replace system-wide; this app has no replace in code views, so the binding is offered and can be changed.

### 5.5 Keyboard and pointer

Single-key commands apply only when a code view or graph has focus and no text field is active.

| Key | Action | Key | Action |
|---|---|---|---|
| `N` | rename | `X` / `Shift+X` | xrefs to / from |
| `Y` | set type | `G` | go to |
| `;` | comment | `Enter` / `Esc` | follow / return |
| `Space` | toggle listing ↔ graph | `Tab` | toggle listing ↔ C |
| `C` `D` `A` `U` | code, data, string, undefine | `H` | cycle operand base |
| `Ctrl+B` | bookmark | `F7` | re-decompile function |

Context menus. Code views: Go To, Xrefs To, Xrefs From, Rename, Set Type, Comment, Bookmark, Tag, Copy Address, Copy Bytes, Patch ▸, Show in Hex, Open in Hex Editor, Show in Graph, Suggest Name. Function list: Open, Open in New Tab, Rename, Tag, Re-decompile, Show Callers, Show Callees, Export C. Graph node: Open in Listing, Focus N Hops, Hide, Collapse Group. Strings: Xrefs, Copy (defanged), Copy raw, Decode ▸.

Drag and drop: a file onto the window opens it in hostile mode when its origin is untrusted. A function onto the call graph focuses it. A type onto a variable applies it. A second sample onto the Diff tab sets side B. Out: selected C or listing as text; a graph as SVG.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Start page: "Drop a binary here" and Open. A note says files are parsed in a sandbox and never run. |
| Loading | Stage list with times; views enable as their data arrives; cancel always available. |
| Error | Unknown format: offers raw image dialog. Helper crash: stage named, partial results kept, sample hash logged. Out of memory: offers lower limits or range selection. |
| Offline | No change. The app has no network use. |
| Locked | Views blanked; analysis continues. |
| Read-only | Project without sample, or project on a read-only handle: no edits, banner says why. |
| Narrow (<640) | Docks become overlays; Synced view shows one pane with a pane switcher. |
| Hostile mode | Amber strip under the header: "Untrusted sample: assistant, debugger and sample-supplied rules are off". |
| No model | Assist menu items disabled with reason. |

### 5.7 Widgets

| Region | FUSE widget | Notes |
|---|---|---|
| Listing, C, IR | `codeEdit` (read-only) | Needs 10 M-line virtual documents, per-line data, gutter arrows, token hit testing (FUSE-WP-37, FUSE-WP-19); `DV07-GAP-1` |
| Function list, strings, xrefs, patches, diff table | `tableView` over `SqliteModel` | Multi-select, column chooser (FUSE-WP-28) |
| Header tree, types tree, hierarchy | `treeView` | Model-backed tree (FUSE-WP-28) |
| CFG, call graph | `nodeGraph` | Needs read-only mode, custom node content, edge labels and colours, minimap, 5,000 nodes (FUSE-WP-50); `DV07-GAP-2` |
| Entropy plot, progress waterfall | `chart`, `timeline` | |
| Hex pane | hex view | Missing today (FUSE-WP-41) |
| Side-by-side diff | diff view | Missing today (FUSE-WP-38) |
| Inspector | `propertyGrid` | |
| Notes, help, report preview | `markdownView` | FUSE-WP-42 |
| Docks and tabs | `dockArea`, `dockWidget`, `tabWidget`, `splitter` | Floating tool windows need FUSE-WP-31 |
| Script and rule editors | `codeEdit` | Grammar files for ForgeScript and rule syntax |
| Dialogs and controls | `dialog`, `lineEdit`, `comboBox`, `spinBox`, `checkBox`, `radioButton`, `slider`, `progressBar`, `popupMenu`, `menuBar`, `tooltip` | |

App gaps: `DV07-GAP-1` large read-only code view with address model. `DV07-GAP-2` graph view at this scale. `DV07-GAP-3` a library interface to the fork (today the GUI starts `retdec-decompiler` as a subprocess and reads sidecar files). `DV07-GAP-4` per-function re-decompile in the fork. `DV07-GAP-5` the fork built on HardenedBSD with its LLVM 8-era pin (Unverified). `DV07-GAP-6` a debug channel from VM01.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Project database access, navigation, view models, undo journal, script API, export |
| UI | FUSE UI | app host | All views |
| Loader | C++ (fork code: `fileformat`, `loader`, `fileinfo`, `cpdetect`, `pelib`, `pdbparser`, `demangler`, `ar-extractor`, `macho-extractor`, bytecode parsers) | helper `loader`, capability mode, sample (r) + scratch (rw) | Parse, triage, hashes, strings, entropy |
| Decompiler | C++ (fork code: `bin2llvmir`, `capstone2llvmir`, `llvmir2hll`, detectors, `stacofin`) | helper `decompiler`, capability mode | Lift, analyse, emit C, IR, listing, config JSON, line maps |
| Unpacker | C++ (fork `unpackertool`, `mini_emu`) | helper `unpacker`, capability mode, tighter CPU and memory limits | Static unpacking by interpretation |
| Rule engine | C++ (libyara, yaramod as vendored by the fork) | helper `rules`, capability mode | Compile and run rules |
| Assembler and patcher | C++23 (own) | helper `patcher`, capability mode | Encode instructions, apply patch records to a copy |
| Diff engine | C++23 (own) | helper `differ`, capability mode | Function matching and scoring |
| Graph layout | ForgeScript (small graphs), C++23 (large) | app host / `differ` helper | Layered and force layouts |
| Debug client | ForgeScript | app host | GDB remote protocol client over a VM01 channel |
| Assistant | ForgeScript | app host → `svc.model` | Prompt assembly, suggestion handling |

```
                        +------------------ app host ------------------+
  sample handle (r) --> | ForgeScript logic · FUSE UI · project.db     |
                        +--+------+-------+-------+-------+-------+----+
                           |      |       |       |       |       |
                           v      v       v       v       v       v
                        loader  decomp  unpack  rules  patcher  differ     (helpers:
                        no net, no fs beyond: sample r, scratch rw)         capability mode)
                           |
                           v  typed records over the bus (schemas), big blobs as descriptors
                        project.db (SQLite)  ·  analysis cache (by sample hash)

  svc.model <-- prompts with fenced, stripped text      VM01 <-- sample handle, debug channel
```

Rule: the app host never parses sample bytes. It only reads typed records the helpers produce, and it treats every string in them (names, strings, comments from debug info) as untrusted text for display.

**The fork's Qt GUI: what exists and what it does** (Verified, `RETDEC:src/gui/`, `RETDEC:include/retdec/gui/`, `RETDEC:README.md`):

| Path under `src/gui/` | What it does |
|---|---|
| `main.cpp`, `mainwindow.cpp` (3,764 lines) | Application shell: triage banner, five document tabs (Decompiled C, Assembly, IR, CFG, Synced), left Functions dock, right workspace, bottom output; open by dialog, drag and drop, argv, re-open last |
| `decompiler_launch.cpp`, `cli_tool_paths.cpp` | Builds `retdec-decompiler` arguments and starts it with `QProcess`; resolves tool paths |
| `artifact_loader.cpp` | Loads `.c`, `.config.json`, `.dsm`, `.ll` sidecars into the panels |
| `project_file.cpp` | `.retdec` JSON project |
| `panels/tri_pane_code_view.cpp` | Three-pane synchronised asm / IR / C view with line mapping, find, history |
| `panels/decompiled_c_panel.cpp`, `assembly_panel.cpp`, `ir_panel.cpp` | Highlighted read-only C, disassembly and SSA IR views |
| `panels/cfg_panel.cpp` | CFG: block items, edge kinds, loop regions, Sugiyama layout, minimap, SVG/PNG export |
| `panels/call_graph_panel.cpp` | Call graph: force layout, SCC collapse, filters, focus mode, DOT export |
| `panels/function_list_panel.cpp` | Function table with filters, inline rename, CSV/JSON export, tags |
| `panels/strings_browser_panel.cpp` | Strings and constants tabs with categories and filters |
| `panels/binary_browser_panel.cpp` | Sections, symbols, imports, exports tree with a hex dump |
| `panels/inspect_panel.cpp` | Runs `retdec-fileinfo --json` and optionally `retdec-unpacker` |
| `panels/target_panel.cpp` | Edit architecture, OS, entry for the project |
| `panels/type_hierarchy_panel.cpp` | Class tree and vtable viewer |
| `panels/signature_studio_panel.cpp` | Drives ar / Mach-O extraction, `bin2pat`, `pat2yara` |
| `panels/diff_panel.cpp` | Before/after text diff of decompiled output per recovery stage |
| `panels/ai_assistant_panel.cpp` | Chat window calling `retdec::neural` (llama.cpp) |
| `panels/live_console_panel.cpp`, `command_log_panel.cpp`, `progress_panel.cpp`, `triage_banner.cpp`, `remaining_panels.cpp` (diagnostics) | Streaming console, tool invocation log, stage progress dashboard, banner, problems list |
| `panels/settings_dialog.cpp`, `settings/settings.cpp` | Eight-tab settings dialog over `QSettings` |
| `settings/plugin_manager.cpp` | Loads shared-library plugins with `QPluginLoader` |
| `batch_decompile_dialog.cpp`, `batch_decompile_queue.cpp` | Batch decompile |
| `export_bundle.cpp`, `zip_writer.cpp` | Store-only ZIP of artefacts |
| `theme.cpp`, `resources/*.qss`, `resources.qrc` | Two QSS themes |
| `launch_options.cpp` | Headless mode (`--headless-decompile`, offscreen platform) |

Not present in the fork's GUI (so new here): a parser sandbox (its threat model states none), an analysis database beyond per-address annotations, a types editor, patching, binary-to-binary diff, a rule editor, scripting, debugger integration.

**Qt dependencies to remove** (Verified):

| Item | Where |
|---|---|
| `find_package(Qt6 … Core Gui Widgets Test)`, optional `Svg` | `RETDEC:src/CMakeLists.txt:94-108`, `RETDEC:src/gui/CMakeLists.txt` |
| Targets `retdec-gui`, `retdec-gui-panels`, `retdec-gui-launch`; AUTOMOC/AUTOUIC/AUTORCC | `RETDEC:src/gui/CMakeLists.txt` |
| Option `RETDEC_REQUIRE_QT6`; the `full-*` presets that require Qt | `RETDEC:cmake/options.cmake:86`, `RETDEC:CMakePresets.json` |
| `qtbase`, `qttools` | `RETDEC:vcpkg.json` |
| `windeployqt`, `macdeployqt`, offscreen platform plugin deployment | `RETDEC:src/gui/CMakeLists.txt`, `src/gui/install-windeployqt-gui.cmake.in` |
| Installer entries for `Qt6Core/Gui/Widgets/Network/OpenGL/Charts/Svg.dll` and platform plugins | `RETDEC:packaging/nsis/retdec.nsi` |
| Qt GUI tests (`Qt6::Test`, 20 files) | `RETDEC:tests/gui/` |
| Qt in scripts and CI (`check_qt_dynamic_link.py`, `qt-lgpl-evidence.yml`, `check_macos_bundle.py`, doctor and installer scripts) | `RETDEC:scripts/`, `RETDEC:.github/workflows/` |
| Qt entry in `NOTICE`; `docs/LGPL_QT.md` | `RETDEC:NOTICE`, `RETDEC:docs/LGPL_QT.md` |
| Qt classes in use: widgets (`QMainWindow`, `QDockWidget`, `QTabWidget`, `QTableView`, `QTreeView`, `QPlainTextEdit`, `QSyntaxHighlighter`, `QGraphicsView/Scene/Item`, dialogs), core (`QProcess`, `QSettings`, `QJson*`, `QFile`, `QThread`, `QTimer`, `QRegularExpression`), `QPluginLoader`, `QSvgGenerator` | throughout `src/gui/` |

The strings "Qt5"/"Qt6" in `RETDEC:src/module_cluster/module_cluster.cpp` are library-name tables for analysing other programs, not a dependency. The non-GUI libraries and tools build without Qt (`core-*` presets, `-DRETDEC_REQUIRE_QT6=OFF`).

### 6.2 Data model

Project store: `<name>.reproj`, SQLite with WAL, `meta(schema_version, app_version, created)`.

| Table | Fields |
|---|---|
| `sample` | `id: i64`, `sha256: blob(32)`, `blake3: blob(32)`, `tlsh: text`, `size: i64`, `name: text`, `format: text`, `arch: text`, `bits: u8`, `endian: text`, `os: text`, `base: u64`, `entry: u64`, `parent_id: i64?`, `parent_offset: i64?`, `origin: text`, `embedded: blob?` |
| `section` | `sample_id`, `name`, `vma: u64`, `vsize: u64`, `offset: u64`, `rsize: u64`, `flags: u32`, `entropy: f32`, `sha256` |
| `symbol` | `sample_id`, `addr: u64`, `name`, `demangled`, `kind: enum(func, object, import, export, section, file)`, `binding`, `lib`, `ordinal: i32?` |
| `function` | `sample_id`, `start: u64`, `end: u64`, `name`, `name_source: enum(auto, symbol, debug, signature, user, model)`, `cc: text`, `proto_type_id: i64?`, `is_library: bool`, `hash_norm: blob(16)`, `tlsh`, `insn_count: i32`, `flags: u32` |
| `block` | `function_id`, `start: u64`, `end: u64`, `loop_depth: u8` |
| `edge` | `function_id`, `src: u64`, `dst: u64`, `kind: enum(taken, fallthrough, jump, switch, exception)` |
| `call` | `sample_id`, `src: u64`, `dst: u64`, `indirect: bool`, `count: i32` |
| `xref` | `sample_id`, `src: u64`, `dst: u64`, `kind: enum(call, jump, read, write, addr)` |
| `string` | `sample_id`, `addr: u64`, `encoding`, `length: i32`, `category`, `text` |
| `type` | `id`, `kind: enum(base, struct, union, enum, typedef, pointer, array, func)`, `name`, `size: u32`, `def_json`, `source` |
| `field` | `type_id`, `offset: u32`, `bit_offset: u8`, `bit_size: u8`, `type_ref: i64`, `name`, `comment` |
| `applied_type` | `sample_id`, `addr: u64`, `scope: enum(data, local, param, ret)`, `type_id`, `var_key: text` |
| `comment` | `sample_id`, `addr: u64`, `kind: enum(line, block, function)`, `text`, `source: enum(user, model, script, import)` |
| `bookmark`, `tag` | `sample_id`, `addr`, `name`, `colour`, `note` |
| `patch` | `sample_id`, `addr: u64`, `old: blob`, `new: blob`, `note`, `enabled: bool` |
| `linemap` | `function_id`, `c_line: i32`, `ir_line: i32`, `asm_addr: u64` |
| `artifact` | `sample_id`, `kind: enum(c, ir, dsm, config, log)`, `settings_hash`, `path`, `blake3` |
| `rule_hit` | `sample_id`, `ruleset`, `rule`, `tags`, `offset: u64`, `length: i32` |
| `journal` | `seq: i64`, `ts: i64`, `user`, `op_json`, `inverse_json` |

Large artefacts (C text, IR, listing) live in the analysis cache under `~/.cache/org.pbsd.reverse-engineering/<sha256>/<settings_hash>/` and are rebuilt on demand; the project stays small. Schema migrates forward only with a copy of the old file.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| ELF, PE, COFF, Mach-O, Intel HEX, raw | yes | patched copy only | fork `fileformat`, `pelib`, `elfio` (MIT) | `loader` |
| `ar`, universal Mach-O | yes | no | fork `ar-extractor`, `macho-extractor` | `loader` |
| PDB, DWARF | yes | no | fork `pdbparser`, `debugformat` | `loader` |
| WebAssembly, JVM class/jar, DEX/APK, CIL, `.pyc`, `.luac` | yes | no | fork parsers | `loader` |
| Firmware containers (uImage, FIT, S-record, UEFI sections) | yes (Phase 2) | no | own code | `loader` |
| Authenticode | yes | no | fork `authenticode-parser` with OpenSSL (Apache-2.0) | `loader` |
| Machine code | yes | encode (own) | Capstone (BSD-3-Clause) for decode | `decompiler`, `patcher` |
| LLVM IR | internal | text | the fork's LLVM (Apache-2.0 with LLVM exceptions; an LLVM 8-era pin per `RETDEC:NOTICE`) | `decompiler` |
| YARA rules | yes | yes (text) | libyara (BSD-3-Clause), yaramod (MIT) | `rules` |
| Fork config JSON, `.dsm`, `.ll`, `.c` sidecars | yes | no | own readers | `decompiler` emits typed records; app host reads records only |
| `.retdec` fork project | yes | no | own | app host (small JSON, own schema check) |
| `.reproj` | yes | yes | SQLite (public domain) | app host |
| C declarations (type import) | yes | yes (header export) | fork `ctypesparser` or own subset parser | `loader` |
| GDB remote serial protocol | yes | yes | own | app host (text protocol, bounded packets) |
| STIX 2.1, CSV, JSON, Markdown, HTML, SVG, DOT, PNG | no | yes | own writers | app host |

Not used: Keystone (GPL-2.0, tool-only in the fork), llama.cpp (replaced by `svc.model`), Qt.

### 6.4 Integration

Intents handled: `open` and `view` for the MIME types below; `org.pbsd.reverse-engineering.goto` (sample hash, address) from DV04. Emitted: `open` to DV04 (handle, offset, read-only), VM01 (`org.pbsd.virtual-machines.run-sample`), DV05, AI02; `share`, `print`.

MIME view: `application/x-executable`, `application/x-sharedlib`, `application/x-object`, `application/x-elf`, `application/vnd.microsoft.portable-executable`, `application/x-mach-binary`, `application/x-archive`, `application/wasm`, `application/java-vm`, `application/x-dex`, `application/x-python-bytecode`, `application/x-lua-bytecode`, `application/octet-stream` (offer only, never default). Edit: `application/x-pbsd-re-project`. Export: `text/x-csrc`, `application/json`, `text/markdown`, `text/html`, `application/pdf`, `image/svg+xml`.

Services: `svc.portal.file`, `svc.settings`, `svc.notify`, `svc.intent`, `svc.model`, `svc.keyring`, `svc.audit`, `svc.thumbnail` (resource icons), `svc.portal.print`, `svc.clipboard`.

Verified from the fork: input formats and architecture maturity table; C is the only output for native binaries; detectors annotate and do not rewrite C; neural refinement uses llama.cpp with a syntax-only compile gate and no differential gate; the GUI runs the decompiler as a subprocess and loads four sidecar files; the fork's own threat model says parsing, YARA, unpacker emulation and the GUI subprocess are not sandboxed and that isolation is left to the operator (`RETDEC:docs/THREAT_MODEL.md`); formal verification in the fork covers arithmetic kernels with ESBMC, not parsers (`RETDEC:docs/FORMAL_VERIFICATION_BRIDGE.md`).

Proposed: everything else, including the helper split, the record schemas, the database, per-function re-decompile and all features marked "Proposed" in section 4.

## 7. Security and privacy

Capabilities requested:

| Capability | Reason |
|---|---|
| `fs.app` | Projects index, analysis cache, rule sets, scripts, optional quarantine store |
| `fs.portal` | Samples, projects, PDB files, exports the user picks |
| `net.none` | Stated explicitly: the app and all helpers have no network |
| `proc.helper:loader`, `:decompiler`, `:unpacker`, `:rules`, `:patcher`, `:differ` | Sandboxed native work on hostile bytes |
| `ai.local`, `svc.model` | Optional naming and summaries; off in hostile mode until enabled |
| `svc.portal.file`, `svc.settings`, `svc.notify`, `svc.intent`, `svc.keyring`, `svc.audit`, `svc.thumbnail`, `svc.portal.print`, `svc.clipboard` | As in section 6.4 |

Not requested by default: `proc.exec` (asked per project only for trusted local debug), any `net.*` beyond none, any `dev.*`, `bg.*`.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| Host and user data | Sample author | Parser memory corruption in format, debug-info, bytecode or rule code | All parsing in helpers in capability mode with one read handle and one scratch directory; no network; rlimits; W^X; helper crash is contained and reported |
| Host | Sample author | Getting the sample executed (double click, export, script, debugger) | No execute right on any handle; exports have no execute bit; no `proc.exec`; execution only in VM01 after an explicit dialog |
| Host | Sample author | Decompression bomb, analysis that never ends | Memory and time limits per stage; unpacker interpreter step limit |
| Analyst's judgement | Sample author | Misleading strings, fake symbols, text aimed at the model | Names from the sample are marked by source; model input fenced and stripped; model output only as suggestions with a mark |
| Host | Sample author | Rule or script delivered beside the sample | Rules and scripts from the sample's folder never auto-load; scripts run with reduced capabilities; unsigned scripts prompt |
| UI | Sample author | Control characters, bidirectional overrides, huge names | Display escaping of all sample-derived text; length caps; bidi controls shown as visible marks |
| Confidentiality of the sample | Other apps | Reading the sample or results | Cache and projects in app-private storage; search index gets nothing; clipboard copies of indicators are defanged |
| Collaboration | Colleague's project file | Malicious `.reproj` or annotation file | Opened as data with schema checks; no code in project files; scripts are separate and signed |
| Network | Sample (in a VM) | Calling home | Out of this app; VM profile defaults to no network |

Stored: project databases, analysis cache keyed by hash, optional sample copies in the quarantine store. Leaves the machine: nothing. Hashes are computed locally and never looked up online.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `analysis.profile` | enum | default | fast, default, deep |
| `analysis.max_memory_mb` | int | 8192 | Per-helper memory cap |
| `analysis.timeout_s` | int | 1800 | Per-sample time limit |
| `analysis.static_code_detection` | bool | true | Library matching |
| `analysis.auto_unpack` | bool | false | Unpack known packers without asking |
| `hostile.default_for_untrusted_origin` | bool | true | Hostile mode by origin |
| `view.asm_syntax` | enum | intel | intel, att |
| `view.sync_follow` | bool | true | Linked panes follow the selection |
| `view.graph_max_nodes` | int | 2000 | Above this, offer list or subgraph |
| `assist.enabled` | bool | false | Local model features |
| `assist.model` | string | empty | Model id in `svc.model` |
| `assist.strip_strings` | bool | true | Remove sample string literals from prompts |
| `scripts.require_signature` | bool | true | Refuse unsigned scripts |
| `export.defang_indicators` | bool | true | Defang URLs and IPs on copy and export |

## 9. Accessibility and languages

- Graphs have list equivalents: blocks with successors and predecessors; call graph as caller and callee lists.
- Listing and C lines expose address, text and comment as separate a11y fields; the screen reader can read "address, instruction" or instruction only.
- Linked selection is announced: "C line 14, matches 3 instructions".
- Sample-derived text is announced as written but bidirectional controls are spoken as "control character".
- Mnemonics, register names and C keywords are not translated. All chrome is.
- Colour never carries meaning alone: edge kinds have line styles and labels; diff lines have `+`, `-`, `~` marks.

## 10. Performance budgets

| Item | Budget |
|---|---|
| Cold start | 400 ms to start page |
| Triage overview of a 50 MB file | under 2 s |
| Listing scroll | 60 fps over 10 million lines |
| Largest supported input | 1 GB image for triage and listing; decompilation bounded by `analysis.max_memory_mb` |
| Navigation (follow reference) | under 50 ms from cache |
| Rename applied in all open views | under 200 ms without re-decompile |
| Xref query | under 50 ms for 1 million xrefs |
| CFG layout, 500 blocks | under 300 ms |
| Reopen analysed sample | under 1 s |
| App host memory, one 10 MB sample open | under 300 MB |

Decompile time depends on the fork; its README withdraws its own speed claims, so no budget is set here until measured.

## 11. Testing and proof

Test plan:

- **Unit (ForgeScript)**: journal and undo, address arithmetic, type layout, annotation merge, defanging.
- **Golden renders**: every view on a small fixed corpus (one binary per format and architecture).
- **Scripted UI**: the 15 top tasks through the a11y tree.
- **Fuzz**: every helper's input (formats, debug info, bytecode, rules, C declarations, annotation files, `.retdec` import, GDB packets). The fork has a standalone fuzz corpus and replay script (`scripts/standalone_fuzz.sh`) to seed from.
- **Differential**: helper output against the fork's CLI tools (`retdec-fileinfo --json`, `retdec-decompiler`) on the fork's corpus; disassembly against a second disassembler where one is available offline.
- **Sandbox tests**: a helper that tries to open a socket, a file or a process must fail and the attempt must appear in the audit log (needs G-06).
- **Never-execute test**: static check that no helper or app code path calls exec on a sample handle; run-time test that sample handles lack the execute right.
- **Patch round-trip and diff**: patch, export, reload, compare bytes; known pairs with ground-truth function matches.
- **Hostile text**: names and strings with control characters and bidi overrides render escaped.

Proof tiers (targets only; nothing has run):

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Sandbox launcher policy | Helpers get exactly the listed handles; no network | T2 | PRISM contracts on the launch path; tests on HardenedBSD | Depends on FORGE-WP-27 design |
| Patch application | Output differs from input only at patch ranges | T2 | PRISM contracts on the patch writer | — |
| Address and offset arithmetic | No overflow; VMA ↔ offset mapping consistent | T2 | PRISM `pir` on the kernels; the fork has ESBMC harnesses for similar arithmetic kernels | — |
| Own format code (firmware, GDB packets, annotation files) | No memory errors | T3 | Safe subset, sanitizers, fuzzing | Size |
| Fork parsers (`fileformat`, `pelib`, bytecode) | Contained, not proved | T4 | Fuzzing plus sandbox confinement | Legacy C++17 code; the fork's own docs say parsers are not verified |
| LLVM-based lifter and C back end | Contained, not proved | T4 | Differential tests | Size; LLVM |
| Undo journal | Applying inverse restores prior state | T3 | Property tests | ForgeScript code |
| UI | No tier | T4 | Golden and scripted tests | Not a proof target |

## 12. Build plan

Phase 1, usable core:

| ID | Work item | Depends on |
|---|---|---|
| DV07-W01 | Build the fork's non-GUI libraries and tools on HardenedBSD without Qt; remove Qt from build, packaging and CI as listed in 6.1 | DV07-GAP-5, G-18 |
| DV07-W02 | `loader` helper with typed records: headers, sections, symbols, imports, strings, entropy, hashes, detection | G-06, G-08, FORGE-WP-27, FORGE-WP-28 |
| DV07-W03 | `decompiler` helper wrapping the fork pipeline; emits C, IR, listing, line maps, config as records | DV07-GAP-3 |
| DV07-W04 | Project database, analysis cache, autosave | G-07 |
| DV07-W05 | Shell: docks, tabs, triage banner, status bar, progress, console, problems | G-02, FUSE-WP-31 |
| DV07-W06 | Overview view with risk flags and entropy plot | DV07-W02, FUSE-WP-46 |
| DV07-W07 | Listing, C and IR views with synchronised navigation | DV07-GAP-1, FUSE-WP-37, DV07-W03 |
| DV07-W08 | Function list, symbols, imports, strings, xrefs, go to, history | DV07-W04, FUSE-WP-28 |
| DV07-W09 | Raw image, archive member and slice dialogs; hostile mode | DV07-W02 |
| DV07-W10 | Export C; hex pane | FUSE-WP-41 |

Phase 2, full feature:

| ID | Work item | Depends on |
|---|---|---|
| DV07-W11 | Rename, retype, comments, bookmarks, tags with the undo journal | DV07-W07 |
| DV07-W12 | Per-function re-decompile | DV07-GAP-4 |
| DV07-W13 | Types and structures editor, C declaration import and export | DV07-W11 |
| DV07-W14 | CFG and call graph views with layout, filters, export | DV07-GAP-2, FUSE-WP-50 |
| DV07-W15 | `unpacker` helper and the unpack flow | DV07-W02 |
| DV07-W16 | Library matching and signature studio | DV07-W03 |
| DV07-W17 | `rules` helper, rule hits, rule editor, rule sets | DV07-W02 |
| DV07-W18 | Script console, script API, capability-reduced script runs | FORGE-WP-14, FORGE-WP-33 |
| DV07-W19 | Patch bytes, patch helpers, patch list, export patched file | DV07-W11 |
| DV07-W20 | Hand-off to and from DV04 | DV04 |
| DV07-W21 | Assistant: names, summaries, explain, with prompt hygiene | G-15, FORGE-WP-29, AI02 |
| DV07-W22 | Reports, JSON export, bundle, annotation exchange, fork project import | G-16 |
| DV07-W23 | Firmware carving, certificates, resources, overlay | DV07-W02 |

Phase 3, advanced:

| ID | Work item | Depends on |
|---|---|---|
| DV07-W24 | `differ` helper, diff views, diff report, annotation porting | DV07-W14, FUSE-WP-38 |
| DV07-W25 | Instruction assembler for x86-64 and ARM64 | DV07-W19 |
| DV07-W26 | VM hand-off and debugger client with debug views | VM01, DV07-GAP-6 |
| DV07-W27 | Trace and memory dump import | DV07-W26 |
| DV07-W28 | Struct from usage, class recovery view, fuzzy function match, local symbol store | DV07-W13, DV07-W16 |
| DV07-W29 | Project merge, snapshots, signed projects, STIX export | DV07-W22 |
| DV07-W30 | Batch analysis and batch scripts | DV07-W18 |

## 13. Open questions and risks

1. **HardenedBSD build is Unverified.** The fork documents Linux, macOS and Windows. It vendors a patched LLVM from an LLVM 8-era pin; building that with the OS toolchain and under W^X needs a spike.
2. **No library API for interactive use.** The fork's GUI works through a subprocess and sidecar files. Rename, retype and per-function re-decompile need either a long-lived helper holding the module in memory or changes to the fork's pipeline.
3. **Capsicum and LLVM.** The decompiler helper must open no files after entering capability mode. Signature files, type data and rule files must be passed as pre-opened handles; the fork's code opens them by path today.
4. **Language rule.** PLATFORM.md asks for C++23 memory-safe helpers checked by the MathScript plugin. The fork is C++17 and far outside that subset. It is allowed under rule (d) "wraps an existing engine" and relies on confinement (T4), which must be stated in the release notes.
5. **Licences.** The fork is AGPL-3.0-or-later or commercial; Keystone is GPL-2.0 and excluded here; yaramod, Capstone, libyara, OpenSSL, TLSH and elfio are permissive. A licence review of the vendored LLVM fork and data files fetched by `scripts/fetch-large-files.sh` (which downloads from upstream) is needed for offline builds.
6. **Large data files** are not in the fork's repository and are fetched at set-up. An offline, hashed package is required.
7. **Semantic detector quality.** The fork's own measured name-blind F1 is low; the UI must keep detector badges modest.
8. **Graph scale.** `nodeGraph` today is an editor for small graphs. Call graphs of large binaries need the list fallback from the first release.
9. **Debugger scope.** A full debugger is a product in itself. Phase 3 delivers a remote client only; local debugging of hostile code is excluded for good.
10. **Model misuse.** A model-suggested name can mislead. Marks and one-key reject are the mitigation; measuring suggestion quality is open.
