<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Sound Recorder

By Odin Loch

| | |
|---|---|
| ID | `ME06` (from CATALOGUE.md) |
| Bundle | `org.pbsd.sound-recorder` |
| Category | media |
| Priority | P2 |
| Backing repos | FUSE (`fuse_audio` output engine, FLAC and Vorbis encoders; no capture code found). No recorder exists. |
| Status | Planned. Nothing built. |

## 1. Purpose

Sound Recorder records voice and other audio from a microphone, from the system or from one app, and keeps the recordings findable. It is for meetings, lectures, interviews, voice notes and quick samples. It trims and cleans recordings and can transcribe them on the device with speaker labels. It replaces KRecorder and similar tools; real editing belongs to CR03.

## 2. Scope

In scope:

- Recording from microphone, system output, or a single app; several sources at once.
- Formats, levels, monitoring, markers, pause and resume.
- Trim, split, join, fades, normalise, noise reduction.
- Local transcription with speaker labels, search inside recordings, tags.
- Export and share; auto-stop and auto-start rules while the app is in use.
- Hand-off to CR03 for editing.
- Indicator and `dev.mic` portal use; no hidden recording.

Out of scope:

| Item | Owner |
|---|---|
| Multitrack editing, effects chains, MIDI, mastering | CR03 Audio Editor |
| Screen recording with sound | ME05 Screen Capture |
| Video recording | ME04 Camera |
| Dictation into other apps, voice control | AX06 Voice |
| Call recording inside the chat app | CO05 Messenger (may hand finished files here) |
| Music library and playback of music | ME02 Music |
| Audio device setup, input gain at system level | SY01 Settings, `svc.media` |
| Model install and verification | AI02 Model Manager |
| Notes that embed recordings | OF02 Notes (receives recordings and transcripts by intent) |
| Codecs, resampling, audio input transport | Media pipeline, ME03 section 6.5 |

## 3. Users and top tasks

Personas:

- **Note taker**: taps record, talks, stops; wants it titled and searchable later.
- **Student**: records a 90-minute lecture, marks key moments, reads the transcript.
- **Interviewer**: two speakers, needs speaker labels, time-stamped quotes, export to text.
- **Musician or maker**: records a sample or an idea in lossless quality.
- **Careful user**: wants proof that nothing records unless the indicator is on, and that nothing leaves the machine.

| # | Task | Target steps |
|---|---|---|
| T1 | Start recording | 1 (`R` or the big button) after the first grant |
| T2 | Stop and keep | 1 (`R` again or `Space`) |
| T3 | Drop a marker while recording | 1 key (`M`) |
| T4 | Pause and resume | 1 key (`P`) |
| T5 | Trim silence from both ends | 2 |
| T6 | Transcribe a recording | 1 (`Ctrl+T`), then wait |
| T7 | Find where a word was said, across all recordings | 1 search line, 1 click to play there |
| T8 | Rename speakers | 2 |
| T9 | Export a quote as audio and text | 3 |
| T10 | Record what an app is playing | 3 (source, pick app, record) |
| T11 | Send to the Audio Editor | 1 |
| T12 | Share a recording without metadata | 2 |
| T13 | Record until silence lasts 30 s | 2 (set rule, record) |

## 4. Features

### 4.1 Sources and recording

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME06-F001 | Microphone by portal | Audio input comes from `svc.portal.mic`; the shell prompt names the device; the app gets a PCM ring, not the device. | M | 1 |
| ME06-F002 | Record, stop | `R` toggles; large primary button; the timer and waveform start at once. Stop saves automatically with a default title. | M | 1 |
| ME06-F003 | Pause, resume | `P`. Pauses are marked in the file as markers; the indicator stays on while paused only if the stream stays open (setting: release the microphone on pause, default on after 30 s). | M | 1 |
| ME06-F004 | Discard | `Ctrl+Backspace` while recording or right after: moves to trash with Undo. | M | 1 |
| ME06-F005 | Input device choice | List from the portal; follows the system default unless pinned; hot-plug handled; a lost device pauses with a bar and keeps the file safe. | M | 1 |
| ME06-F006 | Channels | Mono (default for voice), stereo, or all device channels up to 32 as a multichannel file. Channel pick for interfaces. | M | 1 |
| ME06-F007 | System audio | Record what the system plays through a monitor tap from `svc.media`, granted in the screen-and-sound portal dialog (same consent as ME05); indicator shown. | S | 2 |
| ME06-F008 | Single app audio | Pick one app in the portal dialog; only that app's stream is tapped. | S | 2 |
| ME06-F009 | Several sources | Microphone plus system or app: mixed to one track, or kept as separate tracks (multichannel FLAC or Matroska audio). | S | 2 |
| ME06-F010 | Input gain | Software gain −20 to +30 dB before the file; system input gain is shown and linked to SY01. | M | 1 |
| ME06-F011 | Automatic gain | Optional slow automatic gain with a ceiling; off for music mode. | S | 2 |
| ME06-F012 | Limiter | Soft limiter at −1 dBFS to avoid clipping; clip counter. | S | 1 |
| ME06-F013 | High-pass filter | 80 Hz rumble filter for voice. | S | 1 |
| ME06-F014 | Live noise suppression | Optional real-time suppression while recording (own spectral method; neural model through `svc.model` optional). The raw take can be kept beside the processed one. | S | 2 |
| ME06-F015 | Echo cancellation | When recording mic and system together, optional cancellation of speaker bleed. | C | 3 |
| ME06-F016 | Pre-roll | Keeps the last 0–10 s before `R` was pressed. The microphone must already be open, so this works only while the window is visible and "armed"; the indicator is on while armed. Off by default. | C | 2 |
| ME06-F017 | Record modes | Presets: Voice (mono, 48 kHz, Opus 32 kbit/s, HPF, limiter), Meeting (mono, noise suppression, FLAC), Music (stereo, 48 or 96 kHz, 24-bit FLAC, no processing), Custom. | M | 1 |
| ME06-F018 | Timed recording | Start at a time and stop at a time while the app is open and visible; shows a countdown. No unattended background schedule. | C | 3 |
| ME06-F019 | Append | Continue recording at the end of an existing recording. | S | 2 |
| ME06-F020 | Background recording | Recording continues with the window minimised (`bg.run`), with the shell indicator and a notification with Stop. Closing the window asks: keep recording or stop. | M | 1 |
| ME06-F021 | Lock behaviour | On session lock the recording continues by default (lectures) with the lock screen showing "Recording" and a Stop button (SH03); setting to pause on lock. | S | 2 |
| ME06-F022 | Sleep inhibit | Holds a system-sleep inhibitor while recording (`svc.power`). | M | 1 |

### 4.2 Levels and monitoring

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME06-F023 | Level meters | Peak and RMS per channel, peak hold, clip light; shown before recording starts (after the grant) so levels can be set. | M | 1 |
| ME06-F024 | Loudness | Momentary and short-term LUFS (EBU R128) and integrated loudness of the take. | S | 2 |
| ME06-F025 | Live waveform | Scrolling waveform with markers and pause gaps. | M | 1 |
| ME06-F026 | Spectrogram | Optional live spectrogram view. | C | 3 |
| ME06-F027 | Monitoring | Hear the input through headphones; off by default; warns about feedback when the output is a speaker; latency shown. | S | 2 |
| ME06-F028 | Level test | "Say something" check that tells the user if the input is too quiet, too loud or silent (wrong device). | S | 1 |
| ME06-F029 | Silence warning | If the input is digital silence for 5 s while recording, a bar says "No sound is arriving" with the device name. | M | 1 |
| ME06-F030 | Time and size | Elapsed time, file size, remaining time on disk. | M | 1 |

### 4.3 Formats and files

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME06-F031 | Formats | Opus in Ogg (default for voice), FLAC (default for lossless), WAV (PCM 16/24-bit, float 32), Vorbis; MP3 and AAC export only if the restricted pack has encoders (ME03 section 6.5.3). | M | 1 |
| ME06-F032 | Sample rate and depth | 16, 24, 44.1, 48, 96 kHz; 16 or 24-bit, float. The device rate is used when possible; resampling by the pipeline resampler otherwise. | M | 1 |
| ME06-F033 | Crash-safe writing | Ogg pages and FLAC frames are flushed every second; WAV is written with an RF64-ready header and fixed on stop; a recovery pass repairs files after a crash or power cut. | M | 1 |
| ME06-F034 | Long recordings | No length limit; 64-bit sizes (RF64 for WAV over 4 GB); memory use stays flat. | M | 1 |
| ME06-F035 | Split by size or time | Optional new file every n minutes or MB, sample-continuous. | C | 2 |
| ME06-F036 | Save location | `~/Recordings` granted once; per-recording "Save as" through the portal. | M | 1 |
| ME06-F037 | Titles and names | Default title `Recording {YYYY}-{MM}-{DD} {HH}.{mm}`; after transcription the app can suggest a title from the first sentence (local). File name follows the title. | M | 1 |
| ME06-F038 | Metadata written | Title, date, duration, markers (as chapters or cue points), tags. No host name, no user name, no device serial. Location is never recorded. | M | 1 |
| ME06-F039 | Sidecar | Transcript, speakers, markers and notes in `<name>.pbrec.json` beside the audio, so the folder is portable. | M | 2 |
| ME06-F040 | Import | Add existing audio files (any format the pipeline decodes) to the list for trimming and transcription. | S | 2 |
| ME06-F041 | Disk guard | Warns under 1 GB free; stops cleanly under 200 MB. | M | 1 |

### 4.4 Markers and notes

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME06-F042 | Drop marker | `M` while recording or playing; numbered; optional label typed without stopping (`Shift+M`). | M | 1 |
| ME06-F043 | Marker kinds | Plain, important (star), question, action item; colour plus icon. | S | 2 |
| ME06-F044 | Marker list | Side list with time and label; click to play from there; rename, move, delete. | M | 1 |
| ME06-F045 | Notes while recording | A plain-text notes pane; each line is time-stamped at the moment typing started. Clicking a line plays the audio from that moment. | S | 2 |
| ME06-F046 | Regions | Mark a start and end as a named region (for export of quotes). | S | 2 |
| ME06-F047 | Marker export | Markers as Audacity label text, CUE sheet, chapters in Ogg/Matroska, CSV. | S | 2 |
| ME06-F048 | Global marker shortcut | A global shortcut drops a marker while recording in the background (through the shell shortcut service, ME05-GAP-3). | C | 3 |

### 4.5 Playback

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME06-F049 | Play, seek | `Space`; click the waveform; `Left`/`Right` 5 s; `Shift` 1 s; `Home`/`End`. | M | 1 |
| ME06-F050 | Speed | 0.5×–3× with pitch kept (pipeline time-stretch). | M | 1 |
| ME06-F051 | Skip silence | Shortens gaps longer than a threshold during playback only. | S | 2 |
| ME06-F052 | Loop region | Loop a region or between two markers. | S | 2 |
| ME06-F053 | Follow transcript | The current word or segment is highlighted and scrolled into view. | M | 2 |
| ME06-F054 | Voice boost | Playback-only clarity filter (EQ and compression) for quiet speech. | C | 2 |
| ME06-F055 | Output device | Through `svc.media`; media session published so media keys work. | M | 1 |
| ME06-F056 | Per-track solo and mute | For recordings with separate source tracks. | S | 2 |

### 4.6 Trim, split, clean

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME06-F057 | Non-destructive edits | Edits are an edit list in the sidecar; the original audio is untouched until "Apply and save a copy" or "Apply to file". | M | 2 |
| ME06-F058 | Trim ends | Drag the two end handles; "Trim silence at ends" in one step. | M | 1 |
| ME06-F059 | Cut a section | Select a range, `Delete` removes it with a short crossfade (10 ms). | M | 2 |
| ME06-F060 | Split | `S` splits at the play head or at every marker into separate recordings. | M | 2 |
| ME06-F061 | Join | Join selected recordings in order. | S | 2 |
| ME06-F062 | Fades | Fade in and out with length handles. | S | 2 |
| ME06-F063 | Normalise | Peak to −1 dBFS or loudness to a target (−16 LUFS voice, −23 LUFS). | S | 2 |
| ME06-F064 | Noise reduction | Offline reduction: learn a noise profile from a selected silent part or automatically; strength 0–100; preview before and after. Own spectral-subtraction and Wiener code; optional neural model through `svc.model`. | M | 2 |
| ME06-F065 | Remove silence | Cut or shorten gaps above a length; preview total time saved. | S | 2 |
| ME06-F066 | Hum and click removal | 50/60 Hz notch with harmonics; simple de-click. | C | 3 |
| ME06-F067 | Redact audio | Replace a range with silence or a tone, in the audio and in the transcript; applied to exports irreversibly (samples overwritten before encode). | S | 2 |
| ME06-F068 | Lossless cut | For FLAC, WAV and Opus, cuts are made at frame boundaries without re-encoding when possible; otherwise the app says the file will be re-encoded. | S | 2 |
| ME06-F069 | Undo | Unlimited undo of edits; originals recoverable from trash for applied edits. | M | 2 |
| ME06-F070 | Edit in Audio Editor | Sends intent `edit` with the file handle, markers and regions to CR03. | M | 2 |

### 4.7 Transcription

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME06-F071 | Local transcription | Speech-to-text through `svc.model` (`ai.local`). Audio is passed by shared memory. Runs in the background with progress; can be paused; only on mains power by default. | M | 2 |
| ME06-F072 | Model state | Shows the model name, size, signature state and language coverage; "No speech model installed" links to AI02. | M | 2 |
| ME06-F073 | Language | Auto-detect or chosen; several languages in one recording handled per segment if the model supports it. | S | 2 |
| ME06-F074 | Word timestamps | Each word has a start, end and confidence; low-confidence words are underlined. | M | 2 |
| ME06-F075 | Speaker labels | Diarisation with a local model: segments get "Speaker 1", "Speaker 2"; the user renames speakers; merge and split speakers; reassign a segment. Labels are per recording. | M | 2 |
| ME06-F076 | Known voices (optional) | The user may save a voice profile to label the same person across recordings. Stored encrypted with a key in `svc.keyring`; off by default; one switch deletes all profiles. | C | 3 |
| ME06-F077 | Transcript editor | Edit text in place; timing stays attached; find and replace; fix a speaker for a selection. Edits never change the audio. | M | 2 |
| ME06-F078 | Click to play | Click any word to play from it; select text to make a region. | M | 2 |
| ME06-F079 | Live transcription | Optional live captions while recording when the machine can keep up; marked provisional and refined after stop. | C | 3 |
| ME06-F080 | Translation | Optional local translation of the transcript into another language as a second text. | C | 3 |
| ME06-F081 | Summary and action items | Optional local summary, chapter titles and a list of action items through `svc.model`; always marked machine-made and linked to source times. | C | 3 |
| ME06-F082 | Custom vocabulary | A list of names and terms to prefer; per-recording and global. | C | 3 |
| ME06-F083 | Transcript export | Plain text, Markdown (with speakers and times), SRT, WebVTT, JSON (word level), ODT through intent to OF04. | M | 2 |
| ME06-F084 | Quote export | Select text: export that range as an audio clip plus the text with speaker and time. | S | 2 |
| ME06-F085 | Re-run | Transcribe again with another model or language; previous versions kept until removed. | S | 2 |
| ME06-F086 | No cloud path | There is no setting for a remote transcription service. The app has no network capability. | M | 2 |

### 4.8 Library, search, tags

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME06-F087 | Recordings list | All recordings in the granted folders: title, date, duration, tags, speaker count, transcript state, small waveform. Sorted by date; grouped by day or month. | M | 1 |
| ME06-F088 | Search | One field searches titles, tags, notes, marker labels and transcript text across all recordings; results list the matching lines with times; click plays from there. | M | 2 |
| ME06-F089 | Search inside a recording | `Ctrl+F` in the transcript; hits are shown as ticks on the waveform. | M | 2 |
| ME06-F090 | Sound-alike search | Optional phonetic matching for names the model spelled differently. | C | 3 |
| ME06-F091 | Tags | Free tags with colours; filter by tag; bulk tag. | M | 1 |
| ME06-F092 | Folders | Sub-folders in the recordings root are shown as sections; move by drag. | S | 2 |
| ME06-F093 | Favourites and archive | Star; archive hides from the main list. | S | 2 |
| ME06-F094 | Filters | Date range, duration, has transcript, speaker name, source kind (mic, system, app), format. | S | 2 |
| ME06-F095 | System search | Contributes titles and tags to `svc.search`; transcript text only if the user enables it. | S | 2 |
| ME06-F096 | Retention rules | Optional: move recordings older than n days to trash, except starred. Off by default; shows what would be removed before enabling. | C | 3 |
| ME06-F097 | Storage view | Total size by format and by month; "convert old WAV to FLAC" batch action. | C | 3 |

### 4.9 Export, share, automation

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME06-F098 | Export audio | Whole recording, region or between markers; format, bit rate, sample rate, channels (mixdown or split tracks); edits applied. | M | 1 |
| ME06-F099 | Export bundle | Audio plus transcript plus markers in one folder or a ZIP (through UT01's helper by intent). | S | 2 |
| ME06-F100 | Share | Share sheet (SH04). Default strips metadata except title; the transcript is attached only if ticked. | M | 1 |
| ME06-F101 | Send to | Notes (OF02) as an embedded recording with transcript; Mail (CO01) as attachment; Music (ME02) by folder; Audio Editor (CR03). | S | 2 |
| ME06-F102 | Encrypted recordings | Save into a Vault container (SE05) as the recordings root, or encrypt one export to a recipient key through SE05. | S | 3 |
| ME06-F103 | Auto-stop: silence | Stop (or pause) when the input stays below a threshold for n seconds. | M | 2 |
| ME06-F104 | Auto-stop: time and size | Stop after n minutes or n MB. | M | 2 |
| ME06-F105 | Auto-stop: disk and battery | Stop cleanly at low disk or at a battery level. | M | 2 |
| ME06-F106 | Auto-stop: source ended | For app or system recording: stop when the source has been silent or closed for n seconds. | S | 2 |
| ME06-F107 | Voice-activated pause | Pause during silence and resume on sound; gaps marked. Works only inside a recording the user started. | S | 2 |
| ME06-F108 | No auto-start | There are no auto-start rules. Recording starts only from a user action in this session. | M | 1 |
| ME06-F109 | Record intent | Other apps (OF02, CO05) ask for a recording through intent `pick` (audio): this app opens, the user records and confirms, the caller gets one audio handle. | S | 2 |
| ME06-F110 | Command line | `sound-recorder [--record] [--mode=voice\|meeting\|music]` opens the window and, with `--record`, starts after the normal portal grant. No headless mode. | S | 1 |
| ME06-F111 | Bus interface | `org.pbsd.soundrecorder.Recorder1`: state, stop, drop marker. Start is not offered over the bus. | S | 2 |
| ME06-F112 | Audit | The portal logs every microphone and audio-tap start and stop in `svc.audit`. The app shows its own log of recordings with times. | M | 1 |
| ME06-F113 | Recovery | Unfinished files are repaired on start and listed as "Recovered"; edit lists and transcripts are journalled. | M | 1 |
| ME06-F114 | Signed recordings | Optional: sign the audio file hash with a key from SE04 and write a detached `.sig`; "Verify" shows whether a recording changed since it was signed. | C | 3 |
| ME06-F115 | Consent reminder | Optional reminder text shown before recording ("Tell the people you are recording"); a spoken tone can be inserted at start. Laws differ; the app only reminds. | C | 2 |

## 5. GUI

### 5.1 Window map

```
Sound Recorder
├─ Main window (single instance)
│  ├─ Sidebar: All · Starred · Recovered · Archive · Tags ▸ · Folders ▸
│  ├─ Recordings list
│  ├─ Detail pane: waveform · transport · tabs (Transcript · Markers · Notes · Info)
│  ├─ Record bar (bottom): source, mode, meters, record button
│  └─ Inline bars: permission, no input, silence, low disk, device lost, model missing
├─ Recording view (replaces the detail pane while recording)
├─ Compact recorder (small always-on-top window, optional)
├─ Dialogs and sheets
│  ├─ Source and format
│  ├─ Auto-stop rules
│  ├─ Transcribe
│  ├─ Speakers
│  ├─ Noise reduction
│  ├─ Export
│  └─ Split and join
├─ Preferences: General · Recording · Processing · Transcription · Library · Privacy · Shortcuts
└─ Shell-drawn: microphone prompt, sound-source picker (with ME05's portal), file portal, share sheet
```

### 5.2 Main window

Shown at 1100×700, wide breakpoint, a recording selected, transcript tab.

```
+------------------------------------------------------------------------------------------+
| [=] Sound Recorder              [search all recordings____________]     [▁] [▢] [✕]     |(1)
+-----------+--------------------------+---------------------------------------------------+
| All    42 | Today                    | Interview with Mia          12 Nov 2026 · 34:10   |
| Starred 3 | > Interview with Mia     | ▁▂▅▇▅▃▁▁▂▆█▇▅▂▁▁▃▅▇▆▃▁▁▂▅▇▅▂▁▂▃▅▆▇▅▃▁ ▼1  ▼2 ★ (4)  |
| Recovered |   34:10 · 2 spk · ✓ text | [|<] [ ▶ ] [>|]  04:12 / 34:10   [1.0× v] [Trim]  |
| Archive   |   Voice note             +---------------------------------------------------+
|-----------|   0:48 · mic             | [Transcript] Markers  Notes  Info        [Ctrl+F] |
| Tags      | Yesterday                | Mia  04:02                                         |
|  work  12 |   Lecture 7 — proofs     |  I think the main risk is the schedule, not the   |
|  ideas  5 |   1:31:05 · … 62%   (3)  |  budget. We should [move] the review to March.    |
| Folders   |   Team sync              | Odin  04:15                                       |
|  2026     |   25:40 · 4 spk · ✓ text |  Agreed. Can you send the _details_ by Friday?    (5) |
|       (2) |                          | Mia  04:21                                        |
|           |                          |  Yes.                                             |
+-----------+--------------------------+---------------------------------------------------+
| [Microphone: Built-in v] [Voice v]  ▮▮▮▮▮▯▯▯▯ −18 dB   00:00:00      ( ● Record  R )    |(6)
+------------------------------------------------------------------------------------------+
```

1. Header with global search (`lineEdit`). No `[net]` badge exists; a `[mic]` badge shows while any input is open.
2. Sidebar `treeView`: built-in sections, tags, folders.
3. Recordings list (`modelListView`): title, duration, speakers, transcript state or progress.
4. Waveform (custom paint in `viewportWidget`) with markers `▼`, regions, trim handles, search ticks.
5. Tabs (`tabWidget`): Transcript (rich text needed, see 5.7), Markers (`tableView`), Notes (`codeEdit` plain), Info (`propertyGrid`). The playing word is highlighted; underlined words are low confidence.
6. Record bar: source `comboBox`, mode `comboBox`, level meter (`progressBar` pair), timer, record button (primary; red only while recording, with the word "Stop").

States: empty ("No recordings yet. Press R to record."), permission needed (bar with `[Ask again]`, `[Open Permissions]`), no input device, loading (list skeleton), error (unreadable file with reason), read-only (folder not writable: record disabled with reason), locked (recordings in a locked Vault container listed with a lock), transcribing (progress per recording).

### 5.3 Other views and dialogs

**Recording view** (replaces the detail pane)

```
+------------------------------------------------------------------------------------------+
|  ● Recording   00:12:47     FLAC 48 kHz mono · 31 MB · about 9 h left                    |
|                                                                                          |
|  ▁▂▃▅▇▅▃▂▁▁▂▃▅▆▇▇▅▃▂▁▁▁▂▃▅▇▆▅▃▂▁▁▂▃▅▇▅▃▂▁▁▂▃▅▆▇▅▃▁▁▁▂▃▅▇▆▅▃▂▁▁▂▃▅▇  ▼1        ▼2   now│    |
|                                                                                          |
|  L ▮▮▮▮▮▮▮▯▯▯▯▯ −14 dB  peak −6   clip 0        loudness −19 LUFS (short-term)            |
|                                                                                          |
|  Notes (each line is time-stamped)                Markers                                |
|  12:02 budget is fine                              ▼1 03:40  intro done                  |
|  12:31 review moves to March|                      ▼2 11:58  ★ decision                  |
|                                                                                          |
|  Auto-stop: after 30 s of silence · at 2 h         [Marker M] [Pause P] [ ■ Stop R ]     |
+------------------------------------------------------------------------------------------+
```

States: paused (timer blinks off with reduced motion: shows "Paused"), silence warning, device lost (paused, "Reconnect or choose another input"), low disk.

**Source and format** (dialog, 560×420)

```
+--------------------------------------------------------------+
| Record from                                                  |
|  [x] Microphone  [Built-in microphone v]   Channels [Mono v] |
|  [ ] System sound (asks in the system dialog)                |
|  [ ] One app…    (pick in the system dialog)                 |
|  Tracks (•) Mix to one   ( ) Keep sources separate           |
| Processing [x] High-pass 80 Hz [x] Limiter [ ] Noise suppress|
|            [ ] Automatic gain   Gain ──●── +6 dB             |
| Format [FLAC v]  Rate [48 kHz v]  Depth [24-bit v]           |
| Preset [Meeting v]  [Save as preset…]                        |
|                                   [Cancel]  [Done]           |
+--------------------------------------------------------------+
```

**Transcribe** (dialog, 520×340)

```
+----------------------------------------------------------+
| Transcribe on this device                                |
| Model    [speech-to-text, medium v]  (installed, verified)|
| Language [Detect v]                                      |
| [x] Label speakers   Expected [auto v]                   |
| [ ] Summary and action items                             |
| Vocabulary [Mia, PRISM, HardenedBSD_______]              |
| About 5 min for 34 min. Runs in the background.          |
| Nothing leaves this machine.                             |
|                        [Cancel]  [Transcribe]            |
+----------------------------------------------------------+
```

**Speakers** (sheet, 480×320)

```
+--------------------------------------------------+
| Speakers in "Interview with Mia"                 |
| ● Speaker 1  [Mia_________]  18:02  [▶ sample]   |
| ● Speaker 2  [Odin________]  15:40  [▶ sample]   |
| [Merge selected] [Split…]                        |
| [ ] Remember these voices (stored encrypted)     |
|                                  [Done]          |
+--------------------------------------------------+
```

**Noise reduction** (dialog, 620×400)

```
+------------------------------------------------------------------+
| Reduce noise                                                     |
| Noise profile (•) Automatic  ( ) From selection 00:01.2–00:03.0  |
| Method  (•) Spectral (fast)  ( ) Neural model [denoise-voice v]  |
| Strength ──────●── 60     [x] Keep some room tone                |
| Before ▁▂▃▅▇▅▃▂▁▂▃▅▆▇    After ▁▁▂▅▇▅▂▁▁▁▂▅▆▇                     |
| [▶ Before] [▶ After]   Preview range 00:10–00:20                 |
| ( ) Apply to a copy   (•) Keep as an edit (original untouched)   |
|                                   [Cancel]  [Apply]              |
+------------------------------------------------------------------+
```

**Export** (dialog, 560×400)

```
+--------------------------------------------------------------+
| Export "Interview with Mia"                                  |
| Range  (•) Whole  ( ) Region "quote 1"  ( ) Between markers  |
| Audio  [Opus v] [48 kbit/s v] [Mono v]   [x] Apply edits     |
| Text   [ ] Transcript as [Markdown v]  [ ] Markers [CSV v]   |
| Metadata (•) Title only  ( ) Title, date, tags  ( ) None     |
| Save to [Recordings/Export/Interview with Mia.opus] […]      |
| About 12 MB                        [Cancel]  [Export]        |
+--------------------------------------------------------------+
```

**Auto-stop rules** (popover): `[x] Silence for [30] s below [−45] dB → (•) stop ( ) pause`, `[x] After [120] min`, `[ ] After [___] MB`, `[x] Battery under [10]%`, `[x] Low disk`.

**Compact recorder** (320×88): timer, one meter, `[M] [⏸] [■]`, always on top by request to SH05.

### 5.4 Menus

```
File      New recording R · Import audio… Ctrl+O · Export… Ctrl+E · Export bundle… · Share… ·
          Show in Files · Move to trash Delete · Close Ctrl+W · Quit Ctrl+Q
Edit      Undo Ctrl+Z · Redo Ctrl+Shift+Z · Cut range Delete · Copy text Ctrl+C · Select all Ctrl+A ·
          Find in transcript Ctrl+F · Find in all recordings Ctrl+Shift+F · Rename F2 ·
          Tags… Ctrl+L · Preferences Ctrl+,
Record    Record/Stop R · Pause P · Drop marker M · Marker with label Shift+M · Discard Ctrl+Backspace ·
          Source and format… Ctrl+D · Mode ▸ · Auto-stop rules… · Monitor input · Level test…
Playback  Play/Pause Space · Back 5 s Left · Forward 5 s Right · Previous marker Ctrl+Left ·
          Next marker Ctrl+Right · Speed ▸ [ ] · Skip silence · Loop region Ctrl+Shift+L
Audio     Trim ends T · Trim silence at ends · Split at play head S · Split at markers · Join… ·
          Fade in / out · Normalise… · Reduce noise… Ctrl+N · Remove silence… · Redact range… ·
          Apply edits to a copy · Edit in Audio Editor Ctrl+Shift+E
Transcript Transcribe… Ctrl+T · Speakers… · Language ▸ · Re-run… · Summary… · Translate… ·
          Export transcript… · Delete transcript
View      Sidebar F9 · Detail tabs ▸ · Spectrogram · Compact recorder Ctrl+M · Zoom waveform Ctrl++/- ·
          Full screen F11
Help      Help F1 · Shortcuts · About
```

### 5.5 Keyboard and pointer

Keys are in 5.4. Single letters work when no text field has focus; while typing notes during recording, `Ctrl+M` drops a marker and `Ctrl+P` pauses.

| Context | Menu items |
|---|---|
| Recording row | Play · Rename · Tags… · Star · Transcribe… · Export… · Share… · Edit in Audio Editor · Show in Files · Archive · Move to trash |
| Waveform | Add marker here · Add region from selection · Split here · Cut selection · Redact selection · Zoom to selection · Copy time |
| Transcript text | Play from here · Copy · Copy with time and speaker · Make region · Set speaker ▸ · Mark as wrong (underline) · Export quote… |
| Marker | Rename · Kind ▸ · Move to play head · Delete |

Drag and drop: audio files in (import); recordings onto tags and folders; a recording row or a region out as a file handle (rendered with edits) and, for text selections, as plain text and Markdown. Pointer: wheel zooms the waveform at the pointer with `Ctrl`, scrolls otherwise; drag on the waveform selects a range; drag trim handles.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | One line and the record button. |
| Loading | List skeleton; waveform peaks computed in the background and cached. |
| Error | File unreadable or codec missing: row marked with reason; other rows unaffected. |
| Offline | Not relevant; no network use. Offline volumes: rows greyed with the volume name. |
| Locked | Vault-backed folders show a lock; recording into them is disabled until unlocked. |
| Read-only | Record disabled with the reason; playback and export still work. |
| Narrow (<640) | List and detail become two pages; record bar stays at the bottom; tabs become a `comboBox`. |
| No model | Transcript tab shows "No speech model installed" with `[Open Model Manager]`. |
| Recording in background | Shell indicator plus a persistent notification with Stop and Marker. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar | `treeView` |
| Recordings list | `modelListView` over `SqliteModel` |
| Waveform, spectrogram | `viewportWidget` (custom paint); `timeline` is a candidate for markers and regions |
| Transport | `toolButton`, `slider`, `comboBox` |
| Meters | `progressBar`, `label` |
| Transcript | rich text widget (G-10) for speaker blocks, word highlight and underline; phase 2 fallback `listView` of segment rows with `label`s |
| Markers | `tableView` |
| Notes | `codeEdit` in plain mode |
| Info | `propertyGrid` |
| Dialogs | `dialog`, `checkBox`, `radioButton`, `spinBox`, `slider`, `lineEdit` |

Missing: audio input for apps (G-11, FUSE-R-088). G-10 for the transcript editor. `ME06-GAP-1`: `svc.portal.mic` interface (devices, channels, formats, PCM ring, release semantics) is listed in PLATFORM.md section 8 with no definition. `ME06-GAP-2`: system and per-app audio monitor taps in `svc.media` with portal consent (shared need with ME05). `ME06-GAP-3`: lock-screen surface for an ongoing recording with Stop (SH03).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Library, state machine, rules, edit lists, transcript model, search |
| UI | FUSE UI | app host | Section 5 |
| Capture and processing chain | C++23 (platform media session, ME03 section 6.5) | app host audio thread | Reads the PCM ring; gain, HPF, limiter, suppression, meters, loudness; hands blocks to the encoder |
| `encode-helper` | C++23 plus vendored encoders | helper, one writable descriptor | Opus, FLAC, Vorbis, WAV writing with periodic flush |
| `demux-helper`, `decode-helper` | C++23 plus vendored codecs | helpers, no capabilities | Playback and import of audio files |
| `audio-edit-helper` | C++23 | helper, no capabilities (input and output descriptors only) | Applies edit lists: cuts, fades, normalise, noise reduction, redaction; lossless cuts |
| `peaks-helper` | C++23 | helper, no capabilities | Waveform peak files |
| Transcription, diarisation, summary | — | `svc.model` | Local models; audio by shared memory |
| Index worker | ForgeScript | app host background task | Scans folders, reads sidecars, feeds the search index |

```
 svc.portal.mic ─ PCM ring ─┐
 svc.media tap  ─ PCM ring ─┼─► app host audio thread (gain · HPF · limiter · suppress · meters)
                            │            │ blocks (shm)                 ▲ view models
                            │     ┌──────▼───────┐              app logic (ForgeScript) + FUSE UI
                            │     │ encode-helper│─► file fd            │
                            │     └──────────────┘                      │ edit list, jobs
 file fd ─► demux/decode helpers ─ PCM (shm) ─► playback ─► svc.media   │
                     │                                         ┌────────▼────────┐
                     └──────── PCM (shm) ─────────────────────►│ svc.model       │ words, speakers
                                                               └─────────────────┘
 file fd in/out ─► audio-edit-helper ─► new file fd      peaks-helper ─► cache
```

### 6.2 Data model

Stores: `~/.local/share/org.pbsd.sound-recorder/index.sqlite` (rebuildable from sidecars), peak cache in `~/.cache/org.pbsd.sound-recorder/peaks/`, voice profiles `voices.enc` (only if enabled).

| Entity | Fields |
|---|---|
| `recording` | `id INTEGER PK`, `root_id`, `rel_path TEXT`, `content_hash BLOB(32)`, `title TEXT`, `created_at INTEGER`, `duration_ms INTEGER`, `format TEXT`, `sample_rate INTEGER`, `bit_depth INTEGER`, `channels INTEGER`, `tracks_json TEXT` (source kinds), `size INTEGER`, `integrated_lufs REAL`, `peak_dbfs REAL`, `starred INTEGER`, `archived INTEGER`, `recovered INTEGER`, `transcript_state TEXT (none\|queued\|running\|done\|failed)`, `language TEXT`, `signed INTEGER` |
| `marker` | `id`, `recording_id`, `pos_ms INTEGER`, `end_ms INTEGER NULL` (regions), `kind TEXT`, `label TEXT` |
| `note_line` | `recording_id`, `pos_ms`, `text TEXT` |
| `speaker` | `id`, `recording_id`, `label TEXT`, `name TEXT`, `voice_ref INTEGER NULL` |
| `segment` | `id`, `recording_id`, `speaker_id`, `start_ms`, `end_ms`, `text TEXT`, `version INTEGER` |
| `word` | `segment_id`, `idx INTEGER`, `start_ms`, `end_ms`, `text TEXT`, `confidence REAL`, `edited INTEGER` |
| `tag` / `recording_tag` | `id`, `name`, `colour` / `recording_id`, `tag_id` |
| `edit_op` | `recording_id`, `seq INTEGER`, `op TEXT (trim\|cut\|fade\|gain\|normalise\|denoise\|redact\|silence)`, `params_json TEXT` |
| `fts` | SQLite FTS5 over `title`, `tag`, `note_line.text`, `segment.text`, `marker.label` |
| `preset` | `id`, `name`, `source_json`, `processing_json`, `format_json`, `autostop_json` |

Sidecar `<name>.pbrec.json` (documented, open, the portable truth): `{ "version":1, "title":…, "created":…, "audio":{"file":…,"hash":"blake3:…"}, "markers":[…], "notes":[…], "speakers":[…], "segments":[{"speaker":0,"start":…,"end":…,"words":[[start,end,"text",conf],…]}], "edits":[…], "tags":[…], "model":{"name":…,"hash":…} }`. Schema version in both stores; forward-only migration.

### 6.3 File formats and protocols

Codec libraries and licences are in ME03 section 6.5.3.

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Opus in Ogg | yes | yes | libopus + own Ogg muxer (libogg is Verified vendored in FUSE: `FUSE:cmake/FuseXiph.cmake`) | decode / encode helper |
| FLAC | yes | yes | libFLAC 1.5.0 (Verified vendored in FUSE; encode exists as whole-buffer `encode_flac`: `FUSE:Source/FUSE/Audio/include/fuse/audio/audio_codec.hpp`); streaming use is Proposed | decode / encode helper |
| Vorbis | yes | yes | libvorbis 1.3.7 (Verified vendored) | decode / encode helper |
| WAV, RF64, BWF (`bext`, `cue `, `LIST adtl`) | yes | yes | own C++23 | demux helper / encode helper |
| MP3, AAC, others | yes (pipeline) | only with restricted-pack encoders | ME03 section 6.5.3 | decode / encode helper |
| Matroska audio (multi-track) | yes | yes | pipeline muxer | demux / encode helper |
| `.pbrec.json` sidecar | yes | yes | ForgeScript JSON with schema check and size limits | app host (own data; treated as untrusted when imported) |
| SRT, WebVTT, Markdown, plain text, CSV, Audacity labels, CUE | — | yes | ForgeScript | app host |
| Peak files (`.pk`) | yes | yes | own binary: header, min/max int16 pairs per 256 frames | peaks helper |
| Model calls | — | — | `svc.model` IPC schema (FORGE-WP-29, Proposed) | model service |

### 6.4 Integration

- Intents handled: `open` / `view` (audio files, to trim or transcribe), `pick` (record audio for another app), `share` target (import audio). Emits: `edit` (CR03), `share`, `compose` (CO01), `new` (OF02 note with recording), `open` (OF04 with transcript).
- Services: `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.mic`, `svc.portal.screen` (only for the sound-source consent shared with ME05), `svc.intent`, `svc.search`, `svc.keyring` (voice profiles), `svc.clipboard`, `svc.media`, `svc.power`, `svc.model`, `svc.undo-trash`, `svc.audit`.
- Offers: `org.pbsd.soundrecorder.Recorder1`; noise-suppression and loudness code shared as a platform library with ME05, ME04 and CO05 (Proposed).
- Other apps: CR03 (editing), ME05 (system-sound consent, noise suppression), ME03 (pipeline), ME02 (listen in the library by folder), OF02, OF04, CO01, CO05, AX06 (shares speech models, not code paths), AI02, SE04 (signing), SE05 (encrypted root), SH02/SH03 (indicator, lock screen), AD05.
- Backing repos, Verified: `fuse_audio` has an output path only (`AudioBackendKind { Null, OpenAL }`, `submit()` of stereo float blocks: `FUSE:Source/FUSE/Audio/include/fuse/audio/audio_backend.hpp`); the word "capture" there means a test read-back buffer of the Null output, not microphone input. No `alcCapture*` call was found under `FUSE:Source/FUSE/Audio/src/`. Vorbis and FLAC whole-buffer encoders exist (`audio_codec.hpp`). A lock-free SPSC ring exists (`FUSE:Source/FUSE/Audio/include/fuse/audio/spsc_ring.hpp`). FORGE lists speech-to-text as a model class with a budget "faster than real time on CPU" in a build plan (FORGE-WP-31, Proposed there). Everything else here is Proposed.

## 7. Security and privacy

| Capability | Why |
|---|---|
| `fs.app` | Index, peak cache, presets |
| `fs.portal` | Recordings folder, import, export |
| `dev.mic` | The purpose of the app; prompt on first use; indicator while the stream is open |
| `dev.screen` | Only the sound part of the screen-and-sound portal, for system or app audio; asked each time; never requests pixels |
| `ai.local`, `svc.model` | Transcription, diarisation, optional denoise and summary |
| `proc.helper:encode-helper`, `demux-helper`, `decode-helper`, `audio-edit-helper`, `peaks-helper` | Section 6.1 |
| `bg.run` | Keep recording when the window is minimised; shown by indicator and notification |
| `net.none` | No network use |
| `svc.*` listed in 6.4 | As described |

Not requested: any network capability, `dev.camera`, `dev.location`, `proc.exec`, `bg.autostart`, `fs.path:*`.

| Asset | Adversary | Attack surface | Mitigation |
|---|---|---|---|
| Speech in the room | This app if compromised; another app driving it | Microphone stream; bus interface; intents | Portal-owned device; shell indicator; start only by user action (no start over the bus, no auto-start, no headless flag); audit log; revoke in AD05 |
| Other apps' audio | — | System and app taps | Consent each time in the shell dialog; indicator |
| Recordings and transcripts | Other local users; backups; search | Files, sidecars, index, `svc.search` | User-owned folder; transcripts in search only on opt-in; Vault root option; retention rules |
| Voice profiles (biometric) | Local attacker | `voices.enc` | Off by default; encrypted; key in keyring; one-switch delete; never exported |
| Redacted speech | Recipient of an export | Residual audio, transcript text, sidecar | Samples overwritten before encode; transcript words removed from exported text; the sidecar is not shared by default |
| The app host | Crafted audio file or sidecar on import | Demuxers, decoders, JSON | Zero-capability helpers; schema-checked JSON with limits |
| Model output | Speech that sounds like instructions | Summary and action items | Output is text shown to the user; never executed, never sent anywhere |
| Integrity of evidence | Later tampering | Audio files | Optional signature of the content hash (SE04); hash stored in the sidecar |

Stored: audio files, sidecars, index, peaks, optional voice profiles. Leaves the machine: nothing. The manifest declares `net.none`, so transcription cannot go to a remote service.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `record.mode` | enum `voice\|meeting\|music\|custom` | `voice` | Default preset |
| `record.format` | enum `opus\|flac\|wav\|vorbis` | `opus` | File format (voice mode) |
| `record.opus_kbps` | int | 32 | Opus bit rate for voice |
| `record.sample_rate` | int | 48000 | Sample rate |
| `record.bit_depth` | enum `16\|24\|f32` | `24` | Depth for lossless |
| `record.channels` | enum `mono\|stereo\|all` | `mono` | Channel layout |
| `record.input` | string | `default` | Pinned input device |
| `record.gain_db` | float | 0 | Software gain |
| `record.hpf` | bool | true | 80 Hz high-pass |
| `record.limiter` | bool | true | Soft limiter |
| `record.noise_suppression` | enum `off\|spectral\|neural` | `off` | Live suppression |
| `record.keep_raw` | bool | false | Keep the unprocessed take |
| `record.preroll_s` | int | 0 | Pre-roll (needs the armed state) |
| `record.release_mic_on_pause_s` | int | 30 | Close the stream after this long paused |
| `record.pause_on_lock` | bool | false | Pause when the session locks |
| `record.folder` | grant | `~/Recordings` | Recordings root |
| `record.title_template` | string | `Recording {YYYY}-{MM}-{DD} {HH}.{mm}` | Default title |
| `autostop.silence_s` / `autostop.silence_db` | int / float | 0 (off) / −45 | Silence rule |
| `autostop.max_minutes` | int | 0 (off) | Time rule |
| `autostop.battery_percent` | int | 5 | Battery rule |
| `monitor.enabled` | bool | false | Input monitoring |
| `play.skip_silence` | bool | false | Skip silence on playback |
| `transcribe.auto` | bool | false | Transcribe each new recording |
| `transcribe.on_battery` | bool | false | Allow on battery |
| `transcribe.language` | string | `auto` | Language |
| `transcribe.speakers` | bool | true | Label speakers |
| `transcribe.suggest_title` | bool | true | Title from the first sentence |
| `voices.remember` | bool | false | Keep voice profiles |
| `search.transcripts_in_system_search` | bool | false | Send transcript text to `svc.search` |
| `share.strip_metadata` | bool | true | Title-only metadata on share |
| `privacy.consent_reminder` | bool | false | Show a reminder before recording |
| `library.retention_days` | int | 0 (off) | Auto-trash age |

## 9. Accessibility and languages

- Recording state is announced ("Recording started", "Paused", "Stopped, 12 minutes saved") and always shown in words, not only by a red dot.
- Level meters expose their value as text ("minus 14 decibels"); a spoken level check reads the peak on demand (`Ctrl+I`).
- The waveform has a non-visual equivalent: markers, regions and transcript segments are lists that can be navigated and played with the keyboard.
- Transcripts make recordings usable by deaf and hard-of-hearing users; text size follows the system; speakers are named, not colour-coded only.
- All single-key shortcuts have `Ctrl` variants for use while typing and for sticky keys.
- Transcription languages depend on installed models; UI language and transcript language are independent; right-to-left transcripts need G-03 and G-10.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to window | 150 ms |
| `R` to first sample written (grant already given) | 100 ms |
| Audio thread | No allocation, lock or IPC round trip per block; under 3% of one core at 48 kHz mono with HPF, limiter and spectral suppression |
| Dropped input blocks | 0 in an 8 h soak; any drop is counted and marked in the file |
| Memory while recording | Flat; under 80 MB app host for any length |
| Data at risk on power cut | at most 1 s of audio |
| Open a 3 h recording | waveform visible in 200 ms from cached peaks; 3 s to build peaks the first time |
| Search across 5000 recordings with transcripts | under 80 ms |
| Transcription | faster than real time on CPU (budget from FORGE-WP-31); UI stays at 60 fps |
| Offline noise reduction | 30× real time per core |
| Idle | 0 frames, 0 wake-ups; microphone closed |

## 11. Testing and proof

Nothing has run.

- Unit: state machine (idle, armed, recording, paused, stopping), auto-stop rules, title templates, edit-list algebra (trim then cut equals the composed cut), transcript edits keeping timing, sidecar round trip.
- Golden: `.fuseui` renders of every view; audio goldens for each edit operation, bit-exact for lossless cuts.
- Scripted UI with a synthetic input from the portal test harness: record, marker, pause, stop, trim, export, permission denied, device lost mid-recording, background recording with notification Stop.
- Crash tests: kill the app, the encode helper and the machine (power-cut simulation) during recording; recovered files must be valid up to the last flush.
- Conformance: Opus, FLAC and WAV/RF64 files checked with reference decoders; EBU R128 test signals for the meters.
- Differential: noise reduction against a reference implementation on a noisy-speech corpus (objective scores); transcript exports parsed back.
- Fuzz: WAV/RF64/BWF parser, sidecar JSON import, peak files, edit lists.
- Privacy tests: the portal reports the stream closed within 1 s of stop and after the pause timeout; no start path exists over the bus (negative tests); exported redacted ranges contain only silence samples; packet capture shows zero packets.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Recording state machine | The input stream is open only in states armed, recording, paused-within-timeout; every start has a user-action cause | T1 | Small model; model check plus proof of the transition function | Portal must enforce independently |
| WAV/RF64 writer and repair | Header sizes always match data written after repair; no overflow at 4 GB | T1 | Proof over size arithmetic; kill tests | — |
| Edit-list application | Output length equals the computed length; no out-of-bounds reads | T2 | PRISM; property tests | — |
| Audio redaction | Every output sample in a redacted range is the fill value | T1 | Proof of the overwrite loop (same pattern as ME05) | — |
| Gain, HPF, limiter | Output bounded; no denormal stalls | T2 | PRISM; measurements | Floating-point limits |
| Spectral noise suppression | No runtime errors; bounded latency | T2 | PRISM; corpus tests | Quality is measured, not proved |
| Sidecar import | No runtime errors; schema enforced | T2 | PRISM-equivalent checks on the ForgeScript validator; fuzz | Interpreter is T3 |
| Vendored encoders and decoders | Contained | T4 | Helper sandbox | Third-party C |
| Transcript model | Word times stay within segment times after edits | T3 | Property tests | — |

## 12. Build plan

### Phase 1: record and play

| ID | Work item | Depends on |
|---|---|---|
| ME06-W01 | Microphone portal interface: devices, formats, PCM ring, release (define with SH04) | ME06-GAP-1, G-09, SH04 |
| ME06-W02 | Capture chain on the audio thread: gain, HPF, limiter, meters | G-11, FUSE-R-088, ME03 phase 1 pipeline |
| ME06-W03 | `encode-helper` streaming Opus, FLAC, WAV with flush and recovery | FORGE-WP-27, FORGE-WP-28 |
| ME06-W04 | Recording state machine, background recording, indicator, notification | G-07 (`bg.run`), SH02 |
| ME06-W05 | Main window, recordings list, record bar, recording view | G-01, G-02 |
| ME06-W06 | Playback with speed; waveform peaks (`peaks-helper`) | ME03 pipeline |
| ME06-W07 | Markers, tags, titles, trim ends, export, share | `svc.undo-trash`, SH04 |
| ME06-W08 | Audio input on HardenedBSD verified (devices, rates, latency) | G-18 |

### Phase 2: edit, transcribe, search

| ID | Work item | Depends on |
|---|---|---|
| ME06-W09 | Edit lists, `audio-edit-helper`: cut, split, join, fades, normalise, lossless cuts | — |
| ME06-W10 | Noise reduction (offline and live spectral) | — |
| ME06-W11 | System and per-app audio sources | ME06-GAP-2, ME05 portal work |
| ME06-W12 | Transcription and diarisation through `svc.model`; model state UI | G-15, FORGE-WP-29, FORGE-WP-30, FORGE-WP-31, AI02 |
| ME06-W13 | Transcript editor, click to play, follow playback | G-10 (fallback segment list) |
| ME06-W14 | Sidecar format, index, full-text search, `svc.search` | G-09 |
| ME06-W15 | Auto-stop rules, voice-activated pause | — |
| ME06-W16 | Notes while recording, regions, quote export | — |
| ME06-W17 | Hand-off to CR03, OF02, CO01; `pick` intent; bus interface | G-08, CR03 |
| ME06-W18 | Lock-screen recording surface | ME06-GAP-3, SH03 |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| ME06-W19 | Live transcription, translation, summary, custom vocabulary | G-15 |
| ME06-W20 | Voice profiles (encrypted) | `svc.keyring` |
| ME06-W21 | Echo cancellation, hum and click removal, spectrogram | — |
| ME06-W22 | Signed recordings, encrypted root | SE04, SE05 |
| ME06-W23 | Timed recording, retention rules, storage view, global marker shortcut | ME05-GAP-3 |

## 13. Open questions and risks

1. **Microphone portal and audio taps are undefined** (ME06-GAP-1, ME06-GAP-2). On FreeBSD the kernel audio interface is OSS; how `svc.media` exposes input rings and per-app monitor taps on HardenedBSD is Unverified (G-18).
2. **Background and lock-screen recording** is a real need (lectures) and a real risk. The design relies on the shell indicator, a persistent notification and the lock-screen surface. The owner may prefer "pause on lock" as the default.
3. **Speech models**: which models AI02 ships, their languages, sizes and licences are undecided. Diarisation quality on overlapping speech is limited.
4. **Processing in the app host**: the capture chain is native code on the app host's audio thread, the same platform question as ME02 section 13 item 6.
5. **Recording other people** is regulated differently by place. The app offers a reminder only.
6. **Using `dev.screen` for sound-only consent** is awkward; a separate `dev.audio-capture` capability would state the truth better. Proposed as an addition to PLATFORM.md section 6.
7. **Transcript widget** depends on G-10; the fallback list is workable but weaker for editing.
8. **Voice profiles** are biometric data; shipping the feature at all is an owner decision.
9. **MP3/AAC export** is expected by many recipients but needs restricted-pack encoders (ME03 section 6.5.3).
