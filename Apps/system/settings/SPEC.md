<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Settings

By Odin Loch

| | |
|---|---|
| ID | `SY01` (from CATALOGUE.md) |
| Bundle | `org.pbsd.settings` |
| Category | system |
| Priority | P0 |
| Backing repos | ParanoidBSD (`tools/hardening_sysctls.py`, `docs/ROADMAP.md` section 15), FUSE (widgets), FORGE (ForgeScript, OS layer) |
| Status | Planned. Nothing built. |

## 1. Purpose

Settings is the one place where a person changes how the system behaves: network, display, sound, input, power, appearance, users, date, region, accessibility, privacy and the hardening profile. It is used by every user, from first run to daily tweaks, and by administrators who need the exact state of every HardenedBSD mitigation. It replaces Plasma System Settings from the current plan (PLATFORM.md V20).

## 2. Scope

In scope:

- Every system-wide and per-user setting, shown as pages in one window, with search over every control.
- Editing through `svc.settings` and through narrow, typed configuration services. Settings never edits files under `/etc` itself.
- The hardening profile page: read, explain and change `hardening.*` sysctls within policy.
- Overview pages for privacy, storage, printers and updates that hand off to the owning app.
- Export, import, reset and history of settings.

Out of scope:

| Item | Owner |
|---|---|
| Per-app capability grants, handle lineage, revocation | AD05 Permissions (Settings shows a summary and links) |
| pf rules, per-app network policy, blocklists | SE02 Firewall |
| Tor, anonymous routing, per-app routes | SE03 Anonymity (Settings only lists VPN profiles and the default route) |
| Disks, pools, datasets, encryption | AD02 Disks; space breakdown in UT04 Disk Usage |
| Packages, updates, boot environments | SY05 Software |
| Print queues, printer and scanner setup | UT06 Printers and Scanners |
| Drivers, firmware, USB policy, IOMMU | VM05 Devices |
| Passwords, keys, passkeys, tokens | SE04 Passwords and Keys |
| Quick toggles in the panel | SH02 Notifications and Quick Settings |
| Window rules, tiling engine | SH05 Window Management (Settings hosts its options page) |
| Input method engines, emoji picker | AX04 Input Methods and Emoji |
| Screen reader, magnifier, on-screen keyboard internals | AX01, AX02, AX03 (Settings holds their switches and options) |
| Backup schedules | AD03 Backup |
| Security posture score, intrusion detection | SE01 Security Centre |

## 3. Users and top tasks

Personas:

- **Everyday user**: joins Wi-Fi, pairs headphones, changes wallpaper, sets night light.
- **Administrator**: manages users and groups, DNS, proxies, time servers, hardening state.
- **Security-minded user**: audits which capabilities are in use, tightens the hardening profile, checks nothing drifted.
- **Assistive-technology user**: needs every page to work by keyboard and screen reader, and needs accessibility options within two steps.

| # | Task | Target steps |
|---|---|---|
| T1 | Join a Wi-Fi network | 3 (open Wi-Fi, pick, type passphrase) |
| T2 | Find any setting by name | 2 (`Ctrl+F`, type) |
| T3 | Change display scale and keep or revert | 3 |
| T4 | Arrange two monitors | 2 (open Display, drag) |
| T5 | Pair a Bluetooth device | 3 |
| T6 | Switch output sound device | 2 |
| T7 | Turn on dark theme and pick an accent | 2 |
| T8 | Add a user | 4 |
| T9 | Set a custom DNS-over-TLS resolver | 4 |
| T10 | Import a WireGuard profile | 3 |
| T11 | See which hardening knobs differ from the profile | 1 (open Hardening) |
| T12 | Switch hardening profile and see what will break | 3 |
| T13 | Change the default browser | 3 |
| T14 | Turn on large text and high contrast | 2 |
| T15 | Undo the last settings change | 1 (`Ctrl+Z`) |

## 4. Features

### 4.1 Shell of the app

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY01-F001 | Page tree | Sidebar tree of categories and pages (section 5.1). Each page has a stable id `settings://<category>/<page>#<control>`. | M | 1 |
| SY01-F002 | Deep links | Any app opens a page or control through the `settings:` URL scheme; the control is scrolled to and ringed for 2 s. | M | 1 |
| SY01-F003 | Settings search | Index of every page, group, control label, hint, synonym and value option. Fuzzy, typo-tolerant, in the current language and English. Design in section 5.3.1. | M | 1 |
| SY01-F004 | Instant apply | A change applies at once. No Apply button, except on risky pages (display mode, network address, hardening, users) which stage changes. | M | 1 |
| SY01-F005 | Timed revert | Changes that can lock the user out (resolution, scale, refresh, network address of the active link) show a 15 s keep-or-revert bar. | M | 1 |
| SY01-F006 | Undo and redo | `Ctrl+Z` reverts the last change made in this session; a toast names it. | M | 1 |
| SY01-F007 | Change history | Per-key history (who, when, old, new, source app) kept by `svc.settings`; view and restore from a History sheet. | S | 2 |
| SY01-F008 | Modified marker | A dot beside any control whose value differs from the default; click offers "Reset to default". | S | 1 |
| SY01-F009 | Locked settings | Controls fixed by an administrator policy show a lock and the policy name; they are read-only. | M | 2 |
| SY01-F010 | Elevation | Pages that change system state call the authentication prompt (SH03) once per page per 5 minutes. The page shows "Unlock to change". | M | 1 |
| SY01-F011 | Per-user and system scope | Each control states its scope with a small tag: "this user" or "all users". | M | 1 |
| SY01-F012 | Export settings | Write chosen pages to a signed `.pbsettings` file (JSON, section 6.2). Secrets are left out unless the user opts in and sets a passphrase. | S | 2 |
| SY01-F013 | Import settings | Read a `.pbsettings` file, show a diff, apply selected keys. Signature and schema version are checked first. | S | 2 |
| SY01-F014 | Reset page / reset all | Reset one page or everything for this user to defaults; a snapshot is taken first. | M | 2 |
| SY01-F015 | Favourites and recents | Pin pages; the home view lists pinned and the last 8 visited. | C | 2 |
| SY01-F016 | Home overview | Cards: network state, display, sound device, battery, updates pending, hardening drift, storage free. Each links to its page. | S | 1 |
| SY01-F017 | Pages from other apps | An app may ship a settings page (`.fuseui` + schema) listed under "Apps". It runs with that app's `svc.settings` namespace only. | S | 2 |
| SY01-F018 | Help per control | `F1` on a control opens its help paragraph in a popover; "More" opens UT05 Help. | S | 1 |
| SY01-F019 | Copy as command | For system keys, a menu item copies the equivalent `sysctl`/`sysrc` style line for scripts. Read-only convenience. | C | 3 |
| SY01-F020 | ForgeScript automation | A documented `settings::get/set/watch` module usable from scripts with `svc.settings`; this app adds nothing private. | S | 2 |
| SY01-F021 | Safe mode banner | If the session started with default settings after a bad change, a banner offers the last known good set. | S | 2 |
| SY01-F022 | Offline help | All explanatory text ships in the bundle; no page needs the network to explain itself. | M | 1 |

### 4.2 Network

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY01-F023 | Connections list | All interfaces with state, address, speed, route in use, data counters. | M | 1 |
| SY01-F024 | Ethernet | Auto or manual link speed and duplex, MTU, 802.1X (EAP-TLS, PEAP, TTLS), VLAN id, cloned MAC. | M | 1 |
| SY01-F025 | Wi-Fi scan and join | List by signal; shows security (WPA2, WPA3-SAE, OWE, Enterprise), band, channel. Open networks carry a warning word. | M | 1 |
| SY01-F026 | Wi-Fi saved networks | Order, auto-join, forget, share by QR code (after authentication), metered flag. | M | 1 |
| SY01-F027 | MAC address privacy | Per network: random per network (default), random per connection, device address. | M | 1 |
| SY01-F028 | Hidden network and enterprise | Join by name; certificate pinning for EAP; CA and client certificate picked from SE04. | S | 2 |
| SY01-F029 | Hotspot | Share a connection over Wi-Fi with WPA3; shows connected clients. | C | 3 |
| SY01-F030 | IPv4 settings | DHCP or manual: address, prefix, gateway, routes, DHCP client id. | M | 1 |
| SY01-F031 | IPv6 settings | SLAAC, DHCPv6, manual, off; privacy addresses on by default; stable-privacy option. | M | 1 |
| SY01-F032 | DNS | System resolver mode: from network, custom, DNS-over-TLS, DNS-over-HTTPS, local validating resolver. DNSSEC validate on. Per-connection override. Search domains. | M | 1 |
| SY01-F033 | DNS test | "Test resolver" button reports latency, DNSSEC result and the server certificate. | S | 2 |
| SY01-F034 | VPN profiles | WireGuard, OpenVPN, IPsec IKEv2. Import `.conf`, `.ovpn`; keys stored in `svc.keyring`. Connect, auto-connect, per-network rules. | M | 2 |
| SY01-F035 | VPN kill switch | "Block traffic when the VPN drops" toggle; implemented by SE02; shown here with a link. | M | 2 |
| SY01-F036 | Default route class | Choose system default: clear, a VPN profile, or anonymous (hands to SE03). Maps to `net.route:<clear\|vpn\|tor>`. | S | 2 |
| SY01-F037 | Proxies | None, manual (HTTP, HTTPS, SOCKS5), auto-config URL fetched without script execution, exceptions list. Per-connection or global. | M | 2 |
| SY01-F038 | Metered connections | Mark a network metered; SY05 and AD03 respect it. | S | 2 |
| SY01-F039 | Hostname | Static hostname; option to send no hostname in DHCP (default on). | M | 1 |
| SY01-F040 | Aeroplane mode | All radios off; remembers Bluetooth and Wi-Fi state. | M | 1 |
| SY01-F041 | Network diagnostics | Link, address, gateway, DNS, internet check steps with plain results. The connectivity check target is user-set and off by default. | S | 2 |
| SY01-F042 | Bonds, bridges, VLANs | Create lagg, bridge and vlan interfaces. | C | 3 |
| SY01-F043 | Hosts file entries | Table editor for static name entries. | C | 3 |

### 4.3 Bluetooth

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY01-F044 | Adapter switch | On, off, discoverable for 2 minutes only; adapter name. | M | 2 |
| SY01-F045 | Pairing | Scan, pair with numeric comparison or passkey; legacy PIN pairing warns. | M | 2 |
| SY01-F046 | Device list | Paired devices with type, battery, connect, forget, trust level, allowed profiles (audio, input, file). | M | 2 |
| SY01-F047 | Device hand-off | "Advanced" opens VM05 Devices at the device node. | S | 2 |

### 4.4 Display

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY01-F048 | Arrangement canvas | Drag outputs to place; snapping to edges; identify button shows a number on each screen. | M | 1 |
| SY01-F049 | Mode | Resolution, refresh rate, variable refresh (off, full screen, always). Lists preferred mode first. | M | 1 |
| SY01-F050 | Scale | 100% to 300% in 25% steps, per output, fractional. Preview text sample. | M | 1 |
| SY01-F051 | Rotation and mirror | 0, 90, 180, 270; mirror groups; primary output. | M | 1 |
| SY01-F052 | HDR | Per output on/off, peak brightness, SDR content brightness, tone-mapping test pattern. Disabled with reason when unsupported. | S | 3 |
| SY01-F053 | Colour profile | Assign an ICC profile per output; built-in sRGB, Display P3; import; calibrate hand-off. | S | 2 |
| SY01-F054 | Night light | Schedule (sunset to sunrise from manual coordinates, or fixed times), temperature 1700–6500 K, preview, pause until tomorrow. No location lookup unless granted. | M | 1 |
| SY01-F055 | Brightness | Per panel and per DDC/CI monitor; auto brightness when a sensor exists. | M | 2 |
| SY01-F056 | Display profiles | Save named layouts; auto-apply by connected output set (EDID hash). | S | 2 |
| SY01-F057 | Lid and docking | What happens on lid close with external display; restore layout on undock. | S | 2 |
| SY01-F058 | Privacy screen | Blank external outputs on lock; hide notifications when mirroring. | S | 2 |
| SY01-F059 | Overscan and text rendering | Overscan compensation; subpixel order; hinting level. | C | 3 |

### 4.5 Sound

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY01-F060 | Output device | Pick device and port, volume, balance, mute, over-amplification limit, test tone per channel. | M | 1 |
| SY01-F061 | Input device | Pick device, gain, live level meter, mute, noise suppression (local). | M | 1 |
| SY01-F062 | Per-app volume | List of playing streams with volume and target device. | M | 2 |
| SY01-F063 | Profiles | Card profile (stereo, surround 5.1/7.1, HDMI), sample rate, format, buffer size. | S | 2 |
| SY01-F064 | System sounds | Theme, volume, per-event on/off, alert preview. | S | 2 |
| SY01-F065 | Microphone indicator link | Shows which app holds `dev.mic` now; link to AD05. | M | 1 |
| SY01-F066 | Device rules | Default device per class; auto-switch on plug; disable a device. | S | 2 |

### 4.6 Power

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY01-F067 | Performance profile | Power saver, balanced, performance; per source (battery, mains). | M | 1 |
| SY01-F068 | Screen and sleep timers | Dim, blank, lock, suspend, hibernate after N minutes, per source. | M | 1 |
| SY01-F069 | Buttons and lid | Action for power button, sleep button, lid close. | M | 1 |
| SY01-F070 | Battery | Health, cycles, charge limit (60–100%), time estimates, history graph (hand-off to SY04). | S | 2 |
| SY01-F071 | Low-battery policy | Warn at, save at, suspend at percentages. | M | 2 |
| SY01-F072 | Sleep security | Suspend-to-RAM evicts disk keys (option), hibernate needs encrypted swap; states which is active. | M | 2 |
| SY01-F073 | Inhibitors | Lists apps holding `svc.power` inhibits with reason; allow revoking. | S | 2 |
| SY01-F074 | Scheduled power | Wake and shut down at set times. | C | 3 |

### 4.7 Input devices

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY01-F075 | Keyboard layouts | Add, order, per-window or global, switch shortcut, layout preview drawing. | M | 1 |
| SY01-F076 | Key behaviour | Repeat delay and rate, Caps Lock remap, Compose key, numeric keypad options, test field. | M | 1 |
| SY01-F077 | Shortcuts editor | Every system and app action (from the shared action table), search by name or by pressing keys, conflict detection, reset, export. | M | 1 |
| SY01-F078 | Custom shortcuts | Bind a key to launch an app or a signed ForgeScript action. | S | 2 |
| SY01-F079 | Mouse | Speed, acceleration profile (flat, adaptive), handedness, natural scroll, double-click time, middle-click emulation, per-device overrides. | M | 1 |
| SY01-F080 | Touchpad | Tap to click, two-finger scroll, disable while typing, palm rejection, click method, gestures (3 and 4 fingers) mapped to SH05 actions. | M | 1 |
| SY01-F081 | Pointing stick and trackball | Sensitivity, scroll button. | C | 2 |
| SY01-F082 | Graphics tablets | Map to output or area, aspect lock, pressure curve (`curveEditor`), button and ring mapping, per-app profiles, left-handed. | S | 2 |
| SY01-F083 | Touchscreen | Map to output, calibrate, enable/disable. | S | 2 |
| SY01-F084 | Game controllers | List devices, live axis and button test, dead zones, remap, calibration, rumble test. | S | 3 |
| SY01-F085 | Input device trust | New keyboards on USB need approval when "ask for new input devices" is on (policy owned by VM05; switch shown here). | M | 2 |
| SY01-F086 | Input methods link | Page hosts AX04 options: engines per layout, emoji shortcut. | S | 2 |

### 4.8 Appearance and desktop

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY01-F087 | Theme | Dark, light, high contrast, automatic by schedule. | M | 1 |
| SY01-F088 | Accent colour | Eight presets or a custom colour checked against contrast rules; follows wallpaper option. | M | 1 |
| SY01-F089 | Fonts | UI face, monospace face, document face, sizes, antialiasing; preview pane; opens UT03 for installing. | M | 1 |
| SY01-F090 | Density | Compact, normal, touch; per app override list. | M | 1 |
| SY01-F091 | Wallpaper | Image, slideshow, solid colour, gradient; per output and per workspace; fit modes; images decoded by the thumbnail sandbox. | M | 1 |
| SY01-F092 | Icons and cursors | Icon theme, cursor theme and size. Themes are data only, signed. | S | 2 |
| SY01-F093 | Motion and effects | Animation speed, reduced motion, transparency, blur. | M | 1 |
| SY01-F094 | Panel and launcher options | Hosts SH01 options: panel position, size, auto-hide, clock format. | S | 2 |
| SY01-F095 | Lock screen appearance | Background, notifications on lock (hide content by default), clock style. | S | 2 |
| SY01-F096 | Notifications | Per-app: allow, sound, banner, lock-screen visibility, priority; do-not-disturb schedule and exceptions; history length. | M | 1 |
| SY01-F097 | Search settings | Which locations and apps `svc.search` indexes, exclusions, rebuild index, index size; results order in SH01. | M | 2 |
| SY01-F098 | Multitasking | Hosts SH05 options: snapping, tiling gaps, workspaces count and behaviour, hot corners, window switcher style, focus mode. | M | 2 |
| SY01-F099 | Default apps | Per role (browser, mail, text, images, video, music, PDF, terminal, files, calendar) and per MIME type and URL scheme table; reset. | M | 1 |
| SY01-F100 | Startup apps | List of apps with `bg.autostart`; switch each; delay. Links to SY04 for cost. | S | 2 |
| SY01-F101 | Removable media | Action per media type: ask, open Files, nothing (default nothing). Never auto-run. | M | 2 |

### 4.9 Users, time and region

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY01-F102 | Users list | Name, picture, role (administrator, standard, guest), last login, encrypted home state. | M | 1 |
| SY01-F103 | Add and remove user | Wizard: name, login, role, password or passkey, own ZFS dataset, encryption. Remove keeps or destroys data, with snapshot. | M | 1 |
| SY01-F104 | Authentication methods | Password change with strength meter (local), hardware token, fingerprint where supported; hands key items to SE04. | M | 2 |
| SY01-F105 | Groups | Create, delete, membership matrix; system groups hidden by default. | M | 2 |
| SY01-F106 | Login options | Automatic login off by default and blocked when the home is encrypted; guest session; show user list or not. | M | 2 |
| SY01-F107 | Session limits | Per user: login hours, resource class, apps allowed (maps to capability profile; detail in AD05). | C | 3 |
| SY01-F108 | Date and time | Automatic by NTP with NTS (authenticated time) or manual; server list; drift readout; hardware clock UTC. | M | 1 |
| SY01-F109 | Time zone | Map and list picker; automatic zone is off unless location is granted. | M | 1 |
| SY01-F110 | Language | UI language order; per-app override; install language data from SY05. | M | 1 |
| SY01-F111 | Formats | Region for dates, numbers, currency, paper size, first weekday, units; live examples. | M | 1 |
| SY01-F112 | Spelling languages | Dictionaries for the spell service (gap G-17). | S | 2 |

### 4.10 Accessibility

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY01-F113 | Seeing | Text size 80–300%, bold text, high contrast, colour filters (protanopia, deuteranopia, tritanopia, greyscale), cursor size, focus ring thickness. | M | 1 |
| SY01-F114 | Screen reader switch | Start AX01; voice, rate, verbosity options hosted from AX01. | M | 1 |
| SY01-F115 | Magnifier switch | Start AX02; zoom mode options hosted from AX02. | M | 2 |
| SY01-F116 | Hearing | Visual alerts (flash screen or window), mono audio, captions style. | M | 2 |
| SY01-F117 | Typing | Sticky keys, slow keys, bounce keys, on-screen keyboard switch (AX03). | M | 1 |
| SY01-F118 | Pointing | Mouse keys, dwell click, click assist, pointer trails, locate pointer. | M | 2 |
| SY01-F119 | Voice control switch | Start AX06; local only. | C | 3 |
| SY01-F120 | Accessibility shortcut | One global shortcut and a login-screen menu to toggle chosen aids. | M | 1 |

### 4.11 Privacy, security and hardening

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY01-F121 | Privacy overview | One row per capability class (files, network, camera, microphone, screen, location, USB, print, GPU compute, AI, background). Shows count of apps granted, in use now, last used. Row opens AD05 at that capability. | M | 1 |
| SY01-F122 | Global device switches | System-wide off for camera, microphone, location, Bluetooth. Off means the portal refuses every request. | M | 1 |
| SY01-F123 | Location source | Off (default), manual coordinates, local Wi-Fi-free GNSS device. No network location service. | M | 2 |
| SY01-F124 | Screen lock | Lock after, lock on suspend, grace period, show notifications. | M | 1 |
| SY01-F125 | History and recents | Keep recent files or not, clear now, clipboard history length (hands to AX05). | S | 2 |
| SY01-F126 | Diagnostics | States "nothing is sent". Crash reports stay local; open AD04 Logs. | M | 1 |
| SY01-F127 | Hardening status table | One row per `hardening.*` knob: path, kind (2-state, 4-state, int), compiled default, running value, profile value, status (match, drift, relaxed, tightened), plain meaning. Knob list and defaults follow `tools/hardening_sysctls.py`. | M | 1 |
| SY01-F128 | Hardening profiles | Three shipped profiles: Standard (compiled defaults), Strict (adds `hardening.elf_pie_only=1`, `hardening.pax.tpe.status=2`, `hardening.kmalloc_zero=1`, `hardening.forbid_kmod` after boot), Custom. Profile values are Proposed. | M | 2 |
| SY01-F129 | Impact preview | Before a change: lists installed binaries that would stop running (non-PIE under `elf_pie_only`, untrusted paths under TPE), from an inventory scan by the hardening helper. | M | 2 |
| SY01-F130 | Staged apply with boot environment | Changes that need a reboot are written into a new boot environment through `svc.update`; rollback is one click in SY05. | M | 2 |
| SY01-F131 | Drift alarm | If a running value is weaker than the profile, the home card and SE01 show a warning; one click restores. | M | 2 |
| SY01-F132 | Per-binary exemptions | Table of binaries excused from a 4-state feature (the opt-out list), with reason, who added it and when. Adding one needs authentication and is audited. | S | 3 |
| SY01-F133 | Build options view | Read-only list of compiled mitigations (ASLR, PIE, RELRO, BIND_NOW, SSP, SafeStack, CFI) as reported by the build. Marked as build facts, not run-time state. | S | 2 |
| SY01-F134 | Kernel module policy | Shows `forbid_kmod` state and loaded modules; link to VM05. | S | 2 |
| SY01-F135 | Securelevel and boot state | Read-only: securelevel, verified boot state; link to SE01. | S | 2 |
| SY01-F136 | Export hardening report | Signed text report of all knobs, for audits. Same columns as F127. | S | 2 |

### 4.12 System pages and hand-offs

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SY01-F137 | About | Device name, OS version, build id, kernel, boot environment, hardware summary, uptime, serial numbers hidden until revealed. Copy as text. | M | 1 |
| SY01-F138 | Licences | Licence texts of all shipped components, offline. | M | 2 |
| SY01-F139 | Storage summary | Pool free space, per-category bar, snapshot space; buttons open AD02, UT04, AD03. | M | 1 |
| SY01-F140 | Printers summary | Default printer and paper size; "Manage" opens UT06. | S | 2 |
| SY01-F141 | Updates summary | Last check, pending count, automatic policy (off, download, install at shutdown); "Open Software" goes to SY05. | M | 1 |
| SY01-F142 | Devices summary | Connected devices count; opens VM05. | S | 2 |
| SY01-F143 | Firewall summary | Profile in force and blocked count today; opens SE02. | S | 2 |
| SY01-F144 | Backup summary | Last backup and next run; opens AD03. | S | 2 |
| SY01-F145 | First-run reuse | AD01 embeds the Network, Region, Users and Appearance pages unchanged. | M | 1 |
| SY01-F146 | Remote and sharing | Device visible name, SSH server switch (off), remote desktop switch (hands to VM04), file sharing off. Each shows listening ports. | S | 3 |
| SY01-F147 | Audit of changes | Every system-scope change is written to `svc.audit` with old and new values (secrets redacted). | M | 1 |
| SY01-F148 | Managed configuration | Read a signed policy file that pins keys (see F009); shows its signer. | C | 3 |

## 5. GUI

### 5.1 Window map

```
Settings main window (single instance)
├─ Header bar: menu, title, search field, window buttons
├─ Sidebar (treeView): page tree
│   ├─ Home
│   ├─ Network ── Connections · Wi-Fi · Ethernet · VPN · Proxies · DNS · Hotspot · Diagnostics
│   ├─ Bluetooth
│   ├─ Display ── Arrangement · Night light · Colour and HDR · Profiles
│   ├─ Sound ── Output · Input · Apps · System sounds
│   ├─ Power ── Profile and timers · Battery · Buttons and lid
│   ├─ Input ── Keyboard · Shortcuts · Mouse · Touchpad · Tablets · Touchscreen · Game controllers
│   │           · Input methods
│   ├─ Appearance ── Theme and accent · Fonts · Wallpaper · Icons and cursors · Effects
│   │                · Panel and launcher · Lock screen
│   ├─ Desktop ── Notifications · Search · Multitasking · Default apps · Startup apps
│   │             · Removable media
│   ├─ Users ── Users · Groups · Login options
│   ├─ Time and region ── Date and time · Language · Formats · Spelling
│   ├─ Accessibility ── Seeing · Hearing · Typing · Pointing · Assistive tools
│   ├─ Privacy ── Overview · Device switches · Location · Screen lock · History · Diagnostics
│   ├─ Security ── Hardening · Kernel modules · Boot state · Firewall (link) · Remote and sharing
│   ├─ System ── About · Storage · Printers · Devices · Updates · Backup · Licences
│   └─ Apps ── one page per app that ships one
├─ Content: breadcrumb, page (scrollArea of groups), keep-or-revert bar, unlock bar
├─ Dialogs: Join Wi-Fi · Edit connection · Import VPN · Add user · Remove user · Add layout
│           · Record shortcut · Pair device · Import settings (diff) · Export settings
│           · Hardening impact · Add exemption · Colour profile import
├─ Sheets: History of a key · Reset page · Help popover
└─ Search results overlay
```

### 5.2 Main window

Shown at 1100×720, page Display → Arrangement.

```
+----------------------------------------------------------------------------------------+
| [=] Settings                  [ Search settings…            Ctrl+F ]        [▁] [▢] [✕] | (1)
+----------------------+-----------------------------------------------------------------+
| > Home               | Display › Arrangement                              [?] [Reset]  | (3)
|   Network          ▸ +-----------------------------------------------------------------+
|   Bluetooth          |  +---------------------------------------------------------+    |
| v Display            |  |            +-----------+ +---------------------+        |    |
|   > Arrangement      |  |            |     2     | |          1          |        |    | (4)
|     Night light      |  |            | 1920x1080 | |      2560x1440      |        |    |
|     Colour and HDR   |  |            +-----------+ +---------------------+        |    |
|     Profiles         |  +---------------------------------------------------------+    |
|   Sound            ▸ |  [Identify]  [Mirror…]                    Profile [Desk    v]   |
|   Power            ▸ |                                                                 |
|   Input            ▸ |  Selected: 1 — DP-1, 27 inch                                    |
|   Appearance       ▸ |  Resolution   [2560 x 1440 (preferred) v]   ● modified          | (5)
|   Desktop          ▸ |  Refresh      [144 Hz v]   Variable refresh [Full screen v]     |
|   Users            ▸ |  Scale        [----o-------] 125 %   Sample: The quick brown    |
|   Time and region  ▸ |  Rotation     [None v]      [x] Primary                         |
|   Accessibility    ▸ |  Brightness   [---------o--] 80 %                               |
|   Privacy          ▸ |  HDR          ( ) off   Not supported by this monitor     [?]   |
|   Security         ▸ |                                                                 |
|   System           ▸ +-----------------------------------------------------------------+
|   Apps             ▸ |  Keep these display settings?  Reverting in 12 s  [Keep][Revert]| (6)
+----------------------+-----------------------------------------------------------------+
| this user · 1 change this session · [Undo]                         all users: locked 🔒 | (7)
+----------------------------------------------------------------------------------------+
```

1. Header bar. The search field is always visible; `Ctrl+F` and `Ctrl+K` focus it. No network badge is ever shown: the app has `net.none`.
2. Sidebar, 220 px, `treeView`. Categories expand in place. Type-ahead selects a page. Pinned pages sit above Home when any exist.
3. Breadcrumb, help and page reset. Breadcrumb parts are links.
4. Arrangement canvas (custom-drawn `panel`; pointer and arrow-key drag; `Tab` cycles outputs). Outputs are drawn to scale.
5. Property rows for the selected output. A dot marks a value that differs from the default (F008).
6. Keep-or-revert bar (F005). Appears only after a risky change. `Enter` keeps, `Esc` reverts.
7. Status bar: scope tag, change count, Undo, and the unlock state for system-scope controls. Clicking the lock runs the authentication prompt.

States: loading (skeleton rows while a service answers, at most 300 ms before a spinner), service unavailable (inline message with Retry and a link to AD04), locked (controls dimmed with the lock tag), narrow (section 5.6).

### 5.3 Other views and dialogs

#### 5.3.1 Settings search

```
+----------------------------------------------------------------------------------------+
| [=] Settings                  [ dns over                         ✕ ]        [▁] [▢] [✕] |
+----------------------+-----------------------------------------------------------------+
|                      | 6 results for "dns over"                                        |
|   (sidebar dimmed)   | > Network › DNS › Resolver mode: DNS-over-TLS            control |
|                      |   Network › DNS › Resolver mode: DNS-over-HTTPS          option  |
|                      |   Network › DNS › Validate DNSSEC                        control |
|                      |   Network › VPN › Use VPN DNS only                       control |
|                      |   Firewall › DNS policy                           opens Firewall |
|                      |   Help › How name resolution is protected                  help  |
|                      |                                                                 |
|                      |   Enter open · Ctrl+Enter change here · Esc clear               |
+----------------------+-----------------------------------------------------------------+
```

Design:

- **Index source.** At build time a tool walks every page `.fuseui` and its settings schema and emits `search-index.tbl`: one record per page, group, control and option, with `id`, `path`, `label_key`, `hint_key`, `synonyms_key`, `kind`, `scope`, `needs_hardware`. Strings resolve through the locale tables, so the index exists in every shipped language plus English.
- **Matching.** Lower-cased, accent-folded tokens. Rank: exact label prefix > word prefix > synonym > fuzzy (edit distance 1 for 4–7 letters, 2 for 8+) > hint text. Ties break by page visit count kept locally.
- **Synonyms.** Hand-written per control ("wifi, wireless, WLAN", "resolution, screen size", "ASLR, address randomisation"). Kept in the string tables so translators add local words.
- **Hardware filter.** Results for missing hardware (no battery, no Bluetooth) are hidden unless the query names them; then they show with "not present".
- **Inline change.** For toggles and choice lists `Ctrl+Enter` shows the control in the result row.
- **Other apps.** Pages owned by other apps (Firewall, Permissions, Software) contribute index records through their manifest; the result opens that app through `svc.intent`.
- **System-wide.** The same index is given to `svc.search` so SH01 finds settings.
- **Privacy.** Queries are not stored. Only visit counts per page are kept, and "Clear history" removes them.
- **Budget.** About 4,000 records; a query answers in under 5 ms; the list updates on every keystroke.

#### 5.3.2 Wi-Fi page

```
+-----------------------------------------------------------------------------------+
| Network › Wi-Fi                                         Wi-Fi [ on ]   [?]        |
+-----------------------------------------------------------------------------------+
| Connected                                                                         |
|  > home-5g        ▂▄▆█  WPA3   5 GHz ch 44   192.168.1.23    [Details] [Disconnect]|
| Available                                                    [Rescan]             |
|    library        ▂▄▆_  WPA2                                           [Join]     |
|    cafe-open      ▂▄__  Open — not encrypted                           [Join]     |
|    corp           ▂▄▆_  Enterprise                                     [Join]     |
| Saved (3)                                                    [Manage saved…]      |
|                                                                                   |
| MAC address       [Random per network v]                                          |
| [ ] Join known networks automatically     [Join hidden network…]                  |
+-----------------------------------------------------------------------------------+
```

`modelListView` of networks, sorted by signal, refreshed every 10 s while the page is visible and never in the background. Open networks carry the words "not encrypted" in `--warn`. States: radio off (one line and a switch), no adapter, aeroplane mode, scanning.

#### 5.3.3 Edit connection dialog

```
+---------------------------- Edit "home-5g" -----------------------------+
| [General] [IPv4] [IPv6] [DNS] [Proxy] [Security] [802.1X]               |
|-------------------------------------------------------------------------|
| Method      (•) Automatic (DHCP)   ( ) Manual   ( ) Off                 |
| Address     [192.168.1.23    ] / [24]                                   |
| Gateway     [192.168.1.1     ]                                          |
| Routes      +----------------------+---------------+--------+           |
|             | Destination          | Gateway       | Metric | [+] [-]   |
|             +----------------------+---------------+--------+           |
| DHCP client id  [none sent       v]                                     |
| [ ] Metered connection                                                  |
|-------------------------------------------------------------------------|
| Changes apply to all users.                    [Cancel]  [Save changes] |
+-------------------------------------------------------------------------+
```

`dialog` with `tabWidget`. Addresses validate on blur. Saving a change to the active link triggers the 15 s keep-or-revert bar.

#### 5.3.4 Shortcuts editor

```
+-----------------------------------------------------------------------------------+
| Input › Shortcuts            [ Search actions or press keys…  ]  [Reset all][Export]|
+-----------------------------------------------------------------------------------+
| Scope  [All v]     Show [ ] only changed  [ ] only conflicts                      |
| Action                         | Shortcut        | Scope            |             |
| v System                                                                          |
|    Lock the screen             | Super+L         | global           |             |
|  > Open launcher               | [ press keys… ] | global           | recording   |
|    Take a screenshot           | Print           | global           |             |
| v Files                                                                           |
|    New tab                     | Ctrl+T          | app              | ● modified  |
|    Open terminal here          | Ctrl+Alt+T  ⚠   | app              | conflict    |
+-----------------------------------------------------------------------------------+
| Conflict: Ctrl+Alt+T is also "Open Terminal" (global). [Keep both scopes] [Change] |
+-----------------------------------------------------------------------------------+
```

`treeView` in table mode over the shared action table (PLATFORM.md section 5). Recording grabs keys through a shell-granted, visible grab; `Esc` cancels, `Backspace` clears.

#### 5.3.5 Privacy overview

```
+-----------------------------------------------------------------------------------+
| Privacy › Overview                                             [Open Permissions] |
+-----------------------------------------------------------------------------------+
| Capability      | Granted | In use now       | Last used          | System switch |
| Files (portal)  | 31 apps | Files, Writer    | now                |      —        |
| Network         |  9 apps | Browser          | now                |      —        |
| Camera          |  2 apps | —                | 3 days ago, Camera |    [ on ]     |
| Microphone      |  3 apps | Messenger  ●     | now                |    [ on ]     |
| Screen          |  1 app  | —                | never              |      —        |
| Location        |  0 apps | —                | never              |    [ off]     |
| USB devices     |  2 apps | —                | yesterday          |      —        |
| Local AI        |  4 apps | —                | 2 h ago, Assistant |    [ on ]     |
| Background run  |  5 apps | 3 running        | now                |      —        |
+-----------------------------------------------------------------------------------+
| Nothing on this machine reports usage data to anyone.                             |
+-----------------------------------------------------------------------------------+
```

`tableView`, read-only except the switches. Activating a row opens AD05 filtered to that capability. Data comes from `svc.audit` and the permission store, read-only.

#### 5.3.6 Hardening page

```
+----------------------------------------------------------------------------------------+
| Security › Hardening          Profile [Standard v]   [Compare…] [Export report] 🔒      |
+----------------------------------------------------------------------------------------+
| 19 knobs · 17 match · 1 drift · 1 not compiled in        Filter [All v] [ search… ]    |
| Knob                                   | Kind    | Default | Running | Profile | Status|
| hardening.pax.aslr.status              | 4-state | opt-out | opt-out | opt-out | ✓ ok  |
| hardening.pax.mprotect.status          | 4-state | opt-out | opt-out | opt-out | ✓ ok  |
| hardening.pax.pageexec.status          | 4-state | opt-out | opt-out | opt-out | ✓ ok  |
| hardening.pax.segvguard.status         | 4-state | opt-out | opt-out | opt-out | ✓ ok  |
|>hardening.pax.tpe.status               | 4-state | opt-in  | opt-in  | opt-in  | ✓ ok  |
| hardening.elf_pie_only                 | 2-state | off     | off     | off     | ✓ ok  |
| hardening.harden_tty                   | 2-state | on      | off     | on      | ⚠ drift|
| hardening.pax.aslr.compat.status       | 4-state | ?       | absent  | —       | n/a   |
+----------------------------------------------------------------------------------------+
| hardening.pax.tpe.status — Trusted Path Execution                                      |
| Opt-in: off for every program that does not ask. Strict sets opt-out: programs run     |
| only from trusted directories. [Preview impact…]              Value [opt-in v] [Stage] |
+----------------------------------------------------------------------------------------+
| 1 staged change · needs restart · will be applied in a new boot environment [Apply…]   |
+----------------------------------------------------------------------------------------+
```

`tableView` plus a detail `panel`. Four states are always written as words (disabled, opt-in, opt-out, forced), never as digits alone. "Default" is the compiled-in default for the kernel configuration in use; a knob whose default cannot be decided shows `?`, as the source tool does. Status values: ok, drift (running weaker than profile), tightened (running stronger), n/a (not compiled in).

#### 5.3.7 Hardening impact dialog

```
+--------------------- Apply "Strict" hardening profile? ----------------------+
| 3 knobs change. A restart is needed.                                         |
|  hardening.elf_pie_only      off     → on                                    |
|  hardening.pax.tpe.status    opt-in  → opt-out                               |
|  hardening.kmalloc_zero      off     → on                                    |
|                                                                              |
| What stops working (scan of installed software, 2 min ago):                  |
|  ✕ 4 programs are not position-independent        [Show list]                |
|  ✕ 2 programs run from untrusted directories      [Show list]                |
|  ✓ All system components are compatible                                      |
|                                                                              |
| A new boot environment "hardening-2026-11-12" is created first.              |
| You can return to the current one from the boot menu or from Software.       |
|                                               [Cancel]  [Create and restart] |
+------------------------------------------------------------------------------+
```

#### 5.3.8 Add user wizard

```
+------------------------------ Add a user (2 of 4) ------------------------------+
| Full name      [Ada Example            ]                                        |
| Login name     [ada         ]   lower-case letters, digits, 1–16                |
| Role           (•) Standard   ( ) Administrator                                 |
| Home           [x] Own encrypted dataset   zroot/home/ada                       |
| Sign-in        (•) Password  ( ) Hardware token  ( ) Set at first login         |
| Password       [••••••••••••      ] [reveal]   Strength: good                   |
| Again          [••••••••••••      ]                                             |
|                                                    [Back]  [Cancel]  [Next]     |
+---------------------------------------------------------------------------------+
```

Steps: identity, sign-in, groups and limits, review. Secret fields follow the design-system pattern.

#### 5.3.9 Import settings (diff)

```
+----------------------- Import "desk.pbsettings" ------------------------+
| Signed by: this device (key 7f3a…c2)  ✓ verified     Schema 3           |
| [x] Appearance (6 keys)   [x] Input › Shortcuts (41)   [ ] Network (2)  |
| Key                          | Current        | Incoming                |
| appearance.theme             | dark           | light                   |
| appearance.accent            | #4C8DFF        | #3FB27F                 |
| input.keyboard.repeat_ms     | 250            | 200                     |
|                                    [Cancel]  [Import 47 settings]       |
+-------------------------------------------------------------------------+
```

### 5.4 Menus

```
[=] menu / F10
├─ File
│   ├─ Export settings…                 Ctrl+E
│   ├─ Import settings…                 Ctrl+O
│   ├─ Export hardening report…
│   └─ Quit                             Ctrl+Q
├─ Edit
│   ├─ Undo change                      Ctrl+Z
│   ├─ Redo change                      Ctrl+Shift+Z
│   ├─ Find a setting                   Ctrl+F
│   ├─ Reset this page…                 Ctrl+Shift+R
│   └─ Reset all my settings…
├─ View
│   ├─ Home                             Alt+Home
│   ├─ Back / Forward                   Alt+Left / Alt+Right
│   ├─ Sidebar                          F9
│   ├─ Show only changed settings       Ctrl+M
│   ├─ History of changes…              Ctrl+H
│   └─ Pin this page                    Ctrl+D
├─ Go
│   ├─ Network                          Ctrl+1
│   ├─ Display                          Ctrl+2
│   ├─ Sound                            Ctrl+3
│   ├─ Appearance                       Ctrl+4
│   ├─ Privacy                          Ctrl+5
│   ├─ Hardening                        Ctrl+6
│   └─ About                            Ctrl+7
└─ Help
    ├─ Help for this page               F1
    ├─ Keyboard shortcuts               Ctrl+/
    └─ About Settings
```

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| `Ctrl+F`, `Ctrl+K`, `/` | Focus search |
| `Ctrl+Enter` (in results) | Change the control inline |
| `Alt+Up` | Parent page |
| `Ctrl+L` | Unlock system-scope controls |
| `Ctrl+Shift+C` | Copy the deep link of the focused control |
| `Enter` / `Esc` on the keep bar | Keep / revert |
| Arrow keys on the arrangement canvas | Move the selected output 10 px; `Shift` 1 px |

Context menu on any control: Reset to default · History of this setting… · Copy link · Copy value · Help. Context menu on a sidebar page: Pin · Open in new window · Reset page.

Drag and drop: outputs on the arrangement canvas; rows to reorder layouts, saved networks, languages and DNS servers; an image file onto the Wallpaper page; a `.conf`/`.ovpn` file onto the VPN page (opens the import dialog); a `.pbsettings` file anywhere (opens import); an `.icc` file onto Colour.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Lists with nothing (no VPN, no saved networks) show one line and one button. |
| Loading | Skeleton rows; controls disabled until the service answers. |
| Error | Inline `--danger` line under the control with the service error and Retry. The control returns to its last confirmed value. |
| Offline | Normal: no page needs the internet. NTP and DNS test show "no network". |
| Locked | System-scope controls dimmed until authentication; policy-pinned controls locked permanently. |
| Read-only | Guest session: all system pages read-only, user pages non-persistent. |
| Hardware absent | Page hidden from the tree; reachable by search with a "not present" note. |
| Narrow (< 640) | Sidebar becomes a full-width list; choosing a page pushes the content; back arrow in the header. Tables collapse to two-line rows. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar | `treeView` |
| Page body | `scrollArea` with `panel` groups |
| Switches, choices, ranges | `toggleSwitch`, `checkBox`, `radioButton`, `comboBox`, `slider`, `spinBox`, `lineEdit` |
| Tables (hardening, privacy, shortcuts, routes) | `tableView`, `treeView` |
| Network and device lists | `modelListView` |
| Wallpaper and theme pickers | `gridView`, `image`, `colorPicker`, `fontPicker` |
| Date and time | `calendar`, `dateTimeEdit` |
| Pressure curve | `curveEditor` |
| Battery and level graphs | `chart`, `progressBar` |
| Help text and licences | `markdownView` |
| Dialogs and wizards | `dialog`, `tabWidget`, `fileDialog` (through the portal) |

Missing:

- `SY01-GAP-1` Display arrangement canvas (scaled rectangles, snapping, keyboard move). Can be built on the 2D canvas of G-12 or as a small custom widget.
- `SY01-GAP-2` Key-capture field (records a chord with a visible, shell-granted grab).
- `SY01-GAP-3` Time-zone map picker (static vector map, hit-testing); the list picker works without it.
- `SY01-GAP-4` Live level meter with peak hold (a `progressBar` variant updated at 30 Hz from shared memory).
- G-01 for output configuration and fractional scale; G-02 for all pages; G-03 for language pages.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Settings app | ForgeScript | app host | Page loading, view models, search, undo stack, deep links |
| Page documents | FUSE UI | app host | One `.fuseui` per page; bindings to setting keys |
| Search index builder | ForgeScript (build tool) | build time | Emits `search-index.tbl` from pages and schemas |
| Configuration broker | C++23 | system service, privileged, one per machine | The only writer of system configuration. Offers typed interfaces per domain; checks policy and authentication; writes audit records. Proposed. |
| Network configuration service | C++23 | system service | Interfaces, Wi-Fi, VPN, DNS, proxies. Proposed. |
| Output configuration interface | C++23 | compositor | Modes, scale, arrangement, night light, colour. Proposed. |
| Audio, Bluetooth, input, accounts-and-users, time services | C++23 | system services | One per domain. Proposed. |
| Hardening helper | C++23 | helper, Capsicum, read-only handles | Reads `hardening.*` through a sysctl handle, reads the compiled-default table, scans ELF headers of installed binaries for the impact preview. Never writes. |
| Settings import helper | C++23 | helper, Capsicum, no handles but the file | Parses `.pbsettings`, `.conf`, `.ovpn`, `.icc` and returns validated records |

```
                 +--------------------------- app host ---------------------------+
 settings: URL → | ForgeScript: pages, search, undo  ── FUSE UI pages             |
                 +----+----------------+------------------+-----------------------+
                      | svc.settings   | session bus      | proc.helper
                      v                v                  v
              settings store    configuration broker   hardening helper (read-only)
              (user keys)        │  authenticates via   import helper (parse only)
                                 │  SH03, audits
        +------------+-----------+-----------+-------------+-----------+
        v            v           v           v             v           v
     network     compositor    audio     bluetooth     users/time   svc.update
     service     outputs       service   service       services     (boot env for
                                                                     hardening)
```

Rule: the app holds no privilege. Every system change is a typed request to the broker, which may ask the shell to authenticate the user. The app cannot write a sysctl, a file under `/etc`, or a pf rule.

### 6.2 Data model

| Entity | Fields | Store |
|---|---|---|
| `SettingKey` | `id: string`, `type: bool\|int\|float\|string\|enum\|list\|record`, `default`, `scope: user\|system`, `risk: none\|revert\|stage\|reboot`, `secret: bool`, `schema_version: u32` | schema files in each bundle, compiled into `svc.settings` |
| `SettingValue` | `key`, `value`, `modified_at: i64`, `modified_by: app_id`, `policy_locked: bool` | `svc.settings` |
| `ChangeRecord` | `seq: u64`, `key`, `old`, `new`, `at`, `uid`, `source` | `svc.settings` history, `svc.audit` for system scope |
| `PageDescriptor` | `id`, `title_key`, `parent`, `icon`, `fuseui`, `needs_hardware: string[]`, `scope` | bundle |
| `SearchRecord` | `id`, `path`, `label_key`, `hint_key`, `synonyms_key`, `kind: page\|group\|control\|option\|help\|external`, `target_app` | `search-index.tbl` in the bundle |
| `ConnectionProfile` | `uuid`, `name`, `type: ethernet\|wifi\|vpn\|bridge\|vlan`, `ipv4`, `ipv6`, `dns`, `proxy`, `mac_policy`, `metered`, `autoconnect`, `secret_ref` | network service; secrets in `svc.keyring` |
| `DisplayProfile` | `name`, `edid_hashes: string[]`, `outputs: {mode, refresh_mhz, scale, x, y, rotation, primary, icc_ref, hdr}[]` | `~/.config/org.pbsd.settings/displays.json` via `svc.settings` |
| `HardeningKnob` | `path`, `kind: 2state\|4state\|int`, `compiled_default`, `running`, `profile_value`, `status`, `needs_reboot: bool` | helper (live), profile files in the system bundle |
| `HardeningProfile` | `name`, `values: map<path,int>`, `exemptions: {binary_digest, path, feature, reason, added_by, added_at}[]`, `signature` | system configuration dataset, read through the broker |
| `UiState` | `pinned: string[]`, `recent: string[]`, `visit_counts: map<string,u32>`, window geometry | `~/.config/org.pbsd.settings/ui.json` |

`.pbsettings` format: UTF-8 JSON, `{"format":"pbsettings","schema":3,"created":"…","device":"…","pages":{"<page id>":{"<key>":value}},"secrets":null|{"kdf":"argon2id","cipher":"xchacha20-poly1305","data":"…"}}` with a detached Ed25519 signature. Unknown keys are listed and skipped on import. Migration: each schema carries `schema_version`; `svc.settings` migrates forward and keeps the prior file.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| `.pbsettings` JSON + `.sig` | yes | yes | own code (FORGE JSON, Ed25519 as V14) | import helper |
| WireGuard `.conf` | yes | no | own code | import helper |
| OpenVPN `.ovpn` | yes | no | own code; inline scripts and `up`/`down` directives are rejected | import helper |
| ICC profile | yes (header and tags for display) | no | own code | import helper |
| EDID | yes | no | own code | compositor (already parsed there); the app gets fields only |
| Wallpaper images | yes | no | `svc.thumbnail` | thumbnail sandbox |
| `sysctl hardening` values | yes | no | sysctl handle | hardening helper |
| ELF headers (impact scan) | yes | no | own code, header only | hardening helper |
| Proxy auto-config | URL stored only | no | not parsed here; the network service evaluates a restricted form | network service sandbox |
| Session bus records | yes | yes | schema compiler (G-08) | app host |

### 6.4 Integration

- **Intents handled**: `open` for `settings:` URLs and for `.pbsettings`, `.conf`, `.ovpn`, `.icc` files. **Emitted**: `open` to AD05, SE02, SE03, SE04, SY05, AD02, AD03, AD04, UT03, UT04, UT05, UT06, VM04, VM05.
- **URL scheme**: `settings://<category>/<page>#<control>`.
- **MIME**: `application/x-pbsd-settings+json` (view, import, export).
- **Services used**: `svc.settings`, `svc.notify`, `svc.intent`, `svc.search` (contributes the index), `svc.keyring` (VPN and Wi-Fi secrets by reference), `svc.thumbnail`, `svc.power`, `svc.media`, `svc.update` (boot environment for hardening), `svc.audit` (read for Privacy overview), `svc.a11y`, `svc.portal.file`.
- **Embedded by**: AD01 Installer (F145), SH02 (opens pages).
- **Backing repo facts (Verified)**: `tools/hardening_sysctls.py` extracts each `hardening.*` knob and its compiled-in default from kernel source, resolves `#ifdef` guards against a named kernel configuration, and compares a `sysctl hardening` dump with `--check`. Run here with `--kernconf HARDENEDBSD` it lists 19 knobs, among them `hardening.pax.aslr.status` (opt-out), `hardening.pax.tpe.status` (opt-in) and `hardening.elf_pie_only` (disabled). `docs/ROADMAP.md` section 15 records that asserting these against a booted kernel, ratcheting them, and documenting which are PBSD policy are all still open.
- **Proposed**: the compiled-default table ships as a signed data file generated by that tool at image build; the Strict profile values; the broker and all per-domain services.

## 7. Security and privacy

Capabilities requested:

| Capability | Reason |
|---|---|
| `fs.app` | UI state, display profiles |
| `fs.portal` | Pick a wallpaper, a VPN file, an ICC profile, an export target |
| `net.none` | The app never opens a socket. Network tests are run by the network service. |
| `proc.helper:hardening-helper` | Read knobs and scan binaries, read-only |
| `proc.helper:import-helper` | Parse untrusted import files |
| `svc.settings`, `svc.notify`, `svc.intent`, `svc.search`, `svc.keyring`, `svc.thumbnail`, `svc.power`, `svc.media`, `svc.update`, `svc.audit`, `svc.a11y`, `svc.portal.file` | As listed in 6.4 |
| `svc.config` (new, Proposed) | Talk to the configuration broker and its domain services |

Not requested: `proc.exec`, any `dev.*`, `ai.local`, `bg.run`, `bg.autostart`, any `fs.path`.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| System configuration | A compromised app | Sends `settings:` links or bus messages to weaken settings | Deep links only navigate; they never change a value. The broker accepts system writes only from this bundle id (peer credentials + bundle signature) and only after user authentication. |
| Hardening state | Malware with user rights | Lowers a knob silently | Knob writes go through the broker with authentication; all are audited; drift alarm (F131); many knobs can only tighten after boot. |
| User | Imported VPN or settings file | Parser bug, or a profile with a script hook or a hostile DNS | Parsed in a Capsicum helper with one read handle; script directives rejected; diff shown before apply; signature state displayed. |
| Credentials | Shoulder surfing, clipboard | Reads Wi-Fi passphrase | Secret fields; QR share needs authentication; clipboard entries flagged sensitive. |
| User | Lock-out by a bad change | Wrong resolution, address or profile | Timed revert, boot environment, safe mode banner. |
| User | UI spoofing | An app draws a fake authentication prompt | Prompts are drawn by the shell (SH03), never by this app. |
| Privacy | The app itself | Connectivity checks, time-zone lookup, telemetry | None exist by default; `net.none`; location is manual. |
| Integrity of pages | Third-party settings page | Reaches other apps' keys | An app page gets only its own namespace handle. |

Stored: UI state and display profiles in the app's config directory. Secrets never; only keyring references. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `settings.start_page` | enum(`home`,`last`) | `home` | Page shown at start |
| `settings.show_modified_dots` | bool | true | Mark non-default values |
| `settings.revert_timeout_s` | int 5–60 | 15 | Keep-or-revert countdown |
| `settings.unlock_grace_min` | int 0–30 | 5 | How long an unlock lasts |
| `settings.search.include_help` | bool | true | Help pages in results |
| `settings.search.keep_visit_counts` | bool | true | Rank by visits |
| `settings.show_absent_hardware` | bool | false | Show pages for missing hardware |
| `settings.history.keep_days` | int 0–3650 | 365 | Change history retention |
| `settings.hardening.show_digits` | bool | false | Show numeric values beside state words |
| `settings.hardening.rescan_on_open` | bool | false | Re-run the binary scan when the page opens |
| `settings.confirm_reset` | bool | true | Ask before a page reset |
| `settings.pinned` | list<string> | [] | Pinned pages |

## 9. Accessibility and languages

- The arrangement canvas has a table alternative: each output as a row with X and Y spin boxes.
- The keep-or-revert bar is a live region and announces the remaining seconds every 5 s; the timeout is doubled when a screen reader is running.
- Four-state values are spoken as words. Status cells speak "drift: running value is weaker than the profile".
- Language changes apply to this app at once so the user can find their way back; the search index always includes English.
- The time-zone picker is fully usable from the list; the map is decoration for pointer users.
- Sliders show and accept exact numbers.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | 150 ms |
| Page switch | 50 ms to first frame; services may fill in within 300 ms |
| Search query | 5 ms over 4,000 records |
| Memory | 60 MiB resident with all pages visited |
| Idle | 0 frames, 0 wake-ups; live meters run only while their page is visible |
| Hardening table refresh | 20 ms for all knobs |
| Binary impact scan | 20,000 ELF headers in under 10 s, cancellable, cached by file digest |

## 11. Testing and proof

Test plan (targets; nothing has run):

- Unit: key validation, undo stack, search ranking, deep-link parsing, four-state mapping.
- Golden renders: every page in dark, light, high contrast, at 100% and 200%, in a left-to-right and a right-to-left language.
- Scripted UI through the a11y tree: all 15 top tasks against fake services.
- Fuzz: `.pbsettings`, `.conf`, `.ovpn`, ICC and ELF-header parsers.
- Differential: the hardening helper's table against `tools/hardening_sysctls.py --kernconf` output for the same kernel configuration; the running values against `sysctl hardening` on a booted image.
- Fault injection: each service absent, slow, or refusing; revert timer under load.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Import helper parsers | No out-of-bounds, bounded memory, total on all inputs | T2 | PRISM absence-of-runtime-error job plus fuzzing | Full grammar specification for T1 |
| Hardening status mapping | Status is `drift` exactly when running < profile in the knob's order | T1 | Small pure function; PRISM proof | — |
| Broker request validation | A system write is accepted only with a valid authentication token and caller id | T2 | PRISM plus model of the token state machine | Kernel and bus are outside the proof |
| Timed revert | The old value is restored if no confirmation arrives in time, including after an app crash | T2 | State-machine model; fault-injection tests | Depends on service behaviour |
| ELF header scan | Reads only headers; never follows offsets outside the file | T2 | PRISM plus fuzz | — |
| ForgeScript page logic | Memory safety by the language (V10, V11) | T3 | Unit tests, static checks | Interpreter itself is not proved |
| Search ranking | No property beyond tests | T4 | Unit tests | Not security relevant |

## 12. Build plan

Phase 1, usable core:

| ID | Work item | Depends on |
|---|---|---|
| SY01-W01 | App skeleton: page tree, navigation, deep links, status bar | G-02, G-07 |
| SY01-W02 | Setting-key schema format and binding of controls to `svc.settings` | G-09 |
| SY01-W03 | Search index builder and search overlay | SY01-W01 |
| SY01-W04 | Configuration broker interface definitions (network, outputs, audio, users, time) | G-08, G-09 |
| SY01-W05 | Network pages: connections, Wi-Fi, Ethernet, IPv4/6, DNS, hostname | SY01-W04 |
| SY01-W06 | Display pages: arrangement, mode, scale, rotation, night light; timed revert | G-01, SY01-GAP-1 |
| SY01-W07 | Sound output and input pages | SY01-W04, SY01-GAP-4 |
| SY01-W08 | Power, keyboard, mouse, touchpad pages; shortcuts editor | SY01-GAP-2, FUSE-WP-14 |
| SY01-W09 | Appearance, notifications, default apps | SH01, SH02 |
| SY01-W10 | Users, date and time, language, formats | SH03 |
| SY01-W11 | Accessibility pages and global switches | AX01 |
| SY01-W12 | Privacy overview and device switches | AD05, SH04 |
| SY01-W13 | Hardening status table (read-only), hardening helper, compiled-default table | G-06 |
| SY01-W14 | About, storage, updates summaries; audit of changes | SY05 |
| SY01-W15 | Pages embeddable by AD01 | AD01 |

Phase 2, full feature:

| ID | Work item | Depends on |
|---|---|---|
| SY01-W16 | VPN, proxies, metered, diagnostics | SE02, SE04 |
| SY01-W17 | Bluetooth page | VM05 |
| SY01-W18 | Display profiles, colour profiles, brightness, lid rules | FUSE-WP-11 |
| SY01-W19 | Per-app volume, sound profiles, system sounds | SY01-W07 |
| SY01-W20 | Tablets, touchscreen, input trust, input methods page | AX04, VM05 |
| SY01-W21 | Search, multitasking, startup apps, removable media pages | SH05 |
| SY01-W22 | Groups, login options, authentication methods | SE04 |
| SY01-W23 | Hardening profiles, impact scan, staged apply through boot environments, drift alarm, report | SY05, SE01 |
| SY01-W24 | Export, import, history, reset, locked settings, app-provided pages | SY01-W02 |
| SY01-W25 | Remaining summaries and licences | UT06, AD03 |

Phase 3, advanced:

| ID | Work item | Depends on |
|---|---|---|
| SY01-W26 | HDR, overscan, text rendering options | FUSE-WP-11 |
| SY01-W27 | Game controllers page | — |
| SY01-W28 | Hotspot, bonds, bridges, hosts entries | SY01-W16 |
| SY01-W29 | Per-binary exemptions, managed configuration, session limits | AD05 |
| SY01-W30 | Remote and sharing, scheduled power, copy-as-command | VM04 |

## 13. Open questions and risks

- The configuration broker and the per-domain services do not exist and are not in PLATFORM.md section 8. They are the largest dependency of this app.
- Which `hardening.*` knobs may be changed at run time, which only tighten, and which need a reboot is not established from the sources read. It must be read from the kernel before F128–F130 are final.
- The Strict profile's values are a proposal. `docs/ROADMAP.md` notes that `elf_pie_only` stops every non-PIE binary and that TPE is compiled in but inert; turning either on needs a tested software inventory.
- Wi-Fi, Bluetooth and VPN back ends on HardenedBSD (wpa_supplicant-class functions, a Bluetooth stack) are not chosen. If third-party code is used it lives in the service, not here.
- Whether AX and SH apps' option pages are hosted here as embedded pages or only linked needs one rule across the suite.
- Fractional scale and HDR depend on the compositor (G-01, FUSE-WP-11).
- NTS-capable time client: not identified in the tree.
