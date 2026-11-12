<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Desktop Shell

By Odin Loch

| | |
|---|---|
| ID | `SH01` |
| Bundle | `org.pbsd.desktop-shell` |
| Category | shell |
| Priority | P0 |
| Backing repos | FUSE (UI toolkit), FORGE (ForgeScript), ParanoidBSD `pbsd/compositor/` (Wayland server side) |
| Status | Planned. Nothing built. |

## 1. Purpose

The Desktop Shell is the frame of the desktop: the panel, the launcher and app grid, the task bar, the desktop surface, system-wide search and the command runner. Every user sees it all day. It replaces the Plasma shell (`plasmashell`, KRunner, Kickoff) named in the current plan (`docs/plan/PBSD_PLAN.md`, P5-02) with a Qt-free shell that is an ordinary, confined Wayland client of the PBSD compositor.

## 2. Scope

In scope:

- Panels (any edge, any output), their layout and edit mode.
- Launcher: app grid, categories, favourites, recents, power row.
- Task bar: running windows, pinned apps, previews, grouping, badges.
- Desktop surface: wallpaper, optional icons, widgets.
- System-wide search UI on `svc.search`, and the command runner.
- Applet framework: the slot model, the applet API, first-party applets that are not owned by another shell app.
- Multi-monitor behaviour of all of the above.

Out of scope:

| Item | Owner |
|---|---|
| Notification centre, quick settings, status indicators, OSDs, calendar popover, battery, media controls | SH02 (hosted in SH01 panel slots) |
| Greeter, lock screen, authentication prompts, power and session dialogs | SH03 (SH01 only emits the request) |
| File chooser, capability prompts, share sheet, open-with | SH04 |
| Window decorations, snapping, tiling, workspaces logic, overview, alt-tab | SH05 (SH01 shows a workspace switcher applet and calls SH05) |
| Search index and ranking engine | `svc.search` (PLATFORM.md section 8) |
| Display arrangement, scaling, HDR settings | SY01 |
| Wallpaper slideshow source management beyond a folder | ME01 |
| Clipboard history UI | AX05 |
| On-screen keyboard, input method popups | AX03, AX04 |

## 3. Users and top tasks

Personas:

- **Daily user**: launches apps, switches windows, finds files. Wants it fast and predictable.
- **Power user**: lives on the keyboard, uses the runner as a calculator, converter and action palette, rearranges panels.
- **Security-minded user**: wants to see which sandbox owns each running app and to start things in a disposable sandbox.
- **Administrator**: locks the panel layout for a kiosk or shared machine.
- **Assistive-technology user**: drives the shell by screen reader or switch access.

| # | Task | Target |
|---|---|---|
| T1 | Launch an app by name | `Super`, type 2–3 letters, `Enter` (3 steps) |
| T2 | Switch to a running window from the task bar | 1 click or `Super+1..9` |
| T3 | Find a file by a word in its content | `Super`, type, `Down`, `Enter` (4 steps) |
| T4 | Run a command | `Alt+Space`, type, `Enter` |
| T5 | Calculate or convert a unit | `Alt+Space`, type `12 in to cm`, result shown, `Enter` copies |
| T6 | Pin an app to the task bar | right click, "Pin" (2 steps) |
| T7 | Open an app in a disposable sandbox | right click, "Open in disposable sandbox" (2 steps) |
| T8 | Start a second window of a running app | middle click or `Shift+click` |
| T9 | Add an applet to a panel | right click panel, "Add applet", pick, drop (4 steps) |
| T10 | Move a panel to another screen or edge | edit mode, drag (3 steps) |
| T11 | Change wallpaper | right click desktop, "Change wallpaper", pick (3 steps) |
| T12 | See what an app is allowed to do before launching it | hover or `Space` on a launcher tile (1 step) |
| T13 | Log out, restart, shut down | `Super`, power row, choose (3 steps); hands over to SH03 |
| T14 | Jump to a setting | `Super`, type "night light", `Enter` |

## 4. Features

### 4.1 Panels
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH01-F001 | Panel as layer surface | Each panel is one `zwlr_layer_surface_v1` on layer `Top`, anchored to one edge with an exclusive zone equal to its thickness | M | 1 |
| SH01-F002 | Edges | Top, bottom, left, right. Vertical panels rotate the layout, not the text | M | 1 |
| SH01-F003 | Multiple panels | Any number per output; default is one bottom panel, 40 px | M | 1 |
| SH01-F004 | Thickness | 28–72 px in steps of 4; icon size follows | M | 1 |
| SH01-F005 | Length and alignment | Full width, or fitted to content and aligned start, centre or end (dock style) | S | 2 |
| SH01-F006 | Floating panel | Optional 8 px margin and radius 12; uses layer-surface margin | C | 2 |
| SH01-F007 | Visibility modes | Always visible; auto-hide (reveal on edge touch for 150 ms or on `Super`); dodge windows; windows-can-cover | S | 2 |
| SH01-F008 | Three zones | Start, centre and end zones; each holds an ordered list of applet slots | M | 1 |
| SH01-F009 | Spacers | Fixed and flexible spacer slots | S | 1 |
| SH01-F010 | Edit mode | Right click, "Edit panel": slots get handles, drag to reorder, drop zone markers, remove button, thickness and edge controls | M | 1 |
| SH01-F011 | Layout presets | "Bottom bar", "Top bar + dock", "Left bar", "Minimal"; applied atomically with Undo | S | 2 |
| SH01-F012 | Layout lock | `panel.locked`; an administrator policy can make it read-only | S | 2 |
| SH01-F013 | Opacity and blur | Solid, translucent, or blurred backdrop (DESIGN_SYSTEM.md section 2 allows blur on shell surfaces); opaque when a window is maximised | S | 2 |
| SH01-F014 | Overflow | When slots do not fit, the task list shrinks first, then scrolls; other slots collapse into an overflow menu `…` | M | 1 |
| SH01-F015 | Panel context menu | Add applet, Edit panel, Panel settings, Task bar settings, Lock layout, Open System Monitor (SY04) | M | 1 |
| SH01-F016 | Per-output panels | A panel belongs to an output identified by connector plus EDID hash; layout follows the display when it moves ports | M | 2 |
| SH01-F017 | Primary-only or all outputs | Setting: panel on primary only, on all, or custom per output | M | 1 |
| SH01-F018 | Fullscreen behaviour | Panel is hidden under a fullscreen window; revealed by `Super` | M | 1 |
| SH01-F019 | Scroll actions | Scroll on empty panel area: switch workspace (off by default); on task list: cycle windows | C | 3 |
| SH01-F020 | Panel export and import | Layout saved as a JSON file; import validates against the schema | C | 3 |

### 4.2 Launcher and app grid
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH01-F021 | Open | `Super` tap, panel button, or hot corner (SH05). Opens in under 100 ms on the focused output | M | 1 |
| SH01-F022 | Two forms | Compact menu (560×640, anchored to the button) or full-screen grid; setting `launcher.form` | M | 1 |
| SH01-F023 | Search field focused | Typing anywhere searches at once; no click needed | M | 1 |
| SH01-F024 | Favourites row | Up to 24 pinned tiles; drag to reorder | M | 1 |
| SH01-F025 | All apps grid | `gridView`, alphabetical, virtualised; tile shows icon, name, trust badge | M | 1 |
| SH01-F026 | Categories | Sidebar list matching CATALOGUE.md categories plus "All", "Recent", "Foreign apps" (VM03) | M | 1 |
| SH01-F027 | Recents | Recent apps and recent documents (from `svc.portal.file` recent list); can be turned off and cleared | S | 1 |
| SH01-F028 | Folders | Drag a tile on another to make a folder; rename with `F2` | C | 3 |
| SH01-F029 | Type-ahead ranking | Prefix beats substring beats fuzzy; frequency and recency weighted; ties stable | M | 1 |
| SH01-F030 | Keyboard grid navigation | Arrows, `Home`, `End`, `PgUp`, `PgDn`, `Tab` between regions, `Enter` launches | M | 1 |
| SH01-F031 | Tile context menu | Open, Open new window, Open in disposable sandbox, Pin to task bar, Add to favourites, Add to desktop, Show permissions (AD05), App details (SY05), Uninstall (SY05) | M | 1 |
| SH01-F032 | App actions | Manifest-declared jump actions (e.g. "New private window") listed in the context menu | S | 2 |
| SH01-F033 | Capability peek | `Space` or hover 600 ms: card listing the app's capabilities, signer, version, network route, last used | M | 1 |
| SH01-F034 | Trust badge | Verified (system key), verified (third-party key), unverified, tampered; icon and word, never colour alone | M | 1 |
| SH01-F035 | Disposable launch | Starts the app in a fresh throw-away domain with empty `fs.app`; the window gets the disposable label (SH05) | M | 2 |
| SH01-F036 | Launch feedback | Tile pulses once; task bar shows a starting entry until the first surface maps or 15 s passes, using `xdg_activation_v1` tokens | M | 1 |
| SH01-F037 | Hidden apps | Hide a tile without uninstalling; "Show hidden" toggle | C | 2 |
| SH01-F038 | New and updated marks | Dot on apps installed or updated in the last 3 days; cleared on first open | C | 2 |
| SH01-F039 | Power row | Lock, Log out, Suspend, Restart, Shut down, Switch user; each sends a request to SH03 which draws the dialog | M | 1 |
| SH01-F040 | User tile | Avatar and name; opens SY01 Users page | S | 2 |
| SH01-F041 | Places row | Home, Documents, Downloads, Trash; opens SY02 through the `open` intent | S | 2 |
| SH01-F042 | Launch with route | Submenu "Open with route": clear, VPN, Tor; only routes the manifest allows; hands over to SE03 policy | S | 3 |
| SH01-F043 | App registry watch | The grid updates live when SY05 installs or removes a bundle | M | 1 |
| SH01-F044 | Foreign app entries | Windows, Linux and Android launchers created by VM03 appear with a "foreign" badge and the personality name | S | 3 |

### 4.3 Task bar and dock
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH01-F045 | Window list | One entry per toplevel from `ext_foreign_toplevel_list_v1` (title, app id, identifier) | M | 1 |
| SH01-F046 | Pinned apps | Pinned entries stay when closed; running state shown by an underline | M | 1 |
| SH01-F047 | Grouping | Never, always, or when full; a group shows a count and opens a list | M | 1 |
| SH01-F048 | Labels | Icons only, or icon and title; title elided in the middle | M | 1 |
| SH01-F049 | Activate, minimise | Click focuses; click on the focused window minimises (needs toplevel management, SH01-GAP-1) | M | 1 |
| SH01-F050 | New instance | Middle click or `Shift+click` | M | 1 |
| SH01-F051 | Close | `Ctrl+W` on a focused entry, middle click on a preview, or context menu | M | 1 |
| SH01-F052 | Previews | Hover 400 ms: live thumbnails of the group's windows with title and close button; capture comes from the compositor (SH01-GAP-2), never from the app | S | 2 |
| SH01-F053 | Peek | Hovering a preview shows that window at full size and fades others | C | 3 |
| SH01-F054 | Entry context menu | App actions, recent documents, New window, Move to workspace, Move to output, Keep above, Pin, Show permissions, Close, Force stop | M | 1 |
| SH01-F055 | Jump list | Recent documents per app from the portal recent list (only documents that app opened) | S | 2 |
| SH01-F056 | Badges | Count badge and progress bar from `svc.notify` (badge and progress records) | S | 2 |
| SH01-F057 | Attention | An entry that requests attention gets a steady `--warn` underline; no flashing (DESIGN_SYSTEM.md principle 4) | M | 1 |
| SH01-F058 | Security label on entries | A 3 px colour bar and tooltip showing the owning domain: native, sandboxed, disposable, VM, foreign, elevated (same palette as SH05) | M | 1 |
| SH01-F059 | Device-in-use marks | Small camera, microphone, screen marks on the entry of the app using them (data from SH02 indicator service) | S | 2 |
| SH01-F060 | Network mark | `--net` dot when the app has an open socket; click lists endpoints (from `svc.audit` first contacts) | S | 2 |
| SH01-F061 | Keyboard | `Super+1..9` activates the nth entry; `Super+Shift+n` new window; `Super+T` focuses the task bar | M | 1 |
| SH01-F062 | Drag to reorder | Drag entries; pinned order persists | M | 1 |
| SH01-F063 | Drag over to raise | Dragging a file over an entry for 500 ms raises that window (spring loading) | S | 2 |
| SH01-F064 | Filters | Show windows from: current workspace or all; current output or all | M | 1 |
| SH01-F065 | Minimised look | Minimised entries are dimmed | S | 1 |
| SH01-F066 | Workspace switcher applet | Pager from `ext_workspace_v1`; click switches, scroll cycles, drag an entry on a workspace moves the window (through SH05) | M | 1 |
| SH01-F067 | Show desktop | Applet and `Super+D`; toggles through SH05 | S | 1 |
| SH01-F068 | Audio mark | Speaker mark on entries producing sound; click mutes that app's stream (through `svc.media`) | C | 3 |

### 4.4 Desktop surface
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH01-F069 | Background surface | One layer surface on layer `Background` per output | M | 1 |
| SH01-F070 | Wallpaper types | Image (PNG, JPEG, WebP, AVIF, JPEG XL), solid colour, gradient, slideshow folder, day and night pair | M | 1 |
| SH01-F071 | Fit modes | Fill, fit, stretch, centre, tile, span across outputs | M | 1 |
| SH01-F072 | Safe decoding | Wallpapers are decoded by `svc.thumbnail` in its sandbox; the shell receives pixels only | M | 1 |
| SH01-F073 | Per-output and per-workspace | Different wallpaper per output; optional per workspace | S | 2 |
| SH01-F074 | Slideshow | Interval 1 min to 1 day, order or shuffle, pause on battery | C | 2 |
| SH01-F075 | Colour extraction | Optional accent suggestion from the wallpaper; never applied without the user choosing it | C | 3 |
| SH01-F076 | Desktop icons | Off by default. When on, shows the Desktop folder through a remembered portal grant | S | 2 |
| SH01-F077 | Icon layout | Grid snap, sort by name, type, date, size; free placement saved per output | S | 2 |
| SH01-F078 | Icon actions | Open, rename `F2`, trash `Delete`, drag and drop to and from SY02; all file work is done by SY02's file operation service | S | 2 |
| SH01-F079 | Desktop context menu | New folder, New document (intent `new`), Paste, Change wallpaper, Add widget, Display settings (SY01), Edit desktop, Open Terminal here | M | 1 |
| SH01-F080 | Widgets | Applets placed on the desktop in a 16 px grid: clock, calendar, notes, system load, weather, folder view, media | S | 2 |
| SH01-F081 | Widget edit mode | Move, resize by handles, configure, remove; snap lines; Undo | S | 2 |
| SH01-F082 | Widget isolation | Each third-party widget runs in its own app host with its own capabilities (section 6.1) | M | 2 |
| SH01-F083 | Drop on desktop | Dropping files copies or links into the Desktop folder (asks which); dropping text makes a note widget | C | 3 |
| SH01-F084 | Dim behind windows | Optional dim of wallpaper when any window is open | C | 3 |
| SH01-F085 | Session-state mark | A small watermark for guest, disposable and live-installer sessions, drawn in a corner | M | 1 |

### 4.5 System-wide search
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH01-F086 | One field | The launcher field searches apps, settings, files, contacts, mail, notes, calendar, bookmarks, help | M | 1 |
| SH01-F087 | Providers | Results come from `svc.search` providers; each provider is a category with an icon and can be ordered or turned off | M | 1 |
| SH01-F088 | Streaming results | First results within 50 ms of a key; later ones fill in without moving the selected row | M | 1 |
| SH01-F089 | Result row | Icon, title with match highlighted, one-line context, provider, modified date | M | 1 |
| SH01-F090 | Preview pane | `Right` or `Space` shows a preview (from `svc.thumbnail`) and metadata in the wide form | S | 2 |
| SH01-F091 | Result actions | `Enter` open; `Ctrl+Enter` open containing folder; `Shift+Enter` open with…; `Ctrl+C` copy path or value; `Alt+Enter` properties | M | 1 |
| SH01-F092 | Filters | Prefix words: `app:`, `file:`, `set:`, `mail:`, `note:`, `help:`, `type:pdf`, `in:Documents`, `before:2026-01-01`, `tag:` | S | 2 |
| SH01-F093 | Scope is local | No query leaves the machine. There is no web suggestion provider in the default set | M | 1 |
| SH01-F094 | Web search hand-off | Optional last row "Search the web for …" opens IN01; off by default; nothing is sent until the user picks it | C | 2 |
| SH01-F095 | Private results | Results from locked stores (SE05 vaults, encrypted notebooks) are absent while locked; a row says "n locked sources not searched" | M | 2 |
| SH01-F096 | History | Last 50 queries, local, clearable, can be turned off; never recorded in guest or disposable sessions | S | 2 |
| SH01-F097 | Natural-language query | Optional: `svc.model` turns "pdfs from last week about tax" into filters; shown as editable chips; needs `ai.local` and is off by default | C | 3 |
| SH01-F098 | Index status row | If the index is building or stale, a row says so with a link to SY01 Search settings | S | 2 |
| SH01-F099 | Drag out | A file result can be dragged into any window as a portal handle offer | C | 3 |

### 4.6 Command runner
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH01-F100 | Open | `Alt+Space` or `Alt+F2`: a 640 px bar at top centre on layer `Overlay`, keyboard interactivity `Exclusive` | M | 1 |
| SH01-F101 | Modes by prefix | none = everything; `>` shell action; `=` calculator; `?` help; `@` window; `:` setting; `/` path | M | 1 |
| SH01-F102 | Calculator | Expressions, units, bases, dates; evaluated by the OF07 engine helper; `Enter` copies the result | S | 2 |
| SH01-F103 | Currency | Offline rates table from OF07; shows the table's date | C | 3 |
| SH01-F104 | Run a program | `>name args`: lists matching executables on the user's path; runs through a launcher helper in a new domain, never inside the shell process | M | 1 |
| SH01-F105 | Run in terminal | `Ctrl+Enter` runs the line in SY03 | S | 1 |
| SH01-F106 | Window switch | `@text` lists windows by title and app; `Enter` activates | S | 2 |
| SH01-F107 | Shell actions | Lock, log out, suspend, toggle do-not-disturb, toggle night light, switch workspace, tile left; every shell action is in one action table (PLATFORM.md section 5 `ui::action`) | M | 1 |
| SH01-F108 | App actions | Apps can export actions to the runner through `svc.search` action records; shown with the app's icon | S | 3 |
| SH01-F109 | Path completion | `/` or `~` starts path completion through a read-only directory listing handle limited to names | S | 2 |
| SH01-F110 | Open URL | A typed URL opens through `svc.intent`; the row shows the scheme handler and the route (clear, VPN, Tor) | S | 2 |
| SH01-F111 | History | `Up` recalls; `Ctrl+R` searches history; lines starting with a space are not stored | S | 1 |
| SH01-F112 | Timers and quick notes | `timer 10m tea` hands to UT02; `note buy milk` hands to OF02 through the `compose` intent | C | 3 |
| SH01-F113 | Kill | `kill name` lists matching apps; `Enter` asks SY04's service to stop the domain after one confirm | C | 3 |
| SH01-F114 | No privilege | The runner never elevates. A command that needs privilege raises an SH03 authentication prompt | M | 1 |
| SH01-F115 | Dry description | Each row states what will happen ("Run in a new sandbox with no network") before `Enter` | M | 1 |

### 4.7 Applet framework
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH01-F116 | Applet bundle | An applet is a normal `.app` bundle with `"applet": {...}` in its manifest: sizes, zones allowed, popover size | M | 1 |
| SH01-F117 | Slot contract | Panel gives each applet a rectangle, an orientation, a thickness and a density; the applet returns a preferred length | M | 1 |
| SH01-F118 | Compact and popover forms | Compact view in the slot; optional popover (layer surface popup) on click; optional desktop widget form | M | 1 |
| SH01-F119 | In-process first-party | Applets signed with the system key and listed in the shell's allow list run inside the shell process | M | 1 |
| SH01-F120 | Out-of-process third-party | Other applets run in their own app host and are embedded as a `wl_subsurface` of the panel; a crash shows a "stopped" slot with Restart | M | 2 |
| SH01-F121 | Applet capabilities | An applet has its own manifest capabilities; the shell's are never lent to it | M | 2 |
| SH01-F122 | Applet picker | Grid of installed applets with name, description, capabilities and trust badge; drag to a panel or the desktop | M | 1 |
| SH01-F123 | Per-instance settings | Each placed instance has its own settings namespace `applet.<instance-id>` | M | 1 |
| SH01-F124 | First-party applets in SH01 | Launcher button, task list, workspace switcher, show desktop, spacer, separator, search field, runner button, clock (display only), places menu, trash, system load, quick launch, window title, global menu button, keyboard layout | M | 1 |
| SH01-F125 | Hosted applets from other apps | Status area, notifications button, quick settings, calendar clock, battery, media (SH02); clipboard (AX05); input method (AX04) | M | 1 |
| SH01-F126 | Status items (tray) | Background apps (`bg.run`) register a status item: icon, tooltip, menu, activate action; the menu is drawn by the shell from data, not by the app | M | 2 |
| SH01-F127 | Status item limits | Icon is a bundle asset or a 64×64 pixel buffer; no arbitrary surfaces; rate limit 4 updates per second | M | 2 |
| SH01-F128 | Applet update budget | An applet that wakes more than once per second while hidden is throttled and reported in SY04 | S | 3 |
| SH01-F129 | Applet API version | `applet_api: 1`; a mismatch refuses to load and says so in the slot | M | 1 |

### 4.8 Multi-monitor
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH01-F130 | Output tracking | Reacts to `wl_output` add, remove, mode, scale and transform within one frame | M | 1 |
| SH01-F131 | Hot-plug rules | New output: gets the default panel only if `panel.outputs = all`; known output: restores its saved layout | M | 2 |
| SH01-F132 | Primary output | Launcher, runner and new panels go to the primary; `launcher.follow_focus` sends them to the focused output | M | 1 |
| SH01-F133 | Mixed scale | Each surface renders at its output's fractional scale (`wp_fractional_scale_v1`); icons re-rasterise from vector | M | 2 |
| SH01-F134 | Mirrored outputs | One set of shell surfaces for a mirror group | S | 2 |
| SH01-F135 | Presentation mode | When an output is marked "presentation" (SY01), no panel, no desktop icons and no banners on it | S | 3 |
| SH01-F136 | Output removed | Panels and widgets of a removed output are parked, not deleted; windows are handled by SH05 | M | 2 |

### 4.9 Settings, security, accessibility, recovery
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH01-F137 | Settings page | Shell settings appear as a page in SY01 (schema in section 8); no separate settings window | M | 1 |
| SH01-F138 | Policy lock-down | Keys can be marked mandatory by an administrator policy file; locked controls say who locked them | S | 3 |
| SH01-F139 | No ambient authority | The shell process holds no file access beyond `fs.app`; all file work goes through portals and SY02 | M | 1 |
| SH01-F140 | Launch is brokered | Apps are started by the session's launch service; the shell cannot fork arbitrary programs (no `proc.exec`) | M | 1 |
| SH01-F141 | Audit | Every launch, disposable launch and runner command is written to `svc.audit` with time, bundle id and domain | M | 1 |
| SH01-F142 | Crash restart | The session supervisor restarts the shell within 1 s; panels, pins and widget layout come back from settings; windows are untouched | M | 1 |
| SH01-F143 | Safe mode | Three crashes in 60 s start the shell with third-party applets off and a banner offering to re-enable them one by one | M | 2 |
| SH01-F144 | Screen reader | Panel, launcher, runner and desktop expose full a11y trees; live region for result counts | M | 1 |
| SH01-F145 | Keyboard-only | `Super+T` task bar, `Super+B` status area, `Ctrl+Alt+D` desktop focus, arrows within; `Shift+F10` context menus | M | 1 |
| SH01-F146 | Large and touch densities | Touch density makes panel 48 px and launcher tiles 96 px | S | 2 |
| SH01-F147 | Reduced motion | Launcher and popovers appear without slide or scale | M | 1 |
| SH01-F148 | First-run layout | AD01's first run picks a preset; default pins: SY02, IN01, SY03, SY01, OF01 | S | 1 |
| SH01-F149 | Kiosk mode | Single-app mode: no launcher, no runner, no panel, fixed app; leaving needs SH03 authentication | C | 3 |
| SH01-F150 | Help | `F1` on any shell element opens the matching UT05 page | S | 2 |

## 5. GUI

The shell has no ordinary application window. Every surface below is a Wayland layer surface (`zwlr_layer_shell_v1`; module present at `PBSD:pbsd/compositor/pbsd.compositor.wayland.layer.cppm`, with keyboard, margin, popup and configure parts beside it) or a popup of one. Sizes are logical pixels at scale 1.

### 5.1 Window map
```
Desktop Shell (per output unless noted)
├─ Desktop surface            layer Background, anchored to all four edges
│  ├─ Wallpaper
│  ├─ Icon grid (optional)
│  ├─ Widgets
│  ├─ Session-state mark
│  ├─ Desktop context menu
│  └─ Desktop edit mode (widget handles, snap lines)
├─ Panel (0..n)               layer Top, one edge, exclusive zone
│  ├─ Start zone / Centre zone / End zone (applet slots)
│  ├─ Overflow menu
│  ├─ Panel context menu
│  ├─ Panel edit mode strip (edge, thickness, alignment, visibility)
│  ├─ Task entry popups: preview strip, group list, context menu / jump list
│  └─ Applet popovers (layer popup; SH02 popovers attach here too)
├─ Launcher (one, on the focused or primary output)   layer Overlay, keyboard Exclusive
│  ├─ Compact menu form
│  ├─ Full-screen grid form
│  ├─ Search results view (replaces the grid while typing)
│  ├─ Capability peek card
│  ├─ Tile context menu
│  └─ Folder popover
├─ Command runner (one)       layer Overlay, top centre, keyboard Exclusive
├─ Applet picker sheet        layer Overlay, bottom sheet in edit mode
├─ Wallpaper chooser          sheet; file picking is done by SH04
├─ Safe-mode banner           layer Top, top edge, no exclusive zone
└─ Status item menus          popups drawn by the shell from data
Settings: no window. A page in SY01 (section 8).
```

### 5.2 Main window
The "main window" is the whole screen in the default preset: one bottom panel, 40 px. Shown at 1440×900.

```
+----------------------------------------------------------------------------------------+
|                                                                                  (1)   |
|   +----------------+                                         +---------------------+   |
|   | 14:32          | (2) clock widget                        | Notes         [⋮]   |   |
|   | Thu 12 Nov     |                                         | - call the bank     |   |
|   +----------------+                                         | - renew key         |   |
|                                                              +---------------------+   |
|   [▤] Projects   [▤] Scans   [≡] todo.md     (3) desktop icons (off by default)       |
|                                                                                        |
|                 +--------------------------------------------+                        |
|                 | Files — Documents              [▁][▢][✕]  |  (4) app windows are    |
|                 |                                            |      drawn by SH05      |
|                 +--------------------------------------------+                        |
|                                                                                        |
|                                                          Disposable session  (5)       |
+----------------------------------------------------------------------------------------+
|[◉](6)|[Search… ](7)|[1][2][3][+](8)| [▤]▁ [◍]▁² [>_]  [✎]▔ … (9)        |(10)[⌃][⌨ EN][◔][net][🔔 3]| 14:32 (11)|[▭](12)|
+----------------------------------------------------------------------------------------+
```

1. **Desktop surface.** Layer `Background`. A `viewportWidget` paints the wallpaper; a `panel` with absolute children holds widgets; a `gridView` holds icons. Right click opens the desktop context menu.
2. **Widget.** An applet in its desktop form. Edit handles appear only in desktop edit mode.
3. **Desktop icons.** `gridView` bound to a folder model from a remembered portal grant. Off by default (F076).
4. **App windows.** Not shell surfaces. Frames, labels and stacking belong to SH05.
5. **Session-state mark.** Text in `--fg-2`, bottom right, above the panel; shown for guest, disposable and live sessions (F085).
6. **Launcher button.** `toolButton`. Click or `Super` opens the launcher. Start zone.
7. **Search field applet.** Optional `lineEdit` that opens the launcher in search state with the typed text.
8. **Workspace switcher.** `tabBar` of workspaces from `ext_workspace_v1` (`PBSD:pbsd/compositor/pbsd.compositor.wayland.ext_workspace_v1.cppm`). `[+]` adds one through SH05.
9. **Task list.** `modelListView`, horizontal, over the toplevel model. `▁` is the running underline, `²` a group count, `▔` the 3 px security-label bar (F058). `…` is overflow. Centre zone.
10. **Status area.** Hosted SH02 applets: hidden-items chevron, keyboard layout, capability indicators, network badge, notifications button with count. End zone.
11. **Clock.** Hosted SH02 calendar clock; click opens the SH02 calendar popover.
12. **Show desktop.** `toolButton`, 8 px wide strip at the very end.

Panel regions are a `panel` with `layout: row` holding three child panels (start, centre, end) and flexible spacers. Each slot is a `panel` whose content is the applet's `.fuseui` root (in-process) or a subsurface (out-of-process).

States: normal; edit mode; auto-hidden (4 px reveal strip); fullscreen-covered; safe mode (banner, third-party slots show "off"); locked layout (no edit entries in the menu); output parked.

### 5.3 Other views and dialogs
#### 5.3.1 Launcher, compact form (560×640, anchored above the launcher button)
```
+----------------------------------------------------------------------------------------+
|                                   (desktop dimmed 20%)                                 |
| +------------------------------------------------------+                               |
| | [🔍 Type to search apps, files, settings…        ] (1)|                               |
| +-------------+----------------------------------------+                               |
| | Favourites  | Favourites                         (3) |                               |
| | All apps    | [Files] [Browser] [Terminal] [Settings]|                               |
| | Recent      | [Editor] [Mail]   [Notes]   [Passwords]|                               |
| |-------------| Recent                             (4) |                               |
| | System      | [≡] budget.ods        Sheets · 10:12   |                               |
| | Security    | [≡] notes on pf.md    Notes  · Wed     |                               |
| | Office      | [▤] Scans             Files  · Tue     |                               |
| | Comms       | All apps                           (5) |                               |
| | Internet    | [Archives ✓] [Backup ✓] [Browser ✓]    |                               |
| | Media       | [Calculator ✓] [Calendar ✓] [Camera ✓] |                               |
| | Create      | [Clocks ✓] [Contacts ✓] [Data Studio ✓]|                               |
| | Dev  AI     | [Devices ✓] [Disk Usage ✓] [Demo ⚠]    |                               |
| | Utilities   |                                     …  |                               |
| | Foreign  (2)|                                        |                               |
| +-------------+----------------------------------------+                               |
| | (●) odin (6)   [Home][Docs][Downloads] (7)  [🔒][⇥][☾][↻][⏻] (8)|                    |
| +------------------------------------------------------+                               |
|[◉]|[Search… ]|[1][2][3][+]| [▤]▁ [◍]▁² [>_]  …                 |[⌃][EN][net][🔔]|14:32|[▭]|
+----------------------------------------------------------------------------------------+
```

1. Search field, `lineEdit`, focused on open. Typing switches the right side to the results view (5.3.3).
2. Category list, `listView`, 132 px. Matches CATALOGUE.md categories.
3. Favourites, `gridView`, 4 columns, tile 96×88.
4. Recent documents, `modelListView`, 3 rows; hidden when `launcher.recents` is off.
5. All apps, `gridView`, virtualised. `✓` verified, `⚠` unverified; the word is in the tooltip and the a11y name.
6. User tile, `toolButton` with `image`.
7. Places row, `toolButton` × 4.
8. Power row: Lock, Log out, Suspend, Restart, Shut down. Each sends a request to SH03.

States: empty favourites ("Drag an app here to keep it handy"), app registry loading (skeleton tiles, at most 200 ms), registry error ("App list unavailable. [Retry] [Open Software]"), guest session (no recents, no user tile menu), narrow output under 640 px (category list becomes a `comboBox` above the grid).

#### 5.3.2 Launcher, full-screen grid form (whole output, 1440×900)
```
+----------------------------------------------------------------------------------------+
|                         [🔍 Type to search…                              ]      (1)    |
|                                                                                        |
|   [All] [Favourites] [Recent] [System] [Security] [Office] [Comms] [Internet] [More v] |
|                                                                                  (2)   |
|     [Files]     [Browser]   [Terminal]  [Settings]  [Editor]    [Mail]      [Notes]    |
|      ✓ native    ✓ net       ✓ exec      ✓ native    ✓ native    ✓ net       ✓ native  |
|                                                                                        |
|     [Passwords] [Calendar]  [Contacts]  [Photos]    [Music]     [Video]     [Monitor]  |
|      ✓ secret    ✓ net       ✓ native    ✓ native    ✓ native    ✓ native    ✓ native  |
|                                                                                        |
|     [Software]  [Firewall]  [Vault]     [Backup]    [Disks]     [Logs]      [Demo]     |
|      ✓ net       ✓ native    ✓ secret    ✓ net       ✓ native    ✓ native    ⚠ unverif.|
|                                                                                  (3)   |
|                                    ● ○ ○   (4) pages                                   |
|                                                                                        |
|   (●) odin              [Home] [Documents] [Downloads] [Trash]       [🔒][⇥][☾][↻][⏻]  |
+----------------------------------------------------------------------------------------+
```

1. Search field, 560 px, centred.
2. Category chips, `tabBar`. "More" is a `comboBox` when chips overflow.
3. Tile grid, `gridView`, 7 columns at this width, tile 144×128. The second line under each tile is the capability summary word: `native` (no network), `net`, `exec`, `secret`, or `unverif.` It can be turned off (`launcher.tile_summary`).
4. Page dots when `launcher.grid_paging` is on; otherwise the grid scrolls.

`Esc`, a click on empty space or `Super` closes it. The panel stays visible below when its mode is "always visible".

#### 5.3.3 Search results view (inside either launcher form; shown in the compact form widened to 880 px)
```
+--------------------------------------------------------------------------------------+
| [🔍 tax 2026                                                     ] [app][file][set] (1)|
+---------------------------------------------------+----------------------------------+
| Apps                                          (2) | tax-return-2026.pdf          (5) |
|   [▦] Calculator                 app              | +------------------------------+ |
| Files                                             | |  (first page thumbnail)      | |
| > [≡] tax-return-2026.pdf        Documents · 3 d  | |                              | |
|   [≡] tax notes.md               Notes · 2 w      | +------------------------------+ |
|   [▤] Tax 2026                   Documents        | PDF · 214 kB · 12 pages          |
| Mail                                              | ~/Documents/Tax 2026             |
|   [✉] Re: tax file number        Inbox · 9 Nov    | Modified 9 Nov 2026 16:40        |
| Settings                                          | Opens with Document Viewer       |
|   [⚙] Region and currency        Settings         | Provenance: downloaded, verified |
| Help                                              |                                  |
|   [?] Keeping records            Handbook         | [Open] [Open with…] [Show in     |
|                                               (3) |  Files] [Copy path]          (6) |
+---------------------------------------------------+----------------------------------+
| 2 locked sources not searched · Index up to date 14:30                          (4) |
+--------------------------------------------------------------------------------------+
```

1. Query field and active filter chips (`toolButton`, toggle). Chips mirror the prefix words of F092.
2. Result list, `modelListView` with section headers. One provider per section, at most 5 rows each until "Show all n".
3. The selected row never moves when later results stream in (F088).
4. Status line, `label`: locked-source count (F095) and index state (F098).
5. Preview pane, 320 px: `image` from `svc.thumbnail`, metadata in a `propertyGrid` (read-only).
6. Actions, `button` row; the same actions as F091.

States: no query (launcher home), searching (thin `progressBar` under the field, existing rows kept), no results ("Nothing found for 'x'. Check spelling or remove a filter."), index building, index off (apps and settings only, with a notice), `svc.search` down (apps only, from the shell's own registry cache).

#### 5.3.4 Command runner (640×56 collapsed, up to 640×420 with rows; top centre, 96 px from the top)
```
+----------------------------------------------------------------------------------------+
|                                                                                        |
|            +--------------------------------------------------------------+            |
|            | [› 12 in to cm                                             ] | (1)        |
|            +--------------------------------------------------------------+            |
|            | > [=] 30.48 cm                    Copy result          Enter | (2)        |
|            |   [=] 12 in = 0.3048 m            Copy                       |            |
|            |   [>] Run "12" in a new sandbox   no network, no files       | (3)        |
|            |   [?] Help: unit conversion       Open Help                  |            |
|            +--------------------------------------------------------------+            |
|            | > run  = calc  ? help  @ window  : setting  / path    Tab ⇥  | (4)        |
|            +--------------------------------------------------------------+            |
|                                                                                        |
+----------------------------------------------------------------------------------------+
```

1. `lineEdit` with the mode glyph on the left. History with `Up`.
2. Result rows, `modelListView`, row 32 px: icon, text, the dry description of F115, and the key that triggers it.
3. A run row always states the sandbox it will use.
4. Hint line, `label`, hidden after 20 uses (`runner.show_hints`).

States: empty (hint line only), error ("Cannot evaluate: unknown unit 'cmx'"), busy (spinner in the field for helpers slower than 100 ms), runner disabled by policy (shortcut shows a toast "The command runner is turned off by your administrator").

#### 5.3.5 Panel edit mode and applet picker (1440×900)
```
+----------------------------------------------------------------------------------------+
|                               (desktop, dimmed 30%)                                    |
|                                                                                        |
| +------------------------------------------------------------------------------------+ |
| | Add applets                                    [🔍 Filter…        ]        [Done]  | |
| | [Clock ✓]  [Task list ✓]  [Workspaces ✓]  [System load ✓]  [Places ✓]  [Trash ✓]   | |
| |  display     windows        pager           cpu, ram         folders     trash  (1)| |
| | [Window title ✓] [Spacer] [Separator] [Quick launch ✓] [Weather ✓ net] [Demo ⚠ net]| |
| +------------------------------------------------------------------------------------+ |
| | Edge (•)Bottom ( )Top ( )Left ( )Right   Thickness [40 v]  Length [Full v]         | |
| | Visibility [Always visible v]   Backdrop [Blurred v]   Output [DP-1 v]  [Remove panel] (2)|
| +------------------------------------------------------------------------------------+ |
|  ┆start zone                ┆centre zone                           ┆end zone      (3)  |
|[⠿◉ ✕][⠿Search ✕][⠿1 2 3 ✕]┆[⠿ Task list                    ✕]┆[⠿Status ✕][⠿Clock ✕]|
+----------------------------------------------------------------------------------------+
```

1. Applet picker, `gridView`. Each tile shows name, one-word description, trust badge and a `net` word when the applet has a network capability. Drag to a zone or to the desktop; `Enter` adds to the focused zone.
2. Panel controls: `radioButton` group, `comboBox` × 5, destructive `button`.
3. Slots in edit mode: drag handle `⠿`, remove `✕`, zone boundaries drawn as dashed lines, an insertion marker follows the pointer. `Ctrl+Z` undoes each change.

#### 5.3.6 Capability peek card (320×auto, beside a tile)
```
+--------------------------------------+
| [icon] Web Browser            ✓      |
| Verified · signed by system key      |
| Version 0.1.0 · last used today      |
|--------------------------------------|
| Can reach                            |
|  [net]  sites you type (net.user)    |
|  [▤]    files you pick               |
|  [◉]    camera, mic: asks first      |
| Route: clear  [change…]              |
| Cannot: run programs, read your home |
|--------------------------------------|
| [Open] [Open disposable] [Permissions]|
+--------------------------------------+
```

`dialog` styled as a popover, non-modal. Data comes from the bundle manifest and from AD05's grant store (read through `svc.settings` namespace `permissions`, read-only). "Permissions" opens AD05 at this app.

#### 5.3.7 Task entry preview strip and context menu
```
        +-------------------------+ +-------------------------+
        | Files — Documents   [✕] | | Files — Downloads   [✕] |
        | +---------------------+ | | +---------------------+ |
        | | (live thumbnail)    | | | | (live thumbnail)    | |
        | +---------------------+ | | +---------------------+ |
        | native · workspace 1    | | native · workspace 2    |
        +-------------------------+ +-------------------------+
 … [▤]▁² …
        +--------------------------------+
        | New window            Shift+click |
        | Recent ▸  budget.ods              |
        |           Scans                   |
        |--------------------------------|
        | Move to workspace ▸            |
        | Move to output ▸               |
        | [ ] Keep above                 |
        |--------------------------------|
        | [x] Pinned                     |
        | Show permissions               |
        | Close                   Ctrl+W |
        | Force stop…                    |
        +--------------------------------+
```

Preview tiles are 200×150 `image` widgets fed by compositor thumbnails (SH01-GAP-2). The line under each thumbnail is the security label word and workspace. If thumbnails are unavailable the tile shows the app icon and title only.

#### 5.3.8 Desktop context menu and wallpaper chooser
```
+---------------------------+      +------------------------------------------------------+
| New folder                |      | Wallpaper — DP-1                             [Done]  |
| New document ▸            |      | Type [Image v]  Fit [Fill v]  [x] Same on all outputs |
| Paste              Ctrl+V |      | +--------+ +--------+ +--------+ +--------+ +------+  |
|---------------------------|      | |default1| |default2| |default3| | my.jpg | | [+]  |  |
| Change wallpaper…         |      | +--------+ +--------+ +--------+ +--------+ +------+  |
| Add widget…               |      | Day/night pair [off]   Slideshow every [30 min v]     |
| Edit desktop              |      | Dim when windows are open [ ]                         |
| [ ] Show desktop icons    |      +------------------------------------------------------+
|---------------------------|
| Display settings          |
| Open Terminal here        |
+---------------------------+
```

`[+]` calls the SH04 file chooser with an image filter; the shell receives a read handle and passes it to `svc.thumbnail` for decoding (F072).

### 5.4 Menus
The shell has no menu bar. Its menus are context menus (`popupMenu`):

| Menu | Items (shortcut) |
|---|---|
| Panel | Add applets… · Edit panel (`Super+Shift+E`) · Panel settings · Task bar settings · Lock layout · Add panel ▸ (Top, Bottom, Left, Right) · Open System Monitor |
| Launcher tile | Open (`Enter`) · Open new window (`Ctrl+Enter`) · Open in disposable sandbox (`Ctrl+Shift+Enter`) · Open with route ▸ · App actions… · Pin to task bar · Add to favourites / Remove · Add to desktop · Hide · Show permissions (`Alt+Enter`) · App details · Uninstall… |
| Task entry | New window · App actions… · Recent ▸ · Move to workspace ▸ · Move to output ▸ · Keep above · Pinned · Show permissions · Close (`Ctrl+W`) · Force stop… |
| Task group | Close all (n) · Minimise all · Ungroup |
| Desktop | see 5.3.8 |
| Desktop icon | Open · Open with… · Open in disposable sandbox · Cut · Copy · Rename (`F2`) · Move to trash (`Delete`) · Properties (`Alt+Enter`) |
| Widget (edit mode) | Configure… · Resize · Bring to front · Send to back · Remove |
| Applet slot | Applet's own items · Configure… · Move · Remove |
| Search result | Open · Open with… · Show in Files (`Ctrl+Enter`) · Copy path (`Ctrl+C`) · Properties (`Alt+Enter`) · Exclude this folder from search |
| Status item | Items supplied as data by the owning app; "Quit" and "Show permissions" are always appended by the shell |

### 5.5 Keyboard and pointer
Global shortcuts are registered with the compositor's shortcut table through SH05 (SH05 owns the table); all are rebindable in SY01.

| Keys | Action |
|---|---|
| `Super` (tap) | Open or close the launcher |
| `Alt+Space`, `Alt+F2` | Command runner |
| `Super+1..9` / `Super+Shift+1..9` | Activate / new window of task entry n |
| `Super+T` / `Super+B` | Focus task list / status area |
| `Super+D` | Show desktop |
| `Ctrl+Alt+D` | Focus desktop icons and widgets |
| `Super+Shift+E` | Panel edit mode |
| `Super+E` / `Super+Enter` | Open Files / Terminal (default pins, rebindable) |
| In launcher: `Tab` | Cycle search, categories, grid, footer |
| In launcher: `Space` on a tile | Capability peek |
| In launcher: `Ctrl+1..9` | Launch favourite n |
| In results: `Right` | Open preview pane |
| In runner: `Ctrl+R`, `Up`, `Ctrl+Enter` | History search, recall, run in Terminal |

Pointer: middle click on a task entry opens a new instance, on a preview closes it. Scroll on the workspace switcher cycles workspaces. Long press (500 ms) equals right click in touch density.

Drag and drop: tiles to favourites, task bar or desktop (pin); task entries to reorder, to a workspace button (move window) or to another output's task list; files over a task entry (spring load, F063); files to the desktop (F083); file results out of the launcher (F099); applets from the picker to zones or desktop. Every drag has a keyboard route through the context menu.

Gestures (touchpad and touch; recognised by the compositor, see SH05): swipe up from the bottom edge reveals an auto-hidden panel; swipe up further opens the launcher.

### 5.6 States
| State | Behaviour |
|---|---|
| Empty | New user: preset layout, default pins, default wallpaper. Empty zones show "Drop applets here" only in edit mode |
| Loading | Panel paints within 50 ms of output add with cached pins; task list fills as toplevel events arrive |
| Error | A failed applet slot shows `⚠ stopped` with Restart and Remove. A failed service shows a one-line notice in the surface that needs it |
| Offline | No change; the shell uses no network |
| Locked session | All shell surfaces are unmapped by the compositor while SH03's lock is up; the shell keeps no input |
| Read-only / policy | Edit entries hidden; locked settings say who locked them |
| Guest / disposable | No history, no recents; session mark drawn |
| Narrow (< 640 px) | Task list icons only; search applet collapses to a button; launcher is always full-screen |
| Safe mode | Banner: "The shell restarted 3 times. Third-party applets are off. [Review applets]" |

### 5.7 Widgets
| Region | FUSE widget |
|---|---|
| Panel, zones, slots | `panel`, `separator` |
| Launcher button, power row, places, show desktop | `toolButton` |
| Task list, recents, results, runner rows | `modelListView` |
| App grid, favourites, applet picker, desktop icons, wallpaper tiles | `gridView` |
| Categories | `listView`, `tabBar` (chips), `comboBox` (narrow) |
| Search and runner fields | `lineEdit` |
| Menus, jump lists, status item menus | `popupMenu` |
| Peek card, wallpaper chooser | `dialog` (popover and sheet styles) |
| Previews, avatars, icons | `image` |
| Preview metadata | `propertyGrid` |
| Wallpaper | `viewportWidget` |
| Tooltips | `tooltip` |
| Progress and badges | `progressBar`, `label` |

Missing:

| Gap | What is missing | Where it lands |
|---|---|---|
| G-01 | Wayland backend with layer shell | FUSE-WP-05, FUSE-WP-10 |
| SH01-GAP-1 | A toplevel management protocol in the compositor (activate, minimise, close, move to workspace or output, keep above). `ext_foreign_toplevel_list_v1` only lists | compositor; FUSE-WP-10.3 |
| SH01-GAP-2 | Compositor-made window thumbnails handed to the shell as buffers. No capture module is in `pbsd/compositor/` | compositor; FUSE-WP-10.8 |
| SH01-GAP-3 | Popover and sheet overlay widgets, badge widget, section headers in `modelListView` | FUSE-WP-30, FUSE-WP-28 |
| SH01-GAP-4 | Embedding another client's surface in a panel slot (cross-client subsurface or an embed token) | compositor; FUSE-WP-07 |
| SH01-GAP-5 | Launch service and session supervisor (start an app bundle in a new domain, restart shell parts) | G-07, FORGE-WP-10, FORGE-WP-27 |
| SH01-GAP-6 | Shell-only manifest capabilities (`shell.*`, section 7); not in PLATFORM.md section 6 | G-07, FORGE-WP-08, FORGE-WP-38 |

## 6. Architecture

### 6.1 Components
| Component | Language | Process | Responsibility |
|---|---|---|---|
| Shell logic | ForgeScript | shell app host | Panel layout, launcher, task list, runner, desktop, applet lifecycle, view models |
| Shell UI | FUSE UI (`.fuseui`, CSS) | shell app host | All surfaces in 5.1 |
| `fuse_app_shell` binding | C++23 (platform, FUSE-WP-10) | shell app host | Layer surfaces, toplevel list, workspaces, activation tokens |
| First-party applets | ForgeScript + FUSE UI | shell app host | Slots listed in F124 |
| Third-party applet host | shared app host | one per applet | Runs an applet bundle with its own capabilities |
| App registry cache | ForgeScript | shell app host | Reads installed-bundle records from the launch service; keeps name, icon, capability summary |
| Launch service | C++23 (platform) | session service | Verifies bundles, creates the domain, starts the app host, returns an activation token target |
| Runner evaluator | reuses OF07's calculator helper | OF07 helper process | Evaluates `=` expressions; the shell sends text and receives text |
| Path lister | C++23 helper `path-lister` | own process, capability mode | Lists executable names on the user's path and directory entry names; returns names only |
| Search client | ForgeScript | shell app host | Streams queries to `svc.search`, merges and orders rows |
| Wallpaper decode | `svc.thumbnail` | service sandbox | Decodes and scales images to a shared-memory buffer |

```
                +------------------- compositor (pbsd/compositor) -------------------+
                | layer shell · foreign toplevel list · ext_workspace · activation   |
                +---------^----------------------^------------------------^----------+
                          | Wayland              | Wayland                |
   +----------------------+-------+   +----------+---------+   +----------+---------+
   | shell app host               |   | 3rd-party applet   |   | apps (windows)     |
   |  ForgeScript logic           |   | host (per applet)  |   +--------------------+
   |  FUSE UI surfaces            |   +---------^----------+
   |  first-party applets         |             | slot contract (bus)
   +--+-------+-------+-------+---+-------------+
      |bus    |bus    |bus    |bus
 +----v--+ +--v-----+ +v------+ +v-----------+ +-------------+ +-------------+
 |launch | |svc.    | |svc.   | |svc.settings| | path-lister | | OF07 calc   |
 |service| |search  | |notify | |svc.audit   | | helper      | | helper      |
 +-------+ +--------+ +-------+ +------------+ +-------------+ +-------------+
```

The shell runs as one process per session by default. Setting `shell.split_processes` (Proposed, phase 3) runs the desktop surface in a second host so a panel crash leaves the wallpaper up.

### 6.2 Data model
Stores: `svc.settings` namespace `org.pbsd.desktop-shell` for configuration; `~/.local/share/org.pbsd.desktop-shell/shell.db` (SQLite) for usage data; `~/.cache/org.pbsd.desktop-shell/` for icon and wallpaper rasters.

| Entity | Fields |
|---|---|
| `PanelLayout` (settings, JSON) | `schema: u32`, `panels: [Panel]` |
| `Panel` | `id: uuid`, `output_key: string` (connector + EDID hash, or `primary`), `edge: enum{top,bottom,left,right}`, `thickness: u16`, `length: enum{full,fit}`, `align: enum{start,centre,end}`, `visibility: enum{always,autohide,dodge,cover}`, `floating: bool`, `backdrop: enum{solid,translucent,blur}`, `zones: {start:[Slot], centre:[Slot], end:[Slot]}` |
| `Slot` | `instance_id: uuid`, `applet_id: string` (bundle id), `kind: enum{applet,spacer,flex_spacer,separator}`, `settings_ns: string` |
| `DesktopLayout` (settings, JSON) | `schema`, `per_output: {output_key: {wallpaper: Wallpaper, widgets:[WidgetPlacement], icons_on: bool, icon_positions: {name:[x,y]}}}` |
| `Wallpaper` | `type: enum{image,colour,gradient,slideshow,daynight}`, `grant_id: string` (portal grant, never a raw path), `fit: enum`, `colours: [rgba]`, `interval_s: u32` |
| `WidgetPlacement` | `instance_id`, `applet_id`, `x,y,w,h: i32` (16 px grid units), `z: u16` |
| `Pin` (settings) | `bundle_id`, `position: u16`, `kind: enum{taskbar,favourite,desktop}` |
| `AppRecord` (cache, from launch service) | `bundle_id`, `name`, `icon_ref`, `category`, `trust: enum{system,third_party,unverified,tampered}`, `cap_summary: [string]`, `actions: [{id,label}]`, `installed_at`, `updated_at`, `hidden: bool` |
| `usage` (SQLite) | `bundle_id TEXT`, `launches INTEGER`, `last_launch INTEGER`, `score REAL` |
| `query_history` (SQLite) | `id INTEGER`, `text TEXT`, `kind TEXT` (search or runner), `at INTEGER` |
| `TaskEntry` (memory only) | `toplevel_id: string` (protocol identifier), `app_id`, `title`, `domain_label: enum`, `workspace`, `output`, `state: flags{active,minimised,attention,fullscreen}`, `badge: u32`, `progress: f32` |

Schema version is stored in every JSON document and in `PRAGMA user_version`. Migration is forward only; the previous settings document is kept as `<key>.bak.<version>`.

### 6.3 File formats and protocols
| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Panel and desktop layout JSON | yes | yes | ForgeScript JSON with schema check | shell host |
| Bundle `manifest.json` | no (the launch service parses it; the shell gets typed records) | no | platform | launch service |
| Wallpaper images (PNG, JPEG, WebP, AVIF, JPEG XL) | pixels only | no | `svc.thumbnail` decoders | thumbnail sandbox |
| App icons (SVG subset, PNG) | pixels only | no | FUSE icon system (FUSE-WP-27) through `svc.thumbnail` for third-party bundles | thumbnail sandbox |
| Status item icon buffer (64×64 RGBA) | yes | no | own code, fixed size check | shell host |
| Wayland: layer shell, foreign toplevel list, ext_workspace, xdg_activation, fractional scale | client | client | `fuse_app_shell` | shell host |
| Session bus records (launch, search, notify, settings, audit) | yes | yes | schema compiler (G-08) | shell host |
| Runner command line | parsed into argv by own tokenizer; never passed to a shell | — | own code | shell host; executed by the launch service |

### 6.4 Integration
- Intents emitted: `open` (results, places, desktop icons), `new` (desktop menu), `search` (hand-off to an app's own search), `compose` (quick note), URL schemes (runner). Intents handled: `search` (another app asks for the system search UI with a query).
- MIME types: none.
- Services used: `svc.settings`, `svc.search`, `svc.notify` (badges and progress), `svc.thumbnail`, `svc.intent`, `svc.portal.file` (recents, wallpaper and desktop folder grants), `svc.audit`, `svc.media` (audio mark), `svc.power` (pause slideshow on battery), `svc.a11y`, `svc.model` (optional F097).
- Interfaces offered: `org.pbsd.shell.Applet1` (slot contract: `configure(rect, orientation, thickness, density)`, `preferred_length() -> u16`, `open_popover()`, `close_popover()`); `org.pbsd.shell.StatusItem1` (`register(icon, tooltip, menu)`, `update`, `activated` signal); `org.pbsd.shell.Shell1` (`open_launcher(query)`, `open_runner(text)`, `show_desktop()`, `enter_edit_mode()`).
- Other apps: SH02 (hosted applets, indicator data), SH03 (session requests), SH04 (file chooser, open-with), SH05 (workspaces, window actions, shortcut table, security labels), SY01 (settings page), SY02 (file operations), SY03 (run in terminal), SY04 (force stop), SY05 (install, uninstall, details), AD05 (permissions), OF07 (calculator), UT02, OF02, UT05, VM03, SE03, AX05, AX04.
- Compositor (Verified as module files in `PBSD:pbsd/compositor/`): `…wayland.layer.cppm` (requests `SetSize`, `SetAnchor`, `SetExclusiveZone`, `SetMargin`, `SetKeyboardInteractivity`, `GetPopup`), `…wayland.layer.keyboard.cppm`, `…wayland.layer.margin.cppm`, `…wayland.layer.popup.cppm`, `…wayland.foreign_toplevel.cppm` (`ext_foreign_toplevel_list_v1`, version 1), `…wayland.ext_workspace_v1.cppm`, `…wayland.xdg_activation.cppm`, `…wayland.fractional_scale.cppm`, `…wayland.blur_contrast.cppm` (panel and dialog blur parameters), `…wayland.security_context.cppm`. Only the module headers were read; how complete the server side is remains Unverified. Not found there: toplevel management, capture, idle notify (SH01-GAP-1, SH01-GAP-2).
- The KDE bridge modules (`…kde.bridge.cppm`, `…theme.sync.cppm`) are for the interim Plasma session and are not used by this shell (ROADMAP.md, cross-cutting decisions).

## 7. Security and privacy

Capabilities requested:

| Capability | Why |
|---|---|
| `fs.app` | Layouts, usage database, caches |
| `fs.portal` | Wallpaper files and the Desktop folder, by remembered grant |
| `net.none` | The shell never opens a socket |
| `proc.helper:path-lister` | Name completion for the runner |
| `svc.settings`, `svc.search`, `svc.notify`, `svc.thumbnail`, `svc.intent`, `svc.portal.file`, `svc.audit`, `svc.media`, `svc.power`, `svc.a11y` | As listed in 6.4 |
| `ai.local` (optional, off) | Natural-language query (F097) |
| `bg.run`, `bg.autostart` | The shell is a session component |
| `shell.layer` (Proposed, SH01-GAP-6) | Create layer surfaces |
| `shell.toplevels` (Proposed) | Read the toplevel list and ask for window actions |
| `shell.workspaces` (Proposed) | Read and switch workspaces |
| `shell.launch` (Proposed) | Ask the launch service to start a bundle or a runner command |

Not requested: `proc.exec`, any `net.*` beyond `net.none`, any `dev.*`, `svc.keyring`, `svc.clipboard` (copying a result uses the ordinary data device).

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| The user's trust in what the panel shows | A hostile app | Draws a fake panel, launcher or indicator | Only clients whose security context carries `shell.layer` may bind the layer shell global (compositor filter on `wp_security_context_v1`); ordinary apps cannot place surfaces above others or reserve screen edges |
| Launch authority | Hostile applet or search result | Makes the shell start a program with the shell's rights | The shell cannot exec; the launch service starts a named, signed bundle in a new domain; runner commands start in a fresh no-network sandbox and say so (F115) |
| Window titles and app list | Hostile third-party applet | Reads the task list to profile the user | Out-of-process applets get no toplevel or workspace access unless their own manifest holds `shell.toplevels`, which the install prompt names |
| Search queries and history | Other apps, other users | Read the history | Stored only in the shell's own database; off in guest and disposable sessions; never sent anywhere (F093) |
| Shell process integrity | Malformed image, icon or manifest | Memory corruption in a decoder | No decoder runs in the shell; pixels arrive from `svc.thumbnail`; status icons are fixed-size raw buffers; manifests are parsed by the launch service |
| Input focus | Hostile app | Opens a look-alike launcher to capture typing | Launcher and runner use layer `Overlay` with exclusive keyboard, which ordinary clients cannot request; `Super` is handled by the compositor, never delivered to apps |
| Status area | Background app | Floods updates or draws misleading surfaces | Status items are data only, rate-limited (F127); the shell draws them and appends the true app name |
| Availability | Crashing applet | Takes the panel down | Third-party applets are out of process; safe mode (F143) |
| Locked-store content | Search | Leaks vault names in results while locked | Providers for locked stores return nothing; the shell shows only a count (F095) |

Stored: layouts, pins, usage counts, query history, caches (paths in 6.2). Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `panel.layout` | json | preset "bottom bar" | `PanelLayout` document |
| `panel.locked` | bool | false | Hide edit actions |
| `panel.outputs` | enum{primary,all,custom} | primary | Where default panels appear |
| `panel.scroll_switches_workspace` | bool | false | F019 |
| `taskbar.grouping` | enum{never,always,when_full} | when_full | F047 |
| `taskbar.labels` | enum{icons,icons_titles} | icons | F048 |
| `taskbar.scope_workspace` | enum{current,all} | current | F064 |
| `taskbar.scope_output` | enum{current,all} | all | F064 |
| `taskbar.previews` | bool | true | F052 |
| `taskbar.preview_delay_ms` | u16 | 400 | Hover delay |
| `taskbar.click_active_minimises` | bool | true | F049 |
| `taskbar.security_bar` | bool | true | F058; cannot be turned off in policy-managed sessions |
| `launcher.form` | enum{compact,fullscreen} | compact | F022 |
| `launcher.follow_focus` | bool | true | F132 |
| `launcher.recents` | bool | true | F027 |
| `launcher.tile_summary` | bool | true | Capability word under tiles |
| `launcher.grid_paging` | bool | false | Pages instead of scrolling |
| `launcher.super_opens` | bool | true | `Super` tap opens the launcher |
| `search.providers` | list<{id,enabled,order}> | all local providers on | F087 |
| `search.history` | bool | true | F096 |
| `search.web_handoff` | bool | false | F094 |
| `search.natural_language` | bool | false | F097 |
| `runner.enabled` | bool | true | Policy can turn it off |
| `runner.history_size` | u16 | 200 | 0 turns history off |
| `runner.show_hints` | bool | true | Hint line |
| `runner.default_sandbox` | enum{no_net,net_clear} | no_net | Sandbox for `>` commands |
| `desktop.layout` | json | default wallpaper, no icons | `DesktopLayout` document |
| `desktop.icons` | bool | false | F076 |
| `desktop.dim_with_windows` | bool | false | F084 |
| `applets.third_party` | bool | true | Master switch; false in safe mode |
| `shell.split_processes` | bool | false | Desktop surface in its own host |

## 9. Accessibility and languages

- Each layer surface is a separate a11y root named "Panel (bottom, DP-1)", "Launcher", "Command runner", "Desktop".
- Task entries announce: app name, window title, state words (active, minimised, needs attention), security label word, badge count.
- The launcher announces the result count as a polite live region after typing pauses for 300 ms.
- Vertical panels and right-to-left locales mirror zones (start is right in RTL). The clock and numbers follow the locale.
- Switch access: the panel is one scan group; zones are subgroups.
- App names are matched by search in the user's language and in English; transliterated matching for non-Latin names is Proposed, phase 3.
- High contrast: the security-label bar gains a letter (`N`, `S`, `D`, `V`, `F`, `E`) so it does not rely on colour.

## 10. Performance budgets

| Item | Budget |
|---|---|
| Panel first frame after output add | ≤ 50 ms (FUSE-WP-10 budget) |
| Launcher open (shortcut to first frame) | ≤ 100 ms; ≤ 16 ms per frame while typing |
| First search rows | ≤ 50 ms after a key for apps and settings; other providers stream |
| Runner open | ≤ 60 ms |
| Idle | 0 frames and 0 wake-ups with a static panel; the clock applet wakes once per minute (once per second only if seconds are shown) |
| Memory | ≤ 90 MB resident for the shell host with 10 first-party applets, one 4K wallpaper per output excluded (wallpaper buffers are shared memory owned by the compositor after commit) |
| Largest inputs | 2,000 installed apps, 500 toplevels, 16 outputs, 64 applet instances |
| Crash restart | Panel back in ≤ 1 s |

## 11. Testing and proof

Test plan (targets; nothing has run):

- Unit (ForgeScript): ranking (F029), runner tokenizer and mode parsing, layout schema validation and migration, overflow algorithm, grouping rules, output-key matching.
- Golden renders: panel at 28, 40 and 72 px on each edge; launcher compact and full-screen; results view; runner; edit mode; all themes and densities; RTL.
- Scripted UI through the a11y tree: tasks T1–T14 end to end under the headless test compositor (FUSE-WP-04 `ui-wayland-headless`).
- Protocol tests: output hot-plug storms, toplevel add/remove storms (500 windows), workspace changes during drag.
- Fuzz: layout JSON importer, runner tokenizer, status item records, applet slot contract messages.
- Fault injection: kill `svc.search`, the launch service, an applet host; check the states in 5.6.
- Security tests: an unprivileged client tries to bind the layer shell and toplevel globals and must be refused; a third-party applet tries to read toplevels without the capability.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Runner tokenizer | Output argv never contains shell metacharacter expansion; total on all inputs | T2 | PRISM absence-of-runtime-error job on the C++ mirror, or ForgeScript checker when available | ForgeScript has no proof tooling yet (FORGE-WP-15) |
| Layout schema validator | Rejects every document not matching the schema; no crash | T3 | Fuzzing plus unit tests | Interpreted code |
| `path-lister` helper | Returns names only; never opens a file for reading content; bounded memory | T2 | PRISM on the helper; Capsicum rights limited to `CAP_LOOKUP`, `CAP_READ` on directories (`fstatat`, `getdents`) | none known |
| Launch request path | The shell cannot cause execution of an unsigned bundle | T1 (in the launch service, not here) | Proved in the service; shell side is a client | owned by G-07 |
| Slot contract | A misbehaving applet cannot make the panel exceed its exclusive zone | T3 | Property tests with random preferred lengths | UI code |
| Ranking | Deterministic for equal inputs | T3 | Unit tests | — |
| UI logic | Safe subset, no runtime error in tests | T3/T4 | Sanitizer runs of the host; scripted UI | Interpreted code |

## 12. Build plan

Phase 1 — usable core:

| Item | Work | Depends on |
|---|---|---|
| SH01-W01 | Shell bundle skeleton, session start, crash restart hook | G-07, SH01-GAP-5, FORGE-WP-10 |
| SH01-W02 | Panel layer surface, zones, slots, overflow | G-01, FUSE-WP-10.2, SH01-GAP-6 |
| SH01-W03 | First-party applets: launcher button, spacer, separator, clock (display), show desktop | SH01-W02 |
| SH01-W04 | Task list on the toplevel list; activate, close, minimise | FUSE-WP-10.3, SH01-GAP-1, SH05 |
| SH01-W05 | Workspace switcher applet | FUSE-WP-10.4, SH05 |
| SH01-W06 | App registry cache and launch requests with activation tokens | SH01-GAP-5, FUSE-WP-10.6 |
| SH01-W07 | Launcher compact form: favourites, grid, categories, keyboard | SH01-W06, FUSE-WP-28 |
| SH01-W08 | Search: apps and settings from the registry; `svc.search` client | G-09 (`svc.search`), SY01 |
| SH01-W09 | Command runner: modes, run, shell actions, history | SH01-W06, FUSE-WP-14 |
| SH01-W10 | Desktop surface: wallpaper, context menu, session mark | `svc.thumbnail`, SH04 |
| SH01-W11 | Panel edit mode and applet picker (first-party only) | SH01-W02 |
| SH01-W12 | Hosting SH02 applets in slots | SH02 |
| SH01-W13 | Capability peek and trust badges | AD05 grant store, SH01-W06 |
| SH01-W14 | Settings schema and SY01 page | SY01, G-09 (`svc.settings`) |
| SH01-W15 | A11y names, keyboard-only routes, reduced motion | G-04 |
| SH01-W16 | Audit records for launches and runner commands | G-09 (`svc.audit`) |

Phase 2 — full feature:

| Item | Work | Depends on |
|---|---|---|
| SH01-W17 | Full-screen launcher form, recents, places, user tile | SH01-W07 |
| SH01-W18 | Previews, jump lists, badges, progress, device and network marks | SH01-GAP-2, SH02, `svc.notify` |
| SH01-W19 | Disposable launch | launch service, SH05 labels |
| SH01-W20 | Auto-hide, dodge, floating, blur backdrop | compositor blur (`…wayland.blur_contrast.cppm`), FUSE-WP-24 |
| SH01-W21 | Search filters, preview pane, locked-source row, history | SH01-W08 |
| SH01-W22 | Runner: calculator, window switch, path completion, URL rows | OF07, `path-lister`, SH05 |
| SH01-W23 | Desktop icons, widgets, widget edit mode | SY02, SH01-W24 |
| SH01-W24 | Third-party applets out of process; status items | SH01-GAP-4, FORGE-WP-27 |
| SH01-W25 | Multi-monitor: per-output layouts, hot-plug rules, mixed scale, mirrors | FUSE-WP-11, SY01 |
| SH01-W26 | Safe mode | SH01-W24 |
| SH01-W27 | Layout presets, slideshow, per-workspace wallpaper, touch density | SH01-W10 |

Phase 3 — advanced:

| Item | Work | Depends on |
|---|---|---|
| SH01-W28 | Folders in the grid, peek, scroll actions, audio mark | SH01-W17, `svc.media` |
| SH01-W29 | Natural-language query | G-15, `svc.model` |
| SH01-W30 | App-exported runner actions, timers, notes, kill | UT02, OF02, SY04 |
| SH01-W31 | Launch with route, foreign app entries | SE03, VM03 |
| SH01-W32 | Policy lock-down, kiosk mode, layout import and export | SH03, SY01 |
| SH01-W33 | Presentation outputs, split processes, applet update budget reporting | SY01, SY04 |

## 13. Open questions and risks

1. **Toplevel management (SH01-GAP-1).** Use the wlr foreign-toplevel-management protocol, or a PBSD protocol that also carries the security label and domain id? The second fits SH05's labels better. Decision is shared with SH05.
2. **Thumbnails (SH01-GAP-2).** A capture protocol restricted to shell clients, or thumbnails pushed by the compositor? Pushed buffers keep capture out of every client.
3. **Third-party applet embedding (SH01-GAP-4).** Wayland has no standard cross-client embed. A PBSD embed token is new protocol surface that must be reviewed.
4. **`Super` tap** needs compositor support for "modifier released with no other key".
5. **In-process first-party applets** share the shell's fate. The allow list must stay short.
6. **Desktop icons** need a standing folder grant; this is the shell's only durable file access and should stay off by default.
7. **Runner sandbox default.** A no-network, no-files sandbox makes many commands useless. The row must make "Run in Terminal" (`Ctrl+Enter`) obvious.
8. **Shell capabilities** (`shell.*`) are not in PLATFORM.md section 6. They must be restricted to bundles signed by the system key.
9. **Blur** depends on the Vulkan backend on HardenedBSD (FUSE-WP-24, G-18); the solid backdrop must look right on the CPU path.
10. Risk: the shell is the largest ForgeScript program started at login. Start time depends on FORGE-WP-18 (interpreter start-up without JIT).
