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
