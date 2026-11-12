<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Compatibility

By Odin Loch

| | |
|---|---|
| ID | `VM03` |
| Bundle | `org.pbsd.compatibility` |
| Category | virt |
| Priority | P2 |
| Backing repos | ParanoidBSD (plan items P4-01 to P4-07, `docs/migration/LINUXULATOR.md`, `hbsd/src/sys/compat/linux`) |
| Status | Planned. Nothing built. |

## 1. Purpose

Compatibility installs and runs Windows, Linux and Android applications on PBSD through the kernel personalities, without a virtual machine where that is possible. It is for people who need one program that has no native version. It replaces Wine front ends (Bottles, Lutris, PlayOnLinux), Linux app runners, and Android emulators.

Foreign programs do not know about capabilities. This app's main job is to give each of them a sandbox, map what they ask for onto PBSD capabilities and portals, and show the user the result in the same terms as a native app.

## 2. Scope

### In scope

- Library of foreign apps with launchers in the desktop shell.
- Environments: Windows prefixes, Linux runtimes, Android instances; one per app by default.
- Installer runs with a reviewable change set and rollback.
- Capability mapping: files, network, devices, clipboard, notifications, inter-app links.
- Graphics, audio and input bridges and their settings.
- Local compatibility database with known-good settings and user notes.
- Diagnostics: logs, missing-interface reports, crash reports.
- Fallback to a VM in VM01 when a personality cannot run the app.

### Out of scope

| Item | Owner |
|---|---|
| The personalities and translators themselves (kernel work) | Plan P4-01 to P4-07 |
| Full virtual machines | VM01 (called for fallback) |
| Server-style Linux containers and stacks | VM02 (its service supplies the Linux and Android domains) |
| The launcher, task bar and "Foreign apps" category | SH01 |
| Portals and prompts shown to the user | SH04 |
| Grant management for all apps | AD05 |
| Native packages, runtime component packages, database updates | SY05 |
| Game store clients as a feature | not planned; they are ordinary foreign apps |
| Foreign device drivers (NT kernel drivers in an IOMMU sandbox) | VM05 shows them; the plan lists them last |

## 3. Users and top tasks

Personas:

- **Robin, office user.** Needs one Windows accounting program. Wants a normal launcher icon and working printing.
- **Mika, gamer.** Runs Windows games; cares about graphics translation, controllers, and per-game settings.
- **Dara, developer.** Runs proprietary Linux tools shipped as tarballs, AppImages or `.deb` files.
- **Tess, phone-app user.** Wants two Android apps on the desktop with no vendor account and tight permissions.
- **Ines, analyst.** Runs an unknown Windows tool once with no network and no file access.

| # | Task | Target |
|---|---|---|
| T1 | Install a Windows program from its installer | 4 steps: drop `.exe`/`.msi`, accept proposed environment, run installer, approve change set |
| T2 | Run a portable Windows or Linux executable once | 2 steps: drop the file, "Run once" |
| T3 | Install an Android app from an APK | 3 steps: drop `.apk`, review mapped permissions, "Install" |
| T4 | Start an installed foreign app | 1 step from SH01 or `Enter` here |
| T5 | Give an app access to one folder | 2 steps: Access tab, "Add folder" (portal) |
| T6 | Cut an app off from the network | 1 step: toggle in the inspector |
| T7 | Apply known-good settings | 1 step: "Apply suggested settings" on the database card |
| T8 | Find out why an app fails | 2 steps: "Diagnose", read the ranked findings |
| T9 | Undo a broken update or installer | 2 steps: Snapshots tab, "Roll back" |
| T10 | Move a failing app to a VM | 3 steps: "Run in a VM instead", pick VM, confirm |
| T11 | Remove an app and all its data | 2 steps: Remove, choose "with its environment" |
| T12 | See everything a foreign app can reach | 1 step: Access tab |

## 4. Features

Status of the backend (Verified from `docs/plan/PBSD_PLAN.md`): P4-01 plans a per-thread personality tag chosen at exec, with a dispatcher that routes to Linux, BSD, NT or Binder handlers. P4-02 plans the Linux syscall personality built from `hbsd/src/sys/compat/linux`. P4-04 plans the NT personality by moving the Wine server object model into the kernel, with "pbsd bifrost (VM fallback) — keep". P4-05 plans an Android personality that "runs AOSP system image in a container" with app sandbox UIDs mapped to capabilities. P4-06 plans cross-architecture translation. P4-07 plans checked gateways around foreign code. None of `pbsd/kernel/personality/`, `pbsd/lib/ntdll/` exists in the tree today. `docs/migration/LINUXULATOR.md` keeps the HardenedBSD linuxulator as an optional package. All features below are Proposed.

### 4.1 Library

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM03-F001 | App library | Grid or list of installed foreign apps: icon, name, kind badge (Windows, Linux, Android), environment, last run, rating from the database, network and device badges | M | 1 |
| VM03-F002 | Sidebar | All, Windows, Linux, Android, Running, Environments, Components, Database, Jobs | M | 1 |
| VM03-F003 | Running view | Foreign processes grouped by app with CPU, memory, uptime; Stop and Force stop | M | 1 |
| VM03-F004 | Search and filter | By name, publisher, kind, environment, rating, "has network", "has file access" | M | 1 |
| VM03-F005 | Collections and tags | User collections; an app may be in several | S | 2 |
| VM03-F006 | App page | Tabs: Overview, Access, Settings, Environment, Snapshots, Diagnostics | M | 1 |
| VM03-F007 | Rename and icon | Change display name and icon; the icon is extracted from the executable (PE resource, `.desktop` icon, APK) by a helper and re-encoded to PNG | S | 1 |
| VM03-F008 | Several entry points | An app can have more than one launcher (main program, configuration tool, uninstaller hidden by default) | S | 2 |
| VM03-F009 | Launch options | Arguments, working directory, environment variables, per launcher | M | 1 |
| VM03-F010 | Play time and history | Last run, run count, total run time per app; local only; can be turned off and cleared | C | 3 |
| VM03-F011 | Remove | Remove launcher only, or the app with its environment (held snapshot for 14 days), or for ever | M | 1 |
| VM03-F012 | Duplicate | Copy an app with its environment (ZFS clone) to try settings safely | S | 2 |

### 4.2 Adding apps and installers

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM03-F013 | Add wizard | Pages: What is it, Environment, Access, Run. The file dropped decides the first page | M | 1 |
| VM03-F014 | File recognition | `probe-helper` identifies PE (32/64-bit, GUI/console, .NET, installer family: MSI, NSIS, Inno Setup, InstallShield), ELF (arch, libc, interpreter), AppImage (type 1/2), shell installers, `.deb`, `.rpm`, tarballs, Flatpak bundles (listed as unsupported in phase 1), APK, split APK sets, XAPK | M | 1 |
| VM03-F015 | Signature display | Authenticode signer and chain state for PE and MSI, APK signing certificate digest (v2/v3/v4 schemes), detached signatures for Linux files; shown as verified, unsigned, or invalid; verification is offline against the local trust store | M | 2 |
| VM03-F016 | Hash and provenance | SHA-256 of the dropped file, where it came from (download record from IN03 if present), stored with the app | M | 1 |
| VM03-F017 | Install in a staging snapshot | The environment is snapshotted; the installer runs with no network by default; when it exits the app computes a change set | M | 1 |
| VM03-F018 | Change-set review | Files added and changed, registry keys (Windows), services and scheduled tasks requested, autostart entries, file associations, shortcuts, fonts; the user accepts all, some, or rolls back | M | 2 |
| VM03-F019 | Launcher detection | Shortcuts (`.lnk`), `.desktop` files and Android launcher activities found in the change set become proposed launchers | M | 1 |
| VM03-F020 | Autostart and services | Foreign autostart entries and Windows services are off unless the user enables each; listed on the Environment tab | M | 2 |
| VM03-F021 | Run once | Run a file in a disposable environment clone that is destroyed at exit; no network, no file access unless added | M | 1 |
| VM03-F022 | Portable apps | Register a folder (granted through the portal) as an app without running an installer | S | 1 |
| VM03-F023 | Linux packages | `.deb` and `.rpm` are unpacked into the Linux runtime's overlay without running maintainer scripts by default; scripts can be run inside the sandbox after review | S | 2 |
| VM03-F024 | AppImage | Run type 2 AppImages by mounting the embedded image through a sandboxed read-only file-system helper, or extract | S | 2 |
| VM03-F025 | Installer network | If an installer needs the network, the user allows it for this run; contacted hosts are listed afterwards | M | 2 |
| VM03-F026 | Updates of foreign apps | Self-updaters run inside the sandbox; the app offers "snapshot before update"; an "Update from file" action reruns a newer installer on the same environment | S | 2 |
| VM03-F027 | Installer recipes | Optional signed recipe from the database: which components to add first, installer switches for a quiet install, known post-install fixes | S | 3 |
| VM03-F028 | Unattended install | With a recipe, run the installer without interaction and show only the change set | C | 3 |

### 4.3 Environments

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM03-F029 | One environment per app | Default: each app has its own environment (prefix, runtime overlay or Android data) on its own ZFS dataset; sharing is an explicit choice | M | 1 |
| VM03-F030 | Environment list | Name, kind, base version, size, apps using it, last snapshot | M | 1 |
| VM03-F031 | Base versions | Bases (Windows runtime build, Linux runtime images, Android system image) are signed packages installed by SY05; several versions side by side; an environment pins one | M | 1 |
| VM03-F032 | Thin environments | An environment is a ZFS clone of its base; only differences use space | M | 1 |
| VM03-F033 | Change base version | Switch an environment to another base version after an automatic snapshot | S | 2 |
| VM03-F034 | Snapshots | Manual, before install, before update, before base change; roll back; retention | M | 1 |
| VM03-F035 | Browse files | Open the environment's file tree in SY02 through a handle; the virtual drive C:, the Linux overlay, or Android app data | S | 1 |
| VM03-F036 | Size and cleanup | Size breakdown (base shared, unique, snapshots, caches); clear shader and temp caches | S | 2 |
| VM03-F037 | Export and import | Environment as a ZFS stream or tar with the app definition; signed optionally; import verifies | S | 2 |
| VM03-F038 | Templates | Environment templates (base + components + settings), bundled and user-made | S | 2 |
| VM03-F039 | Shared environment | Several apps in one environment (for suites that must see each other); the Access tab then shows the union and warns | S | 2 |
| VM03-F040 | Encryption | Environment datasets may be encrypted with a key in `svc.keyring` | C | 3 |
| VM03-F041 | Components | Optional runtime parts as signed local packages: fonts, language runtimes, media codecs, graphics translation layers; list with version, licence, installed-in count | M | 2 |
| VM03-F042 | No arbitrary downloads | Components come only from signed packages through SY05; there is no script that fetches redistributables from vendor URLs | M | 2 |
| VM03-F043 | Open a shell | Open SY03 inside the environment's sandbox (Windows command prompt, Linux shell, `adb`-like shell for Android) | S | 2 |

### 4.4 Windows apps

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM03-F044 | Windows version reported | Per app: version string the personality reports (10, 11, older) | M | 1 |
| VM03-F045 | Architecture | 64-bit, 32-bit through WoW64-style thunks; 16-bit not supported and said so | M | 1 |
| VM03-F046 | Library overrides | Per app: prefer built-in or app-supplied version of a DLL; list editor with search | M | 2 |
| VM03-F047 | Registry editor | Tree and value editor for the environment's registry hives; import `.reg` after review; export | S | 2 |
| VM03-F048 | Drives | Map drive letters to granted folders (each a portal grant); C: is the environment; no drive maps the user's home by default | M | 1 |
| VM03-F049 | Fonts | Choose which host fonts the app sees; add fonts to one environment; font smoothing settings | S | 2 |
| VM03-F050 | Virtual desktop | Run the app inside one host window of fixed size (for programs that change resolution or misbehave with window management) | S | 2 |
| VM03-F051 | Process control | List Windows processes and services of the environment; end a process; end all | M | 1 |
| VM03-F052 | COM and shell links | "Open with" from a Windows app goes to the host intent service only for types the user enabled; default is inside the environment | S | 2 |
| VM03-F053 | Printing | Windows print jobs become PDF spool jobs through the print portal | S | 2 |
| VM03-F054 | .NET and scripting hosts | Shown as components; script hosts (cmd, PowerShell-compatible, wscript) off unless needed | S | 3 |
| VM03-F055 | Kernel drivers | An app that installs a kernel driver (anti-cheat, copy protection, device driver) is detected in the change set; the driver is not loaded; the app explains and offers the VM fallback | M | 2 |
| VM03-F056 | Dual-mapped JIT notice | Apps that generate code at run time work only through the kernel's W^X-safe dual mapping (plan P4-06 text); the app page shows whether the program needed it | S | 3 |

### 4.5 Linux apps

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM03-F057 | Runtimes | Linux runtime bases (a glibc-based and a musl-based userland with common desktop libraries) as signed packages or as OCI images taken from VM02's image store | M | 1 |
| VM03-F058 | Library resolution report | Before first run, list the shared libraries the ELF needs and which are missing from the runtime; propose a runtime that satisfies them | M | 2 |
| VM03-F059 | Window system bridge | A per-sandbox filtering Wayland proxy; X11 programs get a private nested X server per app so one X client cannot read another's input or windows | M | 1 |
| VM03-F060 | Desktop portal bridge | Linux portal requests (file chooser, open URI, print, screenshot, notification, inhibit) are translated to PBSD portals (SH04); unknown requests are refused | M | 2 |
| VM03-F061 | Message bus filter | A private session bus per app; only allow-listed names reach host bridges | M | 2 |
| VM03-F062 | `/proc`, `/sys` view | The app sees only its own processes and a minimal hardware view; GPU nodes only with a device grant | M | 1 |
| VM03-F063 | Home directory | A private home inside the environment; XDG folders map to nothing on the host until the user adds a folder | M | 1 |
| VM03-F064 | Environment variables | Editor with presets (scale factor, toolkit back end, locale) | S | 1 |
| VM03-F065 | Syscall coverage report | Counts of unimplemented or refused system calls seen in a run, from the personality's counters | S | 2 |
| VM03-F066 | Which Linux layer | Shows whether the app runs on the new Linux personality (P4-02) or the retained linuxulator (`docs/migration/LINUXULATOR.md`); selectable while both exist | S | 2 |
| VM03-F067 | Command-line tools | Linux console programs get launchers that open SY03 with the sandbox attached | S | 1 |
| VM03-F068 | Set-id and capabilities | Set-id bits and Linux file capabilities inside the runtime have no host meaning; a program that needs them is flagged | M | 2 |

### 4.6 Android apps

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM03-F069 | Android instance | The AOSP system image runs in a container (plan P4-05); one shared instance by default, or one instance per app for isolation | M | 2 |
| VM03-F070 | APK install | Parse the APK in `probe-helper`; verify signature schemes v2, v3 (v1 as fallback with a warning); show package name, version, min and target SDK, native ABIs, signer digest | M | 2 |
| VM03-F071 | Split and bundle sets | Install split APKs and XAPK/APKS sets after checking all parts share one signer | S | 3 |
| VM03-F072 | Permission mapping screen | Each Android permission requested is shown with the PBSD capability it maps to and a switch; dangerous permissions are off at install | M | 2 |
| VM03-F073 | Run-time permission prompts | An Android permission request at run time becomes a shell-drawn PBSD prompt naming the app | M | 2 |
| VM03-F074 | Windows | Each task is a host window; freeform resize; orientation lock; density setting; back gesture mapped to `Alt+Left` and a title-bar button | M | 2 |
| VM03-F075 | Input mapping | Keyboard and pointer to touch; optional key-to-touch maps per app (tap, swipe, joystick areas) with an overlay editor | S | 3 |
| VM03-F076 | Notifications | Android notifications become `svc.notify` notifications with their actions | S | 2 |
| VM03-F077 | Intents and sharing | Share and view intents leaving the app go to the host intent service only for enabled types; incoming host shares appear as Android share targets | S | 3 |
| VM03-F078 | No vendor services | No proprietary account or push services are shipped; apps that require them are flagged from the database; a "no network identifiers" profile blanks advertising id, serial and telephony ids | M | 2 |
| VM03-F079 | Fake or empty providers | Contacts, calendar, location and telephony providers return empty sets unless the user maps them (location needs `dev.location`) | M | 2 |
| VM03-F080 | Background limits | Background execution of Android apps is off when no window is open unless the user enables it per app | M | 2 |
| VM03-F081 | App data tools | Clear cache, clear data, export app data, per app | S | 2 |
| VM03-F082 | Interpreter mode | The Android runtime runs with its interpreter or ahead-of-time compiled code so that no W+X mapping is needed; a per-app note shows which | S | 3 |

### 4.7 Capability mapping and sandbox

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM03-F083 | Generated manifest | Each foreign app gets a manifest with the schema of PLATFORM.md section 12 (`id` `foreign.<kind>.<hash>`), so the shell, AD05 and SE02 treat it like a native app | M | 1 |
| VM03-F084 | Default deny | New apps start with: own environment, display, audio output, clipboard off, no network, no user files, no devices | M | 1 |
| VM03-F085 | Access tab | One list of grants with switches: folders, network (off, named hosts, any; route clear/VPN/Tor), clipboard, camera, microphone, screen capture, location, USB classes, printing, GPU compute, notifications, background run | M | 1 |
| VM03-F086 | File dialogs become portals | Foreign open/save dialogs are replaced by the host file portal; the chosen file is granted for the session or remembered | M | 2 |
| VM03-F087 | Folder grants | Folders appear at a stable path inside (drive letter, mount point, Android storage volume); read-only by default | M | 1 |
| VM03-F088 | Network policy | Per app allow list of hosts and ports learned from a "learning run" (connections are logged, not allowed, then reviewed) or typed; enforced by SE02 | M | 2 |
| VM03-F089 | Clipboard policy | Off, text only, text and images; secret-flagged entries never cross; foreign apps cannot read the clipboard while unfocused | M | 2 |
| VM03-F090 | Device prompts | Camera, microphone, screen, location and USB requests from foreign APIs raise the normal shell prompts with indicators | M | 2 |
| VM03-F091 | Screen capture guard | Foreign screenshot and window-enumeration APIs see only the app's own windows unless `dev.screen` is granted | M | 2 |
| VM03-F092 | Input guard | No global key hooks or input injection outside the app's own windows | M | 1 |
| VM03-F093 | Inter-app isolation | Two foreign apps in different environments cannot see each other's processes, windows, files, registry or bus names | M | 1 |
| VM03-F094 | Reach summary | Plain-language "can reach / cannot reach" card on the Overview tab and in the launcher tooltip | M | 1 |
| VM03-F095 | Foreign security models | NT security descriptors, Linux uids and Android UIDs are mapped to the app's capability set (plan P4-04, P4-05); the Access tab shows the PBSD side only | S | 2 |
| VM03-F096 | Profiles | Offline tool, Office app, Game, Network client, Untrusted; each a preset of the Access tab | S | 2 |
| VM03-F097 | Revoke | Any grant can be revoked while the app runs (SI-6); "Revoke everything and stop" is the emergency action | M | 2 |
| VM03-F098 | Audit | Installs, runs, grants, denials, first network contacts, blocked driver loads: to `svc.audit` | M | 2 |
| VM03-F099 | Identifier hygiene | Per environment: random machine id, hostname, MAC as seen by the app, user name ("user" by default), time zone and locale exposure choice | S | 3 |

### 4.8 Graphics, audio and input bridges

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM03-F100 | Graphics path display | Per app: which API the app used (GDI, Direct3D 9–12, OpenGL, Vulkan, GLES) and which path served it (software, translation to Vulkan, native Vulkan) | M | 2 |
| VM03-F101 | Translation layer choice | Choose the Direct3D-to-Vulkan layer version per app from installed components | S | 2 |
| VM03-F102 | Software rendering | Force CPU rendering for an app (safe mode) | M | 1 |
| VM03-F103 | Display options | Windowed, borderless, full screen, virtual desktop; frame-rate limit; vertical sync; integer or filtered scaling; HiDPI scale passed to the app or emulated | M | 2 |
| VM03-F104 | Shader cache | Per-app shader cache on the environment dataset; size and clear | S | 2 |
| VM03-F105 | GPU choice | On multi-GPU hosts choose the device; needs the app's display grant only | S | 3 |
| VM03-F106 | Overlay | Optional frame time and GPU load overlay drawn by the host bridge, toggled with a host key | C | 3 |
| VM03-F107 | Audio bridge | Foreign audio APIs map to the host audio service; per-app volume, output device, latency (low, balanced); input only with `dev.mic` | M | 1 |
| VM03-F108 | MIDI | MIDI ports exposed only when granted | C | 3 |
| VM03-F109 | Keyboard | Layout passed through; input method editing through the host input method (AX04) as composed text; option to send raw scancodes | M | 2 |
| VM03-F110 | Pointer | Relative pointer lock for games on request with a visible hint and `Esc`-hold release | M | 2 |
| VM03-F111 | Game controllers | Controllers appear to the app through `dev.usb:hid` grants as standard pads; per-app mapping editor; rumble | S | 2 |
| VM03-F112 | Touch and pen | Forwarded where the personality supports them | C | 3 |
| VM03-F113 | Video playback | Foreign media frameworks decode in the sandbox; hardware decode only when the GPU bridge allows it | C | 3 |
| VM03-F114 | Bridge isolation | Graphics, audio and window bridges run as separate host-side broker processes per app; a crash of one does not take down the session | M | 2 |

### 4.9 Launcher and desktop integration

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM03-F115 | Launchers in the shell | Each enabled launcher is registered with SH01 in the "Foreign apps" category with a kind badge; removing the app removes them | M | 1 |
| VM03-F116 | Window identity | Foreign windows carry the generated app id so SH01 and SH05 group, pin and apply rules correctly; title bars show a small kind badge | M | 1 |
| VM03-F117 | File associations | Offer foreign apps as handlers for MIME types; never set as default without the user's choice in SY01 | S | 2 |
| VM03-F118 | Open with a foreign app | The file is granted to the app for that run and mapped to a path inside | M | 2 |
| VM03-F119 | URL schemes | Schemes registered by foreign apps are listed and off by default | S | 2 |
| VM03-F120 | Tray icons | Foreign status-area icons appear in the shell's status area with a kind badge, only while the app runs | S | 3 |
| VM03-F121 | Disposable launch | "Open in disposable sandbox" on a launcher (SH01 action) runs the app on a throw-away clone of its environment | S | 2 |
| VM03-F122 | Search | App names contributed to `svc.search` | S | 2 |

### 4.10 Compatibility database

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM03-F123 | Local database | A signed, read-only data pack installed and updated by SY05; SQLite; entries keyed by executable hash, PE version resource, package name or product code | M | 2 |
| VM03-F124 | Entry content | Rating (works, works with settings, partly, does not work, needs VM), tested base versions, required components, suggested settings, known problems, recipe link | M | 2 |
| VM03-F125 | Match on add | When a file is added the best match is shown with its evidence; the user applies its settings in one step | M | 2 |
| VM03-F126 | Browse and search | Database browser with filters by kind and rating | S | 2 |
| VM03-F127 | User notes | The user's own rating, settings and notes per app in a separate writable store that overrides the pack | M | 2 |
| VM03-F128 | Report export | Export a report file (app identity, base version, settings, result, optional logs after a redaction pass) to share by the user's own means; the app never uploads | S | 3 |
| VM03-F129 | Report import | Import report files or third-party packs signed by a key the user trusts | C | 3 |
| VM03-F130 | Local suggestions | With `ai.local`: rank likely fixes from the diagnostic findings and database text; off by default; local only | C | 3 |

### 4.11 Diagnostics

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM03-F131 | Run log | Per run: start time, base version, settings hash, exit code, duration; stdout and stderr captured | M | 1 |
| VM03-F132 | Diagnose | One action that reruns the app with tracing and produces ranked findings: missing library, unimplemented interface, refused system call, denied capability, graphics feature missing, blocked driver | M | 2 |
| VM03-F133 | Denied-capability hints | When a run fails right after a denial, the app page shows the denial and a one-click grant with the usual prompt | M | 2 |
| VM03-F134 | Trace channels | Choose trace channels (file, registry, loader, network, graphics for Windows; syscalls, loader for Linux; log tags for Android) and level; output to a size-capped file | S | 2 |
| VM03-F135 | Log viewer | Filter, search, follow, level colours, copy, save; opens in AD04 on request | M | 1 |
| VM03-F136 | Crash reports | Foreign crash dumps and backtraces are stored locally in the environment; listed with module and exception; never sent | S | 2 |
| VM03-F137 | Environment check | Verify base integrity against its signed manifest and list modified system files | S | 2 |
| VM03-F138 | Compare runs | Diff settings and findings between a working and a failing run | C | 3 |
| VM03-F139 | Redaction | Logs exported in a report have user name, host name, paths and addresses replaced | S | 3 |
| VM03-F140 | Open in System Monitor | Jump to SY04 at the app's capability domain | S | 2 |

### 4.12 VM fallback

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM03-F141 | Run in a VM instead | For apps rated "needs VM" or failing with a blocked driver: choose an existing VM01 guest or create one from a template | M | 2 |
| VM03-F142 | Hand the installer over | The installer file is offered to the guest through VM01's file-drop path | S | 2 |
| VM03-F143 | VM-backed launcher | A launcher that starts the VM (if needed) and the program through the guest agent, shown as a window through VM01's console | S | 3 |
| VM03-F144 | Same Access tab | For a VM-backed app the Access tab shows the VM's reach summary from VM01 | S | 3 |
| VM03-F145 | Clear statement | The library marks VM-backed apps with a VM badge; the user is told that a licensed guest operating system is their responsibility | M | 2 |

### 4.13 Automation, settings, accessibility, recovery

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM03-F146 | Command-line tool | `compat run <app>`, `compat add <file>`, `compat list`, `compat diagnose <app>` over the same service | S | 2 |
| VM03-F147 | Intents | `open` for `.exe`, `.msi`, `.lnk`, `.AppImage`, ELF executables, `.deb`, `.rpm`, `.apk` | M | 1 |
| VM03-F148 | Command palette | Every app and action in `Ctrl+K` | M | 1 |
| VM03-F149 | Foreign app accessibility | Bridge foreign accessibility trees (UI Automation/MSAA, AT-SPI, Android accessibility nodes) into `svc.a11y` where the personality exposes them; state shown per app | S | 3 |
| VM03-F150 | Crash safety | Foreign apps run under `svc.compat`; this app can crash or close without stopping them | M | 1 |
| VM03-F151 | Settings history | Last 20 versions of each app's settings with diff and restore | S | 2 |
| VM03-F152 | Help | Help per view; a page on what cannot work (kernel drivers, vendor services, 16-bit code) | S | 2 |
