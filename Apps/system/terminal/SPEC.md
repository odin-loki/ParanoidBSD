<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Terminal

By Odin Loch

| | |
|---|---|
| ID | `SY03` (from CATALOGUE.md) |
| Bundle | `org.pbsd.terminal` |
| Category | system |
| Priority | P0 |
| Backing repos | FUSE (terminal grid widget planned as FUSE-WP-39, gap G-13), FORGE (`forge script` REPL, `docs/scripting.md`) |
| Status | Planned. Nothing built. |

## 1. Purpose

Terminal runs command-line programs. It is used by developers, administrators and anyone following instructions that say "type this". It differs from other terminal emulators in one way that matters: a shell can be started with a chosen, visible set of capabilities, for example one folder and no network, and what it was given is always on screen. It replaces Konsole from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- A VT/xterm-compatible terminal emulator with tabs, splits, profiles, search and scrollback.
- Session persistence through a session keeper process (detach and reattach).
- Capability-scoped shells and a scope picker.
- An SSH connection manager (the terminal side: hosts, launching, port forwards display).
- Paste protection, command blocks, recording, broadcast input, drop-down ("quake") window.
- A ForgeScript REPL profile.

Out of scope:

| Item | Owner |
|---|---|
| The terminal grid widget and escape-sequence parser | FUSE (FUSE-WP-39); this app is its first user |
| SSH key generation, storage and the agent | SE04 Passwords and Keys |
| Remote graphical sessions | VM04 Remote Desktop |
| File transfer queues | IN03 Transfers (Terminal offers simple drag-and-drop upload over SFTP only) |
| Editing files | OF01 Text Editor, DV01 IDE |
| VM and jail consoles | VM01, VM02 (they embed the same widget) |
| The shell programs themselves, `tmux`, `ssh` | base system |
| Grant store and lineage | AD05 Permissions |
| Log viewing | AD04 Logs |

## 3. Users and top tasks

Personas:

- **Developer**: many tabs and splits, long scrollback, search, links to files and errors.
- **Administrator**: SSH to many hosts, broadcast input, recordings for audit.
- **Careful user**: pastes a command from a web page and wants to see it before it runs; runs an unknown script with no network and one folder.
- **Learner**: uses the ForgeScript REPL.

| # | Task | Target steps |
|---|---|---|
| T1 | Open a terminal in the home folder | 1 |
| T2 | Split the pane and run a second command | 1 (`Ctrl+Shift+D`) |
| T3 | Search scrollback for a word | 2 |
| T4 | Start a shell limited to one folder with no network | 3 (`Ctrl+Shift+N`, pick folder, Start) |
| T5 | Connect to a saved SSH host | 2 |
| T6 | Copy the output of the last command | 1 (`Ctrl+Shift+O`) |
| T7 | Jump to the previous prompt | 1 (`Ctrl+Shift+Up`) |
| T8 | Reattach to yesterday's session after a restart | 0 (automatic) or 2 |
| T9 | Paste a multi-line command safely | 2 (paste, confirm) |
| T10 | Record a session and export it | 3 |
| T11 | Type the same command on five hosts | 3 |
| T12 | Toggle the drop-down terminal | 1 (`F12` global) |
| T13 | Open a ForgeScript REPL | 2 |
| T14 | Switch a profile's colours | 3 |

## 4. Features

### 4.1 Emulation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY03-F001 | Compatibility levels | Selectable per profile: VT100, VT220, VT420 subset, xterm (default). `TERM` defaults to `xterm-256color`; an own terminfo entry `pbsd-terminal` is shipped for extra features. | M | 1 |
| SY03-F002 | Control functions | C0, C1 (7-bit forms), CSI, OSC, DCS, ESC sequences: cursor movement, erase, insert and delete, scroll regions, left and right margins (DECLRMM), tab stops, character sets (DEC special graphics), origin mode, auto-wrap, reverse wrap. | M | 1 |
| SY03-F003 | SGR attributes | Bold, faint, italic, underline (single, double, curly, dotted, dashed, with colour), blink, inverse, invisible, strike, overline. | M | 1 |
| SY03-F004 | Colours | 16, 256 and 24-bit colour; palette set and query; default foreground, background, cursor colours by OSC 10, 11, 12 (set allowed, query answers per policy F072). | M | 1 |
| SY03-F005 | Alternate screen | DECSET 47, 1047, 1049; option to keep alternate-screen content out of scrollback. | M | 1 |
| SY03-F006 | Mouse reporting | X10, normal, button-event, any-event tracking; SGR (1006) and pixel (1016) encodings; `Shift` bypasses reporting for local selection. | M | 1 |
| SY03-F007 | Bracketed paste | DECSET 2004. | M | 1 |
| SY03-F008 | Focus events | DECSET 1004. | S | 1 |
| SY03-F009 | Synchronised output | DECSET 2026, with a 150 ms cap so a program cannot freeze the screen. | S | 1 |
| SY03-F010 | Unicode | UTF-8 only; grapheme clusters; East Asian wide cells; combining marks; emoji sequences; width table versioned and stated in the profile. | M | 1 |
| SY03-F011 | Complex scripts | Shaping for connected scripts and right-to-left runs on a line, as a profile option (needs G-03). | S | 3 |
| SY03-F012 | Keyboard encoding | xterm `modifyOtherKeys`, application cursor and keypad modes, and the progressive "disambiguate keys" protocol (CSI u) so programs can tell `Ctrl+I` from `Tab`. | M | 2 |
| SY03-F013 | Cursor | Block, bar, underline; blink on or off (DECSCUSR); blink stops when unfocused. | M | 1 |
| SY03-F014 | Resize and reflow | Soft-wrapped lines reflow on resize; `SIGWINCH` sent once per settled size; size report in cells and pixels. | M | 1 |
| SY03-F015 | Hyperlinks | OSC 8 explicit links; detected URLs, file paths with `:line:col`, and e-mail addresses. Hover shows the real target. `Ctrl+click` opens through `svc.intent`. | M | 1 |
| SY03-F016 | Link safety | A link whose text looks like a different host than its target gets a warning and needs a second confirmation. Links never open on a plain click. | M | 1 |
| SY03-F017 | Inline images | Sixel and the Kitty graphics protocol (direct and shared-memory transmission only, no file-path transmission). Decoding in an image helper. Per-profile switch, off for untrusted scopes. | S | 2 |
| SY03-F018 | Shell integration marks | OSC 133 prompt, command and output marks; OSC 7 working directory; OSC 0/2 title. Integration snippets for sh, zsh, fish, bash are shipped and are opt-in per profile. | M | 1 |
| SY03-F019 | Notifications from programs | OSC 9 / 777 raise a desktop notification only if the profile allows and the tab is not focused; rate-limited. | S | 2 |
| SY03-F020 | Clipboard sequences | OSC 52 write is allowed after a per-tab prompt ("always for this profile" possible); OSC 52 read is refused by default. | M | 1 |
| SY03-F021 | Bell | Visual flash, tab mark, sound, urgency hint; each on or off. | M | 1 |
| SY03-F022 | Box drawing and powerline glyphs | Drawn as geometry so lines join at every size (in the widget, FUSE-WP-39). | M | 1 |
| SY03-F023 | Font features | Ligatures on or off, font fallback chain, bold-as-bright option, cell width and height adjust, per-profile font and size. | M | 1 |
| SY03-F024 | Conformance report | A built-in page lists supported sequences and the result of the last self-test run. | C | 3 |

### 4.2 Windows, tabs and splits

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY03-F025 | Tabs | New, close, reorder, detach to a window, pin, rename, colour; activity and silence marks; bell mark; exit-status mark. | M | 1 |
| SY03-F026 | Splits | Split right, down; any depth; resize by keys or drag; zoom one pane (`Ctrl+Shift+Enter`); swap; rotate; equalise. | M | 1 |
| SY03-F027 | Pane navigation | `Alt+arrows` move focus; number overlay to jump (`Ctrl+Shift+Q`). | M | 1 |
| SY03-F028 | New in same folder | New tabs and splits start in the working directory of the current pane (from OSC 7 or the process's directory through the session keeper). | M | 1 |
| SY03-F029 | Layouts | Save a window's tabs, splits, profiles, folders and start commands as a named layout; open it later. | S | 2 |
| SY03-F030 | Drop-down window | A window that slides from the top on a global key (`F12`, set in SY01); width, height, screen, hide on focus loss, always-on-top. Uses layer-shell (G-01). | S | 2 |
| SY03-F031 | Tab bar options | Top, bottom, hidden when single; compact titles; show scope chip per tab. | S | 1 |
| SY03-F032 | Title templates | `{title}`, `{cwd}`, `{command}`, `{host}`, `{user}`, `{scope}`, `{profile}`. | S | 1 |
| SY03-F033 | Close protection | Closing a pane with a running foreground job names the job and asks; closing a persistent session offers "detach" instead. | M | 1 |
| SY03-F034 | Tab overview | Grid of live thumbnails of all tabs, searchable by title and directory. | C | 3 |
| SY03-F035 | Full screen and focus mode | `F11` full screen; focus mode hides chrome. | S | 1 |
| SY03-F036 | Read-only pane | Lock a pane so keys are not sent (`Ctrl+Shift+L`). | S | 2 |

### 4.3 Scrollback, search and selection

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY03-F037 | Scrollback | Byte-capped ring per pane (default 64 MiB); optional unlimited scrollback spilled to an encrypted file in the cache. | M | 1 |
| SY03-F038 | Search | `Ctrl+Shift+F`: plain, case, whole word, regex; highlights all hits; hit count; next and previous; search across all panes in the window. | M | 1 |
| SY03-F039 | Selection | Character, word (double click, word characters configurable), line (triple), block (`Alt+drag`), extend with `Shift+click`; keyboard selection mode (`Ctrl+Shift+Space`) with vi and arrow keys. | M | 1 |
| SY03-F040 | Copy | Plain text, with soft wraps joined; copy as HTML or ANSI; copy without trailing spaces; primary selection on select (option). | M | 1 |
| SY03-F041 | Quick select | `Ctrl+Shift+U` labels every URL, path, hash, IP address and number on screen with a letter; typing it copies or opens. | S | 2 |
| SY03-F042 | Scroll marks | Jump between prompts (`Ctrl+Shift+Up/Down`); marks in the scroll bar for prompts, errors, search hits and bookmarks. | M | 1 |
| SY03-F043 | Bookmarks | Mark a line (`Ctrl+Shift+M`); list and jump. | C | 2 |
| SY03-F044 | Save scrollback | Save pane text as `.txt`, `.ansi` or `.html` through the file portal. | S | 1 |
| SY03-F045 | Clear | Clear screen and scrollback (`Ctrl+Shift+K`); reset terminal state. | M | 1 |
| SY03-F046 | Timestamps | Optional gutter with the wall time each line arrived. | S | 2 |
| SY03-F047 | Triggers | Regex rules on output: highlight, notify, mark, play bell, run a ForgeScript action. Never types into the terminal. | S | 3 |
| SY03-F048 | Minimap | Narrow overview strip of the scrollback. | C | 3 |

### 4.4 Command blocks

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY03-F049 | Blocks | With shell integration, each command and its output is a block with a thin rule, exit status, duration and start time. | M | 2 |
| SY03-F050 | Block actions | Copy command, copy output, copy both, save output, re-run, fold, search within, share as text, bookmark. | M | 2 |
| SY03-F051 | Failed-command mark | Non-zero exit shows a `--danger` bar and the code; scroll-bar mark. | M | 2 |
| SY03-F052 | Long-command notice | Notify when a command that ran longer than N seconds finishes in an unfocused tab. | S | 2 |
| SY03-F053 | Sticky prompt line | While scrolling a long output the command that produced it stays pinned at the top. | S | 2 |
| SY03-F054 | Command history panel | Per-session and global list of commands with directory, exit status and time; filter; insert into the prompt (never auto-run). History is local, encrypted at rest, and can be off. | S | 2 |
| SY03-F055 | Explain and suggest | Optional: "Explain this command" and "Explain this error" send the selected text to `svc.model` locally. Shows the answer in a side panel. Never types or runs anything. Off by default. | C | 3 |
| SY03-F056 | Snippets | Named command snippets with placeholders; insert through the palette. | S | 2 |

### 4.5 Profiles and appearance

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY03-F057 | Profiles | Name, command, arguments, environment, working directory, scope (section 4.6), emulation level, font, colours, cursor, scrollback, bell, keys, integration. | M | 1 |
| SY03-F058 | Built-in profiles | Default shell; Restricted shell (folder, no network); Administrator shell (elevated through authentication); ForgeScript REPL; SSH template. | M | 1 |
| SY03-F059 | Colour schemes | Shipped schemes in dark, light and high contrast; editor with contrast check per colour pair; import and export of scheme files (own JSON, plus import of common `.itermcolors` and base16 YAML). | M | 1 |
| SY03-F060 | Follow system theme | Scheme pairs switch with the system theme. | M | 1 |
| SY03-F061 | Background | Opacity and blur (compositor permitting); no background images by default. | C | 2 |
| SY03-F062 | Minimum contrast | Adjusts foreground colours so text keeps at least a chosen contrast ratio. | S | 2 |
| SY03-F063 | Padding and line spacing | Cell padding, window padding. | S | 1 |
| SY03-F064 | Key bindings | Per profile and global; send text, send escape sequence, run action; presets (default, tmux-like). | M | 1 |
| SY03-F065 | Automatic profile switching | Rules by host, user, directory or scope (for example red background on production hosts, set by OSC 7 host or the SSH manager). | S | 2 |
| SY03-F066 | Profile export and import | Signed profile bundles; importing a profile shows its command and scope before saving. | S | 2 |

### 4.6 Capability-scoped shells and security

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY03-F067 | Scope definition | A scope is: folders with `r` or `rw`, network (none, named hosts, user-typed, route), devices, whether child programs may be run from outside the granted folders (a read-only system tree), environment allow-list, resource limits. | M | 1 |
| SY03-F068 | Scoped launch | The shell launcher helper starts the shell in a new capability domain with exactly the scope's handles. The scope can only be narrower than the Terminal app's own grants. | M | 1 |
| SY03-F069 | New scoped shell dialog | `Ctrl+Shift+N`: pick folders (file portal), network choice, time limit; start. Presets: "This folder, no network", "Home read-only", "Downloads sandbox", "Full user shell". | M | 1 |
| SY03-F070 | Scope chip | Every tab and pane header shows a chip: `home · net`, `~/proj only · no net`, `admin`. Colour: `--net` when network is allowed, `--danger` outline for administrator. Click lists the exact handles. | M | 1 |
| SY03-F071 | Widen on request | A scoped shell cannot widen itself. The user can add a folder or network from the chip menu; the shell draws the confirmation; the grant is logged. | S | 2 |
| SY03-F072 | Reply policy | Sequences that make the terminal answer (device attributes, cursor position, colour query, title report, window size, DECRQSS) follow a per-profile policy. Title report and clipboard read are off by default. | M | 1 |
| SY03-F073 | Paste protection | Pasting text with a newline, a control character, a bidirectional override, or hidden characters opens a preview: the exact text with invisible characters shown, line count, and "Paste", "Paste as one line", "Cancel". Bracketed-paste end markers inside the text are stripped. | M | 1 |
| SY03-F074 | Paste rules | Thresholds: always confirm multi-line; confirm above N bytes; never confirm in listed programs; warn on `sudo`/`doas`, `curl … \| sh` patterns and on text copied from a web origin (clipboard source flag from `svc.clipboard`). | S | 2 |
| SY03-F075 | Secret input awareness | When the pty has echo off, the tab shows a key icon, recording pauses, and broadcast input asks first. | M | 2 |
| SY03-F076 | Administrator shell | A profile that asks for authentication through SH03 and starts a root shell in its own domain, with a red chip, its own colour scheme and an idle timeout. All starts are audited. | M | 2 |
| SY03-F077 | Disposable shell | A shell with a throw-away home on a temporary dataset, destroyed on exit. | S | 2 |
| SY03-F078 | Run file hand-off | Receives "Run in Terminal…" from SY02 and shows the file's origin and signature before starting it in a chosen scope. | M | 2 |
| SY03-F079 | Escape-sequence limits | Caps on parameter count, OSC and DCS length, image size and count, title length; overflow is dropped and counted. | M | 1 |
| SY03-F080 | Scope audit | Scope starts, widenings and administrator shells go to `svc.audit`. Commands are not logged. | M | 1 |
| SY03-F081 | Network badge | The header `[net]` badge is on when any pane's scope allows network; the list shows panes and endpoints. | M | 1 |
| SY03-F082 | Jail and VM shells | Profiles that open a shell inside a jail or VM through VM02 / VM01 interfaces; the chip names the domain. | S | 3 |

### 4.7 Sessions and multiplexing

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY03-F083 | Session keeper | Each pane's pty is owned by a session keeper process, not by the window. Closing or crashing the app leaves programs running. | M | 2 |
| SY03-F084 | Detach and reattach | Detach a tab or window; a sessions list shows detached sessions with title, directory, scope, age; reattach restores screen and scrollback. | M | 2 |
| SY03-F085 | Restore at login | Layouts are restored at login; sessions persist across app restarts and, as an option, across logout (needs `bg.run`). They do not survive reboot; after reboot the layout and scrollback text are restored and marked "ended". | S | 2 |
| SY03-F086 | Session limits | Maximum detached sessions and idle timeout; administrator shells are never kept detached. | M | 2 |
| SY03-F087 | Share a session locally | Attach a second window to the same session (mirror), read-only or read-write. | C | 3 |
| SY03-F088 | tmux control mode | Show a remote `tmux -CC` session as native tabs and splits. | C | 3 |
| SY03-F089 | Scrollback persistence | Scrollback of kept sessions is stored encrypted under the user's session key. | S | 2 |

### 4.8 SSH manager

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY03-F090 | Host list | Saved hosts with name, address, port, user, key reference (SE04), jump host, tags, profile, route (clear, VPN, anonymous). Import of `~/.ssh/config` entries (read-only parse). | M | 2 |
| SY03-F091 | Connect | Opens a tab running the system `ssh` client in a scope that allows only that host and port (and the jump host). | M | 2 |
| SY03-F092 | Host keys | Known-host state shown per host; first contact shows the fingerprint; changes block and explain. Verification itself is done by `ssh`; the manager shows and manages the known-hosts store. | M | 2 |
| SY03-F093 | Agent | Keys come from the SE04 agent; each signing use can require confirmation; no private key is readable by the shell. | M | 2 |
| SY03-F094 | Port forwards | Define local, remote and dynamic forwards per host; live list of active forwards with stop buttons. | S | 3 |
| SY03-F095 | Quick connect | `Ctrl+Shift+H` palette over hosts; type `user@host` to connect once. | S | 2 |
| SY03-F096 | File drop upload | Dropping files on an SSH tab offers "Upload to the remote working directory" over SFTP on the same connection settings, or "Paste path". | C | 3 |
| SY03-F097 | Serial console | Profile type for serial devices (`dev.usb:serial`): baud, parity, flow control. | C | 3 |

### 4.9 Recording, broadcast and automation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY03-F098 | Record | Record a pane's output with timing (asciicast v2 format); a red dot shows while recording; input is not recorded unless asked. | S | 2 |
| SY03-F099 | Replay | Play a recording in a tab with speed control, seek and search. Replay runs the parser with replies and clipboard sequences disabled. | S | 2 |
| SY03-F100 | Export | Export a recording as asciicast, plain text log, or (hand-off to ME05) GIF or video. | C | 3 |
| SY03-F101 | Redaction | On export, mask lines matching patterns (tokens, keys, e-mail addresses) and any period when echo was off. | S | 3 |
| SY03-F102 | Audit recording | Policy option: administrator shells are always recorded to an append-only store readable in AD04. | C | 3 |
| SY03-F103 | Broadcast input | Send keystrokes to all panes in a tab, a window, or a chosen group; panes receiving broadcast show a coloured frame; paste protection applies once for the group. | S | 2 |
| SY03-F104 | ForgeScript REPL profile | Runs `forge script` with no file (the REPL, FORGE:docs/scripting.md "REPL") in a scope chosen like any shell. The REPL itself has no line editing, history or completion (stated in the same document), so the profile adds a line-editor front end: history, multi-line editing, and completion from `:type`. | M | 2 |
| SY03-F105 | REPL helpers | Buttons and keys for `:type`, `:dis`, `:mem`, `:gc`, `:reset`, `:vm on\|off`, `:help` (the commands the REPL documents); `Ctrl+C` cancels the running statement. | S | 2 |
| SY03-F106 | Scripting interface | A bus interface to open windows, tabs and splits, start a profile, send text (needs user-granted `svc.terminal.control`), and read block metadata. Used by DV01 and SY02. | S | 2 |
| SY03-F107 | Command-line launcher | `terminal --profile P --working-directory D --scope S -- command …` handled by the single instance. | M | 1 |
| SY03-F108 | Open intents | "Open terminal here" from SY02; `ssh://` URLs; `.cast` files; `.fs` files offered to the REPL profile. | M | 1 |

### 4.10 Housekeeping

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY03-F109 | Command palette | All actions, profiles, hosts, snippets, layouts. | M | 1 |
| SY03-F110 | Zoom | Font size per pane or window; reset. | M | 1 |
| SY03-F111 | Process information | Pane footer can show the foreground process, its PID, and a link to SY04. | S | 2 |
| SY03-F112 | Send signal | Menu to send INT, TERM, KILL, STOP, CONT, HUP to the foreground process group. | S | 2 |
| SY03-F113 | Environment hygiene | The shell's environment is built from an allow-list; no variable carries bus addresses or tokens beyond what the scope needs. | M | 1 |
| SY03-F114 | Crash recovery | If the app crashes, sessions live on in the keeper and the window reattaches by session token. | M | 2 |
| SY03-F115 | Preferences | Section 8. | M | 1 |
| SY03-F116 | Printing | Print selection or scrollback through `svc.portal.print`. | C | 3 |

## 5. GUI

### 5.1 Window map

```
Terminal window (many windows, one app instance)
├─ Header bar: menu, title, search-in-scrollback / palette, [net] badge, window buttons
├─ Tab bar: tabs with scope chip, activity mark, exit mark; [+] with profile menu
├─ Content: split tree of panes
│   └─ Pane: header strip (title, scope chip, record dot, broadcast mark, lock)
│           + terminal grid + scroll bar with marks + sticky prompt line + search bar
├─ Side panels (right, one at a time): Sessions · SSH hosts · Command history · Snippets
│                                      · Explain (local model) · Block details
├─ Status bar (optional): size in cells, encoding, foreground process, keeper state
├─ Dialogs: New scoped shell · Paste preview · Close with running job · Host details
│           · Host key first contact · Profile import preview · Save layout · Export recording
│           · Clipboard write request · Widen scope
├─ Popovers: Scope details · Profile menu · Block menu · Pane number overlay
├─ Drop-down window (layer surface)
├─ Recording player tab
└─ Preferences window: General · Profiles · Colours · Keys · Scopes · SSH · Sessions · Advanced
```

### 5.2 Main window

Shown at 1100×700 with two tabs and a vertical split.

```
+----------------------------------------------------------------------------------------+
| [=] Terminal — ~/proj/pbsd                     [ Find…  Ctrl+Shift+F ]  [net] [▁][▢][✕] | (1)
+----------------------------------------------------------------------------------------+
| [zsh ~/proj/pbsd  home·net ×] [build01  ssh:build01 ×] [sandbox  ~/dl only·no net ×] [+v]| (2)
+--------------------------------------------+-------------------------------------------+
| zsh — ~/proj/pbsd        [home · net]      | sh — ~/dl          [~/dl only · no net] ● | (3)
|--------------------------------------------|-------------------------------------------|
| ▌ make test                    0 · 4.2 s   | ▌ ./setup.sh                              | (4)
| [ 98%] Linking pbsd_pkg_test               | fetch: example.org: no network            |
| 12 tests, 0 failures                       | ▌ exit 1 ───────────────────── 1 · 0.3 s ✕| (5)
| ───────────────────────────────────────── ▮| ▌ ls                                      |
| ▌ git status                   0 · 0.1 s  ▮| setup.sh  notes.txt                       |
| On branch main                            ▯|                                           | (6)
| nothing to commit                         ▯| $ █                                       |
|                                           ▯|                                           |
| $ █                                       ▯|                                           |
+--------------------------------------------+-------------------------------------------+
| 54×12 · UTF-8 · zsh (pid 4121) · session kept                    2 panes · 3 sessions  | (7)
+----------------------------------------------------------------------------------------+
```

1. Header bar. The `[net]` badge is lit because at least one pane's scope allows network.
2. Tab bar. Each tab shows title and scope chip. `[+v]` opens the profile menu; plain click starts the default profile.
3. Pane header strip, 22 px: process and directory, scope chip, and marks (`●` recording, `⇉` broadcast, `🔒` read-only, key icon when echo is off). Can be hidden; the chip then moves into the tab only.
4. Command block header: prompt mark, command, exit status and duration at the right. The block gutter is clickable (block menu).
5. A failed block: `--danger` bar, exit code and the word "failed" in the a11y name.
6. Scroll bar with marks: prompts (grey), failures (`--danger`), search hits (`--accent`), bookmarks.
7. Status bar: grid size, encoding, foreground process, session keeper state.

States: starting (grid blank with "Starting zsh…" for at most 200 ms), exited (banner "Process exited with code 1" [Restart] [Close]), disconnected from keeper (banner with [Reattach]), scope revoked (banner "Access to ~/proj was revoked in Permissions"; the shell keeps running without the handle), locked (read-only pane).

### 5.3 Other views and dialogs

#### 5.3.1 New scoped shell

```
+------------------------------ New scoped shell ------------------------------+
| Preset   [This folder, no network v]            Profile [Default shell v]    |
|------------------------------------------------------------------------------|
| Folders                                                                      |
|   ~/Downloads/tool-1.2          [read and write v]            [Remove]       |
|   [+ Add folder…]                                                            |
| System programs   [x] May run programs from the system (read-only)           |
| Network           (•) None   ( ) These hosts [____________]   ( ) Any        |
|                   Route [clear v]                                            |
| Devices           [ ] USB serial   [ ] GPU compute                           |
| Home              (•) Empty throw-away home   ( ) My home, read-only         |
| Limits            Memory [2 GiB v]  Processes [256]  Time limit [none v]     |
|------------------------------------------------------------------------------|
| This shell will see: 1 folder, the system programs, no network.              |
| [Save as preset…]                                   [Cancel]  [Start shell]  |
+------------------------------------------------------------------------------+
```

The summary line is generated from the scope and is the same text the scope chip shows. Folders are added through the file portal, so the app receives handles, not paths.

#### 5.3.2 Paste preview

```
+--------------------- Paste 3 lines into "zsh — ~/proj"? ---------------------+
| Source: clipboard, copied from Web Browser (example.org)            ⚠        |
|------------------------------------------------------------------------------|
| 1 | curl -s https://example.org/install.sh | sh␤                             |
| 2 | echo done␤                                                               |
| 3 | ⟦U+202E⟧ rm -rf ~/x                                                      |
|------------------------------------------------------------------------------|
| ⚠ Line 1 downloads and runs a script.   ⚠ Line 3 has a hidden direction mark.|
| [ ] Do not ask for multi-line pastes in this profile                         |
|                         [Cancel]  [Paste as one line]  [Paste 3 lines]       |
+------------------------------------------------------------------------------+
```

Text is shown in a read-only `codeEdit` with invisible characters drawn as tokens. The default button is Cancel when any warning is present.

#### 5.3.3 SSH hosts panel and host details

```
+---------------- SSH hosts ----------------+   +------------- Host: build01 --------------+
| [ filter…            ]  [+ Add] [Import]  |   | Name     [build01        ] Tags [ci    ] |
| v ci                                      |   | Address  [build01.lan    ] Port [22  ]   |
|   > build01     odin@build01.lan   ✓ key  |   | User     [odin    ]                      |
|     build02     odin@build02.lan   ✓ key  |   | Key      [ed25519 laptop (Keys) v]       |
| v prod                                    |   | Jump     [none v]     Route [clear v]    |
|     web-1       admin@10.0.0.11    ⚠ new  |   | Profile  [SSH — default v]               |
|     db-1        admin@10.0.0.12    ✓ key  |   | Forwards  L 8080 → localhost:80   [+][-]  |
|                                           |   | Host key  SHA256:Yk3…9Qe  pinned 3 Oct    |
| [Connect] [Connect in split] [Broadcast…] |   | Scope: network to build01.lan:22 only     |
+-------------------------------------------+   |                [Delete] [Cancel] [Save]  |
                                                +------------------------------------------+
```

`treeView` grouped by tag. `⚠ new` means no host key is pinned yet.

#### 5.3.4 Sessions panel

```
+------------------------------- Sessions --------------------------------+
| Attached (3)                                                            |
|   zsh  ~/proj/pbsd        home·net            this window, tab 1        |
|   ssh  build01            ssh:build01         this window, tab 2        |
|   sh   ~/dl               ~/dl only·no net    this window, tab 3        |
| Detached (2)                                                            |
| > zsh  ~/proj/forge       home·net     2 h    make running   [Attach]   |
|   repl ForgeScript        ~/scratch·no net  1 d   idle        [Attach]  |
| Ended (1)                                                               |
|   zsh  ~/proj/pbsd        ended at restart    [View scrollback]         |
|                                             [End session…] [Attach all] |
+-------------------------------------------------------------------------+
```

#### 5.3.5 Block menu and block details

```
+-------------------------------+
| ▌ make test   0 · 4.2 s       |
|-------------------------------|
| Copy command      Ctrl+Shift+Y|
| Copy output       Ctrl+Shift+O|
| Copy both                     |
| Save output…                  |
| Re-run in this pane           |
| Fold                Ctrl+Shift+-|
| Find in this block            |
| Bookmark            Ctrl+Shift+M|
| Explain…  (local model)       |
+-------------------------------+
```

#### 5.3.6 Profile editor (Preferences → Profiles)

```
+---------------------------------- Preferences ----------------------------------+
| [General][Profiles][Colours][Keys][Scopes][SSH][Sessions][Advanced]             |
+------------------+--------------------------------------------------------------+
| > Default shell  | Name      [Default shell      ]   [x] Default profile        |
|   Restricted     | Command   (•) Login shell  ( ) Custom [/bin/sh          ]    |
|   Administrator  | Directory (•) Same as current pane ( ) Home ( ) [______]     |
|   ForgeScript    | Scope     [Full user shell v]   [Edit scope…]                |
|   SSH template   | Emulation [xterm v]   TERM [xterm-256color]                  |
|                  | Font      [Mono 12 v]  [ ] Ligatures   Cell [1.0] × [1.0]    |
|                  | Colours   Dark [Night v]  Light [Paper v]  Min contrast [4.5]|
|                  | Cursor    [Block v] [ ] Blink     Scrollback [64 MiB v]      |
|                  | Integration [x] Prompt marks [x] Working directory           |
|                  | Replies   [Safe default v]  Clipboard write [Ask v]          |
| [+] [Duplicate]  | Images    [Off v]     Bell [Visual v]                        |
| [Import][Export] | Preview:  $ ls --color   dir/  file.txt  link@               |
+------------------+--------------------------------------------------------------+
```

#### 5.3.7 Broadcast input

```
+--------------------------- Broadcast input ---------------------------+
| Send what I type to:                                                  |
|  ( ) All panes in this tab     ( ) All panes in this window           |
|  (•) Selected panes:  [x] build01  [x] build02  [x] web-1  [ ] db-1   |
| ⚠ web-1 is waiting for hidden input (password). It will be skipped.   |
|                                              [Cancel]  [Start]        |
+-----------------------------------------------------------------------+
```

While active, target panes have a 2 px `--warn` frame and the header shows "Broadcast to 3 — Esc Esc to stop".

#### 5.3.8 ForgeScript REPL tab

```
+----------------------------------------------------------------------------------------+
| ForgeScript — ~/scratch          [~/scratch · no net]       [:type][:dis][:mem][:reset] |
|----------------------------------------------------------------------------------------|
| > auto xs = std::vector<int>{1, 2, 3};                                                 |
| > xs.size()                                                                            |
| 3                                                                                      |
| > for (auto x : xs) {                                                                  |
| .     print(x * 2);                                                                    |
| . }                                                                                    |
| 2                                                                                      |
| 4                                                                                      |
| 6                                                                                      |
| > █                                                    history ↑↓ · Tab complete       |
+----------------------------------------------------------------------------------------+
```

The continuation prompt `. ` is the REPL's own (FORGE:docs/scripting.md). The line editor, history and completion are added by this app's REPL front end, in ForgeScript, because the REPL has none.

### 5.4 Menus

```
[=] menu / F10
├─ File
│   ├─ New tab                            Ctrl+Shift+T
│   ├─ New window                         (no default key; see note)
│   ├─ New tab with profile ▸
│   ├─ New scoped shell…                  Ctrl+Shift+N
│   ├─ New administrator shell…
│   ├─ New ForgeScript REPL               Ctrl+Shift+R
│   ├─ Open layout ▸ · Save layout…
│   ├─ Save scrollback…
│   ├─ Print…
│   ├─ Close pane                         Ctrl+Shift+X
│   ├─ Detach session                     Ctrl+Shift+B
│   └─ Quit                               (no default key; see note)
├─ Edit
│   ├─ Copy                               Ctrl+Shift+C
│   ├─ Copy as ▸ (HTML, ANSI)
│   ├─ Paste                              Ctrl+Shift+V
│   ├─ Paste selection                    Shift+Insert
│   ├─ Select all                         Ctrl+Shift+A
│   ├─ Keyboard selection mode            Ctrl+Shift+Space
│   ├─ Quick select                       Ctrl+Shift+U
│   ├─ Find                               Ctrl+Shift+F
│   ├─ Find next / previous               Ctrl+Shift+G / Ctrl+Shift+J
│   ├─ Copy last output                   Ctrl+Shift+O
│   ├─ Copy last command                  Ctrl+Shift+Y
│   ├─ Insert snippet…                    Ctrl+Shift+I
│   └─ Preferences                        Ctrl+,
├─ View
│   ├─ Split right                        Ctrl+Shift+D
│   ├─ Split down                         Ctrl+Shift+E
│   ├─ Zoom pane                          Ctrl+Shift+Enter
│   ├─ Equalise panes
│   ├─ Sessions panel                     (no default key; see note)
│   ├─ SSH hosts panel                    Ctrl+Shift+H
│   ├─ Command history panel
│   ├─ Pane headers · Status bar · Timestamps
│   ├─ Zoom in / out / reset              Ctrl++ / Ctrl+- / Ctrl+0
│   └─ Full screen                        F11
├─ Terminal
│   ├─ Previous / next prompt             Ctrl+Shift+Up / Ctrl+Shift+Down
│   ├─ Bookmark line                      Ctrl+Shift+M
│   ├─ Clear scrollback                   Ctrl+Shift+K
│   ├─ Reset terminal
│   ├─ Read-only                          Ctrl+Shift+L
│   ├─ Broadcast input…
│   ├─ Start / stop recording
│   ├─ Send signal ▸
│   ├─ Scope ▸ (Show details, Add folder…, Allow network…)
│   └─ Change profile ▸
├─ Tabs
│   ├─ Next / previous tab                Ctrl+Tab / Ctrl+Shift+Tab
│   ├─ Go to tab 1–9                      Alt+1 … Alt+9
│   ├─ Move tab left / right              Ctrl+Shift+PageUp / PageDown
│   ├─ Rename tab…
│   └─ Detach tab to window
└─ Help
    ├─ Terminal help                      F1
    ├─ Supported sequences
    ├─ Keyboard shortcuts
    └─ About Terminal
```

Note: in a terminal the reserved `Ctrl+letter` keys belong to the program in the pane. Every app shortcut therefore uses `Ctrl+Shift`. New window is `Ctrl+Shift+N` in other terminals; here that key is the scoped shell, so new window is in the palette and menu and may be bound by the user. Quit and the sessions panel have no default key for the same reason; the palette (`Ctrl+Shift+P`) reaches them.

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| `Ctrl+Shift+P` | Command palette (`Ctrl+K` goes to the shell) |
| `Alt+Arrow` | Move focus between panes |
| `Alt+Shift+Arrow` | Resize the focused pane |
| `Shift+PageUp/PageDown`, `Shift+Home/End` | Scroll |
| `Ctrl+click` | Open link |
| `Shift+drag` | Select while a program has mouse reporting on |
| `Alt+drag` | Block selection |
| Middle click | Paste primary selection (through paste protection) |
| `Esc Esc` | Stop broadcast |
| `F12` (global, set in SY01) | Drop-down window |

Context menu in a pane: Copy · Paste · Open link · Copy link · Search for selection · Split right · Split down · Scope details · Change profile ▸ · Read-only · Record · Close pane. On a tab: Rename · Colour ▸ · Duplicate · Detach to window · Detach session · Close · Close others.

Drag and drop: files dropped on a pane insert their shell-quoted paths (only paths inside the pane's scope; others are offered "Add this folder to the scope…"); text drops go through paste protection; tabs drag to reorder, to another window, to an edge to split, or out to make a window.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | No sessions: a start page with profiles, recent folders, detached sessions and SSH hosts. |
| Loading | "Starting…" line; reattach shows the stored screen at once, then live data. |
| Error | Launch failure banner (command not found, scope denied) with the exact reason and [Edit profile]. |
| Offline | SSH tabs show the client's own message; hosts panel marks unreachable hosts only when asked to test. |
| Locked | Read-only pane; session locked panes blank their content when the screen locks (option). |
| Exited | Banner with exit code, [Restart], [Close]; "close on exit" is per profile. |
| Narrow (< 640) | Tab bar scrolls; side panels become sheets; splits stay but pane headers collapse to chips. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Terminal content | terminal grid widget (`terminalGrid`, planned FUSE-WP-39; gap G-13) |
| Tabs | `tabBar` with tear-off (FUSE-WP-31) |
| Splits | nested `splitter` |
| Side panels | `dockWidget`, `treeView`, `modelListView` |
| Paste preview | `dialog`, `codeEdit` (read-only) |
| Preferences | `tabWidget`, `listView`, `propertyGrid`, `colorPicker`, `fontPicker` |
| Scope dialog | `dialog`, `listView`, `radioButton`, `checkBox`, `comboBox` |
| Recording player | terminal grid + `slider` |

Missing:

- G-13 (terminal grid) is the hard dependency.
- `SY03-GAP-1` Block decorations over the grid: rules, gutter hit areas, sticky header line and fold markers need hooks in the grid widget for per-row overlays.
- `SY03-GAP-2` Scroll-bar marks (coloured ticks on `scrollBar`).
- `SY03-GAP-3` Inline image cells in the grid (placement by cell rectangle, z-order under text).
- G-01 (layer-shell) for the drop-down window; G-03 for complex scripts.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Terminal app | ForgeScript | app host | Windows, tabs, split tree, profiles, scopes, panels, paste protection, blocks model |
| UI documents | FUSE UI | app host | `.fuseui` for window, panels, dialogs, preferences |
| Terminal core | C++23 (FUSE `fuse_ui_terminal`) | app host UI thread plus one parser thread per pane | Escape-sequence parser, screen model, scrollback, rendering. FUSE does not own the pty (FUSE-WP-39). |
| Session keeper | C++23 | one per user session, background, Capsicum | Owns pty master descriptors, child process groups, a bounded replay buffer per session; passes the pty descriptor to the app on attach |
| Shell launcher | C++23 | helper, short-lived | Builds a capability domain from a scope, opens the pty slave, sets the environment allow-list and limits, executes the command |
| Image decoder | C++23 | helper, Capsicum, no handles | Decodes Sixel and Kitty-protocol pixel data to bitmaps in shared memory |
| Recording writer and reader | ForgeScript + C++23 codec in helper | app host / helper | asciicast read and write; reading untrusted casts in the helper |
| REPL front end | ForgeScript | app host | Line editor, history and completion in front of the `forge script` REPL |
| SSH configuration reader | C++23 | helper, one read handle | Parses `~/.ssh/config` and known-hosts for display |

```
 +-------------------------- app host: Terminal ---------------------------+
 | ForgeScript: tabs, scopes, blocks, paste guard      FUSE UI + terminal  |
 |                                                     core (parser, grid) |
 +------+---------------------+-------------------+------------------------+
        | bus: create/attach  | pty master fd     | shm pixels
        v                     | (passed)          v
   session keeper ────────────+              image decoder
        | start(scope, command)
        v
   shell launcher ──► new capability domain ──► shell and its children
        (handles: scoped folders, optional sockets,  pty slave)
```

Data path: bytes from the pty go straight from the passed descriptor to the parser thread; they do not cross ForgeScript. ForgeScript sees events (title, directory, block marks, bell, link activation), not the byte stream.

### 6.2 Data model

| Entity | Fields | Store |
|---|---|---|
| `Profile` | `id: uuid`, `name`, `command: string[]`, `env: map<string,string>`, `cwd_mode: same\|home\|fixed`, `cwd`, `scope_id`, `emulation: vt100\|vt220\|vt420\|xterm`, `term: string`, `font`, `font_size: f32`, `ligatures: bool`, `scheme_dark`, `scheme_light`, `min_contrast: f32`, `cursor`, `scrollback_bytes: u64`, `reply_policy`, `osc52_write: ask\|allow\|deny`, `images: off\|sixel\|all`, `integration: bool`, `close_on_exit`, `keys_id` | `~/.config/org.pbsd.terminal/profiles.json` |
| `Scope` | `id`, `name`, `folders: {grant_ref, rights: r\|rw}[]`, `system_ro: bool`, `net: none\|hosts\|user`, `hosts: {host,port}[]`, `route`, `devices: string[]`, `home: user\|user_ro\|ephemeral`, `env_allow: string[]`, `limits: {mem_bytes, nproc, cpu_seconds, wall_seconds}`, `elevated: bool` | `~/.config/org.pbsd.terminal/scopes.json`; folder grants are references into the permission store |
| `Session` | `id: uuid`, `profile_id`, `scope_snapshot`, `pid`, `pgid`, `title`, `cwd`, `created_at`, `state: attached\|detached\|ended`, `cols`, `rows`, `exit_code` | session keeper (memory) + `$XDG_RUNTIME_DIR/org.pbsd.terminal/sessions/` |
| `Layout` | `name`, `windows: {tabs: {title, tree: SplitNode}[]}[]` where `SplitNode = {dir, ratio, a, b} \| {profile_id, cwd, command, session_id}` | `~/.config/org.pbsd.terminal/layouts.json` |
| `Block` | `session_id`, `seq: u64`, `prompt_row`, `command_text`, `output_start`, `output_end`, `exit_code: i32`, `started_at`, `duration_ms`, `cwd`, `folded: bool` | in memory per pane; persisted with scrollback |
| `Host` | `id`, `name`, `address`, `port`, `user`, `key_ref`, `jump_id`, `tags[]`, `profile_id`, `route`, `forwards: {kind: L\|R\|D, bind, target}[]`, `host_key_fp`, `pinned_at` | `~/.config/org.pbsd.terminal/hosts.json` |
| `ColourScheme` | `name`, `fg`, `bg`, `cursor`, `selection`, `ansi[16]`, `variant: dark\|light\|hc` | `~/.config/org.pbsd.terminal/schemes/*.json` |
| `HistoryEntry` | `at`, `command`, `cwd`, `exit_code`, `duration_ms`, `session_id`, `host` | `~/.local/share/org.pbsd.terminal/history.sqlite` (encrypted with a keyring key) |
| `Recording` | asciicast v2 header + events `[t, "o"\|"i"\|"r", data]` | `.cast` files where the user saves them |
| `ScrollbackStore` | encrypted pages of serialised lines, keyed by `session_id` | `~/.cache/org.pbsd.terminal/scrollback/` |

All JSON stores carry `"schema": n`. Scopes are saved by reference to grants; a scope whose grant was revoked shows as "needs folder" and will not start until fixed.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| VT/xterm escape sequences | yes | replies | FUSE `fuse_ui_terminal` (own code) | app host parser thread; bounded, T1 target in FUSE-WP-39 |
| Sixel, Kitty graphics payloads (PNG, raw RGBA) | yes | no | own code; PNG through the system image codec | image decoder helper |
| asciicast v2 | yes | yes | own code | helper for reading untrusted files |
| `~/.ssh/config`, `known_hosts` | yes | known-hosts edits only | own code | SSH configuration reader helper |
| Colour schemes: own JSON, `.itermcolors` (XML plist), base16 YAML | yes | own JSON only | own code (XML and YAML subsets) | helper |
| Profile bundle (`.pbterm`, JSON + signature) | yes | yes | own code, Ed25519 as V14 | helper |
| terminfo source for `pbsd-terminal` | no | shipped | text file compiled at build | — |
| SSH protocol | — | — | system `ssh` client inside the pane's scope | the scoped domain |
| Session keeper interface | yes | yes | schema compiler (G-08) | app host and keeper |

### 6.4 Integration

- **Intents handled**: `open` for folders ("terminal here"), `ssh:` URLs, `.cast`, `.pbterm`, `.fs` (REPL, load file). **Emitted**: `open` (links through `svc.intent`), `share`, `print`.
- **Services**: `svc.settings`, `svc.notify`, `svc.intent`, `svc.portal.file`, `svc.portal.print`, `svc.clipboard`, `svc.keyring`, `svc.audit`, `svc.power` (inhibit sleep while a marked job runs, optional), `svc.model` (explain, optional), `svc.a11y`.
- **Offers**: `org.pbsd.terminal.control` (F106) and the session keeper interface. DV01 embeds the same grid widget and may use the keeper; VM01, VM02 and VM04 embed the widget only.
- **Other apps**: SE04 (SSH agent and keys), AD05 (scope grants), SY02 (terminal here, run file), SY04 (process link), SH03 (authentication), ME05 (video export), AD04 (audit recordings), SE03 (route).
- **Backing repo facts (Verified)**: `forge script` with no file is a REPL; a value without `;` is printed; input continues with prompt `. ` while incomplete; `Ctrl-C` cancels with `CancelledError`; commands are `:type`, `:dis`, `:mem`, `:gc`, `:reset`, `:vm on|off`, `:help`, `:quit`; there is no line editing, history or tab completion; the REPL does not run the static checks and cannot carry a signature (FORGE:docs/scripting.md, "REPL" and the signing section). The terminal grid widget is planned, not built (FUSE_BUILD_PLAN.md FUSE-WP-39; PLATFORM.md G-13).
- **Proposed**: everything else in this section.

## 7. Security and privacy

Capabilities requested:

| Capability | Reason |
|---|---|
| `fs.app` | Profiles, scopes, layouts, history, scrollback cache |
| `fs.portal` | Folders for scopes; save scrollback and recordings |
| `fs.path:~:rw` | The "Full user shell" scope. Shown at install. Narrower scopes are subsets. |
| `fs.path:/:r` | Run system programs read-only |
| `proc.exec` | Start shells and commands (through the launcher) |
| `proc.helper:session-keeper`, `proc.helper:shell-launcher`, `proc.helper:image-decoder`, `proc.helper:config-reader` | Section 6.1 |
| `net.user`, `net.route:clear`, `net.route:vpn`, `net.route:tor` | Held only to pass on to shells whose scope allows network. The app's own engine opens no sockets. |
| `dev.usb:serial`, `dev.gpu.compute` | Passed to scopes that ask; prompt on first use |
| `bg.run` | Session keeper keeps detached sessions alive |
| `ai.local` | Explain panel, off by default |
| `svc.*` as in 6.4 | Services |

Not requested: `bg.autostart` (the keeper starts on first use), `dev.camera`, `dev.mic`, `dev.screen`, `dev.location`.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| The user's shell | Hostile program output (a `cat` of a crafted file, a remote server) | Escape sequences that make the terminal type text back (title report, DECRQSS echo, answerback), write the clipboard, or resize and spoof | Reply policy (F072) with unsafe replies off; OSC 52 gated; parser limits; FUSE gate `fuse_ui_term_no_unsafe_replies`. |
| The user | Pasted text | Hidden newline or bidirectional text runs a command | Paste preview (F073), marker stripping, source flag. |
| The user | Misleading links | OSC 8 text shows one host, target is another | Real target on hover; mismatch warning; no plain-click open. |
| The app host | Parser or image decoder bug | Memory corruption from output bytes | Parser is safe-subset C++23 with a T1 target; images decode in a helper with no handles. |
| Files outside the task | An untrusted script | Reads keys, calls home | Scoped shell: only chosen folders, no network, ephemeral home, clean environment. |
| Keys | A compromised remote host or shell | Uses the agent silently | Agent in SE04 with per-use confirmation option; no key files visible in scoped shells. |
| Secrets on screen | Recording, history, scrollback files | Passwords or tokens persisted | Echo-off pauses recording; history and scrollback encrypted; redaction on export; history can be off. |
| Other panes | Broadcast input | Text sent to an unintended pane, or a password prompt | Visible frames, target list, skip of echo-off panes. |
| Privilege | Administrator shell left open | Someone else types | Red chip, idle timeout, not kept detached, audited. |
| The session | Another app | Drives the terminal over the control interface | `svc.terminal.control` is a user-granted capability; "send text" calls go through paste protection. |

Stored: profiles, scopes, hosts (no secrets), encrypted history and scrollback. Leaves the machine: only what programs in network-allowed scopes send. The app sends nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `terminal.default_profile` | uuid | Default shell | Profile for new tabs |
| `terminal.new_tab_cwd` | enum(`same`,`home`) | `same` | Directory of new tabs |
| `terminal.confirm_close_running` | bool | true | Ask before closing a running job |
| `terminal.scrollback.default_bytes` | int | 67108864 | Scrollback cap per pane |
| `terminal.scrollback.spill_to_disk` | bool | false | Unlimited scrollback in the cache, encrypted |
| `terminal.paste.confirm_multiline` | bool | true | Preview multi-line pastes |
| `terminal.paste.confirm_bytes` | int | 4096 | Preview pastes larger than this |
| `terminal.paste.warn_patterns` | list<string> | `sudo `, `doas `, `\| sh`, `\| bash` | Warning patterns |
| `terminal.links.modifier` | enum(`ctrl`,`alt`) | `ctrl` | Key to open links |
| `terminal.replies.policy` | enum(`safe`,`xterm`,`none`) | `safe` | Which queries are answered |
| `terminal.osc52.write` | enum(`ask`,`allow`,`deny`) | `ask` | Clipboard write by programs |
| `terminal.osc52.read` | enum(`deny`,`ask`) | `deny` | Clipboard read by programs |
| `terminal.images` | enum(`off`,`sixel`,`all`) | `off` | Inline images |
| `terminal.bell` | enum(`none`,`visual`,`sound`,`both`) | `visual` | Bell style |
| `terminal.blocks.enabled` | bool | true | Command blocks when integration is present |
| `terminal.notify.long_command_s` | int | 30 | Notify when a longer command ends unfocused; 0 off |
| `terminal.history.enabled` | bool | true | Command history panel |
| `terminal.sessions.persist` | bool | true | Keep sessions when the window closes |
| `terminal.sessions.max_detached` | int | 16 | Limit |
| `terminal.sessions.idle_timeout_h` | int | 72 | End idle detached sessions; 0 never |
| `terminal.admin.idle_timeout_min` | int | 10 | Administrator shell lock |
| `terminal.dropdown.height_pct` | int 20–100 | 45 | Drop-down window height |
| `terminal.dropdown.hide_on_blur` | bool | true | Hide when focus leaves |
| `terminal.select.copy_on_select` | bool | false | Copy to primary selection only by default |
| `terminal.select.word_chars` | string | `-_./~:@%+` | Extra word characters |
| `terminal.ai.explain` | bool | false | Explain panel |
| `terminal.lock.blank_on_screen_lock` | bool | false | Hide content when the screen locks |

## 9. Accessibility and languages

- The grid exposes lines as text with a caret, a live region for new output (rate-limited, summarised when faster than speech), and commands as headings when blocks are on, so a screen-reader user can jump command to command.
- A "review mode" (`Ctrl+Shift+Space`) lets the user read scrollback by line, word and character without moving the program's cursor.
- Exit status is spoken ("failed, code 1"). The scope chip is in the pane's accessible name.
- Minimum-contrast setting and high-contrast schemes; bold is never the only cue.
- Bell can be visual only; flashes respect reduced motion (a border, not a full flash).
- Input methods compose at the cursor (pre-edit in the grid).
- Right-to-left and shaped text is a Phase 3 option because cell-grid programs assume left-to-right cells.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to prompt | 150 ms to first frame; shell prompt within 250 ms (shell start excluded) |
| New tab | 50 ms |
| Parse throughput | at least 100 MiB/s plain text, 30 MiB/s mixed sequences (FUSE-WP-39 budget) |
| Full repaint 200×60 | 2 ms GPU, 6 ms CPU (FUSE-WP-39 budget) |
| Key to glyph latency | under 10 ms at 60 Hz |
| Memory | 40 MiB plus scrollback cap per pane; one empty pane under 6 MiB |
| Idle | 0 frames and 0 wake-ups with a quiet, unfocused window; cursor blink stops unfocused |
| Search | 64 MiB of scrollback in under 150 ms |
| Reattach | Screen shown in under 100 ms |
| Flood | `yes` or a 1 GB `cat` keeps the UI at 60 frames per second and `Ctrl+C` responsive |

## 11. Testing and proof

Test plan (targets; nothing has run):

- Unit: scope narrowing, paste analysis, block state machine, title templates, layout serialisation, host import.
- Conformance: scripted escape-sequence streams against expected screens (shared with FUSE-WP-39 gates); `vttest`-style manual checklist; application matrix: a shell, a full-screen editor, a pager, `tmux`, `top`.
- Differential: same streams through xterm on a reference machine, compared as screen dumps.
- Fuzz: escape parser, image payloads, asciicast, SSH config, colour-scheme importers, profile bundles.
- Security tests: a corpus of known terminal escape attacks (title-report injection, OSC 52 theft, paste-end injection, link spoofing) must produce no input to the pty and no clipboard change.
- Scope tests: inside each preset, attempts to open files, sockets and devices outside the scope must fail; run on HardenedBSD (G-06).
- Scripted UI: the 14 top tasks through the a11y tree.
- Soak: 24 h output flood with bounded memory; keeper survives app kill.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Escape-sequence parser (FUSE) | Cursor stays in the grid, no out-of-range write, bounded parameters | T1 | PRISM job named in FUSE-WP-39 | — |
| Screen operations (FUSE) | No runtime errors | T2 | PRISM, differential model | Size of the control-function set |
| Reply policy | With policy `safe`, no input sequence makes the terminal write attacker-chosen bytes to the pty | T1 | Proof over the reply table (finite) plus the attack corpus | — |
| Scope builder | The launched domain's handle set is a subset of the scope, which is a subset of the app's grants | T1 | PRISM proof of the narrowing function; kernel enforcement assumed | Kernel is outside the proof |
| Paste analyser | Detects every newline, C0/C1 control and bidirectional control in the input | T1 | Small pure function, proof | — |
| Session keeper | No descriptor leak across sessions; bounded replay buffer | T2 | PRISM plus sanitisers | Concurrency model for T1 |
| Image decoder | No runtime errors, bounded output size | T2 | PRISM plus fuzz | — |
| ForgeScript app logic | Language safety | T3 | Tests | Interpreter not proved |
| Block detection | Best effort | T4 | Tests | Depends on shell behaviour |

## 12. Build plan

Phase 1, usable core:

| ID | Work item | Depends on |
|---|---|---|
| SY03-W01 | Shell launcher: pty, environment allow-list, full-user scope | G-06, G-07 |
| SY03-W02 | Window with tabs and one pane on the terminal grid widget | G-13 (FUSE-WP-39), G-02 |
| SY03-W03 | Splits, pane navigation, zoom, close protection | SY03-W02 |
| SY03-W04 | Profiles, colour schemes, fonts, key bindings, preferences | SY03-W02 |
| SY03-W05 | Scrollback, search, selection, copy, links with safety rules | SY03-W02, SY03-GAP-2 |
| SY03-W06 | Paste protection and reply policy; clipboard sequence gate | SY03-W02 |
| SY03-W07 | Scope model, scoped launch, new-scoped-shell dialog, scope chip, audit | SY03-W01, AD05, SH04 |
| SY03-W08 | Shell integration snippets: prompt marks, working directory, prompt jumping | SY03-W05 |
| SY03-W09 | Command-line launcher, intents, "terminal here" | SY02 |
| SY03-W10 | terminfo entry and conformance test set | SY03-W02 |

Phase 2, full feature:

| ID | Work item | Depends on |
|---|---|---|
| SY03-W11 | Session keeper, detach and reattach, sessions panel, crash recovery | SY03-W01, G-09 |
| SY03-W12 | Command blocks, block actions, sticky prompt, history panel, long-command notice | SY03-W08, SY03-GAP-1 |
| SY03-W13 | SSH manager: hosts, connect in a host-only scope, host keys view, agent use | SE04, SE02 |
| SY03-W14 | ForgeScript REPL profile and line-editor front end | FORGE REPL, G-05 |
| SY03-W15 | Recording, replay | SY03-W02 |
| SY03-W16 | Broadcast input; secret-input awareness | SY03-W03 |
| SY03-W17 | Drop-down window | G-01 (FUSE-WP-10) |
| SY03-W18 | Inline images | SY03-GAP-3 |
| SY03-W19 | Administrator shell, disposable shell, run-file hand-off, widen on request | SH03, SY02 |
| SY03-W20 | Layouts, snippets, quick select, automatic profile switching, control interface | SY03-W04 |
| SY03-W21 | Extended keyboard protocol | SY03-W02 |

Phase 3, advanced:

| ID | Work item | Depends on |
|---|---|---|
| SY03-W22 | Triggers, minimap, tab overview, bookmarks list | SY03-W05 |
| SY03-W23 | Port forwards view, file drop upload, serial console | SY03-W13, VM05 |
| SY03-W24 | Export with redaction, audit recording policy | AD04, ME05 |
| SY03-W25 | Explain panel on the local model | G-15, AI02 |
| SY03-W26 | tmux control mode, session mirroring | SY03-W11 |
| SY03-W27 | Complex scripts and right-to-left option | G-03 |
| SY03-W28 | Jail and VM shell profiles | VM01, VM02 |

## 13. Open questions and risks

- G-13 does not exist yet. Until FUSE-WP-39 lands there is nothing to draw a terminal with.
- A useful shell needs many programs. "System programs read-only" exposes the whole base system to a scoped shell; whether that is narrow enough, or whether scopes need a curated program set, is open.
- Programs that expect ambient authority (`ssh` reading `~/.ssh`, `git` reading its config) will fail in narrow scopes. Presets must be tested against common tools.
- Session persistence across logout needs a policy decision: a kept shell is a long-lived domain with handles.
- The REPL front end depends on the REPL's prompt strings staying stable; a machine-readable REPL mode in FORGE would be safer (not present in the document read).
- Capsicum-based scoping of arbitrary shell children is harder than scoping one app: most Unix programs open paths by name. Whether the kernel's handle model lets unmodified programs run inside a scope is Unverified.
- Kitty graphics and Sixel add attack surface for modest value; they stay off by default.
- Reserved `Ctrl+letter` keys cannot be used here; this differs from DESIGN_SYSTEM.md section 6 and needs a stated exception.
