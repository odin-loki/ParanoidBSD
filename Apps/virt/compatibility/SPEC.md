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

## 5. GUI

### 5.1 Window map

```
Compatibility
├─ Main window
│  ├─ Sidebar: All · Windows · Linux · Android · Running · Environments · Components · Database · Jobs
│  ├─ Library (grid or list)
│  ├─ App page (replaces the library; Alt+Left returns)
│  │  └─ Tabs: Overview · Access · Settings · Environment · Snapshots · Diagnostics
│  ├─ Inspector (F4): summary, reach summary, database card
│  ├─ Add wizard (sheet, 4 pages)
│  ├─ Change-set review (sheet)
│  ├─ Android permission mapping (sheet)
│  ├─ Environment page (files, components, registry, processes, services)
│  │  ├─ Registry editor
│  │  └─ Library overrides editor
│  ├─ Components list · Component detail
│  ├─ Database browser · Entry detail · Report export dialog
│  ├─ Diagnose results page · Log viewer · Compare runs
│  ├─ Controller mapping editor · Touch map overlay editor
│  ├─ Run in a VM dialog
│  ├─ Remove confirmation
│  └─ Preferences (Ctrl+,)
├─ Foreign app windows (owned by the bridges, not by this app; host-drawn kind badge)
└─ Shell-drawn prompts (SH04): file portal, device prompts, network learning review
```

### 5.2 Main window

Shown at 1180×720, grid view, one app selected.

```
+----------------------------------------------------------------------------------------+
| [=] Compatibility                    [ Search apps  Ctrl+F ]             [▁] [▢] [✕]  |
+------------+---------------------------------------------------+-----------------------+
|> All    11 | [+ Add app] [Run once…] | [▶ Run] [■] [Diagnose] (1)| LedgerPro 9       (4) |
|  Windows 6 |                          [grid|list] [filter v]     | [Win] works with      |
|  Linux   3 +---------------------------------------------------+       settings        |
|  Android 2 | +--------+ +--------+ +--------+ +--------+     (2)| Environment ledgerpro |
|  Running 1 | | [icon] | | [icon] | | [icon] | | [icon] |        | base win-rt 10.2      |
| ---------- | |LedgerPr| | CADView| | MapTool| | Notes+ |        | --------------------- |
|  Environ. 9| |[Win] ● | |[Win]   | |[Lin]   | |[And]   |        | Can reach:        (5) |
|  Components| +--------+ +--------+ +--------+ +--------+        |  ~/Documents/Books rw |
|  Database  | +--------+ +--------+ +--------+                   |  printing             |
|  Jobs      | | [icon] | | [icon] | | [icon] |                   | Cannot reach:         |
|            | |OldGame | | fwtool | | Chat   |                   |  network, other files,|
|            | |[Win][VM]| |[Lin]  | |[And][net]|                 |  camera, microphone   |
|            | +--------+ +--------+ +--------+                   | [Open app page]       |
|            |                                                     | [Network: off |○ ]    |
+------------+---------------------------------------------------+-----------------------+
| 11 apps · 1 running · environments 8.4 GiB (bases 5.1 shared) (3)                       |
+----------------------------------------------------------------------------------------+
```

1. Toolbar: Add app is primary; Run once starts a disposable run.
2. Library (`gridView`, 128×128 tiles; `tableView` in list mode). Tile badges: kind, running dot, `[net]`, `[VM]`, trust.
3. Status bar: counts and space.
4. Inspector header: kind, database rating, environment and pinned base version.
5. Reach summary with the network switch (`toggleSwitch`).

States: empty ("No foreign apps yet. Drop an installer, an executable or an APK here", `[+ Add app]`), loading, personality missing (per kind: "Windows apps are not available on this system" with a link to SY05 for the base package, and the VM fallback offered), error banner, read-only (policy), narrow (sidebar overlay, two-column grid, inspector sheet).

### 5.3 Other views and dialogs

#### Add wizard, page 1 (sheet, 740×500)

```
+------------------------------------------------------------------------+
| Add an app                              1 What  2 Environment  3  4     |
+------------------------------------------------------------------------+
| File   LedgerPro-9.2-setup.exe   84.1 MiB   SHA-256 6c1f…e20b  [Copy]   |
| Kind   Windows installer (Inno Setup), 64-bit                     (1)   |
| Signed ✓ "Ledger Software Ltd" · chain valid to a local trust anchor    |
| Source downloaded 11 Nov from ledger.example (Transfers record)         |
| Database match: LedgerPro 9.x — works with settings              (2)    |
|   needs: core fonts, runtime v14 · suggests: Windows 10, no 3D          |
|   [x] Apply suggested settings and components                           |
| ( ) Install it   (•) default      ( ) Run once in a disposable sandbox  |
+------------------------------------------------------------------------+
| [Cancel]                                   [Next →]  [Install with defaults] |
+------------------------------------------------------------------------+
```

(1) From `probe-helper`. (2) Database card with evidence of the match. States: unknown file (offer "Treat as…" list), unsigned (`--warn`), invalid signature (`--danger`, defaults to disposable run), unsupported (16-bit, wrong architecture without translation).

#### Change-set review (sheet, 820×560)

```
+--------------------------------------------------------------------------+
| The installer finished. Review what it changed.                           |
+--------------------------------------------------------------------------+
| [x] Files       412 added, 3 changed      C:\Program Files\LedgerPro\…    |
| [x] Registry    96 keys                   HKLM\Software\Ledger…           |
| [x] Launchers   LedgerPro 9 · [ ] LedgerPro Updater · [ ] Uninstall       |
| [ ] Autostart   "LedgerPro Updater" at login                        (1)   |
| [ ] Service     "LedgerSvc" (runs as system inside the environment)       |
| [x] File types  .ldg → LedgerPro (inside this environment only)           |
| ⚠ Driver        ledgerlock.sys — kernel drivers are not loaded     (2)    |
| Network during install: none requested                                    |
+--------------------------------------------------------------------------+
| [Roll back everything]            [Show details…]        [Keep selected]  |
+--------------------------------------------------------------------------+
```

(1) Off by default. (2) Blocked item with a link to the VM fallback. `treeView` with check boxes; details pane lists every path.

#### App page, Access tab

```
+----------------------------------------------------------------------------------------+
| ← LedgerPro 9  [Win]  ○ Not running   [▶ Run] [Diagnose]                    [⋯]        |
| [Overview] [Access] [Settings] [Environment] [Snapshots] [Diagnostics]                  |
+----------------------------------------------------------------------------------------+
| Profile [Office app v]                                                                  |
| Files       ~/Documents/Books → D:\   read and write   [Remove]   [+ Add folder…]       |
| Network     ( ) off  (•) named hosts  ( ) any        route [clear v]                    |
|               licence.ledger.example:443   [Remove]   [+ Add host]  [Learn from a run…] |
| Clipboard   [text only v]        Printing [x]        Notifications [x]                  |
| Camera [ ]  Microphone [ ]  Screen capture [ ]  Location [ ]  USB [none v]              |
| Background  [ ] keep running without a window                                           |
| ------------------------------------------------------------------------------------   |
| Denied in the last run: outbound to telemetry.ledger.example:443 (3 times)  [Allow…]    |
| [Revoke everything and stop]                                   [Open in Permissions]    |
+----------------------------------------------------------------------------------------+
```

Each row is a grant in the generated manifest. The denial line gives the one-click path to a prompt.

#### Android permission mapping (sheet)

```
+------------------------------------------------------------------------+
| Install "Notes+" (com.example.notesplus 4.1, signer 9A:3C:…:11)         |
+------------------------------------------------------------------------+
| Android permission          Maps to                 Allow               |
| INTERNET                    network: named hosts    [ ] (learn first)   |
| READ_MEDIA_IMAGES           folder you choose       [ ]                 |
| CAMERA                      dev.camera (prompt)     [ ]                 |
| POST_NOTIFICATIONS          svc.notify              [x]                 |
| ACCESS_FINE_LOCATION        dev.location (prompt)   [ ]                 |
| READ_CONTACTS               empty provider          –   (1)             |
| RECEIVE_BOOT_COMPLETED      background run          [ ]                 |
+------------------------------------------------------------------------+
| [Cancel]                                              [Install]         |
+------------------------------------------------------------------------+
```

(1) Providers with no PBSD mapping return empty data.

#### Diagnose results

```
+------------------------------------------------------------------------+
| Diagnosis of OldGame — run of 12 Nov 14:02, exit code 0xC0000135        |
+------------------------------------------------------------------------+
| 1 ✕ Missing library   d3dx9_43.dll            [Add component "dx9 extras"]|
| 2 ⚠ Blocked driver    oldguard.sys            [Run in a VM instead…]     |
| 3 ⚠ Denied capability network (2 attempts)    [Review Access…]           |
| 4 ⓘ Graphics          Direct3D 9 → Vulkan translation, no errors        |
+------------------------------------------------------------------------+
| [Open full log]  [Compare with a working run…]  [Export report…]         |
+------------------------------------------------------------------------+
```

Findings are ranked; each has at most one suggested action.

#### Environment page (Windows)

```
+----------------------+-------------------------------------------------+
| ledgerpro            | Base        win-rt 10.2 ✓ intact   [Change…]    |
|  Files (C:)          | Reports as  [Windows 10 v]   Arch 64-bit        |
|  Components 3        | Components  core fonts · runtime v14 · d3d→vk 2.4|
|  Library overrides 2 | Size        612 MiB unique · 2.1 GiB shared     |
|  Registry            | Snapshots   4 (latest "before update", 3 Nov)   |
|  Processes 0         | Autostart   0 enabled of 1   Services 0 of 1    |
|  Services            | [Browse files] [Open shell] [Snapshot] [Export…]|
+----------------------+-------------------------------------------------+
```

### 5.4 Menus

```
File         Add app… Ctrl+N · Run once… Ctrl+Shift+N · Import environment… Ctrl+I ·
             Export… Ctrl+E · Preferences Ctrl+, · Close Ctrl+W · Quit Ctrl+Q
Edit         Undo Ctrl+Z · Redo Ctrl+Shift+Z · Rename F2 · Duplicate Ctrl+D · Find Ctrl+F ·
             Remove Delete · Remove for ever Shift+Delete
View         Grid Ctrl+1 · List Ctrl+2 · Sidebar F9 · Inspector F4 · Running Ctrl+Shift+R ·
             Refresh F5
App          Run Ctrl+R · Run with options… Ctrl+Alt+R · Run in disposable sandbox ·
             Stop Ctrl+. · Force stop Ctrl+Shift+. · Diagnose Ctrl+Shift+D · Access Ctrl+Shift+A ·
             Settings · Snapshot Ctrl+T · Roll back… · Update from file… ·
             Run in a VM instead… · Revoke everything and stop…
Environment  Browse files · Open shell Ctrl+Shift+T · Components… · Registry editor ·
             Library overrides… · Change base… · Check integrity · Clear caches
Database     Browse Ctrl+B · Apply suggested settings · My notes… · Export report… · Import…
Help         Help F1 · What cannot work · About
```

### 5.5 Keyboard and pointer

App shortcuts are those in 5.4. `Enter` runs the selected app; `Space` shows its reach summary popover.

Context menu on an app: Run, Run with options, Run in disposable sandbox, Stop, App page, Access, Diagnose, Browse environment, Snapshot, Duplicate, Pin to task bar (SH01), Remove. On a component: Details, Add to environment, Remove from environment. On a registry key: New key, New value, Rename, Delete, Export, Copy path.

Drag and drop: a file on the window starts the Add wizard; a file on an app tile runs the app with that file (granted for the run); a folder on the Access tab starts "Add folder" through the portal; a `.reg` file on the registry editor opens a review; an app on a collection adds it.

Foreign windows: `Super`-key shortcuts are never delivered to foreign apps. Pointer lock is released by holding `Esc` for 1 s.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Drop hint and `[+ Add app]` |
| Loading | Skeleton tiles; installer and base jobs in the status bar |
| Error | Banner with cause; a failed run puts a `--danger` dot on the tile and a "Diagnose" action |
| Offline | The app is offline by design; only foreign apps with a network grant are affected |
| Locked | Encrypted environment: lock icon and Unlock |
| Read-only | Policy lock; controls disabled with the policy named |
| Narrow | Sidebar overlay, app-page tabs become a `comboBox` |
| Personality missing | Per-kind notice with what to install; VM fallback offered |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Library | `gridView`, `tableView` |
| Sidebar, change set, registry tree, file lists | `treeView` |
| App page tabs | `tabWidget` |
| Access switches | `toggleSwitch`, `checkBox`, `comboBox`, `radioButton` |
| Settings | `propertyGrid` |
| Logs | `modelListView` |
| Registry values, overrides, processes, database | `tableView` |
| Controller and touch map editors | `image` with overlay `panel` nodes; `VM03-GAP-3` for free placement |
| Database entry text, help | `markdownView` |
| Dialogs, wizard | `dialog`, `lineEdit`, `progressBar` |

Missing:

- `VM03-GAP-1`: `svc.compat`, the service that owns environments and launches foreign apps in capability domains, and the per-app bridge brokers (window, graphics, audio, portal, bus).
- `VM03-GAP-2`: a way to register generated manifests for foreign apps with the app host, shell and AD05 (extends G-07).
- `VM03-GAP-3`: a simple canvas for placing touch-map regions over an image (G-12 would cover it).
- `VM03-GAP-4`: compositor support for foreign surfaces: a nested or proxied Wayland endpoint per sandbox and an X11 server per sandbox.
- `VM03-GAP-5`: a11y bridge from foreign accessibility trees to `svc.a11y`.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Library, wizard, access model, settings, database matching, diagnosis ranking |
| UI | FUSE UI | app host | Windows in 5.1 |
| `probe-helper` | C++23 | helper, capability mode | Identify files; parse PE, MSI summary, ELF, AppImage header, `.deb`/`.rpm` headers, APK (ZIP, binary XML manifest, signing block), `.lnk`, `.desktop`; extract icons |
| `diff-helper` | C++23 | helper, capability mode | Compute the installer change set from two snapshots (file tree, registry hives) |
| Compatibility service `svc.compat` | C++23 | system service (Proposed) | Environments on ZFS, generated manifests, launching, run logs, tracing switches |
| Window bridge | C++23 | broker per app | Filtering Wayland proxy; per-app X server for X11 programs; window identity and badges |
| Graphics bridge | C++23 | broker per app | GPU access for the sandbox; hosts the translation layers' host side |
| Audio bridge | C++23 | broker per app | Foreign audio protocols to `svc.media` |
| Portal and bus bridge | C++23 | broker per app | Foreign portal, shell and intent requests to PBSD portals; filtered private bus |
| Linux and Android domains | – | created through `svc.jail` (VM02) | Runtime roots, the AOSP container |
| Personalities and translators | C++23 kernel | kernel | P4-01, P4-02, P4-04, P4-05, P4-06, P4-07 |
| `compat` CLI | ForgeScript | own process | Same bus interface |

```
 app host ──bus──▶ svc.compat ──▶ capability domain per foreign app
   ├─ probe-helper     │             ├─ foreign process (NT / Linux / Android personality)
   └─ diff-helper      │             └─ sees: environment dataset, granted folders, bridge sockets
                       ├─ window bridge ──▶ compositor
                       ├─ graphics bridge ─▶ GPU     ├─ audio bridge ─▶ svc.media
                       └─ portal/bus bridge ─▶ SH04 portals, svc.intent, svc.notify
 fallback: svc.compat ──▶ svc.vm (VM01)
```

Third-party code (Proposed; all run inside the foreign sandbox or as base packages, none inside the app host):

| Library | Licence | Why |
|---|---|---|
| Wine user-mode libraries | LGPL-2.1-or-later | Named by plan P4-04 as the source of the NT user-mode side |
| DXVK | zlib | Direct3D 8–11 to Vulkan translation; no equivalent own code |
| vkd3d-proton or vkd3d | LGPL-2.1 | Direct3D 12 to Vulkan |
| AOSP userland (bionic, ART, servicemanager) | Apache-2.0 (mostly) | Named by plan P4-05 |

Each must be built offline from vendored sources; that build is Unverified on HardenedBSD.

### 6.2 Data model

App record (in `svc.compat`, mirrored read-only to the app):

| Field | Type | Notes |
|---|---|---|
| `id` | string `foreign.<kind>.<hash16>` | Generated manifest id |
| `kind` | enum `windows`, `linux`, `android`, `vm` | |
| `name`, `publisher`, `version`, `icon` | string, string, string, blob ref | |
| `environment` | environment id | |
| `launchers[]` | `{id, label, target, args[], workdir, env{}, enabled}` | |
| `source` | `{file_name, sha256, signer, signature_state, origin, added_at}` | Provenance |
| `grants` | `{folders[], network{mode, hosts[], route}, clipboard, devices[], print, notify, background}` | Becomes manifest capabilities |
| `settings` | kind-specific record (below) | Versioned, 20 kept |
| `db_match` | `{entry_id, evidence, applied_at}` | |
| `stats` | `{last_run, runs, seconds}` | Optional |

Kind-specific settings: Windows `{reports_as, arch, overrides[{dll, mode}], drives[{letter, folder}], virtual_desktop{w,h}, d3d_layer, renderer, fps_limit, vsync, scale}`; Linux `{runtime, layer: personality|linuxulator, env{}, x11: bool, toolkit_hints}`; Android `{instance, density, orientation, permissions{}, touch_map, background, identifiers_profile}`.

Environment record: `{id, kind, name, base{package, version}, dataset, components[], shared: bool, encrypted: bool, snapshots[]}`.

Stores:

| Store | Location | Content |
|---|---|---|
| Environments | dataset `…/compat/env/<id>/` | Prefix, overlay or Android data; `runs/` logs; `crash/` |
| Bases and components | read-only datasets installed by SY05 | Signed manifests |
| Registry of apps | `svc.compat` store `apps.db` (SQLite) | Tables `app`, `launcher`, `grant`, `settings_version`, `run(app, started, base, settings_hash, exit_code, seconds)` |
| Database pack | `/usr/share/pbsd/compat-db/compat.db` (read-only, signed) | Tables `entry`, `match_key(kind, key, entry)`, `setting`, `component_req`, `recipe` |
| User notes | `~/.local/share/org.pbsd.compatibility/notes.db` | `note(app, rating, text, settings_json)` |
| UI state | `~/.local/share/org.pbsd.compatibility/ui.db` | View state, collections |

Schema version in every store; forward-only migration after a snapshot.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| PE/COFF headers, resources, Authenticode (PKCS#7) | yes | no | Own code; signature math by FORGE crypto | `probe-helper` |
| MSI (compound file) summary and tables | yes | no | Own code | `probe-helper` |
| ELF headers, dynamic section | yes | no | Own code | `probe-helper` |
| AppImage, squashfs (mount) | yes | no | Own read-only squashfs reader | sandboxed file-system helper under `svc.compat` |
| `.deb` (ar + tar), `.rpm` (header + cpio) | yes | no | Own code; zlib, zstd, xz from base | `probe-helper` (headers), unpack helper (payload) |
| APK (ZIP, binary XML, APK Signing Block v2/v3) | yes | no | Own code | `probe-helper` |
| `.lnk`, `.desktop`, `.reg` | yes | `.reg` export | Own code | `probe-helper` |
| Windows registry hive files | yes | yes | Own code | `diff-helper`; edits go through `svc.compat` |
| Icons (ICO, PNG, Android adaptive XML) | yes | PNG | Own code + FUSE image codecs | `probe-helper` |
| App and environment records | yes | yes | Own code | app host; re-validated in `svc.compat` |
| Compatibility database (SQLite) | yes | notes only | SQLite (public domain) through `SqliteModel` | app host (read-only handle) |
| Report file (TOML + optional logs, tar) | yes | yes | Own code | app host writes; `probe-helper` reads imports |
| Wayland, X11, audio and portal protocols of foreign apps | – | – | Bridges | per-app brokers |

### 6.4 Integration

- Intents handled: `open` for `application/vnd.microsoft.portable-executable`, `application/x-msi`, `application/x-ms-shortcut`, `application/vnd.appimage`, `application/x-executable`, `application/vnd.debian.binary-package`, `application/x-rpm`, `application/vnd.android.package-archive`. Emitted: `open` to SY02, SY03, VM01, AD04, AD05, SY04, SY05; `share` for reports.
- Services: `svc.compat` (gap), `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.intent`, `svc.search`, `svc.keyring`, `svc.audit`, `svc.update`, `svc.model` (optional). Device portals are requested by the foreign apps' generated manifests, not by this app.
- IPC offered by `svc.compat`: `org.pbsd.compat.Manager/1` (list, add, run, stop, grants, diagnose); used by SH01 (launchers, disposable launch), SY02 (open with), AD05.
- Other apps: SH01 shows launchers in its "Foreign apps" category (Verified in `Apps/shell/desktop-shell/SPEC.md`); VM02's service creates Linux and Android domains; VM01 provides the VM fallback; SE02 enforces network grants; SY05 installs bases, components and the database pack.

## 7. Security and privacy

### Capabilities requested (by this app)

| Capability | Reason |
|---|---|
| `fs.app` | Notes, UI state |
| `fs.portal` | Pick installers and executables, folders to grant, export targets |
| `proc.helper:probe-helper`, `proc.helper:diff-helper` | Parse hostile files; compute change sets |
| `svc.compat` | All environment and launch operations (VM03-GAP-1) |
| `svc.settings`, `svc.notify`, `svc.intent`, `svc.search`, `svc.audit`, `svc.update` | As in 6.4 |
| `svc.keyring` | Environment encryption keys |
| `ai.local` | Optional local suggestions |
| `net.none` | The manager itself opens no sockets |

Foreign apps each have their own generated capability list. This app never lends them its own.

### Threat model

Assets: user files, the session (windows, input, clipboard), other apps, the kernel, the user's identity and identifiers.

| Adversary | Attack | Mitigation |
|---|---|---|
| Malicious foreign program | Read or encrypt user files | Default deny; only granted folders are visible; file dialogs are host portals |
| Malicious foreign program | Keylogging, screen scraping, input injection | Per-app window bridge; no global hooks; capture limited to own windows; X11 programs isolated per app |
| Malicious foreign program | Exfiltration | Network off by default; named-host policy enforced by SE02; first contacts logged |
| Malicious foreign program | Persist | Autostart, services and scheduled tasks off unless enabled; snapshots and rollback; disposable runs |
| Malicious foreign program | Attack the kernel through the large foreign syscall surface | Personalities are the plan's proof and gateway targets (P4-01, P4-07); kernel drivers never loaded; W^X kept (SI-3) |
| Malicious foreign program | Attack bridges (window, graphics, audio protocols) | Bridges are per-app brokers in capability mode; a crash affects one app; fuzzing of each protocol decoder |
| Hostile installer or package | Parser exploits in PE, MSI, APK, deb, rpm, squashfs | Parsing only in helpers with one read handle; fuzzing |
| Hostile `.reg`, recipe or report file | Silent configuration change | Review before apply; packs and recipes must be signed by a trusted key |
| Spoofing | Foreign window imitating a system prompt | Shell prompts are drawn by the shell on a layer foreign surfaces cannot reach; foreign windows carry a kind badge in the title bar |
| Tracking | Hardware and user identifiers | Identifier hygiene per environment; Android identifiers blanked |

Stored: environments and run logs on the user's datasets; notes and UI state in app data. Leaves the machine: nothing by this app. Foreign apps send only what their network grant allows. Reports are files the user exports.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `library.view` | enum `grid`, `list` | `grid` | |
| `add.default_mode` | enum `install`, `run-once` | `install` | Wizard default |
| `add.unsigned_default` | enum `install`, `run-once` | `run-once` | Default for unsigned or invalid files |
| `install.network` | bool | `false` | Network during installers |
| `install.snapshot_before` | bool | `true` | |
| `env.per_app` | bool | `true` | One environment per app |
| `env.keep_days` | u32 | `14` | Retention after removal |
| `defaults.profile` | enum | `offline-tool` | Access profile for new apps |
| `defaults.clipboard` | enum `off`, `text`, `text-images` | `off` | |
| `defaults.windows_version` | enum | `10` | |
| `defaults.linux_layer` | enum `personality`, `linuxulator` | `personality` | While both exist |
| `android.instance_per_app` | bool | `false` | |
| `android.background` | bool | `false` | |
| `graphics.renderer` | enum `auto`, `software` | `auto` | |
| `graphics.fps_limit` | u32 | `0` (off) | |
| `stats.record` | bool | `true` | Run history |
| `diagnose.trace_cap_mib` | u32 | `64` | Trace file cap |
| `ai.suggestions` | bool | `false` | Needs `ai.local` |

## 9. Accessibility and languages

- This app's own UI follows the system rules. Foreign apps are only as accessible as the bridge allows; each app page states "Screen reader support: full, partial or none" from the bridge's report.
- Text scale: the app passes the system scale to foreign toolkits where they accept it and offers bitmap scaling otherwise; the app page states which is in use.
- Input methods compose on the host and deliver committed text to foreign apps.
- Foreign apps keep their own language settings; a per-app locale override exists. Names and log text from foreign apps are never translated.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | 250 ms with 200 apps |
| File recognition | under 300 ms for a 4 GiB installer (headers only) |
| New environment | under 1 s (clone) |
| Launch overhead added by the service and bridges | under 300 ms before the foreign process starts |
| Change-set computation | under 10 s for 50,000 changed files |
| Bridge frame overhead | under 1 ms per frame (buffer hand-over, no copy) |
| App memory | under 90 MiB; each broker under 30 MiB idle |

## 11. Testing and proof

Nothing has been run. All entries are targets.

- Unit: grant model to manifest generation, permission mapping tables, database matching, diagnosis ranking, settings diff.
- Golden renders of 5.2 and 5.3.
- Scripted UI: T1–T12 against a fake `svc.compat`.
- Fuzz: every `probe-helper` parser (PE, Authenticode, MSI, ELF, AppImage, deb, rpm, APK, binary XML, lnk, desktop, reg, ICO), registry hive reader, squashfs reader, each bridge protocol decoder.
- Differential: PE and APK metadata against reference tools on a Linux test machine; APK signature verdicts against the AOSP verifier's test vectors.
- Isolation suite: foreign test programs that try to read outside grants, hook input, capture the screen, enumerate other apps' windows, open sockets, load a driver; each must fail and be audited.
- App matrix: a fixed list of freely redistributable Windows, Linux and Android test programs per release.
- Fault injection: kill each broker, the service, the helper; fill the dataset during install.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Manifest generation | Generated capability list equals the user's grants, never more (SI-1) | T1 | Small model and contracts through PRISM | – |
| Launch path in `svc.compat` | Foreign domain receives only handles from its manifest (SI-2) | T1 | Model in Lean 4 or TLA+, ESBMC contracts | – |
| `probe-helper` parsers | No runtime errors on any input | T2 | Safe subset, contracts, bounded checking, fuzzing | Number of formats |
| `diff-helper` | Total; change set complete relative to the two snapshots | T2 | Contracts, property tests | Registry hive format |
| Bridges | Memory safety; request filtering complete (unknown requests refused) | T3 | Safe subset, sanitizers, fuzzing, allow-list tests | Protocol size |
| Third-party foreign user-mode code | – | T4 | Runs only inside the sandbox | Not our code |
| App logic | No capability widening | T3 | FSS policy, script tests | Interpreter not verified |
| UI | – | T4 | Golden and scripted tests | – |

## 12. Build plan

### Phase 1: usable core

| ID | Work item | Depends on |
|---|---|---|
| VM03-W01 | `org.pbsd.compat.Manager/1` schema and a fake service | G-08, VM03-GAP-1 |
| VM03-W02 | `probe-helper`: PE, ELF, icons, hashes | FORGE-WP-27, FORGE-WP-28 |
| VM03-W03 | Library window, app page (Overview, Access basic, Settings basic) | G-01, G-02, VM03-W01 |
| VM03-W04 | `svc.compat` core: environments as clones, generated manifests, launch | G-07, VM03-GAP-2, G-06 |
| VM03-W05 | Linux apps: runtimes, window bridge, private home, console launchers | P4-02, VM02 (`svc.jail`), VM03-GAP-4 |
| VM03-W06 | Windows apps: launch, version setting, drives, process list | P4-04, VM03-W04 |
| VM03-W07 | Add wizard, run once, portable apps, launcher registration with SH01 | SH01, VM03-W02 |
| VM03-W08 | Audio bridge; software rendering path | G-09 (`svc.media`) |
| VM03-W09 | Run log and log viewer; snapshots and rollback | VM03-W04 |

### Phase 2: full feature

| ID | Work item | Depends on |
|---|---|---|
| VM03-W10 | Installer staging, `diff-helper`, change-set review | VM03-W09 |
| VM03-W11 | Full Access tab: portals for file dialogs, network policy with learning run, clipboard, device prompts | SH04, SE02 |
| VM03-W12 | Graphics bridge with translation layers; display options; controllers | P3-05, VM03-W08 |
| VM03-W13 | Android: instance, APK parse and verify, permission mapping, windows, notifications | P4-05, VM02 |
| VM03-W14 | Components and base management through SY05 | SY05 |
| VM03-W15 | Compatibility database pack, matching, user notes | SY05, VM03-W02 |
| VM03-W16 | Diagnose, trace channels, crash reports, denied-capability hints | personality counters (P4-02, P4-04) |
| VM03-W17 | Registry editor, library overrides, printing, portal and bus bridges | G-16 |
| VM03-W18 | VM fallback with VM01 | VM01 (`svc.vm`) |
| VM03-W19 | `compat` CLI, file associations, disposable launch | VM03-W01 |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| VM03-W20 | Recipes and unattended installs; report export and import | VM03-W15 |
| VM03-W21 | Touch map editor, split APKs, intents bridge | VM03-GAP-3, VM03-W13 |
| VM03-W22 | VM-backed launchers | VM03-W18 |
| VM03-W23 | Foreign accessibility bridge | VM03-GAP-5, G-04 |
| VM03-W24 | Identifier hygiene, overlay, GPU choice, tray icons | VM03-W12 |
| VM03-W25 | Cross-architecture apps | P4-06 |
| VM03-W26 | Local suggestions | G-15 |

## 13. Open questions and risks

- The personalities do not exist yet (Verified absent). This app is blocked on P4-01, P4-02, P4-04 and P4-05; the plan itself lists "NT personality scope" as a risk.
- `docs/migration/LINUXULATOR.md` says Linux tasks never receive PBSD handles. This spec needs granted folder handles and bridge sockets inside Linux sandboxes. The two documents must be reconciled.
- Run-time code generation: many foreign programs (managed runtimes, browsers, the Android runtime in its default mode) generate code. SI-3 forbids W+X; plan P4-06 proposes dual mappings. Whether that is allowed for foreign domains under the suite's no-JIT rule is the owner's decision. The no-JIT rule in PLATFORM.md is written for native apps.
- Licences: Wine and vkd3d are LGPL, DXVK is zlib, AOSP is mostly Apache-2.0. They are separate programs inside sandboxes; the packaging and source-offer duties need a licence review.
- Offline builds of these third-party trees on HardenedBSD are Unverified.
- Anti-cheat and copy-protection drivers will not work by design; the only path is the VM fallback, which those products often also refuse.
- Android apps that need proprietary vendor services will not work; no substitute is planned.
- The X11 server per sandbox is extra code to maintain; an alternative is to support Wayland-only Linux apps first.
- The compatibility database needs a maintainer and a contribution route that does not involve uploads from the app.
