<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Calendar

By Odin Loch

| | |
|---|---|
| ID | `CO02` (from CATALOGUE.md) |
| Bundle | `org.pbsd.calendar` |
| Category | comms |
| Priority | P1 |
| Backing repos | FUSE (`calendar`, `dateTimeEdit` widgets today; scheduler views planned as FUSE-WP-34), FORGE (time zones and calendars in `forge::text`, per FORGE-WP-23.2). No calendar app code exists in any repo read. |
| Status | Planned. Nothing built. |

## 1. Purpose

Calendar shows and edits the user's events from local calendars, CalDAV servers and subscribed iCalendar files. It gets recurrence and time zones right, handles meeting invitations with Mail, and works fully offline. It replaces KOrganizer from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- Day, multi-day, week, work week, month, year, agenda and timeline views.
- Local calendars, CalDAV calendars (RFC 4791), read-only subscriptions (`.ics` over HTTPS or a file).
- Full iCalendar (RFC 5545, RFC 7986) including every RRULE part, exceptions, RDATE, EXDATE and detached overrides.
- Time zones from the system zone database, floating times, embedded VTIMEZONE fall-back.
- Invitations: iTIP (RFC 5546) over iMIP through CO01 and over CalDAV scheduling (RFC 6638).
- Free/busy and availability, reminders through `svc.notify`, natural-language quick add.
- Tasks overlay from CO04, birthdays from CO03, travel time, printing, encrypted calendars, sharing by file.
- The shared iCalendar and WebDAV sync helpers used by CO04, CO03 and CO01.

Out of scope:

| Item | Owner |
|---|---|
| Sending and receiving mail | CO01 Mail (Calendar builds iMIP bodies; Mail sends them) |
| Contact storage and birthdays as data | CO03 Contacts (Calendar shows them) |
| Task storage and editing | CO04 Tasks (Calendar shows an overlay and hands off) |
| Alarms, timers, world clock | UT02 Clocks |
| Maps and routing | IN04 Maps (travel time uses its offline router when installed) |
| Video meeting rooms | CO05 Messenger (Calendar stores the link and opens it) |
| Weather on the calendar | IN05 Weather (may contribute a read-only overlay; Phase 3) |
| Proprietary sync (Exchange EWS, Google API) | Not planned; CalDAV only |
| Booking pages, public scheduling links | Not planned (needs a server) |

## 3. Users and top tasks

Personas:

- **Rae, daily user.** Personal and work CalDAV calendars, a shared family calendar by file.
- **Ines, coordinator.** Schedules meetings across four time zones, lives in week view and free/busy.
- **Sol, privacy-minded.** One encrypted local calendar, no server, routes a subscription over Tor.
- **Tam, keyboard user with a screen reader.** Uses agenda view and quick add.

| # | Task | Target steps |
|---|---|---|
| T1 | See today | 0 (opens on today) or `T` |
| T2 | Create an event by typing "Lunch with Sol tomorrow 12:30 for 1h" | 2 (`Q`, type, Enter) |
| T3 | Create an event by dragging on the grid | 1 drag, type title, Enter |
| T4 | Move or resize an event | 1 drag |
| T5 | Change one occurrence of a repeating event | 2 (edit, choose "Only this event") |
| T6 | Accept an invitation that arrived by mail | 1 click in CO01 |
| T7 | Find a free slot for five people | 3 (open scheduling tab, see grid, click a slot) |
| T8 | Add a CalDAV account | 3 (address, password, pick calendars) |
| T9 | Subscribe to a public `.ics` | 2 (paste URL, Subscribe) |
| T10 | Jump to a date | 2 (`G`, type date) |
| T11 | Search for an event | 2 (`/`, type) |
| T12 | Snooze or dismiss a reminder | 1 click in the notification |
| T13 | Show a second time zone beside the grid | 2 |
| T14 | Print a week | 3 |
| T15 | Share one event as a file | 2 (context menu, Share as .ics) |

## 4. Features

### 4.1 Views

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO02-F001 | Day view | Time grid, all-day row, overlap columns, "now" line, 5/10/15/30/60 minute grid step. | M | 1 |
| CO02-F002 | Week and work-week view | 7 days or the configured work days; first day from locale or setting. | M | 1 |
| CO02-F003 | Multi-day view | 2 to 14 days, chosen by `Ctrl+2…9` or by dragging in the mini calendar. | S | 2 |
| CO02-F004 | Month view | Event chips, multi-day bars, "+n more" popover, week numbers (ISO or locale), 4 to 6 rows or continuous scroll. | M | 1 |
| CO02-F005 | Year view | 12 mini months with a density heat per day; click opens the day; drag selects a range. | S | 2 |
| CO02-F006 | Agenda view | Virtualised list grouped by day with sticky headers; from today, endless in both directions. | M | 1 |
| CO02-F007 | Timeline view | Horizontal time axis, one row per calendar or per attendee; used for resources and scheduling. | S | 3 |
| CO02-F008 | Mini calendar | Month picker in the sidebar; days with events dotted; shows the visible range; range select. | M | 1 |
| CO02-F009 | Today and now | `T` jumps to today; the now line updates each minute only while visible. | M | 1 |
| CO02-F010 | Working hours shading | Hours outside `view.work_start`–`view.work_end` are dimmed; grid opens scrolled to the start. | S | 1 |
| CO02-F011 | Second and third time-zone axis | Extra hour columns with zone labels beside the grid. | S | 2 |
| CO02-F012 | Event appearance | Calendar colour, title, time, location, icons for recurrence, reminder, attendees, private, tentative (hatched), cancelled (struck), declined (dimmed or hidden). | M | 1 |
| CO02-F013 | Overlap layout | Equal-width columns per cluster; widest free space used; stable order by start, duration, title. | M | 1 |
| CO02-F014 | Zoom | `Ctrl++/-` changes hour height (24 to 120 px) or month row count. | S | 1 |
| CO02-F015 | Split view | Two views side by side (for example week and agenda) by dragging a tab to an edge. | C | 3 |
| CO02-F016 | Overlays | Tasks (CO04), birthdays (CO03), public holidays (bundled offline files per region), week numbers, moon phases (computed), sunrise and sunset (computed from a set location, no network). | S | 2 |
| CO02-F017 | Calendar visibility sets | Named sets of visible calendars ("Work", "Everything") switched from the sidebar or `Alt+1…9`. | S | 2 |
| CO02-F018 | Hide declined and cancelled | Per view toggle. | S | 1 |
| CO02-F019 | Date navigation | Previous/next by view span, `G` go to date with free text ("next fri", "2027-03"), scroll and swipe. | M | 1 |
| CO02-F020 | Density summary | Day header shows the count and total booked hours on hover. | C | 3 |

### 4.2 Events: create and edit

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO02-F021 | Quick create on grid | Drag to make a slot, type a title in place, Enter saves; `Tab` opens the full editor. | M | 1 |
| CO02-F022 | Natural-language quick add | `Q` opens a field. Deterministic grammar per locale: title, date, time, duration, "every …", "at <place>", "with <name>", `#calendar`, `!30m` reminder. Parsed fields preview live as chips. No model needed. | M | 1 |
| CO02-F023 | Quick add by local model | When the grammar fails, optional parse through `svc.model`; the result is shown for confirmation, never saved silently. | C | 3 |
| CO02-F024 | Event editor | Title, calendar, start, end, all-day, time zone (start and end may differ), repeat, location, URL, description (rich text subset or plain), reminders, attendees, status, show-as (busy/free), visibility (public/private/confidential), categories, colour, attachments. | M | 1 |
| CO02-F025 | Event popover | Click shows a compact card: time, place, attendees, actions Edit, Delete, Duplicate, Copy, Open link. | M | 1 |
| CO02-F026 | Direct manipulation | Drag to move, drag edges to resize, `Ctrl`+drag to copy, drag to the all-day row and back, drag across days in month view; snap from `view.snap_minutes`. | M | 1 |
| CO02-F027 | Keyboard manipulation | Selected event: `Alt+arrows` move, `Shift+Alt+Up/Down` resize, `Ctrl+D` duplicate. | M | 1 |
| CO02-F028 | Undo and redo | Every local change, including drags and deletes of series, is undoable for the session; a toast offers Undo. | M | 1 |
| CO02-F029 | Multi-day and overnight events | Drawn across day boundaries; all-day events use DATE values. | M | 1 |
| CO02-F030 | Floating events | Events with no zone stay at the same wall time in any zone (birthdays, "take medicine 08:00"). | M | 1 |
| CO02-F031 | Attachments | Stored inline (`ATTACH` binary up to 10 MiB), as CalDAV managed attachments (RFC 8607), or as links; opened by `svc.intent`. | S | 2 |
| CO02-F032 | Location field | Free text with completion from earlier locations and CO03 addresses; `geo:` kept; "Show on map" hands to IN04. | S | 2 |
| CO02-F033 | Conference links | A `CONFERENCE` property or a URL in the description is detected and shown as "Join" (opens by `svc.intent`, asks per host the first time). | S | 2 |
| CO02-F034 | Travel time | "Travel before" and "after" durations stored as `X-PBSD-TRAVEL-BEFORE/AFTER` (and the Apple property read on import); drawn as a hatched lead-in; reminders may count from departure. With IN04 installed, an offline route estimate can fill the value. | S | 2 |
| CO02-F035 | Event templates | Save an event as a template; new from template from the `N` menu. | C | 3 |
| CO02-F036 | Duplicate, copy, paste, move to calendar | Clipboard carries `text/calendar` and plain text. | M | 1 |
| CO02-F037 | Categories and tags | iCalendar `CATEGORIES`; filter by category; colour may follow category. | S | 2 |
| CO02-F038 | Private notes | A local-only note field per event that never syncs (kept in the local store, not in the iCalendar object). | C | 3 |
| CO02-F039 | Conflict hint | When a new or moved event overlaps a busy event, a quiet inline hint lists the clash. | S | 2 |
| CO02-F040 | Delete | To trash with Undo; a series asks "This event / This and following / All". Invited events offer "Decline and delete". | M | 1 |
| CO02-F041 | Link back to source | Events made from a mail, note or task keep a `mid:` or app link shown as "Open source". | S | 2 |

### 4.3 Recurrence

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO02-F042 | Full RRULE engine | All of RFC 5545: FREQ SECONDLY to YEARLY, INTERVAL, COUNT, UNTIL, BYSECOND, BYMINUTE, BYHOUR, BYDAY with ordinals, BYMONTHDAY, BYYEARDAY, BYWEEKNO, BYMONTH, BYSETPOS, WKST. RSCALE (RFC 7529) read; non-Gregorian rules expanded when the calendar system exists in `forge::text`. | M | 1 |
| CO02-F043 | Recurrence set | RRULE plus RDATE (dates, date-times, periods) minus EXDATE, with detached overrides matched by `RECURRENCE-ID`, including `RANGE=THISANDFUTURE`. | M | 1 |
| CO02-F044 | Edit scopes | "Only this event" (override), "This and following" (split the series, keep UID lineage with `RELATED-TO`), "All events". | M | 1 |
| CO02-F045 | Simple repeat menu | Daily, weekdays, weekly on chosen days, fortnightly, monthly by date, monthly by weekday ("second Tuesday"), last day of month, yearly, custom. | M | 1 |
| CO02-F046 | Custom rule editor | Form for every RRULE part with a live plain-language summary and the next 10 occurrences. | M | 2 |
| CO02-F047 | Raw rule field | Paste or edit the RRULE text; errors are located by part. | C | 2 |
| CO02-F048 | Skipped-date handling | Monthly on the 31st and 29 February follow RFC 5545 (skip), with a hint offering "last day of month" instead. | S | 2 |
| CO02-F049 | DST-safe expansion | Occurrences keep wall time across DST; non-existent times move forward by the gap; repeated times use the first. | M | 1 |
| CO02-F050 | Expansion window and cache | Occurrences are expanded on demand per visible range and cached in `occurrence`; endless rules are never fully expanded. | M | 1 |
| CO02-F051 | End conditions | Never, after N, until date; UNTIL stored in UTC for zoned events. | M | 1 |
| CO02-F052 | Exception list | The editor lists exceptions and overrides of a series; each can be restored. | S | 2 |

### 4.4 Time zones

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO02-F053 | System zone database | IANA zones from the OS; version shown in About; updates arrive by `svc.update`. | M | 1 |
| CO02-F054 | Per-event zones | Start and end zones stored as TZID; display converts to the view zone. | M | 1 |
| CO02-F055 | Embedded VTIMEZONE | Written with every exported zoned event (truncated to the needed years). On import, known TZIDs use the system database; unknown ones use the embedded rules; Windows zone names map through a bundled CLDR table. | M | 1 |
| CO02-F056 | View zone switch | The whole view can be shown in another zone ("show my week in Tokyo time"). | S | 2 |
| CO02-F057 | Travel mode | When the system zone changes, a toast asks whether to follow it; events keep their own zones. | S | 2 |
| CO02-F058 | Zone picker | Search by city, country, abbreviation, offset; recent zones first. | M | 1 |
| CO02-F059 | Zone change warnings | After a zone database update that moves a future event's UTC time, the event is listed in a review sheet. | S | 3 |
| CO02-F060 | 23 and 25 hour days | The grid draws the real length of the day. | M | 2 |
| CO02-F061 | Time converter popover | Pick a time and see it in each configured zone. | C | 3 |

### 4.5 Calendars, accounts and sync

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO02-F062 | Local calendars | Any number; name, colour, default reminder, default zone. | M | 1 |
| CO02-F063 | CalDAV accounts | Discovery by RFC 6764 (`.well-known/caldav`, DNS SRV), `current-user-principal`, `calendar-home-set`. Endpoint in `svc.accounts`, secret in `svc.keyring`. Auth: Basic over TLS, Digest, Bearer (OAuth2 hand-off as in CO01), client certificate from SE04. | M | 1 |
| CO02-F064 | Efficient sync | `sync-collection` (RFC 6578) when offered; else CTag then ETag diff with `calendar-multiget`. Writes use `If-Match`. | M | 1 |
| CO02-F065 | Conflict resolution | On HTTP 412: three-way merge by property against the last synced copy; if both sides changed the same property, a side-by-side sheet. Nothing is overwritten silently. | M | 2 |
| CO02-F066 | Offline changes | Edits queue in a journal and replay in order on reconnect. | M | 1 |
| CO02-F067 | Server calendar management | Create, rename, recolour (`calendar-color`), delete, reorder; supported components (VEVENT, VTODO). | S | 2 |
| CO02-F068 | Subscriptions | Read-only `.ics` by HTTPS (ETag and `If-Modified-Since`, refresh from `REFRESH-INTERVAL` or a setting, minimum 15 min) or by a file handle that is re-read on change. `webcal:` is treated as `https:`. | M | 1 |
| CO02-F069 | Per-account network route | `clear`, `vpn:<profile>` or `tor`, enforced by the network service; fail-closed. | S | 2 |
| CO02-F070 | Multiple calendars and colours | 24 palette colours plus custom; colour-blind-safe set; pattern option for high contrast. | M | 1 |
| CO02-F071 | Read-only and shared calendars | WebDAV ACL and `calendarserver` sharing states shown; write actions disabled with a reason. | S | 2 |
| CO02-F072 | Encrypted calendars | A local calendar whose store is encrypted with a key in `svc.keyring`; locked state hides titles everywhere, including notifications and search. Optional file sync as one encrypted blob (age format through SE04). | S | 2 |
| CO02-F073 | Encrypted properties on CalDAV | Title, description, location and attendees are encrypted to the user's own key and stored in `X-PBSD-ENC`; the server sees only time and "Busy". Other clients show "Busy". | C | 3 |
| CO02-F074 | Default calendar rules | Per account default; rule "events created from mail to account X go to calendar Y". | S | 2 |
| CO02-F075 | Sync status and log | Per calendar last sync, error, pending count; log without content. | S | 2 |
| CO02-F076 | Background sync | With `bg.run`: sync on a timer (default 15 min) and on network return; honours `svc.power` and metered state. | S | 2 |

### 4.6 Invitations and scheduling

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO02-F077 | iTIP engine | REQUEST, REPLY, CANCEL, ADD, REFRESH, COUNTER, DECLINECOUNTER, PUBLISH with SEQUENCE and DTSTAMP ordering rules. | M | 2 |
| CO02-F078 | iMIP with Mail | Service `org.pbsd.calendar.itip`: CO01 passes a `text/calendar` part and gets a summary record; Calendar builds the reply and asks CO01's send service, which shows it before sending. | M | 2 |
| CO02-F079 | CalDAV scheduling | Inbox and outbox collections (RFC 6638), `SCHEDULE-AGENT`, `SCHEDULE-STATUS` shown per attendee. | S | 2 |
| CO02-F080 | Attendee editor | Add from the CO03 picker; role (chair, required, optional, non-participant), type (person, group, room, resource), RSVP flag. | M | 2 |
| CO02-F081 | Reply with comment | Accept, tentative, decline, with an optional comment; "propose new time" sends COUNTER. | M | 2 |
| CO02-F082 | Organiser tools | See replies, resend, cancel with a message, accept or decline a counter proposal, message all attendees (compose intent). | S | 2 |
| CO02-F083 | Free/busy lookup | CalDAV `free-busy-query` on the outbox; a per-contact free/busy URL from CO03 (`FBURL`). Requests are listed before first use per server. | S | 2 |
| CO02-F084 | Scheduling grid | Attendee rows against time, busy blocks, a suggested-slots list ranked by "all free", working hours and zone fairness. | S | 2 |
| CO02-F085 | Availability | The user's working hours and VAVAILABILITY (RFC 7953) published to the CalDAV server when supported. | C | 3 |
| CO02-F086 | Publish own free/busy as a file | Export a VFREEBUSY `.ifb` for a range to share by file or mail; titles never included. | S | 3 |
| CO02-F087 | Invitation safety | Unsolicited invitations are never added silently: they stay "pending" until answered. Organiser address is checked against the mail's authenticated sender (result from CO01). | M | 2 |
| CO02-F088 | Delegation | `DELEGATED-TO` and `DELEGATED-FROM` read and shown; delegate from the reply menu. | C | 3 |

### 4.7 Reminders

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO02-F089 | Reminders | VALARM `DISPLAY` and `AUDIO`; relative to start or end, or absolute; several per event. `EMAIL` and `PROCEDURE` alarms are imported but never run. | M | 1 |
| CO02-F090 | Notification actions | Through `svc.notify`: Snooze 5 min, 15 min, 1 h, until start; Dismiss; Open; Join. Snooze state syncs with `X-MOZ-SNOOZE-TIME` and RFC 9074 `ACKNOWLEDGED`. | M | 1 |
| CO02-F091 | Default reminders | Per calendar, separate for timed and all-day events. | M | 1 |
| CO02-F092 | Reminder daemon | A small background part (`bg.run`, `bg.autostart`) that holds only the next alarm times, not the event text, until it fires. | M | 1 |
| CO02-F093 | Missed reminders | On start or resume, reminders missed in the last 24 h are shown once in one grouped notification. | M | 1 |
| CO02-F094 | Private notifications | Lock-screen text is "Event in 10 minutes" unless the user allows titles; encrypted calendars always hide titles while locked. | M | 2 |
| CO02-F095 | Travel-aware reminders | "Leave at" reminder from the travel time. | C | 3 |
| CO02-F096 | Do-not-disturb | Follows the system; option to set do-not-disturb during events marked busy. | C | 3 |

### 4.8 Search, import, export, sharing, printing

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO02-F097 | Search | Local FTS over title, location, description, attendees, categories; operators `cal:`, `with:`, `at:`, `before:`, `after:`, `is:recurring`. Results list in agenda form. | M | 1 |
| CO02-F098 | System search | Titles and times contributed to `svc.search` when enabled (default on for unencrypted calendars, never for encrypted ones). | S | 2 |
| CO02-F099 | Import iCalendar | `.ics` with many components; preview with counts, duplicates by UID, target calendar; lenient reader with a list of repaired problems. | M | 1 |
| CO02-F100 | Import other formats | CSV with column mapping; vCalendar 1.0 (`.vcs`); calendar items handed over from CO01's PST import. | S | 3 |
| CO02-F101 | Export | Calendar, range, selection or one event to `.ics`; CSV; options to strip attendees, alarms, private events. | M | 1 |
| CO02-F102 | Share by file | Event or calendar as `.ics` through the share sheet (`svc.intent`), optionally signed or encrypted by SE05. | M | 2 |
| CO02-F103 | QR for one event | Show a QR code holding a compact VEVENT. | C | 3 |
| CO02-F104 | Print | Day, week, month, agenda and year layouts; range, calendars, paper, orientation, colour or mono, hide private. Output is PDF to `svc.portal.print`. | S | 2 |
| CO02-F105 | Open `.ics` files | Opening a file shows a preview window with "Add to calendar"; nothing is imported on open. | M | 1 |
| CO02-F106 | Backup and snapshots | Store in `svc.backup`; "restore event from snapshot" browses earlier versions of an event from ZFS snapshots. | S | 3 |
| CO02-F107 | Trash | Deleted events kept 30 days locally with restore. | S | 2 |

### 4.9 Integration, settings, accessibility, recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO02-F108 | Tasks overlay | Tasks with due or start dates from CO04's read interface, drawn in the all-day row or at their time; tick to complete; drag to reschedule (write goes through CO04). | S | 2 |
| CO02-F109 | Birthdays and anniversaries | A read-only virtual calendar from CO03's dates interface; age shown when the year is known; default reminder optional. | S | 2 |
| CO02-F110 | Public holidays | Bundled, signed `.ics` per country and region; chosen in settings; no network. | S | 2 |
| CO02-F111 | Shell calendar popover data | Read-only interface `org.pbsd.calendar.upcoming` for the panel clock popover (SH01, SH02): next events for N days, honouring locks. | S | 2 |
| CO02-F112 | Intents | Handles `open`/`view` for `text/calendar`, `new` (event with fields), `webcal:`; emits `compose`, `share`, `print`, `pick`, `open`. | M | 1 |
| CO02-F113 | Command palette | All actions, calendars, "go to date", "new event …" with the quick-add grammar. | M | 1 |
| CO02-F114 | Session restore | View, date, visible calendars and open editors return after restart or crash; unsaved editor text is journalled. | M | 1 |
| CO02-F115 | Store recovery | Integrity check; rebuild occurrence cache and search index; re-sync a calendar from the server keeping unsynced local changes. | M | 2 |
| CO02-F116 | Audit entries | First contact with each server, subscription fetches, free/busy queries to `svc.audit`. | S | 2 |
| CO02-F117 | Agenda-first accessibility mode | Opens in agenda view with full sentences per event for screen readers. | M | 2 |
| CO02-F118 | Statistics | Hours per calendar or category over a range, as a `chart`; local only. | C | 3 |
| CO02-F119 | Scripting hooks | ForgeScript automation entries on "event created/changed/starting" with no added capabilities; user-signed. | C | 3 |

## 5. GUI

### 5.1 Window map

```
Calendar
├─ Main window
│  ├─ Header bar: menu, title (visible range), search, [net] badge
│  ├─ Toolbar: [New] · Today · ‹ › · range title · view switch · zone button
│  ├─ Sidebar: mini calendar, visibility sets, calendars by account, overlays
│  ├─ Content: day | multi-day | week | month | year | agenda | timeline
│  ├─ Inspector (F4): selected event details, attendees, reminders, history
│  └─ Status bar: sync state, pending changes, view zone
├─ Event editor (dialog; tabs Details, Repeat, Attendees and scheduling, Reminders, Attachments)
├─ Popovers: event card · quick add · "+n more" day list · go to date · zone picker ·
│  time converter · colour picker · reminder snooze list
├─ Sheets: edit scope (this / following / all) · sync conflict · import preview ·
│  zone-change review · invitation review
├─ Dialogs: account wizard · subscribe · calendar properties · custom recurrence ·
│  print setup · export · preferences
├─ File preview window (for an opened .ics)
└─ Shell-drawn: notifications with actions, file portal, print dialog, capability prompts
```

### 5.2 Main window

Week view at 1280×800.

```
+------------------------------------------------------------------------------------------+
| [=] Calendar — 16–22 Nov 2026        [/ Search events…        ]            [net] _ ▢ ✕   |
+------------------------------------------------------------------------------------------+
| [+ New] [Today] [‹] [›]  November 2026 · Week 47 (1) [Day|Week|Month|Year|Agenda] [AEDT v]|
+--------------------+---------------------------------------------------------------------+
|   November 2026 (2)|      | Mon 16 | Tue 17 | Wed 18 | Thu 19 | Fri 20 | Sat 21 | Sun 22  |
| Mo Tu We Th Fr Sa Su| all  |        |[Rae away ══════════════]|        |🎂 Sol  |      (4)|
|                  1 | day  |        |        |        |        |☐ Tax   |        |         |
|  2  3  4  5  6  7  8+------+--------+--------+--------+--------+--------+--------+---------+
|  9 10 11 12 13 14 15| UTC  AEDT    |        |        |        |        |        |         |
| [16 17 18 19 20 21 22]| 21  08     |        |        |        |        |        |      (5)|
| 23 24 25 26 27 28 29| 22  09 |Stand- |Stand-  |Stand-  |Stand-  |Stand-  |        |         |
| 30                  |        |up ↻   |up ↻    |up ↻    |up ↻    |up ↻    |        |         |
|                     | 23  10 |       |        |┌──────┐|        |        |        |         |
| SETS            (3) |        |       |        |│Design│|        |        |        |         |
|  (•) Everything     | 00  11 |       |▒travel▒|│review│|        |        |        |         |
|  ( ) Work           |        |       |┌──────┐|│ 👥 4 │|        |        |        |      (6)|
| PERSONAL            | 01  12 |       |│Lunch ││└──────┘|        |        |        |         |
|  [x] ● Home         |        |       |│w/ Sol│|        |────────|── now ─|        |      (7)|
|  [x] ● Family 🔒    | 02  13 |       |└──────┘|        |        |        |        |         |
| work.example.org    |        |       |        |        |░Budget░|        |        |         |
|  [x] ● Work         | 03  14 |       |        |        |░review░|        |        |      (8)|
|  [ ] ● Team         |        |       |        |        |░ ?    ░|        |        |         |
| SUBSCRIBED          | 04  15 |       |        |        |        |        |        |         |
|  [x] ● Holidays AU  |        |       |        |        |        |        |        |         |
| OVERLAYS            |        |       |        |        |        |        |        |         |
|  [x] Tasks [x] Bdays|        |       |        |        |        |        |        |         |
+--------------------+---------------------------------------------------------------------+
| Synced 09:15 · 0 pending · Showing times in Australia/Sydney (AEDT, UTC+11)        (9)   |
+------------------------------------------------------------------------------------------+
```

1. Toolbar: one primary action, date stepping, range title, view switch (segmented `tabBar`), view zone button.
2. Mini calendar; the visible range is outlined; drag across days sets a multi-day view.
3. Visibility sets, then calendars grouped by account with a check box and colour dot; a lock marks encrypted calendars; a warning icon marks sync errors.
4. All-day row: multi-day bars, birthday overlay (cake icon), task overlay (check box).
5. Hour gutter with a second zone column (UTC here).
6. Timed event block with attendee count; the hatched lead-in above "Lunch" is travel time.
7. The "now" line in `--accent` with the word "now" for non-colour recognition.
8. A pending invitation: dashed border, question mark, hatched fill until answered.
9. Status bar: sync state, pending journal count, view zone (click to change).

Sizes: sidebar 220 px, hour gutter 48 px per zone column, minimum day column 80 px (below that the view drops to fewer days and scrolls).

### 5.3 Other views and dialogs

#### 5.3.1 Month view (content area)

```
+------+----------+----------+----------+----------+----------+----------+----------+
|  Wk  | Mon      | Tue      | Wed      | Thu      | Fri      | Sat      | Sun      |
+------+----------+----------+----------+----------+----------+----------+----------+
|  47  | 16       | 17       | 18       | 19       |[20]      | 21       | 22       |
|      | 09 Stand…|[Rae away ═══════════]| 14 Budg ?| 09 Stand…| 🎂 Sol   |          |
|      |          | 12 Lunch | 10 Desig…| 09 Stand…| ☐ Tax    |          |          |
|      |          | +2 more  |          |          |          |          |      (1) |
+------+----------+----------+----------+----------+----------+----------+----------+
|  48  | 23       | 24       | …                                                    |
```

1. "+2 more" opens a popover with the day's full list. Today is boxed and labelled. Drag an event to another day keeps its time. Double-click a day creates an event. States: loading (grey chips), filtered ("3 calendars hidden" hint in the status bar).

#### 5.3.2 Event editor (720×600)

```
+---------------------------------------------------------------------------+
| Edit event                                                                |
+---------------------------------------------------------------------------+
| [Details] [Repeat] [Attendees and scheduling] [Reminders] [Attachments]   |
+---------------------------------------------------------------------------+
| Title     [Design review_____________________________]   Calendar [● Work v]|
| Starts    [Wed 18 Nov 2026] [10:00] [Australia/Sydney v]   [ ] All day    |
| Ends      [Wed 18 Nov 2026] [11:30] [same zone v]          1 h 30 min (1) |
| Repeat    [Does not repeat v]                                              |
| Location  [Room 4_____________________] [Show on map]                     |
| Join link [https://meet.example.org/abc__] [Open]                         |
| Travel    before [15 min v]  after [none v]                               |
| Show as   (•) Busy ( ) Free     Visibility [Default v]   Status [Confirmed v]|
| Tags      [design ✕] [____]                                               |
| Notes     +-------------------------------------------------------------+ |
|           | Bring the wireframes.                                       | |
|           +-------------------------------------------------------------+ |
+---------------------------------------------------------------------------+
| ⚠ Overlaps "Stand-up" 10:00–10:15                                    (2)  |
| [Delete]                                   [Cancel]  [Save and send] (3)  |
+---------------------------------------------------------------------------+
```

1. Duration updates live; changing start keeps the duration. 2. Inline conflict hint, not a dialog. 3. The primary button reads "Save" with no attendees, "Save and send" when invitations go out, "Send update" for changes. States: read-only (fields disabled, banner "This calendar is read-only"), invited (only reminders, show-as, private notes editable; reply buttons shown), offline ("Will sync later").

#### 5.3.3 Custom recurrence (560×520)

```
+------------------------------------------------------------+
| Repeat                                                     |
+------------------------------------------------------------+
| Every [1 ] [month(s) v]                                    |
| ( ) On day [18]                                            |
| (•) On the [third v] [Wednesday v]                         |
| ( ) On days matching… [BYSETPOS editor]                    |
| Only in months  [x]J [x]F [x]M [x]A [x]M [x]J …            |
| Ends  (•) Never  ( ) After [10] times  ( ) On [date]       |
+------------------------------------------------------------+
| Every month on the third Wednesday.                   (1)  |
| Next: 16 Dec 2026 · 20 Jan 2027 · 17 Feb 2027 · … [more]   |
| Rule: FREQ=MONTHLY;BYDAY=3WE                 [Edit text](2)|
|                                    [Cancel]  [Done]        |
+------------------------------------------------------------+
```

1. Plain-language summary from the locale table. 2. Raw RRULE; invalid parts are underlined with the reason.

#### 5.3.4 Attendees and scheduling tab

```
+---------------------------------------------------------------------------+
| [+ Add attendee…]                         Wed 18 Nov  [‹][›]  [Zoom v]    |
+----------------------------+----------------------------------------------+
| Attendee        Role  RSVP | 08   09   10   11   12   13   14   15   16   |
| Rae (organiser) chair  ✔   | ░░░░░    ████                    ████        |
| Sol Marsh       req    ?   |      ████     ▒▒▒▒▒▒                          |
| Ines Vale       req    ✔   | ████                    ████████             |
| Room 4          room   –   |           ████                               |
| tam@example.net opt    ✖   | ·· no free/busy data ··                 (1)  |
+----------------------------+----------------------------------------------+
| Suggested: Wed 11:30–13:00 (all free) · Thu 09:30 · Thu 15:00       (2)   |
| Free/busy asked from: work.example.org                              (3)   |
+---------------------------------------------------------------------------+
```

1. Missing data is stated, never drawn as free. 2. Click a suggestion to set the time. 3. Lists every server asked. Legend: solid busy, hatched tentative, dotted outside working hours.

#### 5.3.5 Agenda view (content area)

```
| Today · Thursday 19 November 2026                              3 events   |
|   09:00–09:15   ● Stand-up ↻                    Work                      |
|   14:00–15:00   ● Budget review  ? pending      Work · Room 4   [Reply v] |
|   all day       ═ Rae away (day 3 of 4)         Home                      |
| Friday 20 November                                             3 items    |
|   09:00–09:15   ● Stand-up ↻                    Work                      |
|   all day       ☐ Tax return (task)             Tasks                     |
| Saturday 21 November                                                      |
|   all day       🎂 Sol Marsh turns 34            Birthdays                 |
| — no events until Tuesday 24 November —                              (1)  |
```

1. Empty stretches collapse to one line. Sticky day headers. `Enter` opens, `Delete` deletes, `Space` shows the card.

#### 5.3.6 Account wizard and subscribe (600×460)

```
+----------------------------------------------------------+
| Add a calendar                                           |
+----------------------------------------------------------+
| (•) CalDAV account   ( ) Subscribe to a link  ( ) Local  |
| Address or server [rae@example.org__________]            |
| Route  (•) Clear ( ) VPN [v] ( ) Tor                     |
| Found: https://dav.example.org/cal/rae/ (well-known) (1) |
| Calendars   [x] ● Work      read/write                   |
|             [x] ● Team      read only                    |
|             [ ] ● Tasks     tasks only → offered to CO04 |
| [x] Also add address books to Contacts (2 found)    (2)  |
|                                  [Cancel]  [Add]         |
+----------------------------------------------------------+
```

1. Discovery source stated. 2. One account record in `svc.accounts` serves CO02, CO03 and CO04; each app only enables its own collections.

#### 5.3.7 Sync conflict sheet

```
+----------------------------------------------------------------+
| "Design review" changed here and on the server                 |
+-----------------------------+----------------------------------+
| Yours                       | Server                           |
| Starts  Wed 10:00           | Starts  Wed 10:30  ← differs     |
| Place   Room 4              | Place   Room 4                   |
| Notes   Bring wireframes ←  | Notes   (empty)                  |
+-----------------------------+----------------------------------+
| [Keep mine] [Keep server] [Merge: time from server, notes mine]|
+----------------------------------------------------------------+
```

### 5.4 Menus

Items separated by ` · `; shortcut in brackets.

```
File     New event [Ctrl+N] · New from quick add [Q] · New calendar… · Add account… ·
         Subscribe… · Open file… [Ctrl+O] · Import… · Export… [Ctrl+Shift+S] ·
         Share event… · Print… [Ctrl+P] · Close [Ctrl+W] · Quit [Ctrl+Q]
Edit     Undo [Ctrl+Z] · Redo [Ctrl+Shift+Z] · Cut · Copy · Paste [Ctrl+X/C/V] ·
         Duplicate [Ctrl+D] · Delete [Delete] · Edit event [Enter] · Find [Ctrl+F, /] ·
         Preferences [Ctrl+,]
View     Day [1 or D] · Week [2 or W] · Work week [Shift+W] · Month [3 or M] · Year [4 or Y] ·
         Agenda [5 or A] · Timeline [6] · Multi-day > 2…14 [Ctrl+2…9] · Sidebar [F9] ·
         Inspector [F4] · Second time zone… · Show view in zone… · Week numbers ·
         Hide declined · Overlays > Tasks | Birthdays | Holidays | Moon | Sun ·
         Zoom in/out/reset [Ctrl++ / Ctrl+- / Ctrl+0] · Full screen [F11]
Go       Today [T] · Previous/Next [Left/Right, or P/N] · Previous/Next event [K/J] ·
         Go to date… [G] · Back/Forward [Alt+Left/Right]
Event    Accept [Ctrl+Shift+A] · Tentative · Decline [Ctrl+Shift+D] · Propose new time… ·
         Move to calendar > · Show as > Busy | Free · Add reminder > · Copy as text ·
         Email attendees · Open join link [Ctrl+J] · Show on map · Open source
Calendar Sync now [F5] · Sync all [Shift+F5] · Properties… · Colour > · Visibility sets… ·
         Lock encrypted calendars [Ctrl+Shift+L] · Sync log · Check store
Help     Calendar help [F1] · Keyboard shortcuts [Ctrl+/] · About Calendar
```

### 5.5 Keyboard and pointer

Grid focus: arrows move the time cursor by one slot or day; `Shift+arrows` extend a selection; `Enter` on a selection creates an event, on an event opens it; `Space` shows the event card; `Alt+arrows` move the event; `Shift+Alt+Up/Down` resize; `Tab` walks events in time order; `Home`/`End` go to the start and end of working hours; `PageUp`/`PageDown` step the view.

Context menus:

- **Event**: Open, Edit, Duplicate, Copy, Cut, Delete, Move to calendar, Reply (Accept, Tentative, Decline), Show as, Add reminder, Share as .ics, Email attendees, Copy as text, Open source.
- **Empty slot**: New event here, New all-day event, Paste, Go to date, Zoom.
- **Calendar in sidebar**: Show only this, Colour, Rename (`F2`), Properties, Sync now, Export, Unsubscribe or Delete, Lock.
- **Day header**: Open day, New all-day event, Print this day.

Drag and drop:

| Drag | Onto | Result |
|---|---|---|
| Event | Another time or day | Move (Ctrl: copy); series asks for scope |
| Event edge | Time | Resize |
| Event | Calendar in sidebar | Move to that calendar |
| Event | Mini calendar day | Move to that date, same time |
| Event | Outside (Files, Mail compose) | `.ics` file; plain-text summary |
| `.ics` file | Window | Import preview |
| Text or a mail message | Grid slot | New event with the text as title or the message linked |
| Task (from CO04) | Grid slot | Sets the task's start and due through CO04 |
| Contact (from CO03) | Event | Adds an attendee |

Pointer: wheel scrolls time; `Ctrl`+wheel zooms; `Shift`+wheel steps days; pinch zooms; two-finger swipe steps the range.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Grid still shown. Sidebar says "No calendars yet" with [Add local calendar] and [Add account]. A default local calendar is created on first event. |
| Loading | Cached events drawn at once; a thin progress line; new data fades in at 80 ms. |
| Error | Warning icon on the calendar row; status bar "Work: sign-in failed · Details". Events stay visible from cache. |
| Offline | Status "Offline · 2 pending"; edits allowed; free/busy and invitations by CalDAV disabled with a reason. |
| Locked | Encrypted calendars show blocks as "Busy" with a lock, or hide entirely (setting); [Unlock] in the sidebar. |
| Read-only | Events have no drag handles; editor fields disabled; banner states why. |
| Narrow (<640) | Sidebar is an overlay; week view becomes a 3-day or 1-day pager; month shows dots and a day list below; editor is a full-window sheet. |
| Zone database missing zone | Event shows a warning chip "Unknown zone, using embedded rules". |

### 5.7 Widgets

| Region | FUSE widget | Notes |
|---|---|---|
| Day, week, multi-day | `dayView`, `weekView` | Not in V3. Planned as FUSE-WP-34 (`CO02-GAP-1`) |
| Month, year | `monthView`, `yearView` | FUSE-WP-34 |
| Agenda | `agendaView` or `modelListView` with sticky headers | FUSE-WP-34 or FUSE-WP-28 |
| Timeline, scheduling grid | `timeline` (exists, V3) adapted, or a time-grid variant | `CO02-GAP-2`: horizontal resource time grid |
| Mini calendar | `calendar` | needs range select and week numbers (FUSE-WP-34.6) |
| Sidebar lists | `treeView`, `checkBox`, `radioButton` | |
| Toolbar | `button`, `toolButton`, `tabBar`, `comboBox` | |
| Editor fields | `lineEdit`, `dateTimeEdit`, `comboBox`, `checkBox`, `spinBox`, `tabWidget` | zone picker is a `comboBox` with search |
| Notes field | G-10 rich text (subset) or `codeEdit` in plain mode | |
| Attendee list | `tableView` | |
| Colour | `colorPicker` | |
| Statistics | `chart` | |
| Popovers, sheets, toasts | missing in V3; FUSE-WP-30 | |
| Tags, attendees input | missing: tag input (same gap as CO01-GAP-1) | |
| Print preview | page view (G-16) | |

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | View models, editing, undo, journal, quick-add grammar, iTIP state machine |
| UI | FUSE UI | app host | Windows and views |
| Recurrence engine | ForgeScript module (pure functions) | app host; also loaded by CO04 | RRULE/RDATE/EXDATE expansion, zone arithmetic over FORGE date-time |
| iCalendar helper | C++23 | pooled, `proc.helper:ical` | Parse and serialise iCalendar, vCalendar 1.0, VTIMEZONE; lenient repair. No capabilities. Offered to CO01, CO04 through the bus. |
| DAV sync helper | C++23 | one per account, `proc.helper:dav` | WebDAV, CalDAV, CardDAV: PROPFIND, REPORT, PUT, DELETE, sync-collection; XML parser. Sockets from the network service. Shared binary with CO03 and CO04. |
| Fetch helper | C++23 | `proc.helper:fetch` | Subscription downloads, discovery; no cookies, fixed user agent |
| Reminder daemon | ForgeScript (small entry) | background app host | Holds next alarm times; fires `svc.notify` |
| Print composer | ForgeScript | app host | Lays out pages to the PDF spool (G-16) |

```
 CO01 ──itip──┐      CO04 ──tasks read/write──┐     CO03 ──dates, picker──┐
              v                               v                          v
        +---------------------------------------------------------------------+
        | app host: view models · recurrence engine · journal · iTIP · store  |
        +----+--------------+--------------+----------------+-----------------+
             |              |              |                |
        +----v----+   +-----v-----+   +----v----+    +------v-------+
        | ical    |   | dav sync  |   | fetch   |    | reminder     |
        | helper  |   | helper    |   | helper  |    | daemon       |
        | no caps |   | sockets   |   | sockets |    | svc.notify   |
        +---------+   +-----+-----+   +----+----+    +--------------+
                            v              v
                     network service (rules, route, TLS)
```

### 6.2 Data model

Store: `~/.local/share/org.pbsd.calendar/calendar.sqlite` (schema version in `meta`), `objects/` raw iCalendar per object for round-trip fidelity, `enc/<calendar-id>.sqlite` for encrypted calendars (page-encrypted with XChaCha20-Poly1305 under a key from `svc.keyring`; FORGE has this primitive, see FORGE-WP-22).

| Entity | Fields |
|---|---|
| `account` | `id: uuid`, `accounts_ref: text`, `principal_url: text`, `home_set: text`, `route: text`, `schedule_inbox: text`, `schedule_outbox: text` |
| `calendar` | `id: uuid`, `account_id: uuid?`, `kind: enum(local, caldav, subscription, virtual)`, `name`, `colour: text`, `url: text`, `ctag: text`, `sync_token: text`, `components: u8`, `read_only: bool`, `encrypted: bool`, `default_alarm_timed: i32?`, `default_alarm_allday: i32?`, `default_tz: text`, `order: u16`, `visible: bool`, `refresh_s: u32` |
| `object` | `id: i64`, `calendar_id`, `uid: text`, `href: text`, `etag: text`, `raw_ref: text`, `base_ref: text` (last synced copy for merges), `dirty: bool`, `deleted_at: i64?` |
| `event` | `id: i64`, `object_id`, `recurrence_id: text?`, `summary`, `location`, `description`, `dtstart: i64` (UTC s), `dtend: i64`, `start_tz: text?`, `end_tz: text?`, `all_day: bool`, `floating: bool`, `rrule: text?`, `rdates: json`, `exdates: json`, `status: enum`, `transp: enum`, `class: enum`, `sequence: u32`, `organizer: text?`, `my_partstat: enum?`, `categories: json`, `url: text`, `conference: json`, `travel_before_s: u32`, `travel_after_s: u32`, `colour: text?`, `source_link: text?` |
| `attendee` | `event_id`, `address`, `name`, `role: enum`, `cutype: enum`, `partstat: enum`, `rsvp: bool`, `schedule_status: text`, `delegated_to`, `delegated_from` |
| `alarm` | `event_id`, `action: enum(display, audio)`, `trigger_rel_s: i32?`, `related_end: bool`, `trigger_abs: i64?`, `acknowledged: i64?`, `snoozed_until: i64?` |
| `occurrence` | `event_id`, `start: i64`, `end: i64`, `override_id: i64?` (cache; rebuilt at will) |
| `next_alarm` | `fire_at: i64`, `event_id`, `occurrence_start: i64` (the only table the daemon reads) |
| `journal_op` | `seq`, `calendar_id`, `kind: enum(put, delete, move, reply)`, `object_id`, `state`, `attempts`, `error` |
| `itip_pending` | `id`, `uid`, `method: enum`, `sequence`, `dtstamp`, `from_addr`, `auth_result: text`, `raw_ref`, `state` |
| `visibility_set` | `id`, `name`, `calendar_ids: json` |
| `local_note` | `uid`, `recurrence_id`, `text` (never synced) |

Unknown properties and parameters are preserved because writes patch the raw object, not regenerate it. Migration is forward only with a copy of the previous database.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| iCalendar RFC 5545, 7986, 9073, 9074 | yes | yes | own | iCalendar helper |
| vCalendar 1.0 | yes | no | own | iCalendar helper |
| iTIP RFC 5546, iMIP RFC 6047 | yes | yes | own | helper (parse), app (state) |
| VFREEBUSY, VAVAILABILITY RFC 7953 | yes | yes | own | iCalendar helper |
| WebDAV RFC 4918, sync RFC 6578, ACL RFC 3744 | yes | yes | own; HTTP and TLS from FORGE-WP-21 | DAV sync helper |
| CalDAV RFC 4791, scheduling RFC 6638, managed attachments RFC 8607 | yes | yes | own | DAV sync helper |
| XML (DAV bodies) | yes | yes | own non-validating parser, no DTD, no entities | DAV sync helper |
| IANA time-zone data (TZif) | yes | — | FORGE zones (FORGE-WP-23.2) | app host (trusted system data) |
| CLDR Windows-zone map | yes | — | bundled table | app host |
| CSV | yes | yes | own | iCalendar helper (import), app (export) |
| PDF (print) | — | yes | G-16 print pipeline | print service |
| QR code | — | yes | own encoder | app host |
| age-encrypted calendar blob | yes | yes | SE04 engine | SE04 engine |

No third-party library is proposed.

### 6.4 Integration

- **Intents handled**: `open`, `view` (`text/calendar`), `new` (event from fields), `share` (receive an `.ics`). **Emitted**: `compose` (CO01), `share`, `print`, `pick` (contact), `open` (links, attachments, map).
- **URL schemes**: `webcal:`, `webcals:`; app links `pbsd-calendar:event/<uid>`.
- **MIME**: view/edit/import/export `text/calendar`; import `text/x-vcalendar`, `text/csv`; export `text/csv`, `application/pdf`.
- **Services used**: `svc.accounts`, `svc.keyring`, `svc.notify`, `svc.settings`, `svc.intent`, `svc.portal.file`, `svc.portal.print`, `svc.search`, `svc.backup`, `svc.audit`, `svc.power`, `svc.clipboard`, `svc.undo-trash`, `svc.a11y`, `svc.model` (optional).
- **Services offered** (Proposed): `org.pbsd.calendar.itip` (summarise and answer invitations; used by CO01), `org.pbsd.calendar.ical` (parse and serialise; used by CO04, CO01), `org.pbsd.calendar.upcoming` (read-only next events; used by SH01/SH02), `org.pbsd.calendar.freebusy` (own busy blocks for a range; used by CO01's invitation card).
- **Other apps**: CO01 (iMIP transport), CO03 (picker, birthdays, `FBURL`), CO04 (task overlay, shared DAV and recurrence code), IN04 (travel estimate, map), CO05 (join links), UT02 (no overlap: Clocks owns alarms), SE04 (certificates, age), SE05 (sign or encrypt shared files), SE03 (routes).
- **Backing repos**: Verified: FUSE has `calendar`, `dateTimeEdit`, `timeline`, `chart` (`FUSE:Source/FUSE/UI/Widgets/include/fuse/ui/widgets/widgets.hpp` per PLATFORM.md V3) and `calendarDate` helpers (`FUSE:docs/ui/README.md`). Scheduler views do not exist; they are planned in `Apps/00-foundation/build-plans/FUSE_BUILD_PLAN.md` (FUSE-WP-34). FORGE zones and calendars are cited there from `FORGE:docs/text-locale.md`, which this writer did not open: Unverified here.

## 7. Security and privacy

### 7.1 Capabilities requested

| Capability | Reason |
|---|---|
| `fs.app` | Store, raw objects, cache |
| `fs.portal` | Import, export, file-based subscriptions and sharing |
| `net.user` | CalDAV servers and subscription hosts the user typed |
| `net.route:clear`, `net.route:vpn`, `net.route:tor` | Per-account route |
| `proc.helper:ical`, `proc.helper:dav`, `proc.helper:fetch` | Helpers in 6.1 |
| `svc.accounts`, `svc.keyring` | Endpoints; passwords, tokens, calendar keys |
| `svc.notify` | Reminders, invitation notices |
| `svc.intent`, `svc.portal.file`, `svc.portal.print` | Hand-offs, files, printing |
| `svc.search` | Optional contribution of titles |
| `svc.settings`, `svc.backup`, `svc.audit`, `svc.power`, `svc.clipboard`, `svc.undo-trash`, `svc.a11y` | Standard services |
| `svc.model`, `ai.local` | Optional quick-add parse; off by default |
| `bg.run`, `bg.autostart` | Reminder daemon and background sync |

Not requested: `dev.location` (sun times use a typed location), `proc.exec`, `fs.path`.

### 7.2 Threat model

Assets: where the user will be and when, who they meet, join links, credentials.

| Adversary | Attack | Mitigation |
|---|---|---|
| Sender of an invitation or `.ics` | Malformed iCalendar to crash or exploit the parser | Parsing in a zero-capability helper; T2 target; fuzzing |
| Sender | Calendar spam: events appearing unasked | Invitations stay pending; nothing auto-added; organiser checked against authenticated sender |
| Sender | Recurrence bomb (`FREQ=SECONDLY`, huge COUNT, 10 000 RDATEs) | Lazy expansion with a hard cap per query (10 000 occurrences), time budget, size limits per object (1 MiB) |
| Sender | Alarm abuse (`PROCEDURE`, `EMAIL`, `ATTACH` to a URL) | Only DISPLAY and AUDIO run; audio is a system sound, never a fetched file |
| Sender | Phishing join links, look-alike hosts | Real host shown; per-host first-open confirmation |
| Subscription host | Tracking by fetch time, IP, user agent | Fixed user agent, no cookies, per-account route, minimum interval, jittered schedule |
| Subscription host | Replacing content with hostile or misleading events | Subscriptions are read-only and visually marked; alarms from subscriptions off by default |
| CalDAV server | Reads all event content | Encrypted calendars (local) and encrypted properties (CalDAV) |
| Network attacker | Interception | TLS required; certificate errors stop |
| Other local app | Reading the schedule | Store under `fs.app`; read interfaces (`upcoming`, `freebusy`) are capability-gated and honour locks |
| Shoulder surfer, lock screen | Titles in notifications | Private notification mode |
| Free/busy requester | Learning details | Published free/busy holds times only |

What leaves the machine: DAV traffic to the user's servers, subscription fetches, free/busy queries the user started, invitations the user sent through CO01. Nothing else.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `view.default` | enum day/week/work/month/agenda | week | View at start |
| `view.first_day` | enum locale/mon/sat/sun | locale | First day of week |
| `view.work_days` | set | Mon–Fri | Work-week days |
| `view.work_start`, `view.work_end` | time | 09:00, 17:00 | Working hours |
| `view.snap_minutes` | enum 5/10/15/30 | 15 | Drag snap |
| `view.week_numbers` | enum off/iso/locale | off | Week numbers |
| `view.extra_zones` | list of TZID | empty | Extra hour columns |
| `view.hide_declined` | bool | true | Hide declined events |
| `view.time_format` | enum locale/12/24 | locale | Clock format |
| `event.default_duration_min` | int | 60 | New event length |
| `event.default_calendar` | calendar id | first local | Target for new events |
| `event.default_visibility` | enum | default | CLASS for new events |
| `reminder.default_timed_min` | int? | 10 | Default reminder, timed |
| `reminder.default_allday` | time? | 09:00 the day before | Default reminder, all-day |
| `reminder.show_titles_locked` | bool | false | Titles on the lock screen |
| `reminder.subscriptions` | bool | false | Allow alarms from subscriptions |
| `invite.auto_add_pending` | bool | true | Show pending invitations on the grid |
| `invite.send_replies` | enum ask/always/never | ask | Reply behaviour |
| `sync.interval_min` | int | 15 | Background sync |
| `sync.subscription_min` | int ≥15 | 360 | Subscription refresh |
| `sync.metered` | enum normal/pause | pause | Metered networks |
| `tz.follow_system` | enum ask/always/never | ask | Follow system zone changes |
| `overlay.tasks`, `overlay.birthdays` | bool | true | Overlays |
| `overlay.holidays` | list of region codes | empty | Holiday files |
| `overlay.location` | lat, lon? | none | For sun times; typed by the user |
| `search.system_index` | bool | true | Titles to `svc.search` |
| `locked.show_busy` | bool | true | Show locked events as "Busy" |
| `trash.keep_days` | int | 30 | Local trash retention |
| `ai.quick_add` | bool | false | Model fall-back for quick add |

## 9. Accessibility and languages

- Each event's a11y name is a sentence: "Design review, Wednesday 18 November, 10 to 11:30 am, Room 4, Work calendar, 4 attendees, repeats monthly."
- Grid uses grid roles; row and column headers are spoken on move; the "now" position is reachable with `Home` then `N`.
- Every drag action has a keyboard equivalent (section 5.5) and a menu equivalent ("Move to…", "Change duration…").
- Calendar colours are paired with a pattern or initial in high contrast; state (tentative, cancelled, pending) never relies on colour.
- Calendar systems: Gregorian display everywhere; optional secondary date line in another calendar (Hijri, Hebrew, Chinese, Persian, Japanese era) when `forge::text` supplies it (Unverified which exist).
- First day of week, weekend days, week numbering and date formats follow the locale; right-to-left locales mirror the week (Sunday or Saturday on the right).
- Quick-add grammars are per language table; a language without a table falls back to English with ISO dates.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame (week view from cache) | under 300 ms |
| Switch view or step range | under 50 ms with 100 000 stored events |
| Frame time with 10 000 occurrences in range | 4 ms layout (FUSE-WP-34 budget), 16 ms total |
| Expand a rule with 5 000 occurrences in range | under 20 ms |
| Import a 50 MiB `.ics` (200 000 events) | under 60 s, UI responsive, cancellable |
| Initial CalDAV sync of 20 000 objects | under 3 min on 20 Mbit/s |
| Memory | under 150 MiB with 200 000 events; reminder daemon under 20 MiB |
| Idle | zero frames; one timer wake per minute only while the now line is visible; daemon sleeps until the next alarm |
| Largest supported | 500 calendars, 1 000 000 events |

## 11. Testing and proof

Test plan (targets; nothing has run):

- **Unit**: recurrence engine against the RFC 5545 examples and a vendored corpus of rule and expected-date pairs; differential comparison with occurrence lists generated ahead of time by `python-dateutil` and `libical` (data files only, no runtime dependency).
- **Property tests**: expansion is monotonic; windowed expansion equals the slice of full expansion; split series ("this and following") plus the original equals the pre-split set except the edited part; move and undo is identity.
- **Zones**: every zone transition for 1970–2040 in the system database; events on gap and overlap times; a zone database swap test.
- **iCalendar corpus**: files produced by common servers and clients; round-trip must keep unknown properties byte-equal after unfolding.
- **iTIP**: the state table from RFC 5546 as table-driven tests; out-of-order and replayed messages.
- **DAV**: fake server with scripted responses; differential runs against Radicale, Baïkal and a Cyrus jail offline; 412 conflict paths; interrupted sync.
- **Fuzz**: iCalendar parser, RRULE text parser, DAV XML parser, CSV import, quick-add grammar.
- **Golden renders**: each view with the six overlap patterns, DST days, RTL, high contrast, 200%.
- **Scripted UI**: the 15 top tasks through the a11y tree, keyboard only.
- **Reminder timing**: fake clock; suspend and resume; zone change while asleep.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Recurrence engine | Terminates within the cap; results sorted, unique, inside the window; windowed equals sliced | T1 for termination and window bounds, T3 for RFC conformance | PRISM on a C++23 transcription or on ForgeScript if PRISM gains a front end; property tests | PRISM does not read ForgeScript today (assumption, Unverified) |
| iCalendar helper | No runtime errors; bounded nesting and line length | T2 | PRISM bounded checks, fuzzing | — |
| DAV XML parser | No runtime errors; no entity expansion | T2 | PRISM, fuzzing | — |
| iTIP state machine | A lower SEQUENCE never replaces a higher one; CANCEL only from the organiser | T1 | Small state model proved; table tests | Needs the model written |
| Sync journal and merge | No silent overwrite: a 412 always yields merge or sheet | T3 | Model-based tests against the fake server | Server model |
| Reminder daemon | Never reads event text | T3 | Code review; schema grants only `next_alarm` | — |
| Zone arithmetic | Round trip local→UTC→local outside gaps | T2/T3 | Exhaustive transition tests | Depends on FORGE's library tier |
| SQLite | Vendored C | T4 | — | Third-party C |

## 12. Build plan

### Phase 1 — usable core

| ID | Work item | Depends on |
|---|---|---|
| CO02-W01 | Store, raw object files, journal, migrations | G-07, G-09, FORGE-WP-23.1 |
| CO02-W02 | iCalendar helper (parse, serialise, VTIMEZONE) | FORGE-WP-27, FORGE-WP-28 |
| CO02-W03 | Recurrence engine and occurrence cache | FORGE-WP-23.2 |
| CO02-W04 | Zone handling, zone picker, Windows-zone map | FORGE-WP-23.2 |
| CO02-W05 | Day, week, month, agenda views and mini calendar | G-01, G-02, FUSE-WP-34 |
| CO02-W06 | Event editor, popover, direct manipulation, undo, edit scopes | CO02-W03, CO02-W05 |
| CO02-W07 | Quick-add grammar (English first) | CO02-W03 |
| CO02-W08 | Local calendars; import, export, `.ics` preview | CO02-W02 |
| CO02-W09 | DAV sync helper; CalDAV accounts; offline journal replay | FORGE-WP-21, `svc.accounts`, `svc.keyring` |
| CO02-W10 | Subscriptions and fetch helper | FORGE-WP-21 |
| CO02-W11 | Reminders and reminder daemon | `svc.notify`, `bg.run` |
| CO02-W12 | Search, command palette, session restore | CO02-W01 |
| CO02-W13 | Test corpus, fake DAV server, fuzzers, goldens | FORGE-WP-35 |

### Phase 2 — full feature

| ID | Work item | Depends on |
|---|---|---|
| CO02-W14 | iTIP engine and `itip` service; invitation flow with Mail | CO01 (send service), CO02-W02 |
| CO02-W15 | Attendee editor, CalDAV scheduling, free/busy, scheduling grid | CO03 picker, CO02-W09 |
| CO02-W16 | Conflict merge sheet, server calendar management, shared calendars | CO02-W09 |
| CO02-W17 | Year and multi-day views, extra zone axes, view zone, 23/25 h days | FUSE-WP-34 |
| CO02-W18 | Custom recurrence editor, exception list | CO02-W03 |
| CO02-W19 | Tasks overlay, birthdays, holidays, `upcoming` service | CO04, CO03, SH01 |
| CO02-W20 | Encrypted local calendars, private notifications | `svc.keyring`, FORGE-WP-22 |
| CO02-W21 | Travel time, conference links, attachments, categories | IN04 (optional) |
| CO02-W22 | Printing | G-16 |
| CO02-W23 | Per-account route, background sync, audit entries | SE03, `svc.power` |
| CO02-W24 | More quick-add languages; agenda-first a11y mode | CO02-W07 |

### Phase 3 — advanced

| ID | Work item | Depends on |
|---|---|---|
| CO02-W25 | Timeline view, split view | `CO02-GAP-2` |
| CO02-W26 | Encrypted properties on CalDAV; encrypted file sync | SE04 |
| CO02-W27 | Availability publishing, free/busy file, delegation | CO02-W15 |
| CO02-W28 | CSV and vCalendar import, QR, templates, statistics | CO02-W08 |
| CO02-W29 | Model fall-back for quick add | G-15 |
| CO02-W30 | Snapshot restore of events, zone-change review, scripting hooks | `svc.backup` |

## 13. Open questions and risks

1. **Scheduler widgets** (FUSE-WP-34) are the main dependency; nothing exists today beyond the month `calendar` widget.
2. **Shared account record.** One `svc.accounts` entry serving CO02, CO03 and CO04 needs a per-app "enabled collections" field. The schema of `svc.accounts` is not defined yet.
3. **Shared helpers across bundles.** The DAV and iCalendar helpers are needed by three apps. Either each bundle ships the same signed binary, or Calendar offers them as services and becomes a hard dependency of CO03 and CO04. This spec assumes: iCalendar as a Calendar service, DAV as a binary shipped in each bundle from one source tree.
4. **A system timer service** is missing from PLATFORM.md section 8. Without it the reminder daemon must autostart and stay resident.
5. **Time-zone data updates** must reach the machine offline-first through `svc.update`; a stale database gives wrong times. The update path is not specified.
6. **Encrypted properties on CalDAV** break interoperability with other clients and with server scheduling. Kept in Phase 3 and off by default.
7. **Non-Gregorian recurrence (RSCALE)** depends on which calendars `forge::text` has: Unverified.
8. **PRISM on ForgeScript.** The recurrence engine is ForgeScript for sharing with CO04; a T1 proof may need a C++23 version. Decide before Phase 2.
9. **iMIP trust** depends on CO01 passing the authentication result reliably.
10. **Holiday data** needs a maintained offline source with a compatible licence; none chosen.
