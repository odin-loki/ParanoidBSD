<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Music

By Odin Loch

| | |
|---|---|
| ID | `ME02` (from CATALOGUE.md) |
| Bundle | `org.pbsd.music` |
| Category | media |
| Priority | P1 |
| Backing repos | FUSE (`fuse_audio` game audio engine, vendored Vorbis and FLAC). No music player exists. |
| Status | Planned. Nothing built. |

## 1. Purpose

Music is the library and player for the user's own audio files. It is for listeners with local collections, for people who run their own streaming server at home, and for anyone who wants correct, gapless, bit-exact playback. It replaces Elisa, Strawberry and similar players.

## 2. Scope

In scope:

- Library of local files: tags, artwork, albums, artists, genres, composers.
- Playback: gapless, ReplayGain and EBU R128, crossfade, equaliser and DSP, bit-perfect output.
- Queue, playlists, smart playlists, lyrics from local files, mini player, visualisations.
- Tag editor, batch rename, CD ripping with verification.
- Self-hosted servers (Subsonic and Jellyfin APIs), DLNA/UPnP casting on the LAN, device sync.
- Scrobbling to self-hosted targets only.

Out of scope:

| Item | Owner |
|---|---|
| Podcast subscriptions and episode download | IN02 Feeds (plays through this app's player by intent) |
| Internet radio directory | IN02 Feeds keeps station feeds; Music plays a station URL handed over or typed |
| Video playback, music videos | ME03 Video |
| Recording | ME06 Sound Recorder |
| Waveform editing, mastering, MIDI | CR03 Audio Editor |
| Commercial streaming services, DRM | not planned |
| Audio device setup, system volume, routing policy | SY01 Settings, `svc.media` |
| Downloading music, torrents | IN03 Transfers |
| The decoders and audio output path | Media pipeline, ME03 section 6.5 |

## 3. Users and top tasks

Personas:

- **Listener**: plays albums and playlists, uses media keys.
- **Collector**: 60 000 tracks, careful tags, FLAC, wants bit-perfect output to a DAC.
- **Classical listener**: needs composer, work, movement, conductor, and true gapless.
- **Home-server user**: music on a NAS behind a Subsonic or Jellyfin server.
- **Ripper**: turns CDs into verified FLAC files.

| # | Task | Target steps |
|---|---|---|
| T1 | Play an album | 2 (find, double-click) |
| T2 | Pause, skip from any app | 1 media key |
| T3 | Add a track to the queue next | 1 (`Ctrl+Enter`) |
| T4 | Find a track by any tag | 1 search line |
| T5 | Make a playlist from a selection | 2 |
| T6 | Fix the tags of an album | 3 (select, `Ctrl+I`, edit, Save) |
| T7 | Rip a CD to FLAC with verification | 3 (insert, check tags, Rip) |
| T8 | Play from the home server | 2 after one-time setup |
| T9 | Cast to a LAN renderer | 2 |
| T10 | Switch to bit-perfect output on a DAC | 2 |
| T11 | Sync a playlist to a player device | 3 |
| T12 | Show lyrics | 1 key (`Ctrl+L`) |
| T13 | Open the mini player | 1 key (`Ctrl+M`) |
| T14 | Scan the library for loudness | 2 |

## 4. Features

### 4.1 Playback

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME02-F001 | Play, pause, stop | `Space`, media keys; stop after current (`Ctrl+Shift+.`). | M | 1 |
| ME02-F002 | Next, previous | `Ctrl+Right`/`Ctrl+Left`; previous restarts the track after 3 s. | M | 1 |
| ME02-F003 | Seek | Seek bar with optional waveform; `Left`/`Right` 5 s, `Shift` 30 s; sample-accurate for lossless. | M | 1 |
| ME02-F004 | Gapless | The next track is opened and decoded ahead; samples join with no padding. Encoder delay and padding honoured (LAME header, iTunSMPB, Opus pre-skip, Vorbis granule). | M | 1 |
| ME02-F005 | Crossfade | 0–12 s; curve (equal power, linear); "never within an album" on by default; separate manual-skip fade 0–2 s. | S | 2 |
| ME02-F006 | Fade on pause and stop | 0–500 ms. | C | 2 |
| ME02-F007 | ReplayGain | Read `REPLAYGAIN_*` tags and `R128_*` (Opus); modes off, track, album, auto (album when playing in album order); pre-amp ±15 dB; clip prevention from peak tags. | M | 1 |
| ME02-F008 | EBU R128 scan | Compute integrated loudness, range and true peak for tracks and albums in the decode helper; write tags or store in the database only. Target −18 LUFS (ReplayGain 2) or −23 LUFS. | S | 2 |
| ME02-F009 | Repeat and shuffle | Repeat off, track, queue; shuffle tracks or albums; shuffle order is stable until changed and avoids recent repeats. | M | 1 |
| ME02-F010 | Playback speed | 0.5×–2× with pitch correction (audiobooks, practice); pitch shift ±12 semitones. | C | 3 |
| ME02-F011 | A-B loop | For practice. | C | 3 |
| ME02-F012 | Resume | Remembers queue, track and position across restarts. Long tracks (over 20 min) remember their own position. | M | 1 |
| ME02-F013 | Sleep timer | After n minutes, at end of track, at end of album; fade out 10 s. | S | 2 |
| ME02-F014 | Stop on output loss | Pauses when headphones or the output device disappear (signal from `svc.media`). | M | 1 |
| ME02-F015 | Volume | App volume 0–100% with a logarithmic curve; fixed at 100% and greyed in bit-perfect mode. | M | 1 |
| ME02-F016 | Sample-rate handling | Shared mode: resample to the mixer rate with the pipeline resampler. Exclusive mode: switch the device rate per track. | M | 2 |
| ME02-F017 | Play counts and skips | Count a play at 50% or 4 min; store last played, skip count. Local only. | S | 1 |
| ME02-F018 | Preload to memory | Optionally read the whole next track into memory for network volumes. | C | 2 |

### 4.2 Formats

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME02-F019 | FLAC | Native and Ogg FLAC, up to 32-bit and 384 kHz, embedded cue sheets, seek tables. | M | 1 |
| ME02-F020 | Opus, Vorbis | Ogg containers, chained streams, multichannel. | M | 1 |
| ME02-F021 | MP3 | MPEG-1/2/2.5 Layer III, VBR headers (Xing, VBRI), gapless info. | M | 1 |
| ME02-F022 | AAC | LC, HE-AAC v1 and v2 in MP4 and ADTS; needs the restricted pack (ME03 section 6.5.3). | S | 2 |
| ME02-F023 | ALAC | In MP4 and CAF. | S | 2 |
| ME02-F024 | WAV, AIFF, RF64, BWF | PCM 8–32-bit, float, ADPCM; `bext` and `cue ` chunks read. | M | 1 |
| ME02-F025 | WavPack, Monkey's Audio, TTA, Musepack | Lossless and legacy. | C | 2 |
| ME02-F026 | DSD | DSF and DFF: converted to PCM (352.8 kHz then resampled) or sent as DoP in exclusive mode to DACs that accept it. | C | 3 |
| ME02-F027 | Tracker modules | MOD, XM, S3M, IT through an own player. | C | 3 |
| ME02-F028 | Cue sheets | `.cue` beside a single-file album or embedded: tracks appear as separate items with gapless joins. | S | 2 |
| ME02-F029 | Unsupported files | Listed with the reason and the pack that would play them. | M | 1 |

### 4.3 Library

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME02-F030 | Library folders | Granted through the file portal; scanned by the index worker; file events keep the index current. | M | 1 |
| ME02-F031 | Tag reading | ID3v1, ID3v2.2/2.3/2.4, Vorbis comments, MP4 atoms, APEv2, RIFF INFO, AIFF ID3, Matroska tags. Parsed in the tag helper. | M | 1 |
| ME02-F032 | Multi-value tags | Multiple artists, genres, composers kept as lists; separators configurable for legacy tags. | M | 1 |
| ME02-F033 | Views | Albums, Artists (album artist), Tracks, Genres, Composers, Works, Years, Folders, Recently added, Recently played, Most played, Never played. | M | 1 |
| ME02-F034 | Album grouping | By album artist + album + disc; compilations handled; MusicBrainz release id used when present to split same-named albums. | M | 1 |
| ME02-F035 | Classical fields | Work, movement number and name, composer, conductor, orchestra, soloists; "Works" view groups movements. | S | 2 |
| ME02-F036 | Sort tags | Honours sort-name tags; article rules per language ("The", "Die", "Les"). | S | 1 |
| ME02-F037 | Artwork | Embedded pictures, `cover.jpg`/`folder.jpg`/`front.png` in the folder. Decoded in the image codec helper (ME01). No online lookups. | M | 1 |
| ME02-F038 | Artwork editing | Add from file, paste, remove, choose which embedded picture is the front; resize on embed. | S | 2 |
| ME02-F039 | Ratings and favourites | 0–5 stars and a heart; optional write to `POPM`/`RATING` tags. | S | 1 |
| ME02-F040 | Search | Instant search over all text tags, lyrics and file names; tokens `artist:`, `album:`, `genre:`, `year:1970..1979`, `rating:>=4`, `format:flac`, `bitdepth:24`. | M | 1 |
| ME02-F041 | Column browser | Genre ▸ artist ▸ album columns above the track table. | S | 2 |
| ME02-F042 | Track table | Columns for every tag and technical field; column chooser; multi-sort; group by album with art. | M | 1 |
| ME02-F043 | Missing and moved files | Greyed; relink by audio hash (hash of decoded PCM header frames, independent of tags). | S | 2 |
| ME02-F044 | Duplicate tracks | By audio hash and by acoustic similarity (local fingerprint, own code); keep rules. | C | 3 |
| ME02-F045 | Library statistics | Counts, total time, formats, sample rates, loudness distribution. | C | 2 |
| ME02-F046 | Offline volumes | Entries stay; marked with the volume name. | S | 2 |
| ME02-F047 | System search | Contributes artist, album and track names to `svc.search`. | S | 2 |
| ME02-F048 | Local recommendations | "More like this" from tag similarity and optional audio embeddings through `svc.model`. Never uses a network service. | C | 3 |

### 4.4 Queue and playlists

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME02-F049 | Queue | Visible list of what plays next; reorder by drag; remove; clear; "play next" and "add to end". History above the current row. | M | 1 |
| ME02-F050 | Queue vs source | A temporary queue sits in front of the playing source (album or playlist); when it empties, the source continues. | M | 1 |
| ME02-F051 | Save queue as playlist | One action. | S | 1 |
| ME02-F052 | Playlists | Manual playlists with order, description, cover collage; folders of playlists. | M | 1 |
| ME02-F053 | Smart playlists | Rule editor over any field with AND/OR groups, limit by count, time or size, sort or random, live update. | M | 2 |
| ME02-F054 | Playlist files | Import and export M3U, M3U8, PLS, XSPF, CUE; relative or absolute paths; optional "keep in sync with file" for a playlist folder. | M | 2 |
| ME02-F055 | Auto playlists | Built in: Favourites, Recently added, Most played, Not played in a year. | S | 1 |
| ME02-F056 | Radio from a seed | Endless queue from a track, album or genre using local similarity. | C | 3 |
| ME02-F057 | Undo | Queue and playlist edits are undoable. | M | 1 |

### 4.5 Output and DSP

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME02-F058 | Output device | Follows the system default or a pinned device from `svc.media`. | M | 1 |
| ME02-F059 | Bit-perfect mode | Exclusive access through `svc.media`: no mixing, no resampling, no volume, no DSP; device rate and bit depth follow the track. A badge shows "bit-perfect 96 kHz / 24-bit" only when the service confirms the device format equals the source format. | M | 2 |
| ME02-F060 | Signal path view | Popover listing every stage from file to device with format at each stage; lossy or resampling stages are marked. | S | 2 |
| ME02-F061 | Equaliser | 10-band graphic and 5-band parametric (frequency, gain, Q, type); presets; per-output-device preset; AutoEQ-style text preset import. | M | 2 |
| ME02-F062 | Pre-amp and limiter | Pre-amp ±12 dB; true-peak limiter to prevent clipping after EQ and ReplayGain. | M | 2 |
| ME02-F063 | Crossfeed | For headphones, three strengths. | C | 3 |
| ME02-F064 | Channel tools | Mono downmix, swap, balance, stereo width. | S | 2 |
| ME02-F065 | Dither | TPDF dither when reducing bit depth; noise shaping optional. | S | 2 |
| ME02-F066 | Resampler quality | Fast, balanced, best (band-limited sinc lengths). | S | 2 |
| ME02-F067 | Convolution | Load an impulse response (WAV) for room or headphone correction. The convolution code path exists in FUSE's engine (`FUSE:Source/FUSE/Audio/include/fuse/audio/conv_reverb_cpu.hpp`); reuse is Proposed. | C | 3 |
| ME02-F068 | Multichannel | Plays 5.1/7.1 files to matching devices; downmix matrix otherwise. | S | 2 |
| ME02-F069 | Passthrough | DTS-CD and AC-3 in WAV detected and offered as IEC 61937 passthrough. | C | 3 |
| ME02-F070 | Per-device profiles | DSP chain stored per output device id. | S | 2 |

### 4.6 Lyrics, visualisation, mini player

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME02-F071 | Lyrics from tags | `USLT`, `SYLT`, `LYRICS`, `©lyr`. | M | 2 |
| ME02-F072 | Lyrics from files | `.lrc` (simple and enhanced word-timed), `.txt`, `.srt` beside the track or in a lyrics folder. | M | 2 |
| ME02-F073 | Synced display | Current line highlighted, click a line to seek, offset adjust ±, auto-scroll. | S | 2 |
| ME02-F074 | Lyrics editor | Edit text; tap-to-time (`T` stamps the current line); save to `.lrc` or tag. | C | 3 |
| ME02-F075 | No online lyrics | The app never fetches lyrics. | M | 2 |
| ME02-F076 | Local lyric transcription | Optional speech-to-text through `svc.model` produces a draft `.lrc` marked machine-made. | C | 3 |
| ME02-F077 | Mini player | 360×96 always-on-top window (SH05): art, title, seek, previous, play, next, volume. `Ctrl+M`. | M | 1 |
| ME02-F078 | Now-playing view | Large art, title, queue peek, lyrics, signal path. | S | 1 |
| ME02-F079 | Visualisations | Spectrum bars, spectrogram, oscilloscope, stereo vectorscope, level and loudness meters. Drawn at the display rate only while visible; off with reduced motion. | C | 2 |
| ME02-F080 | Waveform seek bar | Precomputed peak files in cache. | C | 2 |
| ME02-F081 | Notifications | Track-change toast through `svc.notify`, off by default; never while full-screen apps are active. | S | 1 |

### 4.7 Tag editor and file tools

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME02-F082 | Tag editor | Edit one or many tracks: common fields plus a raw table of every frame. Mixed values shown as "(multiple)". | M | 2 |
| ME02-F083 | Safe writes | Tag writer helper writes a temp file, verifies the audio hash is unchanged, then renames atomically. Undo restores previous tags from a journal. | M | 2 |
| ME02-F084 | Tag from file name | Patterns such as `%track% - %title%`; preview table. | S | 2 |
| ME02-F085 | Batch rename and move | Templates such as `{albumartist}/{year} - {album}/{disc}{track:02} {title}`; illegal-character policy; collision preview; undo. | M | 2 |
| ME02-F086 | Auto-number, case tools, find and replace | Across a selection, with regular expressions (bounded engine, FORGE-WP-23). | S | 2 |
| ME02-F087 | Tag version control | ID3 version and encoding (v2.4 UTF-8 default, v2.3 option); remove ID3v1; remove APE from MP3. | S | 2 |
| ME02-F088 | Offline metadata match | Match albums against a local MusicBrainz data dump if the user installed one; no network lookups. | C | 3 |
| ME02-F089 | Convert | Transcode a selection to FLAC, Opus, Vorbis, WAV (base pack encoders) with tags and art; never lossy-to-lossy without a warning. | S | 2 |
| ME02-F090 | Verify files | Check FLAC MD5, decode errors, truncated files; report. | S | 2 |
| ME02-F091 | Strip metadata on share | Share sheet offers "remove personal tags" (ratings, play counts, comments, encoder paths). | S | 2 |

### 4.8 CD ripping

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME02-F092 | Disc detection | Through the optical drive portal (ME03-GAP-3); read-only device handle passed to the rip helper. | S | 3 |
| ME02-F093 | Table of contents | TOC, CD-Text, ISRC, pre-gaps, hidden track one audio, data tracks ignored. | S | 3 |
| ME02-F094 | Secure read | Overlapping re-reads, C2 error pointers where the drive offers them, drive read offset correction, cache defeat. | M | 3 |
| ME02-F095 | Verification | Each track's CRC compared with a local AccurateRip-style or CTDB-style database if the user has installed an offline copy; otherwise two independent passes must match. The log states which method was used. | M | 3 |
| ME02-F096 | Drive offset | Offset entered by the user or detected against a verified disc; stored per drive model. | S | 3 |
| ME02-F097 | Metadata | From CD-Text, from a local MusicBrainz dump by disc id, or typed. No network lookup. | S | 3 |
| ME02-F098 | Output | FLAC by default; Opus, Vorbis, WAV; single file + cue or per-track; ReplayGain scan after rip. | M | 3 |
| ME02-F099 | Rip log | Text log with read errors, retries, CRCs, drive and offset; signed with BLAKE3 checksum line. | S | 3 |
| ME02-F100 | Play CD | Play a disc directly, gapless, without ripping. | S | 3 |
| ME02-F101 | Pre-emphasis | Detected from sub-channel flags; de-emphasis applied or flagged in tags. | C | 3 |

### 4.9 Streams, servers, casting, sync

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME02-F102 | Internet radio | Play a station URL (HTTP/HTTPS Icecast/Shoutcast, HLS audio); ICY metadata shown; stations saved locally; import M3U/PLS. Needs `net.user`; first contact per host is prompted. | S | 2 |
| ME02-F103 | Radio hand-off | IN02 sends stations and podcast episodes by intent `open`; Music plays them and reports position back to IN02 for episodes. | S | 2 |
| ME02-F104 | Subsonic API | Server added in Preferences ▸ Servers: URL, user, token auth (salted) or API key; browse, search, play, playlists, stars, cover art. Each server needs its host granted (`net.user`). | S | 2 |
| ME02-F105 | Jellyfin API | Same functions through the Jellyfin HTTP API with a device token stored in `svc.keyring`. | S | 2 |
| ME02-F106 | Server library in the sidebar | Each server is a separate source with its own views; never merged silently with local files. | S | 2 |
| ME02-F107 | Offline cache | Pin albums or playlists from a server for offline play; size limit; original or transcoded as the server offers. | S | 3 |
| ME02-F108 | TLS rules | HTTPS required by default; plain HTTP only for private address ranges after a warning; certificate pinning on first use with a visible fingerprint. | M | 2 |
| ME02-F109 | Route | Per server: clear, VPN, Tor within `net.route` grants. | C | 3 |
| ME02-F110 | DLNA/UPnP casting | Discover renderers on the LAN while the cast popover is open; send the stream from a one-time listener; control play, seek, volume (ME03-GAP-4). | S | 3 |
| ME02-F111 | UPnP media servers | Browse and play from DLNA media servers on the LAN. | C | 3 |
| ME02-F112 | Scrobbling to self-hosted targets | ListenBrainz-compatible API and Subsonic `scrobble` to a host the user enters. No built-in public endpoints. Offline queue; every submission is logged in a viewable list. Off by default. | C | 3 |
| ME02-F113 | Device sync | Copy playlists, albums or smart selections to a mounted player, phone (MTP through the USB portal) or SD card; optional transcode; free-space plan; remove what is no longer selected; M3U written for the device. | S | 3 |
| ME02-F114 | Sync verification | BLAKE3 check after copy; resumable. | S | 3 |

### 4.10 System integration, security, recovery

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME02-F115 | Media session | Publishes metadata, art, position and actions to `svc.media`; media keys, lock screen (SH03) and quick settings (SH02) controls work. | M | 1 |
| ME02-F116 | Background play | Keeps playing with the window closed (`bg.run`); a shell indicator shows it; quitting is one click. | M | 1 |
| ME02-F117 | Sleep inhibit | Does not inhibit screen sleep; inhibits system sleep while playing (`svc.power`), configurable. | S | 1 |
| ME02-F118 | Decode in zero-capability helpers | Every audio file, tag block, cue sheet, playlist and picture is parsed in a helper with no capabilities. | M | 1 |
| ME02-F119 | Network honesty | `[net]` badge lists every endpoint; a library with no servers and no radio never opens a socket. | M | 1 |
| ME02-F120 | Credentials | Server passwords and tokens only in `svc.keyring`. | M | 2 |
| ME02-F121 | Bus interface | `org.pbsd.music.Player1`: play, pause, next, queue handles, get state; per-caller approval. | S | 2 |
| ME02-F122 | Command line | `music [--play\|--pause\|--next\|--enqueue] <handles…>` routed to the running instance. | S | 1 |
| ME02-F123 | Open with | Opening a file from Files plays it without adding it to the library; a bar offers "Add folder to library". | M | 1 |
| ME02-F124 | Database recovery | Integrity check on start; automatic restore from the last good copy; play counts journalled. | M | 1 |
| ME02-F125 | Export library data | Ratings, play counts and playlists as JSON and M3U for migration. | S | 2 |
| ME02-F126 | Import from other players | Read playlists and statistics from exported XML/JSON files of common players (file import only). | C | 3 |
| ME02-F127 | Private listening | No play counts, no history, no scrobbles while on. | S | 2 |

## 5. GUI

### 5.1 Window map

```
Music
├─ Main window (single instance)
│  ├─ Sidebar: Now playing · Albums · Artists · Tracks · Genres · Composers · Folders ·
│  │           Playlists ▸ · Smart ▸ · Radio · Servers ▸ · Devices ▸ · CD
│  ├─ Content: album grid | artist page | track table | column browser | now playing
│  ├─ Queue panel (right, F4)
│  ├─ Player bar (bottom)
│  └─ Popovers: volume, output device and signal path, cast, sleep timer
├─ Mini player
├─ Dialogs and sheets
│  ├─ Tag editor
│  ├─ Batch rename
│  ├─ Smart playlist rules
│  ├─ Equaliser and DSP
│  ├─ Rip CD
│  ├─ Add server
│  ├─ Sync device
│  ├─ Convert
│  ├─ Loudness scan
│  └─ Lyrics editor
├─ Preferences: General · Library · Playback · Output · DSP · Lyrics · Servers ·
│               Radio · Ripping · Scrobbling · Privacy · Shortcuts
└─ Shell-drawn: file portal, USB portal, capability prompts, share sheet
```

### 5.2 Main window

Shown at 1200×720, wide breakpoint, album view, queue open.

```
+------------------------------------------------------------------------------------------+
| [=] Music                         [search: artist:coltrane year:1957..1965]  [net][▁][▢][✕]|(1)
+--------------+-----------------------------------------------------+---------------------+
| Now playing  | Albums   Sort [Artist v]  [Grid|Table]  size ─●─    | Queue          (5)  |
| Albums    (2)+-----------------------------------------------------+ history             |
| Artists      | +-------+ +-------+ +-------+ +-------+ +-------+   |   Naima             |
| Tracks       | | art   | | art   | | art   | | art   | | art   |   | > Giant Steps  4:43 |
| Genres       | +-------+ +-------+ +-------+ +-------+ +-------+   | next                |
| Composers    | Blue Train Giant St. A Love S. Ballads  Crescent    |   Cousin Mary  5:45 |
| Folders      | 1957 FLAC  1960 24/96 1965      1963     1964   (3) |   Countdown    2:21 |
|--------------| +-------+ +-------+ +-------+ +-------+ +-------+   |   Spiral       5:56 |
| ▾ Playlists  | | art   | | art   | | art   | | art   | | art   |   | then: album continues|
|   Evening    | +-------+ +-------+ +-------+ +-------+ +-------+   |                     |
| ▸ Smart      |                                                     | [Clear] [Save as…]  |
| Radio        |                                                     |                     |
| ▸ Servers    |                                                     |                     |
| CD           |                                                 (4) |                     |
+--------------+-----------------------------------------------------+---------------------+
| [art] Giant Steps — John Coltrane        [|<] [ ▶ ] [>|]  [⇄] [↻]   [🔊 ===-] [out] [☰] |(6)
|       1:12 [=======|--------------------------------] 4:43   FLAC 96/24 · bit-perfect ✓ |(7)
+------------------------------------------------------------------------------------------+
```

1. Header with search (`lineEdit`). `[net]` appears only while a radio, server or cast endpoint is open.
2. Sidebar `treeView`; drop tracks on a playlist to add.
3. Album grid `gridView`; each cell: art, album, artist, year, format badge. `Enter` opens the album page; double-click plays.
4. Content area switches to `tableView` (tracks), artist page, column browser.
5. Queue panel 280 px, `listView`: history, current, next, then the source.
6. Player bar: art (click opens Now playing), title, transport `toolButton`s, shuffle, repeat, volume `slider`, output popover, queue toggle.
7. Seek `slider` with times; format line; the bit-perfect badge is `--ok` with a tick only when confirmed by `svc.media`, otherwise the words "resampled to 48 kHz" in `--fg-1`.

States: empty library ("Add a music folder"), scanning (status line with count and pause), error (unplayable track marked with a reason; playback moves on), offline (servers greyed, cached items playable), read-only (tag editing disabled on read-only volumes).

### 5.3 Other views and dialogs

**Album page**

```
+------------------------------------------------------------------------------------+
| [←]  +---------+  Giant Steps                                   [Play] [Shuffle]   |
|      |  art    |  John Coltrane · 1960 · Jazz · 7 tracks · 37:03                   |
|      |         |  FLAC 96 kHz / 24-bit · album gain −6.2 dB · DR 11                |
|      +---------+  ★★★★★  [♥]  [Add to playlist v] [Edit tags Ctrl+I] […]           |
|  #  Title                         Artist            Time   Plays  Rating           |
|  1  Giant Steps                   John Coltrane     4:43   12     ★★★★★            |
|  2  Cousin Mary                   John Coltrane     5:45    9     ★★★★             |
|  …                                                                                 |
+------------------------------------------------------------------------------------+
```

**Tag editor** (dialog, 860×600)

```
+----------------------------------------------------------------------------------+
| Edit tags — 7 tracks                               [Common] [All fields] [Art]   |
| Title        [(multiple)_______________]   Track [(multiple)] of [7]             |
| Artist       [John Coltrane____________]   Disc  [1] of [1]                      |
| Album artist [John Coltrane____________]   Year  [1960]   Genre [Jazz v]         |
| Album        [Giant Steps______________]   Composer [(multiple)______]           |
| Comment      [_________________________]   [ ] Compilation                       |
| +--------+ Front cover 1400×1400 JPEG  [Replace…] [Remove] [Paste]               |
| |  art   |                                                                       |
| +--------+ Tools: [From file name…] [Auto-number] [Case v] [Find/replace…]       |
| Writes ID3v2.4 UTF-8 / Vorbis comments. Audio data is verified unchanged.        |
|                                    [Cancel]  [Save 7 files]                      |
+----------------------------------------------------------------------------------+
```

"All fields" is a `tableView` of raw frames. Error state: file read-only or on an offline volume (row marked, others still saved).

**Equaliser and DSP** (dialog, 760×460)

```
+--------------------------------------------------------------------------+
| DSP for "USB DAC"        [x] Enabled     Preset [Flat v] [Save…] [Import…]|
| Pre-amp ──●── −3.0 dB    [x] Limiter     Mode (•) Graphic ( ) Parametric  |
| +12 |  |   |   |   |   |   |   |   |   |   |                              |
|   0 |--●---●---●---●---●---●---●---●---●---●--   response curve           |
| −12 |  |   |   |   |   |   |   |   |   |   |                              |
|      31  62 125 250 500 1k  2k  4k  8k 16k                                |
| Chain: ReplayGain → EQ → Crossfeed [ ] → Limiter → Dither [TPDF v]        |
| Bit-perfect mode turns this whole chain off.            [Close]           |
+--------------------------------------------------------------------------+
```

**Rip CD** (sheet, 900×600)

```
+----------------------------------------------------------------------------------+
| Rip CD — drive "ASUS DRW-24" offset +6 samples (stored)      [Detect offset…]    |
| Album [Giant Steps____]  Artist [John Coltrane___]  Year [1960]  Source: CD-Text |
|  # [x] Title                 Length  Status                     CRC      Match    |
|  1 [x] Giant Steps           4:43    done, 0 re-reads           A1B2C3D4 2 passes |
|  2 [x] Cousin Mary           5:45    reading 43%  (2 re-reads)                    |
|  3 [x] Countdown             2:21    waiting                                      |
| Format [FLAC level 5 v]  To [Music/{albumartist}/{album}] [x] Log [x] ReplayGain  |
| Verification: two matching passes (no offline database installed)                 |
|                                              [Cancel]  [Rip 7 tracks]             |
+----------------------------------------------------------------------------------+
```

**Smart playlist rules** (dialog, 640×420)

```
+------------------------------------------------------------------+
| Smart playlist [Unplayed jazz________]                           |
| Match (•) all ( ) any of:                                        |
|  [Genre v]      [is v]            [Jazz______]            [−]    |
|  [Play count v] [is v]            [0]                     [−]    |
|  [Rating v]     [at least v]      [★★★]                   [−] [+]|
| Limit [x] to [50] [tracks v] chosen by [random v]                |
| [x] Update live          214 tracks match                        |
|                                   [Cancel]  [Save playlist]      |
+------------------------------------------------------------------+
```

**Add server** (dialog, 560×380)

```
+--------------------------------------------------------------+
| Add a music server                                           |
| Type     (•) Subsonic API   ( ) Jellyfin                     |
| Address  [https://music.home.example:4533_______________]    |
| User     [odin______]   Password [••••••••] (to the keyring) |
| Route    [Clear v]                                           |
| Certificate: not yet seen. Fingerprint shown on first contact|
| This lets Music contact music.home.example:4533 only.        |
|                               [Cancel]  [Test and add]       |
+--------------------------------------------------------------+
```

**Output popover with signal path**

```
+----------------------------------------------+
| Output  (•) System default  ( ) USB DAC      |
| [x] Bit-perfect (exclusive) on USB DAC       |
| Signal path                                  |
|  File     FLAC 96 kHz / 24-bit stereo        |
|  Decode   lossless                           |
|  DSP      off (bit-perfect)                  |
|  Device   96 kHz / 24-bit   ✓ identical      |
+----------------------------------------------+
```

**Mini player** (360×96)

```
+------------------------------------------------+
| [art] Giant Steps — John Coltrane        [▢][✕]|
|       [|<] [▶] [>|]  1:12 [====|------] 4:43 🔊|
+------------------------------------------------+
```

**Sync device** (sheet): left `treeView` with checkboxes (playlists, artists, albums), right a plan (`tableView`: add 214 files 6.2 GB, remove 12, free after 1.1 GB), options (transcode lossless to Opus 128 kbit/s, write M3U, verify), `[Sync]`.

### 5.4 Menus

```
File     Add folder to library… · Open file… Ctrl+O · Open location… Ctrl+U · New playlist Ctrl+N ·
         New smart playlist… Ctrl+Shift+N · Import playlist… · Export playlist… · Rip CD… ·
         Sync device… · Convert… · Share… · Close window Ctrl+W · Quit Ctrl+Q
Edit     Undo Ctrl+Z · Redo Ctrl+Shift+Z · Copy Ctrl+C · Select all Ctrl+A · Find Ctrl+F ·
         Edit tags… Ctrl+I · Rename files… Ctrl+R · Rating ▸ Ctrl+0–5 · Favourite Ctrl+D ·
         Preferences Ctrl+,
Playback Play/Pause Space · Stop · Stop after current Ctrl+Shift+. · Next Ctrl+Right ·
         Previous Ctrl+Left · Seek ▸ · Shuffle Ctrl+Shift+S · Repeat ▸ Ctrl+Shift+R ·
         Play next Ctrl+Enter · Add to queue Shift+Enter · Sleep timer ▸ · Private listening
Audio    Output device ▸ · Bit-perfect mode · Equaliser and DSP… Ctrl+E · ReplayGain ▸ ·
         Crossfade ▸ · Scan loudness… · Signal path · Cast…
View     Now playing Ctrl+1 · Albums Ctrl+2 · Artists Ctrl+3 · Tracks Ctrl+4 · Genres Ctrl+5 ·
         Queue F4 · Sidebar F9 · Lyrics Ctrl+L · Visualisation Ctrl+Shift+V · Mini player Ctrl+M ·
         Column browser · Columns… · Full screen F11
Library  Rescan F5 · Find duplicates… · Verify files… · Relink missing… · Statistics ·
         Export library data…
Help     Help F1 · Supported formats · Shortcuts · About
```

### 5.5 Keyboard and pointer

Extra keys: `J` jump to the playing track, `Ctrl+J` jump to its album, `Ctrl+Up`/`Down` volume, `Alt+Up`/`Down` move queue row, `Delete` removes from queue or playlist (never the file; `Shift+Delete` trashes the file after confirmation), `T` tap-time in the lyrics editor.

| Context | Menu items |
|---|---|
| Track | Play · Play next · Add to queue · Add to playlist ▸ · Go to album / artist · Edit tags… · Rating ▸ · Show in Files · Convert… · Share… · Properties · Remove from library · Move to trash |
| Album | Play · Shuffle · Play next · Add to playlist ▸ · Edit tags… · Scan loudness · Pin for offline (server) · Sync to device ▸ |
| Queue row | Play now · Move to top · Remove · Remove all above / below |
| Playlist | Play · Rename F2 · Duplicate · Export… · Delete |
| Art | Copy image · Replace… · Save as… |

Drag and drop: files and folders in (play, or add to a playlist when dropped there); tracks to queue, playlists, devices; rows out as file handles and an M3U text offer. Pointer: wheel over the volume changes volume; wheel over the seek bar seeks 5 s; middle click on a track queues it next.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | "Add a music folder" plus "Open a file". |
| Loading | Skeleton grid; scan progress in the status line; playback available at once for scanned items. |
| Error | Unplayable track: row icon and reason; player skips after 1 s. Output device lost: paused with a bar "Output disconnected". |
| Offline | Server sources greyed with "offline"; pinned items playable; radio shows "no route". |
| Locked | Music on a locked Vault volume is greyed with a lock. |
| Read-only | Tag edits and renames disabled with a tooltip. |
| Narrow (<640) | Sidebar overlay, queue as a sheet, player bar collapses to art, title, play, next. |
| Bit-perfect unavailable | Toggle greyed with the reason ("device is shared by another app"). |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar | `treeView` |
| Album and artist grids | `gridView`, `image` |
| Track table, raw tags, rip list, sync plan | `tableView` over `SqliteModel` / `ui::ItemModel` |
| Queue | `listView` |
| Player bar | `toolButton`, `slider`, `label`, `image` |
| Equaliser | `slider` (vertical), `chart` (response), `comboBox`, `dial` (Q) |
| Rule editor | `comboBox`, `lineEdit`, `spinBox`, `dateTimeEdit` |
| Visualisations | `chart` or `viewportWidget` painting an image |
| Dialogs | `dialog`, `tabWidget`, `progressBar`, `checkBox`, `radioButton` |

Missing: audio output and input for apps (G-11, FUSE-R-088). `ME02-GAP-1`: `svc.media` exclusive (bit-perfect) device mode and format report. Optical drive portal: ME03-GAP-3. LAN discovery capability: ME03-GAP-4. Vertical `slider` and a waveform seek bar need custom paint only.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Library views, queue, playlists, rules, server clients' logic, sync planner |
| UI | FUSE UI | app host | Section 5 |
| Media session client and DSP chain | C++23 (platform, ME03 section 6.5) | app host audio thread | Gapless join, crossfade, ReplayGain, EQ, limiter, resampler, dither; PCM ring to `svc.media` |
| `demux-helper`, `decode-helper` | C++23 plus vendored codecs | helpers, no capabilities | Shared with ME03 |
| `tag-helper` | C++23 | helper, no capabilities | Reads tags, cue sheets, playlists, lyrics files, embedded pictures (bytes only) |
| `tag-writer` | C++23 | helper, one writable descriptor per call | Writes tags to a temp file; verifies audio hash |
| `encode-helper` | C++23 plus vendored encoders | helper, no capabilities | Convert, rip encode, sync transcode |
| `rip-helper` | C++23 | helper, one read-only optical device handle | Secure read, TOC, sub-channel |
| `net-fetch-helper` | C++23 | helper, sockets to approved hosts | Radio streams, Subsonic and Jellyfin HTTP |
| `cast-helper` | C++23 | helper, LAN sockets (ME03-GAP-4) | SSDP, UPnP AV, one-time listener |
| `device-sync-helper` | C++23 | helper, one USB handle or one folder handle | MTP writes |
| Index worker | ForgeScript | app host background task | Scan, tag read, loudness jobs |

```
 portal handles ─► app host: logic (ForgeScript) ─ UI (FUSE) ─ audio thread (DSP chain)
        │ fd            │ tags (records)        │ packets/PCM (shm)        │ PCM ring
   ┌────▼─────┐   ┌─────▼─────┐   ┌─────────────▼────┐              ┌──────▼─────┐
   │ demux    │   │ tag-helper│   │ decode-helper    │              │ svc.media  │→ device
   └──────────┘   └───────────┘   └──────────────────┘              │ shared /   │
   ┌──────────┐   ┌───────────┐   ┌──────────────────┐              │ exclusive  │
   │ rip      │   │ tag-writer│   │ net-fetch / cast │→ approved    └────────────┘
   │ helper   │   │ (temp fd) │   │ helpers          │  hosts only
   └──────────┘   └───────────┘   └──────────────────┘
```

### 6.2 Data model

Store: `~/.local/share/org.pbsd.music/library.sqlite`; art cache `~/.cache/org.pbsd.music/art/`; waveform peaks `~/.cache/org.pbsd.music/peaks/`; offline server cache `~/.cache/org.pbsd.music/offline/` (marked "keep" so it is not purged silently).

| Entity | Fields |
|---|---|
| `track` | `id INTEGER PK`, `root_id`, `rel_path TEXT`, `cue_index INTEGER`, `audio_hash BLOB(16)`, `size`, `mtime`, `title`, `sort_title`, `album_id`, `disc INTEGER`, `track_no INTEGER`, `duration_ms`, `year INTEGER`, `date TEXT`, `codec TEXT`, `sample_rate INTEGER`, `bit_depth INTEGER`, `channels INTEGER`, `bitrate INTEGER`, `lossless INTEGER`, `rg_track_gain REAL`, `rg_track_peak REAL`, `lufs REAL`, `lra REAL`, `encoder_delay INTEGER`, `encoder_padding INTEGER`, `rating INTEGER`, `favourite INTEGER`, `play_count INTEGER`, `skip_count INTEGER`, `last_played INTEGER`, `added_at INTEGER`, `resume_ms INTEGER`, `mb_recording_id TEXT`, `lyrics_ref TEXT`, `missing INTEGER` |
| `album` | `id`, `title`, `sort_title`, `album_artist_id`, `year`, `compilation INTEGER`, `rg_album_gain REAL`, `rg_album_peak REAL`, `art_ref TEXT`, `mb_release_id TEXT` |
| `artist` | `id`, `name`, `sort_name`; `track_artist(track_id, artist_id, role TEXT)` with roles `artist\|composer\|conductor\|performer\|lyricist` |
| `genre` / `track_genre` | `id`, `name` / `track_id`, `genre_id` |
| `work` | `id`, `composer_id`, `title`; `track.work_id`, `track.movement_no`, `track.movement_name` |
| `playlist` | `id`, `parent_id`, `name`, `kind TEXT (manual\|smart\|auto)`, `rules_json`, `file_sync_path TEXT`; `playlist_item(playlist_id, pos, track_id, remote_ref)` |
| `queue_item` | `pos`, `track_id`, `remote_ref`, `origin TEXT`, `state TEXT` |
| `server` | `id`, `kind TEXT (subsonic\|jellyfin)`, `base_url`, `user`, `keyring_ref`, `route`, `cert_fingerprint TEXT` |
| `station` | `id`, `name`, `url`, `route`, `last_title TEXT` |
| `device` | `id`, `label`, `kind`, `selection_json`, `transcode TEXT`, `last_sync INTEGER` |
| `dsp_profile` | `output_device_id TEXT PK`, `chain_json TEXT` |
| `scrobble_queue` | `id`, `target_id`, `payload_json`, `created_at`, `sent_at` |
| `tag_journal` | `id`, `track_id`, `before_json`, `after_json`, `at` |

`audio_hash` = BLAKE3 of the first 30 s of compressed audio frames (tags excluded), truncated to 16 bytes. Schema version in `meta`; forward-only migration with a kept copy.

### 6.3 File formats and protocols

Codec and container support is the table in ME03 section 6.5.3. App-specific:

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| ID3v1, ID3v2.2–2.4, APEv2, Vorbis comments, MP4 atoms, RIFF INFO, Matroska tags | yes | yes | own C++23 (TagLib, LGPL-2.1/MPL-1.1, is the differential reference) | tag helper / tag writer |
| Embedded pictures | yes | yes | image codec helper (ME01 section 6.3) | image codec helper |
| CUE sheets | yes | yes | own code | tag helper |
| M3U, M3U8, PLS, XSPF | yes | yes | own code | tag helper |
| LRC, enhanced LRC, SRT | yes | LRC | own code | tag helper |
| ReplayGain 2.0, R128 tags | yes | yes | own code (ITU-R BS.1770-4 meter) | decode helper computes, tag writer writes |
| CD-DA (Red Book), CD-Text, sub-channel Q | yes | — | own code over SCSI MMC read commands | rip helper |
| Offline rip verification databases | yes | no | own reader for the installed dump format | app host (trusted local data), parsed defensively |
| Icecast/Shoutcast HTTP with ICY metadata, HLS audio | yes | — | own code | net-fetch helper |
| Subsonic REST API (JSON) | yes | yes | own code | net-fetch helper |
| Jellyfin HTTP API (JSON) | yes | yes | own code | net-fetch helper |
| ListenBrainz-compatible submit API | — | yes | own code | net-fetch helper |
| UPnP AV, DLNA | yes | yes | own code | cast helper |
| MTP | yes | yes | own code | device-sync helper |
| Equaliser preset text (AutoEQ-style), `.pbeq` JSON | yes | yes | ForgeScript | app host (bounded text) |

### 6.4 Integration

- Intents handled: `open`, `view` (audio MIME types, playlists, `http(s)` stream URLs handed over). Emits: `share`, `edit` (CR03), `open` (ME03 for video files).
- Services: `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.usb`, `svc.intent`, `svc.search`, `svc.keyring`, `svc.thumbnail`, `svc.media`, `svc.power`, `svc.model`, `svc.undo-trash`, `svc.audit`.
- Offers: `org.pbsd.music.Player1`; thumbnailer for audio (embedded art); the equaliser preset format shared with ME03.
- Other apps: IN02 (radio and podcast hand-off, position report), ME03 (pipeline, casting helper, optical portal), ME01 (image codec), ME06 (recordings can be added to the library by folder), CR03 (edit), SH02/SH03 (media controls), SY01 (audio devices), VM05 (USB devices).
- Backing repos, Verified: `fuse_audio` mixes voices in software to one stereo float output stream with Null and OpenAL back ends (`FUSE:Source/FUSE/Audio/include/fuse/audio/audio_backend.hpp`); it decodes Ogg Vorbis and FLAC from memory with vendored Xiph libraries (`FUSE:Source/FUSE/Audio/include/fuse/audio/audio_codec.hpp`, `FUSE:cmake/FuseXiph.cmake`); it has a streaming Vorbis decoder with a fixed window (`FUSE:Source/FUSE/Audio/include/fuse/audio/audio_stream.hpp`). It is a game engine: no gapless join, no tag reading, no exclusive device mode, stereo only at the output. Reuse of its ring and convolution code is Proposed.

## 7. Security and privacy

| Capability | Why |
|---|---|
| `fs.app` | Library database, caches |
| `fs.portal` | Library folders, opened files, export and sync folders |
| `net.user` | Radio stations and self-hosted servers the user enters; off until the first one is added |
| `net.route:vpn`, `net.route:tor` | Optional per server |
| `proc.helper:*` (nine helpers in 6.1) | Parsing, decoding, network and device work outside the app host |
| `dev.usb:mtp` | Device sync |
| `ai.local`, `svc.model` | Optional similarity and lyric transcription |
| `bg.run` | Play with the window closed |
| `svc.*` listed in 6.4 | As described |

Not requested: `dev.mic`, `dev.camera`, `dev.screen`, `dev.location`, `proc.exec`, `bg.autostart`, any fixed `net.host`.

| Asset | Adversary | Attack surface | Mitigation |
|---|---|---|---|
| Session and files | Crafted audio file, tag, picture, cue, playlist | Decoders, tag parsers | Zero-capability helpers, limits, fuzzing |
| Audio files | Bugs in tag writing or renaming | Tag writer, batch rename | Temp file + audio-hash verify + atomic rename, journal, undo, preview |
| Listening habits | Network observers, third-party services | Lookups, scrobbles, radio | No lookups at all; scrobbling only to hosts the user enters, off by default, with a visible log; private listening |
| Server credentials | Network attacker, hostile server | HTTP API, redirects | HTTPS required, certificate pin on first use, credentials in keyring, no cross-host redirects, JSON parsed in the helper |
| LAN exposure | Hostile LAN device | SSDP, UPnP XML, cast listener | Discovery only while the popover is open; one peer, one token; XML parsed in the cast helper |
| Optical and USB devices | Hostile disc or device firmware | SCSI replies, MTP | Device helpers hold one handle and nothing else |
| Radio stream | Hostile ICY metadata | Text shown in UI and notifications | Treated as plain text, length-limited, control characters removed |

Stored: library index, play counts, ratings, playlists, DSP profiles, server entries (secrets in the keyring). Leaves the machine: nothing by default. With a server or station: requests to that host. With scrobbling on: track metadata to the user's own target.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `library.folders` | list of grants | empty | Library roots |
| `library.multi_value_separators` | list | `;` | Splitters for legacy tags |
| `library.group_compilations` | bool | true | One album for compilations |
| `library.write_ratings_to_tags` | bool | false | Mirror ratings into files |
| `playback.gapless` | bool | true | Gapless join |
| `playback.crossfade_s` | float | 0 | Crossfade length |
| `playback.crossfade_in_album` | bool | false | Crossfade inside albums |
| `playback.replaygain` | enum `off\|track\|album\|auto` | `auto` | Gain mode |
| `playback.preamp_db` | float | 0 | ReplayGain pre-amp |
| `playback.no_gain_fallback_db` | float | −6 | Gain for untagged tracks |
| `playback.count_at_percent` | int | 50 | Play-count rule |
| `playback.resume_long_tracks` | bool | true | Per-track resume over 20 min |
| `playback.pause_on_device_loss` | bool | true | Pause when output disappears |
| `output.device` | string | `default` | Pinned device |
| `output.bit_perfect` | bool | false | Exclusive mode |
| `output.resampler` | enum `fast\|balanced\|best` | `balanced` | Quality |
| `output.dither` | enum `off\|tpdf\|shaped` | `tpdf` | Dither |
| `dsp.enabled` | bool | false | DSP chain |
| `lyrics.folder` | grant | none | Extra lyrics folder |
| `notify.track_change` | bool | false | Toast on track change |
| `rip.format` | enum | `flac` | Rip output |
| `rip.path_template` | string | `{albumartist}/{album}/{track:02} {title}` | Rip layout |
| `rip.passes` | int | 2 | Minimum matching passes |
| `tags.id3_version` | enum `2.3\|2.4` | `2.4` | ID3 version written |
| `net.require_tls` | bool | true | Refuse plain HTTP outside private ranges |
| `scrobble.enabled` | bool | false | Self-hosted scrobbling |
| `scrobble.targets` | list | empty | User-entered endpoints |
| `privacy.private_listening` | bool | false | No counts or history |
| `privacy.contribute_search` | bool | true | Titles to `svc.search` |
| `background.keep_playing` | bool | true | Play with window closed |
| `visual.enabled` | bool | false | Visualisations |

## 9. Accessibility and languages

- Track changes are announced only on request (`Ctrl+I` reads title, artist, position) unless the user turns on announcements.
- The album grid has full names per cell (album, artist, year); art has no meaning beyond the name.
- The equaliser sliders expose frequency and gain as text; presets can be chosen without touching sliders.
- Visualisations are decorative, off by default, hidden from the a11y tree, and disabled with reduced motion or photosensitivity mode.
- Lyrics are real text and readable by AX01; synced lyrics can be sent to a live region.
- Sorting is locale-aware (collation), with sort tags taking priority; tags in any script (G-03).

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to window | 200 ms; resume playback 400 ms |
| Track start after double-click | 100 ms local; 500 ms server |
| Gap between gapless tracks | 0 samples |
| Audio thread | No allocation, no locks, no IPC round trip in the callback; under 10% of one core at 192 kHz with EQ |
| Underruns | 0 in a 24 h soak on an idle system |
| Library | 500 000 tracks; search under 50 ms; scroll 60 fps |
| Scan | 1000 tracks/s tags only on SSD; loudness scan 40× real time per core |
| Memory | app host under 200 MB with 100 000 tracks; decode helper under 64 MB |
| CPU while playing FLAC to a shared device, window hidden | under 1% of one core; 0 frames drawn |
| Rip | Drive speed limited; secure mode at least 4× on a clean disc |

## 11. Testing and proof

Nothing has run.

- Unit: queue and source logic, shuffle properties, smart-playlist rules, rename templates, ReplayGain mode choice, sort rules.
- Golden: `.fuseui` renders of all views; audio goldens (bit-exact output for lossless decode, gapless joins checked sample by sample across MP3, Opus, Vorbis, FLAC, AAC test pairs).
- Scripted UI: add folder, play, queue, tag edit and undo, smart playlist, mini player.
- Conformance: FLAC decoder test suite, Opus test vectors, EBU R128 test signals (EBU Tech 3341/3342), resampler measurements (passband ripple, stopband).
- Differential: tag reading and writing against TagLib on a corpus; loudness against `libebur128`; rip results against a known-good image.
- Fuzz: every tag format, cue, playlist, LRC, ICY metadata, Subsonic and Jellyfin JSON, SSDP and UPnP XML, MTP replies, CD-Text.
- Bit-perfect test: loopback device capture compared with source PCM.
- Network test: with no servers and no radio configured, a packet capture shows zero packets.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Tag parsers | No runtime errors; bounded memory | T2 | PRISM; fuzz | — |
| Tag writer | Audio bytes unchanged; output re-parses to the intended model | T2 for safety, T3 for function | PRISM; hash check at run time; round-trip tests | Format quirks |
| Gapless joiner and crossfader | Sample count conservation; no overflow | T1 | Small pure functions; proof of the sample arithmetic | — |
| ReplayGain and limiter maths | No overflow; output within ±1.0 | T2 | PRISM; property tests | — |
| EQ filters, resampler | Bounded output; no denormal stalls | T2 | PRISM; measurements | Floating-point proofs are limited |
| BS.1770 meter | Matches the EBU test set within 0.1 LU | T3 | Conformance tests | Standard is prose |
| Rename planner | No two sources map to one target; no path leaves the granted root | T1 | Proof over the plan function | — |
| Rip secure-read logic | Result accepted only when passes match | T2 | PRISM; simulated drive | Drive behaviour is external |
| Vendored codecs | Contained | T4 | Helper sandbox | Third-party C |
| Server clients | Requests only to the configured host | T2 | Allow-list function proof (shared with ME03); tests | — |

## 12. Build plan

### Phase 1: player and library

| ID | Work item | Depends on |
|---|---|---|
| ME02-W01 | Audio session on the media pipeline: gapless join, ReplayGain, resampler, PCM ring to `svc.media` | G-11, ME03 phase 1 pipeline items, FUSE-R-088 |
| ME02-W02 | `tag-helper` read path: ID3, Vorbis comments, MP4, APE | FORGE-WP-27, FORGE-WP-28 |
| ME02-W03 | Library index, album/artist/track views, search | G-02, G-09 |
| ME02-W04 | Queue, manual playlists, resume | — |
| ME02-W05 | Player bar, now playing, mini player | G-01, SH05 |
| ME02-W06 | Media session, media keys, background play | `svc.media`, G-07 (`bg.run`) |
| ME02-W07 | Artwork through the image codec helper | ME01 phase 1 |
| ME02-W08 | FLAC, Opus, Vorbis, MP3, WAV playback | ME03 base pack |

### Phase 2: full feature

| ID | Work item | Depends on |
|---|---|---|
| ME02-W09 | `tag-writer`, tag editor, batch rename, journal | — |
| ME02-W10 | Smart playlists, playlist file import and export, cue sheets | — |
| ME02-W11 | DSP chain: EQ, limiter, crossfade, dither, per-device profiles | — |
| ME02-W12 | Bit-perfect exclusive mode and signal path view | ME02-GAP-1, `svc.media` |
| ME02-W13 | Loudness scanner | — |
| ME02-W14 | Lyrics view | — |
| ME02-W15 | Radio, Subsonic and Jellyfin clients | FORGE-WP-21, ME03 net-fetch helper |
| ME02-W16 | Convert and verify tools | ME03 `encode-helper` |
| ME02-W17 | AAC, ALAC and legacy lossless formats | restricted pack decision |
| ME02-W18 | IN02 hand-off protocol | IN02 |
| ME02-W19 | Bus interface, `svc.search` contribution | G-08 |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| ME02-W20 | CD ripping and playback (`rip-helper`) | ME03-GAP-3 |
| ME02-W21 | Casting and UPnP servers | ME03-GAP-4, ME03 `cast-helper` |
| ME02-W22 | Device sync (folder and MTP) | SH04 USB portal, VM05 |
| ME02-W23 | Scrobbling to self-hosted targets | — |
| ME02-W24 | Offline cache for servers | — |
| ME02-W25 | DSD, convolution, crossfeed, speed and pitch | — |
| ME02-W26 | Local similarity, seed radio, lyric transcription | G-15, FORGE-WP-31 |
| ME02-W27 | Lyrics editor, visualisations, duplicate finder | — |

## 13. Open questions and risks

1. **`svc.media` exclusive mode** (ME02-GAP-1) is assumed. On FreeBSD the kernel audio interface is OSS; whether its bit-perfect mode and per-device rate switching can be exposed safely through a service on HardenedBSD is Unverified.
2. **AAC** needs the restricted pack. Many purchased collections are AAC.
3. **Rip verification without an online database** is weaker: two matching passes can share a consistent read error. Offline database dumps may have licence terms that prevent bundling.
4. **Optical drive portal** (ME03-GAP-3) and raw SCSI MMC access from a confined helper need kernel and portal work.
5. **Subsonic and Jellyfin** servers vary; the Subsonic password scheme is weak by design. The app must prefer token or API-key modes and say so.
6. **`bg.run` and the audio thread**: the interpreter must not be on the audio path. The DSP chain is native code inside the app host, which stretches PLATFORM.md section 2 ("helper only"). The alternative, a separate audio helper process, costs latency. Needs a platform decision.
7. **Scrobbling** to "self-hosted only" cannot be verified technically; the app can only refuse to ship public endpoints.
8. **Own tag library** is a large compatibility effort; vendoring TagLib inside the helpers is the fallback.
