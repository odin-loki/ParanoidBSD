<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Voice

By Odin Loch

| | |
|---|---|
| ID | `AX06` |
| Bundle | `org.pbsd.voice` |
| Category | access |
| Priority | P3 |
| Backing repos | FORGE (`docs/support.md` signal headers, `docs/text-locale.md`), FUSE (`Source/FUSE/UI/A11y`), this repo (`pbsd/compositor`) |
| Status | Planned. Nothing built. |

## 1. Purpose

Voice lets people write and control the desktop by speaking: dictation into any text field, and commands for windows, apps, clicking and editing. All recognition runs on the machine through `svc.model`; audio is never stored unless the user asks and never leaves the machine. It serves people who cannot use a keyboard or pointer, and anyone who prefers to talk. Nothing in the interim Plasma plan does this (PLATFORM V20).

## 2. Scope

In scope: streaming dictation; punctuation and formatting by voice; custom vocabulary; voice control (windows, apps, click by name, number and grid overlays, text editing, keys, pointer); custom commands in ForgeScript; optional local wake word; microphone indicator and hard-mute behaviour; privacy rules; rules for security prompts; noise handling; adaptation.

Out of scope:

| Item | Owner |
|---|---|
| Speech output (reading back, spoken feedback voices) | AX01 (`speech.say`) |
| Recording and transcribing audio files and meetings | ME06 |
| Subtitles for video | ME03 |
| Conversational assistant ("ask a question, get an answer") | AI01 (Voice can send dictated text to it by intent) |
| Model install, verification, benchmarking | AI02 |
| Speech-to-text model class and streaming session | FORGE (FORGE-WP-31.3), `svc.model` |
| Microphone portal, indicator, hard mute | SH02, SH04, `svc.portal.mic` |
| Text commit path into fields | AX04 (panel connection) |
| Drawing capability prompts | SH04 |

## 3. Users and top tasks

Personas: **Noor**, repetitive strain injury, dictates and commands all day, can press one large button. **Eli**, tetraplegic, voice only, no physical input at all. **Asha**, low vision, dictates long text and edits by keyboard. **Leo**, developer, uses custom commands for code. **Kim**, journalist, needs proof that nothing is recorded or sent.

| # | Task | Target steps |
|---|---|---|
| 1 | Start dictating into the focused field | 1 (`Super+H`, or say "start dictation") |
| 2 | Dictate a sentence with punctuation | speak; 0 extra |
| 3 | Fix a wrong word | 2 utterances ("correct there", "two") |
| 4 | Undo the last thing dictated | 1 ("scratch that") |
| 5 | Click a button by its name | 1 ("click Save") |
| 6 | Click something with no name | 2 ("show numbers", "fourteen") |
| 7 | Click an exact point | 3–4 ("show grid", "five", "two", "click") |
| 8 | Switch to an app, or open one | 1 ("switch to Mail", "open Files") |
| 9 | Stop listening at once | 1 ("go to sleep", or `Super+Alt+H`, or the hardware mute) |
| 10 | See whether the microphone is live and where audio goes | 0 (indicator and HUD always show it) |
| 11 | Deny a capability prompt by voice | 1 ("deny") |
| 12 | Approve a capability prompt hands-free | voice plus a second factor (section 7.4) |
| 13 | Add a name the recogniser gets wrong | 3 (say "add word", spell or type, confirm) |
| 14 | Find out what can be said here | 1 ("what can I say") |

## 4. Features

### 4.1 Recognition engine

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX06-F001 | Local streaming recognition | Audio frames go to a streaming speech-to-text session in `svc.model`; partial results arrive at least every 300 ms, final results at utterance end. | M | 1 |
| AX06-F002 | Model choice | Lists installed speech models with language, size, speed class and signer; small model for commands, larger for dictation (setting). Install and verify in AI02. | M | 1 |
| AX06-F003 | Languages | One primary dictation language and up to three more; switch by command ("switch to German") or key. Each needs a model that covers it. | M | 1 |
| AX06-F004 | Audio front end | Helper process: capture at the device rate, resample to the model rate, high-pass, automatic gain, noise suppression, voice activity detection, mel features. | M | 1 |
| AX06-F005 | Voice activity detection | Energy plus a small neural detector; start and end padding 200 ms and 500 ms (settings); end-of-utterance silence 0.4–2.0 s. | M | 1 |
| AX06-F006 | Confidence | Each word and each command has a confidence; below a threshold dictation marks the word, commands ask "did you say…". | M | 1 |
| AX06-F007 | N-best alternatives | Up to 5 alternatives per phrase are kept for the correction list. | M | 2 |
| AX06-F008 | Resource budget | Uses `svc.model` queueing and memory budget; shows when the model is busy; never blocks typing. | M | 1 |
| AX06-F009 | CPU and GPU | Runs on CPU; uses GPU through `svc.model` when present (FORGE-WP-32). | S | 2 |
| AX06-F010 | Offline always | No network path exists; the bundle has `net.none`. | M | 1 |
| AX06-F011 | Long-form stability | Segments are committed in order; a later partial never rewrites text already committed. | M | 1 |
| AX06-F012 | Latency meter | Developer view: end of speech to text on screen, real-time factor. | C | 2 |

### 4.2 Dictation

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX06-F013 | Dictate into any field | Final text is committed into the focused field through AX04's panel connection. Works wherever typing works. | M | 1 |
| AX06-F014 | Provisional text | The partial hypothesis is shown as pre-edit in the field (underlined) when the field supports it, else in the HUD bubble near the caret. | M | 1 |
| AX06-F015 | Automatic punctuation | The model's punctuation and capitalisation, on or off. | M | 1 |
| AX06-F016 | Spoken punctuation | "comma", "full stop" / "period", "question mark", "exclamation mark", "colon", "semicolon", "open quote", "close quote", "open bracket", "dash", "ellipsis", "new line", "new paragraph", "tab"; per-language word lists. | M | 1 |
| AX06-F017 | Capitalisation commands | "cap <word>", "all caps <word>", "caps on/off", "all caps on/off", "no caps <word>", "cap that", "lowercase that". | M | 2 |
| AX06-F018 | Spacing commands | "no space", "no space on/off", "space bar", "hyphenate that", "compound that". | S | 2 |
| AX06-F019 | Number and unit formatting | Inverse text normalisation per locale: "twenty five dollars" → $25, "the third of March" → 3 March, "five kilometres" → 5 km, "half past two" → 2:30; formats from `forge::locale`; "numeral five" forces a digit, "spell five" forces the word. | M | 2 |
| AX06-F020 | Spelling mode | "spell that" or "spell mode on": letters by name or phonetic alphabet ("alpha bravo"), "capital A", digits, symbols by name. | M | 2 |
| AX06-F021 | Symbols and emoji | "at sign", "hash", "ampersand", "percent", "backslash"; "emoji thumbs up" inserts by AX04's emoji names. | S | 2 |
| AX06-F022 | Literal escape | "literal comma" types the word "comma". | M | 1 |
| AX06-F023 | Profanity filter | Off by default; masks with asterisks when on. | C | 3 |
| AX06-F024 | Auto-spacing and casing by context | Uses surrounding text from the field (when offered) to decide a leading space and capital. | M | 2 |
| AX06-F025 | Field awareness | Field purpose changes behaviour: e-mail and URL fields use no spaces and lower case; number fields accept digits only; terminal fields default to command mode with explicit "type …". | S | 2 |
| AX06-F026 | Never into secret fields | Dictation is refused in password, PIN and sensitive fields: saying a secret aloud discloses it. The HUD says why. | M | 1 |
| AX06-F027 | Dictation scratchpad | A small window to dictate into when no field has focus; then "copy that" or "send to <app>" (intent). | S | 2 |
| AX06-F028 | Continuous and hold modes | Toggle on and off, or hold `Super+H` to talk (push to talk). | M | 1 |
| AX06-F029 | Auto-stop | Dictation stops after N seconds of silence (default 30) or on focus change to a secret field. | M | 1 |
| AX06-F030 | Mixed mode | In dictation, a phrase spoken alone after a pause that exactly matches an editing command is run as a command; otherwise it is text. The match list is short and shown in help. | M | 2 |
| AX06-F031 | Translation dictation | Not planned; listed so it is not assumed. | C | 3 |

### 4.3 Correction and text editing

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX06-F032 | Scratch that | Removes the last utterance (uses delete-surrounding on the committed length; exact because the app tracks what it committed). Repeatable. | M | 1 |
| AX06-F033 | Undo and redo | "undo that", "redo that" send the app's Undo and Redo actions. | M | 1 |
| AX06-F034 | Correct word | "correct <word>" selects the nearest match before the caret and shows numbered alternatives; "choose two" replaces. | M | 2 |
| AX06-F035 | Correct that | Alternatives for the whole last utterance. | M | 2 |
| AX06-F036 | Spell correction | In the correction list, "spell that" switches to spelling mode to type the right word; the pair is offered to the vocabulary. | S | 2 |
| AX06-F037 | Select | "select <word or phrase>", "select previous / next word / sentence / paragraph / line", "select all", "select from <x> to <y>", "unselect". Needs text ranges from `svc.a11y` (AX01-GAP-1) or surrounding text; else key-based fallback. | M | 2 |
| AX06-F038 | Move | "go to start / end of line / sentence / paragraph / document", "move left / right N words", "insert before / after <word>". | M | 2 |
| AX06-F039 | Delete | "delete that" (selection or last utterance), "delete previous word", "delete line", "backspace N". | M | 2 |
| AX06-F040 | Replace | "replace <x> with <y>". | S | 2 |
| AX06-F041 | Format | "bold that", "italic that", "underline that", "make that a heading": invokes the focused app's actions by id through the action list in the a11y tree (FUSE-WP-14.6). | S | 3 |
| AX06-F042 | Clipboard | "copy that", "cut that", "paste that", "paste number three", "show clipboard" through the app's actions and AX05's actions. | M | 2 |
| AX06-F043 | Read back | "read that", "read line", "read paragraph" through AX01's `speech.say`; recognition pauses while speech plays. | S | 2 |
| AX06-F044 | Learn from corrections | A correction updates the personal vocabulary and replacement rules after the user confirms once per new word. | S | 2 |

### 4.4 Voice control: windows, apps, system

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX06-F045 | Open and switch | "open <app>", "switch to <app or window title>", "next window", "previous window", "list windows" (numbered). App names from the shell's launcher index. | M | 2 |
| AX06-F046 | Window commands | "close window", "minimise", "maximise", "restore", "full screen", "snap left / right / top", "move to workspace N", "move to next monitor". Through SH05's actions. | M | 2 |
| AX06-F047 | Shell commands | "show launcher", "search for <text>", "show notifications", "show desktop", "show overview", "lock screen". | M | 2 |
| AX06-F048 | Keys | "press <key>" with modifiers ("press control shift T"), "press escape", "hold shift", "release shift", "press tab three times". | M | 2 |
| AX06-F049 | Scrolling | "scroll down / up / left / right", "page down", "scroll to top / bottom", "keep scrolling down", "stop". Uses the element's scroll action, else wheel events. | M | 2 |
| AX06-F050 | Media and system | "volume up / down", "mute sound", "play", "pause", "next track" through `svc.media`; "brightness up". | S | 3 |
| AX06-F051 | Tools | "zoom in / out" (AX02 actions), "show keyboard" (AX03), "screen reader on" (SY01 switch; asks for confirmation). | S | 3 |
| AX06-F052 | Confirmed commands | Commands in the destructive set ("close all windows", "shut down", "restart", "delete", "send", "empty trash") ask "Say confirm to …" within 5 s. The set is editable; shutdown and restart cannot be removed from it. | M | 2 |
| AX06-F053 | What can I say | Shows commands valid now, grouped: global, this app, this control; searchable by voice ("show commands about windows"). | M | 2 |
| AX06-F054 | Repeat | "do that again", "repeat N times". Not allowed for confirmed commands. | S | 3 |

### 4.5 Voice control: clicking and pointing

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX06-F055 | Click by name | "click <name>": matches the names of actionable elements in the active window from the a11y tree; one match invokes the element's action; several matches show numbers on those only. | M | 2 |
| AX06-F056 | Name matching | Exact, then prefix, then fuzzy (`forge::text::jaro_winkler`), accent and case insensitive; the visible names bias recognition when the model supports phrase boosting. | M | 2 |
| AX06-F057 | Role words | "click Save button", "check Remember me", "expand Advanced", "choose Large" (in a combo), "tab Network", "menu File". | M | 2 |
| AX06-F058 | Number overlay | "show numbers": every actionable element in the active window gets a numbered tag (overlay). "fourteen" clicks; "right click fourteen", "double click fourteen", "focus fourteen", "hide numbers". | M | 2 |
| AX06-F059 | Numbers always | Option to keep numbers on in chosen apps. | S | 3 |
| AX06-F060 | Number scope | "show numbers everywhere" covers panels and other windows; default is the active window plus open menus. | S | 2 |
| AX06-F061 | Grid overlay | "show grid": a 3×3 numbered grid over the screen or the window; a number zooms the grid into that cell (up to 4 levels); "click", "right click", "double click", "drag", "drop". For apps with no tree. | M | 2 |
| AX06-F062 | Grid variants | 3×3 recursive, fixed fine grid with letter-number cells ("C seven"), window-only grid. | S | 3 |
| AX06-F063 | Pointer movement | "mouse up / down / left / right N", "mouse to centre", "mouse to <name>", "keep moving left", "faster", "slower", "stop". | S | 3 |
| AX06-F064 | Drag by voice | "drag <a> to <b>" with names or numbers; "drag", then grid, then "drop". | S | 3 |
| AX06-F065 | Hover and menus | "hover <name>", "open context menu", "menu <item>". | S | 2 |
| AX06-F066 | Text links and lists | "click link <text>", "select item <text>", "go to row N". | S | 3 |
| AX06-F067 | Overlay style | Tag size, colours, opacity; tags avoid covering the element's text where space allows; high-contrast pair default. | M | 2 |
| AX06-F068 | Stale guard | A number or grid click is cancelled if the element under the tag changed or moved in the last 500 ms; the overlay refreshes and says so. | M | 2 |

### 4.6 Custom commands and vocabulary

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX06-F069 | Personal vocabulary | Words and phrases with optional "sounds like" spelling and written form ("jay son" → JSON); per language; visible and erasable. | M | 2 |
| AX06-F070 | Phrase boosting | The vocabulary, contact names (opt-in from CO03), and visible control names are passed as boost phrases if the model class supports it; else applied as post-recognition replacements. | S | 2 |
| AX06-F071 | Replacement rules | Spoken form → written form after recognition: plain or capped regex; per app. | M | 2 |
| AX06-F072 | Text macros | A custom command that types fixed text or an AX04 snippet ("insert signature"). | M | 2 |
| AX06-F073 | Key and action macros | A custom command that sends keys or runs named app actions in order with optional waits. | M | 2 |
| AX06-F074 | ForgeScript commands | Commands as ForgeScript functions bound to phrase patterns with slots: `"open ticket <number:int>"`. API: `voice::type`, `voice::press`, `voice::invoke(action)`, `voice::say`, `tree::find`, `tree::focus`, `intent::emit`, `voice::confirm`. | S | 3 |
| AX06-F075 | Script sandbox | Command scripts run with no `fs`, `net`, `proc` or `env`; they act only through the API, which is subject to the same rules as built-in commands (no secure surfaces, confirmations). | M | 3 |
| AX06-F076 | Script signing and enable | System command sets are signed with the bundle; user scripts are listed, viewable and enabled one by one (PLATFORM V14). | M | 3 |
| AX06-F077 | Per-app command sets | Sets bound to a bundle id are active only while that app has focus; built-in sets for OF01, SY03, IN01, CO01, SY02. | S | 3 |
| AX06-F078 | Command editor | Create, test ("say it now": shows what was recognised and which command matched), enable, disable. | M | 2 |
| AX06-F079 | Conflict check | Warns when two active commands share a phrase or when a command shadows common dictation words. | M | 2 |
| AX06-F080 | Import and export | Vocabulary, rules and commands as JSON through the file portal; imported scripts start disabled. | S | 3 |
| AX06-F081 | Rename built-ins | Alternative phrases for built-in commands; built-in phrases can be disabled individually. | S | 2 |

### 4.7 Modes, wake word, microphone

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX06-F082 | Modes | Off (no audio capture at all), Sleeping (only the wake phrase is acted on), Command, Dictation, Mixed. The HUD always shows the mode in words. | M | 1 |
| AX06-F083 | Mode switching | "start dictation", "stop dictation", "command mode", "go to sleep", "wake up", `Super+H` (dictation), `Super+Alt+H` (voice control on, off). | M | 1 |
| AX06-F084 | Wake word | Optional. A small keyword-spotting model runs in the audio helper; default phrase "wake up" plus a chosen word; sensitivity setting. Off by default. | S | 3 |
| AX06-F085 | Wake word is local | While sleeping, audio is held in a 2 s ring in locked memory and discarded; nothing is sent to the full recogniser until the wake phrase fires. | M | 3 |
| AX06-F086 | Wake word honesty | While sleeping with the wake word on, the microphone is in use and the system microphone indicator stays on. The HUD says "listening for the wake word only". There is no hidden listening state. | M | 3 |
| AX06-F087 | Microphone portal | Capture only through `svc.portal.mic` after the `dev.mic` prompt; device choice follows SY01 Sound or an override here. | M | 1 |
| AX06-F088 | Hard mute | If the hardware kill switch or SH02's system microphone mute is on, Voice shows "Microphone is off" and stops. No voice or script command can turn the microphone on. | M | 1 |
| AX06-F089 | Push to talk | A key, switch or gamepad button held to listen; nothing is captured otherwise. | M | 1 |
| AX06-F090 | Mute on lock | Capture stops when the session locks and does not resume by itself unless "resume after unlock" is on. | M | 1 |
| AX06-F091 | Pause during calls | When another app holds the microphone for a call (CO05), Voice goes to sleep unless set to keep command mode. | S | 2 |
| AX06-F092 | Level meter and clipping warning | Live input level in the HUD and settings; warns on clipping, too quiet, or noisy input. | M | 1 |
| AX06-F093 | Microphone setup guide | Pick device, read two sentences, set gain and noise floor, test recognition. | M | 2 |

### 4.8 Noise, echo, robustness

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX06-F094 | Noise suppression | Spectral suppression in the helper (own code on `forge::signal` FFT); strength low, medium, high; optional neural suppressor through `svc.model`. | M | 2 |
| AX06-F095 | Playback rejection | The helper gets the system playback mix as a reference from `svc.media` and cancels it from the input. A command is dropped if the recognised audio correlates with playback above a threshold. Stops audio played by an app, a video or a call from issuing commands. | M | 2 |
| AX06-F096 | Self-speech rejection | Recognition pauses while AX01 or `speech.say` is speaking, unless headphones are in use. | M | 2 |
| AX06-F097 | Non-speech sounds | Coughs, laughs, keyboard noise are discarded by the detector, not typed. | S | 2 |
| AX06-F098 | Far-field warning | Detects low direct-to-reverberant ratio and suggests a closer microphone. | C | 3 |
| AX06-F099 | Sound commands | Optional non-speech triggers (a pop or a click sound) mapped to actions for users with limited speech. | C | 3 |
| AX06-F100 | Atypical speech | Thresholds, longer pauses, and adaptation (4.9) aimed at dysarthric and accented speech; never auto-stops on slow speech when this profile is on. | S | 3 |

### 4.9 Adaptation and training

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX06-F101 | Vocabulary adaptation | Learns words and replacements from confirmed corrections (F044). First and main adaptation path. | M | 2 |
| AX06-F102 | Threshold tuning | From the setup guide and from use: detector thresholds, pause length, confidence cut-offs per user. | M | 2 |
| AX06-F103 | Guided reading | The user reads prompts; recordings are kept only with explicit consent, listed with dates and sizes, playable and erasable. | S | 3 |
| AX06-F104 | Speaker adaptation | If the model class supports a small per-user adapter, train it locally from guided-reading recordings through `svc.model`; stored as a signed-by-user local file; can be turned off and deleted. Depends on FORGE; otherwise not offered. | C | 3 |
| AX06-F105 | Command practice | A practice page: say each command, see the match and the confidence. | S | 2 |
| AX06-F106 | Accuracy report | Local counts of corrections per 100 words over time; never sent anywhere. | C | 3 |
| AX06-F107 | Reset adaptation | Delete all adaptation data in one step. | M | 2 |

### 4.10 Security prompts and secure surfaces

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX06-F108 | Prompt mode | When SH04 shows a capability prompt, Voice enters prompt mode: all other commands and dictation stop; only "deny", "cancel", "read it again", "what are my choices" and, if configured, the approval phrase are accepted. | M | 2 |
| AX06-F109 | Deny by voice | "deny" and "cancel" are passed to SH04's prompt interface as a denial. Denial is always safe, so voice alone is enough. | M | 2 |
| AX06-F110 | No approval by voice alone | Voice cannot select Allow. Its synthetic input and a11y actions are refused on secure surfaces by the compositor and `svc.a11y`. | M | 1 |
| AX06-F111 | Second factor: physical confirm | Default hands-free path: say "allow once" (or the choice), the prompt arms that choice for 10 s, and the user completes it with one physical act: any key, a switch press, a touch, or the hardware button of a security key. The act goes through the trusted input path, not through Voice. | M | 2 |
| AX06-F112 | Second factor: spoken challenge | Opt-in, for users with no physical input at all. The prompt shows a random three-word challenge on the secure surface only (and on a braille display or headphones through AX01, never on loudspeakers). The user says the choice and the challenge. Limited to capability classes the user ticked; never offered for `dev.mic`, `dev.screen`, `dev.camera`, `net.*`, `fs.path`, `proc.exec`, authentication or administrator prompts. Requires playback rejection (F095) to be active. | S | 3 |
| AX06-F113 | No voice biometrics | Speaker verification is not used as a factor: voices can be recorded and synthesised, and it would need stored biometric data. | M | 1 |
| AX06-F114 | Authentication prompts | Passwords and passphrases are never accepted by voice (F026). Voice can cancel the prompt. | M | 1 |
| AX06-F115 | Lock screen and greeter | Voice does not run on the greeter. On the lock screen it is stopped (F090). | M | 1 |
| AX06-F116 | Prompt announcement | The reading of the prompt is AX01's job; Voice only tells the HUD "Security prompt: say deny, or allow then confirm". | M | 2 |
| AX06-F117 | Audit | Every prompt-mode outcome (denied by voice, armed by voice and confirmed by which factor, timed out) is written to `svc.audit`. | M | 2 |
| AX06-F118 | Permissions app guard | In AD05 and in SE04, commands that grant, reveal or delete need the confirmed-command step (F052) even though those are normal windows. | M | 2 |

### 4.11 Privacy, HUD, settings, recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX06-F119 | No audio storage | Audio lives in locked memory for at most the current utterance plus 2 s, then is zeroed. No file is written unless the user records training prompts (F103). | M | 1 |
| AX06-F120 | No transcript storage | Recognised text is kept in a memory ring of the last 50 utterances for corrections; cleared on sleep, lock and exit; not written to disk. | M | 1 |
| AX06-F121 | HUD | A small always-visible bar while Voice is on: mode, level meter, last recognised phrase, the matched command, and a mute button. | M | 1 |
| AX06-F122 | Indicators | The system microphone indicator (SH02) is on whenever audio is captured. The Voice indicator adds mode and a menu. | M | 1 |
| AX06-F123 | Stop words always work | "go to sleep" and "stop listening" are recognised in every mode and by the small model, so they work when the large model is busy. | M | 1 |
| AX06-F124 | Feedback sounds | Short cues for listening started, stopped, command accepted, not understood; through `svc.media`; excluded from recognition. | S | 2 |
| AX06-F125 | Settings window | All settings, vocabulary, commands, training and privacy in one window (section 5.2). | M | 1 |
| AX06-F126 | Profiles | Named sets (quiet room, noisy room, headset) and per-user adaptation data. | S | 3 |
| AX06-F127 | Crash recovery | If the helper or the model session dies, capture stops, the HUD says so, and Voice returns to Sleeping after restart; it never restarts into a listening mode without the user. | M | 1 |
| AX06-F128 | Start at login | Optional `bg.autostart` into Off or Sleeping; never into Command or Dictation. | M | 2 |
| AX06-F129 | No telemetry | No audio, text, command or accuracy data leaves the machine. | M | 1 |
| AX06-F130 | Privacy page | Shows what is in memory now (seconds of audio, utterances), what is on disk (vocabulary, training recordings), with delete buttons, and a link to the audit entries. | M | 2 |

## 5. GUI

### 5.1 Window map

```
Voice (background process; indicator in SH02 beside the system microphone indicator)
├─ HUD bar (layer surface, never focusable)
│   ├─ provisional-text bubble (near the caret)
│   └─ "did you say" strip · confirm strip · prompt-mode strip
├─ Overlays through svc.a11y:overlay: number tags · grid · pointer target ring
├─ Correction list (popup at the caret)
├─ What can I say (panel window)
├─ Dictation scratchpad (window)
├─ Settings window (main window)
│   ├─ General · Dictation · Commands · Vocabulary · Microphone · Wake word
│   ├─ Security prompts · Training · Privacy · Shortcuts
│   └─ dialogs: Add word · Command editor · Script viewer · Import/Export · Reset adaptation
├─ Microphone setup guide (wizard)
├─ Guided reading (wizard)
└─ Command practice (window)
```

### 5.2 Main window (Settings)

Shown at 960×640. Sidebar + content; page "Commands".

```
+------------------------------------------------------------------------------------------+
| [=] Voice — Commands                       [search Ctrl+K]           [🎤] [▁] [▢] [✕]   | (1)
+-------------------+----------------------------------------------------------------------+
|   General         | Voice control [ON ]   Mode now: Command      [Go to sleep]           | (3)
|   Dictation       +----------------------------------------------------------------------+
| > Commands        | Set [All sets        v]  [🔍 filter_________]  [x] Show built-in     | (4)
|   Vocabulary      |  Phrase                      Does                    Set       On    |
|   Microphone      | > click <name>               invoke element          Global    [x]   |
|   Wake word       |   show numbers               number overlay          Global    [x]   |
|   Security prompts|   scratch that               remove last utterance   Dictation [x]   |
|   Training        |   close all windows  ⚠       close every window      Global    [x]   |
|   Privacy         |   insert signature           type snippet “;sig”     Mine      [x]   |
|   Shortcuts       |   open ticket <number>       script: tickets.fs      Mine      [ ]   |
|                   |  [New command…] [Edit…] [Duplicate] [Delete] [Practice…]             |
| (2)               +----------------------------------------------------------------------+
|                   | click <name>                                                     (5) |
|                   |  Also say: “press <name>”, “tap <name>”            [Add phrase…]     |
|                   |  Needs confirmation: no        Works in: all apps                    |
|                   |  Try it: [🎤 Say it now]   heard: “click save” → click <name=Save> ✓ |
+-------------------+----------------------------------------------------------------------+
| Model: English small (commands), English large (dictation) · mic: USB headset · level ▮▮▯| (6)
+------------------------------------------------------------------------------------------+
```

1. Header bar. The microphone badge `[🎤]` shows while this app captures audio (DESIGN_SYSTEM section 3). No `[net]` badge can appear.
2. Sidebar, 220 px, `listView`.
3. Master `toggleSwitch`, mode `label`, sleep `button`.
4. Commands `tableView`: phrase pattern, effect, set, enabled `checkBox`. ⚠ with the word "confirmed" in the tooltip marks commands that ask for confirmation.
5. Detail `panel` for the selected command with the live test (`button` + result `label`).
6. Status bar with models, device and a level meter (`progressBar`).

States: empty (no speech model installed: "Voice needs a speech model. Open Model Manager to install one." with a button; everything else disabled); loading (model load: status bar `progressBar`, "Loading model… 3 s"); error (microphone denied: banner with `[Open Permissions…]`; helper crashed: banner with `[Restart]`); offline (no effect); locked (window hidden, capture stopped); read-only (built-in commands cannot be deleted, only disabled or given extra phrases); hard mute (banner "The microphone is switched off for the whole system"); narrow (sidebar becomes a `comboBox`; the detail panel becomes a sheet).

### 5.3 Other views and dialogs

**HUD bar** (420×44, bottom centre by default; draggable; never takes focus).

```
+----------------------------------------------------------------------+
| 🎤 DICTATION  ▮▮▮▮▯▯   “…and send it to the team comma”   [⏸] [⋯]   | (1)
+----------------------------------------------------------------------+
| 🎤 COMMAND    ▮▮▯▯▯▯   heard “click safe” → Did you say: 1 click Save  2 click Safari | (2)
+----------------------------------------------------------------------+
| 💤 SLEEPING   wake word on: mic in use, listening for “wake up” only  [Off]          | (3)
+----------------------------------------------------------------------+
| ⚠ SECURITY PROMPT   say “deny”, or “allow once” then press any key   [Deny]          | (4)
+----------------------------------------------------------------------+
```

Four states of the same bar. 1. Mode word, level meter, last phrase, pause and menu `toolButton`s. 2. Low-confidence strip with numbered choices. 3. Sleeping with the wake word: the text is fixed and cannot be hidden. 4. Prompt mode: border in `--warn`; the `[Deny]` button is clickable too. Widgets: `panel`, `label`, `progressBar`, `toolButton`.

**Number overlay** (over the active window; example on a 900 px wide window).

```
+------------------------------------------------------------------------------------------+
| [=]① Mail — Inbox                 [search]②                       [net] [▁]③ [▢]④ [✕]⑤ |
+-----------+------------------------------------------------------------------------------+
| Inbox ⑥   | [Compose]⑦ [Reply]⑧ [Forward]⑨ [Archive]⑩ [Delete]⑪                        |
| Sent ⑫    +------------------------------------------------------------------------------+
| Drafts ⑬  | ⑭ Dana     Invoice for October            14:02                              |
| Trash ⑮   | ⑯ Leo      Re: build failure              13:40                              |
+-----------+------------------------------------------------------------------------------+
   say a number · “right click 14” · “hide numbers”
```

Tags are drawn on an input-transparent overlay surface at each element's bounds corner. Tags use a dark box with light digits and a light outline; size follows `overlay.tag_pt`.

**Grid overlay** (full screen, first level).

```
+-----------------------------+-----------------------------+-----------------------------+
|                             |                             |                             |
|              1              |              2              |              3              |
|                             |                             |                             |
+-----------------------------+-----------------------------+-----------------------------+
|                             |   +-----+-----+-----+       |                             |
|              4              |   |  1  |  2  |  3  |  (1)  |              6              |
|                             |   +-----+-----+-----+       |                             |
+-----------------------------+---|  4  |  5  |  6  |-------+-----------------------------+
|              7              |   +-----+-----+-----+       |              9              |
|                             |   |  7  |  8  |  9  |       |                             |
+-----------------------------+-----------------------------+-----------------------------+
   “five” zoomed into the centre cell (1) · say a number, “click”, “back”, or “hide grid”
```

Lines and digits on an overlay; after two levels a ring marks the exact target point.

**Correction list** (popup at the caret).

```
+--------------------------------------+
| Correct “there”                      |
| 1  their                             |
| 2  they're                           |
| 3  there                             |
| 4  Spell it…                         |
+--------------------------------------+
| say “choose two” · “cancel”          |
+--------------------------------------+
```

`popupMenu`-style `listView`; also usable by keyboard and pointer.

**What can I say** (panel window, 380×560, docked right by default).

```
+--------------------------------------+
| What can I say            [🔍] [✕]   |
+--------------------------------------+
| Always                               |
|   go to sleep · stop listening       |
|   what can I say · show numbers      |
| Here: Mail                           |
|   compose · reply · next message     |
| This control: message list           |
|   select item <text> · go to row N   |
| Dictation                            |
|   scratch that · correct <word>      |
|   new line · cap <word> …            |
+--------------------------------------+
```

`treeView` with groups, filtered by context; `lineEdit` search.

**Security prompts page** (settings content).

```
| When a security prompt appears                                                            |
|  Deny or cancel by voice                         always on                                |
|  Approve                                                                                  |
|   (•) Voice chooses, a physical act confirms (key, switch, touch, security key)       (1)|
|   ( ) Spoken challenge  — only if you cannot use any physical input                   (2)|
|        Allowed for:  [x] files you choose   [x] notifications   [ ] printing              |
|        Never for: microphone, camera, screen, network, fixed paths, running programs,     |
|                   passwords, administrator actions                                        |
|        Challenge shown on: (•) screen only ( ) screen + braille ( ) screen + headphones   |
|   ( ) Never by voice                                                                      |
|  Why voice alone cannot approve  [Read more]                                              |
```

1. Default. 2. Choosing it opens a confirmation that must itself be completed with a physical act or by an administrator.

**Microphone setup guide** (wizard, 4 steps): pick device (`comboBox`, live meter) → read two sentences (meter, clipping and noise read-outs) → result (gain, noise floor, suggested noise profile) → test dictation in a `lineEdit`.

**Privacy page**.

```
| In memory now      audio 0.0 s · utterances 12                 [Clear now]                |
| On this disk       vocabulary 84 entries · replacement rules 9 [Open] [Delete all]        |
|                    training recordings 0 (0 MB)                [Open] [Delete all]        |
|                    speaker adapter: none                                                  |
| Never stored       audio of dictation and commands, recognised text                       |
| Network            none: this app cannot open connections                                 |
| Audit              18 voice entries this week                  [Show in Logs…]            |
```

### 5.4 Menus

```
Settings window [=] (F10)
├─ File ▸ Import… · Export… · Close Ctrl+W · Quit Voice Ctrl+Q
├─ Edit ▸ Undo Ctrl+Z · Redo Ctrl+Shift+Z · New command Ctrl+N · Delete Delete · Find Ctrl+F
├─ Voice ▸ Voice control on/off Super+Alt+H · Dictation Super+H · Go to sleep
│         · Language ▸ <languages> · Microphone ▸ <devices>
├─ View ▸ HUD · What can I say · Scratchpad · Latency meter
├─ Tools ▸ Microphone setup… · Guided reading… · Command practice… · Reset adaptation…
└─ Help ▸ Voice help F1 · Command reference · Privacy · About

HUD menu [⋯] and indicator menu
├─ Mode ▸ Command · Dictation · Mixed · Sleeping · Off
├─ Language ▸ …
├─ Show numbers · Show grid · What can I say
├─ Mute microphone for Voice
└─ Settings…
```

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| `Super+H` | Dictation on, off; hold for push to talk |
| `Super+Alt+H` | Voice control on, off (Off means no capture) |
| `Super+Alt+N` | Number overlay |
| `Super+Alt+G` is taken by AX02; grid is `Super+Alt+Y` | Grid overlay |
| `Super+Alt+W` | What can I say |
| `Esc` | Hide overlays, cancel correction, cancel an armed confirmation |

Global chords are registered with the shell and arrive as actions; Voice never receives other keys. Every voice command has a keyboard or pointer equivalent somewhere in the system; Voice adds no function that exists only by voice.

Pointer: the HUD is draggable by its left edge; click the mode word to cycle modes; click a "did you say" choice. Context menu on a command row: Edit, Duplicate, Disable, Practice, Delete. Drag and drop: a commands JSON file dropped on the settings window opens the import preview (scripts arrive disabled).

### 5.6 States

| State | Behaviour |
|---|---|
| Off | No capture, no model loaded, no indicator. |
| Sleeping (no wake word) | No capture. Woken by key or button only. |
| Sleeping (wake word on) | Capture into the 2 s ring; microphone indicator on; small model only. |
| Command / Dictation / Mixed | Capture on; HUD shows mode. |
| Model loading or busy | HUD "loading"; stop words still work through the small model. |
| Low confidence | "Did you say" strip; nothing is done until chosen or timed out (4 s). |
| Awaiting confirmation | Confirm strip with countdown; only "confirm" and "cancel" accepted. |
| Prompt mode | F108. |
| Secret field focused | Dictation refused; commands still work. |
| Hard mute or microphone revoked | Stopped; banner; no retry until the user acts. |
| Call in progress | Sleeping (F091). |
| Session locked | Stopped (F090). |
| Helper crashed | Stopped, then Sleeping after restart (F127). |

### 5.7 Widgets

| Region | Widget | Note |
|---|---|---|
| Settings lists | `listView`, `tableView`, `treeView`, `propertyGrid` | in tree |
| Controls | `toggleSwitch`, `radioButton`, `checkBox`, `comboBox`, `slider`, `spinBox`, `lineEdit` | in tree |
| Level meter | `progressBar` (fast updates), or the meter of FUSE-WP-49 when it exists | |
| Script viewer | `codeEdit` read-only | in tree |
| HUD | `panel`, `label`, `toolButton` on a layer surface | FUSE-WP-10 |
| Number tags, grid | custom paint on overlay surfaces | `svc.a11y:overlay` |
| Correction list | `listView` in a popup | in tree |
| Wizards | `dialog` with pages | in tree |
| Help | `markdownView` | in tree |

Gaps:

| ID | Gap | Evidence | Lands in |
|---|---|---|---|
| AX06-GAP-1 | Speech-to-text model class with a streaming session and timestamps; FORGE has no template for it (a mel filterbank and STFT exist) | `FORGE:docs/support.md` (`signal/fft.hpp`: FFT, STFT power, Slaney mel filterbank; `signal/audio.hpp`: resample, biquad); FORGE_BUILD_PLAN FORGE-WP-31.3 | FORGE-WP-31.3, G-15 |
| AX06-GAP-2 | Keyword-spotting model class for the wake word | none read | FORGE-WP-31 (new sub-item) |
| AX06-GAP-3 | Playback reference stream from `svc.media` for echo cancellation and playback rejection | none read | `svc.media` (G-09, G-11) |
| AX06-GAP-4 | A voice class of synthetic input in the compositor: accepted without a hardware serial only while a voice session is active, never on secure surfaces | no virtual input module in `PBSD:pbsd/compositor/` | compositor, FUSE-WP-56 |
| AX06-GAP-5 | Prompt interface in SH04 for assistive tools: `deny`, `arm(choice)`, challenge display, outcome events | none read | SH04 |
| AX06-GAP-6 | Pre-edit from a panel (dictation) through AX04's broker | AX04 panel connection is commit-only as specified | AX04 |
| AX06-GAP-7 | Text ranges, selection and named actions in the a11y tree | Verified actions today: Focus, Click, Increment, Decrement, SetValue (`FUSE:Source/FUSE/UI/A11y/include/fuse/ui/a11y/a11y.hpp:19`) | FUSE-WP-55, FUSE-WP-14.6 |

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Voice core | ForgeScript | app host (`bg.run`) | Modes, command grammar and matching, dictation post-processing, correction state, prompt mode, overlays logic |
| Voice UI | FUSE UI | app host | HUD, overlays, settings, wizards |
| Audio front helper | C++23 | `audio-front-helper`, capability mode; holds the microphone stream handle and the playback reference handle | Capture, resample, gain, noise suppression, echo cancel, detector, wake-word spotter, mel features; hands frames to `svc.model` as a shared-memory stream. Native for speed and because it holds raw audio (PLATFORM section 2 (b), (c)) |
| Speech session | FORGE (platform) | `svc.model` | Streaming recognition; returns text, word times, confidences, alternatives |
| Command scripts | ForgeScript | child engine per script in the app host, `Caps` none | Custom commands (F074) |
| Text post-processor | ForgeScript | app host | Spoken punctuation, inverse text normalisation, replacements, casing |

```
 microphone ─▶ svc.portal.mic ─▶ audio-front-helper ──features──▶ svc.model (speech session)
 svc.media playback reference ──▶      │  (echo cancel, playback veto, wake word)   │
                                        └──────────── level, VAD events ─────────────┤
                                                                                     ▼
                                      Voice core (ForgeScript)  ◀── partial / final text, n-best
         ┌───────────────┬──────────────────┬───────────────────────┬─────────────────────────┐
         ▼               ▼                  ▼                       ▼                         ▼
  AX04 panel commit   svc.a11y        svc.a11y:input.synth     shell / SH05 / AX05      SH04 prompt
  (dictation text)    tree, act       (voice class: keys,      actions (open, switch,   interface
                      (click by name) pointer; grid clicks)    snap, paste)             (deny / arm)
```

Audio never enters the script heap. The helper passes features to the model service through a shared-memory handle; the core sees only text.

### 6.2 Data model

Stores: `svc.settings` namespace `org.pbsd.voice`; `~/.local/share/org.pbsd.voice/voice.db` (SQLite, `schema_version`); `commands/*.fs` user scripts; `training/` recordings (only with consent).

| Entity | Fields |
|---|---|
| `vocab_entry` | `id: i64`, `lang: string`, `written: string`, `sounds_like: string?`, `boost: f32`, `source: enum{user, correction, contact}`, `created: i64`, `uses: u32` |
| `replacement` | `id: i64`, `lang: string`, `pattern: string`, `is_regex: bool`, `replacement: string`, `apps: string[]`, `enabled: bool` |
| `command` | `id: uuid`, `set_id: uuid`, `pattern: string` (words and slots: `<name>`, `<number:int>`, `<text:rest>`, `<key>`), `aliases: string[]`, `effect: Effect`, `confirm: bool`, `modes: enum[]`, `apps: string[]`, `enabled: bool`, `builtin: bool` |
| `Effect` | `kind: enum{builtin, type_text, snippet, keys, actions, script}`, `text: string?`, `keys: string[]?`, `actions: string[]?`, `script_id: string?`, `function: string?` |
| `command_set` | `id: uuid`, `name: string`, `bundle_id: string?`, `builtin: bool`, `enabled: bool` |
| `script` | `id: string`, `path: string`, `blake3: bytes32`, `signed_by: string`, `enabled: bool`, `last_error: string?` |
| `user_profile` | `id: uuid`, `name: string`, `vad: {start_db: f32, end_silence_ms: u16, pad_ms: u16}`, `noise_level: enum`, `confidence_cmd: f32`, `confidence_dictation: f32`, `mic_device: string`, `gain_db: f32` |
| `training_clip` | `id: i64`, `prompt: string`, `path: string`, `seconds: f32`, `recorded: i64`, `lang: string` |
| `adapter` | `model_id: string`, `path: string`, `blake3: bytes32`, `trained: i64`, `clips: u32` |
| `utterance` (memory only) | `seq: u64`, `mode: enum`, `text: string`, `alternatives: string[]`, `confidence: f32`, `committed_len: u32`, `target_window: bytes16` |
| `recognition_result` (wire, from `svc.model`) | `session: u64`, `final: bool`, `text: string`, `words: {text: string, t0_ms: u32, t1_ms: u32, conf: f32}[]`, `nbest: string[]`, `lang: string` |

Migration is forward only with a copy kept. Limits: command patterns at most 12 tokens; 2 000 active commands; vocabulary 20 000 entries; training clips at most 20 s each.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| PCM from the microphone portal (16 or 24 bit, any rate) | yes | no | own code; `forge::signal` resample and biquad (Verified, `FORGE:docs/support.md`) | audio front helper |
| Mel features to `svc.model` | — | yes | `forge::signal` FFT, STFT power, mel filterbank (Verified, same file) | audio front helper |
| `svc.model` speech session interface | yes | yes | schema compiler output (G-08); FORGE-WP-29, FORGE-WP-30 | app host; model service |
| Speech and keyword models `.fpk` | by `svc.model` | no | FORGE | model service |
| Commands, vocabulary, rules export (JSON) | yes | yes | `forge::json` + validator | app host (ForgeScript) |
| Command scripts (`.fs`) | yes | no | `forge::script::engine`, `Caps` none | child engine |
| Training clips (WAV, PCM 16 kHz mono) | yes | yes | own WAV writer and reader (own files only) | audio front helper |
| `svc.a11y` tree, action and synth records | yes | yes | schema compiler output | app host |
| SH04 prompt interface | yes | yes | schema compiler output | app host; SH04 validates |
| AX04 panel connection | — | yes | schema compiler output | AX04 broker validates |

### 6.4 Integration

- Intents: handles none; emits `open` (apps by name), `search`, `compose` (send scratchpad text), `view` (help).
- Interface offered: `voice.state()`, `voice.set_mode(mode)` (only towards less listening: other apps can put Voice to sleep, never wake it), `voice.commands(bundle_id)` for app-provided command sets shipped in an app's bundle (data only: phrases mapped to the app's own action ids).
- Services used: `svc.model`, `svc.portal.mic`, `svc.media` (reference stream, cues), `svc.a11y`, `svc.settings`, `svc.notify`, `svc.audit`, `svc.intent`, `svc.portal.file`, AX04's `input-method.panel`, AX01's `speech.say`.
- Other apps: AI02 (models), AX04 (commit path, emoji names, snippets), AX05 (clipboard actions), AX02 (zoom actions; tags and grid are tracked), AX03 (show keyboard; the trusted instance provides the physical confirm for switch users), AX01 (reads prompts; speaks feedback; command table shared), SH01 (app index, search), SH05 (window actions), SH02 (microphone indicator, hard mute), SH04 (prompt interface), CO03 (optional contact names), CO05 (call state), AD05 (rights), AD04 (audit view), ME06 (file transcription is there, not here).
- Verified in repos: FORGE has FFT, STFT power, a Slaney mel filterbank, windowed-sinc resampling and biquad filters (`FORGE:docs/support.md`); its plan lists Whisper-style speech as a planned template, not a present one (`FORGE:FORGE_PLAN.md:577`), and FORGE_BUILD_PLAN FORGE-WP-31.3 records "no template; streaming session and mel filterbank exist". ForgeScript has no model bindings today (PLATFORM V17). FORGE has fuzzy matching (`jaro_winkler`, `fuzzy_filter`), word segmentation and locale number and date formatting (`FORGE:docs/text-locale.md`); it has no number spell-out, so the reverse direction (words to numbers) is own code. The FUSE a11y bridge can Focus and Click a node and carries name, role and bounds (`FUSE:Source/FUSE/UI/A11y/include/fuse/ui/a11y/a11y.hpp`, `FUSE:Source/FUSE/UI/Core/include/fuse/ui/ui.hpp:86-101`), which is the minimum for click by name and number tags. No audio capture code was read in either repo. Everything else is Proposed.

## 7. Security and privacy

### 7.1 Trust model

Voice control is remote control of the whole desktop by sound. Sound is not a trusted input: a video, a web page, a call, a nearby speaker or another person can produce it. The design treats recognised speech as untrusted input that is allowed to do ordinary things in ordinary windows, and nothing on the trusted path.

Rights, all granted through `svc.a11y` to this system-signed bundle after the user enables Voice in SY01 through an SH04 prompt (which must be answered physically), shown and revocable in AD05:

| Right | Grants | Limits |
|---|---|---|
| `svc.a11y:tree.read` | Names, roles, states, bounds of the active window and shell surfaces (all windows only while "numbers everywhere" is shown) | No secure surfaces |
| `svc.a11y:text.read` | Text ranges of the focused field, for selection and correction commands | Never secret-field values; off until editing commands are enabled |
| `svc.a11y:events` | Focus and window changes | — |
| `svc.a11y:act` | Element actions (click, focus, set value, scroll, named app actions) | Refused on secure surfaces |
| `svc.a11y:input.synth` (voice class) | Keys and pointer events without a hardware serial | Only while a voice session is active and its indicator is shown; never to secure surfaces; at most 20 events a second; modifiers auto-release after 10 s |
| `svc.a11y:overlay` | Number tags, grid, target ring | Input-transparent; below secure surfaces |
| `dev.mic` | The microphone stream | Portal prompt; system indicator; hard mute wins |

Voice holds no `secure.read`: it cannot read a prompt. It learns only that a prompt is open and which choices exist, from SH04's prompt interface.

### 7.2 Capabilities requested

| Capability | Why |
|---|---|
| `fs.app` | Vocabulary, commands, scripts, training clips, adapter |
| `fs.portal` | Import and export |
| `net.none` | Audio and text must not be able to leave |
| `proc.helper:audio-front-helper` | Audio processing outside the script engine |
| `dev.mic` | Speech input |
| `svc.portal.mic`, `svc.media` | Capture; playback reference and cues |
| `svc.model`, `ai.local` | Recognition, wake word, optional neural noise suppression, optional adapter training |
| `dev.gpu.compute` | Optional: faster recognition through `svc.model` |
| `svc.a11y` (rights above) | Voice control |
| `svc.input-method` | Dictation commits through AX04 |
| `svc.clipboard` | "copy that", "paste that" as user-commanded actions (routed to AX05's actions; no history access) |
| `svc.intent` | Open apps, search, send text |
| `svc.settings`, `svc.notify`, `svc.audit` | Settings, notices, audit of prompt outcomes and confirmed commands |
| `bg.run`, `bg.autostart` | Runs without a window; optional start into Off or Sleeping |

### 7.3 Threat model

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| The desktop | Audio from an app, a video, a call | Play "open terminal, type …" through the speakers | Playback rejection (F095); confirmed-command set (F052); HUD shows every recognised command; stop words |
| The desktop | A person or device in the room | Speak commands | Push to talk; sleep mode; no approval of prompts by voice; not solvable in general, stated in help |
| Capability grants | Any of the above | Say "allow" at a prompt | F110; second factor (7.4) |
| Capability grants | Compromised Voice process | Click Allow by synthetic input or `act` | Refused on secure surfaces by the compositor and `svc.a11y`, whatever Voice asks |
| Passwords | The user's own voice | Dictating a password aloud | Dictation refused in secret fields (F026); authentication never by voice (F114) |
| Conversations in the room | Voice itself | Record or send ambient audio | `net.none`; no audio storage (F119); 2 s ring for the wake word (F085); microphone indicator always on while capturing (F086) |
| Recognised text | Voice itself | Keep a transcript of everything said | Memory ring of 50 utterances, cleared on sleep and lock (F120); Privacy page shows contents |
| Training recordings | Other local users, backups | Voice samples leak | Stored only with consent, in the encrypted home, listed and erasable; excluded from sync |
| Wrong target | Timing | A window appears under a number tag just before the click | Stale guard (F068) |
| Wrong command | Misrecognition | "delete" heard for "repeat" | Confidence threshold and "did you say"; confirmation for destructive commands; Undo-first design of the suite |
| Voice integrity | Hostile command script | Do more than the user sees | No capabilities; API only; same secure-surface rules; scripts disabled on import (F080) |
| Helper | Hostile audio or model output | Parser or DSP memory errors | Helper in capability mode; model output schema-checked with size limits; fuzzing |
| Model | Tampered `.fpk` | Biased or backdoored recognition (a trigger sound that types text) | Signed models only (AI02); a backdoored model can still only produce text, which goes through the same command rules |
| Other apps | Rogue caller of `voice.set_mode` | Wake Voice silently | The interface can only reduce listening |

Stored: vocabulary, replacement rules, commands, scripts, thresholds; training clips and adapter only if the user made them. Not stored: audio, transcripts. Leaves the machine: nothing.

### 7.4 Hands-free handling of security prompts

The OS rule is that a capability is granted only by an act that software cannot fake (AX01 section 7.1 rule 4). Speech fails that test: it can be replayed, synthesised, or injected through the loudspeakers. So:

1. **Deny by voice is always allowed.** A false "deny" costs a retry.
2. **Allow by voice alone is never allowed.**
3. **Default second factor: a physical act.** Voice arms a choice; a key, switch, touch or security-key button completes it within 10 s. The act is read by the trusted input path. For a switch user this is one press; for a user who can press nothing, see 4.
4. **Fallback second factor: spoken challenge** (F112). It proves the speaker can perceive the secure surface right now, which defeats pre-recorded audio and blind injection by apps (apps cannot read secure surfaces). It does not defeat a live attacker who can both see or hear the challenge and speak into the microphone, for example a person in the room or a remote party if the challenge were played on loudspeakers. Hence the limits: opt-in, never on loudspeakers, low-risk capability classes only, playback rejection required, every use audited.
5. **Not used:** voice biometrics (F113); "trusted phrases" that never change (replayable).
6. **Alternative for users with no physical input:** an administrator or the user (with help, once) can pre-grant chosen capabilities to chosen apps in AD05, so fewer prompts appear. This is the recommended set-up and the first-run guide says so.

Open point: whether the spoken challenge should exist at all is an owner decision (section 13).

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `enabled` | bool | false | Voice available at all |
| `start_mode` | enum{off, sleeping} | `off` | Mode after login |
| `lang.primary`, `lang.others` | string, string[] | system language, `[]` | Recognition languages |
| `model.command`, `model.dictation` | string | smallest and largest installed for the language | Speech models |
| `dictation.auto_punctuation` | bool | true | |
| `dictation.spoken_punctuation` | bool | true | |
| `dictation.itn` | bool | true | Number and date formatting |
| `dictation.auto_stop_s` | u16 | 30 | Silence before stop; 0 = never |
| `dictation.mixed_mode` | bool | true | Commands inside dictation |
| `dictation.profanity_mask` | bool | false | |
| `dictation.provisional` | enum{preedit, bubble, off} | `preedit` | Where partial text shows |
| `control.enabled` | bool | false | Voice control |
| `control.confirm_set` | string[] | built-in destructive list | Commands that need "confirm" |
| `control.confidence` | f32 | 0.75 | Below this, ask |
| `control.did_you_say_timeout_s` | u8 | 4 | |
| `overlay.tag_pt`, `overlay.colours` | u8, enum | 14, `high_contrast` | Number tags |
| `overlay.grid` | enum{recursive3, fine, window} | `recursive3` | |
| `overlay.numbers_scope` | enum{window, everywhere} | `window` | |
| `mic.device` | string | `system` | |
| `mic.push_to_talk` | bool | false | |
| `mic.ptt_binding` | string | `Super+H` hold | |
| `mic.resume_after_unlock` | bool | false | |
| `mic.sleep_during_calls` | bool | true | |
| `vad.end_silence_ms` | u16 | 700 | 400–2000 |
| `vad.pad_start_ms`, `vad.pad_end_ms` | u16 | 200, 500 | |
| `noise.suppression` | enum{off, low, medium, high, neural} | `medium` | |
| `noise.playback_rejection` | bool | true | Cannot be turned off while spoken challenge is enabled |
| `wake.enabled` | bool | false | |
| `wake.phrase`, `wake.sensitivity` | string, u8 | `wake up`, 50 | |
| `prompts.approve` | enum{physical, challenge, never} | `physical` | Section 7.4 |
| `prompts.challenge_classes` | string[] | `[]` | Capability classes allowed for the challenge |
| `prompts.challenge_output` | enum{screen, screen_braille, screen_headphones} | `screen` | |
| `feedback.sounds` | bool | true | |
| `feedback.speak` | bool | false | Spoken feedback through AX01 |
| `history.utterances` | u8 | 50 | 0 disables corrections of older text |
| `vocab.use_contacts` | bool | false | Names from CO03 |
| `training.keep_recordings` | bool | false | Consent for guided reading clips |
| `hud.position`, `hud.opacity_percent` | enum, u8 | `bottom_centre`, 90 | |
| `profile.atypical_speech` | bool | false | F100 |

## 9. Accessibility and languages

- Voice is itself an access tool; its own windows must be fully operable by voice (every control has a speakable name, checked by the AX01 inspector lint) and by one switch through AX03.
- All state is shown in text on the HUD; cues have a visual twin; nothing depends on hearing a sound.
- Spoken command phrases are translated per language as data tables, not code; each language ships its punctuation words, number grammar and confirm words.
- Languages without a speech model are listed as unavailable with the reason; the interface language and the recognition language are independent.
- Inverse text normalisation follows the locale: decimal comma, date order, currency position (`forge::locale`).
- Word-level editing commands need word segmentation; for Thai, Lao, Khmer and Burmese they fall back to grapheme and space units (`FORGE:docs/text-locale.md` Limits).
- Number tags and grid digits scale with the system text size and with AX02 zoom (the overlay is in the zoomed scene).
- Users with limited speech: longer pauses, sound commands (F099), small fixed vocabularies with custom phrases.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| End of a command utterance to action | under 300 ms with the small model |
| Partial dictation text update | at least every 300 ms |
| End of speech to final dictation text | under 500 ms; real-time factor under 0.5 on the reference CPU (FORGE plan budget: faster than real time) |
| Wake word detection delay | under 400 ms |
| Sleeping with wake word, CPU | under 2% of one core |
| Off, or Sleeping without wake word | zero wake-ups, model unloaded after 60 s |
| Audio front helper memory | under 30 MB |
| App host memory (no model) | under 40 MB |
| Number overlay for 500 elements | under 50 ms |
| Longest continuous dictation | unbounded; memory constant |
| Model memory | set by the model; shown before enabling; within the `svc.model` budget |

## 11. Testing and proof

Nothing has run. All of this is plan.

- Unit (ForgeScript): command grammar parser and matcher with slots and aliases; conflict check; spoken punctuation and inverse text normalisation tables per language; replacement rules; mixed-mode decision; scratch-that length tracking; prompt-mode state machine; confirmation timing.
- Unit (C++): resampler, detector and echo canceller against synthetic signals; ring buffer zeroing; frame timing.
- Golden: HUD states, number overlay and grid renders over fixture windows; settings pages.
- Scripted UI with recorded audio: a corpus of command and dictation clips (recorded with consent, kept in the test repository) fed to the helper; assert actions through a fake `svc.a11y` (FORGE-WP-35.4) and committed text in fixture fields.
- Security tests: commands played through the speakers while Voice listens (must be rejected); "allow" spoken at a prompt (must do nothing); synthetic input and `act` aimed at a secure surface; dictation into a password field; a script calling beyond its API; `voice.set_mode` trying to wake; kill the helper while capturing and check the ring is zeroed and the mode after restart.
- Fuzz: model result records; command and vocabulary import JSON; WAV reader; replacement regexes (FORGE step cap).
- Accuracy: word error rate per language and noise level; command accuracy and false-accept rate per hour of background speech; wake-word false accepts per day. Tracked as numbers.
- Human: sessions with voice-only users, users with atypical speech, and a privacy review of the HUD wording.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Secure-surface gate (compositor, `svc.a11y`) | No voice-class input or `act` reaches a secure surface | T1 | Shared model with AX03's emit gate; proof | Compositor is a scaffold |
| Prompt-mode state machine | "Allow" is reached only by `arm` followed by a trusted physical event, or by a matching challenge within its class limits; `deny` needs no factor | T1 | Model in Lean; refinement of the SH04 interface by PRISM | SH04 interface not designed |
| Listening-state honesty | Capture is on if and only if the mode is not Off and (not Sleeping or wake word on); the indicator matches | T1 | Small state model across core and helper | Indicator is SH02's; cross-process proof |
| Audio ring | Audio older than the bound is zeroed; no path writes audio to a file unless training consent is set | T2 | PRISM on the helper; code review of file paths (the helper has no file handle unless training is on) | — |
| Audio front helper DSP and record parsers | No runtime errors | T2 | PRISM bounded checks; sanitizers; fuzzing | Floating-point reasoning |
| Command matcher (ForgeScript) | Terminates; picks the unique best match or asks | T3 | Unit tests; property tests | ForgeScript verification tooling |
| Command scripts | Cannot gain capabilities | T2 | Policy compiler proof (FORGE-WP-08) with an empty set | Depends on that package |
| Recognition accuracy | — | T4 | Measured | Models are not provable |

## 12. Build plan

### Phase 1: usable core (dictation)

| ID | Work item | Depends on |
|---|---|---|
| AX06-W01 | Audio front helper: capture, resample, gain, detector, features | `svc.portal.mic`, FORGE-WP-27, FORGE-WP-28 |
| AX06-W02 | Speech session client and model selection | AX06-GAP-1, G-15, FORGE-WP-29, FORGE-WP-30, FORGE-WP-31.3, AI02 |
| AX06-W03 | Modes, HUD, indicator, hard-mute and lock behaviour | G-02, G-07, SH02 |
| AX06-W04 | Dictation commit through AX04's panel; provisional text | AX04-W10, AX06-GAP-6 |
| AX06-W05 | Spoken punctuation, automatic punctuation switch, literal escape, scratch that, undo | AX06-W04 |
| AX06-W06 | Secret-field refusal; no-storage rules; privacy page basics | AX06-W03 |
| AX06-W07 | Settings window: general, dictation, microphone | AX06-W03 |
| AX06-W08 | Push to talk, auto-stop, stop words on the small model | AX06-W02 |
| AX06-W09 | Crash recovery | AX06-W01 |

### Phase 2: full feature (voice control)

| ID | Work item | Depends on |
|---|---|---|
| AX06-W10 | Command grammar, matcher, built-in command sets, conflict check | AX06-W02 |
| AX06-W11 | Voice-class synthetic input and `svc.a11y` rights | AX06-GAP-4, G-04, FUSE-WP-56 |
| AX06-W12 | Click by name, role words, name matching | AX06-W11, FUSE-WP-55 |
| AX06-W13 | Number overlay and grid overlay with stale guard | AX06-W11, `svc.a11y:overlay` |
| AX06-W14 | Window, app, shell, key and scroll commands | SH01, SH05, AX06-W10 |
| AX06-W15 | Confirmed commands; "did you say"; what can I say | AX06-W10 |
| AX06-W16 | Correction list, n-best, select, move, delete, replace | AX06-GAP-7, AX06-W05 |
| AX06-W17 | Formatting commands, inverse text normalisation, spelling mode, capitalisation and spacing | AX06-W05 |
| AX06-W18 | Vocabulary, replacement rules, learning from corrections, command editor and practice | AX06-W10 |
| AX06-W19 | Noise suppression, playback rejection, self-speech rejection | AX06-GAP-3, AX01-W23 |
| AX06-W20 | Prompt mode: deny by voice, arm plus physical confirm, audit | AX06-GAP-5, SH04, AX03-W08 |
| AX06-W21 | Microphone setup guide, threshold tuning, level warnings | AX06-W01 |
| AX06-W22 | Clipboard and read-back commands; scratchpad | AX05, AX01 |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| AX06-W23 | Wake word | AX06-GAP-2 |
| AX06-W24 | ForgeScript commands, per-app command sets, import and export | FORGE-WP-08, AX06-W10 |
| AX06-W25 | Spoken challenge (if approved) | owner decision, AX06-W20, AX06-W19 |
| AX06-W26 | Guided reading, speaker adapter | FORGE adapter support (Unverified), AX06-W21 |
| AX06-W27 | Pointer movement, drag by voice, grid variants, numbers always | AX06-W13 |
| AX06-W28 | App action commands ("bold that"), media and tool commands | FUSE-WP-14.6, AX02, AX03 |
| AX06-W29 | Atypical speech profile, sound commands, far-field warning | AX06-W21 |
| AX06-W30 | Profiles, accuracy report, profanity mask | AX06-W18 |

## 13. Open questions and risks

1. **The speech model does not exist in FORGE yet.** FORGE-WP-31.3 is a research-sized import (FORGE plan risk RK-10). No trained model is chosen; its licence and its training-data licence must allow redistribution (FORGE plan Q11). Voice is P3 for this reason.
2. **Spoken challenge.** It gives users with no physical input a way to approve prompts, at a real security cost (section 7.4 item 4). Owner decision: ship it limited as specified, or ship only pre-granting in AD05.
3. **Voice-class synthetic input.** It weakens the "hardware provenance" rule that AX03 keeps. The limits (active session, indicator, no secure surfaces, rate cap) contain it, but a compromised Voice process can drive ordinary windows. That is inherent in voice control and must be stated in AD05 when the user enables it.
4. **Playback rejection quality.** Echo cancellation fails with loud speakers, distortion or Bluetooth delay. The veto errs towards dropping commands; users with speakers may see refusals. Headsets are recommended in the setup guide.
5. **Phrase boosting.** Whether the model class can take boost phrases at run time is unknown until FORGE-WP-31.3 is designed; the fallback is post-recognition matching, which is weaker for rare names.
6. **Speaker adaptation.** Depends on adapter support in FORGE that is not planned in the build plan read; marked Could.
7. **Pre-edit from a panel** needs an extension to AX04's panel connection (AX06-GAP-6).
8. **Languages.** Command tables and number grammars per language are large data work; Phase 1 targets English only, then languages in the order of available models.
9. **Greeter and lock screen.** No voice path exists there by design. Users who can only speak need another way to log in: automatic unlock by a hardware token (SE04, SH03) is the suggested route, outside this spec.
10. **Shortcut choices.** `Super+H`, `Super+Alt+H`, `Super+Alt+N`, `Super+Alt+Y`, `Super+Alt+W` must be reserved in SH05's map and checked against AX01–AX05.
