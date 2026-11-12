<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Portals and Prompts

By Odin Loch

| | |
|---|---|
| ID | `SH04` |
| Bundle | `org.pbsd.portals` |
| Category | shell |
| Priority | P0 |
| Backing repos | FUSE (UI toolkit), FORGE (ForgeScript, sandbox, IPC), ParanoidBSD `pbsd/compositor/`, `docs/specs/HANDLES.md` |
| Status | Planned. Nothing built. |

## 1. Purpose

SH04 is how a confined app reaches anything outside its own data: the user picks it in a chooser that the system draws, and the app receives a handle to exactly that thing. It covers the file open, save and folder choosers, capability grant prompts, the share sheet, the open-with chooser, the print dialog, the screen and window picker, the colour picker, the account chooser and the device choosers. Every user meets it in every app. It replaces the KDE file dialog, `xdg-desktop-portal-kde` and the Plasma open-with and print dialogs of the current plan (`docs/plan/PBSD_PLAN.md`, P5-02).

An app never draws these itself. That is what makes "no ambient authority" (SI-2, `docs/security/INVARIANTS.md`) usable.

## 2. Scope

In scope:

- The user interface of `svc.portal.file`, `.print`, `.screen`, `.camera`, `.mic`, `.location`, `.usb` and of `svc.intent` (open-with, share), plus the account chooser for `svc.accounts` and the item prompt for `svc.keyring`.
- Capability grant prompts for every prompting capability in PLATFORM.md section 6.
- The portal broker: turns a user's choice into a narrowed handle and records the grant.
- The rules that stop apps imitating these surfaces (built on SH03's trusted path).
- The install-time capability review sheet used by SY05.

Out of scope:

| Item | Owner |
|---|---|
| Trusted layer, trust mark, secure attention key, elevation prompt | SH03 (SH04 uses them) |
| Reviewing and revoking grants later; lineage tree | AD05 |
| File management (copy, move, trash, bulk rename) | SY02 |
| Device back ends: camera, audio, location, USB stack | system services, VM05 |
| Capture pipeline and frames | compositor, ME05 |
| Privacy indicators and kill switches | SH02 |
| Printer set-up and queues; spooler | UT06 |
| Account set-up and credentials | SY01 accounts page, `svc.accounts` |
| Secret storage | SE04 |
| Metadata cleaning engine | SE05 |
| Nearby-device transfer engine | IN03 |
| Default-apps settings page | SY01 |

## 3. Users and top tasks

Personas:

- **Daily user**: opens and saves files all day and wants the chooser to be as quick as a file manager.
- **Careful user**: wants to see exactly what an app will get, and for how long.
- **Keyboard user**: types paths and names; never touches the pointer.
- **Person sharing a screen**: wants to share one window and nothing else.
- **Administrator**: sets policy for which grants may be remembered.

| # | Task | Target |
|---|---|---|
| T1 | Open a recent file | chooser opens on Recent; `Down`, `Enter` (2 steps) |
| T2 | Open a file by typing its path | `Ctrl+L`, type with completion, `Enter` |
| T3 | Save with a new name in the same folder | type name, `Enter` (2 steps) |
| T4 | Save in another format | pick format in the drop-down, `Enter` |
| T5 | Give an app a folder for this session only | pick folder, "This session", "Allow" (3 steps) |
| T6 | Allow the camera once | read, `Allow once` (1 click after the delay) |
| T7 | Deny and never be asked again | "Deny", tick "Do not ask again" (2 steps) |
| T8 | Share a file to an app | Share, pick target (2 steps) |
| T9 | Share a photo without location data | Share, tick "Remove metadata", pick target (3 steps) |
| T10 | Open a file with another app once | Open with, pick, `Enter` |
| T11 | Open an untrusted download safely | Open with, "In a disposable sandbox" (2 steps) |
| T12 | Print pages 2–5 double-sided | `Ctrl+P`, range, duplex, Print (4 steps) |
| T13 | Share one window in a call | picker opens, click window, "Share" (2 steps) |
| T14 | Pick a colour from the screen | click eyedropper, click pixel (2 steps) |
| T15 | Let a mail app use one account | account chooser, pick, "Allow" (2 steps) |

## 4. Features

### 4.1 File chooser: browsing
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH04-F001 | Out of process | The chooser runs in the portal UI agent, never in the app; the app sees no path listing, no thumbnails and no part of the file system | M | 1 |
| SH04-F002 | Attached to the caller | The chooser is tied to the requesting window by a window token; the compositor keeps it above that window and draws a tether (4.9) | M | 1 |
| SH04-F003 | Places sidebar | Recent, Home, Desktop, Documents, Downloads, Pictures, Music, Videos, bookmarks, tags, removable media, remote locations mounted by SY02, vaults (SE05), Trash (open mode only, read-only) | M | 1 |
| SH04-F004 | Path bar | Breadcrumbs; click a crumb to go up; `Ctrl+L` turns it into a text field with completion | M | 1 |
| SH04-F005 | Views | List (name, size, type, modified, tags), grid with thumbnails, compact; `Ctrl+1/2/3`; remembered per app and mode | M | 1 |
| SH04-F006 | Sorting | By name (natural order), size, type, modified, created; folders first option | M | 1 |
| SH04-F007 | Type-ahead | Typing selects the first match in the view; `Esc` clears | M | 1 |
| SH04-F008 | Search | `Ctrl+F`: search under the current folder by name; "content too" uses `svc.search`; results show their folder | M | 2 |
| SH04-F009 | Filters | Filter drop-down from the app's request (e.g. "Images: png, jpg") plus "All files"; matching by MIME type first, then extension | M | 1 |
| SH04-F010 | Hidden files | `Ctrl+H` toggles; off by default | M | 1 |
| SH04-F011 | Preview pane | `F4`: thumbnail or first page from `svc.thumbnail`, size, type, dates, tags, provenance, signature state | S | 2 |
| SH04-F012 | Quick look | `Space` shows a larger preview sheet; still rendered by `svc.thumbnail` in its sandbox | S | 2 |
| SH04-F013 | Multi-select | When the app asked for several: Shift and Ctrl selection, `Ctrl+A`, a count in the footer | M | 1 |
| SH04-F014 | Recent | Per-app recent files and folders first, then system-wide recents; entries whose file is gone are dimmed | M | 1 |
| SH04-F015 | Bookmarks | Add with `Ctrl+D`, reorder by drag, rename, remove; shared with SY02 | S | 1 |
| SH04-F016 | Tags | Filter by SY02 tags; tags shown as chips | C | 3 |
| SH04-F017 | Start folder | Order: folder of the app's current document (by handle), last folder this app used in this mode, the type's default folder, Home | M | 1 |
| SH04-F018 | Navigation keys | `Alt+Up` parent, `Alt+Left/Right` history, `Backspace` parent, `Enter` open folder, `Ctrl+Shift+N` new folder | M | 1 |
| SH04-F019 | Folder operations | New folder, rename (`F2`), move to trash (`Delete`); performed by SY02's file operation service with Undo; nothing else (no copy, no move) | M | 1 |
| SH04-F020 | Drop a file in | Dragging a file or folder from SY02 onto the chooser navigates to it and selects it | S | 2 |
| SH04-F021 | Paste a path | Pasting text that is a path navigates to it | S | 1 |
| SH04-F022 | Snapshots | "Earlier versions" lists ZFS snapshots of the current folder; a file from a snapshot opens read-only | S | 3 |
| SH04-F023 | Removable and remote | Devices appear when mounted by SY02; a slow location shows a spinner per row and never blocks the dialog | S | 2 |
| SH04-F024 | Locked locations | A locked vault or encrypted dataset shows a lock; opening it calls the owner's unlock flow (SE05) on the trusted path | S | 2 |
| SH04-F025 | Provenance column | Optional column: where a file came from (downloaded from host, received from contact, created here) and whether it is quarantined | S | 2 |
| SH04-F026 | Size and count limits | Views are virtualised; folders with 1,000,000 entries list incrementally; thumbnails load for visible rows only | M | 1 |
| SH04-F027 | Symbolic links | Shown with a mark and their target; a link pointing outside the granted area is resolved by the broker, not the app | M | 1 |
| SH04-F028 | Dangerous types mark | Executables, scripts and macros-capable documents carry a `--warn` mark and word in open mode | S | 2 |

### 4.2 File chooser: open, save, folder and grants
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH04-F029 | Open returns handles | For each chosen file the broker opens it and passes a descriptor with rights narrowed to what was asked (read, or read and write); the app never learns the full path unless the user allowed "show location" | M | 1 |
| SH04-F030 | Display name | The app receives a display name and a document id; the id lets it ask again for the same file while the grant lasts | M | 1 |
| SH04-F031 | Access mode shown | The footer says what the app gets: "Text Editor will be able to read and change this file" | M | 1 |
| SH04-F032 | Read-only option | Tick "Open read-only" to narrow a read-write request | M | 1 |
| SH04-F033 | Open a copy | "Give the app a copy" places a copy in the app's own data area; the original is untouched | S | 2 |
| SH04-F034 | Grant duration | Once (until the app closes the file), this session, or always (remembered for this app and file); default is session for files, once for folders; policy can cap it | M | 1 |
| SH04-F035 | Save: name field | Focused with the base name selected; extension follows the chosen format and can be shown or hidden | M | 1 |
| SH04-F036 | Save: formats | Format drop-down from the app's list; the app's open default format first (PLATFORM.md section 9) | M | 1 |
| SH04-F037 | Save: overwrite | Choosing an existing name asks "Replace report.odt?" inline, showing size and date of the old file; the old file goes to the undo journal (`svc.undo-trash`) | M | 1 |
| SH04-F038 | Save: name checks | Empty, too long, reserved characters and names differing only by case are reported under the field before the button enables | M | 1 |
| SH04-F039 | Save: atomic | The broker gives the app a descriptor to a temporary file in the target folder and renames it on commit; a crash never leaves a half file under the chosen name | M | 1 |
| SH04-F040 | Save: options area | App-declared typed options (check box, choice) shown under the name field, e.g. "Embed fonts"; data only, drawn by SH04 | S | 2 |
| SH04-F041 | Save: free space | Free space of the target shown; a warning when the app's size hint exceeds it | S | 2 |
| SH04-F042 | Save: encrypt | Optional "Encrypt with…" (passphrase or recipient keys) performed by SE05's engine after the app writes | C | 3 |
| SH04-F043 | Folder chooser | Picks a folder; footer says "will be able to read / read and change everything in Scans, including new files" with the item count and size | M | 1 |
| SH04-F044 | Folder scope warnings | Choosing Home, a whole disk or a folder holding keys (`.ssh`, keyring, vault files) shows a `--warn` explanation and needs a second confirmation; policy can forbid it | M | 1 |
| SH04-F045 | Folder sub-scope | Options: include subfolders (default yes), include hidden files (default no), read-only | S | 2 |
| SH04-F046 | Remembered grants | Stored in the grant store with app, object, rights, duration, time; listed and revocable in AD05; revocation closes the handle at once (SI-6) | M | 1 |
| SH04-F047 | Re-open without a prompt | An app with a live grant re-opens by document id through the broker; a moved or renamed file is followed by file id when the file system allows | M | 2 |
| SH04-F048 | Recent files per app | The app may list its own recent documents (names and ids only) through the portal | M | 1 |
| SH04-F049 | Drag and drop as a grant | Dropping a file on an app window makes a once-grant through the broker; the data offer carries a transfer token, not a path | M | 2 |
| SH04-F050 | Paste as a grant | Pasting copied files works the same way; the grant is read-only | S | 2 |
| SH04-F051 | Open in disposable sandbox | In open mode for files with unknown or quarantined provenance: "Open in a disposable copy of the app" (the app declares support) | S | 2 |
| SH04-F052 | Intent-driven open | Files opened from SY02, the launcher or another app arrive through the same broker and grant path; no chooser is shown | M | 1 |
| SH04-F053 | Show location | Optional per grant: the app may see the real path (needed by IDE, Terminal); shown as a tick box only when the app asks for it | S | 2 |
| SH04-F054 | Audit | Every grant, denial and cancellation is written to `svc.audit` with app, object id, rights and duration | M | 1 |

### 4.3 Capability prompts
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH04-F055 | One prompt design | Title as a question, who asks (real identity), what is asked, the app's stated reason (boxed, marked as from the app), choices, trust mark | M | 1 |
| SH04-F056 | Prompting capabilities | `dev.camera`, `dev.mic`, `dev.screen`, `dev.location`, `dev.usb:<class>`, `dev.print` (first use), `dev.gpu.compute`, `bg.run`, `bg.autostart`, `ai.local`, `proc.exec` (first use), `net.user` (first use, informational), keyring item access, clipboard read without focus | M | 1 |
| SH04-F057 | Manifest gate | A capability not declared in the app's manifest is refused without a prompt and logged | M | 1 |
| SH04-F058 | Choices | Deny; Allow once; Allow this session; Always allow. Which are offered depends on the capability and policy (screen capture never offers "always") | M | 1 |
| SH04-F059 | Safe default | No button is the default; `Esc` denies; Allow buttons enable after 800 ms (same rule as SH03-F075) | M | 1 |
| SH04-F060 | Per resource | The prompt names the resource (which camera, which USB device) and lets the user pick when several exist | M | 1 |
| SH04-F061 | Device preview | Camera prompt shows a live preview from the chosen camera inside the prompt (the app gets nothing until Allow); microphone prompt shows a level meter | S | 2 |
| SH04-F062 | Location precision | Exact, approximate (about 3 km), city, or a fixed place the user sets; the precision is part of the grant | S | 2 |
| SH04-F063 | User gesture rule | A request made within 5 s of user input in the app's focused window shows the prompt; any other request becomes a quiet request chip on the window's header (SH05) and a row in SH02's centre, with no modal prompt | M | 2 |
| SH04-F064 | Do not ask again | A tick box on Deny records a denial; the app gets an immediate refusal next time; AD05 can clear it | M | 1 |
| SH04-F065 | Rate limits | One prompt per app at a time; a denied capability cannot be asked again for 30 s; three denials in a session imply "do not ask again" for that session | M | 1 |
| SH04-F066 | Combined requests | Camera and microphone asked together show one prompt with two rows and separate choices | S | 2 |
| SH04-F067 | Kill switch awareness | If SH02's kill switch for the device is off, the prompt says so and offers "Turn the microphone on and allow"; turning on is a separate, explicit click | M | 2 |
| SH04-F068 | Background and autostart | `bg.run` prompt states what the app will do with no window (from a manifest string table key) and that a status item will show while it runs | S | 2 |
| SH04-F069 | Network first contact | `net.user` and `net.host` do not prompt per connection; the first contact with a new host is logged and shown as a row under the window's `[net]` badge; a setting can make first contacts ask | S | 3 |
| SH04-F070 | Route in prompts | Any prompt that leads to network use shows the app's route (clear, VPN, Tor) | S | 3 |
| SH04-F071 | Keyring item prompt | "Allow Mail to use the saved password for imap.example.org?" for items the app did not create; drawn here, decided by `svc.keyring` | S | 2 |
| SH04-F072 | USB device chooser | Lists matching devices (name, vendor, serial tail, class); the app gets one device handle; refused for classes not in its manifest | S | 2 |
| SH04-F073 | Explain link | "What does this allow?" opens a built-in text for that capability; the text is fixed by the system | M | 1 |
| SH04-F074 | Result to the app | The app receives granted, denied or cancelled, never the reason or whether "do not ask again" was ticked | M | 1 |
| SH04-F075 | Revocation notice | When a grant is revoked in AD05 or by a kill switch, the app gets a typed "revoked" event and its handles stop working at once | M | 1 |
| SH04-F076 | Install-time review | SY05 shows the manifest capabilities before install using SH04's review sheet: fixed paths, network endpoints, helpers, background use; grouped by risk with plain words | M | 1 |
| SH04-F077 | Update diff | On update, only added capabilities are shown; removals are listed quietly; an update that adds capabilities is never applied silently | M | 2 |
| SH04-F078 | Expiring grants | "Always" grants unused for 90 days expire (setting); the next use prompts again | S | 3 |
| SH04-F079 | Policy | An administrator policy can pre-grant, pre-deny or cap the duration per app and capability; prompts then say "Managed by your administrator" | S | 3 |

### 4.4 Share sheet
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH04-F080 | Open | An app emits the `share` intent with typed items (files by handle, text, URL, image); the sheet is drawn by SH04 | M | 2 |
| SH04-F081 | What is shared | Top area lists the items with thumbnails, names, sizes and types; items can be removed before sharing | M | 2 |
| SH04-F082 | App targets | Apps whose manifest handles `share` for these types, ordered by recent use; pinned targets first | M | 2 |
| SH04-F083 | Direct targets | Apps may publish data-only direct targets (a conversation, a notebook): label, icon, id; shown in a row; drawn by SH04 | S | 3 |
| SH04-F084 | Built-in actions | Copy, Save to file… (save chooser), Print, Send to a nearby device (IN03), Open with…, Add to archive (UT01) | M | 2 |
| SH04-F085 | Remove metadata | Tick box: strip EXIF, document author and similar through SE05's cleaner before the target receives a copy; on by default for images going to `net.*` apps | S | 2 |
| SH04-F086 | Convert | Optional: share as PDF, as PNG or JPEG at a chosen size, as plain text | C | 3 |
| SH04-F087 | Target gets a narrow grant | The target receives read-only handles to copies or originals, valid until it closes them; the sender learns only "shared" or "cancelled", not the target | M | 2 |
| SH04-F088 | Network warning | A target with network capability carries the `[net]` mark and its route; sharing a file from a vault to such a target asks once more | S | 2 |
| SH04-F089 | Share as link | Not built in (no vendor service). A target app may offer it | C | 3 |
| SH04-F090 | Keyboard | Arrows move, `Enter` shares, digits `1–9` pick a target | M | 2 |

### 4.5 Open-with chooser and defaults
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH04-F091 | When shown | No default for the type, the user asked "Open with…", the default is missing, or the file is quarantined | M | 1 |
| SH04-F092 | App list | Recommended (manifest lists the MIME type under `view` or `edit`), then other apps that accept any file; each row shows name, trust badge, and whether it will edit or only view | M | 1 |
| SH04-F093 | Type detection | MIME type from content sniffing in a sandboxed helper, compared with the extension; a mismatch is shown ("named .pdf but is a program") | M | 1 |
| SH04-F094 | Remember | "Always use for PDF documents" sets the default in `svc.intent`; changeable in SY01 | M | 1 |
| SH04-F095 | Options | Open read-only; open in a disposable sandbox; open a copy | M | 2 |
| SH04-F096 | URL schemes | Same chooser for `mailto:`, `magnet:`, `matrix:` and others; shows the full URL, the host in bold and the route | M | 2 |
| SH04-F097 | External open confirm | When an app opens a URL or file in another app, the first time per pair asks "Allow Mail to open links in Web Browser?" | S | 2 |
| SH04-F098 | No handler | "No installed app opens .xyz files. [Search Software] [Open as text] [Open in Hex Editor]" | M | 1 |
| SH04-F099 | Executables | Programs and scripts are never run from this chooser; the options are Terminal (SY03) with the command shown, or VM03 for foreign programs | M | 1 |
| SH04-F100 | Foreign apps | VM03 launchers appear with the personality name; opening passes a copy unless a grant is made | C | 3 |

### 4.6 Print dialog
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH04-F101 | Input | The app hands a PDF handle (the spool format, G-16) or asks for a paged callback; the dialog owns printer choice and options | M | 2 |
| SH04-F102 | Printers | List from UT06's service: name, state, location, colour or mono, duplex; "Save as PDF" and "Open in Document Viewer" are always present | M | 2 |
| SH04-F103 | Preview | Page thumbnails and a main preview rendered by the sandboxed PDF renderer; reflects range, n-up, scale and margins | M | 2 |
| SH04-F104 | Basic options | Copies, collate, page range (`1-3,7,10-`), odd or even, orientation, paper size, colour or grey, one or two sides (long or short edge) | M | 2 |
| SH04-F105 | Layout options | Pages per sheet 1, 2, 4, 6, 9, 16; order; scale (fit, actual, percent); margins; booklet | S | 2 |
| SH04-F106 | Quality and media | Resolution, tray, media type, borderless, when the printer reports them | S | 3 |
| SH04-F107 | Presets | Save and pick named option sets per printer | S | 3 |
| SH04-F108 | App options tab | App-declared typed options ("Print comments", "Selection only"); a change asks the app for a new PDF | S | 2 |
| SH04-F109 | Privacy | "Remove metadata" for PDF output; a note when the printer is on the network ("sent unencrypted" if so) | S | 3 |
| SH04-F110 | Held jobs | "Hold until released at the printer" with a PIN when supported | C | 3 |
| SH04-F111 | Result | The app gets "queued", "saved" or "cancelled"; job progress goes to SH02 | M | 2 |

### 4.7 Screen and window picker
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH04-F112 | Sources | Tabs: Windows, Screens, Region; live thumbnails come from the compositor to SH04 only | M | 2 |
| SH04-F113 | Window rows | Thumbnail, title, app name, security label (SH05); windows marked secret (SE04, vault content) and all trusted surfaces are not listed | M | 2 |
| SH04-F114 | Options | Include pointer; include system audio or this app's audio; hide notifications (on, SH02-F033); follow window when it moves | M | 2 |
| SH04-F115 | Duration | This capture only (default); "this session for this source"; never "always" for a screen. A restore token lets ME05 re-take a region without a new pick, within the session | M | 2 |
| SH04-F116 | Multiple sources | Pick several when the app allows; each is listed in SH02's indicator popover | S | 3 |
| SH04-F117 | Region | Drag a rectangle on a frozen frame; numeric size fields; snap to window edges | S | 2 |
| SH04-F118 | During capture | SH05 draws the capture outline; SH02 shows the indicator and Stop; the picker can be re-opened from the indicator to change source | M | 2 |
| SH04-F119 | Protected content | Windows that set a "no capture" content flag appear as black in screen captures; the picker warns "1 protected window will be hidden" | S | 3 |
| SH04-F120 | Remote control | A request to inject input (VM04 server) is a separate prompt, never bundled with capture, session-only | S | 3 |
| SH04-F121 | Single screenshot | A one-shot request shows the picker without the audio options; ME05 holds the only standing right to skip it for its own shortcuts | M | 2 |

### 4.8 Colour picker, account chooser and small pickers
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH04-F122 | Screen colour pick | Eyedropper mode: a magnifier loupe (11×11 pixels, ×12) follows the pointer; click returns one colour to the app; the app never receives pixels around it | S | 2 |
| SH04-F123 | Loupe details | Shows hex, RGB and the colour space of the output; arrow keys move one pixel; `Esc` cancels | S | 2 |
| SH04-F124 | Colour dialog | Optional full dialog around the FUSE `colorPicker`: wheel, sliders (RGB, HSL, OKLCH), hex field, system palette, recent colours, contrast check against a second colour | C | 3 |
| SH04-F125 | Account chooser | An app asks for an account of a kind (mail, calendar, contacts, chat, file sync); the user picks from `svc.accounts`; the app gets a scoped account handle, never the password | M | 2 |
| SH04-F126 | Account scope | The chooser states what the handle allows (e.g. "read and send mail for ada@example.org") and the duration; "Add account…" opens SY01 | M | 2 |
| SH04-F127 | Contact picker | An app without contacts access asks for one or more contacts; the user picks in a list from CO03; the app gets only the chosen fields | S | 3 |
| SH04-F128 | Font and emoji | Not portals: fonts are public system data; emoji input belongs to AX04 | M | 1 |
| SH04-F129 | Camera capture hand-off | "Take a photo" request opens ME04 in capture mode and returns one image; the app needs no camera capability | C | 3 |
| SH04-F130 | Scan hand-off | "Scan a document" opens UT06 and returns a PDF or images | C | 3 |

### 4.9 Trusted-path rules for portals
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH04-F131 | Trusted layer | Capability prompts, the screen picker, keyring and account prompts are drawn on SH03's trusted layer with the trust frame and trust mark (SH03-F087 to F091) | M | 1 |
| SH04-F132 | Choosers | File choosers, share, open-with and print are large and long-lived; they are system windows with a compositor-drawn "System" title strip and trust frame that no app can have, and show the trust mark in the footer | M | 1 |
| SH04-F133 | Tether | The compositor draws a short connector from the prompt or chooser to the header of the window that asked, and dims that window; a prompt with no tether says "requested in the background" | M | 2 |
| SH04-F134 | Real identity | Name, icon, trust badge and security label come from the requesting connection's credentials and the bundle signature, never from the request | M | 1 |
| SH04-F135 | App text is boxed | Reasons, titles, button labels and filter names from the app are length-limited, shown as plain text in a marked area, and can never replace system text or buttons | M | 1 |
| SH04-F136 | No nesting inside apps | Portal UI is never embedded in an app surface; an app cannot position, resize, restyle or screenshot it | M | 1 |
| SH04-F137 | Input rules | Input delay on allow buttons; no default allow; synthetic input refused (SH03-F089); the pointer cannot be warped onto a button (`…pointerwarp_v1` requests are ignored while a prompt is up) | M | 1 |
| SH04-F138 | Focus rules | A prompt takes keyboard focus only when the user gesture rule (F063) holds; otherwise it waits as a chip. Keys typed during the first 800 ms are dropped, not buffered | M | 1 |
| SH04-F139 | Secure attention check | `Ctrl+Alt+Delete` raises SH03's session dialog above everything; real portal prompts remain listed there ("1 request waiting: Camera for Messenger") | S | 2 |
| SH04-F140 | Not captured | Portal prompts are excluded from capture; file choosers are excluded by default (setting) | M | 2 |
| SH04-F141 | Look-alike detection | The compositor refuses app windows without decorations that cover more than 90% of an output unless they are fullscreen (which shows the SH05 fullscreen hint) | C | 3 |

### 4.10 Settings, accessibility, recovery
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH04-F142 | Settings | Schema in section 8, shown in SY01 under Privacy and Files | M | 1 |
| SH04-F143 | Screen reader | Every prompt reads: question, app, what, reason ("the app says: …"), choices; the chooser announces folder changes and selection counts | M | 1 |
| SH04-F144 | Keyboard complete | Every chooser and prompt is fully usable from the keyboard (5.5) | M | 1 |
| SH04-F145 | Agent crash | The broker survives a UI agent crash; an open chooser returns "cancelled" to the app; the agent restarts within 1 s | M | 1 |
| SH04-F146 | Broker crash | Handles already passed stay valid (they are kernel objects); remembered grants reload from the store; pending requests fail closed | M | 1 |
| SH04-F147 | Timeouts | A prompt with no answer for 5 min is cancelled; a chooser never times out | S | 1 |
| SH04-F148 | Help | `F1` opens built-in help text for the surface | S | 2 |
