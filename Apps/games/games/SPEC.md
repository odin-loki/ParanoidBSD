<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Games

By Odin Loch

| | |
|---|---|
| ID | `GM01` (from CATALOGUE.md) |
| Bundle | `org.pbsd.games` |
| Category | games |
| Priority | P3 |
| Backing repos | FUSE: game UI library `fuse::ui::game` (`FUSE:docs/ui/README.md` "Game UI (UI7)", headers in `FUSE:Source/FUSE/UI/Game/include/fuse/ui/game/`), engine libraries listed in `FUSE:docs/modules.md` (Verified as listed; not built or run here) |
| Status | Planned. Nothing built. |

## 1. Purpose

Games is a hub for everything playable on the machine and a pack of small, well-made games: three solitaires, mines, sudoku, chess, a sliding-tiles game, tile-matching mahjong, and one 3D showcase. The games show what FUSE can do, give the system something pleasant out of the box, and prove that games can run under the same default-deny rules as every other app: no network, no accounts, no tracking. It replaces the KDE games collection from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- The hub: a library of installed games (this pack, CR04 Studio exports, VM03 compatibility titles), statistics, achievements, controller set-up, save management.
- The pack: Klondike, Spider, FreeCell, Mines, Sudoku, Chess, Tiles (2048-style), Mahjong (tile matching), and a 3D showcase.
- Shared game services inside the bundle: save games, statistics, achievements, replays, daily deals from a local seed, input mapping, sound.
- Local multiplayer on one machine (hot-seat, shared keyboard, several gamepads).
- Gamepad support for the hub and every game.

Out of scope:

| Item | Owner |
|---|---|
| Making games | CR04 Studio |
| Running Windows, Linux and Android games; prefixes and their sandboxes | VM03 Compatibility (the hub lists and launches them through VM03) |
| Installing and updating games | SY05 Software |
| Online play, matchmaking, leaderboards, cloud saves, game stores | not planned (no network by default; see 7) |
| Gamepad pairing, USB authorisation, Bluetooth | VM05 Devices, SY01 Settings |
| Screen recording and screenshots of games | ME05 Screen Capture |
| Voice chat | CO05 Messenger |
| Emulators for consoles | not planned |
| Chess over the network or against internet engines | not planned |

## 3. Users and top tasks

Personas:

- **Casual player**: five minutes of solitaire or mines; expects instant start and resume.
- **Puzzle player**: sudoku and mahjong with hints, notes and fair, solvable deals.
- **Chess player**: plays the engine at a chosen strength, reviews games, opens PGN files.
- **Child or family**: two players at one machine; simple controls; no way to reach the network.
- **Controller user**: couch use, no keyboard.
- **Maker**: built a game in CR04 and wants it in the library.

| # | Task | Target steps |
|---|---|---|
| T1 | Continue the last game played | 1 (hub "Continue") |
| T2 | Start a new Klondike deal | 2 |
| T3 | Undo a move | 1 (`Ctrl+Z`) |
| T4 | Get a hint | 1 (`H`) |
| T5 | Start a sudoku at a chosen difficulty | 2 |
| T6 | Play chess against the engine at a chosen level | 3 |
| T7 | Open a PGN file and step through it | 1 + arrow keys |
| T8 | Start a two-player chess game on one machine | 3 |
| T9 | See statistics for a game | 2 |
| T10 | Launch a game made in Studio | 2 |
| T11 | Navigate and play with a gamepad only | 0 extra steps |
| T12 | Play today's daily deal | 2 |
| T13 | Pause and hide a game quickly | 1 (`Esc`) |

## 4. Features

### 4.1 Hub and library

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| GM01-F001 | Library grid | Cards for every game: art, title, source badge (Pack, Studio, Compatibility, Installed app), last played, play time. Grid or list. | M | 1 |
| GM01-F002 | Continue | The hub's first card resumes the most recent unfinished game exactly where it was left. | M | 1 |
| GM01-F003 | Sources | Pack games (in this bundle); other installed game bundles (manifest category `games`); CR04 Studio exports registered by the user; VM03 titles listed through VM03's bus interface. | M | 2 |
| GM01-F004 | Launch contained | Pack games run inside this app. Other bundles are launched through `svc.intent` and run under their own manifests. VM03 titles are launched by VM03 in their own sandboxes. The hub never executes anything itself. | M | 2 |
| GM01-F005 | Game page | Description, how to play, controls, statistics summary, achievements, save slots, capabilities the game holds (for non-pack games, read from their manifest) and a link to AD05. | M | 1 |
| GM01-F006 | Search, sort, filter | By name, genre (cards, puzzle, board, arcade, 3D), source, players, controller support, last played, most played. | S | 2 |
| GM01-F007 | Favourites and hiding | Star games; hide ones the user does not want to see. | S | 2 |
| GM01-F008 | Collections | User-made groups ("Kids", "Quick"). | C | 3 |
| GM01-F009 | Studio exports | "Add a Studio game" registers an exported game bundle signed with the user's own key; it shows an "unreviewed, made on this computer" badge and its capability list. | S | 3 |
| GM01-F010 | Network badge on cards | Any game whose manifest holds a `net.*` capability other than `net.none` shows the `--net` mark on its card before launch. Pack games never do. | M | 2 |
| GM01-F011 | Play time | Per game and per day, local only, with an option to turn tracking off and a clear button. | S | 2 |
| GM01-F012 | Play limits | Optional daily time limit per user account with a calm reminder; an admin-set limit for child accounts hands off to SY01's user settings. | C | 3 |
| GM01-F013 | Big-screen mode | A full-screen hub layout with large cards and controller navigation; starts automatically when the hub is opened from a gamepad. | S | 2 |
| GM01-F014 | Launcher integration | Each pack game also appears as its own launcher entry in SH01 and opens straight into that game (`games:<id>` address). | M | 1 |
| GM01-F015 | Hub statistics page | Totals across games: games played, time, streaks, achievements earned. | S | 2 |
| GM01-F016 | Uninstall and data | For non-pack games: "Remove…" hands off to SY05. For any game: "Delete saves and statistics" with a confirmation. | S | 2 |

### 4.2 Shared game services

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| GM01-F020 | Autosave | Every move is saved. Closing the window, a crash or a power cut loses nothing. | M | 1 |
| GM01-F021 | Save slots | Chess, sudoku, mahjong and the 3D showcase have named slots; card games keep the current deal plus up to 5 parked deals. | S | 2 |
| GM01-F022 | Unlimited undo and redo | For every turn-based game; undo count is shown and recorded in statistics (a game won "without undo" is noted). | M | 1 |
| GM01-F023 | Hints | Each game has a hint function that explains itself in a line ("Move the red 6 onto the black 7"). Hint use is recorded. | M | 1 |
| GM01-F024 | Statistics | Per game and per variant: played, won, win rate, current and best streak, best time, best score, average time, moves, hints and undos used. Charts over time. | M | 1 |
| GM01-F025 | Achievements, local only | A fixed list per game (for example "Win Spider with four suits"). Earned on this machine, stored in the app's data, never sent anywhere. Progress shown for counted ones. | S | 2 |
| GM01-F026 | Daily deal | A deal or puzzle per day per game generated from the date with a fixed, published algorithm. No server. The same date gives the same deal on every machine. Past days can be played from a calendar. | S | 2 |
| GM01-F027 | Seeds | Every deal and puzzle has a seed number shown in the game; "New game from seed…" replays it; seeds can be copied and shared as text. | M | 1 |
| GM01-F028 | Replays | Finished games can be replayed move by move and exported as a text move list. | C | 3 |
| GM01-F029 | Timer and pause | Optional timer; `Esc` or losing focus pauses and hides the board (so a pause cannot be used to think) for timed modes. | M | 1 |
| GM01-F030 | Sounds | Small set of sound effects through the game UI sound hook; master, effects and music sliders; follows system mute. Off in do-not-disturb if set. | S | 2 |
| GM01-F031 | Themes | Card backs and faces (standard, large-index, four-colour), tile sets, board and piece sets, table colours. All bundled; user themes from a folder could come later. | S | 2 |
| GM01-F032 | Animations | Dealing, moving, winning. Speed setting; all cut to instant under reduced motion. Never longer than 200 ms per move (the win animation can be skipped). | M | 1 |
| GM01-F033 | Export and import of data | All saves, statistics and achievements as one file, for backup or another machine. | S | 2 |
| GM01-F034 | Reset | Reset statistics per game or all; undoable for 10 s. | M | 1 |
| GM01-F035 | Rules pages | Each game's rules and variants are help pages (`help/*.md`), opened in UT05 by `F1` or shown in the game page. | M | 1 |
| GM01-F036 | Boss key | `Esc` pauses; `Ctrl+M` minimises and mutes at once. | C | 2 |

### 4.3 Input and controllers

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| GM01-F040 | Keyboard complete | Every game is fully playable from the keyboard: arrow-key cursor, `Enter`/`Space` to pick and place, letter shortcuts. | M | 1 |
| GM01-F041 | Pointer and touch | Click-click and drag; double click for the obvious move; touch targets at least 44 px in touch density; long press as right click. | M | 1 |
| GM01-F042 | Gamepad navigation | Hub and menus use FUSE `GamepadNav` (d-pad and stick mapped to spatial navigation with a 0.4 s delay and 0.12 s repeat, A to Enter, B to Escape: `FUSE:docs/ui/README.md`). | M | 2 |
| GM01-F043 | Gamepad in games | A cursor moves between piles, cells or tiles spatially; buttons: A pick/place, B cancel, X hint, Y undo, shoulder buttons redo and cycle, Start pause, Select menu. | M | 2 |
| GM01-F044 | Button prompts | On-screen prompts switch between keyboard, pointer and controller glyphs as the user changes device (FUSE `GameUi::prompt` with `noteDevice`). | S | 2 |
| GM01-F045 | Input arbitration | FUSE `InputArbiter` decides whether the UI or the game receives each device's input, so an open menu takes everything and a text field takes the keyboard. | M | 2 |
| GM01-F046 | Remapping | Per-game key and button bindings, shown and changed in one screen; conflicts are flagged; reset to defaults. | S | 2 |
| GM01-F047 | Several controllers | Up to 4 gamepads for local multiplayer; each player claims a device by pressing a button. | S | 3 |
| GM01-F048 | Device access | Gamepads arrive through the platform input path for the focused window. No raw USB access is requested. Rumble is optional and off by default. | M | 2 |
| GM01-F049 | One-switch and slow modes | A scanning mode that steps through legal moves; a no-timer mode everywhere. | S | 3 |

### 4.4 Solitaire (Klondike, Spider, FreeCell)

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| GM01-F050 | Klondike | Draw 1 or draw 3; unlimited or limited (1 or 3) passes; standard or Vegas scoring; timed or relaxed. | M | 1 |
| GM01-F051 | Spider | 1, 2 or 4 suits; deal only when no column is empty (rule can be relaxed); score = 500 − moves + 100 per completed suit. | M | 1 |
| GM01-F052 | FreeCell | 4 free cells (variant: 0–3 for a harder game); numbered deals compatible with the classic 1–32000 numbering (the generator is the well-known linear congruential deal; stated as compatible only after the differential test passes). | M | 1 |
| GM01-F053 | Winnable deals | Option "only deals that can be won": a solver checks the deal before it is shown (FreeCell and Klondike; Spider with a time cap, otherwise marked "not checked"). | S | 2 |
| GM01-F054 | Auto-move | Safe cards go to the foundations automatically (off, safe only, all). Double click or `Enter` sends a card to its best place. | M | 1 |
| GM01-F055 | Legal-move highlighting | On pick-up, valid targets are outlined; an option hides this for purists. | M | 1 |
| GM01-F056 | Super-moves | Multi-card moves in FreeCell are done in one gesture when the free cells and empty columns allow it; the count allowed is shown. | M | 1 |
| GM01-F057 | No-moves detection | When no move changes the position the game says so and offers Undo, New deal or Restart. | M | 1 |
| GM01-F058 | Auto-finish | When the win is forced the game offers to finish. | S | 1 |
| GM01-F059 | Hint quality | Hints come from the solver when one exists within 200 ms, else from a rule list; the hint never lies about solvability. | S | 2 |
| GM01-F060 | Restart same deal | Without counting as a new game in the streak unless the user chose so. | M | 1 |
| GM01-F061 | Left-handed layout | Mirror stock and foundations. | S | 2 |
| GM01-F062 | Card readability | Large-index faces, four-colour deck, high-contrast deck; cards scale with the window; columns squeeze before they scroll. | M | 1 |
| GM01-F063 | More patience games | Could: Golf, Pyramid, TriPeaks, Yukon, Canfield, as rule files on the same card engine. | C | 3 |

### 4.5 Mines

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| GM01-F070 | Boards | Beginner 9×9/10, Intermediate 16×16/40, Expert 30×16/99, custom up to 60×40 with any density under 80%. | M | 1 |
| GM01-F071 | Safe first click | The first reveal is never a mine and opens an area. | M | 1 |
| GM01-F072 | No-guess mode | Boards are generated so that logic alone solves them from the first click; a solver verifies each one. | S | 2 |
| GM01-F073 | Chording | Reveal neighbours of a satisfied number with middle click, both buttons, or `Space`. | M | 1 |
| GM01-F074 | Flags and question marks | Right click, `F`, or long press; question marks optional. | M | 1 |
| GM01-F075 | Hex and triangle grids | Could: other tilings on the same engine. | C | 3 |
| GM01-F076 | Timer and metrics | Time, 3BV, clicks, efficiency, for players who care; hidden in relaxed mode. | S | 2 |
| GM01-F077 | Hint | Marks one cell that is provably safe, or says "a guess is needed here" with the least risky cell and its probability. | S | 2 |
| GM01-F078 | Non-colour numbers | Numbers differ by shape of digit only (they are digits); the palette is colour-blind safe; mines and flags have distinct shapes. | M | 1 |

### 4.6 Sudoku

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| GM01-F080 | Generator with unique solutions | Every puzzle has exactly one solution, checked by a complete solver at generation time. | M | 1 |
| GM01-F081 | Difficulty by technique | Easy, Medium, Hard, Expert, graded by the hardest human technique a logical solver needs (singles, pairs and triples, pointing, box-line, X-wing, swordfish, XY-wing, chains), not by clue count. | M | 2 |
| GM01-F082 | Entry modes | Cell-first or digit-first; keyboard digits; a number pad for pointer and touch. | M | 1 |
| GM01-F083 | Pencil marks | Corner and centre notes; auto-notes (fill all candidates); auto-remove notes when a digit is placed (option). | M | 1 |
| GM01-F084 | Mistake handling | Three modes: none (pure), show conflicts (same digit in a unit), check against the solution (limited lives option). | M | 1 |
| GM01-F085 | Explained hints | The hint names the technique and highlights the cells and candidates involved ("Hidden pair 3 and 7 in row 4"), in steps: where to look, what technique, the result. | S | 2 |
| GM01-F086 | Highlighting | Same digit, row/column/box of the selection, remaining count per digit. | M | 1 |
| GM01-F087 | Enter your own | Type or paste a puzzle (81-character string); the app checks uniqueness and grades it. | S | 2 |
| GM01-F088 | Variants | 6×6 for beginners; could: killer, diagonal, jigsaw regions on the same solver. | C | 3 |
| GM01-F089 | Print puzzles | One, two or four per page through `svc.portal.print`, with or without solutions. | C | 3 |
| GM01-F090 | Colour marks | Cell colouring for advanced solving, with patterns in high contrast. | C | 3 |

### 4.7 Chess

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| GM01-F100 | Rules complete | All moves, castling, en passant, promotion (choice dialog; under-promotion allowed), check, mate, stalemate, threefold and fivefold repetition, 50- and 75-move rules, insufficient material. | M | 1 |
| GM01-F101 | Engine in a helper | An own chess engine runs in a native helper process with no file, network or device handles. It speaks UCI over a pipe. | M | 2 |
| GM01-F102 | Strength levels | 10 levels from beginner to full strength, by limiting depth, adding evaluation noise and blunder probability at low levels; an approximate rating label per level, marked approximate. | M | 2 |
| GM01-F103 | Time controls | None, fixed time per move, sudden death, increment; common presets (1+0 to 30+0, 3+2, 10+5, 15+10). Flag fall is detected. | S | 2 |
| GM01-F104 | Two players, one machine | Hot-seat with optional board flip per move; both clocks shown. | M | 1 |
| GM01-F105 | Move input | Drag, click-click, keyboard (type `e4`, `Nf3`, or move a cursor), gamepad cursor. Legal targets shown on pick-up (option). Premove off (local play). | M | 1 |
| GM01-F106 | Move list | Standard algebraic notation with figurines option; click to jump; variations as a tree. | M | 1 |
| GM01-F107 | PGN | Open, save, copy and paste PGN: tags, comments, NAGs, variations, several games per file with a game list. FEN copy, paste and position set-up board. | M | 2 |
| GM01-F108 | Analysis | Engine evaluation bar, best line, multi-line (1–5), depth and nodes shown; on demand, never running in the background unasked. | S | 2 |
| GM01-F109 | Game review | After a game: an evaluation graph and marks for mistakes and blunders with the better move; all computed locally. | S | 3 |
| GM01-F110 | Take back and hints | Take back in casual games; a hint shows the engine's suggestion at the current level. Both are off in "rated-style" casual mode. | M | 1 |
| GM01-F111 | Opening names | A small bundled table of opening names (ECO codes) shown as moves are played. | C | 3 |
| GM01-F112 | Opening book and tablebases | Could: a small book for variety; endgame tablebases are large and are not bundled (3–4 piece could be). | C | 3 |
| GM01-F113 | Puzzles | A bundled set of mate-in-n and tactics positions (from own generation or a licence-compatible set; licence recorded). | C | 3 |
| GM01-F114 | Chess960 | Start positions 0–959, castling rules handled. | C | 3 |
| GM01-F115 | External engines | Not supported: no `proc.exec`. Stated in help. A future signed engine bundle could plug in through the same pipe protocol. | M | 2 |
| GM01-F116 | Board and pieces | 2D sets (classic, high contrast, large), coordinates on or off, last-move and check highlights with shape as well as colour, board flip. | M | 1 |
| GM01-F117 | Spoken and typed play | Moves announced as text for screen readers ("knight to f3, check"); typed input accepted; board readable square by square. | M | 2 |

### 4.8 Tiles and Mahjong

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| GM01-F120 | Tiles (2048-style) | 4×4 default; 3×3, 5×5, 6×6; slide with arrows, swipe or d-pad; merges once per move; new tile 2 (90%) or 4 (10%). | M | 1 |
| GM01-F121 | Tiles: undo | Undo limited to 1 in classic mode, unlimited in relaxed mode; statistics kept apart. | M | 1 |
| GM01-F122 | Tiles: continue after target | Reaching the target (2048 by default; selectable) offers "Keep going". | M | 1 |
| GM01-F123 | Tiles: readability | Tile values are numbers with size scaling; colours are a single-hue ramp with sufficient contrast; no meaning in colour alone. | M | 1 |
| GM01-F124 | Tiles: hint | A simple expectimax search (depth-limited, in ForgeScript or the puzzle helper) suggests a direction. | C | 3 |
| GM01-F125 | Mahjong layouts | Turtle (classic 144 tiles) plus at least 11 more layouts stored as data files; a layout picker with previews. | M | 2 |
| GM01-F126 | Mahjong: solvable deals | Tiles are placed by playing the game in reverse so every deal has at least one solution; option for fully random deals. | M | 2 |
| GM01-F127 | Mahjong: matching rules | Identical tiles match; any season matches any season, any flower any flower; only free tiles (no tile on top, one side open) can be picked. | M | 2 |
| GM01-F128 | Mahjong: aids | Highlight free tiles, show matching tiles for the selected one, remaining pairs count, shuffle remaining tiles (counts in statistics), hint, undo. | M | 2 |
| GM01-F129 | Mahjong: tile sets | Traditional, large-symbol, and a numbers-and-letters set for players who cannot tell the traditional faces apart. Bundled art must be own or licence-compatible. | M | 2 |
| GM01-F130 | Mahjong: depth cues | Layer shown by offset, shadow and an optional layer tint with a number badge; 2D by default, optional 3D tilt view. | S | 2 |
| GM01-F131 | Mahjong: no-moves handling | Detects a dead position and offers Shuffle, Undo or New. | M | 2 |

### 4.9 3D showcase

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| GM01-F140 | A small complete 3D game | A marble-rolling puzzle: tilt a board to roll a ball through 20 short levels with hazards and collectables. Chosen because it shows rendering, physics, audio and HUD with little content. | S | 3 |
| GM01-F141 | Uses the engine as documented | Physics facade (`fuse_physics`), spatial audio (`fuse_audio`), particles (`fuse_vfx`), assets (`fuse_asset`), entity world (`fuse_ecs`), all listed in `FUSE:docs/modules.md`; HUD from `fuse::ui::game` (`hudBar`, markers, `floatText`, `LoadingScreen`). | S | 3 |
| GM01-F142 | In-world UI | One level uses a `UiSurface` panel placed in the world (`surfaceRaycast`, `surfacePointer`) as a control console, to show that feature. | C | 3 |
| GM01-F143 | Graphics settings | Resolution scale, quality preset (low, medium, high), frame cap (30, 60, display), vertical sync, field of view. A CPU-renderer fallback at low resolution if no GPU path is available, or a clear "needs a GPU" message. | S | 3 |
| GM01-F144 | Controls | Keyboard, pointer, gamepad stick; sensitivity and inversion; a one-hand layout. | S | 3 |
| GM01-F145 | Level times and medals | Best time per level, three medal thresholds, local only. | S | 3 |
| GM01-F146 | Made in Studio | The showcase project opens in CR04 Studio as an example; the shipped game is its export. | C | 3 |
| GM01-F147 | Comfort | No flashing; camera shake off under reduced motion; subtitles for any spoken line through the HUD subtitle layer. | M | 3 |

### 4.10 Local multiplayer, safety, settings

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| GM01-F150 | Hot-seat | Chess; Mines race (two boards side by side, same seed); Tiles race (same seed, split screen). | S | 3 |
| GM01-F151 | Player profiles | Local names and colours for players at one machine; statistics per profile; no passwords, no accounts. | S | 3 |
| GM01-F152 | No network | The bundle declares `net.none`. There is no code path that opens a socket. | M | 1 |
| GM01-F153 | No purchases, no advertising | Stated in the game page and enforced by having no network. | M | 1 |
| GM01-F154 | Power awareness | Turn-based games draw nothing while idle; the 3D showcase caps frame rate on battery and pauses when not focused. | M | 1 |
| GM01-F155 | Do-not-disturb | Optional: starting a full-screen game asks SH02 for do-not-disturb with a lease. Off by default. | C | 3 |
| GM01-F156 | Preferences | Section 8, shared and per game. | M | 1 |
| GM01-F157 | Corrupt save recovery | A save that fails its checksum or schema check is set aside and the previous autosave generation is loaded; the user is told. | M | 1 |
| GM01-F158 | Opening files | `.pgn` and `.fen` open in Chess; a sudoku text file opens in Sudoku; all through portal handles. | S | 2 |

## 5. GUI

### 5.1 Window map

```
Games
├─ Hub window
│  ├─ Library (grid | list | big-screen)
│  ├─ Game page (About | Statistics | Achievements | Saves | Controls)
│  ├─ Statistics overview
│  ├─ Daily deals calendar
│  └─ Controllers and input
├─ Game window (one per running pack game; same process)
│  ├─ Board (game canvas)
│  ├─ HUD (score, time, moves; FUSE game UI layer)
│  ├─ Side panel (chess move list and analysis; sudoku number pad; optional)
│  ├─ Pause menu
│  ├─ New game dialog (per game)
│  ├─ Win / lose sheet
│  └─ Promotion, set-up position, PGN game list (chess)
├─ Add Studio game dialog
├─ Remap controls dialog
├─ Import / export data dialog
└─ Preferences (General, Appearance, Sound, Input, per game)
```

### 5.2 Main window

The hub at 1200×760, library grid.

```
+------------------------------------------------------------------------------------------+
| [=] Games                     [Search games Ctrl+F]                           [_][o][x] |
+-----------+------------------------------------------------------------------------------+
| Library(1)| Continue                                                                (2) |
| Favourites| +----------------------------------------------+                             |
| Daily deals| |  Spider · 2 suits · 14 min in · 61 moves     |   [Continue  Enter]        |
| Statistics| +----------------------------------------------+                             |
| Achievements                                                                             |
| Controllers| Pack                                                   [Grid|List] Sort [v](3)|
| ─────     | +----------+ +----------+ +----------+ +----------+ +----------+             |
| SOURCES   | | Klondike | | Spider   | | FreeCell | | Mines    | | Sudoku   |             |
| Pack    9 | |  ♠       | |  ♠♠      | |  ♣       | |  ▣       | |  9       |         (4) |
| Studio  2 | | 62% won  | | 31% won  | | 88% won  | | best 74s | | 12 solved|             |
| Compat. 3 | +----------+ +----------+ +----------+ +----------+ +----------+             |
| Apps    1 | +----------+ +----------+ +----------+ +----------+                          |
|           | | Chess    | | Tiles    | | Mahjong  | | Marble 3D|                          |
|           | |  ♞       | |  2048    | |  [中]    | |  ●       |                          |
|           | | level 4  | | best 4096| | 7 layouts| | 6 / 20   |                          |
|           | +----------+ +----------+ +----------+ +----------+                          |
|           | Made in Studio                                                               |
|           | +----------+ +----------+                                                    |
|           | | My Maze  | | Pong 2   |   unreviewed, made on this computer           (5) |
|           | +----------+ +----------+                                                    |
+-----------+------------------------------------------------------------------------------+
| 14 games · played 3 h 12 min this week                    (6)[✓ No network]   pad: 1     |
+------------------------------------------------------------------------------------------+
```

1. Sidebar (`treeView`): sections and sources with counts.
2. Continue card (`panel` with one primary `button`). Absent when nothing is in progress.
3. View and sort controls (`tabBar` segment, `comboBox`).
4. Library (`gridView`; card = `image` + `label`s). One stat line per card.
5. Source note for Studio exports, as a trust badge in words (`--warn` unreviewed). Compatibility titles show "runs in Compatibility"; any game with network capability shows the `--net` mark.
6. Status bar: trust badge "No network" for the hub itself, and connected gamepad count.

States: empty (cannot be: pack games exist; a filtered view can be: "No games match"), loading (cards for other sources fill in when VM03 and the bundle registry answer), error (a source is unavailable: its section shows "Compatibility is not installed" or "not answering"), narrow (sidebar overlay, two columns of cards), big-screen (sidebar becomes a top row, cards 2× size).

### 5.3 Other views and dialogs

**Solitaire game window** (Klondike, 1100×760)

```
+------------------------------------------------------------------------------------------+
| [=] Klondike — draw 3 · deal #48213      (1) Score 215  Time 04:12  Moves 61  [_][o][x] |
+------------------------------------------------------------------------------------------+
| [New] [Restart] [Undo] [Redo] [Hint]                                     [Pause] [⚙]    |
+------------------------------------------------------------------------------------------+
|  ┌──┐ ┌──┐            ┌──┐ ┌──┐ ┌──┐ ┌──┐                                               |
|  │▒▒│ │7♥│            │A♠│ │2♥│ │  │ │  │        (2) stock, waste, foundations          |
|  └──┘ └──┘            └──┘ └──┘ └──┘ └──┘                                               |
|                                                                                          |
|  ┌──┐ ┌──┐ ┌──┐ ┌──┐ ┌──┐ ┌──┐ ┌──┐                                                     |
|  │K♣│ │▒▒│ │▒▒│ │▒▒│ │▒▒│ │▒▒│ │▒▒│                                                     |
|  │Q♦│ │9♠│ │▒▒│ │▒▒│ │▒▒│ │▒▒│ │▒▒│        (3) tableau                                  |
|  │J♠│ │8♦│ │5♣│ │▒▒│ │▒▒│ │▒▒│ │▒▒│                                                     |
|  └──┘ │7♣│ └──┘ │J♥│ │▒▒│ │▒▒│ │▒▒│                                                     |
|       └──┘      └──┘ │3♦│ │Q♠│ │▒▒│                                                     |
|                      └──┘ └──┘ │6♥│                                                     |
|                                └──┘                                                     |
+------------------------------------------------------------------------------------------+
| Hint: move the 6♥ onto the 7♣.                                  (4)   ⌨ H hint · Z undo |
+------------------------------------------------------------------------------------------+
```

1. HUD values (FUSE game UI labels in the header). 2–3. The board is one `viewportWidget` painted by the card engine; every pile and card is also a node in the a11y tree. 4. Message line (live region) and device prompts.

**Sudoku game window** (1000×720)

```
+------------------------------------------------------------------------------------------+
| [=] Sudoku — Hard · #90211              Time 12:40   Mistakes 0               [_][o][x] |
+------------------------------------------------------------------------------------------+
| [New] [Undo] [Redo] [Hint] [Notes: off] [Auto-notes]                         [Pause] [⚙]|
+--------------------------------------------------------+---------------------------------+
|  ┌───────┬───────┬───────┐                             |  ┌───┬───┬───┐                  |
|  │ 5 3 . │ . 7 . │ . . . │                             |  │ 1 │ 2 │ 3 │  remaining      |
|  │ 6 . . │ 1 9 5 │ . . . │                             |  ├───┼───┼───┤  1: 4   6: 5    |
|  │ . 9 8 │ . . . │ . 6 . │                             |  │ 4 │ 5 │ 6 │  2: 6   7: 3    |
|  ├───────┼───────┼───────┤                             |  ├───┼───┼───┤  3: 5   8: 4    |
|  │ 8 . . │ . 6 . │ . . 3 │                             |  │ 7 │ 8 │ 9 │  4: 6   9: 2    |
|  │ 4 . . │ 8[.]3 │ . . 1 │   (1)                       |  └───┴───┴───┘  5: 7           |
|  │ 7 . . │ . 2 . │ . . 6 │                             |  [Erase] [Notes]            (2) |
|  ├───────┼───────┼───────┤                             +---------------------------------+
|  │ . 6 . │ . . . │ 2 8 . │                             | Hint                        (3) |
|  │ . . . │ 4 1 9 │ . . 5 │                             | Hidden single: in box 5 only    |
|  │ . . . │ . 8 . │ . 7 9 │                             | this cell can hold 5.           |
|  └───────┴───────┴───────┘                             | [Show me] [Fill it]             |
+--------------------------------------------------------+---------------------------------+
```

1. Grid (`viewportWidget`; 81 cells as a11y nodes "row 5, column 5, empty, candidates 5"). 2. Number pad (`button` grid) with remaining counts. 3. Hint panel with stepped disclosure.

**Chess game window** (1200×760)

```
+------------------------------------------------------------------------------------------+
| [=] Chess — You vs Engine (level 4)                                           [_][o][x] |
+------------------------------------------------------------------------------------------+
| [New] [Take back] [Hint] [Flip] [Analyse] [Open PGN] [Save PGN]              [Pause] [⚙]|
+---------------------------------------------------+--------------------------------------+
|  Engine  ♟♟       04:31                           | Moves                            (2) |
|  8 ♜ . ♝ ♛ ♚ ♝ . ♜                                |  1. e4    e5      6. Re1  b5        |
|  7 ♟ ♟ ♟ ♟ . ♟ ♟ ♟                                |  2. Nf3   Nc6     7. Bb3  d6        |
|  6 . . ♞ . . ♞ . .                                |  3. Bb5   a6      8. c3   O-O       |
|  5 . ♗ . . ♟ . . .                           (1)  |  4. Ba4   Nf6     9. h3   …         |
|  4 . . . . ♙ . . .                                |  5. O-O   Be7                       |
|  3 . . . . . ♘ . .                                | Ruy Lopez, Closed (C84)              |
|  2 ♙ ♙ ♙ ♙ . ♙ ♙ ♙                                +--------------------------------------+
|  1 ♖ ♘ ♗ ♕ ♔ . . ♖                                | Analysis (on request)            (3) |
|    a b c d e f g h                                |  +0.3  depth 18                      |
|  You     ♙        05:02                           |  1. … Na5 10. Bc2 c5 11. d4 Qc7      |
|                                                   |  2. … Bb7 10. d4 Re8                 |
|  Type a move [______]                        (4)  |  [Lines 2 v] [Stop]                  |
+---------------------------------------------------+--------------------------------------+
| Your move.                                                  Engine: idle · helper, no net|
+------------------------------------------------------------------------------------------+
```

1. Board (`viewportWidget`), clocks and captured pieces as `label`s. 2. Move list (`tableView`, two plies per row, variations indented). 3. Analysis panel; the engine runs only while this is open or it is the engine's turn. 4. Typed move `lineEdit`.

**New game dialog** (example: Klondike, 460×420)

```
+-- New Klondike game ------------------------------------+
| Draw      (•) One card   ( ) Three cards                |
| Passes    [Unlimited v]                                 |
| Scoring   [Standard v]        [ ] Timed                 |
| Deal      (•) Random  ( ) Winnable only  ( ) Daily deal |
|           ( ) Seed [ 48213      ]                       |
|                               [Cancel]  [Deal]          |
+---------------------------------------------------------+
```

**Pause menu** (FUSE game UI menu mode; input arbiter gives all input to the menu)

```
        +---------------------------+
        |          Paused           |
        |   [ Resume          Esc ] |
        |   [ New game…           ] |
        |   [ Restart             ] |
        |   [ Options…            ] |
        |   [ How to play     F1  ] |
        |   [ Back to hub         ] |
        +---------------------------+
```

In timed modes the board behind is blanked.

**Win sheet** (480×360)

```
+-- You won ----------------------------------------------+
| Klondike · draw 3 · deal #48213                         |
| Time 06:48 (best 05:12)   Moves 118   Score 640         |
| Undos 3 · Hints 1                                       |
| Streak 4 (best 9)                                       |
| ★ New achievement: Ten wins in draw-three               |
|         [Statistics]  [Same deal again]  [New deal]     |
+---------------------------------------------------------+
```

**Game page: Statistics tab**

```
+------------------------------------------------------------------------------------------+
| Spider                         Variant [2 suits v]     Profile [Odin v]      [Reset…]    |
+------------------------------------------------------------------------------------------+
| Played 84 · Won 26 (31%) · Streak 2 (best 5) · Best time 09:41 · Fewest moves 98         |
|  wins/week                                                                               |
|   6 |          ▇                                                                         |
|   4 |    ▇  ▇  ▇     ▇                                                                   |
|   2 | ▇  ▇  ▇  ▇  ▇  ▇  ▇                                                                |
|     +--------------------------                                                          |
| Recent games                                                                             |
|  12 Nov  won   11:02  104 moves  #77120     [Replay] [Play this deal]                    |
|  11 Nov  lost  —      61 moves   #10388     [Play this deal]                             |
+------------------------------------------------------------------------------------------+
```

`chart` (bar), `tableView`.

**Controllers and input**: list of connected gamepads with a live button test (pressed buttons light up with their names), the binding table per game (`tableView`: action, keyboard, gamepad), `[Change]` captures the next input, conflicts flagged in `--warn`.

**Add Studio game dialog**: pick an exported bundle through the portal; the dialog shows title, signing key ("yours" or unknown), and the capability list; `[Add to library]` is disabled for bundles with an invalid signature.

### 5.4 Menus

Hub:

```
File    Add Studio Game… · Open Game File… Ctrl+O (PGN, FEN, sudoku) · Import Data… · Export Data…
        Close Ctrl+W · Quit Ctrl+Q
Edit    Find Ctrl+F · Preferences Ctrl+,
View    Grid / List · Big-Screen Mode F11 · Sidebar F9 · Show Hidden Games
Go      Library Ctrl+1 · Daily Deals Ctrl+2 · Statistics Ctrl+3 · Achievements Ctrl+4 · Controllers Ctrl+5
Game    Play Enter · Continue Last Ctrl+Enter · Game Page Alt+Enter · Favourite Ctrl+D · Hide
Help    Games Help F1 · About
```

Game window (common part; each game adds its own items):

```
Game    New Game… Ctrl+N · Restart Ctrl+R · Game from Seed… · Daily Deal · Open… Ctrl+O · Save As… Ctrl+Shift+S
        Statistics… · Back to Hub Ctrl+W · Quit Ctrl+Q
Edit    Undo Ctrl+Z · Redo Ctrl+Shift+Z · Copy Seed · Copy Position (FEN / grid text) · Paste Position
Move    Hint H · Auto-Move A · Auto-Finish · (Sudoku) Notes N · Auto-Notes · Erase Backspace
        (Chess) Take Back · Flip Board F · Offer Draw · Resign · Analyse Ctrl+Shift+A
        (Mahjong) Shuffle S · (Mines) Flag F · Chord Space
View    Full Screen F11 · Theme ▸ · Card/Tile/Piece Set ▸ · Show Legal Moves · Show Timer · Side Panel F4
        Zoom Ctrl++ / Ctrl+- / Ctrl+0
Options Sound ▸ · Animation Speed ▸ · Controls… · Preferences Ctrl+,
Help    How to Play F1 · Rules and Variants · Keyboard Shortcuts
```

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| Arrow keys | Move the board cursor (spatially between piles, cells, tiles); slide in Tiles |
| `Enter` / `Space` | Pick up, place, activate; `Space` chords in Mines |
| `Esc` | Cancel a pick-up; otherwise pause |
| `H` | Hint |
| `Ctrl+Z` / `Ctrl+Shift+Z` | Undo / redo |
| `A` | Auto-move to foundations (solitaire) |
| `1`–`9` | Enter digit (sudoku); jump to column (solitaire) |
| `N` | Notes mode (sudoku) |
| `F` | Flag (mines); flip board (chess) |
| `D` | Draw from stock (Klondike); deal a row (Spider) |
| `Ctrl+N`, `Ctrl+R` | New game, restart |
| `Ctrl+M` | Minimise and mute |

Gamepad: d-pad and left stick move; A pick/place; B cancel/back; X hint; Y undo; LB/RB redo and cycle piles; Start pause; Select side panel.

Pointer: click-click or drag for moves; double click sends to the best place; right click flags (mines), toggles a note (sudoku), cancels a pick-up elsewhere; wheel zooms the board with `Ctrl`.

Context menus: library card (Play, Game Page, Favourite, Hide, Delete Saves…, Remove…); chess move (Go to Position, Promote Variation, Delete from Here, Add Comment); board (Copy Position, Paste Position, Flip).

Drag and drop: a `.pgn` or `.fen` file onto Chess or the hub; an exported game bundle onto the hub (opens the Add dialog); PGN text out of the move list to another app.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | A game with no save opens its New game dialog. Statistics with no games: "Play a game to see numbers here". |
| Loading | Pack games: none visible (under 150 ms). 3D showcase: FUSE `LoadingScreen` with a progress bar. |
| Error | Corrupt save: notice and previous generation loaded. Engine helper crashed: "The chess engine stopped and was restarted"; the game continues. No GPU for the showcase: message with the CPU fallback option. |
| Offline | Always. |
| Paused | Board hidden in timed modes; timer stopped; music lowered. |
| Locked | Session lock pauses every game. |
| Read-only | A PGN opened from a read-only handle can be studied; "Save" becomes "Save As…". |
| Narrow (<640) | Side panels become sheets; card columns squeeze then scroll; the hub shows two columns. |

### 5.7 Widgets

| Region | FUSE widget or library |
|---|---|
| Hub library | `gridView`, `modelListView`, `treeView` |
| Game board | `viewportWidget` (forwards input to a callback and paints the game's image: `FUSE:docs/ui/README.md`) |
| HUD, pause menu, prompts | `fuse::ui::game`: `GameUi` (safe area, subtitles), `hudBar`, `floatText`, `GamepadNav`, `InputArbiter`, `GameUi::prompt`, `StringTable`, sound hook; from `game_extras.hpp`: `LoadingScreen`, `TouchControls`, `SoundTheme` (class names Verified in the header; behaviour not run) |
| Move list, statistics tables, bindings | `tableView` |
| Charts | `chart` |
| Dialogs | `dialog`, `radioButton`, `comboBox`, `spinBox`, `checkBox`, `slider` |
| Number pad | `button` grid |
| In-world panel (showcase) | `UiSurface` |

Missing or to be confirmed: the game UI library is bound to C++ today; it needs the ForgeScript binding like the rest of FUSE UI (G-02; the game layer is not named in FUSE-WP-12's title: `GM01-GAP-1`). The sound hook is not wired to `fuse_audio` ("wiring it … is the game's job for now", `FUSE:docs/ui/README.md`): `GM01-GAP-2`. Gamepad state reaching a sandboxed app through the compositor is not specified (FUSE-WP-08 covers pointer kinds; gamepads are not named): `GM01-GAP-3`. A 2D sprite and card renderer on the draw list (`Painter`, FUSE-WP-23) is assumed for boards.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Hub logic | ForgeScript | App host | Library, sources, statistics views, achievements, data import/export |
| Game framework | ForgeScript | App host | Common loop for turn-based games: state, move, undo stack, autosave, seed, timer, hint interface, statistics events |
| Card engine | ForgeScript | App host | Piles, rules as data, legal moves, layout, animation targets; Klondike, Spider, FreeCell rule sets |
| Mines, Sudoku front end, Tiles, Mahjong | ForgeScript | App host | Game rules and boards |
| UI | FUSE UI + `fuse::ui::game` | App host | Hub, boards, HUD, menus |
| `chess-engine` | C++23 | Helper, capability mode, no handles but its pipe | Search and evaluation; UCI subset over a pipe; memory and thread caps |
| `puzzle-helper` | C++23 | Helper, capability mode, pipe only | Heavy search: sudoku generation and grading, FreeCell/Klondike/Spider solvers, no-guess Mines generation, Mahjong solvable deals, Tiles hint |
| Chess rules and PGN | ForgeScript | App host | Move generation for legality and notation, PGN and FEN reading and writing (text formats, bounded) |
| 3D showcase runtime | C++23 (FUSE engine runtime) with game logic in the engine's script layer | Separate process started as a helper; GPU surface | Renders the showcase; talks to the hub for saves and statistics over the bus |

```
 SH01 launcher / hub ──► App host (ForgeScript + FUSE UI + fuse::ui::game)
                            │  game framework ── saves, stats ──► ~/.local/share/org.pbsd.games/
                            ├── pipe (UCI) ──► chess-engine   (cap mode, no fds, no net)
                            ├── pipe ───────► puzzle-helper  (cap mode, no fds, no net)
                            ├── bus ────────► showcase runtime (GPU surface, own assets read-only)
                            ├── svc.intent ─► other game bundles (own manifests)
                            └── bus ────────► VM03 (list and launch compatibility titles)
```

Helpers are justified by speed (PLATFORM.md section 2, case b). They parse nothing from outside: their only input is positions and parameters from the app host. Turn-based game rules stay in ForgeScript, where a move costs microseconds and memory safety comes from the language.

The 3D showcase is the one place where engine code runs in the game process. How FUSE engine game logic is scripted (`fuse_script`, `FUSE:docs/scripting.md`) was not read; whether it can be ForgeScript is open (section 13).

### 6.2 Data model

Stores under `~/.local/share/org.pbsd.games/`: `games.sqlite` (statistics, achievements, library state, profiles), `saves/<game>/<slot>.save` (one file per save, two generations kept), `replays/`.

| Entity | Fields |
|---|---|
| `GameEntry` | `id: string` (`pack:klondike`, `bundle:<bundle id>`, `compat:<vm03 id>`, `studio:<hash>`), `title`, `source: enum{pack, bundle, studio, compat}`, `genre: enum`, `players_max: u8`, `gamepad: bool`, `net: bool`, `favourite: bool`, `hidden: bool`, `last_played: i64`, `play_seconds: u64` |
| `Profile` | `id: u32`, `name`, `colour: u8`, `created: i64` |
| `SaveHeader` | `magic: "PBGS"`, `format: u16`, `game: string`, `variant: string`, `seed: u64`, `rules_version: u16`, `profile_id`, `created: i64`, `updated: i64`, `elapsed_ms: u64`, `checksum: blake3` |
| `SaveBody` | `initial: bytes` (deal or puzzle), `moves: list<Move>`, `cursor: u32` (undo position), `flags: bitset{used_hint, used_undo, timed}` — state is rebuilt by replaying moves from `initial`, so a save is also a replay |
| `Move` (cards) | `from_pile: u8`, `to_pile: u8`, `count: u8`, `kind: enum{move, draw, deal, flip, recycle}` |
| `Move` (sudoku) | `cell: u8`, `kind: enum{set, clear, note_on, note_off}`, `digit: u8` |
| `Move` (chess) | `from: u8`, `to: u8`, `promo: u8`, `clock_ms: u32`; plus PGN tags and comments in the body |
| `Move` (mines) | `cell: u16`, `kind: enum{reveal, flag, chord}` |
| `Move` (tiles) | `dir: u2`, `spawn_cell: u8`, `spawn_value: u8` |
| `Move` (mahjong) | `tile_a: u16`, `tile_b: u16`, or `shuffle: seed` |
| `StatRow` | `game`, `variant`, `profile_id`, `played: u32`, `won: u32`, `streak: u32`, `best_streak: u32`, `best_time_ms: u32?`, `best_score: i32?`, `total_time_ms: u64`, `total_moves: u64`, `hints: u32`, `undos: u32` |
| `ResultRow` | `id`, `game`, `variant`, `profile_id`, `seed`, `when: i64`, `outcome: enum{won, lost, abandoned, draw}`, `time_ms`, `moves`, `score`, `clean: bool` (no hint, no undo) |
| `Achievement` (bundle data) | `id: string`, `game`, `title_key`, `desc_key`, `kind: enum{flag, counter}`, `target: u32`, `secret: bool` |
| `AchievementState` | `id`, `profile_id`, `progress: u32`, `earned_at: i64?` |
| `Binding` | `game`, `action: string`, `key: string?`, `pad_button: string?`, `profile_id?` |
| `MahjongLayout` (bundle data) | `id`, `name_key`, `slots: list<{x: i8, y: i8, z: i8}>` in half-tile units |
| `DailyDeal` | derived, not stored: `seed = blake3("pbsd-games-daily" ‖ game ‖ variant ‖ yyyy-mm-dd)[0..8]` |

Random numbers: one documented generator (a 64-bit permuted congruential generator; FreeCell numbered deals use their classic generator) seeded from the seed. Seeds for ordinary new games come from the system random source. `rules_version` guards replays: a save made under older rules is replayed with those rules or marked unverifiable.

`games.sqlite` has `meta.schema_version`; migration is forward with a copy kept.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Save files (`.save`, own binary, checksummed) | yes | yes | own ForgeScript | App host (own files; still bounds- and schema-checked) |
| PGN (export format with tags, comments, NAGs, variations) | yes | yes | own ForgeScript, size cap 32 MiB per file, depth cap on variations | App host (untrusted text: memory-safe language, strict limits) |
| FEN, X-FEN (Chess960) | yes | yes | own | App host |
| UCI (subset: `uci`, `isready`, `ucinewgame`, `position`, `go`, `stop`, `setoption`, `info`, `bestmove`) | yes | yes | own, line length and rate caps | App host ↔ chess-engine |
| Sudoku text (81 characters; common variants with `.` or `0`) | yes | yes | own | App host |
| Mahjong layouts, achievements, rule files (bundle data) | yes | no | own | App host (signed bundle data) |
| Data export (`.pbgames`: zstd tar of saves + JSON of statistics) | yes | yes | UT01's archive service for packing and safe unpacking | UT01 archive-helper |
| Studio game bundles | inspect only | no | bundle reader and signature check from the platform (FORGE-WP-09) | platform |
| Cooked engine assets for the showcase (FMSH, `.fusetex`, `.fusemat`: named in `FUSE:docs/modules.md` under `fuse_asset`) | yes | no | FUSE `fuse_asset` | showcase runtime (own signed assets only) |
| Audio assets (bundled, open formats) | yes | no | `fuse_audio` or `svc.media` | showcase runtime / media service |

No third-party library is added. The chess engine and solvers are own code. No opening book, tablebase or puzzle set is bundled unless its licence is recorded and compatible.

### 6.4 Integration

- Intents handled: `open` and `view` for `application/vnd.chess-pgn`, `application/x-chess-fen` (Proposed type name), `text/x-sudoku` (Proposed); URL scheme `games:` (`games:klondike`, `games:chess?fen=…`); `search` (game names in the launcher).
- Intents emitted: `open` (launch other game bundles; rules pages in UT05), `print` (sudoku sheets), `share` (PGN, seed text).
- Services: `svc.settings`, `svc.notify` (rare: achievement toast is in-app; a daily-deal reminder only if the user turns it on), `svc.portal.file`, `svc.intent`, `svc.backup`, `svc.power` (inhibit screen dimming during the 3D showcase only), `svc.media` (audio), `svc.portal.print`, `svc.a11y`.
- Other apps: CR04 Studio (exports register here; the showcase project opens there), VM03 Compatibility (list and launch interface, Proposed), SY05 Software (install and remove), AD05 Permissions (capabilities of each listed game), SH02 (do-not-disturb lease), ME05 (capture), UT05 (rules pages), UT01 (data export packing).
- FUSE facts used (Verified in `FUSE:docs/ui/README.md` and the headers): `GameUi` HUD layer with safe area, kill feed, subtitles, dialogue, pools; `hudBar`/`setHudBar`; world markers; `floatText`; game-time clock pause; `GamepadNav`; `InputArbiter`; `GameUi::prompt`; `UiSurface` with `surfaceRaycast`, `surfacePointer`, `surfaceRender`; `StringTable`; sound hook with Move, Accept, Back, Open, Close; in `game_extras.hpp`: `ItemCatalog`, `Inventory`, `TouchControls`, `LoadingScreen`, `RichDialogue`, `SoundTheme`; in `ui_audio.hpp`: `UiAudio`; in `ecs_surface.hpp`: `UiSurfaceSystem`. From `FUSE:docs/modules.md`: `fuse_audio`, `fuse_physics`, `fuse_ecs`, `fuse_vfx`, `fuse_asset`, `fuse_animation`, `fuse_navigation` (grid A*), `fuse_net` ("session / net stubs", not used). Proposed: everything this spec builds on them.

## 7. Security and privacy

| Capability | Why |
|---|---|
| `fs.app` | Saves, statistics, achievements |
| `fs.portal` | Open and save PGN and puzzle files; import and export data; pick a Studio export |
| `net.none` | No network, by design |
| `proc.helper:chess-engine` | Fast search, contained |
| `proc.helper:puzzle-helper` | Generators and solvers, contained |
| `proc.helper:showcase-runtime` | The 3D showcase process |
| `dev.gpu.compute` | Proposed use for the showcase's rendering. PLATFORM.md section 6 has no plain GPU-render capability; rendering may need none beyond the compositor surface. Listed so the need is visible (`GM01-GAP-4`) |
| `svc.settings`, `svc.portal.file`, `svc.intent` | Core operation |
| `svc.media` | Sound |
| `svc.power` | Keep the screen on in the showcase |
| `svc.backup` | Include saves |
| `svc.notify` | Optional daily-deal reminder; do-not-disturb lease |
| `svc.portal.print` (`dev.print`) | Sudoku sheets |
| `svc.a11y` | Board trees and announcements |

Not requested: any network capability, `proc.exec`, `dev.usb:*` (gamepads come through the input path), camera, microphone, location, `ai.local`, `bg.run`, `bg.autostart`.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| The rest of the system | A game listed in the hub (Studio export, other bundle, compatibility title) | Using the hub as a way to run with more rights | The hub launches through `svc.intent` or VM03; each game runs under its own manifest or sandbox; the hub shows each game's capabilities and network mark before launch |
| The hub process | Hostile PGN, FEN, sudoku or save files | Parser bugs, huge inputs, deep variation nesting | Parsers in ForgeScript (memory-safe) with size, depth and count caps; saves are checksummed and schema-checked |
| The hub process | Hostile data-import archive | Path traversal on unpack | Unpacking is done by UT01's helper into the app's own directory with its limits |
| CPU, memory, battery | Engine or solver run away | Resource exhaustion | Helpers have thread, memory and time caps; analysis runs only on request; idle games draw nothing |
| Play habits | Other apps, other users | Reading statistics and play time | Data in `fs.app`; play-time tracking can be turned off; nothing is sent anywhere |
| Children | Network strangers, purchases, advertising | — | No network, no store, no accounts in the pack |
| Fairness of local records | The user | Editing save files | Not defended beyond a checksum; records are for the player's own interest |
| Lock screen and focus | A full-screen game | Hiding system prompts or the lock | Full screen is a compositor state; shell prompts and the lock always draw above (SH05, SH03) |
| Users sensitive to flashing | Game effects | Seizure risk | No flashing above 3 Hz; reduced-motion honoured (F147) |

Stored: saves, statistics, achievements, play time, bindings, profiles (names only). Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `general.resume_on_start` | bool | false | Open straight into the last game |
| `general.track_play_time` | bool | true | Record play time |
| `general.pause_on_focus_loss` | bool | true | Pause timed games when the window loses focus |
| `general.daily_reminder` | bool | false | Notify when a new daily deal is available |
| `general.big_screen_on_gamepad` | bool | true | Switch layout when opened from a gamepad |
| `appearance.animation_speed` | enum | `normal` | off, fast, normal, slow |
| `appearance.card_faces` | enum | `standard` | standard, large_index, four_colour, high_contrast |
| `appearance.card_back` | string | `plain` | Card back id |
| `appearance.table` | string | `green` | Table background id |
| `appearance.tile_set` | enum | `traditional` | traditional, large_symbol, alphanumeric |
| `appearance.piece_set` | enum | `classic` | classic, high_contrast, large |
| `sound.master` | u8 | 60 | Volume percent |
| `sound.effects` | bool | true | Effects on |
| `sound.music` | bool | false | Music on (showcase only) |
| `input.show_legal_moves` | bool | true | Highlight targets |
| `input.auto_move` | enum | `safe` | off, safe, all |
| `input.left_handed` | bool | false | Mirror layouts |
| `input.rumble` | bool | false | Gamepad rumble |
| `klondike.draw` | u8 | 1 | 1 or 3 |
| `klondike.winnable_only` | bool | false | Solver-checked deals |
| `spider.suits` | u8 | 1 | 1, 2, 4 |
| `freecell.cells` | u8 | 4 | 0–4 |
| `mines.size` | enum | `beginner` | beginner, intermediate, expert, custom |
| `mines.no_guess` | bool | false | Logic-only boards |
| `mines.question_marks` | bool | false | Third cell state |
| `sudoku.level` | enum | `medium` | easy, medium, hard, expert |
| `sudoku.mistakes` | enum | `conflicts` | none, conflicts, solution |
| `sudoku.auto_remove_notes` | bool | true | Remove notes when a digit is placed |
| `chess.level` | u8 | 3 | 1–10 |
| `chess.time_control` | string | `none` | `none`, `10+0`, … |
| `chess.engine_threads` | u8 | 1 | Helper threads (cap: cores − 1) |
| `chess.engine_hash_mib` | u16 | 64 | Helper hash table size |
| `chess.flip_in_hot_seat` | bool | true | Flip the board each move with two players |
| `tiles.size` | u8 | 4 | 3–6 |
| `tiles.undo` | enum | `one` | none, one, unlimited |
| `mahjong.layout` | string | `turtle` | Layout id |
| `mahjong.solvable_only` | bool | true | Reverse-played deals |
| `showcase.quality` | enum | `medium` | low, medium, high |
| `showcase.frame_cap` | enum | `60` | 30, 60, display |

## 9. Accessibility and languages

- Every board is in the a11y tree: piles and cards ("tableau 3, five cards, top card six of hearts, face up"), sudoku cells, chess squares, mahjong tiles with layer and freedom, mines cells. Moves and results are announced through a live region.
- No game needs colour vision: suits have shapes (plus the four-colour option), mine numbers are digits, tiles carry numbers, mahjong has an alphanumeric set, chess highlights use outlines as well as tints.
- No game needs speed: every timer can be turned off; the 3D showcase has a slow mode and no fail-on-time levels in that mode.
- No game needs fine pointer control: keyboard, gamepad and a one-switch scanning mode cover every action; drag is never the only way.
- Text in games comes from `StringTable`-style `key=value` tables matching the platform's locale tables; card ranks and chess piece letters follow the language (figurine notation avoids letters).
- Right-to-left layouts mirror the hub and menus; boards do not mirror except by the left-handed option.
- Sound is never the only cue; every sound has a visual equivalent.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Hub cold start to first frame | 150 ms |
| Pack game start from the hub or launcher | 150 ms to a playable board (deal animation may follow) |
| Idle | Zero frames and zero wake-ups in turn-based games with no animation and no running timer; a visible timer redraws once per second |
| Move response | Input to first animation frame under 16 ms; hint under 200 ms or "thinking…" |
| Memory | Hub plus one card game under 80 MiB; chess-engine under `engine_hash_mib` + 32 MiB; puzzle-helper under 128 MiB |
| Sudoku generation | Easy–Hard under 300 ms; Expert under 2 s (in the helper; a pre-generated pool is topped up in idle time while the game is open) |
| Winnable-deal check | FreeCell under 1 s typical; cap 5 s then "not checked" |
| Chess engine | At least 1 million nodes per second per thread on the reference machine (target for the engine, not a claim) |
| 3D showcase | 60 fps at 1080p medium on the reference GPU; 30 fps on the CPU fallback at reduced resolution; load a level under 2 s |
| Save | Each move persisted in under 2 ms (append) |
| Bundle size | Under 60 MiB for the pack including the showcase |

## 11. Testing and proof

- **Unit**: rules of every game (legal moves, win and loss detection), scoring, undo/redo round trips, seeds give identical deals, daily seed derivation, save replay equals live state.
- **Property tests**: for random move sequences, undo-all returns the initial state; a replay of any save reproduces the same final state; card counts are conserved; sudoku generator outputs have exactly one solution (checked by an independent second solver).
- **Differential**: chess move generation against known perft node counts for the standard test positions; FreeCell numbered deals against published deal layouts for a sample of numbers; PGN round trip on a corpus of public-domain games.
- **Solver cross-checks**: "winnable" deals are won by the solver's own line replayed through the rules engine; no-guess Mines boards are solved by a logic-only solver from the first click; Mahjong solvable deals are solved by the recorded reverse order.
- **Engine tests**: a fixed tactical test set, self-play at each level for sanity of strength ordering (lower level loses to higher), time-control compliance. Strength labels are estimates and are worded so.
- **Golden**: hub, each game board in each theme, high contrast, 200%, right-to-left menus.
- **Scripted UI**: T1–T13 through the a11y tree; one full game of each kind played by script with keyboard only, and again with simulated gamepad input.
- **Fuzz**: PGN, FEN, sudoku text, save files, UCI responses from the helper, layout and rule data files.
- **Idle test**: a parked game produces no frames and no wake-ups for 60 s.
- **Sandbox**: helpers cannot open files or sockets (G-06).

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Sudoku solver and uniqueness check | Sound and complete: reports exactly the number of solutions up to 2 | T1 | PRISM proof of the search over the constraint model | — |
| Card rules engine | Card set is conserved by every move; undo is the inverse of move | T1 | PRISM proof on the pile model | Interpreter tier limits the claim to the model |
| Chess move generator | Generates exactly the legal moves | T2 | Perft differential; PRISM absence-of-runtime-error | A full proof against the rules is large |
| Save replay | Replay(initial, moves) is deterministic and equals the live state | T2 | PRISM plus property tests | — |
| Seeded generators | Same seed gives the same deal on every platform (no floating point, fixed-width arithmetic) | T2 | PRISM; cross-platform test vectors | — |
| Chess engine search | Memory safety; always returns a legal move within the time limit | T2 | PRISM; tests | Playing strength is not a provable property |
| PGN and FEN parsers | No crash; bounded resources | T3 | Fuzzing | — |
| Game UI and hub | Memory-safe by language | T3 | Tests | — |
| 3D showcase runtime | None beyond the engine's own | T4 | Play tests | FUSE engine tier is outside this spec |

Nothing here has been run.

## 12. Build plan

Phase 1: the first playable pack.

| ID | Work item | Depends on |
|---|---|---|
| GM01-W01 | Game framework: state, moves, undo, autosave, seeds, timer, statistics events | G-02, G-07, FORGE-WP-13 |
| GM01-W02 | Board rendering on `viewportWidget` with a 2D sprite layer; a11y nodes for board items | FUSE-WP-23, FUSE-WP-12 |
| GM01-W03 | Card engine and Klondike | GM01-W01, GM01-W02 |
| GM01-W04 | Spider and FreeCell (with numbered deals) | GM01-W03 |
| GM01-W05 | Mines (classic boards, safe first click, chording) | GM01-W01, GM01-W02 |
| GM01-W06 | Tiles | GM01-W01, GM01-W02 |
| GM01-W07 | Sudoku with generator and uniqueness check (in ForgeScript first, pooled) | GM01-W01 |
| GM01-W08 | Chess rules, two-player hot-seat, move list | GM01-W01, GM01-W02 |
| GM01-W09 | Hub v1: pack library, Continue, game pages, statistics, launcher entries | G-09, SH01 |
| GM01-W10 | Save format, corruption recovery, replay property tests | GM01-W01 |

Phase 2: full pack.

| ID | Work item | Depends on |
|---|---|---|
| GM01-W11 | `chess-engine` helper, levels, time controls, analysis | G-06, FORGE-WP-27, FORGE-WP-28 |
| GM01-W12 | PGN and FEN, game list, position set-up | GM01-W08 |
| GM01-W13 | `puzzle-helper`: solvers for winnable deals, sudoku grading by technique, explained hints | GM01-W11 (shared helper kit) |
| GM01-W14 | Mahjong with layouts, solvable deals, tile sets | GM01-W02 |
| GM01-W15 | Game UI binding: HUD, pause menu, prompts; gamepad navigation and in-game cursor; remapping | `GM01-GAP-1`, `GM01-GAP-3`, G-02 |
| GM01-W16 | Sound through the game UI sound hook | `GM01-GAP-2`, G-09 (`svc.media`) |
| GM01-W17 | Achievements, daily deals, themes, big-screen mode | GM01-W09 |
| GM01-W18 | Other sources in the library: installed game bundles, VM03 titles; capability and network marks | VM03, AD05, FORGE-WP-09 |
| GM01-W19 | No-guess Mines; Mines metrics; sudoku entry of own puzzles | GM01-W13 |
| GM01-W20 | Data export and import | UT01 |

Phase 3: showcase and extras.

| ID | Work item | Depends on |
|---|---|---|
| GM01-W21 | 3D showcase: levels, physics, audio, HUD, settings | G-18, FUSE-WP-01, FUSE-WP-24, CR04, `GM01-GAP-4` |
| GM01-W22 | Studio exports in the hub | CR04, FORGE-WP-09 |
| GM01-W23 | Local multiplayer: profiles, several gamepads, race modes | GM01-W15 |
| GM01-W24 | Chess review, opening names, puzzles, Chess960 | GM01-W11 |
| GM01-W25 | Replays, more patience games, sudoku variants, printing | GM01-W03, GM01-W07, G-16 |
| GM01-W26 | One-switch scanning mode; play limits | G-04, SY01 |

## 13. Open questions and risks

- The game UI library is C++ today and FUSE UI scripting is Lua (PLATFORM.md V4). A ForgeScript binding for `fuse::ui::game` is assumed but is not named in the FUSE build plan's work-package titles (`GM01-GAP-1`).
- How a sandboxed app receives gamepad input is not specified anywhere read (`GM01-GAP-3`). Wayland has no core gamepad protocol; the platform needs an input portal or a compositor extension. Without it, controller support slips.
- The 3D showcase needs the FUSE engine to build and run on HardenedBSD with a working GPU path (G-18, FUSE-WP-01, FUSE-WP-24). It also raises the language rule: engine game logic is not ForgeScript today. Options: treat the showcase runtime as a native helper wrapping an existing engine (PLATFORM.md section 2, case d, by analogy), or wait for a ForgeScript game-logic binding. The showcase is phase 3 for this reason.
- There is no capability for GPU rendering beyond `dev.gpu.compute` (`GM01-GAP-4`); whether a game's swapchain needs one should be decided by the platform.
- Art: card faces, tile faces and piece sets must be own work or licence-compatible and recorded in the SBOM. Mahjong tile art is the largest item.
- Playing strength of an own chess engine is a long project; phase 2 aims for "fun at ten levels", not for top strength.
- Compatibility with the classic FreeCell deal numbers is a claim to make only after the differential test has run.
- VM03's list-and-launch interface and CR04's export format are assumed, not read.
- ForgeScript speed for sudoku generation and card solvers may be too low; the puzzle helper is the planned answer, with pre-generated pools as a fallback.
