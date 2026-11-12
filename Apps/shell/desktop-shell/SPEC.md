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
