<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Magnifier

By Odin Loch

| | |
|---|---|
| ID | `AX02` |
| Bundle | `org.pbsd.magnifier` |
| Category | access |
| Priority | P1 |
| Backing repos | FUSE (`Source/FUSE/UI/A11y`), this repo (`pbsd/compositor`) |
| Status | Planned. Nothing built. |

## 1. Purpose

The Magnifier enlarges and recolours the screen for low-vision users, and makes the pointer, the focus and the caret easy to find for everyone. Zoom and colour filters are done by the compositor, so the Magnifier never sees a pixel of another app. It replaces the KWin zoom, magnifier, invert and colour-blindness effects of the Plasma plan (PLATFORM V20).

## 2. Scope

In scope: full-screen, lens and docked zoom; smooth scaling; pointer, focus and caret tracking; colour filters; pointer enlargement and highlight; focus and caret highlight; crosshair; reading ruler and line focus; text smoothing choices under zoom; per-monitor settings; gestures and shortcuts; a text lens that re-renders text large.

Out of scope:

| Item | Owner |
|---|---|
| Speech and braille | AX01 |
| System theme, high-contrast theme, text size, interface scale | SY01 (FUSE-WP-21) |
| Display colour profiles, night light, HDR | SY01, compositor |
| Screenshots and recordings | ME05 |
| The compositor's transform and colour pipeline itself | compositor (AX02-GAP-1, AX02-GAP-2) |
| Cursor theme files | SY01 |
| On/off switch on the Accessibility page | SY01 |

## 3. Users and top tasks

Personas: **Asha**, low vision, works at 4× full-screen all day with inverted colours. **Ken**, age-related sight loss, uses a lens for small print now and then. **Lena**, colour-blind (deuteranomaly), wants a correction filter only. **Raj**, a presenter who uses zoom, pointer highlight and spotlight on a projector. **Mira**, a screen reader user who wants the focus rectangle visible for a sighted colleague.

| # | Task | Target steps |
|---|---|---|
| 1 | Zoom in from anywhere, including a capability prompt and the lock screen | 1 (`Super+=`) |
| 2 | Zoom out to 1× and back to the last level | 1 (`Super+Esc`) |
| 3 | Zoom with the pointer | 1 (`Super+Scroll`) |
| 4 | Switch full-screen, lens, docked | 1 (`Super+Alt+F/L/D`) |
| 5 | Invert colours | 1 (`Super+Alt+I`) |
| 6 | Find the pointer | 1 (`Super+Alt+P`, or shake) |
| 7 | Follow typing without moving the pointer | 0 (caret tracking) |
| 8 | See the whole screen for a moment while zoomed | 1 held (`Super+Alt+O`) |
| 9 | Turn on a colour-blindness correction | 3 (quick panel, filter, pick) |
| 10 | Read a line with the ruler | 1 (`Super+Alt+R`) |
| 11 | Set different zoom on a second monitor | 3 |
| 12 | Read a small label as large reflowed text | 1 held (`Super+Alt+T`) |
| 13 | Recover when lost or when the view is wrong | 1 (`Super+Alt+Backspace` resets everything) |

## 4. Features

### 4.1 Zoom modes

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX02-F001 | Full-screen zoom | The whole output is scaled about a tracked point by the compositor. | M | 1 |
| AX02-F002 | Lens | A movable rectangle or circle shows the area under the pointer magnified; the rest of the screen stays 1×. Size 100–1200 px each way. | M | 1 |
| AX02-F003 | Docked | A strip on one edge (top, bottom, left, right) shows the magnified area; windows are laid out in the remaining area (exclusive zone). Size 10–70% of the output. | M | 2 |
| AX02-F004 | Split | Docked without an exclusive zone: half the output magnified, half 1×, no window relayout. | C | 3 |
| AX02-F005 | Mode cycle | One key cycles the enabled modes; each mode keeps its own zoom level. | M | 1 |
| AX02-F006 | Lens shape | Rectangle, rounded rectangle, circle; border 0–8 px in a chosen colour. | S | 2 |
| AX02-F007 | Lens offset | Lens centred on the pointer, or offset so the pointer target is not covered. | S | 2 |
| AX02-F008 | Fixed lens | Pin the lens to a screen position; it shows the area around the pointer. | C | 2 |
| AX02-F009 | Resize lens | `Super+Alt+Shift+Arrows`, or drag its border while holding `Super`. | S | 2 |
| AX02-F010 | Zoom on one output | Zoom applies to the output with the pointer, or to a chosen output, or to all. | M | 1 |
| AX02-F011 | Second-monitor view | Show the magnified view of output A full-screen on output B. | C | 3 |

### 4.2 Zoom control

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX02-F012 | Range | 1.0× to 32×. Lens and docked: 1.5× to 16×. | M | 1 |
| AX02-F013 | Step | Key step: 10%, 25%, 50%, 100% (setting). `Super+=` and `Super+-`. | M | 1 |
| AX02-F014 | Smooth scaling | Zoom changes animate over 120 ms; continuous while a key is held. Off under reduced motion. | M | 1 |
| AX02-F015 | Pointer zoom | `Super+Scroll`; zoom is centred on the pointer; touchpad scroll gives fine steps. | M | 1 |
| AX02-F016 | Pinch zoom | Three-finger pinch on a touchpad; two-finger pinch with `Super` held on a touch screen. | S | 2 |
| AX02-F017 | Toggle | `Super+Esc` goes to 1× and back to the last level. | M | 1 |
| AX02-F018 | Presets | Up to five named levels (for example 2×, 4×, 8×); `Super+Alt+1…5`. | S | 2 |
| AX02-F019 | Peek overview | Hold `Super+Alt+O`: zoom out to 1× with a frame showing the zoomed area; release returns. Moving the pointer while held picks a new area. | M | 1 |
| AX02-F020 | Temporary zoom | Hold `Super+Alt+Z`: zoom to a preset while held. | S | 2 |
| AX02-F021 | Zoom indicator | A small on-screen display shows the level for 1 s after a change (through SH02's display service). | M | 1 |
| AX02-F022 | Start-up level | Start at last level, at a fixed level, or at 1×. | M | 1 |
| AX02-F023 | Minimum-change rule | The integer levels 2×, 3×, 4× snap exactly so pixels map cleanly. | S | 2 |

### 4.3 Tracking

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX02-F024 | Pointer tracking: continuous | The view moves in proportion to the pointer; the pointer can reach every edge. | M | 1 |
| AX02-F025 | Pointer tracking: centred | The pointer stays at the centre of the view; the view stops at output edges. | M | 1 |
| AX02-F026 | Pointer tracking: edge push | The view moves only when the pointer nears its edge; margin 5–40% of the view. | M | 1 |
| AX02-F027 | Focus tracking | The view moves to show the keyboard focus rectangle from `svc.a11y`. | M | 1 |
| AX02-F028 | Caret tracking | The view follows the text caret rectangle; a leading margin keeps upcoming text visible in the writing direction. | M | 1 |
| AX02-F029 | Tracking priority | Order among pointer, focus and caret; default: the one that moved last wins, caret beats focus for 1.5 s after a key press. | M | 1 |
| AX02-F030 | Screen reader cursor tracking | Follow the AX01 review or virtual cursor rectangle. | S | 2 |
| AX02-F031 | Menu and popup tracking | When a menu, tooltip or dialog opens, bring it into view. | S | 2 |
| AX02-F032 | Notification peek | On a notification, slide to it for its display time or show it inside the current view (setting). | S | 2 |
| AX02-F033 | Prompt tracking | When SH04 shows a capability or authentication prompt, the view moves to it at once and a border in `--warn` marks it. Cannot be disabled. | M | 1 |
| AX02-F034 | Smooth panning | View moves ease over 100 ms; jumps over half a view are faded, not slid, to reduce motion. | M | 1 |
| AX02-F035 | Tracking pause | Hold `Super+Alt` to freeze the view while moving the pointer. | S | 2 |
| AX02-F036 | Keyboard panning | `Super+Alt+Arrows` pan by 10% of the view; with `Shift` by one view; `Super+Alt+Home` to the pointer. | M | 1 |
| AX02-F037 | Edge and corner jumps | `Super+Alt+Ctrl+Arrows` jump to output edges; twice for the corner. | C | 2 |
| AX02-F038 | Axis lock | Lock tracking to horizontal or vertical for reading across a line. | C | 3 |
| AX02-F039 | Saved views | Up to nine saved positions per output and workspace; jump by key. | C | 3 |
| AX02-F040 | Follow window focus | When a window gets focus, show its title bar corner or its focused control (setting). | S | 2 |
| AX02-F041 | Tracking in the lens | Lens may follow the caret instead of the pointer. | S | 2 |
| AX02-F042 | Voice and switch tracking | Follow the element AX06 or AX03 scanning has highlighted. | C | 3 |

### 4.4 Rendering quality and text smoothing

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX02-F043 | Scaling filter | Nearest (sharp pixels), bilinear, bicubic, or sharp-bilinear (integer part nearest, rest bilinear). Per mode. | M | 1 |
| AX02-F044 | Re-render at scale | Up to a set limit (default 3×) the compositor asks clients for a higher buffer scale through the fractional-scale path, so text and vectors are drawn sharp instead of stretched. | S | 2 |
| AX02-F045 | Re-render budget | Re-render only the windows that intersect the view; fall back to bitmap scaling when a client is slow (over 2 frames). | S | 2 |
| AX02-F046 | Text smoothing choice | While zoomed: keep the app's smoothing, force greyscale antialiasing (no colour fringes when enlarged), or no antialiasing. Needs a client hint (AX02-GAP-3). | S | 2 |
| AX02-F047 | Edge sharpening | Optional unsharp pass in the zoom shader; strength 0–100. | C | 3 |
| AX02-F048 | Frame pacing | The zoomed view updates at the output refresh rate; damage outside the view is not processed. | M | 1 |
| AX02-F049 | HDR and wide gamut | Zoom and filters run in the compositor's blending space so HDR content is not clipped. | C | 3 |
| AX02-F050 | CPU fallback | With no GPU, zoom works with nearest and bilinear filters at reduced refresh. | M | 2 |

### 4.5 Colour filters

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX02-F051 | Invert | Full inversion. | M | 1 |
| AX02-F052 | Smart invert | Invert lightness and keep hue, so photos keep their colours roughly; done as a matrix in a luma-chroma space. | S | 2 |
| AX02-F053 | Greyscale | Luma-weighted. | M | 1 |
| AX02-F054 | Inverted greyscale | Both. | M | 1 |
| AX02-F055 | Colour-blindness correction | Daltonisation for protan, deutan and tritan types with a strength slider 0–100. | M | 2 |
| AX02-F056 | Colour-blindness simulation | Shows what a colour-blind viewer sees; for designers. Marked "simulation" in the indicator. | C | 3 |
| AX02-F057 | Contrast | −50 to +100. | M | 1 |
| AX02-F058 | Brightness and gamma | Brightness −50 to +50; gamma 0.5–2.5. | S | 2 |
| AX02-F059 | Saturation | 0–200%. | S | 2 |
| AX02-F060 | Tint | A colour overlay (for example amber on black) with strength; presets for common low-vision pairs: yellow on black, white on blue, green on black. | S | 2 |
| AX02-F061 | Two-colour mode | Threshold the screen to two chosen colours. | C | 3 |
| AX02-F062 | Filter scope | Whole output, magnified view only, or lens only. | M | 2 |
| AX02-F063 | Per-app exclusion | Named apps (photo and video apps) are shown unfiltered; needs a per-surface filter flag in the compositor. | C | 3 |
| AX02-F064 | Filter stack | Filters combine into one 4×5 matrix where possible; non-linear ones (two-colour, smart invert) use a shader stage. | M | 1 |
| AX02-F065 | Quick toggle | `Super+Alt+I` invert, `Super+Alt+G` greyscale, `Super+Alt+C` cycles the user's filter list. | M | 1 |
| AX02-F066 | Transition | Filter changes fade over 140 ms; instant under reduced motion. | S | 1 |
| AX02-F067 | Screenshots unfiltered | Captures by ME05 show true colours and no zoom unless the user ticks "as seen". | M | 2 |
| AX02-F068 | Test chart | A page with colour swatches, the system status colours with their icons and words, and contrast ratios under the current filter. | S | 2 |
| AX02-F069 | Status colour check | Warn in settings if a filter makes `--ok`, `--warn` and `--danger` closer than 3:1 to each other; status is never colour alone (DESIGN_SYSTEM section 2), so this is advice. | S | 2 |

### 4.6 Pointer

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX02-F070 | Pointer size | 1× to 8× of the theme size; vector cursors so large sizes stay sharp. | M | 1 |
| AX02-F071 | Pointer colour | Theme, black with white edge, white with black edge, custom fill and edge, or inverting. | S | 2 |
| AX02-F072 | Pointer size under zoom | Scale with the zoom, keep constant, or cap. | M | 1 |
| AX02-F073 | Locate pointer | `Super+Alt+P`: rings contract onto the pointer for 800 ms. | M | 1 |
| AX02-F074 | Shake to locate | Shaking the pointer enlarges it briefly. | S | 2 |
| AX02-F075 | Pointer halo | A constant ring or disc around the pointer; colour, size, opacity. | S | 2 |
| AX02-F076 | Click feedback | A ripple on press; different colour for left, middle, right. | S | 2 |
| AX02-F077 | Pointer trail | Short trail while moving; off under reduced motion. | C | 3 |
| AX02-F078 | Spotlight | Dim everything outside a circle around the pointer; radius and dim level. | C | 3 |
| AX02-F079 | Text cursor shape under pointer | Thicker I-beam. | C | 2 |

### 4.7 Focus and caret highlight

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX02-F080 | Focus highlight | A rectangle around the keyboard focus, drawn as an overlay; thickness 2–12 px; colour; solid or dashed; optional glow. | M | 1 |
| AX02-F081 | Focus flash | The rectangle animates from large to exact on each focus move. | S | 2 |
| AX02-F082 | Caret highlight | A thick bar, a triangle marker above and below, or a circle at the caret. | M | 2 |
| AX02-F083 | Caret locate | `Super+Alt+K` flashes the caret position. | S | 2 |
| AX02-F084 | Review cursor highlight | Draws the AX01 review cursor in a second colour. | S | 2 |
| AX02-F085 | Contrast-safe colour | Highlight uses a double line (light and dark) so it shows on any background. | M | 1 |
| AX02-F086 | Hide while typing | Pointer halo hides while keys are typed. | C | 2 |

### 4.8 Crosshair and reading aids

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX02-F087 | Crosshair | Lines through the pointer across the output; thickness, colour, opacity, gap at the centre, fixed length or full. | M | 2 |
| AX02-F088 | Reading ruler | A horizontal band follows the pointer or the caret; band height 1–5 lines; underline, band tint or both. | M | 2 |
| AX02-F089 | Line focus | Everything outside the band is dimmed 20–90%. | M | 2 |
| AX02-F090 | Vertical ruler | The same for vertical writing and for columns of numbers. | C | 3 |
| AX02-F091 | Ruler step | `Super+Alt+Up/Down` (while the ruler is on and zoom is 1×) moves the band by one line height. | S | 2 |
| AX02-F092 | Screen tint | A full-screen colour overlay at low opacity (reading overlays for visual stress). | S | 2 |
| AX02-F093 | Screen mask | Dim all windows except the focused one. | C | 3 |
| AX02-F094 | Text lens | Hold `Super+Alt+T`: the text of the element under the pointer (from the a11y tree) is shown in a popup in a large font, with chosen colours, reflowed to fit. No pixels involved. | S | 2 |
| AX02-F095 | Text lens speak | A button in the text lens sends the text to AX01's `speech.say`. | C | 3 |
| AX02-F096 | Text lens for secret fields | Shows nothing: the tree carries no value for secret fields. | M | 2 |

### 4.9 Monitors, profiles, apps

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX02-F097 | Per-monitor settings | Each output (keyed by EDID hash) has its own mode, level, filter and pointer size. | M | 2 |
| AX02-F098 | Hot-plug | When an output goes away its view state is kept and restored on return; the view never ends up outside the outputs. | M | 2 |
| AX02-F099 | Pointer crossing outputs | Zoom on the old output stays or resets to 1× (setting); the new output gets its own state. | M | 2 |
| AX02-F100 | Profiles | Named sets of all settings; switch by key or from the quick panel. | S | 2 |
| AX02-F101 | Per-app profile | Apply a profile while a given app has focus. | C | 3 |
| AX02-F102 | Presentation profile | Built-in: pointer highlight, click ripple, spotlight key, no tracking jumps. | C | 3 |
| AX02-F103 | Mirrored outputs | One view state for a mirror group. | S | 2 |

### 4.10 Gestures, shortcuts, control surfaces

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX02-F104 | Global shortcuts | Registered with the shell; all rebindable; work on secure surfaces and before login. | M | 1 |
| AX02-F105 | Modifier choice | The zoom modifier for scroll and pinch: `Super`, `Ctrl+Alt`, or `Super+Ctrl`. | M | 1 |
| AX02-F106 | Touch gestures | Three-finger double-tap toggles zoom; three-finger drag pans; three-finger double-tap-and-hold is temporary zoom. | S | 3 |
| AX02-F107 | Touchpad gestures | Three-finger pinch zoom; `Super` + two-finger scroll pans. | S | 2 |
| AX02-F108 | Control bar | A small floating bar: − level + , mode, filter, close. Shown at 1× scale inside the zoomed view; auto-hides. | M | 1 |
| AX02-F109 | Quick panel tile | A tile in SH02's quick settings: on/off, level, filter list. | M | 1 |
| AX02-F110 | Command line | `magnifier --zoom 4 --mode lens --filter invert` through single-instance hand-off; for scripts and for AX06 voice commands. | S | 2 |
| AX02-F111 | Actions interface | `magnifier.set`, `magnifier.get`, `magnifier.toggle` on the session bus for AX01, AX06 and SH01's command runner. | S | 2 |

### 4.11 Security, pre-login, recovery, settings

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX02-F112 | No pixel access | Zoom, lens, docked view and filters are composed by the compositor from parameters. The app holds no `dev.screen`. | M | 1 |
| AX02-F113 | Secure surfaces | Zoom and filters apply to prompts, the lock screen and the greeter. Overlays from this app are drawn below secure surfaces, never on top. | M | 1 |
| AX02-F114 | Greeter and lock screen | A restricted instance with zoom, invert, pointer size and the default shortcuts; settings come from the system namespace. | M | 1 |
| AX02-F115 | Installer | Available on install media (AD01) with the same restricted set. | M | 1 |
| AX02-F116 | Active indicator | An icon in SH02's panel while zoom above 1× or any filter is on; click opens the quick panel. | M | 1 |
| AX02-F117 | Reset key | `Super+Alt+Backspace`: zoom 1×, filters off, overlays off, on every output. Handled by the compositor itself so it works when this app is dead. | M | 1 |
| AX02-F118 | Crash behaviour | If the app dies the compositor keeps the current transform and filter; the session restarts the app within 2 s and it re-reads the state. | M | 1 |
| AX02-F119 | Parameter limits | The compositor clamps all parameters: the view always lies within the output, the colour matrix is bounded so the screen cannot be made blank (minimum contrast between black and white input of 3:1), overlay opacity at most 90%. | M | 1 |
| AX02-F120 | Remote sessions | In VM04 server sessions, zoom is applied on the viewer side by default. | C | 3 |
| AX02-F121 | Settings window | All settings with live preview (section 5.2). | M | 1 |
| AX02-F122 | Import and export | Profiles as JSON through the file portal. | C | 2 |
| AX02-F123 | Start at login | `bg.autostart`; also toggled in SY01. | M | 1 |
| AX02-F124 | Reduced motion | Honours the system flag: no slide, no trail, fades only. | M | 1 |
| AX02-F125 | Audit | Enabling the tool and each change of `svc.a11y` rights is logged; zoom changes are not (noise, and they reveal nothing). | M | 1 |
| AX02-F126 | First-run guide | A four-step sheet: pick mode, pick level, learn three keys, learn the reset key. | S | 2 |

## 5. GUI

### 5.1 Window map

```
Magnifier (background process, indicator in SH02)
├─ Settings window (main window)
│   ├─ Zoom · Tracking · Colour filters · Pointer · Focus and caret
│   ├─ Reading aids · Monitors · Shortcuts and gestures · Profiles · Before login
│   └─ dialogs: New profile · Custom filter · Import/Export · Reset
├─ Control bar (floating, layer surface)
├─ Quick panel tile (inside SH02 quick settings)
├─ Text lens (popup)
├─ First-run guide (sheet)
├─ Compositor-drawn, parameterised by this app (no surface of ours):
│   ├─ Full-screen zoom view · Lens · Docked view · Peek-overview frame
│   └─ Colour filter stage · Enlarged pointer
└─ Overlays through svc.a11y:overlay (input-transparent):
    focus rectangle · caret marker · crosshair · reading ruler · line-focus dim · halo · ripple · spotlight
```

### 5.2 Main window (Settings)

Shown at 920×620. Sidebar + content.

```
+------------------------------------------------------------------------------------------+
| [=] Magnifier — Zoom                        [search Ctrl+K]              [▁] [▢] [✕]     | (1)
+--------------------+---------------------------------------------------------------------+
| > Zoom             | Magnifier  [ON ]      Profile [Default        v]                    | (3)
|   Tracking         +---------------------------------------------------------------------+
|   Colour filters   | Mode   (•) Full screen   ( ) Lens   ( ) Docked   ( ) Split          | (4)
|   Pointer          | Level   1× [=======|------------------------] 32×      4.0×  [-][+] | (5)
|   Focus and caret  | Step   [25 %  v]      Presets  [2×] [4×] [8×] [+]                   |
|   Reading aids     | Scaling [Sharp-bilinear v]   [x] Re-draw apps sharp up to [3× v]    | (6)
|   Monitors         | Text while zoomed  (•) as the app draws it ( ) greyscale ( ) none   |
|   Shortcuts        | [x] Smooth zoom     [x] Snap to whole levels                        |
|   Profiles         +---------------------------------------------------------------------+
|   Before login     | Preview                                                              | (7)
| (2)                | +-------------------------------+   Lens size  W [ 480 ] H [ 320 ]  |
|                    | |  The quick brown fox…   ▒▒▒▒  |   Shape      [Rounded       v]    |
|                    | |  ABCabc 0123  ✓ ! ✕           |   Border     [ 2 px ] [■ accent]  |
|                    | +-------------------------------+   Docked at  [Top v]  [ 30 % ]    |
+--------------------+---------------------------------------------------------------------+
| Zoom 4.0× full screen · filter: none · output DP-1      Reset all: Super+Alt+Backspace   | (8)
+------------------------------------------------------------------------------------------+
```

1. Header bar. No capability badges are possible: the app has no network and no device capability.
2. Sidebar, 220 px, `listView`.
3. Master `toggleSwitch` and profile `comboBox`.
4. Mode `radioButton` group. Changing mode applies at once; an Undo toast follows.
5. Level `slider` with `label` and two `toolButton`s. The window itself is exempt from tracking while it has focus, so dragging the slider does not move the view under the pointer.
6. Rendering options: `comboBox`, `checkBox`, `radioButton`.
7. Preview `panel`: a sample drawn by this app with the current filter and scaling maths applied in the app's own draw list. It is a simulation, not a capture. Mode-specific fields at the right enable by mode.
8. Status bar with the reset key always visible.

States: empty (first run: the guide sheet covers the content); loading (none; settings load under 50 ms); error (compositor lacks the view-transform interface: banner "Zoom is not available on this compositor. Pointer and overlays still work."); locked (window hidden); read-only (greeter instance has no window; system-namespace keys show a lock icon and need the admin prompt); narrow (under 640 px the sidebar becomes a `comboBox`, the preview drops below the options).

### 5.3 Other views and dialogs

**Control bar** (floating, 300×44, drawn at 1× inside the view).

```
+----------------------------------------------------+
| [-]  4.0×  [+] | [Full v] | [Filter v] | [⚙] [✕]   |
+----------------------------------------------------+
   (1)              (2)         (3)         (4)
```

1. `toolButton`s and `label`; hold to repeat. 2. Mode `comboBox`. 3. Filter `comboBox` with the user's list. 4. Settings and "zoom off". Auto-hides after 3 s without the pointer; shows on any zoom key. States: hidden, shown, pinned.

**Lens** (compositor-drawn; shown on a 1920×1080 output).

```
+------------------------------------------------------------------------------------------+
|  Files — Documents                                                                       |
|  name          size     modified                                                         |
|  report.odt    48 kB    12 Nov      +===========================================+        |
|  notes.md      2 kB     11 Nov      ‖                                           ‖  (1)   |
|  budget.ods    91 kB     3 Nov      ‖   n o t e s . m d        2  k B           ‖        |
|                                     ‖                    ➤                      ‖  (2)   |
|                                     ‖   b u d g e t . o d s    9 1  k B         ‖        |
|                                     +===========================================+        |
|                                                                               (3)        |
+------------------------------------------------------------------------------------------+
```

1. Lens border, colour and width from settings. 2. The pointer is drawn inside the lens at the chosen pointer size. 3. The area under the lens is hidden by it; "offset lens" (F007) places the lens beside the pointer instead. Input goes to the real window under the pointer: the lens is not an input surface.

**Docked** (compositor-drawn, top dock at 30%).

```
+------------------------------------------------------------------------------------------+
|   r e p o r t . o d t      4 8  k B      1 2  N o v                                (1)   |
|   n o t e s . m d          2  k B        1 1  N o v                                      |
+==========================================================================================+ (2)
|  Files — Documents                                                                 (3)   |
|  name          size     modified                                                         |
|  report.odt    48 kB    12 Nov    [┌────────────┐]  (4)                                  |
|  notes.md      2 kB     11 Nov                                                           |
+------------------------------------------------------------------------------------------+
```

1. Docked view, an exclusive zone; windows never go under it. 2. Divider, draggable with `Super` held. 3. Normal desktop at 1×. 4. Optional frame showing which area the dock shows.

**Colour filters page** (content area).

```
| Filter list (order = Super+Alt+C cycle)            Scope (•) whole screen ( ) zoomed view |
|  [x] Invert                        Super+Alt+I                                            |
|  [x] Greyscale                     Super+Alt+G                                            |
|  [ ] Inverted greyscale                                                                   |
|  [x] Deuteranopia correction       strength 0 [======|----] 100                      (1) |
|  [ ] Yellow on black                                                                      |
|  [+ Custom…]                                                                              |
| Contrast   -50 [=====|---------] +100     Brightness -50 [====|----] +50            (2)  |
| Saturation   0 [=====|---------] 200 %    Gamma      0.5 [===|-----] 2.5                 |
| Test chart                                                                           (3) |
|  ■ ok ✓ Verified   ■ warn ! Unverified   ■ danger ✕ Blocked   ■ net   ■ secret           |
|  ok/warn 3.4:1   warn/danger 2.1:1 ⚠ close under this filter — icons and words still differ|
```

1. `listView` with `checkBox` rows, drag to reorder, inline `slider` for strength. 2. `slider` rows. 3. Test chart `panel` drawn by the app with the filter maths applied; the warning row is F069.

**Reading aids page**.

```
| Reading ruler [ON ]   Follows (•) pointer ( ) caret      Height [ 2 lines v]              |
|   Style [x] underline [x] band tint [■]   [x] Line focus: dim the rest by [ 60 % ]       |
| Crosshair     [OFF]   Thickness [ 2 px ]  Colour [■]  Opacity [ 70 % ]  Gap [ 24 px ]    |
| Screen tint   [OFF]   Colour [■ #FFE9A8]  Strength [ 20 % ]                              |
| Text lens     Hold Super+Alt+T   Font size [ 36 pt ]  Colours [Yellow on black v]        |
| +---------------------------------- preview -----------------------------------------+   |
| |░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░|   |
| |  The reading ruler keeps your place on the line you are reading.                   |   |
| |░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░|   |
| +------------------------------------------------------------------------------------+   |
```

`toggleSwitch`, `radioButton`, `comboBox`, `spinBox`, `colorPicker` (popup), preview `panel`.

**Monitors page**.

```
| +-----------+ +----------------+                                                          |
| |  eDP-1    | |     DP-1       |   Selected: DP-1 (27", 2560×1440)                    (1) |
| |  2.0× FS  | |  4.0× lens     |   [x] Use own settings for this monitor                  |
| +-----------+ +----------------+   Mode [Lens v]  Level [4.0×]  Filter [Invert v]     (2) |
|  Zoom applies to (•) monitor with the pointer ( ) all monitors ( ) only [DP-1 v]          |
|  When the pointer leaves a monitor (•) keep its zoom ( ) return it to 1×                  |
```

1. Layout picture, `panel` with one `button` per output, laid out as in SY01 Displays (read-only here). 2. Per-output overrides.

**Text lens** (popup near the pointer, up to 60% of the output).

```
+--------------------------------------------------------------+
|  Signed by the system · verified                             |
|  Text Editor wants to open                                   |
|  files you choose.                                 [🔊] [✕]  |
+--------------------------------------------------------------+
```

A `panel` with a `label`; text and role come from `svc.a11y:text.read`; trust words are included because they are in the node's description. Closes on key release. States: no text ("No text here"), secret field ("Hidden field").

### 5.4 Menus

```
[=] menu of the Settings window (F10)
├─ File ▸ Import profiles… · Export profiles… · Close Ctrl+W · Quit Magnifier Ctrl+Q
├─ Edit ▸ Undo change Ctrl+Z · Redo Ctrl+Shift+Z · Reset this page · Reset all…
├─ View ▸ Control bar · First-run guide · Test chart
├─ Magnifier ▸ On/off Super+Esc · Zoom in Super+= · Zoom out Super+- 
│            · Mode ▸ Full screen Super+Alt+F · Lens Super+Alt+L · Docked Super+Alt+D
│            · Filter ▸ Invert Super+Alt+I · Greyscale Super+Alt+G · Next Super+Alt+C · None
│            · Aids ▸ Ruler Super+Alt+R · Crosshair Super+Alt+X · Locate pointer Super+Alt+P
└─ Help ▸ Magnifier help F1 · Shortcuts · About

Indicator menu (SH02 panel icon)
├─ Magnifier on/off
├─ Level ▸ 1× · 2× · 4× · 8× · presets
├─ Mode ▸ … · Filter ▸ … · Profile ▸ …
├─ Reset everything                 Super+Alt+Backspace
└─ Settings…
```

### 5.5 Keyboard and pointer

| Keys | Action | Keys | Action |
|---|---|---|---|
| `Super+=` / `Super+-` | zoom in, out | `Super+Esc` | zoom off, last level |
| `Super+Scroll` | zoom at pointer | `Super+Alt+F / L / D` | mode |
| `Super+Alt+Arrows` | pan | `Super+Alt+Shift+Arrows` | pan one view; resize lens in lens mode |
| `Super+Alt+Home` | view to pointer | `Super+Alt+O` (hold) | peek overview |
| `Super+Alt+Z` (hold) | temporary zoom | `Super+Alt+1…5` | presets |
| `Super+Alt+I / G / C` | invert, greyscale, next filter | `Super+Alt+P` | locate pointer |
| `Super+Alt+K` | locate caret | `Super+Alt+R / X` | ruler, crosshair |
| `Super+Alt+T` (hold) | text lens | `Super+Alt+Backspace` | reset everything |

These chords are registered with the shell as global actions of an accessibility tool; SH05 must not reuse them. The app receives action events, not key events: it never sees other keys.

Pointer: `Super`+drag on the lens border resizes; `Super`+drag on the dock divider resizes; middle-click on the control bar level resets to 1×. Context menu on the control bar: Pin, Hide, Settings. No drag and drop except reordering the filter list. Gestures are in 4.10.

### 5.6 States

| State | Behaviour |
|---|---|
| Off | Zoom 1×, no filter, no overlays; process may still run for the indicator. |
| Zoomed | Indicator on; control bar available. |
| Filter only | Indicator shows the filter icon. |
| App not running | Compositor keeps the last parameters; reset key still works (F117). |
| `svc.a11y` unavailable | Pointer tracking still works (pointer position comes from the compositor interface); focus and caret tracking stop; one notification. |
| No GPU | CPU path, two filters, lower refresh (F050); the settings page says so. |
| Output removed | State parked (F098). |
| Locked, greeter, installer | Restricted instance (F114, F115). |
| Secure prompt open | View jumps to the prompt (F033); overlays stay below it. |
| Offline | No effect. |

### 5.7 Widgets

| Region | Widget |
|---|---|
| Sidebar, filter list | `listView` |
| Options | `toggleSwitch`, `radioButton`, `checkBox`, `comboBox`, `spinBox`, `slider`, `colorPicker` |
| Preview, test chart, monitor picture | `panel` with custom paint through the draw list |
| Shortcuts page | `tableView` + key-capture field (AX01-GAP-3, FUSE-WP-14) |
| Control bar | `toolButton`, `label`, `comboBox` on a layer-shell surface (FUSE-WP-10) |
| Text lens | `panel`, `label` on a popup surface |
| Overlays | plain surfaces with custom paint; no widgets |

Gaps:

| ID | Gap | Evidence | Lands in |
|---|---|---|---|
| AX02-GAP-1 | Compositor view-transform interface: per-output scale and offset, lens and dock rectangles, pointer scale, clamps, reset key | No zoom or magnifier module among `PBSD:pbsd/compositor/pbsd.compositor.wayland.*.cppm` (file names read) | compositor |
| AX02-GAP-2 | Compositor colour-filter stage: per-output 4×5 matrix and a small set of fixed shader filters | `color_management*.cppm` modules exist by name; behaviour Unverified | compositor |
| AX02-GAP-3 | Client hint "zoomed: use greyscale antialiasing" and re-render at a preferred scale | `fractional_scale.cppm` exists by name; FUSE text is MINIMAL by default (PLATFORM V6) | FUSE-WP-06, FUSE-WP-16 |
| AX02-GAP-4 | Caret rectangle and focus rectangle events without text content (`svc.a11y:track`) | Verified snapshot has bounds and focus only (`FUSE:Source/FUSE/UI/Core/include/fuse/ui/ui.hpp:86-101`); caret rectangle exists only as the IME area (`FUSE:Source/FUSE/Core/include/fuse/platform/text_services.hpp`) | FUSE-WP-55, FUSE-WP-56 |
| AX02-GAP-5 | Vector cursor themes for sizes up to 8× | `cursor_shape.cppm` exists by name | compositor, SY01 |

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Magnifier core | ForgeScript | app host (`bg.run`) | State per output, tracking logic, filter matrix maths, profiles, action handlers |
| Magnifier UI | FUSE UI | app host | Settings window, control bar, text lens, overlays |
| View-transform client | ForgeScript binding over a schema interface | app host | Sends parameters to the compositor; reads output list and pointer position |
| Zoom and filter stage | C++23 (compositor) | compositor | Scales, composes lens and dock, applies colour matrix, draws the pointer; enforces clamps |
| `svc.a11y` | C++23 (platform) | system service | Focus, caret and review rectangles; text for the text lens |

No native helper ships with this app. The per-frame work is in the compositor; the app only sends a new view rectangle when a tracked point moves.

```
 pointer motion ─┐                      ┌────────── compositor ──────────┐
 focus / caret ──┼─▶ Magnifier core ──▶ │ view transform (scale, offset) │──▶ scanout
 (svc.a11y:track)│   (ForgeScript)      │ lens / dock rectangles         │
 actions (keys) ─┘        │             │ colour matrix + shader filter  │
                          │             │ pointer scale                  │
                          ▼             └────────────────────────────────┘
                 overlay surfaces (focus ring, ruler, crosshair) ─▶ composed below secure surfaces
```

Tracking at pointer rate: to avoid a round trip per motion event, the app sends the tracking mode and margins once and the compositor moves the view for pointer tracking itself. Focus and caret moves are sent by the app as "bring rectangle into view" requests.

### 6.2 Data model

Stores: `svc.settings` namespace `org.pbsd.magnifier`; profiles in `~/.config/org.pbsd.magnifier/profiles.json`; system namespace `system.a11y.magnifier` for the greeter.

| Entity | Fields |
|---|---|
| `profile` | `id: uuid`, `name: string`, `builtin: bool`, `zoom: ZoomSettings`, `tracking: TrackingSettings`, `filters: FilterEntry[]`, `pointer: PointerSettings`, `highlight: HighlightSettings`, `aids: AidSettings` |
| `ZoomSettings` | `mode: enum{full, lens, docked, split}`, `level: f32`, `step_percent: u8`, `presets: f32[5]`, `filter_kind: enum{nearest, bilinear, bicubic, sharp_bilinear}`, `rerender_max: f32`, `text_aa: enum{app, grey, none}`, `smooth: bool`, `snap_integer: bool`, `lens: {w: u16, h: u16, shape: enum, border_px: u8, border_colour: rgba, offset: bool}`, `dock: {edge: enum, percent: u8}` |
| `TrackingSettings` | `pointer: enum{continuous, centred, edge}`, `edge_margin_percent: u8`, `focus: bool`, `caret: bool`, `caret_lead_percent: u8`, `review_cursor: bool`, `popups: bool`, `notifications: enum{slide, inset, off}`, `priority: enum{last_moved, pointer, keyboard}` |
| `FilterEntry` | `id: string`, `kind: enum{invert, smart_invert, grey, grey_invert, protan, deutan, tritan, tint, two_colour, custom}`, `strength: u8`, `colours: rgba[2]`, `enabled_in_cycle: bool`, `matrix: f32[20]?` |
| `PointerSettings` | `scale: f32`, `under_zoom: enum{scale, constant, cap}`, `fill: rgba`, `edge: rgba`, `halo: {on: bool, radius: u16, colour: rgba}`, `ripple: bool`, `shake: bool` |
| `output_state` | `edid_hash: blake3`, `own_settings: bool`, `profile_overrides: json`, `view: {x: f32, y: f32, level: f32}`, `saved_views: View[9]` |
| `app_binding` | `bundle_id: string`, `profile_id: uuid` |

Compositor request record (Proposed): `set_view{output: u32, mode: u8, scale: f32, cx: f32, cy: f32, lens: rect, dock: rect, track: TrackParams, filter: u8}`, `set_colour{output: u32, matrix: f32[20], shader: u8, params: f32[4], scope: u8}`, `set_pointer{scale: f32, fill: rgba, edge: rgba}`, `reveal{output: u32, rect: rect, margin: f32}`. Schema version in every message (PLATFORM section 7). Profile file has `"schema": 1`; migration is forward only.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| View-transform and colour interface (session bus schema or a private Wayland extension; open, section 13) | yes | yes | schema compiler output (G-08) | app host; compositor validates and clamps |
| `svc.a11y` track and text records | yes | no | schema compiler output | app host |
| Profile JSON | yes | yes | `forge::json` | app host (ForgeScript) |
| Colour matrices | — | yes | own maths (3×3 daltonisation matrices in linear RGB) | app host |
| Layer-shell overlay surfaces | — | yes | FUSE Wayland backend (G-01, FUSE-WP-10) | app host |

### 6.4 Integration

- Intents: none handled or emitted, except `view` for help.
- Interface offered: `magnifier.get() -> state`, `magnifier.set(patch)`, `magnifier.toggle(what)`; callers need the user-granted `svc.magnifier` service right (AX01, AX06, SH01, SH02 by default).
- Services used: `svc.a11y` (rights in section 7), `svc.settings`, `svc.notify`.
- Other apps: SH02 (indicator, quick tile, level display), SH03 (greeter and lock instance), SH04 (prompt geometry events), SH05 (dock exclusive zone, shortcut reservation), SY01 (on/off, theme and scale are there), AX01 (review cursor rectangle, `speech.say`), AX03 and AX06 (highlight rectangle), ME05 (unfiltered capture rule), AD01 (installer).
- Verified in repos: the per-app a11y snapshot carries `bounds`, `focused`, `focusable` per node (`FUSE:Source/FUSE/UI/Core/include/fuse/ui/ui.hpp:86-101`); the text-input caret area call `setTextInputArea(window, x, y, width, height)` exists for Win32 and X11 (`FUSE:Source/FUSE/Core/include/fuse/platform/text_services.hpp`); the compositor tree has modules named for fractional scale, viewporter, cursor shape, colour management, layer shell and pointer warp (`PBSD:pbsd/compositor/`), each a small opcode table. Everything about zoom is Proposed.

## 7. Security and privacy

### 7.1 Trust model

A classic magnifier captures the screen and redraws it. That makes it a screen recorder with a friendly name. Here the Magnifier holds only narrow rights, each granted through `svc.a11y` and listed in AD05:

| Right | Grants | Why it is narrow |
|---|---|---|
| `svc.a11y:view.transform` | Set scale, offset, lens and dock rectangles, pointer scale, colour matrix | Parameters only. No pixel ever reaches the app. The compositor clamps every value (F119). |
| `svc.a11y:track` | Rectangles of the focused element, the caret, popups and prompts; pointer position | Geometry only: no names, no text. |
| `svc.a11y:overlay` | Input-transparent surfaces above normal windows | Cannot take input; always below secure surfaces; opacity capped. |
| `svc.a11y:text.read` (optional, off until the text lens is first used) | Text of the element under the pointer | Prompted separately; never returns secret-field values. |

Rules shared with all assistive tools (AX01 section 7.1): system-signed bundle, user enable in SY01 through an SH04 prompt, immediate revocation, audit of grants. The Magnifier holds no `act`, no key rights and no `input.synth`: it cannot click or type.

### 7.2 Capabilities requested

| Capability | Why |
|---|---|
| `fs.app` | Profiles |
| `fs.portal` | Import and export of profiles |
| `net.none` | No network use |
| `svc.a11y` (`view.transform`, `track`, `overlay`, optional `text.read`) | Zoom, tracking, highlights, text lens |
| `svc.settings`, `svc.notify` | Settings; indicator and level display |
| `bg.run`, `bg.autostart` | Runs without a window; starts at login |

Not requested: `dev.screen`, `dev.gpu.compute`, `proc.helper`, `svc.clipboard`, `ai.local`.

### 7.3 Threat model

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| Screen contents | A compromised Magnifier | Exfiltrate what is on screen | No pixels, `net.none`; with `text.read` off it has geometry only |
| The user's view of a prompt | A compromised Magnifier | Zoom away from, cover or recolour a capability prompt so the user approves blind | Compositor forces the view to include the prompt while one is open (F033); overlays below secure surfaces; colour clamp (F119); reset key in the compositor (F117) |
| Session usability | A compromised Magnifier or bad profile | Make the screen unreadable (extreme zoom, black matrix) | Clamps; reset key; greeter instance ignores user profiles |
| Other apps | Any app | Call `magnifier.set` to hide part of the screen | Interface needs a granted service right; callers are logged |
| Status colours | Colour filter | A filter makes "blocked" and "verified" look alike | Status always has icon and word (DESIGN_SYSTEM section 2); F069 warning |
| Privacy | Shoulder surfer | Large text is easier to read from afar | Not solved here; text lens closes on release; noted in help |
| Screenshots | Confusion | Captures show the filtered view and mislead a recipient | Captures are unfiltered by default (F067) |
| Typing rhythm | Compromised Magnifier | Infer typing from caret rectangle events | Accepted residual risk; caret events carry no characters; noted in AD05's description of `track` |

Stored: settings and profiles only. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `enabled` | bool | false | Master switch |
| `zoom.mode` | enum | `full` | Mode |
| `zoom.level` | f32 | 2.0 | Last level |
| `zoom.start` | enum{last, fixed, one} | `one` | Level at login |
| `zoom.step_percent` | u8 | 25 | Key step |
| `zoom.presets` | f32[] | `[2, 4, 8]` | Presets |
| `zoom.filter` | enum | `sharp_bilinear` | Scaling filter |
| `zoom.rerender_max` | f32 | 3.0 | 1.0 turns re-render off |
| `zoom.text_aa` | enum{app, grey, none} | `app` | Text smoothing while zoomed |
| `zoom.smooth` | bool | true | Animated zoom |
| `zoom.modifier` | enum{super, ctrl_alt, super_ctrl} | `super` | Scroll and pinch modifier |
| `zoom.outputs` | enum{pointer, all, fixed} | `pointer` | Which outputs zoom |
| `lens.width`, `lens.height` | u16 | 480, 320 | Lens size |
| `lens.shape` | enum{rect, rounded, circle} | `rounded` | |
| `dock.edge`, `dock.percent` | enum, u8 | `top`, 30 | Docked view |
| `track.pointer` | enum | `edge` | Pointer tracking mode |
| `track.edge_margin_percent` | u8 | 15 | |
| `track.focus`, `track.caret` | bool | true, true | |
| `track.caret_lead_percent` | u8 | 30 | Space ahead of the caret |
| `track.popups` | bool | true | |
| `track.notifications` | enum | `inset` | |
| `filter.active` | string | `none` | Active filter id |
| `filter.cycle` | string[] | `[invert, grey]` | Cycle list |
| `filter.scope` | enum{output, view, lens} | `output` | |
| `filter.contrast`, `.brightness`, `.saturation`, `.gamma` | i8, i8, u8, f32 | 0, 0, 100, 1.0 | |
| `pointer.scale` | f32 | 1.0 | 1–8 |
| `pointer.under_zoom` | enum | `cap` | |
| `pointer.halo`, `pointer.ripple`, `pointer.shake` | bool | false, false, true | |
| `highlight.focus` | bool | false | Focus rectangle |
| `highlight.focus_px`, `highlight.focus_colour` | u8, rgba | 4, `--accent` | |
| `highlight.caret` | enum{off, bar, markers, circle} | `off` | |
| `aids.ruler`, `aids.line_focus_percent` | bool, u8 | false, 60 | |
| `aids.crosshair` | bool | false | |
| `aids.tint_colour`, `aids.tint_percent` | rgba, u8 | `#FFE9A8`, 0 | |
| `textlens.font_pt`, `textlens.colours` | u8, enum | 36, `theme` | |
| `capture.as_seen` | bool | false | Screenshots include zoom and filter |
| `system.a11y.magnifier.greeter` | bool (system) | false | Available keys always work; this starts it zoomed |
| `system.a11y.magnifier.greeter_level` | f32 (system) | 2.0 | |

## 9. Accessibility and languages

- The settings window is exempt from its own tracking while it has focus, and stays fully inside the view at any zoom up to 4× at its minimum size 480×320.
- All controls work with AX01; slider changes are announced with units ("4 times").
- Caret lead (F028) follows the writing direction: for right-to-left text the space is kept on the left. Vertical ruler (F090) serves vertical scripts.
- Colour-blindness terms are shown with plain descriptions ("red-green, type 1 (protan)").
- The control bar is 44 px high in touch density.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Key press to zoom change on screen | next frame, under 20 ms |
| Added latency of the zoom stage | 0 frames (done in the same composition pass) |
| Added GPU time per frame, full-screen zoom, 4K output | under 1 ms |
| Pointer tracking | no app round trip (compositor side) |
| Focus or caret move to view move started | under 30 ms |
| App memory | under 25 MB |
| App idle | zero wake-ups when focus, caret and settings do not change |
| Settings window first frame | under 150 ms |
| Colour filter change | one frame after the fade |

## 11. Testing and proof

Nothing has run. All of this is plan.

- Unit (ForgeScript): tracking maths for the three pointer modes at output edges; "reveal rectangle" for all writing directions; filter matrix composition; daltonisation matrices against published values; clamp logic mirror; per-output state on hot-plug sequences.
- Golden: compositor zoom output for a fixed scene at 2×, 3×, 4.5× with each scaling filter; each colour filter over a fixed swatch chart; `.fuseui` renders of every settings page; overlay renders.
- Scripted UI: drive fixture apps; assert the view contains the focus and caret rectangles after each move; open a fake prompt and assert the view contains it and no overlay is above it.
- Fuzz: compositor interface with hostile parameters (NaN, infinities, negative sizes, huge rectangles); profile JSON import.
- Security tests: an unprivileged client calling the interface; matrix that blanks the screen; overlay at 100% opacity; kill the app while zoomed and press the reset key.
- Performance: frame-time traces with zoom on and off on reference hardware and on the CPU rasteriser.
- Human: sessions with low-vision users on tracking comfort before Phase 1 closes.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Compositor clamp of view parameters | For every input the view rectangle lies inside the output and scale is in [1, 32] | T1 | Small pure function; proof in Lean or by PRISM | — |
| Compositor colour clamp | The output contrast of black versus white input is at least 3:1 for every accepted matrix | T1 | Proof over the matrix acceptance test | Floating-point model |
| Prompt visibility | While a secure surface is shown, its rectangle is within the view and no app overlay is above it | T1 | State-machine model of the compositor's stacking and view rules | Compositor does not exist beyond scaffolds |
| Zoom shader stage | No out-of-bounds texture access | T2 | PRISM on the CPU path; validation layers on the GPU path | GPU shaders are outside PRISM |
| Interface parser | No runtime errors on any message | T2 | PRISM bounded checks; fuzzing | — |
| Magnifier core (ForgeScript) | Memory safety by the interpreter | T3 | Interpreter guarantees; unit tests | FORGE-WP-01 |
| Tracking comfort | — | T4 | Human testing | Not formal |

## 12. Build plan

### Phase 1: usable core

| ID | Work item | Depends on |
|---|---|---|
| AX02-W01 | Design and agree the view-transform interface with the compositor | AX02-GAP-1 |
| AX02-W02 | Compositor: full-screen zoom, pointer tracking modes, clamps, reset key | AX02-W01 |
| AX02-W03 | Compositor: lens composition | AX02-W02 |
| AX02-W04 | Compositor: colour matrix stage; invert, greyscale, contrast | AX02-GAP-2 |
| AX02-W05 | Magnifier core: state, actions, global shortcuts | G-07, G-08, SH05 |
| AX02-W06 | Focus and caret tracking through `svc.a11y:track` | AX02-GAP-4, G-04, FUSE-WP-56 |
| AX02-W07 | Prompt tracking and secure-surface stacking rule | SH04, AX02-W02 |
| AX02-W08 | Pointer size and locate pointer | AX02-GAP-5 |
| AX02-W09 | Focus highlight overlay | FUSE-WP-10, AX02-W06 |
| AX02-W10 | Settings window, control bar, indicator, quick tile | G-02, SH02 |
| AX02-W11 | Greeter, lock screen and installer instance | SH03, AD01 |
| AX02-W12 | Peek overview and zoom toggle | AX02-W02 |

### Phase 2: full feature

| ID | Work item | Depends on |
|---|---|---|
| AX02-W13 | Docked mode with exclusive zone | SH05, AX02-W03 |
| AX02-W14 | Re-render at scale and text smoothing hint | AX02-GAP-3, FUSE-WP-06, FUSE-WP-16 |
| AX02-W15 | Daltonisation, smart invert, tint, brightness, saturation, gamma, filter scope | AX02-W04 |
| AX02-W16 | Per-monitor state, hot-plug, mirrored outputs | AX02-W05 |
| AX02-W17 | Reading ruler, line focus, crosshair, screen tint | AX02-W09 |
| AX02-W18 | Caret highlight, halo, ripple, shake | AX02-W09 |
| AX02-W19 | Text lens | `svc.a11y:text.read`, FUSE-WP-55 |
| AX02-W20 | Profiles, presets, import and export | AX02-W10 |
| AX02-W21 | Touchpad gestures | FUSE-WP-08, compositor gestures |
| AX02-W22 | Actions interface and command line | AX02-W05 |
| AX02-W23 | AX01 review cursor tracking and highlight | AX01-W06 |
| AX02-W24 | Unfiltered capture rule | ME05, compositor |
| AX02-W25 | CPU fallback path | FUSE-WP-25 or compositor CPU path |
| AX02-W26 | Test chart, status colour check, first-run guide | AX02-W15 |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| AX02-W27 | Split mode, second-monitor view | AX02-W13 |
| AX02-W28 | Per-app profiles and per-app filter exclusion | compositor per-surface filter flag |
| AX02-W29 | Two-colour mode, simulation filters, edge sharpening | AX02-W15 |
| AX02-W30 | Spotlight, screen mask, pointer trail, presentation profile | AX02-W18 |
| AX02-W31 | Touch-screen gestures | FUSE-WP-08 |
| AX02-W32 | HDR-correct zoom and filters | FUSE-WP-26, compositor colour management |
| AX02-W33 | Saved views, axis lock, vertical ruler | AX02-W17 |
| AX02-W34 | Voice and switch highlight tracking; remote sessions | AX03, AX06, VM04 |

## 13. Open questions and risks

1. **Where the interface lives.** A private Wayland extension keeps zoom in the display path and avoids a bus hop; a session-bus interface fits PLATFORM section 7. Either way the compositor must authenticate the caller as the holder of `svc.a11y:view.transform`. To decide with the compositor owner.
2. **Compositor reality.** The compositor modules read are opcode tables of 20 to 40 lines (`PBSD:pbsd/compositor/pbsd.compositor.wayland.*.cppm`). No composition code was read. All compositor work items here are new work; their size is unknown.
3. **Re-render at scale.** Asking clients for 3× buffers costs memory (a 4K window at 3× is over 280 MB per buffer). The limit, and whether only the visible part can be re-rendered (viewporter source rectangles), needs a prototype.
4. **Non-FUSE clients.** Apps from the compatibility personalities (VM03) will not honour the text smoothing hint and may not report focus or caret; tracking falls back to the pointer.
5. **Docked mode and window management.** The exclusive zone changes work-area geometry; SH05 must handle it changing at run time.
6. **Typing-rhythm leak through caret events** (section 7.3). Could be closed by moving caret tracking into the compositor as well, using the text-input caret rectangle; costs flexibility. Open.
7. **Smart invert quality.** A matrix cannot tell photos from UI. Per-surface content-type hints (`content_type.cppm` exists by name) could let the compositor skip images and video; Unverified.
8. **Shortcut clashes.** `Super+=`, `Super+-` and `Super+Esc` must be reserved in SH05's default map.
