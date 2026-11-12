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
| SH05-F058 | Layout cycle | `Super+Space`... is the keyboard layout; layouts cycle with `Super+Shift+Y`; a layout picker popover on `Super+Ctrl+Y` | S | 2 |
| SH05-F059 | Focus by direction | `Super+H/J/K/L` or `Super+Alt+arrows` in tiling mode move focus left, down, up, right, across outputs | S | 2 |
| SH05-F060 | Swap and move | `Super+Shift+H/J/K/L` swaps with the neighbour; dragging a tile onto another swaps; dropping on an edge splits | S | 2 |
| SH05-F061 | Master controls | Grow or shrink the master ratio (`Super+[`, `Super+]`), change master count (`Super+,`, `Super+.`), promote to master (`Super+Enter` is Terminal by default, so `Super+M`) | S | 2 |
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
