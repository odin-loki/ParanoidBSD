<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Tasks

By Odin Loch

| | |
|---|---|
| ID | `CO04` (from CATALOGUE.md) |
| Bundle | `org.pbsd.tasks` |
| Category | comms |
| Priority | P2 |
| Backing repos | FUSE (widgets), FORGE (ForgeScript). Shares the recurrence engine, iCalendar service and DAV helper planned in CO02. No task code exists in any repo read. |
| Status | Planned. Nothing built. |

## 1. Purpose

Tasks keeps the user's to-dos and small projects: lists, subtasks, dates, repeats, a board, and reminders. It stores everything locally, syncs as CalDAV VTODO, and reads and writes todo.txt and Markdown so the data is never locked in. It replaces the to-do part of KOrganizer from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- Lists, projects with sections, subtasks to any depth, tags, priorities, start and due dates.
- Recurrence by fixed rule and by "after completion".
- List, board (kanban), and a calendar hand-off; smart lists; saved filters.
- Quick add with a natural-language grammar; global quick-add window.
- Reminders through `svc.notify`.
- CalDAV VTODO sync; todo.txt and Markdown import, export and live file link.
- Optional time tracking; templates; attachments; links to mail, notes, files, events.

Out of scope:

| Item | Owner |
|---|---|
| Calendar views of time (day, week, month) | CO02 Calendar (shows a tasks overlay; Tasks opens it) |
| Events and invitations | CO02 Calendar |
| Long-form notes and wikis | OF02 Notes (Tasks links to notes) |
| Mail | CO01 Mail (a message becomes a task by intent) |
| Alarms and timers not tied to a task | UT02 Clocks |
| Team servers, assignment workflows, comments feeds, Gantt resource levelling | Not planned. Sharing is by CalDAV shared lists or by file. |
| Issue trackers (forge APIs) | Not planned here; DV03 may link out |

## 3. Users and top tasks

Personas:

- **Rae, daily user.** Shopping list, home list, a work list synced by CalDAV to a phone.
- **Ines, planner.** Projects with sections and a board; weekly review; templates for repeated procedures.
- **Tam, plain-text user.** Keeps `todo.txt` in a synced folder and wants a good front end to it.
- **Sol, freelancer.** Tracks time per task and exports a timesheet.

| # | Task | Target steps |
|---|---|---|
| T1 | Add a task from anywhere | 2 (global shortcut, type "Call Sol tomorrow 10am #work !2", Enter) |
| T2 | Add a task in the open list | 1 (`N` or just type in the add row, Enter) |
| T3 | Complete a task | 1 (click the box or `Space`) |
| T4 | See what is due today | 0 (Today is the start view) |
| T5 | Postpone a task to tomorrow | 1 key (`Shift+.` then `T`) or one drag |
| T6 | Add a subtask | 1 (`Tab` on a new row under the parent) |
| T7 | Move a card on the board | 1 drag or `Ctrl+Right` |
| T8 | Make a mail into a task | 1 in CO01, 0 here (toast with Edit) |
| T9 | Find a task | 2 (`/`, type) |
| T10 | Start and stop a timer on a task | 1 key (`Ctrl+T`) |
| T11 | Create a project from a template | 3 |
| T12 | Snooze a reminder | 1 click on the notification |
| T13 | Open a `todo.txt` file and edit it live | 2 (File > Link file, pick) |
| T14 | Undo an accidental completion or delete | 1 (`Ctrl+Z`) |

## 4. Features

### 4.1 Tasks and structure

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO04-F001 | Task fields | Title, notes (Markdown), list, section, parent, status, priority, start, due, completed time, percent, tags, repeat, reminders, estimate, location text, URL, attachments, links, colour. | M | 1 |
| CO04-F002 | Status model | `needs-action`, `in-process`, `completed`, `cancelled` (VTODO `STATUS`), plus a local `waiting` flag stored as a tag. | M | 1 |
| CO04-F003 | Lists | Any number, with colour and icon; folders of lists in the sidebar; archive a list. | M | 1 |
| CO04-F004 | Projects and sections | A list may have ordered sections (headings); sections are the default board columns. | M | 1 |
| CO04-F005 | Subtasks | Unlimited depth by `RELATED-TO;RELTYPE=PARENT`; collapse and expand; progress "2 of 5" on the parent; completing a parent offers to complete children. | M | 1 |
| CO04-F006 | Manual order | Drag order within a list or section, stored as a fractional rank and synced as `X-APPLE-SORT-ORDER`. | M | 1 |
| CO04-F007 | Priorities | None, low, medium, high (iCalendar `PRIORITY` 0, 9, 5, 1); shown as a coloured flag plus `!`, `!!`, `!!!`. | M | 1 |
| CO04-F008 | Tags | Free tags (`CATEGORIES`), colour per tag, nested by `/` ("work/client-a"). | M | 1 |
| CO04-F009 | Dates | Due and start as date or date-time, with or without a zone (floating default); duration instead of due allowed on import. | M | 1 |
| CO04-F010 | Deferred tasks | A task with a future start date is hidden from active views until that date ("defer until"). | S | 2 |
| CO04-F011 | Notes | Markdown with check-list items, rendered by `markdownView`; plain `DESCRIPTION` on the wire. | M | 1 |
| CO04-F012 | Checklist inside a task | Lightweight check items in notes that are not full subtasks; convert either way. | S | 2 |
| CO04-F013 | Estimates | Estimated duration per task; sums per section and list. | S | 2 |
| CO04-F014 | Dependencies | "Blocked by" links (`RELATED-TO;RELTYPE=DEPENDS-ON`, RFC 9253); blocked tasks are dimmed and excluded from Next. | C | 3 |
| CO04-F015 | Inline edit | Edit the title in the row (`F2` or `Enter`); other fields in the inspector; saves on leave. | M | 1 |
| CO04-F016 | Multi-select edit | Change list, date, priority, tags, status for many at once. | M | 1 |
| CO04-F017 | Complete, cancel, reopen | Completion animates for 140 ms then moves to Completed; reduced motion skips it. | M | 1 |
| CO04-F018 | Delete and trash | To trash with Undo; 30 days; restore. | M | 1 |
| CO04-F019 | Undo and redo | Every change in the session, including drags, bulk edits and completion of repeats. | M | 1 |
| CO04-F020 | Duplicate and convert | Duplicate a task or subtree; convert task to project (subtasks become tasks, sections from first level); convert to event (hand-off to CO02). | S | 2 |
| CO04-F021 | Change history | Per task: created, edits, completions; restore an earlier version. | C | 3 |

### 4.2 Views

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO04-F022 | List view | Tree rows: check box, title, tag chips, due chip, priority flag, subtask count, attachment and repeat icons; add row at top or bottom. | M | 1 |
| CO04-F023 | Grouping and sorting | Group by section, due date, priority, tag, list, status; sort by manual, due, start, priority, title, created, changed. | M | 1 |
| CO04-F024 | Board view | Columns by section (default), status, priority, tag or due bucket; cards show title, chips, progress; drag between columns writes the field; optional work-in-progress limit per column (warns, does not block). | M | 2 |
| CO04-F025 | Board swimlanes | Rows by priority, tag or parent project. | C | 3 |
| CO04-F026 | Today | Overdue, due today, starting today, plus tasks pinned to today; an evening section for tasks with a time after 18:00. | M | 1 |
| CO04-F027 | Upcoming | Next 7 or 30 days grouped by day; drag between days to reschedule. | M | 1 |
| CO04-F028 | Smart lists | Built in: Inbox, Today, Upcoming, Next (first open task of each project), Anytime (no date), Waiting, Overdue, Completed, Trash. | M | 1 |
| CO04-F029 | Custom smart lists | Saved filter query with icon and colour in the sidebar. | S | 2 |
| CO04-F030 | Filter query language | `list:`, `tag:`, `prio:>=2`, `due:<today+7d`, `start:`, `is:overdue/done/open/repeating/blocked/waiting`, `has:notes/attachment/subtasks`, `text:`, `created:`, `AND`/`OR`/`NOT`, parentheses. | S | 2 |
| CO04-F031 | Calendar hand-off | "Show in Calendar" opens CO02 at the task's date with the tasks overlay on. Tasks does not draw time grids. | M | 2 |
| CO04-F032 | Month strip | A compact `calendar` widget in the sidebar: dots on days with due tasks; click filters Upcoming to that day. | S | 2 |
| CO04-F033 | Timeline (Gantt-like) view | Bars from start to due per task on a day scale, dependency arrows; drag to shift; uses the `timeline` widget. | C | 3 |
| CO04-F034 | Focus mode | One task full window with notes, subtasks and timer; everything else hidden. | C | 3 |
| CO04-F035 | Completed log | Completed tasks by day with counts; search; reopen. | S | 2 |
| CO04-F036 | Show or hide completed | Per view toggle; completed sink to the bottom or vanish after N hours. | M | 1 |
| CO04-F037 | Density and columns | Compact, normal; optional table mode with columns (due, start, estimate, spent, tags) via `tableView`. | S | 2 |
| CO04-F038 | Review mode | Weekly review walker: steps through each project, shows stalled ones (no open next task), old Inbox items, and overdue; marks project "reviewed" with a date. | C | 3 |
| CO04-F039 | Print | List, board snapshot or checklist with boxes, to `svc.portal.print`. | C | 3 |

### 4.3 Quick add and capture

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO04-F040 | Natural-language quick add | Deterministic grammar per locale: date and time words ("tomorrow", "fri 3pm", "in 2 weeks", "1 Dec"), `#tag`, `@list` or `/list`, `!1..3` priority, `*` repeat ("every monday", "every 2 weeks", "monthly on the 1st", "after 3 days"), `~30m` estimate, `^` start date, `rem 1h` reminder. Parsed parts become chips; `Backspace` on a chip turns it back to text. | M | 1 |
| CO04-F041 | Global quick-add window | A small window opened by a system shortcut (registered through the shell) or the `new` intent; adds to Inbox or the parsed list and closes. | M | 2 |
| CO04-F042 | Multi-line paste | Pasting several lines offers "Add N tasks"; indented lines become subtasks; `- [ ]` and todo.txt syntax recognised. | S | 2 |
| CO04-F043 | Capture from other apps | `new` and `share` intents: text, URL, file handle, `mid:` link from CO01, note link from OF02, event link from CO02. A toast confirms with Edit and Undo. | M | 2 |
| CO04-F044 | Model-assisted capture | Optional `svc.model` pass that splits pasted prose into tasks; always a preview; off by default. | C | 3 |
| CO04-F045 | Inbox | Default landing list for captures; "process Inbox" walks items one by one with keys for list, date, delete. | S | 2 |
| CO04-F046 | Shared date parser | The date grammar module is the one CO02 uses, so both apps read the same phrases the same way. | M | 1 |

### 4.4 Recurrence

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO04-F047 | Fixed-schedule repeat | Full RRULE through the CO02 recurrence engine; on completion the next instance's due and start move to the next rule date. | M | 1 |
| CO04-F048 | After-completion repeat | "Every N days/weeks/months after completion"; stored as `X-PBSD-REPEAT-AFTER` plus a compatible `RRULE` for other clients. | M | 2 |
| CO04-F049 | Completion model | One VTODO per series: completing advances dates and records the completion in a local log (interoperable with clients that do the same). Option per list: write `RECURRENCE-ID` overrides instead. | M | 1 |
| CO04-F050 | Subtasks on repeat | Subtasks of a repeating task reset when the parent advances. | S | 2 |
| CO04-F051 | Skip and catch-up | "Skip this occurrence"; after a long gap, one overdue instance is shown, not one per missed date. | M | 2 |
| CO04-F052 | Repeat editor | Simple menu and the custom form shared in design with CO02; summary in words; next 5 dates. | M | 1 |
| CO04-F053 | End conditions and streak | Until date or count; a streak counter for repeating tasks (local). | C | 3 |

### 4.5 Reminders

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO04-F054 | Reminders | VALARM relative to due or start, or absolute; several per task. | M | 1 |
| CO04-F055 | Notification actions | Complete, Snooze (10 min, 1 h, tomorrow 09:00, pick), Open. | M | 1 |
| CO04-F056 | Default reminder | Per list: none, at due time, a fixed time on the due day for date-only tasks (default 09:00). | M | 1 |
| CO04-F057 | Daily summary | Optional notification at a set time: "5 due today, 2 overdue". | S | 2 |
| CO04-F058 | Reminder daemon | Background entry that holds next fire times only (`bg.run`, `bg.autostart`); titles hidden on the lock screen unless allowed. | M | 1 |
| CO04-F059 | Missed reminders | Shown once, grouped, after start or resume. | M | 1 |
| CO04-F060 | Overdue nagging control | Overdue tasks do not re-notify unless `reminder.repeat_overdue` is set. | S | 2 |
| CO04-F061 | Badge | Count of due-today and overdue on the launcher icon; can be turned off. | S | 2 |

### 4.6 Sync and files

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO04-F062 | Local lists | Default; no account needed. | M | 1 |
| CO04-F063 | CalDAV VTODO | Collections that support VTODO from the `svc.accounts` record shared with CO02 and CO03; `sync-collection` or CTag/ETag; `If-Match` writes; offline journal. | M | 2 |
| CO04-F064 | Conflict resolution | Property-level three-way merge; a sheet only for same-property conflicts; completion wins over edits of the same task when both happened. | M | 2 |
| CO04-F065 | Interoperable extras | Reads and writes the de-facto properties other clients use for order, sections and subtasks (`X-APPLE-SORT-ORDER`, `RELATED-TO`), keeps unknown properties byte-for-byte. | M | 2 |
| CO04-F066 | Sections on the wire | Sections stored as `X-PBSD-SECTION` on the task and a list-level order property; other clients ignore them without loss. | S | 2 |
| CO04-F067 | Shared lists | CalDAV shared collections shown with their ACL state; read-only lists disable writes. | S | 3 |
| CO04-F068 | todo.txt import and export | Full todo.txt format: `x`, completion and creation dates, `(A)` priority, `+project`, `@context`, `key:value` (`due:`, `t:`, `rec:`, `pri:`, `h:`). | M | 2 |
| CO04-F069 | todo.txt live link | A list bound to a `todo.txt` file handle (and `done.txt`): the file is the store; changes on disk are reloaded; writes keep line order and unknown tokens; atomic replace. | S | 2 |
| CO04-F070 | Markdown import and export | GitHub-style task lists (`- [ ]`, `- [x]`), headings as sections, indentation as subtasks, inline metadata (`📅 2026-11-20`, `due:2026-11-20`, `#tag`, `!!`). | M | 2 |
| CO04-F071 | Markdown live link | A list bound to one `.md` file handle; only task lines are edited; other text is preserved exactly. | S | 3 |
| CO04-F072 | Other formats | Import and export iCalendar `.ics` (VTODO), CSV, JSON (own documented schema); import Taskwarrior JSON export. | S | 2 |
| CO04-F073 | Encrypted lists | A local list with a store key in `svc.keyring`; locked lists hide titles in the app, notifications and search. | S | 3 |
| CO04-F074 | Per-account route | Inherited from the account record (`clear`, `vpn`, `tor`); fail-closed. | S | 2 |
| CO04-F075 | Background sync | Timer and on network return; honours `svc.power`. | S | 2 |
| CO04-F076 | Sync status and log | Per list. | S | 2 |

### 4.7 Time tracking (optional)

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO04-F077 | Enable per user | Off by default; when off, no timer UI is shown. | S | 3 |
| CO04-F078 | Timer | Start and stop on a task (`Ctrl+T`); one running timer at a time; the status bar and the shell show elapsed time through `svc.notify` progress. | S | 3 |
| CO04-F079 | Manual entries | Add, edit, split, delete time entries with start, end, note. | S | 3 |
| CO04-F080 | Idle detection | If the session was locked or suspended while a timer ran, ask to keep or discard that span (uses lock and resume signals only; no input monitoring). | S | 3 |
| CO04-F081 | Pomodoro | Optional 25/5 cycle on the timer with a notification at each boundary. | C | 3 |
| CO04-F082 | Reports | Time per task, list, tag, day, week as a table and a `chart`; estimate against spent. | S | 3 |
| CO04-F083 | Timesheet export | CSV and Markdown; rounding rule (none, 5, 15 min). | S | 3 |

### 4.8 Templates, attachments, links

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO04-F084 | Task and project templates | Save a task subtree or a project as a template with relative dates ("due +3d from start") and variables (`{name}`); instantiate with a start date. | S | 2 |
| CO04-F085 | Template files | Templates are Markdown files in the app data folder; import and export by file. | S | 2 |
| CO04-F086 | Attachments | Files added through the portal are copied into the app store (default) or kept as a remembered portal handle (linked). CalDAV: inline `ATTACH` under 5 MiB or managed attachments when offered. | S | 2 |
| CO04-F087 | Open attachments safely | Open through `svc.intent`; files that came with a shared or imported task offer "Open in disposable sandbox". | S | 2 |
| CO04-F088 | Links to other apps | Typed links: `mid:` (CO01 message), note link (OF02), event link (CO02), contact (CO03), file handle, URL. Shown as chips; opening hands off by intent. | M | 2 |
| CO04-F089 | Backlinks interface | `org.pbsd.tasks.links`: another app asks "tasks linked to this item" and gets titles and states, so CO01 and OF02 can show a task chip. | C | 3 |
| CO04-F090 | People on a task | Optional `ATTENDEE`/assignee text chosen through the CO03 picker; display only, no workflow. | C | 3 |

### 4.9 Search, automation, settings, accessibility, recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO04-F091 | Search | FTS over title, notes, tags, list names; results grouped by list; the filter language works in the same field. | M | 1 |
| CO04-F092 | System search | Titles to `svc.search` (default on, never for encrypted lists). | S | 2 |
| CO04-F093 | Command palette | All actions, lists, tags, "add task …" using the quick-add grammar. | M | 1 |
| CO04-F094 | Read interface for Calendar | `org.pbsd.tasks.dated`: tasks with dates in a range; write calls for complete and reschedule. Used by CO02's overlay and the shell. | M | 2 |
| CO04-F095 | Automation rules | Local rules: "when added to list X, add tag Y", "when completed, move to section Done", "when overdue 7 days, raise priority". Form-built; no scripts in Phase 2. | C | 3 |
| CO04-F096 | Script hooks | User-signed ForgeScript on task events with no extra capabilities. | C | 3 |
| CO04-F097 | Session restore | View, selection, expansion state, unsaved inspector edits. | M | 1 |
| CO04-F098 | Store recovery | Integrity check, index rebuild, re-sync keeping local changes; linked files are never rewritten when the parse failed. | M | 2 |
| CO04-F099 | Backup | Store in `svc.backup`; optional daily plain export (Markdown per list) to a chosen folder. | S | 3 |
| CO04-F100 | Audit entries | First contact with servers; access by other apps through the service interfaces. | S | 2 |
| CO04-F101 | Keyboard-first operation | Every list and board action has a key (section 5.5); board is fully operable without a pointer. | M | 1 |
| CO04-F102 | Statistics | Completed per day and week, per list and tag, as a `chart`; local only. | C | 3 |

## 5. GUI

### 5.1 Window map

```
Tasks
├─ Main window
│  ├─ Header bar: menu, title (list name), search and filter field, [net] badge
│  ├─ Sidebar: smart lists, month strip, lists and folders by account, tags, saved filters
│  ├─ Toolbar: [+ Add] · view switch (List | Board | Timeline) · group, sort · show completed
│  ├─ Content: list tree | board | timeline | review walker | reports
│  ├─ Inspector (F4): task details (notes, dates, repeat, reminders, subtasks, links, time)
│  └─ Status bar: counts, sync state, running timer
├─ Quick-add window (global, 560×120)
├─ Focus window (one task)
├─ Dialogs: repeat editor · list properties · account lists chooser · import wizard ·
│  export · link file (todo.txt, Markdown) · template manager · new from template ·
│  time entry editor · automation rules · print setup · preferences
├─ Popovers: date picker with shortcuts · tag chooser · list chooser · priority ·
│  reminder · snooze · link chooser
├─ Sheets: sync conflict · paste-many preview · complete-with-open-subtasks · file changed on disk
└─ Shell-drawn: notifications with actions, file portal, print dialog, capability prompts
```

### 5.2 Main window

List view at 1180×760.

```
+------------------------------------------------------------------------------------------+
| [=] Tasks — Website relaunch       [/ Search or filter…        ] (1)       [net] _ ▢ ✕   |
+------------------------------------------------------------------------------------------+
| [+ Add] (2)        | [List|Board|Timeline] Group: Section v  Sort: Manual v  [✓ Done] (4) |
+--------------------+----------------------------------------------+----------------------+
| ▣ Inbox          3 | [ Add a task… e.g. "Draft copy fri #writing" ] (5)| Write landing copy |
| ☀ Today          5 | DESIGN                                   2 of 4   | ─────────────── (9)|
| ▦ Upcoming      12 |  [x] Wireframes                                   | List  [Website v]  |
| ➜ Next           4 |  [ ] Visual design            !!  [Fri 20 Nov]    | Sect. [Content v]  |
| ⏳ Waiting        1 |   ▾ [ ] Icons                3/7 [#assets]   (6)  | Due   [Fri 20 Nov] |
| ✔ Completed        |       [x] Toolbar set                             | Start [—]          |
|   Nov 2026     (3) |       [ ] Status set                              | Repeat[none v]     |
| Mo Tu We Th Fr Sa Su| CONTENT                                 0 of 3   | Prio  [!! v]       |
| 16 17 18·19 20·21 22|> [ ] Write landing copy  !! [Fri 20 Nov] ✉ ↻ (7)  | Tags  [writing ✕]  |
| LISTS              |  [ ] Privacy page             [Mon 23 Nov]        | Remind[Fri 09:00 +]|
|  ● Home          8 |  [ ] Translate (waiting)      [#waiting]          | Est.  [2 h] 0:40 ⏱ |
|  ● Shopping      6 | LAUNCH                                  0 of 2    | ───────────────    |
|  ▾ Work            |  [ ] DNS switch          🔒 blocked by 2      (8)  | Notes              |
|     ● Website   11 |  [ ] Announce                 [Tue 1 Dec]         | Keep it under 200  |
|     ● Admin      4 |                                                   | words.             |
|  ● todo.txt 📄   23 |                                                   | - [ ] headline     |
| TAGS               |                                                   | - [ ] call to act. |
|  #writing #assets  |                                                   | Links              |
| FILTERS            |                                                   | ✉ "Re: copy brief" |
|  High this week    |                                                   | 📎 brief.pdf        |
+--------------------+----------------------------------------------+----------------------+
| 11 tasks · 3 done · 1 overdue · Synced 09:15 · ⏱ 0:40 Write landing copy [Stop]   (10)   |
+------------------------------------------------------------------------------------------+
```

1. One field for search and the filter language.
2. Primary action; `N` focuses the add row instead.
3. Month strip: a dot marks days with due tasks; click filters by day.
4. View switch, grouping, sorting, show or hide completed.
5. Add row with the quick-add grammar; chips appear as parts are recognised.
6. Subtask tree with progress on the parent; tag chip.
7. Selected task: priority flag, due chip, link icon (from a mail), repeat icon.
8. A blocked task is dimmed with a lock and the count of blockers.
9. Inspector: every field of the selected task; the timer line appears only when time tracking is on.
10. Status bar with the running timer.

Due chips: `--danger` with the word "overdue" when past, `--warn` for today, neutral otherwise.

### 5.3 Other views and dialogs

#### 5.3.1 Board view (content area)

```
+----------------------+----------------------+----------------------+-----------------+
| DESIGN        2 (1)  | CONTENT       3 / 3 ⚠| LAUNCH        2      | [+ Section]     |
+----------------------+----------------------+----------------------+-----------------+
| +------------------+ | +------------------+ | +------------------+ |                 |
| | Visual design    | | |>Write landing    | | | DNS switch  🔒   | |                 |
| | !!   Fri 20 Nov  | | | copy  !! Fri 20  | | | blocked by 2     | |                 |
| +------------------+ | | #writing ✉ ↻     | | +------------------+ |                 |
| +------------------+ | +------------------+ | +------------------+ |                 |
| | Icons      3/7   | | +------------------+ | | Announce         | |                 |
| | ▓▓▓░░░░ #assets  | | | Privacy page     | | | Tue 1 Dec        | |                 |
| +------------------+ | | Mon 23 Nov       | | +------------------+ |                 |
| [+ Add card]         | +------------------+ | [+ Add card]         |                 |
|                      | [+ Add card]    (2)  |                      |                 |
+----------------------+----------------------+----------------------+-----------------+
```

1. Column header: name, count, optional limit ("3 / 3" with a warning icon and word when reached). 2. Add card in place. Keyboard: arrows move focus between cards, `Ctrl+arrows` move the card, `Enter` opens the inspector. Column source is chosen in "Columns by" (section, status, priority, tag, due bucket). States: empty column shows "Drop a task here".

#### 5.3.2 Today view (content area)

```
| Today · Thursday 19 November                         5 tasks · est. 3 h 10 |
| OVERDUE                                                                    |
|  [ ] Send invoice                 Home        [overdue 2 d] [→ today] (1)  |
| TODAY                                                                      |
|  [ ] 10:00 Call Sol               Work  !                                  |
|  [ ] Buy milk                     Shopping                                 |
|  [ ] Stand-up notes   ↻           Work                                     |
| THIS EVENING                                                               |
|  [ ] 19:30 Water plants  ↻        Home                                     |
| — Calendar today: 3 events · [Show in Calendar] —                     (2)  |
```

1. One-click reschedule of overdue items. 2. A count from CO02's `upcoming` interface (if granted) and the hand-off button; no event details are drawn here.

#### 5.3.3 Quick-add window (560×120, global)

```
+------------------------------------------------------------+
| [ Call Sol tomorrow 10am #work !2 rem 15m______________ ]  |
| [📅 Fri 20 Nov 10:00] [#work] [!! medium] [⏰ 09:45] → Inbox |
|                                   Enter to add · Esc close |
+------------------------------------------------------------+
```

Chips preview the parse. `Tab` moves into the chips to change them. `Ctrl+Enter` adds and keeps the window open for the next task.

#### 5.3.4 Repeat editor (520×440)

```
+------------------------------------------------------+
| Repeat "Water plants"                                |
+------------------------------------------------------+
| (•) On a schedule    Every [3 ] [day(s) v]           |
|     On  [ ]M [ ]T [ ]W [ ]T [ ]F [ ]S [ ]S           |
| ( ) After completion Every [__] [day(s) v]           |
| Ends (•) Never ( ) After [__] times ( ) On [date]    |
| When I am late: (•) one overdue task ( ) keep each   |
+------------------------------------------------------+
| Every 3 days. Next: 22 Nov · 25 Nov · 28 Nov         |
|                           [Remove]  [Cancel]  [Done] |
+------------------------------------------------------+
```

#### 5.3.5 Link a file (600×420)

```
+--------------------------------------------------------------+
| Link a text file as a list                                   |
+--------------------------------------------------------------+
| File    [~/Sync/todo.txt                    ] [Choose…]  (1) |
| Format  (•) todo.txt   ( ) Markdown task list                |
| Done items  (•) move to done.txt  ( ) keep in file with x    |
| Found: 23 open, 4 done, 2 lines not understood [Show]   (2)  |
| [x] Watch the file for outside changes                       |
| The file stays the source. Tasks edits it line by line.      |
|                                   [Cancel]  [Link]           |
+--------------------------------------------------------------+
```

1. The portal grant for this one file (and `done.txt` beside it) is remembered. 2. Lines that are not understood are kept untouched and listed.

#### 5.3.6 Time report (content area, when time tracking is on)

```
| Week 47 · 16–22 Nov          [‹][›]   Group by [List v]   [Export CSV]     |
| List / task                Mon   Tue   Wed   Thu   Fri   Total   Estimate  |
| ▾ Website                  1:30  2:10  0:45  0:40    –    5:05     8:00    |
|     Write landing copy       –     –     –   0:40    –    0:40     2:00    |
|     Visual design          1:30  2:10  0:45    –     –    4:25     6:00    |
| ▾ Admin                    0:20    –   0:15    –     –    0:35      –      |
| Total                      1:50  2:10  1:00  0:40    –    5:40             |
| [bar chart: hours per day, stacked by list]                                |
```

### 5.4 Menus

Items separated by ` · `; shortcut in brackets.

```
File     New task [Ctrl+N] · New subtask [Ctrl+Shift+N] · New list… · New section [Ctrl+Shift+S] ·
         New from template… · Link file… · Import… [Ctrl+O] · Export… · Print… [Ctrl+P] ·
         Close [Ctrl+W] · Quit [Ctrl+Q]
Edit     Undo [Ctrl+Z] · Redo [Ctrl+Shift+Z] · Cut · Copy · Paste [Ctrl+X/C/V] ·
         Copy as Markdown [Ctrl+Shift+C] · Duplicate [Ctrl+D] · Delete [Delete] ·
         Select all [Ctrl+A] · Find [Ctrl+F, /] · Preferences [Ctrl+,]
View     List [Ctrl+1] · Board [Ctrl+2] · Timeline [Ctrl+3] · Group by > · Sort by > ·
         Columns by > (board) · Show completed [Ctrl+H] · Expand all [Ctrl+]] ·
         Collapse all [Ctrl+[] · Table columns… · Focus mode [Ctrl+Shift+F] · Sidebar [F9] ·
         Inspector [F4] · Zoom [Ctrl++ / - / 0]
Go       Inbox [G I] · Today [G T] · Upcoming [G U] · Next [G N] · Go to list… [G L] ·
         Show in Calendar [G C] · Back/Forward [Alt+Left/Right]
Task     Complete [Space] · Cancel task · Set due… [D] · Set start… [Shift+D] ·
         Postpone > Tomorrow | Next week | Pick [Shift+.] · Priority > [1 2 3 0] · Tags… [T] ·
         Move to list… [M] · Repeat… [R] · Reminder… [A] · Indent [Tab] · Outdent [Shift+Tab] ·
         Move up/down [Ctrl+Up/Down] · Add link… [Ctrl+L] · Attach file… · Convert to project ·
         Convert to event · Start/stop timer [Ctrl+T] · Save as template…
Tools    Process Inbox · Weekly review · Templates… · Automation rules… · Time report ·
         Statistics · Sync now [F5] · Lock encrypted lists [Ctrl+Shift+L] · Sync log · Check store
Help     Tasks help [F1] · Keyboard shortcuts [Ctrl+/] · About Tasks
```

### 5.5 Keyboard and pointer

List focus: `Up/Down` move; `Right/Left` expand, collapse; `Enter` or `F2` edit title; `Enter` at the end of an edit makes a new task below; `Tab`/`Shift+Tab` indent and outdent; `Space` complete; `Ctrl+Up/Down` reorder; `Shift+Up/Down` extend selection; single keys `D`, `T`, `M`, `R`, `A`, `1–3`, `0` open the matching popover or set the value. Board: arrows move focus, `Ctrl+Left/Right` move the card to the next column, `Ctrl+Up/Down` reorder. Single-key shortcuts are inactive while a text field has focus and can be turned off.

Context menus:

- **Task**: Complete, Edit, Set due, Postpone, Priority, Tags, Move to list, Repeat, Reminder, Add subtask, Duplicate, Copy as Markdown, Copy link, Show in Calendar, Start timer, Convert, Delete.
- **List**: Rename (`F2`), Colour and icon, Properties, New section, Export, Sync now, Archive, Lock, Delete.
- **Section or column**: Rename, Set limit, Move left or right, Collapse, Delete (tasks move to "No section").
- **Tag**: Rename, Colour, Merge into…, Delete.
- **Link chip**: Open, Copy, Remove.

Drag and drop:

| Drag | Onto | Result |
|---|---|---|
| Task | Another position | Reorder; onto a task: make subtask |
| Task | List, section, column | Move; board column writes the column's field |
| Task | Today, Upcoming day, month-strip day | Set due date |
| Task | Tag in sidebar | Add tag |
| Task | Outside (text editor, Mail, Notes) | Markdown task line and plain text; `.ics` VTODO |
| Task | CO02 time grid | CO02 sets start and due through the `dated` interface |
| Text, URL, file, mail message | List or add row | New task with the item linked or attached |
| `.txt`, `.md`, `.ics`, `.csv` | Window | Import preview |

### 5.6 States

| State | Behaviour |
|---|---|
| Empty app | "Nothing to do yet." with the add row focused and a hint line showing the quick-add grammar. |
| Empty list | "No tasks in Website." The add row stays. |
| All done (Today) | "All done for today." No illustration, no sound. |
| Loading | Cached rows at once; sync progress in the status bar. |
| Error | Warning icon on the list; status bar message with Details. |
| Offline | "Offline · 4 pending"; all edits allowed. |
| Locked | Encrypted lists show a lock and no counts; [Unlock]. |
| Read-only | Check boxes and drags disabled; banner states why (shared read-only list or file not writable). |
| File changed on disk | Linked file: sheet "todo.txt changed outside Tasks" with Reload, or Merge when there are unsaved edits. |
| Narrow (<640) | Sidebar is an overlay; inspector is a full-height sheet; board scrolls one column at a time with snap. |

### 5.7 Widgets

| Region | FUSE widget | Notes |
|---|---|---|
| Sidebar | `treeView`, `calendar` (month strip) | |
| List view | `treeView` with a custom row delegate (check box, chips) | drag reorder inside a tree needs FUSE-WP-33 |
| Table mode | `tableView` | |
| Board | missing: kanban column container with card drag (`CO04-GAP-1`); can be built from `scrollArea` + `listView` per column once in-app drag and drop (FUSE-WP-33) exists | |
| Timeline | `timeline` (V3) | needs dependency arrows (`CO04-GAP-2`) |
| Inspector | `scrollArea`, `lineEdit`, `comboBox`, `dateTimeEdit`, `spinBox`, `markdownView`, `codeEdit` (notes edit) | |
| Add row, quick-add | `lineEdit` with chips | missing: tag input (same gap as CO01-GAP-1) |
| Date popover | `calendar`, `dateTimeEdit`, `button` | |
| Reports, statistics | `tableView`, `chart` | |
| Progress on parent | `progressBar` | |
| Toolbar | `button`, `tabBar`, `comboBox`, `toggleSwitch` | |
| Popovers, sheets, toasts | missing in V3; FUSE-WP-30 | |

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | View models, tree and order logic, filter engine, undo, journal, templates, time tracking |
| UI | FUSE UI | app host | Windows, quick-add window |
| Quick-add grammar | ForgeScript module shared with CO02 | app host | Date, repeat and token parsing |
| Recurrence engine | ForgeScript module from CO02 | app host | Next-date computation |
| Text-format module | ForgeScript | app host | todo.txt and Markdown task-line reader and writer. Runs in the app host because the input is the user's own file reached by handle and the grammar is line-based and bounded (line cap 64 KiB). |
| iCalendar service client | — | CO02's helper process | VTODO parse and serialise through `org.pbsd.calendar.ical`; if CO02 is absent, the bundle's own copy of the `ical` helper is used |
| DAV sync helper | C++23 | per account, `proc.helper:dav` | CalDAV; same source as CO02 |
| Import helper | C++23 | per job, `proc.helper:import` | CSV, JSON, Taskwarrior export, `.ics` files from others. No capabilities. |
| Reminder daemon | ForgeScript (small entry) | background app host | Next fire times; `svc.notify` |
| Tasks service | ForgeScript (headless entry) | background app host, on demand | `dated`, `links` interfaces for CO02 and others |

```
   CO01/OF02/any app ──new, share──┐        CO02 ──dated (read, complete, reschedule)──┐
                                   v                                                    v
        +--------------------------------------------------------------------------------+
        | app host: tree model · filters · quick-add grammar · recurrence · journal · undo|
        +------+----------------+----------------+----------------+----------------------+
               |                |                |                |
        +------v-----+   +------v------+   +-----v------+   +-----v--------+
        | ical helper|   | dav helper  |   | import     |   | reminder     |
        | (CO02 svc) |   | sockets     |   | helper     |   | daemon       |
        +------------+   +------+------+   +------------+   +--------------+
                                v
                         network service        linked files: handles from svc.portal.file
```

### 6.2 Data model

Store: `~/.local/share/org.pbsd.tasks/tasks.sqlite`, `objects/` raw VTODO per synced task, `attachments/<blake3>`, `templates/*.md`, `enc/<list-id>.sqlite` for encrypted lists, `time.sqlite`.

| Entity | Fields |
|---|---|
| `list` | `id: uuid`, `account_ref: text?`, `kind: enum(local, caldav, todotxt, markdown)`, `name`, `colour`, `icon`, `folder_id: uuid?`, `url`, `ctag`, `sync_token`, `file_grant: text?` (portal grant id), `read_only: bool`, `encrypted: bool`, `archived: bool`, `default_reminder: json`, `section_order: json`, `order: f64` |
| `section` | `id: uuid`, `list_id`, `name`, `rank: f64`, `wip_limit: u16?`, `collapsed: bool` |
| `task` | `id: i64`, `list_id`, `section_id: uuid?`, `parent_id: i64?`, `uid: text`, `href`, `etag`, `raw_ref`, `base_ref`, `title`, `notes`, `status: enum`, `priority: u8`, `percent: u8`, `start: i64?`, `start_is_date: bool`, `due: i64?`, `due_is_date: bool`, `tz: text?`, `completed_at: i64?`, `created_at: i64`, `modified_at: i64`, `rank: f64`, `rrule: text?`, `repeat_after: text?`, `estimate_s: u32?`, `location`, `url`, `colour`, `pinned_today: i64?`, `dirty: bool`, `deleted_at: i64?`, `source_line: u32?` (linked files) |
| `tag` | `task_id`, `name` |
| `alarm` | `task_id`, `trigger_rel_s: i32?`, `related: enum(start, due)`, `trigger_abs: i64?`, `acknowledged: i64?`, `snoozed_until: i64?` |
| `link` | `task_id`, `kind: enum(mail, note, event, contact, file, url, task)`, `target: text`, `label: text`, `rel: enum(ref, depends_on)` |
| `attachment` | `task_id`, `name`, `mime`, `size: u64`, `blob: text?`, `grant: text?`, `origin: enum(local, synced, imported)` |
| `completion_log` | `task_id`, `occurrence_due: i64?`, `completed_at: i64` |
| `next_alarm` | `fire_at: i64`, `task_id` (the only table the daemon reads) |
| `saved_filter` | `id`, `name`, `query`, `icon`, `colour`, `order` |
| `template` | `id`, `name`, `path`, `variables: json` |
| `rule` | `id`, `name`, `enabled`, `when: json`, `then: json` |
| `journal_op` | `seq`, `list_id`, `kind`, `task_id`, `state`, `attempts`, `error` |
| `version` | `task_id`, `rev`, `at`, `origin`, `snapshot: json` |
| `time_entry` (in `time.sqlite`) | `id`, `task_id`, `start: i64`, `end: i64?`, `note`, `source: enum(timer, manual)` |

Order uses fractional ranks; a list is re-ranked when the gap falls under 1e-9. Linked-file lists keep `source_line` and rewrite only changed lines. Migration is forward only with a copy of the previous database.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| iCalendar VTODO (RFC 5545, 9253 relations, 9074 alarms) | yes | yes | CO02 iCalendar helper | iCalendar helper |
| CalDAV, WebDAV, sync-collection | yes | yes | own; shared with CO02 | DAV sync helper |
| todo.txt | yes | yes | own | app host (own file by handle; bounded line grammar) or import helper for files from others |
| Markdown task lists | yes | yes | own line scanner; does not parse full Markdown | app host, as above |
| CSV | yes | yes | own | import helper (read), app (write) |
| JSON (own schema, documented in `help/format.md`) | yes | yes | own | import helper (read), app (write) |
| Taskwarrior JSON export | yes | no | own | import helper |
| Markdown templates | yes | yes | own | app host (own data) |
| PDF (print) | — | yes | G-16 | print service |

No third-party library is proposed.

### 6.4 Integration

- **Intents handled**: `new` (task from text, URL, handle, link), `share` (same), `open`/`view` for `text/x-todo-txt` and `.ics` holding only VTODO. **Emitted**: `open` (links, attachments), `view` (to CO02: show date), `new` (to CO02: convert to event), `pick` (CO03), `print`, `share`.
- **URL schemes**: `pbsd-tasks:task/<uid>`, `pbsd-tasks:list/<id>`.
- **MIME**: import and export `text/calendar`, `text/markdown`, `text/plain` (todo.txt), `text/csv`, `application/json`.
- **Services used**: `svc.accounts`, `svc.keyring`, `svc.notify`, `svc.settings`, `svc.intent`, `svc.portal.file`, `svc.portal.print`, `svc.search`, `svc.backup`, `svc.audit`, `svc.power`, `svc.clipboard`, `svc.undo-trash`, `svc.a11y`, `svc.model` (optional).
- **Services offered** (Proposed): `org.pbsd.tasks.dated` (read by range; complete; reschedule), `org.pbsd.tasks.links` (tasks linked to an item), `org.pbsd.tasks.add` (the path behind the `new` intent for the global quick-add).
- **Other apps**: CO02 (overlay, recurrence engine, iCalendar service, DAV helper, date grammar), CO01 (`mid:` links, "make task"), OF02 (note links), CO03 (picker), SH01 (global shortcut registration, upcoming count), UT02 (none: Clocks owns free-standing timers).
- **Backing repos**: Verified: FUSE has `treeView`, `tableView`, `timeline`, `calendar`, `chart`, `markdownView` (PLATFORM.md V3). In-app drag and drop is planned as FUSE-WP-33 in `Apps/00-foundation/build-plans/FUSE_BUILD_PLAN.md`; its text was not read beyond the title: details Unverified. Everything else is Proposed.

## 7. Security and privacy

### 7.1 Capabilities requested

| Capability | Reason |
|---|---|
| `fs.app` | Store, attachments, templates |
| `fs.portal` | Import, export, attachments, linked todo.txt and Markdown files (remembered grants) |
| `net.user` | CalDAV servers the user typed |
| `net.route:clear`, `net.route:vpn`, `net.route:tor` | Route inherited from the account |
| `proc.helper:dav`, `proc.helper:ical`, `proc.helper:import` | Helpers in 6.1 |
| `svc.accounts`, `svc.keyring` | Endpoints, secrets, list keys |
| `svc.notify` | Reminders, daily summary, timer state |
| `svc.intent`, `svc.portal.file`, `svc.portal.print` | Hand-offs, files, printing |
| `svc.search`, `svc.settings`, `svc.backup`, `svc.audit`, `svc.power`, `svc.clipboard`, `svc.undo-trash`, `svc.a11y` | Standard services |
| `svc.model`, `ai.local` | Optional capture help; off by default |
| `bg.run`, `bg.autostart` | Reminder daemon, background sync, on-demand service |

With no account and no linked file, the app runs with `net.none` in effect: the network capabilities are requested at install but stay unused until the user adds an account.

### 7.2 Threat model

Assets: the user's plans, habits and whereabouts implied by tasks; attachments; time records.

| Adversary | Attack | Mitigation |
|---|---|---|
| Sender of a shared task, `.ics` or import file | Parser exploit | iCalendar and import parsing in zero-capability helpers; T2 targets; fuzzing |
| Shared list member or server | Hostile attachment, hostile link, script in notes | Notes are inert Markdown; links confirm per host; attachments from others open with the disposable-sandbox option |
| Shared list member | Flood of tasks or alarms | Alarms from shared lists are off until allowed per list; per-sync caps with a review sheet above 500 new tasks |
| CalDAV server | Reads all tasks | Local and encrypted lists for sensitive matter |
| Other local app | Read all tasks through the service | `dated` and `links` are capability-gated per app, return only dated tasks or linked titles, honour locks, and are audited |
| Other local app | Inject tasks silently through `new` | Every capture shows a toast naming the source app, with Undo; rate limit 30 per minute per app |
| Outside process editing the linked file | Corrupt or conflicting writes | Atomic replace, re-read before write, hash check; unknown lines preserved; merge sheet on conflict |
| Lock-screen observer | Task titles in reminders | Titles hidden when locked unless allowed |
| Time tracking misuse | Becoming activity surveillance | Uses only explicit timers and lock or resume signals; no window titles, no input monitoring, no screenshots |

Stored: section 6.2. Leaves the machine: CalDAV traffic to the user's servers. Nothing else.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `view.start` | enum today/inbox/last | today | View at start |
| `view.density` | enum compact/normal | normal | Row density |
| `view.completed` | enum hide/bottom/fade_hours | bottom | Completed tasks in lists |
| `view.fade_hours` | int | 24 | Hours before completed tasks hide |
| `view.add_position` | enum top/bottom | top | Where new tasks go |
| `view.week_start` | enum locale/mon/sat/sun | locale | Upcoming and month strip |
| `add.default_list` | list id | Inbox | Target for captures |
| `add.grammar_lang` | list of languages | UI language, en | Quick-add grammars |
| `add.single_keys` | bool | true | Single-key shortcuts |
| `task.default_priority` | 0–3 | 0 | New task priority |
| `task.date_only_due_time` | time | 17:00 | Sort and overdue cut-off for date-only tasks |
| `task.complete_children` | enum ask/always/never | ask | Completing a parent with open subtasks |
| `repeat.late_mode` | enum one/each | one | Missed occurrences |
| `reminder.default_time` | time | 09:00 | Reminder time for date-only tasks |
| `reminder.daily_summary` | time? | off | Daily summary time |
| `reminder.repeat_overdue` | enum off/daily | off | Re-notify overdue |
| `reminder.show_titles_locked` | bool | false | Titles on the lock screen |
| `reminder.shared_lists` | bool | false | Alarms from shared lists |
| `badge.mode` | enum off/today/overdue/both | both | Launcher badge |
| `board.columns_by` | enum section/status/priority/tag/due | section | Board columns |
| `board.wip_warn` | bool | true | Warn at column limit |
| `time.enabled` | bool | false | Time tracking |
| `time.rounding_min` | enum 0/5/15 | 0 | Report rounding |
| `time.pomodoro` | bool | false | Pomodoro cycle |
| `sync.interval_min` | int | 15 | Background sync |
| `file.watch` | bool | true | Watch linked files |
| `search.system_index` | bool | true | Titles to `svc.search` |
| `trash.keep_days` | int | 30 | Trash retention |
| `ai.capture` | bool | false | Model-assisted capture |

## 9. Accessibility and languages

- Row a11y name: "Write landing copy, not done, high priority, due Friday 20 November, 2 tags, has subtasks 0 of 2, repeats".
- Tree roles with level and position ("level 2, 1 of 7"); board columns are lists with a heading; moving a card announces "moved to Launch, position 2".
- Completion is announced; the 140 ms animation is skipped with reduced motion.
- Priority and due state use icon and word, never colour alone.
- Quick-add grammars are per language; chips show the parsed result so a mis-parse is visible before saving; dates always display in the locale format.
- Right-to-left: indentation, chips and the board order mirror.
- Emoji metadata in Markdown export (`📅`) can be switched to text keys (`due:`) for screen-reader users and plain tools.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | under 200 ms |
| Quick-add window from shortcut to ready | under 100 ms when the service is warm, under 250 ms cold |
| Add or complete a task (local commit and repaint) | under 16 ms |
| Open a list of 20 000 tasks | first rows under 80 ms; 60 fps scroll |
| Filter query over 200 000 tasks | under 100 ms |
| Board with 2 000 cards | drag at 60 fps |
| Reload a 5 MiB todo.txt | under 300 ms |
| Memory | under 100 MiB with 200 000 tasks; daemon under 20 MiB |
| Idle | zero frames, zero wake-ups; daemon sleeps until the next alarm; file watch is event-driven |
| Largest supported | 500 000 tasks, 1 000 lists, nesting depth 32 |

## 11. Testing and proof

Test plan (targets; nothing has run):

- **Unit**: tree operations (indent, outdent, reorder, move with subtree), fractional ranks and re-ranking, filter parser and evaluator, quick-add grammar tables per language, repeat advance (fixed and after-completion), late modes, time rounding.
- **Property tests**: todo.txt and Markdown read then write is byte-identical for untouched lines; edit of line k changes only line k; tree invariants (no cycles, depth cap); undo then redo is identity; rank order is total and stable.
- **VTODO corpus**: objects from common servers and clients, with their extension properties; unknown properties survive round trip.
- **DAV**: fake server with VTODO fixtures; conflict paths; differential runs against Radicale and Baïkal offline.
- **Fuzz**: todo.txt and Markdown line scanners, filter parser, quick-add parser, CSV and JSON import, Taskwarrior import.
- **Golden renders**: list, board, Today, inspector, quick-add, in all themes, 200%, RTL.
- **Scripted UI**: the 14 top tasks keyboard only; board operation without a pointer.
- **Linked-file tests**: concurrent outside edits, truncated file, file replaced by a directory, permission lost.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Tree model | No cycles; every task has one parent or none; depth ≤ 32 | T1 | Invariant proved on a small model; property tests on the code | ForgeScript not readable by PRISM (Unverified) |
| Linked-file writer | Lines not edited are byte-identical after a write; never writes after a failed parse | T1 as a stretch, T3 | Property tests; a C++23 version of the line splice could be proved | Language front end |
| Filter and quick-add parsers | Terminate; bounded input | T3 | Fuzzing, unit tests | Interpreter not verified |
| Repeat advance | Next date is strictly after the completed one | T1 | Proved with the CO02 engine properties | Shares CO02's blocker |
| Import helper | No runtime errors | T2 | PRISM bounded checks, fuzzing | — |
| Service rate limiter | Bound on calls per app per window | T1 | Small pure function | — |
| Reminder daemon | Reads only `next_alarm` | T3 | Schema grant, review | — |
| DAV and iCalendar helpers | As in CO02 | T2 | Shared | — |

## 12. Build plan

### Phase 1 — usable core

| ID | Work item | Depends on |
|---|---|---|
| CO04-W01 | Store, tree model, ranks, journal, undo | G-07, G-09, FORGE-WP-23.1 |
| CO04-W02 | Main window: sidebar, list tree, inspector, add row | G-01, G-02, FUSE-WP-28, FUSE-WP-33 |
| CO04-W03 | Smart lists: Inbox, Today, Upcoming, Completed, Trash | CO04-W01 |
| CO04-W04 | Quick-add grammar module (shared with CO02) | CO02-W07 |
| CO04-W05 | Repeat (fixed schedule) with the CO02 engine; repeat editor | CO02-W03 |
| CO04-W06 | Reminders and daemon | `svc.notify`, `bg.run` |
| CO04-W07 | Search, grouping, sorting, multi-select edit, command palette | CO04-W01 |
| CO04-W08 | Session restore, trash, keyboard map | CO04-W02 |
| CO04-W09 | Unit and property tests, goldens | FORGE-WP-35 |

### Phase 2 — full feature

| ID | Work item | Depends on |
|---|---|---|
| CO04-W10 | CalDAV VTODO sync, conflict merge, interoperable properties | CO02-W02, CO02-W09 |
| CO04-W11 | Board view | `CO04-GAP-1`, FUSE-WP-33 |
| CO04-W12 | Global quick-add window, `new` and `share` intents, Inbox processing | SH01 (shortcut), `svc.intent` |
| CO04-W13 | todo.txt and Markdown import, export, todo.txt live link | `svc.portal.file` (remembered grants, file watch) |
| CO04-W14 | Filter language, custom smart lists, deferred tasks | CO04-W07 |
| CO04-W15 | `dated` service and Calendar hand-off | CO02-W19 |
| CO04-W16 | After-completion repeat, skip and catch-up, subtasks on repeat | CO04-W05 |
| CO04-W17 | Templates, attachments, typed links | CO01, OF02 |
| CO04-W18 | Import helper: CSV, JSON, Taskwarrior, `.ics` | FORGE-WP-27, FORGE-WP-28 |
| CO04-W19 | Table mode, completed log, month strip, daily summary, badge | CO04-W02 |

### Phase 3 — advanced

| ID | Work item | Depends on |
|---|---|---|
| CO04-W20 | Time tracking, reports, timesheet export | CO04-W01 |
| CO04-W21 | Timeline view and dependencies | `CO04-GAP-2` |
| CO04-W22 | Encrypted lists, shared lists, Markdown live link | `svc.keyring`, CO04-W13 |
| CO04-W23 | Review mode, focus mode, statistics, streaks | CO04-W14 |
| CO04-W24 | Automation rules and script hooks | CO04-W01 |
| CO04-W25 | Backlinks interface, people on tasks, change history | CO01, OF02, CO03 |
| CO04-W26 | Model-assisted capture, printing | G-15, G-16 |

## 13. Open questions and risks

1. **Repeating VTODO interoperability.** Clients disagree on how to complete a repeating to-do. The single-object model chosen here matches several common clients but not all; the per-list switch is the escape.
2. **Sections and order on CalDAV** rely on extension properties. Other clients may reorder or drop them. Data is not lost, but layout may be.
3. **Dependence on CO02** (recurrence engine, iCalendar service, DAV helper, date grammar) although Tasks is P2 and Calendar P1. If Tasks must ship without Calendar installed, the bundle carries its own copies of the helpers; the build must produce both from one source.
4. **Global shortcut registration** by an app is not described in PLATFORM.md; assumed to be a shell facility (SH01).
5. **File watching through a portal handle** (change events on a granted file) is not listed in `svc.portal.file`. Without it the live link must poll on focus.
6. **Parsing the user's own text files in the app host** departs from the "hostile bytes go to a helper" habit. It is allowed because the file is user-authored and the grammar is trivial; files received from others go through the import helper. To confirm with the platform owner.
7. **Board widget** does not exist; in-app drag and drop (FUSE-WP-33) is its prerequisite.
8. **Time tracking** must stay explicit to avoid becoming monitoring; any request for automatic activity capture is out of scope.
9. **PRISM on ForgeScript**: same blocker as CO02 for T1 targets. Unverified.
