<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Notifications and Quick Settings

By Odin Loch

| | |
|---|---|
| ID | `SH02` |
| Bundle | `org.pbsd.notifications` |
| Category | shell |
| Priority | P0 |
| Backing repos | FUSE (UI toolkit), FORGE (ForgeScript, IPC), ParanoidBSD `pbsd/compositor/` (Wayland server side) |
| Status | Planned. Nothing built. |

## 1. Purpose

SH02 is the status side of the desktop: notification banners and the notification centre, the quick settings panel, the privacy indicators, on-screen displays, media controls, the calendar popover and the battery view. Every user reads it many times a day, and it is where the system tells the truth about what is using the camera, microphone, screen, location and network. It replaces Plasma's notification system, system tray applets and OSDs named in the current plan (`docs/plan/PBSD_PLAN.md`, P5-02).

## 2. Scope

In scope:

- The user-facing half of `svc.notify`: banners, centre, history, grouping, actions, inline reply, do-not-disturb, per-app rules.
- Quick settings: network, Bluetooth, sound, brightness, night light, power profile, route (clear, VPN, Tor), capability kill switches, and the smaller toggles.
- Privacy indicators for camera, microphone, screen capture, location, network, USB and clipboard reads.
- On-screen displays for hardware keys and state changes.
- Media controls, calendar popover, battery and power popover.
- The panel applets that open all of the above (hosted in SH01 panel slots).

Out of scope:

| Item | Owner |
|---|---|
| Panel, slots, applet framework, status items of background apps | SH01 |
| Lock-screen surface that shows notifications | SH03 (SH02 supplies redacted rows) |
| Capability grant prompts | SH04 |
| Full settings pages (network, sound, display, power, notifications per app) | SY01 |
| Per-app capability review and revocation history | AD05 |
| Firewall rules, per-app network policy | SE02 |
| VPN and Tor configuration, identities | SE03 |
| Wi-Fi, Bluetooth, audio, backlight and power back ends | system services (SH02-GAP-1..4) |
| Calendar data and editing | CO02 |
| Alarms and timers | UT02 |
| Clipboard history UI | AX05 |
| Screen capture itself | ME05 |

## 3. Users and top tasks

Personas:

- **Daily user**: wants to change volume or Wi-Fi in two clicks and not be interrupted.
- **Privacy-minded user**: wants proof that nothing is listening, and one switch that makes it so.
- **Focused worker**: uses do-not-disturb with schedules and exceptions.
- **Laptop traveller**: watches battery, changes route to VPN or Tor, turns radios off.
- **Screen-reader user**: needs notifications spoken once, in order, with actions reachable.

| # | Task | Target |
|---|---|---|
| T1 | Read and dismiss a notification | glance; `Super+N` then `Delete`, or one click on `✕` |
| T2 | Act on a notification (reply, open) | 1 click on the action, or `Super+N`, `Enter` |
| T3 | Turn on do-not-disturb for one hour | `Super+A`, DND arrow, "1 hour" (3 steps); or `Super+Shift+N` toggles |
| T4 | Join a known Wi-Fi network | `Super+A`, network tile arrow, pick (3 steps) |
| T5 | Change volume or output device | scroll on the sound indicator; or `Super+A`, drag slider (2 steps) |
| T6 | Cut the microphone and camera for every app | `Super+A`, click two kill switches (3 steps); or `Super+Shift+M`, `Super+Shift+C` |
| T7 | See which app is using the microphone | click the indicator (1 step) |
| T8 | Stop an app using a device | click the indicator, "Stop" (2 steps) |
| T9 | Send all traffic through Tor | `Super+A`, route tile, "Tor" (3 steps) |
| T10 | Cut all network at once | `Super+A`, network kill switch (2 steps) |
| T11 | Pause music | media key, or 1 click in the media card |
| T12 | See today's events | click the clock (1 step) |
| T13 | Check battery time and switch to power saver | click battery, pick profile (2 steps) |
| T14 | Find a notification from yesterday | `Super+N`, History, type (3 steps) |
| T15 | Silence one noisy app for good | on its notification: `⋮`, "Turn off for this app" (2 steps) |

## 4. Features

### 4.1 Notification intake and model
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH02-F001 | Service front end | SH02 provides the `svc.notify` interface on the session bus; apps send typed records, never markup or surfaces | M | 1 |
| SH02-F002 | Sender identity | The sending bundle id is taken from the bus peer credentials, not from the record. The banner shows that app's real name, icon and trust badge | M | 1 |
| SH02-F003 | Record fields | Title (120 chars), body (2,000 chars, plain text with `*bold*`, `_italic_` and links only), icon, image, category, urgency, actions, reply field, progress, group key, expiry, sensitivity | M | 1 |
| SH02-F004 | Urgency levels | `low` (centre only), `normal` (banner 6 s), `high` (banner until dismissed), `critical` (system only: battery, security, integrity) | M | 1 |
| SH02-F005 | Categories | `message`, `call`, `email`, `reminder`, `alarm`, `transfer`, `device`, `update`, `security`, `system`, `media`, `error`; used by rules and DND | M | 1 |
| SH02-F006 | Replace and update | A record with the same app-local id replaces the old one in place; no second banner unless `renotify` is set | M | 1 |
| SH02-F007 | Withdraw | The app can withdraw its own notifications; rows leave the centre, history keeps a "withdrawn" mark | M | 1 |
| SH02-F008 | Expiry | `expires_at`; expired rows move to history without a banner | S | 1 |
| SH02-F009 | Rate limit | Per app: 10 banners per minute, 60 records per minute; excess is merged into one "n more from App" row and logged | M | 1 |
| SH02-F010 | Size limits | Record ≤ 64 kB; image handle ≤ 8 MB before decode; over-limit records are refused with an error to the app | M | 1 |
| SH02-F011 | Safe images | Images and third-party icons are decoded by `svc.thumbnail`; SH02 receives pixels only | M | 1 |
| SH02-F012 | Links | Links are shown with their full host; a click goes through `svc.intent` and shows the route (clear, VPN, Tor) in the tooltip | M | 1 |
| SH02-F013 | Sensitivity | `public`, `private` (default), `secret`. Private bodies are hidden on the lock screen; secret records show only "1 notification" anywhere outside an unlocked session, are never stored in history and are never mirrored | M | 1 |
| SH02-F014 | Persistence flag | `resident` records stay in the centre until withdrawn (ongoing call, long transfer) | S | 2 |
| SH02-F015 | Sounds | Category default sound or one of the system sounds; apps cannot supply audio files; honours DND and the mute switch | S | 2 |
| SH02-F016 | Capability check | An app needs `svc.notify`; without it the bus refuses the call. `critical` needs a system-key signature | M | 1 |
| SH02-F017 | Notification when app is closed | Actions on a notification start the app through the launch service with the action id as an activation argument | M | 2 |
| SH02-F018 | Activation token | Clicking a notification hands an `xdg_activation_v1` token to the app so its window may take focus; without a click no app can raise itself | M | 1 |

### 4.2 Banners
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH02-F019 | Banner surface | Layer surface on layer `Overlay`, no exclusive zone, keyboard interactivity `None` until reply is opened | M | 1 |
| SH02-F020 | Position | Top right by default; any corner or top centre; on the output with keyboard focus, or primary | M | 1 |
| SH02-F021 | Stack | Up to 3 banners visible, newest first; more collapse to "+n more" | M | 1 |
| SH02-F022 | Timeout | 6 s normal; paused while the pointer is over it; `high` stays; setting 3–30 s | M | 1 |
| SH02-F023 | Dismiss | `✕`, swipe sideways, middle click, or `Super+Shift+Delete` for all; dismissing moves to history | M | 1 |
| SH02-F024 | Default action | Click on the body runs the record's default action and closes the banner | M | 1 |
| SH02-F025 | Action buttons | Up to 3 text actions; a destructive action is outlined in `--danger` and never first | M | 1 |
| SH02-F026 | Inline reply | A reply field with send button; `Enter` sends, `Shift+Enter` newline; the text returns to the app as an action result | S | 2 |
| SH02-F027 | Progress banner | Determinate or indeterminate bar, speed and remaining text, Pause and Cancel if the app declares them | M | 1 |
| SH02-F028 | Call banner | `call` category: stays until answered or declined, large Answer and Decline, ring sound; breaks through DND only if the caller is on the exceptions list | S | 2 |
| SH02-F029 | Expand | Chevron or `Down` expands long bodies and shows the image at up to 320×180 | S | 1 |
| SH02-F030 | Per-banner menu `⋮` | Snooze (15 min, 1 h, tomorrow), Turn off for this app, Make silent, Notification settings, Copy text | M | 1 |
| SH02-F031 | Snooze | The record leaves view and returns as a banner at the chosen time | S | 2 |
| SH02-F032 | Fullscreen rule | No banners over a fullscreen window unless urgency is `high` or `critical`; setting can allow all | M | 1 |
| SH02-F033 | Screen sharing rule | While any output or window is being captured, banners are suppressed and bodies hidden on the shared output; an indicator row says "n hidden while sharing" | M | 1 |
| SH02-F034 | Presentation outputs | Never draw banners on an output marked "presentation" (SY01) | S | 2 |
| SH02-F035 | Keyboard focus | `Super+N` focuses the newest banner; arrows move; `Enter` default action; `Tab` reaches actions; `Esc` returns focus | M | 1 |
| SH02-F036 | No focus theft | A banner never takes keyboard focus by itself | M | 1 |
| SH02-F037 | Critical banner | `critical` records use a `--danger` or `--warn` side bar, an icon and a word, and stay until acted on | M | 1 |
| SH02-F038 | Copy guard | "Copy text" on a `secret` record is absent | S | 2 |

### 4.3 Notification centre and history
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH02-F039 | Open | Click the bell applet, `Super+N` twice, or swipe in from the right edge; 400 px wide panel on the right, full height minus panels | M | 1 |
| SH02-F040 | Current list | Unread and resident rows, newest first, grouped by app, then by group key (conversation, transfer) | M | 1 |
| SH02-F041 | Group rows | A group shows the latest two rows and "n more"; expand in place; "Clear group" | M | 1 |
| SH02-F042 | Row actions | Same actions and reply as the banner; `Delete` dismisses; `Shift+Delete` removes from history too | M | 1 |
| SH02-F043 | Clear all | Button and `Ctrl+Shift+Delete`; a 5 s Undo toast | M | 1 |
| SH02-F044 | History tab | Every record of the last 7 days (setting 0–90), with dismissed, expired and withdrawn marks | M | 1 |
| SH02-F045 | History search | Field filters by text, app (`app:mail`), category (`cat:security`), date (`on:2026-11-11`) | S | 2 |
| SH02-F046 | History store | SQLite in `fs.app`; bodies of `private` records are stored; `secret` never; the store sits on the user's encrypted dataset | M | 1 |
| SH02-F047 | History off | Setting turns history off and deletes the store; guest and disposable sessions never keep one | M | 1 |
| SH02-F048 | Filters | Chips: All, Unread, Security, Messages, Transfers, System | S | 2 |
| SH02-F049 | Jobs section | Running progress records (downloads, backups, exports) pinned at the top with one bar each | M | 1 |
| SH02-F050 | Security section | Records of category `security` (from SE01, SE02, `svc.audit`) are pinned above ordinary rows and cannot be turned off, only snoozed | M | 1 |
| SH02-F051 | Unread count | The bell shows a count of unread rows; `low` rows do not count | M | 1 |
| SH02-F052 | Mark read | Opening the centre marks visible rows read after 1 s | S | 1 |
| SH02-F053 | Export | Save history as JSON Lines or CSV through the file portal | C | 3 |
| SH02-F054 | Search provider | History rows are offered to `svc.search` (provider `notifications`), off by default | C | 3 |
| SH02-F055 | Digest | Optional: `low` records are held and shown as one digest row at set times (e.g. 12:00, 17:00) | C | 3 |
| SH02-F056 | Local summary | Optional: a long backlog after DND is summarised per app by `svc.model`; needs `ai.local`, off by default; the original rows stay below | C | 3 |

### 4.4 Do-not-disturb and rules
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH02-F057 | DND toggle | Quick settings tile, bell context menu, `Super+Shift+N`; the bell shows a crossed icon and the word in its tooltip | M | 1 |
| SH02-F058 | Timed DND | 30 min, 1 h, 2 h, until tomorrow 07:00, until the next calendar event ends, until turned off | M | 1 |
| SH02-F059 | Schedules | Any number of schedules: days, start, end; e.g. weekdays 22:00–07:00 | S | 2 |
| SH02-F060 | Automatic DND | Optional triggers: fullscreen app, screen sharing (on by default), presentation output connected, a calendar event marked busy | S | 2 |
| SH02-F061 | Exceptions | Apps, categories (alarms and calls on by default), and contacts (from CO03 by id) that break through | S | 2 |
| SH02-F062 | Critical always shows | `critical` system records ignore DND | M | 1 |
| SH02-F063 | What DND does | No banners, no sounds; records go to the centre; count still updates | M | 1 |
| SH02-F064 | Summary on exit | When DND ends, one banner: "12 notifications while you were away" opening the centre | S | 2 |
| SH02-F065 | Per-app rules | For each app: allow, silent (centre only), off; banner yes or no; sound yes or no; lock-screen detail (show, hide body, hide all); per category overrides | M | 1 |
| SH02-F066 | Rule from the banner | "Turn off for this app" and "Make silent" write the rule and show Undo | M | 1 |
| SH02-F067 | Keyword rules | Mute or highlight records whose title or body matches a word list or a simple pattern (no regular expressions beyond `*`) | C | 3 |
| SH02-F068 | Focus modes | Named sets of DND rules (Work, Sleep, Presentation) selectable from the DND arrow; each can also set the power profile and night light | C | 3 |
| SH02-F069 | Policy | An administrator policy can force rules (e.g. security records always on) and mark them read-only | S | 3 |

### 4.5 Quick settings panel
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH02-F070 | Open | Click the status area, `Super+A`, or swipe down from the status area; 400×auto panel anchored to the status area | M | 1 |
| SH02-F071 | Tile grid | Two-column grid of tiles; each tile is a toggle with a label, a state line and an optional arrow that opens a detail page inside the panel | M | 1 |
| SH02-F072 | Sliders | Volume and brightness sliders always visible under the tiles | M | 1 |
| SH02-F073 | Edit tiles | Pencil button: reorder by drag, add and remove from the full tile list; Reset | S | 2 |
| SH02-F074 | Detail pages | Each arrow slides to a page with a back button and a "Settings…" link to the SY01 page | M | 1 |
| SH02-F075 | Footer | User name, battery, Settings button, Lock, Power (hands to SH03) | M | 1 |
| SH02-F076 | Keyboard | Arrows move between tiles, `Space` toggles, `Enter` opens the detail page, `Backspace` goes back, `Esc` closes | M | 1 |
| SH02-F077 | Third-party tiles | Not allowed in phase 1–2. Phase 3: data-only tiles (label, state, toggle action) from apps holding `svc.notify`; drawn by SH02 | C | 3 |
| SH02-F078 | Lock-screen subset | A reduced panel on the lock screen (SH03): volume, brightness, DND, kill switches on (never off), power; no network changes | S | 2 |

### 4.6 Network, Bluetooth and route
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH02-F079 | Network tile | Shows the active link (wired, Wi-Fi name and bars, tethering, none); click toggles Wi-Fi | M | 1 |
| SH02-F080 | Wi-Fi list | Detail page: known and visible networks, signal, security word (WPA3, WPA2, open), connect, forget, hidden network | M | 1 |
| SH02-F081 | Wi-Fi password | Asked in an SH04 trusted prompt, stored by `svc.keyring`; SH02 never sees the passphrase | M | 1 |
| SH02-F082 | Open network warning | Joining an open network shows "Unencrypted. Use VPN or Tor?" with the two buttons | S | 2 |
| SH02-F083 | Wired and tethering rows | Link speed, address summary; USB and Bluetooth tethering when present | S | 2 |
| SH02-F084 | MAC randomisation mark | Each Wi-Fi row shows whether a random address is used; default random per network | S | 2 |
| SH02-F085 | Captive portal | Detected portals raise a notification that opens the portal page in a disposable browser window (IN01) | S | 2 |
| SH02-F086 | Airplane mode | One tile turns Wi-Fi, Bluetooth and mobile radios off and remembers what was on | M | 1 |
| SH02-F087 | Bluetooth tile | On, off; detail page lists paired devices with battery level, connect, disconnect; "Pair new…" opens VM05 | M | 2 |
| SH02-F088 | Route tile | Shows the system default route: Clear, VPN (name), Tor. Detail page picks one; SE03 does the work and reports state | M | 2 |
| SH02-F089 | Route states | Connecting, connected, failed, "blocked: kill switch" each with an icon and word; a failed protected route never falls back to clear without the user choosing it | M | 2 |
| SH02-F090 | Per-app route peek | Detail page lists apps whose route differs from the default (from SE03), read-only, with a link to SE03 | S | 3 |
| SH02-F091 | Network badge | Status-area `[net]` badge in `--net`: count of apps with open sockets; click lists app, endpoint count, route; link to SE02 live connections | M | 1 |
| SH02-F092 | Hotspot tile | Start a Wi-Fi hotspot with a random passphrase shown as text and QR code | C | 3 |
| SH02-F093 | Data use row | Per-link bytes today, from SE02 counters | C | 3 |
| SH02-F094 | Metered mark | Mark a network metered: updates (SY05) and backups (AD03) wait | S | 3 |

### 4.7 Sound and media
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH02-F095 | Volume slider | Master output volume 0–100% (to 150% if `sound.allow_boost`); mute button | M | 1 |
| SH02-F096 | Output and input device | Detail page: choose output and input device, per-device volume, input level meter | M | 1 |
| SH02-F097 | Per-app mixer | One slider and mute per playing or recording app; shows the app's real name from `svc.media` | S | 2 |
| SH02-F098 | Scroll and middle click | Scroll on the sound indicator changes volume by 5% (2% with `Shift`); middle click mutes | M | 1 |
| SH02-F099 | Microphone mute key | `XF86AudioMicMute` and `Super+Shift+M` mute the default input; this is a soft mute, the kill switch (F121) is separate and stronger | M | 1 |
| SH02-F100 | Media card | For each `svc.media` session: art, title, artist, app, previous, play or pause, next, seek bar, shuffle and repeat when declared | M | 1 |
| SH02-F101 | Media keys | Play, pause, next, previous, stop keys act on the most recent session; work on the lock screen without unlocking | M | 1 |
| SH02-F102 | Multiple sessions | Cards stack; a pin sets which session the keys control | S | 2 |
| SH02-F103 | Output switch on connect | When headphones or a Bluetooth sink appear, a banner asks "Switch to X?" once per device, then remembers | S | 2 |
| SH02-F104 | Art safety | Album art is a handle decoded by `svc.thumbnail`; remote URLs are never fetched by SH02 | M | 1 |
| SH02-F105 | Volume limit warning | Headphone volume above 85% shows a one-time hint; no nag | C | 3 |

### 4.8 Display, power and battery
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH02-F106 | Brightness slider | Per internal panel; external displays through DDC/CI when the display service offers it; one slider per display in the detail page | M | 1 |
| SH02-F107 | Night light tile | On, off, or schedule (sunset to sunrise from a manually set place, or fixed times); temperature slider 1,900–6,500 K in the detail page | S | 2 |
| SH02-F108 | No location needed | Sunset times use the region set in SY01, not `dev.location` | M | 2 |
| SH02-F109 | Dark mode tile | Switches the system theme light or dark; optional schedule | S | 2 |
| SH02-F110 | Power profile tile | Power saver, balanced, performance, through `svc.power`; shows which apps hold sleep inhibitors | M | 1 |
| SH02-F111 | Keep awake tile | Holds a sleep and idle inhibitor for 30 min, 1 h, 2 h or until off; a mark in the status area while held | S | 2 |
| SH02-F112 | Battery indicator | Icon with level and charging state, optional percent text; tooltip gives time left or time to full | M | 1 |
| SH02-F113 | Battery popover | Level, state, time estimate, rate in W, health (design and full capacity), cycle count, power profile selector, top 5 energy users (from SY04) | M | 2 |
| SH02-F114 | Peripheral batteries | Mouse, keyboard, headset, UPS levels when reported | S | 2 |
| SH02-F115 | Low battery | `critical` record at 15% and 7%; at 3% the system suspends or hibernates (policy in SY01); thresholds configurable | M | 1 |
| SH02-F116 | Charge limit | Toggle "limit to 80%" when the hardware supports it | C | 3 |
| SH02-F117 | Rotation lock and display mode | Rotation lock tile on convertible devices; `Super+P` OSD cycles internal only, mirror, extend, external only (applies through SY01's display service) | S | 2 |
| SH02-F118 | Keyboard backlight | Slider or steps when present | C | 3 |

### 4.9 Capability kill switches and privacy indicators
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH02-F119 | Kill switch row | A row of switches in quick settings: Camera, Microphone, Network, Location, Screen capture, USB (new devices), Bluetooth | M | 1 |
| SH02-F120 | Camera kill switch | Off: the camera portal revokes every outstanding camera handle at once (SI-6) and refuses new ones; apps see "device unavailable" | M | 1 |
| SH02-F121 | Microphone kill switch | Same for microphone handles; stronger than mute: no app receives samples, not even silence-filled streams | M | 1 |
| SH02-F122 | Network kill switch | The firewall service (SE02) drops all traffic except loopback and the session bus; the tile turns `--danger` with the word "Network cut" | M | 1 |
| SH02-F123 | Location, screen, USB, Bluetooth | Location and screen-capture portals revoke and refuse; USB switch refuses newly attached devices (VM05 policy); Bluetooth radio off | S | 2 |
| SH02-F124 | Hardware switch state | Where the machine has a hardware switch or firmware-level disable, the tile shows "hardware: off" and cannot be turned on in software | S | 3 |
| SH02-F125 | Turning back on | Turning a switch on needs no prompt in an unlocked session, but never happens from the lock screen, from a notification action or from another app | M | 1 |
| SH02-F126 | Switch persistence | State survives restart; default on first boot is camera, mic and location off until first used and granted | M | 1 |
| SH02-F127 | Panic shortcut | `Super+Shift+K` (hold 1 s) turns camera, mic, location, screen capture and network off together; one banner offers Undo for each | S | 2 |
| SH02-F128 | Indicators | Status-area marks while in use: camera (`--danger` dot + icon), microphone, screen capture, location, clipboard read by a background app; each with a word in the tooltip and a11y name | M | 1 |
| SH02-F129 | Indicator source | "In use" events come from the device portals and the compositor, never from the app; an app cannot hide or fake one | M | 1 |
| SH02-F130 | Indicator popover | Click: list of apps using each device now and in the last 15 min, with duration, a Stop button (revokes the handle) and "Permissions…" (AD05) | M | 1 |
| SH02-F131 | Minimum display time | An indicator stays at least 3 s after use ends, then a hollow "recently used" mark for 30 s, so a quick grab cannot go unseen | M | 1 |
| SH02-F132 | Fullscreen and hidden panel | When no panel is visible on an output, the compositor draws a small indicator dot in the top right corner itself (SH02-GAP-6) | M | 2 |
| SH02-F133 | First-use banner | The first time an app uses a device in a session, a banner says "App is using the microphone" with Stop | S | 1 |
| SH02-F134 | Screen capture frame | While an output or window is captured, SH05 draws a `--danger` outline around it; the indicator popover names the capturing app and the target | M | 2 |
| SH02-F135 | Audit link | Every indicator event and switch change is written to `svc.audit`; the popover links to AD04 filtered to that device | M | 1 |
| SH02-F136 | Keyboard and input capture mark | A mark when an app holds a keyboard-shortcuts inhibitor or pointer lock (`…keyboard_shortcuts_inhibit`, `…pointer_constraints` modules) with the key to release it | S | 2 |
| SH02-F137 | Remote session mark | A persistent mark while VM04 serves a remote session, with Disconnect | M | 2 |

### 4.10 On-screen displays
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH02-F138 | OSD surface | 240×64 pill, bottom centre 96 px above the bottom edge, layer `Overlay`, no input region (clicks pass through), 1.5 s then fades in 140 ms | M | 1 |
| SH02-F139 | Volume OSD | Icon, bar, percent, device name when it changes | M | 1 |
| SH02-F140 | Brightness and keyboard backlight OSD | Icon, bar, percent | M | 1 |
| SH02-F141 | Mute and mic mute OSD | Icon with a slash and the words "Muted" / "Microphone muted" | M | 1 |
| SH02-F142 | Lock keys OSD | Caps Lock, Num Lock on and off (setting, on by default for Caps) | S | 1 |
| SH02-F143 | Keyboard layout OSD | Layout name on switch (data from AX04) | S | 2 |
| SH02-F144 | Media OSD | Track title and state on media-key use when no window of the player is visible | C | 2 |
| SH02-F145 | Kill switch OSD | "Camera off", "Network cut" with the switch icon in `--danger` | M | 1 |
| SH02-F146 | Display mode OSD | `Super+P` chooser: four options as tiles, arrows to move, releases to apply, reverts after 15 s unless confirmed | S | 2 |
| SH02-F147 | Workspace OSD | Workspace name on switch; drawn on request from SH05 | C | 2 |
| SH02-F148 | Airplane, touchpad, power profile OSD | State word and icon on hardware key | S | 2 |
| SH02-F149 | One OSD at a time | A new OSD replaces the current one in place without animation restart | M | 1 |
| SH02-F150 | Not captured | OSDs and banners are marked so screen capture and shared outputs leave them out (setting) | S | 3 |

### 4.11 Calendar popover and clock
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH02-F151 | Clock applet | Time, optional date, seconds, week number, second time zone; 12 or 24 h from the locale | M | 1 |
| SH02-F152 | Calendar popover | Month grid with week numbers, today marked, dots on days with events; arrows and `PgUp`/`PgDn` change month | M | 1 |
| SH02-F153 | Agenda | Events of the selected day from CO02's read-only agenda interface: time, title, calendar colour and name; click opens CO02 | S | 2 |
| SH02-F154 | Locked calendars | Events of encrypted calendars show as "Busy" unless CO02 says they are unlocked | S | 2 |
| SH02-F155 | New event | `[+]` emits the `compose` intent to CO02 with the selected date | S | 2 |
| SH02-F156 | World clocks, alarms, timers | Up to 4 world clocks and the next alarm and running timers from UT02; click opens UT02 | C | 3 |
| SH02-F157 | Time source mark | A `--warn` mark when the clock is unsynchronised or was stepped by more than 5 s (from the time service) | S | 3 |
| SH02-F158 | Holidays | Offline holiday tables per region | C | 3 |

### 4.12 Settings, security, accessibility, recovery
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH02-F159 | Settings page | A "Notifications" page and parts of Sound, Network and Power pages in SY01 use SH02's schema (section 8) | M | 1 |
| SH02-F160 | Lock-screen feed | SH02 gives SH03 a redacted list: app name and count, or title and body, per rule F065 | M | 1 |
| SH02-F161 | Screen reader | Banners are announced once as polite live regions (`critical` assertive): app, title, body, "n actions available" | M | 1 |
| SH02-F162 | Visual alerts | Optional screen-edge flash for sounds, for deaf users | S | 2 |
| SH02-F163 | Longer timeouts | Banner timeout multiplier 1×, 2×, 5×, or "until dismissed" in accessibility settings | M | 1 |
| SH02-F164 | Reduced motion | Banners and OSDs appear and disappear without slide | M | 1 |
| SH02-F165 | Crash recovery | The service restarts within 1 s; current rows reload from the store; apps reconnect by bus name; in-flight banners are shown again only if unread | M | 1 |
| SH02-F166 | Fallback path | If the SH02 UI is down, `critical` records are drawn by SH03's trusted-path surface as plain text | S | 3 |
| SH02-F167 | Test notification | Button in settings sends one sample of each urgency | S | 1 |
| SH02-F168 | Audit | Refused records, rate-limit hits and rule changes are written to `svc.audit` | M | 1 |
| SH02-F169 | Help | `F1` in any popover opens the matching UT05 page | S | 2 |
| SH02-F170 | Remote mirroring | Off and not built in phases 1–3; if added later it needs an explicit per-device pairing in SY01 and never carries `secret` records | C | 3 |

## 5. GUI

SH02 has no ordinary window. Its surfaces are layer surfaces (`zwlr_layer_shell_v1`, `PBSD:pbsd/compositor/pbsd.compositor.wayland.layer.cppm`) and popups of them, plus applets hosted in SH01 panel slots.

### 5.1 Window map
```
Notifications and Quick Settings
├─ Panel applets (in SH01 slots)
│  ├─ Privacy indicators        (camera, mic, screen, location, clipboard, input capture, remote)
│  ├─ Network badge
│  ├─ Sound indicator
│  ├─ Battery indicator
│  ├─ Status cluster button     (opens quick settings)
│  ├─ Bell with count           (opens the centre)
│  └─ Clock                     (opens the calendar popover)
├─ Banner stack                 layer Overlay, per focused output
│  └─ Banner: body, actions, reply field, ⋮ menu
├─ Notification centre          layer Top, right edge, 400 px
│  ├─ Current tab (Jobs, Security, groups)
│  └─ History tab (search, filters)
├─ Quick settings panel         layer Top popup of the status cluster
│  ├─ Main page (tiles, kill switches, sliders, media card, footer)
│  ├─ Detail pages: Wi-Fi, Bluetooth, Route, Sound, Mixer, Display, Night light,
│  │                Power, Do-not-disturb, Keep awake
│  └─ Tile edit mode
├─ Indicator popover            (who is using what; Stop)
├─ Network badge popover
├─ Battery popover
├─ Calendar popover
├─ OSD                          layer Overlay, input-transparent
│  └─ Display mode chooser (Super+P)
└─ Compositor-drawn indicator dot (not an SH02 surface; SH02-GAP-6)
Settings: pages in SY01. No window.
```

### 5.2 Main window
The main view is the quick settings panel, drawn here on the whole screen (1440×900) with the status area it belongs to.

```
+----------------------------------------------------------------------------------------+
|                                                                                        |
|                                              +---------------------------------------+ |
|                                              | Quick settings              [✎] [⚙] (1)| |
|                                              | +-----------------+ +---------------+ | |
|                                              | |[≋] Wi-Fi       >| |[ᛒ] Bluetooth >| | |
|                                              | | home-5G · WPA3  | | Off           | | |
|                                              | +-----------------+ +---------------+ | |
|                                              | |[⇄] Route       >| |[☾] Do not    >| | |
|                                              | | Tor · connected | | disturb · Off | | |
|                                              | +-----------------+ +---------------+ | |
|                                              | |[◐] Night light >| |[⚡] Balanced  >| | |
|                                              | | On until 06:40  | | Power profile | | |
|                                              | +-----------------+ +---------------+ (2)|
|                                              | Kill switches                      (3)| |
|                                              | [◉ Camera  OFF] [🎙 Mic     OFF]      | |
|                                              | [⇅ Network  ON] [⌖ Location OFF]      | |
|                                              | [▣ Screen   ON] [⎘ USB new   ON]      | |
|                                              | 🔈 [=========|------------] 62% [>](4)| |
|                                              | ☼  [===============|------] 80% [>](5)| |
|                                              | +-----------------------------------+ | |
|                                              | | [art] Track title — Artist        | | |
|                                              | |       Music   [⏮] [⏯] [⏭]   1:12 (6)| |
|                                              | +-----------------------------------+ | |
|                                              | odin · 🔋 78% 3 h 10 m  [⚙][🔒][⏻] (7)| |
|                                              +---------------------------------------+ |
|[◉]| … task list …                         |[◉ mic](8)[net 3](9)[🔈][🔋](10)[🔔 3](11)| 14:32 (12)|
+----------------------------------------------------------------------------------------+
```

1. Header: title, edit tiles `[✎]`, open SY01 `[⚙]`. `toolButton`s.
2. Tile grid: `gridView` of tile components. Each tile is a `toggleSwitch`-behaving `panel` with an icon, label, state line and an arrow `toolButton` that opens the detail page. On tiles use `--accent`; the Route tile uses `--net`.
3. Kill switch row: six `toggleSwitch` controls in a 2-column grid. OFF means the capability is cut for every app. A cut switch is `--danger` with the word OFF; state is never colour alone.
4. Volume: `slider` with mute `toolButton` on the icon and an arrow to the Sound page.
5. Brightness: `slider`, arrow to the Display page.
6. Media card: `image`, two `label`s, three `toolButton`s, thin `progressBar` as seek bar (a `slider` when the session allows seeking).
7. Footer: `label`s and three `toolButton`s; Lock and Power call SH03.
8. Privacy indicator applet: icon and optional word while a device is in use. Click opens the indicator popover (5.3.4).
9. Network badge: `--net`, count of apps with open sockets.
10. Sound and battery indicators. Clicking any of the cluster opens this panel.
11. Bell with unread count. Opens the centre.
12. Clock. Opens the calendar popover.

Panel size: 400 px wide; height fits content up to the output height minus 24 px, then the tile grid scrolls in a `scrollArea`.

States: normal; a back end missing (tile greyed with "Unavailable" and a tooltip naming the missing service); airplane mode (radio tiles off, state line "Airplane mode"); network cut (Wi-Fi, Bluetooth and Route tiles show "Blocked by kill switch"); lock-screen subset (F078); policy-locked tile (padlock and "Managed"); narrow output (< 640 px: the panel is a full-width bottom sheet).

### 5.3 Other views and dialogs
#### 5.3.1 Notification centre (400×(output height − panel), right edge; shown on 1440×900)
```
+----------------------------------------------------------------------------------------+
|                                               | Notifications        [☾ DND v] [Clear all] (1)|
|                                               | [Current] [History]   [🔍           ] (2)|
|                                               |---------------------------------------|
|                                               | Jobs                              (3) |
|                                               |  Backup to nas   [=======|---] 71%    |
|                                               |  4 min left             [Pause][✕]    |
|                                               | Security                          (4) |
|                                               |  ⚠ Firewall blocked 14 connections    |
|                                               |    from Demo  · 14:20   [Review]      |
|                                               | Messenger ✓                    3  (5) |
|                                               |  Ada: are we still on for 3?   14:31  |
|                                               |  [Reply…                    ] [Send]  |
|                                               |  Ada: sent a file              14:25  |
|                                               |  1 more                        [v]    |
|                                               | Mail ✓                         2      |
|                                               |  Invoice 1182 — from accounts  13:58  |
|                                               |  [Open] [Archive] [⋮]                 |
|                                               | Software ✓                            |
|                                               |  3 updates ready · signed      12:10  |
|                                               |  [Review and install]                 |
|                                               |                                       |
|                                               | 2 hidden while sharing your screen (6)|
+----------------------------------------------------------------------------------------+
```

1. Header: DND `comboBox` (Off, 30 min, 1 h, 2 h, Until tomorrow, Until this event ends, Schedules…), `button` Clear all.
2. `tabBar` Current and History; `lineEdit` filter (History only).
3. Jobs: `progressBar` rows from progress records (F049).
4. Security section, pinned (F050), `--warn` or `--danger` icon and word.
5. App groups: `treeView` with two levels (group, record). The header shows the real app name, its trust badge `✓` and the count. Rows carry `button` actions and an optional `lineEdit` reply.
6. Notice row, `label`.

States: empty ("No notifications" and, in DND, "Do not disturb is on until 15:30"), loading (store opening, at most 100 ms), history off ("History is turned off. [Turn on]"), store error ("History could not be read. New notifications still arrive. [Details]"), locked session (the centre is not reachable; SH03 shows the redacted feed).

#### 5.3.2 Banners (360×auto each, top right, 12 px from the edges)
```
+----------------------------------------------------------------------------------------+
|                                                    +--------------------------------+  |
|                                                    | [M] Messenger ✓      14:31 [⋮][✕]| (1)
|                                                    | Ada                             |  |
|                                                    | are we still on for 3?          |  |
|                                                    | [Reply] [Mark read]             | (2)
|                                                    +--------------------------------+  |
|                                                    +--------------------------------+  |
|                                                    |▌[!] Security Centre ✓  14:30 [✕]| (3)
|                                                    |▌Boot measurement changed        |  |
|                                                    |▌The kernel hash differs from    |  |
|                                                    |▌the last boot.   [Review]       |  |
|                                                    +--------------------------------+  |
|                                                    +--------------------------------+  |
|                                                    | [T] Transfers ✓        [⋮][✕]   |  |
|                                                    | pbsd-15.iso  [=====|----] 54%   | (4)
|                                                    | 12 MB/s · 2 min   [Pause][Cancel]|  |
|                                                    +--------------------------------+  |
|                                                              + 2 more             (5)  |
+----------------------------------------------------------------------------------------+
```

1. Header line: app icon, real app name, trust badge, time, menu, close. Drawn by SH02 from the bus identity (F002).
2. Up to 3 actions; "Reply" turns into a `lineEdit` in place.
3. `critical` banner with a 4 px `--danger` bar and icon; stays until acted on.
4. Progress banner with `progressBar`.
5. Overflow count; click opens the centre.

Widgets: each banner is a `panel` with `label`, `image`, `button`, `toolButton`, `lineEdit`, `progressBar`. States: collapsed, expanded, replying, hidden body (sharing or lock screen: "Messenger · 1 new message"), snoozed (gone).

#### 5.3.3 Wi-Fi detail page (inside the quick settings panel, 400×520)
```
+---------------------------------------+
| [<] Wi-Fi                     [ON ●] |
|---------------------------------------|
| Connected                             |
| > [≋▮▮▮▮] home-5G        WPA3   ✓    |
|     random address · 866 Mb/s  [ⓘ]   |
| Known                                 |
|   [≋▮▮▮ ] office          WPA2-Ent    |
| Available                             |
|   [≋▮▮▮ ] cafe-guest      Open  ⚠    |
|   [≋▮▮  ] neighbour-2G    WPA2        |
|   [≋▮   ] printer-setup   Open  ⚠    |
|                                       |
| [Join hidden network…]   [Rescan F5]  |
|---------------------------------------|
| Route: Tor · connected       [change] |
| [Network settings…]                   |
+---------------------------------------+
```

`modelListView` with section headers; `toggleSwitch` in the header; `[ⓘ]` expands address, DNS, gateway and "Forget". Open networks carry `⚠` and the word "Open". States: Wi-Fi off, scanning (spinner), no adapter ("No Wi-Fi adapter found. [Devices]"), network cut by kill switch (list disabled with the reason), connecting, wrong password (inline error under the row; retry opens the SH04 prompt).

The Bluetooth, Route, Sound, Display, Night light, Power, DND and Keep awake pages follow the same pattern: header with back and master switch, a list or controls, footer link to SY01 or the owning app.

#### 5.3.4 Privacy indicator popover (340×auto, under the indicator)
```
+--------------------------------------------+
| In use now                                 |
| [🎙] Microphone                             |
|    Messenger ✓    call · 12 min    [Stop]  |
| [▣] Screen                                 |
|    Screen Capture ✓  DP-1 · 2 min  [Stop]  |
|--------------------------------------------|
| Recently (last 15 min)                     |
| [◉] Camera      Camera ✓      14:18, 40 s  |
| [⎘] Clipboard   Demo ⚠        14:11, read  |
|--------------------------------------------|
| Kill switches   [🎙 ON] [◉ OFF] [▣ ON]     |
| [Permissions…]            [Device log…]    |
+--------------------------------------------+
```

`modelListView` rows; `button` Stop revokes the handle through the owning portal; `toggleSwitch` copies of the kill switches; links open AD05 and AD04. States: nothing in use ("No app is using the camera, microphone, screen or location"), portal service down ("Device state unknown" in `--warn`; kill switches still shown).

#### 5.3.5 Calendar popover (340×460) and battery popover (320×auto)
```
+-------------------------------------+   +------------------------------------+
| Thursday 12 November 2026           |   | Battery                 78%  [🔋]  |
| [<]  November 2026  [>]   [Today]   |   | Discharging · 3 h 10 m left        |
| Wk  Mo Tu We Th Fr Sa Su            |   | 11.4 W · health 93% · 212 cycles   |
| 44                       1          |   |------------------------------------|
| 45   2  3  4  5  6  7  8            |   | Power profile                      |
| 46   9 10 11[12]13 14 15            |   | ( ) Saver (•) Balanced ( ) Perform.|
| 47  16 17·18 19 20·21 22            |   |------------------------------------|
| 48  23 24 25 26 27 28 29            |   | Using the most energy              |
| 49  30                              |   |  Browser            4.1 W          |
|-------------------------------------|   |  Video              2.3 W          |
| Today                          [+]  |   | Keeping the system awake           |
| 15:00 Call with Ada      ■ Work     |   |  Video (playing)                   |
| 18:30 Busy               ■ Private  |   |------------------------------------|
|-------------------------------------|   | Mouse 60%   Headset 35%            |
| Sydney 14:32 · UTC 03:32 · ⏰ 07:00  |   | [ ] Limit charge to 80%            |
+-------------------------------------+   | [Power settings…]                  |
                                          +------------------------------------+
```

Calendar: `calendar` widget, `modelListView` agenda, `label` row for world clocks and next alarm. Battery: `label`s, `radioButton` group, two short `listView`s, `checkBox`. States: no calendars ("No calendars. [Open Calendar]"), CO02 not installed (month grid only), desktop machine with no battery (popover shows power profile and UPS only).

#### 5.3.6 OSDs and the display mode chooser
```
        volume                      kill switch                  Super+P chooser
+------------------------+   +------------------------+   +-----------------------------------+
| 🔈 [========|---] 62%  |   | ◉  Camera off          |   | [Internal] [Mirror] [>Extend] [External]|
+------------------------+   +------------------------+   | Keep this? Reverting in 12 s  [Keep]   |
                                                          +-----------------------------------+
```

OSD: `panel` with `image`, `progressBar`, `label`; empty input region. The chooser is the only OSD that takes keyboard input (it holds exclusive keyboard while `Super` is down).

#### 5.3.7 Sound detail page and tile edit mode
```
+---------------------------------------+   +---------------------------------------+
| [<] Sound                             |   | Edit tiles               [Reset][Done]|
|---------------------------------------|   | In the panel (drag to reorder)        |
| Output  [Headphones (USB)         v]  |   | [⠿ Wi-Fi ✕] [⠿ Bluetooth ✕]           |
| 🔈 [=========|-----------] 62%         |   | [⠿ Route ✕] [⠿ Do not disturb ✕]      |
| Input   [Internal microphone      v]  |   | [⠿ Night light ✕] [⠿ Power ✕]         |
| 🎙 [======|--------------] 40% ▮▮▮▯▯   |   | More tiles (drag up to add)           |
|---------------------------------------|   | [Airplane] [Dark mode] [Keep awake]   |
| Apps                                  |   | [Rotation] [Hotspot] [Screen capture] |
|  Music ✓      [=======|---] 70% [🔈]  |   | [Keyboard layout] [Magnifier]         |
|  Browser ✓    [====|------] 45% [🔇]  |   | [Screen reader] [High contrast]       |
|  Messenger ✓  🎙 recording     [Stop] |   +---------------------------------------+
| [Sound settings…]                     |
+---------------------------------------+
```

`comboBox` device pickers, `slider`s, a level meter drawn with `progressBar`, per-app rows in a `modelListView`. Edit mode uses two `gridView`s with drag and drop and keyboard reordering (`Ctrl+Arrow`).

### 5.4 Menus
No menu bar. Context menus (`popupMenu`):

| Menu | Items (shortcut) |
|---|---|
| Bell applet | Open notifications (`Super+N`) · Do not disturb ▸ (Off, 30 min, 1 h, 2 h, Until tomorrow, Schedules…) (`Super+Shift+N`) · Clear all (`Ctrl+Shift+Delete`) · Notification settings |
| Banner / row `⋮` | Snooze ▸ (15 min, 1 h, Tomorrow 09:00) · Turn off for this app · Make silent · Copy text (`Ctrl+C`) · Notification settings for App · Show permissions |
| App group header | Clear group (`Delete`) · Mute for 1 hour · Settings for App |
| Sound indicator | Mute (`middle click`) · Output ▸ devices · Input ▸ devices · Mute microphone (`Super+Shift+M`) · Sound settings |
| Network badge / tile | Wi-Fi on/off · Airplane mode · Cut network (kill switch) · Open Firewall · Network settings |
| Route tile | Clear · VPN ▸ profiles · Tor · Open Anonymity |
| Battery indicator | Show percentage · Power profile ▸ · Keep awake ▸ · Power settings |
| Clock | Copy date and time · Show seconds · Show week numbers · Date and time settings · Open Calendar · Open Clocks |
| Privacy indicator | Stop all · Kill switch for this device · Permissions · Device log |
| Kill switch | Turn off / on · What does this do? (`F1`) · Device log |
| Media card | Pin as key target · Open app · Hide this player |

### 5.5 Keyboard and pointer
| Keys | Action |
|---|---|
| `Super+N` | Focus newest banner; again opens the centre |
| `Super+A` | Quick settings |
| `Super+Shift+N` | Toggle do-not-disturb |
| `Super+Shift+Delete` | Dismiss all banners |
| `Super+Shift+M` / `Super+Shift+C` | Microphone / camera kill switch |
| `Super+Shift+K` (hold 1 s) | Panic: cut camera, mic, location, screen capture, network |
| `Super+P` | Display mode chooser |
| `Super+C` | Calendar popover |
| Volume, brightness, media, mic-mute, airplane keys | Their actions with an OSD |
| In the centre: `Up`/`Down`, `Left`/`Right` | Move rows; collapse or expand a group |
| In the centre: `Enter`, `Delete`, `Shift+Delete`, `R`, `S` | Default action, dismiss, remove from history, reply, snooze |
| In quick settings: `Space`, `Enter`, `Backspace` | Toggle, open detail, back |

Pointer: scroll on sound (volume), brightness (if the applet is shown) and clock (month); middle click on sound mutes, on a banner dismisses; swipe a banner sideways to dismiss; drag a banner's image or file chip out as a typed offer when the record carries a file handle. Drag and drop is otherwise only used in tile edit mode. Touch: swipe from the right edge opens the centre; swipe down on the status area opens quick settings.

### 5.6 States
| State | Behaviour |
|---|---|
| Empty | Centre: "No notifications". Media card hidden when no session |
| Loading | Applets show last known values from settings for at most 200 ms, then live values |
| Error | A dead back end greys its tile and sets a `--warn` mark on the status cluster; the tooltip names it |
| Offline | Network tile says "Not connected"; nothing else changes |
| Locked | Only OSDs, media keys and the lock-screen subset work; banners become redacted rows in SH03 |
| Read-only / policy | Managed tiles and rules show a padlock and cannot be changed |
| Do not disturb | Bell crossed; banners held; `critical` still shown |
| Sharing | Banners suppressed and bodies hidden on shared outputs (F033) |
| Guest / disposable | No history store; rules reset at session end |
| Narrow (< 640 px) | Centre and quick settings become full-width sheets; banners 100% width minus 16 px |

### 5.7 Widgets
| Region | FUSE widget |
|---|---|
| Banners, tiles, OSD, cards | `panel`, `label`, `image`, `separator` |
| Actions, header buttons, indicators | `button`, `toolButton` |
| Kill switches, master switches | `toggleSwitch` |
| Volume, brightness, temperature, mixer | `slider` |
| Progress, level meters, seek display | `progressBar` |
| Centre groups | `treeView` |
| Lists (Wi-Fi, devices, agenda, indicator rows, history) | `modelListView`, `listView` |
| Tile grid, edit mode | `gridView` |
| Tabs | `tabBar` |
| Device and DND pickers | `comboBox` |
| Reply and filter fields | `lineEdit` |
| Month grid | `calendar` |
| Schedules editor (in SY01 page) | `dateTimeEdit`, `checkBox` |
| Menus, tooltips | `popupMenu`, `tooltip` |
| Scrolling | `scrollArea` |

Missing:

| Gap | What is missing | Where it lands |
|---|---|---|
| G-01, FUSE-WP-10 | Layer surfaces with empty input regions and popups | FUSE |
| SH01-GAP-3 | Popover, sheet, toast and badge overlay widgets | FUSE-WP-30 |
| SH02-GAP-1 | Network service (links, Wi-Fi scan and join, MAC policy, captive portal detect). Not in PLATFORM.md section 8 | platform, G-09 |
| SH02-GAP-2 | Bluetooth service | platform; VM05 |
| SH02-GAP-3 | Display service: backlight, DDC/CI, night-light colour ramp, display mode. No gamma or output-management module is in `pbsd/compositor/` | compositor; SY01 |
| SH02-GAP-4 | Audio service behind `svc.media`: devices, per-app streams, levels | G-11, G-09 |
| SH02-GAP-5 | Device policy interface on each portal: global revoke-and-refuse switch, "in use" events with app identity | G-09 portals |
| SH02-GAP-6 | Compositor-drawn indicator dot and capture outline when no shell surface is visible; "exclude from capture" surface flag | compositor |
| SH02-GAP-7 | Global hardware-key routing (volume, brightness, media) from the compositor to SH02 | compositor; SH05 shortcut table |

## 6. Architecture

### 6.1 Components
| Component | Language | Process | Responsibility |
|---|---|---|---|
| Notification service | C++23 | `notify-service` (session service, capability mode) | Bus endpoint for `svc.notify`; validates records, applies limits and rules, keeps the store, routes actions back to apps |
| Status UI logic | ForgeScript | SH02 app host | Banners, centre, quick settings, popovers, OSDs, view models |
| Status UI | FUSE UI (`.fuseui`, CSS) | SH02 app host | All surfaces in 5.1 except the panel applets |
| Panel applets | ForgeScript + FUSE UI | SH01 shell host (in-process, system key, SH01-F119) | Thin views: icons, counts, clock text; they ask the SH02 host to open popovers |
| Indicator aggregator | C++23 | inside `notify-service` | Subscribes to portal "in use" events and compositor capture events; applies minimum display times |
| Kill switch controller | C++23 | inside `notify-service` | Holds switch state; calls the device policy interface of each portal and SE02 |
| Back-end clients | ForgeScript | SH02 app host | Clients for network, Bluetooth, display, audio, power, SE03, CO02, UT02 |
| Rule engine | C++23 | inside `notify-service` | DND, schedules, per-app and keyword rules; pure function of record, rules and time |

The service is native because it handles records from every app (hostile input), must stay up when the UI restarts, and controls the kill switches (PLATFORM.md section 2, reasons a and c).

```
 apps ──svc.notify──▶ +-------------------- notify-service (C++23) ------------------+
                      | validate → rules → store(SQLite) → route actions back       |
 portals ─in-use───▶  | indicator aggregator      kill switch controller            |
 compositor ─capture▶ +----^-----------------------------+-----------+--------------+
                           | bus (typed models)          |           | device policy
                  +--------+---------+        +----------v---+   +---v-----------------+
                  | SH02 app host    |        | SH03 lock    |   | portals: camera, mic|
                  | banners, centre, |        | (redacted    |   | screen, location,   |
                  | quick settings,  |        |  feed)       |   | usb; SE02 firewall  |
                  | popovers, OSD    |        +--------------+   +---------------------+
                  +---+----------+---+
        Wayland layer |          | bus
              +-------v--+   +---v--------------------------------------------+
              |compositor|   | network · bluetooth · display · svc.media ·    |
              +----------+   | svc.power · SE03 · CO02 · UT02 · svc.settings  |
     SH01 panel slots host the applets; applets talk to the SH02 host over the bus.
```

### 6.2 Data model
Store: `~/.local/share/org.pbsd.notifications/notify.db` (SQLite, WAL, `PRAGMA user_version`). Rules and switches live in `svc.settings`.

| Entity | Fields |
|---|---|
| `notification` | `id INTEGER PK`, `app_id TEXT` (from bus credentials), `app_local_id TEXT`, `group_key TEXT`, `category TEXT`, `urgency INTEGER`, `sensitivity INTEGER`, `title TEXT`, `body TEXT`, `icon_ref TEXT`, `image_blob_id INTEGER NULL`, `created_at INTEGER`, `updated_at INTEGER`, `expires_at INTEGER NULL`, `state INTEGER` (0 unread, 1 read, 2 dismissed, 3 expired, 4 withdrawn, 5 snoozed), `snooze_until INTEGER NULL`, `resident INTEGER`, `progress REAL NULL`, `progress_text TEXT NULL` |
| `action` | `notification_id`, `ordinal INTEGER`, `action_id TEXT`, `label TEXT`, `kind INTEGER` (0 default, 1 button, 2 destructive, 3 reply) |
| `blob` | `id`, `width`, `height`, `rgba BLOB` (already decoded and scaled to at most 640×360) |
| `AppRule` (settings) | `app_id`, `mode: enum{allow,silent,off}`, `banner: bool`, `sound: bool`, `lock_detail: enum{show,hide_body,hide_all}`, `category_overrides: map<category, mode>` |
| `DndState` (settings) | `on: bool`, `until: timestamp?`, `source: enum{manual,schedule,fullscreen,sharing,calendar}`, `focus_mode: string?` |
| `DndSchedule` (settings) | `id`, `days: u8 bitmask`, `start: hh:mm`, `end: hh:mm`, `enabled` |
| `DndExceptions` (settings) | `apps: [app_id]`, `categories: [category]`, `contact_ids: [string]` |
| `KillSwitch` (system settings namespace) | `device: enum{camera,mic,network,location,screen,usb,bluetooth}`, `enabled: bool`, `changed_at`, `changed_by: enum{user,panic,policy,hardware}` |
| `IndicatorEvent` (memory, mirrored to `svc.audit`) | `device`, `app_id`, `domain_id`, `started_at`, `ended_at?`, `target: string?` (output or window for capture) |
| `TileLayout` (settings) | `tiles: [tile_id]` |
| `MediaSession` (memory, from `svc.media`) | `session_id`, `app_id`, `title`, `artist`, `album`, `art_handle`, `state`, `position_ms`, `duration_ms`, `can: flags{seek,next,prev,shuffle,repeat}` |

`secret` records never reach the `notification` table. History pruning runs at start and hourly: rows older than `history.days`, and blobs with no row.

### 6.3 File formats and protocols
| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| `svc.notify` records (bus schema v1) | yes | — | schema compiler (G-08, FORGE-WP-26); bounds checked again in the service | `notify-service` (capability mode) |
| Body mini-markup (`*bold*`, `_italic_`, links) | yes | — | own code, single pass, no nesting deeper than 2 | `notify-service`; output is typed runs |
| Notification images, album art | pixels only | — | `svc.thumbnail` | thumbnail sandbox |
| SQLite store | yes | yes | SQLite (public domain), already used through `fuse_ui_sql` (PLATFORM.md V8) | `notify-service` |
| History export: JSON Lines, CSV | — | yes | own code | SH02 host, to a portal handle |
| Wayland: layer shell, activation, idle inhibit | client | client | `fuse_app_shell` (FUSE-WP-10) | SH02 host |
| Network, Bluetooth, display, audio, power interfaces | client | client | generated bus stubs | SH02 host |
| Holiday tables | yes | — | own key=value table format | SH02 host; files are in the signed bundle |

### 6.4 Integration
- Intents emitted: `open` (links, files in records), `compose` (new event). Handled: none.
- Services provided: `svc.notify` (v1), `org.pbsd.status.Indicators1` (read-only stream of indicator state for SH01 task marks, SH03 and SH05), `org.pbsd.status.KillSwitch1` (read for all shell parts; write only for SH02 UI, SY01 and SE01), `org.pbsd.status.Osd1` (`show(kind, value, text)` for SH05 and AX04).
- Services used: `svc.settings`, `svc.media`, `svc.power`, `svc.thumbnail`, `svc.intent`, `svc.audit`, `svc.keyring` (indirectly, through SH04's Wi-Fi prompt), `svc.portal.camera`, `.mic`, `.screen`, `.location`, `.usb` (device policy and in-use events), `svc.a11y`, `svc.model` (optional F056), `svc.search` (optional provider), `svc.portal.file` (export).
- Other apps: SH01 (slots, task marks), SH03 (lock feed, session actions, fallback path), SH04 (Wi-Fi password prompt), SH05 (capture outline, workspace OSD, shortcut table), SY01 (settings pages, display service), SY04 (energy users), SE01 and SE02 (security records, firewall cut, connection list), SE03 (routes), AD04 and AD05 (links), VM05 (Bluetooth pairing, USB policy), VM04 (remote mark), CO02, CO03, UT02, AX04, ME05, IN01 (captive portal window).
- Compositor (module files Verified present in `PBSD:pbsd/compositor/`): layer shell (`…wayland.layer.cppm` and parts), `…wayland.xdg_activation.cppm` (focus only after a click, F018), `…wayland.idle_inhibit.cppm` (Keep awake), `…wayland.keyboard_shortcuts_inhibit.cppm` and `…wayland.pointer_constraints.cppm` (source for F136), `…wayland.security_context.cppm` (identifying sandboxed clients). Only headers were read; server completeness is Unverified. Not found: capture, gamma or output management (SH02-GAP-3, SH02-GAP-6).

## 7. Security and privacy

Capabilities requested:

| Capability | Why |
|---|---|
| `fs.app` | History store, caches |
| `fs.portal` | History export only |
| `net.none` | SH02 opens no socket; network changes are requests to the network service |
| `proc.helper:notify-service` | The native service (started by the session supervisor) |
| `svc.settings`, `svc.media`, `svc.power`, `svc.thumbnail`, `svc.intent`, `svc.audit`, `svc.a11y` | As in 6.4 |
| `svc.portal.camera`, `svc.portal.mic`, `svc.portal.screen`, `svc.portal.location`, `svc.portal.usb` | Policy side only: in-use events and the revoke-and-refuse switch. SH02 never opens a device |
| `bg.run`, `bg.autostart` | Session component |
| `shell.layer` (Proposed, SH01-GAP-6) | Banners, centre, panels, OSD |
| `shell.device-policy` (Proposed) | Write access to kill switches |
| `ai.local` (optional, off) | Backlog summary |

Not requested: `dev.*` (no device is opened), `proc.exec`, `svc.keyring`, `svc.clipboard`.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| The user's belief about who is speaking | Hostile app | Sends a record that looks like it comes from the system or another app | Identity from bus credentials (F002); `critical` and `security` categories need a system-key bundle; the header line is drawn by SH02 |
| Indicator truth | Hostile app | Uses a device without showing it, or for under a blink | Events come from portals and the compositor (F129); minimum display and "recently used" trail (F131); compositor dot when the panel is hidden (F132) |
| Kill switch | Hostile app or remote attacker | Turns a switch back on | Write access needs `shell.device-policy`; never from the lock screen, a notification action or the bus by an ordinary app (F125); every change audited |
| `notify-service` integrity | Any app | Malformed or huge record | Schema-checked, size-limited, rate-limited; no decoder in the service; capability mode; T2 target on the validator |
| Message contents | Shoulder surfer, screen share viewer | Reads banners | Sensitivity levels, sharing rule (F033), lock-screen detail rules, presentation outputs |
| History | Other users, forensic reader | Reads old notifications | Store in the user's encrypted home; `secret` never stored; history off switch; none in guest sessions |
| User attention | Spamming app | Floods banners, fake urgency | Rate limit, per-app rules one click away, `high` urgency can be capped per app |
| Clicks | Hostile app | Times a banner under the pointer to catch a click (clickjacking) | Banner actions ignore input for 300 ms after the banner appears or moves |
| Focus | Hostile app | Raises itself by self-clicking a notification | Activation tokens are minted only on a real input serial |
| Links | Phishing record | Disguised link text | Link host always shown in full; no hidden-target links; opens through `svc.intent` |
| Wi-Fi passphrases | SH02 compromise | Reads secrets | Entered in SH04, held by `svc.keyring` and the network service only |

Stored: notification history (7 days default), rules, tile layout, switch state. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `banner.position` | enum{top_right,top_left,bottom_right,bottom_left,top_centre} | top_right | F020 |
| `banner.timeout_s` | u8 (3–30) | 6 | Normal urgency |
| `banner.timeout_multiplier` | enum{1,2,5,sticky} | 1 | Accessibility |
| `banner.max_visible` | u8 (1–5) | 3 | F021 |
| `banner.over_fullscreen` | enum{none,high,all} | high | F032 |
| `banner.hide_when_sharing` | bool | true | F033 |
| `banner.input_delay_ms` | u16 | 300 | Clickjacking guard; minimum 200 |
| `history.enabled` | bool | true | F047 |
| `history.days` | u8 (0–90) | 7 | Retention |
| `history.search_provider` | bool | false | F054 |
| `dnd.state` | json | off | `DndState` |
| `dnd.schedules` | json | [] | `DndSchedule` list |
| `dnd.exceptions` | json | categories alarm, call | `DndExceptions` |
| `dnd.auto_fullscreen` | bool | false | F060 |
| `dnd.auto_sharing` | bool | true | F060 |
| `dnd.exit_summary` | bool | true | F064 |
| `rules.apps` | map<app_id, AppRule> | {} | F065 |
| `rules.keywords` | list | [] | F067 |
| `lock.default_detail` | enum{show,hide_body,hide_all} | hide_body | Lock-screen default |
| `sound.enabled` | bool | true | Notification sounds |
| `sound.allow_boost` | bool | false | Volume above 100% |
| `sound.step_percent` | u8 | 5 | Key and scroll step |
| `osd.enabled` | bool | true | All OSDs |
| `osd.caps_lock` | bool | true | F142 |
| `osd.exclude_from_capture` | bool | true | F150 |
| `quick.tiles` | list<string> | wifi, bluetooth, route, dnd, night_light, power | Tile order |
| `indicators.min_display_s` | u8 | 3 | Cannot be set below 3 |
| `indicators.recent_s` | u8 | 30 | "Recently used" trail |
| `indicators.first_use_banner` | bool | true | F133 |
| `killswitch.<device>` | bool | camera, mic, location false; others true | F126 |
| `killswitch.panic_shortcut` | bool | true | F127 |
| `nightlight.mode` | enum{off,on,schedule,sun} | off | F107 |
| `nightlight.kelvin` | u16 | 3400 | Temperature |
| `battery.show_percent` | bool | false | Indicator text |
| `battery.low_percent` / `battery.critical_percent` | u8 | 15 / 7 | F115 |
| `clock.seconds`, `clock.date`, `clock.week_numbers` | bool | false, true, false | Clock applet |
| `clock.world` | list<tz> | [] | Up to 4 |
| `media.keys_on_lock_screen` | bool | true | F101 |
| `ai.summaries` | bool | false | F056 |

## 9. Accessibility and languages

- Banners are live regions: `polite` for low and normal, `assertive` for high and critical. Speech order is app name, title, body, action count. A repeated update of the same record is not spoken again unless `renotify`.
- Every indicator has a text alternative in the a11y tree ("Microphone in use by Messenger"). State changes of kill switches are announced.
- OSD values are announced only when a screen reader is running and the change came from a key press.
- Sound-only alerts have a visual option (F162); visual-only alerts (attention marks) have a sound option.
- Timeouts scale (F163). No information is shown only in a banner: everything also lands in the centre.
- The month grid follows the locale's first weekday and calendar system labels; week numbers follow ISO 8601 or the locale rule.
- Right-to-left: banners default to the top left, the centre opens on the left, sliders fill from the right.

## 10. Performance budgets

| Item | Budget |
|---|---|
| Record accepted to banner on screen | ≤ 50 ms (≤ 150 ms with an image) |
| Hardware key to OSD frame | ≤ 30 ms |
| Quick settings open | ≤ 80 ms to first frame with last known values |
| Centre open with 500 rows | ≤ 100 ms; list is virtualised |
| History search over 20,000 rows | ≤ 50 ms per key (indexed `LIKE` prefix on title and body trigram table) |
| Idle | 0 frames; the clock wakes once per minute; indicator timers only while a trail is shown |
| `notify-service` memory | ≤ 20 MB resident with 5,000 stored rows |
| SH02 host memory | ≤ 70 MB resident |
| Throughput | 200 records per second accepted (most rate-limited away) without dropping bus messages |

## 11. Testing and proof

Test plan (targets; nothing has run):

- Unit: record validator, mini-markup parser, rule engine (table-driven: record × rules × time → outcome), DND schedule arithmetic across midnight and daylight-saving changes, rate limiter, history pruning, indicator timing.
- Golden renders: banners of every urgency and state, centre, each quick settings page, popovers, OSDs, in all themes, densities and RTL.
- Scripted UI through the a11y tree: tasks T1–T15 under the headless compositor with fake back ends.
- Fuzz: `svc.notify` record decoder, mini-markup, settings documents, back-end event streams.
- Fault injection: kill each back end and the SH02 host; the service must keep accepting records and the kill switches must hold their state.
- Security tests: an app sends a record claiming another app's id (must show the real one); an app without `shell.device-policy` writes a kill switch (refused and audited); a device grab of 100 ms must leave a visible indicator for at least 3 s; banner input delay holds.
- Differential: indicator state against `svc.audit` records over a scripted session; they must agree event for event.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Record validator (`notify-service`) | No runtime error on any byte string; output within field limits | T2 | PRISM absence-of-runtime-error job; fuzzing | — |
| Rule engine | Pure and total; `critical` is never suppressed; a record with mode `off` never produces a banner | T1 | Small functional model checked by PRISM against the C++ | Model must be written first |
| Kill switch controller | Once a switch is off, no "grant" or "resume" is sent for that device until an authorised "on"; state machine has no path that skips audit | T1 | Model check of the state machine | Depends on portal interface being fixed (SH02-GAP-5) |
| Indicator aggregator | Every in-use start yields a visible interval of at least `min_display_s` | T2 | Proof over the timer logic; property tests | Real-time delivery is outside the proof |
| Mini-markup parser | Bounded output, no nesting overflow | T2 | PRISM plus fuzz | — |
| SQLite use | Parameterised statements only | T3 | Static check, code review | Third-party C library |
| UI logic (ForgeScript) | No runtime error in tests | T3/T4 | Scripted UI, sanitizer host | Interpreted code |

## 12. Build plan

Phase 1 — usable core:

| Item | Work | Depends on |
|---|---|---|
| SH02-W01 | `svc.notify` schema v1 and `notify-service` skeleton: validate, identity, limits | G-08, FORGE-WP-25, FORGE-WP-26 |
| SH02-W02 | Store, replace, withdraw, expiry, pruning | SH02-W01 |
| SH02-W03 | Banner stack: layout, timeout, actions, default action, activation tokens | G-01, FUSE-WP-10, SH01-GAP-6 |
| SH02-W04 | Notification centre: current list, groups, jobs, security section, clear, history tab | SH02-W02, FUSE-WP-28 |
| SH02-W05 | Rule engine: DND toggle and timed DND, per-app rules, rule from banner | SH02-W01 |
| SH02-W06 | Panel applets: bell, clock, sound, battery, network badge, indicators | SH01-W12 |
| SH02-W07 | Quick settings shell: tiles, sliders, footer, keyboard | SH02-W03 |
| SH02-W08 | Sound: volume, mute, devices | SH02-GAP-4, `svc.media` |
| SH02-W09 | Network tile and Wi-Fi page | SH02-GAP-1, SH04 |
| SH02-W10 | Brightness, power profile, battery indicator, low battery records | SH02-GAP-3, `svc.power` |
| SH02-W11 | Kill switches: camera, mic, network; controller and audit | SH02-GAP-5, SE02 |
| SH02-W12 | Indicators: aggregator, popover, Stop, first-use banner | SH02-GAP-5 |
| SH02-W13 | OSDs: volume, brightness, mute, lock keys, kill switch | SH02-GAP-7 |
| SH02-W14 | Media card and media keys | `svc.media` |
| SH02-W15 | Calendar popover (month grid only) | — |
| SH02-W16 | Lock-screen feed for SH03 | SH03 |
| SH02-W17 | Settings schema and SY01 pages; a11y announcements | SY01, G-04 |

Phase 2 — full feature:

| Item | Work | Depends on |
|---|---|---|
| SH02-W18 | Inline reply, call banner, snooze, sounds, resident records | SH02-W03 |
| SH02-W19 | DND schedules, automatic DND, exceptions, exit summary | SH02-W05, CO03 |
| SH02-W20 | History search and filters | SH02-W04 |
| SH02-W21 | Bluetooth tile and page | SH02-GAP-2, VM05 |
| SH02-W22 | Route tile and states | SE03 |
| SH02-W23 | Night light, dark mode, keep awake, display mode chooser, rotation | SH02-GAP-3, SY01 |
| SH02-W24 | Per-app mixer, multiple media sessions, output switch prompt | SH02-GAP-4 |
| SH02-W25 | Battery popover, peripheral batteries | SY04, `svc.power` |
| SH02-W26 | Remaining kill switches, panic shortcut, compositor dot, capture outline, input-capture and remote marks | SH02-GAP-6, SH05, VM04, VM05 |
| SH02-W27 | Agenda and new event | CO02 |
| SH02-W28 | Tile edit mode, lock-screen quick settings subset | SH03 |
| SH02-W29 | Start-app-on-action for closed apps | SH01-GAP-5 |

Phase 3 — advanced:

| Item | Work | Depends on |
|---|---|---|
| SH02-W30 | Focus modes, keyword rules, digest, policy | SH02-W19 |
| SH02-W31 | Local summaries | G-15, `svc.model` |
| SH02-W32 | Third-party data-only tiles | SH02-W07 |
| SH02-W33 | Hotspot, data use, metered networks, per-app route peek | SH02-GAP-1, SE02, SE03 |
| SH02-W34 | Hardware switch state, charge limit, keyboard backlight | VM05, firmware support |
| SH02-W35 | World clocks, alarms, holidays, time source mark | UT02 |
| SH02-W36 | History export, search provider, capture exclusion, fallback path through SH03 | SH02-GAP-6, SH03 |

## 13. Open questions and risks

1. **Back ends do not exist.** Network, Bluetooth, display and audio services (SH02-GAP-1..4) are not in PLATFORM.md section 8 and no code for them was seen in this repo. Quick settings is only as good as they are.
2. **Device policy on portals (SH02-GAP-5)** must be designed with the portals (SH04, G-09). The kill switch proof depends on that interface.
3. **Microphone kill versus calls in progress.** Cutting the microphone ends the stream for the app; apps must handle "device unavailable" cleanly. This needs a platform test for every app that uses `dev.mic`.
4. **Network kill switch and the session bus.** The bus is Unix-domain and stays up; anything using loopback TCP must be reviewed.
5. **Compositor-drawn indicator (SH02-GAP-6)** adds drawing to the compositor. The alternative, forcing a strip of the panel visible over fullscreen windows, is worse for video and games.
6. **Applets in the SH01 process** means an SH02 applet bug can crash the panel. The applets must stay thin; all logic is in the SH02 host.
7. **Notification history is sensitive.** Seven days is a compromise; some users will want 0 by default. The first-run tour (AD01) should ask.
8. **`secret` sensitivity** relies on apps marking records correctly. SE04 and CO05 must mark one-time codes and message bodies.
9. **Hardware keys** vary by machine; mapping tables belong to the input stack, not SH02.
10. Risk: T1 targets for the rule engine and kill switch controller need PRISM models that do not exist yet.
