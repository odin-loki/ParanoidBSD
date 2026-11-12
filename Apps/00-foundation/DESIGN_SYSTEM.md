<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Design system

By Odin Loch

One look, one set of behaviours, for every app. All of it is Proposed; the styling features it relies on (CSS subset, tokens, pseudo-classes, layouts) are Verified in `FUSE:docs/ui/README.md`.

## 1. Principles

1. **Show the truth about security.** What an app can reach is always visible: a network badge, device indicators, a lock on encrypted things. Nothing sensitive happens silently.
2. **Content first.** Chrome is thin. One primary action per view.
3. **Keyboard complete.** Everything has a shortcut or is reachable from the command palette.
4. **Calm.** No animations longer than 200 ms, no attention-seeking badges, no modal dialog where an inline message works.
5. **Reversible.** Undo instead of "are you sure?". Confirm only what cannot be undone.
6. **Same thing, same place.** Sidebar left, inspector right, search top, status bottom, in every app.
7. **Dense when asked.** Three densities; power-user apps default to compact.

## 2. Tokens

Defined once in the system style sheet under `:root`; apps use `var(--name)` and never hard-code values.

### Colour (dark default, light and high-contrast variants)

| Token | Dark | Light | Use |
|---|---|---|---|
| `--bg-0` | `#0E1013` | `#F5F6F8` | window background |
| `--bg-1` | `#15181D` | `#FFFFFF` | panels, sidebars |
| `--bg-2` | `#1C2027` | `#EDEFF2` | cards, inputs |
| `--bg-3` | `#262B34` | `#E1E4E9` | hover, selected rows |
| `--line` | `#2E343E` | `#CFD4DB` | borders, separators |
| `--fg-0` | `#E8EAED` | `#16181C` | primary text |
| `--fg-1` | `#A9B0BA` | `#4A515C` | secondary text |
| `--fg-2` | `#6F7783` | `#7A828E` | disabled, hints |
| `--accent` | `#4C8DFF` | `#1F5FD6` | focus, primary action, selection |
| `--ok` | `#3FB27F` | `#1F8A5B` | safe, verified, done |
| `--warn` | `#E0A030` | `#A86A00` | attention, unverified |
| `--danger` | `#E5534B` | `#C4302B` | destructive, blocked, breach |
| `--net` | `#B07CFF` | `#7A3FD6` | anything that touches the network |
| `--secret` | `#2BB8C4` | `#0E7C86` | encrypted, key material |

Status is never colour alone: every state also has an icon and a word. Contrast is at least 4.5:1 for text and 3:1 for controls; the high-contrast theme reaches 7:1.

### Type

System face for UI, a monospace face for code, terminals, hashes and addresses. Sizes: 11 (caption), 12 (small), 13 (body), 15 (subhead), 18 (title), 24 (display). Line height 1.4. Numbers in tables use tabular figures.

### Space, shape, depth, motion

- Spacing scale: 2, 4, 8, 12, 16, 24, 32, 48. Density: compact (row 24), normal (row 32), touch (row 44).
- Radius: 4 (controls), 8 (cards, dialogs), 12 (windows). Borders 1 px `--line`.
- Elevation: 0 flat, 1 popups and menus, 2 dialogs, 3 drag ghosts. Shadows use `box-shadow`; backdrop blur only on shell surfaces.
- Motion: 80 ms (state), 140 ms (enter and leave), 200 ms (layout). Ease-out. Everything honours reduced motion.

## 3. Window anatomy

```
+--------------------------------------------------------------------------------+
| [=] App title — document name            [search ⌘K]        [net] [▁] [▢] [✕] |  header bar
+-----------+----------------------------------------------------+---------------+
| Sidebar   | Toolbar: primary actions · view switch · filters   | Inspector     |
| places,   +----------------------------------------------------+ properties of |
| sections, |                                                    | the selection |
| filters   |                    Content                         |               |
|           |                                                    |               |
+-----------+----------------------------------------------------+---------------+
| Status: selection · progress · background jobs            trust · encoding · zoom |
+--------------------------------------------------------------------------------+
```

- **Header bar**: menu button `[=]` (full menu is also on `F10`), title, global in-app search and command palette, capability badges, window buttons. Badges: `[net]` when a socket is open (click lists endpoints), camera, microphone, screen and location indicators while in use, a lock when the document is encrypted.
- **Sidebar** 220 px, collapsible to icons (`F9`). **Inspector** 280 px, toggled with `F4`. **Status bar** optional per app.
- Minimum window 480×320. Layout breakpoints: narrow under 640 (sidebar becomes an overlay, inspector a sheet), medium 640–1100, wide above 1100.
- Tabs live under the header bar. Split views by dragging a tab to an edge.
- Every window remembers size, position, panels and scroll per document.

## 4. Layout patterns

| Pattern | When | Apps |
|---|---|---|
| Sidebar + content | Navigation among places or sections | Files, Settings, Mail |
| Three-pane master-detail | List of items, one open | Mail, Notes, Feeds, Passwords |
| Document canvas + inspector | Editing one object | Writer, Sheets, Image Editor |
| Dashboard | Live status | System Monitor, Security Centre |
| Workbench with docks | Many tools at once | IDE, Reverse Engineering, Video Editor |
| Wizard | Rare multi-step setup | Installer, new VM, account setup |
| Single-purpose utility | One small task | Calculator, Clocks |

## 5. Components

Use the FUSE widgets in PLATFORM.md V3. Rules on top:

- **Buttons**: one primary (filled `--accent`) per view; destructive is outlined `--danger` and never the default.
- **Inputs**: label above, hint below, error below in `--danger` with an icon; validation on blur, never on each key.
- **Lists, tables, grids**: virtualised; multi-select with Shift and Ctrl; type-ahead; column chooser; sortable headers; inline rename with `F2`; context menu on right click and `Shift+F10`.
- **Menus**: every item shows its shortcut; disabled items say why in a tooltip.
- **Dialogs**: title is the question, buttons are verbs ("Delete 3 files", "Keep"). Esc cancels. No dialog for success.
- **Toasts**: bottom centre, 5 s, with Undo where possible; errors stay until dismissed and offer "Details".
- **Empty states**: one line saying what goes here and one button to make the first thing.
- **Progress**: determinate when known; long jobs move to the status bar and to notifications and survive closing the window.
- **Command palette** (`Ctrl+K` / `Ctrl+Shift+P`): fuzzy search over every action, setting, recent item and help page.
- **Security patterns**: trust badge (verified, unverified, tampered), capability prompt (drawn by the shell), secret field (hidden by default, reveal on hold, clipboard auto-clear), "open in a disposable sandbox" for untrusted files.

Missing widgets each spec may need are listed as platform gaps (PLATFORM.md section 13).

## 6. Keyboard

Reserved everywhere:

| Keys | Action | Keys | Action |
|---|---|---|---|
| `Ctrl+N / O / S / Shift+S / W / Q` | new, open, save, save as, close, quit | `Ctrl+Z / Shift+Z` | undo, redo |
| `Ctrl+X / C / V / A` | cut, copy, paste, select all | `Ctrl+F / G / H` | find, find next, replace |
| `Ctrl+K` | command palette | `Ctrl+,` | preferences |
| `Ctrl+P` | print | `Ctrl+Tab / Shift+Tab` | next, previous tab |
| `Ctrl++ / - / 0` | zoom | `F1` | help for the focused thing |
| `F2` | rename | `F4` | inspector |
| `F5` | refresh | `F9` | sidebar |
| `F10` | menu | `F11` | full screen |
| `Alt+Left / Right` | back, forward | `Delete / Shift+Delete` | to trash, delete for ever |
| `Super` | launcher | `Super+L` | lock |

Focus is always visible (2 px `--accent` ring). Tab order follows reading order. All shortcuts are listed and rebindable in Settings.

## 7. Accessibility and languages

- Every control has a role, a name and a state in the a11y tree; images have text; live regions announce progress and errors.
- Works at 100–300% scale, with large text, in high contrast, with reduced motion, and with a screen reader, switch access or voice control.
- Targets are at least 24 px (44 px in touch density).
- All text comes from string tables; layouts mirror for right-to-left; dates, numbers and units follow the locale; plural forms are handled by the table format.

## 8. Wireframe notation used in the specs

ASCII drawings in fenced blocks. `[Button]`, `[x] checkbox`, `( ) radio`, `[____] input`, `[v] dropdown`, `|` and `+-` for panes, `>` for a selected row, `…` for overflow, `(n)` for a callout explained under the drawing. Each wireframe states the window size it shows and lists the states it has (empty, loading, error, offline, locked, read-only).
