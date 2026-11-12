<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Virtual Machines

By Odin Loch

| | |
|---|---|
| ID | `VM01` |
| Bundle | `org.pbsd.virtual-machines` |
| Category | virt |
| Priority | P1 |
| Backing repos | ParanoidBSD (`pbsd/bifrost`, `hbsd/src/usr.sbin/bhyve`, `hbsd/src/sys/amd64/vmm`) |
| Status | Planned. Nothing built. |

## 1. Purpose

Virtual Machines creates, imports, runs and manages virtual machines on the BIFROST hypervisor (with bhyve as the reference device model). It is for people who need another operating system, a throw-away machine for risky work, or a server guest that runs without a window. It replaces virt-manager, VirtualBox, GNOME Boxes and hand-written `bhyve` command lines.

Every VM is a kernel object reached only through a `VmHandle` (SI-8, `docs/security/INVARIANTS.md`). The app shows that handle, its rights and everything derived from it, and never holds more than the user granted.

## 2. Scope

### In scope

- VM library: create, import, clone, export, delete, group, tag.
- New-VM wizard with OS templates and unattended-install helpers.
- Console window: display, keyboard, pointer, clipboard policy, file drop, USB redirection, serial console.
- Virtual hardware editor (CPU, memory, firmware, disks, network, display, sound, USB, serial, TPM, entropy, balloon).
- Disks on ZFS volumes; snapshots and clones backed by ZFS; VM state snapshots.
- Virtual networks (isolated, NAT, bridged, routed through VPN or Tor).
- PCI passthrough with an IOMMU group view.
- Disposable VMs.
- Per-VM capability view (`VmHandle` lineage).
- Live migration between two PBSD hosts.
- Headless run, autostart, resource graphs.
- Confidential-computing options (SEV-SNP, TDX) where the host supports them.
- Import of OVA/OVF, QCOW2, VMDK, VHDX, VDI and raw images; export to OVA, QCOW2, raw.

### Out of scope

| Item | Owner |
|---|---|
| Jails and OCI containers | VM02 |
| Running single Windows, Linux or Android apps without a VM | VM03 (which calls VM01 for the VM fallback) |
| Remote desktop to machines that are not local VMs | VM04 |
| Host device tree, driver binding, firmware, host USB allow-list | VM05 (VM01 links to it for passthrough preparation) |
| Host firewall rules and per-app network policy | SE02 |
| VPN and Tor circuits | SE03 |
| Pool and dataset administration, GELI | AD02 |
| Scheduled backups of VM datasets | AD03 |
| Grant and revocation management for all apps | AD05 (VM01 shows only its own VMs' handles) |
| Host process and domain monitoring | SY04 |

## 3. Users and top tasks

Personas:

- **Dara, developer.** Runs Linux and Windows guests to test builds. Wants fast clone, snapshot and revert.
- **Ines, analyst.** Opens untrusted files and links in disposable VMs. Wants one click and no residue.
- **Sam, home administrator.** Runs two headless server guests that start at boot. Wants graphs and alerts.
- **Mika, gamer or media user.** Passes a GPU and a USB controller to a Windows guest.
- **Noor, privacy user.** Runs a guest whose only route is Tor and whose disk is encrypted with a key held in SE04.

| # | Task | Target |
|---|---|---|
| T1 | Create a VM from an installer image with a template | 4 steps: `Ctrl+N`, drop ISO, accept template, "Create and start" |
| T2 | Start a VM and reach its console | 1 step: double click or `Enter` |
| T3 | Take a snapshot before a risky change | 1 step: `Ctrl+T` |
| T4 | Revert to the last snapshot | 2 steps: `Ctrl+Shift+T`, confirm |
| T5 | Open a file in a disposable VM | 2 steps: "Open in disposable VM" from SY02, pick template (remembered) |
| T6 | Clone a VM (linked) | 2 steps: `Ctrl+D`, name |
| T7 | Attach a USB device to the running guest | 2 steps: Devices menu, tick the device (shell prompt on first use) |
| T8 | Import an OVA or QCOW2 | 3 steps: drop file, review mapping, "Import" |
| T9 | Add a second disk | 3 steps: Hardware, "Add disk", size |
| T10 | Pass a PCI device through | 5 steps: Hardware, "Add PCI device", pick group, review, restart guest |
| T11 | See exactly what a VM can reach | 1 step: `F4` then "Capabilities" tab |
| T12 | Make a VM start at boot without a window | 2 steps: Inspector, "Start with the system" |
| T13 | Migrate a running VM to another host | 4 steps: VM menu, "Migrate", pick host, confirm |
| T14 | Send Ctrl+Alt+Del or another reserved chord | 1 step: `Host+Del` |
| T15 | Release keyboard and pointer from the guest | 1 step: `Host` key (default Right Ctrl) |

## 4. Features

Status of the backend (Verified): `pbsd/bifrost` holds 59 module files, 2,460 lines in total. `pbsd/bifrost/pbsd.bifrost.cppm` defines `Hypervisor` with `create_vm`, `start_vm`, `stop_vm`, `destroy_vm`, `handle_exit` and `guest_mmio`, all acting on one `placeholder_` object; `VmConfig` has `vcpus`, `ram_bytes`, `enable_pv`, `backend`. The other modules are type tables and validators (for example `pbsd.bifrost.migration.cppm` only defines `Phase {Idle, PreCopy, StopCopy, Resume}` and `validate_phase`). Every feature below is therefore Proposed and depends on P5-03 (`docs/plan/PBSD_PLAN.md`).

### 4.1 Library and organisation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F001 | VM library | All VMs the user owns, from `svc.vm`. List or grid view; each entry shows name, OS icon, state, uptime, vCPU, memory, disk use, trust badges | M | 1 |
| VM01-F002 | States | Stopped, Starting, Running, Paused, Suspended (to disk), Stopping, Migrating, Crashed, Locked (disk key not loaded). Icon, word and colour for each | M | 1 |
| VM01-F003 | Groups | User-defined folders in the sidebar; a VM is in one group; drag to move | S | 1 |
| VM01-F004 | Tags and notes | Free tags and a Markdown note per VM; shown in the inspector | S | 2 |
| VM01-F005 | Search | `Ctrl+F` filters by name, tag, OS, IP address, MAC, UUID, disk path | M | 1 |
| VM01-F006 | Live thumbnail | Grid tiles show a console thumbnail refreshed every 5 s while the window is visible; off for confidential VMs | S | 2 |
| VM01-F007 | Rename | `F2`; changes the display name only. UUID and dataset name stay | M | 1 |
| VM01-F008 | Delete | Moves the VM definition to the trash and keeps disks for 30 days as a held ZFS snapshot; "Delete for ever" destroys the datasets after a typed confirmation | M | 1 |
| VM01-F009 | Multi-select actions | Start, stop, snapshot, tag, move, export on several VMs | S | 2 |

### 4.2 New-VM wizard and templates

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F010 | Wizard | Five pages: Source, System, Storage, Network and devices, Review. Any page can be skipped with defaults; "Create" is enabled from page 1 | M | 1 |
| VM01-F011 | Source kinds | Installer image (ISO, IMG), existing disk image, appliance (OVA), network boot (PXE over the chosen virtual network), empty machine, clone of a VM | M | 1 |
| VM01-F012 | Image detection | The image helper reads ISO 9660/UDF volume ids and El Torito boot records and proposes an OS template; the guess is shown with its evidence | M | 1 |
| VM01-F013 | Image verification | Optional checksum (SHA-256, SHA-512, BLAKE3) or detached signature (Ed25519, OpenPGP via SE04) check before first boot; result stored as provenance | M | 1 |
| VM01-F014 | OS templates | Local template set: PBSD, HardenedBSD, FreeBSD, OpenBSD, NetBSD, Linux (generic, Debian family, Red Hat family, Arch, Alpine), Windows 10, Windows 11, Windows Server, Android-x86, generic UEFI, generic BIOS | M | 1 |
| VM01-F015 | Template content | Firmware, chipset, vCPU count, memory, disk size and bus, NIC model, display, clock (UTC or local), TPM, Secure Boot, tablet pointer, recommended guest tools | M | 1 |
| VM01-F016 | Template files | Templates are signed TOML files in the bundle; user templates live in `~/.local/share/org.pbsd.virtual-machines/templates/` | S | 2 |
| VM01-F017 | Save as template | Any stopped VM's hardware can be saved as a user template (without disks) | S | 2 |
| VM01-F018 | Sizing advice | Wizard shows host free memory and pool space and warns when the request exceeds 75% of either | M | 1 |
| VM01-F019 | Unattended install | For supported templates the wizard builds an answer medium (cloud-init NoCloud, kickstart, preseed, `autounattend.xml`, `installerconfig`) from user name, password, locale, keyboard, time zone | S | 2 |
| VM01-F020 | Secrets in answers | Passwords typed for unattended install are held in memory, written only to the generated medium, and the medium is destroyed after first boot | S | 2 |
| VM01-F021 | Windows 11 preset | Enables UEFI, Secure Boot, virtual TPM 2.0, 4 GB, 64 GB; attaches a virtio driver image if the user supplied one | S | 2 |
| VM01-F022 | Start after create | "Create and start" opens the console; "Create only" returns to the library | M | 1 |
| VM01-F023 | Review as text | Review page shows the resulting definition as readable TOML with a copy button | S | 1 |
| VM01-F024 | Network default | New VMs default to the isolated NAT network; a wizard switch offers "No network" | M | 1 |
| VM01-F025 | Disposable template tick | "Use as a disposable template" makes the VM a read-only base (4.13) | S | 2 |

### 4.3 Import and export

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F026 | Import OVA/OVF | Parse OVF 1.x and 2.x envelope; map CPU, memory, disks, NICs, firmware; unsupported items listed, never dropped silently | M | 2 |
| VM01-F027 | OVA manifest check | Verify `.mf` digests and, if present, the `.cert` signature; show the signer; refuse on mismatch unless the user overrides | M | 2 |
| VM01-F028 | Import QCOW2 | Versions 2 and 3, backing chains (flattened), zlib and zstd clusters; encrypted images need the passphrase; internal snapshots listed and optionally imported | M | 1 |
| VM01-F029 | Import VMDK | monolithicSparse, monolithicFlat, streamOptimized, twoGbMaxExtent; descriptor files resolved only inside the picked folder handle | M | 2 |
| VM01-F030 | Import VHD/VHDX | Fixed, dynamic and differencing (chain flattened) | M | 2 |
| VM01-F031 | Conversion in a helper | All foreign formats are parsed by `image-helper` in capability mode; it gets a read handle on the source and a write handle on the target volume, nothing else | M | 1 |
| VM01-F032 | Import preview | Before writing: virtual size, allocated size, partition table, detected OS, required pool space | M | 1 |
| VM01-F033 | Export | To OVA (OVF 2.0 + streamOptimized VMDK), QCOW2, raw, or a ZFS send stream; optional compression (zstd) | M | 2 |
| VM01-F034 | Signed export | Export can be signed with a key from SE04; the signature covers manifest digests | S | 2 |
| VM01-F035 | Export hygiene | Option to strip MAC addresses, UUID, notes, shared-folder paths and passthrough entries from the exported definition | S | 2 |
| VM01-F036 | Progress and resume | Import and export run in `svc.vm` jobs; progress in the status bar and `svc.notify`; interrupted jobs resume or clean up their partial dataset | M | 1 |

### 4.4 Lifecycle and run control

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F037 | Start | Normal; with console, headless, or paused at reset | M | 1 |
| VM01-F038 | Shut down | ACPI power button; if a guest agent is present, an agent shutdown; timeout 90 s (setting) then offer "Force off" | M | 1 |
| VM01-F039 | Force off | Immediate stop; confirmation states that unsaved guest data is lost | M | 1 |
| VM01-F040 | Reboot and reset | ACPI reboot; hard reset | M | 1 |
| VM01-F041 | Pause and resume | Freeze vCPUs; memory stays resident; shown as Paused | M | 1 |
| VM01-F042 | Suspend to disk | Save device and memory state to the VM dataset and free host memory; resume later. Uses the snapshot save path (device set in `pbsd.bifrost.snapshot.cppm`: Vioapic, Vm, Vlapic, VmMem, Vhpet, Vmcx, Vatpic, Vatpit, Vpmtmr, Vrtc) | S | 2 |
| VM01-F043 | Autostart | Per VM: start with the system (before login, headless) or at login; order and delay; handled by `svc.vm`, not by the app | M | 2 |
| VM01-F044 | Host shutdown policy | Per VM: shut down, suspend to disk, or force off when the host shuts down; global timeout | M | 2 |
| VM01-F045 | Host sleep policy | Pause guests on host sleep and correct guest clocks on wake; optional sleep inhibit while a chosen VM runs (`svc.power`) | S | 2 |
| VM01-F046 | Boot once from | One-shot boot device choice (disk, optical, network, firmware setup) | S | 1 |
| VM01-F047 | Crash handling | On triple fault or device-model crash the VM enters Crashed; the app offers restart, revert to snapshot, save a guest core dump, and open the log | M | 1 |
| VM01-F048 | Headless | VMs run in `svc.vm`; closing the console or quitting the app never stops a VM | M | 1 |
| VM01-F049 | Guest agent | Optional agent over a virtio console port: clean shutdown, quiesce file systems, IP report, time sync, display resize. The agent protocol is a fixed schema; the guest is never trusted | S | 2 |

### 4.5 Console: display

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F050 | Console window | One window per VM; the guest frame buffer is a shared-memory or dmabuf handle from the device model, shown by `viewportWidget` | M | 1 |
| VM01-F051 | Scaling modes | 1:1, fit window, integer scale, fill; nearest or linear filter | M | 1 |
| VM01-F052 | Guest resize | With a guest display driver or agent, the guest resolution follows the window | S | 2 |
| VM01-F053 | Full screen | `F11` or `Host+F`; a pull-down bar at the top edge shows VM name, Host key, and Leave | M | 1 |
| VM01-F054 | Text console | Serial console tab using the terminal grid widget; scrollback 10,000 lines; log to file | M | 1 |
| VM01-F055 | Boot firmware console | VGA text and frame buffer during firmware and boot loader | M | 1 |
| VM01-F056 | Screenshot | `Host+S` saves a PNG of the guest frame through the file portal or copies it; flagged with the VM name in provenance | S | 1 |
| VM01-F057 | Accelerated 3D | virtio-gpu with a host renderer when the host GPU stack allows it; off by default because it widens the attack surface; disabled for disposable VMs | C | 3 |
| VM01-F058 | Console lock | Console can require the user's session authentication (SH03 prompt) before it shows; for sensitive VMs | S | 2 |
| VM01-F059 | Idle console | No frames are drawn while the guest frame does not change; a hidden console unmaps the frame buffer | M | 1 |

### 4.6 Console: input, clipboard and files

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F060 | Keyboard grab | Click or focus grabs the keyboard using the compositor's shortcut-inhibit; the Host key (default Right Ctrl) releases it. Grab state is always shown in the status bar | M | 1 |
| VM01-F061 | Pointer modes | Absolute tablet (no grab) by default; relative pointer with lock for guests without tablet support | M | 1 |
| VM01-F062 | Send keys | Menu and shortcuts for Ctrl+Alt+Del, Ctrl+Alt+F1–F12, Print Screen, SysRq, Super, Alt+Tab; custom chord list | M | 1 |
| VM01-F063 | Keyboard layout | Send scancodes by physical key (default) or map by host layout; per VM | S | 2 |
| VM01-F064 | Type text | "Type clipboard as keystrokes" for guests without clipboard support; rate limited; refuses secret-flagged clipboard entries unless confirmed | S | 2 |
| VM01-F065 | Clipboard policy | Per VM: off (default), host to guest, guest to host, both. Text only by default; images optional; files never by clipboard | M | 1 |
| VM01-F066 | Clipboard rules | Size limit 1 MiB; secret-flagged entries (`svc.clipboard`) never cross; every guest-to-host paste is typed and length-checked; a toast names the VM on each crossing (setting) | M | 1 |
| VM01-F067 | One-shot paste | `Host+V` sends the clipboard once to a VM whose policy is off | S | 2 |
| VM01-F068 | File drop into guest | Drag a file on the console: with the guest agent it is copied to an inbox folder; without it the app offers to attach it as a read-only virtual disk image | S | 2 |
| VM01-F069 | Shared folders | virtio-fs or 9p share of a folder picked in the file portal; read-only by default; each share is a derived handle listed in the capability view | S | 2 |
| VM01-F070 | Files out of guest | Files leave a guest only into a quarantine inbox folder with a provenance tag "from VM <name>"; SY02 offers to open them in a disposable VM | S | 2 |
| VM01-F071 | Audio | Output on by default; microphone off and gated by `dev.mic` with the system indicator; per-VM volume | S | 2 |

### 4.7 USB and device redirection

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F072 | USB list | Devices menu lists host USB devices the user may redirect (from `svc.portal.usb`): name, vendor:product, class, current owner | M | 2 |
| VM01-F073 | Attach and detach | Ticking a device asks the shell for a `dev.usb:<class>` grant for this VM; the host driver is detached and the device appears on the guest's virtual xHCI | M | 2 |
| VM01-F074 | Policy check | Devices blocked by VM05's USB policy (for example a keyboard that appeared after lock) cannot be redirected; the reason is shown | M | 2 |
| VM01-F075 | Never-redirect classes | Host input devices in use, the boot disk and security tokens enrolled in SE04 are never offered unless unlocked in VM05 | M | 2 |
| VM01-F076 | Optical and floppy media | Insert, eject and swap ISO or floppy images at run time; physical optical drive pass-through read-only | M | 1 |
| VM01-F077 | Redirection indicator | Status bar and header show a USB badge while any device is redirected; clicking lists them with Detach | M | 2 |

### 4.8 Virtual hardware editor

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F078 | Hardware page | Tree of devices on the left, `propertyGrid` on the right; changes that need a restart are queued and marked "applies at next start" | M | 1 |
| VM01-F079 | CPU | vCPU count; sockets, cores, threads; host-passthrough or a named baseline model; CPUID feature mask list | M | 1 |
| VM01-F080 | CPU pinning and limits | Pin vCPUs to host CPUs; shares, hard cap (percent), NUMA node hint | S | 2 |
| VM01-F081 | Memory | Size; maximum for hot-plug; balloon on or off with a floor (balloon device per `pbsd.bifrost.vballoon.cppm`); wired or pageable; large pages | M | 1 |
| VM01-F082 | Firmware | UEFI (default), UEFI with Secure Boot, legacy BIOS (CSM), direct kernel boot (kernel, initrd, command line) | M | 1 |
| VM01-F083 | Secure Boot keys | Per-VM variable store; enrol standard keys, custom PK/KEK/db from files, or reset; store is a file on the VM dataset | S | 2 |
| VM01-F084 | Virtual TPM | TPM 2.0 emulator in its own sandboxed process; state encrypted with a key from `svc.keyring`; cleared on "reset TPM" | S | 2 |
| VM01-F085 | Boot order | Ordered list with drag reorder; per-device bootable flag; boot menu timeout | M | 1 |
| VM01-F086 | Disk controllers | virtio-blk, virtio-scsi, NVMe, AHCI; one device tree node each | M | 1 |
| VM01-F087 | Network adapters | virtio-net (default) or e1000-class emulation; MAC (random, fixed, re-randomise on each start); link up or down at run time | M | 1 |
| VM01-F088 | Display | Frame buffer size limit, heads, VGA compatibility, virtio-gpu 2D | M | 1 |
| VM01-F089 | Sound | HDA codec emulation, output and input endpoints, or none | S | 2 |
| VM01-F090 | Input devices | PS/2 keyboard and mouse, USB tablet, virtio input | M | 1 |
| VM01-F091 | Serial ports | Up to 4; backends: console tab, file, Unix socket handle, null | S | 1 |
| VM01-F092 | Entropy device | virtio entropy from the host generator; on by default | M | 1 |
| VM01-F093 | Nested virtualisation | Expose VMX or SVM to the guest (limit `kMaxNestDepth = 3` in `pbsd.bifrost.nested.cppm`); off by default; warning about attack surface | C | 3 |
| VM01-F094 | Definition as text | "Edit as text" opens the VM definition (TOML) in a `codeEdit` view with schema validation; saved only if valid | S | 2 |
| VM01-F095 | Change review | A diff of old and new hardware is shown before applying; Undo restores the previous definition (kept as 20 versions) | S | 2 |
| VM01-F096 | Hot-plug devices | Add or remove disks, NICs and USB devices while running where the device model allows | S | 2 |

### 4.9 Disks and storage

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F097 | ZFS volume disks | Each virtual disk is a zvol (or a file on a dataset) under the VM's own dataset `…/vm/<uuid>/`; the device model gets one handle per disk | M | 1 |
| VM01-F098 | Create disk | Size, thin (default) or reserved, block size 4K/8K/16K/64K, compression (lz4, zstd), bus | M | 1 |
| VM01-F099 | Grow disk | Online grow; shrink only when stopped, after a snapshot, with a typed confirmation | M | 2 |
| VM01-F100 | Disk options | Read-only, discard/TRIM pass-down, write cache mode (safe, fast-unsafe with warning), serial number, rotational flag, logical and physical sector size | S | 2 |
| VM01-F101 | Encryption | VM dataset uses ZFS native encryption with a key in `svc.keyring`; the VM is Locked until the key loads; optional separate passphrase per VM | M | 2 |
| VM01-F102 | Attach existing | Attach an existing image through the file portal or a block device handle from AD02 | M | 1 |
| VM01-F103 | Detach and keep | Detaching a disk keeps the volume and lists it under "Unattached disks" | S | 2 |
| VM01-F104 | Space view | Per VM: logical size, used, referenced by snapshots, compression ratio, clones depending on it | M | 2 |
| VM01-F105 | Integrity | Show the dataset's checksum errors and last scrub result; a VM with unrecoverable errors is flagged | S | 2 |

### 4.10 Snapshots and clones

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F106 | Disk snapshot | Atomic recursive ZFS snapshot of the VM dataset (all disks, definition, NVRAM, TPM state); name, note, time | M | 1 |
| VM01-F107 | State snapshot | Running VM: also saves memory and device state so a revert resumes at the same instant | S | 2 |
| VM01-F108 | Snapshot tree | Tree view with the current position marked; branches come from reverting and continuing (each branch is a ZFS clone promoted as needed) | M | 2 |
| VM01-F109 | Revert | Roll back to a snapshot; the present state is first saved as "before revert" unless the user unticks it | M | 1 |
| VM01-F110 | Delete snapshot | Shows the space that will be freed and refuses if a clone depends on it (names the clone) | M | 1 |
| VM01-F111 | Automatic snapshots | Policies: before each start, hourly while running, before hardware change; retention count per policy | S | 2 |
| VM01-F112 | Quiesce | With the guest agent, freeze file systems before a snapshot of a running guest | S | 2 |
| VM01-F113 | Linked clone | ZFS clone of a snapshot: instant, shares blocks; new UUID, MACs, and optionally hostname through unattended data | M | 1 |
| VM01-F114 | Full clone | Independent copy by send and receive | M | 2 |
| VM01-F115 | Hand-over to Backup | "Back up this VM" opens AD03 with the dataset preselected | S | 2 |

### 4.11 Networks

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F116 | Network list | Sidebar section "Networks": name, kind, subnet, attached VMs, route | M | 1 |
| VM01-F117 | Kinds | None; Isolated (guests only); NAT (default); Bridged to a host interface; Routed; Host-only | M | 1 |
| VM01-F118 | Built-in DHCP and DNS | Per virtual network, run by `svc.vm` in its own sandbox; static leases; DNS forwarding follows the network's route | M | 2 |
| VM01-F119 | Route per network | Clear, VPN or Tor (`net.route` classes of PLATFORM.md section 6); a Tor-routed network drops every packet that is not on the Tor path (fail closed) | M | 2 |
| VM01-F120 | Per-VM firewall | Inbound and outbound rules per adapter; defaults: outbound allowed on NAT, inbound denied; rules are handed to SE02 and shown read-only there | M | 2 |
| VM01-F121 | Port forwarding | Host port to guest port on NAT networks; bound to loopback by default | M | 2 |
| VM01-F122 | Bridged consent | Bridging to a physical interface needs an administrator approval through SH03 because the guest becomes visible on the LAN | M | 2 |
| VM01-F123 | MAC policy | Random locally administered MAC by default; anti-spoofing filter (guest may send only its own MAC and leased IP) | M | 2 |
| VM01-F124 | Link control | Disconnect cable, reconnect, move adapter to another network while running | S | 2 |
| VM01-F125 | Network badge | A VM with any network shows the `[net]` badge in `--net`; a VM with none shows "offline" | M | 1 |

### 4.12 PCI passthrough and IOMMU

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F126 | IOMMU status | Shows whether the host IOMMU is present and in strict mode (backends named in `pbsd.bifrost.iommu.cppm`: Intel VT-d, AMD-Vi); passthrough is unavailable without it and the UI says why | M | 2 |
| VM01-F127 | Group view | Table of IOMMU groups from `svc.device` (VM05): group, address, device, class, current driver, assignable or the reason not | M | 2 |
| VM01-F128 | Whole-group rule | A device can be passed only with every other device in its group or when the others are unbound; the dialog shows the full group | M | 2 |
| VM01-F129 | Add PCI device | Reserves the device for passthrough at next host boot or detaches it live when the driver allows; the reservation is made by VM05 after administrator approval | M | 2 |
| VM01-F130 | Safety blocks | Refuse devices that host the boot pool, the only display, or the only console input, unless the user confirms a named risk | M | 2 |
| VM01-F131 | GPU passthrough | Option ROM file, primary-display flag, vendor quirks list (the reference device model keeps `pci_passthru_quirks.c`, Verified in `hbsd/src/usr.sbin/bhyve/`) | S | 3 |
| VM01-F132 | Wired memory notice | A VM with passthrough must wire all guest memory; balloon, suspend, state snapshot and migration are disabled and marked as such | M | 2 |
| VM01-F133 | Passthrough modes | Per device: disabled, passthrough, emulated (the three values of `PassMode` in `pbsd.bifrost.pci_pass.cppm`) | S | 2 |

### 4.13 Disposable VMs

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F134 | Disposable template | A VM marked as a template is read-only; starting it creates a linked clone that is destroyed at power-off | M | 2 |
| VM01-F135 | Open in disposable VM | Intent `open` with flag `disposable`: the file is copied into the clone's inbox (read-only handle), the default handler in the guest opens it | M | 2 |
| VM01-F136 | Open link in disposable VM | Same for URLs from IN01, CO01 and others | S | 2 |
| VM01-F137 | No residue | Clone dataset, memory image, NVRAM and logs are destroyed at stop; only an audit line (time, template, file hash) remains | M | 2 |
| VM01-F138 | Tight defaults | Clipboard off, no shared folders, no USB, no audio input, network off or Tor-only per template | M | 2 |
| VM01-F139 | Visual marking | Disposable consoles have a `--warn` header stripe and the word "Disposable" in the title | M | 2 |
| VM01-F140 | Template update flow | "Update template" starts the template itself writable, with a snapshot taken first | S | 2 |

### 4.14 Capability view and security

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F141 | VmHandle tree | Inspector tab showing the VM's handle lineage: the `VmHandle` root, then derived handles for each disk, network endpoint, shared folder, USB device, PCI device, frame buffer, console | M | 2 |
| VM01-F142 | Rights per handle | Each row shows the rights bits (Read, Write, Execute, Grant, Duplicate, Destroy, Map, Irq from `pbsd/rights/pbsd.rights.cppm`) and who holds it (device model, console, agent bridge) | M | 2 |
| VM01-F143 | Revoke one | Revoke a derived handle: the device disappears from the guest at once (SI-6); logged | M | 2 |
| VM01-F144 | Revoke all | "Revoke VM" destroys the lineage subtree, which is what `destroy_vm` does (`tree.revoke_subtree(lid)` in `pbsd.bifrost.cppm`); offered as the emergency stop | M | 2 |
| VM01-F145 | Reach summary | Plain-language summary: "Can reach: 2 disks, network 'NAT' (clear route), 1 USB device. Cannot reach: your files, camera, microphone" | M | 1 |
| VM01-F146 | Isolation profile | Presets: Standard, Hardened (no clipboard, no shares, no 3D, no nested, MAC randomised), Air-gapped (no network, no USB) | S | 2 |
| VM01-F147 | Audit trail | Start, stop, snapshot, revert, hardware change, redirection, clipboard crossing (count only), migration: written to `svc.audit`, shown per VM | M | 2 |
| VM01-F148 | Provenance | Each VM records where its first disk came from: file name, hash, signature result, import time | S | 2 |
| VM01-F149 | Open in Permissions | Jump to AD05 at this VM's lineage node | S | 2 |

### 4.15 Confidential computing

Host and guest support for AMD SEV-SNP and Intel TDX is a plan item (`docs/plan/PBSD_PLAN.md` section 7), not present in `pbsd/bifrost` today.

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F150 | Capability probe | Shows whether the host supports SEV, SEV-ES, SEV-SNP or TDX and the firmware versions reported; hidden section when unsupported | S | 3 |
| VM01-F151 | Enable per VM | "Encrypt guest memory" with technology choice; requires UEFI and a supported guest | S | 3 |
| VM01-F152 | Launch policy | SNP policy bits (debug off, migration agent, SMT allowed) or TDX attributes in a `propertyGrid` | S | 3 |
| VM01-F153 | Launch measurement | Shows the expected measurement computed from firmware, kernel and command line, and the measured value; mismatch blocks start | S | 3 |
| VM01-F154 | Attestation report | Fetch, verify (certificate chain from local files, no network) and export the report; copy as text | C | 3 |
| VM01-F155 | Feature interlocks | With memory encryption on: no state snapshots, no suspend, no balloon, no memory inspection, no console thumbnail, no passthrough without device attestation; each shown as disabled with the reason | S | 3 |
| VM01-F156 | Honest label | A confidential VM is labelled "protected from the host kernel's memory access by the CPU; the display and virtual devices are still host-visible" | S | 3 |

### 4.16 Live migration

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F157 | Paired hosts | Hosts are paired once by exchanging public keys (QR, file, or typed fingerprint); keys live in SE04; no discovery broadcast by default | S | 3 |
| VM01-F158 | Pre-flight | Checks CPU feature compatibility, free memory, pool space, network names, passthrough and confidential interlocks; lists blockers | S | 3 |
| VM01-F159 | Phases | Pre-copy, stop-and-copy, resume (the `Phase` values in `pbsd.bifrost.migration.cppm`); the dialog shows phase, dirty rate, transferred bytes, expected downtime | S | 3 |
| VM01-F160 | Storage migration | Disks travel as an incremental ZFS send; a final increment is sent in the stop phase | S | 3 |
| VM01-F161 | Transport | Mutually authenticated, encrypted channel between the two `svc.vm` services; optional route through an SE03 tunnel; bandwidth limit | S | 3 |
| VM01-F162 | Cancel and failure | Cancel at any time before resume keeps the VM running on the source; a failed resume restarts on the source | S | 3 |
| VM01-F163 | Handle transfer | The source revokes the VM's lineage only after the target acknowledges resume; the audit trail records both sides | S | 3 |

### 4.17 Monitoring

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F164 | Resource graphs | Per VM: CPU per vCPU, memory (assigned, resident, ballooned), disk read and write (bytes, IOPS, latency), network in and out; `chart` widget; ranges 1 min, 1 h, 24 h, 7 d | M | 2 |
| VM01-F165 | Host budget bar | Status bar shows memory committed to running VMs against host memory, and pool free space | M | 1 |
| VM01-F166 | Alerts | Rules: disk above 90%, vCPU pegged for 10 min, guest unresponsive, pool errors; delivered by `svc.notify` | S | 3 |
| VM01-F167 | History store | Samples every 5 s kept 1 h, 1 min kept 7 d, in the app's SQLite store; written by `svc.vm`, read by the app | S | 2 |
| VM01-F168 | Logs | Per VM device-model log and serial log viewer with filter; "Open in Logs" (AD04) | M | 1 |
| VM01-F169 | Open in System Monitor | Jump to SY04 at the VM's capability domain | S | 2 |

### 4.18 Automation, settings, accessibility, recovery

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM01-F170 | Command-line tool | `vm` CLI over the same `svc.vm` interface: list, start, stop, snapshot, console, import; same grants | S | 2 |
| VM01-F171 | Intents | Handles `open` for `.ova`, `.ovf`, `.qcow2`, `.vmdk`, `.vhd`, `.vhdx`, `.vdi`, `.iso`, `.img` and `vm:` URLs (`vm:<uuid>/console`) | M | 1 |
| VM01-F172 | Command palette | Every VM action and every VM by name is in `Ctrl+K` | M | 1 |
| VM01-F173 | Local assistant help | With `ai.local`: explain a guest boot failure from the serial log, suggest a template for an unknown image. Text never leaves the machine; off by default | C | 3 |
| VM01-F174 | Session restore | Console windows that were open reopen at login for running VMs (setting) | S | 2 |
| VM01-F175 | Service loss | If `svc.vm` restarts, VMs keep running in their device-model processes; the app reconnects and re-reads state | M | 1 |
| VM01-F176 | App crash | The app holds no VM state; a crash or restart loses nothing | M | 1 |
| VM01-F177 | Definition repair | A definition that fails validation opens in a repair view listing each error with a fix | S | 2 |
| VM01-F178 | Accessible console | Screen-reader mode reads the serial console; for graphical guests the console exposes one image node with OCR text through ME05's OCR helper on request | S | 3 |
| VM01-F179 | First-run check | On first start the app shows whether hardware virtualisation and the IOMMU are available and what is missing | M | 1 |

## 5. GUI

### 5.1 Window map

```
Virtual Machines
├─ Library window (main)
│  ├─ Sidebar: All, Running, groups, smart filters, Networks, Templates, Images, Jobs
│  ├─ Content: VM list / grid · network list · template list · image list · job list
│  ├─ Inspector (F4): Summary · Hardware · Snapshots · Capabilities · Graphs · Log
│  ├─ New-VM wizard (sheet, 5 pages)
│  ├─ Import sheet (mapping review)
│  ├─ Export dialog
│  ├─ Hardware editor (full page inside the window)
│  │  ├─ Add device popover
│  │  ├─ Add PCI device dialog (IOMMU groups)
│  │  └─ Edit as text view
│  ├─ Snapshot tree page
│  │  ├─ Take snapshot popover
│  │  └─ Revert confirmation
│  ├─ Network editor dialog (general, DHCP, forwarding, firewall)
│  ├─ Migrate dialog (host, pre-flight, progress)
│  ├─ Pair host dialog
│  ├─ Delete VM confirmation
│  └─ Preferences (Ctrl+,)
├─ Console window (one per running VM; one more per extra head)
│  ├─ Toolbar · Devices menu · Send-key menu
│  ├─ Tabs: Display · Serial 1–4 · Log
│  ├─ Full-screen pull-down bar
│  └─ USB redirect popover
├─ Disposable console window (same, with warning stripe)
└─ Shell-drawn prompts (SH04): dev.usb grant, dev.mic, dev.camera, administrator approval
```

### 5.2 Main window

Shown at 1180×720, wide breakpoint, one VM selected.

```
+----------------------------------------------------------------------------------------+
| [=] Virtual Machines                 [ Search VMs  Ctrl+F ]            [▁] [▢] [✕]    |
+------------+---------------------------------------------------+-----------------------+
| LIBRARY (1)| [+ New] [Import] | [▶ Start] [⏸] [■] [Snapshot]  (2)| debian-dev        (5) |
|  All    12 |                          [list|grid]  [filter v]    | ● Running  3 h 12 m   |
|> Running 3 +---------------------------------------------------+ Linux · UEFI · 4 vCPU |
|  Work    5 | Name         State     CPU   Mem     Disk   Net (3)| 8 GiB · 64 GiB        |
|  Lab     4 |> debian-dev  ● Running 12 %  6.1/8G  21/64G [net]  | [Summary][Hardware]   |
|  Servers 3 |  win11-test  ⏸ Paused   0 %  8/8G    48/80G [net]  | [Snapshots][Caps]     |
| FILTERS    |  dns-server  ● Running  1 %  0.4/1G  2/8G   [net]  | [Graphs][Log]         |
|  Autostart |  openbsd-fw  ○ Stopped  –    –/1G    1/4G   offline| --------------------- |
|  Passthru  |  tor-ws      ○ Stopped  –    –/4G    9/32G  [tor]  | Can reach:        (6) |
|  Disposable|  win-games   ○ Stopped  –    –/16G   210/256G [pci]|  2 disks (rw)         |
| NETWORKS   |  …                                                  |  network NAT (clear)  |
|  NAT       |                                                     |  1 USB: YubiKey       |
|  Isolated  |                                                     | Cannot reach:         |
|  Tor-only  |                                                     |  your files, camera,  |
| TEMPLATES  |                                                     |  microphone           |
| IMAGES     |                                                     | [Open console]        |
| JOBS    1  |                                                     | [Revoke VM…]          |
+------------+---------------------------------------------------+-----------------------+
| 3 running · memory 14.5 / 32 GiB committed (4) · pool 412 GiB free · 1 job: import 43% |
+----------------------------------------------------------------------------------------+
```

1. Sidebar (`treeView`, 220 px): library groups, smart filters, networks, templates, images, jobs. Counts on the right. Drop target for VMs.
2. Toolbar (`toolButton` row): New is the primary action. Run controls act on the selection and are disabled with a reason when not applicable.
3. VM table (`tableView`, virtualised). Columns: name, state, CPU, memory used/assigned, disk used/size, network badge. Grid view is a `gridView` of 240×180 tiles with thumbnails.
4. Status bar: running count, host memory budget (`progressBar`, turns `--warn` above 85%), pool free space, job summary (click opens Jobs).
5. Inspector (`tabWidget`, 280 px): header with state and uptime, then six tabs.
6. Reach summary: plain list built from the handle tree. "Revoke VM…" is the outlined destructive button.

States: empty (no VMs: "No virtual machines yet" and `[+ New]`), loading (skeleton rows while `svc.vm` answers), error (service unreachable: banner with Retry and "Open Logs"), unsupported host (banner: "Hardware virtualisation is off in firmware"; VMs listed read-only), locked (a VM whose key is not loaded shows a lock and "Unlock"), read-only (administrator policy), narrow (sidebar becomes overlay, inspector a bottom sheet, table shows name and state only).

### 5.3 Other views and dialogs

#### New-VM wizard, page 1 (sheet, 760×520)

```
+--------------------------------------------------------------------------+
| New virtual machine                              1 Source  2 3 4 5       |
+--------------------------------------------------------------------------+
| What do you want to run?                                                  |
| (•) Installer image   ( ) Existing disk image   ( ) Appliance (OVA)       |
| ( ) Network boot      ( ) Empty machine         ( ) Clone of…             |
|                                                                           |
| Image  [ debian-13.1-amd64-netinst.iso            ] [Choose…]   (1)       |
|        ✓ SHA-256 matches the value you pasted · 742 MiB                    |
| Checksum or signature (optional) [______________________________]  (2)    |
|                                                                           |
| Detected: Debian 13 (volume id "Debian 13.1.0 amd64 n")           (3)     |
| Template [ Linux – Debian family         v]                               |
|   UEFI · 2 vCPU · 4 GiB · 32 GiB virtio disk · virtio network · NAT       |
| Name     [ debian-13                    ]                                 |
| [ ] Unattended install…   [ ] Use as a disposable template                |
+--------------------------------------------------------------------------+
| [Cancel]                          [Customise →]   [Create and start]      |
+--------------------------------------------------------------------------+
```

(1) Drop target; uses the file portal. (2) Accepts a hex digest or a `.sig`/`.asc` file. (3) Evidence for the guess. States: no image, verifying, verified, mismatch (`--danger`, Create disabled until overridden), unknown OS (template "Generic UEFI").

#### Console window (1024×820)

```
+--------------------------------------------------------------------------+
| [=] debian-dev — Console                    [net] [usb]   [▁] [▢] [✕]    |
+--------------------------------------------------------------------------+
| [⏸] [■ v] [Snapshot] | [Send key v] [Devices v] [Clipboard: off v] [⛶]   |
+--------------------------------------------------------------------------+
| [Display] [Serial 1] [Log]                                          (1)  |
| +----------------------------------------------------------------------+ |
| |                                                                      | |
| |                     guest frame buffer (2)                           | |
| |                                                                      | |
| +----------------------------------------------------------------------+ |
+--------------------------------------------------------------------------+
| Keyboard: captured · release with Right Ctrl (3)   1280×800 · fit · 60 Hz |
+--------------------------------------------------------------------------+
```

(1) `tabBar`. (2) `viewportWidget` fed by a frame handle; letterboxed on `--bg-0`. (3) Grab state, always visible. States: starting (firmware text), paused (dimmed frame, "Paused" overlay, Resume button), stopped ("This VM is stopped" and Start), crashed (reason, log excerpt, Restart, Revert), locked (session authentication needed), disposable (4 px `--warn` stripe under the header and "Disposable" in the title).

#### Hardware editor (page, 1180×720)

```
+----------------------+---------------------------------------------------+
| debian-dev hardware  | Disk 1 — virtio-blk                               |
| ▸ System             | Volume    tank/home/dara/vm/3f2a…/disk0           |
|   CPU   4            | Size      [ 64 ] GiB   [Grow…]                    |
|   Memory 8 GiB       | Used      21.3 GiB · compression 1.62×            |
|   Firmware UEFI      | Bus       [virtio-blk v]   Bootable [x]  Order 1  |
|   TPM 2.0            | Read-only [ ]   Discard [x]   Cache [safe v]      |
| ▾ Storage            | Serial    [ PBSD3F2A0001 ]                        |
|  > Disk 1  64 GiB    | I/O limit [ none v]                               |
|   Optical (empty)    |                                                   |
| ▾ Network            | ⚠ Bus change applies at next start          (1)   |
|   Adapter 1  NAT     |                                                   |
| ▸ Display · Sound    | [Detach and keep]   [Delete disk…]                |
| ▸ USB · Serial · PCI |                                                   |
| [+ Add device v]     | 2 pending changes  [Review…] [Discard] [Apply]    |
+----------------------+---------------------------------------------------+
```

`treeView` plus `propertyGrid`. (1) Pending-change note. Apply is primary. States: running (hot-plug-able fields live, others queued), read-only (snapshot view or policy).

#### Snapshot tree (page)

```
+--------------------------------------------------------------------------+
| Snapshots of debian-dev                    [Take snapshot Ctrl+T] [Policy]|
+--------------------------------------------------------------------------+
| ● clean-install        12 Oct 09:14   disk         1.2 GiB   🔒 keep      |
| └─● toolchain-ready    14 Oct 16:02   disk+state   3.4 GiB                |
|    ├─● before-upgrade  02 Nov 11:40   disk         0.8 GiB                |
|    │  └─▶ You are here                                                    |
|    └─● experiment-a    20 Oct 10:05   disk         2.1 GiB   clone: lab-2 |
+--------------------------------------------------------------------------+
| Selected: before-upgrade · "apt full-upgrade next"                        |
| [Revert to this…] [Clone from this…] [Compare…] [Delete…]  frees 0.8 GiB  |
+--------------------------------------------------------------------------+
```

`treeView` with columns. Delete is disabled with a tooltip when a clone depends on the snapshot.

#### Add PCI device (dialog, 820×520)

```
+--------------------------------------------------------------------------+
| Pass a PCI device to win-games                                            |
+--------------------------------------------------------------------------+
| IOMMU: AMD-Vi · strict mode ✓                                             |
| Group Address   Device                        Driver     Status           |
|  12   0b:00.0   GPU (display controller)      drm        assignable  (1)  |
|  12   0b:00.1   HDMI audio                    snd_hda    same group       |
|  14   0d:00.3   USB 3 controller              xhci       assignable       |
|   7   04:00.0   NVMe controller               nvme       ✕ boot pool (2)  |
+--------------------------------------------------------------------------+
| Selecting 0b:00.0 also takes 0b:00.1 (same group).                        |
| The host loses this display output until the VM stops.              (3)   |
| Needs: administrator approval · all guest memory wired (16 GiB)           |
| [Cancel]                 [Open in Devices]          [Reserve and add]     |
+--------------------------------------------------------------------------+
```

(1) Rows are grouped; selecting one selects its group. (2) Blocked rows say why. (3) Consequences in plain words.

#### Capabilities tab (inspector, widened to 420 px)

```
+------------------------------------------------------------+
| VmHandle 3f2a…  rights: R W Map Destroy     holder: svc.vm |
| ├─ disk0   zvol …/disk0       R W        device model      |
| ├─ disk1   zvol …/disk1       R          device model      |
| ├─ net0    network "NAT"      R W        device model      |
| ├─ fb0     frame buffer       R  Map     console window    |
| ├─ usb     YubiKey 1050:0407  R W        device model  [✕] |
| └─ share   ~/Projects/demo    R          9p server     [✕] |
| [Revoke selected]  [Open in Permissions]  [Revoke VM…]     |
+------------------------------------------------------------+
```

`treeView`; `[✕]` revokes one derived handle.

### 5.4 Menus

```
File      New VM… Ctrl+N · Import… Ctrl+I · Export… Ctrl+E · New network… ·
          New group · Preferences Ctrl+, · Close Ctrl+W · Quit Ctrl+Q
Edit      Undo Ctrl+Z · Redo Ctrl+Shift+Z · Rename F2 · Duplicate (linked clone) Ctrl+D ·
          Full clone… Ctrl+Shift+D · Select all Ctrl+A · Find Ctrl+F · Delete Delete ·
          Delete for ever Shift+Delete
View      List Ctrl+1 · Grid Ctrl+2 · Sidebar F9 · Inspector F4 · Hardware Ctrl+H ·
          Snapshots Ctrl+Shift+S · Graphs Ctrl+G · Jobs Ctrl+J · Refresh F5 · Full screen F11
Machine   Start Ctrl+R · Start headless Ctrl+Shift+R · Open console Enter · Pause Ctrl+. ·
          Shut down Ctrl+Shift+Q · Reboot · Reset · Force off… · Suspend to disk ·
          Boot once from ▸ · Take snapshot Ctrl+T · Revert to last… Ctrl+Shift+T ·
          Migrate… · Start with the system · Use as disposable template · Revoke VM…
Devices   (console) Optical ▸ · USB ▸ · Network adapters ▸ · Shared folders ▸ ·
          Clipboard ▸ (Off, To guest, From guest, Both) · Audio ▸ · Paste once Host+V
Input     (console) Send key ▸ · Type clipboard · Grab keyboard · Release Host ·
          Pointer mode ▸ · Screenshot Host+S
File also has Print summary Ctrl+P.
Help      Help F1 · Guest tools guide · Host support check · About
```

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| `Enter` / double click | Open console (start if stopped, after one confirmation setting) |
| `Ctrl+R`, `Ctrl+.`, `Ctrl+Shift+Q` | Start, pause or resume, shut down |
| `Ctrl+T`, `Ctrl+Shift+T` | Snapshot, revert to last |
| `Ctrl+D` | Linked clone |
| `Host` (Right Ctrl) | Release keyboard and pointer |
| `Host+Del`, `Host+F`, `Host+S`, `Host+V`, `Host+P`, `Host+Q` | Ctrl+Alt+Del, full screen, screenshot, paste once, pause, close console |
| `Host+F1…F12` | Ctrl+Alt+Fn |

Context menu on a VM: Open console, Start/Stop submenu, Snapshot, Clone, Hardware, Move to group, Tags, Export, Show dataset in Disks (AD02), Open in Permissions (AD05), Delete. On a snapshot: Revert, Clone from, Keep, Rename, Delete. On a network: Edit, Attached VMs, Delete.

Drag and drop: image file on the library starts the wizard or import; VM on a group moves it; VM on a network attaches adapter 1; file on a console (copies it to the guest inbox); device row on the hardware tree reorders boot order. Every drag has a menu equivalent.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | One line and `[+ New]`; a second link "Import an existing machine" |
| Loading | Skeleton rows; controls disabled; no spinner over 150 ms without text |
| Error | Inline banner with cause, Retry, Details; a failing VM row shows `--danger` icon and the last log line |
| Offline | Irrelevant to the app (it has no network). Guests with no route show "offline" |
| Locked | Encrypted VM: lock icon, "Unlock" asks `svc.keyring`; console requires authentication when set |
| Read-only | Policy or snapshot view: fields disabled, banner names the policy |
| Narrow (<640) | Sidebar overlay, inspector sheet, toolbar collapses to overflow menu; console toolbar keeps Send key and Devices |
| Service restarting | Banner "Reconnecting to the VM service"; VMs keep running |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar, hardware tree, snapshot tree, handle tree | `treeView` |
| VM table, IOMMU groups, jobs, leases | `tableView` (model-backed) |
| VM grid | `gridView` |
| Inspector tabs, console tabs | `tabWidget`, `tabBar` |
| Device properties, launch policy | `propertyGrid` |
| Graphs | `chart` |
| Guest display | `viewportWidget` with an external frame source |
| Definition text | `codeEdit` |
| Notes, help | `markdownView` |
| Wizard, dialogs | `dialog`, `radioButton`, `comboBox`, `lineEdit`, `spinBox`, `slider`, `checkBox`, `toggleSwitch`, `progressBar` |
| Menus, toolbars | `menuBar`, `popupMenu`, `toolButton` |

Missing:

- `VM01-GAP-1`: `svc.vm`, a VM manager system service (not in PLATFORM.md section 8).
- `VM01-GAP-2`: a `viewportWidget` frame source that takes an external shm or dmabuf handle with damage rectangles (related to G-11 and FUSE-WP-10.8).
- `VM01-GAP-3`: compositor keyboard-shortcut inhibit and pointer lock exposed to apps (FUSE-WP-08.5, FUSE-WP-10.7).
- `VM01-GAP-4`: an administrator-approval prompt in SH03/SH04 for actions that a per-app capability cannot cover (bridging, passthrough).
- Serial console needs the terminal grid widget (G-13).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | View models, wizard, definition editing and validation, intents |
| UI | FUSE UI | app host | Windows in 5.1 |
| `image-helper` | C++23 | helper, capability mode | Parse ISO 9660/UDF ids, OVF, QCOW2, VMDK, VHD/VHDX, VDI; convert to raw; write OVA/QCOW2 |
| VM service `svc.vm` | C++23 | system service (Proposed, VM01-GAP-1) | Registry of VMs, holds `VmHandle` roots, starts device models, networks, jobs, autostart, metrics, migration endpoint |
| Device model | C++23 | one process per VM, capability mode | Virtual devices, frame buffer, virtio back ends; holds only this VM's derived handles. Reference: `hbsd/src/usr.sbin/bhyve` (C, Verified to exist); `pbsd/usr.sbin/bhyve/console_m` and `usb_emul_m` hold `port.cppm` files (Verified to exist, content not reviewed) |
| vTPM | C++23 | one process per VM | TPM 2.0 emulation |
| Network helpers | C++23 | one sandbox per virtual network | DHCP, DNS forwarder, NAT set-up through SE02's interface |
| Hypervisor | C++23 kernel modules | kernel | `pbsd/bifrost` (P5-03) |
| `vm` CLI | ForgeScript | own process | Same bus interface |

```
 app host (ForgeScript + FUSE UI) ──bus──▶ svc.vm ──VmHandle──▶ BIFROST (kernel)
     │  frame handle (shm/dmabuf) ◀──────────┤
     └─ image-helper (cap mode)              ├─ device model  (per VM, cap mode)
                                             ├─ vTPM          (per VM)
                                             ├─ dhcp/dns      (per network)
                                             └─ migration endpoint ◀─TLS─▶ peer svc.vm
```

The app never holds a `VmHandle` with `Grant`. It holds a narrowed handle (Read, plus Write for run control) per VM that `svc.vm` hands out; console frame handles are Read+Map only.

### 6.2 Data model

VM definition `vm.toml` (schema version 1), stored by `svc.vm` on the VM dataset; the app edits it through the service.

| Field | Type | Notes |
|---|---|---|
| `uuid` | uuid | Stable identity; dataset name |
| `name`, `notes`, `tags[]`, `group` | string | Display data |
| `template` | string | Template id used at creation |
| `firmware.kind` | enum `uefi`, `uefi-secure`, `bios`, `direct` | |
| `cpu.count`, `cpu.max`, `cpu.sockets`, `cpu.cores`, `cpu.threads` | u32 | Maps to `VmConfig.vcpus` |
| `cpu.model`, `cpu.mask[]`, `cpu.pin[]`, `cpu.cap_percent` | string, list, list, u8 | |
| `memory.bytes`, `memory.max_bytes`, `memory.balloon_floor`, `memory.wired` | u64, u64, u64, bool | Maps to `VmConfig.ram_bytes` |
| `disks[]` | `{id, volume, bus, bootable, readonly, discard, cache, serial, iops_limit, bps_limit}` | |
| `nics[]` | `{id, model, network, mac, mac_policy, vlan, rules[], limit_up, limit_down}` | |
| `display` | `{heads, max_width, max_height, vga, gpu3d}` | |
| `sound`, `usb`, `serial[]`, `tpm`, `rng`, `watchdog` | records | |
| `pci[]` | `{address, group, mode, rom, return_to_host}` | `mode` mirrors `PassMode` |
| `confidential` | `{tech, policy, expected_measurement}` | |
| `policy` | `{clipboard, shares[], usb_rules[], isolation_profile, console_auth}` | |
| `lifecycle` | `{autostart, order, delay_s, on_host_shutdown, disposable_template}` | |
| `provenance` | `{source_name, source_hash, signature, imported_at}` | |

Other stores:

| Store | Location | Content |
|---|---|---|
| VM dataset | `<pool>/…/vm/<uuid>/` | `vm.toml`, `vm.toml.history/` (20 versions), disks, `nvram.bin`, `tpm/`, `state/` (memory images), `log/` |
| Networks | `svc.vm` store, `networks.toml` | `{id, name, kind, subnet4, subnet6, dhcp, route, forwards[], isolated}` |
| App state | `~/.local/share/org.pbsd.virtual-machines/library.db` (SQLite) | Tables `ui_state`, `metric_sample(vm, t, cpu, mem, rd, wr, rx, tx)`, `recent`, `job_history` |
| Templates | bundle `assets/templates/*.toml`, user `templates/` | Signed (bundle) or user-owned |
| Paired hosts | `svc.keyring` | Peer name, public key, fingerprint |

Every store carries `schema_version`. Migration is forward only; the previous `vm.toml` is kept in history and the dataset is snapshotted first.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| VM definition TOML | yes | yes | Own code (FORGE data library, FORGE-WP-23) | app host; re-validated in `svc.vm` |
| ISO 9660, UDF, El Torito (ids only) | yes | no | Own code | `image-helper` |
| OVF 1.x/2.x XML, OVA tar | yes | yes | Own code (small XML subset reader, tar) | `image-helper` |
| QCOW2 v2/v3 | yes | yes | Own code | `image-helper` |
| VMDK, VHD, VHDX, VDI | yes | VMDK streamOptimized only | Own code | `image-helper` |
| Raw, GPT/MBR preview | yes | yes | Own code | `image-helper` |
| ZFS send stream | no (kernel) | no (kernel) | OpenZFS in base | kernel, started by `svc.vm` |
| Cloud-init NoCloud, kickstart, preseed, `autounattend.xml` | no | yes | Own code, templates | app host (writes text), `image-helper` (builds ISO) |
| virtio (blk, net, console, input, balloon, entropy, gpu) | – | – | Device model; kinds listed in `pbsd.bifrost.virtio.cppm` `PvKind` | device model |
| Guest agent protocol | yes | yes | Own schema over a virtio console port | agent bridge inside the device model sandbox |
| Migration stream | yes | yes | Own framing over an authenticated channel (FORGE-WP-21, FORGE-WP-22) | `svc.vm` migration endpoint sandbox |
| SEV-SNP / TDX reports | yes | no | Own code; certificate checks through FORGE crypto | `image-helper`-class sandbox |

No third-party library is required. zstd and zlib decompression use the copies already in the base system (BSD and zlib licences).

### 6.4 Integration

- Intents handled: `open` (image and appliance MIME types, `vm:` URLs), `open` with `disposable`. Emitted: `share` (exported files), `print`, `open` to AD02, AD03, AD04, AD05, SY04, VM05, DV05.
- MIME: `application/ovf`, `application/x-virtualbox-ova`, `application/x-qemu-disk`, `application/x-vmdk-disk`, `application/x-vhd-disk`, `application/x-vhdx-disk`, `application/x-virtualbox-vdi`, `application/x-cd-image`, `application/x-raw-disk-image`, `application/x-pbsd-vm+toml`.
- Services: `svc.vm` (gap), `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.usb`, `svc.portal.mic`, `svc.portal.camera`, `svc.intent`, `svc.search`, `svc.keyring`, `svc.clipboard`, `svc.power`, `svc.backup`, `svc.audit`, `svc.model` (optional).
- IPC offered: none by the app. `svc.vm` offers `org.pbsd.vm.Manager/1` (list, define, start, stop, snapshot, clone, console, jobs), used also by VM03 (VM fallback), SY02 (open in disposable VM), SE01 (isolation posture).
- Other apps: VM05 owns PCI reservation and USB policy; SE02 shows VM rules; SE03 supplies VPN and Tor routes; SE04 holds keys; AD02 shows datasets; AD03 backs up; AD05 shows lineage; SY03 can attach to a serial console through `vm console`.
- Backing repo facts are cited inline in section 4. Everything else is Proposed.

## 7. Security and privacy

### Capabilities requested

| Capability | Reason |
|---|---|
| `fs.app` | UI state, user templates, metric cache |
| `fs.portal` | Pick installer images, images to import, export targets, shared folders |
| `proc.helper:image-helper` | Parse and convert foreign image formats |
| `svc.vm` | All VM operations (VM01-GAP-1) |
| `svc.settings`, `svc.notify`, `svc.intent`, `svc.search`, `svc.audit` | Preferences, job and alert notices, intents, library search, audit lines |
| `svc.keyring` | Dataset keys, vTPM state keys, paired-host keys |
| `svc.clipboard` | Clipboard policy bridge (only when a VM's policy is not off) |
| `svc.power` | Sleep inhibit, host shutdown coordination |
| `svc.backup` | Include and exclude rules for VM datasets |
| `dev.usb:*` | USB redirection; prompted per device and per VM |
| `dev.mic`, `dev.camera` | Guest microphone and webcam; off by default |
| `ai.local` | Optional log explanation |
| `net.none` | The app itself opens no sockets |

Not requested: `net.*` (guests' traffic is carried by `svc.vm` under SE02 rules), `proc.exec`, `fs.path`, `dev.screen`, `bg.run` (VMs live in the service).

### Threat model

Assets: the host kernel and other domains; the user's files; other VMs; guest disks and memory; keys; the truth of what the UI shows.

| Adversary | Attack | Mitigation |
|---|---|---|
| Hostile guest | Escape through a virtual device | Device model per VM in capability mode with W^X, only that VM's handles (SI-8, SI-2); small default device set; 3D and nested off by default; device model at proof tier T3 with fuzzing |
| Hostile guest | Abuse the console channel (frame size, cursor, clipboard, agent messages) | Frame handle is Read+Map with size checked by `svc.vm`; clipboard typed and size-limited; agent schema fixed; all guest strings shown as plain text |
| Hostile guest | Read host or other VMs through passthrough DMA | IOMMU strict mode required; whole-group rule; memory wired |
| Hostile image file | Parser bug in QCOW2/VMDK/OVF | Parsing only in `image-helper` (capability mode, two handles); size and chain-depth limits; fuzzers |
| Hostile image file | Path escape through VMDK descriptor or OVF `href` | References resolved only inside the picked folder handle; absolute paths refused |
| Network attacker | Reach a guest or the migration endpoint | Inbound denied by default; forwards on loopback; migration only between paired keys; no listener unless pairing is configured |
| Malicious USB device | Attack the host driver before redirection | VM05 policy decides before any driver binds; redirection detaches the host driver |
| Local other app | Drive VMs | `svc.vm` checks peer credentials and per-VM grants; VMs are per user |
| User error | Confusing a guest window for the host | Console chrome is host-drawn; disposable stripe; grab state always shown; shell prompts are drawn by the shell |
| Host compromise (limited) | Read guest memory | Optional SEV-SNP/TDX with the honest label of section 4.15 |

Stored: definitions, disks and state on the VM dataset; UI state and metrics in the app data directory; keys in `svc.keyring`. Leaves the machine: nothing by the app. Guest traffic leaves only on networks the user attached. Migration and export leave only on explicit action.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `library.view` | enum `list`, `grid` | `list` | Library layout |
| `console.host_key` | key | `Right Ctrl` | Release and chord key |
| `console.scale_mode` | enum `one`, `fit`, `integer`, `fill` | `fit` | Default scaling |
| `console.grab_on_focus` | bool | `true` | Grab keyboard when focused |
| `console.reopen_at_login` | bool | `false` | Reopen consoles of running VMs at login |
| `defaults.clipboard` | enum `off`, `to-guest`, `from-guest`, `both` | `off` | Policy for new VMs |
| `defaults.network` | string | `nat` | Network for new VMs |
| `defaults.pool` | dataset path | user VM dataset | Where new VMs are created |
| `shutdown.timeout_s` | u32 | `90` | Before offering Force off |
| `shutdown.on_host_shutdown` | enum `shutdown`, `suspend`, `force` | `shutdown` | Default policy |
| `snapshots.before_revert` | bool | `true` | Save present state on revert |
| `disposable.pool_size` | u8 0–3 | `0` | Warm pool |
| `delete.keep_days` | u32 | `30` | Disk retention after delete |
| `ai.explain_logs` | bool | `false` | Needs `ai.local` |

## 9. Accessibility and languages

- The guest display is opaque to the a11y tree. The console exposes: VM name, state, grab state, resolution, and an action "Read screen text" (OCR on request). The serial tab is fully readable.
- The Host key and every chord are rebindable; a one-hand mode maps Host chords to a menu.
- Keyboard grab can never trap a user: `Host` always releases, and the shell's `Super+L` is never forwarded.
- Graph data is available as a table view for screen readers.
- Guest keyboard layout mapping honours AX04 input methods when "map by host layout" is on.
- All strings in tables; device names from the guest are shown verbatim and never translated.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | 300 ms with 200 VMs in the library |
| Library refresh | 16 ms per frame while 20 VMs report metrics at 1 Hz |
| Console latency added by the app | under 1 frame at 60 Hz; no copy of the frame (handle is mapped) |
| Console idle | zero frames when the guest frame is unchanged |
| App memory | under 120 MiB with 4 consoles open, excluding mapped guest frames |
| Linked clone | under 1 s |
| Disposable VM to usable console | under 10 s cold, under 2 s from the warm pool |
| Import | limited by disk; helper uses at most 256 MiB |
| Largest supported | 512 VMs in the library, 64 vCPUs and 1 TiB memory per VM (subject to the hypervisor) |

## 11. Testing and proof

Nothing has been run. All entries are targets.

- Unit: definition validation, template merge, diffing, policy evaluation, MAC generation, snapshot-tree model.
- Golden renders: every view in 5.2 and 5.3 in dark, light, high contrast, 100% and 200%.
- Scripted UI (a11y tree): tasks T1–T15 against a fake `svc.vm`.
- Fuzz: each `image-helper` parser (QCOW2, VMDK, VHD, VHDX, VDI, OVF, tar, ISO ids); the agent protocol decoder; the migration stream decoder; the definition parser.
- Differential: converted images compared block-for-block with `qemu-img convert` output on a Linux reference machine (test-time only); OVF mapping compared with a table of known appliances.
- Integration: boot matrix of template guests under the device model; snapshot and revert; clone identity; USB redirection with an emulated device; passthrough on lab hardware; migration between two hosts.
- Fault injection: kill the app, `svc.vm`, a device model, the helper; pull the pool; fill the pool.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Handle narrowing in `svc.vm` | A client never receives rights beyond its grant (SI-1) | T1 | Model in Lean 4 or TLA+, code contracts by ESBMC through PRISM | – |
| Lineage revoke on destroy | All derived handles invalid after revoke (SI-6) | T1 | Same, on the lineage model | – |
| `image-helper` parsers | No out-of-bounds, no overflow, bounded allocation | T2 | Safe subset, contracts, bounded model checking, fuzzing | Format complexity (QCOW2 refcounts) |
| Definition validator | Every accepted definition satisfies the schema invariants | T2 | Contracts and property tests | – |
| Device model | Memory safety of device emulation | T3 | Safe subset, sanitizers, fuzzing per device | Size; code adapted from bhyve |
| Migration framing | Decoder total on any input | T2 | Contracts, fuzzing | – |
| App logic (ForgeScript) | No capability widening | T3 | FSS policy from the manifest; script tests | Interpreter not verified |
| UI | – | T4 | Golden and scripted tests | – |

## 12. Build plan

### Phase 1: usable core

| ID | Work item | Depends on |
|---|---|---|
| VM01-W01 | `svc.vm` interface schema `org.pbsd.vm.Manager/1` and a fake service for tests | G-08, VM01-GAP-1 |
| VM01-W02 | Definition schema, validator, template set | G-07 |
| VM01-W03 | Library window (list, sidebar, inspector summary, search) | G-01, G-02, VM01-W01 |
| VM01-W04 | `svc.vm` core: registry, datasets, start and stop through BIFROST | P5-03, G-06, G-09 |
| VM01-W05 | Device model with virtio blk, net, console, entropy, frame buffer, PS/2, UEFI | P5-03, VM01-W04 |
| VM01-W06 | Console window: display, keyboard and pointer, send keys, serial tab | VM01-GAP-2, VM01-GAP-3, G-13, VM01-W05 |
| VM01-W07 | New-VM wizard pages 1–5, image detection and verification | VM01-W02, `image-helper` (W08) |
| VM01-W08 | `image-helper`: ISO ids, raw, QCOW2 import | FORGE-WP-27, FORGE-WP-28 |
| VM01-W09 | Hardware editor (CPU, memory, firmware, disks, NICs, boot order) | VM01-W03 |
| VM01-W10 | Disk snapshots, revert, linked clone | VM01-W04 |
| VM01-W11 | NAT and isolated networks with DHCP | SE02 interface, VM01-W04 |
| VM01-W12 | Crash handling, service reconnect, logs tab | VM01-W04 |

### Phase 2: full feature

| ID | Work item | Depends on |
|---|---|---|
| VM01-W13 | OVA, VMDK, VHD/VHDX, VDI import; OVA and QCOW2 export; signed export | VM01-W08, SE04 |
| VM01-W14 | Snapshot tree, state snapshots, suspend to disk, policies | VM01-W10, P5-03 snapshot path |
| VM01-W15 | Clipboard policy, shared folders, file drop, guest agent | VM01-W06, G-09 (`svc.clipboard`) |
| VM01-W16 | USB redirection with VM05 policy | VM05 (`svc.device`), `svc.portal.usb` |
| VM01-W17 | Capability view and revoke | FORGE-WP-06, AD05 |
| VM01-W18 | Disposable VMs and the `open disposable` intent | VM01-W10, SY02, SH04 |
| VM01-W19 | Autostart, host shutdown policy, encryption, vTPM, Secure Boot | SE04, VM01-W04 |
| VM01-W20 | Network routes (VPN, Tor), firewall rules, port forwards, bridged with approval | SE02, SE03, VM01-GAP-4 |
| VM01-W21 | PCI passthrough and IOMMU group dialog | P3-02, VM05, VM01-GAP-4 |
| VM01-W22 | Graphs, history store, audit trail | VM01-W04, G-09 (`svc.audit`) |
| VM01-W23 | `vm` CLI | VM01-W01 |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| VM01-W24 | Live and offline migration, host pairing | P5-03 migration, FORGE-WP-21, FORGE-WP-22, SE04 |
| VM01-W25 | Confidential computing options | Plan section 7 item, VM01-W19 |
| VM01-W26 | GPU passthrough quirks, SR-IOV, return to host | VM01-W21 |
| VM01-W27 | Multi-monitor, 3D, seamless view, recording | G-11, VM01-W06 |
| VM01-W28 | Warm pool, lifetime limits, alerts, scheduled actions | VM01-W18, VM01-W22 |
| VM01-W29 | Nested virtualisation, debug stub, exit statistics | P5-03 |
| VM01-W30 | Other definition imports, physical-to-virtual, hooks, assistant help | AD02, G-15 |

## 13. Open questions and risks

- BIFROST today is a scaffold with one placeholder VM (Verified). The whole app waits on P5-03. Risk: high.
- Device model source. The reference is bhyve's C userland. The platform rule asks for memory-safe C++23; the port size is large. Decide whether an interim sandboxed C device model is acceptable.
- Who owns VMs: per user (this spec) or system-wide with delegation? Autostart before login needs a system owner; proposed answer is a per-user registry inside one system service.
- `docs/migration/LINUXULATOR.md` describes the BIFROST guest as "Windows guest via SI-8 VmHandle", and SI-8 itself says "(Windows)". This spec assumes any guest OS; the invariant text should be widened.
- Keyboard grab depends on compositor protocols not yet verified (VM01-GAP-3).
- Licence: QCOW2/VMDK/VHDX readers are written from the public format documents; no GPL code is copied.
- Confidential computing and migration interlock rules depend on hardware not available in CI.
- Tor-routed virtual networks need SE03 to offer a transparent, fail-closed gateway; Unverified.
- Windows guests need signed virtio drivers; the suite does not ship them. The user supplies the driver image.
