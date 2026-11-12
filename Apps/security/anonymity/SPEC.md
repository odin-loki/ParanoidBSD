<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Anonymity

By Odin Loch

| | |
|---|---|
| ID | `SE03` |
| Bundle | `org.pbsd.anonymity` |
| Category | security |
| Priority | P1 |
| Backing repos | ANONYMOUS (contains AEGIS, a Rust mix-network datapath), ParanoidBSD (WireGuard in the `hbsd/` reference tree), FORGE (network service, FORGE-WP-21), FUSE (UI) |
| Status | Planned. Nothing built. |

## 1. Purpose

Anonymity controls how traffic leaves the machine: in the clear, through a VPN, through an onion-routing network, or as messages through the AEGIS mix network. It sets the route each app may use, keeps identities apart, tests for leaks and shows honestly what each route hides and what it does not. It is the FUSE front end for ANONYMOUS / AEGIS and for the system's VPN and onion-routing helpers.

### 1.1 What the ANONYMOUS repo provides today (Verified, read 12 Nov 2026)

| Fact | Source |
|---|---|
| The repo's product is AEGIS: "a metadata-hiding transport for a permissioned multi-party consortium". It hides who talks to whom, when and how much from a global passive adversary. Content encryption is assumed solved elsewhere. | `ANONYMOUS:README.md` |
| Two planes. Mode 1 is a shaped mix network: Sphinx packets, constant-rate emission, receiver padding, a stratified topology of 4 layers, stable layered guards. Mode 2 is a bulk plane negotiated per transfer. | `ANONYMOUS:README.md`, `docs/AEGIS_SPEC_v3_consolidated.md` |
| Bulk transfers have a security dial `L0Raw`, `L1Bucketed`, `L2UniformBatched`, chosen from a stated `ThreatLevel` (`RelationshipExposureOk`, `PartialHidingRequired`, `FullHidingRequired`). Full hiding needs about 40 concurrent transfers. | `ANONYMOUS:crates/aegis-negotiator/src/dial.rs` |
| Rust workspace, eight crates: `aegis-crypto`, `aegis-relay`, `aegis-topology`, `aegis-node`, `aegis-client`, `aegis-negotiator`, `aegis-trust`, `aegis-crypto-dudect-ffi`. Workspace lint `unsafe_code = "forbid"`. Licence AGPL-3.0-or-later. | `ANONYMOUS:crates/Cargo.toml` |
| Cryptography: hybrid KEM of X25519 and ML-KEM-768, ChaCha20-Poly1305 links, SHA-3, optional Noise handshake (`snow`), threshold BLS beacon, Bulletproofs range proofs for reputation. | `ANONYMOUS:crates/Cargo.toml`, `README.md` |
| The client is a command-line packet injector: `aegis-client --config <toml>` with `--payload`, `--stdin`, `--raw`, `--roster-path`, `--cover-secs` (default 2.0), `--tau-secs` (default 0.35), `--peak-rate` (default 2.0), `--allow-high-rho`, `--require-kem-binding`. It also exposes a library `PacedSession` (`connect`, `enqueue_packet`, `send_payload_via_session`, `wait_idle_cover`, `shutdown`). | `ANONYMOUS:crates/aegis-client/src/bin/aegis-client.rs`, `crates/aegis-client/src/session.rs` |
| Client configuration: `first_hop_addr`, `ingress_link_key`, a signed roster (`path`, `threshold`, `authority_pubkeys`), optional `[guard_mitigation] preset` (`adaptive_v4` and earlier), optional `[path]` with `require_diverse_jurisdictions` and `max_per_jurisdiction`, and per-hop KEM seeds. | `ANONYMOUS:deploy/templates/client.production.toml` |
| The relay is `aegis-node` (`--config`, `validate`). Its exit sink delivers peeled payloads to standard output or a file only. | `ANONYMOUS:crates/aegis-node/src/main.rs`, `crates/aegis-node/src/exit_sink.rs:25` |
| Relay statistics pass an export gate that quantises values and limits scrape rate. | `ANONYMOUS:crates/aegis-relay/src/metrics_export.rs` |
| Relay KEM seeds are protected by Windows DPAPI or the Unix keyring crate, with a mode-0600 file as fall-back. | `ANONYMOUS:crates/aegis-node/src/kem_seed_protect.rs`, `crates/aegis-node/Cargo.toml` |
| Stated limits: strong guarantees are for internal client-to-client traffic; clearnet exit is weaker; results are empirical bounds, not proofs; not interactive, multi-second latency (mixing mean about 2 s); wrong tool for two-party traffic hiding. Trusted-hardware attestation is deferred. | `ANONYMOUS:README.md` ("Honest boundaries", build plan table) |
| Test and evidence claims (195 Rust tests, 25 simulation tests, fuzzing) are the repo's own; not run here. | `ANONYMOUS:README.md` |

What the repo does **not** contain (searched `crates/*/src`, nothing found): a SOCKS or transparent proxy, a tunnel device, WireGuard, onion routing for general internet traffic, bridges or pluggable transports, a graphical interface, any FreeBSD-specific code, and a client-side receive or inbox path in `aegis-client`.

Consequence: AEGIS cannot today carry an arbitrary app's sockets. It can carry **messages** between members of a roster. This spec therefore plans four routes, and is clear about which exist:

| Route | Carries | Exists today | Source of the engine |
|---|---|---|---|
| `clear` | Any socket | yes (the normal network) | OS |
| `vpn` | Any socket, inside a WireGuard tunnel | WireGuard is in the reference tree | `hbsd/src/sys/dev/wg`, `hbsd/src/usr.bin/wg` |
| `tor` | TCP streams, through an onion-routing network | No engine in any repo read. Proposed third-party helper (section 6.3) | Proposed |
| `mix` | Messages and files between roster members, through AEGIS | Datapath and CLI exist; no app-facing service | `ANONYMOUS:crates/` |

`mix` is not one of the `net.route` values in PLATFORM.md section 6. Adding it is a platform change (section 13).

## 2. Scope

In scope:

- Connect and disconnect each route; status and health.
- Circuit and path views for the onion and mix routes.
- Per-app route assignment (`net.route`), with the firewall enforcing it.
- Identities: separation of circuits and state per identity; new identity.
- Leak tests.
- Bridges and transports for the onion route.
- VPN profiles (WireGuard): import, create, connect, chain.
- AEGIS client set-up: roster, guards, first hop, pacing, bulk dial, send and receive queue.
- Kill switch for routes (uses SE02).
- Threat-level presets.
- Quick-settings tile (SH02).

Out of scope:

| Item | Owner |
|---|---|
| pf rules, blocklists, DNS resolver configuration | SE02 (SE03 asks for route guards and per-route DNS) |
| Browser fingerprint defences, per-site isolation | IN01 |
| End-to-end message encryption and chat protocols | CO05 |
| Wi-Fi and interface set-up, MAC address randomisation switch | SY01 (SE03 shows its state) |
| Key storage | SE04 through `svc.keyring` |
| Running an AEGIS relay or an onion relay | Not a desktop function; a server guide in UT05 |
| Posture scoring | SE01 |

## 3. Users and top tasks

Personas:

- **Privacy-minded user**: wants some apps always anonymous and never to leak.
- **Journalist or researcher**: needs separate identities, bridges on censored networks, proof nothing leaks.
- **Consortium member**: uses AEGIS to message other members without revealing the relationship.
- **Remote worker**: uses a work VPN for some apps only.
- **Traveller**: turns everything onto a VPN on hotel networks.

| # | Task | Target |
|---|---|---|
| T1 | Turn a route on or off | 1 click (app or quick settings) |
| T2 | See whether traffic is protected right now | 0 steps (Overview) |
| T3 | Send one app through the onion route | 2 steps: Apps → route |
| T4 | Get a new identity | 1 click or `Ctrl+Shift+U` |
| T5 | Import a WireGuard profile | 2 steps: drop file → confirm |
| T6 | Run a leak test | 1 click, result in under 20 s |
| T7 | Add a bridge from a pasted line or QR code | 2 steps |
| T8 | See the circuit for a connection | 2 steps: Circuits → row |
| T9 | Switch threat level | 2 steps |
| T10 | Engage the route kill switch | 1 toggle, on by default for anonymous routes |
| T11 | Join an AEGIS roster | Wizard, 4 steps |
| T12 | Send a file through AEGIS with full relationship hiding | 3 steps: file → recipient → level |
| T13 | Chain VPN then onion route | 2 steps in the route editor |
| T14 | Start an app once under a temporary identity | Context action "Open with new identity" (2 steps) |

## 4. Features

### 4.1 Status and connection

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE03-F001 | Route cards | One card per route: state (Off, Connecting, On, Degraded, Blocked), uptime, bytes, apps using it | M | 1 |
| SE03-F002 | Connect and disconnect | Per route; connecting shows staged progress (bootstrap percentage for onion, handshake for VPN, first-hop link for mix) | M | 1 |
| SE03-F003 | Default route | The route for apps with no assignment: clear by default; changeable | M | 1 |
| SE03-F004 | Plain-language summary | "3 apps use the onion route. 1 app uses the work VPN. Everything else is in the clear." | M | 1 |
| SE03-F005 | What this hides | Per route, a fixed honest table: hides from the local network, from the internet provider, from the destination, from a global observer; yes, no or partly, with a sentence each | M | 1 |
| SE03-F006 | Exit address | The address a destination sees per route and identity; obtained from the route's own control data where possible, not by calling a web service | S | 2 |
| SE03-F007 | Auto-connect | Per route: at login, on untrusted networks (profile from SE02), never | S | 1 |
| SE03-F008 | Reconnect policy | Back-off with jitter; while down, the route guard blocks the route's apps | M | 1 |
| SE03-F009 | Health checks | Clock skew (onion routes need a correct clock), directory freshness, handshake age, roster validity, kill-switch armed | M | 1 |
| SE03-F010 | Quick-settings tile | State, default route, new identity, kill-switch toggle, in SH02 | M | 1 |
| SE03-F011 | Panel indicator | Icon and word per active route; a warning mark when an anonymous app is blocked by the guard | M | 1 |
| SE03-F012 | Notifications | Route down, guard engaged, bridge failing, roster expiring; never for normal circuit changes | S | 1 |

### 4.2 Onion route

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE03-F013 | Onion-routing helper | A sandboxed helper process provides a local stream interface used only by the network service | M | 1 |
| SE03-F014 | Bootstrap view | Stages and percentage, log lines on demand, common failure hints (clock, blocked network) | M | 1 |
| SE03-F015 | Circuit list | Each circuit: purpose, hops with nickname, country and flags, age, streams on it, identity that owns it | M | 2 |
| SE03-F016 | Per-connection circuit | From an app connection (data from SE02), show the circuit it rides | S | 2 |
| SE03-F017 | New circuit for a destination | Closes and rebuilds the circuit for one destination without changing identity | S | 2 |
| SE03-F018 | Stream isolation | Streams are isolated by app and by identity; optional isolation by destination | M | 1 |
| SE03-F019 | Guard information | Shows the entry guards in use, their age and when they rotate; explains why they are stable | S | 2 |
| SE03-F020 | Exit country preference | Optional; a warning explains that it reduces anonymity | C | 3 |
| SE03-F021 | Excluded relays | By fingerprint or country; same warning | C | 3 |
| SE03-F022 | Onion services (client) | Resolve and connect to onion addresses; client authorisation keys kept in `svc.keyring` | S | 2 |
| SE03-F023 | Onion services (host) | Publish a local service at an onion address for an app that declares it; keys in `svc.keyring` | C | 3 |
| SE03-F024 | DNS on the route | Names for onion-route apps resolve inside the route only (with SE02) | M | 1 |
| SE03-F025 | UDP stance | Apps on the onion route cannot send UDP; it is blocked, not sent in the clear | M | 1 |
| SE03-F026 | Bandwidth view | Read and written bytes per second on the route | S | 2 |
| SE03-F027 | Directory cache | Shown with age; kept on disk so start is fast; can be cleared | S | 2 |

### 4.3 Bridges and transports

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE03-F028 | Bridge list | Type, address, fingerprint, state, last success, source | M | 2 |
| SE03-F029 | Add bridge | Paste a bridge line, scan a QR code (camera portal), or read a file | M | 2 |
| SE03-F030 | Obfuscating transports | Transport helpers run as separate sandboxed processes; which ones ship is an open question (section 13) | S | 2 |
| SE03-F031 | Bridge test | Try each bridge and report reachability without using it for traffic | S | 2 |
| SE03-F032 | Censorship assistant | When bootstrap fails, a guided flow: check clock → try built-in bridges → ask for bridges out of band | S | 3 |
| SE03-F033 | Upstream proxy | Optional SOCKS5 or HTTP CONNECT proxy in front of the onion route for networks that require one | C | 3 |
| SE03-F034 | No automatic bridge fetch | Bridges are never fetched over the clear route without an explicit action | M | 2 |

### 4.4 VPN (WireGuard)

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE03-F035 | Profile list | Name, endpoint, state, last handshake, bytes, apps bound | M | 1 |
| SE03-F036 | Import profile | `.conf` file, pasted text or QR code; private key moved into `svc.keyring` at import and removed from the stored profile | M | 1 |
| SE03-F037 | Create profile | Generate key pair, show public key, form for peer public key, endpoint, allowed addresses, keepalive, optional pre-shared key | M | 1 |
| SE03-F038 | Edit profile | All fields; allowed-address editor with a "route everything" switch | M | 1 |
| SE03-F039 | Several profiles at once | More than one tunnel up; apps bind to one by name (`vpn:<profile>`) | S | 2 |
| SE03-F040 | Split tunnelling | By app (preferred) and by address range | M | 1 |
| SE03-F041 | DNS inside the tunnel | Resolver for the profile used only by apps on that profile (with SE02) | M | 1 |
| SE03-F042 | Handshake monitor | Latest handshake time; "stale" after 180 s; reconnect action | M | 1 |
| SE03-F043 | Roaming | Tunnel survives network change; guard blocks during the gap | M | 2 |
| SE03-F044 | Endpoint by name | Endpoint host names resolve once on the clear route through the system resolver; the result and time are shown | M | 1 |
| SE03-F045 | IPv6 handling | If the profile has no IPv6, IPv6 for bound apps is blocked, not leaked | M | 1 |
| SE03-F046 | Post-quantum pre-shared key | Optional pre-shared key field; a later option derives it by an ML-KEM exchange when the kernel supports it (PBSD plan item) | C | 3 |
| SE03-F047 | Export profile | Without the private key by default; with it only after authentication | S | 2 |
| SE03-F048 | Share public key | Copy or show as QR | S | 1 |
| SE03-F049 | MTU helper | Finds a working MTU by probe; shows the value used | C | 3 |
| SE03-F050 | Provider-neutral | No built-in provider list, no accounts | M | 1 |

### 4.5 Mix route (AEGIS)

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE03-F051 | Roster set-up wizard | Load the roster file, enter authority public keys and threshold, verify signatures, show members and layers | M | 2 |
| SE03-F052 | Roster view | Relays by layer (4 layers), jurisdiction, reputation where published, admission date; verification state of the roster | M | 2 |
| SE03-F053 | First hop and link | First-hop address, link key reference, handshake mode (legacy or Noise), state | M | 2 |
| SE03-F054 | Path view | The guard set and the current path through the layers; refresh epoch | S | 2 |
| SE03-F055 | Guard mitigation preset | Off, or one of the presets the client accepts (`adaptive_v4` preferred) | S | 3 |
| SE03-F056 | Jurisdiction diversity | Switch for `require_diverse_jurisdictions` and `max_per_jurisdiction`; labelled a soft filter | S | 3 |
| SE03-F057 | Pacing | Shows slot period τ and offered load ρ; blocks settings with ρ above 0.7 as the client does | M | 2 |
| SE03-F058 | Cover traffic state | Constant-rate emission on or off, cover seconds after send, bytes of cover against payload | M | 2 |
| SE03-F059 | Outbound queue | Messages waiting, estimated delivery time given τ and queue depth | M | 2 |
| SE03-F060 | Send message or small file | To a roster member; used by apps through the mix service and available here for a test send | S | 2 |
| SE03-F061 | Bulk transfer with dial | Choose a threat requirement; the app shows the dial level the negotiator would pick, the size ceiling and the wait for a batch round | S | 3 |
| SE03-F062 | Anonymity-set honesty | For bulk level 2, shows the current number of concurrent transfers against the 40 needed; below it, says the hiding is partial | M | 3 |
| SE03-F063 | Latency honesty | States the expected multi-second delay before any send; no "fast mode" | M | 2 |
| SE03-F064 | Internal against exit | Internal delivery is marked strong; anything leaving through an exit is marked weaker, in words | M | 2 |
| SE03-F065 | Raw mode hidden | The client's `--raw` and `--allow-high-rho` are never exposed in the GUI | M | 2 |
| SE03-F066 | KEM binding check | Requires roster KEM commitments on every hop; a failure stops the route | M | 2 |
| SE03-F067 | Key material | Hop registry and link keys stored through `svc.keyring`; never shown | M | 2 |
| SE03-F068 | Receive queue | Inbound messages handed to the owning app. Needs a receive path that was not found in the client crate (section 13) | S | 3 |
| SE03-F069 | Roster update | Load a newer signed roster; show the diff; refuse a roster signed by fewer authorities than the threshold | M | 2 |

### 4.6 Per-app routing

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE03-F070 | App route table | Every app with a network capability: declared routes, granted routes, assigned route, live state | M | 1 |
| SE03-F071 | Assign route | Clear, a VPN profile, onion; only routes the app's `net.route` grant allows; others link to AD05 | M | 1 |
| SE03-F072 | Strict binding | An app assigned to a route never falls back to clear; when the route is down its sockets are refused | M | 1 |
| SE03-F073 | Route chains | VPN → onion (hides onion use from the local network) and onion → VPN are separate, labelled choices with their trade-offs | S | 3 |
| SE03-F074 | Per-app isolation | Each app gets its own circuits by default | M | 1 |
| SE03-F075 | Launch with route | "Open with route…" and "Open with new identity" as shell context actions (SH01) | S | 2 |
| SE03-F076 | Route badge | The app's header-bar `[net]` badge shows the route name (design system); data supplied by SE03 | M | 2 |
| SE03-F077 | Rules by destination | Optional: for one app, send named destinations by a different route (e.g. local network in the clear) | C | 3 |
| SE03-F078 | Unroutable apps | Apps that need UDP or inbound connections are flagged when assigned to the onion route | M | 2 |
| SE03-F079 | Mix-capable apps | Lists apps that integrate with the mix service (e.g. CO05, IN03) and their roster identity | S | 3 |
| SE03-F080 | Bulk assign | Select several apps and set one route | S | 2 |

### 4.7 Identities

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE03-F081 | Identity list | Named identities with colour and icon; each has its own circuits, its own isolation token, and a list of app profiles bound to it | M | 2 |
| SE03-F082 | New identity (global) | Closes all circuits, drops isolation tokens, asks bound apps to clear session state, shows what was reset and what was not | M | 1 |
| SE03-F083 | New identity (one identity or one app) | Same, scoped | M | 2 |
| SE03-F084 | App reset contract | Apps that support identities implement a reset intent (clear cookies, caches, open connections); apps that do not are restarted in a fresh data profile or flagged | M | 2 |
| SE03-F085 | Temporary identity | A disposable identity and disposable app sandbox removed when the window closes | S | 2 |
| SE03-F086 | Identity-to-route binding | An identity can be pinned to a route; using it on another route is refused | S | 2 |
| SE03-F087 | Cross-identity guard | Warns when the same app profile is about to be used under two identities | S | 3 |
| SE03-F088 | Clock and locale hints | Shows which system properties apps can still read (time zone, language) and links to per-identity overrides in IN01 | C | 3 |
| SE03-F089 | Identity timeline | When each identity was created and reset; kept locally, can be turned off | C | 3 |
| SE03-F090 | No account, no sync | Identities never leave the machine | M | 2 |

### 4.8 Leak tests

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE03-F091 | Local leak test | Without contacting any server: for each anonymous app, check on the wire (through the SE02 connection monitor) that no packet left by another interface during a probe | M | 1 |
| SE03-F092 | DNS leak test | Issue a probe name from a test sandbox on each route and confirm the query did not appear on the clear resolver path | M | 1 |
| SE03-F093 | IPv6 leak test | Confirm IPv6 is carried or blocked, never bypassing | M | 1 |
| SE03-F094 | Kill-switch test | Drop the route on purpose for 3 s and confirm no packet escapes | M | 2 |
| SE03-F095 | Local network exposure | Check that bound apps cannot reach private ranges or multicast | S | 2 |
| SE03-F096 | Clock check | Compare the system clock with the route's own time source; large skew is a correlation risk and breaks bootstrap | S | 2 |
| SE03-F097 | External check (optional) | Ask a user-chosen check service what address it sees; off by default, needs `net.host` for that host | C | 3 |
| SE03-F098 | Scheduled tests | Run the local tests at connect and daily; failures alert and feed SE01 | S | 2 |
| SE03-F099 | Test report | Pass, fail, not tested per item, with evidence; export as Markdown | S | 2 |
| SE03-F100 | Device identifiers | Report hardware address randomisation state and host name sent on the network (data from SY01) | S | 2 |

### 4.9 Kill switch and presets

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE03-F101 | Route guard | For each anonymous route, a pf guard blocks its apps when the route is down (SE02 `RouteGuard`); on by default | M | 1 |
| SE03-F102 | Global route kill switch | "Nothing leaves except through route X"; persists across reboot | M | 1 |
| SE03-F103 | Guard state display | Armed, engaged, released; engaged shows which apps are held | M | 1 |
| SE03-F104 | Release needs authentication | And is audited | M | 1 |
| SE03-F105 | Threat presets | Standard, Safer, Safest: fixed bundles of settings (table in section 5.3) | M | 2 |
| SE03-F106 | Preset diff | Shows what a preset will change before applying | M | 2 |
| SE03-F107 | Custom preset | Save current settings as a named preset; export as signed JSON | C | 3 |
| SE03-F108 | Preset per network profile | e.g. Safest on Public networks (profile from SE02) | S | 3 |
| SE03-F109 | Panic action | One shortcut: engage global kill switch, new identity, lock the session | S | 2 |

### 4.10 Logging, basics, recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE03-F110 | Audit records | Route up and down, guard engage and release, route assignment, new identity, profile import go to `svc.audit`; destinations are never logged by SE03 | M | 1 |
| SE03-F111 | Helper logs | Per-helper log view with levels; scrubbed of addresses by default | S | 2 |
| SE03-F112 | No history by default | No connection history is kept by this app | M | 1 |
| SE03-F113 | Search | `Ctrl+K` over apps, profiles, bridges, settings, help | M | 1 |
| SE03-F114 | Read-only mode | Without authentication, view only | M | 1 |
| SE03-F115 | Helper crash recovery | A helper restarts under supervision; the guard holds traffic meanwhile | M | 1 |
| SE03-F116 | Command-line twin | `anonctl status|connect|new-identity|assign|leak-test` | S | 2 |
| SE03-F117 | Backup rules | Profiles and assignments are backed up; private keys stay in the keyring backup path; directory caches are not backed up | S | 2 |
| SE03-F118 | First-run explainer | Three screens: what each route hides, what it does not, what the user must still do | M | 1 |
| SE03-F119 | Help in place | `F1` opens the handbook page for the focused route or setting | S | 2 |

## 5. GUI

Layout pattern: sidebar + content; dashboard for the Overview.

### 5.1 Window map

```
Main window
├─ Sidebar: Overview · Apps · Identities · Onion route · VPN · Mix network · Leak tests · Presets · Logs
├─ Content
│   ├─ Overview (route cards, summary, guard state)
│   ├─ Apps (route table)
│   ├─ Identities (list + detail)
│   ├─ Onion route ── tabs: Status · Circuits · Bridges · Onion services · Advanced
│   ├─ VPN ── profile list + editor
│   ├─ Mix network ── tabs: Status · Roster · Path · Queue · Bulk transfers · Set-up
│   ├─ Leak tests ── tabs: Run · Results · Schedule
│   ├─ Presets
│   └─ Logs
├─ Inspector (F4): relay, circuit, profile or app properties
├─ Dialogs: Import VPN profile · New VPN profile · Add bridge · Join roster (wizard) · New identity
│           confirm · Apply preset (diff) · Release kill switch · Send through mix · Route chain editor
├─ Sheets: What this route hides · Censorship assistant · First-run explainer
└─ Popovers: Relay card · Handshake detail · Pacing detail
Shell surfaces (SH02/SH01): quick-settings tile, panel indicator, "Open with route…" context action
```

### 5.2 Main window

Overview at 1280×800.

```
+------------------------------------------------------------------------------------------+
| [=] Anonymity                        [ Search…  Ctrl+K ]                 [net][▁][▢][✕]  |
+--------------+---------------------------------------------------------------------------+
| > Overview   | (1) 3 apps use the onion route. 1 app uses "work" VPN. The rest are clear.|
|   Apps       |     Threat level: Safer  [Change…]            [New identity ⌃⇧U] (2)      |
|   Identities +---------------------------------------------------------------------------+
|   Onion route| (3)                                                                       |
|   VPN        | +--Clear----------+ +--Onion route----+ +--VPN: work-----+ +--Mix--------+|
|   Mix network| | ● On (default)  | | ● On   2 h 14 m | | ● On  hs 41 s  | | ○ Off       ||
|   Leak tests | | 11 apps         | | 3 apps · 6 circ | | 1 app          | | not set up  ||
|   Presets    | | ↓ 1.2 MiB/s     | | ↓ 210 KiB/s     | | ↓ 12 KiB/s     | |             ||
|   Logs       | | hides: nothing  | | guard: armed    | | guard: armed   | | [Set up…]   ||
|              | | [What it hides] | | [Disconnect]    | | [Disconnect]   | |             ||
|              | +-----------------+ +-----------------+ +----------------+ +-------------+|
|              +--------------------------------------+------------------------------------+
|              | (4) Kill switch                      | (5) Last leak test 09:02 today     |
|              | Route guards   ✔ armed (2 routes)    | ✔ No traffic outside routes        |
|              | Global         ( o) off              | ✔ DNS stays on route               |
|              | [Panic: block all + new identity]    | ✔ IPv6 blocked on VPN "work"       |
|              |                                      | ! Clock is 41 s off      [Run now] |
|              +--------------------------------------+------------------------------------+
|              | (6) What the active routes hide                                           |
|              |              local network   provider   destination   global observer     |
|              | Onion route  ✔ content+dest  ✔ dest     ✔ your address ✖ no               |
|              | VPN work     ✔ content+dest  ✔ dest     ◐ sees VPN's   ✖ no (and VPN sees)|
+--------------+---------------------------------------------------------------------------+
| (7) Onion: 6 circuits · VPN work: handshake 41 s ago · Guards armed · Clock ! 41 s       |
+------------------------------------------------------------------------------------------+
```

Callouts:

1. Summary sentence (`label`, live region) and threat level with a `link`.
2. Primary action: `button` "New identity".
3. Route cards: `gridView` of `panel`; each has state icon and word, counts, a `chart` sparkline, a `link` to the honesty sheet and one action `button`.
4. Kill-switch panel: `toggleSwitch` for global; guards read-only state; panic `button` outlined `--danger`.
5. Leak-test summary: `listView` with icon and word per item.
6. Honesty table: `tableView`, fixed text from string tables; `✔ ◐ ✖` always with words in the cell tooltip and a11y name.
7. Status bar.

States: nothing set up (cards show "Set up…"), connecting (staged progress in the card), degraded (card `--warn` with reason), guard engaged (red banner naming the held apps), read-only, narrow (cards one column).

### 5.3 Other views and dialogs

**Apps**

```
+---------------------------------------------------------------------------------------+
| Apps   [ Filter…        ]  Show (•) All ( ) Anonymous ( ) Clear       [Assign route ▾]|
+---------------------------------------------------------------------------------------+
| App            Allowed routes     Assigned        Identity    State                   |
| Browser        clear, vpn, tor    [ Onion     v]  research    ● 4 circuits            |
| Messenger      clear, tor         [ Onion     v]  personal    ● 1 circuit             |
| Mail           clear, vpn         [ VPN: work v]  —           ● tunnel up             |
| Feeds          clear, tor         [ Onion     v]  default     ⏸ held by guard         |
| Software       clear              [ Clear     v]  —           ● clear                 |
| Video          clear              [ Clear     v]  —           needs UDP: not onion    |
| Notes          no network         —               —           —                       |
+---------------------------------------------------------------------------------------+
| Browser is allowed the onion route by its manifest and your grant. [Open in Permissions]|
+---------------------------------------------------------------------------------------+
```

`tableView` with an inline `comboBox`; options outside the grant are disabled with a tooltip that names the missing capability.

**Onion route › Circuits**

```
+---------------------------------------------------------------------------------------+
| Onion route [Status] [Circuits] [Bridges] [Onion services] [Advanced]                 |
+-----------------------------------------------+---------------------------------------+
| Circuit  Identity  Purpose   Age   Streams    | Circuit 4 · identity "research"       |
|   1      default   general   12 m  2          |                                       |
|   2      personal  general   3 m   1          |  You ──▶ guard ──▶ middle ──▶ exit ──▶|
| > 4      research  general   1 m   3          |         relayA    relayM     relayX   |
|   5      research  onion svc 8 m   1          |         (stable)  DE         NL       |
|                                               |                                       |
|                                               | Streams: Browser → example.org:443 ×3 |
|                                               | [New circuit for this site] [Close]   |
+-----------------------------------------------+---------------------------------------+
```

Left `tableView`; right a hop diagram built from `panel` and `label` nodes (a small fixed layout, not `nodeGraph`), with relay cards in a popover.

**VPN profile editor**

```
+---------------------------------------------------------------------------------------+
| VPN   [Import…] [New profile…]                                                        |
+----------------------+----------------------------------------------------------------+
| > work      ● up     | Name [ work              ]            [Disconnect] [Export…]   |
|   home      ○ down   | Interface                                                      |
|   travel    ○ down   |   Public key  9fQ…c1=  [Copy] [QR]   Private key: in keyring 🔒|
|                      |   Addresses   [ 10.8.0.2/32, fd00:8::2/128 ]   MTU [ auto ]    |
|                      |   DNS         [ 10.8.0.1 ]  (used only by apps on this profile)|
|                      | Peer                                                           |
|                      |   Public key  [ hQ2…9s=                         ]              |
|                      |   Endpoint    [ vpn.example.org:51820 ]  resolved 09:01 clear  |
|                      |   Allowed     (•) Everything  ( ) Ranges [                ]    |
|                      |   Keepalive   [ 25 ] s      Pre-shared key  [ set 🔒 ] [Clear] |
|                      | Behaviour   [x] Route guard   [ ] Connect at login             |
|                      |             [x] Block IPv6 when the tunnel has none            |
|                      | Last handshake 41 s ago · ↓ 3.1 MiB ↑ 220 KiB                  |
+----------------------+----------------------------------------------------------------+
```

**Mix network › Status and Queue**

```
+---------------------------------------------------------------------------------------+
| Mix network [Status] [Roster] [Path] [Queue] [Bulk transfers] [Set-up]                |
+---------------------------------------------------------------------------------------+
| ● Linked to first hop · roster verified (3 of 3 authorities) · 24 relays in 4 layers  |
| Pacing: one packet every 0.35 s · load 0.42 of 0.70 allowed · cover on                |
| This route delivers in seconds, not milliseconds. It is for messages and files.       |
+--------------------------------------------+------------------------------------------+
| Outbound queue                             | Sent in the last hour                    |
| To          Size    Fragments  ETA         |  payload ▁▂▁▁▃▁▁   cover ▇▇▇▇▇▇▇         |
| member-07   2 KiB   3 of 5     2 s         |  The wire looks the same whether or not  |
| member-12   40 KiB  0 of 84    31 s        |  you are sending.                        |
+--------------------------------------------+------------------------------------------+
| Internal delivery (member to member): strong.   Exit to the open internet: weaker.    |
+---------------------------------------------------------------------------------------+
```

**Bulk transfer dialog**

```
+---------------------------------------------------------------------+
| Send a file through the mix network                                 |
| File   report.tar (38 MiB)          To [ member-12            v]    |
| How much must an observer not learn?                                |
| ( ) That you two are exchanging a file may be seen (fastest)        |
| ( ) Partly hidden: sizes bucketed, rounds aligned                   |
| (•) Fully hidden: uniform size, batched rounds, relay cover         |
| Level chosen: uniform + batched · size ceiling 64 MiB per round     |
| ! 11 transfers are in this round; about 40 are needed for full      |
|   hiding. With 11, hiding is partial.                               |
|                              [Cancel] [Wait for a fuller round] [Send]|
+---------------------------------------------------------------------+
```

**Leak tests › Run**

```
+---------------------------------------------------------------------------------------+
| Leak tests [Run] [Results] [Schedule]                         [Run all tests] 00:14   |
+---------------------------------------------------------------------------------------+
| Test                         Route         Result      Evidence                       |
| Traffic outside the route    Onion         ✔ pass      0 packets on em0 from 3 apps   |
| DNS stays on route           Onion         ✔ pass      probe not seen on clear path   |
| IPv6                         VPN work      ✔ pass      blocked (tunnel has no IPv6)   |
| Kill switch                  Onion         ✔ pass      0 packets in 3 s drop          |
| Local network exposure       Onion         ✔ pass      private ranges refused         |
| Clock                        system        ! warn      41 s off             [Fix…]    |
| External address check       —             not run     off (needs a check service)    |
+---------------------------------------------------------------------------------------+
```

**Presets** (the fixed bundles)

```
+---------------------------------------------------------------------------------------+
| Presets                                             Active: Safer     [Apply Safest…] |
+---------------------------------------------------------------------------------------+
| Setting                          | Standard        | Safer             | Safest       |
| Default route for new apps       | clear           | clear             | onion        |
| Route guards                     | on              | on                | on           |
| Global kill switch               | off             | on untrusted nets | always       |
| Isolation                        | per app         | per app + identity| + per site   |
| Bridges                          | off             | if blocked        | always       |
| Clear-route fall-back            | never           | never             | never        |
| Mix bulk requirement             | may be seen     | partly hidden     | fully hidden |
| Leak tests                       | at connect      | at connect + daily| + hourly     |
| Local network for anonymous apps | blocked         | blocked           | blocked      |
+---------------------------------------------------------------------------------------+
```

**New identity confirmation**

```
+---------------------------------------------------------------------+
| Start a new identity?                                               |
| Will be reset: 6 circuits · isolation tokens · Browser session      |
|   (cookies, cache) · Feeds connections                              |
| Will NOT be reset: Messenger account (it identifies you by design), |
|   files you downloaded, your entry guards                           |
|                                    [Cancel] [Start new identity]    |
+---------------------------------------------------------------------+
```

### 5.4 Menus

```
File      Import VPN Profile… Ctrl+O · New VPN Profile… Ctrl+N · Add Bridge… Ctrl+B ·
          Join Mix Roster… · Export Profile… · Export Leak Report… Ctrl+E · Close Ctrl+W · Quit Ctrl+Q
Edit      Copy Ctrl+C · Copy Public Key Ctrl+Shift+C · Find Ctrl+F · Preferences Ctrl+,
View      Overview Ctrl+1 · Apps Ctrl+2 · Identities Ctrl+3 · Onion Route Ctrl+4 · VPN Ctrl+5 ·
          Mix Network Ctrl+6 · Leak Tests Ctrl+7 · Presets Ctrl+8 · Logs Ctrl+9 ·
          Sidebar F9 · Inspector F4 · Refresh F5
Route     Connect/Disconnect Ctrl+Shift+C · Reconnect Ctrl+R · Set as Default · What This Route Hides…
          · New Circuit for Site Ctrl+Shift+L · Edit Route Chain…
Identity  New Identity Ctrl+Shift+U · New Identity for This App · New Temporary Identity Ctrl+Shift+T ·
          Rename F2 · Delete Delete
Safety    Kill Switch Ctrl+Shift+K · Panic Ctrl+Shift+Delete · Run Leak Tests Ctrl+L ·
          Apply Preset ▸ · Release Guard…
Help      Anonymity Help F1 · What Each Route Hides · Keyboard Shortcuts · About
```

### 5.5 Keyboard and pointer

- App shortcuts beyond the reserved set: `Ctrl+Shift+U` new identity, `Ctrl+Shift+K` kill switch, `Ctrl+Shift+Delete` panic, `Ctrl+L` leak tests, `Ctrl+R` reconnect, `Ctrl+B` add bridge.
- Context menu, app row: Assign route ▸, Assign identity ▸, New identity for this app, Open with temporary identity, Show connections (SE02), Open in Permissions (AD05).
- Context menu, circuit: Close, New circuit for this site, Copy relay fingerprint, Exclude relay….
- Context menu, VPN profile: Connect, Duplicate, Export…, Show QR of public key, Delete.
- Context menu, bridge: Test, Disable, Copy line, Delete.
- Drag and drop: drop a `.conf` file on VPN to import; drop a roster file on Mix network; drag apps onto a route card to assign; drag an app onto an identity to bind.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | No routes set up: Overview shows the first-run explainer and "Set up a route" |
| Loading | Route cards show "Connecting" with staged progress; nothing shows as protected until the route reports up |
| Error | Card shows the helper's reason in plain words and one suggested fix; details under "Show log" |
| Offline | No network: routes show "Waiting for network"; guards stay armed |
| Guard engaged | Red banner listing held apps; "Reconnect" primary |
| Locked | Presets or assignments locked by an administrator: lock icon and owner |
| Read-only | No authentication: view only |
| Helper missing | Onion route shows "Not installed" with a link to SY05; the route cannot be assigned |
| Narrow | Sidebar overlay; cards stack; tables drop the identity column |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar | `listView` |
| Route cards | `gridView` of `panel` |
| App table, circuits, bridges, roster, queue, tests | `tableView` |
| Inline route picker | `comboBox` |
| Sparklines and payload/cover chart | `chart` |
| Profile editor | form of `lineEdit`, `spinBox`, `checkBox`, `radioButton`; secret fields use the design-system secret field |
| Wizard | `dialog` with a page stack |
| Hop diagram | `panel` + `label` (fixed layout) |
| Logs | `codeEdit` read-only |
| Honesty sheet, explainer, help | `markdownView` |
| QR display | `image` |

Missing:

- **SE03-GAP-1**: QR code encode (display) and decode (from the camera portal frame) helper; no widget exists.
- **SE03-GAP-2**: sheet and wizard page-stack patterns (FUSE-WP-30).
- **SE03-GAP-3**: a world map view for relay countries is deliberately not planned; a table is used. Listed so nobody adds a map dependency later without need (FUSE-WP-48 exists if wanted).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | View models, wizards, presets, test orchestration |
| UI | FUSE UI | app host | Pages and dialogs |
| Route service | C++23 | system service `svc.route` | Owns route definitions, app-to-route assignment, identities and isolation tokens; supervises route helpers; tells the network service which route and token to use; asks SE02 for guards |
| VPN helper | C++23 | privileged helper, one per profile, holds only the tunnel control handle | Configures the kernel WireGuard interface, monitors handshakes; private key fetched from `svc.keyring` at bring-up |
| Onion-routing helper | Rust or C (third party, section 6.3) | own sandboxed process, own user, sockets only to relays and bridges | Builds circuits; offers a local stream interface reachable only by the network service; control interface reachable only by the route service |
| Transport helpers | third party | one sandboxed process per transport | Obfuscated links to bridges |
| Mix helper | Rust (`aegis-client` library and a thin service wrapper) | own sandboxed process | Holds a `PacedSession`; queues messages; exposes send, queue state and (later) receive over IPC |
| Leak-test runner | C++23 | helper in a disposable sandbox bound to the route under test | Sends probes; the verdict comes from the SE02 connection monitor's observation, not from the probe itself |
| Profile parser | C++23 | helper, capability mode, input handle only | Parses WireGuard `.conf`, bridge lines, roster files before anything else touches them |
| `anonctl` | ForgeScript | command line | Same bus interface |

```
 Anonymity app ── org.pbsd.route.v1 ──▶ route service ──▶ SE02 firewall service (RouteGuard, per-route DNS)
 (ForgeScript+FUSE UI)                     │   │   │
                                           │   │   └──▶ mix helper (Rust, aegis-client) ──▶ first hop (TCP)
                                           │   └──────▶ onion-routing helper ──▶ guards / bridges
                                           └──────────▶ VPN helper ──▶ kernel WireGuard interface
 apps ──socket request──▶ network service (FORGE-WP-21) ──asks──▶ route service: route + isolation token
          ◀── socket bound to: clear | tunnel interface | onion stream ──┘
 apps with mix integration ── org.pbsd.mix.v1 (send, receive) ──▶ mix helper
```

### 6.2 Data model

Store: `/var/db/pbsd/route/route.db` (SQLite, service-owned). Secrets are only references into `svc.keyring`.

| Entity | Fields |
|---|---|
| `route` | `id` text (`clear`, `tor`, `vpn:<name>`, `mix`), `kind` enum, `enabled` bool, `auto_connect` enum(never, login, untrusted), `guard` bool, `chain_over` text null, `state` enum, `since_ts` |
| `vpn_profile` | `name`, `private_key_ref` keyring id, `public_key` text44, `addresses` list, `dns` list, `mtu` u16 null, `peer_public_key` text44, `psk_ref` keyring id null, `endpoint_host`, `endpoint_port` u16, `endpoint_resolved` text, `endpoint_resolved_ts`, `allowed_ips` list, `keepalive_s` u16, `block_v6_if_absent` bool |
| `app_route` | `app_id`, `route_id`, `identity_id` null, `strict` bool (always true for anonymous routes), `set_ts`, `set_by` |
| `identity` | `id` u64, `name`, `colour`, `icon`, `pinned_route` null, `temporary` bool, `created_ts`, `last_reset_ts`, `isolation_token` blob32 (rotated on reset) |
| `identity_app` | `identity_id`, `app_id`, `data_profile` text |
| `bridge` | `id`, `transport` text, `addr`, `port`, `fingerprint`, `params` JSON, `enabled`, `last_ok_ts`, `source` enum(pasted, qr, file, builtin) |
| `onion_client_auth` | `onion_addr`, `key_ref` keyring id |
| `mix_config` | `first_hop_addr`, `ingress_link_key_ref`, `roster_path`, `roster_threshold` u8, `authority_pubkeys` list, `guard_preset` text null, `diverse_jurisdictions` bool, `max_per_jurisdiction` u8, `tau_ms` u32, `peak_rate` f32, `cover_secs` f32, `require_kem_binding` bool (names follow `ANONYMOUS:deploy/templates/client.production.toml` and the client's flags) |
| `mix_message` | `id`, `app_id`, `peer` text, `size`, `fragments_total`, `fragments_sent`, `state` enum(queued, sending, sent, failed), `enqueued_ts` |
| `bulk_transfer` | `id`, `peer`, `size`, `threat` enum(exposure_ok, partial, full), `dial` enum(l0, l1, l2), `round_peers` u16, `state` |
| `leak_result` | `id`, `ts`, `test` text, `route_id`, `result` enum(pass, fail, warn, not_run), `evidence` text |
| `preset` | `name`, `builtin` bool, `settings` JSON, `signer_fpr` null |

No table of destinations or connection history exists in this store.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| WireGuard `.conf` (INI-like) | yes | yes | Own parser | profile parser |
| WireGuard protocol | — | — | Kernel driver in the reference tree (`hbsd/src/sys/dev/wg`); configured through its control interface as `hbsd/src/usr.bin/wg` does | kernel; VPN helper configures |
| Onion-routing protocol | — | — | **Proposed third party.** Option A: Arti (Rust, MIT or Apache-2.0), fits the Rust-helper rule. Option B: the reference C implementation (BSD-3-Clause). Neither is in a repo read here; licences and offline build on HardenedBSD are Unverified | onion-routing helper |
| Bridge lines | yes | yes | Own parser | profile parser |
| Obfuscating transports | — | — | Third party; common ones are written in Go, which adds a toolchain. Unverified; open question | transport helper |
| AEGIS client configuration (TOML) | yes | yes | Generated by the route service from `mix_config`; read by the AEGIS code | mix helper |
| AEGIS roster (signed JSON) | yes | no | AEGIS code (`aegis-topology` roster verification) | mix helper; the app shows only what the helper reports |
| Sphinx packets, link handshake | — | — | AEGIS crates | mix helper |
| QR codes | yes | yes | Own encoder and decoder (small, well specified) | profile parser (decode); app host (encode) |
| Session bus `org.pbsd.route.v1`, `org.pbsd.mix.v1` | yes | yes | Schema compiler (G-08) | app host; services |
| Preset `.pbsd-anonpreset` (signed JSON) | yes | yes | Own; FORGE signing | app host verifies; service verifies again |
| Leak report (Markdown) | no | yes | Own | app host |

### 6.4 Integration

- Intents: handles `open` for WireGuard `.conf`, roster files, `.pbsd-anonpreset`; `open-with-route` and `open-with-identity` (shell context actions); URL scheme `anonymity:`; bridge lines pasted or scanned. Emits `open` to SE02, AD05, IN01, SY01; a new intent `identity-reset` to apps bound to an identity.
- Services used: `svc.keyring`, `svc.audit`, `svc.notify`, `svc.settings`, `svc.search`, `svc.portal.file`, `svc.portal.camera` (QR), `svc.backup`.
- Interfaces offered: `org.pbsd.route.v1` (`Status`, `Connect`, `Disconnect`, `Assign`, `NewIdentity`, `RouteFor(app) → route, token` for the network service, `RunLeakTests`, `ApplyPreset`), `org.pbsd.mix.v1` (`Send`, `QueueState`, `Receive` when it exists) for CO05 and IN03.
- Other apps: SE02 enforces guards and per-route DNS and supplies observation for leak tests; AD05 grants `net.route`; SH02 hosts the tile; SH01 offers "Open with route…"; IN01 implements `identity-reset` and its own per-site isolation; CO05 uses the onion route or the mix service as its anonymous transport; SE01 reads state and leak-test failures; SY01 owns hardware address randomisation.
- Backing repo: all AEGIS behaviour cited in section 1.1 is Verified from source; the mix helper wrapper, the IPC surface, the receive path and any FreeBSD build are Proposed.

## 7. Security and privacy

Capabilities requested by the app:

| Capability | Reason |
|---|---|
| `fs.app` | Window state, draft profiles |
| `fs.portal` | Import profiles and rosters, export reports |
| `net.none` | The app has no sockets; helpers under the route service do the networking |
| `svc.route` | Talk to the route service (new service, section 13) |
| `svc.firewall` | Read guard state and leak-test observations |
| `svc.keyring` | Store key references at import; the app never reads keys back |
| `svc.audit`, `svc.notify`, `svc.settings`, `svc.search`, `svc.backup` | Records, notices, preferences, search, backup rules |
| `svc.portal.file`, `svc.portal.camera` | Files; QR scanning |
| `dev.camera` | QR scanning only, prompt on first use, indicator shown |
| `proc.helper:profile-parser` | Parse hostile profile, bridge and roster text outside the app |

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| The user's network address | Destination, local network | Traffic bypasses the route (DNS, IPv6, UDP, reconnect gap) | Strict binding, route guards in pf, per-route DNS, UDP blocked on onion route, leak tests that observe the wire |
| Link between identities | Destination, exit | Shared circuits or shared app state | Isolation tokens per app and identity; reset contract; temporary identities in disposable sandboxes |
| Fact of using an anonymity network | Local network, provider | Protocol fingerprinting | Bridges and transports; VPN → onion chain; stated as a limit when neither is used |
| VPN and mix keys | Local malware, backup theft | Read keys from profile files | Keys live only in `svc.keyring`; profiles hold references; export without key by default |
| Route service | Hostile profile, roster or bridge text | Parser exploit | Profile parser in capability mode; roster signatures checked by AEGIS code in the mix helper |
| Onion or mix helper | Hostile relay | Exploit the helper | Helpers run as their own user in a sandbox with sockets only; cannot read user files; restart under supervision with the guard holding traffic |
| The user's understanding | Over-trust | Belief that a VPN gives anonymity, or that the mix route is instant or exit-safe | The honesty table and sentences are fixed text; the AEGIS limits from its README are shown where they apply |
| Timing correlation | Global passive observer | Correlate entry and exit traffic | Not defended on the onion and VPN routes (stated). Defended for internal mix traffic within AEGIS's stated empirical bounds |
| Leak-test honesty | The test itself | A probe that reports its own success | Verdicts come from an independent observer (SE02 connection monitor) |
| Metadata on disk | Device seizure | Reading history | No destination history kept; logs scrubbed; identity timeline optional |

Stored: route and profile definitions, assignments, identities, bridge list, leak results. Leaves the machine: traffic of the routes themselves; nothing else. No external "what is my address" call unless the user turns it on and names the service.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `default_route` | text | `clear` | Route for apps with no assignment |
| `guard.enabled` | bool | true | Route guards for anonymous routes |
| `guard.global` | enum | off | off, untrusted, always |
| `guard.persist` | bool | true | Keep the global kill switch across reboot |
| `preset.active` | text | `standard` | Active preset |
| `tor.auto_connect` | enum | never | never, login, untrusted |
| `tor.isolate_by_destination` | bool | false | Extra stream isolation |
| `tor.use_bridges` | enum | never | never, if_blocked, always |
| `tor.exit_countries` | list | empty | Preference; reduces anonymity |
| `tor.cache_dir_keep` | bool | true | Keep directory cache between sessions |
| `vpn.stale_handshake_s` | int | 180 | Handshake age that counts as stale |
| `vpn.block_v6_if_absent` | bool | true | Block IPv6 when a tunnel has none |
| `vpn.export_with_key` | bool | false | Include the private key on export (asks every time) |
| `mix.tau_ms` | int | 350 | Slot period (client default 0.35 s) |
| `mix.cover_secs` | float | 2.0 | Cover after the last fragment (client default) |
| `mix.peak_rate` | float | 2.0 | Peak enqueue rate for the load check (client default) |
| `mix.guard_preset` | text | empty | Guard mitigation preset |
| `mix.diverse_jurisdictions` | bool | false | Soft path filter |
| `mix.bulk_default_threat` | enum | full | Default requirement for bulk transfers |
| `identity.reset_restarts_apps` | bool | true | Restart apps that lack the reset intent |
| `identity.keep_timeline` | bool | false | Keep the identity timeline |
| `leak.run_at_connect` | bool | true | Run local tests at connect |
| `leak.schedule` | enum | daily | off, daily, hourly |
| `leak.external_service` | text | empty | Optional check service host |
| `log.level` | enum | notice | Helper log level |
| `log.scrub_addresses` | bool | true | Remove addresses from helper logs |
| `notify.route_down` | bool | true | Notify when a route drops |

## 9. Accessibility and languages

- Route state is always a word with an icon; the honesty table cells have full-sentence a11y names ("Onion route: the destination does not see your address").
- The hop diagram has a list twin: "Hop 1, guard, relayA. Hop 2, middle, relayM, Germany."
- New identity and kill switch announce their result in a live region, including what was not reset.
- Relay nicknames, fingerprints and keys are never translated or mirrored; they are read character by character on request.
- The honesty texts are legal-sensitive; translations are reviewed, and a missing translation falls back to English, never to an empty cell.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| First frame | under 300 ms cold |
| `RouteFor(app)` answer to the network service | under 50 µs cached |
| VPN bring-up | under 1 s to first handshake on a reachable peer |
| Onion bootstrap | under 15 s with a warm directory cache on a normal link; progress shown throughout |
| New identity | circuits closed and tokens rotated in under 500 ms; app resets reported as they finish |
| Guard engage after route loss | under 100 ms (rule already loaded, matched on route state) |
| Local leak tests | full set under 20 s |
| Mix helper idle | constant-rate by design: one packet per slot; CPU under 1% of one core at τ = 0.35 s |
| App memory | under 100 MiB; route service under 30 MiB; helpers measured separately |

## 11. Testing and proof

Test plan (targets; nothing has run):

- **Unit**: profile parser, bridge-line parser, assignment rules (never wider than the grant), preset application, identity token rotation.
- **Golden**: `.fuseui` renders of every page, including each state of the route cards.
- **Scripted UI**: the 14 top tasks through the a11y tree.
- **Fuzz**: WireGuard `.conf` parser, bridge lines, QR decoder, preset JSON, bus messages. AEGIS's own fuzz targets are reused for its parsers (the repo reports libFuzzer targets for Sphinx and roster code; not run here).
- **Network tests on an isolated test network**: an app bound to each route never emits a packet on another interface, including during helper kill, interface flap, suspend and resume, and system start; DNS, IPv6 and UDP cases; chains.
- **Isolation tests**: two identities never share a circuit; after reset, a test destination sees a different exit and no shared token.
- **Mix helper**: reuse of AEGIS's testnet tests (`ANONYMOUS:crates/aegis-node/tests/tcp_testnet.rs`) driven through the IPC wrapper; the wire rate must stay constant with and without payload.
- **Build**: the AEGIS workspace and the chosen onion-routing helper build offline on HardenedBSD from vendored sources.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Assignment rule | An app is never assigned a route outside its granted `net.route` set | T1 | Pure function; PRISM | none known |
| Strict binding | For an anonymous route, the service never returns `clear` as a fall-back | T1 | State-machine model (TLA+) of route up, down, restart | Network service is a separate proof (FORGE-WP-21) |
| Guard timing | No interval exists in which the route is down and the guard is not matching | T2 | Model of the pf rule keyed on route state; system tests | pf behaviour assumed |
| Identity isolation | Tokens are unique per (app, identity) and rotate on reset | T2 | Contracts; bounded proof | Helper honouring tokens is trusted |
| Profile parser, QR decoder | No runtime errors on any input | T2 | Contracts, fuzzing | none known |
| VPN helper | Configures only the interface it was started for | T2 | Contracts on the control path | Kernel driver trusted |
| Mix helper wrapper | Does not weaken pacing: every send goes through `PacedSession` | T3 | Code review, wire-rate test | Rust code is outside PRISM's C++ scope; AEGIS claims are empirical |
| Onion-routing helper | Third-party code | T4 | Sandbox, fuzzing of our IPC edge | Not our code |
| App logic | Safe subset | T3 | ForgeScript checker, unit tests | Interpreter not verified |

## 12. Build plan

**Phase 1 — usable core**

| ID | Work item | Depends on |
|---|---|---|
| SE03-W01 | Route mechanism decision with the OS and SE02: how a socket is bound to a route (per-app routing table, network domain or stream proxy) | FORGE-WP-21, SE02-W16, PBSD network package |
| SE03-W02 | `svc.route` skeleton, schema `org.pbsd.route.v1`, `RouteFor` for the network service | G-08, G-09, FORGE-WP-25, FORGE-WP-26 |
| SE03-W03 | VPN helper on the kernel WireGuard interface; profile store with keyring references; profile parser | WireGuard on PBSD, SE04 / `svc.keyring`, FORGE-WP-27, FORGE-WP-28 |
| SE03-W04 | Route guards and per-route DNS with SE02 | SE02-W09, SE02-W14 |
| SE03-W05 | Choose, vendor and sandbox the onion-routing helper; local stream interface to the network service | SE03-W01, Rust or C offline build on HardenedBSD |
| SE03-W06 | App: Overview, Apps, VPN, Onion route status; first-run explainer; honesty table | G-01, G-02, G-07, FUSE-WP-12, FUSE-WP-28 |
| SE03-W07 | New identity (global), per-app isolation tokens | SE03-W05 |
| SE03-W08 | Local leak tests using the SE02 connection monitor | SE02-W07 |
| SE03-W09 | Quick-settings tile and panel indicator | SH02 |

**Phase 2 — full feature**

| ID | Work item | Depends on |
|---|---|---|
| SE03-W10 | Identities: list, binding, scoped reset, temporary identities, `identity-reset` intent | SE03-W07, IN01, SH04 |
| SE03-W11 | Circuits view, guard information, onion services (client) | SE03-W05 |
| SE03-W12 | Bridges: list, add by paste, file, QR; transport helpers | SE03-W05, SE03-GAP-1, transport decision |
| SE03-W13 | Build the AEGIS workspace offline on HardenedBSD; replace the `keyring` crate back end with `svc.keyring` | Rust toolchain vendored, FORGE-WP-03 style offline build |
| SE03-W14 | Mix helper: service wrapper around `PacedSession`, `org.pbsd.mix.v1` send and queue state | SE03-W13 |
| SE03-W15 | Mix pages: set-up wizard, roster, path, status, queue | SE03-W14 |
| SE03-W16 | Presets, preset diff, panic action | SE03-W04 |
| SE03-W17 | Kill-switch test, scheduled tests, report export, SE01 feed | SE03-W08, SE01 |
| SE03-W18 | "Open with route…" and route badge data | SH01, FUSE-WP-29 |
| SE03-W19 | `anonctl`, several VPN profiles at once, roaming | SE03-W03 |

**Phase 3 — advanced**

| ID | Work item | Depends on |
|---|---|---|
| SE03-W20 | Route chains | SE03-W01 |
| SE03-W21 | Mix bulk transfers with the dial and anonymity-set display | SE03-W14, `aegis-negotiator` integration |
| SE03-W22 | Mix receive path and delivery to apps | AEGIS upstream work (receive not found in `aegis-client`) |
| SE03-W23 | Censorship assistant, upstream proxy | SE03-W12 |
| SE03-W24 | Onion services (host) | SE03-W11 |
| SE03-W25 | Custom and signed presets, preset per network profile | SE04, SE02 |
| SE03-W26 | Post-quantum pre-shared key exchange for WireGuard | PBSD plan item (post-quantum crypto in WireGuard) |

## 13. Open questions and risks

1. **AEGIS is a consortium message mix network, not a general anonymous route** (section 1.1). It cannot replace an onion-routing network for web traffic. The catalogue line "front end for ANONYMOUS / AEGIS" is met for messages and files only. The owner should confirm that an onion-routing helper from a third party is acceptable, or accept that the `tor` route will not exist.
2. **`net.route` has no value for the mix network.** Proposed platform change: add `net.route:mix` or, better, a separate capability `svc.mix`, because the mix route is a message service and not a socket route.
3. `svc.route` and `svc.mix` are not in PLATFORM.md section 8. Need adding.
4. The route mechanism (how the OS binds an app's sockets to a tunnel or a stream proxy) is undecided; FORGE-WP-21 marks it Unverified. SE03-W01 blocks most of Phase 1.
5. Rust at build time: AEGIS needs a stable Rust toolchain (`ANONYMOUS:rust-toolchain.toml`) and about a dozen external crates (`tokio`, `snow`, `ml-kem`, `x25519-dalek`, `blsttc` and others). All must be vendored for offline builds; their HardenedBSD support is Unverified. This is the same class of problem as G-04.
6. AEGIS has no FreeBSD-specific code and its key protection uses DPAPI or the `keyring` crate; behaviour of that crate on HardenedBSD is Unverified. Plan: replace with `svc.keyring`.
7. No client receive path was found in `aegis-client`. Until one exists the mix route is send-only from the desktop.
8. AEGIS needs a consortium: a roster, authorities and relays run by others. A single user cannot use it alone. The GUI must not suggest otherwise.
9. Obfuscating transports are commonly written in Go; shipping them means a third toolchain. Alternative: support only bridges that need no transport at first.
10. Identity reset depends on apps honouring the reset intent. Apps that do not are restarted in a new data profile, which loses state; this must be clear in the confirmation.
11. Clock: onion routing needs a correct clock, and time sync itself is a clear-route service. The time-sync path on each preset needs a decision with SY01.
12. Legal and safety wording of the honesty texts needs review before release.
