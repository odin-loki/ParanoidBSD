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
