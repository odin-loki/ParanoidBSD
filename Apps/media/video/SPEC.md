<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Video

By Odin Loch

| | |
|---|---|
| ID | `ME03` (from CATALOGUE.md) |
| Bundle | `org.pbsd.video` |
| Category | media |
| Priority | P1 |
| Backing repos | FUSE (`videoWidget` stub, Theora source, game audio engine). No player exists. |
| Status | Planned. Nothing built. |

## 1. Purpose

Video plays local files, discs where legal, and network streams the user names. It is for anyone who watches films, lectures, recordings and clips, and it replaces mpv, VLC and the Plasma players. This spec also defines, once, the shared media pipeline (platform gap G-11) that ME01, ME02, ME04, ME05, ME06, CR02 and CR03 build on; see section 6.5.

## 2. Scope

In scope:

- Playback of local files, folders, playlists, discs (limits in 4.14) and user-named streams.
- Subtitles, audio and video track selection, chapters, speed, frame step, A-B loop.
- Screenshots and short clips cut from the playing file.
- HDR tone mapping, hardware decode, filters, audio passthrough.
- A video library with offline metadata from NFO files.
- Casting to devices on the local network.
- The definition of the shared media pipeline (section 6.5).

Out of scope:

| Item | Owner |
|---|---|
| Editing, timelines, effects, export presets | CR02 Video Editor |
| Music library and audio-only listening | ME02 Music |
| Recording the screen | ME05 Screen Capture |
| Recording from a camera | ME04 Camera |
| Downloading media, torrents | IN03 Transfers |
| Podcast and feed subscriptions | IN02 Feeds |
| Web video inside pages | IN01 Web Browser |
| DRM-protected streaming services | not planned; no DRM module ships |
| Picture-in-picture window rules | SH05 Window Management (this app asks for the window type) |
| Device and network pickers, prompts | SH04 Portals |

## 3. Users and top tasks

Personas:

- **Viewer**: double-clicks a file and wants it to play with the right subtitles.
- **Collector**: keeps a library of films and series on a local disk or NAS, with NFO files.
- **Student**: watches lectures at 1.75×, loops sections, takes frame-accurate screenshots.
- **Reviewer**: steps frames, compares tracks, checks HDR metadata and stream details.
- **Careful user**: opens a file from an untrusted source and wants nothing to reach the network or the disk.

| # | Task | Target steps |
|---|---|---|
| T1 | Play a file from Files (SY02) | 1 (double-click) |
| T2 | Resume where I stopped | 0 (automatic), 1 to restart |
| T3 | Turn on a subtitle track | 1 key (`J`) or 2 clicks |
| T4 | Load an external subtitle file | 1 (drop on window) |
| T5 | Fix subtitle delay | 1 key per 50 ms step |
| T6 | Change speed with correct pitch | 1 key (`]` / `[`) |
| T7 | Step one frame forward or back | 1 key (`.` / `,`) |
| T8 | Take a screenshot of the frame | 1 key (`S`) |
| T9 | Cut a clip between two points | 3 (mark A, mark B, export) |
| T10 | Open a network stream | 2 (`Ctrl+L`, paste, Enter) plus one prompt on first contact |
| T11 | Cast to a television on the LAN | 2 (cast button, pick device) |
| T12 | Move playback to picture-in-picture | 1 key (`Ctrl+Shift+P`) |
| T13 | Find a film in the library | 2 (focus search, type) |
| T14 | Generate subtitles for a file with no subtitles | 2 (Subtitles ▸ Generate, confirm) |
| T15 | Open an untrusted file with hardware decode off | 1 (context menu "Open in strict mode") |

## 4. Features

### 4.1 Playback core

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| @F | Open file | From intent `open`, drop, `Ctrl+O` (file portal), command line handle. One window per file unless "reuse window" is set. | M | 1 |
| @F | Open folder | Plays every playable file in name order as a temporary playlist. Natural sort (`ep2` before `ep10`). | M | 1 |
| @F | Play, pause | `Space`, `K`, click on video. Pause shows the frame at full quality (no post-filter skip). | M | 1 |
| @F | Stop | `Ctrl+.` releases the decoder and the file handle and shows the start view. | M | 1 |
| @F | Seek bar | Slider with buffered ranges, chapter ticks, A-B markers, hover time and thumbnail preview. | M | 1 |
| @F | Relative seek | `Left`/`Right` 5 s, `Shift` 1 s, `Ctrl` 60 s, `Up`/`Down` volume. Steps set in settings. | M | 1 |
| @F | Exact and keyframe seek | Keyframe seek while dragging, exact seek on release. Setting to force either. | M | 1 |
| @F | Go to time | `Ctrl+G` dialog accepts `h:mm:ss.mmm`, seconds, frame number, or percent. | S | 1 |
| @F | Frame step | `.` forward one frame, `,` back one frame (decodes from the previous keyframe). Works while paused. | M | 1 |
| @F | Playback speed | 0.1× to 8× in steps of 0.05; `[` `]` step, `Backspace` resets to 1×. | M | 1 |
| @F | Pitch correction | Time-stretch (WSOLA, own code) keeps pitch from 0.5× to 4×; outside that range audio is muted or resampled by choice. | M | 1 |
| @F | A-B loop | `L` sets A, then B, then clears. Loop count: forever or n times. Markers shown on the seek bar. | M | 1 |
| @F | Loop file and loop playlist | Off, one, all. Shuffle for playlists. | S | 1 |
| @F | Volume and mute | 0 to 100%, optional boost to 200% with a soft limiter. `M` mutes. Per-file volume is not remembered unless set. | M | 1 |
| @F | Audio delay | `Ctrl++` / `Ctrl+-` in 10 ms steps, shown in the on-screen display, remembered per file and per output device. | M | 1 |
| @F | A/V sync | Audio is the master clock; video frames are dropped or repeated; display-resample mode matches video to the display refresh. | M | 1 |
| @F | Resume position | Stored per file content key (size + BLAKE3 of first and last 1 MiB). Resume if stopped between 2% and 95%. | M | 1 |
| @F | Resume prompt policy | Always resume, ask, never. "Ask" is an inline bar, not a dialog. | S | 1 |
| @F | Per-file memory | Remembers audio track, subtitle track, delays, aspect, zoom, speed per file. Can be turned off or cleared. | S | 2 |
| @F | End-of-file action | Next in playlist, stop, close window, repeat, or run the session action "sleep" through `svc.power`. | S | 2 |
| @F | Sleep inhibit | Holds a `svc.power` inhibitor only while playing video on a visible window. | M | 1 |
| @F | Sleep timer | Stop after n minutes or at end of the current file. | C | 2 |
| @F | Stream information | Inspector tab: container, codecs, profiles, bit rates, colour tags, HDR metadata, dropped frames, decoder in use. | M | 1 |
| @F | Statistics overlay | `I` toggles a live overlay: frame times, drops, A/V offset, buffer, decode path. | S | 2 |

### 4.2 Video output

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| @F | Aspect ratio | Auto from stream, then presets 4:3, 16:9, 16:10, 1.85:1, 2.35:1, 2.39:1, custom. Key `A` cycles. | M | 1 |
| @F | Zoom, pan, crop | Zoom 25% to 800%, pan with `Alt`+arrows, crop presets, automatic black-bar detection. | S | 2 |
| @F | Rotate and flip | 90° steps and mirror; honours the container rotation tag. | S | 1 |
| @F | Scaling filters | Bilinear, bicubic, Lanczos 3, plus sharper luma scalers on the GPU. Separate chroma upscaler. | M | 2 |
| @F | Deinterlace | Off, auto (from stream flags), bob, yadif-style, motion-adaptive on GPU. Field order override. | M | 2 |
| @F | Denoise | Temporal-spatial denoise, strength 0 to 10. Off by default. | C | 3 |
| @F | Debanding | Gradient debanding with dither. Off by default. | C | 3 |
| @F | Sharpen | Unsharp mask, 0 to 10. | C | 3 |
| @F | Picture controls | Brightness, contrast, saturation, gamma, hue. Reset button. Not saved per file unless asked. | S | 2 |
| @F | Neural upscaling | 2× and 4× upscalers run through `svc.model` (`ai.local`, `dev.gpu.compute`). Only offered when the model is installed (AI02). Falls back to Lanczos when the frame budget is missed for 2 s. | C | 3 |
| @F | Frame interpolation | Motion interpolation to the display rate. Off by default. Local model or optical flow. | C | 3 |
| @F | Dithering | Ordered or error-diffusion dither to the display bit depth. | S | 2 |
| @F | Display refresh matching | Asks the compositor to switch refresh rate to a multiple of the video rate while full screen (needs ME03-GAP-2). | C | 3 |
| @F | Full screen | `F11`, `F`, double-click. Controls hide after 2 s without pointer movement; pointer hides too. | M | 1 |
| @F | Always on top, borderless | Window options passed to SH05. | S | 2 |
| @F | Picture-in-picture | `Ctrl+Shift+P` asks SH05 for a PiP window: small, always on top, on every workspace, with play, seek ±10 s, close and "return" controls. | M | 2 |
| @F | Multi-monitor | Choose which screen full screen uses; remembers per file type. | S | 2 |

### 4.3 Colour and HDR

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| @F | Colour-space handling | Reads matrix, primaries, transfer and range tags; guesses by resolution when untagged; override in the inspector. | M | 1 |
| @F | Display colour management | Converts to the display ICC profile supplied by the compositor. 3D LUT built per profile and cached. | M | 2 |
| @F | HDR tone mapping | PQ (SMPTE ST 2084) and HLG to SDR with BT.2390, Hable, Reinhard, clip. Per-scene peak detection. Target nits setting. | M | 2 |
| @F | HDR passthrough | Sends HDR10 static metadata to an HDR display when the compositor supports it (ME03-GAP-1). | S | 3 |
| @F | HDR10+ dynamic metadata | Parsed and used to guide tone mapping. | C | 3 |
| @F | Dolby Vision | Profiles 5 and 8 tone mapped using the reshaping metadata where the format is publicly documented; otherwise the base layer is played and the inspector says so. | C | 3 |
| @F | Gamut mapping | BT.2020 to display gamut: clip, perceptual, or desaturate. | S | 2 |
| @F | HDR screenshot choice | Screenshots are saved tone-mapped (PNG) or as HDR (AVIF 10-bit, JPEG XL) by choice. | C | 3 |

### 4.4 Audio

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| @F | Audio track selection | Key `#` cycles; menu lists language, title, codec, channels, default and forced flags. | M | 1 |
| @F | Preferred languages | Ordered list for audio and for subtitles. Rule: "subtitles only when audio is not in a preferred language". | M | 1 |
| @F | Output device | Chosen through `svc.media`; follows the system default unless pinned. | M | 1 |
| @F | Downmix and upmix | 7.1/5.1 to stereo with selectable centre and LFE gains; stereo stays stereo unless upmix is asked for. | M | 1 |
| @F | Dynamic range compression | "Night mode" compressor; honours AC-3 DRC metadata. | S | 2 |
| @F | Loudness normalisation | EBU R128 running normalisation to −23 LUFS, off by default. | S | 2 |
| @F | Equaliser | 10-band graphic equaliser with presets, shared format with ME02. | C | 2 |
| @F | Audio passthrough | AC-3, E-AC-3, DTS, DTS-HD, TrueHD sent unmodified as IEC 61937 to a receiver over HDMI or S/PDIF when `svc.media` reports support. Per-codec switches. Falls back to decode. | S | 2 |
| @F | External audio file | Add an audio file as an extra track (drop with `Shift`, or Audio ▸ Add file). | S | 2 |
| @F | Channel mapping check | Test tone per speaker in Preferences ▸ Audio. | C | 3 |
| @F | Audio-only display | For audio files: cover art or a plain visualisation; suggests ME02 once. | S | 2 |

### 4.5 Subtitles

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| @F | Text subtitle formats | SRT, WebVTT, ASS/SSA, MicroDVD, SubViewer, SAMI, TTML, LRC. Embedded and external. | M | 1 |
| @F | Bitmap subtitle formats | PGS (Blu-ray), VobSub (DVD), DVB subtitles. Scaled with the video. | M | 2 |
| @F | Closed captions | CEA-608 and CEA-708 extracted from the video stream. | S | 2 |
| @F | ASS rendering | Full ASS styling, positioning, karaoke, embedded fonts (fonts are parsed in the subtitle helper only). | M | 2 |
| @F | Auto-load external subtitles | Same folder, same base name, also `Subs/` and `Subtitles/` subfolders. Needs a folder grant; otherwise an inline bar offers "Allow this folder". | M | 1 |
| @F | Encoding detection | UTF-8, UTF-16 with BOM, then a statistical guess among legacy code pages; manual override; re-decodes live. | M | 1 |
| @F | Subtitle track selection | `J` cycles, `Shift+J` back, `V` toggles visibility. | M | 1 |
| @F | Two subtitles at once | Primary at the bottom, secondary at the top. For language learning. | S | 2 |
| @F | Subtitle delay | `Z` / `X` in 50 ms steps; "sync to next line" (`Ctrl+Shift+Right`) snaps the next cue to now. | M | 1 |
| @F | Subtitle speed fix | Rescale timing for 23.976 ↔ 25 fps mismatches with presets. | S | 2 |
| @F | Styling override | Font, size, colour, outline, shadow, background box, position, margin. For text formats; "force over ASS" option. | M | 1 |
| @F | Position in black bars | Draw subtitles in the letterbox area when there is one. | S | 2 |
| @F | Subtitle list panel | Inspector tab listing every cue with time; click seeks; filter box searches the text. | S | 2 |
| @F | Local subtitle search | Search subtitle files across granted library folders by file name, language tag and content hash. No online lookup. | S | 2 |
| @F | Copy cue text | Click a cue in the panel and `Ctrl+C`; on-video selection in pause mode. | C | 3 |
| @F | Local subtitle generation | Speech-to-text through `svc.model` (`ai.local`). Extracts audio, runs in the background, shows progress, writes a `.srt` or `.vtt` beside the file (portal save) or into app data. Language auto-detect or chosen. Marked "machine generated" in the track list. | S | 3 |
| @F | Local subtitle translation | Translate an existing text track with a local model; output is a new track marked "machine translated". | C | 3 |
| @F | Save edited timing | Export the current track with the applied delay and speed as SRT or WebVTT. | S | 2 |
| @F | Forced subtitles | Honours the forced flag; "forced only" mode. | M | 1 |
| @F | Subtitle sandboxing | Every subtitle and font file is parsed in the subtitle helper with no capabilities; the app receives cue records and glyph bitmaps only. | M | 1 |

### 4.6 Chapters, tracks and navigation

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| @F | Chapters | Read from Matroska, MP4, Ogg; `PgUp` / `PgDn` move between chapters; chapter list in the sidebar with thumbnails. | M | 1 |
| @F | Editions | Matroska ordered chapters and editions selectable; linked segments are off unless the folder is granted. | C | 3 |
| @F | Video track selection | For files with several video streams or angles. | S | 2 |
| @F | Bookmarks | `B` adds a named bookmark with thumbnail; stored in app data, exportable as chapters (FFMETADATA text) or CSV. | S | 2 |
| @F | Thumbnail seek preview | Generated on first open in the decode helper at 1 per 10 s, cached in `~/.cache`; uses embedded trick-play images when present. | S | 2 |
| @F | Jump to next scene | `Ctrl+Right` uses a scene-cut detector over cached thumbnails. | C | 3 |
| @F | Skip silence, skip intros | Optional local detectors; manual "skip 85 s" key (`Ctrl+Shift+Right` is rebindable). | C | 3 |

### 4.7 Playlists and queue

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| @F | Play queue | Sidebar list; reorder by drag; remove; "play next"; "add to end". | M | 1 |
| @F | Playlist files | Read and write M3U, M3U8, PLS, XSPF. Entries resolve only inside granted folders or named hosts. | M | 1 |
| @F | Next and previous | `N` / `P`, `Shift+N`. Auto-advance with an optional 5 s countdown. | M | 1 |
| @F | Series auto-queue | Opening `S01E03` offers to queue the following episodes in the folder (needs folder grant). | S | 2 |
| @F | Saved playlists | Named playlists stored in the library database. | S | 2 |
| @F | Recently played | Start view grid with progress bars; can be turned off; "Clear history" in one click. | M | 1 |
| @F | Private session | No history, no resume positions, no thumbnails written. Header shows a mask badge. | M | 1 |

### 4.8 Screenshots and clips

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| @F | Screenshot | `S` saves the frame at source resolution to the screenshots folder (default `~/Pictures/Video`, granted at first use). PNG, JPEG, WebP, AVIF, JPEG XL. | M | 1 |
| @F | Screenshot with subtitles | `Shift+S` includes subtitles and filters as shown. | S | 1 |
| @F | Copy frame | `Ctrl+C` puts the frame on the clipboard as PNG. | S | 1 |
| @F | Screenshot name template | `{title}-{time}-{n}`; metadata written: source name optional, off by default. | S | 2 |
| @F | Burst and contact sheet | n frames every m seconds; a contact sheet grid image with time codes. | C | 3 |
| @F | Clip export, stream copy | Cut between A and B without re-encoding (keyframe-aligned, exact end) into MKV or MP4. | M | 2 |
| @F | Clip export, re-encode | Frame-exact clip to WebM (VP9 or AV1 + Opus) or MP4 (AV1 + Opus/AAC). Optional subtitles burned in, optional scale. Runs in the encode helper. | S | 2 |
| @F | Animated image export | GIF, animated WebP or AVIF from a selection up to 30 s, with frame-rate and width controls. | S | 3 |
| @F | Audio extract | Save the selected audio track or selection as FLAC, Opus or WAV. | C | 3 |
| @F | Hand-off to editor | "Edit in Video Editor" sends intent `edit` to CR02 with the file handle and A-B range. | S | 2 |
| @F | Share | Share sheet (SH04) for the file, a clip or a screenshot. Metadata stripped from screenshots by default. | S | 2 |

### 4.9 Network streams

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| @F | Open location | `Ctrl+L`. Accepts `http(s)`, `rtsp(s)`, `rtmp(s)`, `srt`, `udp`/`rtp` (multicast needs ME03-GAP-4). Needs `net.user`; first contact with each host is prompted and logged. | M | 2 |
| @F | HLS | Master and media playlists, fMP4 and MPEG-TS segments, AES-128 with keys from the same origin policy, live and VOD, alternate audio and subtitles. | M | 2 |
| @F | MPEG-DASH | Static and dynamic MPD, SegmentTemplate/Timeline/Base, multi-period. No DRM. | S | 2 |
| @F | RTSP | RTSP 1.0 over TCP interleaved by default; UDP by choice. Basic and digest auth, credentials in `svc.keyring`. For local cameras. | S | 2 |
| @F | Progressive HTTP | Range requests, seek, resume after drop. | M | 2 |
| @F | Adaptive quality | Auto by throughput and buffer; manual quality pick; cap by resolution. | M | 2 |
| @F | Buffer control | Target buffer seconds, shown on the seek bar; low-latency mode for live. | S | 2 |
| @F | Host allow-list per stream | A stream may only fetch from hosts listed in a visible "Hosts" popover; a new host needs approval (a playlist cannot pull the app to arbitrary hosts). | M | 2 |
| @F | Route choice | Per stream: clear, VPN or Tor, within what `net.route` grants. Shown as a badge. | S | 2 |
| @F | No silent requests | No artwork, metadata, subtitle or update lookups ever. The `[net]` badge lists every open endpoint. | M | 1 |
| @F | Save stream list | Named streams stored locally (name, URL, route). Import and export as M3U. | S | 2 |
| @F | Record stream | Save the incoming stream unmodified to a file the user picks. | C | 3 |
| @F | Hand-off from other apps | Handles intent `open` for URLs from IN01, IN02 and CO05 after the user confirms the host. | S | 2 |

### 4.10 Local casting

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| @F | Discover renderers | UPnP/DLNA MediaRenderer discovery by SSDP on the local link. Runs only while the cast popover is open (needs ME03-GAP-4). | S | 3 |
| @F | Cast file | Serves the file from a one-time HTTP listener bound to the LAN address, random path token, only reachable by the chosen device address; closed when casting stops. | S | 3 |
| @F | Transcode for the device | When the renderer cannot play the format, re-encode on the fly in the encode helper to a profile the device lists. | C | 3 |
| @F | Remote control | Play, pause, seek, volume, stop on the renderer; local window shows the position. | S | 3 |
| @F | Cast subtitles | Burned in when transcoding; otherwise sent as an SRT side resource when the device supports it. | C | 3 |
| @F | Be a renderer | Optional: accept playback from other LAN devices. Off by default; every sender must be approved. | C | 3 |
| @F | Cast indicator | Header badge with the device name while a listener is open; one click stops and closes the port. | M | 3 |

### 4.11 Library

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| @F | Library folders | Folders granted through the portal, remembered. Scanned by the index worker; changes picked up by file events. | S | 2 |
| @F | NFO metadata | Reads Kodi-style `movie.nfo`, `tvshow.nfo`, episode `.nfo` (XML): title, year, plot, cast, genres, ratings, ids, set. Parsed in the metadata helper. | S | 2 |
| @F | Local artwork | `poster.jpg`, `fanart.jpg`, `folder.jpg`, `<name>-thumb.jpg`, embedded cover attachments. | S | 2 |
| @F | File-name parsing | Title, year, season, episode from common naming patterns when no NFO exists. | S | 2 |
| @F | Views | Films, Series (show ▸ season ▸ episode), Home videos (by date), Folders, Recently added, Continue watching, Unwatched. | S | 2 |
| @F | Sort and filter | By title, year, added, duration, resolution, codec, HDR, audio languages, subtitle languages, watched. | S | 2 |
| @F | Watched state | Automatic at 90%; manual toggle; per profile; export and import as JSON. | S | 2 |
| @F | NFO editor | Edit fields and write the NFO back (needs write grant on the folder). | C | 3 |
| @F | Collections and tags | User tags and collections stored in the database, not in the files. | C | 3 |
| @F | Search | Library search over titles, plots, cast and subtitle text index; contributes titles to `svc.search` (off in private session). | S | 2 |
| @F | Missing files | Entries on unmounted volumes are greyed with the volume name; never deleted automatically. | S | 2 |
| @F | Duplicate detection | Same content key in two places is shown once with "2 copies". | C | 3 |
| @F | No online scraping | The app never fetches metadata. A separate tool may write NFO files; this app only reads them. | M | 2 |

### 4.12 Security, privacy and trust

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| @F | Decode in zero-capability helpers | Demux, decode, subtitle and metadata parsing each run in a helper in capability mode with no file, network or device rights (section 6.5). | M | 1 |
| @F | Strict mode | For untrusted files: software decode only, no GPU decode, no font loading from the file, no linked segments, no playlist expansion, private session on. One click in the open menu; default for files with the "from network" provenance mark. | M | 1 |
| @F | Provenance badge | Shows where the file came from (downloaded, removable disk, local) using the provenance tag set by IN03/SY02. | S | 2 |
| @F | Helper crash recovery | A decoder crash shows "The decoder stopped. Retry in software / Skip 5 s / Stop" and is written to `svc.audit` with the stream position. | M | 1 |
| @F | Resource limits | Helpers run with memory, CPU-time and output-size limits; a file that exceeds them is stopped with a clear message. | M | 1 |
| @F | History controls | Clear all, clear one, pause history, exclude folders from history. | M | 1 |
| @F | Encrypted volumes | Thumbnails and positions for files on Vault (SE05) volumes are stored inside that volume's app-data area or not at all. | S | 2 |
| @F | Signature display | If a detached signature (`.sig`, minisign, OpenPGP) sits beside the file, verify through SE04 and show the result. | C | 3 |

### 4.13 Accessibility, automation and recovery

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| @F | Audio description | Prefers tracks flagged as visual-impaired when the setting is on. | S | 2 |
| @F | Caption preference | Prefers hearing-impaired subtitle tracks when the setting is on; larger default style in high contrast. | S | 2 |
| @F | Screen-reader transport | All controls named; position announced on demand (`Ctrl+I`), not continuously. | M | 1 |
| @F | Subtitles to screen reader | Optional live region that speaks or brailles cues. | C | 3 |
| @F | Media session | Publishes title, position, artwork (local only) and transport actions to `svc.media`; media keys work. | M | 1 |
| @F | Command line | `video [--strict] [--start=T] [--sub=H] [--private] <handles…>`; handles come from the launcher, not raw paths. | S | 1 |
| @F | Script interface | Bus interface `org.pbsd.video.Player1`: open, play, pause, seek, set track, get state. Callers need the user's approval per app. | S | 2 |
| @F | Key and pointer bindings | Every action rebindable; mouse wheel, side buttons and gestures configurable. | M | 1 |
| @F | Profiles | Named settings profiles (for example "Projector", "Headphones") switchable from the menu. | C | 3 |
| @F | Session restore | After a crash or reboot, reopens the last file paused at the last position. | M | 1 |
| @F | Printing a frame | `Ctrl+P` prints the current frame through the print portal. | C | 3 |

### 4.14 Disc playback (limits stated)

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| @F | Unencrypted DVD-Video | Plays `VIDEO_TS` folders, ISO images and discs without CSS: titles, chapters, angles, audio and subpicture tracks, simple menus. IFO and navigation are parsed in a helper. | S | 3 |
| @F | Unencrypted Blu-ray | Plays `BDMV` folders and images without AACS: playlists (`.mpls`), chapters, PGS. No BD-J menus (no Java runtime ships). | C | 3 |
| @F | Encrypted discs | Not supported. The system ships no CSS, AACS or BD+ circumvention code and no keys. The app says: "This disc is copy-protected. This system does not include software to play it." No download offer. | M | 3 |
| @F | Audio CD | Hands off to ME02. | S | 3 |
| @F | Optical drive access | Through a device portal for optical drives (ME03-GAP-3); read-only block access passed to the disc helper. | S | 3 |
| @F | Region code | The app never changes a drive's region setting. | M | 3 |
