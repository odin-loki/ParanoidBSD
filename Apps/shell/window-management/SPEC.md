<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Window Management

By Odin Loch

| | |
|---|---|
| ID | `SH05` |
| Bundle | `org.pbsd.window-management` |
| Category | shell |
| Priority | P0 |
| Backing repos | ParanoidBSD `pbsd/compositor/` (Wayland server, mechanism), FUSE (UI toolkit), FORGE (ForgeScript) |
| Status | Planned. Nothing built. |

## 1. Purpose

SH05 decides where windows go and how they look from the outside: frames and security labels, moving, snapping and tiling, workspaces, the overview, the window switcher, window rules, screen edges, gestures and picture-in-picture. Every user depends on it for every window. It replaces KWin's window management, its effects, its scripts and its rules pages named in the current plan (`docs/plan/PBSD_PLAN.md`, P5-02; `pbsd/compositor/README.md` describes the KWin retirement path).

One thing sets it apart from other window managers: every window carries a label, drawn by the compositor, that says which app and which security domain owns it. An app cannot remove, change or imitate that label.

## 2. Scope

In scope:

- Window frames: borders, the label tab, server-side title bars for apps that need one, the window menu.
- Security labels and the colour, pattern and word vocabulary used across the shell.
- Interactive move and resize, snapping, snap layouts, tiling layouts.
- Workspaces (virtual desktops), the overview, the window switcher.
- Window rules and placement policy.
- Hot corners, screen edges, touchpad and touch gestures, the global shortcut table.
- Picture-in-picture, always-on-top, sticky and other special window states.
- Multi-monitor window behaviour.
- The policy protocol between the compositor and the window policy service.

Out of scope:

| Item | Owner |
|---|---|
| Rendering, buffers, input devices, DRM, frame pacing, colour pipeline | compositor (`pbsd/compositor/`) |
| Display arrangement, resolution, refresh rate, fractional scale, HDR, night light settings | SY01 (SH05 hands over, section 4.11) |
| Panels, task bar, launcher, workspace switcher applet | SH01 |
| OSDs, privacy indicators | SH02 (SH05 asks for the workspace OSD; SH02 asks for the capture outline) |
| Lock, trusted layer, secure attention key | SH03 |
| Portal windows and the "System" strip content | SH04 (SH05 draws the strip and tether) |
| Header bar inside a window (menu button, title, search, badges) | each app through FUSE's chrome kit (DESIGN_SYSTEM.md section 3, FUSE-WP-29) |
| Magnifier and zoom | AX02 |
| Screen capture | ME05 |
| VM and foreign-app windows' content | VM01, VM03 (SH05 labels them) |

## 3. Users and top tasks

Personas:

- **Daily user**: drags windows to edges, uses two or three workspaces, switches with `Alt+Tab`.
- **Keyboard tiler**: wants automatic layouts and never touches the pointer.
- **Multi-monitor worker**: docks and undocks a laptop; wants windows to come back where they were.
- **Security-minded user**: runs disposable sandboxes and VMs side by side and must tell at a glance which window is which.
- **Assistive-technology user**: needs large frames, clear focus, and no pointer-only actions.

| # | Task | Target |
|---|---|---|
| T1 | Put two windows side by side | drag to left edge, pick the other from snap assist (2 steps); or `Super+Left`, `Enter` |
| T2 | Switch to the previous window | `Alt+Tab` (1 step) |
| T3 | Find a window among many | `Super+W`, type, `Enter` |
| T4 | Move a window to another workspace | `Super+Shift+2`; or drag in the overview |
| T5 | Switch workspace | `Super+2`, `Ctrl+Super+Right`, or three-finger swipe |
| T6 | Tell which sandbox owns a window | read its label tab (0 steps); hold `Super` to enlarge all labels |
| T7 | Maximise, restore | double click the header, or `Super+Up` / `Super+Down` |
| T8 | Turn on tiling for this workspace | `Super+Y` (1 step) |
| T9 | Make a window always open on screen 2, workspace 3 | window menu, "Remember for this app…", tick, Save (3 steps) |
| T10 | Keep a video on top while working | window menu, "Picture-in-picture" (2 steps) or `Super+Ctrl+P` |
| T11 | Close a frozen window | `Alt+F4`; after 5 s a "not responding" bar offers Force stop |
| T12 | Move a window to the other monitor | `Super+Shift+Right` |
| T13 | See every window on every workspace | `Super+W` twice, or `Super+G` |
| T14 | Resize two tiled windows together | drag the shared edge (1 step) |

## 4. Features

### 4.1 Frames and decorations
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH05-F001 | Compositor-drawn frame | Every toplevel gets a frame drawn by the compositor outside the client surface: border, label tab, shadow. The client cannot draw there | M | 1 |
| SH05-F002 | Border | 2 px in the label colour (4 px for disposable, VM, foreign and elevated windows); focused windows use full strength, unfocused 60% | M | 1 |
| SH05-F003 | App header bar | Suite apps draw their own header bar inside the surface (DESIGN_SYSTEM.md section 3); the compositor negotiates this with `zxdg_decoration_manager_v1` (`ClientSide`) | M | 1 |
| SH05-F004 | Server-side title bar | Clients that ask for `ServerSide`, or have no decoration support (foreign apps, VM windows), get a 28 px title bar: icon, title, minimise, maximise, close | M | 1 |
| SH05-F005 | Header regions | The app host reports drag regions and button rectangles of its header bar so double click, middle click and right click on the header behave the same in every app | M | 1 |
| SH05-F006 | Window buttons | Minimise, maximise or restore, close; order and side follow one setting; 24 px targets (44 in touch density) | M | 1 |
| SH05-F007 | Header actions | Double click: maximise (setting: shade, minimise, nothing). Middle click: lower. Right click: window menu. Scroll: nothing by default (setting: shade, opacity, workspace) | M | 1 |
| SH05-F008 | Window menu | `Alt+F3` or right click on header or label tab: move, resize, minimise, maximise, tile, keep above or below, sticky, workspace, output, picture-in-picture, remember, permissions, close, force stop | M | 1 |
| SH05-F009 | Shadows and corners | Radius 12 on floating windows, 0 when maximised or tiled without gaps; shadow elevation by focus; all off on the CPU renderer if the frame budget is missed | S | 1 |
| SH05-F010 | Resize edges | 8 px invisible grab zone outside the border, 16 px at corners; cursor shapes through `wp_cursor_shape_manager_v1` | M | 1 |
| SH05-F011 | Not responding | A window whose client misses pings for 5 s is dimmed with a bar "Not responding: [Wait] [Force stop]"; move, minimise and close still work because the frame is the compositor's | M | 1 |
| SH05-F012 | Close always works | `Alt+F4` asks the client to close; if it does not within 5 s the not-responding bar appears. A client cannot block the request, hide the frame or cover it | M | 1 |
| SH05-F013 | Icons | Window icon from the bundle (by verified app id). `xdg_toplevel_icon_v1` icons from the client are used only for foreign apps and are shown inside a "foreign" ring | S | 2 |
| SH05-F014 | Shade | Roll a window up to its header or title bar | C | 3 |
| SH05-F015 | Tabbed windows | Group several windows into one frame with a tab strip drawn by the compositor; each tab keeps its own label colour | C | 3 |
| SH05-F016 | Borderless maximised | A maximised window keeps a 3 px label bar on top instead of the full border | M | 1 |
| SH05-F017 | Fullscreen entry hint | Entering fullscreen shows "App is full screen. Hold Esc or press Super to leave" for 3 s, drawn by the compositor; the label bar reappears when the pointer touches the top edge | M | 2 |
| SH05-F018 | System strip and tether | For SH04 system windows the compositor draws the "System" strip and the tether to the requesting window (SH04-F132, F133) | M | 2 |

### 4.2 Security labels
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH05-F019 | Label source | The compositor learns each client's app id and domain from the connection (`wp_security_context_v1` metadata set by the app host) and asks the launch service for the bundle's trust state. Nothing the client sends later can change it | M | 1 |
| SH05-F020 | Label kinds | `native` (system-key app), `sandboxed` (third-party app, verified signer), `disposable`, `vm`, `foreign` (VM03 personality), `elevated` (holds a live SH03 approval or is an administration tool), `system` (shell and portal surfaces) | M | 1 |
| SH05-F021 | Colour, pattern, letter, word | Each kind has all four: native grey solid `N`; sandboxed blue solid `S`; disposable white dashed `D`; vm one of eight user-set colours, double line `V`; foreign magenta dotted `F`; elevated red solid thick `E`; system reserved pattern. Never colour alone | M | 1 |
| SH05-F022 | Label tab | A tab 18 px high above the frame's top-left corner: kind icon, kind word, real app name, and for VMs and disposables the instance name ("VM: work-debian", "Disposable 3") | M | 1 |
| SH05-F023 | Tab when there is no room | Maximised, tiled-to-top and fullscreen windows fold the tab into the label bar; hovering the bar or holding `Super` for 400 ms shows the full tab over the window | M | 1 |
| SH05-F024 | Modifiers | Extra marks on the tab: `⚠ unverified` or `tampered`, `[net]` with route word (clear, VPN, Tor), `◉` capturing or `●` being captured, `🔒` secret content (excluded from capture), `remote` | M | 2 |
| SH05-F025 | Per-VM colours | Each VM and each named sandbox profile has a colour chosen from eight (set in VM01 or AD05); the same colour is used in the task bar (SH01-F058), switcher and overview | M | 2 |
| SH05-F026 | Label everywhere | Overview tiles, switcher tiles, task entries, snap assist and the screen picker (SH04-F113) show the same kind colour, letter and word | M | 1 |
| SH05-F027 | Label detail | Click the tab: popover with app, signer, domain id, route, devices in use, grants count, "Permissions…" (AD05), "Show in System Monitor" (SY04) | S | 2 |
| SH05-F028 | Child windows | Dialogs and popups inherit the parent's label and cannot extend beyond its output without a frame of their own | M | 1 |
| SH05-F029 | Paste and drop crossing labels | When a drag or paste crosses from a higher-trust kind to a lower one (native to disposable, host to VM) the compositor shows a one-line confirm chip at the drop point; rules per pair are set in AD05 | S | 3 |
| SH05-F030 | Focus colour is not label colour | Focus is shown by border strength and the header's active state, never by changing the label colour | M | 1 |
| SH05-F031 | Unlabelled is impossible | A client with no resolvable identity is labelled `sandboxed ⚠ unknown` and gets the thick border | M | 1 |
| SH05-F032 | Enlarged labels | Holding `Super` for 400 ms shows large labels (app name, kind word, 15 pt) on every visible window, with numbers 1–9 to focus | S | 2 |
| SH05-F033 | Label settings limits | Users may change VM colours and tab size (small, normal, large); they cannot turn labels off. Policy may force "large" | M | 1 |

### 4.3 Move, resize and snapping
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH05-F034 | Pointer move and resize | Drag the header; `Super+drag` anywhere moves; `Super+right-drag` resizes from the nearest corner | M | 1 |
| SH05-F035 | Keyboard move and resize | `Alt+F7` move, `Alt+F8` resize with arrows (`Shift` 1 px, `Ctrl` 100 px), `Enter` accepts, `Esc` reverts | M | 1 |
| SH05-F036 | Size hints | Minimum, maximum, aspect and increment hints honoured; a window never becomes smaller than 160×80 or larger than the output unless spanning is asked | M | 1 |
| SH05-F037 | Edge snap | Drag to left or right edge: half. Top: maximise. Corners: quarter. A translucent preview appears after 150 ms at the edge | M | 1 |
| SH05-F038 | Snap layouts | Dragging near the top centre, hovering the maximise button, or `Super+Z` shows a layout picker: halves, 2/3 + 1/3, thirds, quarters, 3-column, focus + stack | S | 2 |
| SH05-F039 | Snap assist | After snapping one window, the remaining zones show thumbnails of other windows to fill them; `Esc` skips | S | 2 |
| SH05-F040 | Snap groups | Windows snapped together are remembered as a group: raising one raises all; the group appears as one entry in the switcher (setting) | C | 3 |
| SH05-F041 | Linked resize | Dragging the edge shared by two snapped or tiled windows resizes both | M | 2 |
| SH05-F042 | Custom zones | A zone editor per output: split any zone horizontally or vertically, set gaps; saved as named layouts | C | 3 |
| SH05-F043 | Snap by key | `Super+Left/Right` halves (repeat cycles 1/2, 1/3, 2/3), `Super+Up` maximise, `Super+Down` restore then minimise, `Super+Alt+arrows` quarters | M | 1 |
| SH05-F044 | Restore size | Dragging a snapped window away restores its floating size | M | 1 |
| SH05-F045 | Window-to-window snap | Edges snap to other windows and to panels within 8 px; hold `Ctrl` to suppress | S | 2 |
| SH05-F046 | Centre and fit | `Super+C` centres; `Super+Shift+C` centres at 2/3 of the work area | S | 1 |
| SH05-F047 | Placement of new windows | Order: matching rule, saved position for this app and output set, tiling layout if on, else smart placement (least overlap), else cascade; never under a panel, never off-screen | M | 1 |
| SH05-F048 | Remember positions | Size, position, workspace and output saved per app and per output set; restored when the app's window reappears with the same window tag (`xdg_toplevel_tag_v1`) | S | 2 |
| SH05-F049 | Session restore | With SH03's "Reopen my windows", windows return to their workspace, output and geometry using app session tokens (PLATFORM.md section 4) | S | 2 |
| SH05-F050 | Focus policy | Click to focus (default), focus follows pointer, sloppy; raise on click; new windows take focus only with a valid `xdg_activation_v1` token, otherwise they open behind and mark attention | M | 1 |
| SH05-F051 | Focus stealing prevention | Levels: strict (token required, default), normal, off; password fields in focus always force strict | M | 1 |
| SH05-F052 | Minimise, maximise variants | Maximise vertically or horizontally only (middle or right click on the button) | C | 2 |
| SH05-F053 | Keep above, keep below, sticky | Per window; sticky shows on all workspaces | M | 1 |
| SH05-F054 | Opacity | Per-window opacity 20–100% by rule or `Super+scroll` on the header (off by default) | C | 3 |

### 4.4 Tiling layouts
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH05-F055 | Tiling per workspace and output | Toggle with `Super+Y`; each workspace on each output is floating or has a layout | S | 2 |
| SH05-F056 | Layouts | Master and stack (left, top), columns, rows, spiral, binary tree, grid, monocle (one visible, tabs strip), centred master | S | 2 |
| SH05-F057 | Scrolling columns | An endless horizontal strip of columns; focus scrolls the view; each column 1/3, 1/2 or 2/3 wide | C | 3 |
| SH05-F058 | Layout cycle | `Super+Shift+Y` cycles layouts; `Super+Ctrl+Y` opens a layout picker popover (`Super+Space` stays the keyboard-layout key) | S | 2 |
| SH05-F059 | Focus by direction | `Super+H/J/K/L` or `Super+Alt+arrows` in tiling mode move focus left, down, up, right, across outputs | S | 2 |
| SH05-F060 | Swap and move | `Super+Shift+H/J/K/L` swaps with the neighbour; dragging a tile onto another swaps; dropping on an edge splits | S | 2 |
| SH05-F061 | Master controls | Grow or shrink the master ratio (`Super+[`, `Super+]`), change master count (`Super+,`, `Super+.`), promote to master (`Super+M`; `Super+Enter` stays the Terminal key) | S | 2 |
| SH05-F062 | Gaps | Inner and outer gaps 0–32 px; smart gaps (none when one window) | S | 2 |
| SH05-F063 | Float exceptions | Dialogs, fixed-size windows, picture-in-picture and rule-marked apps float above the layout; `Super+Shift+F` toggles float for one window | S | 2 |
| SH05-F064 | Minimum sizes | If hints make a layout impossible, the window floats and a toast says why | S | 2 |
| SH05-F065 | New window position | Setting: becomes master, after focused, at end | S | 2 |
| SH05-F066 | Tabs and stacks inside a tile | Several windows in one tile shown as tabs (uses F015) | C | 3 |
| SH05-F067 | Layout per rule | A rule can set the layout of a workspace or pin an app to a tile slot | C | 3 |
| SH05-F068 | Layout persistence | Layout kind, ratios and order saved per workspace and output set | S | 2 |
| SH05-F069 | Pointer and tiling together | Tiled windows can still be dragged out (they float) and back in (drop zones appear) | S | 2 |
| SH05-F070 | Label bars in tiles | Tiled windows show the label bar and a compact tab at the top-left inside the gap; with zero gaps the tab overlays on `Super` hold | M | 2 |

### 4.5 Workspaces
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH05-F071 | Workspace model | Exposed through `ext_workspace_v1` (`PBSD:pbsd/compositor/pbsd.compositor.wayland.ext_workspace_v1.cppm`); SH01's pager is a client of it | M | 1 |
| SH05-F072 | Fixed or dynamic | Fixed count (default 4, 1–36) or dynamic (always one empty at the end; empty ones in the middle close) | M | 1 |
| SH05-F073 | Names | Workspaces have names and optional icons; rename in the overview with `F2` | S | 2 |
| SH05-F074 | Layout of workspaces | One row, or a grid (rows × columns) for directional switching | S | 2 |
| SH05-F075 | Switch | `Super+1..9`, `Ctrl+Super+Left/Right` (and `Up/Down` in a grid), `Super+Tab` last used, scroll on the pager, gestures | M | 1 |
| SH05-F076 | Move window | `Super+Shift+1..9`, `Ctrl+Super+Shift+arrows` (move and follow), window menu, drag in the overview, drag to a pager button | M | 1 |
| SH05-F077 | Per-output or spanning | Setting: workspaces span all outputs (default), or each output has its own set, or only the primary switches | S | 2 |
| SH05-F078 | Switch animation | Slide 200 ms; none with reduced motion; the workspace name OSD is requested from SH02 (`Osd1`) | S | 1 |
| SH05-F079 | Wrap | Optional wrap-around at the ends | C | 2 |
| SH05-F080 | Windows on all workspaces | Sticky windows; "show on all workspaces" rule | M | 1 |
| SH05-F081 | Activation across workspaces | Activating a window on another workspace switches to it (default) or brings the window here (setting) | M | 1 |
| SH05-F082 | Workspace security profile | A workspace can be bound to a label kind or instance ("only windows of VM work-debian"); other windows cannot be moved in; the wallpaper and panel show the instance colour bar | S | 3 |
| SH05-F083 | Workspace-scoped route | Optional: apps launched from a bound workspace start with that workspace's network route (through SE03); shown in the workspace name ("Research · Tor") | C | 3 |
| SH05-F084 | Empty workspace actions | Dynamic mode closes empty workspaces after 2 s; fixed mode keeps them | S | 2 |
| SH05-F085 | Save and restore | Workspace names, order, layouts and window membership are saved for session restore | S | 2 |
| SH05-F086 | Show desktop | `Super+D` hides all windows of the current workspace; again restores them; a newly opened window ends the state | S | 1 |

### 4.6 Overview
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH05-F087 | Open | `Super+W`, top-left hot corner, three-finger swipe up, or the pager's overview button | M | 1 |
| SH05-F088 | Window grid | Live thumbnails of the current workspace's windows laid out to keep relative position and size; each with icon, title and label tab | M | 1 |
| SH05-F089 | Workspace strip | Thumbnails of all workspaces along the top with names and `[+]`; click switches; drag windows between them | M | 1 |
| SH05-F090 | Search | Typing filters windows by title and app across all workspaces; `Enter` activates the first; the field hands over to the launcher (SH01) with `Tab` | M | 2 |
| SH05-F091 | Keyboard | Arrows move by geometry, `Enter` activates, `Delete` or `Ctrl+W` closes, `1–9` jump, `Ctrl+arrows` change workspace inside the overview | M | 1 |
| SH05-F092 | Pointer | Click activates; middle click closes; drag to a workspace or another output; drag to a screen edge inside the overview to snap there | M | 1 |
| SH05-F093 | All-workspaces grid | `Super+G` or `Super+W` twice: every workspace as a row of its windows | S | 2 |
| SH05-F094 | Per-app mode | `Super+Shift+W`: only windows of the focused app | S | 2 |
| SH05-F095 | Multi-monitor | Each output shows its own windows; the workspace strip is on each output that switches | M | 2 |
| SH05-F096 | Minimised windows | Shown smaller at the bottom, dimmed | S | 2 |
| SH05-F097 | Close button on tiles | `✕` on hover; closing asks the app as usual | M | 1 |
| SH05-F098 | Secret content | Windows marked secret show an icon tile instead of a live thumbnail when `overview.hide_secret` is on or a capture is running | M | 2 |
| SH05-F099 | Group by label | Optional grouping of tiles by label kind and instance, with a coloured band per group | C | 3 |
| SH05-F100 | Thumbnails are compositor-side | The overview UI gets textures by reference from the compositor; no client, including the overview's own script, can read window pixels | M | 1 |
| SH05-F101 | Animation | Windows glide to their tile in 200 ms; none with reduced motion; never above the frame budget (drops to cross-fade) | S | 1 |
| SH05-F102 | Empty workspace | "No windows here" and a `[Open launcher]` button | M | 1 |

### 4.7 Window switcher
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH05-F103 | `Alt+Tab` | Windows in most-recently-used order; release `Alt` to activate; `Alt+Shift+Tab` backwards | M | 1 |
| SH05-F104 | Quick tap | A tap shorter than 150 ms switches to the previous window without showing the switcher | M | 1 |
| SH05-F105 | Tiles | Each tile: thumbnail or icon, title, app name, label colour bar and letter; the selected one is outlined and also highlighted in place on the desktop | M | 1 |
| SH05-F106 | App switcher | `Super+Tab` is last workspace by default; ``Alt+` `` cycles windows of the current app; setting to make `Alt+Tab` group by app | M | 1 |
| SH05-F107 | Scope | Current workspace or all; current output or all; include minimised (settings) | M | 1 |
| SH05-F108 | Keys while open | Arrows, `Home`, `End`, `Q` or `W` closes the selected window, `M` minimises, `Esc` cancels, digits jump | S | 2 |
| SH05-F109 | Pointer | Hover does not select (avoids accidents); click selects and activates | M | 1 |
| SH05-F110 | Layouts | Strip (default), grid for more than 12 windows, compact list with titles only | S | 2 |
| SH05-F111 | Sticky mode | `Ctrl+Alt+Tab` keeps the switcher open until `Enter` or `Esc`, for switch access and one-handed use | M | 1 |
| SH05-F112 | Works when the policy service is down | A minimal built-in switcher (icons and titles) lives in the compositor | M | 1 |
| SH05-F113 | Output and workspace switchers | `Super+O` cycles focus between outputs; `Ctrl+Super+Tab` shows a workspace switcher strip | S | 2 |
| SH05-F114 | Trusted surfaces excluded | SH03 and SH04 trusted prompts are never in the list; a waiting prompt is shown as a fixed first tile "1 system request waiting" | M | 2 |

### 4.8 Window rules
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH05-F115 | Rule model | A rule has matchers and actions; rules are ordered; first match per property wins | M | 2 |
| SH05-F116 | Matchers | App id (verified), window tag, title (exact, contains, simple pattern with `*`), window type (normal, dialog, utility), label kind, VM or sandbox instance, output | M | 2 |
| SH05-F117 | Actions | Position, size, output, workspace, state (maximised, fullscreen, minimised, tiled zone), keep above or below, sticky, float in tiling, no focus on open, opacity, skip switcher, skip task bar, shortcut inhibit allowed, tearing allowed, picture-in-picture | M | 2 |
| SH05-F118 | Apply mode | Per action: apply at open, force always, remember last value | M | 2 |
| SH05-F119 | Create from a window | Window menu "Remember for this app…": a sheet pre-filled from the window's current state with tick boxes | M | 2 |
| SH05-F120 | Rules editor | A page in SY01 with list, editor, test ("which rules match this window?") and import or export | S | 2 |
| SH05-F121 | Pick a window | "Pick…" turns the pointer into a cross-hair; click a window to fill the matchers (compositor-side pick, returns identity only) | S | 2 |
| SH05-F122 | Rules cannot weaken security | No rule can hide a label, remove a frame, let a window cover a panel indicator, grant capture or mark a window trusted | M | 2 |
| SH05-F123 | App-suggested rules | A manifest may suggest default size and "prefers floating"; the user's rules override | C | 3 |
| SH05-F124 | Policy rules | Administrator rules sit above user rules and are read-only | C | 3 |
| SH05-F125 | Rule file | JSON, schema-checked; a broken rule is skipped and reported, never fatal | M | 2 |
| SH05-F126 | Per-kind defaults | Default rules by label kind: disposable windows open centred and never remembered; VM consoles open on their bound workspace | S | 3 |

### 4.9 Edges, corners, gestures and shortcuts
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH05-F127 | Hot corners | Each corner: nothing, overview, launcher, show desktop, lock (SH03), notification centre (SH02), command; default top-left overview, others nothing | S | 2 |
| SH05-F128 | Corner trigger | Needs pointer pressure: 100 px of movement into the corner within 300 ms, or a 250 ms dwell (setting); off while a button is held unless dragging a window; off under fullscreen | S | 2 |
| SH05-F129 | Edge actions | Each edge: nothing, switch workspace (while dragging a window, after 500 ms), reveal panel (SH01), snap; edge barriers between outputs with adjustable resistance | S | 2 |
| SH05-F130 | Touchpad gestures | Three fingers: up overview, down show desktop or minimise, left and right workspace (1:1 tracking). Four fingers: left and right move window to workspace, up all-workspaces grid | S | 2 |
| SH05-F131 | Touch screen gestures | Swipe from left edge: switcher; from top with a window: move or snap; from right: notification centre (SH02); from bottom: panel and launcher (SH01) | S | 3 |
| SH05-F132 | Pinch | Four-finger pinch in: overview | C | 3 |
| SH05-F133 | Global shortcut table | One table owned by SH05 for compositor-level shortcuts of all shell parts (SH01–SH05, AX apps); conflict detection; edited in SY01 Keyboard | M | 1 |
| SH05-F134 | App global shortcuts | Apps cannot grab keys. An app may register named actions; the user binds keys to them in SY01; the compositor delivers the action, not the key | S | 3 |
| SH05-F135 | Reserved keys | `Ctrl+Alt+Delete` (SH03), `Super+L`, `Super` tap and the panic shortcuts cannot be bound by rules or inhibited | M | 1 |
| SH05-F136 | Shortcut inhibit | `zwp_keyboard_shortcuts_inhibit_manager_v1` requests (VM consoles, remote desktop) are granted per app by rule or prompt; a mark shows (SH02-F136); `Super+Esc` always releases | M | 2 |
| SH05-F137 | Pointer lock and confine | `zwp_pointer_constraints_v1` honoured only for the focused window; released by `Super+Esc`, focus loss or any trusted prompt | M | 2 |
| SH05-F138 | Modifier choice | The window-management modifier is `Super` (default) or `Alt` | S | 1 |
| SH05-F139 | Shortcut help | Hold `Super+/` : an overlay sheet lists all window-management shortcuts | S | 2 |

### 4.10 Picture-in-picture and special windows
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH05-F140 | Picture-in-picture | A window state: small, always on top, on all workspaces, no header, snaps to corners, resizable keeping aspect; the label border stays | S | 2 |
| SH05-F141 | PiP controls | Hover shows compositor-drawn controls: back to normal, close, and media play or pause from `svc.media` for that app | S | 2 |
| SH05-F142 | PiP from the app | An app asks for PiP for a sub-surface (video); allowed only for apps with a visible window and a user gesture; one PiP per app | S | 2 |
| SH05-F143 | PiP of any window | "Picture-in-picture" in the window menu scales any window down as a live miniature; click returns to it | C | 3 |
| SH05-F144 | Dodge | PiP moves away from the pointer's corner when a text caret or snap preview needs the space (setting) | C | 3 |
| SH05-F145 | Dialogs | Modal dialogs (`xdg_dialog_v1`) dim and block their parent only; they move with it; never system-modal | M | 1 |
| SH05-F146 | Popups | Menus and tooltips are constrained to the output and to the parent's label frame rules (F028) | M | 1 |
| SH05-F147 | Utility and splash windows | Skip the switcher; float in tiling; no label tab but keep the border | S | 2 |
| SH05-F148 | On-screen keyboard and input panels | AX03 and AX04 surfaces are placed by the compositor and resize the work area when docked | S | 2 |
| SH05-F149 | Fullscreen | Per output; panels hidden; `Super` and hot corners still work; exclusive fullscreen with direct scan-out when nothing overlaps | M | 1 |
| SH05-F150 | Tearing and content type | `wp_tearing_control_v1` and `wp_content_type_v1` hints honoured for fullscreen games and video only when a rule allows tearing | C | 3 |
| SH05-F151 | Capture outline | While a window or output is captured the compositor draws a `--danger` outline around it and a `●` on its label (SH02-F134) | M | 2 |

### 4.11 Multi-monitor, scaling and HDR hand-off
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH05-F152 | Output sets | The set of connected outputs (by EDID hash) is a key; window positions, layouts and workspace mapping are saved per set | M | 2 |
| SH05-F153 | Output removed | Its windows move to the nearest remaining output keeping relative position; they return when the output comes back | M | 1 |
| SH05-F154 | Move between outputs | `Super+Shift+Left/Right`, window menu, drag; size scales with the work area when the window was maximised or snapped | M | 1 |
| SH05-F155 | Mixed scale | A window on two outputs renders at the scale of the output holding most of it (`wp_fractional_scale_v1`); frames and labels are vector and stay sharp | M | 2 |
| SH05-F156 | Spanning | Optional maximise across all outputs for one window (rule or menu) | C | 3 |
| SH05-F157 | Primary output | New windows open on the output with the pointer (default), the focused output, or the primary | M | 1 |
| SH05-F158 | Display settings hand-off | Arrangement, resolution, refresh, scale, rotation, HDR, colour profile and VRR are set in SY01; SH05 applies the resulting work areas and re-places windows; `Super+P` is SH02's chooser | M | 1 |
| SH05-F159 | Confirm display changes | After a mode change SY01 shows a 15 s keep-or-revert dialog; SH05 guarantees the dialog is on an output that was working before | M | 2 |
| SH05-F160 | HDR per window | Window menu shows "HDR content" when a surface uses the colour management protocol (`…wayland.color_management_v1`); the toggle and brightness mapping live in SY01 | C | 3 |
| SH05-F161 | Per-output frame | Border widths and tab sizes are in logical pixels and snap to device pixels per output | M | 1 |
| SH05-F162 | Presentation output | An output marked "presentation" in SY01 shows only windows the user moved there; no new windows, banners or switcher appear on it | S | 3 |

### 4.12 Effects, settings, accessibility, recovery
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH05-F163 | Small fixed effect set | Open, close, minimise, maximise, snap preview, workspace slide, overview glide, dim parent, attention pulse (once). Each ≤ 200 ms. No scripted or third-party effects | M | 1 |
| SH05-F164 | Blur | Backdrop blur only behind shell surfaces (`…wayland.blur_contrast` pipeline); never behind app windows | S | 2 |
| SH05-F165 | Reduced motion | All effects become instant or a 80 ms fade | M | 1 |
| SH05-F166 | Focus aids | Optional thick focus ring, dim unfocused windows 10–40%, pointer-to-focused-window warp on keyboard focus change (off by default) | S | 2 |
| SH05-F167 | Keyboard-only complete | Every action in this spec has a key path; the window menu and move or resize modes are announced by the screen reader | M | 1 |
| SH05-F168 | Settings | Schema in section 8; pages "Windows", "Workspaces", "Tiling", "Edges and gestures", "Window rules" in SY01 | M | 1 |
| SH05-F169 | Policy service crash | The compositor keeps windows, frames and labels, and falls back to built-in policy (floating, cascade placement, basic move, resize, minimal switcher); the service restarts within 1 s and re-reads state | M | 1 |
| SH05-F170 | Slow policy | If the service does not answer a placement query within 50 ms the compositor places the window by fallback and applies the late answer only if the user has not touched the window | M | 1 |
| SH05-F171 | Audit | Label changes, shortcut-inhibit grants, cross-label paste confirmations and force stops are written to `svc.audit` | M | 2 |
| SH05-F172 | Debug view | `Super+Ctrl+Shift+I` (developer setting): overlay listing each window's app id, domain, label, rule matches, geometry and scale | C | 3 |

## 5. GUI

SH05's visible parts are of two kinds. **Compositor-drawn** parts (frames, label tabs, snap previews, outlines, the fallback switcher) are data-driven: the policy service sends a theme and geometry, the compositor paints. **Shell surfaces** (overview, switcher, snap layout picker, window menu, shortcut sheet) are FUSE UI on layer surfaces (`PBSD:pbsd/compositor/pbsd.compositor.wayland.layer.cppm`) that show window thumbnails by reference.

### 5.1 Window map
```
Window Management
├─ Per-window frame (compositor-drawn)
│  ├─ Border · shadow · resize zones
│  ├─ Label tab / label bar · label detail popover
│  ├─ Server-side title bar (foreign and undecorated clients)
│  ├─ Not-responding bar
│  ├─ Capture outline · System strip and tether (for SH04)
│  └─ PiP controls
├─ Window menu (popup)
│  └─ "Remember for this app…" sheet
├─ Snap preview (compositor-drawn) · Snap layout picker · Snap assist
├─ Overview (layer Overlay, one surface per output)
│  ├─ Workspace strip · window grid · search field
│  └─ All-workspaces grid · per-app mode
├─ Window switcher (layer Overlay) · app switcher · workspace switcher strip
│  └─ Built-in minimal switcher (compositor)
├─ Tiling: layout picker popover · drop zones (compositor-drawn)
├─ Enlarged labels overlay (Super held)
├─ Fullscreen entry hint · cross-label paste chip (compositor-drawn)
├─ Shortcut help sheet
├─ Zone editor (per output, phase 3)
├─ Debug overlay
└─ Settings pages in SY01: Windows, Workspaces, Tiling, Edges and gestures, Window rules
   └─ Rule editor · window picker cross-hair
```

### 5.2 Main window
The overview on a 1440×900 output with a bottom panel. This is the most complete single view of what SH05 manages.

```
+----------------------------------------------------------------------------------------+
| [🔍 Type to find a window…                    ] (1)                    All workspaces ▾ |
| +-----------+ +-----------+ +-----------+ +-----------+ +---+                          |
| | 1 Main  ▣ | | 2 Mail    | | 3 Research| | 4 VM work | | + | (2)                      |
| | [▭][▭][▭] | | [▭]       | | [▭][▭]·Tor| | [▭] ▌green| |   |                          |
| +=====▲=====+ +-----------+ +-----------+ +-----------+ +---+                          |
|  ┌N Files┐                         ┌S Web Browser · net clear┐                         |
|  +-----------------------+        +================================+                   |
|  |                       |        ‖                                ‖   ┌D Disposable 3┐|
|  |   Files — Documents   |        ‖   Web Browser — docs      (3)  ‖   +┄┄┄┄┄┄┄┄┄┄┄┄┄┄+|
|  |   (live thumbnail)    |        ‖   (live thumbnail)             ‖   ┆ Document     ┆|
|  |                    [✕]|        ‖                             [✕]‖   ┆ Viewer —     ┆|
|  +-----------------------+        +================================+   ┆ invoice.pdf  ┆|
|                                     selected: 2 px accent outline      +┄┄┄┄┄┄┄┄┄┄┄┄┄┄+|
|  ┌E Disks · elevated┐                                              (4)                 |
|  +━━━━━━━━━━━━━━━━━━━+     ┌V VM: work-debian┐                                         |
|  ┃ Disks             ┃     +═════════════════+        minimised:                       |
|  ┃ (live thumbnail)  ┃     ║ 🔒 secret (5)    ║        [▫ Notes] [▫ Music]   (6)        |
|  +━━━━━━━━━━━━━━━━━━━+     +═════════════════+                                         |
|[◉]|[1][2][3][4][+]| [▤]▁ [◍]▁ [D]▁ [⛁]▁ [V]▁                 |[net 1][🔔]| 14:32 |[▭]|(7)|
+----------------------------------------------------------------------------------------+
```

1. **Search field.** `lineEdit`, focused; typing filters across all workspaces (F090). The drop-down on the right switches scope: this workspace, all workspaces, this app.
2. **Workspace strip.** `listView`, horizontal; each item is a miniature with the name, window miniatures, a route word or an instance colour band when the workspace is bound (F082, F083). The current one is marked `▲`. `[+]` adds a workspace.
3. **Window tiles.** `gridView` with custom geometry (positions come from the layout algorithm, not a regular grid). Each tile: live thumbnail (`viewportWidget` showing a compositor texture by reference), a close `toolButton` on hover, and the label tab above it in the same colour, pattern, letter and word as on the real window.
4. **Label patterns.** Solid (native `N`, sandboxed `S`), dashed (disposable `D`), thick (elevated `E`), double (VM `V`, in the VM's colour), dotted (foreign `F`).
5. **Secret content.** Tile shows an icon and "secret" instead of pixels (F098).
6. **Minimised windows.** Small dimmed tiles, `listView`.
7. **Panel.** Stays visible; SH01's task entries show the same label bars.

Sizes: workspace strip 96 px high; tiles keep aspect; minimum tile 160×100; at most 7×5 tiles before paging.

States: empty workspace (F102); searching with no match ("No window matches 'x'. Press Tab to search apps."); thumbnails unavailable (icon tiles with titles); capture running (secret tiles forced); policy service restarting (the compositor's minimal switcher is shown instead of the overview); narrow output (< 640 px: strip becomes a `comboBox`, tiles in one column).

### 5.3 Other views and dialogs
#### 5.3.1 Window frame anatomy (compositor-drawn; a 640×360 suite app window, focused)
```
  ┌ S  Web Browser · Tor  ◉ ┐ (1)
  +==================================================================+ (2)
  ‖ [=] Web Browser — docs.example.org   [search ⌘K]   [net][▁][▢][✕] ‖ (3)
  ‖------------------------------------------------------------------‖
  ‖                                                                  ‖
  ‖                        client surface                            ‖ (4)
  ‖                                                                  ‖
  +==================================================================+
   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░ shadow ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░   (5)
  Foreign or undecorated client, with a server-side title bar:
  ┌ F  Linux: gimp ┐
  +┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈+
  ┊ (icon) image.xcf — GIMP                              [▁][▢][✕]   ┊ (6)
  ┊------------------------------------------------------------------┊
  ┊                        client surface                            ┊
  +┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈+
  Maximised: the tab folds into a 3 px label bar
  ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀ (7)
  | [=] Web Browser — docs.example.org   [search ⌘K]   [net][▁][▢][✕] |
```
1. **Label tab.** 18 px high, outside the surface: letter, kind word, verified app name, modifiers (route, capture). Click opens the label detail popover.
2. **Border.** Label colour and pattern; 2 or 4 px.
3. **App header bar.** Drawn by the app (FUSE chrome kit). The app reports the drag region and button rectangles (F005).
4. **Client surface.** The app's own pixels. Nothing of the frame is inside it.
5. **Shadow.** Elevation by focus.
6. **Server-side title bar.** 28 px, compositor-drawn, for clients that ask for or need it.
7. **Label bar.** For maximised, top-tiled and fullscreen windows; expands to the full tab on hover or `Super` hold.

States: focused; unfocused (60% strength); attention (one pulse, then a steady `--warn` dot on the tab); not responding (dim plus bar); being captured (`--danger` outline outside the border); capturing (`◉` on the tab); modal child open (parent dimmed); moving or resizing (size readout "960 × 540" near the pointer).

#### 5.3.2 Snap layout picker and snap assist (1440×900)
```
+----------------------------------------------------------------------------------------+
|                    +----------------------------------------------+                    |
|                    | [▌▐] [▌▌▐] [▌▐▐] [▌│▐] [▚▞] [▌▐═] [▣]   (1) |                    |
|                    +----------------------------------------------+                    |
| +===========================================+ +---------------------------------------+|
| ‖ ┌S Web Browser┐                           ‖ | Choose a window for this side    (3)  ||
| ‖                                           ‖ | +-------------+  +-------------+      ||
| ‖   snapped window (left half)         (2)  ‖ | |N Files      |  |N Terminal   |      ||
| ‖                                           ‖ | | (thumb)     |  | (thumb)     |      ||
| ‖                                           ‖ | +-------------+  +-------------+      ||
| ‖                                           ‖ | +-------------+                       ||
| ‖                                           ‖ | |D Disposable3|      [Skip  Esc]      ||
| ‖                                           ‖ | +-------------+                       ||
| +===========================================+ +---------------------------------------+|
|[◉]|[1][2][3][4]| … panel …                                                             |
+----------------------------------------------------------------------------------------+
```
1. Layout picker: a row of `toolButton`s with zone diagrams (halves, 2/3 + 1/3, 1/3 + 2/3, thirds, quarters, focus + stack, maximise). Hovering a zone previews it; dropping or clicking applies. Opens under the top edge while dragging, under the maximise button on hover, or centred on `Super+Z` (arrow keys and `Enter`).
2. Snapped window with its frame.
3. Snap assist: `gridView` of candidate windows with label tabs; arrows and `Enter`, or click; `Esc` skips.

#### 5.3.3 Window switcher (`Alt+Tab`; strip layout, centred, up to 1200 px wide)
```
+----------------------------------------------------------------------------------------+
|   +--------------------------------------------------------------------------------+   |
|   | +------------+ +============+ +------------+ +┄┄┄┄┄┄┄┄┄┄┄┄+ +━━━━━━━━━━━━+     |   |
|   | | (thumb)    | ‖ (thumb)    ‖ | (thumb)    | ┆ (thumb)    ┆ ┃ (thumb)    ┃     |   |
|   | |            | ‖            ‖ |            | ┆            ┆ ┃            ┃     |   |
|   | +------------+ +============+ +------------+ +┄┄┄┄┄┄┄┄┄┄┄┄+ +━━━━━━━━━━━━+     |   |
|   |  N Files       S Web Browser   N Terminal     D Doc. Viewer   E Disks          |   |
|   |  Documents    >docs.example…   ~/src          invoice.pdf     da1          (1) |   |
|   +--------------------------------------------------------------------------------+   |
|   Web Browser — docs.example.org · sandboxed · Tor · workspace 1                 (2)   |
+----------------------------------------------------------------------------------------+
```
1. Tiles, `gridView` in one row: thumbnail with the label border pattern, letter and app name, window title (elided in the middle). `>` marks the selection, which also gets the accent outline.
2. Detail line, `label`: full title, label kind word, route, workspace.

Variants: grid (more than 12 windows), compact list (`modelListView` with icon, title, label letter), app switcher (one tile per app with a count), workspace strip. The minimal built-in switcher is a single row of icons and titles drawn by the compositor.

#### 5.3.4 Window menu and "Remember for this app…" sheet
```
+--------------------------------+   +------------------------------------------------------+
| Move                    Alt+F7 |   | Remember for Document Viewer                         |
| Resize                  Alt+F8 |   | Applies to  (•) all its windows                      |
| Minimise               Super+H |   |             ( ) windows titled like [invoice*      ] |
| Maximise              Super+Up |   | [x] Open on workspace   [3 Research        v]        |
| Tile ▸  Left Right Top-left …  |   | [x] Open on output      [DP-2 (Dell 27)    v]        |
|--------------------------------|   | [x] Size and position   [960 × 1040 at 960,0]        |
| [ ] Keep above                 |   | [ ] Open maximised      [ ] Keep above               |
| [ ] Keep below                 |   | [ ] Float in tiling     [ ] Show on all workspaces   |
| [ ] On all workspaces          |   | [ ] Do not take focus when opening                   |
| Move to workspace ▸            |   | Apply  [At open v]                                   |
| Move to output ▸               |   | Labels, frames and capture rules cannot be changed   |
| Picture-in-picture Super+Ctrl+P|   | here.                                                |
|--------------------------------|   | [All rules…]                    [Cancel]   [Save]    |
| Remember for this app…         |   +------------------------------------------------------+
| Show permissions               |
| Close                   Alt+F4 |
| Force stop…                    |
+--------------------------------+
```
The menu is a `popupMenu` drawn by SH05's shell surface host at the position the compositor gives. The sheet is a `dialog` with `radioButton`, `checkBox`, `comboBox`, `lineEdit`. "Force stop…" asks once ("Stop Document Viewer? Unsaved work in it will be lost.") and calls SY04's service.

#### 5.3.5 Tiling: master and stack with gaps (1440×900), and the layout picker popover
```
+----------------------------------------------------------------------------------------+
| ┌S Web Browser┐                              ┌N Terminal┐                               |
| +==========================================+ +---------------------------------------+ |
| ‖                                          ‖ |                                       | |
| ‖                                          ‖ |             stack 1                   | |
| ‖              master                      ‖ +---------------------------------------+ |
| ‖                                       ↔ (1) ┌N Files┐                                |
| ‖                                          ‖ +---------------------------------------+ |
| ‖                                          ‖ |             stack 2                   | |
| +==========================================+ +---------------------------------------+ |
|            +--------------------------------------------------------------+            |
|            | Layout: [Master+stack] [Columns] [Rows] [Spiral] [Tree] [Grid] [Monocle] (2)|
|            | Master ratio [=======|----] 60%   Masters [1 ^v]   Gaps [8 ^v]              |
|            +--------------------------------------------------------------+            |
|[◉]|[1][2][3][4]| … panel …                                          | tiling ▦ | 14:32 |
+----------------------------------------------------------------------------------------+
```
1. Shared edge: dragging resizes both sides (F041).
2. Layout picker popover (`Super+Ctrl+Y`): `tabBar` of layouts, `slider`, two `spinBox`es. Dragging a window in tiling mode shows drop zones (compositor-drawn translucent rectangles: swap, split left, right, top, bottom, tab into).

#### 5.3.6 Picture-in-picture, enlarged labels and the fullscreen hint
```
  PiP (320×180, bottom right)        Super held: enlarged labels            fullscreen hint
  +━━━━━━━━━━━━━━━━━━━━━━━━+        +--------------------------+   +--------------------------------+
  ┃ (video)                ┃        | 1  Web Browser           |   | Video is full screen.          |
  ┃   [⤢] [⏯] [✕]     (1)  ┃        |    sandboxed · Tor       |   | Hold Esc or press Super to     |
  +━━━━━━━━━━━━━━━━━━━━━━━━+        +--------------------------+   | leave.                         |
   N Video                          (one card centred on each       +--------------------------------+
                                     visible window; digit focuses)
```
1. Compositor-drawn controls on hover: return to the normal window, play or pause, close. The border keeps the label colour; the letter and app name sit under the PiP.

#### 5.3.7 Window rules page (in SY01, 900×600 content area)
```
+----------------------+---------------------------------------------------------------+
| [🔍 Filter rules   ] | Rule: Document Viewer on Research                  [on ●]     |
| > Document Viewer →3 | Match                                                         |
|   Terminal float     |  App        [Document Viewer ✓        v] [Pick a window…]     |
|   VM consoles →4     |  Title      [any v] [                 ]                       |
|   Disposable centred |  Kind       [any label kind           v]  Type [Normal v]     |
|   (policy) Kiosk  🔒 |  Output     [any                      v]                      |
|                      | Do                                                            |
|                      |  [x] Workspace     [3 Research v]        [At open        v]   |
|                      |  [x] Output        [DP-2       v]        [At open        v]   |
|                      |  [x] Geometry      [960×1040 at 960,0]   [Remember last  v]   |
|                      |  [ ] Keep above   [ ] Float in tiling   [ ] Skip switcher     |
|                      |  [ ] Allow shortcut inhibit   [ ] Allow tearing               |
| [+] [−] [↑] [↓]      | Test: [Pick a window…]  → matches rules 1, 4                  |
| [Import…] [Export…]  |                                   [Revert]   [Apply]          |
+----------------------+---------------------------------------------------------------+
```
`modelListView` of rules with drag reordering, a form of `comboBox`, `lineEdit`, `checkBox`, `toggleSwitch`. Policy rules show a padlock and are read-only. States: no rules ("No rules yet. Use a window's menu and choose 'Remember for this app…'"), a broken rule (`--warn` row with the reason).

### 5.4 Menus
No menu bar. Menus (`popupMenu`):

| Menu | Items (shortcut) |
|---|---|
| Window menu | Move (`Alt+F7`) · Resize (`Alt+F8`) · Minimise (`Super+H`) · Maximise / Restore (`Super+Up` / `Super+Down`) · Tile ▸ (Left `Super+Left`, Right `Super+Right`, quarters `Super+Alt+arrows`, Layouts… `Super+Z`) · Keep above · Keep below · On all workspaces · Move to workspace ▸ (`Super+Shift+1..9`) · Move to output ▸ (`Super+Shift+Left/Right`) · Float (`Super+Shift+F`, tiling only) · Picture-in-picture (`Super+Ctrl+P`) · Remember for this app… · Show permissions · Close (`Alt+F4`) · Force stop… |
| Label tab | Label details · Show permissions · Show in System Monitor · Stop capture (when capturing) · Copy app id |
| Maximise button (right click) | Maximise vertically · Maximise horizontally · Snap layouts… |
| Overview tile | Activate (`Enter`) · Close (`Delete`) · Move to workspace ▸ · Move to output ▸ · Minimise · Window menu ▸ |
| Workspace miniature | Switch · Rename (`F2`) · New workspace after · Close (windows move left) · Bind to… (label kind or instance) · Set route… |
| Empty desktop edge (zone editor) | Split horizontally · Split vertically · Merge · Set gap · Save layout as… |
| Switcher tile | Activate · Close (`W`) · Minimise (`M`) |

### 5.5 Keyboard and pointer
All are in the global shortcut table (F133) and rebindable, except the reserved ones (F135).

| Keys | Action |
|---|---|
| `Alt+Tab` / `Alt+Shift+Tab` / `Ctrl+Alt+Tab` | Switcher forward / back / sticky |
| ``Alt+` `` | Windows of the current app |
| `Super+Tab` / `Ctrl+Super+Tab` | Last workspace / workspace switcher |
| `Super+W` / `Super+G` / `Super+Shift+W` | Overview / all workspaces / this app |
| `Super+1..9` / `Super+Shift+1..9` | Go to workspace / move window there (the task bar's `Super+1..9` in SH01 applies only when `shortcuts.super_digits = taskbar`; default is workspaces when more than one exists) |
| `Ctrl+Super+arrows` / `Ctrl+Super+Shift+arrows` | Neighbour workspace / move window and follow |
| `Super+arrows`, `Super+Alt+arrows`, `Super+Z` | Snap halves and maximise, quarters, layout picker |
| `Super+Shift+Left/Right` / `Super+O` | Move window to output / focus next output |
| `Super+H` / `Super+D` | Minimise / show desktop |
| `Super+C` | Centre |
| `Alt+F3` / `Alt+F4` / `Alt+F7` / `Alt+F8` | Window menu / close / move / resize |
| `Super+Y` / `Super+Shift+Y` / `Super+Ctrl+Y` | Tiling on or off / next layout / layout picker |
| `Super+H/J/K/L` in tiling (`Super+H` minimise moves to `Super+N`) | Focus by direction |
| `Super+Shift+H/J/K/L`, `Super+[`, `Super+]`, `Super+,`, `Super+.`, `Super+M`, `Super+Shift+F` | Swap, master ratio, master count, promote, float |
| `Super+Ctrl+P` | Picture-in-picture |
| `Super+Esc` | Release shortcut inhibit and pointer lock |
| `Super` (hold 400 ms) / `Super+/` (hold) | Enlarged labels / shortcut help |
| `F11` | Fullscreen (delivered to the app; `Super+F11` forces it from the compositor) |

Pointer: header double, middle and right click (F007); `Super+drag` move, `Super+right-drag` resize, `Super+middle-click` lower; drag to edges and corners (snap); drag onto the layout picker; drag tiles in the overview to workspaces and outputs; drag shared edges. Every drag has a key path. Gestures as in F130–F132.

### 5.6 States
| State | Behaviour |
|---|---|
| Empty | No windows: overview shows the empty message; switcher does not open (a short "No other windows" toast on `Alt+Tab`) |
| Loading | A launched app shows SH01's starting entry; no placeholder window. Thumbnails appear as textures become available |
| Error | Broken rule skipped with a notification; policy service down: fallback policy (F169) |
| Offline | No effect |
| Locked | SH03's lock covers all outputs; SH05 shortcuts are inactive except the reserved ones |
| Read-only / policy | Managed rules and settings show a padlock |
| Trusted prompt up | Move, resize, switcher and overview are suspended; only the prompt takes input (SH03-F089) |
| Capture running | Outline and marks shown; secret windows excluded |
| Narrow output | Snap offers halves only (no quarters under 960 px width); the label tab shows the letter and app name without the kind word |
| Low performance | Shadows, blur and glide are dropped in that order when the frame budget is missed for 1 s |

### 5.7 Widgets
| Region | FUSE widget |
|---|---|
| Overview and switcher tiles, snap assist | `gridView` (custom geometry), `viewportWidget` (external texture), `image`, `label`, `toolButton` |
| Workspace strip, minimised row, compact switcher | `listView`, `modelListView` |
| Search field | `lineEdit` |
| Layout pickers | `toolButton`, `tabBar`, `slider`, `spinBox` |
| Window menu and context menus | `popupMenu` |
| Remember sheet, force-stop confirm | `dialog`, `radioButton`, `checkBox`, `comboBox`, `lineEdit` |
| Rules page | `modelListView`, `comboBox`, `checkBox`, `toggleSwitch`, `splitter` |
| Shortcut help sheet | `tableView` |
| Label detail popover | `propertyGrid` (read-only), `button` |
| Frames, tabs, previews, outlines, hints, PiP controls, fallback switcher | not FUSE widgets: compositor-drawn from theme data (FUSE draw list reused in the compositor is Proposed, SH05-GAP-3) |

Missing:

| Gap | What is missing | Where it lands |
|---|---|---|
| SH05-GAP-1 | Window policy protocol between compositor and policy service (events: map, unmap, hints, output change, shortcut; requests: place, state, stack, workspace, theme and zone data) | compositor |
| SH01-GAP-1 | Toplevel management for other shell clients; SH05's protocol can serve SH01 too | compositor |
| SH05-GAP-2 | Thumbnails as textures by reference for shell surfaces (same as SH01-GAP-2, SH04-GAP-3) | compositor; FUSE-WP-10.8 |
| SH05-GAP-3 | Compositor-side frame painter: borders with patterns, label tabs with text, title bars, outlines. Needs text in the compositor (FUSE CPU rasteriser or draw list linked in) | compositor; FUSE-WP-23, FUSE-WP-25 |
| SH05-GAP-4 | Verified client identity: app host sets security-context metadata; compositor maps it to bundle and domain through the launch service | compositor, G-07 |
| SH05-GAP-5 | Gesture recognition in the native compositor. A pointer-gestures module exists only under `PBSD:pbsd/kde/kwin/wayland/pointer_gestures.cppm` | compositor |
| SH05-GAP-6 | Header region reporting from the FUSE chrome kit (drag region, button rectangles) | FUSE-WP-29, FUSE-WP-07 |
| SH05-GAP-7 | `gridView` with caller-supplied item geometry and animated moves | FUSE-WP-28, FUSE-WP-22 |
| SH05-GAP-8 | Compositor global shortcut table with "modifier tap" and "hold" detection, and named-action delivery to apps | compositor |
| SH05-GAP-9 | Cross-label drag and paste hook in the data device path | compositor |

## 6. Architecture

### 6.1 Components
| Component | Language | Process | Responsibility |
|---|---|---|---|
| Compositor mechanism | C++23 modules (`pbsd/compositor/`) | compositor | Surfaces, stacking, input routing, interactive move and resize, animation, frame painting, thumbnails, fallback policy, shortcut table |
| Frame painter | C++23 | compositor | Paints borders, label tabs, title bars, outlines, previews, hints from a theme record and per-window label record |
| Identity resolver | C++23 | compositor, asking the launch service | Maps a client connection to app id, domain id, label kind and modifiers |
| Fallback policy | C++23 | compositor | Cascade placement, floating only, minimal switcher; used when the policy service is absent or late |
| Window policy service | ForgeScript | SH05 app host, `shell.window-policy` | Placement, rules, snapping zones, tiling layouts, workspace logic, focus policy decisions, session restore |
| Layout engine | ForgeScript (pure functions) with a C++23 mirror for the proof target | SH05 app host | Computes rectangles for tiling layouts, snap zones and overview tiles |
| Shell surfaces | ForgeScript + FUSE UI | SH05 app host, `shell.layer` | Overview, switcher, pickers, window menu, sheets |
| Rules store | ForgeScript | SH05 app host | Loads, validates and matches rules |
| Settings pages | ForgeScript + FUSE UI | loaded by SY01 | Windows, Workspaces, Tiling, Edges and gestures, Window rules |

Why the split: anything that must hold when a script crashes or is slow (frames, labels, input routing, close, lock, trusted layer) is compositor mechanism in C++23. Anything that is a preference (where a window goes, which layout) is policy in ForgeScript. Interactive drags never cross the process boundary per frame: the policy service sends zone rectangles and constraints ahead of time and the compositor tracks the pointer against them.

```
 clients (apps) ── xdg-shell, decoration, activation, security context ──▶ +------------------+
                                                                           |   compositor     |
 +-------------------------------+   policy protocol (SH05-GAP-1)          |  mechanism       |
 | SH05 app host (ForgeScript)   |◀── map/unmap/hints/output/shortcut ─────|  frame painter   |
 |  window policy service        |─── place/state/workspace/zones/theme ──▶|  identity        |
 |  layout engine · rules store  |                                         |  resolver        |
 |  shell surfaces (FUSE UI):    |─── layer surfaces; textures by ref ────▶|  fallback policy |
 |  overview, switcher, pickers  |                                         |  shortcut table  |
 +----+-------------+------------+                                         +----+--------+----+
      | bus         | bus                                                       |        |
 +----v-----+  +----v-----------+   SH01: ext_workspace, toplevel list     launch service  DRM,
 | settings |  | SH02 Osd1,     |   SH03: trusted layer, lock              (bundle, domain) input
 | audit    |  | Indicators1    |   SH04: system strip, tether, chips
 +----------+  +----------------+
```

### 6.2 Data model
Settings in `svc.settings` namespace `org.pbsd.window-management`; state in `~/.local/share/org.pbsd.window-management/state.db` (SQLite).

| Entity | Fields |
|---|---|
| `WindowInfo` (from the compositor, memory) | `id: u64`, `app_id: string` (verified), `domain_id: string`, `tag: string?`, `title: string`, `type: enum{normal,dialog,utility,splash,popup}`, `parent: u64?`, `geometry: rect`, `min_size`, `max_size`, `aspect?`, `state: flags{maximised_h,maximised_v,fullscreen,minimised,tiled,sticky,above,below,pip,shaded,attention,unresponsive}`, `workspace: u32`, `output: string`, `label: Label` |
| `Label` | `kind: enum{native,sandboxed,disposable,vm,foreign,elevated,system}`, `instance: string?`, `colour_index: u8?`, `modifiers: flags{unverified,tampered,net,capturing,captured,secret,remote}`, `route: enum{none,clear,vpn,tor}` |
| `FrameTheme` (sent to the compositor) | `border_px: [u8; per kind]`, `pattern: [enum{solid,dashed,dotted,double,thick}; per kind]`, `colours: [rgba; per kind and vm index]`, `tab_height: u8`, `tab_size: enum{small,normal,large}`, `radius: u8`, `shadow: [elevation params]`, `button_order: string`, `font_size: u8` |
| `Workspace` | `id: u32`, `name: string`, `icon: string?`, `index: u16`, `bound_kind: Label.kind?`, `bound_instance: string?`, `route: enum?`, `layout_by_output: map<output_key, LayoutState>` |
| `LayoutState` | `mode: enum{floating,master_stack,columns,rows,spiral,tree,grid,monocle,centred_master,scrolling}`, `master_ratio: f32`, `master_count: u8`, `gap_inner: u8`, `gap_outer: u8`, `order: [window ref]`, `tree: json?` |
| `Rule` (JSON) | `id: uuid`, `name`, `enabled`, `source: enum{user,policy,manifest}`, `match: {app_id?, tag?, title?: {op: enum{is,contains,pattern}, value}, type?, kind?, instance?, output?}`, `actions: [{prop: enum, value, apply: enum{at_open,force,remember}}]` |
| `saved_geometry` (SQLite) | `app_id TEXT`, `tag TEXT`, `output_set TEXT`, `x,y,w,h INTEGER`, `state INTEGER`, `workspace INTEGER`, `output TEXT`, `updated_at INTEGER` |
| `ZoneLayout` (settings) | `name`, `output_key`, `zones: [{x,y,w,h: f32 (fractions of the work area)}]`, `gap: u8` |
| `Shortcut` (compositor table, from settings) | `action_id: string`, `keys: [chord]`, `kind: enum{press,tap,hold}`, `hold_ms: u16?`, `owner: enum{sh01..sh05,ax,app}`, `reserved: bool` |
| `EdgeConfig` (settings) | `corners: {tl,tr,bl,br: action}`, `edges: {top,bottom,left,right: action}`, `pressure_px: u16`, `dwell_ms: u16` |
| `SessionWindows` (for restore) | `[{app_id, session_token, tag, workspace, output, geometry, state}]` |

Rule and layout documents carry `schema: u32`; migration is forward only with a `.bak.<version>` copy.

### 6.3 File formats and protocols
| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| xdg-shell, xdg-decoration, xdg-activation, xdg-dialog, toplevel tag, toplevel icon | server | server | compositor modules in `pbsd/compositor/` | compositor |
| Window policy protocol (Proposed, SH05-GAP-1) | both | both | new compositor module; client side in `fuse_app_shell` or a small C++23 binding | compositor validates every request; policy service is the client |
| `ext_workspace_v1`, `ext_foreign_toplevel_list_v1` | server | server | compositor modules | compositor |
| Rule files and zone layouts (JSON) | yes | yes | ForgeScript JSON with schema | SH05 app host |
| Policy rule file (JSON, signed) | yes | no | same, after signature check by the app host | SH05 app host |
| Frame theme record | — | yes | typed record over the policy protocol | compositor (bounds-checked) |
| Window titles and client icons (untrusted text and pixels) | yes | — | titles: length-limited, control characters stripped, direction isolated; client icons only for foreign apps, fixed sizes, raw buffers | compositor |
| Saved geometry (SQLite) | yes | yes | SQLite | SH05 app host |

### 6.4 Integration
- Intents: none. MIME types: none.
- Interfaces offered: `org.pbsd.wm.Windows1` (for SH01: `activate`, `minimise`, `close`, `move_to_workspace`, `move_to_output`, `set_above`, list with labels), `org.pbsd.wm.Workspaces1` (`add`, `remove`, `rename`, `bind`), `org.pbsd.wm.Shortcuts1` (register shell actions; read and write bindings for SY01), `org.pbsd.wm.Labels1` (read-only label vocabulary and per-window labels for SH01, SH02, SH04), `org.pbsd.wm.Rules1` (for the SY01 page), `org.pbsd.wm.Chips1` (SH04 request chips on a window header).
- Services used: `svc.settings`, `svc.audit`, `svc.media` (PiP play or pause), `svc.a11y`, the launch service (identity), SH02 `Osd1` and `Indicators1`, SY04's stop service.
- Other apps: SH01 (task bar, pager, launcher hand-off from overview search, `Super` tap), SH02 (workspace OSD, capture outline, input-capture mark), SH03 (trusted layer, lock, reserved keys, session restore choice), SH04 (system strip, tether, chips, screen picker labels), SY01 (settings pages, display hand-off), SY04 (force stop, "Show in System Monitor"), AD05 (permissions, VM and sandbox colours, cross-label rules), VM01 and VM03 (instance names and colours), VM04 (remote modifier, shortcut inhibit), SE03 (route words, workspace routes), AX02 (zoom follows focus), AX03 and AX04 (input panels), ME05 (capture).
- Compositor (module files Verified present in `PBSD:pbsd/compositor/`; headers read, server completeness Unverified):

| Module | Used for |
|---|---|
| `…wayland.xdg.cppm`, `…wayland.xdg_dialog_v1.cppm` | Toplevels, popups, modal dialogs (F145) |
| `…wayland.xdg_decoration.cppm` (`zxdg_decoration_manager_v1`, modes `ClientSide`, `ServerSide`) | F003, F004 |
| `…wayland.server_decoration.cppm` (`org_kde_kwin_server_decoration`) | KDE-era protocol; kept only for foreign clients that use it |
| `…wayland.xdg_activation.cppm` | Focus policy (F050, F051) |
| `…wayland.ext_workspace_v1.cppm` | F071 |
| `…wayland.foreign_toplevel.cppm` (`ext_foreign_toplevel_list_v1`) | Listing for SH01 |
| `…wayland.xdgtopleveltag_v1.cppm` | Stable window tags for rules and restore (F048, F116) |
| `…wayland.xdg_toplevel_icon_v1.cppm` | F013 |
| `…wayland.security_context.cppm` | Identity (F019) |
| `…wayland.cursor_shape.cppm` | Resize and move cursors (F010) |
| `…wayland.fractional_scale.cppm`, `…wayland.viewporter.cppm`, `…wayland.output.cppm` | Mixed scale and outputs (4.11) |
| `…wayland.keyboard_shortcuts_inhibit.cppm`, `…wayland.pointer_constraints.cppm`, `…wayland.relative_pointer.cppm` | F136, F137 |
| `…wayland.color_management_v1.cppm`, `…wayland.content_type.cppm`, `…wayland.tearing_control_v1.cppm`, `…wayland.presentation.cppm` | F150, F160 |
| `…wayland.blur_contrast.cppm`, `pbsd.compositor.aero.cppm` | Blur parameters and the Aero theme hooks (tint, blur, shadow, snap animation flags per `pbsd/compositor/README.md`); the frame theme here would replace the KWin-metadata bridge |
| `pbsd.compositor.frame.pacing.cppm` | Frame budget for effects |
| `pbsd.compositor.kde.bridge.cppm`, `pbsd.compositor.theme.sync.cppm` | Interim KWin and Plasma bridge; retired at roadmap milestone M2 |

Not found in `pbsd/compositor/`: a policy or management protocol, gestures, capture, idle notify, session lock (gaps above and in SH01, SH03).

## 7. Security and privacy

Capabilities requested:

| Capability | Why |
|---|---|
| `fs.app` | Saved geometry, caches |
| `net.none` | No network use |
| `svc.settings`, `svc.audit`, `svc.media`, `svc.a11y` | As in 6.4 |
| `bg.run`, `bg.autostart` | Session component |
| `shell.layer` (Proposed) | Overview, switcher and picker surfaces |
| `shell.window-policy` (Proposed) | Bind the policy protocol: place and manage windows |
| `shell.toplevels`, `shell.workspaces` (Proposed) | Window and workspace lists |
| `shell.shortcuts` (Proposed) | Register and edit the global shortcut table |

Not requested: `fs.portal`, any `net.*`, `proc.*`, `dev.*` (no screen capture: thumbnails are textures the shell surfaces cannot read back), `svc.keyring`, `svc.clipboard`.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| The label's truth | Hostile app | Draws a fake label, border or title bar inside its surface to look like a trusted or different app | The real label is outside the surface in a region no client can draw in (F001); `Super` hold shows compositor-drawn labels over everything (F032); users are taught that the tab above the window is the only label |
| The label's truth | Hostile app | Lies about its app id | Identity from the connection and the launch service, not from `set_app_id` (F019); unknown identity gets the warning label (F031) |
| The label's truth | Compromised policy service | Sends a theme that hides labels | The compositor enforces minimum border widths, tab presence and contrast; the theme can restyle but not remove; label kind is computed in the compositor, not by the policy service |
| Trusted prompts | Hostile app | Covers, imitates or outlasts them | Stacking order is compositor mechanism: trusted layer above everything; fullscreen hint; no client can request "always on top" without the user or a rule |
| Focus and typed secrets | Hostile app | Steals focus as the user types a password | Activation tokens required; strict mode while a password field has focus (F051) |
| Input | Hostile app | Grabs the keyboard or pointer for good | Inhibit and lock only by grant; `Super+Esc` and reserved keys always work (F135–F137) |
| Window contents | Shell script bug or hostile applet | Reads thumbnails | Textures by reference, no read-back path (F100); secret windows excluded |
| Data crossing domains | User error | Pastes a secret into a disposable or VM window | Cross-label confirm chip (F029) |
| Availability | Hostile app | Opens thousands of windows or resizes in a loop | Per-client limits (200 toplevels, configure-rate cap), not-responding handling, force stop |
| Window titles | Hostile app | Control characters or right-to-left override to disguise a title in the switcher | Titles sanitised in the compositor; app name shown separately from verified identity |
| Rules | Malicious rule file | A rule that hides or mislabels | Rules cannot touch labels, frames or capture (F122); schema-checked |
| Policy protocol | Any client | Binds the protocol and moves other windows | Global is visible only to a client whose security context carries `shell.window-policy` |

Stored: rules, layouts, saved geometry, workspace names, shortcut bindings. Window titles are not stored. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `frame.button_order` | string | `"min,max,close"` right | F006 |
| `frame.double_click` | enum{maximise,shade,minimise,nothing} | maximise | F007 |
| `frame.middle_click` | enum{lower,minimise,nothing} | lower | F007 |
| `frame.shadows` | bool | true | F009 |
| `label.tab_size` | enum{small,normal,large} | normal | F033 |
| `label.vm_colours` | map<instance, u8> | assigned in order | F025 |
| `label.super_hold_ms` | u16 | 400 | F032 |
| `focus.mode` | enum{click,follows_pointer,sloppy} | click | F050 |
| `focus.raise_on_click` | bool | true | F050 |
| `focus.stealing_prevention` | enum{strict,normal,off} | strict | F051 |
| `focus.dim_unfocused` | u8 (0–40) | 0 | F166 |
| `snap.edges` | bool | true | F037 |
| `snap.assist` | bool | true | F039 |
| `snap.layout_picker` | bool | true | F038 |
| `snap.threshold_px` | u8 | 8 | F045 |
| `snap.groups` | bool | false | F040 |
| `place.new_windows` | enum{smart,cascade,centre,pointer} | smart | F047 |
| `place.remember` | bool | true | F048 |
| `place.new_window_output` | enum{pointer,focused,primary} | pointer | F157 |
| `tiling.default_mode` | enum (LayoutState.mode) | floating | F055 |
| `tiling.gap_inner`, `tiling.gap_outer` | u8 | 8, 8 | F062 |
| `tiling.smart_gaps` | bool | true | F062 |
| `tiling.new_window` | enum{master,after_focused,end} | after_focused | F065 |
| `workspaces.mode` | enum{fixed,dynamic} | fixed | F072 |
| `workspaces.count` | u8 (1–36) | 4 | F072 |
| `workspaces.rows` | u8 | 1 | F074 |
| `workspaces.per_output` | enum{span,separate,primary_only} | span | F077 |
| `workspaces.wrap` | bool | false | F079 |
| `workspaces.activate_switches` | bool | true | F081 |
| `overview.hide_secret` | bool | true | F098 |
| `overview.group_by_label` | bool | false | F099 |
| `switcher.scope_workspace` | enum{current,all} | current | F107 |
| `switcher.scope_output` | enum{current,all} | all | F107 |
| `switcher.include_minimised` | bool | true | F107 |
| `switcher.group_by_app` | bool | false | F106 |
| `switcher.layout` | enum{strip,grid,list} | strip | F110 |
| `edges.config` | json | top-left overview | `EdgeConfig` |
| `gestures.touchpad` | bool | true | F130 |
| `shortcuts.modifier` | enum{super,alt} | super | F138 |
| `shortcuts.super_digits` | enum{workspaces,taskbar} | workspaces | 5.5 |
| `shortcuts.bindings` | map<action_id, [chord]> | defaults in 5.5 | F133 |
| `pip.size` | enum{small,medium,large} | medium | F140 |
| `pip.dodge` | bool | false | F144 |
| `effects.enabled` | bool | true | F163 |
| `effects.blur` | bool | true | F164 |
| `rules` | json | [] | `Rule` list |

## 9. Accessibility and languages
- Labels use colour, pattern, letter and word together; the high-contrast theme thickens borders to 4 and 6 px and enlarges the tab one step.
- The screen reader announces on focus change: app name, window title, label kind word ("disposable"), then state ("maximised"). Moving between label kinds adds an earcon (setting).
- Move and resize modes announce position and size every 500 ms while changing; snap targets are announced by name ("left half").
- The overview and switcher are full a11y trees; tiles are named by title and app, never "thumbnail".
- Sticky switcher (F111), sticky modifier keys from AX settings, and dwell activation of hot corners support one-handed and switch use. Hot corners can be turned off entirely.
- Focus aids (F166) help low-vision users find the active window.
- Right-to-left locales mirror button order, snap left and right naming in menus (the keys keep their physical direction), and put the label tab at the top-right.
- Window titles are shown as given by the app, bidi-isolated; the kind word and menus follow the system language.

## 10. Performance budgets
| Item | Budget |
|---|---|
| Interactive move and resize | Every frame at the output's refresh rate; no policy-service round trip during a drag |
| Policy decision on window map | ≤ 5 ms typical; hard limit 50 ms then fallback (F170) |
| `Alt+Tab` press to switcher frame | ≤ 50 ms; quick tap switches in ≤ 1 frame after release |
| Overview open | First frame ≤ 50 ms; animation holds frame rate with 40 windows on the GPU path; cross-fade on the CPU path |
| Frame painting cost | ≤ 0.3 ms per window per frame on the CPU path; label tabs cached as textures and repainted only on change |
| Workspace switch | ≤ 200 ms animation, input accepted at once |
| Idle | Policy service: 0 wake-ups with no window events. No timers except pending animations |
| Memory | SH05 app host ≤ 60 MB; compositor additions ≤ 2 MB plus 16 kB per window for frame caches |
| Limits | 2,000 windows, 36 workspaces, 16 outputs, 1,000 rules (matched in ≤ 1 ms per window) |

## 11. Testing and proof
Test plan (targets; nothing has run):

- Unit (ForgeScript and the C++ mirror): every layout as a pure function (inputs: work area, windows with hints, parameters; outputs: rectangles) with properties: no overlap, inside the work area, hints honoured or window floated; snap zone maths; rule matching and ordering; placement; workspace add, remove and renumber; output-set keying.
- Golden renders: frames and tabs for each label kind, modifier and state at 100, 125, 150, 200 and 300% scale; overview, switcher variants, pickers, window menu; all themes; RTL.
- Protocol tests under the headless compositor: scripted clients that map, resize, set hints, request states, die, hang; output hot-plug; activation with and without tokens.
- Scripted UI through the a11y tree: tasks T1–T14.
- Spoofing tests: a client draws a look-alike tab inside its surface and requests no decoration (the real tab must still be present above it); a client sets another app's id (label must show the real one); a client tries to bind the policy protocol, to stay above a trusted prompt, to keep an inhibitor through `Super+Esc`.
- Fault injection: kill the policy service during a drag, during overview, during a workspace switch; delay its answers; feed it a broken rule file.
- Fuzz: policy protocol decoder in the compositor, theme record, rule and zone files, window titles (random Unicode), configure storms.
- Performance: frame-time traces for drag, overview and switch with 1, 40 and 500 windows on CPU and GPU paths.
- Differential: tiling results against a small reference implementation written independently for the same layouts.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Stacking invariant (compositor) | Trusted layer > lock > overlay > top > windows; no request sequence from any non-shell client changes its layer; PiP and "above" never exceed the top layer | T1 | PRISM model of the stacking state machine | Compositor stacking code must be written against the model |
| Label computation (compositor) | Label kind is a function of verified identity only; every mapped toplevel has a label and a border of at least the minimum width | T1 | Proof over the resolver and painter preconditions | Launch service interface (G-07) |
| Policy protocol decoder (compositor) | No runtime error on any message; all geometry clamped to output bounds | T2 | PRISM; fuzz | — |
| Fallback policy | Always places a window fully inside some output's work area | T2 | Proof over integer geometry | — |
| Layout engine (C++ mirror) | For each layout: rectangles are disjoint, cover at most the work area, and respect minimum sizes or report "float" | T1 for master-stack, columns, rows, grid; T2 for spiral, tree | PRISM proofs on the mirror; differential tests tie the ForgeScript version to it | ForgeScript itself has no prover (FORGE-WP-15) |
| Rule matcher | Deterministic; a broken rule never aborts matching | T3 | Unit and fuzz | Interpreted code |
| Title sanitiser | Output has no control or direction-override characters; bounded length | T2 | PRISM; fuzz | — |
| Shortcut table | Reserved chords cannot be bound or inhibited | T2 | Proof over the table update function | — |
| Shell surfaces (ForgeScript) | No runtime error in tests | T3/T4 | Scripted UI | Interpreted code |

## 12. Build plan
Phase 1 — usable core:

| Item | Work | Depends on |
|---|---|---|
| SH05-W01 | Policy protocol design and compositor module; fallback policy | SH05-GAP-1 |
| SH05-W02 | Identity resolver and label computation | SH05-GAP-4, G-07 |
| SH05-W03 | Frame painter: borders, patterns, label tab and bar, shadows, server-side title bar | SH05-GAP-3 |
| SH05-W04 | Header region reporting from the FUSE chrome kit; uniform header actions | SH05-GAP-6, FUSE-WP-29 |
| SH05-W05 | Policy service skeleton: placement, focus policy with activation tokens, minimise, maximise, fullscreen, above, sticky | SH05-W01, G-07 |
| SH05-W06 | Pointer and keyboard move and resize; edge snap with preview; snap keys | SH05-W01 |
| SH05-W07 | Workspaces through `ext_workspace_v1`: fixed and dynamic, switch, move window, show desktop | SH05-W05 |
| SH05-W08 | Global shortcut table with tap and hold; reserved keys | SH05-GAP-8, SH03 |
| SH05-W09 | Window menu; close and not-responding handling; force stop | SH05-W03, SY04 |
| SH05-W10 | Window switcher (strip) with thumbnails; minimal built-in switcher | SH05-GAP-2, FUSE-WP-10 |
| SH05-W11 | Overview: grid, workspace strip, keyboard, drag between workspaces | SH05-GAP-2, SH05-GAP-7 |
| SH05-W12 | `Windows1`, `Workspaces1`, `Labels1` interfaces for SH01, SH02, SH04 | SH05-W05 |
| SH05-W13 | Multi-monitor basics: output add and remove, move between outputs, display hand-off | SY01, FUSE-WP-11 |
| SH05-W14 | Effects set, reduced motion, frame budget fallbacks | `frame.pacing` module |
| SH05-W15 | Settings schema and SY01 pages (Windows, Workspaces); a11y announcements | SY01, G-04 |

Phase 2 — full feature:

| Item | Work | Depends on |
|---|---|---|
| SH05-W16 | Label modifiers, VM colours, label detail popover, enlarged labels | SH02 `Indicators1`, AD05, VM01 |
| SH05-W17 | Snap layout picker, snap assist, linked resize, window-to-window snap | SH05-W06 |
| SH05-W18 | Tiling layouts, keys, gaps, float exceptions, drop zones, persistence | SH05-W05, layout engine |
| SH05-W19 | Rules engine, "Remember for this app…", rules page, window picker | SH05-W05, SY01 |
| SH05-W20 | Remembered positions and session restore | SH03-W18, app session tokens |
| SH05-W21 | Overview search, all-workspaces grid, per-app mode, secret tiles | SH05-W11, SH01 |
| SH05-W22 | Switcher variants, keys while open, workspace and output switchers | SH05-W10 |
| SH05-W23 | Hot corners, edge actions, touchpad gestures | SH05-GAP-5 |
| SH05-W24 | Picture-in-picture | `svc.media` |
| SH05-W25 | Shortcut inhibit and pointer lock grants with marks; fullscreen hint | SH02-W26, SH04 |
| SH05-W26 | System strip, tether and request chips for SH04; capture outline | SH04-W23, SH02 |
| SH05-W27 | Output sets, mixed scale polish, confirm-display-change guarantee | SY01 |
| SH05-W28 | Workspace names, grid, per-output workspaces; audit records | SH05-W07 |

Phase 3 — advanced:

| Item | Work | Depends on |
|---|---|---|
| SH05-W29 | Tabbed windows, shade, snap groups, tabs in tiles | SH05-W03 |
| SH05-W30 | Scrolling columns layout, zone editor, per-rule layouts | SH05-W18 |
| SH05-W31 | Workspace security profiles and routes | SE03, AD05 |
| SH05-W32 | Cross-label drag and paste chips | SH05-GAP-9, AD05 |
| SH05-W33 | App named-action shortcuts | SH05-W08 |
| SH05-W34 | Touch-screen gestures, pinch, PiP of any window, dodge | SH05-W23 |
| SH05-W35 | HDR mark, tearing rules, spanning, presentation outputs | SY01, colour management |
| SH05-W36 | Policy rules, per-kind default rules, debug overlay | SH05-W19 |

## 13. Open questions and risks
1. **Policy in a separate process.** The plan puts policy in a ForgeScript service behind a new protocol (SH05-GAP-1) with a C++ fallback in the compositor. The alternative is policy as C++23 modules inside the compositor: simpler and faster, but a policy bug then crashes the compositor and the rule "app logic is ForgeScript" is broken. Needs an early prototype to measure map-time latency.
2. **Where the label tab goes.** A tab above the window costs 18 px of vertical space per floating window and must fold away when maximised. Users may miss the 3 px bar. The `Super`-hold overlay is the backstop; whether that is enough needs user testing.
3. **Client-side header bars and label trust.** Suite apps draw their own header inside the surface (DESIGN_SYSTEM.md section 3). A hostile app can draw a header that looks like another app's. Only the compositor-drawn tab is authoritative; this must be taught (AD01 tour) and may still confuse.
4. **Text in the compositor (SH05-GAP-3).** Painting app names in label tabs needs a text rasteriser in the compositor process. Linking FUSE's CPU rasteriser there enlarges the most trusted process; pre-rendering tab textures in a helper is the alternative.
5. **Identity for foreign and VM windows.** VM01 and VM03 proxy many guest windows through one host client. The label can name the VM or personality, not the guest app, unless the proxy is trusted to pass names (shown as untrusted text).
6. **`Super+digit` conflict** between workspaces (here) and task entries (SH01-F061). The setting `shortcuts.super_digits` chooses; the default needs one owner decision across SH01 and SH05.
7. **Key conflicts in tiling mode** (`Super+H` minimise versus focus-left). The plan remaps minimise to `Super+N` only while tiling is on; that is a mode and may confuse.
8. **Gestures** are not in the native compositor tree yet (SH05-GAP-5); 1:1 tracking needs them in the mechanism, not the policy.
9. **Aero theme hooks** in `pbsd/compositor/` (tint, blur, glass per its README) reflect the interim Plasma look. The suite's design system is flat with blur only on shell surfaces; which parts of the Aero path survive is an owner decision.
10. **CPU rendering path.** Overview with 40 live thumbnails on a software renderer will not hold frame rate; the fallbacks (static thumbnails, cross-fade) must be designed in from the start. GPU status on HardenedBSD is Unverified (G-18).
11. Risk: T1 targets on compositor stacking and label computation depend on compositor code that is still a scaffold (`pbsd/compositor/README.md` calls the native compositor a façade at this stage).
