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
