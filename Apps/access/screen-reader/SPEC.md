<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Screen Reader

By Odin Loch

| | |
|---|---|
| ID | `AX01` |
| Bundle | `org.pbsd.screen-reader` |
| Category | access |
| Priority | P1 |
| Backing repos | FUSE (`Source/FUSE/UI/A11y`), FORGE (`docs/text-locale.md`, `docs/support.md`), this repo (`pbsd/compositor`) |
| Status | Planned. Nothing built. |

## 1. Purpose

The Screen Reader turns the whole desktop into speech and braille for blind and low-vision users. It reads every app, the shell, the greeter, the lock screen and the installer, and it announces capability prompts and security indicators so a blind user gets the same security truth a sighted user sees. It replaces Orca and the AT-SPI stack of the Plasma plan (PLATFORM V20).

## 2. Scope

In scope: speech output, braille output and input, navigation modes, reader commands, verbosity, per-app profiles and ForgeScript scripts, security announcements, OCR and image description, sound cues, speech history, tutorials, pre-login use.

Out of scope:

| Item | Owner |
|---|---|
| Zoom, colour filters, focus highlight drawing | AX02 |
| Typing without a keyboard, switch scanning | AX03 |
| Input methods, layouts | AX04, SY01 |
| Dictation and voice commands | AX06 |
| Model install and verification | AI02 |
| The Accessibility page that turns tools on and off | SY01 |
| Drawing the capability prompt itself | SH04 |
| Greeter and lock screen surfaces | SH03 |
| OCR of screenshots for sighted users | ME05 |
| Text-to-speech as a general app service ("read aloud" in OF03, IN01) | this app offers `speech.say` (section 6.4) |

## 3. Users and top tasks

Personas: **Mira**, blind, daily keyboard user, speech at 450 words a minute. **Tom**, deafblind, braille display only. **Asha**, low vision, uses speech for long reading with AX02 for layout. **Dev**, an app developer who tests an app with the reader. **Admin**, who installs the OS for a blind user.

| # | Task | Target steps |
|---|---|---|
| 1 | Turn the reader on from anywhere, including the greeter | 1 chord (`Super+Alt+S`) |
| 2 | Hear what has focus and its state | 0 (automatic) or 1 (`RM+Tab`) |
| 3 | Read a document from the caret to the end | 1 (`RM+Down`) |
| 4 | Stop speech at once | 1 (`Ctrl`) |
| 5 | Jump to the next heading, link or form field in browse mode | 1 key (`H`, `K`, `F`) |
| 6 | Find a control by name in the current window | 3 (`RM+F7`, type, `Enter`) |
| 7 | Hear who is asking for a capability, what for, and answer | 0 to hear, 1 to answer |
| 8 | Ask "is the microphone, camera or network in use?" | 1 (`RM+I`) |
| 9 | Re-read something that was just said | 1 (`RM+F9` history) |
| 10 | Change speech rate | 1 (`RM+Ctrl+Up/Down`) |
| 11 | Read a terminal's new output automatically | 0 |
| 12 | Recognise text in an image or an unlabelled window | 1 (`RM+R`) |
| 13 | Learn what a key does without doing it | 2 (`RM+1`, press key) |
| 14 | Install the OS without sight | 0 extra: reader starts on a chord in the installer |

`RM` is the reader modifier: `Insert` or `CapsLock` (setting).

## 4. Features

### 4.1 Speech output

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX01-F001 | Formant voice | Built-in rule-based synthesiser in the speech helper. Under 20 ms to first audio. Default for navigation. Must be built (section 6.4). | M | 1 |
| AX01-F002 | Neural voices | `.fpk` voice models run through `svc.model`. Used for long reading. Falls back to formant if the model is busy or missing. | S | 2 |
| AX01-F003 | Voice list | Lists installed voices with language, type (formant, neural), size, signer. Install and verify is done in AI02. | M | 1 |
| AX01-F004 | Rate | 80 to 900 words a minute for formant; neural voices clamp to their trained range plus time-stretch. | M | 1 |
| AX01-F005 | Pitch, volume, inflection | Each 0–100. Volume is relative to the system output level. | M | 1 |
| AX01-F006 | Capital pitch change | Raise pitch by a set percent for capitals; or say "cap"; or beep. | M | 1 |
| AX01-F007 | Language switching | Switch voice by the `lang` attribute of text or by script detection (`forge::text::props`). Per-language voice map. | S | 2 |
| AX01-F008 | Interrupt and queue | Three priorities: interrupt (focus change), queue (reading), polite (live regions). Security announcements use a fourth, non-interruptible class (4.6). | M | 1 |
| AX01-F009 | Silence, pause, resume | `Ctrl` silences; `Shift` pauses and resumes at the word. | M | 1 |
| AX01-F010 | Speech modes | Talk, beeps only, off, on-demand (`RM+S` cycles). | M | 1 |
| AX01-F011 | Audio ducking | Lower other audio by a set amount while speaking, through `svc.media`. | S | 2 |
| AX01-F012 | Output device | Pick the output device; follow default; optional separate device for sound cues. | S | 1 |
| AX01-F013 | Rate boost | One toggle doubles the rate for skimming. | C | 2 |
| AX01-F014 | Text normalisation | Numbers, dates, times, units, currency read by locale using `forge::locale` formatting data; own spell-out rules per language (FORGE has no spellout: `FORGE:docs/text-locale.md` Limits). | M | 1 |
| AX01-F015 | Spelling | Spell current word or line; phonetic alphabet on second press. | M | 1 |
| AX01-F016 | Emoji and symbol names | Names from a bundled CLDR annotation table per language. | S | 2 |
| AX01-F017 | Voice per role | Different voice or pitch for links, headings, quotes, code comments. | C | 3 |
| AX01-F018 | Latency meter | Developer view: key press to first audio sample in ms. | C | 2 |

### 4.2 Braille

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX01-F019 | Display drivers | USB HID braille (the HID braille usage page) first; serial and Bluetooth vendor protocols after. Drivers run in the braille helper. | M | 2 |
| AX01-F020 | Display detection | `dev.usb:hid-braille` portal prompt on first connect; remembered per device serial. | M | 2 |
| AX01-F021 | Uncontracted tables | Six-dot and eight-dot computer braille, grade 1 literary, per language. Own table format. | M | 2 |
| AX01-F022 | Contracted tables | Grade 2 for English (UEB) first, then other languages. Translation engine choice in section 13 (liblouis is LGPL-2.1-or-later). | S | 3 |
| AX01-F023 | Back-translation | Braille keyboard input, contracted or not, to text. | S | 3 |
| AX01-F024 | Cursor routing | Routing keys move the caret or activate the element under the cell. | M | 2 |
| AX01-F025 | Panning | Pan left and right by display width; word-wrap panning; auto-pan while reading. | M | 2 |
| AX01-F026 | Status cells | Mode, line number, attributes, security state (a fixed cell shows when a capability prompt is open). | S | 2 |
| AX01-F027 | Braille follows | Focus, review cursor or both ("tether"). | M | 2 |
| AX01-F028 | Flash messages | Announcements shown for a set time, then the line returns. Security messages stay until acknowledged. | M | 2 |
| AX01-F029 | Caret and selection marks | Dots 7 and 8 mark caret and selection; blink rate setting. | M | 2 |
| AX01-F030 | Braille viewer | On-screen window that shows the cells and the text under them; for sighted helpers and developers. | M | 1 |
| AX01-F031 | Key mapping | Every reader command can be bound to display keys; per-display defaults. | M | 2 |
| AX01-F032 | Maths braille | Nemeth and UEB technical output from the maths tree (4.7). | C | 3 |
| AX01-F033 | Speech off with braille | Braille-only operation with no audio device at all. | M | 2 |
| AX01-F034 | Multi-line displays | Lay out lines and tables on displays with more than one row. | C | 3 |

### 4.3 Navigation modes

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX01-F035 | Focus mode | Keys go to the app; the reader speaks focus, caret and selection changes. | M | 1 |
| AX01-F036 | Browse mode (virtual cursor) | The reader builds a flat text view of a document or web page and moves its own cursor through it. Keys are quick-nav. | M | 2 |
| AX01-F037 | Auto mode switch | Enter focus mode on text fields and browse mode on leaving them; a tone marks each switch; `RM+Space` forces. | M | 2 |
| AX01-F038 | Object review | Move through the a11y tree: parent, first child, next and previous sibling, without moving focus. | M | 1 |
| AX01-F039 | Flat review | Review the window as lines laid out by element bounds: line, word, character. Works on apps with poor structure. | M | 1 |
| AX01-F040 | Text review | Character, word, line, sentence, paragraph in the focused text. Segmentation from `forge::text::graphemes/words/sentences`. | M | 1 |
| AX01-F041 | Read all | Continuous reading from caret or review cursor; follows the caret; stops on any key. | M | 1 |
| AX01-F042 | Route cursors | Move focus to review; move review to focus; move pointer to review; click at review. | M | 1 |
| AX01-F043 | Pointer tracking | Speak the element under the pointer; optional. | S | 2 |
| AX01-F044 | Table navigation | `Ctrl+Alt+Arrows` by cell; row and column headers announced; `RM+T` cell coordinates. Needs table roles (AX01-GAP-1). | M | 2 |
| AX01-F045 | Tree and list navigation | Level, position "3 of 12", expanded state. Needs tree roles (AX01-GAP-1). | M | 1 |
| AX01-F046 | Window list | Speak title, app, workspace and trust badge of each window; switch to one. | M | 1 |
| AX01-F047 | Where am I | One press: element. Two: window and app. Three: full path from window to element. | M | 1 |
| AX01-F048 | Place markers | Up to 10 markers per document; jump back; stored per document handle. | C | 3 |
| AX01-F049 | Skim reading | Read the first sentence of each paragraph. | C | 3 |

### 4.4 Quick-nav and commands

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX01-F050 | Quick-nav keys | Browse mode single keys: `H` heading, `1`–`6` level, `K` link, `U` unvisited, `V` visited, `F` form field, `B` button, `E` edit, `X` checkbox, `R` radio, `C` combo, `L` list, `I` item, `T` table, `G` graphic, `D` landmark, `Q` quote, `M` frame, `N` skip links, `S` separator, `A` annotation, `O` embedded object. `Shift` reverses. | M | 2 |
| AX01-F051 | Elements list | Dialog listing headings, links, landmarks, fields, buttons of the window; filter by typing. | M | 2 |
| AX01-F052 | Find in view | Find text in the browse buffer or flat review; next and previous. | M | 2 |
| AX01-F053 | Command palette | Every reader command, searchable by name; shows its key. | M | 1 |
| AX01-F054 | Key layouts | Desktop (numpad) and laptop layouts; both fully rebindable. | M | 1 |
| AX01-F055 | Pass-through | `RM+F2` sends the next key to the app untouched. | M | 1 |
| AX01-F056 | Gestures | Touch screen: one-finger explore, double-tap activate, two-finger read all, three-finger scroll. Needs compositor touch routing. | C | 3 |
| AX01-F057 | Sleep mode | Per-app: the reader goes silent and passes all keys (for self-voicing apps). | S | 2 |
| AX01-F058 | Time, date, battery, network | `RM+F12` time and date; `RM+Shift+B` battery; network state by route (clear, VPN, Tor). | M | 1 |
| AX01-F059 | Clipboard commands | Speak clipboard text; copy last utterance; append to clipboard. Uses `svc.clipboard` on a user gesture only. | S | 2 |
| AX01-F060 | Notification commands | Read last notification; open the list in SH02; do-not-disturb aware. | M | 1 |

### 4.5 Verbosity, punctuation, pronunciation

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX01-F061 | Verbosity levels | Low, medium, high, custom. Each level is a table: role, state, position, shortcut, description, tooltip on or off. | M | 1 |
| AX01-F062 | Order of information | Name-role-state or role-name-state; per level. | S | 2 |
| AX01-F063 | Punctuation levels | None, some, most, all; table of symbol to spoken name per language; `RM+P` cycles. | M | 1 |
| AX01-F064 | Typing echo | Characters, words, both, none. | M | 1 |
| AX01-F065 | Secret field echo | In password fields: nothing or "star" per key. The reader never receives the character (section 7). | M | 1 |
| AX01-F066 | Pronunciation dictionary | Global, per-voice and per-app entries: pattern (text or regex with FORGE's capped regex), replacement, case flag. | M | 2 |
| AX01-F067 | Repeated characters | "40 dashes" instead of forty reads. | M | 1 |
| AX01-F068 | Indentation | Speak or tone-code indentation changes; needed for code. | M | 2 |
| AX01-F069 | Formatting announcements | Font, size, bold, colour, alignment, spelling errors, links; each a toggle. Needs text attributes (AX01-GAP-1). | S | 3 |
| AX01-F070 | Progress bars | Speak every N percent, beep with rising pitch, or both; only the foreground window by default. | M | 1 |
| AX01-F071 | Live regions | Polite and assertive regions honoured; per-app mute. Needs live flags (AX01-GAP-1). | M | 2 |
| AX01-F072 | Number reading | Digits, pairs, or full numbers; hashes and addresses in the monospace token style are read in groups of four. | S | 2 |

### 4.6 Security announcements

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX01-F073 | Capability prompt reading | When SH04 shows a prompt the reader says, in fixed order: "Security prompt from the system", the app name, its signer state, the capability, the resource, the duration choices. It cannot be interrupted by app speech. | M | 1 |
| AX01-F074 | Trusted-source earcon | A sound that only the reader itself plays, and only for shell-drawn secure surfaces. Apps cannot play it through the reader (section 7). | M | 1 |
| AX01-F075 | Spoof guard | Text from an app is never spoken with the phrase or the voice reserved for system prompts. App speech that contains the reserved phrase is prefixed "app says". | M | 1 |
| AX01-F076 | Indicator announcements | Speak when camera, microphone, screen capture, location or network indicators turn on or off, with the app name. Source: SH02 indicator events. | M | 1 |
| AX01-F077 | Status query | `RM+I` reads all active indicators and which apps hold them. | M | 1 |
| AX01-F078 | Trust badge reading | Verified, unverified or tampered state is part of a window's and a file's spoken description. | M | 1 |
| AX01-F079 | Authentication prompts | Password and passkey prompts: reader says the requesting service and the reason; echo follows F065. | M | 1 |
| AX01-F080 | Network first contact | Optional announcement when the focused app makes a first contact to a new host (from `svc.audit`). | S | 2 |
| AX01-F081 | Lock and unlock | Announce session locked, unlocked, user switched. | M | 1 |
| AX01-F082 | Reader self-status | `RM+Shift+I` says which a11y rights the reader holds now and whether a script is active for this app. | S | 2 |
| AX01-F083 | Secure surface restriction | On secure surfaces the reader will read and move review, but its `act` requests are refused by the shell; the user answers with real keys. | M | 1 |
| AX01-F084 | Denial explanations | When an app action fails for lack of a capability, the reader reads the shell's denial toast and offers "open Permissions" (AD05). | S | 2 |

### 4.7 Text, terminal, code, maths, web

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX01-F085 | Editable text | Caret moves, selection changes, deletions and insertions spoken; needs text ranges (AX01-GAP-1). | M | 1 |
| AX01-F086 | Terminal output | New lines in SY03 are spoken as they arrive; diffing of the grid snapshot; flood control cuts at N lines and says "more output". | M | 2 |
| AX01-F087 | Terminal review | Flat review over the terminal grid; cursor tracking; prompt detection for jump to previous command. | M | 2 |
| AX01-F088 | Code reading | Indentation, line numbers on demand, syntax role by voice change, bracket matching announcement, diagnostics from DV01 read with severity. | S | 2 |
| AX01-F089 | Diff reading | Added, removed and changed lines announced with a word or a tone. | C | 3 |
| AX01-F090 | Maths reading | Reads a maths tree (MathML-shaped) exposed by AI04, OF07, OF04: speak whole, then explore sub-expressions with arrows. Speech rules are own tables. | S | 3 |
| AX01-F091 | Web content | Browse mode over IN01 content: headings, landmarks, forms, tables, live regions. Depends on G-14 exposing a document tree. | M | 3 |
| AX01-F092 | Documents | Page, heading and annotation navigation in OF03 and OF04; page number on demand. | S | 2 |
| AX01-F093 | Spreadsheets | Cell address, content, formula on second press, row and column headers in OF05. | S | 3 |
| AX01-F094 | Markdown and help | `markdownView` content exposes headings and links; browse mode works in UT05. | M | 2 |
| AX01-F095 | Bidi and mixed scripts | Reads in logical order; announces direction changes when verbosity is high. Uses `forge::text::direction`. | S | 2 |
| AX01-F096 | Charts | A chart exposes a summary and its data table; the reader offers "open as table". Needs `chart` widget support. | C | 3 |

### 4.8 OCR and image description

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX01-F097 | Recognise element | `RM+R`: OCR the review object. The pixels come from the screen portal scoped to that window rectangle. | S | 2 |
| AX01-F098 | Recognise window | OCR the whole focused window; result opens as a browse buffer with positions kept. | S | 2 |
| AX01-F099 | Click in OCR result | `Enter` on a recognised word clicks its screen position. | S | 3 |
| AX01-F100 | Image description | Local vision-language model through `svc.model` describes the image at the review cursor. Marked "machine description". | C | 3 |
| AX01-F101 | Unlabelled control guess | Offer an icon label guess; the user can save it as a custom label for that app. | C | 3 |
| AX01-F102 | Custom labels | User-given names for unlabelled controls, keyed by app id, role and tree path. | S | 2 |
| AX01-F103 | Secure surfaces excluded | OCR and image description refuse secure surfaces and windows marked `no-capture`. | M | 2 |

### 4.9 Sound cues and history

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX01-F104 | Sound cue set | Cues for focus mode, browse mode, link, button, checked, unchecked, error, wrap, progress, window open and close. Bundled PCM; replaceable sets. | M | 1 |
| AX01-F105 | Cue instead of role | Per role: speak, cue, or both. | S | 2 |
| AX01-F106 | Positional cues | Pan cues left to right by the element's x position. | C | 3 |
| AX01-F107 | Speech history | Ring of the last 500 utterances in memory; `RM+F9` opens the viewer; never written to disk unless the user saves. | M | 1 |
| AX01-F108 | History privacy | Utterances from secret fields, secure prompts and private windows are not kept. | M | 1 |
| AX01-F109 | Speech viewer | Live on-screen text of what is spoken; for developers and for deaf helpers. | M | 1 |
| AX01-F110 | Save log | Save history to a file the user picks through the file portal. | C | 2 |

### 4.10 Profiles and scripts

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX01-F111 | Configuration profiles | Named sets of all settings; switch by key; default, plus user-made. | M | 1 |
| AX01-F112 | Per-app profiles | A profile bound to a bundle id is applied while that app has focus. | M | 2 |
| AX01-F113 | Triggers | Apply a profile on "read all", on a braille display connecting, on a given output device. | C | 3 |
| AX01-F114 | App scripts | ForgeScript modules per bundle id: event hooks (`on_focus`, `on_text_changed`, `on_live_region`), custom commands, custom announcements. | S | 2 |
| AX01-F115 | Script sandbox | Scripts run in the reader's engine with capabilities narrowed to nothing: no `fs`, `net`, `proc`, `env` (PLATFORM V12). They see only the tree of their bound app. | M | 2 |
| AX01-F116 | Script signing | System scripts are signed with the bundle. User scripts live in `fs.app`, are listed, and need a one-time enable per script (PLATFORM V14). | M | 2 |
| AX01-F117 | Script manager | List, enable, disable, view source, show bound app, show last error. | S | 2 |
| AX01-F118 | Script API | `reader::say`, `reader::braille`, `reader::cue`, `reader::bind`, `tree::focus`, `tree::find`, `tree::children`, `tree::text`. No raw key access. | S | 2 |
| AX01-F119 | Import and export | Profiles and dictionaries as one JSON file through the file portal. | S | 2 |

### 4.11 Learning and help

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX01-F120 | Keyboard help mode | `RM+1`: each key press says its name and its reader command; nothing is executed. `Esc` twice leaves. | M | 1 |
| AX01-F121 | Tutorial | Spoken interactive lessons: basics, navigation, text, browse mode, security prompts, braille. Progress saved. | S | 2 |
| AX01-F122 | First-run greeting | Welcome dialog: modifier choice, rate, keyboard layout, start-at-login. | M | 1 |
| AX01-F123 | Command reference | Help pages in `help/*.md`, opened in browse mode. | M | 1 |
| AX01-F124 | Practice prompt | A safe sample capability prompt in the tutorial so users learn the reserved sound and phrase. | S | 2 |
| AX01-F125 | Context help | `RM+F1` speaks the commands that apply to the current element. | S | 2 |

### 4.12 Before login

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX01-F126 | Greeter | `Super+Alt+S` starts a restricted reader in the greeter session: formant voice, default profile, no scripts, no history, no OCR. | M | 1 |
| AX01-F127 | Lock screen | The user's reader keeps running but is limited to the lock surface; history is frozen. | M | 1 |
| AX01-F128 | Installer and live media | The reader and one formant voice per supported language are on the install image (AD01). | M | 1 |
| AX01-F129 | Early start setting | A system setting (stored outside the home dataset) starts the greeter reader automatically. | M | 1 |
| AX01-F130 | Disk unlock prompt | Speech at the GELI passphrase stage is out of reach of a session app; listed as a risk (section 13). | C | 3 |
| AX01-F131 | Braille before login | Braille helper may run in the greeter for HID braille displays only. | S | 3 |

### 4.13 Settings, recovery, privacy

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX01-F132 | Settings window | All settings in one window (section 5.2), fully usable by speech. | M | 1 |
| AX01-F133 | Start at login | Uses `bg.autostart`; toggle also in SY01. | M | 1 |
| AX01-F134 | Crash recovery | The reader is restarted by the session within 2 s and says "Screen reader restarted". The speech helper restarts on its own. | M | 1 |
| AX01-F135 | Safe restart key | `Super+Alt+S` twice restarts the reader with the default profile and scripts off. | M | 1 |
| AX01-F136 | No telemetry | Nothing spoken, typed or read leaves the machine. The app has `net.none`. | M | 1 |
| AX01-F137 | Reset | Reset one page or all settings; keeps a backup of the old profile. | S | 1 |
| AX01-F138 | Developer inspector | Tree view of the a11y tree with role, name, state, bounds and events log; for app authors. | S | 2 |
| AX01-F139 | Announce API | Other apps may ask for an announcement through `svc.a11y:announce`; rate limited to 4 a second per app; marked with the app name at high verbosity. | M | 1 |
| AX01-F140 | Read-aloud service | `speech.say` interface for OF03, IN01, UT05 to read documents with the same voices without running the full reader. | S | 2 |
