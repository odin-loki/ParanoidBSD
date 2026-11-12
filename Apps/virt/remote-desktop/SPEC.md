<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Remote Desktop

By Odin Loch

| | |
|---|---|
| ID | `VM04` |
| Bundle | `org.pbsd.remote-desktop` |
| Category | virt |
| Priority | P2 |
| Backing repos | none |
| Status | Planned. Nothing built. |

## 1. Purpose

Remote Desktop connects to other computers (RDP, VNC, SPICE, and SSH by hand-off to the terminal) and lets this computer be reached from elsewhere, either by sharing the running session or by serving a headless one. It is for administrators, people who work on a second machine, and people who give or receive help. It replaces Remmina, the KDE remote-desktop client and server, TigerVNC viewers and xrdp.

Every connection is an explicit grant: which host, which route, what may cross (clipboard, files, devices, sound). Nothing listens unless the user turned sharing on, and the screen shows it while it is shared.

## 2. Scope

### In scope

- Connection manager: saved connections, folders, quick connect, import and export.
- Client sessions: RDP, VNC (RFB), SPICE (optional), SSH (hand-off to SY03).
- Tunnels: SSH jump hosts, VPN and Tor routes from SE03, RD Gateway.
- Trust: certificate and host-key pinning, credentials and keys from SE04.
- Per-connection redirection policy: clipboard, files, printers, audio, microphone, USB, smart cards.
- Multi-monitor, scaling, dynamic resolution, full screen.
- Session recording and an audit trail.
- Server: share this session (view or control) and serve headless sessions over RDP and VNC.
- Unattended access rules.

### Out of scope

| Item | Owner |
|---|---|
| The terminal emulator and interactive SSH shells | SY03 (VM04 stores the host entry and hands off) |
| File transfer queues over SFTP, FTP, WebDAV | IN03 and SY02 remote locations |
| Consoles of local VMs | VM01 |
| VPN, Tor and tunnel set-up | SE03 (VM04 selects a route) |
| Firewall rules (VM04 requests a rule for its listener) | SE02 |
| Passwords, SSH keys, certificates, hardware tokens | SE04 |
| Screen recording of the local desktop in general | ME05 |
| Video calls and screen sharing inside chats | CO05 |
| Login greeter, lock screen and the remote login prompt | SH03 |
| Rendezvous or relay servers on the internet | not planned; no vendor service |

## 3. Users and top tasks

Personas:

- **Sam, administrator.** Keeps 40 saved hosts in folders; uses jump hosts; needs fast reconnect and safe clipboard rules.
- **Robin, office worker.** Connects to one work desktop over a VPN with two monitors.
- **Dara, developer.** Reaches a headless PBSD build box and VM consoles on other hosts.
- **Alex, helper.** Views a relative's screen after they share it for 30 minutes.
- **Noor, privacy user.** Reaches a home machine only through a Tor onion address with a pinned key.

| # | Task | Target |
|---|---|---|
| T1 | Connect to a saved host | 1 step: double click or `Enter` |
| T2 | Quick connect to a new host | 2 steps: `Ctrl+L`, type `rdp://host`, `Enter` (trust prompt on first contact) |
| T3 | Send Ctrl+Alt+Del | 1 step: `Host+Del` |
| T4 | Switch between window, full screen and all monitors | 1 step each: `Host+F`, `Host+M` |
| T5 | Paste text into the remote session where clipboard is off | 1 step: `Host+V` (one-shot) |
| T6 | Give the remote session one local folder | 2 steps: Session menu, "Share a folder…" (portal) |
| T7 | Connect through a jump host | 0 extra steps once saved in the connection |
| T8 | Share this screen for help | 3 steps: "Share this screen", choose view or control and duration, give the code |
| T9 | Stop sharing at once | 1 step: `Super+Shift+Esc` or the indicator's Stop |
| T10 | Allow unattended access from one key only at night | 4 steps in the Server page rule editor |
| T11 | Import connections from another client | 2 steps: Import, pick files |
| T12 | Record a session | 1 step: `Host+R` (indicator shown) |
| T13 | See what a connection may cross | 1 step: `F4` |

## 4. Features

No backing repository exists for this app. Everything is Proposed.

### 4.1 Connection manager

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM04-F001 | Saved connections | List or grid: name, protocol icon, address, route badge, trust state, last used, tags | M | 1 |
| VM04-F002 | Folders | Nested folders in the sidebar; drag to move; settings can be inherited from a folder (credentials reference, jump host, route, redirection policy) | M | 1 |
| VM04-F003 | Quick connect | `Ctrl+L` field accepts `rdp://`, `vnc://`, `spice://`, `ssh://` URLs, `host:port`, or a saved name; history kept locally and clearable | M | 1 |
| VM04-F004 | Connection editor | Pages: General, Display, Security, Redirection, Route, Advanced; one page per protocol where needed | M | 1 |
| VM04-F005 | Search | By name, address, tag, folder, protocol, user | M | 1 |
| VM04-F006 | Tags and colours | Tags; an optional colour band shown on the session window (for example red for production) | S | 2 |
| VM04-F007 | Duplicate and templates | Duplicate a connection; save one as a template for new connections | S | 2 |
| VM04-F008 | Import | `.rdp` files, `.vnc` files, Remmina `.remmina` files, `.vv` (SPICE) files, OpenSSH `config` Host blocks, CSV; passwords found in files are moved to SE04 and removed from the copy | M | 2 |
| VM04-F009 | Export | `.rdp`, `.vnc`, native TOML bundle; never includes secrets; optional signature | S | 2 |
| VM04-F010 | Signed `.rdp` files | Verify the signature block of signed `.rdp` files; unsigned files opened from outside show every setting that redirects local resources before connecting | M | 2 |
| VM04-F011 | Recent and favourites | Recent sessions list; favourites pinned to the top; shell jump list (SH01) | S | 2 |
| VM04-F012 | Status probe | Optional reachability check (TCP connect to the service port through the connection's route) for selected hosts on demand; never in the background by default | S | 2 |
| VM04-F013 | Wake-on-LAN | Send a magic packet (MAC and broadcast address stored) before connecting; optional wait | S | 3 |
| VM04-F014 | Local discovery | List RDP and VNC services announced on the local link; off by default because it sends multicast queries | C | 3 |
| VM04-F015 | Bulk edit | Change route, folder, tags or policy on a selection | S | 3 |
| VM04-F016 | Notes | Markdown note per connection | C | 2 |
| VM04-F017 | System search | Connection names contributed to `svc.search`; result action is Connect | C | 3 |

### 4.2 Session window and display

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM04-F018 | Session tabs | Sessions open as tabs in a session window; tabs can be torn off into their own window and split side by side | M | 1 |
| VM04-F019 | Scaling | 1:1 with scroll, fit, fill, integer; nearest or linear filter; per connection | M | 1 |
| VM04-F020 | Dynamic resolution | Remote desktop size follows the window where the protocol allows (RDP display control channel, VNC extended desktop size, SPICE agent) | M | 2 |
| VM04-F021 | Fixed resolution | Choose a size or "this monitor" | M | 1 |
| VM04-F022 | Full screen | `Host+F`; top-edge pull-down bar with name, trust badge, Host key hint, Leave | M | 1 |
| VM04-F023 | Multi-monitor | Use all or chosen local monitors as separate remote monitors (RDP monitor layout, SPICE heads) or span one large desktop (VNC) | M | 2 |
| VM04-F024 | HiDPI | Send the local scale factor to the remote where supported (RDP desktop and device scale factors); otherwise local scaling | S | 2 |
| VM04-F025 | Colour depth and quality | Depth 16/24/32; quality presets (LAN, broadband, slow link) that set codec and compression choices | M | 1 |
| VM04-F026 | View only | A switch that drops all input to the remote; shown in the status bar | M | 1 |
| VM04-F027 | Screenshot | `Host+S` saves or copies the remote frame; provenance tag names the host | S | 1 |
| VM04-F028 | Zoom and pan | `Ctrl+scroll` zoom in 1:1 mode; minimap popover for large desktops | C | 3 |
| VM04-F029 | Connection bar | Per-session toolbar: disconnect, send keys, clipboard state, redirections, record, statistics | M | 1 |
| VM04-F030 | Statistics | Round-trip time, bandwidth, frames per second, codec in use, packet loss if known; small overlay or popover | S | 2 |
| VM04-F031 | Reconnect | Automatic reconnect with back-off (1, 2, 5, 10, 30 s, then stop) after a network drop; the last frame stays, dimmed, with a banner | M | 1 |
| VM04-F032 | Idle | No redraw when no update arrives; hidden tabs ask the server to pause updates where the protocol allows | M | 1 |
| VM04-F033 | Colour band and title | Window title "name — host"; optional colour band from the tag | S | 2 |

### 4.3 Input

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM04-F034 | Keyboard grab | On focus the session takes shortcuts through the compositor's shortcut-inhibit; `Host` (default Right Ctrl) releases; `Super+L` and the stop-sharing chord are never forwarded | M | 1 |
| VM04-F035 | Send keys | Ctrl+Alt+Del, Ctrl+Alt+End, Alt+Tab, Super, Print Screen, Ctrl+Alt+F1–F12, custom chords | M | 1 |
| VM04-F036 | Keyboard mapping | Scancode mode (physical keys) or keysym/Unicode mode; per connection; a layout hint is sent to the remote | M | 1 |
| VM04-F037 | Input methods | Composed text from the local input method is sent as Unicode input events where the protocol allows | S | 2 |
| VM04-F038 | Pointer | Absolute pointer; relative mode with lock on request; local cursor drawn from the remote shape | M | 1 |
| VM04-F039 | Scroll and gestures | Smooth scroll mapped to wheel steps; horizontal scroll; pinch mapped to Ctrl+wheel (optional) | S | 2 |
| VM04-F040 | Touch and pen | Forwarded as RDP touch and pen input where the server supports it | C | 3 |
| VM04-F041 | Type clipboard | Type the clipboard text as keystrokes for servers without clipboard; rate limited; refuses secret-flagged entries unless confirmed | S | 2 |
| VM04-F042 | Left-handed and swap options | Swap buttons, swap Ctrl and Cmd for macOS hosts, per connection | C | 3 |

### 4.4 RDP

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM04-F043 | Security layers | TLS 1.2/1.3 with Network Level Authentication (CredSSP with NTLMv2 or Kerberos); plain RDP security is refused; TLS-only allowed per connection with a warning | M | 1 |
| VM04-F044 | Graphics | Bitmap updates (RLE, planar), RemoteFX codec, graphics pipeline channel with progressive codec and H.264 (AVC420, AVC444) through the media pipeline | M | 2 |
| VM04-F045 | Clipboard channel | Text, HTML, images, file lists by stream; governed by the redirection policy | M | 1 |
| VM04-F046 | Drive redirection | Chosen local folders appear as remote drives; read-only by default | M | 2 |
| VM04-F047 | Audio | Playback through `svc.media`; recording (microphone) only with `dev.mic` | M | 2 |
| VM04-F048 | Printer redirection | A virtual printer on the remote side that delivers PDF or XPS to the local print portal | S | 3 |
| VM04-F049 | Smart card | Redirect a local smart card reader for login; needs a `dev.usb:ccid` grant | S | 3 |
| VM04-F050 | Display control | Dynamic resize and monitor layout changes during the session | M | 2 |
| VM04-F051 | RemoteApp | Run single remote programs as local windows; listed per connection | C | 3 |
| VM04-F052 | Gateway | Connect through an RD Gateway over HTTPS/WebSocket with its own credentials and certificate pin | S | 3 |
| VM04-F053 | Admin session and load-balance info | Console/admin session flag; load-balance cookie field | S | 2 |
| VM04-F054 | Restricted admin / remote guard | Options that avoid sending reusable credentials to the remote host, where the server supports them | S | 3 |
| VM04-F055 | Server redirection | Follow a server redirection packet only to hosts the user approves; the new host goes through the trust check | M | 2 |
| VM04-F056 | Performance flags | Wallpaper, font smoothing, composition, animations toggles | S | 1 |

### 4.5 VNC

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM04-F057 | Protocol versions | RFB 3.3, 3.7, 3.8 | M | 1 |
| VM04-F058 | Encodings | Raw, CopyRect, RRE, Hextile, ZRLE, Tight (with JPEG), cursor and desktop-size pseudo-encodings, extended desktop size, continuous updates | M | 1 |
| VM04-F059 | Security types | VeNCrypt (TLS with X.509 or anonymous TLS plus password), RA2 where documented; classic VNC password authentication only inside a tunnel or after an explicit "insecure" confirmation | M | 1 |
| VM04-F060 | Clipboard | Latin-1 cut text and the extended clipboard pseudo-encoding (UTF-8, limits) | M | 1 |
| VM04-F061 | Shared or exclusive | Shared-session flag | S | 1 |
| VM04-F062 | Listen mode | Wait for a reverse connection from a server on a chosen port for a limited time; off by default | C | 3 |
| VM04-F063 | Bell and LED state | Bell as a notification sound; lock-key state sync where supported | C | 2 |
| VM04-F064 | Unencrypted warning | A session without transport encryption shows a persistent `--danger` "Not encrypted" badge | M | 1 |

### 4.6 SSH hand-off and tunnels

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM04-F065 | SSH entries | Host, port, user, key reference (SE04), jump hosts, options; stored like other connections | M | 1 |
| VM04-F066 | Terminal hand-off | Connecting an SSH entry emits an intent to SY03 with the entry's parameters; SY03 owns the terminal session | M | 1 |
| VM04-F067 | Files hand-off | "Browse files" opens the host as a remote location in SY02 (SFTP) | S | 2 |
| VM04-F068 | SSH tunnel for RDP/VNC/SPICE | A connection may name an SSH entry as its tunnel; `tunnel-helper` opens a direct-tcpip channel and hands the stream to the protocol helper; no local listening port is opened | M | 2 |
| VM04-F069 | Jump chains | Up to 4 jump hosts, each with its own key and pinned host key | S | 2 |
| VM04-F070 | Host key trust | First contact shows the key type and fingerprint (SHA-256, and a word list form) and pins it; a changed key blocks with a comparison view | M | 1 |
| VM04-F071 | Authentication | Keys and certificates held by SE04 (signing happens in the key service; the helper never sees private keys), hardware tokens, password, keyboard-interactive | M | 2 |
| VM04-F072 | Algorithms | Modern defaults only (Ed25519, ECDSA, curve25519 key exchange with a post-quantum hybrid where available, AEAD ciphers); legacy algorithms per connection with a warning | M | 2 |
| VM04-F073 | Agent forwarding | Off; can be enabled per connection with a per-use confirmation from SE04 | C | 3 |
| VM04-F074 | Known-hosts import | Import `known_hosts` entries as pins | S | 2 |

### 4.7 SPICE (optional)

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM04-F075 | Channels | Main, display, inputs, cursor, playback; record only with `dev.mic` | S | 3 |
| VM04-F076 | Security | TLS with pinned certificate or CA; ticket (password) authentication; SASL not supported | S | 3 |
| VM04-F077 | Agent features | Clipboard, dynamic resolution, multiple heads through the guest agent | S | 3 |
| VM04-F078 | USB redirection | Redirect a local USB device over the usbredir channel with a `dev.usb:<class>` grant | C | 3 |
| VM04-F079 | Connection files | Open `.vv` files; one-time tickets are used once and not stored | S | 3 |

### 4.8 Security

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM04-F080 | Certificate pinning | First contact shows subject, issuer, validity, SHA-256 fingerprint; the user pins the certificate or its public key; later changes block and show old against new | M | 1 |
| VM04-F081 | Trust modes | Per connection: pinned (default), local trust anchors plus host name check, or a chosen CA file | M | 1 |
| VM04-F082 | No silent downgrade | If the server offers a weaker security layer than last time, the connection stops and says so | M | 1 |
| VM04-F083 | Credentials from SE04 | Connections hold references to SE04 items; secrets are requested at connect time and held in the protocol helper's memory only | M | 1 |
| VM04-F084 | Ask every time | Option to never store a credential; the prompt is drawn by the shell | M | 1 |
| VM04-F085 | Route per connection | Clear, VPN profile, or Tor (from SE03); a connection bound to VPN or Tor fails closed if the route is down; onion addresses accepted | M | 2 |
| VM04-F086 | Endpoint log | Each first contact with a host is logged to `svc.audit`; the `[net]` badge lists live endpoints | M | 1 |
| VM04-F087 | Helper per session | Each session runs in its own protocol helper process in capability mode holding one socket (or tunnel stream), one frame buffer handle and the handles for approved redirections | M | 1 |
| VM04-F088 | Remote content is untrusted | Server names, window titles, clipboard, file names and error strings from the remote are shown as plain text with length limits; host-drawn chrome names the real endpoint | M | 1 |
| VM04-F089 | Reach summary | Inspector lists what this connection may cross in each direction in plain words | M | 1 |
| VM04-F090 | Session lock on leave | Option to send a lock command or disconnect when the local screen locks | S | 2 |
| VM04-F091 | Idle disconnect | Disconnect after N minutes without input (per connection) | S | 2 |
| VM04-F092 | Two-factor prompts | TOTP codes can be filled from SE04 on request, never automatically without a click | C | 3 |
| VM04-F093 | Kerberos | Tickets obtained through SE04's credential cache if the user set one up; no ticket handling in this app | C | 3 |
| VM04-F094 | Policy lock | Administrator policy can forbid insecure protocols, clipboard from remote, drive redirection or the server role | S | 3 |

### 4.9 Redirection policy

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM04-F095 | Policy page | One table per connection: Clipboard, Files, Printers, Audio out, Microphone, USB, Smart card, Camera; each Off, Ask, On, with direction where it applies | M | 1 |
| VM04-F096 | Defaults | Clipboard off, files off, audio out on, everything else off | M | 1 |
| VM04-F097 | Clipboard direction | Local to remote, remote to local, both; text only, or text and images; file lists never by default | M | 1 |
| VM04-F098 | Clipboard limits | 1 MiB text, 16 MiB images; secret-flagged local entries never cross; remote-to-local content is tagged with its source host in `svc.clipboard` | M | 1 |
| VM04-F099 | One-shot paste | `Host+V` sends the current clipboard once regardless of policy; `Host+C` fetches once | S | 2 |
| VM04-F100 | Shared folders | Folders picked in the file portal, read-only by default, listed with Remove; the grant ends with the session unless "remember" is ticked | M | 2 |
| VM04-F101 | Incoming files | Files written by the remote land only in shared folders or a quarantine inbox with a provenance tag | M | 2 |
| VM04-F102 | File drag | Drag local files onto the session to copy (where the protocol allows); drag out goes to the inbox | S | 3 |
| VM04-F103 | Live toggles | Redirections can be switched during the session from the connection bar; turning one off revokes the helper's handle at once | M | 2 |
| VM04-F104 | Indicators | Microphone, camera and USB redirection use the system indicators; clipboard crossings can show a toast | M | 2 |
| VM04-F105 | USB classes | Allow by class (storage, HID excluded by default, audio, printer, CCID, vendor-specific); follows VM05's host policy | S | 3 |
| VM04-F106 | Policy inheritance | Folder-level policy with per-connection overrides; the effective value and its source are shown | S | 2 |

### 4.10 Recording and audit

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM04-F107 | Session recording | Record the remote frames (and optionally audio) to an open video container through the media pipeline; a red indicator and elapsed time show while recording | S | 3 |
| VM04-F108 | Lossless event recording | Alternative format: timestamped frame updates and input markers (no key values) that replay exactly in the app | C | 3 |
| VM04-F109 | Recording rules | Per folder: always record, ask, never; storage folder chosen through the portal; retention days | C | 3 |
| VM04-F110 | Encrypted recordings | Optional encryption to a public key from SE04 | C | 3 |
| VM04-F111 | Session log | Per connection: start, end, duration, route, security layer, certificate fingerprint, redirections used, bytes | M | 2 |
| VM04-F112 | Audit | Connects, trust decisions, pin changes, redirection grants, server-side events: to `svc.audit` | M | 2 |
| VM04-F113 | Replay viewer | Play recordings with speed control and seek | C | 3 |

### 4.11 Server: sharing this computer

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM04-F114 | Server off by default | No listener exists until the user enables sharing; the Server page shows state in one line | M | 2 |
| VM04-F115 | Share this session | Offer the running session for view only or view and control; the compositor supplies frames through `dev.screen` and accepts input through the input-injection portal | M | 2 |
| VM04-F116 | Choose what is shared | Whole desktop, one monitor, or one window (window sharing is view-only in phase 2) | S | 3 |
| VM04-F117 | Consent prompt | Each incoming attended connection raises a shell prompt naming the peer address and identity; Accept view, Accept control, Refuse; times out to Refuse | M | 2 |
| VM04-F118 | Sharing indicator | A persistent shell indicator and a screen-edge border while anyone is connected; click shows who and offers Stop | M | 2 |
| VM04-F119 | Stop chord | `Super+Shift+Esc` ends all remote sessions at once; handled by the shell so a remote controller cannot block it | M | 2 |
| VM04-F120 | Protocols served | RDP (TLS with NLA) and VNC (VeNCrypt TLS); each can be turned off | M | 2 |
| VM04-F121 | Listening addresses | Loopback only by default (for tunnels); LAN or a chosen interface needs administrator approval and creates a visible rule in SE02 | M | 2 |
| VM04-F122 | Accounts and keys | Who may connect: named remote identities with a password, a client certificate or an SSH key (through a tunnel); not the local login password by default | M | 2 |
| VM04-F123 | One-time invitation | Create an invitation: code and optional file with address, fingerprint, expiry (10 min to 24 h), view or control; single use | M | 2 |
| VM04-F124 | Server certificate | Self-signed key generated locally and kept in SE04; fingerprint shown as text, word list and QR code for the remote user to check | M | 2 |
| VM04-F125 | Server-side policy | Clipboard direction, file transfer, audio, for remote clients; default clipboard off, files off | M | 2 |
| VM04-F126 | Sensitive windows | Shell prompts, the lock screen, SE04 and windows marked secret are blanked in the shared stream unless the local user holds a key; input to shell prompts from remote is refused | M | 2 |
| VM04-F127 | Privacy mode | Blank the local display and block local input while a remote user controls (for unattended use) | S | 3 |
| VM04-F128 | Session list | Connected clients with address, identity, mode, duration, bandwidth; Disconnect, Demote to view | M | 2 |
| VM04-F129 | Rate limit and lockout | Failed logins: delay doubling from 1 s; 10 failures block the source for 15 min; logged | M | 2 |
| VM04-F130 | Reverse connection | Connect out to a helper who is listening, so the sharer needs no inbound port | S | 3 |

### 4.12 Unattended access and headless sessions

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM04-F131 | Unattended rules | Rules: who (identity), from where (addresses or "tunnel only"), when (days and hours), what (view, control, headless login), with what redirections; first match wins; default deny | M | 3 |
| VM04-F132 | Headless session | A remote login creates a new session with a virtual output of the client's size, not tied to a physical display; login goes through SH03's remote greeter | M | 3 |
| VM04-F133 | Reattach | Disconnecting leaves a headless session running; reconnecting reattaches after authentication | M | 3 |
| VM04-F134 | Take over the local session | A rule may allow connecting to the user's locked local session; unlock uses the normal SH03 authentication | S | 3 |
| VM04-F135 | Lock on disconnect | Sessions lock when the remote user disconnects (default on) | M | 3 |
| VM04-F136 | Service at boot | The server role runs in `svc.remote`; whether it starts at boot is one switch that needs administrator approval | M | 3 |
| VM04-F137 | Session limits | Maximum concurrent headless sessions, idle timeout, maximum duration | S | 3 |
| VM04-F138 | Notifications of access | A notification and an audit line for each unattended login, shown at the next local login | M | 3 |
| VM04-F139 | Emergency disable | A file or command on the machine and a switch in SE01 that disables all remote access until re-enabled locally | S | 3 |

### 4.13 Automation, settings, accessibility, recovery

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM04-F140 | URL schemes and intents | Handles `rdp:`, `vnc:`, `spice:` URLs and `.rdp`, `.vnc`, `.vv` files; `ssh:` is passed to SY03 | M | 1 |
| VM04-F141 | Command line | `remote connect <name or url>`, `remote list`, `remote share --view --minutes 30` | S | 2 |
| VM04-F142 | Command palette | Every connection and action in `Ctrl+K` | M | 1 |
| VM04-F143 | Session restore | Reopen the sessions that were open at last quit (off by default; credentials asked again) | C | 3 |
| VM04-F144 | Crash containment | A protocol helper crash closes one session with an error banner and a Reconnect button; other sessions continue | M | 1 |
| VM04-F145 | Store recovery | The connection store is written atomically with the previous version kept; a damaged store opens in a repair view | M | 1 |
| VM04-F146 | Print the connection list | Export the list (no secrets) as PDF or CSV | C | 3 |
| VM04-F147 | Help | Help per view; a page on choosing a protocol and a route | S | 2 |

## 5. GUI

### 5.1 Window map

```
Remote Desktop
├─ Manager window (main)
│  ├─ Sidebar: All · Favourites · Recent · folders · Server · Recordings · Trust store
│  ├─ Connection list or grid
│  ├─ Inspector (F4): summary, reach summary, last sessions
│  ├─ Quick connect field (Ctrl+L)
│  ├─ Connection editor (sheet; pages General, Display, Security, Redirection, Route, Advanced)
│  ├─ Import sheet (mapping and secrets report) · Export dialog
│  ├─ Trust dialogs: first contact, certificate changed, host key changed, downgrade refused
│  ├─ Server page
│  │  ├─ Share this screen sheet (mode, duration, invitation)
│  │  ├─ Identities and keys list · Rule editor (unattended access)
│  │  ├─ Listening and certificate panel
│  │  └─ Connected clients table
│  ├─ Trust store page (pins: certificates, host keys)
│  ├─ Recordings page · Replay viewer window
│  └─ Preferences (Ctrl+,)
├─ Session window (one or more; tabs, splits)
│  ├─ Connection bar · Send-key menu · Redirection popover · Statistics popover
│  ├─ Full-screen pull-down bar
│  └─ Banners: reconnecting, not encrypted, recording, view only
└─ Shell-drawn (SH04/SH02): credential prompt, folder picker, device prompts,
   incoming-connection consent, sharing indicator and border, administrator approval
```

### 5.2 Main window

Shown at 1100×680, one connection selected.

```
+----------------------------------------------------------------------------------------+
| [=] Remote Desktop      [ rdp://host, vnc://host or a name   Ctrl+L ] (1)  [▁] [▢] [✕] |
+------------+--------------------------------------------------+------------------------+
|  All    42 | [+ New] [Connect] [Edit] | [list|grid] [filter v]  | build-01           (4) |
|  Favourites+--------------------------------------------------+ RDP · 10.20.0.14:3389  |
|  Recent    | Name          Proto Address         Route  Trust (2)| via jump "bastion"     |
| ▾ Work     |> build-01     RDP   10.20.0.14      vpn    ✓ pinned| Route: VPN "office"    |
|   ▸ Prod   |  build-02     RDP   10.20.0.15      vpn    ✓ pinned| TLS 1.3 + NLA          |
|   ▸ Lab    |  bastion      SSH   bastion.ex.org  clear  ✓ key   | Cert 9F:2C:…:41 pinned |
| ▾ Home     |  nas-console  VNC   nas.home        clear  ⚠ none  | ---------------------- |
|   media-pc |  pi-kiosk     VNC   10.0.0.31       clear  ✓ pinned| May cross:         (5) |
| ---------- |  home-onion   RDP   abcd…wxyz.onion tor    ✓ pinned|  → clipboard text      |
|  Server ○  |  win-vdi      RDP   vdi.ex.org      vpn    ✓ CA    |  → folder ~/Work/out   |
|  Recordings|  …                                                 |  ← sound               |
|  Trust     |                                                    | Blocked: files in,     |
|  store     |                                                    |  clipboard in, USB     |
|            |                                                    | Last: 11 Nov, 2 h 05 m |
|            |                                                    | [Connect] [Edit…]      |
+------------+--------------------------------------------------+------------------------+
| 42 connections · 1 session open · server: off (3)                                       |
+----------------------------------------------------------------------------------------+
```

1. Quick connect (`lineEdit` with completion from saved names and history).
2. Connection table (`tableView`): protocol, address, route badge in `--net` when not local, trust state with icon and word.
3. Status bar: counts and the server state. When the server is on, the text is `--warn` and names the listening address.
4. Inspector header: protocol, endpoint, tunnel, route, security, pin.
5. Reach summary with arrows for direction.

States: empty ("No saved connections. Type an address above or add one", `[+ New]`), loading, error (store damaged: repair view), offline (connections whose route is down show "route down" and Connect is disabled with the reason), read-only (policy), narrow (sidebar overlay, table shows name and protocol, inspector sheet).

### 5.3 Other views and dialogs

#### Session window (1280×800)

```
+----------------------------------------------------------------------------------------+
| [=] build-01 — 10.20.0.14                         [net] [rec]          [▁] [▢] [✕]    |
+----------------------------------------------------------------------------------------+
| [build-01 ×] [nas-console ×] [+]                                                        |
+----------------------------------------------------------------------------------------+
| [⏏ Disconnect] [Send key v] [Clipboard: → v] [Share v] [● Rec] [View only] [⛶] [ⓘ] (1) |
+----------------------------------------------------------------------------------------+
|                                                                                         |
|                              remote desktop frame (2)                                   |
|                                                                                         |
+----------------------------------------------------------------------------------------+
| ✓ TLS 1.3 · pinned · VPN "office" (3) | Keyboard: captured, Right Ctrl releases | 34 ms |
+----------------------------------------------------------------------------------------+
```

(1) Connection bar (`toolButton` row). (2) `viewportWidget` fed by the helper's frame handle. (3) Security summary, always visible; "Not encrypted" in `--danger` when that is the case. States: connecting (steps listed: route, tunnel, TLS, authentication, channels), authenticating (shell prompt), reconnecting (dimmed last frame, banner with countdown and Cancel), disconnected (reason and Reconnect), view only (banner), recording (red dot and elapsed time).

#### First-contact trust dialog (560×400)

```
+--------------------------------------------------------------+
| Do you trust build-01 (10.20.0.14:3389)?                      |
+--------------------------------------------------------------+
| This is the first connection. The server presented:           |
| Subject   CN=build-01.office.example                          |
| Issuer    self-signed                                         |
| Valid     2 Jan 2026 – 2 Jan 2027                             |
| SHA-256   9F2C 77A0 1B3E … 0D41                               |
| Words     river-stone-amber-quill-north-seven                 |
| Compare this with the value shown on the server.              |
| (•) Pin this certificate   ( ) Pin its public key             |
| [Cancel]                 [Connect once]   [Pin and connect]   |
+--------------------------------------------------------------+
```

The changed-certificate variant shows old and new values side by side and has no default button.

#### Connection editor, Redirection page (sheet, 720×520)

```
+------------------------------------------------------------------------+
| Edit build-01     General  Display  Security [Redirection] Route  Adv.  |
+------------------------------------------------------------------------+
| Policy source: folder "Work" (overrides marked •)                       |
| Clipboard    [ local → remote v]  [text only v]                    •    |
| Files        [ Ask v]   Folders: ~/Work/out (read-only)  [+ Add…]       |
| Printers     [ Off v]                                                   |
| Audio out    [ On v]      Microphone [ Off v]                           |
| USB          [ Off v]     Smart card [ Off v]     Camera [ Off v]       |
| Incoming files go to  ~/Inbox/Remote/build-01   (quarantine)            |
| [ ] Show a notice each time the clipboard crosses                       |
+------------------------------------------------------------------------+
| [Cancel]                                                    [Save]      |
+------------------------------------------------------------------------+
```

#### Share this screen (sheet, 600×440)

```
+--------------------------------------------------------------+
| Share this screen                                             |
| Allow   (•) view only   ( ) view and control                  |
| Share   [ whole desktop v]                                    |
| For     [ 30 minutes v]   then sharing stops by itself        |
| Reach   (•) through my tunnel/VPN only  ( ) local network…    |
| Invitation                                                    |
|   Code         4821-7730-19                                   |
|   Fingerprint  river-stone-amber-quill-north-seven            |
|   [Copy invitation text]  [Save invitation file…]  [QR]       |
| You will be asked before anyone connects.                     |
| [Cancel]                                  [Start sharing]     |
+--------------------------------------------------------------+
```

#### Server page (in the main window)

```
+----------------------------------------------------------------------------------------+
| Server                                           Sharing is OFF   [Share this screen…]  |
+----------------------------------------------------------------------------------------+
| Listening   RDP [x] 127.0.0.1:3389   VNC [ ] –          [Change… needs approval]        |
| Certificate 9F2C … 0D41 (Ed25519, in Passwords and Keys)  [Show] [Renew]                |
| Identities  alex (key), sam-laptop (client certificate)   [Manage…]                     |
| Unattended access rules                                              [+ Rule]           |
|  1  sam-laptop · tunnel only · Mon–Fri 08–18 · headless login · clipboard off           |
|  2  alex · any · never (disabled)                                                       |
|  –  everyone else: refuse                                                               |
| Connected now  (none)                                                                   |
| Last access    10 Nov 21:14 sam-laptop headless 1 h 02 m             [Open audit…]      |
+----------------------------------------------------------------------------------------+
```

#### Rule editor (dialog)

```
+--------------------------------------------------------------+
| Unattended access rule                                        |
| Who     [ sam-laptop v]                                       |
| From    (•) tunnel only  ( ) addresses [______________]       |
| When    [x]Mon [x]Tue [x]Wed [x]Thu [x]Fri [ ]Sat [ ]Sun      |
|         from [08:00] to [18:00]                               |
| Allow   ( ) view  ( ) control  (•) headless login             |
| Cross   clipboard [off v]  files [off v]  sound [on v]        |
| [x] Lock the session when they disconnect                     |
| [x] Tell me at next login                                     |
| [Cancel]                                          [Save rule] |
+--------------------------------------------------------------+
```

#### Import sheet

```
+--------------------------------------------------------------+
| Import connections                                            |
| 14 found in 3 files                                           |
| [x] build-01.rdp       RDP   ⚠ redirects drives: turned off   |
| [x] nas.vnc            VNC   ⚠ password in file → moved to    |
| [x] ~/.ssh/config (9)  SSH      Passwords and Keys            |
| [ ] old-xp.rdp         RDP   ✕ needs plain RDP security       |
| Into folder [ Imported v]                                     |
| [Cancel]                                      [Import 13]     |
+--------------------------------------------------------------+
```

### 5.4 Menus

```
File       New connection… Ctrl+N · New folder Ctrl+Shift+N · Quick connect Ctrl+L ·
           Import… Ctrl+I · Export… Ctrl+E · Preferences Ctrl+, · Close Ctrl+W · Quit Ctrl+Q
Edit       Undo Ctrl+Z · Redo Ctrl+Shift+Z · Rename F2 · Duplicate Ctrl+D · Edit… Ctrl+Enter ·
           Find Ctrl+F · Delete Delete
View       List Ctrl+1 · Grid Ctrl+2 · Sidebar F9 · Inspector F4 · Server Ctrl+Shift+R ·
           Trust store · Recordings · Refresh F5
Session    Connect Enter · Connect view-only Shift+Enter · Disconnect Host+Q ·
           Reconnect Host+N · Send key ▸ · Full screen Host+F · All monitors Host+M ·
           Scaling ▸ · View only Host+O · Screenshot Host+S · Record Host+R ·
           Paste once Host+V · Copy once Host+C · Share a folder… · Redirections ▸ ·
           Statistics Host+I · Next tab Ctrl+Tab · Detach tab
Server     Share this screen… · Stop sharing Super+Shift+Esc · Identities… · Rules… ·
           Listening… · Show fingerprint
Help       Help F1 · Choosing a protocol · About
```

### 5.5 Keyboard and pointer

Host-key chords are listed in 5.4; `Host+Del` sends Ctrl+Alt+Del; `Host+F1…F12` send Ctrl+Alt+Fn. All are rebindable. The stop-sharing chord belongs to the shell and cannot be rebound to nothing.

Context menus: connection (Connect, Connect view-only, Edit, Duplicate, Move to, Copy address, Copy as URL, Show pin, Forget pin, Delete); session tab (Reconnect, Detach, Split right, Split down, Rename tab, Disconnect); pin row (Show, Export, Remove); client row on the Server page (Disconnect, Demote to view).

Drag and drop: `.rdp`, `.vnc`, `.vv` files on the manager import them; a connection on a folder moves it; a tab out of the window detaches it; local files on a session copy them if the Files policy is On or Ask (then a prompt); a folder on the session offers "Share this folder".

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Quick connect hint and `[+ New]` |
| Loading | Connection steps listed with the current one marked; Cancel always available |
| Error | Plain cause (unreachable, refused, authentication failed, trust failed, downgrade refused, protocol error) and one next action |
| Offline / route down | Connect disabled with the reason; a VPN- or Tor-bound connection never falls back to the clear route |
| Locked | Keyring locked: connections that need stored secrets show a lock and ask on connect |
| Read-only | Policy lock: editors disabled, policy named |
| Narrow | Manager: sidebar overlay. Session: connection bar collapses into one menu button |
| Sharing | Shell indicator and border; Server page turns `--warn`; status bar names the peers |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar, policy inheritance view | `treeView` |
| Connections, pins, rules, clients, recordings | `tableView`, `gridView` |
| Session tabs and splits | `tabBar`, `splitter` |
| Remote frame | `viewportWidget` with an external frame source |
| Replay viewer | `videoWidget` (needs G-11) and `timeline` |
| Editor pages | `tabWidget`, `comboBox`, `lineEdit`, `spinBox`, `checkBox`, `toggleSwitch`, `dateTimeEdit` |
| Statistics | `chart` |
| Notes, help | `markdownView` |
| Dialogs | `dialog`; QR code as `image` |

Missing:

- `VM04-GAP-1`: `svc.remote`, a system service for the server role (listeners, authentication, headless sessions).
- `VM04-GAP-2`: an input-injection portal and capability (PLATFORM.md section 6 has `dev.screen` but nothing for injecting input).
- `VM04-GAP-3`: compositor support for headless sessions with virtual outputs, a remote greeter in SH03, and blanking of sensitive surfaces in captured streams.
- `VM04-GAP-4`: external frame source for `viewportWidget` (same as VM01-GAP-2) and shortcut inhibit (same as VM01-GAP-3).
- `VM04-GAP-5`: a shell-owned sharing indicator, border and stop chord.
- H.264 decode and video encode need the media pipeline (G-11).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Connection store, policy resolution, trust decisions UI, session orchestration |
| UI | FUSE UI | app host | Windows in 5.1 |
| `rdp-helper` | C++23 | helper per session, capability mode | RDP client: X.224, MCS, security, channels, codecs |
| `rfb-helper` | C++23 | helper per session | VNC client |
| `spice-helper` | C++23 | helper per session | SPICE client (optional) |
| `tunnel-helper` | C++23 | helper per tunnel | SSH transport for tunnels and jump chains; RD Gateway transport; asks SE04 to sign |
| `import-helper` | C++23 | helper, on demand | Parse `.rdp`, `.vnc`, `.remmina`, `.vv`, OpenSSH config, CSV |
| Server service `svc.remote` | C++23 | system service (Proposed) | Listeners, authentication, rules, invitations, RDP and VNC server protocol processes (one per client, capability mode), headless session requests to the compositor |
| Compositor hooks | – | compositor | Capture, input injection, virtual outputs, indicator (gaps above) |
| `remote` CLI | ForgeScript | own process | Same store and bus interface |

```
 app host ──┬─ rdp-helper ─┐                      ┌──▶ remote host
            ├─ rfb-helper ─┼─ stream ─ tunnel-helper ─ SSH / gateway ─┘
            ├─ spice-helper┘      (or a direct socket on the chosen route)
            └─ import-helper
 frames: helper ── shm/dmabuf handle ──▶ viewportWidget      keys: svc.keyring (SE04)

 remote client ──▶ svc.remote ─ per-client protocol process ─▶ compositor (capture, input)
                        └─ consent prompt / indicator: shell
```

Each protocol helper receives: one connected stream handle (opened by the app host's network layer under `net.user` and the chosen route, or by `tunnel-helper`), one frame buffer handle, and handles for approved redirections. It cannot open sockets or files itself.

### 6.2 Data model

Connection store `connections.db` (SQLite) in `~/.local/share/org.pbsd.remote-desktop/`:

| Table | Fields |
|---|---|
| `folder` | `id`, `parent`, `name`, `policy_json`, `defaults_json` |
| `connection` | `id` uuid, `folder`, `name`, `protocol` (`rdp`, `vnc`, `spice`, `ssh`), `host`, `port`, `username`, `domain`, `secret_ref` (keyring id or null), `key_ref`, `tunnel` (connection id or null), `gateway_json`, `route` (`clear`, `vpn:<profile>`, `tor`), `trust_mode`, `display_json`, `input_json`, `policy_json`, `proto_json`, `tags`, `colour`, `notes`, `wol_mac`, `created`, `last_used` |
| `pin` | `id`, `host`, `port`, `kind` (`x509-cert`, `x509-spki`, `ssh-hostkey`), `algorithm`, `fingerprint_sha256`, `first_seen`, `last_seen`, `security_layer` |
| `session_log` | `id`, `connection`, `started`, `ended`, `route`, `security`, `pin`, `redirections`, `bytes_in`, `bytes_out`, `end_reason` |
| `recording` | `id`, `connection`, `started`, `seconds`, `file_grant`, `format`, `encrypted_to` |
| `history` | `text`, `last_used` (quick connect) |

`policy_json`: `{clipboard{dir, kinds}, files{mode, folders[]}, printers, audio_out, mic, usb{mode, classes[]}, smartcard, camera}` with each value `off`, `ask`, `on` or `inherit`.

Server store (in `svc.remote`): `identities.toml` (`{name, kind: password|cert|sshkey, verifier}`; passwords stored as Argon2id verifiers), `rules.toml` (`{order, who, from, days, start, end, allow, cross{}, lock_on_disconnect, notify}`), `listen.toml`, `invitations` (in memory only), access log to `svc.audit`.

Schema version in every store; forward-only migration with the previous file kept.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| RDP (MS-RDPBCGR and channel specifications: clipboard, drive, audio, display control, graphics pipeline, input) | yes | yes (client; server in `svc.remote`) | Own code from the published specifications | `rdp-helper`; server protocol process |
| CredSSP, SPNEGO, NTLMv2; Kerberos through SE04 | yes | yes | Own code + FORGE crypto (FORGE-WP-22) | `rdp-helper` |
| RemoteFX, progressive, planar, RLE codecs | decode | encode (server, subset) | Own code | `rdp-helper` |
| H.264 | decode | encode (server, phase 3) | Media pipeline (G-11) | media helper |
| RFB 3.3–3.8, encodings, VeNCrypt | yes | yes | Own code; zlib from base (zlib licence); JPEG through FUSE image codecs | `rfb-helper` |
| SPICE | yes | no | Own code from the protocol document | `spice-helper` |
| SSH transport, authentication, connection protocol (RFC 4251–4254 and current algorithm RFCs) | yes | yes | Own code + FORGE crypto, or the SSH library planned for SY03 if one is chosen there | `tunnel-helper` |
| TLS 1.2/1.3 | yes | yes | FORGE network stack (FORGE-WP-21) | each helper |
| RD Gateway (HTTP/WebSocket transport) | yes | no | Own code | `tunnel-helper` |
| `.rdp`, `.vnc`, `.remmina`, `.vv`, OpenSSH `config`, `known_hosts`, CSV | yes | `.rdp`, `.vnc` | Own code | `import-helper` |
| Native connection bundle (TOML) | yes | yes | Own code | app host |
| Recording: Matroska with an open video codec, or the event format `.rdrec` (length-prefixed records) | yes | yes | Media pipeline; own code for `.rdrec` | media helper; app host |
| Wake-on-LAN magic packet | – | yes | Own code | app host network layer |

FreeRDP (Apache-2.0) and libvncclient (GPL-2.0) are not proposed: both are C and large. If own RDP code slips, running FreeRDP inside the same helper sandbox is the fallback (see section 13).

### 6.4 Integration

- Intents handled: `open` for `application/x-rdp`, `application/x-vnc`, `application/x-virt-viewer`, URL schemes `rdp`, `vnc`, `spice`. Emitted: `open` to SY03 (SSH entries, with parameters and key reference), SY02 (SFTP location), SE03 (route set-up), SE04 (item), SE02 (listener rule), AD04 (audit view), ME03 (exported recordings).
- Services: `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.screen`, `svc.portal.mic`, `svc.portal.camera`, `svc.portal.usb`, `svc.portal.print`, `svc.intent`, `svc.search`, `svc.keyring`, `svc.clipboard`, `svc.media`, `svc.power` (inhibit sleep during sessions), `svc.audit`, `svc.remote` (gap).
- IPC offered by `svc.remote`: `org.pbsd.remote.Server/1` (state, share, stop, identities, rules, clients); used by SH02 (indicator and quick-settings tile), SE01 (posture: "remote access is on"), SY01 (sharing settings page).
- Other apps: VM01 may use the same client helpers for consoles of VMs on remote hosts (Proposed); SE03 supplies routes; SE04 keys; SH03 the remote greeter.

## 7. Security and privacy

### Capabilities requested

| Capability | Reason |
|---|---|
| `fs.app` | Connection store, pins, session log |
| `fs.portal` | Shared folders, import and export, recordings folder, inbox |
| `net.user` | Hosts the user typed or saved; first contacts logged |
| `net.route:clear`, `net.route:vpn`, `net.route:tor` | Per-connection route |
| `proc.helper:rdp-helper`, `proc.helper:rfb-helper`, `proc.helper:spice-helper`, `proc.helper:tunnel-helper`, `proc.helper:import-helper` | Protocol and file parsing outside the app host |
| `dev.screen` | Server role: sharing this session (requested only when sharing starts) |
| `dev.mic`, `dev.camera`, `dev.usb:*`, `dev.print` | Redirections, each prompted |
| `svc.keyring`, `svc.clipboard`, `svc.media`, `svc.power` | Secrets and keys; clipboard policy; audio; sleep inhibit |
| `svc.settings`, `svc.notify`, `svc.intent`, `svc.search`, `svc.audit` | As in 6.4 |
| `svc.remote` | Server role (VM04-GAP-1) |

Not requested: `proc.exec`, `fs.path`, `dev.location`, `ai.local`, `bg.run` (the server role lives in `svc.remote`; client sessions end with their window).

### Threat model

Assets: local files, clipboard and devices; credentials and keys; the local session when shared; the truth of which host is on screen.

| Adversary | Attack | Mitigation |
|---|---|---|
| Malicious or compromised server | Exploit the client through protocol messages or codecs | One helper per session in capability mode with only its stream and frame handles; fuzzing per message type and codec; size limits |
| Malicious server | Steal local data through redirection (clipboard sniffing, drive enumeration) | All redirection off by default; direction control; folders by portal grant; secret-flagged clipboard never crosses |
| Malicious server | Plant files locally | Incoming files only in shared folders or the quarantine inbox with provenance |
| Malicious server | Phish through remote-drawn dialogs | Host-drawn chrome names the endpoint; shell prompts are on a layer the remote frame cannot reach |
| Network attacker | Man in the middle, downgrade | Pinning; no silent downgrade; plain RDP security refused; unencrypted VNC only after explicit confirmation and marked throughout |
| Network attacker | Reach the server role | Off by default; loopback by default; TLS with NLA or VeNCrypt; rate limit and lockout; rules default deny |
| Credential theft on the remote | Reuse of credentials sent to a compromised host | Restricted-admin style modes; keys stay in SE04 (sign requests only); agent forwarding off |
| Hostile `.rdp` or `.vv` file | Enable redirections or point at an attacker host | Review of all resource-redirecting settings for unsigned files; imports turn redirections off |
| Remote controller of a shared session | Approve prompts, read secrets, block the stop chord | Shell prompts refuse remote input; sensitive surfaces blanked; stop chord handled by the shell |
| Local other app | Start sharing or read pins | `svc.remote` accepts sharing requests only from this app or SY01 and always raises shell consent; store is in the app's private data |

Stored: connections, pins, logs in app data; secrets only in `svc.keyring`; recordings where the user chose. Leaves the machine: only traffic to hosts the user connected to, over the chosen route. No discovery, probes or announcements unless enabled.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `manager.view` | enum `list`, `grid` | `list` | |
| `session.host_key` | key | `Right Ctrl` | Release and chord key |
| `session.scale_mode` | enum `one`, `fit`, `fill`, `integer` | `fit` | Default for new connections |
| `session.dynamic_resolution` | bool | `true` | |
| `session.grab_on_focus` | bool | `true` | |
| `session.reconnect` | bool | `true` | Automatic reconnect |
| `session.inhibit_sleep` | bool | `true` | While a session is open |
| `session.tabs` | bool | `true` | Tabs or one window per session |
| `defaults.clipboard` | enum `off`, `to-remote`, `from-remote`, `both` | `off` | |
| `defaults.audio_out` | bool | `true` | |
| `defaults.route` | string | `clear` | |
| `defaults.trust_mode` | enum `pin`, `anchors`, `ca-file` | `pin` | |
| `security.allow_insecure_vnc` | bool | `false` | Permits the "insecure" confirmation at all |
| `security.legacy_ssh_algorithms` | bool | `false` | |
| `clipboard.notify` | bool | `true` | Toast on each crossing |
| `inbox.folder` | portal grant | `~/Inbox/Remote` | Quarantine for incoming files |
| `recording.default` | enum `never`, `ask`, `always` | `never` | |
| `recording.retention_days` | u32 | `0` (keep) | |
| `history.quick_connect` | bool | `true` | Keep quick-connect history |
| `server.share_duration_min` | u32 | `30` | Default sharing time |
| `server.lock_on_disconnect` | bool | `true` | |
| `discovery.enabled` | bool | `false` | Local service discovery |

## 9. Accessibility and languages

- The remote frame is one opaque image in the a11y tree with name, host, security state and size. Remote screen readers work through redirected audio. An action "Read screen text" uses the OCR helper of ME05 on request.
- Every state banner (reconnecting, not encrypted, recording, view only, shared) is a live-region announcement and has an icon and a word.
- The Host key and all chords are rebindable; a sticky Host mode lets chords be typed one key at a time.
- The local user can always leave: `Host` releases the grab; the stop-sharing chord is global.
- Fingerprints are offered as hex in groups, as a word list, and as QR, so they can be compared by voice.
- Remote text (host names, titles) is shown as received and marked with its direction; it is never translated.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | 200 ms with 2,000 saved connections |
| Connect on a LAN | under 1.5 s to first remote frame after authentication |
| Added input latency | under 5 ms from key event to socket write |
| Frame path | Decode in the helper to a shared buffer; no extra copy in the app host; 60 Hz at 2560×1440 with the graphics pipeline codecs |
| Idle session | Zero frames drawn without updates |
| App host memory | under 80 MiB; each helper under 150 MiB at 4K |
| Server role | Capture and encode one 1920×1080 stream under 15% of one core with damage tracking |
| Reconnect | First attempt within 1 s of route recovery |

## 11. Testing and proof

Nothing has been run. All entries are targets.

- Unit: policy inheritance, URL parsing, rule matching (who, where, when), pin comparison, import mapping.
- Golden renders of 5.2 and 5.3.
- Scripted UI: T1–T13 against loop-back test servers.
- Fuzz: every protocol decoder by message type (RDP PDUs and each channel, each codec, RFB messages and encodings, SPICE messages, SSH packets, CredSSP/SPNEGO ASN.1), every import parser, the server-side decoders.
- Interoperability matrix: reference RDP servers (current Windows versions, xrdp), VNC servers (TigerVNC, a QEMU console, a macOS screen-sharing host), SPICE (QEMU), OpenSSH servers; and the reverse for the server role with common clients.
- Differential: decoded frames compared with a reference client's output for recorded sessions.
- Security tests: downgrade attempts, certificate swap, hostile `.rdp` files, clipboard and drive policy enforcement, remote input aimed at shell prompts, lockout.
- Fault injection: kill helpers, drop the route, stall the server, fill the disk during recording.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Redirection policy resolver | The effective policy never exceeds the most specific explicit setting; `off` wins over inherited `on` | T1 | Small model and contracts through PRISM | – |
| Unattended rule matcher | Default deny; first match wins; no match outside the time window | T1 | Model and contracts | – |
| Pin check | A connection proceeds only if the presented key equals the pin, or the user decided at a prompt | T2 | Contracts, tests | – |
| Protocol decoders (RDP, RFB, SPICE, SSH) | No runtime errors on any input; bounded allocation | T2 | Safe subset, contracts, bounded checking, fuzzing | Size of RDP |
| Codecs | Memory safety | T3 | Sanitizers, fuzzing, differential frames | Arithmetic-heavy code |
| Server authentication path | No session before successful authentication and rule match | T2 | State-machine model, contracts | – |
| App logic | No capability widening | T3 | FSS policy, script tests | Interpreter not verified |
| UI | – | T4 | Golden and scripted tests | – |

## 12. Build plan

### Phase 1: usable core

| ID | Work item | Depends on |
|---|---|---|
| VM04-W01 | Connection store, folders, editor, quick connect, search | G-01, G-02, G-07 |
| VM04-W02 | Session window with tabs, scaling, connection bar, keyboard grab | VM04-GAP-4, VM04-W01 |
| VM04-W03 | `rfb-helper`: RFB 3.8, core encodings, VeNCrypt | FORGE-WP-21, FORGE-WP-27, FORGE-WP-28 |
| VM04-W04 | `rdp-helper` core: connection sequence, TLS + NLA (NTLMv2), bitmap updates, input | FORGE-WP-21, FORGE-WP-22 |
| VM04-W05 | Trust: pinning, first-contact and changed-key dialogs, downgrade refusal | VM04-W03, VM04-W04 |
| VM04-W06 | Credentials through `svc.keyring`; shell credential prompt | SE04, SH04, G-09 |
| VM04-W07 | Clipboard channels with policy (text) | G-09 (`svc.clipboard`) |
| VM04-W08 | SSH entries and hand-off to SY03 | SY03 |
| VM04-W09 | Reconnect, error states, crash containment | VM04-W02 |

### Phase 2: full feature

| ID | Work item | Depends on |
|---|---|---|
| VM04-W10 | `tunnel-helper`: SSH tunnels and jump chains with SE04 signing | SE04, FORGE-WP-22 |
| VM04-W11 | Routes through SE03 (VPN, Tor), fail closed | SE03 |
| VM04-W12 | RDP graphics pipeline, RemoteFX, H.264, dynamic resolution, multi-monitor | G-11, VM04-W04 |
| VM04-W13 | Drive redirection, audio, full redirection policy with inheritance and live toggles | SH04, G-09 (`svc.media`) |
| VM04-W14 | Import and export, signed `.rdp` review | `import-helper` |
| VM04-W15 | Session log, audit, statistics | G-09 (`svc.audit`) |
| VM04-W16 | `svc.remote`: share this session over RDP and VNC with consent, indicator, invitations | VM04-GAP-1, VM04-GAP-2, VM04-GAP-5, SH02, SH04 |
| VM04-W17 | Server page, identities, listening, SE02 rule hand-over | SE02, VM04-W16 |
| VM04-W18 | `remote` CLI, URL schemes | VM04-W01 |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| VM04-W19 | Unattended rules, headless sessions, reattach, remote greeter | VM04-GAP-3, SH03, VM04-W16 |
| VM04-W20 | Recording, event format, replay viewer | G-11, ME05 |
| VM04-W21 | SPICE client | VM04-W02 |
| VM04-W22 | RD Gateway, RemoteApp, smart card, printer and USB redirection | G-16, VM05, VM04-W12 |
| VM04-W23 | Privacy mode, reverse connections, window sharing | VM04-W16 |
| VM04-W24 | Wake-on-LAN, discovery, bulk edit, session restore | VM04-W01 |

## 13. Open questions and risks

- RDP is very large. Writing a client and a server from the specifications in memory-safe C++23 is the biggest cost in this app. Fallback: FreeRDP (Apache-2.0, C) inside the same per-session sandbox until own code is ready. The owner must accept or reject that interim.
- The server role depends on compositor features that do not exist in the plan read here: input injection, virtual outputs, headless sessions, protected surfaces in capture (VM04-GAP-2, -3, -5). The current repo plan (P5-02) names KWin; the suite replaces it, so these belong to the new compositor's plan.
- PLATFORM.md section 6 has no capability for input injection. A new capability (for example `dev.input.inject`) with a shell-owned indicator is needed.
- H.264 decode and encode depend on G-11 and on patent-free availability in the distribution; without it the client falls back to RemoteFX and progressive codecs.
- NTLMv2 is weak by modern standards but still needed for many RDP servers. Kerberos needs SE04 to provide a ticket cache; Unverified there.
- SSH: sharing one implementation between SY03, IN03 and this app's tunnel helper would avoid three code bases. Which app owns it is undecided.
- No rendezvous server means helping someone behind NAT needs a VPN, a Tor onion service or a reverse connection. This is deliberate, but it costs ease of use.
- Classic VNC authentication (DES challenge) is insecure; allowing it at all, even with warnings, is a policy choice (`security.allow_insecure_vnc`).
