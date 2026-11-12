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

## 5. GUI

The reader is mostly heard, not seen. Its windows are small, plain and fully usable by speech and braille alone.

### 5.1 Window map

```
Screen Reader (background process, panel indicator in SH02)
├─ Reader menu (popup, RM+N)
├─ Settings window (main window)
│   ├─ Speech · Braille · Navigation · Verbosity · Punctuation and pronunciation
│   ├─ Security announcements · Sounds · OCR and description · Profiles · Scripts
│   ├─ Keys (rebinding) · Before login · Advanced
│   └─ dialogs: Add dictionary entry · New profile · Bind profile to app · Import/Export · Reset
├─ Elements list (dialog, RM+F7)
├─ Find in view (dialog, RM+Ctrl+F)
├─ Speech history and viewer (window, RM+F9)
├─ Braille viewer (window, always-on-top option)
├─ Reader command palette (popup, RM+Space twice or RM+K)
├─ OCR result (browse buffer window)
├─ Script manager (inside Settings › Scripts; source viewer window)
├─ Accessibility inspector (developer window)
├─ Tutorial (window)
├─ First-run greeting (dialog)
└─ Overlays drawn through svc.a11y:overlay: review-cursor rectangle, focus rectangle
```

### 5.2 Main window (Settings)

Shown at 900×600. Pattern: sidebar + content (DESIGN_SYSTEM section 4).

```
+----------------------------------------------------------------------------------------+
| [=] Screen Reader — Speech                 [search Ctrl+K]            [▁] [▢] [✕]      | (1)
+----------------------+-----------------------------------------------------------------+
| > Speech             | Profile: [Default            v] [New…] [Bind to app…]           | (3)
|   Braille            +-----------------------------------------------------------------+
|   Navigation         | Voice for navigation   [Formant: English (AU)        v] [Test] | (4)
|   Verbosity          | Voice for reading      [Neural: English, female 1    v] [Test] |
|   Punctuation        | Rate        80 [=========|------------------] 900   420 wpm    | (5)
|   Security           | Pitch        0 [=====|----------------------] 100    50        |
|   Sounds             | Volume       0 [===============|------------] 100    80        |
|   OCR and images     | Capitals     (•) raise pitch 20%  ( ) say "cap"  ( ) beep      |
|   Profiles           | [x] Switch voice by language of the text                        |
|   Scripts            | [x] Lower other audio while speaking      by [ 60 % ]          |
|   Keys               | Output device [Follow system default               v]          |
|   Before login       +-----------------------------------------------------------------+
|   Advanced           | Voices installed                                                | (6)
|                      |  Name                 Lang    Type     Size    Signer           |
| (2)                  | > Formant English     en-AU   formant  0.4 MB  system ✓         |
|                      |   Neural female 1     en      neural   62 MB   system ✓         |
|                      |   Formant Deutsch     de      formant  0.5 MB  system ✓         |
|                      |  [Manage voices in Model Manager…]                              |
+----------------------+-----------------------------------------------------------------+
| Reader on · focus mode · speech talk · braille none · a11y rights: 6 held [Details]    | (7)
+----------------------------------------------------------------------------------------+
```

1. Header bar. No `[net]` badge can appear: the app has `net.none`.
2. Sidebar, 220 px, `listView`. One page per row. Type-ahead moves by first letter.
3. Profile strip, `comboBox` + two `button`. Every page edits the selected profile.
4. Voice pickers, `comboBox`. `[Test]` speaks a fixed sample in the chosen voice.
5. `slider` rows with a live value `label`. Arrow keys step 1, PageUp/PageDown step 10. Each change is spoken once on release.
6. Installed voices, `tableView`, read-only. Signer column shows the trust badge word, not colour alone.
7. Status bar, `label` + `link`. `[Details]` opens the list of `svc.a11y` rights held (F082).

States: empty voice list (formant voice missing: error banner "No voice installed. Braille and sound cues still work." with a link to AI02); loading (voices enumerate under 100 ms, no spinner); error (speech helper down: banner with `[Restart helper]`); locked (window hidden while the session is locked); read-only (greeter reader has no settings window); narrow (sidebar becomes a `comboBox` above the content under 640 px).

### 5.3 Other views and dialogs

**Elements list** (560×480, `dialog`).

```
+------------------------------------------------------------+
| Elements in “Release notes — Document Viewer”         [✕]  |
+------------------------------------------------------------+
| (•) Headings ( ) Links ( ) Landmarks ( ) Fields ( ) Buttons| (1)
| Filter: [secur____________________]                        | (2)
+------------------------------------------------------------+
| > 1  Security changes                                      | (3)
|     2  Capability prompts                                  |
|     2  Signed updates                                      |
|   1  Known issues                                          |
+------------------------------------------------------------+
|                      [Move to]  [Activate]  [Cancel]       | (4)
+------------------------------------------------------------+
```

1. `radioButton` group; `Alt+H/L/D/F/B` pick a type. 2. `lineEdit`; filters as you type, result count is announced after 400 ms of no typing. 3. `treeView`; headings nest by level, other types are flat. 4. `[Move to]` moves the review or virtual cursor; `[Activate]` also invokes. States: empty ("No headings in this window"), loading (tree walk over 150 ms shows a `progressBar`).

**Speech history and viewer** (640×420, window).

```
+--------------------------------------------------------------------------+
| [=] Speech history                                    [Pin on top] [✕]  |
+--------------------------------------------------------------------------+
| Filter: [______________]   [x] Follow live   Source: [All        v]      | (1)
+--------------------------------------------------------------------------+
| 14:02:11  focus   Save button                                            | (2)
| 14:02:13  system  Security prompt from the system. Text Editor, signed   |
|                   by the system, asks to open files you choose. …        |
| 14:02:19  focus   Allow once button                                      |
| 14:02:20  —       (not kept: secure surface)                             | (3)
| 14:02:31  live    Download finished                                      |
+--------------------------------------------------------------------------+
| 5 of 500 kept · in memory only            [Copy line] [Save…] [Clear]    | (4)
+--------------------------------------------------------------------------+
```

1. `lineEdit`, `checkBox`, `comboBox` (focus, reading, live, system, script). 2. `listView`, monospace time column. 3. Withheld rows show only that something was withheld (F108). 4. `[Save…]` goes through `svc.portal.file`. States: empty ("Nothing spoken yet"), locked (cleared view, "History is frozen while locked").

**Braille viewer** (720×140, window).

```
+----------------------------------------------------------------------------------+
| Braille viewer — 40 cells, table: en-ueb-g1                     [Pin on top] [✕] |
+----------------------------------------------------------------------------------+
| ⠠⠎⠁⠧⠑⠀⠃⠥⠞⠞⠕⠝⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀                                   | (1)
| S a v e   b u t t o n                                                            | (2)
| status: [f][ ][ ][!]     [◀ Pan] [Pan ▶]     routing: click a cell               | (3)
+----------------------------------------------------------------------------------+
```

1. Cell row, `gridView` of 40 fixed cells in a braille-capable font; clicking a cell is a routing key. 2. Text under each cell, monospace `label`. 3. Status cells (mode, attributes, security `!` while a prompt is open), pan `button`s. States: no display and no virtual size set ("Choose a size: 20, 40, 80").

**Reader command palette** (popup, 520×360).

```
+--------------------------------------------------+
| [read al____________________________________]    | (1)
+--------------------------------------------------+
| > Read all from review cursor        RM+Down     | (2)
|   Read all from start of document    RM+Ctrl+Home|
|   Read current line                  RM+Up       |
|   Read clipboard                     RM+C        |
+--------------------------------------------------+
| Enter run · Ctrl+Enter rebind · F1 describe      | (3)
+--------------------------------------------------+
```

1. `lineEdit`, fuzzy match with `forge::text::fuzzy_filter`. 2. `listView`, command and current key. 3. Hint `label`. States: empty query lists the ten most recent commands.

**Accessibility inspector** (developer, 1000×640, workbench).

```
+------------------------------------------------------------------------------------------+
| [=] Accessibility inspector — Text Editor (org.pbsd.text-editor)   [Pick ⌖] [▁][▢][✕]    |
+------------------------------+-------------------------------+---------------------------+
| Tree                         | Properties                    | Events                    |
| v Window "notes.txt"         | role        Button            | 14:05:01 focus  #1e       |
|   v Group "toolbar"          | name        "Save"            | 14:05:01 name   #1e       |
|     > Button "Save"      (1) | state       focusable,focused | 14:05:03 value  #2a       |
|       Button "Open"          | bounds      12,40 64×24       | 14:05:04 live   #31       |
|   v TextField "editor"       | value       —                 |                           |
|                              | actions     Focus, Click  (2) | [Pause] [Clear] [Save…]   |
|                              | issues      none          (3) |                       (4) |
+------------------------------+-------------------------------+---------------------------+
| 212 nodes · 3 unlabelled [Show]                              contrast 7.2:1              |
+------------------------------------------------------------------------------------------+
```

1. `treeView` of the target app's tree; `[Pick]` selects by pointing. 2. `propertyGrid`, read-only; actions list is what the node offers. 3. Lint: missing name, zero bounds, focusable but hidden. 4. `tableView` event log. States: target closed ("App ended"), secure surface ("Inspection is not allowed on secure surfaces").

**First-run greeting** (dialog, 520×400; spoken on open).

```
+----------------------------------------------------+
| Welcome to the screen reader                       |
+----------------------------------------------------+
| Reader key    (•) Insert  ( ) CapsLock  ( ) both   |
| Key layout    (•) Desktop ( ) Laptop               |
| Speech rate   80 [=======|----------] 900          |
| [x] Start the reader when I log in                 |
| [ ] Start the reader on the login screen           |
| [Start the tutorial]            [Done]             |
+----------------------------------------------------+
```

`radioButton`, `slider`, `checkBox`, `button`. "On the login screen" writes a system setting and needs the admin prompt from SH03.

**Tutorial** (window, 720×520): `markdownView` lesson text on the left, a practice `panel` with real controls on the right, `progressBar` for lesson progress, `[Back] [Next] [Repeat]`.

### 5.4 Menus

```
Reader menu (RM+N)
├─ Settings…                         RM+Ctrl+,
├─ Profiles ▸ Default · <user profiles> · New… 
├─ Speech ▸ Talk · Beeps · Off · On demand        RM+S
├─ Braille ▸ Viewer · Table ▸ … · Tether ▸ Focus/Review/Both
├─ Tools ▸ Elements list RM+F7 · Find in view RM+Ctrl+F · Speech history RM+F9
│         · Recognise text RM+R · Describe image RM+Shift+R · Inspector
├─ Security ▸ What is in use RM+I · Reader rights RM+Shift+I · Open Permissions
├─ Help ▸ Keyboard help RM+1 · Tutorial · Command reference F1 · Context help RM+F1
├─ Restart with defaults             Super+Alt+S, Super+Alt+S
└─ Quit reader                       RM+Q

Settings window menu ([=], F10)
├─ File ▸ Import settings… · Export settings… · Close Ctrl+W
├─ Edit ▸ Undo change Ctrl+Z · Reset this page · Reset all…
├─ View ▸ Speech viewer · Braille viewer · Inspector
└─ Help ▸ Tutorial · Command reference F1 · About
```

### 5.5 Keyboard and pointer

Desktop layout defaults (laptop layout maps numpad keys to `RM+` letter keys). All rebindable in Settings › Keys.

| Keys | Command | Keys | Command |
|---|---|---|---|
| `Super+Alt+S` | reader on, off | `Ctrl` | silence |
| `Shift` | pause, resume | `RM+Down` | read all |
| `RM+Up` | current line | `RM+Tab` | focus description |
| `RM+Space` | focus/browse mode | `RM+F2` | pass next key |
| `Numpad 8/2` | review parent, child | `Numpad 4/6` | previous, next object |
| `Numpad 7/9` | previous, next line (flat) | `Numpad 1/3` | previous, next character |
| `Numpad Enter` | activate at review | `Numpad /` | click at review |
| `RM+Numpad -` | review to focus | `RM+Shift+Numpad -` | focus to review |
| `RM+T` | title, table cell | `RM+B` | read window |
| `RM+I` / `RM+Shift+I` | indicators, reader rights | `RM+F12` | time, date |
| `RM+P` | punctuation level | `RM+V` | verbosity level |
| `RM+Ctrl+Up/Down` | rate | `RM+Ctrl+Left/Right` | choose setting ring |
| `RM+F7`, `RM+F9` | elements list, history | `RM+R` | recognise text |
| `RM+1` | keyboard help | `RM+N` | reader menu |

Key routing: the reader holds `svc.a11y:keys.reader`. The compositor sends it (a) every chord that includes the registered reader modifier, (b) all keys of the focused window while browse mode is on, and (c) `Ctrl` and `Shift` taps while speech is active. Nothing else. On secure surfaces only class (a) and (c) apply, and typed characters are never delivered.

Context menu in history and viewer lists: Copy, Copy all, Add to pronunciation dictionary, Save. Drag and drop: a profile JSON file dropped on Settings opens Import. Pointer: hover speech is off by default (F043). Touch gestures are F056.

### 5.6 States

| State | Behaviour |
|---|---|
| Off | No process except the shell's chord listener. |
| Starting | Says "Screen reader on" within 1 s, then the focused element. |
| No audio device | Braille and viewer continue; a notification says speech has no output. |
| Speech helper crashed | Restart within 500 ms; tone, then "speech restarted"; queue is dropped except security class. |
| `svc.a11y` unavailable | Says "Accessibility service not running", retries every 2 s, flat review by OCR is offered if `dev.screen` is granted. |
| App without a tree | Says "This window has no accessibility information"; offers OCR. |
| Locked | Limited to the lock surface (F127). |
| Secure surface focused | Read-only review; `act` refused (F083); history off. |
| Model busy or missing | Neural voice falls back to formant; one cue, no speech interruption. |
| Offline | No effect. The app never uses the network. |

### 5.7 Widgets

| Region | Widget | Note |
|---|---|---|
| Settings sidebar, lists | `listView`, `tableView`, `treeView` | in tree (V3) |
| Sliders, toggles, choices | `slider`, `checkBox`, `radioButton`, `comboBox`, `spinBox`, `toggleSwitch` | in tree |
| Key rebinding | `tableView` + key-capture `lineEdit` | capture field is new: `AX01-GAP-3` |
| Help and tutorial text | `markdownView` | in tree |
| Inspector | `treeView`, `propertyGrid`, `tableView`, `splitter` | in tree |
| Script source viewer | `codeEdit` read-only | in tree |
| Braille cells | `gridView` + braille font | needs a font with U+2800 block; G-03 |

Gaps:

| ID | Gap | Evidence | Lands in |
|---|---|---|---|
| AX01-GAP-1 | A11y model has 22 roles and no tree, table, cell, heading, menu, dialog roles; no text ranges, attributes, live regions, descriptions or relations | `FUSE:Source/FUSE/UI/Core/include/fuse/ui/ui.hpp:86-101` | FUSE-WP-55 |
| AX01-GAP-2 | No low-latency PCM output path for a session app; FORGE has DSP only (resample, biquad) | `FORGE:docs/support.md` (`signal/audio.hpp`) | `svc.media` (G-09, G-11) |
| AX01-GAP-3 | Key-capture field for rebinding | not in V3 | FUSE-WP-14 |
| AX01-GAP-4 | Reader key routing and secure-surface flag in the compositor | no such module in `PBSD:pbsd/compositor` | compositor, FUSE-WP-56 |
| AX01-GAP-5 | Speech before the session exists (greeter, installer, disk unlock) | none read | SH03, AD01 |

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Reader core | ForgeScript | app host (`bg.run`) | Event handling, modes, cursors, verbosity, utterance building, profiles |
| Reader UI | FUSE UI | app host | Settings, dialogs, viewers, inspector |
| App scripts | ForgeScript | app host, child engine per script, `Caps` = none | Per-app hooks (F114) |
| Speech helper | C++23 | `speech-helper`, capability mode | Formant synthesis, text normalisation, queue and priorities, PCM to `svc.media`; neural audio arrives from `svc.model` as PCM |
| Braille helper | C++23 | `braille-helper`, capability mode | Display drivers, translation tables, key events; holds only the USB or serial handle passed by the portal |
| OCR client | ForgeScript | app host | Sends a window-scoped capture handle to `svc.model`; no pixels enter the script heap |
| `svc.a11y` | C++23 (platform) | system service | Aggregates app trees, checks rights, routes actions and keys |
| A11y bridge in each app | C++23 (FUSE) | each app host | `Bridge::sync`, `perform`, `enqueue`, `drain` (Verified, `FUSE:Source/FUSE/UI/A11y/include/fuse/ui/a11y/a11y.hpp`) |

```
 app host (any app)                      svc.a11y                    Screen Reader host
 UiContext ─ Bridge::sync ─ tree diff ─▶ rights check ─ events ───▶ reader core (ForgeScript)
            Bridge::enqueue ◀─ action ── secure-surface gate ◀ act ─┘   │        │
 compositor ── reader chords, browse-mode keys ─────────────────────────┘        │
 SH02 indicators, SH04 prompt events ── signed system events ───────────────────▶│
                                              ┌──────────────────────────────────┤
                                 speech-helper (C++23) ─ PCM ─▶ svc.media        │
                                 braille-helper (C++23) ◀─▶ HID braille display  │
                                 svc.model: neural voice, OCR, description ◀─────┘
```

What exists today (Verified): the per-app bridge with snapshot diff and five actions (Focus, Click, Increment, Decrement, SetValue); an optional AccessKit AT-SPI adapter that needs a prebuilt accesskit-c (`FUSE:Source/FUSE/UI/A11y/src/a11y_accesskit.cpp`, `FUSE:docs/ui/README.md`). Everything else in this table is Proposed. The reader does not use AT-SPI or D-Bus: `svc.a11y` speaks the session bus of PLATFORM section 7 (G-04, FUSE-WP-56).

### 6.2 Data model

Stores: `~/.config/org.pbsd.screen-reader/` through `svc.settings`; `~/.local/share/org.pbsd.screen-reader/reader.db` (SQLite, `schema_version` table). Greeter settings live in the system namespace `system.a11y.reader`.

| Entity | Fields |
|---|---|
| `profile` | `id: uuid`, `name: string`, `settings: json`, `created: i64`, `builtin: bool` |
| `app_binding` | `bundle_id: string`, `profile_id: uuid`, `sleep: bool` |
| `dictionary_entry` | `id: i64`, `scope: enum{global, voice, app}`, `scope_key: string`, `pattern: string`, `is_regex: bool`, `case_sensitive: bool`, `replacement: string`, `lang: string` |
| `custom_label` | `bundle_id: string`, `role: u8`, `tree_path: string`, `name_hash: blake3`, `label: string` |
| `key_binding` | `layout: enum{desktop, laptop, braille:<model>}`, `chord: string`, `command_id: string` |
| `script` | `id: string`, `bundle_id: string`, `path: string`, `blake3: bytes32`, `signed_by: string`, `enabled: bool`, `last_error: string` |
| `place_marker` | `doc_token: bytes16`, `slot: u8`, `anchor: json` |
| `tutorial_progress` | `lesson_id: string`, `step: u16`, `done: bool` |
| `braille_device` | `serial_hash: blake3`, `model: string`, `cells: u16`, `table_in: string`, `table_out: string` |
| `utterance` (memory only) | `seq: u64`, `time: i64`, `class: enum{focus, read, live, system, script}`, `text: string`, `withheld: bool` |

Wire record from `svc.a11y` (Proposed, extends the Verified `A11ySnapshotNode`: `node`, `parent`, `role`, `name`, `bounds`, `focusable`, `focused`, `checked`, `disabled`, `value`, `lo`, `hi`): adds `description`, `level`, `pos_in_set`, `set_size`, `expanded`, `live`, `secret: bool`, `secure_surface: bool`, `text_ranges`, `row`, `col`, `lang`. Migration is forward only with a copy of the old database kept (PLATFORM section 10).

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| `svc.a11y` tree and event records | yes | actions only | schema compiler output (G-08) | app host |
| Profile and dictionary export (JSON) | yes | yes | `forge::json` | app host (ForgeScript) |
| Braille table (own text format) | yes | no | own code | braille helper |
| liblouis tables (`.ctb`, `.utb`), if adopted | yes | no | liblouis (LGPL-2.1-or-later) or own reader, section 13 | braille helper |
| HID braille reports | yes | yes | own code | braille helper |
| Vendor serial braille protocols | yes | yes | own code, one module per family | braille helper |
| Voice model `.fpk` | by `svc.model` | no | FORGE | model service |
| Formant voice data (phoneme tables, rules) | yes | no | own code | speech helper |
| PCM 16-bit or float, 22.05/48 kHz | no | yes | `forge::signal` resample (Verified, `FORGE:docs/support.md`) | speech helper |
| CLDR annotation table (emoji names) | yes | no | generated `.tbl` | app host |
| Maths tree (MathML-shaped record) | yes | no | own code | app host |
| ForgeScript app scripts | yes | no | `forge::script::engine` | child engine, no capabilities |

### 6.4 Integration

- Intents: handles none; emits `open` (Permissions page for an app), `view` (help).
- Interfaces offered: `speech.say(text, lang, voice?, rate?) -> handle`, `speech.stop(handle)`, `speech.voices()` for read-aloud (F140); `reader.status()` for SH02's indicator. Announcements go through `svc.a11y:announce`, not through this app's own interface.
- Services used: `svc.a11y`, `svc.settings`, `svc.notify`, `svc.media`, `svc.model`, `svc.audit`, `svc.clipboard`, `svc.power`, `svc.portal.file`, `svc.portal.screen`, `svc.portal.usb`.
- Other apps: SH04 emits prompt events and marks secure surfaces. SH02 emits indicator events. SH03 hosts the greeter and lock readers. SY01 owns the on/off switch. AD05 lists and revokes the reader's rights. AI02 installs voices. SY03 exposes the terminal grid as text ranges. AX02 shares the focus and review rectangles. AX06 shares the command table so voice can run reader commands.
- FORGE (Verified): `forge::text::graphemes`, `words`, `sentences`, `direction`, `props`, capped regex, and `forge::locale` number, date, unit and plural formatting (`FORGE:docs/text-locale.md`). FORGE has no spellout and no dictionary word breaking for Thai, Lao, Khmer or Burmese (same file, Limits). The reader carries its own number spell-out rules.

What must be built for speech (none exists in the repos read):

| Piece | Owner |
|---|---|
| Formant synthesiser (cascade and parallel resonators, glottal source, noise) | this app, speech helper |
| Grapheme-to-phoneme rules and exception lexicon per language | this app |
| Text normaliser (numbers, dates, abbreviations, punctuation) | this app, on `forge::locale` |
| Prosody rules (stress, phrase contour, pauses) | this app |
| Rate change without pitch change (overlap-add time stretch) | this app |
| Word and index marks back to the reader | this app |
| Neural text-to-speech model class (acoustic model and vocoder sessions) | FORGE-WP-31.4 |
| Signed voice packages and a licence-clean trained voice | AI02; owner decision (FORGE plan Q11) |
| Low-latency PCM sink | `svc.media` (AX01-GAP-2) |

## 7. Security and privacy

A screen reader can read and drive every app. On this OS that power is a set of explicit rights on `svc.a11y`, not an ambient property of being an assistive tool.

### 7.1 Trust model of `svc.a11y` (Proposed)

| Right (manifest string) | Grants | Held by this app |
|---|---|---|
| `svc.a11y:tree.read` | Roles, names, states, bounds of all normal windows | yes |
| `svc.a11y:text.read` | Text content and ranges; never the value of a node with `secret = true` | yes |
| `svc.a11y:events` | Focus, caret, selection, value, live-region and window events | yes |
| `svc.a11y:act` | Actions on nodes; refused on secure surfaces | yes |
| `svc.a11y:keys.reader` | The three key classes in section 5.5 | yes |
| `svc.a11y:overlay` | Input-transparent rectangles over other windows | yes |
| `svc.a11y:secure.read` | Read-only tree of shell-drawn secure surfaces (prompts, lock, greeter) | yes |
| `svc.a11y:announce` | Ask a running reader to speak (any app, rate limited) | n/a (consumer) |
| `svc.a11y:input.synth`, `:view.transform` | Synthetic input; zoom and filters | no (AX03, AX06; AX02) |

Rules:

1. A bundle gets `svc.a11y` rights other than `announce` only if it is signed by the system key and the user turned the tool on in SY01. The enable prompt is drawn by SH04 and lists each right in words.
2. Rights narrow only. Scripts get a view limited to their bound app and no key access.
3. Secret fields never cross the service: the app-side bridge sends `secret = true` and an empty value (FUSE-R-102). The reader hears "password field, 8 characters" at most.
4. Secure surfaces accept no `act` from any assistive tool, and no synthetic input except from the shell's own trusted on-screen keyboard instance, whose every event must carry a fresh hardware-event serial (AX03 section 7.1). A prompt can only be answered by a physical act of the user: a key, a touch, a click or a switch press. This stops a compromised reader, script or voice command from granting capabilities to an app.
5. Every grant, action on another app and refusal is written to `svc.audit`. AD05 shows and revokes each right; revocation is immediate (SI-6).
6. System speech has a reserved earcon, phrase and queue class that only the reader core can use, and only for events that arrive on the signed system channel (F073–F075).

### 7.2 Capabilities requested

| Capability | Why |
|---|---|
| `fs.app` | Profiles, dictionaries, scripts, labels |
| `fs.portal` | Import, export, save speech log |
| `net.none` | The reader never needs a network |
| `proc.helper:speech-helper` | Synthesis outside the script engine; timing |
| `proc.helper:braille-helper` | Parses device bytes; holds the device handle |
| `svc.a11y` with the seven rights above | The app's function |
| `svc.settings`, `svc.notify`, `svc.media`, `svc.audit`, `svc.power` | Settings, notices, audio out and ducking, security events (read of own app and system indicator events only), battery reading |
| `svc.clipboard` | Speak or copy on a user command only (F059) |
| `svc.model`, `ai.local` | Neural voices, OCR, image description |
| `dev.screen` | OCR of one window rectangle through the screen portal; refused for secure and `no-capture` surfaces |
| `dev.usb:hid-braille` | Braille displays; prompt per device |
| `bg.run`, `bg.autostart` | Runs without a window; starts at login |

### 7.3 Threat model

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| Everything on screen | A malicious app | Fake a system prompt in speech ("Security prompt from the system…") | Spoof guard F075; reserved earcon F074; system text only from the signed channel |
| Capability grants | A malicious app or script | Drive the reader to click "Allow" | Rule 4: no `act` on secure surfaces |
| Passwords | Reader script, history, OCR | Read a secret field | Rule 3; F108; F103 |
| The user's attention | A noisy app | Flood announcements or live regions to mask a prompt | Rate limit F139; security class cannot be interrupted or queued behind app speech |
| Reader integrity | Hostile braille device | Malformed HID reports | Braille helper in capability mode; fuzzing; no other handles |
| Reader integrity | Hostile app tree | Huge or cyclic trees, 1 MB names | `svc.a11y` caps node count, depth and string length; reader truncates and says so |
| Privacy | Other local users | Read speech history | Memory only; cleared on lock option; never in backups |
| Pronunciation dictionary | Malicious import | Regex that never ends | FORGE regex step cap (Verified, `FORGE:docs/text-locale.md`) |
| Pre-login | Attacker at the greeter | Use the reader to learn user data | Greeter reader has no profiles, history, scripts or OCR (F126) |
| User scripts | Social engineering | "Install this script" | Per-script enable, visible source, no capabilities (F115–F117) |

Stored: settings, dictionaries, labels, scripts, tutorial progress. Not stored: spoken text, typed text, OCR results, images. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `reader.modifier` | enum{insert, capslock, both} | `insert` | Reader key |
| `reader.layout` | enum{desktop, laptop} | `desktop` | Key layout |
| `speech.voice.nav` | string | `formant:<system lang>` | Voice for navigation |
| `speech.voice.read` | string | same | Voice for read all |
| `speech.rate` | u16 wpm | 220 | 80–900 |
| `speech.pitch`, `speech.volume`, `speech.inflection` | u8 | 50, 80, 50 | 0–100 |
| `speech.capitals` | enum{pitch, word, beep, none} | `pitch` | Capital indication |
| `speech.capital_pitch_percent` | u8 | 20 | Pitch rise |
| `speech.auto_language` | bool | true | Switch voice by text language |
| `speech.duck_percent` | u8 | 60 | 0 disables ducking |
| `speech.device`, `cues.device` | string | `default` | Output devices |
| `braille.table.out`, `braille.table.in` | string | `<lang>-g1` | Translation tables |
| `braille.tether` | enum{focus, review, both} | `both` | What braille follows |
| `braille.flash_ms` | u16 | 3000 | Message time; 0 = until key |
| `braille.caret_blink_ms` | u16 | 500 | 0 = steady |
| `nav.auto_mode_switch` | bool | true | Focus/browse switching |
| `nav.pointer_speech` | bool | false | Speak under pointer |
| `verbosity.level` | enum{low, medium, high, custom} | `medium` | |
| `verbosity.order` | enum{name_role_state, role_name_state} | `name_role_state` | |
| `punctuation.level` | enum{none, some, most, all} | `some` | |
| `echo.typing` | enum{none, chars, words, both} | `chars` | |
| `echo.secret` | enum{silent, star} | `star` | Password fields |
| `progress.mode` | enum{off, speak, beep, both} | `beep` | |
| `progress.step_percent` | u8 | 10 | |
| `security.announce_indicators` | bool | true | Camera, mic, screen, location, network |
| `security.announce_first_contact` | bool | false | F080 |
| `security.prompt_voice` | string | `formant:<system lang>` | Fixed voice for system prompts; cannot equal a script voice |
| `history.size` | u16 | 500 | 0 turns history off |
| `history.clear_on_lock` | bool | true | |
| `ocr.enabled`, `describe.enabled` | bool | false | Need `ai.local` grant |
| `scripts.enabled` | bool | true | Master switch |
| `terminal.flood_lines` | u16 | 20 | F086 |
| `system.a11y.reader.greeter` | bool (system) | false | Start on the login screen |

## 9. Accessibility and languages

- The reader's own windows must pass their own inspector lint with zero issues.
- Every setting change is spoken as "name, new value". No setting needs sight to verify.
- Speech languages in Phase 1: the formant voice covers English, German, French, Spanish, Italian, Portuguese. Each further language needs rules and a lexicon (section 12). Interface strings follow the system locale and its plural rules.
- Right-to-left: braille and review follow logical order; the braille viewer mirrors only its chrome.
- Thai, Lao, Khmer, Burmese word review falls back to grapheme and space units until a dictionary breaker exists (`FORGE:docs/text-locale.md` Limits).
- Deafblind use: every sound cue and every security announcement has a braille form (status cell and flash message).

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Reader on to first speech | under 1 s |
| Key press to first audio sample, formant voice | under 50 ms at the 95th percentile |
| Focus change to first audio | under 80 ms |
| Neural voice first audio | under 150 ms (FORGE plan budget), else fall back |
| Silence after `Ctrl` | under 20 ms |
| Memory, reader core and helpers, no neural voice | under 60 MB |
| Largest tree handled without truncation | 100 000 nodes per window |
| Browse buffer build, 10 000 nodes | under 200 ms |
| Idle | zero wake-ups when nothing changes and nothing is spoken |
| Braille refresh after focus change | under 50 ms |

## 11. Testing and proof

Nothing has run. All of this is plan.

- Unit (ForgeScript): utterance builder for each role and verbosity level; text normaliser per language; punctuation tables; quick-nav search; flat review line builder; dictionary matching.
- Golden: speech text goldens ("what would be said") for scripted sessions over fixture apps; braille cell goldens per table; `.fuseui` renders of all reader windows.
- Scripted UI: drive fixture apps through a fake `svc.a11y` (FORGE-WP-35.4) and compare the utterance stream.
- Security tests: an app that speaks the reserved phrase; an app that floods announcements during a prompt; `act` on a secure surface; a script that asks for `fs`; secret field values at every layer.
- Fuzz: HID braille reports, vendor protocols, braille tables, profile import JSON, `svc.a11y` records with hostile sizes, formant voice data.
- Differential: contracted braille output against liblouis for the same tables, if own translation code is written; number spell-out against CLDR test data.
- Timing: latency meter (F018) recorded in CI on reference hardware.
- Human: test sessions with blind and deafblind users before each phase closes.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Speech queue and priority classes | A security-class utterance is never dropped, reordered behind, or interrupted by a lower class | T1 | Model of the queue state machine in Lean; refinement to code by PRISM | Needs PRISM support for the helper's concurrency |
| Spoof guard | No app-supplied text reaches the reserved voice, phrase or earcon | T1 | Information-flow proof on the utterance builder | ForgeScript verification tooling does not exist yet |
| Braille helper parsers | No runtime errors on any input | T2 | PRISM bounded checks at each entry point; fuzzing | — |
| Formant synthesiser DSP | No out-of-bounds, no NaN to the sink | T2 | PRISM plus sanitizers | Floating-point reasoning limits |
| Text normaliser | Terminates; output length bounded | T3 | Unit tests, fuzzing | Rule tables are data |
| Reader core (ForgeScript) | Memory safety by the interpreter | T3 | Interpreter guarantees; unit tests | Interpreter proof (FORGE-WP-01) |
| App scripts | Cannot gain capabilities | T2 | Policy compiler proof (FORGE-WP-08) applied to an empty set | Depends on that work package |
| Neural voice models | — | T4 | Golden audio comparison only | Models are not provable |

## 12. Build plan

### Phase 1: usable core

| ID | Work item | Depends on |
|---|---|---|
| AX01-W01 | `svc.a11y` client, event loop, focus and caret tracking | G-04, G-08, G-09, FUSE-WP-56, FORGE-WP-24.1 |
| AX01-W02 | Speech helper: queue, priority classes, PCM out | AX01-GAP-2, FORGE-WP-27, FORGE-WP-28 |
| AX01-W03 | Formant synthesiser and English rules | AX01-W02 |
| AX01-W04 | Text normaliser, punctuation and symbol tables | `forge::locale` (present) |
| AX01-W05 | Utterance builder, verbosity tables, focus mode | AX01-W01 |
| AX01-W06 | Object, flat and text review; read all | AX01-W05, AX01-GAP-1 (text ranges) |
| AX01-W07 | Key routing, layouts, pass-through, keyboard help | AX01-GAP-4 |
| AX01-W08 | Security announcements, spoof guard, reserved earcon | SH04, SH02, AX01-W02 |
| AX01-W09 | Settings window, profiles, first-run greeting | G-02, G-07 |
| AX01-W10 | Speech history and viewer; braille viewer (virtual) | AX01-W05 |
| AX01-W11 | Sound cues | AX01-W02 |
| AX01-W12 | Greeter, lock screen and installer mode | SH03, AD01, AX01-GAP-5 |
| AX01-W13 | Crash recovery and safe restart | G-07 |
| AX01-W14 | Five more formant languages | AX01-W03 |

### Phase 2: full feature

| ID | Work item | Depends on |
|---|---|---|
| AX01-W15 | Browse mode, quick-nav, elements list, find | AX01-GAP-1, FUSE-WP-55 |
| AX01-W16 | Braille helper: HID braille, uncontracted tables, routing, panning | `svc.portal.usb`, AX01-W10 |
| AX01-W17 | Neural voices through `svc.model` | G-15, FORGE-WP-29, FORGE-WP-31.4, AI02 |
| AX01-W18 | Tables, trees, live regions, progress | FUSE-WP-55 |
| AX01-W19 | Terminal output and review | SY03, G-13 |
| AX01-W20 | Per-app profiles, app scripts, script manager | FORGE-WP-08, AX01-W09 |
| AX01-W21 | Pronunciation dictionaries, custom labels, import and export | AX01-W09 |
| AX01-W22 | OCR of element and window | FORGE-WP-31.5, `svc.portal.screen` |
| AX01-W23 | Read-aloud interface `speech.say` | AX01-W02 |
| AX01-W24 | Tutorial with practice prompt | AX01-W08 |
| AX01-W25 | Accessibility inspector | AX01-W01 |
| AX01-W26 | Code reading; documents; Markdown | OF01, DV01, OF03, UT05 |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| AX01-W27 | Contracted braille and back-translation | decision in section 13 |
| AX01-W28 | Web content browse mode | G-14, IN01, FUSE-WP-51 |
| AX01-W29 | Maths speech and maths braille | OF07, AI04, OF04 |
| AX01-W30 | Image description and label guesses | FORGE-WP-31.6 |
| AX01-W31 | Serial and Bluetooth braille drivers, multi-line displays | VM05 |
| AX01-W32 | Touch gestures | FUSE-WP-08, compositor |
| AX01-W33 | Formatting announcements, charts, spreadsheets, diff | FUSE-WP-55, OF05, FUSE-WP-46 |
| AX01-W34 | Speech at disk unlock | boot loader work, outside this suite |

## 13. Open questions and risks

1. **Braille translation and liblouis.** liblouis is LGPL-2.1-or-later. That licence can be combined with this AGPL-3.0-or-later suite. The concern is safety, not licence: it is C code and a large table parser outside the safe C++23 subset. Options: (a) run liblouis unchanged inside the braille helper, which has no capabilities beyond the device handle; (b) write an own engine that reads liblouis table files, shipping the tables as separate LGPL data; (c) own tables and engine. Plan: own uncontracted tables in Phase 2; option (a) for contracted braille in Phase 3; option (b) as the long-term target with liblouis as the differential oracle. Owner to decide.
2. **Formant synthesiser effort.** A clear, fast formant voice per language is specialist work. eSpeak NG (GPL-3.0-or-later, C) is an existing engine and licence-compatible; it could run in the speech helper as an interim. It is not in the safe subset. Owner to decide between own code only and an interim import.
3. **Neural voice licences.** No trained voice is chosen. Training data and model licences must allow redistribution (FORGE plan Q11).
4. **Greeter and disk unlock.** Speech needs audio before login; the GELI passphrase stage has no session at all (AX01-GAP-5).
5. **Web content.** Browse mode on the web depends on a web engine that does not exist (G-14).
6. **Key routing design.** Sending browse-mode keys to the reader gives it a keylogger-shaped right. The class limits in section 5.5 narrow it, but the reader is in the trusted base for typed text outside secret fields. This must be stated in AD05.
7. **AccessKit.** The existing adapter path needs Rust at build time (PLATFORM V7). This spec assumes the native `svc.a11y` protocol instead; third-party toolkits (VM03 personalities) would need a bridge from AT-SPI, not planned here.
8. **Self-voicing and games.** Sleep mode (F057) is the only plan; game UI through FUSE's game HUD module is not covered.
9. **Compositor reality.** The compositor modules read are opcode tables of 20 to 40 lines each (`PBSD:pbsd/compositor/pbsd.compositor.wayland.*.cppm`); behaviour is Unverified.
