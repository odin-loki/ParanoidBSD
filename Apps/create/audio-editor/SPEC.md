<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Audio Editor

By Odin Loch

| | |
|---|---|
| ID | `CR03` (from CATALOGUE.md) |
| Bundle | `org.pbsd.audio-editor` |
| Category | create |
| Priority | P3 |
| Backing repos | FORGE (ForgeScript, signal code, model service), FUSE (UI) |
| Status | Planned. Nothing built. |

Labels: **Verified** = read in a repo at the cited path. **Proposed** = design in this plan. **Unverified** = could not be read.

## 1. Purpose

A multitrack audio workstation for recording, editing, mixing and mastering music, podcasts and sound for picture. It replaces Audacity and Ardour on this OS. Everything runs locally; plugins are confined so that a third-party effect cannot read files or reach the network.

## 2. Scope

**In scope:** multitrack recording; non-destructive clip editing; waveform and spectral views; fades; time-stretch and pitch shift; built-in effects; a sandboxed host for LV2 and CLAP plugins; MIDI tracks, piano roll and built-in instruments; mixer, buses and automation; metering; mastering; stem separation and transcription with local models; export with loudness targets; a podcast workflow.

**Out of scope:**

| Item | Owner |
|---|---|
| One-button voice memos | ME06 Sound Recorder (its files open here) |
| Music library and playback | ME02 Music |
| Video editing; this app only shows a reference picture | CR02 Video Editor |
| System sound settings, device routing policy | SY01 Settings, `svc.media` |
| Game audio authoring (spatial emitters in a world) | CR04 Studio |
| Dictation and voice control | AX06 Voice |
| Score engraving and printing | not planned |
| Model install and signing | AI02 Model Manager |

## 3. Users and top tasks

Personas: **Podcaster** (speech, clean-up, loudness, chapters). **Musician** (multitrack recording, MIDI, mixing). **Sound editor** (dialogue repair, spectral edits, sync to picture). **Mastering engineer** (meters, limiter, dither, formats). **Casual user** (trim a file, convert a format).

| # | Task | Target steps |
|---|---|---|
| T1 | Open a file, trim both ends, export FLAC | 4 |
| T2 | Arm a track and record with a count-in | 2 (`R` on the track, `Ctrl+Space`) |
| T3 | Cut a mistake from a spoken track and close the gap | 2 (select, `Shift+Delete`) |
| T4 | Crossfade two clips | 1 drag |
| T5 | Remove background noise from a voice | 3 |
| T6 | Remove a cough in the spectral view | 3 |
| T7 | Put an EQ and compressor on a track | 2 |
| T8 | Draw volume automation | 2 |
| T9 | Record a MIDI part and fix timing | 3 |
| T10 | Normalise the mix to −16 LUFS and export | 3 |
| T11 | Split a song into vocal, drums, bass, other | 2 |
| T12 | Make a podcast episode with chapters and a transcript | 5 |
| T13 | Recover a take after a crash during recording | 1 |
| T14 | Try an untrusted plugin without risk to files | 2 |

## 4. Features

Priority: M must, S should, C could. Phase 1 usable core, 2 full feature, 3 advanced.

### 4.1 Session and files

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR03-F001 | Session folder | A session is a folder: `session.json`, audio pool, MIDI, peaks cache, autosave (section 6.2). | M | 1 |
| CR03-F002 | Single-file mode | Open one audio file, edit, export, with no session folder. Edits stay non-destructive until export. | M | 1 |
| CR03-F003 | Session settings | Sample rate (44.1 to 192 kHz), recording bit depth (16, 24, 32-float), tempo, time signature, timecode rate, pan law. | M | 1 |
| CR03-F004 | Import | WAV, BWF, AIFF, FLAC, Ogg Vorbis, Opus, MP3, AAC/M4A, WavPack; copy to the pool or link by handle; resample on import or on the fly. | M | 1 |
| CR03-F005 | Import report | Lists rate, channels, clipping, DC offset and embedded markers per file. | S | 2 |
| CR03-F006 | Audio pool | List of all source files with usage count, length, format, hash; find unused; consolidate. | M | 2 |
| CR03-F007 | Templates | Session templates: podcast (2 voices + music), band, voice-over, mastering, empty. | S | 2 |
| CR03-F008 | Snapshots | Named mix snapshots (all mixer and plugin states); recall; compare A/B. | S | 2 |
| CR03-F009 | Session versions | "Save version" writes a new `session.json` beside the old one; audio is shared. Works with DV03 because the file is text. | S | 2 |
| CR03-F010 | Archive session | Collect used audio, trim unused ranges with handles, write a manifest with BLAKE3 hashes, optional FLAC packing. | S | 3 |
| CR03-F011 | Reference picture | Load a video file as a picture track (decoded by the platform pipeline) for sound-to-picture work. | C | 3 |

### 4.2 Recording

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR03-F012 | Track arming | Arm one or many tracks; choose input device and channels per track through the microphone portal. | M | 1 |
| CR03-F013 | Input monitoring | Off, software (through effects), or auto (monitor when stopped or recording). Shows round-trip latency in ms. | M | 1 |
| CR03-F014 | Record modes | Normal, punch in/out at markers, loop record with takes, pre-roll and post-roll. | M | 2 |
| CR03-F015 | Count-in and metronome | 1 or 2 bars; click sound, level, accent; click to its own output. | M | 1 |
| CR03-F016 | Takes and comping | Each loop pass is a take lane; swipe to pick parts; flatten to a comp. | S | 2 |
| CR03-F017 | Pre-record buffer | Keep the last 5–60 s of armed input; "capture what I just played". | C | 3 |
| CR03-F018 | Safe recording | Audio is written to disk every second with a journal; a crash keeps everything up to the last second. | M | 1 |
| CR03-F019 | Level check | Input meters with peak hold and a clip counter; a "set level" helper that suggests a gain. | M | 1 |
| CR03-F020 | Latency compensation | Measured loop-back latency per device stored and applied to recorded clips. | M | 2 |
| CR03-F021 | Record indicator | The shell's microphone indicator is on while any track is armed, not only while rolling. | M | 1 |
| CR03-F022 | System audio capture | Record what another app plays, only through a `svc.media` capture grant with a prompt. | C | 3 |

### 4.3 Arrangement and clip editing

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR03-F023 | Tracks | Audio (mono, stereo, up to 7.1.4), MIDI, instrument, bus, VCA/group, master, marker, tempo, picture. | M | 1 |
| CR03-F024 | Non-destructive clips | A clip is a reference to a source range with gain, fades, stretch and pitch; sources are never changed. | M | 1 |
| CR03-F025 | Basic edits | Select, move, split (`S`), trim, duplicate, mute, lock, delete, ripple delete, nudge. | M | 1 |
| CR03-F026 | Range selection | Time range across tracks; cut, copy, paste, insert silence, crop to range. | M | 1 |
| CR03-F027 | Snap and grid | Snap to grid (bars, beats, timecode, seconds, samples), markers, clip edges, zero crossings, transients. | M | 1 |
| CR03-F028 | Fades | Fade in and out handles; shapes linear, equal power, S-curve, exponential; default fade length. | M | 1 |
| CR03-F029 | Crossfades | Drag one clip over another on a track; editable shape and length; automatic short crossfades on every cut (setting). | M | 1 |
| CR03-F030 | Clip gain envelope | Gain line inside a clip with points. | M | 2 |
| CR03-F031 | Slip and edit modes | Slip content within a clip; ripple mode (later clips follow), shuffle mode. | S | 2 |
| CR03-F032 | Groups | Group clips or tracks so edits apply to all (multi-mic drums stay phase-aligned). | S | 2 |
| CR03-F033 | Strip silence | Detect silence by threshold and minimum length; split or remove; pads before and after. | M | 2 |
| CR03-F034 | Transient detection and slicing | Find transients; slice at them; quantise slices. | S | 3 |
| CR03-F035 | Markers and regions | Named markers, ranges, chapter markers with URL-free titles and images; marker list dock. | M | 1 |
| CR03-F036 | Tempo and time signature track | Tempo changes and ramps; beat-map a free recording by tapping or detection. | S | 2 |
| CR03-F037 | Loop range and locators | Left and right locators; loop play; select between locators. | M | 1 |
| CR03-F038 | Track folders and colours | Fold tracks into folders; colour labels; track notes. | S | 2 |
| CR03-F039 | Zoom and navigate | Horizontal and vertical zoom, zoom to selection, waveform zoom, overview strip, follow playhead (page or centred). | M | 1 |
| CR03-F040 | Clip reverse, invert, swap channels, mono sum, split stereo | Applied as clip properties (non-destructive). | S | 2 |

### 4.4 Waveform and spectral views

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR03-F041 | Waveform | Peak and RMS, linear or dB scale, rectified option, sample dots above 1:1 zoom. | M | 1 |
| CR03-F042 | Sample editing | Draw samples with a pencil at high zoom (creates an edit layer, source untouched). | C | 3 |
| CR03-F043 | Spectrogram | STFT with window size 256–16 384, linear, log or mel axis, colour maps, gain and range. | M | 2 |
| CR03-F044 | Spectral selection | Rectangle, lasso, brush, magic wand (similar energy), harmonics select. | S | 2 |
| CR03-F045 | Spectral edits | Attenuate, delete, heal (interpolate from surroundings), copy and paste, apply an effect to the selection only. | S | 2 |
| CR03-F046 | Combined view | Waveform over spectrogram with a blend slider; per-track or in a clip editor dock. | S | 2 |
| CR03-F047 | Analysis read-outs | Frequency, note name, level under the pointer; spectrum of the selection; pitch contour overlay. | S | 3 |
| CR03-F048 | Phase and stereo tools | Goniometer, correlation meter, mid/side view. | S | 2 |

### 4.5 Time-stretch and pitch

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR03-F049 | Stretch a clip | `Alt`+drag a clip edge; ratio 25%–400%; algorithms: phase vocoder with transient preservation, WSOLA for speech, resample (tape). | M | 2 |
| CR03-F050 | Pitch shift | ±24 semitones and cents per clip; formant preservation option. | M | 2 |
| CR03-F051 | Follow tempo | Clips with a known tempo follow session tempo changes. | S | 3 |
| CR03-F052 | Warp markers | Pin points in a clip to the grid and stretch between them. | S | 3 |
| CR03-F053 | Pitch correction | Detect notes of a monophonic line; show as blobs; drag to correct pitch, drift and timing. | C | 3 |
| CR03-F054 | Varispeed playback | Play the whole session faster or slower with or without pitch change (for transcription and review). | S | 2 |

### 4.6 Built-in effects

All built-in effects are native DSP inside the audio engine, with reported latency, and work at every session rate.

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR03-F055 | Effects chain | Ordered inserts per clip, track, bus and master; bypass, reorder, copy, wet/dry, A/B, presets. | M | 1 |
| CR03-F056 | Parametric EQ | 8 bands: bell, shelf, high and low pass (6–48 dB/oct), notch; spectrum display behind the curve; linear-phase mode. | M | 1 |
| CR03-F057 | Compressor | Threshold, ratio, knee, attack, release, make-up, look-ahead, sidechain input and filter, gain-reduction meter. | M | 1 |
| CR03-F058 | Limiter | Look-ahead true-peak limiter with ceiling and release; oversampling 4×. | M | 1 |
| CR03-F059 | Gate and expander | Threshold, range, hold, hysteresis, sidechain. | S | 2 |
| CR03-F060 | De-noise | Spectral subtraction with a learned noise print or adaptive mode; reduction and smoothing; "output noise only" audition. | M | 2 |
| CR03-F061 | De-ess | Split-band or wideband; frequency, threshold, listen. | M | 2 |
| CR03-F062 | De-click, de-hum, de-clip | Click repair; hum removal with harmonics (50/60 Hz); reconstruct clipped peaks. | S | 3 |
| CR03-F063 | Reverb | Algorithmic (room, hall, plate) and convolution with impulse files (WAV, FLAC). | M | 2 |
| CR03-F064 | Delay | Time or tempo-synced, feedback, filter, ping-pong, modulation. | M | 2 |
| CR03-F065 | Modulation | Chorus, flanger, phaser, tremolo, vibrato. | S | 3 |
| CR03-F066 | Saturation and distortion | Soft clip, tape, tube curves with oversampling. | S | 3 |
| CR03-F067 | Multiband compressor and dynamic EQ | 3–5 bands with crossover display. | S | 3 |
| CR03-F068 | Utility | Gain, phase invert, DC remove, stereo width, mid/side encode, channel router, tone and noise generator. | M | 1 |
| CR03-F069 | Offline processing | Apply any effect to a selection and render to a new source ("freeze"); still undoable. | M | 2 |
| CR03-F070 | Voice chain preset | One control ("clean voice"): high-pass, de-noise, de-ess, compressor, limiter with a single amount. | S | 2 |

### 4.7 Plugins

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR03-F071 | Policy | By default only built-in effects and signed ForgeScript effect graphs. Native plugins are off until the user turns on the plugin host in Preferences. | M | 2 |
| CR03-F072 | ForgeScript effect graphs | A signed script wires built-in DSP nodes (filters, delays, dynamics, maths) into a new effect and declares its parameters. The script runs at set-up and on parameter change, never per sample. | S | 2 |
| CR03-F073 | Sandboxed native host | LV2 and CLAP plugins run in `plugin-host`, one process per plugin instance (or per plugin, setting), with no capabilities: no files, no network, no devices. Audio and events cross by shared memory. | S | 3 |
| CR03-F074 | Plugin scan | The user picks a plugin folder through the portal; each plugin is loaded once in a throwaway host to read its ports; crashes mark it "failed" and are never retried silently. | S | 3 |
| CR03-F075 | Generic plugin UI | Plugin parameters are shown in a generated panel (sliders, toggles, lists). Plugin-supplied GUIs are not loaded. | S | 3 |
| CR03-F076 | Plugin files | A plugin that needs a file (impulse, sample) gets a read-only handle the user picked; state is saved by the app from the plugin's state interface. | S | 3 |
| CR03-F077 | Crash isolation | A plugin crash mutes that insert, shows a badge and offers "Restart" or "Bypass"; the session keeps playing. | S | 3 |
| CR03-F078 | Plugin trust list | Each plugin binary's BLAKE3 hash is recorded; a changed binary is rescanned and flagged. | S | 3 |
| CR03-F079 | Latency and CPU per plugin | Reported delay is compensated; a per-insert CPU meter shows cost. | S | 3 |

### 4.8 MIDI and instruments

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR03-F080 | MIDI tracks and clips | Record, import and export Standard MIDI Files (type 0 and 1); clips on the timeline. | S | 2 |
| CR03-F081 | Piano roll | Notes on a key/time grid; draw, move, resize, split, glue, mute; velocity lane; scale highlight; fold to used keys. | S | 2 |
| CR03-F082 | Controller lanes | CC, pitch bend, aftertouch, program change as editable curves. | S | 2 |
| CR03-F083 | Quantise and humanise | Grid, strength, swing, quantise ends; random timing and velocity. | S | 2 |
| CR03-F084 | MIDI devices | Input from USB MIDI devices through the device portal; per-track input filter by device and channel; MIDI thru. | S | 2 |
| CR03-F085 | Step input and on-screen keys | Enter notes from the computer keyboard; an on-screen keyboard dock. | C | 3 |
| CR03-F086 | Drum editor | Grid view with named drum lanes. | C | 3 |
| CR03-F087 | Sampler instrument | Built-in: multi-sample zones, velocity layers, loop points, ADSR, filter; reads SFZ and SF2 (read-only). | S | 3 |
| CR03-F088 | Synth instrument | Built-in subtractive synth: 2 oscillators, filter, 2 envelopes, LFO, unison. | S | 3 |
| CR03-F089 | Drum machine instrument | Built-in: 16 pads, one sample each, per-pad tune, decay, pan, choke groups. | C | 3 |
| CR03-F090 | Instrument plugins | CLAP and LV2 instruments through the sandboxed host (4.7). | C | 3 |
| CR03-F091 | Freeze and bounce | Render an instrument track to audio to save CPU; unfreeze restores it. | S | 3 |
| CR03-F092 | MIDI effects | Arpeggiator, chord, transpose, velocity curve as built-in event processors. | C | 3 |

### 4.9 Mixer and automation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR03-F093 | Channel strips | Fader (−∞ to +12 dB), pan (balance or dual pan), mute, solo (in place, safe), record arm, monitor, phase, input gain, inserts, sends, output. | M | 1 |
| CR03-F094 | Buses and sends | Any number of buses; pre- or post-fader sends with level and pan; bus to bus routing without loops (refused with a reason). | M | 2 |
| CR03-F095 | Groups and VCAs | Link faders, mutes, solos; VCA masters. | S | 3 |
| CR03-F096 | Delay compensation | Full plugin and routing delay compensation across tracks, buses and sidechains. | M | 2 |
| CR03-F097 | Automation lanes | Any parameter (volume, pan, mute, send level, effect parameter) as a lane under the track; points and curves. | M | 2 |
| CR03-F098 | Automation modes | Read, touch, latch, write, trim; thin on write. | S | 2 |
| CR03-F099 | Automation editing | Draw with pencil, line, curve tools; move with clips (setting); copy and paste; scale a selection. | M | 2 |
| CR03-F100 | Surround | Bus layouts to 7.1.4; surround panner; downmix check. | C | 3 |
| CR03-F101 | Routing view | A matrix of inputs, tracks, buses and outputs as a `tableView`; click to connect. | S | 3 |
| CR03-F102 | Control surfaces | MIDI controllers mapped to faders and transport by "learn"; mappings saved per device. | C | 3 |
| CR03-F103 | Engine load meter | DSP load percent, buffer under-run counter, disk read and write rate in the status bar. | M | 1 |

### 4.10 Metering and mastering

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR03-F104 | Peak and RMS meters | Per channel with hold and clip indicators; meter ballistics options. | M | 1 |
| CR03-F105 | True peak | Inter-sample peak by 4× oversampling, in dBTP. | M | 2 |
| CR03-F106 | Loudness | Momentary, short-term, integrated LUFS and loudness range (ITU-R BS.1770, EBU R 128); history graph. | M | 2 |
| CR03-F107 | Spectrum analyser | Real-time FFT with averaging, peak hold, 1/3-octave option. | S | 2 |
| CR03-F108 | Mastering chain | A master-bus preset chain: EQ, multiband compressor, stereo width, limiter, dither, with per-stage bypass and a level-matched bypass of the whole chain. | S | 3 |
| CR03-F109 | Dither and noise shaping | TPDF dither; shaped curves; applied last, only when reducing bit depth. | M | 2 |
| CR03-F110 | Reference tracks | Load reference files; switch A/B with loudness matching. | S | 3 |
| CR03-F111 | Album assembly | Track order, gaps, crossfades between songs, per-track metadata, ISRC fields, export a cue sheet. | C | 3 |
| CR03-F112 | Loudness report | Per export: integrated loudness, range, true peak, clipped samples, as text and JSON. | S | 2 |

### 4.11 Local AI tools (through `svc.model`, offline)

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR03-F113 | Stem separation | Split a clip into vocals, drums, bass, other (and piano, guitar where the model allows) as new tracks. | S | 3 |
| CR03-F114 | Speech enhancement | Model de-noise and de-reverb for voice with a strength control; result is a new take. | C | 3 |
| CR03-F115 | Transcription | Speech to text with word timings and speaker turns; a text dock where deleting words makes the matching cuts (edits stay undoable). | S | 3 |
| CR03-F116 | Filler and silence finder | Mark "um", long pauses and repeated words from the transcript for review; never deletes on its own. | C | 3 |
| CR03-F117 | Model disclosure | Model name, version, hash, signer, expected time and memory before first use; jobs run in the background and can be cancelled. | M | 3 |
| CR03-F118 | Provenance | Clips made by a model are flagged in the session and, on request, in export metadata. | S | 3 |

### 4.12 Export, podcast, sharing

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR03-F119 | Export mix | Range (all, loop, selection, regions), format, rate, depth, channels, dither, normalise, real-time or offline. | M | 1 |
| CR03-F120 | Formats | Write WAV, BWF (with bext), FLAC, Ogg Vorbis, Opus, AIFF, MP3, AAC/M4A, WavPack. Defaults are FLAC and Opus. | M | 1 |
| CR03-F121 | Loudness targets | Presets: −14 LUFS (streaming music), −16 LUFS (podcast stereo), −19 LUFS (podcast mono), −23 LUFS (EBU R 128), −24 LKFS (ATSC A/85), custom; true-peak ceiling −1.0 dBTP default. | M | 2 |
| CR03-F122 | Stems and multi-export | Each track, each bus, or each region to its own file with a name pattern. | S | 2 |
| CR03-F123 | Metadata | Title, artist, album, track, year, genre, comment, cover image; Vorbis comments, ID3v2, BWF bext and iXML, MP4 atoms. Nothing personal is written unless typed. | M | 2 |
| CR03-F124 | Podcast chapters | Chapter markers to Opus/Ogg chapter comments, ID3 CHAP frames, MP4 chapters, and a JSON chapters file. | S | 2 |
| CR03-F125 | Podcast workflow | Template with host and guest tracks, music bed with automatic ducking, voice chain, intro/outro slots, loudness target, chapter and show-notes dock, transcript export (VTT, SRT, text). | S | 2 |
| CR03-F126 | Batch convert | Convert a folder of files with a preset and an effect chain; report per file. | S | 2 |
| CR03-F127 | Export queue | Jobs run in the background with progress in notifications. | S | 2 |
| CR03-F128 | Share and hand-off | Share an export through the share sheet (SH04); receive clips from CR02 and return them. | S | 2 |
| CR03-F129 | Verify and sign | BLAKE3 of each export in the report; optional detached signature with a keyring key. | C | 3 |

### 4.13 Safety, recovery, settings, accessibility

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| CR03-F130 | Sandboxed decoding | All compressed formats are decoded in the no-capability codec helper. | M | 1 |
| CR03-F131 | Undo history | Unlimited; a history list dock; survives save. | M | 1 |
| CR03-F132 | Autosave and recovery | Session journal on every edit; on start after a crash, restore edits and any recording in progress. | M | 1 |
| CR03-F133 | Untrusted session | A session from elsewhere opens with script effects and native plugins disabled until allowed. | M | 2 |
| CR03-F134 | Hearing safety | Monitor level limiter (setting) and a warning when output jumps more than 20 dB on play. | S | 2 |
| CR03-F135 | Shortcuts | All actions rebindable; ready-made sets that mirror common workstations. | M | 1 |
| CR03-F136 | Workspaces | Edit, Mix, Record, Spectral, MIDI, Master, Podcast layouts. | S | 2 |
| CR03-F137 | Engine set-up panel | Device, sample rate, buffer size (32–4096 frames), measured latency, a test tone, and a check of the real-time requirements of section 7 with plain pass/fail lines. | M | 1 |
| CR03-F138 | Disk and CPU guard | Free space check before recording and export; estimated recording time left shown beside the transport. | M | 1 |
| CR03-F139 | Screen-reader editing | Announce selection start, end and length; audible cursor; edit by keys only (section 9). | M | 2 |

## 5. GUI

### 5.1 Window map

```
Main window (workbench with docks; workspaces Edit · Mix · Record · Spectral · MIDI · Master · Podcast)
├─ Header bar: menu, session name, search, badges ([mic] while armed, lock)
├─ Transport bar
├─ Arrangement (tracks + timeline)            ├─ Mixer dock
├─ Clip editor dock (waveform / spectral)     ├─ Piano roll dock
├─ Inspector (track, clip, effect)            ├─ Effects rack dock
├─ Browser dock (pool, files, presets, instruments, plugins)
├─ Meters dock (loudness, true peak, spectrum, goniometer)
├─ Markers and chapters dock   ├─ Transcript dock   ├─ History dock   └─ Jobs dock
├─ Dialogs
│   ├─ New session              ├─ Audio engine set-up       ├─ Export
│   ├─ Import report            ├─ Strip silence             ├─ Stretch and pitch
│   ├─ Noise print / De-noise   ├─ Stem separation           ├─ Batch convert
│   ├─ Plugin manager           ├─ Routing matrix            ├─ Archive session
│   ├─ Preferences              └─ Keyboard shortcuts
├─ Effect editors (one floating panel per built-in effect; generic panel for plugins)
└─ Popovers: fade shape, marker edit, tempo edit, input chooser, send level
```

### 5.2 Main window

Shown at 1440 × 900, Edit workspace, compact density. States: empty, loading, recording, engine stopped, read-only.

```
+------------------------------------------------------------------------------------------+
| [=] Audio Editor — episode-12*            [search Ctrl+K]          [mic] [▁] [▢] [✕]    |(1)
+------------------------------------------------------------------------------------------+
| |◀ ◀◀ ■ ▶ ● ⟳ | 00:03:21.480 | 97.1.2 | 120 bpm 4/4 | [click] [snap: grid v] | DSP 12% |(2)
+----------+-------------------------------------------------------------------+-----------+
| Browser  | ruler  0:00      0:30      1:00  ▼   1:30      2:00      2:30     | Inspector |
| ▾ Pool   | markers  ⚑Intro            ⚑Ch 1: Guests                         | Track:Host|
|  host.wav+-----------+-------------------------------------------------------+ In [Mic 1]|
|  guest.. | Host      |[▁▃▆█▆▃▁▁▃▆█▆▃▁  ][▁▃▆▆▃▁▁▃▆█▆▃▁▁▃▆█▆▃▁]            | Out [Voice]|
| ▸ Presets| [M][S][●]◐|   ╲fade            ╳crossfade                         | Gain  0 dB|
| ▸ Instr. | ─ volume  |──────●─────────●──────────────                        | Pan   C   |
| ▸ Plugins+-----------+-------------------------------------------------------+ Inserts   |
|   (3)    | Guest     |[▁▂▅▇▅▂▁▁▂▅▇▅▂▁▁▂▅▇▅▂▁▁▂▅▇▅▂▁      ]                  | 1 EQ      |
|          | [M][S][●]◐|                                                       | 2 De-noise|
|          +-----------+-------------------------------------------------------+ 3 Comp    |
|          | Music     |[▁▁▂▂▃▃▂▂▁▁▂▂▃▃▂▂▁▁▂▂▃▃▂▂▁▁▂▂▃▃▂▂▁▁▂▂▃▃▂▂▁▁▂▂▃▃]     | [+ effect]|
|          | [M][S][ ]◐|                                                       | Sends     |
|          +-----------+-------------------------------------------------------+ Reverb −12|
|          | Voice bus |                           (5)                         |    (6)    |
|          |    (4)    |                                                       |           |
+----------+-----------+-------------------------------------------------------+-----------+
| Clip editor: host.wav  [wave|spectral]  ▁▂▃▅▇█▇▅▃▂▁▁▂▃▅▇█▇▅▃▂▁  sel 0:41.2–0:43.0  (7)  |
+------------------------------------------------------------------------------------------+
| 48 kHz 24-bit · buffer 128 (5.3 ms round trip) · rec time left 41 h · −16.2 LUFS · ✓ saved|(8)
+------------------------------------------------------------------------------------------+
```

1. **Header bar.** `[mic]` shows while any track is armed. No `[net]` badge: the app has `net.none`.
2. **Transport bar**, 40 px: to start, rewind, stop, play, record, loop; time display (click to switch time, bars, timecode, samples); tempo and signature; metronome; snap mode; DSP load.
3. **Browser**, 220 px: `treeView` of pool, presets, instruments, plugins; drag items into tracks.
4. **Track headers**, 160 px: name, mute, solo, arm, monitor `◐`, small meter; automation lanes fold under the track.
5. **Arrangement.** Upgraded `timeline` widget with waveforms from a `PeakSource` (FUSE-WP-49, FUSE-WP-50). Fades and crossfades are drawn on clips with handles.
6. **Inspector**, 240 px: `propertyGrid` for the selected track or clip; insert and send lists.
7. **Clip editor dock**, 160 px: `waveformView` or `spectrogramView` of the selected clip at high zoom.
8. **Status bar.** Rate and depth, buffer and latency, recording time left, integrated loudness of the last play, save state.

### 5.3 Other views and dialogs

**Mixer** (dock or own window, 1200 × 420). States: engine stopped (meters grey), plugin crashed (insert slot shows `✕`).

```
+------------------------------------------------------------------------------------------+
| Host    | Guest   | Music   | FX Rev  || Voice   | Master  |   Loudness                 |
| [Mic 1] | [Mic 2] | [—]     | [bus]   || [bus]   |         |   M −15.8  S −16.4 LUFS    |
| EQ      | EQ      | EQ      | Reverb  || Comp    | Limiter |   I −16.2  LRA 5.1         |
| De-noise| Comp    |         |         || De-ess  | Dither  |   TP −1.3 dBTP             |
| Comp    | [+]     | [+]     | [+]     || [+]     | [+]     |   ▁▂▃▄▅▅▄▅▅▄▃▄▅ history    |
| S Rev−12| S Rev−18|         |         ||         |         |                            |
| pan (C) | pan (C) | pan (C) | pan (C) ||         |         |   Target [−16 LUFS v]      |
|  █ ▮    |  █ ▮    |  █ ▮    |  █ ▮    ||  █ ▮    |  █ ▮▮   |   ✓ within 0.5 LU          |
|  ○ fader|  ○      |  ○      |  ○      ||  ○      |  ○      |                            |
| [M][S][●]| [M][S][●]| [M][S] | [M][S] || [M][S]  | [M]     |                            |
| −3.0 dB | −4.5 dB | −14 dB  | 0.0 dB  || 0.0 dB  | 0.0 dB  |                            |
+------------------------------------------------------------------------------------------+
```

**Spectral workspace** (clip editor maximised). States: computing tiles (coarse first), selection empty (tools disabled).

```
+------------------------------------------------------------------------------------------+
| [▭][lasso][brush][wand][harm] | [attenuate −12 dB] [delete] [heal] [copy] | FFT [4096 v] |
+------------------------------------------------------------------------------------------+
| 16k |░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ |
|  8k |░░▒▒░░░░░░▒▒▒░░░░░░░░░┌──────┐░░░░░░░░▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ |
|  2k |▒▒▓▓▒▒░░▒▒▓▓▓▒▒░░░░░░░│ cough│░░░░▒▒▓▓▒▒░░░░░▒▒▓▓▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ |
| 500 |▓▓██▓▓▒▒▓▓███▓▓▒▒░░░░░└──────┘░░▒▒▓▓██▓▓▒▒░░▒▓▓██▓▓▒▒░░░░░░░░░░░░░░░░░░░░░░░░░░░░ |
| 100 |████████████████▓▓▒▒░░░░░░░░░░▒▒▓▓██████▓▓▒▒▓▓██████▓▓▒▒░░░░░░░░░░░░░░░░░░░░░░░░ |
+------------------------------------------------------------------------------------------+
| pointer: 1.84 kHz (A♯6 −32 ct) −41 dB · selection 0:41.20–0:41.85, 300 Hz–6 kHz         |
+------------------------------------------------------------------------------------------+
```

**Piano roll** (dock, 1200 × 420). States: no MIDI clip selected (empty state with "Create clip").

```
+------------------------------------------------------------------------------------------+
| [✎][select][split][mute] grid [1/16 v] swing [0%] scale [C minor v] [quantise Q]        |
+----+-------------------------------------------------------------------------------------+
| C5 |      [████]                 [██]                                                    |
| B4 |                                                                                     |
| A♯4| [██████]        [████████]        [████]                                            |
| G4 |           [██]                           [██████████]                               |
+----+-------------------------------------------------------------------------------------+
| vel|  ▮   ▮  ▮     ▮           ▮    ▮      ▮                                             |
| CC1|  ╱‾‾‾‾╲______╱‾‾‾‾‾‾‾‾╲___________                                                  |
+----+-------------------------------------------------------------------------------------+
```

**Audio engine set-up** (640 × 520). States: device missing, check failed (row shows what to change and where).

```
+------------------------- Audio engine -------------------------------+
| Output [USB Interface: 1-2 v]   Input [USB Interface: 1-4 v]         |
| Sample rate [48000 v]  Buffer [128 frames v]  → 2.7 ms per buffer    |
| Round-trip latency: 5.3 ms measured  [Measure with loop-back cable]  |
| [Play test tone]                                                     |
| System checks                                                        |
|  ✓ Real-time scheduling granted to the audio helper                  |
|  ✓ Audio buffers locked in memory                                    |
|  ✓ Device opened in exclusive, bit-exact mode                        |
|  ⚠ CPU on power-saving profile — [Switch while this app runs]        |
|  ✓ 0 under-runs in the last test (10 s)                              |
|                                              [Close] [Apply]         |
+----------------------------------------------------------------------+
```

**Export** (760 × 560). States: measuring loudness, over ceiling (warning with the excess), not enough disk.

```
+------------------------------- Export -----------------------------------+
| Preset [Podcast stereo Opus v]            [Save preset…]                 |
| Source (•) Master ( ) Stems by bus ( ) Each track ( ) Each region        |
| Range  (•) Whole session ( ) Loop ( ) Selection                          |
| Format [Opus v]  Bitrate [96 kb/s v]  Rate [48000 v]  Channels [Stereo v]|
| Loudness [x] Normalise to [−16 LUFS v]  Ceiling [−1.0 dBTP]              |
| Dither [none (not needed) v]                                             |
| Metadata  Title [Episode 12___] Artist [________]  [x] Chapters (7)      |
| Also write [x] loudness report  [x] transcript (VTT)  [ ] signature      |
| Measured: −18.4 LUFS, TP −3.2 → gain +2.4 dB, limiter not needed         |
| To [~/Podcasts/ep12.opus…]                 [Cancel] [Queue] [Export]     |
+--------------------------------------------------------------------------+
```

**Plugin manager** (760 × 480). States: host disabled (explains the policy and has one switch), scanning.

```
+----------------------------- Plugins ---------------------------------------+
| Native plugin host [ off |ON ]   Each plugin runs with no file, network or  |
|                                  device access. Plugin windows are not shown.|
| Folders: ~/Plugins/lv2 [x]  ~/Plugins/clap [x]   [Add folder…] [Rescan]     |
| Name             Type  Kind        State      Hash      Latency             |
| Plate reverb     LV2   effect      ✓ ok       9a3f…     0                   |
| Tape delay       CLAP  effect      ✓ ok       11c0…     64                  |
| Big synth        CLAP  instrument  ✕ failed   77be…     —   needs a window  |
| Old comp         LV2   effect      ≠ changed  e2d1…     —   [Rescan]        |
|                                                         [Close]             |
+-----------------------------------------------------------------------------+
```

**Stem separation** (dialog): model name, hash and signer; stems to make (check boxes); quality (fast, best); estimated time and memory; output as new tracks below the source, muted source; Run goes to the Jobs dock.

**Transcript dock**: `listView` of paragraphs with speaker label and time; words are selectable; `Delete` on a word range cuts the audio with ripple; a search field; export button.

### 5.4 Menus

```
File      New Session Ctrl+N · Open Ctrl+O · Recent ▸ · Save Ctrl+S · Save Version · Save As
          Import Audio Ctrl+I · Import MIDI · Export… Ctrl+E · Export Regions · Batch Convert…
          Archive Session… · Session Settings… · Close Ctrl+W · Quit Ctrl+Q
Edit      Undo Ctrl+Z · Redo Ctrl+Shift+Z · Cut/Copy/Paste · Duplicate Ctrl+D · Delete Del
          Ripple Delete Shift+Del · Split at Playhead S · Split at Selection Ctrl+Shift+S…
          Trim to Selection Ctrl+T · Insert Silence · Select ▸ · Shortcuts… · Preferences Ctrl+,
Track     Add Audio Track Ctrl+Shift+N · Add MIDI/Instrument Track · Add Bus · Add Folder
          Duplicate · Delete · Freeze · Arm R · Monitor · Show Automation A · Colour ▸
Clip      Fade In/Out · Crossfade X · Gain… · Normalise… · Reverse · Stretch and Pitch…
          Strip Silence… · Slice at Transients · Group Ctrl+G · Lock · Rename F2
          Separate Stems… · Transcribe… · Open in Clip Editor Enter
Transport Play/Stop Space · Record Ctrl+Space · Loop L · Return to Start Home · Go to… Ctrl+J
          Punch In/Out · Metronome K · Count-in · Follow Playhead F · Varispeed ▸
Markers   Add Marker M · Add Region Shift+M · Add Chapter Ctrl+M · Next/Previous . / ,
          Markers List
Effects   Add Effect ▸ · Voice Chain · Apply to Selection (offline)… · Noise Print
          Bypass All · Plugin Manager…
MIDI      Quantise Q · Humanise · Transpose… · Legato · Step Input · Export MIDI
View      Workspace ▸ · Mixer F3 · Clip Editor F6 · Piano Roll F7 · Meters F8 · Browser F9
          Inspector F4 · Zoom In/Out = / - · Zoom to Selection Z · Waveform/Spectral Tab
          Full Screen F11
Window    (each dock) · Jobs · Audio Engine… · Routing Matrix · Reset Layout
Help      Audio Editor Help F1 · Shortcuts · About
```

### 5.5 Keyboard and pointer

Tools: `1` select, `2` range, `3` split, `4` pencil (automation, samples), `5` stretch, `6` mute, `7` spectral select. Nudge: `Alt+Left/Right` by the grid, `Alt+Shift` by one sample. Track: `Up/Down` select track, `M` mute (when a track header has focus), `Shift+S` solo, `R` arm. `Ctrl+wheel` horizontal zoom at pointer, `Shift+wheel` scroll in time, `Alt+wheel` waveform height.

| Where | Context menu |
|---|---|
| Clip | Split, Duplicate, Mute, Lock, Fade ▸, Gain…, Normalise…, Reverse, Stretch and Pitch…, Strip Silence…, Separate Stems…, Transcribe…, Reveal in Pool, Rename, Colour ▸ |
| Track header | Add Track ▸, Duplicate, Delete, Freeze, Input ▸, Output ▸, Show Automation ▸, Height ▸, Colour ▸ |
| Fade or crossfade | Shape ▸, Length…, Remove |
| Automation point | Value…, Curve ▸, Delete, Select All in Range |
| Insert slot | Bypass, Edit, Replace ▸, Move Up/Down, Copy, Save Preset, Remove |
| Fader | Reset to 0 dB, Enter Value…, Automation Mode ▸ |
| Marker | Rename, Make Chapter, Move to Playhead, Delete |
| Piano-roll note | Mute, Split, Velocity…, Quantise, Delete |
| Spectral selection | Attenuate…, Delete, Heal, Copy, Apply Effect ▸ |

Drag and drop: audio files from SY02 onto a track (new clip) or the empty area (new track); pool item to a track; effect preset onto a track or clip; impulse file onto a convolution reverb; a clip out to SY02 exports the clip range as FLAC (offer includes WAV and FLAC); a MIDI file onto a MIDI track.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | "Record or open audio" with Record, Open, New from template and recent sessions (`gridView`). |
| Loading | Tracks draw at once; waveforms fill in as peaks are computed; playback is allowed before peaks finish. |
| Recording | Record button and armed track headers in `--danger` with the word "REC"; growing waveform; elapsed time and time left shown. |
| Engine stopped | Banner "Audio engine is not running" with the reason (device missing, permission denied) and "Open set-up". Editing still works. |
| Device lost | Recording is saved up to the last second; banner offers another device. |
| Error | Decode error marks the pool item `!` with the helper's message; missing files are listed for relink. |
| Offline (network) | No change; the app never uses the network. |
| Locked | Session lock stops monitoring and playback; a recording in progress continues only if `record.continue_when_locked` is on. |
| Read-only | Banner; edits disabled; Save As enabled. |
| Narrow (< 640 px) | Single-track waveform editor with transport; browser and inspector become overlays; mixer is a bottom sheet. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Workbench | `dockArea`, `dockWidget`, `tabWidget`, `splitter` |
| Arrangement | `timeline` after FUSE-WP-50 (today: tracks, clips, ruler, playhead, fixed-step snap, zoom; no waveforms, fades, lanes or headers; Verified `FUSE:Source/FUSE/UI/Widgets/include/fuse/ui/widgets/widgets.hpp:307-320`) |
| Waveform, spectrogram, meters | **Missing**: `waveformView`, `spectrogramView`, `levelMeter`, `spectrumBars` (FUSE-WP-49) |
| Automation, envelopes, EQ curve | `curveEditor` after FUSE-WP-50 (today one curve, three interpolation kinds) |
| Faders, pans | `slider`, `dial` |
| Browser, pool, markers, plugins | `treeView`, `tableView`, `listView` |
| Inspector, generic plugin panels | `propertyGrid` |
| Transport | `toolButton`, `label`, `spinBox`, `comboBox` |
| Transcript | `listView` with selectable text; word-level selection needs FUSE-WP-18 |
| Routing graph (optional view) | `nodeGraph` |

Gaps:

- **G-11**, audio half (FUSE-WP-52): a callback-driven output and input path with a known, small, fixed buffer and a sample clock the UI can follow.
- **CR03-GAP-1**: piano-roll grid widget (notes on a pitch/time grid with lanes). Could be a mode of the upgraded `timeline`; to be decided with FUSE-WP-50.
- **CR03-GAP-2**: real-time scheduling for a helper process. PLATFORM.md section 6 has no capability for it (see section 7 and 13).
- **CR03-GAP-3**: a MIDI device portal. PLATFORM.md lists `dev.usb:<class>`; MIDI needs a class-specific portal with hot-plug and timestamps.
- **CR03-GAP-4**: EQ curve widget with draggable band nodes over a live spectrum (can be built on `curveEditor` if it gains custom node shapes).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Session model, edit operations, undo journal, automation data, MIDI data, presets, jobs, intents |
| UI | FUSE UI (`.fuseui` + CSS) | app host | Workbench, docks, dialogs, effect panels |
| `audio-engine` | C++23 | helper, real-time thread | Device I/O, disk streaming, mixing graph, built-in DSP, instruments, delay compensation, metering, recording to disk, offline render. PLATFORM.md section 2 (b). |
| `audio-codec` | C++23 (wraps decoders and encoders) | helper, one per file or job, no capabilities | Decode and encode compressed formats; metadata parsing. Section 2 (a). |
| `plugin-host` | C++23 | helper, one per plugin instance, no capabilities | Loads one LV2 or CLAP plugin, runs its process call on shared buffers. Section 2 (a) and (d). |
| `analysis` | C++23 | helper, no capabilities | Peaks, spectrogram tiles, loudness scans, transient and tempo detection, stretch rendering (non-real-time work kept off the engine) |
| Model client | ForgeScript | app host | Sends audio to `svc.model` for stems, enhancement and transcription |

```
 +---------------------- app host (org.pbsd.audio-editor) -----------------------+
 | ForgeScript: session model, edits, journal, automation, MIDI                  |
 | FUSE UI: arrangement, mixer, editors  ◀── peaks, meter frames (shm ring)      |
 +-----┬--------------------┬------------------┬-----------------┬---------------+
       │ graph + edits      │ file handles     │ ranges          │ PCM
       ▼                    ▼                  ▼                 ▼
   audio-engine ◀──PCM── audio-codec       analysis          svc.model
   (RT thread, dev.mic,   (no caps)        (no caps)         (ai.local)
    svc.media, fs.app)
       ▲  shared audio buffers + event queues (lock-free)
       ▼
   plugin-host × N  (no capabilities; one plugin each)
```

Rules for the engine: the real-time thread never allocates, never takes a lock that another thread can hold for long, never does file or IPC calls; it reads commands from a lock-free queue and audio from pre-filled ring buffers. The UI learns positions and meter values from a shared-memory ring, never by round-trip calls. A plugin host that misses its deadline is bypassed for that buffer (its last output is faded) and counted.

### 6.2 Data model

Session folder:

```
<name>.asession/
  session.json           settings, tracks, clips, routing, automation, markers, schema version
  versions/              earlier session.json files
  audio/                 recorded and imported sources (WAV/BWF or FLAC)
  midi/<uuid>.mid        MIDI clips
  plugins/<uuid>.state   opaque plugin states (from the plugin's own state interface)
  peaks/                 peak and spectrogram caches (not backed up)
  transcripts/<uuid>.json
  autosave/              journal, in-progress recordings
```

| Entity | Fields |
|---|---|
| `Session` | `schema: u32`, `id: uuid`, `name`, `rate: u32`, `rec_depth: enum{i16,i24,f32}`, `tempo_map: [{pos: i64, bpm: f64, ramp: bool}]`, `signatures: [{bar: u32, num: u8, den: u8}]`, `tc_rate: rational`, `pan_law: f32`, `loudness_target: f32` |
| `Source` | `id: uuid`, `file: path or handle_token`, `blake3: hex`, `frames: u64`, `rate: u32`, `channels: u8`, `origin: enum{recorded,imported,rendered,model}`, `model: {name, hash}?` |
| `Track` | `id`, `kind: enum{audio,midi,instrument,bus,vca,master,folder,marker,picture}`, `name`, `channels`, `input: Port?`, `output: BusId`, `gain_db: f32`, `pan: f32`, `mute`, `solo`, `arm`, `monitor: enum`, `phase: bool`, `inserts: [Insert]`, `sends: [Send]`, `lanes: [Lane]`, `colour: u8`, `folder: TrackId?`, `frozen: SourceId?` |
| `Clip` | `id`, `track`, `source: SourceId or MidiId`, `pos: i64` (session samples), `len: i64`, `src_in: i64`, `gain_db: f32`, `fade_in: {len: i64, shape: enum}`, `fade_out`, `stretch: f64`, `pitch_cents: i32`, `algo: enum`, `warp: [{src: i64, dst: i64}]`, `env: [{t: i64, db: f32}]`, `reverse`, `mute`, `lock`, `group: uuid?`, `take: u16`, `name` |
| `Insert` | `id`, `kind: enum{builtin,script,lv2,clap}`, `type: string` (built-in name or plugin URI/id), `hash: hex?`, `params: map<string,f64>`, `state: path?`, `bypass`, `wet: f32`, `latency: u32` |
| `Send` | `to: BusId`, `level_db: f32`, `pan: f32`, `pre: bool` |
| `Lane` (automation) | `target: {insert?: id, param: string}`, `mode: enum`, `points: [{t: i64, v: f64, curve: f32}]` |
| `Marker` | `id`, `pos: i64`, `len: i64`, `name`, `kind: enum{marker,region,chapter,punch}`, `colour` |
| `MidiClip` | `id`, `file`, `ppq: u16`; notes as `{tick: u32, len: u32, key: u8, vel: u8, ch: u8}`; controllers as `{tick, cc, value}` |
| `Snapshot` | `name`, `mixer: json` (gains, pans, mutes, insert params) |

Positions are integer samples at the session rate; musical positions are ticks resolved through the tempo map. `session.json` is written with sorted keys and one entity per line so DV03 can diff it. Schema changes migrate forward in memory and keep the old file in `versions/`.

### 6.3 File formats and protocols

| Format | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| WAV, RF64, BWF (bext, iXML), AIFF | yes | yes | Own code | `audio-codec` (import); `audio-engine` writes its own recordings |
| FLAC | yes | yes | libFLAC (BSD) or own decoder (format is small and public) | `audio-codec` |
| Ogg Vorbis | yes | yes | libogg, libvorbis (BSD) | `audio-codec` |
| Opus | yes | yes | libopus (BSD) | `audio-codec` |
| MP3 | yes | yes | Decode: minimp3 (CC0) or own; encode: LAME (LGPL) as a separate optional helper build | `audio-codec` |
| AAC / M4A | yes | optional | Decode and encode through the platform media pipeline (G-11) if present; else off | `audio-codec` |
| WavPack | yes | yes | WavPack (BSD) | `audio-codec` |
| Standard MIDI File | yes | yes | Own code | app host (ForgeScript, bounded) |
| SFZ, SF2 | yes | no | Own code | `audio-codec` (parse to a checked instrument description) |
| LV2 (C ABI, Turtle manifests) | host | — | Own minimal host; own Turtle subset parser (no librdf stack) | `plugin-host` (throwaway instance for scanning) |
| CLAP (C ABI) | host | — | CLAP headers (MIT) | `plugin-host` |
| Cue sheet, chapters JSON, VTT, SRT | yes | yes | Own code | app host |
| Impulse responses (WAV, FLAC) | yes | no | As above | `audio-codec` |
| Engine IPC | — | — | Session-bus schema `org.pbsd.audio-editor.engine.v1` for control; shared-memory rings for audio and meters | both ends validate |

Resampling and time-stretch are own code (windowed-sinc resampler; phase vocoder and WSOLA). Third-party alternatives if own quality falls short: libsamplerate (BSD), Rubber Band (GPL-2+, compatible with AGPL-3.0-or-later). VST2, VST3 and AU are not supported: the first is closed to new licences, the second pulls a large SDK, the third is tied to another OS.

### 6.4 Integration

- **Intents handled:** `open`, `edit`, `new`, `share` target (receive audio). **Emitted:** `share`, `open` (ME02 plays an export; CR02 round trip).
- **MIME:** edit `application/x-pbsd-audio-session+json`, `audio/wav`, `audio/flac`, `audio/ogg`, `audio/opus`, `audio/aiff`; import `audio/mpeg`, `audio/aac`, `audio/mp4`, `audio/x-wavpack`, `audio/midi`; export the same audio types plus `text/vtt`.
- **Services:** `svc.portal.file`, `svc.portal.mic`, `svc.portal.usb` (MIDI), `svc.media` (devices, routing, transport keys, media session), `svc.settings`, `svc.notify`, `svc.power` (inhibit sleep while recording or exporting; performance profile request), `svc.model`, `svc.clipboard`, `svc.keyring`, `svc.backup` (excludes `peaks/`), `svc.intent`, `svc.a11y`.
- **Other apps:** ME06 "Open in Audio Editor"; CR02 sends clips and mixes and takes them back; ME02 plays exports; AX06 and ME06 share the transcription model class; AI02 installs models; DV03 versions sessions; SE04 holds signing keys.
- **Backing repos.** Verified: FORGE has signal-processing headers `forge/signal/audio.hpp` (at `FORGE:support/include/forge/signal/audio.hpp`) and `forge/signal/fft.hpp`, and a streaming inference session `FORGE:inference/include/forge/inference/streaming.hpp`; the sample `FORGE:examples/integrations/neuralvfx_stream.cpp` uses a biquad low-pass, a real FFT, and a session with `window`, `hop` and `deadline` options that reports frames, dropped, overruns and worst time. FUSE has a spatial audio library `fuse_audio` (`FUSE:docs/modules.md`), which is a game-audio engine and is not used here. Proposed: the audio engine may reuse FORGE's FFT and biquad code after review; everything else in this spec. Unverified: the full contents and quality of `forge/signal`; whether it is allocation-free and safe for a real-time thread.

## 7. Security and privacy

Capabilities requested:

| Capability | Reason |
|---|---|
| `fs.app` | Settings, presets, default session location, autosave |
| `fs.portal` | Sessions, imports, exports, plugin folders, impulse and sample files |
| `net.none` | No network use |
| `proc.helper:audio-engine`, `:audio-codec`, `:plugin-host`, `:analysis` | The helpers of 6.1 |
| `dev.mic` | Recording and monitoring; prompt on first use; indicator while armed |
| `dev.usb:midi` | MIDI keyboards and control surfaces through the device portal |
| `dev.gpu.compute` | Optional: spectrogram tiles on GPU |
| `ai.local`, `svc.model` | Stems, enhancement, transcription |
| `svc.media` | Audio devices and routing, media session |
| `svc.power` | Sleep inhibit, performance profile while the engine runs |
| `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.mic`, `svc.portal.usb`, `svc.clipboard`, `svc.keyring`, `svc.backup`, `svc.intent`, `svc.a11y` | As listed in 6.4 |
| `bg.run` | Keep recording or exporting when the window is closed; always shown by the shell |

Not requested: `net.host`, `net.user`, `proc.exec`, `dev.camera`, `dev.screen`, `dev.location`, `bg.autostart`.

**Decision on plugins, with reasons.** Arbitrary native plugins are off by default. When the user turns the host on, LV2 and CLAP plugins run one per process in `plugin-host` with an empty capability list.

1. A native plugin is unreviewed third-party machine code. In-process it would have the app's microphone grant and every open file. In a no-capability helper it can only compute on the buffers it is given.
2. One process per instance means a crash or hang costs one insert, not the session.
3. Plugin GUIs are not loaded. They need X11, GTK, Qt or a web view, none of which exist on this OS, and a window from an untrusted process would break the rule that prompts and chrome come from the shell. A generated parameter panel replaces them. Some plugins will be less pleasant to use; a few that work only through their GUI will not work at all.
4. Plugins that need writable-executable memory (their own JIT) fail under W^X. That is intended.
5. LV2 and CLAP are chosen because both are open (ISC and MIT), have a plain C ABI and do not need a vendor SDK.
6. The cost is latency and CPU: each hop to a helper adds a context switch per buffer. Budget in section 10. Plugins can be chained inside one host process only if the user marks them trusted (setting, off by default).
7. Signed ForgeScript effect graphs are the preferred way to extend the app: they add no machine code.

**What the OS must provide for low-latency audio (stated requirements, all Proposed or Unverified on HardenedBSD):**

| Need | Why | Status |
|---|---|---|
| Real-time scheduling class for the engine's audio thread and for plugin-host threads, granted by the system, bounded (CPU share cap, watchdog) | Without it, buffers under 256 frames under-run when the system is busy | Not in PLATFORM.md section 6: CR03-GAP-2. FreeBSD has `rtprio(2)`; a capability-mode process cannot raise itself, so the launcher or `svc.media` must set it. |
| Locked memory (`mlock`) for audio buffers and engine code, with a raised limit | No page faults in the audio thread | Needs a resource-limit grant in the helper launcher (FORGE-WP-27) |
| Direct, exclusive, bit-exact device path with a chosen buffer size and no mixing resampler in between | Predictable latency and no rate conversion | Through `svc.media`; Unverified. FreeBSD's sound(4) offers bit-perfect mode and small fragments; USB audio class support exists in the base system (Unverified on HardenedBSD here). |
| Device time stamps and a sample-accurate position call | Sync of record and playback, latency measurement | G-11 (FUSE-WP-52) |
| Low timer jitter; a way to ask for the performance CPU profile and to stop deep idle states while the engine runs | Wake-up latency adds to buffer time | `svc.power` request; Unverified |
| MIDI device access with time stamps and hot-plug | MIDI input timing | CR03-GAP-3 |
| Shared memory and wake-up primitives usable in capability mode between helpers (no syscall that can block for long in the audio path) | Engine to plugin-host hand-off | FORGE-WP-27, FORGE-WP-28; G-06 |
| Hardened-allocator and PaX settings that do not add faults in the audio thread | Real-time safety | Unverified; to be measured |

**Assets:** recordings (voices, unreleased music), the live microphone, session files, plugin states, signing keys.

**Adversaries:** crafted audio, MIDI, SFZ/SF2 or session files; a malicious or buggy plugin; a session from someone else that names plugins or scripts; another app trying to hear the microphone through this app; a hostile model.

| Surface | Mitigation |
|---|---|
| Decoders and metadata | Only in `audio-codec` with no capabilities; frame counts and channel counts checked before the engine accepts audio. |
| Plugins | Policy above; hash-pinned; no capabilities; time-outs; no GUI. |
| Sessions from elsewhere | Parsed in ForgeScript; files referenced by token and hash, not raw paths; plugins and scripts disabled until allowed (feature F133); plugin state blobs go only to the sandboxed plugin. |
| Microphone | Portal prompt; indicator while armed; arming is cleared on session close; no background capture without `bg.run` shown. |
| Real-time priority abuse | The grant is to the helper only, with a CPU cap and a watchdog that demotes a thread that runs past its budget. |
| Export metadata | Only what the user typed. Recording device names are not written. |
| Hearing damage | Output limiter option and jump warning (feature F134). |
| Models | Loaded by `svc.model` from signed `.fpk` files only. |

**Stored:** sessions where the user saves them; presets, device calibrations, plugin scan results in the app's data directory. **Leaves the machine:** nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `engine.output_device` | string | system default | Output device id |
| `engine.input_device` | string | none | Input device id |
| `engine.sample_rate` | int | `48000` | Engine rate when no session is open |
| `engine.buffer_frames` | int | `256` | Device buffer size |
| `engine.exclusive` | bool | `true` | Ask for an exclusive bit-exact device |
| `engine.realtime` | bool | `true` | Ask for real-time scheduling |
| `engine.performance_profile` | bool | `true` | Ask `svc.power` for the performance profile while running |
| `record.depth` | enum | `i24` | i16, i24, f32 |
| `record.file_format` | enum | `bwf` | bwf, flac |
| `record.pre_buffer_s` | int | `0` | Pre-record buffer length |
| `record.continue_when_locked` | bool | `true` | Keep recording when the session locks |
| `record.count_in_bars` | int | `1` | Count-in length |
| `edit.auto_crossfade_ms` | float | `5.0` | Crossfade on every cut; 0 = off |
| `edit.default_fade_shape` | enum | `equal_power` | Fade shape |
| `edit.snap_zero_crossing` | bool | `false` | Snap cuts to zero crossings |
| `edit.automation_follows_clips` | bool | `true` | Move automation with clips |
| `view.waveform_scale` | enum | `linear` | linear, db |
| `view.spectrogram_fft` | int | `4096` | Default window |
| `view.follow` | enum | `page` | none, page, centred |
| `meter.loudness_target` | float | `-16.0` | Default LUFS target |
| `meter.true_peak_ceiling` | float | `-1.0` | dBTP ceiling |
| `export.default_format` | enum | `flac` | Default export format |
| `export.write_report` | bool | `true` | Write a loudness report |
| `plugins.native_host` | bool | `false` | Enable LV2 and CLAP |
| `plugins.process_per` | enum | `instance` | instance, plugin |
| `plugins.folders` | list | empty | Portal grants for plugin folders |
| `plugins.deadline_factor` | float | `0.5` | Share of the buffer time a plugin may use |
| `safety.monitor_limiter` | bool | `true` | Limit monitor output to −1 dBFS |
| `ai.enabled` | bool | `false` | Show model tools |
| `scripts.allow_foreign` | bool | `false` | Run signed effect graphs from received sessions without asking |

## 9. Accessibility and languages

- **Editing by ear and keys.** Every selection change can be announced: start, end, length, track. `Ctrl+Alt+Left/Right` moves the edit cursor by the grid with an audible click or a short audition of the audio at the cursor.
- **Audible meters.** An optional tone or spoken read-out of peak level and loudness on demand (`Ctrl+Alt+L`), so meters do not need sight.
- **Mixer as a table.** The a11y tree presents the mixer as rows (tracks) and columns (gain, pan, mute, solo, inserts), each adjustable with arrow keys and typed values.
- **Spectral view** has a text summary of the selection (time range, frequency range, peak frequency) since the picture cannot be read aloud.
- **No colour-only state.** Mute, solo, arm and clip states use letters and shapes; clipping shows a number.
- **Screen-reader audio must not be recorded by accident.** The engine never captures system output unless the user granted system capture (feature F022).
- Note names follow the locale (C D E or Do Re Mi; B or H). Time and number formats follow the locale. Transcripts support any language the installed model supports and right-to-left text (G-03).

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | ≤ 400 ms |
| Engine start to audio running | ≤ 300 ms |
| Round-trip latency at 48 kHz, 64-frame buffer, class-compliant USB interface | ≤ 6 ms (target; depends on the device) |
| Under-runs | 0 in a 1-hour run at 128 frames with 32 tracks and 64 built-in inserts on the reference machine |
| Extra cost per sandboxed plugin instance | ≤ 30 µs per buffer hand-off on the reference machine; no added latency when it meets its deadline |
| Tracks | 128 audio tracks playing from disk at 48 kHz 24-bit |
| Edit operation applied and audible | ≤ 50 ms |
| Waveform draw | Any zoom in ≤ 4 ms per frame (peak pyramid) |
| Open a session with 10 hours of audio | ≤ 2 s to usable (peaks cached) |
| Offline export | ≥ 20× real time for a 16-track session with light processing |
| Spectrogram tile | ≤ 20 ms per 1024 × 256 tile on CPU |
| Memory | ≤ 600 MiB for app and helpers with a 32-track session, plus the disk stream buffers |
| Idle | 0 UI frames when stopped; the engine may be stopped entirely after 30 s idle (setting) |

## 11. Testing and proof

Nothing below has run. All are plans and targets.

- **Unit (ForgeScript):** edit operations with invariants (clips on a track do not overlap unless crossfaded, fades fit inside clips, automation points stay ordered, routing has no loops); tempo-map conversions; journal replay.
- **Unit (C++):** each DSP block against closed-form or reference responses (EQ magnitude, compressor static curve, limiter never exceeds its ceiling, resampler passband ripple and stopband); delay compensation sums; denormal handling.
- **Null tests:** import → export with no edits is bit-identical; a clip split and rejoined nulls against the original; bypassed inserts null.
- **Standards:** loudness and true peak against the EBU Tech 3341/3342 and ITU-R BS.1770 conformance signals.
- **Real-time safety:** a build that traps on allocation, lock or system call in the audio thread; a stress run with CPU and disk load counting under-runs.
- **Golden renders:** each dock, dialog and effect panel; waveform and spectrogram drawings from fixed signals.
- **Scripted UI through the a11y tree:** the 14 top tasks.
- **Fuzz:** each decoder, the WAV/BWF chunk parser, MIDI files, SFZ/SF2, session JSON, the LV2 manifest subset parser, plugin state blobs passed back to plugins (host side), the engine IPC.
- **Plugin host:** a set of deliberately bad test plugins (crash, hang, write out of bounds, try to open a file, try to open a socket) must each be contained and reported.
- **Recovery:** kill the engine and the app during recording; no more than 1 s of audio is lost.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Sample and tick arithmetic, tempo map | No overflow; conversions monotonic | T1 | PRISM proof | — |
| Lock-free queues and rings (engine ↔ UI, engine ↔ plugin host) | No data race; no out-of-bounds index | T2 | PRISM plus model checking of the algorithm | Concurrency proof of the full engine |
| Limiter | Output never exceeds the ceiling | T1 | Proof on the gain computation with look-ahead | Oversampled true-peak estimate stays T3 |
| DSP blocks | No runtime errors (index, division, NaN traps) | T2 | PRISM absence-of-runtime-error | Float accuracy is tested, not proved |
| WAV/BWF/MIDI parsers | No out-of-bounds read on any input | T2 | PRISM plus fuzzing | — |
| Third-party codecs | Contained | T4 | Sandbox tests | Not in the safe subset |
| Native plugins | Contained | T4 | Bad-plugin test set | Unreviewed machine code by definition |
| Edit operations (ForgeScript) | Invariants hold | T3 | Property tests | No ForgeScript prover yet |
| Real-time thread | No allocation, lock or blocking call | T3 | Trap build and static call-graph check | A proof of the whole call graph |

## 12. Build plan

### Phase 1 — usable core (record, edit, export)

| ID | Work item | Depends on |
|---|---|---|
| CR03-W01 | Session model, edit operations, journal, undo | G-02 |
| CR03-W02 | `audio-engine` v1: device I/O, disk streaming, mixing, fades, gain, pan, metering, recording with a journal | G-11 (FUSE-WP-52), FORGE-WP-27, FORGE-WP-28, CR03-GAP-2 |
| CR03-W03 | `audio-codec` helper: WAV, BWF, AIFF, FLAC, Vorbis, Opus, MP3 decode; FLAC, Opus, WAV encode | G-06 |
| CR03-W04 | `analysis` helper: peak pyramids | W03 |
| CR03-W05 | Workbench, transport, arrangement with waveforms, inspector | G-01, G-07, G-08, FUSE-WP-49, FUSE-WP-50 |
| CR03-W06 | Built-in effects v1: EQ, compressor, limiter, utility; effect chain and panels | W02 |
| CR03-W07 | Mixer v1: strips, master, peak/RMS meters | W02, W05 |
| CR03-W08 | Export mix, single-file mode, autosave and recovery | W02, W03 |
| CR03-W09 | Audio engine set-up panel and system checks | W02, G-09 (`svc.media`, `svc.power`) |
| CR03-W10 | Markers, loop, metronome, count-in | W02, W05 |

### Phase 2 — full feature

| ID | Work item | Depends on |
|---|---|---|
| CR03-W11 | Buses, sends, delay compensation, automation lanes and modes | W07 |
| CR03-W12 | Punch and loop recording, takes, comping, latency calibration | W02 |
| CR03-W13 | Time-stretch and pitch (vocoder, WSOLA), resampler quality pass, varispeed | W04 |
| CR03-W14 | Spectrogram view and spectral edits | W04, FUSE-WP-49 |
| CR03-W15 | Effects v2: de-noise, de-ess, gate, reverb, delay; offline apply; voice chain | W06 |
| CR03-W16 | Loudness (LUFS, true peak), targets, report, dither | W07 |
| CR03-W17 | MIDI tracks, piano roll, controller lanes, device input, SMF | CR03-GAP-1, CR03-GAP-3 |
| CR03-W18 | Strip silence, clip envelopes, groups, tempo track | W01 |
| CR03-W19 | Podcast workflow: template, ducking, chapters, metadata | W11, W16 |
| CR03-W20 | Stems export, batch convert, export queue | W08 |
| CR03-W21 | ForgeScript effect graphs, untrusted-session mode | FORGE-WP-14, FORGE-WP-33 |
| CR03-W22 | Screen-reader editing pass | G-04 |

### Phase 3 — advanced

| ID | Work item | Depends on |
|---|---|---|
| CR03-W23 | `plugin-host`: CLAP then LV2, scan, generic panels, trust list, bad-plugin tests | W11, G-06 |
| CR03-W24 | Instruments: sampler (SFZ, SF2), synth, drum machine; freeze | W17 |
| CR03-W25 | Stem separation, speech enhancement | G-15, FORGE-WP-29, FORGE-WP-31, AI02 |
| CR03-W26 | Transcription and text-based editing | W25, FUSE-WP-18 |
| CR03-W27 | Mastering chain, multiband, reference tracks, album assembly | W16 |
| CR03-W28 | Restoration: de-click, de-hum, de-clip; pitch correction; warp markers | W13, W14 |
| CR03-W29 | Surround buses and panner; routing matrix; VCAs | W11 |
| CR03-W30 | Control surfaces, pre-record buffer, system audio capture | W17, `svc.media` |
| CR03-W31 | Reference picture track; CR02 round trip; signed export; archive session | G-11 (FUSE-WP-53), CR02, SE04 |

## 13. Open questions and risks

1. **Real-time scheduling (CR03-GAP-2) is the gating risk.** PLATFORM.md has no capability for it. Proposal: a `sched.realtime` grant given only to named helpers through `svc.media`, with a CPU cap and watchdog. Without it, the app works only at large buffers (1024 frames or more) and is not fit for monitoring through effects.
2. **Audio stack on HardenedBSD is Unverified.** Bit-exact exclusive mode, USB audio class 2 devices, device time stamps and jitter under the hardened kernel all need measurement before the latency budget means anything.
3. **Plugin usefulness without GUIs.** Many plugins are usable from parameters alone; some are not. If users need plugin windows, the only route consistent with the platform is a plugin drawing into a buffer that the shell composites as an untrusted surface (FUSE-WP-51 remote surfaces). Not planned here.
4. **Per-plugin process cost.** A session with 100 plugin instances means 100 helper processes and 100 hand-offs per buffer. The trusted-chain option reduces this at a security cost. Needs measurement.
5. **MIDI portal (CR03-GAP-3)** does not exist in the platform list.
6. **Own DSP quality.** Time-stretch, de-noise and reverb take long to make good. Rubber Band (GPL-2+) is a fallback for stretching; owner decision on adding it.
7. **Patent-encumbered formats** (AAC; MP3 patents have expired) are optional and depend on the platform media pipeline.
8. **Models** for stem separation and transcription must exist as signed `.fpk` files with licences that allow shipping. None is named here.
9. **`forge/signal` reuse** is attractive but its real-time safety is Unverified.
10. **Priority.** The catalogue ranks this app P3. CR02 needs the same `audio-engine` core at P2, so phase 1 items W02–W04 should be scheduled with CR02, not left to P3.
