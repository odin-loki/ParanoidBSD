<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# On-Screen Keyboard

By Odin Loch

| | |
|---|---|
| ID | `AX03` |
| Bundle | `org.pbsd.on-screen-keyboard` |
| Category | access |
| Priority | P2 |
| Backing repos | FUSE (`Source/FUSE/UI/A11y`, `Source/FUSE/Core/include/fuse/platform/text_services.hpp`), FORGE (`docs/text-locale.md`), this repo (`pbsd/compositor`) |
| Status | Planned. Nothing built. |

## 1. Purpose

The On-Screen Keyboard lets people type and click without a physical keyboard: by touch, by pointer, by dwelling, by one or two switches, or with a gamepad. It serves tablet users and people with motor impairments. It replaces the Plasma virtual keyboard of the interim plan (PLATFORM V20).

## 2. Scope

In scope: key layouts and keyboard shapes; word prediction and correction display; swipe typing; emoji and symbol pages; dwell click; switch-access scanning of the keyboard and of the desktop; interplay with sticky, slow and bounce keys; auto-show on text focus; password-field privacy; greeter and lock screen use; haptic and sound feedback; one-handed modes; gamepad typing.

Out of scope:

| Item | Owner |
|---|---|
| Layout definitions, compose, dead keys, CJK and other engines, the prediction engine and the user dictionary | AX04 (this app is a front end to them) |
| Sticky, slow, bounce and mouse keys for physical keyboards; layout list | SY01 (compositor input settings) |
| Emoji data and search index | AX04 (shared picker data) |
| Dictation and voice commands | AX06 |
| Screen reading of keys | AX01 |
| Key injection into the display server | compositor (AX03-GAP-1) |
| Eye-tracker drivers | VM05; this app consumes pointer events only |

## 3. Users and top tasks

Personas: **Tia**, tablet user with no keyboard attached. **Sam**, uses a head pointer and dwell; cannot press a key. **Jo**, uses one switch at the cheek and scanning for everything. **Ravi**, one-handed after an injury, uses a compact floating layout. **Mel**, on a sofa with a gamepad. **Admin**, who must type a disk and login password on a kiosk with a touch screen.

| # | Task | Target steps |
|---|---|---|
| 1 | Keyboard appears when a text field gets touch or dwell focus | 0 |
| 2 | Show or hide the keyboard by hand | 1 (`Super+Alt+K`, panel icon, or edge swipe) |
| 3 | Type a word with prediction | first 2–3 letters + 1 tap |
| 4 | Swipe a word | 1 stroke |
| 5 | Type `Ctrl+Shift+T` | 3 taps (latched modifiers) |
| 6 | Enter a password on the lock screen | taps only; no prediction shown |
| 7 | Switch language layout | 1 (globe key) |
| 8 | Insert an emoji | 2 (emoji key, tap) |
| 9 | Left-click, right-click, double-click and drag by dwell | 0 for left; 1 dwell on the palette for others |
| 10 | Type a letter with one switch | 2 presses (row, key) |
| 11 | Answer a capability prompt with one switch | scan the prompt's buttons, 1 press, 1 confirm press |
| 12 | Move the keyboard out of the way | 1 drag, or auto-dodge 0 |
| 13 | Type with a gamepad | stick to a group, button for the letter |

## 4. Features

### 4.1 Layouts and keyboard shapes

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX03-F001 | System layouts | Every layout enabled in SY01 is available; key legends come from the layout definition owned by AX04, so the on-screen and physical layouts never differ. | M | 1 |
| AX03-F002 | Full shape | All keys of a 104/105-key board: function row, navigation block, optional numpad. | M | 1 |
| AX03-F003 | Compact shape | Letters, space, shift, backspace, enter, symbols switch; numbers by long press or page. Default on touch. | M | 1 |
| AX03-F004 | Split shape | Two halves at the left and right screen edges for thumbs; gap adjustable. | S | 2 |
| AX03-F005 | Floating shape | A free window; drag by its handle; resize 30–100% of output width; keeps its place per output. | M | 1 |
| AX03-F006 | Docked shape | Fixed to the bottom or top edge; reserves the area (exclusive zone) or overlays (setting). | M | 1 |
| AX03-F007 | Pages | Letters, numbers and symbols, more symbols, function and navigation, numpad, emoji. Page keys at fixed places. | M | 1 |
| AX03-F008 | Long-press alternates | Hold a key for accented and related characters from the layout's alternates table; slide to pick. Hold time 150–1000 ms. | M | 1 |
| AX03-F009 | Layout switch key | Globe key cycles enabled layouts; long press lists them. Uses the same input-source state as AX04. | M | 1 |
| AX03-F010 | Modifier keys | Shift, Ctrl, Alt, AltGr, Super: tap latches for one key, double-tap locks, third tap releases. State shown on the key. | M | 1 |
| AX03-F011 | Shortcut row | Optional row: Esc, Tab, arrows, Cut, Copy, Paste, Undo, Redo, Select all. | S | 2 |
| AX03-F012 | Terminal row | Optional row for SY03: Esc, Tab, Ctrl, `|`, `/`, `-`, `~`, arrows; shown when the focused app declares a terminal field. | S | 2 |
| AX03-F013 | Content-type layouts | Number pad for numeric fields, `@` and `.com`-free neutral row for e-mail, `/` and `.` for URL, date and time pads; from the text-input content purpose. | M | 2 |
| AX03-F014 | Custom layouts | User layouts in a JSON file: rows, key widths, labels, actions, alternates. Validated; stored in `fs.app`. | C | 3 |
| AX03-F015 | Layout editor | Simple grid editor for custom layouts. | C | 3 |
| AX03-F016 | Key size and gaps | Scale 50–200%; minimum key 44 px in touch density; gap 0–12 px. | M | 1 |
| AX03-F017 | Large-key mode | A reduced set of big keys (letters in 3 pages) for low accuracy pointing. | S | 2 |
| AX03-F018 | Alphabetical and frequency layouts | ABC order and a frequency-ordered layout for scanning users. | S | 2 |
| AX03-F019 | Right-to-left and complex scripts | Layouts mirror control keys for RTL; Indic, Thai and others show conjunct previews through AX04's engines. | S | 2 |
| AX03-F020 | CJK integration | The candidate row shows AX04 engine candidates; the keyboard shows the engine's key set (for example zhuyin or kana). | S | 3 |

### 4.2 Typing behaviour

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX03-F021 | Text commit path | Characters go to the focused field through the input-method connection (commit string). The field sees text, not fake key codes, so the layout of the physical keyboard does not matter. | M | 1 |
| AX03-F022 | Key event path | Keys with no text (arrows, F-keys, shortcuts with modifiers) are sent as key events through `svc.a11y:input.synth`. | M | 1 |
| AX03-F023 | Key repeat | Hold backspace, arrows, space: repeat after delay 250–1000 ms at 5–30 per second; accelerating delete by word after 2 s. | M | 1 |
| AX03-F024 | Activate on release | Keys fire on release (slide off to cancel) or on press (setting). | M | 1 |
| AX03-F025 | Key popup | Enlarged legend above the pressed key; off in password fields by default. | M | 1 |
| AX03-F026 | Multi-touch | Up to 10 contacts; rollover so fast two-thumb typing never drops a key. | M | 1 |
| AX03-F027 | Auto-capitalise | Capital after sentence end and at field start, when the field's hints allow. | S | 2 |
| AX03-F028 | Double-space full stop | Two spaces make ". " (setting). | C | 2 |
| AX03-F029 | Cursor control on space | Slide along the space bar to move the caret; with Shift latched, to select. | S | 2 |
| AX03-F030 | Backspace swipe | Swipe left from backspace deletes by word; release cancels if slid back. | C | 3 |
| AX03-F031 | Undo last correction | Backspace right after an auto-correction restores what was typed. | S | 2 |
| AX03-F032 | Hold delay filter | Ignore touches shorter than N ms (0–500), for tremor. Same idea as slow keys, applied to the on-screen keys only. | M | 2 |
| AX03-F033 | Repeat-touch filter | Ignore a second touch on the same key within N ms (0–1000). Same idea as bounce keys. | M | 2 |
| AX03-F034 | Edge guard | Ignore touches that start on the bezel-side 4 px unless they move inwards. | C | 3 |

### 4.3 Prediction and correction

The engine and dictionaries belong to AX04. This app shows results and reports choices.

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX03-F035 | Suggestion row | Three to seven suggestions above the keys: completions while typing, next words after a space. | M | 2 |
| AX03-F036 | Per-language models | The model of the active layout's language is used; a second language can be mixed in (setting in AX04). | M | 2 |
| AX03-F037 | Auto-correction | The middle suggestion replaces the typed word on space when the engine's confidence is over a threshold; off by default for pointer and switch users. | S | 2 |
| AX03-F038 | Touch model | Tap position relative to key centres is sent with each letter so the engine can weigh neighbouring keys. | S | 2 |
| AX03-F039 | Learn and forget | Picking a new word offers "add to dictionary"; long press on a suggestion offers "never suggest". Both act on AX04's visible dictionary. | M | 2 |
| AX03-F040 | Private dictionary only | Learning data stays in AX04's local store. Nothing is uploaded. | M | 2 |
| AX03-F041 | Phrase and snippet suggestions | AX04 text expansions appear as suggestions with a marker. | C | 3 |
| AX03-F042 | Next-word by local model | Optional: a small text model through `svc.model` re-ranks next-word suggestions using the surrounding text the field shares. Off by default. | C | 3 |
| AX03-F043 | Emoji suggestions | Typing a word offers its emoji as the last suggestion (setting). | C | 3 |
| AX03-F044 | Suggestion scanning | In switch scanning the suggestion row is the first group. | M | 2 |
| AX03-F045 | Incognito | A toggle key stops learning until turned off; forced on in private windows of IN01 and in fields marked sensitive. | M | 2 |

### 4.4 Swipe typing

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX03-F046 | Swipe decode | Drag through the letters of a word; the swipe helper matches the path against the lexicon and the language model. | S | 3 |
| AX03-F047 | Trail | A fading trail under the finger; off under reduced motion. | S | 3 |
| AX03-F048 | Alternatives | The suggestion row shows other readings of the path; tap replaces. | S | 3 |
| AX03-F049 | Auto space | A space is added before the next swiped word. | S | 3 |
| AX03-F050 | Mixed input | Tap and swipe mix freely in one word boundary-safe way: a tap after a swipe starts a new word. | S | 3 |
| AX03-F051 | Dwell-free swipe for pointers | Head-pointer and eye-pointer users can "swipe" without pressing: start and end by pausing on a key (dwell-free typing). | C | 3 |
| AX03-F052 | Off in sensitive fields | No swipe decoding in password or sensitive fields: the lexicon would leak nothing, but paths must not be stored or learned. | M | 3 |

### 4.5 Emoji and symbols

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX03-F053 | Emoji page | Categories, recents, search field (typed on the keyboard itself); data and recents shared with AX04's picker. | M | 2 |
| AX03-F054 | Skin tone and variants | Long press for variants; chosen tone remembered per emoji or globally. | S | 2 |
| AX03-F055 | Symbols pages | Currency, maths, arrows, punctuation, box drawing; long press for related symbols. | M | 1 |
| AX03-F056 | Kaomoji and special characters | Hand-off to AX04's full picker by a key. | C | 3 |
| AX03-F057 | Clipboard key | Opens AX05's quick paste popup; this app has no clipboard access itself. | S | 2 |

### 4.6 Dwell click and pointer aids

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX03-F058 | Dwell on keys | Resting the pointer on a key for the dwell time presses it; a ring fills to show progress. Dwell 200–3000 ms. | M | 2 |
| AX03-F059 | Dwell click anywhere | Resting the pointer anywhere performs the armed click type. Movement tolerance 2–40 px. | M | 2 |
| AX03-F060 | Click-type palette | A small always-on-top palette: left, right, middle, double, drag, scroll, hover only, pause. The next dwell uses the armed type, then returns to left (or stays: setting). | M | 2 |
| AX03-F061 | Drag by dwell | First dwell presses, second dwell releases; the pointer shows a drag badge. | M | 2 |
| AX03-F062 | Scroll by dwell | Scroll pad with up, down, left, right zones; speed grows with distance from centre. | S | 2 |
| AX03-F063 | Dwell lock areas | No dwell clicks while the pointer rests on the palette's "pause" or on a chosen rest corner. | M | 2 |
| AX03-F064 | Gesture confirm | Optional: a dwell only arms; a small move in a set direction fires (reduces unwanted clicks). | C | 3 |
| AX03-F065 | Dwell progress overlay | Ring at the pointer, drawn through `svc.a11y:overlay`; colour and size settings. | M | 2 |
| AX03-F066 | Different dwell for keys and desktop | Separate times; a longer time for Enter, Delete and for buttons of prompts. | S | 2 |

### 4.7 Switch-access scanning

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX03-F067 | Switch sources | Any keyboard key, gamepad button, pointer button, or HID switch-interface input can be a switch. The compositor delivers only the assigned inputs (section 7). | M | 2 |
| AX03-F068 | One-switch auto scan | The highlight moves by itself; the switch selects. | M | 2 |
| AX03-F069 | Two-switch step scan | One switch moves, the other selects. | M | 2 |
| AX03-F070 | Inverse scan | Hold to move, release to select. | S | 3 |
| AX03-F071 | Row-column scan | Rows first, then keys in the row. | M | 2 |
| AX03-F072 | Group scan | Blocks first (suggestions, left half, right half, bottom row), then rows, then keys. Depth 2–4. | M | 2 |
| AX03-F073 | Linear scan | Every key in turn; for small layouts. | S | 2 |
| AX03-F074 | Scan timing | Step time 0.2–5 s; first-item extra time; pause after select; loops before stopping 1–10. | M | 2 |
| AX03-F075 | Switch filters | Accept time (hold at least N ms), ignore repeat within N ms, long-press action (back one level, or open the scan menu). | M | 2 |
| AX03-F076 | Scan highlight | Thick border and optional fill in a high-contrast pair; optional sound cue or spoken label per step through AX01's `speech.say`. | M | 2 |
| AX03-F077 | Scan menu | After selecting, or on long press: Back, Repeat key, Hold modifier, Move keyboard, Point mode, Item mode, Stop. | M | 2 |
| AX03-F078 | Desktop item scan | Scan the focusable elements of the active window from the a11y tree (groups by container); select invokes the element's action. Needs `tree.read` and `act`, granted on first use. | S | 3 |
| AX03-F079 | Point scan | Two sweeping lines (x then y) pick a screen point; then the click-type menu. For apps with no tree. | S | 3 |
| AX03-F080 | Prompt scan | When a capability prompt is open, scanning moves to its buttons at once. Selecting "Allow" needs a second, separate confirm press after a 1 s guard; "Deny" does not. Performed by the trusted instance (4.9). | M | 2 |
| AX03-F081 | Frequency order | Scan order follows letter frequency for the language; suggestions first. | S | 3 |
| AX03-F082 | Error recovery | A "undo last" cell is the first item after every selection for one cycle. | S | 2 |
| AX03-F083 | Switch test page | Shows each press with its duration so timings can be tuned. | M | 2 |

### 4.8 Showing, hiding, placement

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX03-F084 | Auto-show | Shown when a text field gets focus by touch, dwell or switch, or always on focus when no physical keyboard is present. Signal: the text-input enable event relayed by AX04's input-method service. | M | 1 |
| AX03-F085 | Auto-hide | Hidden when the field loses focus, after a delay of 300 ms to avoid flicker; never hides while a key is held. | M | 1 |
| AX03-F086 | Physical keyboard rule | Typing on a physical keyboard hides the on-screen one (setting: always, never, when docked). | M | 1 |
| AX03-F087 | Manual toggle | `Super+Alt+K`, the SH02 panel icon, a bottom-edge swipe, or a floating handle. | M | 1 |
| AX03-F088 | Never takes focus | The keyboard surface has keyboard interactivity "none": the target field keeps focus and its caret. | M | 1 |
| AX03-F089 | Dodge the caret | A floating keyboard moves if it would cover the caret rectangle; a docked one asks the shell to pan or shrink the window's area. | M | 2 |
| AX03-F090 | Per-output placement | Shows on the output of the focused field; follows the touch device's mapped output. | M | 2 |
| AX03-F091 | Opacity | 30–100%; fades to the set value when the pointer leaves. | S | 2 |
| AX03-F092 | Minimise to handle | A small handle remains; tap restores. | S | 2 |
| AX03-F093 | Full-screen apps | Shown above full-screen windows; games may ask for no auto-show through a content-type hint. | S | 2 |

### 4.9 Password fields, greeter, lock screen, prompts

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX03-F094 | Password mode | In a field with the password purpose: no suggestion row, no learning, no swipe, no key popups, no per-key sound difference, no speech of letters. A lock icon and "Password mode" label show on the keyboard. | M | 1 |
| AX03-F095 | No learning from sensitive fields | Fields marked sensitive by the app (PIN, card number, one-time code, recovery phrase) get password mode for learning, with normal key popups allowed. | M | 1 |
| AX03-F096 | Shoulder-surf option | In password mode, optionally shuffle the number pad and remove press highlights. | C | 3 |
| AX03-F097 | Trusted instance | On the greeter, the lock screen, authentication prompts and capability prompts the shell runs a restricted instance of this keyboard inside the secure session: fixed layouts, no prediction, no custom layouts, no scripts, no user settings beyond size, shape and scanning timings from the system namespace. | M | 1 |
| AX03-F098 | Hardware provenance | Every key or click this app emits carries the serial of the hardware event that caused it (touch, pointer press, switch press, dwell end). The compositor refuses emitted input with no fresh serial. | M | 1 |
| AX03-F099 | Greeter and lock availability | An accessibility button on the greeter and lock screen shows the keyboard; a touch on a password field shows it too. | M | 1 |
| AX03-F100 | Installer | On the install media (AD01), including the disk passphrase step inside the installer session. | M | 1 |
| AX03-F101 | Layout indicator on lock screen | The active layout name is always shown: a wrong layout is the top cause of failed passwords. | M | 1 |
| AX03-F102 | Caps state | Caps lock state is shown on the key and as text in password mode. | M | 1 |

### 4.10 Feedback

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX03-F103 | Key sounds | Short clicks through `svc.media`: one for letters, one for space, one for delete, one for modifiers; volume 0–100; uniform sound in password mode. | S | 1 |
| AX03-F104 | Haptics | A short pulse per key on devices with a haptic actuator, through an input-device feedback portal; strength 0–100. Needs AX03-GAP-3. | C | 3 |
| AX03-F105 | Visual press | Key darkens for 80 ms. | M | 1 |
| AX03-F106 | Spoken keys | With AX01 running: explore by touch speaks the key, lift types it. Uses the a11y tree of the keyboard itself. | M | 2 |
| AX03-F107 | Gamepad rumble | A light rumble on select in gamepad mode (setting). | C | 3 |

### 4.11 One-handed and gamepad

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX03-F108 | One-handed left and right | The compact keyboard narrows to 60–80% and hugs one side; an arrow key flips sides. | S | 2 |
| AX03-F109 | Thumb arc | Keys laid on an arc around a bottom corner for one thumb on large tablets. | C | 3 |
| AX03-F110 | Gamepad grid typing | D-pad or stick moves a cursor over the keys; A selects, X backspace, Y space, B hide, triggers shift and symbols, bumpers move the caret. | S | 2 |
| AX03-F111 | Gamepad radial typing | Left stick picks one of eight groups, face buttons pick one of four letters in the group. | C | 3 |
| AX03-F112 | Gamepad prediction | Right stick left and right walks the suggestion row; click accepts. | C | 3 |
| AX03-F113 | Gamepad auto-show | Opens when a text field is focused and the last input device was a gamepad. | S | 2 |

### 4.12 Interplay with SY01 input aids

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX03-F114 | Sticky keys | The physical-keyboard sticky state (compositor, set in SY01) and the on-screen latches are one state: latching Ctrl on screen and pressing a physical `C` copies. The modifier keys on screen show the shared state. | M | 2 |
| AX03-F115 | Slow keys | Slow keys in SY01 applies to physical keys only. On-screen keys use F032. Emitted events bypass the slow-keys delay, or the user would wait twice. | M | 2 |
| AX03-F116 | Bounce keys | Same rule: SY01's filter is for physical keys; F033 is for on-screen keys. | M | 2 |
| AX03-F117 | Mouse keys | Not duplicated here; the click-type palette can drive SY01's mouse-keys click type. | C | 3 |
| AX03-F118 | Shared indicator | SH02 shows latched and locked modifiers for both sources in one indicator. | S | 2 |
| AX03-F119 | Settings links | Each related page links to the SY01 page, and SY01's Accessibility page links back. | M | 1 |

### 4.13 Appearance, settings, recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX03-F120 | Themes | Follows the system theme; high-contrast key theme; own key colour and border width. | M | 1 |
| AX03-F121 | Legend size and weight | 80–200%; bold option; lower-case legends when shift is off (setting). | M | 1 |
| AX03-F122 | Profiles | Named sets (touch, dwell, one switch, gamepad); switch from the keyboard's menu key. | S | 2 |
| AX03-F123 | Settings window | All settings with a live keyboard preview. | M | 1 |
| AX03-F124 | Crash recovery | The session restarts the keyboard within 1 s and re-shows it if a field has focus. Latched modifiers are released by the compositor when the emitter disconnects. | M | 1 |
| AX03-F125 | Stuck-key guard | The compositor releases any emitted key held over 10 s without a refresh. | M | 1 |
| AX03-F126 | Import and export | Profiles and custom layouts as JSON through the file portal. | C | 3 |
| AX03-F127 | Usage tips | First-run sheet per input style (touch, dwell, switch, gamepad). | S | 2 |
| AX03-F128 | No telemetry | No typed text, timing or usage data is stored (outside AX04's visible dictionary) or sent. | M | 1 |

## 5. GUI

### 5.1 Window map

```
On-Screen Keyboard (background process; indicator and toggle in SH02)
├─ Keyboard surface (layer surface, never focusable)
│   ├─ Suggestion row · Key area (pages: letters, symbols 1, symbols 2, function, numpad, emoji)
│   ├─ Long-press alternates popup · Layout list popup · Keyboard menu popup
│   └─ Shapes: full, compact, split, floating, one-handed, large-key, gamepad grid, gamepad radial
├─ Click-type palette (layer surface)
├─ Scroll pad (layer surface)
├─ Scan menu (popup on the keyboard or at the scanned item)
├─ Handle (minimised keyboard)
├─ Overlays through svc.a11y:overlay: dwell ring, scan highlight on desktop items, point-scan lines
├─ Settings window (main window)
│   ├─ Keyboard · Typing · Suggestions · Dwell · Switches · Gamepad · Feedback · Appearance
│   ├─ Showing and hiding · Profiles · Before login
│   └─ dialogs: Assign switch · Switch test · Layout editor · Import/Export
├─ First-run sheet
└─ Trusted instance (run by SH03/SH04 in the secure session): keyboard surface, scan highlight
```

### 5.2 Main window (the keyboard, full shape)

Shown docked at 1280×360 on a 1280×800 output. The Settings window is secondary and is in 5.3.

```
+----------------------------------------------------------------------------------------+
|  recei|      [ receive ]      [ receipt ]      [ received ]      [ recent ]     [👁‍🗨] | (1)
+----------------------------------------------------------------------------------------+
| [Esc] [F1][F2][F3][F4] [F5][F6][F7][F8] [F9][F10][F11][F12]        [PrtSc][Del]  [≡][✕]| (2)
| [ ` ][ 1 ][ 2 ][ 3 ][ 4 ][ 5 ][ 6 ][ 7 ][ 8 ][ 9 ][ 0 ][ - ][ = ][  ⌫   ]  [Ins][Hom][PU]| (3)
| [Tab ][ q ][ w ][ e ][ r ][ t ][ y ][ u ][ i ][ o ][ p ][ [ ][ ] ][  \  ]  [Del][End][PD]|
| [Caps  ][ a ][ s ][ d ][ f ][ g ][ h ][ j ][ k ][ l ][ ; ][ ' ][  Enter ]                |
| [Shift●  ][ z ][ x ][ c ][ v ][ b ][ n ][ m ][ , ][ . ][ / ][   Shift   ]       [ ↑ ]    | (4)
| [Ctrl][Sup][Alt][🌐 EN][            space            ][AltGr][☺][?123][Ctrl] [ ← ][ ↓ ][ → ]| (5)
+----------------------------------------------------------------------------------------+
```

1. Suggestion row, a `panel` with `button`s; the typed stem at the left; the eye-slash `toolButton` at the right toggles incognito (F045). Hidden in password mode.
2. Function row with the keyboard menu `[≡]` and hide `[✕]`.
3. Key rows. Each key is a `button` in the a11y tree with a name ("q", "backspace") so AX01 and scanning work.
4. `Shift●` shows a latched modifier; a filled bar under the legend means locked.
5. Bottom row: globe key with the layout code, emoji key, page key.

Key sizes: 1 unit = (width − gaps) / 15 in this shape; height 48 px normal, 44 px minimum. States: normal; password mode (row 1 replaced by "🔒 Password mode — nothing is suggested or learned"); loading (engine not ready: suggestion row shows nothing, keys work); error (emit refused: a banner on row 1 "Cannot type into this window" with the reason); locked (trusted instance only); read-only field (keyboard does not auto-show); narrow output (under 900 px wide the full shape is not offered and compact is used).

### 5.3 Other views and dialogs

**Compact shape** (720×300, default on touch).

```
+------------------------------------------------------------------------+
|        [ hello ]            [ help ]             [ held ]              |
| [ q ][ w ][ e ][ r ][ t ][ y ][ u ][ i ][ o ][ p ]                     |
|   [ a ][ s ][ d ][ f ][ g ][ h ][ j ][ k ][ l ]                        |
| [ ⇧ ][ z ][ x ][ c ][ v ][ b ][ n ][ m ][   ⌫   ]                      |
| [?123][🌐][ , ][          space          ][ . ][ ☺ ][   ↵   ]          |
+------------------------------------------------------------------------+
```

Long press on the top row gives digits. `[?123]` switches page. States as the main shape.

**Split shape** (on a 1280 px output).

```
+---------------------------+                              +---------------------------+
| [ q ][ w ][ e ][ r ][ t ] |                              | [ y ][ u ][ i ][ o ][ p ] |
| [ a ][ s ][ d ][ f ][ g ] |        (app content          | [ h ][ j ][ k ][ l ][ ⌫ ] |
| [ ⇧ ][ z ][ x ][ c ][ v ] |         stays visible)       | [ b ][ n ][ m ][ . ][ ↵ ] |
| [?123][🌐][   space   ]   |                              | [   space   ][ ☺ ][ ✕ ]   |
+---------------------------+                              +---------------------------+
```

Two layer surfaces anchored bottom-left and bottom-right. The suggestion row floats above the caret line.

**Emoji page** (compact shape).

```
+------------------------------------------------------------------------+
| [🔍 search____________]  [🕘][😀][🐻][🍎][⚽][🚗][💡][🔣][🏁]          | (1)
| 😀 😃 😄 😁 😆 😅 🤣 😂 🙂 🙃 😉 😊                                      | (2)
| 😇 🥰 😍 🤩 😘 😗 😚 😙 😋 😛 😜 🤪                                      |
| 😝 🤑 🤗 🤭 🤫 🤔 🤐 🤨 😐 😑 😶 😏                                      |
| [ABC]            [ space ]            [ ⌫ ]   skin [✋🏽 v]   [more…]    | (3)
+------------------------------------------------------------------------+
```

1. Search `lineEdit` (focusing it swaps the grid for letter keys) and category `tabBar`. 2. `gridView`, virtualised; long press shows variants. 3. Back to letters; skin tone `comboBox`; `[more…]` opens AX04's picker. Colour emoji need the font work of FUSE-WP-17.

**Scanning** (compact shape, group scan, depth 3).

```
+------------------------------------------------------------------------+
|#[ hello ]            [ help ]             [ held ]                    #| (1) group 1: suggestions
|╔══════════════════════════════════════════╗                            |
|║[ q ][ w ][ e ][ r ][ t ]║[ y ][ u ][ i ][ o ][ p ]                    | (2) highlighted group
|║[ a ][ s ][ d ][ f ][ g ]║[ h ][ j ][ k ][ l ]                         |
|║[ ⇧ ][ z ][ x ][ c ][ v ]║[ b ][ n ][ m ][   ⌫   ]                      |
|╚══════════════════════════╝                                            |
| [?123][🌐][ , ][          space          ][ . ][ ☺ ][   ↵   ]          | (3)
+------------------------------------------------------------------------+
  step 1.2 s · loop 1 of 3 · hold switch 1 s = back
```

1. Groups in scan order. 2. The highlight (double border, 6 px, colours from settings). 3. The status line under the keyboard shows timing and the long-press action; it is a `label`.

**Scan menu** (popup).

```
+---------------------------+
| > Back one level          |
|   Repeat last key         |
|   Hold Ctrl / Alt / Shift |
|   Point mode              |
|   Item mode (window)      |
|   Move keyboard           |
|   Stop scanning           |
+---------------------------+
```

`popupMenu`, scanned linearly.

**Click-type palette** (56×400 vertical, or horizontal).

```
+------+
| [◐L] |  left click (armed: filled)
| [ R ] |  right click
| [ M ] |  middle click
| [ 2 ] |  double click
| [ ⇄ ] |  drag
| [ ↕ ] |  scroll pad
| [ ◌ ] |  hover only
| [ ⏸ ] |  pause dwell
| [ ⌨ ] |  keyboard
+------+
```

`toolButton`s, each a dwell target with its own ring. Draggable by its top edge.

**Trusted instance on a capability prompt** (drawn by the shell session; one-switch scanning).

```
+--------------------------------------------------------------+
| ⚠ Security prompt from the system                            |
| Text Editor (signed: system ✓) wants to use the microphone.  |
|                                                              |
|   ╔════════╗                                                 |
|   ║ [Deny] ║   [Allow once]   [Allow this session]           | (1)
|   ╚════════╝                                                 |
|  Scanning: press to choose. “Allow” asks for a second press. | (2)
+--------------------------------------------------------------+
```

1. Scan starts on Deny. 2. Guard text is part of the prompt (SH04), not of this app.

**Settings window** (860×600, sidebar + content; page "Switches").

```
+------------------------------------------------------------------------------------+
| [=] On-Screen Keyboard — Switches             [search Ctrl+K]        [▁] [▢] [✕]   |
+-------------------+----------------------------------------------------------------+
|   Keyboard        | Scanning [ON ]    Method (•) auto ( ) step ( ) inverse         |
|   Typing          | Pattern  [Groups, then rows, then keys v]                      |
|   Suggestions     | Step time   0.2 [=====|---------] 5.0 s      1.2 s             |
|   Dwell           | First item extra  [ 0.5 s ]   Loops [ 3 ]   After select [0.3 s]|
| > Switches        +----------------------------------------------------------------+
|   Gamepad         | Switches                                                       |
|   Feedback        |  Name        Input               Action        Hold ≥   Ignore |
|   Appearance      | > Switch 1   Key: Space          Select        80 ms    300 ms |
|   Showing         |   Switch 2   Gamepad: A          Next          0 ms     200 ms |
|   Profiles        |  [Assign…] [Remove] [Test switches…]                           |
|   Before login    +----------------------------------------------------------------+
|                   | Highlight  [■ yellow] on [■ black]  Width [ 6 px ] [x] Speak    |
+-------------------+----------------------------------------------------------------+
| Preview: [ live compact keyboard with the scan running ]                           |
+------------------------------------------------------------------------------------+
```

Sidebar `listView`; `toggleSwitch`, `radioButton`, `comboBox`, `slider`, `spinBox`; switches in a `tableView`; `colorPicker` popups; the preview is the real keyboard document in a `panel`. **Assign switch** dialog: "Press the key or button to use" with a 5 s capture; only that input is then delivered. **Switch test** dialog: a `listView` of presses with duration and gap in ms.

### 5.4 Menus

```
Keyboard menu [≡]
├─ Shape ▸ Full · Compact · Split · Large keys · One-handed left · One-handed right
├─ Place ▸ Docked bottom · Docked top · Floating
├─ Layout ▸ <enabled layouts> · Layout settings… (SY01)
├─ Profile ▸ Touch · Dwell · One switch · Gamepad · <user>
├─ Incognito (no learning)                      toggle
├─ Suggestions                                  toggle
├─ Click palette                                toggle
├─ Scanning                                     Super+Alt+Shift+K
├─ Opacity ▸ 100% · 80% · 60% · 40%
├─ Settings…                                    Ctrl+,
└─ Hide keyboard                                Super+Alt+K

Settings window [=] (F10)
├─ File ▸ Import… · Export… · Close Ctrl+W · Quit Ctrl+Q
├─ Edit ▸ Undo change Ctrl+Z · Reset this page · Reset all…
├─ View ▸ Show keyboard · Click palette · Switch test
└─ Help ▸ Help F1 · Tips for touch / dwell / switches / gamepad · About
```

### 5.5 Keyboard and pointer

| Input | Action |
|---|---|
| `Super+Alt+K` | Show or hide the keyboard |
| `Super+Alt+Shift+K` | Start or stop scanning |
| `Super+Alt+J` | Show or hide the click-type palette |
| Swipe up from the bottom edge (touch) | Show; swipe the keyboard down to hide |
| Drag the handle or top edge | Move (floating); drag an edge to resize |
| Long press a key | Alternates popup |
| Slide on space | Move the caret |
| Two-finger tap on the keyboard | Keyboard menu |
| Right click or long press on a suggestion | Add to dictionary, Never suggest, Open dictionary (AX04) |
| Gamepad: A select, B hide, X backspace, Y space, LT shift, RT symbols, LB/RB caret, Start enter | Gamepad typing |

No drag and drop. The keyboard is not a drop target and is never the source of a drag.

### 5.6 States

| State | Behaviour |
|---|---|
| Hidden | No surface; process sleeps; wakes on a text-input event, toggle, or switch. |
| Shown, idle | Zero frames until input. |
| Password mode | F094. |
| Sensitive field | F095. |
| No input-method service (AX04 down) | Falls back to key events only for the active physical layout; a banner says suggestions and other layouts are unavailable. |
| No emit right | Keys are greyed; banner with a link to SY01 Accessibility. |
| Secure surface focused | This instance hides; the trusted instance shows (F097). |
| Engine loading | Keys work; suggestions empty. |
| Output rotated or changed | Re-layout within one frame; floating position clamped into the output. |
| Offline | No effect; the app never uses the network. |

### 5.7 Widgets

| Region | Widget | Note |
|---|---|---|
| Keys, suggestions, palette | `button`, `toolButton` in `panel`s with a grid layout | in tree; multi-touch per key needs FUSE-WP-08 |
| Emoji grid | `gridView`, `tabBar`, `lineEdit` | colour emoji: FUSE-WP-17 |
| Alternates, layout list, menus | `popupMenu`, popup `panel` | in tree |
| Settings | `listView`, `tableView`, `slider`, `spinBox`, `comboBox`, `toggleSwitch`, `radioButton`, `colorPicker` | in tree |
| Layout editor | `gridView` + `propertyGrid` | in tree |
| Dwell ring, scan highlight, swipe trail | custom paint through the draw list | FUSE-WP-23 for smooth arcs |

Gaps:

| ID | Gap | Evidence | Lands in |
|---|---|---|---|
| AX03-GAP-1 | Compositor interface for emitted keys and pointer buttons with hardware-provenance serials, stuck-key guard and a trusted-instance class | No virtual-keyboard or virtual-pointer module among `PBSD:pbsd/compositor/pbsd.compositor.wayland.*.cppm` (file names read) | compositor, FUSE-WP-56 |
| AX03-GAP-2 | Input-method panel connection: text-input enable/disable, content purpose, surrounding text, commit string, relayed to an on-screen keyboard | `textinput_v3.cppm` is an opcode table only; no input-method module seen | compositor, AX04, FUSE-WP-59 |
| AX03-GAP-3 | Haptic feedback portal for touch screens and gamepads | none read | VM05, `svc.portal` |
| AX03-GAP-4 | Delivery of only the assigned switch inputs to a background app | none read | compositor, `svc.a11y` |
| AX03-GAP-5 | Layer surface that is on top, never focusable, and can reserve an exclusive zone that changes at run time | `layer.keyboard.cppm` validates interactivity modes up to `OnDemand` (Verified); surface behaviour Unverified | compositor, FUSE-WP-10 |

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Keyboard core | ForgeScript | app host (`bg.run`) | Layout model, pages, modifier state, show and hide rules, scanning and dwell state machines, profiles |
| Keyboard UI | FUSE UI | app host | Key surfaces, popups, palette, settings |
| Swipe helper | C++23 | `swipe-helper`, capability mode, no handles but a read-only lexicon descriptor passed by AX04's service | Path-to-word decoding (speed; reason (b) of PLATFORM section 2) |
| Emit client | ForgeScript over a schema interface | app host | Sends key and button events with provenance serials |
| Input-method service | ForgeScript + C++23 (AX04) | AX04's service process | Text-input relay, commit, prediction, dictionary |
| Trusted instance | same bundle, restricted profile | started by SH03/SH04 in the secure session | Typing and scanning on secure surfaces |

```
 touch / pointer / switch / gamepad ──▶ compositor ──(events + serials)──▶ keyboard core
                                                                             │
        text ── commit string ──▶ AX04 input-method service ──▶ text-input ──▶ focused field
        keys ── key events + serial ──▶ compositor emit interface ──────────▶ focused window
        path ──▶ swipe helper ──▶ candidates ──▶ suggestion row
        stem + touch offsets ──▶ AX04 prediction ──▶ suggestions
        field focus, purpose, caret rect ◀── AX04 service / svc.a11y:track
```

### 6.2 Data model

Stores: `svc.settings` namespace `org.pbsd.on-screen-keyboard`; `~/.local/share/org.pbsd.on-screen-keyboard/layouts/*.json` for custom layouts; system namespace `system.a11y.osk` for the trusted instance. No typed text is stored by this app.

| Entity | Fields |
|---|---|
| `layout_view` (derived from AX04's layout definition) | `layout_id: string`, `lang: string`, `pages: Page[]`, `alternates: map<string, string[]>` |
| `Page` | `id: enum{letters, sym1, sym2, fn, numpad, emoji, custom}`, `rows: Row[]` |
| `Row` | `keys: Key[]`, `indent_units: f32` |
| `Key` | `id: string`, `label: string`, `label_shift: string?`, `action: Action`, `width_units: f32`, `scan_group: u8`, `repeat: bool`, `a11y_name: string` |
| `Action` | `kind: enum{text, keycode, modifier, page, layout_next, hide, menu, emoji, clipboard, command}`, `text: string?`, `keycode: u32?`, `mods: u8?`, `target: string?` |
| `switch_binding` | `id: u8`, `source: enum{key, gamepad, pointer, hid}`, `code: u32`, `device_hash: blake3?`, `role: enum{select, next, back, menu}`, `accept_ms: u16`, `ignore_ms: u16`, `long_press_ms: u16`, `long_action: enum` |
| `scan_config` | `method: enum{auto, step, inverse}`, `pattern: enum{linear, row_col, group}`, `step_ms: u16`, `first_extra_ms: u16`, `after_select_ms: u16`, `loops: u8`, `highlight: {fg: rgba, bg: rgba, px: u8}`, `speak: bool` |
| `dwell_config` | `key_ms: u16`, `desktop_ms: u16`, `guarded_ms: u16`, `tolerance_px: u8`, `default_click: enum`, `revert_to_left: bool`, `rest_corner: enum?` |
| `profile` | `id: uuid`, `name: string`, `shape: enum`, `place: enum`, `scale: f32`, `scan: scan_config?`, `dwell: dwell_config?`, `feedback: json`, `appearance: json` |
| `emit_event` (wire) | `kind: enum{key, button, axis}`, `code: u32`, `state: u8`, `mods: u8`, `cause_serial: u32`, `time_ms: u32` |

Custom layout files carry `"schema": 1`; unknown fields are refused. Limits: at most 8 pages, 8 rows per page, 24 keys per row, 16 alternates per key.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Custom layout JSON | yes | yes | `forge::json` + own validator | app host (ForgeScript) |
| Profile JSON | yes | yes | `forge::json` | app host |
| Layout definitions (AX04's format) | yes | no | AX04 module | AX04 service; this app gets a parsed record |
| Emit interface (keys, buttons, serials) | — | yes | schema compiler output (G-08) | compositor validates |
| Input-method panel interface (focus, purpose, commit, surrounding text) | yes | yes | schema compiler output | app host; AX04 service |
| Prediction interface (`predict`, `learn`, `forget`) | yes | yes | schema compiler output | AX04 service |
| HID switch-interface reports | by the compositor's input stack | no | — | compositor |
| Emoji data and names | yes | no | AX04's generated tables | AX04 service |
| Key sounds (PCM, bundled) | yes | no | own WAV-subset reader on bundle assets (trusted, signed) | app host |

### 6.4 Integration

- Intents: none handled; emits `view` (help) and `open` (SY01 pages).
- Interface offered: `osk.show()`, `osk.hide()`, `osk.state()` for SH02, SH03 and AX06.
- Services used: `svc.a11y` (rights in section 7), `svc.settings`, `svc.notify`, `svc.media` (key sounds), `svc.model` (only for F042), AX04's `input-method.panel` and `text.predict`.
- Other apps: AX04 (layouts, engines, prediction, dictionary, emoji data), SY01 (sticky, slow, bounce, layout list), SH02 (indicator, modifier state), SH03 and SH04 (host the trusted instance), SH05 (exclusive zone), AX01 (spoken keys), AX02 (dwell ring and scan highlight are tracked), AX05 (clipboard key), AD01 (installer).
- Verified in repos: FUSE delivers committed text and pre-edit as `TextInput` and `TextComposition` events and has `beginTextInput`, `endTextInput`, `setTextInputArea` for Win32 and X11 only (`FUSE:Source/FUSE/Core/include/fuse/platform/text_services.hpp`); FUSE events include touch down, move and up (`FUSE:Source/FUSE/Core/include/fuse/platform/events_v2.hpp`); FUSE UI Game has gamepad navigation (`FUSE:docs/ui/README.md`); the compositor tree has `textinput_v3`, `layer.keyboard` and `keyboard_shortcuts_inhibit` modules that are opcode tables (`PBSD:pbsd/compositor/`); the a11y bridge can `Click` a node (`FUSE:Source/FUSE/UI/A11y/include/fuse/ui/a11y/a11y.hpp`), which desktop item scan uses. Fuzzy and edit-distance functions exist in `forge::text` (`FORGE:docs/text-locale.md`). The rest is Proposed.

## 7. Security and privacy

### 7.1 Trust model

An on-screen keyboard is, by function, a program that can type into any app. Unchecked, that is the same power as malware that injects keystrokes. The design narrows it three ways.

1. **Explicit rights through `svc.a11y`** (system-signed bundle, enabled by the user in SY01 with an SH04 prompt, shown and revocable in AD05):

| Right | Grants | Limits |
|---|---|---|
| `svc.a11y:input.synth` | Emit key and pointer-button events to the focused window | Each event needs a fresh hardware serial (F098); rate cap 60 events a second; stuck-key guard |
| `svc.a11y:track` | Caret and focus rectangles | Geometry only |
| `svc.a11y:overlay` | Dwell ring, scan highlight, point-scan lines | Input-transparent; below secure surfaces |
| `svc.a11y:switch` | Receive the inputs the user assigned as switches | Only those codes; never other keys |
| `svc.a11y:tree.read`, `svc.a11y:act` | Desktop item scan | Off until the user turns item scan on; refused on secure surfaces for the user instance |

2. **Hardware provenance.** The app can only turn a real touch, click, switch press or completed dwell into input. The compositor checks the serial and its age (at most 5 s for scanning selections, 500 ms otherwise). A compromised keyboard process cannot type by itself while the user is away.
3. **A separate trusted instance for secure surfaces.** The user's instance cannot emit into the greeter, the lock screen or prompts at all. The shell starts a restricted instance inside the secure session; it has no prediction, no dictionary, no scripts, no custom layouts and no user profile. This is how a touch-only or switch-only user enters a password or answers a capability prompt, and it is the only assistive path that may do so. "Allow" on a prompt needs a second confirm press (F080).

The text path adds one more rule: text commits go through AX04's input-method service, which refuses commits when the focused field did not enable text input.

### 7.2 Capabilities requested

| Capability | Why |
|---|---|
| `fs.app` | Custom layouts, profiles |
| `fs.portal` | Import and export |
| `net.none` | No network use |
| `proc.helper:swipe-helper` | Swipe decoding speed |
| `svc.a11y` (`input.synth`, `track`, `overlay`, `switch`; optional `tree.read`, `act`) | Typing, clicking, placement, scanning |
| `svc.settings`, `svc.notify` | Settings, notices |
| `svc.media` | Key sounds |
| `svc.model`, `ai.local` | Optional next-word re-ranking (F042); off by default |
| `bg.run`, `bg.autostart` | Must be ready when a field gets focus |

Not requested: `svc.clipboard` (AX05 handles paste), `dev.usb` (switch interfaces are ordinary input devices handled by the compositor), `dev.screen`.

### 7.3 Threat model

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| Other apps | Compromised keyboard process | Inject commands (open a terminal, type) | Hardware provenance; rate cap; audit of emit bursts; no emit to secure surfaces |
| Capability grants | Compromised keyboard process | Press "Allow" | User instance cannot emit to prompts; trusted instance is restricted and needs provenance and a confirm press |
| Passwords | The keyboard itself | Log taps in password fields | Password mode has no learning and no storage; the trusted instance has no writable store at all; `net.none` |
| Passwords | Onlooker or screen recorder | Read key popups and highlights | No popups in password mode; keyboard surface is marked `no-capture` while in password mode |
| Typed text | Prediction learning | Sensitive words learned and later suggested | Password and sensitive fields never learn; incognito; visible, erasable dictionary in AX04 |
| The user | Malicious app | Fake a keyboard to capture typing | Apps cannot create top-layer surfaces; the real keyboard shows a system mark the shell draws |
| The user | Malicious app | Pop a window under a dwelling pointer or scanning highlight to catch a click | Dwell and scan selections are cancelled if the element under the target changed in the last 500 ms |
| Keyboard integrity | Hostile custom layout file | Oversized or malformed JSON | Strict validator with limits (section 6.2) |
| Swipe helper | Hostile lexicon or path data | Memory errors | Capability mode, no handles; fuzzing; T2 target |
| Switch input | Another local device | A second device sends the switch code | Binding may be tied to a device hash |

Stored: settings, profiles, custom layouts. Not stored: typed text, tap positions, swipe paths. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `enabled` | bool | false (true when a touch screen and no keyboard are present at first run) | Master switch |
| `shape` | enum{full, compact, split, large, one_left, one_right} | `compact` | Shape |
| `place` | enum{dock_bottom, dock_top, floating} | `dock_bottom` | Placement |
| `dock_reserves_area` | bool | true | Exclusive zone |
| `scale_percent` | u8 | 100 | Key size |
| `opacity_percent` | u8 | 100 | |
| `show.auto` | enum{touch_focus, always_no_keyboard, always, never} | `touch_focus` | Auto-show rule |
| `show.hide_on_physical_key` | enum{always, docked, never} | `always` | |
| `typing.fire_on` | enum{release, press} | `release` | |
| `typing.long_press_ms` | u16 | 400 | Alternates |
| `typing.repeat_delay_ms`, `typing.repeat_rate` | u16, u8 | 500, 15 | |
| `typing.hold_filter_ms`, `typing.repeat_filter_ms` | u16 | 0, 0 | F032, F033 |
| `typing.auto_capitalise`, `typing.double_space_stop` | bool | true, false | |
| `suggest.enabled` | bool | true | Suggestion row |
| `suggest.count` | u8 | 3 | 3–7 |
| `suggest.auto_correct` | bool | false | |
| `suggest.model_rerank` | bool | false | F042 |
| `swipe.enabled` | bool | false | |
| `dwell.enabled` | bool | false | |
| `dwell.key_ms`, `dwell.desktop_ms`, `dwell.guarded_ms` | u16 | 800, 1000, 1500 | |
| `dwell.tolerance_px` | u8 | 10 | |
| `dwell.revert_to_left` | bool | true | |
| `scan.enabled` | bool | false | |
| `scan.method`, `scan.pattern` | enum | `auto`, `group` | |
| `scan.step_ms`, `scan.first_extra_ms`, `scan.after_select_ms` | u16 | 1200, 500, 300 | |
| `scan.loops` | u8 | 3 | |
| `scan.speak` | bool | false | Needs AX01's speech service |
| `scan.prompt_confirm_guard_ms` | u16 | 1000 | Fixed minimum 500 |
| `gamepad.mode` | enum{grid, radial, off} | `grid` | |
| `gamepad.auto_show` | bool | true | |
| `feedback.sound_percent`, `feedback.haptic_percent` | u8 | 0, 50 | |
| `feedback.popup` | bool | true | Key popup |
| `appearance.theme` | enum{system, high_contrast, custom} | `system` | |
| `appearance.legend_percent`, `appearance.bold` | u8, bool | 100, false | |
| `password.shuffle_numpad`, `password.no_highlight` | bool | false, false | F096 |
| `system.a11y.osk.greeter` | bool (system) | true on touch-only devices | Button on greeter and lock screen |
| `system.a11y.osk.scan` | json (system) | — | Scanning timings for the trusted instance, copied on request with the admin prompt |

## 9. Accessibility and languages

- The keyboard is itself in the a11y tree: every key has a role, a name in the interface language and a state (latched, locked). AX01 explore-by-touch works on it.
- Scanning and dwell must be able to reach every function of this app, including its settings window, with one switch.
- Legends for all scripts need the FULL text profile (G-03); until then only layouts covered by the bundled font are offered.
- RTL layouts mirror the navigation keys and the suggestion row order.
- Key names for speech come from AX04's layout tables, per language.
- High-contrast key theme reaches 7:1.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Field focus to keyboard visible | under 120 ms |
| Touch down to visual press | next frame, under 16 ms |
| Touch up to commit sent | under 5 ms |
| Suggestions after a key | under 30 ms |
| Swipe decode after lift, 100 000-word lexicon | under 60 ms |
| Scan step timing error | under 10 ms |
| Memory, hidden | under 20 MB |
| Memory, shown with emoji page | under 50 MB |
| Idle while shown | zero frames, zero wake-ups (scan timer excepted) |
| Largest custom layout | 8 pages × 8 rows × 24 keys |

## 11. Testing and proof

Nothing has run. All of this is plan.

- Unit (ForgeScript): modifier latch and lock state machine; page switching; scanning state machine for each method and pattern with timed inputs; dwell state machine with jitter inside and outside tolerance; show and hide rules; custom layout validator.
- Golden: `.fuseui` renders of each shape and page per layout at three scales and in the high-contrast theme; scan highlight positions.
- Scripted UI: a fixture app with text, password, number and e-mail fields; assert committed text, the layout chosen, and that password mode hides the suggestion row; drive scanning with a simulated switch and check the typed text.
- Security tests: emit without a serial, with a stale serial, with a reused serial; emit at a secure surface from the user instance; a window raised under the dwell target; a fake top-layer surface from an app; a stuck key after killing the process.
- Fuzz: custom layout JSON; swipe helper inputs (paths, lexicon); emit interface in the compositor.
- Differential: swipe decoder accuracy against a recorded path corpus (to be recorded with consent, stored only in the test repository).
- Human: sessions with dwell and switch users; timing defaults are set from those sessions.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Compositor emit gate | An emitted event is accepted only with a fresh, unused hardware serial delivered to the emitting client, and never for a secure surface unless the client is the trusted instance | T1 | State-machine model and proof; refinement by PRISM | Compositor is a scaffold today |
| Prompt confirm rule | "Allow" cannot be selected by fewer than two distinct switch presses separated by the guard time | T1 | Model of the trusted instance's prompt scan | — |
| Modifier state machine | No modifier stays down after the client disconnects; latch clears after one key | T1 | Small model; proof | — |
| Swipe helper | No runtime errors on any input | T2 | PRISM bounded checks; fuzzing | — |
| Scanning and dwell state machines (ForgeScript) | No stuck state; every state has an exit by one switch | T2 | Exhaustive exploration of the model; unit tests | ForgeScript verification tooling |
| Layout validator | Rejects every file over the limits | T3 | Fuzzing, unit tests | — |
| Prediction quality | — | T4 | Corpus measurement in AX04 | Not formal |

## 12. Build plan

### Phase 1: usable core

| ID | Work item | Depends on |
|---|---|---|
| AX03-W01 | Emit interface with provenance serials in the compositor | AX03-GAP-1 |
| AX03-W02 | Keyboard surface: top layer, never focusable, exclusive zone | AX03-GAP-5, G-01, FUSE-WP-10 |
| AX03-W03 | Layout model from AX04 definitions; compact and full shapes; pages | AX04 layout format |
| AX03-W04 | Text commit through the input-method panel connection; key events for non-text keys | AX03-GAP-2, FUSE-WP-59, AX04 |
| AX03-W05 | Modifier latch and lock; repeat; long-press alternates | AX03-W03 |
| AX03-W06 | Auto-show, auto-hide, physical keyboard rule | AX03-W04 |
| AX03-W07 | Password mode and sensitive-field rule | AX03-W04 |
| AX03-W08 | Trusted instance for greeter, lock screen, prompts, installer | SH03, SH04, AD01, AX03-W01 |
| AX03-W09 | Multi-touch keys | FUSE-WP-08 |
| AX03-W10 | Settings window, indicator, key sounds, themes | G-02, G-07, SH02 |
| AX03-W11 | Crash recovery and stuck-key guard | AX03-W01 |

### Phase 2: full feature

| ID | Work item | Depends on |
|---|---|---|
| AX03-W12 | Suggestion row on AX04's prediction service; learn and forget; incognito | AX04 prediction |
| AX03-W13 | Dwell on keys and desktop; click-type palette; scroll pad; overlay ring | `svc.a11y:overlay`, AX03-W01 |
| AX03-W14 | Switch delivery and assignment | AX03-GAP-4 |
| AX03-W15 | Scanning: auto and step; row-column, group, linear; scan menu; test page | AX03-W14 |
| AX03-W16 | Prompt scan in the trusted instance with confirm rule | AX03-W08, AX03-W15 |
| AX03-W17 | Split, floating dodge, one-handed, large-key shapes | AX03-W02, `svc.a11y:track` |
| AX03-W18 | Content-type layouts, shortcut row, terminal row | AX03-W04, SY03 |
| AX03-W19 | Emoji page | AX04 emoji data, FUSE-WP-17 |
| AX03-W20 | Sticky state shared with the compositor; SH02 indicator | SY01, SH02 |
| AX03-W21 | Gamepad grid typing | FUSE gamepad input |
| AX03-W22 | Spoken keys and explore by touch | AX01 |
| AX03-W23 | Hold and repeat-touch filters; profiles | AX03-W05 |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| AX03-W24 | Swipe helper and swipe typing | AX04 lexicons, FORGE-WP-27, FORGE-WP-28 |
| AX03-W25 | Desktop item scan and point scan | FUSE-WP-55, FUSE-WP-56 |
| AX03-W26 | Inverse scan, frequency order, dwell-free typing | AX03-W15, AX03-W24 |
| AX03-W27 | Custom layouts and layout editor | AX03-W03 |
| AX03-W28 | CJK candidate integration | AX04 engines |
| AX03-W29 | Haptics and rumble | AX03-GAP-3 |
| AX03-W30 | Radial gamepad typing; thumb arc | AX03-W21 |
| AX03-W31 | Model re-ranking of next words | G-15, FORGE-WP-30 |
| AX03-W32 | Shoulder-surf options | AX03-W07 |

## 13. Open questions and risks

1. **Provenance for dwell and auto-scan.** A dwell has no press event; its "cause" is pointer motion that ended. The compositor must itself time the rest of the hardware pointer to vouch for a dwell, or accept the app's word within a window. The first is safer and is assumed; it moves dwell timing into the compositor.
2. **Two keyboards.** The trusted instance and the user instance are the same code with different rights. They can look different (no user theme in the trusted one). Users may find the switch between them jarring; the design keeps shape and size from the system namespace to reduce it.
3. **Disk unlock before the system starts.** No session exists at the GELI passphrase stage of a normal boot. Touch-only or switch-only users cannot type there. Same class of problem as AX01-GAP-5; needs boot-loader work outside this suite.
4. **Text commit versus key events.** Some apps (terminals, games, remote desktops in VM04) need key codes, not text. The rule "text by commit, the rest by key event" may need a per-field hint ("raw keys").
5. **Compositor reality.** The modules read are opcode tables; the `textinput_v3` table lists `CommitString`, `SetPreeditString` and `DeleteSurroundingText` as text-input requests (`PBSD:pbsd/compositor/pbsd.compositor.wayland.textinput_v3.cppm`), which upstream defines as events sent to the client. The module needs review before it is built on.
6. **Scan-and-click races.** The 500 ms change guard (section 7.3) may annoy users on busy windows; needs testing.
7. **Eye tracking.** Treated as a pointer. Gaze-specific needs (calibration, smoothing) are not planned.
8. **Prediction ownership.** This spec assumes AX04 ships its prediction service by Phase 2 of this app; if not, Phase 2 here has no suggestions.
