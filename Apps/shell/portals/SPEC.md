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

## 5. GUI

Two families of surface. **Choosers** (file, share, open-with, print, colour dialog) are system windows with a compositor-drawn "System" strip. **Prompts** (capability, keyring, account, USB, screen picker) are on SH03's trusted layer. Neither can be drawn by an app.

### 5.1 Window map
```
Portals and Prompts
├─ File chooser (system window, tethered to the requesting window)
│  ├─ Open mode · Save mode · Folder mode
│  ├─ Places sidebar · path bar · view · preview pane · footer (grant line)
│  ├─ Quick look sheet
│  ├─ Replace confirmation (inline) · folder scope warning (inline)
│  ├─ Earlier versions popover
│  └─ Context menus
├─ Capability prompt (trusted layer)
│  ├─ Device chooser row · preview / level meter
│  ├─ Location precision row
│  └─ "What does this allow?" sheet
├─ Request chip (drawn by SH05 in the window header; opens the prompt)
├─ Install-time review sheet (hosted in SY05)
├─ Share sheet (system window)
├─ Open-with chooser (system window)
├─ External-open confirmation (trusted layer)
├─ Print dialog (system window)
│  └─ Preset editor
├─ Screen and window picker (trusted layer)
│  └─ Region selection overlay (per output)
├─ Colour loupe (trusted layer overlay) · Colour dialog (system window)
├─ Account chooser · Contact picker · Keyring item prompt · USB device chooser (trusted layer)
└─ Remote-control prompt (trusted layer)
Settings: pages in SY01. No window.
```

### 5.2 Main window
File chooser in open mode, 920×600 by default (resizable, minimum 640×420), shown on the whole 1440×900 screen with the requesting window behind it.

```
+----------------------------------------------------------------------------------------+
| Text Editor — notes.md                                            [net] [▁] [▢] [✕]   |
| ░░░░░░░░░░░░░░░░░░░░░░░░ requesting window, dimmed ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ |
| ░░┃(1) tether                                                                          |
| +=╋====================================================================================+
| ‖ System · Open a file for Text Editor ✓                                     [✕]  (2) ‖
| ‖ [<][>][^]  Home ▸ Documents ▸ Projects ▸               [🔍 Search   ] [≣][▦][◫] (3) ‖
| ‖ +-------------+----------------------------------------------+-------------------+  ‖
| ‖ | Recent      | Name ▲            Size    Type      Modified |  report.md    (6) |  ‖
| ‖ | Home        | [▤] archive                 Folder  3 Nov    | +---------------+ |  ‖
| ‖ | Desktop     | [▤] drafts                  Folder  9 Nov    | | # Report      | |  ‖
| ‖ | Documents   | [≡] budget.csv     12 kB    CSV     Tue      | | Draft of the… | |  ‖
| ‖ | Downloads   |>[≡] report.md      48 kB    Markdown 14:02   | +---------------+ |  ‖
| ‖ | Pictures    | [≡] todo.md         2 kB    Markdown Wed     | Markdown · 48 kB  |  ‖
| ‖ |-------------| [≡] setup.sh ⚠      1 kB    Script  Mon      | Modified 14:02    |  ‖
| ‖ | ★ Projects  | [≡] letter.odt     31 kB    ODF text 1 Nov   | Created here      |  ‖
| ‖ | ★ Scans     |                                          (5) | Tags: work        |  ‖
| ‖ |-------------|                                              |                   |  ‖
| ‖ | ⏏ USB stick |                                              |                   |  ‖
| ‖ | 🔒 Vault    |                                              |                   |  ‖
| ‖ | nas (sftp)  |                                              |                   |  ‖
| ‖ +---------(4)-+----------------------------------------------+-------------------+  ‖
| ‖ Show [Text files (txt, md, csv) v]  [ ] Open read-only  [ ] Give the app a copy (7) ‖
| ‖ Text Editor will be able to read and change "report.md"  · Keep access [This session v] (8)‖
| ‖ [🦉] amber · lantern (9)                               [Cancel  Esc]   [Open] (10)  ‖
| +======================================================================================+
+----------------------------------------------------------------------------------------+
```

1. **Tether and dim.** Drawn by the compositor from the chooser's strip to the requester's header (F133).
2. **System strip.** Compositor-drawn. Text: the mode, the real app name and trust badge. Apps cannot have this strip.
3. **Toolbar.** Back, forward, up `toolButton`s; breadcrumb path bar (a row of `toolButton`s; `Ctrl+L` swaps in a `lineEdit` with completion); search `lineEdit`; view switch.
4. **Places sidebar.** `listView` with sections: places, bookmarks (★), devices and remote locations. 180 px. `F9` collapses.
5. **File view.** `tableView` (list), `gridView` (thumbnails) or `listView` (compact). `⚠` marks dangerous types (F028). Virtualised.
6. **Preview pane.** `image` from `svc.thumbnail`, read-only `propertyGrid`, provenance line. 220 px. `F4` toggles.
7. **Options row.** Filter `comboBox`, `checkBox`es for read-only and copy.
8. **Grant line.** `label` stating exactly what the app gets, and the duration `comboBox` (Once, This session, Always).
9. **Trust mark.** `image` and `label`.
10. **Buttons.** Cancel and the primary verb (Open, Save, Choose folder). The primary enables when the selection is valid.

Save mode adds a row above (7): `Name [report-final        ] . [md v]` with the format `comboBox`, the inline replace question, and app options (F040). Folder mode replaces (7) with the sub-scope options (F045) and (8) with the folder sentence (F043).

States: loading a folder (rows appear incrementally, spinner in the path bar); empty folder ("This folder is empty"); no match for the filter ("No text files here. [Show all files]"); location unavailable ("nas is not reachable. [Retry]"); locked location (lock icon and [Unlock…]); permission denied by the system ("You cannot read this folder"); read-only location in save mode (Save disabled with the reason); policy cap (duration options removed with "Managed"); narrow (< 640 px: sidebar becomes a `comboBox`, preview a sheet).

### 5.3 Other views and dialogs
#### 5.3.1 Capability prompt (520×auto on the trusted layer; whole 1440×900 screen shown)
```
+----------------------------------------------------------------------------------------+
|░░░░░░░░░░░░░░░░░░░░░░░░░░░░ everything else dimmed to 40% ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░|
|░░+------------------------------------+░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░|
|░░| Messenger — Ada            (1)     |░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░|
|░░+----------------┃-------------------+░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░+=======╋==================================================+░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ Allow Messenger to use the camera and microphone?    (2) ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ [M] Messenger ✓ · signed by system key · sandboxed · route: Tor (3)‖░░░░░░░|
|░░░░░░░░░░░‖ +------------------------------------------------------+ ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ | The app says: "To make a video call with Ada."    (4) | ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ +------------------------------------------------------+ ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ ◉ Camera      [Integrated camera        v]  +---------+  ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖   ( ) Deny (•) Once ( ) Session ( ) Always  | preview |  ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖                                             +---------+(5)‖░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ 🎙 Microphone  [Headset (USB)            v]  ▮▮▮▯▯▯       ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖   ( ) Deny (•) Once ( ) Session ( ) Always           (6) ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ ⚠ The camera kill switch is off. [Turn camera on]    (7) ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ An indicator shows while these are in use. You can stop  ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ at any time from the panel. [What does this allow?]  (8) ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ [🦉] amber · lantern   [ ] Do not ask again              ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖                         [Deny all  Esc]   [▓▓▒ Confirm] (9)‖░░░░░░░░░░░░░░░|
|░░░░░░░░░░░+==========================================================+░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░░░░░░░░░░░ Press Ctrl+Alt+Delete if you are unsure this is real ░░░░░░░░░░░░░|
+----------------------------------------------------------------------------------------+
```
1. The requesting window, the only thing not fully dimmed, with the tether.
2. Question built from system strings and the real app name.
3. Identity line: trust badge, signer, security label word (SH05 vocabulary), route.
4. The app's reason, boxed and prefixed "The app says:"; 200 characters; plain `label`.
5. Camera row: device `comboBox`, choice `radioButton`s, live preview (`viewportWidget` fed by the camera portal, 160×120). The app receives nothing yet.
6. Microphone row with a level meter (`progressBar`).
7. Kill switch notice (F067).
8. Fixed system text and the explain `link`.
9. Deny all (always enabled) and Confirm with the 800 ms fill. Confirm applies the per-row choices. For a single-capability prompt the buttons are `[Deny] [Allow once] [Allow this session] [Always allow]` instead of radio rows.

States: single or combined; no device present ("No camera found"); device busy; managed by policy (choices fixed, shown read-only); requester exited (closes); waiting as a chip (not shown); denied-again cooldown (not shown; app refused).

#### 5.3.2 Share sheet (560×auto system window)
```
+==============================================================================+
‖ System · Share from Photos ✓                                          [✕]    ‖
‖ +--------+ +--------+ 2 items · 7.4 MB                                 (1)   ‖
‖ | beach  | | dunes  | beach.jpg  4.1 MB   [✕]                                ‖
‖ +--------+ +--------+ dunes.jpg  3.3 MB   [✕]                                ‖
‖ [x] Remove metadata (location, camera, dates)   Send as [Original     v] (2) ‖
‖ People and places                                                      (3)   ‖
‖  (●) Ada · Messenger   (●) Family · Messenger   [≡] Trip notes · Notes       ‖
‖ Apps                                                                   (4)   ‖
‖  [1 Messenger net·Tor] [2 Mail net] [3 Notes] [4 Image Editor] [5 Vault] […] ‖
‖ Actions                                                                (5)   ‖
‖  [Copy] [Save to file…] [Print…] [Nearby device…] [Add to archive…] [Open with…]‖
‖ [🦉] amber · lantern                                         [Cancel  Esc]   ‖
+==============================================================================+
```
1. Items: `gridView` of thumbnails and a `listView` of names with remove buttons.
2. `checkBox` and `comboBox` (Original, Smaller image, PDF, Plain text; only those that apply).
3. Direct targets (F083), `gridView`; hidden when none.
4. App targets, `gridView`; `net` and the route shown as words on network-capable targets.
5. Built-in actions, `button`s.

States: no target for these types ("No app accepts these items. [Save to file…]"); converting (progress line); metadata cleaner unavailable (tick box disabled with the reason); vault-origin warning before a network target.

#### 5.3.3 Open-with chooser (480×auto system window)
```
+============================================================+
‖ System · Open "invoice-1182.pdf"                    [✕]    ‖
‖ PDF document · 214 kB · downloaded from mail.example.org   ‖
‖ ⚠ Quarantined: not opened since it was downloaded    (1)   ‖
‖ Recommended                                                ‖
‖ > [D] Document Viewer ✓      view, annotate          (2)   ‖
‖   [B] Web Browser ✓          view                          ‖
‖ Other apps                                                 ‖
‖   [H] Hex Editor ✓           view bytes                    ‖
‖   [T] Text Editor ✓          view as text                  ‖
‖ [x] In a disposable sandbox   [x] Read-only          (3)   ‖
‖ [ ] Always use Document Viewer for PDF documents     (4)   ‖
‖ [🦉] amber · lantern   [Search Software]  [Cancel] [Open]  ‖
+============================================================+
```
1. Type from content sniffing, size, provenance, quarantine state; a mismatch line when the name and content disagree (F093).
2. `modelListView` with sections.
3. Options; "disposable" is ticked by default for quarantined files.
4. Default-setting `checkBox`; disabled when "disposable" is ticked.

#### 5.3.4 Print dialog (880×620 system window)
```
+======================================================================================+
‖ System · Print from Writer ✓                                                 [✕]     ‖
‖ +-----------------------------------------+ +--------------------------------------+ ‖
‖ |                                         | | Printer [Office laser (ready)     v] | ‖
‖ |        +-----------------------+        | |         duplex · mono · network  (2) | ‖
‖ |        |                       |        | | [General] [Layout] [Quality] [Writer]| ‖
‖ |        |   page 2 preview      |        | | Copies  [ 1 ^v]  [x] Collate         | ‖
‖ |        |                       |        | | Pages   ( ) All (•) [2-5        ]    | ‖
‖ |        |                       |        | |         ( ) Current  ( ) Selection   | ‖
‖ |        +-----------------------+        | | Sides   [Two-sided, long edge   v]   | ‖
‖ |  [<]  page 2 of 12 · sheet 1 of 2  [>]  | | Paper   [A4                     v]   | ‖
‖ |  [▭][▭][▭][▭] thumbnails          (1)  | | Orient. (•) Portrait ( ) Landscape   | ‖
‖ +-----------------------------------------+ | Colour  [Greyscale              v]   | ‖
‖                                             | Preset  [Default v] [Save preset…](3)| ‖
‖                                             +--------------------------------------+ ‖
‖ 4 pages on 2 sheets                                                                  ‖
‖ [🦉] amber · lantern        [Save as PDF…]   [Cancel  Esc]   [Print]                 ‖
+======================================================================================+
```
1. Preview: `viewportWidget` showing bitmaps from the sandboxed PDF renderer; thumbnails in a horizontal `listView`.
2. Printer `comboBox` with a capability summary line.
3. `tabWidget`: General, Layout, Quality, and one tab named after the app for its typed options. Widgets: `spinBox`, `checkBox`, `radioButton`, `lineEdit`, `comboBox`.

States: no printers ("No printer set up. [Save as PDF…] [Add a printer]"); printer offline (`--warn`, Print still queues); preview rendering (skeleton page); the app is regenerating the PDF (preview dimmed); spool error.

#### 5.3.5 Screen and window picker (760×540, trusted layer)
```
+================================================================================+
‖ Choose what Messenger ✓ can see                                         (1)    ‖
‖ [Windows] [Screens] [Region]                                                   ‖
‖ +----------------+ +----------------+ +----------------+ +----------------+    ‖
‖ | (thumbnail)    | | (thumbnail)    | | (thumbnail)    | | (thumbnail)    |    ‖
‖ +----------------+ +----------------+ +----------------+ +----------------+    ‖
‖ >Slides — deck     Files — Scans      Terminal           Browser — docs  (2)   ‖
‖  native            native             exec               sandboxed · net       ‖
‖ 2 windows are not listed because they hold protected content.           (3)    ‖
‖ [x] Show pointer   [ ] Share sound from this window   [x] Hide notifications   ‖
‖ Keep access (•) Until I stop   ( ) This session for this window         (4)    ‖
‖ A red outline and a panel indicator show while sharing. [Stop] is in the panel.‖
‖ [🦉] amber · lantern                         [Cancel  Esc]   [▓▓▒ Share]        ‖
+================================================================================+
```
1. Question with the real app identity.
2. `gridView` of sources: live thumbnail (`image` from compositor buffers), title, security label word.
3. Protected-content notice (F113, F119).
4. Options and duration; no "always".

Region tab: closes the picker and shows a full-output overlay with a frozen frame, a drag rectangle with eight handles, a size readout and `[Cancel] [Use this region]`.

#### 5.3.6 Colour loupe, account chooser and USB chooser
```
   colour loupe (follows the pointer)       account chooser (480×auto, trusted layer)
   +-----------------------+          +====================================================+
   | ▓▓▓▓▓▒▒▒▒▒▒           |          ‖ Which account can Mail ✓ use?                      ‖
   | ▓▓▓▓▓▒▒▒▒▒▒           |          ‖ > [✉] ada@example.org        IMAP · SMTP           ‖
   | ▓▓▓▓[▣]▒▒▒▒  ×12      |          ‖   [✉] work@example.com       JMAP                  ‖
   | ▓▓▓▓▓▒▒▒▒▒▒           |          ‖   [+] Add an account…                              ‖
   +-----------------------+          ‖ Mail will be able to read and send mail for this  ‖
   | #4C8DFF  76 141 255   |          ‖ account. It will not see the password.            ‖
   | sRGB · click to pick  |          ‖ Keep access ( ) Session (•) Always                ‖
   +-----------------------+          ‖ [🦉] amber · lantern        [Deny  Esc] [▓▒ Allow] ‖
                                      +====================================================+
   USB chooser: same frame; rows "Arduino Uno · 2341:0043 · …9A1F · serial"; one row selectable.
```
The loupe is a trusted-layer overlay drawn from compositor pixels; it holds the pointer until click or `Esc`.

#### 5.3.7 Install-time review sheet (hosted in SY05, 560×auto)
```
+----------------------------------------------------------------+
| Demo Chat 1.2 ⚠ unverified publisher (key 9F3A…C210)           |
| This app asks for:                                             |
|  Always                                                        |
|   [net]  Connect to chat.example.net:443                       |
|   [▤]    Read and change ~/Chat (a fixed folder)        ⚠      |
|  Will ask when first used                                      |
|   [◉] Camera   [🎙] Microphone   [🔔] Notifications            |
|  Background                                                    |
|   Keeps running with no window; starts at login          ⚠     |
|  Not asked for: your files, other apps, location, USB          |
| New since 1.1:  + Read and change ~/Chat   + starts at login   |
| [Details…]                           [Cancel]   [Install]      |
+----------------------------------------------------------------+
```
A `treeView` grouped by when the capability applies; `--warn` rows for fixed paths, `proc.exec`, background use and unverified signers. The same component is shown by AD05 for an installed app.

### 5.4 Menus
No menu bar. Context menus (`popupMenu`):

| Menu | Items (shortcut) |
|---|---|
| File row (chooser) | Open / Choose (`Enter`) · Quick look (`Space`) · Rename (`F2`) · Move to trash (`Delete`) · Add to bookmarks (`Ctrl+D`) · Copy location (`Ctrl+Shift+C`) · Earlier versions… · Show in Files · Properties (`Alt+Enter`) |
| Empty area (chooser) | New folder (`Ctrl+Shift+N`) · Paste location (`Ctrl+V`) · Sort by ▸ · View ▸ · Show hidden files (`Ctrl+H`) · Refresh (`F5`) |
| Sidebar item | Open · Rename bookmark · Remove bookmark · Eject (devices) · Unlock / Lock (vaults) |
| Column header | Column chooser: Size, Type, Modified, Created, Tags, Provenance |
| Share target | Share · Pin / Unpin · Hide this target |
| Open-with row | Open · Open in a disposable sandbox · Set as default · Show permissions |
| Print preview | Zoom in (`Ctrl++`) · Zoom out (`Ctrl+-`) · Fit page (`Ctrl+0`) · First / Last page |
| Picker source | Share this · Show window (raises it briefly behind the picker) |

### 5.5 Keyboard and pointer
| Keys | Action |
|---|---|
| `Ctrl+L` / `/` or `~` typed in the view | Edit the path with completion |
| `Alt+Up`, `Backspace` / `Alt+Left`, `Alt+Right` | Parent / history |
| `Ctrl+1`, `Ctrl+2`, `Ctrl+3` | List, grid, compact |
| `Ctrl+F` / `Ctrl+H` / `F5` | Search / hidden files / refresh |
| `F4` / `F9` / `Space` | Preview pane / sidebar / quick look |
| `Ctrl+D` / `Ctrl+Shift+N` / `F2` / `Delete` | Bookmark / new folder / rename / trash |
| `Alt+N` / `Alt+T` | Focus the name field / the format or filter drop-down |
| `Alt+R` | Toggle read-only |
| `Enter` / `Esc` | Primary verb (after the delay on prompts) / cancel or deny |
| In prompts: `Alt+D`, `Alt+O`, `Alt+S`, `Alt+A` | Deny, once, session, always |
| In share sheet: `1–9` | Pick target n |
| In loupe: arrows, `Shift+arrows`, `Enter` | Move 1 px, 10 px, pick |
| In print: `PgUp` / `PgDn` | Previous or next preview page |
| In region overlay: arrows, `Shift+arrows` | Move, resize the rectangle by 1 px (`Ctrl` for 10 px) |

Pointer: double click opens; middle click on a folder does nothing (no tabs in the chooser); drag bookmarks to reorder; drag a file from SY02 into the chooser (F020). Nothing can be dragged out of a chooser or prompt. Rubber-band selection in multi-select grid view. Touch: long press for the context menu; pinch in the print preview.

### 5.6 States
| State | Behaviour |
|---|---|
| Empty | Empty folder, no targets, no printers, no accounts: one line and one action each, as listed per view |
| Loading | Incremental lists; thumbnails fill in; never a blocked dialog |
| Error | Inline message with Retry; a broker failure cancels the request and tells the app "cancelled" |
| Offline | Remote locations show "not reachable"; nearby-device action disabled |
| Locked | Vault and encrypted locations ask to unlock; while the session is locked no portal UI exists and pending requests are denied |
| Read-only | Save disabled in read-only locations; snapshot files open read-only |
| Policy | Fixed choices with "Managed by your administrator" |
| Narrow | Chooser collapses sidebar and preview; prompts keep 520 px or the output width minus 32 px |
| Guest / disposable | "Always" durations are not offered; recents are session-only |

### 5.7 Widgets
| Region | FUSE widget |
|---|---|
| File views | `tableView`, `gridView`, `listView` (models from the broker's listing service) |
| Sidebar, target and source lists | `listView`, `modelListView`, `gridView` |
| Review sheet | `treeView` |
| Path bar, toolbar | `toolButton`, `lineEdit` |
| Name, search, range fields | `lineEdit` |
| Filters, formats, durations, devices, printers | `comboBox` |
| Choices and options | `radioButton`, `checkBox`, `spinBox`, `toggleSwitch` |
| Preview images, thumbnails, trust mark | `image`, `viewportWidget` |
| Metadata | `propertyGrid` (read-only) |
| Print options | `tabWidget` |
| Level meter, input-delay fill, progress | `progressBar` |
| Colour dialog | `colorPicker` |
| Panes | `splitter`, `scrollArea`, `panel`, `separator` |
| Menus, tooltips | `popupMenu`, `tooltip` |

FUSE has an in-process `fileDialog` widget (PLATFORM.md V3). Confined apps must not use it: it would need ambient file access. The app host maps `ui::dialog` file requests to this portal (FUSE-WP-32, FORGE-WP-24). SH04 may reuse that widget's view code inside the portal UI agent.

Missing:

| Gap | What is missing | Where it lands |
|---|---|---|
| SH03-GAP-2 | Trusted layer, trust frame, capture exclusion | compositor (owned by SH03) |
| SH04-GAP-1 | Portal broker services and handle minting on PBSD handles with lineage | G-09, FORGE-WP-06, FORGE-WP-20 |
| SH04-GAP-2 | Window token so a system window or prompt can be tied to another client's window, the "System" strip and the tether. No `xdg_foreign` module is in `pbsd/compositor/`; `…wayland.xdg_dialog_v1.cppm` covers only modal hints within one client | compositor |
| SH04-GAP-3 | Compositor thumbnails and a capture source for the screen portal (same need as SH01-GAP-2) | compositor |
| SH04-GAP-4 | Pixel sampling and magnified loupe for the colour pick | compositor |
| SH04-GAP-5 | Grant store shared with AD05 (schema in 6.2) | AD05, G-09 |
| SH04-GAP-6 | Sandboxed PDF renderer for the print preview | G-16, OF03 helper |
| SH04-GAP-7 | Breadcrumb path bar widget; sectioned list headers; rubber-band selection in `gridView` | FUSE-WP-28, FUSE-WP-32 |
| SH04-GAP-8 | Transfer tokens in drag and drop and clipboard offers | FUSE-WP-09, compositor data device |

## 6. Architecture

### 6.1 Components
| Component | Language | Process | Responsibility |
|---|---|---|---|
| Portal broker | C++23 | `portal-broker`, session service | Receives requests, checks the manifest and policy, runs the decision state machine, opens files, narrows rights, passes descriptors, records grants, revokes |
| Listing service | C++23 | `portal-lister`, capability mode with directory descriptors | Lists directories, stats files, resolves links inside granted roots; gives the UI agent typed rows |
| Type sniffer | C++23 | `type-sniffer`, capability mode, read-only descriptor | Detects MIME type from the first 64 kB |
| Portal UI agent | ForgeScript + FUSE UI | app host with `shell.trusted-path` | All choosers and prompts; holds no file descriptors to user files |
| Review sheet component | ForgeScript + FUSE UI | loaded by SY05 and AD05 through an interface, drawn in the agent | Install-time and update review |
| PDF preview renderer | C++23 (G-16, shared with OF03) | helper, capability mode | Page bitmaps for the print dialog |
| Device portals (camera, mic, location, USB, screen) | C++23, platform | one service each | Own the devices; SH04 asks them to grant or revoke |
| Grant store | SQLite, owned by the broker | `portal-broker` | Durable grants and denials |

The broker and its helpers are native because they hold broad file access and parse untrusted bytes (PLATFORM.md section 2, a and c). The UI agent is the only part with complex UI code, and it has no authority of its own: it returns the user's choice to the broker as a typed record.

```
 app ──request(svc.portal.*, svc.intent)──▶ +----------- portal-broker (C++23) -----------+
  ▲                                          | manifest + policy check · decision machine  |
  │ handle (descriptor, narrowed rights)     | open + cap_rights_limit · grant store · audit|
  └──────────────────────────────────────────+----^-----------^--------------^-------------+
                                                  | choice     | rows         | grant/revoke
                           +----------------------+--+   +-----+--------+  +--v-------------+
                           | portal UI agent         |   | portal-lister|  | device portals |
                           | (ForgeScript + FUSE UI) |   | type-sniffer |  | camera mic usb |
                           | choosers and prompts    |   +--------------+  | location screen|
                           +----+---------------+----+                     +----------------+
                  trusted layer |               | thumbnails, preview
                        +-------v-----+   +-----v--------+   svc.thumbnail · PDF renderer
                        | compositor  |   | sandboxes    |   svc.accounts · svc.keyring · UT06
                        +-------------+   +--------------+
```

### 6.2 Data model
Grant store: `~/.local/share/org.pbsd.portals/grants.db` (SQLite, `PRAGMA user_version`), readable by the broker and, read-only through an interface, by AD05.

| Entity | Fields |
|---|---|
| `grant` | `id INTEGER PK`, `app_id TEXT`, `kind TEXT` (file, folder, camera, mic, screen, location, usb, print, gpu, bg, ai, exec, account, keyring_item, external_open), `object_id TEXT` (document id, device id, account id, target app), `rights INTEGER` (bit set: read, write, list, create, delete), `options TEXT` (JSON: precision, include_hidden, show_location), `duration INTEGER` (0 once, 1 session, 2 always), `session_id TEXT NULL`, `granted_at INTEGER`, `last_used INTEGER`, `expires_at INTEGER NULL`, `parent_handle TEXT` (lineage id), `source INTEGER` (0 user, 1 policy, 2 drag-drop, 3 intent) |
| `denial` | `app_id`, `kind`, `object_id NULL`, `sticky INTEGER`, `denied_at`, `count INTEGER` |
| `document` | `doc_id TEXT PK` (random 128-bit), `fs_id TEXT` (dataset guid + object number), `last_path TEXT`, `display_name TEXT`, `mime TEXT` |
| `recent` | `app_id`, `doc_id`, `mode INTEGER` (open, save), `used_at` |
| `chooser_state` | `app_id`, `mode`, `last_folder_doc_id`, `view INTEGER`, `sort_key`, `sort_desc`, `preview_on`, `width`, `height` |
| `default_handler` (owned by `svc.intent`, written through it) | `mime_or_scheme`, `app_id`, `set_at` |
| `PortalRequest` (memory) | `id`, `app_id`, `window_token?`, `input_serial?`, `kind`, `params`, `state: enum{received,checked,queued_chip,shown,granted,denied,cancelled}` |
| `PrintTicket` (to UT06) | `printer_id`, `copies: u16`, `collate`, `ranges: [(from,to)]`, `sides: enum`, `paper`, `orientation`, `colour`, `nup: u8`, `scale`, `margins_mm: [f32;4]`, `quality`, `tray`, `hold_pin?` |

Handles follow `docs/specs/HANDLES.md`: each granted handle is derived from the broker's handle with rights that only narrow (SI-1) and a recorded parent, so AD05 can show the lineage and revoke the subtree (SI-6). On FreeBSD-level descriptors the same narrowing is done with Capsicum rights limits (`docs/specs/CAPSICUM_LINEAGE.md`).

Bookmarks are shared with SY02 in `svc.settings` namespace `files.bookmarks`. Migration is forward only; the store is backed up as `grants.db.bak.<version>` before a schema change.

### 6.3 File formats and protocols
| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Portal request and reply records (bus schemas v1) | yes | yes | schema compiler (G-08); re-validated in the broker | `portal-broker` |
| Directory listings | yes | — | own code over `openat`, `fstatat`, `getdents` on directory descriptors | `portal-lister` |
| File content sniffing (magic numbers) | yes | — | own table-driven matcher, first 64 kB | `type-sniffer` |
| Thumbnails and previews of any file type | pixels only | — | `svc.thumbnail` | thumbnail sandbox |
| PDF (print preview, save as PDF) | yes (render) | pass-through | PDF renderer of G-16 | PDF helper |
| Manifest capability lists | typed records | — | parsed by the launch service or SY05 | not SH04 |
| Policy file (JSON, signed) | yes | — | own JSON reader with schema | `portal-broker` |
| Grant store (SQLite) | yes | yes | SQLite (public domain) | `portal-broker` |
| Wayland data offers with transfer tokens | yes | yes | FUSE data device (FUSE-WP-09) plus broker | broker resolves tokens |
| Print ticket | — | yes | typed bus record to UT06 | UT06 |
| Image metadata removal | — | — | SE05 cleaner | SE05 helper |

### 6.4 Integration
- Intents: SH04 resolves `open`, `edit`, `view`, `share`, `print`, `pick` and URL schemes on behalf of `svc.intent` when a choice by the user is needed. It emits `open` (to the chosen app) and `search` (to SY05 for "Search Software").
- Services provided (UI and broker side): `svc.portal.file` (`open_file`, `save_file`, `choose_folder`, `reopen(doc_id)`, `recent()`, `commit_save`), `svc.portal.print`, `svc.portal.screen` (`pick_source`, `screenshot`), `org.pbsd.portal.Prompt1` used by the device portals (`ask(app, capability, resource, reason) -> decision`), `org.pbsd.portal.Review1` (review sheet for SY05, AD05), `org.pbsd.portal.Share1`, `org.pbsd.portal.OpenWith1`, `org.pbsd.portal.Colour1`, `org.pbsd.portal.Account1`.
- Services used: `svc.settings`, `svc.thumbnail`, `svc.search`, `svc.audit`, `svc.undo-trash`, `svc.accounts`, `svc.keyring` (prompt only), `svc.intent`, `svc.a11y`, SY02's file operation service, UT06's print service, SE05's cleaner, SH02's `KillSwitch1` and `Indicators1`, SH03's trust mark through `shell.trusted-path`.
- Other apps: every app (as callers); SH01 (wallpaper pick, results drag), SH02 (Wi-Fi password prompt uses the keyring prompt frame), SH03 (trusted path; "Keep these files…"), SH05 (request chip, tether target, capture outline, security labels), SY02, SY05, AD05, SE04, SE05, IN03, ME04, ME05, UT01, UT06, CO03, VM03, VM04, VM05.
- Compositor (module files Verified present in `PBSD:pbsd/compositor/`): `…wayland.security_context.cppm` (`wp_security_context_manager_v1`: lets the compositor know a client is sandboxed and attach its app id, the basis for F134), `…wayland.xdg_dialog_v1.cppm`, `…wayland.xdg_activation.cppm` (input serials for the user gesture rule F063), `…wayland.data_device.cppm` and `…wayland.abstract_data_source.cppm` (drag and drop, F049), `…wayland.data_control.cppm` (`zwlr_data_control_manager_v1`, which must be restricted to AX05 and the shell or paste-as-grant can be bypassed), `…wayland.pointerwarp_v1.cppm` (F137), `…wayland.content_type.cppm`. Only headers were read. Not found: capture, cross-client parenting (SH04-GAP-2, -3).
- FORGE: FSS path-scoped policy and Capsicum mapping exist (PLATFORM.md V12, V13); mapping grants onto PBSD handles is planned as FORGE-WP-06 and file access through handles as FORGE-WP-20.

## 7. Security and privacy

Capabilities requested (UI agent bundle; the broker and helpers are session services with their own fixed rights):

| Capability | Why |
|---|---|
| `fs.app` | Chooser state, caches |
| `net.none` | No network use |
| `proc.helper:portal-lister`, `proc.helper:type-sniffer` | Listings and type detection without file access in the UI |
| `svc.settings`, `svc.thumbnail`, `svc.search`, `svc.audit`, `svc.undo-trash`, `svc.accounts`, `svc.intent`, `svc.a11y` | As in 6.4 |
| `svc.portal.file`, `.print`, `.screen`, `.camera`, `.mic`, `.location`, `.usb` (provider side) | Drawing their UI |
| `dev.camera`, `dev.mic` (preview only) | Live preview and level meter inside the prompt (F061); frames never leave the agent |
| `bg.run`, `bg.autostart` | Session component |
| `shell.trusted-path` (Proposed) | Draw prompts; read the trust mark |
| `shell.system-window` (Proposed) | Windows with the "System" strip and tether |

The broker holds the user's file authority (`fs.path:~:rw` in effect). It is the one place where that authority is narrowed and handed out. Not requested by anything in SH04: `net.*` beyond none, `proc.exec`, `ai.local`.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| The user's files | Confined app | Asks for a path directly, or reaches outside a granted folder with `..` or a link | Apps hold only descriptors; `openat` under capability mode cannot escape a directory descriptor; links resolved by the broker (F027) |
| The user's consent | Hostile app | Draws a fake chooser or prompt to learn habits or get a password | System strip, tether, trust mark, trusted layer (4.9); nothing in a portal ever asks for the login password |
| The user's consent | Hostile app | Misleading reason or title text | App text boxed and marked (F135); system text fixed |
| The user's consent | Hostile app | Clickjacking: prompt appears under a click or keypress | Gesture rule, input delay, no default allow, dropped early keys, pointer warp ignored (F137, F138) |
| The user's patience | Hostile app | Prompt flooding | Rate limits and chips (F063, F065) |
| Broker integrity | Any app | Malformed request, huge filter lists, path-like display names | Schema and bounds checks; display names sanitised; T2 target on the decoder |
| UI agent | Hostile file names, hostile files | Crafted names (control characters, right-to-left override) or files that exploit a decoder | Names rendered with control characters visible and direction isolated; no decoding in the agent; thumbnails from the sandbox |
| Grants over time | Updated app | Gains new capability silently | Update diff (F077); grants tied to the signing key and bundle id |
| Screen content | App with capture | Captures more than chosen | Capture source fixed by the picker; protected windows excluded; outline and indicator; session-only |
| Data in transit between apps | Share target | Receives hidden metadata | Remove-metadata option, on by default towards network apps |
| Clipboard and drag data | Background app | Reads file paths from offers | Transfer tokens instead of paths (F049); `data_control` restricted |
| Accounts and secrets | App | Gets the password | Scoped account handles; keyring items stay in `svc.keyring` |
| Location | App | Gets exact position when a city is enough | Precision choice is part of the grant (F062) |

Stored: grants, denials, document ids and last paths, recents, chooser state. Paths in the store are sensitive: the store is in the user's encrypted home and is cleared of session grants at logout. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `chooser.default_view` | enum{list,grid,compact} | list | F005 |
| `chooser.show_hidden` | bool | false | F010 |
| `chooser.folders_first` | bool | true | F006 |
| `chooser.preview_pane` | bool | false | F011 |
| `chooser.show_provenance` | bool | false | F025 |
| `chooser.start_in_recent` | bool | true | Open mode starts on Recent when the app has no current document |
| `chooser.exclude_from_capture` | bool | true | F140 |
| `grant.file_default` | enum{once,session,always} | session | F034 |
| `grant.folder_default` | enum{once,session,always} | once | F034 |
| `grant.max_duration` | enum{once,session,always} | always | Policy cap |
| `grant.expire_unused_days` | u16 (0 = never) | 90 | F078 |
| `grant.warn_broad_folders` | bool | true | F044; cannot be turned off under default policy |
| `prompt.input_delay_ms` | u16 (≥ 500) | 800 | F059 |
| `prompt.gesture_window_s` | u8 | 5 | F063 |
| `prompt.timeout_s` | u16 | 300 | F147 |
| `prompt.retry_cooldown_s` | u16 | 30 | F065 |
| `prompt.network_first_contact` | enum{log,ask} | log | F069 |
| `prompt.device_preview` | bool | true | F061 |
| `location.default_precision` | enum{exact,approximate,city,fixed} | approximate | F062 |
| `location.fixed_place` | {lat, lon, label} | none | F062 |
| `share.remove_metadata_default` | enum{never,network_targets,always} | network_targets | F085 |
| `share.pinned_targets` | list<app_id> | [] | F082 |
| `openwith.confirm_external_open` | bool | true | F097 |
| `openwith.disposable_for_quarantined` | bool | true | 5.3.3 |
| `print.default_printer` | string | "" | Last used if empty |
| `print.presets` | json | {} | F107 |
| `screen.hide_notifications` | bool | true | F114 |
| `screen.show_pointer` | bool | true | F114 |
| `recent.enabled` | bool | true | Recent lists |
| `recent.max` | u16 | 200 | Per app |

## 9. Accessibility and languages

- Prompts are announced as alert dialogs: question first, then the app, then "the app says" and its reason, then the choices. The input delay is announced as "available in a moment" and its end as "you can answer now".
- The file view is a table with row and column headers in the a11y tree; type-ahead announces the match; selection counts are a polite live region.
- The print preview exposes page text (from the PDF renderer's text layer) so screen-reader users can check what prints.
- The screen picker lists sources by title and app; thumbnails are not required to choose.
- The colour loupe speaks the colour name (nearest of 140 named colours) and hex on movement pauses.
- App-supplied strings are shown in the app's language; system strings in the system language; mixed-direction text is isolated per field.
- File names keep their script; sorting uses the locale's collation with natural number order.
- All drag actions have menu or key equivalents; rubber-band selection is never the only way to multi-select.

## 10. Performance budgets

| Item | Budget |
|---|---|
| Request to chooser first frame | ≤ 150 ms (agent kept warm); ≤ 400 ms cold |
| Request to prompt on screen | ≤ 150 ms |
| Folder with 10,000 entries: first rows | ≤ 50 ms; full sort ≤ 300 ms |
| Folder with 1,000,000 entries | Usable within 200 ms (incremental); memory ≤ 150 MB for the listing |
| Path completion | ≤ 30 ms per key |
| Handle delivery after "Open" | ≤ 20 ms per file; 1,000 files ≤ 2 s |
| Print preview first page | ≤ 300 ms for a 100-page PDF |
| Picker thumbnails | 5 frames per second per source, 320 px wide; 0 when the picker is closed |
| Idle | The agent has 0 frames and 0 wake-ups with no surface open; the broker wakes only on requests |
| Memory | Agent ≤ 60 MB idle; broker ≤ 15 MB plus 200 bytes per live grant |

## 11. Testing and proof

Test plan (targets; nothing has run):

- Unit: decision state machine (table-driven over capability × manifest × policy × prior grants × gesture), name validation, range parser (`1-3,7,10-`), duration and expiry rules, start-folder choice, MIME match and mismatch logic.
- Golden renders: every view in 5.2–5.3 in all themes, densities, RTL and scales; chooser in open, save and folder modes; prompts for each capability.
- Scripted UI through the a11y tree: tasks T1–T15 against a fake file tree and fake devices.
- Escape tests: a confined test app with a folder grant tries `..`, absolute paths, links out of the folder, hard links, renames across the boundary and descriptor passing to a helper; all must fail or stay inside.
- Spoofing tests: an app tries to create a window with the "System" strip, to cover a prompt, to capture it, to warp the pointer, to inject keys, to trigger a prompt with no gesture.
- Revocation tests: revoke during read, during write, during save commit; the handle must fail at once and no partial file may appear under the final name.
- Fuzz: request decoders, policy file reader, type sniffer, file-name rendering (random Unicode), print range parser, transfer-token resolver.
- Differential: type sniffer against the `file` utility's results on a corpus; listing service against `ls -la` on a generated tree.
- Fault injection: kill the agent with a chooser open; kill the broker with live grants; pull a USB stick while its folder is shown.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Decision state machine (broker) | A grant is issued only if the manifest declares the capability, policy allows it, and a user choice record exists for this request id; denial is the outcome of every other path | T1 | PRISM model check against the C++ | Policy format must be frozen |
| Rights narrowing | Rights on a passed handle are a subset of the requested rights and of the broker's own (SI-1) | T1 | Proof over the rights bit-set function; ties to the handle model in `docs/specs/HANDLES.md` | Kernel handle interface must exist |
| Revocation | After `revoke(grant)`, no descriptor derived from it remains usable (SI-6) | T1 at the handle layer; T3 here | Relies on the kernel property; integration tests here | Outside this component |
| Request decoders | No runtime error on any input | T2 | PRISM; fuzz | — |
| Listing service | Never follows a link out of its root; bounded memory per batch | T2 | PRISM on path handling; escape tests | — |
| Save commit | Final name appears only with complete content (rename after fsync) | T2 | Proof of the sequence; crash-injection tests | File system semantics assumed |
| Type sniffer | Total, bounded read (64 kB), no runtime error | T2 | PRISM; fuzz | — |
| Range parser | Accepts exactly the stated grammar | T2 | PRISM; unit | — |
| UI agent (ForgeScript) | Holds no file descriptor to user files; no runtime error in tests | T3/T4 | Capability audit of the agent's manifest and FSS policy; scripted UI | Interpreted code |

## 12. Build plan

Phase 1 — usable core:

| Item | Work | Depends on |
|---|---|---|
| SH04-W01 | Broker skeleton: request schemas, manifest check, decision machine, audit | G-08, G-07, SH04-GAP-1 |
| SH04-W02 | Listing service and type sniffer | G-06 |
| SH04-W03 | Grant store, document ids, recents; read interface for AD05 | SH04-GAP-5 |
| SH04-W04 | File chooser open mode: places, path bar, list view, filters, type-ahead, keyboard | G-01, FUSE-WP-28, SH04-W02 |
| SH04-W05 | Handle delivery with narrowed rights; read-only option; grant line and durations | FORGE-WP-06, FORGE-WP-20, SH04-W01 |
| SH04-W06 | Save mode: name checks, formats, replace, atomic commit | SH04-W05, `svc.undo-trash` |
| SH04-W07 | Folder mode with scope warnings | SH04-W05 |
| SH04-W08 | System-window strip, trust mark footer; prompts on the trusted layer | SH03-W07, SH03-W08, SH04-GAP-2 |
| SH04-W09 | Capability prompt: single capability, choices, input delay, deny memory, rate limits | SH04-W08, device portals |
| SH04-W10 | Open-with chooser, defaults, no-handler state, executables rule | `svc.intent`, SH04-W02 |
| SH04-W11 | Install-time review sheet for SY05 | SY05 |
| SH04-W12 | App host mapping of `ui::dialog` file calls to the portal | FUSE-WP-32, FORGE-WP-24 |
| SH04-W13 | Settings schema, a11y announcements, crash behaviour | SY01, G-04 |

Phase 2 — full feature:

| Item | Work | Depends on |
|---|---|---|
| SH04-W14 | Grid and compact views, preview pane, quick look, search, provenance column | `svc.thumbnail`, `svc.search` |
| SH04-W15 | Re-open by document id, follow renames, give-a-copy, show-location | SH04-W05 |
| SH04-W16 | Drag and drop and paste as grants (transfer tokens) | SH04-GAP-8 |
| SH04-W17 | Combined prompts, device preview, location precision, gesture rule and chips, kill switch awareness | SH05, SH02 |
| SH04-W18 | Share sheet with app targets, built-in actions, remove metadata | SE05, IN03, UT01 |
| SH04-W19 | Print dialog: printers, preview, basic and layout options, app options | G-16, UT06, SH04-GAP-6 |
| SH04-W20 | Screen and window picker, region overlay, restore tokens | SH04-GAP-3, SH05, ME05 |
| SH04-W21 | Colour loupe | SH04-GAP-4 |
| SH04-W22 | Account chooser, keyring item prompt, USB chooser | `svc.accounts`, SE04, VM05 |
| SH04-W23 | Tether, capture exclusion, secure-attention listing | SH04-GAP-2, SH03 |
| SH04-W24 | Disposable open, external-open confirm, URL schemes, update diff | SY05, launch service |
| SH04-W25 | Removable, remote and locked locations | SY02, SE05 |

Phase 3 — advanced:

| Item | Work | Depends on |
|---|---|---|
| SH04-W26 | Snapshots ("Earlier versions"), tags filter | SY02, ZFS |
| SH04-W27 | Direct share targets, convert on share, save-encrypted | CO05, OF02, SE05 |
| SH04-W28 | Print quality, presets, privacy, held jobs | UT06 |
| SH04-W29 | Multi-source capture, protected content, remote-control prompt | VM04, SH04-W20 |
| SH04-W30 | Colour dialog, contact picker, camera and scan hand-off | CO03, ME04, UT06 |
| SH04-W31 | Policy file, expiring grants, network first-contact ask mode, route in prompts | SE02, SE03 |
| SH04-W32 | Look-alike window refusal; foreign app handlers | compositor, VM03 |

## 13. Open questions and risks

1. **Paths versus handles.** Some tools (IDE, Terminal, build systems) need real paths. "Show location" (F053) and `fs.path` cover them, but every path given out weakens the model. Which apps may ask needs a rule.
2. **File chooser on the trusted layer or as a system window?** A chooser is large and long-lived; dimming the whole screen for it is heavy. The plan uses a system window with a compositor-drawn strip. This needs SH04-GAP-2 and a review of whether the strip is spoof-proof enough without the full dim.
3. **Gesture rule (F063)** needs reliable input serials from the compositor and will surprise apps that ask at start-up. Apps must be written to ask in response to a user action.
4. **Following renamed files** needs stable file ids; ZFS object numbers within a dataset should do, across datasets they do not.
5. **Camera preview in the prompt** means the agent opens the camera before consent to the app. The indicator must show "System prompt" as the user, and frames must never leave the agent.
6. **`zwlr_data_control_manager_v1`** is present in the compositor tree. Left open to all clients it lets any app read the clipboard, including file offers. It must be gated.
7. **Broker is a high-value target.** It holds the user's whole file authority. It must stay small; all UI and all decoding stay outside it.
8. **Print pipeline (G-16)** does not exist; the dialog cannot be finished before the spool format and UT06's service are fixed.
9. **Capture source (SH04-GAP-3)** is shared with SH01 and SH05; one compositor design should serve thumbnails, the picker and ME05.
10. **Remembered grants and app identity.** A grant is tied to bundle id plus signing key. Key rotation by a publisher needs a migration rule or users will be re-prompted for everything.
11. Risk: users trained by other systems to click "Allow". The input delay and chips reduce reflex approvals but do not remove them.
