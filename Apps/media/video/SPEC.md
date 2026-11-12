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
| ME03-F001 | Open file | From intent `open`, drop, `Ctrl+O` (file portal), command line handle. One window per file unless "reuse window" is set. | M | 1 |
| ME03-F002 | Open folder | Plays every playable file in name order as a temporary playlist. Natural sort (`ep2` before `ep10`). | M | 1 |
| ME03-F003 | Play, pause | `Space`, `K`, click on video. Pause shows the frame at full quality (no post-filter skip). | M | 1 |
| ME03-F004 | Stop | `Ctrl+.` releases the decoder and the file handle and shows the start view. | M | 1 |
| ME03-F005 | Seek bar | Slider with buffered ranges, chapter ticks, A-B markers, hover time and thumbnail preview. | M | 1 |
| ME03-F006 | Relative seek | `Left`/`Right` 5 s, `Shift` 1 s, `Ctrl` 60 s, `Up`/`Down` volume. Steps set in settings. | M | 1 |
| ME03-F007 | Exact and keyframe seek | Keyframe seek while dragging, exact seek on release. Setting to force either. | M | 1 |
| ME03-F008 | Go to time | `Ctrl+G` dialog accepts `h:mm:ss.mmm`, seconds, frame number, or percent. | S | 1 |
| ME03-F009 | Frame step | `.` forward one frame, `,` back one frame (decodes from the previous keyframe). Works while paused. | M | 1 |
| ME03-F010 | Playback speed | 0.1× to 8× in steps of 0.05; `[` `]` step, `Backspace` resets to 1×. | M | 1 |
| ME03-F011 | Pitch correction | Time-stretch (WSOLA, own code) keeps pitch from 0.5× to 4×; outside that range audio is muted or resampled by choice. | M | 1 |
| ME03-F012 | A-B loop | `L` sets A, then B, then clears. Loop count: forever or n times. Markers shown on the seek bar. | M | 1 |
| ME03-F013 | Loop file and loop playlist | Off, one, all. Shuffle for playlists. | S | 1 |
| ME03-F014 | Volume and mute | 0 to 100%, optional boost to 200% with a soft limiter. `M` mutes. Per-file volume is not remembered unless set. | M | 1 |
| ME03-F015 | Audio delay | `Ctrl++` / `Ctrl+-` in 10 ms steps, shown in the on-screen display, remembered per file and per output device. | M | 1 |
| ME03-F016 | A/V sync | Audio is the master clock; video frames are dropped or repeated; display-resample mode matches video to the display refresh. | M | 1 |
| ME03-F017 | Resume position | Stored per file content key (size + BLAKE3 of first and last 1 MiB). Resume if stopped between 2% and 95%. | M | 1 |
| ME03-F018 | Resume prompt policy | Always resume, ask, never. "Ask" is an inline bar, not a dialog. | S | 1 |
| ME03-F019 | Per-file memory | Remembers audio track, subtitle track, delays, aspect, zoom, speed per file. Can be turned off or cleared. | S | 2 |
| ME03-F020 | End-of-file action | Next in playlist, stop, close window, repeat, or run the session action "sleep" through `svc.power`. | S | 2 |
| ME03-F021 | Sleep inhibit | Holds a `svc.power` inhibitor only while playing video on a visible window. | M | 1 |
| ME03-F022 | Sleep timer | Stop after n minutes or at end of the current file. | C | 2 |
| ME03-F023 | Stream information | Inspector tab: container, codecs, profiles, bit rates, colour tags, HDR metadata, dropped frames, decoder in use. | M | 1 |
| ME03-F024 | Statistics overlay | `I` toggles a live overlay: frame times, drops, A/V offset, buffer, decode path. | S | 2 |

### 4.2 Video output
| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME03-F025 | Aspect ratio | Auto from stream, then presets 4:3, 16:9, 16:10, 1.85:1, 2.35:1, 2.39:1, custom. Key `A` cycles. | M | 1 |
| ME03-F026 | Zoom, pan, crop | Zoom 25% to 800%, pan with `Alt`+arrows, crop presets, automatic black-bar detection. | S | 2 |
| ME03-F027 | Rotate and flip | 90° steps and mirror; honours the container rotation tag. | S | 1 |
| ME03-F028 | Scaling filters | Bilinear, bicubic, Lanczos 3, plus sharper luma scalers on the GPU. Separate chroma upscaler. | M | 2 |
| ME03-F029 | Deinterlace | Off, auto (from stream flags), bob, yadif-style, motion-adaptive on GPU. Field order override. | M | 2 |
| ME03-F030 | Denoise | Temporal-spatial denoise, strength 0 to 10. Off by default. | C | 3 |
| ME03-F031 | Debanding | Gradient debanding with dither. Off by default. | C | 3 |
| ME03-F032 | Sharpen | Unsharp mask, 0 to 10. | C | 3 |
| ME03-F033 | Picture controls | Brightness, contrast, saturation, gamma, hue. Reset button. Not saved per file unless asked. | S | 2 |
| ME03-F034 | Neural upscaling | 2× and 4× upscalers run through `svc.model` (`ai.local`, `dev.gpu.compute`). Only offered when the model is installed (AI02). Falls back to Lanczos when the frame budget is missed for 2 s. | C | 3 |
| ME03-F035 | Frame interpolation | Motion interpolation to the display rate. Off by default. Local model or optical flow. | C | 3 |
| ME03-F036 | Dithering | Ordered or error-diffusion dither to the display bit depth. | S | 2 |
| ME03-F037 | Display refresh matching | Asks the compositor to switch refresh rate to a multiple of the video rate while full screen (needs ME03-GAP-2). | C | 3 |
| ME03-F038 | Full screen | `F11`, `F`, double-click. Controls hide after 2 s without pointer movement; pointer hides too. | M | 1 |
| ME03-F039 | Always on top, borderless | Window options passed to SH05. | S | 2 |
| ME03-F040 | Picture-in-picture | `Ctrl+Shift+P` asks SH05 for a PiP window: small, always on top, on every workspace, with play, seek ±10 s, close and "return" controls. | M | 2 |
| ME03-F041 | Multi-monitor | Choose which screen full screen uses; remembers per file type. | S | 2 |

### 4.3 Colour and HDR
| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME03-F042 | Colour-space handling | Reads matrix, primaries, transfer and range tags; guesses by resolution when untagged; override in the inspector. | M | 1 |
| ME03-F043 | Display colour management | Converts to the display ICC profile supplied by the compositor. 3D LUT built per profile and cached. | M | 2 |
| ME03-F044 | HDR tone mapping | PQ (SMPTE ST 2084) and HLG to SDR with BT.2390, Hable, Reinhard, clip. Per-scene peak detection. Target nits setting. | M | 2 |
| ME03-F045 | HDR passthrough | Sends HDR10 static metadata to an HDR display when the compositor supports it (ME03-GAP-1). | S | 3 |
| ME03-F046 | HDR10+ dynamic metadata | Parsed and used to guide tone mapping. | C | 3 |
| ME03-F047 | Dolby Vision | Profiles 5 and 8 tone mapped using the reshaping metadata where the format is publicly documented; otherwise the base layer is played and the inspector says so. | C | 3 |
| ME03-F048 | Gamut mapping | BT.2020 to display gamut: clip, perceptual, or desaturate. | S | 2 |
| ME03-F049 | HDR screenshot choice | Screenshots are saved tone-mapped (PNG) or as HDR (AVIF 10-bit, JPEG XL) by choice. | C | 3 |

### 4.4 Audio
| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME03-F050 | Audio track selection | Key `#` cycles; menu lists language, title, codec, channels, default and forced flags. | M | 1 |
| ME03-F051 | Preferred languages | Ordered list for audio and for subtitles. Rule: "subtitles only when audio is not in a preferred language". | M | 1 |
| ME03-F052 | Output device | Chosen through `svc.media`; follows the system default unless pinned. | M | 1 |
| ME03-F053 | Downmix and upmix | 7.1/5.1 to stereo with selectable centre and LFE gains; stereo stays stereo unless upmix is asked for. | M | 1 |
| ME03-F054 | Dynamic range compression | "Night mode" compressor; honours AC-3 DRC metadata. | S | 2 |
| ME03-F055 | Loudness normalisation | EBU R128 running normalisation to −23 LUFS, off by default. | S | 2 |
| ME03-F056 | Equaliser | 10-band graphic equaliser with presets, shared format with ME02. | C | 2 |
| ME03-F057 | Audio passthrough | AC-3, E-AC-3, DTS, DTS-HD, TrueHD sent unmodified as IEC 61937 to a receiver over HDMI or S/PDIF when `svc.media` reports support. Per-codec switches. Falls back to decode. | S | 2 |
| ME03-F058 | External audio file | Add an audio file as an extra track (drop with `Shift`, or Audio ▸ Add file). | S | 2 |
| ME03-F059 | Channel mapping check | Test tone per speaker in Preferences ▸ Audio. | C | 3 |
| ME03-F060 | Audio-only display | For audio files: cover art or a plain visualisation; suggests ME02 once. | S | 2 |

### 4.5 Subtitles
| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME03-F061 | Text subtitle formats | SRT, WebVTT, ASS/SSA, MicroDVD, SubViewer, SAMI, TTML, LRC. Embedded and external. | M | 1 |
| ME03-F062 | Bitmap subtitle formats | PGS (Blu-ray), VobSub (DVD), DVB subtitles. Scaled with the video. | M | 2 |
| ME03-F063 | Closed captions | CEA-608 and CEA-708 extracted from the video stream. | S | 2 |
| ME03-F064 | ASS rendering | Full ASS styling, positioning, karaoke, embedded fonts (fonts are parsed in the subtitle helper only). | M | 2 |
| ME03-F065 | Auto-load external subtitles | Same folder, same base name, also `Subs/` and `Subtitles/` subfolders. Needs a folder grant; otherwise an inline bar offers "Allow this folder". | M | 1 |
| ME03-F066 | Encoding detection | UTF-8, UTF-16 with BOM, then a statistical guess among legacy code pages; manual override; re-decodes live. | M | 1 |
| ME03-F067 | Subtitle track selection | `J` cycles, `Shift+J` back, `V` toggles visibility. | M | 1 |
| ME03-F068 | Two subtitles at once | Primary at the bottom, secondary at the top. For language learning. | S | 2 |
| ME03-F069 | Subtitle delay | `Z` / `X` in 50 ms steps; "sync to next line" (`Ctrl+Shift+Right`) snaps the next cue to now. | M | 1 |
| ME03-F070 | Subtitle speed fix | Rescale timing for 23.976 ↔ 25 fps mismatches with presets. | S | 2 |
| ME03-F071 | Styling override | Font, size, colour, outline, shadow, background box, position, margin. For text formats; "force over ASS" option. | M | 1 |
| ME03-F072 | Position in black bars | Draw subtitles in the letterbox area when there is one. | S | 2 |
| ME03-F073 | Subtitle list panel | Inspector tab listing every cue with time; click seeks; filter box searches the text. | S | 2 |
| ME03-F074 | Local subtitle search | Search subtitle files across granted library folders by file name, language tag and content hash. No online lookup. | S | 2 |
| ME03-F075 | Copy cue text | Click a cue in the panel and `Ctrl+C`; on-video selection in pause mode. | C | 3 |
| ME03-F076 | Local subtitle generation | Speech-to-text through `svc.model` (`ai.local`). Extracts audio, runs in the background, shows progress, writes a `.srt` or `.vtt` beside the file (portal save) or into app data. Language auto-detect or chosen. Marked "machine generated" in the track list. | S | 3 |
| ME03-F077 | Local subtitle translation | Translate an existing text track with a local model; output is a new track marked "machine translated". | C | 3 |
| ME03-F078 | Save edited timing | Export the current track with the applied delay and speed as SRT or WebVTT. | S | 2 |
| ME03-F079 | Forced subtitles | Honours the forced flag; "forced only" mode. | M | 1 |
| ME03-F080 | Subtitle sandboxing | Every subtitle and font file is parsed in the subtitle helper with no capabilities; the app receives cue records and glyph bitmaps only. | M | 1 |

### 4.6 Chapters, tracks and navigation
| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME03-F081 | Chapters | Read from Matroska, MP4, Ogg; `PgUp` / `PgDn` move between chapters; chapter list in the sidebar with thumbnails. | M | 1 |
| ME03-F082 | Editions | Matroska ordered chapters and editions selectable; linked segments are off unless the folder is granted. | C | 3 |
| ME03-F083 | Video track selection | For files with several video streams or angles. | S | 2 |
| ME03-F084 | Bookmarks | `B` adds a named bookmark with thumbnail; stored in app data, exportable as chapters (FFMETADATA text) or CSV. | S | 2 |
| ME03-F085 | Thumbnail seek preview | Generated on first open in the decode helper at 1 per 10 s, cached in `~/.cache`; uses embedded trick-play images when present. | S | 2 |
| ME03-F086 | Jump to next scene | `Ctrl+Right` uses a scene-cut detector over cached thumbnails. | C | 3 |
| ME03-F087 | Skip silence, skip intros | Optional local detectors; manual "skip 85 s" key (`Ctrl+Shift+Right` is rebindable). | C | 3 |

### 4.7 Playlists and queue
| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME03-F088 | Play queue | Sidebar list; reorder by drag; remove; "play next"; "add to end". | M | 1 |
| ME03-F089 | Playlist files | Read and write M3U, M3U8, PLS, XSPF. Entries resolve only inside granted folders or named hosts. | M | 1 |
| ME03-F090 | Next and previous | `N` / `P`, `Shift+N`. Auto-advance with an optional 5 s countdown. | M | 1 |
| ME03-F091 | Series auto-queue | Opening `S01E03` offers to queue the following episodes in the folder (needs folder grant). | S | 2 |
| ME03-F092 | Saved playlists | Named playlists stored in the library database. | S | 2 |
| ME03-F093 | Recently played | Start view grid with progress bars; can be turned off; "Clear history" in one click. | M | 1 |
| ME03-F094 | Private session | No history, no resume positions, no thumbnails written. Header shows a mask badge. | M | 1 |

### 4.8 Screenshots and clips
| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME03-F095 | Screenshot | `S` saves the frame at source resolution to the screenshots folder (default `~/Pictures/Video`, granted at first use). PNG, JPEG, WebP, AVIF, JPEG XL. | M | 1 |
| ME03-F096 | Screenshot with subtitles | `Shift+S` includes subtitles and filters as shown. | S | 1 |
| ME03-F097 | Copy frame | `Ctrl+C` puts the frame on the clipboard as PNG. | S | 1 |
| ME03-F098 | Screenshot name template | `{title}-{time}-{n}`; metadata written: source name optional, off by default. | S | 2 |
| ME03-F099 | Burst and contact sheet | n frames every m seconds; a contact sheet grid image with time codes. | C | 3 |
| ME03-F100 | Clip export, stream copy | Cut between A and B without re-encoding (keyframe-aligned, exact end) into MKV or MP4. | M | 2 |
| ME03-F101 | Clip export, re-encode | Frame-exact clip to WebM (VP9 or AV1 + Opus) or MP4 (AV1 + Opus/AAC). Optional subtitles burned in, optional scale. Runs in the encode helper. | S | 2 |
| ME03-F102 | Animated image export | GIF, animated WebP or AVIF from a selection up to 30 s, with frame-rate and width controls. | S | 3 |
| ME03-F103 | Audio extract | Save the selected audio track or selection as FLAC, Opus or WAV. | C | 3 |
| ME03-F104 | Hand-off to editor | "Edit in Video Editor" sends intent `edit` to CR02 with the file handle and A-B range. | S | 2 |
| ME03-F105 | Share | Share sheet (SH04) for the file, a clip or a screenshot. Metadata stripped from screenshots by default. | S | 2 |

### 4.9 Network streams
| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME03-F106 | Open location | `Ctrl+L`. Accepts `http(s)`, `rtsp(s)`, `rtmp(s)`, `srt`, `udp`/`rtp` (multicast needs ME03-GAP-4). Needs `net.user`; first contact with each host is prompted and logged. | M | 2 |
| ME03-F107 | HLS | Master and media playlists, fMP4 and MPEG-TS segments, AES-128 with keys from the same origin policy, live and VOD, alternate audio and subtitles. | M | 2 |
| ME03-F108 | MPEG-DASH | Static and dynamic MPD, SegmentTemplate/Timeline/Base, multi-period. No DRM. | S | 2 |
| ME03-F109 | RTSP | RTSP 1.0 over TCP interleaved by default; UDP by choice. Basic and digest auth, credentials in `svc.keyring`. For local cameras. | S | 2 |
| ME03-F110 | Progressive HTTP | Range requests, seek, resume after drop. | M | 2 |
| ME03-F111 | Adaptive quality | Auto by throughput and buffer; manual quality pick; cap by resolution. | M | 2 |
| ME03-F112 | Buffer control | Target buffer seconds, shown on the seek bar; low-latency mode for live. | S | 2 |
| ME03-F113 | Host allow-list per stream | A stream may only fetch from hosts listed in a visible "Hosts" popover; a new host needs approval (a playlist cannot pull the app to arbitrary hosts). | M | 2 |
| ME03-F114 | Route choice | Per stream: clear, VPN or Tor, within what `net.route` grants. Shown as a badge. | S | 2 |
| ME03-F115 | No silent requests | No artwork, metadata, subtitle or update lookups ever. The `[net]` badge lists every open endpoint. | M | 1 |
| ME03-F116 | Save stream list | Named streams stored locally (name, URL, route). Import and export as M3U. | S | 2 |
| ME03-F117 | Record stream | Save the incoming stream unmodified to a file the user picks. | C | 3 |
| ME03-F118 | Hand-off from other apps | Handles intent `open` for URLs from IN01, IN02 and CO05 after the user confirms the host. | S | 2 |

### 4.10 Local casting
| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME03-F119 | Discover renderers | UPnP/DLNA MediaRenderer discovery by SSDP on the local link. Runs only while the cast popover is open (needs ME03-GAP-4). | S | 3 |
| ME03-F120 | Cast file | Serves the file from a one-time HTTP listener bound to the LAN address, random path token, only reachable by the chosen device address; closed when casting stops. | S | 3 |
| ME03-F121 | Transcode for the device | When the renderer cannot play the format, re-encode on the fly in the encode helper to a profile the device lists. | C | 3 |
| ME03-F122 | Remote control | Play, pause, seek, volume, stop on the renderer; local window shows the position. | S | 3 |
| ME03-F123 | Cast subtitles | Burned in when transcoding; otherwise sent as an SRT side resource when the device supports it. | C | 3 |
| ME03-F124 | Be a renderer | Optional: accept playback from other LAN devices. Off by default; every sender must be approved. | C | 3 |
| ME03-F125 | Cast indicator | Header badge with the device name while a listener is open; one click stops and closes the port. | M | 3 |

### 4.11 Library
| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME03-F126 | Library folders | Folders granted through the portal, remembered. Scanned by the index worker; changes picked up by file events. | S | 2 |
| ME03-F127 | NFO metadata | Reads Kodi-style `movie.nfo`, `tvshow.nfo`, episode `.nfo` (XML): title, year, plot, cast, genres, ratings, ids, set. Parsed in the metadata helper. | S | 2 |
| ME03-F128 | Local artwork | `poster.jpg`, `fanart.jpg`, `folder.jpg`, `<name>-thumb.jpg`, embedded cover attachments. | S | 2 |
| ME03-F129 | File-name parsing | Title, year, season, episode from common naming patterns when no NFO exists. | S | 2 |
| ME03-F130 | Views | Films, Series (show ▸ season ▸ episode), Home videos (by date), Folders, Recently added, Continue watching, Unwatched. | S | 2 |
| ME03-F131 | Sort and filter | By title, year, added, duration, resolution, codec, HDR, audio languages, subtitle languages, watched. | S | 2 |
| ME03-F132 | Watched state | Automatic at 90%; manual toggle; per profile; export and import as JSON. | S | 2 |
| ME03-F133 | NFO editor | Edit fields and write the NFO back (needs write grant on the folder). | C | 3 |
| ME03-F134 | Collections and tags | User tags and collections stored in the database, not in the files. | C | 3 |
| ME03-F135 | Search | Library search over titles, plots, cast and subtitle text index; contributes titles to `svc.search` (off in private session). | S | 2 |
| ME03-F136 | Missing files | Entries on unmounted volumes are greyed with the volume name; never deleted automatically. | S | 2 |
| ME03-F137 | Duplicate detection | Same content key in two places is shown once with "2 copies". | C | 3 |
| ME03-F138 | No online scraping | The app never fetches metadata. A separate tool may write NFO files; this app only reads them. | M | 2 |

### 4.12 Security, privacy and trust
| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME03-F139 | Decode in zero-capability helpers | Demux, decode, subtitle and metadata parsing each run in a helper in capability mode with no file, network or device rights (section 6.5). | M | 1 |
| ME03-F140 | Strict mode | For untrusted files: software decode only, no GPU decode, no font loading from the file, no linked segments, no playlist expansion, private session on. One click in the open menu; default for files with the "from network" provenance mark. | M | 1 |
| ME03-F141 | Provenance badge | Shows where the file came from (downloaded, removable disk, local) using the provenance tag set by IN03/SY02. | S | 2 |
| ME03-F142 | Helper crash recovery | A decoder crash shows "The decoder stopped. Retry in software / Skip 5 s / Stop" and is written to `svc.audit` with the stream position. | M | 1 |
| ME03-F143 | Resource limits | Helpers run with memory, CPU-time and output-size limits; a file that exceeds them is stopped with a clear message. | M | 1 |
| ME03-F144 | History controls | Clear all, clear one, pause history, exclude folders from history. | M | 1 |
| ME03-F145 | Encrypted volumes | Thumbnails and positions for files on Vault (SE05) volumes are stored inside that volume's app-data area or not at all. | S | 2 |
| ME03-F146 | Signature display | If a detached signature (`.sig`, minisign, OpenPGP) sits beside the file, verify through SE04 and show the result. | C | 3 |

### 4.13 Accessibility, automation and recovery
| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME03-F147 | Audio description | Prefers tracks flagged as visual-impaired when the setting is on. | S | 2 |
| ME03-F148 | Caption preference | Prefers hearing-impaired subtitle tracks when the setting is on; larger default style in high contrast. | S | 2 |
| ME03-F149 | Screen-reader transport | All controls named; position announced on demand (`Ctrl+I`), not continuously. | M | 1 |
| ME03-F150 | Subtitles to screen reader | Optional live region that speaks or brailles cues. | C | 3 |
| ME03-F151 | Media session | Publishes title, position, artwork (local only) and transport actions to `svc.media`; media keys work. | M | 1 |
| ME03-F152 | Command line | `video [--strict] [--start=T] [--sub=H] [--private] <handles…>`; handles come from the launcher, not raw paths. | S | 1 |
| ME03-F153 | Script interface | Bus interface `org.pbsd.video.Player1`: open, play, pause, seek, set track, get state. Callers need the user's approval per app. | S | 2 |
| ME03-F154 | Key and pointer bindings | Every action rebindable; mouse wheel, side buttons and gestures configurable. | M | 1 |
| ME03-F155 | Profiles | Named settings profiles (for example "Projector", "Headphones") switchable from the menu. | C | 3 |
| ME03-F156 | Session restore | After a crash or reboot, reopens the last file paused at the last position. | M | 1 |
| ME03-F157 | Printing a frame | `Ctrl+P` prints the current frame through the print portal. | C | 3 |

### 4.14 Disc playback (limits stated)
| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| ME03-F158 | Unencrypted DVD-Video | Plays `VIDEO_TS` folders, ISO images and discs without CSS: titles, chapters, angles, audio and subpicture tracks, simple menus. IFO and navigation are parsed in a helper. | S | 3 |
| ME03-F159 | Unencrypted Blu-ray | Plays `BDMV` folders and images without AACS: playlists (`.mpls`), chapters, PGS. No BD-J menus (no Java runtime ships). | C | 3 |
| ME03-F160 | Encrypted discs | Not supported. The system ships no CSS, AACS or BD+ circumvention code and no keys. The app says: "This disc is copy-protected. This system does not include software to play it." No download offer. | M | 3 |
| ME03-F161 | Audio CD | Hands off to ME02. | S | 3 |
| ME03-F162 | Optical drive access | Through a device portal for optical drives (ME03-GAP-3); read-only block access passed to the disc helper. | S | 3 |
| ME03-F163 | Region code | The app never changes a drive's region setting. | M | 3 |

## 5. GUI

### 5.1 Window map

```
Video
├─ Player window (main; one per playing file unless "reuse window")
│  ├─ Header bar (auto-hides in full screen)
│  ├─ Sidebar (F9): Queue · Chapters · Bookmarks · Library shortcut
│  ├─ Video surface with on-screen display (OSD) and subtitle overlay
│  ├─ Transport bar (auto-hides in full screen)
│  ├─ Inspector (F4): Info · Tracks · Subtitles · Picture · Audio · Hosts
│  ├─ Inline bars: resume, folder grant, strict mode, decoder stopped, new host
│  └─ Popovers: volume, speed, tracks, cast devices, hosts, quality
├─ Start view (no file): Continue watching · Recent · Open · Library
├─ Library window (single instance)
│  ├─ Sidebar: Films · Series · Home videos · Folders · Playlists · Streams
│  ├─ Grid or table of items
│  └─ Item detail sheet (plot, cast, tracks, files, Play / Resume)
├─ Picture-in-picture window (type granted by SH05)
├─ Dialogs
│  ├─ Open location (Ctrl+L)
│  ├─ Go to time (Ctrl+G)
│  ├─ Export clip
│  ├─ Generate subtitles
│  ├─ Subtitle style
│  ├─ Animated image export
│  ├─ Contact sheet
│  ├─ NFO editor
│  └─ Add library folder (file portal, drawn by SH04)
├─ Preferences (Ctrl+,): General · Playback · Video · Colour and HDR · Audio ·
│     Subtitles · Network · Library · Privacy · Shortcuts · Advanced
└─ Shell-drawn: capability prompts, file portal, share sheet, print dialog (SH04)
```

### 5.2 Main window

Shown at 1100×660, wide breakpoint, sidebar and inspector open, a file playing.

```
+------------------------------------------------------------------------------------------+
| [=] Video — Lecture 04.mkv           [search Ctrl+K]    [strict][net][cast] [▁] [▢] [✕] |(1)
+----------------+------------------------------------------------------+------------------+
| Queue      (2) |                                                      | Info | Tracks (6)|
| > Lecture 04   |                                                      | Subs | Picture   |
|   Lecture 05   |                                                      +------------------+
|   Lecture 06   |                    VIDEO SURFACE                 (3) | Container  MKV   |
|----------------|                                                      | Video  AV1 10bit |
| Chapters       |                                                      |  3840×2160 23.976|
|  00:00 Intro   |                                                      |  PQ BT.2020      |
| >12:40 Proofs  |        ┌──────────────────────────────┐              | Decoder software |
|  31:05 Q and A |        │ subtitle line shown here  (4)│              | Audio  Opus 5.1  |
|----------------|        └──────────────────────────────┘              | Subs   ASS eng   |
| Bookmarks      |  1.25×  ·  sub delay +150 ms                     (5) | Dropped 0        |
|  14:02 lemma 2 |                                                      | A/V offset +2 ms |
+----------------+------------------------------------------------------+------------------+
| 12:58 [=====|====A~~~~B=======·=========·==============----------] 48:10        (7)      |
| [|<] [<5] [ ▶ ] [5>] [>|]  [1.25× v] [A-B]   [🔊 ====--] [aud v] [sub v] [cast] [PiP] [⛶] |(8)
+------------------------------------------------------------------------------------------+
```

1. Header bar. Badges: `[strict]` when strict mode is on (`--warn`), `[net]` lists open endpoints, `[cast]` while a cast listener is open, a mask badge in a private session.
2. Sidebar, 220 px, `listView` sections: Queue (drag to reorder), Chapters (click seeks), Bookmarks.
3. Video surface, `videoWidget` after G-11. Click toggles play, double-click full screen, wheel changes volume.
4. Subtitle overlay. Drawn from cue records and glyph bitmaps sent by the subtitle helper.
5. OSD line, 2 s, for every key action (speed, delay, track, volume). Reduced-motion: no fade.
6. Inspector, 280 px, `tabWidget` of `propertyGrid` pages. "Hosts" tab appears only for streams.
7. Seek bar, custom `slider`: played, buffered (`-`), chapter ticks (`·`), A and B markers, hover thumbnail.
8. Transport, `toolButton` row: previous, back 5 s, play, forward 5 s, next, speed `comboBox`, A-B, volume `slider`, audio track and subtitle track `comboBox`, cast, picture-in-picture, full screen.

States: empty (start view), loading (spinner on the surface with "Opening…"), buffering (percent in OSD), error (inline bar with Retry), offline (streams only, bar "No route to host"), strict, private, read-only (always: the app never writes to the media file).

### 5.3 Other views and dialogs

**Start view** (720×480, no file open)
```
+----------------------------------------------------------------------------------------+
| [=] Video                               [search Ctrl+K]                  [▁] [▢] [✕]  |
+----------------------------------------------------------------------------------------+
|  Continue watching                                             [Clear history]         |
|  +-----------+  +-----------+  +-----------+  +-----------+                            |
|  | thumb     |  | thumb     |  | thumb     |  | thumb     |                            |
|  | ▓▓▓▓░░░░  |  | ▓░░░░░░░  |  | ▓▓▓▓▓▓░░  |  | ▓▓░░░░░░  |                            |
|  +-----------+  +-----------+  +-----------+  +-----------+                            |
|  Lecture 04     Holiday 2025    Film A         Episode 3                               |
|                                                                                        |
|  [Open file… Ctrl+O]  [Open folder…]  [Open location… Ctrl+L]  [Library Ctrl+B]        |
|  Drop a file here.                                    [ ] Private session              |
+----------------------------------------------------------------------------------------+
```

`gridView` of recent items with progress. Empty state: "Nothing played yet. Open a file." Private session hides the grid.

**Library window** (1100×700)
```
+------------------------------------------------------------------------------------------+
| [=] Video — Library                    [search library____]              [▁] [▢] [✕]   |
+--------------+---------------------------------------------------------------------------+
| Films     42 | [Grid|Table]  Sort [Title v]  Filter [Unwatched] [HDR] [Has subtitles]    |
| Series     7 +---------------------------------------------------------------------------+
| Home videos  | +--------+ +--------+ +--------+ +--------+ +--------+ +--------+         |
| Folders      | | poster | | poster | | poster | | poster | | poster | | poster |         |
| Recently     | |        | |        | |    ✓   | |        | | ▓▓░░   | |        |         |
|  added       | +--------+ +--------+ +--------+ +--------+ +--------+ +--------+         |
| Continue     | Film A     Film B     Film C     Film D     Film E     Film F             |
|--------------| 2019       2021 HDR   2008       1994       2023       1977               |
| Playlists    |                                                                           |
| Streams      | (offline volume "nas-films": 12 items greyed)                             |
| [+ Folder]   |                                                                           |
+--------------+---------------------------------------------------------------------------+
| 42 films · 3 unwatched · index up to date                         no network used        |
+------------------------------------------------------------------------------------------+
```

`gridView` or `tableView` over `SqliteModel`. Item detail opens as a sheet with plot, cast, technical details and Play.

**Open location** (dialog, 560×300)
```
+--------------------------------------------------------------+
| Open a network stream                                        |
| Address [https://media.example.org/live/index.m3u8_________] |
| Route   (•) Clear   ( ) VPN   ( ) Tor                        |
| [ ] Remember as [__________]                                 |
| Hosts this stream may contact:                               |
|   media.example.org:443            (from the address)        |
| The first contact with each host is asked and logged.        |
|                                   [Cancel]  [Open stream]    |
+--------------------------------------------------------------+
```

Error states under the field: unsupported scheme, host denied, no route granted.

**Export clip** (dialog, 600×420)
```
+------------------------------------------------------------------+
| Export clip  12:40.000 → 14:02.480  (1 min 22 s)                 |
| Mode   (•) Copy streams (fast, starts on keyframe 12:38.125)     |
|        ( ) Re-encode (frame exact)                               |
| Format [MKV v]   Video [copy v]   Audio [track 1: Opus 5.1 v]    |
| Subtitles ( ) none  (•) keep as track  ( ) burn in               |
| Scale [source v]     Quality [high v]   (re-encode only)         |
| [x] Strip metadata (title, chapters, attachments)                |
| Save to [Videos/clips/Lecture 04 - 12m40s.mkv] [Choose…]         |
| Estimated size 41 MB                                             |
|                                   [Cancel]  [Export clip]        |
+------------------------------------------------------------------+
```

Progress moves to the status area and to `svc.notify`; the job survives closing the window.

**Generate subtitles** (dialog, 560×360)
```
+--------------------------------------------------------------+
| Generate subtitles on this device                            |
| Model     [speech-to-text, medium v]   (installed, verified) |
| Audio     [track 1: English? Opus 5.1 v]                     |
| Language  [Detect v]      [ ] Translate to [English v]       |
| Output    (•) Beside the file (.srt)   ( ) App data only     |
| Estimated time 6 min for 48 min on this machine              |
| Nothing leaves this machine. Result is marked machine-made.  |
|                              [Cancel]  [Generate subtitles]  |
+--------------------------------------------------------------+
```

States: no model installed (button "Open Model Manager" → AI02), running (progress, cues appear live), failed.

**Subtitle style** (sheet in the inspector)
```
+----------------------------------+
| Font [system sans v]  Size [46]  |
| Colour [■]  Outline [■] [2.0]    |
| Shadow [1.0]   Box [ ] opacity   |
| Position [bottom v] Margin [5%]  |
| [ ] Force over ASS styles        |
| Preview: The quick brown fox     |
| [Reset]              [Done]      |
+----------------------------------+
```

**Cast popover** (320×260)
```
+--------------------------------------+
| Cast on the local network            |
| > Living room TV   192.168.1.40      |
|   Study speaker    (audio only)      |
| Searching… (discovery stops on close)|
| This opens one port on 192.168.1.12  |
| reachable only by the chosen device. |
| [Stop casting]                       |
+--------------------------------------+
```

**Decoder stopped** (inline bar): "The decoder stopped at 31:05. [Retry in software] [Skip 5 s] [Stop] [Details]".
**Picture-in-picture** (360×202): video surface only; on hover a strip `[<10] [▶] [10>] [return] [✕]`.
### 5.4 Menus

```
File      Open file… Ctrl+O · Open folder… Ctrl+Shift+O · Open location… Ctrl+L · Open disc… ·
          Open in strict mode… · Recent ▸ · Private session Ctrl+Shift+N · Library Ctrl+B ·
          Save playlist… Ctrl+S · Export clip… Ctrl+E · Share… · Print frame… Ctrl+P ·
          Close Ctrl+W · Quit Ctrl+Q
Playback  Play/Pause Space · Stop Ctrl+. · Previous P · Next N · Back 5 s Left · Forward 5 s Right ·
          Go to time… Ctrl+G · Frame back , · Frame forward . · Speed ▸ ([ ] Backspace) ·
          A-B loop L · Repeat ▸ · Shuffle · Chapters ▸ (PgUp PgDn) · Add bookmark B ·
          Sleep timer ▸ · At end ▸
Video     Track ▸ · Aspect ▸ A · Zoom ▸ · Rotate ▸ · Deinterlace ▸ D · Filters ▸ · Scaling ▸ ·
          HDR ▸ · Picture controls… · Screenshot S · Screenshot with subtitles Shift+S ·
          Copy frame Ctrl+C · Full screen F11 · Picture-in-picture Ctrl+Shift+P · Always on top
Audio     Track ▸ # · Add audio file… · Output device ▸ · Delay + Ctrl++ · Delay − Ctrl+- ·
          Channels ▸ · Night mode · Normalise loudness · Equaliser… · Passthrough ▸ · Mute M
Subtitles Track ▸ J · Second track ▸ · Show/Hide V · Add file… · Delay + X · Delay − Z ·
          Sync to next line Ctrl+Shift+Right · Speed fix ▸ · Style… · Encoding ▸ ·
          Find subtitles on this machine… · Generate… · Translate… · Export timing…
View      Sidebar F9 · Inspector F4 · Statistics I · Stream information Ctrl+I · Cast… ·
          Hosts… · Compact controls
Help      Help F1 · Shortcuts · Supported formats · About
```

### 5.5 Keyboard and pointer

App shortcuts are listed in 5.4 and in section 4. Single letters work only while the video surface has focus.

| Context | Menu items |
|---|---|
| Video surface | Play/Pause · Audio track ▸ · Subtitle track ▸ · Aspect ▸ · Screenshot · Copy frame · Set A / Set B · Add bookmark · Picture-in-picture · Stream information |
| Queue row | Play · Play next · Remove · Show in Files (SY02) · Open in strict mode · Properties |
| Seek bar | Go to time… · Set A here · Set B here · Add bookmark here · Copy time |
| Library item | Play · Resume · Mark watched · Add to playlist ▸ · Edit NFO… · Show in Files · Remove from library (files stay) |
| Subtitle cue | Seek to cue · Copy text · Sync this cue to now |

Drag and drop: media files onto the surface (play), onto the queue (add), with `Shift` (add as audio track); subtitle files (load); folders (queue); a URL as text (opens the Open location dialog pre-filled, never auto-connects). Drag out: the current frame as PNG from the surface with `Alt`+drag; queue rows as file handles.

Pointer: wheel = volume, `Ctrl`+wheel = zoom, `Shift`+wheel = seek, horizontal swipe = seek, middle click = full screen. Touch: tap = controls, double-tap left or right = ±10 s.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Start view. |
| Loading | Surface shows "Opening…" after 200 ms; cancel with `Esc`. |
| Buffering | OSD percent; seek bar shows the buffered range. |
| Error | Inline bar with cause in words ("No decoder for codec VC-1 is installed") and actions. |
| Offline | Streams only: bar "Cannot reach host"; local playback unaffected. |
| Locked | Files on a locked Vault volume show as greyed library items with a lock; opening asks SE05 to unlock. |
| Read-only | The media file is always opened read-only. |
| Narrow (<640) | Sidebar becomes an overlay, inspector a sheet, transport collapses to play, seek, volume, overflow `…`. |
| Strict | `[strict]` badge; hardware decode, fonts from file and playlist expansion disabled and greyed with a tooltip. |
| Private | Mask badge; history and resume controls disabled. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Video surface | `videoWidget` (Verified as a stub: `FUSE:Source/FUSE/UI/Widgets/include/fuse/ui/widgets/widgets.hpp:373-392`); needs G-11 |
| Seek bar | `slider` with custom paint layers |
| Transport | `toolButton`, `comboBox`, `slider` |
| Sidebar | `listView`, `treeView` (series) |
| Inspector | `tabWidget`, `propertyGrid`, `tableView` (cue list) |
| Library | `gridView`, `tableView`, `modelListView` over `SqliteModel` |
| Dialogs | `dialog`, `lineEdit`, `radioButton`, `checkBox`, `spinBox`, `colorPicker`, `fontPicker`, `progressBar` |
| Menus | `menuBar`, `popupMenu` |

Missing:

| Gap | What is missing |
|---|---|
| G-11 | Real pipeline behind `videoWidget` (section 6.5) |
| ME03-GAP-1 | HDR output path in the compositor (surface colour description, metadata) |
| ME03-GAP-2 | Compositor request for refresh-rate switching and variable refresh |
| ME03-GAP-3 | Optical drive device portal (`dev.usb:<class>` does not cover SATA drives) |
| ME03-GAP-4 | A capability for local-link discovery and LAN listeners (SSDP multicast, one-shot listener); proposed name `net.local` |
| ME03-GAP-5 | Picture-in-picture window type in SH05 |

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Player logic | ForgeScript | app host | Queue, state machine, settings, resume, key actions, view models |
| UI | FUSE UI | app host | Windows in 5.1 |
| Media session client | C++23 binding in the app host (platform, 6.5) | app host | Clock, A/V sync, frame queue, audio ring, render of frames to the surface |
| `demux-helper` | C++23 | helper, no capabilities | Container parsing; emits packets |
| `decode-helper` (one per active stream) | C++23 plus vendored codec libraries | helper, no capabilities | Packets in, frames or PCM out through shared memory |
| `subtitle-helper` | C++23 | helper, no capabilities | Subtitle and font parsing, shaping, rasterising to bitmaps |
| `metadata-helper` | C++23 | helper, no capabilities | NFO XML, playlist files, artwork decode |
| `encode-helper` | C++23 plus vendored encoders | helper, no capabilities | Clip export, cast transcode; writes to a passed descriptor |
| `net-fetch-helper` | C++23 | helper, sockets to approved hosts only | HTTP range, HLS, DASH, RTSP transport; passes bytes to the demux helper |
| `cast-helper` | C++23 | helper, LAN sockets only (ME03-GAP-4) | SSDP, UPnP AV control, one-time HTTP listener |
| `disc-helper` | C++23 | helper, one read-only device descriptor | DVD and Blu-ray navigation |
| Library index worker | ForgeScript | app host background task | Scans granted folders, feeds the database |
| Hardware decode broker | C++23 | system service (part of `svc.media`) | Owns the GPU decode device; see 6.5.4 |

```
            ┌──────────────── app host (ForgeScript + FUSE UI) ────────────────┐
 file handle│ player logic ── media session client ── videoWidget / audio ring │
 ───────────┼──────┬───────────────┬─────────────┬──────────────┬─────────────┘
            │ fd   │ packets (shm) │ frames (shm)│ cues, glyphs │ PCM ring
       ┌────▼────┐ │        ┌──────▼──────┐ ┌────▼─────┐   ┌────▼─────┐
       │ demux   ├─┘        │ decode      │ │ subtitle │   │ svc.media│→ audio device
       │ helper  │          │ helper(s)   │ │ helper   │   │ (mixer)  │
       └────▲────┘          └──────┬──────┘ └──────────┘   └──────────┘
            │ bytes                │ validated bitstream (optional)
       ┌────┴─────┐          ┌─────▼────────────┐
       │ net-fetch│          │ hardware decode  │→ GPU frames (dmabuf) → compositor
       │ helper   │          │ broker           │
       └──────────┘          └──────────────────┘
```

### 6.2 Data model

Stores: `~/.local/share/org.pbsd.video/library.sqlite` (schema version in `meta.schema_version`), `~/.config/org.pbsd.video/` through `svc.settings`, `~/.cache/org.pbsd.video/thumbs/`.

| Entity | Fields |
|---|---|
| `media_file` | `id INTEGER PK`, `content_key BLOB(32)`, `volume_id TEXT`, `rel_path TEXT`, `size INTEGER`, `mtime INTEGER`, `duration_ms INTEGER`, `container TEXT`, `video_codec TEXT`, `width INTEGER`, `height INTEGER`, `fps_num INTEGER`, `fps_den INTEGER`, `hdr TEXT`, `audio_langs TEXT`, `sub_langs TEXT`, `provenance TEXT`, `added_at INTEGER`, `missing INTEGER` |
| `title` | `id`, `kind TEXT (film\|show\|season\|episode\|home)`, `parent_id`, `name TEXT`, `sort_name TEXT`, `year INTEGER`, `season INTEGER`, `episode INTEGER`, `plot TEXT`, `genres TEXT`, `cast_json TEXT`, `nfo_path TEXT`, `poster_ref TEXT` |
| `title_file` | `title_id`, `file_id`, `part INTEGER` |
| `play_state` | `content_key BLOB PK`, `position_ms INTEGER`, `duration_ms INTEGER`, `watched INTEGER`, `last_played INTEGER`, `audio_track INTEGER`, `sub_track INTEGER`, `sub_delay_ms INTEGER`, `audio_delay_ms INTEGER`, `speed REAL`, `aspect TEXT`, `zoom REAL` |
| `bookmark` | `id`, `content_key`, `position_ms`, `label TEXT`, `thumb_ref TEXT`, `created_at` |
| `playlist` | `id`, `name TEXT`, `created_at`; `playlist_item(playlist_id, pos, file_id, url, title)` |
| `stream` | `id`, `name`, `url TEXT`, `route TEXT`, `hosts_json TEXT`, `keyring_ref TEXT` |
| `library_folder` | `id`, `grant_token TEXT`, `label`, `kind TEXT`, `last_scan INTEGER` |
| `tag` / `title_tag` | `id`, `name` / `title_id`, `tag_id` |

`content_key` = BLAKE3(size ‖ first 1 MiB ‖ last 1 MiB). No paths are stored for files played in a private session. Migration is forward only, with a copy of the old file kept as `library.sqlite.v<n>`.

### 6.3 File formats and protocols

The container and codec tables are in 6.5.2 and 6.5.3. App-specific formats:

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| SRT, WebVTT, MicroDVD, SubViewer, SAMI, LRC | yes | SRT, WebVTT | own C++23 | subtitle helper |
| ASS/SSA | yes | no | own C++23 renderer (libass, ISC, is the differential reference) | subtitle helper |
| TTML | yes | no | own C++23 XML subset parser | subtitle helper |
| PGS, VobSub, DVB subtitles, CEA-608/708 | yes | no | own C++23 | subtitle helper |
| Embedded fonts (TTF, OTF) | yes | no | FUSE text stack (FreeType in FULL profile, `FUSE:docs/ui/README.md`) | subtitle helper |
| M3U, M3U8, PLS, XSPF | yes | yes | own code | metadata helper |
| NFO (Kodi XML) | yes | yes | own XML subset parser, no entities, no DTD | metadata helper |
| HLS (RFC 8216) | yes | no | own code | net-fetch helper (playlist), demux helper (segments) |
| MPEG-DASH MPD (ISO/IEC 23009-1) | yes | no | own code | net-fetch helper |
| RTSP 1.0 (RFC 2326), RTP | yes | no | own code | net-fetch helper |
| HTTP/1.1, HTTP/2, TLS 1.3 | yes | — | FORGE network stack (FORGE-WP-21, Proposed) | net-fetch helper |
| UPnP AV / DLNA (SSDP, SOAP, DIDL-Lite) | yes | yes | own code | cast helper |
| DVD-Video (IFO, VOB, navigation) | yes | no | own code; no CSS | disc helper |
| Blu-ray (`index.bdmv`, `.mpls`, `.clpi`, M2TS) | yes | no | own code; no AACS, no BD-J | disc helper |
| Screenshot PNG, JPEG, WebP, AVIF, JPEG XL | — | yes | image codec helper (ME01 section 6.3) | encode helper |

### 6.4 Integration

- Intents handled: `open`, `view` for the MIME types in the manifest and for `http`, `https`, `rtsp`, `rtmp`, `srt` URLs (after host confirmation). Emits: `share`, `edit` (to CR02), `print`, `open` (audio-only files to ME02).
- Services: `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.intent`, `svc.search`, `svc.keyring`, `svc.clipboard`, `svc.thumbnail`, `svc.media`, `svc.power`, `svc.model`, `svc.audit`.
- Offers: bus interface `org.pbsd.video.Player1` (4.13) and a thumbnailer for video MIME types registered with `svc.thumbnail` (runs the demux and decode helpers; no app code).
- Other apps: SY02 (open, provenance), SH04 (prompts, share), SH05 (PiP, full screen), ME02 (audio hand-off, shared equaliser preset format), ME01 (image codec helper), CR02 (edit), AI02 (model install), SE04 (signature check), SE05 (encrypted volumes), IN03 (provenance mark), SY01 (default app, display HDR state).
- Backing repos, Verified: `videoWidget` pulls premultiplied RGBA8 frames by index from a function table on the UI thread; "Audio sync is not done" (`FUSE:Source/FUSE/UI/Widgets/include/fuse/ui/widgets/widgets.hpp:373-392`). A Theora source exists (`FUSE:Source/FUSE/UI/Video/include/fuse/ui/video/theora_video.hpp`). Everything else in this spec is Proposed.

### 6.5 The shared media pipeline (platform gap G-11)

Defined here once. ME01, ME02, ME04, ME05, ME06, CR02, CR03, CO05 and IN02 reference this section.

#### 6.5.1 What exists today (Verified)

| Fact | Source |
|---|---|
| `videoWidget` takes a `VideoSource` (`width`, `height`, `frames`, `fps`, a `frame(user, index, rgba)` callback) and paints RGBA8 through the rasteriser image. No audio, no A/V sync, no YUV, no GPU frames, no HDR. | `FUSE:Source/FUSE/UI/Widgets/include/fuse/ui/widgets/widgets.hpp:373-392` |
| `fuse_ui_video` decodes Ogg Theora held fully in memory with vendored libtheora 1.2.0 (BSD-3-Clause); other streams in the file are skipped; BT.601 conversion on the CPU. | `FUSE:Source/FUSE/UI/Video/include/fuse/ui/video/theora_video.hpp`, `FUSE:Source/FUSE/UI/Video/CMakeLists.txt` |
| `fuse_audio` is a game audio engine: a software spatial mixer feeding one output stream. Back ends: Null and OpenAL. Stereo float blocks through a lock-free ring. | `FUSE:Source/FUSE/Audio/include/fuse/audio/audio_backend.hpp` |
| Audio codecs in tree: whole-buffer and streaming Ogg Vorbis decode, Vorbis encode, FLAC decode and encode, from vendored libogg 1.3.6, libvorbis 1.3.7, libFLAC 1.5.0. | `FUSE:Source/FUSE/Audio/include/fuse/audio/audio_codec.hpp`, `FUSE:cmake/FuseXiph.cmake` |
| Engine pieces that could be reused: SPSC ring, bus gains, side-chain ducking, convolution reverb, HRTF panning. | `FUSE:Source/FUSE/Audio/include/fuse/audio/` |
| No audio capture call was found under `Source/FUSE/Audio`. No general demuxer, no video decoder other than Theora, no hardware decode, no subtitle code. | `FUSE:Source/FUSE/Audio/src/` (searched), `FUSE:Source/FUSE/UI/Video/` (3 files) |
| Vendored image code: `stb_image`, `stb_image_write`, `lpng`; also `libsndfile` 1.2.2 archive and `openal-soft`. | `FUSE:vendor/` |

Conclusion: there is no media pipeline. The decoders that exist run in the caller's process on whole in-memory files, which breaks the rule that hostile data is parsed in a helper. They can be moved into helpers.

#### 6.5.2 What the pipeline must provide (Proposed)

| Part | Requirement |
|---|---|
| Session API | ForgeScript module `media::` in the app host: `open(handle\|stream)`, `tracks()`, `select(track)`, `play/pause/seek(t, exact)`, `rate(r)`, `step(±1)`, `on(event)`, `snapshot()`, `close()`. One clock per session. Depends on G-02, G-15 style typed handles. |
| Demuxers | Own C++23, in `demux-helper`: Matroska/WebM, ISO BMFF (MP4, MOV, fMP4, CMAF), Ogg, MPEG-TS and M2TS, MPEG-PS (VOB), AVI, FLV, WAV/RF64, AIFF, FLAC native, raw ADTS, MP3 frames, DSF/DFF, CAF. Output: packet records `{stream, pts, dts, duration, flags, bytes}` in a shared-memory ring. |
| Decoders | One `decode-helper` process per active stream. No capabilities: it has a packet ring (read), a frame pool (write) and a control channel. No file descriptor to disk, no socket, no device. CPU-time, memory and frame-size limits set by the launcher (FORGE-WP-27). |
| Frame transport | Shared-memory frame pool (planar YUV 8/10/12-bit, or PCM float32). Zero copy into the UI: the surface samples YUV planes directly on the Vulkan path and converts on the CPU path. Needs FUSE-R-089 and FORGE-R-080. |
| A/V sync | Audio clock is master; video-only uses the monotonic clock; display-resample mode optional. Drift correction by resampling within ±0.5%. |
| Hardware decode | Through the hardware decode broker (6.5.4). Optional, off in strict mode. |
| Audio output | Through `svc.media`: the app gets a shared-memory PCM ring and a clock, never the device. Formats float32, 44.1–384 kHz, up to 7.1.4; exclusive bit-perfect mode and IEC 61937 passthrough on request (ME02). Needs FUSE-R-088. |
| Audio input | Through `svc.portal.mic` and `svc.media`: capture ring with indicator (ME04, ME06). System and per-app audio capture are `svc.media` monitor taps granted through the screen portal (ME05, ME06). |
| Audio processing | Resampler (band-limited sinc, own code), channel mixer, WSOLA time-stretch, loudness meter (EBU R128), limiter, 10-band equaliser, DRC. Own C++23, in the app host's audio thread (trusted math over PCM, no parsing). |
| Subtitles | `subtitle-helper` returns cue records and premultiplied glyph bitmaps with positions; the app composites. Needs FUSE-R-090. |
| Colour management | Every frame carries matrix, primaries, transfer, range, mastering metadata. Conversion to the display profile in the surface shader; ICC parsing in a helper; 3D LUT cache. Shared with ME01 and CR01. |
| HDR | PQ and HLG input; tone mapping to SDR (BT.2390 default); passthrough when the compositor supports it (ME03-GAP-1). |
| Encoders and muxers | `encode-helper`: AV1 (SVT-AV1 or rav1e), VP9 (libvpx), Opus, FLAC, Vorbis, PCM; muxers for Matroska/WebM, MP4, Ogg, WAV. Patent-restricted encoders only in the optional pack. |
| Capture sources | Camera frames from `svc.portal.camera` (ME04), screen frames from `svc.portal.screen` (ME05), as the same frame pool type. |
| Thumbnails | `svc.thumbnail` runs demux and decode helpers with a 2 s CPU limit. |
| Failure model | A helper crash is an event (`decoder_stopped`), never an app crash. The session can restart the helper at the next keyframe. |

#### 6.5.3 Codec and container support

Containers (all own C++23, parsed in `demux-helper`): see 6.5.2. Protocol demux for HLS and DASH is in 6.3.

| Codec | Decode | Encode | Proposed library | Licence | Patent note | Pack |
|---|---|---|---|---|---|---|
| AV1 | yes | yes | dav1d (decode), SVT-AV1 (encode) | BSD-2-Clause; BSD-3-Clause-Clear + AOM patent licence | AOM royalty-free licence | base |
| VP9, VP8 | yes | VP9 | libvpx | BSD-3-Clause + patent grant | royalty-free grant | base |
| Theora | yes | no | libtheora 1.2.0 (Verified vendored in FUSE) | BSD-3-Clause | royalty-free | base |
| H.264/AVC | yes | no | restricted libavcodec build (decoders only) | LGPL-2.1-or-later | Patent pool active in some countries; check per country | restricted |
| H.265/HEVC | yes | no | restricted libavcodec build | LGPL-2.1-or-later | Several active patent pools | restricted |
| H.266/VVC | later | no | restricted libavcodec build or vvdec (BSD-3-Clause-Clear) | as stated | Active patent pools | restricted, phase 3 |
| MPEG-2 video, MPEG-1 | yes | no | restricted libavcodec build | LGPL-2.1-or-later | Believed expired in most countries; verify | restricted until verified |
| MPEG-4 Part 2, VC-1, WMV | yes | no | restricted libavcodec build | LGPL-2.1-or-later | Some pools may be active; verify | restricted |
| ProRes, DNxHD, MJPEG, FFV1, HuffYUV, UT Video | yes | FFV1 (phase 3) | restricted libavcodec build; FFV1 and MJPEG candidates for own code | LGPL-2.1-or-later | FFV1 and MJPEG free; ProRes and DNxHD are vendor formats | base for FFV1/MJPEG, restricted for the rest |
| Opus | yes | yes | libopus | BSD-3-Clause | royalty-free grants | base |
| Vorbis | yes | yes | libvorbis 1.3.7 (Verified vendored) | BSD-3-Clause | royalty-free | base |
| FLAC | yes | yes | libFLAC 1.5.0 (Verified vendored) | BSD-3-Clause | royalty-free | base |
| MP3, MP2 | yes | no | own code or minimp3 (CC0) | CC0 | Believed expired (2017); verify | base |
| AAC-LC, HE-AAC | yes | no (optional pack: yes) | restricted libavcodec build | LGPL-2.1-or-later | AAC-LC largely expired; HE-AAC parts may be active | restricted |
| ALAC | yes | no | own code from the public specification | own | Apple published under Apache-2.0 | base |
| AC-3, E-AC-3 | yes, and passthrough | no | restricted libavcodec build | LGPL-2.1-or-later | AC-3 believed expired; E-AC-3 verify | restricted until verified |
| DTS, DTS-HD, TrueHD, MLP | yes, and passthrough | no | restricted libavcodec build | LGPL-2.1-or-later | Vendor licensing; verify | restricted |
| PCM, ADPCM, G.711, WavPack, TTA, Monkey's Audio | yes | PCM, WavPack | own code; WavPack library (BSD-3-Clause) | as stated | free | base |
| DSD (DSF, DFF) | yes (to PCM or DoP) | no | own code | own | free | base, phase 3 |

Rules:

1. The **base pack** contains only royalty-free codecs and ships in every image. The **restricted pack** is a separate signed package the owner or user installs after reading the notice; it holds decoders only unless stated. The system never downloads codecs on demand.
2. "restricted libavcodec build" means FFmpeg's `libavcodec` and `libavutil` configured with an explicit decoder allow-list, LGPL options only, no `libavformat`, no protocols, no devices, no filters, no assembly that needs W+X, built offline from a pinned archive. It is C, not C++23, so it runs only inside `decode-helper` with no capabilities. Proof tier T4 for the library, T3 for the wrapper.
3. Patent entries are engineering notes, not legal advice. Each must be checked for the countries of distribution before release (open question in section 13).
4. No DRM: no CSS, AACS, BD+, Widevine, PlayReady or FairPlay code or keys ship.
5. Every library is vendored as a pinned archive with a SHA-256, the way FUSE pins its Xiph archives (`FUSE:Source/FUSE/UI/Video/CMakeLists.txt`).

#### 6.5.4 Hardware decode

Hardware decode needs a device, and a helper with a device is not a no-capability helper. The split:

1. `decode-helper` (no capabilities) parses the bitstream headers and slice data, checks every syntax element against the profile limits, and emits a validated access unit plus parsed parameter sets.
2. The hardware decode broker (system service, part of `svc.media`) holds the render node. It accepts only validated access units, allocates surfaces with fixed sizes from the parsed sequence header, and returns frames as GPU buffers to the compositor path.
3. Interfaces considered: VA-API on the DRM render node and Vulkan Video. Which works on HardenedBSD with which GPUs is Unverified (G-18).
4. Driver bugs stay in the trusted base. Strict mode and files with a "from network" provenance mark default to software decode.

## 7. Security and privacy

| Capability | Why |
|---|---|
| `fs.app` | Library database, settings, thumbnails |
| `fs.portal` | Open media, subtitle files, library folders, save screenshots and clips |
| `net.user` | User-named streams (off until first use; per-host prompt) |
| `net.route:vpn`, `net.route:tor` | Optional route per stream |
| `proc.helper:demux-helper`, `decode-helper`, `subtitle-helper`, `metadata-helper`, `encode-helper`, `net-fetch-helper`, `cast-helper`, `disc-helper` | Section 6.1 |
| `dev.gpu.compute` | GPU scaling, tone mapping, neural upscaling |
| `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.intent`, `svc.search`, `svc.keyring`, `svc.clipboard`, `svc.thumbnail`, `svc.media`, `svc.power`, `svc.model`, `svc.audit` | Section 6.4 |
| `ai.local` | Subtitle generation, translation, upscaling |

Not requested: `dev.camera`, `dev.mic`, `dev.screen`, `dev.location`, `proc.exec`, `bg.autostart`, `fs.path:*`.

Threat model:

| Asset | Adversary | Attack surface | Mitigation |
|---|---|---|---|
| The user's files and session | Crafted media file | Demuxers, decoders, subtitle and font parsers | Zero-capability helpers, resource limits, own-code demuxers at T3, fuzzing, strict mode |
| GPU and kernel driver | Crafted bitstream | Hardware decode | Syntax validation before the broker; off in strict mode |
| Network position and identity | Hostile playlist or stream | HLS/DASH playlists naming other hosts, redirects, key URLs | Per-stream host allow-list, redirect to a new host needs approval, no cookies, no referrer, fixed user agent |
| Viewing history | Other local users, forensic reader | Database, thumbnails, recent lists | Private session, history controls, per-volume storage for encrypted volumes, nothing sent to `svc.search` in private mode |
| LAN exposure | Device on the LAN | Cast listener, SSDP replies | Listener bound to one address, random token, one peer, closed on stop; XML from devices parsed in the cast helper |
| Local model output | Prompt-like text in audio | Generated subtitles | Output is plain text cues only; never executed or used as commands |
| Stream credentials | Local malware in another app | RTSP passwords | Stored in `svc.keyring`, passed to the net-fetch helper only |

Stored: library database, play states, bookmarks, thumbnails, settings. Leaves the machine: nothing by default. With a stream: requests to the approved hosts only. With casting: the file bytes to the chosen device.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `playback.resume` | enum `always\|ask\|never` | `ask` | Resume policy |
| `playback.exact_seek` | enum `auto\|always\|never` | `auto` | Seek precision |
| `playback.pitch_correction` | bool | true | Keep pitch when speed changes |
| `video.hwdec` | enum `off\|auto\|on` | `auto` | Hardware decode (always off in strict mode) |
| `video.scaler` | enum | `lanczos3` | Luma scaling filter |
| `video.deinterlace` | enum `off\|auto\|bob\|adaptive` | `auto` | Deinterlacer |
| `colour.tone_map` | enum `bt2390\|hable\|reinhard\|clip` | `bt2390` | HDR to SDR curve |
| `colour.target_nits` | int | 203 | SDR reference white |
| `colour.hdr_passthrough` | bool | false | Send HDR to the display |
| `audio.languages` | list | system language | Preferred audio languages |
| `audio.downmix` | enum `auto\|stereo\|keep` | `auto` | Channel policy |
| `audio.normalise` | bool | false | EBU R128 running normalisation |
| `audio.passthrough` | set of codecs | empty | Bitstream passthrough |
| `subs.languages` | list | system language | Preferred subtitle languages |
| `subs.mode` | enum `off\|forced\|foreign_audio\|always` | `foreign_audio` | When to show subtitles |
| `subs.autoload` | bool | true | Look for external files in granted folders |
| `subs.style.*` | font, size, colours, margin | system sans, 46, white, 5% | Text style |
| `net.default_route` | enum `clear\|vpn\|tor` | `clear` | Default stream route |
| `library.folders` | list of grants | empty | Library folders |
| `privacy.history` | bool | true | Keep recent list and positions |
| `privacy.strict_for_downloads` | bool | true | Strict mode for files marked "from network" |
| `privacy.contribute_search` | bool | true | Send titles to `svc.search` |
| `screenshot.format` / `screenshot.folder` | enum / grant | `png` / `~/Pictures/Video` | Screenshot output |
| `cast.enabled` | bool | false | Allow discovery and casting |

## 9. Accessibility and languages

- Subtitles are an accessibility feature: default style meets 7:1 contrast with outline; a high-contrast preset adds an opaque box.
- Position, duration, chapter and track changes are announced on demand, never as a stream of updates.
- Every transport control has a name and state; the seek bar exposes value as time text.
- Optional live region reads subtitle cues to the screen reader (AX01) and braille.
- Full keyboard use, including the cue list and library grid; single-letter keys can be switched to `Ctrl`+letter for sticky-key users.
- Subtitle rendering needs the FULL text profile (G-03) for complex scripts, right-to-left and vertical text.
- Language tags follow BCP 47; names shown in the UI language.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first window frame | 250 ms |
| Open local file to first video frame | 400 ms (SSD, software decode, 1080p) |
| Seek to keyframe | 100 ms; exact seek 300 ms at 1080p |
| Dropped frames, 1080p60 AV1 software, 4 cores | under 0.1% |
| 2160p60 HDR with hardware decode | no drops; under 30% of one core in the app host |
| Idle when paused | 0 frames, 0 wake-ups |
| Memory, app host | under 120 MB plus frame pool (8 frames) |
| Decode helper memory limit | 1 GiB default; 4 GiB for 8K |
| Library | 100 000 files; search under 50 ms; scan 500 files/s on SSD |
| Largest file | No limit from the app; 64-bit offsets everywhere |
| Subtitle render | under 2 ms per frame for typical ASS; 8 ms cap, then cached |

## 11. Testing and proof

Nothing has run. All of this is the plan.

- Unit: ForgeScript tests for the state machine, queue, resume rules, file-name parser, language preference rules.
- Golden: `.fuseui` renders of every view in 5.2 and 5.3, in dark, light and high contrast, at 100% and 200%.
- Scripted UI: through the a11y tree: open, seek, track switch, subtitle load, clip export, strict mode.
- Conformance: decoder outputs compared by frame hash with the codec reference vectors (AV1 Argon or libaom vectors, VP9 vectors, FLAC and Opus test suites).
- Differential: demuxers against FFmpeg `ffprobe` packet lists; ASS renderer against libass images within a pixel tolerance; tone mapping against reference images.
- Fuzz: every demuxer, subtitle parser, NFO, playlist, MPD, M3U8, RTSP, SSDP and IFO parser; corpus kept in the repo; sanitizer builds.
- Sandbox tests: a helper build that tries `open`, `connect` and `ioctl` must be refused on HardenedBSD (depends on G-06).

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Matroska, MP4, Ogg, TS demuxers | No runtime errors on any input; bounded memory | T2 | PRISM on the C++23 subset; fuzzing | Size of the code; T1 needs a formal format model |
| Packet and frame ring protocol | No out-of-bounds, no data race | T2 | PRISM; model check of the SPSC protocol | — |
| SRT, WebVTT, playlist, NFO parsers | No runtime errors | T2 | PRISM; fuzzing | — |
| ASS renderer | Memory safety | T3 | Safe subset, sanitizers, fuzz, differential | Complexity |
| Bitstream validator for hardware decode | Accepts only in-profile syntax | T2 | PRISM; conformance vectors | T1 needs the standard formalised |
| Resampler, mixer, time-stretch | No overflow, bounded output | T2 | PRISM; property tests | — |
| Vendored codec libraries (C) | Contained, not proved | T4 | Zero-capability helper, limits, fuzzing | Third-party C |
| Host allow-list for streams | No request leaves the list | T1 | Small pure function; proof of the check plus a test that the helper has no other socket path | — |
| Player logic (ForgeScript) | Memory safe by language | T3 | Interpreter guarantees; unit tests | Interpreter itself is not proved |
| Cast listener | One peer, one token | T3 | Tests; fuzz | Network code size |

## 12. Build plan

### Phase 1: usable core

| ID | Work item | Depends on |
|---|---|---|
| ME03-W01 | Media session API and clock in the app host (6.5.2) | G-02, G-07, G-08, FORGE-WP-27, FORGE-WP-28 |
| ME03-W02 | Shared-memory packet ring and frame pool | FORGE-R-080 |
| ME03-W03 | `demux-helper` with Matroska/WebM, MP4, Ogg, WAV, FLAC | packet ring |
| ME03-W04 | `decode-helper` with base pack: AV1, VP9, VP8, Theora, Opus, Vorbis, FLAC, MP3, PCM | vendored archives |
| ME03-W05 | Video surface: YUV frames into `videoWidget`, CPU and Vulkan paths | G-11, FUSE-R-089 |
| ME03-W06 | Audio output ring through `svc.media`; resampler, mixer, time-stretch | G-09, FUSE-R-088 |
| ME03-W07 | Player window, transport, OSD, start view | G-01, G-02 |
| ME03-W08 | Text subtitles (SRT, WebVTT) in `subtitle-helper`; overlay | FUSE-R-090, G-03 |
| ME03-W09 | Resume, recent list, private session, strict mode | — |
| ME03-W10 | Screenshots; media session and media keys | `svc.media` |
| ME03-W11 | Sandbox refusal tests on HardenedBSD | G-06, G-18 |

### Phase 2: full feature

| ID | Work item | Depends on |
|---|---|---|
| ME03-W12 | Remaining demuxers (TS, PS, AVI, FLV) and restricted pack build | owner decision on patents |
| ME03-W13 | ASS renderer, bitmap subtitles, closed captions | G-03 |
| ME03-W14 | Colour management, tone mapping, scalers, deinterlace | `dev.gpu.compute` |
| ME03-W15 | Hardware decode broker and bitstream validators | G-18, `svc.media` |
| ME03-W16 | `net-fetch-helper`: HTTP, HLS, DASH, RTSP; host allow-list | FORGE-WP-21, SE02 |
| ME03-W17 | Library: index worker, NFO, views, search | `svc.search`, G-09 |
| ME03-W18 | Clip export (stream copy and re-encode) with `encode-helper` | — |
| ME03-W19 | Picture-in-picture | SH05, ME03-GAP-5 |
| ME03-W20 | Audio passthrough, loudness, equaliser | `svc.media`, ME02 |
| ME03-W21 | Thumbnailer for `svc.thumbnail` | G-09 |
| ME03-W22 | Bus interface `org.pbsd.video.Player1` | G-08 |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| ME03-W23 | Subtitle generation and translation | G-15, FORGE-WP-29, FORGE-WP-31, AI02 |
| ME03-W24 | Neural upscaling, frame interpolation | G-15, `dev.gpu.compute` |
| ME03-W25 | HDR passthrough, refresh matching | ME03-GAP-1, ME03-GAP-2 |
| ME03-W26 | Casting (`cast-helper`) | ME03-GAP-4 |
| ME03-W27 | Disc playback (`disc-helper`) | ME03-GAP-3 |
| ME03-W28 | Animated image export, contact sheet, audio extract | ME01 image codec helper |
| ME03-W29 | Dolby Vision and HDR10+ guided tone mapping | public documentation only |

## 13. Open questions and risks

1. **Patents.** Which codecs may ship in which countries is a legal decision. The restricted pack isolates the question; it does not answer it.
2. **libavcodec in a helper.** It is large C code. Containment is the only defence. Alternative: write own decoders for H.264 and AAC in C++23 over time (large effort).
3. **Hardware decode on HardenedBSD** is Unverified (G-18). Without it, 4K HEVC and AV1 playback relies on CPU speed.
4. **`svc.media` scope.** This spec assumes it owns the audio device, the mixer, capture taps and the hardware decode broker. That design is not written anywhere else yet.
5. **New capability** for LAN discovery and listeners (ME03-GAP-4). Without it casting cannot be expressed in PLATFORM.md section 6.
6. **HDR in the compositor** (ME03-GAP-1) does not exist in any plan read here.
7. **ASS compatibility.** An own renderer will differ from libass on edge cases; fansub files depend on exact behaviour. Fallback: vendor libass (ISC) with FreeType and HarfBuzz inside the subtitle helper.
8. **Disc playback** without decryption plays few commercial discs. That limit is stated in the UI rather than hidden.
9. **Dolby Vision** is only partly documented in public; support may stay at "base layer only".
