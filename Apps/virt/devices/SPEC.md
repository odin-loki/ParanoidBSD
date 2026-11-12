<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Devices

By Odin Loch

| | |
|---|---|
| ID | `VM05` |
| Bundle | `org.pbsd.devices` |
| Category | virt |
| Priority | P1 |
| Backing repos | ParanoidBSD (`pbsd/uda`, `docs/specs/UDA_DESCRIPTOR.md`, plan Phase 3, `hbsd/src/usr.sbin/fwget`) |
| Status | Planned. Nothing built. |

## 1. Purpose

Devices shows every piece of hardware in and attached to the machine, which driver runs it and how that driver is confined, what firmware it has, and what policy applies to it. It is where the user decides which USB, Bluetooth and Thunderbolt devices may be used at all. It replaces device managers, `pciconf`/`usbconfig`/`devinfo` reading, Bluetooth and Thunderbolt settings panels, and firmware update tools.

On PBSD a driver is not trusted code with the run of the kernel: it works inside an MMIO, IRQ and DMA grant (SI-7; plan P3-02 to P3-04). This app makes that grant visible per device.

## 2. Scope

### In scope

- Device tree (by connection, by type, by driver, by IOMMU group) and device details.
- Drivers: origin (Linux driver through the driver host, HardenedBSD fallback, UDA descriptor with a generic engine), confinement path, state, restart, bind and unbind, parameters.
- UDA descriptor viewer.
- Firmware: versions present, firmware files that drivers need (the `fwget` logic), updates from signed local sources.
- USB device policy: allow-list, BadUSB protection, lock-screen rules, per-app `dev.usb` grants.
- Bluetooth: adapters, pairing, per-device profiles and policy.
- Thunderbolt and USB4 authorisation; DMA protection state.
- IOMMU groups and reservation of devices for VM passthrough.
- Sensors and per-device power management.
- Hardware report export to a file. No upload.

### Out of scope

| Item | Owner |
|---|---|
| Display arrangement, sound devices, input settings, network connections, printers | SY01, UT06 (VM05 links to them from the device) |
| Disks, partitions, pools, SMART tests | AD02 (VM05 shows the controller and a health summary) |
| Assigning a reserved PCI device to a VM | VM01 |
| System power profile, sleep, battery settings | SY01 |
| Live monitoring dashboards and history | SY04 (VM05 shows current sensor values and short graphs) |
| Installing packages (firmware packages are installed through it) | SY05 |
| Grants of all apps and the lineage tree | AD05 |
| Intrusion alerts raised from device events | SE01 |
| Verified and measured boot state | SE01 |

## 3. Users and top tasks

Personas:

- **Robin, everyday user.** Plugs in a USB stick, pairs headphones, wants a clear yes or no and no jargon.
- **Sam, administrator.** Sets the USB policy, checks drivers after an update, exports a hardware report for a support request.
- **Mika, VM user.** Prepares a GPU for passthrough and needs the IOMMU groups.
- **Noor, security-minded user.** Wants unknown USB devices blocked while the screen is locked and Thunderbolt DMA protection confirmed.
- **Lee, driver developer.** Reads UDA descriptors, restarts a sandboxed driver, reads its gateway faults.

| # | Task | Target |
|---|---|---|
| T1 | Find a device and see whether it works | 2 steps: `Ctrl+F`, type; state shown in the row |
| T2 | Allow a newly plugged USB device | 1 step: "Allow" in the shell notification (or 2 here) |
| T3 | Block all new USB devices while locked | 1 step: switch on the USB policy page (default on) |
| T4 | Pair a Bluetooth device | 3 steps: "Add device", pick, confirm the number |
| T5 | See which driver runs a device and how it is confined | 1 step: select the device, Driver tab |
| T6 | Restart a misbehaving driver | 2 steps: Driver tab, "Restart driver" |
| T7 | Find and install missing firmware | 2 steps: Firmware page, "Install 2 packages" (hands to SY05) |
| T8 | Reserve a PCI device for a VM | 4 steps: IOMMU view, select group, "Reserve for passthrough", approve |
| T9 | Stop a device from waking the computer | 2 steps: Power tab, untick "May wake" |
| T10 | Export a hardware report without serial numbers | 3 steps: `Ctrl+E`, tick "Redact identifiers", save |
| T11 | Revoke an app's access to a USB device | 2 steps: device, Access tab, "Revoke" |
| T12 | Authorise a Thunderbolt dock | 2 steps: notification "Authorise", choose once or always |
| T13 | Read a UDA descriptor | 2 steps: Driver tab, "Show descriptor" |

## 4. Features

Status of the backend (Verified):

- `docs/specs/UDA_DESCRIPTOR.md` defines a descriptor (`name`, `provenance`, `device_class`, `vendor_id`, `device_id`, `init_sequence`, `reset_sequence`), a bytecode (`Write8/16/32`, `Read8/32`, `WaitUs`, `CheckEq`, `Done`), `MmioCapability` and `IrqCapability`, and states that out-of-bounds access returns `Status::Denied` (SI-7).
- `pbsd/uda/pbsd.uda.schema.cppm` implements those types; `DeviceClass` has five values (Block, Network, Display, Input, Sensor). `pbsd/uda/engines/` holds eight engine modules (block, crypto, display, input, net, pci, sensor, usb). `pbsd/uda/descriptors/` holds 197 `.cppm` files (for example `ahci`, `nvme`, `xhci`, `virtio_blk`, `acpi_battery`, `acpi_thermal`, `coretemp`, `usbhid`). `pbsd/uda/pbsd.uda.newbus.cppm` gives typed probe, attach and detach.
- The plan (`docs/plan/PBSD_PLAN.md`) lists P3-01 device model and PCI/ACPI enumeration, P3-02 IOMMU with per-device domains and strict mode by default, P3-03 Linux driver host fast path, P3-04 sandboxed restartable path (`pbsd/kernel/drvsandbox/`, `pbsd/sbin/driverhost/`), P3-06 driver sets with HardenedBSD drivers as fallback, P4-07 checked gateways. None of those directories exists yet.
- `hbsd/src/usr.sbin/fwget/fwget.sh` maps PCI and USB ids to firmware package names (tables under `pci/` and `usb/`), has a dry-run flag `-n`, and otherwise runs `pkg install -qy`.
- `hbsd/src/usr.sbin/` has `pciconf`, `usbconfig`, `usbdump`, `devctl`, `devinfo`, `iovctl` and a `bluetooth/` tool set.

All features below are Proposed.

### 4.1 Device tree and navigation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM05-F001 | Device tree | All devices from `svc.device`: name, class icon, state (working, no driver, driver error, blocked by policy, disabled, reserved for VM, sleeping), driver, confinement badge | M | 1 |
| VM05-F002 | Views | By connection (bus topology), by type (class), by driver, by IOMMU group, by power state | M | 1 |
| VM05-F003 | Sidebar pages | Overview, All devices, USB, Bluetooth, Thunderbolt, Firmware, Drivers, IOMMU, Sensors, Power, Policy, Events | M | 1 |
| VM05-F004 | Overview | Cards: problems (devices without a driver, blocked devices, failed drivers), new since last boot, policy state, firmware needed, DMA protection | M | 1 |
| VM05-F005 | Search | By name, vendor, id (`8086:15f3`), driver, serial, address (`0b:00.0`), class | M | 1 |
| VM05-F006 | Filters | Problems only, external only, removable, with firmware, sandboxed drivers, fast-path drivers, hidden devices | S | 1 |
| VM05-F007 | Live updates | Hot-plug adds and removes rows at once; a removed device stays greyed for 10 s | M | 1 |
| VM05-F008 | Events page | Time-ordered list: attach, detach, driver bound, driver restarted, policy decision, firmware loaded, authorisation; filter by device | M | 2 |
| VM05-F009 | Human names | Vendor and device names from local id databases (PCI, USB, PNP, OUI) shipped as signed data; raw ids always available | M | 1 |
| VM05-F010 | Nicknames | The user can name a device ("Desk dock"); the name is keyed by the device identity | S | 2 |
| VM05-F011 | Hide noise | Bridges, hubs and virtual devices are collapsed by default; "Show all" expands | S | 1 |
| VM05-F012 | Physical location | Port label where firmware provides it (ACPI `_PLD`, USB port path) and a "which port?" highlight for the last plugged device | S | 2 |
| VM05-F013 | Compare with last boot | List devices added, removed or changed (firmware version, driver) since the previous boot or a saved report | S | 3 |
| VM05-F014 | System search | Device names contributed to `svc.search` | C | 3 |

### 4.2 Device details

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM05-F015 | Summary tab | Name, class, vendor, ids, revision, serial (hidden until revealed), bus address, parent, state in words, first seen, last attached | M | 1 |
| VM05-F016 | Resources tab | MMIO windows (BAR, size, the granted sub-window), IRQ or MSI/MSI-X vectors, I/O ports, DMA domain, bus mastering; shown as the device's grant | M | 2 |
| VM05-F017 | PCI details | Configuration header fields, capabilities list (PM, MSI, MSI-X, PCIe link speed and width, AER, ACS, SR-IOV, ATS), decoded; raw hex view of configuration space | M | 2 |
| VM05-F018 | USB details | Device, configuration, interface and endpoint descriptors decoded; speed; hub path; power draw; string descriptors as plain text; descriptor hash | M | 1 |
| VM05-F019 | ACPI and platform devices | Hardware id, compatible ids, status, resources; device-tree node path and properties on platforms that use a device tree | S | 2 |
| VM05-F020 | Storage controllers | Controller, attached drives with model and a health word; "Open in Disks" (AD02) | S | 2 |
| VM05-F021 | Network adapters | MAC (hidden until revealed), link state, speed, driver; "Open network settings" (SY01) | S | 2 |
| VM05-F022 | Displays and GPUs | GPU model, memory, driver, connected outputs with EDID summary (maker, model, sizes; serial hidden) | S | 2 |
| VM05-F023 | Input devices | Kind, report descriptor summary for HID (usage pages), which seat it belongs to | S | 2 |
| VM05-F024 | Cameras and microphones | Which apps hold a grant; a hardware kill-switch state when the platform reports one | S | 2 |
| VM05-F025 | Access tab | Apps holding a handle to the device or a derived handle (`dev.usb:<class>`, camera, microphone, GPU compute), since when; Revoke | M | 2 |
| VM05-F026 | Enable and disable | Disable a device (driver detached, device left unbound and powered down where possible); persistent across boots; needs administrator approval for built-in devices | M | 2 |
| VM05-F027 | Copy and export | Copy any field; copy the whole device as text or JSON | S | 1 |
| VM05-F028 | Open related app | Buttons to the app that owns settings for the device class (SY01, AD02, UT06, VM01) | S | 1 |

### 4.3 Drivers

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM05-F029 | Driver tab | Driver name, version, origin, licence, signature state, path, state, bound since, restart count | M | 1 |
| VM05-F030 | Origin | One of: Linux driver through the driver host (plan P3-03, P3-04), HardenedBSD driver (fallback, plan P3-06), UDA descriptor with a generic engine, built-in | M | 1 |
| VM05-F031 | Confinement path | Fast path (in kernel behind IOMMU and typed gateways) or sandboxed path (separate domain, restartable); badge and explanation in plain words | M | 1 |
| VM05-F032 | Grant view | What the driver may touch: MMIO windows, IRQs, DMA windows, parent bus operations; nothing else (SI-7 for descriptors, P3-02 for DMA) | M | 2 |
| VM05-F033 | Restart driver | Restart a sandboxed driver (plan target: recovery under 100 ms); for fast-path drivers, unbind and rebind with a warning | M | 2 |
| VM05-F034 | Move between paths | Run a driver on the sandboxed path instead of the fast path (slower, safer) where both exist; persistent per device; administrator approval | S | 3 |
| VM05-F035 | Automatic fallback notice | When a fast-path driver crashed and the system fell back to the sandboxed path (plan section 6a), the device shows it with the time and reason | M | 2 |
| VM05-F036 | Gateway faults | Count and last entries of contract violations caught at the gateway for this driver (plan P4-07) | S | 3 |
| VM05-F037 | Bind and unbind | Choose among candidate drivers for a device; unbind; the choice can be made persistent | S | 2 |
| VM05-F038 | Driver parameters | Tunables the driver declares, with type, range, current and default value; changes validated; marked "applies at next bind" where needed | S | 3 |
| VM05-F039 | Deny list | Never bind a given driver, or never bind any driver to a given device id | S | 2 |
| VM05-F040 | Drivers page | Table of all loaded drivers and modules: devices bound, origin, path, size, signature, restarts, faults | M | 2 |
| VM05-F041 | Signature and source | Module signature verdict and signer; source package name and version; licence (for example GPL-2.0 for Linux-derived modules, as noted in the plan's risk table) | M | 2 |
| VM05-F042 | Missing driver help | For a device with no driver: ids, class, whether a driver exists in an installable package (local package index through SY05), and whether firmware is the missing part | M | 2 |
| VM05-F043 | Driver log | Messages from the driver and the driver host for this device; follow mode; "Open in Logs" (AD04) | M | 2 |
| VM05-F044 | Driver update history | Which driver version served the device per boot, from the events store | C | 3 |
| VM05-F045 | Foreign NT drivers | Windows kernel drivers run in the IOMMU sandbox (plan P4-04) are listed with origin "NT driver" and the same grant view | C | 3 |
| VM05-F046 | Test a device | Per-class self tests where safe: keyboard key test, pointer test, speaker tone, camera preview through `dev.camera`, link check | C | 3 |

### 4.4 UDA descriptors

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM05-F047 | Descriptor view | For a device run by a UDA engine: `name`, `provenance`, `device_class`, `vendor_id`, `device_id`, engine | M | 2 |
| VM05-F048 | Sequence listing | `init_sequence` and `reset_sequence` shown as a table: step, op (`Write32`, `CheckEq`, `WaitUs`, …), offset, value, mask, timeout | M | 2 |
| VM05-F049 | Grant bounds | The `MmioCapability` length and the `IrqCapability` line granted to the engine; every listed offset is marked inside or outside the grant | M | 2 |
| VM05-F050 | Denials | Count of `Status::Denied` results from the interpreter for this device, with the last offending offset | S | 3 |
| VM05-F051 | Descriptor catalogue | Browse all descriptors installed on the system by class and id, whether or not hardware is present | S | 3 |
| VM05-F052 | Provenance link | The provenance citation is shown as text; if it names a document in the help set, a link opens UT05 | C | 3 |
| VM05-F053 | Export descriptor | Save the decoded descriptor as JSON or text | C | 3 |
| VM05-F054 | Read-only | Descriptors are immutable at run time (per the specification); the app never edits them | M | 2 |

### 4.5 Firmware

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM05-F055 | Firmware inventory | Table: device, firmware kind (system UEFI, embedded controller, CPU microcode, storage, network, GPU, dock, peripheral), version, date, updatable or not, source | M | 2 |
| VM05-F056 | Loadable firmware needed | Compute which firmware packages the present hardware needs, using the id-to-package tables of `fwget` (dry-run logic only) | M | 1 |
| VM05-F057 | Install through Software | Missing firmware packages are handed to SY05 as an install request; SY05 verifies signatures and uses configured repositories or local media; this app never runs a package tool itself | M | 1 |
| VM05-F058 | Offline note | With no repository reachable the page says which package files to bring on removable media | M | 2 |
| VM05-F059 | Loaded firmware files | Per device: which firmware file the driver loaded, its version string and hash, and whether it matches the signed package manifest | M | 2 |
| VM05-F060 | Licence display | Firmware packages show their licence and "non-free binary" label; the user can refuse classes of firmware (policy) | S | 2 |
| VM05-F061 | Flash updates from files | Apply vendor update files the user supplies (UEFI capsules, device update archives) after signature check; staged for next boot where the platform requires | S | 3 |
| VM05-F062 | Update pre-flight | Checks AC power or battery above 50%, no pending sleep, version is newer (downgrade needs a second confirmation), device supports rollback or not | S | 3 |
| VM05-F063 | Update result | After reboot: success or failure per device, old and new version; recorded in events and audit | S | 3 |
| VM05-F064 | Microcode | CPU microcode revision per core and whether the loaded revision came from firmware or from the OS package | S | 2 |
| VM05-F065 | Security-relevant settings | Read-only list of firmware settings the platform reports: Secure Boot, TPM presence and version, IOMMU enabled, kernel DMA protection flag, Thunderbolt security level; link to SE01 | S | 2 |
| VM05-F066 | Firmware history | Version changes over time per device | C | 3 |
| VM05-F067 | No vendor service | No online firmware catalogue is contacted; metadata comes from signed local packages or user files | M | 2 |
| VM05-F068 | Block firmware load | Per device: refuse to load firmware (device then stays without driver); for people who exclude binary blobs | C | 3 |

### 4.6 USB policy

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM05-F069 | Decision before binding | Every USB device is evaluated by `svc.device` before any class driver binds; outcomes: allow, allow limited (some interfaces), ask, block | M | 1 |
| VM05-F070 | Default policy | Ask for unknown devices when unlocked; block new devices when locked; allow devices present at boot on internal ports; internal ports are marked from firmware data | M | 1 |
| VM05-F071 | Allow-list | Rules by vendor:product, serial, descriptor hash, interface classes, port path; action; scope (always, this session, this port only) | M | 1 |
| VM05-F072 | Identity binding | A remembered device must match vendor, product, serial and descriptor hash; a device that claims a known identity but shows different interfaces is treated as unknown and flagged | M | 2 |
| VM05-F073 | New-device prompt | Shell notification: device name, what it claims to be (keyboard, storage, network adapter, …), port; actions Allow once, Always allow, Block | M | 1 |
| VM05-F074 | Per-interface authorisation | For composite devices, allow some interfaces only (for example storage yes, HID no) | M | 2 |
| VM05-F075 | Keyboard challenge | A new keyboard is not trusted for typing into the session until the user confirms with an already trusted input device (click) or, if none exists, types a shown 4-digit code on the new keyboard | M | 2 |
| VM05-F076 | Keystroke-injection heuristics | A HID device that sends keys within 500 ms of attach, types faster than 50 keys per second, or appears on a device that was storage a moment ago is suspended and reported | M | 2 |
| VM05-F077 | Hidden-function check | Flag devices whose interface set does not fit their shape: a "storage stick" with HID or network interfaces, a charger cable with data functions | M | 2 |
| VM05-F078 | Network adapters by USB | New USB network adapters are never made the default route or DNS source without consent | M | 2 |
| VM05-F079 | Storage rules | New storage: allow read-only, allow read-write, or block; never auto-run anything; mounting itself is SY02/AD02 | M | 2 |
| VM05-F080 | Charge-only | Per port or per device: power but no data (device left unconfigured) | S | 2 |
| VM05-F081 | Lock-screen rule | While the session is locked: no new device is authorised; already authorised devices keep working; option to also suspend external HID | M | 1 |
| VM05-F082 | Per-app grants | List of apps with `dev.usb:<class>` grants and the devices each can open; revoke per app and device | M | 2 |
| VM05-F083 | Class grant definitions | Shows what each `dev.usb:<class>` covers (hid, storage, audio, video, ccid, printer, serial, vendor) so prompts are understandable | S | 2 |
| VM05-F084 | Port control | Disable a physical port; power-cycle a port to reset a device | S | 3 |
| VM05-F085 | USB traffic capture | Hand a capture handle for one device to DV05 (the base system has `usbdump`); administrator approval | C | 3 |
| VM05-F086 | Policy export and import | Rules as a signed TOML file for use on several machines | S | 3 |
| VM05-F087 | Policy simulation | "What would happen if this device were plugged in now?" for a chosen past device | C | 3 |
| VM05-F088 | Blocked list | Devices blocked recently with the reason and "Allow now" | M | 1 |

### 4.7 Bluetooth

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM05-F089 | Adapters | List adapters with address (hidden until revealed), name, version, powered state; power on and off; choose the default | M | 2 |
| VM05-F090 | Off by default | The radio is off until the user turns it on; state shown in the shell indicator (SH02) | M | 2 |
| VM05-F091 | Discoverable | Off by default; when turned on it lasts 2 minutes (setting 1–10) and shows a countdown | M | 2 |
| VM05-F092 | Scan | "Add device" scans for 30 s; list shows name, kind, signal strength, classic or low energy; names are untrusted text | M | 2 |
| VM05-F093 | Pairing methods | Numeric comparison, passkey entry, out-of-band where supported; legacy PIN pairing only after a warning; "Just Works" pairing refused for input devices by default | M | 2 |
| VM05-F094 | Paired devices | Name, kind, connected state, battery level if reported, last connected, trust level, profiles | M | 2 |
| VM05-F095 | Per-device profiles | Switch each service on or off: input (HID), audio output, headset microphone, file transfer (off by default), network tethering (off), serial port (off), MIDI | M | 2 |
| VM05-F096 | Connect policy | Per device: connect automatically, ask, never; refuse incoming connections from unpaired devices | M | 2 |
| VM05-F097 | Security requirements | Policy: require Secure Connections and authenticated pairing; minimum encryption key size 16 bytes; devices that cannot meet it are refused with the reason | M | 2 |
| VM05-F098 | Keyboard rule | A Bluetooth keyboard is subject to the same new-keyboard confirmation as USB | M | 2 |
| VM05-F099 | Address privacy | Use resolvable private addresses for low energy; option to randomise the adapter name | S | 3 |
| VM05-F100 | Forget and block | Forget a device (keys erased from `svc.keyring`); block an address | M | 2 |
| VM05-F101 | Lock-screen rule | No new pairing while locked; option to disconnect chosen devices on lock | S | 2 |
| VM05-F102 | Key storage | Link keys and long-term keys live in `svc.keyring`, never in plain files | M | 2 |
| VM05-F103 | File receive | When file transfer is on, incoming files need a per-transfer consent and land in a quarantine folder | S | 3 |
| VM05-F104 | Adapter firmware | Shows whether the adapter needs loadable firmware and whether it is present (links to the Firmware page) | S | 2 |

### 4.8 Thunderbolt, USB4 and DMA protection

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM05-F105 | Domain and security level | Shows each controller's security level as the firmware reports it (none, user, secure, display-only, USB-only) and whether pre-boot protection is on | M | 2 |
| VM05-F106 | Device authorisation | A new Thunderbolt or USB4 device gets no PCIe tunnel until authorised; prompt names vendor, device and the chain it hangs on | M | 2 |
| VM05-F107 | Secure connect | Where the controller supports key verification, enrol a key at first authorisation and challenge on later connections | S | 3 |
| VM05-F108 | Remembered devices | List with authorise once, always, or forget | M | 2 |
| VM05-F109 | DMA protection state | One line: IOMMU on, strict mode, external-facing ports isolated per device; green only when all hold; otherwise the missing part is named | M | 2 |
| VM05-F110 | External ports are untrusted | Devices behind external-facing ports always get their own IOMMU domain and never the fast path without consent | M | 2 |
| VM05-F111 | Lock-screen rule | No authorisation while locked; option to de-authorise docks on lock | M | 2 |
| VM05-F112 | Limited modes | Per device: allow only DisplayPort and USB tunnels, no PCIe | S | 3 |
| VM05-F113 | Other DMA-capable buses | The same rule set covers ExpressCard, FireWire and PCIe hot-plug slots where present; FireWire physical DMA is off | S | 3 |
| VM05-F114 | Boot access list | View and edit the list of devices authorised before the OS starts, where firmware exposes it | C | 3 |

### 4.9 IOMMU groups

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM05-F115 | Status | IOMMU kind (Intel VT-d, AMD-Vi, or an ARM SMMU), enabled, strict mode, interrupt remapping, per-device domains (plan P3-02) | M | 2 |
| VM05-F116 | Group table | Group number, devices in it, class, driver, isolation quality (alone in group, shares with a bridge, shares with other functions) | M | 2 |
| VM05-F117 | Why grouped | Explanation per group: missing access control services on a bridge, multi-function device, conventional PCI bridge | S | 3 |
| VM05-F118 | Reserve for passthrough | Mark every device of a group as reserved: the host driver is detached (now or at next boot) and a stub holds the device; administrator approval; VM01 can then assign it | M | 2 |
| VM05-F119 | Safety checks | Refuse or warn when the group contains the boot disk controller, the only GPU, or the only input controller | M | 2 |
| VM05-F120 | Release | Return a reserved group to the host and rebind its drivers | M | 2 |
| VM05-F121 | Domain view | For each device: its DMA domain id and mapped window count; devices sharing a domain are listed together | S | 3 |
| VM05-F122 | SR-IOV | Create and remove virtual functions on capable devices (count, MAC per function); each function appears as its own device and group | C | 3 |
| VM05-F123 | Fault log | DMA faults reported by the IOMMU per device (address, kind, time); a device with repeated faults is flagged to SE01 | S | 3 |

### 4.10 Sensors

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM05-F124 | Sensor list | Temperatures (CPU cores, GPU, drives, board zones), fan speeds, voltages, currents, power, battery, ambient light, lid and tablet-mode switches; grouped by device | M | 2 |
| VM05-F125 | Current values and limits | Value, unit, minimum and maximum seen this session, warning and critical thresholds from firmware | M | 2 |
| VM05-F126 | Short graphs | Last 10 minutes per sensor (`chart`); longer history is in SY04 | S | 2 |
| VM05-F127 | Battery health | Design capacity, full-charge capacity, cycle count, chemistry, manufacturer, wear percentage, current rate | M | 2 |
| VM05-F128 | Alerts | Notify when a sensor crosses its warning threshold or a fan stops; rate limited | S | 3 |
| VM05-F129 | Units | Celsius or Fahrenheit, from locale with override | S | 2 |
| VM05-F130 | Fan control view | Shows the firmware's fan mode; manual fan curves are not offered in phase 1–2 | C | 3 |
| VM05-F131 | Sensor access | Motion and light sensors are not readable by apps without a grant; the list shows which apps hold one | S | 3 |
| VM05-F132 | Copy and export | Copy a snapshot of all sensors as text or CSV | C | 2 |

### 4.11 Power management per device

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM05-F133 | Power tab | Current device power state (D0–D3), runtime suspend on or off, time suspended, link power state (PCIe ASPM, USB link states, SATA link power) | M | 2 |
| VM05-F134 | Runtime suspend switch | Per device: allow automatic suspend when idle; delay | M | 2 |
| VM05-F135 | Wake sources | Which devices may wake the computer from sleep; switch per device; defaults: keyboard and power button yes, network and pointer no | M | 2 |
| VM05-F136 | Last wake reason | What woke the machine last time, with time | S | 2 |
| VM05-F137 | Power page | Table of all devices with state, estimated draw where known, and "keeps the system awake" markers | S | 3 |
| VM05-F138 | USB power | Port power budget and draw; per-port power off | S | 3 |
| VM05-F139 | Blockers | Devices or drivers that refused the last suspend, with the reason | S | 3 |
| VM05-F140 | Profiles respected | Shows how the system power profile (SY01) maps to device settings; per-device overrides are marked | C | 3 |
| VM05-F141 | Radio switches | State of hardware and software radio kill switches (Wi-Fi, Bluetooth, mobile broadband) | S | 2 |

### 4.12 Hardware report

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM05-F142 | Export | Full report to a file through the file portal: system, firmware, device tree with drivers and paths, IOMMU groups, sensors snapshot, policies summary | M | 2 |
| VM05-F143 | Formats | JSON (schema documented), Markdown, PDF, plain text | M | 2 |
| VM05-F144 | Redaction | "Redact identifiers" replaces serial numbers, MAC and Bluetooth addresses, UUIDs, asset tags, host name and user name with stable placeholders; on by default | M | 2 |
| VM05-F145 | Preview | The exact content is shown before saving; sections can be unticked | M | 2 |
| VM05-F146 | No upload | The report goes only to the file the user chose; the app has no network capability | M | 2 |
| VM05-F147 | Compare reports | Diff two reports (or a report and the live system): devices, drivers, firmware versions | S | 3 |
| VM05-F148 | Signed report | Optional signature with a key from SE04 so a recipient can check it was not edited | C | 3 |
| VM05-F149 | Print | Print the report through the print portal | C | 3 |

### 4.13 Automation, settings, accessibility, recovery

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM05-F150 | Command-line tool | `dev list`, `dev show <id>`, `dev usb allow|block`, `dev driver restart`, `dev report` over the same service; same approvals | S | 2 |
| VM05-F151 | Command palette | Every page, device and action in `Ctrl+K` | M | 1 |
| VM05-F152 | Undo | Policy edits, nicknames, wake and suspend switches are undoable; disable and reserve actions show a confirmation instead | M | 2 |
| VM05-F153 | Safe mode | If a policy would block the only keyboard and pointer, the app refuses and explains; a boot-time recovery switch resets USB policy to "allow devices present at boot" | M | 2 |
| VM05-F154 | Service loss | If `svc.device` restarts, devices keep working; the app reconnects and reloads the tree | M | 1 |
| VM05-F155 | Policy history | Last 50 versions of each policy with diff and restore | S | 3 |
| VM05-F156 | Local explanations | With `ai.local`: explain a driver error message or a descriptor in plain words; off by default; local only | C | 3 |
| VM05-F157 | Help | Help per page; glossary for bus terms | S | 2 |

## 5. GUI

### 5.1 Window map

```
Devices
├─ Main window
│  ├─ Sidebar: Overview · All devices · USB · Bluetooth · Thunderbolt · Firmware · Drivers ·
│  │           IOMMU · Sensors · Power · Policy · Events
│  ├─ Content: page for the chosen section (tree, table or cards)
│  ├─ Inspector (F4): device details
│  │  └─ Tabs: Summary · Driver · Resources · Firmware · Power · Access
│  ├─ UDA descriptor view (page)
│  ├─ USB rule editor (dialog) · Blocked devices popover · Policy simulation dialog
│  ├─ Bluetooth: Add device sheet · Pairing dialog · Device profiles popover
│  ├─ Thunderbolt authorisation dialog
│  ├─ IOMMU: Reserve for passthrough dialog · SR-IOV dialog
│  ├─ Firmware: Install plan sheet (hand-off to SY05) · Flash update wizard
│  ├─ Driver: parameters dialog · choose driver dialog · driver log view
│  ├─ Hardware report sheet (sections, redaction, preview)
│  ├─ Compare reports view
│  └─ Preferences (Ctrl+,)
└─ Shell-drawn (SH02/SH04): new USB device prompt, keyboard challenge, Thunderbolt prompt,
   Bluetooth pairing confirmation, administrator approval
```

### 5.2 Main window

Shown at 1180×720, All devices, view "by connection", one device selected.

```
+----------------------------------------------------------------------------------------+
| [=] Devices                          [ Search devices  Ctrl+F ]          [▁] [▢] [✕]  |
+------------+-------------------------------------------------+-------------------------+
|  Overview  | View [by connection v] [Problems only] [filter v]| Intel AX211 Wi-Fi   (4) |
|> All    87 +-------------------------------------------------+ ● Working               |
|  USB    11 | ▾ PCI root                                   (1)| [Summary][Driver]       |
|  Bluetooth | ▾ 00:14.0 USB controller    xhci    ▣ fast      | [Resources][Firmware]   |
|  Thunderb. |   ▾ Hub (internal)                               | [Power][Access]         |
|  Firmware 2|       Keyboard (built-in)   usbhid  ▢ sandbox    | ----------------------- |
|  Drivers   |       Webcam                uvc     ▢ sandbox    | Driver  iwlwifi 6.12    |
|  IOMMU     |     ⚠ USB stick "KINGSTON"  –       blocked  (2) | Origin  Linux driver    |
|  Sensors   |   00:02.0 Graphics          i915    ▣ fast       | Path    ▢ sandboxed (5) |
|  Power     | > 00:14.3 Wi-Fi AX211       iwlwifi ▢ sandbox    | Signed  ✓ pbsd-drivers  |
|  Policy    |   01:00.0 NVMe SSD          nvme    ▣ fast       | Restarts 1 (10 Nov)     |
|  Events    |   ✕ 03:00.0 Card reader     –       no driver    | May touch:              |
|            |   ▾ Thunderbolt port 1                           |  MMIO 16 KiB, 1 MSI-X,  |
|            |       (nothing attached)                         |  own DMA domain         |
|            |                                                  | [Restart driver]        |
|            |                                                  | [Driver log] [More v]   |
+------------+-------------------------------------------------+-------------------------+
| 87 devices · 1 without driver · 1 blocked · DMA protection ✓ · USB policy: ask (3)      |
+----------------------------------------------------------------------------------------+
```

1. Device tree (`treeView` with columns): name, driver, confinement badge (▣ fast path, ▢ sandboxed), state word.
2. Problem rows carry an icon and a word; a blocked device offers "Allow…" in its context menu.
3. Status bar: counts, DMA protection state, USB policy mode. Each part is a link to its page.
4. Inspector with six tabs.
5. Confinement path with a plain explanation in the tooltip.

States: empty (cannot be empty on real hardware; if the service returns nothing, the error state shows), loading (skeleton tree), error (service unreachable: banner, Retry, "Open Logs"), read-only (policy lock or no administrator right: actions disabled with the reason, everything still visible), narrow (sidebar overlay; tree shows name and state; inspector sheet).

### 5.3 Other views and dialogs

#### USB policy page

```
+----------------------------------------------------------------------------------------+
| USB policy                                                                              |
| New devices while unlocked [ Ask v]     while locked [ Block v]                         |
| [x] Confirm new keyboards   [x] Suspend devices that type by themselves                 |
| [x] Flag devices with hidden functions   [ ] Suspend external keyboards on lock         |
+----------------------------------------------------------------------------------------+
| Rules                                                        [+ Rule] [Import] [Export] |
|  # Match                              Interfaces      Port     Action       Used        |
|  1 046d:c52b serial 7A…  "Desk mouse" HID             any      allow        today       |
|  2 0951:1666 hash 91c…   "Backup key" storage         any      allow, r/o   3 Nov       |
|  3 class: storage                     storage         any      ask          –           |
|  4 class: network                     cdc, rndis      any      block        –           |
|  – everything else                                             ask                      |
+----------------------------------------------------------------------------------------+
| Blocked recently: "KINGSTON" 0951:1666 (new serial) 14:02  [Allow now] [Details]        |
| App grants: Virtual Machines → YubiKey (ccid)  [Revoke] · Camera → Webcam (video)       |
+----------------------------------------------------------------------------------------+
```

`tableView` with drag reorder; first match wins.

#### New USB device prompt (drawn by the shell, 420×220)

```
+--------------------------------------------------+
| A USB device was plugged in                       |
| "KINGSTON DataTraveler" · left port               |
| It says it is: storage                            |
| Never seen before.                                |
| [Block]   [Allow once]   [Always allow ▾]         |
+--------------------------------------------------+
```

The variant for a hidden-function device has a `--danger` line: "It also says it is a keyboard. That is unusual for this kind of device." and no "Always allow".

#### Bluetooth page with pairing dialog

```
+----------------------------------------------------------------------------------------+
| Bluetooth  [ on |● ]   adapter "Intel AX211" · not discoverable   [Make discoverable 2m]|
+----------------------------------------------------------------------------------------+
| Paired                                                              [+ Add device]      |
|  WH-1000  headphones  ● connected  battery 80 %   audio [x] mic [ ] files [ ]           |
|  MX Keys  keyboard    ○ away       trusted input  input [x]                             |
| Nearby (scanning 21 s)                                                                  |
|  "Pixel 9"  phone  ▂▄▆    "TV-living"  display  ▂▄                                      |
+----------------------------------------------------------------------------------------+
   +----------------------------------------------+
   | Pair with "Pixel 9"?                          |
   | Check that both show this number:             |
   |              4 8 2 9 1 7                      |
   | Method: numeric comparison · secure           |
   | [Cancel]                  [They match — pair] |
   +----------------------------------------------+
```

#### IOMMU page and reserve dialog

```
+----------------------------------------------------------------------------------------+
| IOMMU: AMD-Vi · on · strict · interrupt remapping ✓ · per-device domains ✓              |
| Group Address  Device                  Driver   Isolation          Reserved             |
|   7   04:00.0  NVMe controller         nvme     alone              –    (boot pool)     |
|  12   0b:00.0  GPU                     drm      shares (function)  –                    |
|  12   0b:00.1  HDMI audio              snd_hda  shares (function)  –                    |
|  14   0d:00.3  USB 3 controller        xhci     alone              ✓ for VMs            |
+----------------------------------------------------------------------------------------+
| Selected group 12 (2 devices)   [Reserve for passthrough…]  [Why are these grouped?]    |
+----------------------------------------------------------------------------------------+
   +------------------------------------------------------------+
   | Reserve group 12 for virtual machines?                      |
   | GPU 0b:00.0 and HDMI audio 0b:00.1 will leave the host.     |
   | ⚠ This is the only graphics device with a display attached. |
   | (•) At next boot   ( ) Now (drivers detach at once)         |
   | Needs administrator approval.                               |
   | [Cancel]                                [Reserve…]          |
   +------------------------------------------------------------+
```

#### Firmware page

```
+----------------------------------------------------------------------------------------+
| Firmware                                                                                |
| Needed by your hardware                                    [Install 2 packages…]  (1)   |
|  ⚠ Wi-Fi AX211           wifi-firmware-iwlwifi-kmod-…   not installed   non-free binary |
|  ⚠ GPU (amd)             gpu-firmware-amd-kmod-…        not installed   non-free binary |
| Installed                                                                               |
|  System UEFI             1.24.0   2 Sep 2026   updatable from file  [Update from file…] |
|  CPU microcode           0x12B    from OS package ✓                                     |
|  NVMe SSD                4B2QGXA7 not updatable here                                    |
|  Dock "Desk dock"        1.0.14   updatable from file                                   |
| Platform settings: Secure Boot on · TPM 2.0 · IOMMU on · Thunderbolt level: user        |
+----------------------------------------------------------------------------------------+
```

(1) Opens the install plan sheet, which lists the packages and hands the request to SY05. Package names shown come from the `fwget` tables.

#### UDA descriptor view

```
+----------------------------------------------------------------------------------------+
| ← virtio-blk-1.0                              engine: block   class: Block              |
| Provenance  hbsd/src/sys/dev/virtio/block/virtio_blk.h                                  |
| Ids         vendor 0x1af4  device (from descriptor)                                     |
| Grant       MMIO length 0x200 · IRQ line 11                                             |
+----------------------------------------------------------------------------------------+
| init_sequence                                                                           |
|  # Op       Offset  Value       Mask        Timeout  In grant                           |
|  0 Write32  0x070   0x00000001  –           –        ✓                                  |
|  1 Write32  0x070   0x00000003  –           –        ✓                                  |
|  2 Write32  0x070   0x0000000B  –           –        ✓                                  |
|  3 CheckEq  0x070   0x00000008  0x00000008  –        ✓                                  |
|  4 Write32  0x070   0x0000000F  –           –        ✓                                  |
|  5 Done                                                                                 |
| reset_sequence  (2 steps)                                           Denials so far: 0   |
+----------------------------------------------------------------------------------------+
```

The example rows mirror the shape of `pbsd/uda/descriptors/virtio_blk.cppm`; offsets and grant values in the drawing are illustrative.

#### Hardware report sheet

```
+------------------------------------------------------------------------+
| Export hardware report                                                  |
| Sections [x] System [x] Firmware [x] Devices and drivers [x] IOMMU      |
|          [x] Sensors snapshot [ ] Policies [ ] Events (last 7 days)     |
| [x] Redact identifiers (serials, MAC and Bluetooth addresses, names)    |
| Format   (•) JSON  ( ) Markdown  ( ) PDF  ( ) Text     [ ] Sign         |
| Preview ------------------------------------------------------------   |
| | system: { vendor: "…", model: "…", serial: "<serial-1>" }        |   |
| | devices[12]: { id: "8086:51f0", driver: "iwlwifi", path: …       |   |
| This file is saved where you choose. Nothing is sent anywhere.          |
| [Cancel]                                              [Save report…]    |
+------------------------------------------------------------------------+
```

### 5.4 Menus

```
File      Export hardware report… Ctrl+E · Compare reports… · Import policy… ·
          Export policy… · Print… Ctrl+P · Preferences Ctrl+, · Close Ctrl+W · Quit Ctrl+Q
Edit      Undo Ctrl+Z · Redo Ctrl+Shift+Z · Copy Ctrl+C · Copy as JSON Ctrl+Shift+C ·
          Rename (nickname) F2 · Find Ctrl+F
View      By connection Ctrl+1 · By type Ctrl+2 · By driver Ctrl+3 · By IOMMU group Ctrl+4 ·
          By power state Ctrl+5 · Problems only Ctrl+Shift+O · Show hidden devices Ctrl+Shift+H ·
          Sidebar F9 · Inspector F4 · Refresh F5 · Events Ctrl+Shift+E
Device    Allow… · Block… · Enable / Disable… · Restart driver Ctrl+R · Choose driver… ·
          Driver parameters… · Show descriptor Ctrl+D · Driver log Ctrl+L ·
          Reserve for passthrough… · Power: may wake · Power: runtime suspend ·
          Open in related app ▸
Policy    USB policy Ctrl+U · New USB rule… · Bluetooth policy · Thunderbolt policy ·
          Simulate a device… · Policy history
Radio     Bluetooth on/off · Make discoverable · Add Bluetooth device Ctrl+B
Help      Help F1 · Glossary · About
```

### 5.5 Keyboard and pointer

Tree: arrows move, `Right`/`Left` expand and collapse, `*` expands all under the node, type-ahead by name. `Space` on a switch column toggles it.

Context menus: device (Allow/Block for removable devices, Enable/Disable, Restart driver, Show descriptor, Copy id, Copy as JSON, Nickname, Open in related app, Reserve group, Show in events); USB rule (Edit, Duplicate, Move up, Move down, Delete); Bluetooth device (Connect, Disconnect, Profiles, Forget, Block); sensor (Copy value, Alert when above…).

Drag and drop: reorder USB rules; drag a device from the tree to the rule table to create a rule matching it; drop a report JSON on the window to open "Compare reports"; drop a policy TOML to open the import review.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Per page: "No Bluetooth adapter", "No Thunderbolt controller", "No sensors reported"; one line each |
| Loading | Skeleton rows; the tree fills by bus as enumeration results arrive |
| Error | Banner with cause; a device in error shows the driver's last message |
| Offline | Not relevant; the app has no network. Firmware page explains offline installation |
| Locked | While the session is locked the app is not visible; prompts for new devices are queued and shown after unlock |
| Read-only | Without administrator approval: view everything, change only own-session items (nicknames, Bluetooth profiles of own devices) |
| Narrow | Sidebar overlay, inspector sheet, tables reduce columns |
| Service restarting | Banner "Reconnecting"; devices keep working |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Device tree | `treeView` with columns |
| Rules, drivers, firmware, IOMMU, sensors, power, events, descriptor steps | `tableView` |
| Inspector | `tabWidget`, `propertyGrid` |
| Overview cards | `panel`, `label`, `link`, `progressBar` |
| Sensor graphs | `chart` |
| Raw configuration space | `codeEdit` (read-only, monospace) |
| Report preview | `codeEdit` (read-only) or `markdownView` |
| Policy switches | `toggleSwitch`, `comboBox`, `checkBox`, `spinBox` |
| Dialogs | `dialog`, `lineEdit`, `radioButton` |

Missing:

- `VM05-GAP-1`: `svc.device`, a device manager system service (tree, events, policy decisions before driver binding, driver control, firmware plan, Bluetooth and Thunderbolt management).
- `VM05-GAP-2`: an administrator-approval prompt (same need as VM01-GAP-4).
- `VM05-GAP-3`: no Bluetooth capability exists in PLATFORM.md section 6; apps that talk to Bluetooth devices directly need `dev.bluetooth:<profile>` and a portal.
- `VM05-GAP-4`: a kernel hook that holds a newly attached USB, Bluetooth or Thunderbolt device unbound until policy answers (not named in plan P3-01 to P3-06).
- `VM05-GAP-5`: a shell-drawn "trusted input" confirmation for the keyboard challenge.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | View models, policy editors, report assembly, redaction |
| UI | FUSE UI | app host | Windows in 5.1 |
| `decode-helper` | C++23 | helper, capability mode | Decode raw PCI configuration space, USB descriptors, HID report descriptors, EDID, ACPI resource blobs, UDA descriptor dumps; parse imported report and policy files |
| Device service `svc.device` | C++23 | system service (Proposed) | Device tree and event stream; policy engine; driver bind, unbind, restart; firmware plan (the `fwget` tables as data); IOMMU reservation; power switches |
| Policy engine | C++23 | inside `svc.device`, separate module | Pure function from (device identity, rules, session state) to a decision |
| Bluetooth daemon | C++23 | system service | Host stack control, pairing, key storage through `svc.keyring` |
| Flash helper | C++23 | sandbox per update | Parse and verify update files; stage them |
| Kernel | C++23 | kernel | Device model (P3-01), IOMMU (P3-02), driver host (P3-03, P3-04), UDA (`pbsd/uda`), gateways (P4-07) |
| `dev` CLI | ForgeScript | own process | Same bus interface |

```
 app host ──bus──▶ svc.device ──▶ kernel device model ──▶ drivers
   └─ decode-helper     ├─ policy engine ◀── attach event (device held unbound)
                        ├─ firmware plan ──request──▶ SY05 (svc.update)
                        ├─ Bluetooth daemon ── keys ──▶ svc.keyring
                        └─ prompts ──▶ shell (SH02 / SH04)
 drivers:  [fast path: in kernel, IOMMU + gateways]   [sandboxed: driver host process]
           [UDA engine + descriptor: MMIO/IRQ grant only]   [HardenedBSD fallback]
```

The app is a viewer and a policy editor. Decisions about a newly attached device are made by `svc.device` and the shell even when the app is not running.

### 6.2 Data model

Device record (from `svc.device`, schema version 1):

| Field | Type | Notes |
|---|---|---|
| `id` | string | Stable path-based id, for example `pci:0000:00:14.3`, `usb:1-4.2` |
| `identity` | `{bus, vendor_id, device_id, subsystem, revision, serial, descriptor_hash}` | Used by policy matching |
| `parent`, `children[]` | id | Topology |
| `class` | enum | Block, Network, Display, Input, Sensor (the UDA classes) plus Audio, Video, Bridge, Hub, Controller, Radio, Security, Other |
| `name`, `nickname`, `location` | string | |
| `state` | enum `working`, `no-driver`, `error`, `blocked`, `disabled`, `reserved`, `suspended` | |
| `driver` | `{name, version, origin: linux\|hbsd\|uda\|nt\|builtin, path: fast\|sandboxed, signer, licence, bound_at, restarts, gateway_faults}` | |
| `grant` | `{mmio[{bar, offset, length}], irqs[], dma_domain, io_ports[]}` | Mirrors `MmioCapability{base, length}` and `IrqCapability{line}` |
| `uda` | `{descriptor_name, provenance, engine, denials}` or null | |
| `firmware` | `{kind, version, date, file, file_hash, package, updatable}` | |
| `power` | `{d_state, runtime_suspend, may_wake, link_state, est_mw}` | |
| `iommu_group` | u32 or null | |
| `external` | bool | Behind an external-facing port |
| `access[]` | `{app_id, capability, since}` | |

Policy stores (in `svc.device`, TOML, signed on export):

| File | Records |
|---|---|
| `usb.toml` | `defaults{unlocked, locked, confirm_keyboards, injection_guard, hidden_function_flag}`; `rule[]{order, match{vid, pid, serial, hash, classes[], port}, interfaces[], action: allow\|allow-ro\|ask\|block\|charge-only, scope, name, last_used}` |
| `bluetooth.toml` | `defaults{require_secure_connections, min_key_size, refuse_just_works_hid, discoverable_s}`; `device[]{address, name, kind, trust, profiles{}, connect}` (keys are in `svc.keyring`) |
| `thunderbolt.toml` | `device[]{uuid, vendor, name, mode: once\|always, tunnels: all\|no-pcie, key_ref}` |
| `drivers.toml` | `deny[]`, `bind[]{device, driver}`, `path[]{device, path}`, `params[]{driver, key, value}` |
| `passthrough.toml` | `reserved[]{group, devices[], when}` |
| `power.toml` | `device[]{id, runtime_suspend, may_wake}` |

App store `~/.local/share/org.pbsd.devices/devices.db` (SQLite): `event(t, device, kind, detail_json)`, `nickname(identity_hash, name)`, `seen(identity_hash, first_seen, last_seen)`, `firmware_history(device, version, t)`, `report(path_grant, t, redacted)`, `ui_state`. Events are written by `svc.device` and read by the app.

Report JSON schema `org.pbsd.hardware-report/1`: `{generated, redacted, system{}, firmware[], devices[], iommu{}, sensors[], policies{}}`.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| PCI configuration space and capability lists | yes | no | Own code | `decode-helper` |
| USB descriptors, HID report descriptors | yes | no | Own code | `decode-helper` (the same decoder is used inside the policy engine, built at tier T2) |
| EDID, DisplayID | yes | no | Own code | `decode-helper` |
| ACPI resource templates, `_PLD` | yes | no | Own code (ACPICA stays in the kernel) | `decode-helper` |
| UDA descriptor dump (fields of `pbsd.uda.schema`) | yes | JSON export | Own code | `decode-helper` |
| Id databases (PCI, USB, PNP, OUI) | yes | no | Text tables compiled to a binary index at package build | app host (read-only handle) |
| `fwget` id-to-package tables | yes | no | Converted at build time from the shell `case` tables into a data file; the shell script is not executed | `svc.device` |
| UEFI capsule, ESRT entries, vendor update archives | yes | no | Own code; signature checks through FORGE crypto (FORGE-WP-22) | flash helper |
| Policy TOML, report JSON | yes | yes | Own code (FORGE-WP-23) | app host; imports through `decode-helper` |
| Report Markdown, PDF, text | no | yes | Own writer; PDF through the print pipeline (G-16) | app host |
| Bluetooth HCI, L2CAP, SMP, profiles | – | – | Bluetooth daemon (base tools exist under `hbsd/src/usr.sbin/bluetooth/`) | Bluetooth daemon and its profile sandboxes |
| Thunderbolt / USB4 connection manager interface | – | – | Kernel and `svc.device` | `svc.device` |

No third-party library is proposed.

### 6.4 Integration

- Intents handled: `open` for `application/x-pbsd-hardware-report+json` (opens the compare view) and `application/x-pbsd-device-policy+toml`; URL scheme `device:` (`device:pci:0000:00:14.3`) so other apps can link to a device. Emitted: `open` to SY01, AD02, UT06, VM01, SY05 (install request), AD04, AD05, SE01, SY04, DV05; `print`; `share` (report file).
- Services: `svc.device` (gap), `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.portal.camera` (device test only), `svc.intent`, `svc.search`, `svc.keyring`, `svc.update`, `svc.power`, `svc.audit`, `svc.model` (optional).
- IPC offered by `svc.device`: `org.pbsd.device.Manager/1` (tree, events, policy, drivers, firmware plan, IOMMU groups, power). Used by VM01 (IOMMU groups, USB redirection checks), VM04 (USB class policy), SY04 (sensors), SE01 (device posture and alerts), SH02 (Bluetooth tile, new-device prompts), AD01 (hardware check and firmware plan at install time).
- Backing repo: facts at the top of section 4 are Verified. `svc.device`, the policy engine and all UI are Proposed.

## 7. Security and privacy

### Capabilities requested

| Capability | Reason |
|---|---|
| `fs.app` | Events cache, nicknames, UI state |
| `fs.portal` | Save reports, import and export policies, pick firmware update files |
| `proc.helper:decode-helper` | Decode device-supplied bytes outside the app host |
| `svc.device` | Everything about devices (VM05-GAP-1) |
| `svc.update` | Hand firmware package requests to the update service |
| `svc.keyring` | Report signing key; Bluetooth key management is done by the daemon |
| `svc.power` | Read wake reasons and battery data |
| `svc.settings`, `svc.notify`, `svc.intent`, `svc.search`, `svc.audit` | Preferences, alerts, links, search, audit lines |
| `dev.camera`, `dev.print` | Camera self-test preview; printing a report |
| `ai.local` | Optional explanations |
| `net.none` | The app opens no sockets; this is what makes "no upload" checkable |

Not requested: any `net.*`, `proc.exec`, `fs.path`, `dev.usb:*` (the app manages USB devices through the service and never opens one itself), `bg.run`.

### Threat model

Assets: the kernel and DMA-reachable memory; the user's input channel (a keyboard is authority); device identifiers (tracking); the integrity of firmware; the policy itself.

| Adversary | Attack | Mitigation |
|---|---|---|
| Malicious USB device | Keystroke injection (BadUSB) | Decision before binding; keyboard challenge; injection heuristics; lock-screen block; per-interface authorisation |
| Malicious USB device | Exploit a class driver with crafted descriptors | USB drivers run on the sandboxed path (plan P3-04); descriptor decoding for policy runs in a T2 decoder; unknown devices get no driver until allowed |
| Malicious USB device | Spoof a remembered device | Identity binding to serial and descriptor hash; interface-set change voids the match |
| Malicious Thunderbolt or PCIe device | DMA read and write of host memory | IOMMU strict mode and per-device domains (plan P3-02); no tunnel before authorisation; external ports never trusted |
| Malicious Bluetooth peer | Unauthenticated pairing, key downgrade, HID injection | Radio off and not discoverable by default; Secure Connections required; Just Works refused for HID; keyboard challenge |
| Hostile device strings | UI spoofing or injection through names and descriptors | All device-supplied text is plain text with length limits and control characters shown escaped |
| Hostile firmware file | Persistent compromise | Signature required; flash helper sandboxed; pre-flight and audit; no online catalogue |
| Hostile report or policy file | Parser exploit, silent policy change | Parsed in `decode-helper`; policy import shows a diff and needs approval; signature shown |
| Malicious local app | Change policy, authorise devices, unblock itself | `svc.device` accepts policy writes only from this app and SY01 with administrator approval; prompts are shell-drawn |
| Buggy or hostile driver | Reach outside its device | MMIO, IRQ and DMA grants (SI-7, P3-02); gateway checks (P4-07); restartable sandbox |
| Observer of an exported report | Track or fingerprint the machine | Redaction on by default with stable placeholders; preview before save |

Stored: policies and reservations in `svc.device` (system), events and nicknames in app data, Bluetooth keys in `svc.keyring`. Leaves the machine: nothing. Firmware packages are fetched by SY05 under its own rules.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `view.mode` | enum `connection`, `type`, `driver`, `iommu`, `power` | `connection` | Default tree view |
| `view.show_hidden` | bool | `false` | Show bridges, hubs, virtual devices |
| `view.reveal_identifiers` | bool | `false` | Show serials and addresses without a click |
| `usb.unlocked` | enum `allow`, `ask`, `block` | `ask` | New devices while unlocked |
| `usb.locked` | enum `allow`, `block` | `block` | New devices while locked |
| `usb.confirm_keyboards` | bool | `true` | Keyboard challenge |
| `usb.injection_guard` | bool | `true` | Suspend self-typing devices |
| `usb.injection_rate_keys_s` | u32 | `50` | Heuristic threshold |
| `usb.suspend_external_hid_on_lock` | bool | `false` | |
| `bluetooth.discoverable_s` | u32 60–600 | `120` | |
| `bluetooth.require_secure_connections` | bool | `true` | |
| `bluetooth.refuse_just_works_hid` | bool | `true` | |
| `thunderbolt.authorise_when_locked` | bool | `false` | Fixed off unless policy changes it |
| `thunderbolt.deauthorise_on_lock` | bool | `false` | |
| `firmware.allow_nonfree` | bool | `true` | Offer non-free firmware packages |
| `sensors.unit` | enum `auto`, `c`, `f` | `auto` | |
| `sensors.alerts` | bool | `true` | |
| `report.redact` | bool | `true` | Default for exports |
| `report.format` | enum `json`, `md`, `pdf`, `txt` | `json` | |
| `events.keep_days` | u32 | `90` | |
| `ai.explain` | bool | `false` | Needs `ai.local` |

USB, Bluetooth and Thunderbolt keys are system settings: changing them needs administrator approval.

## 9. Accessibility and languages

- The device tree exposes level, position, state and confinement path as text for each row ("Wi-Fi AX211, working, driver iwlwifi, sandboxed, level 2").
- New-device prompts are shell prompts and are announced by AX01 with the device's claimed kind first.
- The keyboard challenge has a route for users who cannot type a code: confirmation from any already trusted input device, including switch access.
- Pairing numbers are read digit by digit.
- Badges (▣, ▢, ⚠, ✕) always carry a word; colour is never the only signal.
- Bus terms have a glossary; device and vendor names are never translated; ids are shown in a monospace face.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | 200 ms; tree of 300 devices filled within 400 ms |
| Hot-plug to row update | under 100 ms after the service event |
| Policy decision for a new USB device (service side) | under 50 ms excluding user prompts |
| Search over 2,000 devices and drivers | under 20 ms per key |
| Sensor refresh | 1 Hz while the page is visible; zero wake-ups when hidden |
| Report generation | under 1 s for 300 devices; PDF under 3 s |
| App memory | under 70 MiB |

## 11. Testing and proof

Nothing has been run. All entries are targets.

- Unit: rule matching and ordering, identity binding, redaction (no identifier survives; placeholders stable), report diff, tree building from service records.
- Golden renders of 5.2 and 5.3.
- Scripted UI: T1–T13 against a fake `svc.device` that replays recorded device trees and hot-plug traces.
- Fuzz: PCI configuration decoder, USB and HID descriptor decoders, EDID, ACPI resource decoder, capsule and update-archive parsers, policy and report importers.
- Differential: decoded PCI and USB output compared with `pciconf -lvc` and `usbconfig dump_all_desc` text on the same machine; firmware plan compared with `fwget -n` output on the same machine.
- Policy tests with emulated USB devices: storage stick, keyboard, composite storage+HID, device that re-enumerates as HID, fast typer, spoofed serial; each must reach the expected decision.
- Hardware lab: Thunderbolt dock authorisation, DMA attack device against a locked machine (must fail, as in the acceptance of plan P3-02), Bluetooth pairing matrix.
- Fault injection: restart `svc.device` and a sandboxed driver during use; unplug during firmware staging; corrupt a policy file.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| USB policy engine | Deterministic; default action applies when no rule matches; a locked session never yields `allow` for a new device | T1 | Model in Lean 4 or TLA+; code contracts by ESBMC through PRISM | – |
| Identity binding | A rule with a serial and hash never matches a device with a different interface set | T1 | Same | – |
| Descriptor decoders used by policy | No runtime errors on any input; bounded output | T2 | Safe subset, contracts, bounded checking, fuzzing | – |
| Redaction | Every field tagged as identifier is replaced in every output format | T2 | Schema-driven check; property tests | Free-text fields from devices |
| IOMMU reservation logic | A reservation always covers the whole group | T2 | Contracts, tests | – |
| UDA interpreter grant check | Already an SI-7 obligation of `pbsd.uda.interp` | per the UDA specification | Outside this app | – |
| Flash helper | Memory safety; refuses unsigned input | T3 | Sanitizers, fuzzing | Vendor formats |
| App logic | No capability widening | T3 | FSS policy, script tests | Interpreter not verified |
| UI | – | T4 | Golden and scripted tests | – |

## 12. Build plan

### Phase 1: usable core

| ID | Work item | Depends on |
|---|---|---|
| VM05-W01 | `org.pbsd.device.Manager/1` schema; fake service with recorded trees | G-08, VM05-GAP-1 |
| VM05-W02 | Main window: tree views, search, inspector Summary and Driver tabs | G-01, G-02, VM05-W01 |
| VM05-W03 | `svc.device` core on the existing device model: tree, events, id databases | G-09; P3-01 for the final form |
| VM05-W04 | `decode-helper`: PCI, USB descriptors | FORGE-WP-27, FORGE-WP-28 |
| VM05-W05 | USB policy engine: defaults, allow-list, new-device prompt, lock-screen rule, blocked list | VM05-GAP-4, SH02, SH04 |
| VM05-W06 | Firmware needed list from the `fwget` tables; hand-off to SY05 | SY05, G-09 (`svc.update`) |
| VM05-W07 | Overview page and status bar | VM05-W02 |

### Phase 2: full feature

| ID | Work item | Depends on |
|---|---|---|
| VM05-W08 | Driver origin and path view, restart, bind and unbind, deny list, driver log | P3-03, P3-04 |
| VM05-W09 | UDA descriptor view and grant bounds | `pbsd/uda`, VM05-W04 |
| VM05-W10 | BadUSB protection: keyboard challenge, injection heuristics, hidden-function check, per-interface authorisation | VM05-GAP-5, VM05-W05 |
| VM05-W11 | Per-app USB grants view and revoke | AD05, FORGE-WP-06 |
| VM05-W12 | Bluetooth daemon integration, pairing, profiles, policy | VM05-GAP-3, SH02 |
| VM05-W13 | Thunderbolt authorisation and DMA protection state | P3-02 |
| VM05-W14 | IOMMU page, reservation and release | P3-02, VM05-GAP-2, VM01 |
| VM05-W15 | Sensors page, battery health, power tab, wake sources | G-09 (`svc.power`) |
| VM05-W16 | Hardware report with redaction and preview; events page | G-16 (PDF) |
| VM05-W17 | Firmware inventory, loaded-file verification, platform settings | SY05, SE01 |
| VM05-W18 | `dev` CLI | VM05-W01 |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| VM05-W19 | Flash updates from files with pre-flight | flash helper, FORGE-WP-22 |
| VM05-W20 | Move drivers between paths, parameters, gateway faults | P3-04, P4-07 |
| VM05-W21 | SR-IOV, domain view, DMA fault log | P3-02 |
| VM05-W22 | Secure connect, limited tunnel modes, boot access list | VM05-W13 |
| VM05-W23 | Report compare and signing; policy export, import, history, simulation | SE04 |
| VM05-W24 | Power page, blockers, port control, capture hand-off | DV05 |
| VM05-W25 | Descriptor catalogue, NT drivers, device tests, local explanations | P4-04, G-15 |

## 13. Open questions and risks

- The UDA specification and code disagree in small ways (Verified): the specification gives `vendor_id` and `device_id` as `uint32_t`, the schema module uses `std::uint16_t`; the specification lists classes "Block, Net, Hid, …", the code has five classes while eight engines exist (crypto, usb and pci engines have no class value). The app's device-class list must follow whatever is settled.
- The plan has two driver worlds: UDA descriptors with generic engines (existing code) and Linux drivers through the driver host (Phase 3, not yet present). P3-04 says "pbsd/uda descriptors — keep as descriptor format". How a device chooses between a UDA engine and a Linux driver is not stated; the Driver tab assumes the kernel reports one origin per device.
- Holding a device unbound until user-space policy answers (VM05-GAP-4) is not in the plan's Phase 3 packages. Without it USB policy cannot be enforced before a driver sees the device.
- The keyboard challenge needs a notion of "trusted input device" in the compositor and shell; Unverified.
- `fwget` installs with `pkg install` from the network. Here the tables are reused as data and installation goes through SY05; keeping the tables in step with upstream needs an import script.
- Firmware flashing varies by vendor; only UEFI capsules are well specified. Scope for phase 3 may shrink to capsules.
- Bluetooth host stack: the base system's tools are C and old; whether the stack moves to the sandboxed driver path or a new daemon is undecided.
- Linux-derived driver modules are GPL-2.0-only while PBSD's own code is AGPL-3.0-or-later (the plan lists this as a risk). The Drivers page shows licences but the legal structure is the owner's decision.
- No Bluetooth capability exists in PLATFORM.md section 6 (VM05-GAP-3).
