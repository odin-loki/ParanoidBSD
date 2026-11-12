<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Clocks

By Odin Loch

| | |
|---|---|
| ID | `UT02` (from CATALOGUE.md) |
| Bundle | `org.pbsd.clocks` |
| Category | utils |
| Priority | P2 |
| Backing repos | FUSE (UI); FORGE (text and locale layer, time library: Proposed use, FORGE-WP-23); base time-zone data `hbsd/src/contrib/tzdata` (Verified present) |
| Status | Planned. Nothing built. |

## 1. Purpose

Clocks shows the time in other places, wakes the user, counts down, counts up and runs focus sessions. It is the small app everyone uses daily, so it must start instantly, never miss an alarm, and never touch the network. It replaces KClock from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- World clock with a day/night map and a meeting planner.
- Alarms: one-off, recurring, wake from suspend, gradual volume, snooze.
- Timers: several at once, named, presets.
- Stopwatch with laps.
- Focus (pomodoro) sessions that switch do-not-disturb on.
- Bedtime: wind-down reminder and wake alarm as one schedule.
- Time-zone converter.
- A panel applet for SH01 and a read-only time-source status card.

Out of scope:

| Item | Owner |
|---|---|
| Setting the system time, time zone, NTP servers, 12/24-hour format | SY01 Settings (this app links there) |
| Calendar events and their reminders | CO02 Calendar |
| To-do deadlines | CO04 Tasks |
| The do-not-disturb mechanism itself, notification display | SH02 Notifications and Quick Settings |
| Lock-screen drawing of a ringing alarm | SH03 Login, Lock and Session (this app supplies the content) |
| Night light, display dimming | SY01 Settings |
| Sleep tracking from sensors | not planned |
| The panel clock itself | SH01 Desktop Shell (this app supplies the applet popover) |

## 3. Users and top tasks

Personas:

- **Remote worker**: colleagues in four zones; plans meetings.
- **Sleeper**: one weekday alarm, one weekend alarm, wants a gentle wake.
- **Cook / student**: several timers; focus sessions.
- **Traveller**: converts times before and during trips.

| # | Task | Target steps |
|---|---|---|
| T1 | See the time in a saved city | 0 (visible on open or in the applet) |
| T2 | Add a city | 3 (`Ctrl+N`, type, Enter) |
| T3 | Set an alarm for 07:00 on weekdays | 4 |
| T4 | Start a 10-minute timer | 2 (preset click) or type "10m" Enter |
| T5 | Stop a ringing alarm or timer | 1 (key, click, or notification action) |
| T6 | Snooze | 1 |
| T7 | Start the stopwatch and take laps | 1 + 1 per lap (`Space`, `L`) |
| T8 | Find a meeting hour that is daytime in three cities | 3 |
| T9 | Convert "15:30 Tuesday in Tokyo" to local time | 3 |
| T10 | Start a focus session | 1 from the applet |
| T11 | Turn tomorrow's alarm off once | 2 |
| T12 | Check whether the clock is synchronised | 1 (status card) |

## 4. Features

### 4.1 World clock

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT02-F001 | City list | Saved places as cards: name, local time, date if different, UTC offset, offset from here ("+9 h, tomorrow"), zone abbreviation, day or night icon. | M | 1 |
| UT02-F002 | Add a city | Search an offline table of about 25 000 places (name, alternate names, country, zone id, coordinates). Fuzzy match, alias match ("NYC"), zone id match ("Europe/Vienna"). | M | 1 |
| UT02-F003 | Add a raw zone | Add a zone id or a fixed UTC offset with a custom label. | S | 1 |
| UT02-F004 | Custom labels | Rename a card ("Mum", "Sydney office"). | S | 1 |
| UT02-F005 | Reorder and sort | Drag to reorder; sort by offset or name. | M | 1 |
| UT02-F006 | Day/night map | World map (equirectangular, bundled vector outline) with the terminator and markers for saved cities. Updates once a minute. | S | 2 |
| UT02-F007 | Map pick | Click the map to list the nearest known places. | C | 2 |
| UT02-F008 | Time scrubber | A slider moves all clocks forward or back up to ±7 days; "Now" resets. | S | 1 |
| UT02-F009 | DST notes | Each card shows the next transition ("clocks go back 1 h on 5 Apr") from zone data. A warning shows within 14 days. | S | 2 |
| UT02-F010 | Sun times | Sunrise, sunset and day length per city, computed locally from coordinates. | C | 2 |
| UT02-F011 | Analogue or digital faces | Per-list choice; seconds optional. With seconds off the view updates once a minute. | S | 1 |
| UT02-F012 | Home zone marker | The system zone is shown first and marked; if the system zone changes (travel) the list re-bases. | M | 1 |
| UT02-F013 | Copy time | Copy a card's time as text, ISO 8601, or "time in all cities". | S | 2 |
| UT02-F014 | Zone data version | About page shows the tz database version read from the system. | S | 1 |

### 4.2 Meeting planner and converter

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT02-F020 | Planner grid | Rows are chosen cities, columns are hours of a chosen day. Cells are shaded: working hours, waking hours, night. | S | 2 |
| UT02-F021 | Working hours per city | Default 09:00–17:00 Monday–Friday; editable per card; weekend days follow the locale of the city where known. | S | 2 |
| UT02-F022 | Best slots | Highlights hours inside working hours for all rows; if none, the least bad three. | S | 2 |
| UT02-F023 | Pick and share a slot | Select a column range; "Copy" gives a text block with the time in each city; "Create event" emits a `compose` intent to CO02 with start, end and zone. | S | 2 |
| UT02-F024 | Converter | Enter a time, a date and a source zone; see it in the local zone and all saved cities. Accepts free text ("3pm tue tokyo") parsed locally by rules. | M | 1 |
| UT02-F025 | Ambiguous and missing times | A time in a DST gap or overlap is flagged and both readings are shown. | M | 1 |
| UT02-F026 | Duration and difference | Difference between two date-times across zones; add a duration to a date-time. | C | 2 |
| UT02-F027 | Unix and ISO forms | Converter shows Unix seconds, ISO 8601 and RFC 3339 and accepts them as input. | C | 2 |

### 4.3 Alarms

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT02-F030 | Alarm list | Cards with time, label, repeat summary, next ring ("in 7 h 12 min"), toggle. Sorted by next ring. | M | 1 |
| UT02-F031 | One-off and recurring | Repeat: once, daily, weekdays, weekends, chosen days, every n days, chosen dates, monthly on a day. | M | 1 |
| UT02-F032 | Zone behaviour | An alarm is "local wall time" (default, moves with the system zone) or pinned to a zone. | S | 2 |
| UT02-F033 | DST behaviour | An alarm in a skipped hour rings at the first valid minute after it; an alarm in a repeated hour rings once (first occurrence). | M | 1 |
| UT02-F034 | Skip next | Turn off only the next occurrence. | M | 1 |
| UT02-F035 | Holidays and pauses | "Pause until date" for a set of alarms; optional skip on dates from a calendar the user picks (read from CO02 through its bus interface). | C | 3 |
| UT02-F036 | Sounds | Bundled sounds (open formats), or a file the user picks through the portal (handle kept). Preview button. | M | 1 |
| UT02-F037 | Gradual volume | Volume rises from 5% to the set level over 0–5 minutes (default 30 s). | M | 1 |
| UT02-F038 | Output choice | Ring on the default output, a chosen device, or "speakers even when headphones are connected" through `svc.media` routing. | S | 2 |
| UT02-F039 | Volume independence | Alarm volume is its own stream and ignores media mute. A warning shows when the system alarm stream is at zero. | M | 1 |
| UT02-F040 | Snooze | Length 1–30 min (default 9), limit count (default unlimited), per alarm. | M | 1 |
| UT02-F041 | Ring screen | Full-window ring view with time, label, Stop and Snooze. Shown on the lock screen through SH03 without unlocking. | M | 1 |
| UT02-F042 | Dismiss challenge | Optional: solve a sum or type a phrase to stop. | C | 3 |
| UT02-F043 | Auto-silence | Rings stop after 1–60 min (default 10) and the alarm is logged as missed with a notification. | M | 1 |
| UT02-F044 | Wake from suspend | The daemon asks `svc.power` for a wake timer 30 s before the next alarm. If the hardware cannot wake, the card shows `--warn` "This computer cannot wake itself: keep it on". | M | 2 |
| UT02-F045 | Rings without the app | A background daemon owns scheduling; the window need not be open (`bg.run`, `bg.autostart`). | M | 1 |
| UT02-F046 | Survives restart | The schedule is on disk; after boot, missed alarms in the last `alarm.missed_window` (default 15 min) ring at once, older ones notify as missed. | M | 1 |
| UT02-F047 | Do-not-disturb | Alarms ring through do-not-disturb by default (SH02 treats them as critical). Per-alarm option to obey it. | M | 1 |
| UT02-F048 | Visual and haptic alarm | Screen flash option for deaf users; no sound mode. | S | 2 |
| UT02-F049 | Pre-alarm notice | Notification n minutes before with "Skip this one". | C | 2 |
| UT02-F050 | Duplicate, delete, undo | Duplicate a card; delete with Undo toast. | M | 1 |
| UT02-F051 | Quick alarm | Type "7:30" or "in 45m" in the add field. | S | 1 |
| UT02-F052 | Suspend/shutdown warning | When the user shuts down with an alarm due in the next 12 h, SH03 is told and may show "an alarm is set for 07:00". | C | 3 |

### 4.4 Timers

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT02-F060 | Multiple timers | Any number running at once, each a card with ring progress, name, remaining time and end time. | M | 1 |
| UT02-F061 | Quick entry | Type `90`, `1:30`, `1h30m`, `25m tea`; the trailing words become the name. | M | 1 |
| UT02-F062 | Presets | Buttons 1, 3, 5, 10, 15, 30 min by default; editable; named presets ("Eggs 6:30"). | M | 1 |
| UT02-F063 | Pause, resume, reset, add time | `+1 min` and `+5 min` on a running or ringing timer. | M | 1 |
| UT02-F064 | Repeat | A timer can auto-restart n times or for ever. | S | 2 |
| UT02-F065 | Sequences | Chain timers (interval training: 40 s work, 20 s rest × 8). | C | 3 |
| UT02-F066 | Finish actions | Ring (default), notify only, or stop media playback through `svc.media` ("sleep timer"). | S | 2 |
| UT02-F067 | Timers survive restart | Stored as end instants on the monotonic-plus-wall pair; restored after a crash or reboot. | M | 1 |
| UT02-F068 | Wake for timers | Timers longer than 5 minutes also set a wake timer (option). | S | 2 |
| UT02-F069 | Taskbar progress | The running timer nearest to its end drives the app's badge and panel progress through `svc.notify`. | S | 2 |
| UT02-F070 | History | Last 20 used durations for one-click reuse. | C | 2 |

### 4.5 Stopwatch

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT02-F080 | Start, stop, reset | Monotonic clock; display to 1/100 s; redraw at display rate only while visible and running. | M | 1 |
| UT02-F081 | Laps | Lap table: number, lap time, split (total), difference from the previous lap; fastest and slowest marked. | M | 1 |
| UT02-F082 | Export laps | Copy or save as CSV or text. | S | 2 |
| UT02-F083 | Keeps running closed | Elapsed time is derived from the start instant, so closing the window costs nothing. | M | 1 |
| UT02-F084 | Several stopwatches | Named extra stopwatches in tabs. | C | 3 |
| UT02-F085 | Lap chart | Bar chart of lap times. | C | 3 |

### 4.6 Focus sessions

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT02-F090 | Focus cycle | Work 25 / short break 5 / long break 15 after 4 rounds by default; all editable; named profiles. | S | 2 |
| UT02-F091 | Do-not-disturb link | Starting a work phase asks SH02 to turn do-not-disturb on with the reason "Focus until 10:25"; it is released on break, stop or crash (SH02 holds the request with a lease that expires). | S | 2 |
| UT02-F092 | Task label | Optional label per session; can be picked from CO04 Tasks by intent (`pick`). | C | 3 |
| UT02-F093 | Auto-start options | Auto-start breaks and next rounds, or wait for the user. | S | 2 |
| UT02-F094 | Phase sounds and notices | Distinct soft sounds for work end and break end. | S | 2 |
| UT02-F095 | Statistics | Sessions per day and week, total focus minutes, streaks. Local only. Chart view. Export CSV. Clear. | S | 2 |
| UT02-F096 | Skip and extend | Skip the break; extend the phase by 5 min. | S | 2 |
| UT02-F097 | Idle awareness | If the session is locked during a work phase the phase pauses (option). | C | 3 |

### 4.7 Bedtime

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT02-F100 | Bedtime schedule | Per weekday: bedtime and wake time; shows the sleep length; a dial editor. | S | 2 |
| UT02-F101 | Wind-down reminder | Notification 0–60 min before bedtime; optional do-not-disturb from bedtime to wake (via SH02). | S | 2 |
| UT02-F102 | Wake alarm | The wake time is an ordinary alarm with gradual volume, shown in the alarm list as "Bedtime". | S | 2 |
| UT02-F103 | Night light hand-off | Link to SY01's night-light schedule; this app does not control the display. | C | 3 |

### 4.8 Panel applet and shell integration

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT02-F110 | Applet popover | SH01 hosts a popover: next alarm, running timers with controls, quick timer field, focus start, two world clocks. | S | 2 |
| UT02-F111 | Panel indicators | Alarm-set icon, running timer countdown text, focus phase, supplied to SH01 through the applet interface. | S | 2 |
| UT02-F112 | Notification actions | Ringing items appear as critical notifications with Stop, Snooze, +1 min. | M | 1 |
| UT02-F113 | Launcher and search | "timer 10m", "alarm 7:30", "time in Tokyo" work from SH01's command runner through the `search` and custom intents. | S | 2 |
| UT02-F114 | Media keys | Any key or the media play key stops a ringing item when the ring view has focus. | S | 1 |
| UT02-F115 | Time-source card | Read-only: synchronised or not, source name, last sync, offset estimate, zone, tz data version. "Open Date and Time settings" hands off to SY01. | S | 2 |
| UT02-F116 | Clock-jump handling | When the wall clock steps (manual change, first sync) all schedules are recomputed and a notice is logged; alarms within the jumped-over span ring once. | M | 1 |

### 4.9 Data, settings, recovery, accessibility

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT02-F120 | Import and export | All data as one JSON file; alarms as iCalendar `VALARM`/`VEVENT` export (C). | S | 2 |
| UT02-F121 | Backup | Data directory is included in `svc.backup` by default. | M | 1 |
| UT02-F122 | Undo | Delete and edits are undoable in the session. | M | 1 |
| UT02-F123 | Large display mode | `F11` shows the current view's main number full screen (clock, timer, stopwatch); screen stays awake only if the user ticks "Keep screen on". | S | 2 |
| UT02-F124 | Spoken time | Shortcut to announce remaining time through `svc.a11y`. | S | 2 |
| UT02-F125 | Reliability self-check | At start and daily the daemon verifies: schedule file readable, sound file reachable, alarm stream not muted, wake timer armed. Failures show in the app and as a notification. | M | 2 |
| UT02-F126 | Fallback sound | If the chosen sound cannot be opened the built-in sound plays. An alarm never fails silently. | M | 1 |
| UT02-F127 | Audit of missed alarms | A missed or late ring (over 5 s) is recorded locally with the cause (suspended, daemon restarted, clock jump). | S | 2 |

## 5. GUI

### 5.1 Window map

```
Clocks
├─ Main window (single-purpose utility, view switch in the header)
│  ├─ World view (cards | map | planner)
│  ├─ Alarms view
│  ├─ Timers view
│  ├─ Stopwatch view
│  └─ Focus view (session | statistics)
├─ Add city popover
├─ Alarm editor dialog
├─ Timer preset editor
├─ Focus profile editor
├─ Bedtime editor
├─ Converter sheet
├─ Ring view (window and lock-screen content)
├─ Large display mode (full screen)
├─ Time-source card (popover from the status area)
├─ Panel applet popover (hosted by SH01)
└─ Preferences
```

### 5.2 Main window

Shown at 900×600, World view, cards.

```
+------------------------------------------------------------------------------------------+
| [=] Clocks      (1)[ World | Alarms | Timers | Stopwatch | Focus ]            [_][o][x] |
+------------------------------------------------------------------------------------------+
| [+ Add city]  [Cards|Map|Planner] (2)        Scrub [-----------o-----------] [Now]  (3) |
+------------------------------------------------------------------------------------------+
|  +--------------------+  +--------------------+  +--------------------+                  |
|  | Vienna  (home)  ☀  |  | Tokyo           ☾  |  | New York        ☀  |             (4) |
|  |      14:32         |  |      22:32         |  |      08:32         |                  |
|  | Thu 12 Nov  UTC+1  |  | Thu 12 Nov  UTC+9  |  | Thu 12 Nov  UTC-5  |                  |
|  | CET                |  | +8 h               |  | -6 h               |                  |
|  +--------------------+  +--------------------+  +--------------------+                  |
|  +--------------------+  +--------------------+                                          |
|  | Sydney office   ☾  |  | UTC                |                                          |
|  |      00:32         |  |      13:32         |                                          |
|  | Fri 13 Nov UTC+11  |  | Thu 12 Nov         |                                          |
|  | +10 h, tomorrow    |  | -1 h               |                                          |
|  +--------------------+  +--------------------+                                          |
|                                                                                          |
+------------------------------------------------------------------------------------------+
| Next alarm 07:00 Fri (in 16 h 28 min) · 1 timer 04:12       (5)[✓ Time synchronised]    |
+------------------------------------------------------------------------------------------+
```

1. View switch (`tabBar` in the header bar). `Ctrl+1`…`Ctrl+5`.
2. `toolButton` plus a three-way segmented `tabBar` for the sub-view.
3. Scrubber (`slider`, ±7 days in 15 min steps) and a Now `button`. While scrubbed, cards show a `--warn` "Not the current time" strip.
4. City cards in a `gridView`; each card is a `panel` with `label`s; analogue faces are drawn with the draw list (custom paint, `UT02-GAP-1`).
5. Status bar: next alarm, running timers (click jumps to the view), and the time-source badge. Badge states: `--ok` synchronised, `--warn` not synchronised, `--warn` unknown.

States: empty (only the home card plus "Add a city to compare times"), loading (none: all data local), error (zone data unreadable: banner, UTC only), narrow (cards one per row, view switch becomes a `comboBox`).

### 5.3 Other views and dialogs

**Alarms view** (900×600)

```
+------------------------------------------------------------------------------------------+
| [+ New alarm]  [7:30 or "in 45m"_______] (1)                    [Bedtime…] [Pause all…] |
+------------------------------------------------------------------------------------------+
|  07:00   Work                      Mon Tue Wed Thu Fri      in 16 h 28 min       [ON ]  |
|  09:30   Weekend                   Sat Sun                  in 2 d 18 h          [ON ]  |
|  22:45   Bedtime: wind down        Every day                in 8 h 13 min        [ON ]  |
|  06:15   Flight                    Once, 20 Nov             skipped once (2)     [ON ]  |
|  12:00   Medication                Every 2 days             off                  [OFF]  |
|                                                                                          |
| ! This computer cannot wake from suspend by itself. Keep it on for alarms.  [Details](3) |
+------------------------------------------------------------------------------------------+
```

1. Quick-add `lineEdit`. 2. Per-row state text. 3. Inline `--warn` message shown only when `svc.power` reports no wake support. Rows are a `modelListView`; the toggle is a `toggleSwitch`. Empty state: "No alarms. [New alarm]".

**Alarm editor dialog** (480×600)

```
+-- Alarm -----------------------------------------------------+
| Time     [07] : [00]                                         |
| Label    [Work______________________]                        |
| Repeat   [Mon][Tue][Wed][Thu][Fri][ Sat][ Sun]  [More… v]    |
| Zone     (•) Local time   ( ) Pinned to [Europe/Vienna v]    |
| Sound    [Dawn v]  [▶]        [Choose file…]                 |
| Volume   [---------o---]  Rise over [30 s v]                 |
| Snooze   [9 min v]   Limit [None v]                          |
| Stop ringing after [10 min v]                                |
| [x] Ring during do-not-disturb                               |
| [x] Wake the computer from suspend                           |
| [ ] Flash the screen                                         |
| Next ring: Fri 13 Nov 07:00 (in 16 h 28 min)                 |
|                         [Delete]      [Cancel]  [Save]       |
+--------------------------------------------------------------+
```

`spinBox` pair for time, `lineEdit`, seven `toolButton` toggles, `radioButton`s, `comboBox`es, `slider`, `checkBox`es. "Next ring" is computed live. Delete is outlined `--danger`.

**Timers view** (900×600)

```
+------------------------------------------------------------------------------------------+
| [25m tea_____________] [Start]    Presets: [1] [3] [5] [10] [15] [30] [Eggs 6:30] [Edit] |
+------------------------------------------------------------------------------------------+
|  +-------------------------+  +-------------------------+  +-------------------------+   |
|  |  Tea                    |  |  Laundry                |  |  Pasta          RINGING |   |
|  |      ( 04:12 )          |  |      ( 31:40 )          |  |      ( -00:23 )         |   |
|  |  ends 14:36             |  |  ends 15:04   paused    |  |  ended 14:32            |   |
|  |  [||] [+1] [+5] [Reset] |  |  [▶]  [+1] [+5] [Reset] |  |  [Stop] [+1] [Restart]  |   |
|  +-------------------------+  +-------------------------+  +-------------------------+   |
+------------------------------------------------------------------------------------------+
```

Cards in a `gridView`; the ring is a `dial`-like progress drawn by custom paint (`UT02-GAP-1`) with a `progressBar` fallback. A ringing card has a `--warn` border and counts up.

**Stopwatch view** (900×600)

```
+------------------------------------------------------------------------------------------+
|                                   00:12:47.31                                            |
|                          [Stop  Space]  [Lap  L]  [Reset  R]                             |
+------------------------------------------------------------------------------------------+
| Lap   Lap time     Split        Difference                                               |
|  4    00:03:02.10  00:12:31.90  -00:00:11.05   fastest                                   |
|  3    00:03:13.15  00:09:29.80  +00:00:01.40                                             |
|  2    00:03:11.75  00:06:16.65  +00:00:06.85   slowest                                   |
|  1    00:03:04.90  00:03:04.90                                                           |
|                                                                 [Copy] [Export CSV…]     |
+------------------------------------------------------------------------------------------+
```

`label` (display size, tabular figures), `button`s, `tableView`.

**Planner sub-view** (900×600)

```
+------------------------------------------------------------------------------------------+
| Day [Fri 13 Nov 2026 v]   Length [1 h v]            Legend: ▓ working  ▒ awake  ░ night |
+------------------------------------------------------------------------------------------+
|            00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23       |
| Vienna     ░░ ░░ ░░ ░░ ░░ ░░ ░░ ▒▒ ▒▒ ▓▓ ▓▓ ▓▓ ▓▓ ▓▓[▓▓]▓▓ ▓▓ ▒▒ ▒▒ ▒▒ ▒▒ ▒▒ ░░ ░░       |
| New York   ▒▒ ▒▒ ▒▒ ▒▒ ░░ ░░ ░░ ░░ ░░ ░░ ░░ ░░ ░░ ▒▒[▒▒]▓▓ ▓▓ ▓▓ ▓▓ ▓▓ ▓▓ ▓▓ ▓▓ ▒▒       |
| Tokyo      ▒▒ ▓▓ ▓▓ ▓▓ ▓▓ ▓▓ ▓▓ ▓▓ ▓▓ ▒▒ ▒▒ ▒▒ ▒▒ ▒▒[▒▒]░░ ░░ ░░ ░░ ░░ ░░ ░░ ░░ ▒▒       |
|                                                                                          |
| Selected 14:00–15:00 Vienna = 08:00–09:00 New York = 22:00–23:00 Tokyo                   |
| No hour is inside working hours everywhere. Least bad: 14:00, 15:00, 08:00 (Vienna).     |
|                                                   [Copy times]  [Create event in Calendar]|
+------------------------------------------------------------------------------------------+
```

A `tableView` with a cell delegate (`setTableDelegate`, Verified in `FUSE:docs/ui/README.md`) painting the shades; each cell's accessible name says the state in words. Columns are in the home zone.

**Focus view**: large phase label and countdown, round dots (● ● ○ ○), `[Start] [Skip] [+5]`, profile `comboBox`, task `lineEdit`; a Statistics sub-tab with a `chart` (bar, minutes per day) and totals.

**Ring view** (window 520×360; same content on the lock screen)

```
+--------------------------------------------------------------+
|                          07:00                               |
|                           Work                               |
|                                                              |
|          [ Snooze 9 min ]            [ Stop ]                |
|                                                              |
|          Space or Enter stops · S snoozes                    |
+--------------------------------------------------------------+
```

Buttons are 64 px high. On the lock screen SH03 draws this from data the daemon supplies; Stop and Snooze work without unlocking and nothing else is reachable.

**Converter sheet** (560×360): `lineEdit` free text, or `dateTimeEdit` plus zone `comboBox`; results `listView` (local first, then saved cities); a `--warn` row when the time is ambiguous or does not exist.

**Time-source card** (popover, 340 wide): rows Synchronised yes/no, Source, Last sync, Offset, Time zone, Zone data version; `[Open Date and Time settings]`.

**Panel applet popover** (320×380, hosted by SH01): next alarm with toggle; timer rows with pause and stop; quick timer field; `[Start focus]`; two pinned world clocks; `[Open Clocks]`.

### 5.4 Menus

```
File    New Alarm Ctrl+N (Alarms) / Add City Ctrl+N (World) / New Timer Ctrl+N (Timers)
        Import… · Export… · Close Ctrl+W · Quit Ctrl+Q
Edit    Undo Ctrl+Z · Redo Ctrl+Shift+Z · Copy Time Ctrl+C · Duplicate Ctrl+D · Delete Delete
        Rename F2 · Preferences Ctrl+,
View    World Ctrl+1 · Alarms Ctrl+2 · Timers Ctrl+3 · Stopwatch Ctrl+4 · Focus Ctrl+5
        Cards / Map / Planner · Analogue Faces · Show Seconds · Large Display F11 · Converter… Ctrl+T
Clock   Start/Pause Space · Lap L · Reset R · Add 1 Minute + · Stop Ringing Esc · Snooze S
        Skip Next Alarm · Pause All Alarms… · Start Focus Ctrl+Enter · Bedtime…
Help    Clocks Help F1 · Time Source Status · About
```

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| `Ctrl+1…5` | Switch view |
| `Space` | Start/pause the focused timer or the stopwatch; stop a ring |
| `L`, `R` | Lap, reset (stopwatch) |
| `+`, `Shift++` | Add 1 min, 5 min to the focused timer |
| `S` | Snooze a ringing alarm |
| `Enter` in quick field | Create and start |
| `Ctrl+T` | Converter |
| Arrow keys in planner | Move the selected hour; `Shift+Arrow` extends |

Context menus: city card (Rename, Copy Time, Set Working Hours…, Show on Map, Pin to Applet, Remove); alarm (Edit, Skip Next, Duplicate, Delete); timer (Rename, Save as Preset, Repeat, Delete).

Drag and drop: reorder cards and presets; drop an audio file on an alarm to set its sound (handle from the drag offer); drag a planner selection as text to another app.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Each view has its one-line empty state and one button. |
| Loading | None; first frame is complete. |
| Error | Schedule store unreadable: banner, last good backup offered, alarms rebuilt from backup. Zone data missing: UTC only, banner. |
| Offline | Always; no network use. |
| Locked session | Ring view on the lock screen; nothing else. |
| Daemon not running | `--danger` banner "Alarms will not ring" with `[Start]`; the app also tries to start it. |
| Narrow (<640) | View switch becomes a dropdown; cards single column; planner scrolls horizontally with the city column frozen. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| View switch | `tabBar` |
| City, timer cards | `gridView`, `panel`, `label`, `toolButton` |
| Alarm list | `modelListView`, `toggleSwitch` |
| Editors | `dialog`, `spinBox`, `lineEdit`, `comboBox`, `slider`, `checkBox`, `radioButton`, `dateTimeEdit`, `calendar` |
| Laps, planner | `tableView` with cell delegate |
| Statistics | `chart` |
| Scrubber | `slider` |
| Map | `image` under custom-painted overlay |

Missing: a custom-paint canvas node for analogue faces, ring progress, the terminator and the bedtime dial (`UT02-GAP-1`; expected from FUSE-WP-23 `Painter`); popover and sheet (FUSE-WP-30); the applet hosting interface in SH01 (`UT02-GAP-2`, owned by SH01).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | App host (window) | Views, editors, converter, planner, statistics |
| UI | FUSE UI | App host | All views |
| `clocksd` (schedule daemon) | ForgeScript, headless host | Own process, `bg.autostart` | Owns the schedule; arms timers and wake timers; rings; talks to `svc.notify`, `svc.power`, `svc.media`, SH02, SH03 |
| Zone and place tables | data files in the bundle | read by both | Place names → zone ids; zone rules come from system tz data |
| Sun and terminator maths | ForgeScript | App host | Pure functions |
| Applet view | ForgeScript + FUSE UI | SH01-hosted surface | Popover content, talks to `clocksd` over the bus |

```
 Main window (app host) ──bus org.pbsd.clocks.v1──► clocksd (headless host, autostart)
 SH01 applet surface  ───────────────────────────►    │
                                                      ├─► svc.power  (wake timer, inhibit)
                                                      ├─► svc.notify (ring, actions, DND lease via SH02)
                                                      ├─► svc.media  (alarm stream, routing)
                                                      └─► SH03 (ring content on the lock screen)
```

No native helper is needed. Sound files are decoded by `svc.media`, not by this app. The daemon is small on purpose: it keeps one sorted list of next instants and sleeps until the earliest.

Time rules: schedules are stored as wall-clock rules and resolved to instants with the zone database each time the list is rebuilt (at start, on change, on zone change, on clock step, at each DST transition). Running timers and the stopwatch use the monotonic clock that counts through suspend.

### 6.2 Data model

Store: `~/.local/share/org.pbsd.clocks/clocks.sqlite` (`meta.schema_version`), written by `clocksd` only; the window sends edits over the bus. A copy `clocks.sqlite.bak` is refreshed after each successful write batch.

| Entity | Fields |
|---|---|
| `City` | `id: u32`, `place_id: u32?`, `label: string`, `zone: string` (IANA id or `UTC±hh:mm`), `lat: f32?`, `lon: f32?`, `order: u16`, `work_start: u16 min`, `work_end: u16 min`, `work_days: u8 bitset`, `pinned_applet: bool` |
| `Alarm` | `id: u32`, `label: string`, `time: u16 min`, `repeat: RepeatRule`, `zone: string?` (null = local), `enabled: bool`, `skip_until: i64?`, `sound: SoundRef`, `volume: u8`, `ramp_s: u16`, `snooze_min: u8`, `snooze_limit: u8`, `silence_min: u8`, `through_dnd: bool`, `wake: bool`, `flash: bool`, `kind: enum{normal, bedtime_wake, bedtime_winddown}` |
| `RepeatRule` | `kind: enum{once, daily, weekdays_mask, every_n_days, monthly_day, dates}`, `mask: u8`, `n: u16`, `dates: list<i32 day>`, `anchor: i32 day` |
| `SoundRef` | `kind: enum{builtin, portal}`, `name: string`, `grant_token: string?` |
| `Timer` | `id: u32`, `name: string`, `duration_ms: u64`, `state: enum{idle, running, paused, ringing}`, `end_mono_ns: u64`, `end_wall: i64`, `remaining_ms: u64`, `repeat: u16`, `finish: enum{ring, notify, stop_media}` |
| `Preset` | `id`, `name`, `duration_ms`, `order` |
| `Stopwatch` | `id`, `name`, `state`, `start_mono_ns`, `accum_ns`, `laps: list<u64 ns>` |
| `FocusProfile` | `id`, `name`, `work_min`, `short_min`, `long_min`, `rounds`, `auto_break: bool`, `auto_work: bool` |
| `FocusSession` | `id`, `profile_id`, `task: string`, `started: i64`, `work_ms: u64`, `rounds_done: u8`, `completed: bool` |
| `Bedtime` | `weekday: u8`, `bed: u16 min`, `wake: u16 min`, `winddown_min: u8`, `dnd: bool`, `enabled: bool` |
| `RingLog` | `id`, `item_kind`, `item_id`, `due: i64`, `rang: i64?`, `outcome: enum{stopped, snoozed, auto_silenced, missed}`, `cause: string` |

Because monotonic values do not survive a reboot, running timers also store `end_wall`; after boot the daemon rebuilds from `end_wall`.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| IANA tz database (compiled zone files on the system; source in `hbsd/src/contrib/tzdata`) | yes | no | FORGE time library (Proposed, FORGE-WP-23) or own TZif reader in ForgeScript | App host and daemon (trusted system data, read-only) |
| Place table (`places.tbl`: binary, sorted names + zone index) | yes | no | own; built offline from an open-data gazetteer (licence to be recorded; CC BY expected) | App host |
| World outline (pre-tessellated vector, own format) | yes | no | own | App host |
| Clocks export JSON | yes | yes | own, strict JSON | App host (user-picked file; size cap 4 MiB) |
| iCalendar export | no | yes | own writer | App host |
| CSV (laps, statistics) | no | yes | own | App host |
| Audio files for alarm sounds | — | — | never parsed here; the handle is passed to `svc.media` | media service sandbox |
| Bus `org.pbsd.clocks.v1` | — | — | schema compiler (G-08) | both |

### 6.4 Integration

- Intents handled: `new` (alarm, timer), `search` ("timer 10m", "time in Tokyo"), custom `clocks.start-timer`, `clocks.start-focus`.
- Intents emitted: `compose` to CO02 (event from the planner), `pick` to CO04 (task label), `open` to SY01 (Date and Time page).
- Services: `svc.settings`, `svc.notify`, `svc.power`, `svc.media`, `svc.portal.file`, `svc.backup`, `svc.a11y`, `svc.audit` (missed alarms).
- Offers `org.pbsd.clocks.v1`: list and edit items, start/stop, subscribe to next-ring. Used by the window, the SH01 applet and SH03.
- Links: SH01 (applet, command runner), SH02 (do-not-disturb lease, critical notifications), SH03 (lock-screen ring, shutdown notice), SY01 (time settings, time-source data), CO02 (events, holiday dates), CO04 (task labels).
- Repo facts: `hbsd/src/contrib/tzdata` and `hbsd/src/usr.sbin/ntp` exist in the tree (Verified by listing). Which time daemon PBSD runs and how SY01 exposes its status is not specified anywhere read here (Unverified); F115 depends on it.

## 7. Security and privacy

| Capability | Why |
|---|---|
| `fs.app` | Schedule store, statistics |
| `fs.portal` | A sound file the user picks; import and export |
| `net.none` | No network use |
| `svc.settings` | Preferences; read system zone and clock format |
| `svc.notify` | Ring notifications, pre-alarm notices, progress badge, do-not-disturb lease |
| `svc.power` | Wake timers; sleep inhibitor while ringing |
| `svc.media` | Alarm audio stream, output routing, stop playback for sleep timers |
| `svc.portal.file` | Sound, import, export |
| `svc.intent` | Launcher phrases, hand-offs |
| `svc.backup` | Include the store |
| `svc.a11y` | Spoken time, live regions |
| `svc.audit` | Missed-alarm records |
| `bg.run`, `bg.autostart` | The schedule daemon must run with no window and from login |

Not requested: `dev.location` (cities are typed, never detected), any `net.*`, `proc.*`, `ai.local`.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| The user being woken on time | Another app | Mute or lower the alarm stream, cancel the wake timer, hold do-not-disturb | Alarm stream is separate; only this app's daemon and the user can change it; critical notifications pass do-not-disturb; self-check (F125) |
| Do-not-disturb state | This app, if buggy | Leaving do-not-disturb on for ever | Lease with expiry held by SH02, renewed each minute |
| Lock screen | Person with physical access | Use the ring view to reach the session | Ring view exposes only Stop and Snooze; label text is plain text; SH03 draws it |
| Sleep and work habits | Other apps | Read the schedule or focus statistics | Store is in `fs.app`; bus interface is limited to SH01, SH03 and this bundle by peer identity |
| Daemon | Malformed import file | Crash or huge allocation | Strict JSON, size cap, field ranges checked, import happens in the window process, edits reach the daemon as typed bus messages |
| Wake-ups | This app | Battery drain from needless wakes | One wake timer for the earliest item only; none when nothing is enabled |
| Time itself | Network attacker moving the clock | Alarms early or late | Out of this app's control; clock steps are detected and shown (F116); source status is shown (F115) |

Stored: cities, alarms, timers, focus history, ring log. Nothing leaves the machine.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `general.start_view` | enum | `last` | world, alarms, timers, stopwatch, focus, last |
| `general.show_seconds` | bool | false | Seconds on clocks |
| `general.faces` | enum | `digital` | digital, analogue |
| `general.hour_format` | enum | `system` | system, 12, 24 |
| `world.sort` | enum | `manual` | manual, offset, name |
| `world.show_sun` | bool | false | Sunrise and sunset on cards |
| `alarm.default_sound` | string | `Dawn` | Built-in sound name |
| `alarm.default_volume` | u8 | 70 | Percent |
| `alarm.ramp_s` | u16 | 30 | Volume rise time |
| `alarm.snooze_min` | u8 | 9 | Default snooze |
| `alarm.silence_min` | u8 | 10 | Auto-silence |
| `alarm.missed_window_min` | u8 | 15 | Ring late alarms inside this window after boot |
| `alarm.wake_lead_s` | u16 | 30 | Wake this long before |
| `alarm.through_dnd` | bool | true | Default for new alarms |
| `timer.presets` | list | 1,3,5,10,15,30 | Preset minutes |
| `timer.wake` | bool | true | Wake timers for timers over 5 min |
| `timer.default_finish` | enum | `ring` | ring, notify, stop_media |
| `stopwatch.precision` | enum | `centi` | centi, deci, second |
| `focus.default_profile` | string | `Classic` | 25/5/15 × 4 |
| `focus.dnd` | bool | true | Do-not-disturb during work phases |
| `focus.pause_on_lock` | bool | false | Pause when the session locks |
| `bedtime.enabled` | bool | false | Bedtime schedule on |
| `large.keep_screen_on` | bool | false | In large display mode |
| `applet.cities` | list<u32> | home | Cities shown in the applet |

## 9. Accessibility and languages

- Running numbers are not live regions (they would flood speech). A shortcut reads the current value; milestones (1 min left, finished) are announced.
- Ring view buttons are 64 px and reachable by any key; the flash option serves deaf users; no information is carried by sound alone.
- Planner shades use pattern as well as colour and each cell names its state.
- Weekday order, first day of week, 12/24-hour form, digits and calendar follow the locale. City names are shown in the UI language where the table has them, with search across all stored names.
- Analogue faces have a text equivalent.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | 150 ms |
| Window memory | 40 MiB |
| Daemon memory | 12 MiB |
| Daemon wake-ups | One per scheduled item; zero periodic polling |
| Window redraws | 1 per minute (clocks without seconds), 1 per second (with seconds or timers), display rate only for a visible running stopwatch; zero when hidden |
| Alarm latency | Ring starts within 1 s of the due instant when awake; within 5 s of resume when woken |
| City search | Results under 20 ms per key over 25 000 places |
| Capacity | 500 alarms, 100 timers, 10 000 laps |

## 11. Testing and proof

- **Unit**: repeat rules (next occurrence for every rule kind), DST gap and overlap, zone change, clock step, leap day, month-end rules, quick-entry parser, free-text converter parser, sun maths against published tables.
- **Simulated clock**: the daemon runs against an injected clock and power model; scripted years of schedule are replayed, including suspend, reboot and clock jumps, and every due item must ring exactly once.
- **Golden**: every view in dark, light, high contrast, 200%, right-to-left.
- **Scripted UI**: T1–T12 through the a11y tree.
- **Differential**: zone conversions against the system `date` and zone files for 10 000 random instants per zone.
- **Fuzz**: import JSON, quick-entry and free-text parsers.
- **Hardware matrix**: wake-from-suspend on each supported machine class; recorded per model.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Next-occurrence function | Returns the least instant after `now` matching the rule; total; never in the past | T1 | PRISM proof over a model of civil time with zone transitions as data | Agreement on the civil-time model |
| Scheduler core | Every enabled item is armed; no item rings twice for one occurrence | T2 | PRISM plus simulated-clock replay | Interpreter tier (FORGE) |
| Timer arithmetic | No overflow; remaining time never negative while running | T2 | PRISM | — |
| Parsers (quick entry, import) | No crash on any input | T3 | Fuzzing | — |
| UI logic | Memory-safe by language | T3 | Unit and UI tests | — |
| Wake path | Machine wakes | T4 | Hardware tests only | Depends on firmware |

Nothing here has been run.

## 12. Build plan

Phase 1: usable core.

| ID | Work item | Depends on |
|---|---|---|
| UT02-W01 | Civil-time module: zone reader, rule resolution, next-occurrence | FORGE-WP-23 |
| UT02-W02 | Schedule daemon skeleton with store and bus interface | G-07, G-08, FORGE-WP-10 (headless host), FORGE-WP-17 |
| UT02-W03 | Alarms: list, editor, ring view, snooze, auto-silence, notifications | G-02, G-09 (`svc.notify`, `svc.media`), SH02 |
| UT02-W04 | Timers: multiple, quick entry, presets | UT02-W02 |
| UT02-W05 | Stopwatch with laps | G-02 |
| UT02-W06 | World clock cards, place table, add city, scrubber | UT02-W01 |
| UT02-W07 | Converter with gap/overlap handling | UT02-W01 |
| UT02-W08 | Restart and clock-step recovery; simulated-clock test rig | UT02-W02 |

Phase 2: full feature.

| ID | Work item | Depends on |
|---|---|---|
| UT02-W09 | Wake from suspend, self-check, missed-alarm log | G-09 (`svc.power`), hardware matrix |
| UT02-W10 | Lock-screen ring | SH03 |
| UT02-W11 | Focus sessions, do-not-disturb lease, statistics | SH02 |
| UT02-W12 | Bedtime | UT02-W03 |
| UT02-W13 | Map with terminator, sun times, analogue faces, ring progress | `UT02-GAP-1` (FUSE-WP-23) |
| UT02-W14 | Meeting planner, event hand-off | CO02 |
| UT02-W15 | Panel applet and indicators | SH01 (`UT02-GAP-2`) |
| UT02-W16 | Time-source card | SY01 |
| UT02-W17 | Import/export, launcher phrases, large display mode | SH01 |

Phase 3: advanced.

| ID | Work item | Depends on |
|---|---|---|
| UT02-W18 | Timer sequences, several stopwatches, lap chart | UT02-W04 |
| UT02-W19 | Holiday skip from a calendar; task label pick | CO02, CO04 |
| UT02-W20 | Dismiss challenge, shutdown notice | SH03 |

## 13. Open questions and risks

- Wake from suspend depends on firmware and on `svc.power` offering wake timers; neither is specified in anything read. Without it, a suspended machine misses alarms. The app must say so plainly (F044).
- The daemon is ForgeScript in a headless host. If the headless host (FORGE-WP-10.1) is late, a small C++23 daemon is the fallback, with the same bus schema.
- How SH03 draws third-party content on the lock screen is undefined; the proposal is data-only (time, label, two actions), never app-drawn UI.
- The alarm audio stream and its exemption from mute must be defined in `svc.media`.
- Licence of the place table source must be checked before bundling.
- Leap seconds are ignored (POSIX time). Stated in help.
