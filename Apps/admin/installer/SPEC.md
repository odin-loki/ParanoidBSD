<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Installer and First Run

By Odin Loch

| | |
|---|---|
| ID | `AD01` (from CATALOGUE.md) |
| Bundle | `org.pbsd.installer` |
| Category | admin |
| Priority | P0 |
| Backing repos | ParanoidBSD (`hbsd/src/usr.sbin/bsdinstall` as reference, OpenZFS, GELI, `libbe`); FUSE (UI); FORGE (ForgeScript, keystore) |
| Status | Planned. Nothing built. |

## 1. Purpose

Installs ParanoidBSD from live media onto a machine, with ZFS on root, full-disk encryption, a verified boot chain and a hardening profile chosen by default rather than as an afterthought. After the first boot it runs a short welcome that sets privacy choices, appearance and optional data import. It replaces the text-mode `bsdinstall` (Verified: `hbsd/src/usr.sbin/bsdinstall/`), and it is the first installer work in this repo (Verified: `docs/ROADMAP.md` section 20 says "there is no installer work at all yet").

## 2. Scope

In scope:

- Graphical installer on live media: language, keyboard, optional network, disk choice, ZFS layout, encryption, boot chain, recovery key, users, hardening, packages, summary, progress.
- Offline install from the media's own package set; nothing is fetched unless the user turns the network on.
- Unattended install from a signed plan file.
- Dual boot beside an existing system; repair mode for an installed system.
- Hardware compatibility check before any write.
- First-run welcome in the installed system: privacy, theme, optional accounts, tour, import from another OS.

Out of scope:

| Item | Owner |
|---|---|
| Day-to-day disk, pool and dataset management after install | AD02 Disks |
| Snapshot schedules and off-site backup (first run only links to it) | AD03 Backup |
| Package install, update, boot-environment rollback after install | SY05 Software |
| Storing the recovery key, passphrases and keys in normal use | SE04 Passwords and Keys |
| Per-app permission review (first run only sets the profile) | AD05 Permissions |
| Full settings pages (display, sound, network, users) | SY01 Settings |
| Login greeter and disk unlock prompt at boot | SH03 Login, Lock and Session |
| Firmware updates and driver detail | VM05 Devices |
| Building the live image and release signing | repo `release/` tooling (Verified gap: `docs/ROADMAP.md` section 13 "No release signing") |

## 3. Users and top tasks

Personas: **Asha**, a new user moving from another OS with one laptop; **Bo**, an administrator installing twenty machines from one plan; **Cal**, a security researcher who wants every boot-chain detail; **Dee**, a user whose system no longer boots.

| # | Task | Target |
|---|---|---|
| T1 | Install on one empty disk with all defaults | 9 clicks, 2 typed fields (name, passphrase twice) |
| T2 | Install with mirror or RAID-Z across several disks | T1 + 2 clicks |
| T3 | Install beside an existing OS (dual boot) | T1 + 3 clicks, one size slider |
| T4 | Install with no network at all | No extra step; network page is skippable |
| T5 | Record the recovery key and prove it was recorded | 1 page, 3 actions (show, save or print, confirm 2 groups) |
| T6 | Enrol the TPM so the disk unlocks only on an untampered boot chain | 1 toggle + PIN |
| T7 | Run an unattended install from a plan on a USB stick | 0 clicks after boot when the plan is signed and says `confirm: false` |
| T8 | Check whether this machine is supported before changing it | 2 clicks from the welcome page |
| T9 | Repair the boot loader of an installed system | 4 clicks + passphrase |
| T10 | Roll an unbootable system back to a previous boot environment | 3 clicks + passphrase |
| T11 | Finish first run with strict privacy defaults | 5 clicks ("Next" on each page) |
| T12 | Import documents, bookmarks and keys from the old OS on the same disk | 4 clicks |
| T13 | Export the choices of this install as a plan for other machines | 2 clicks on the summary page |

## 4. Features

### 4.1 Live session and start

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD01-F001 | Boot menu entries | Loader menu on the media: Install, Try without installing, Repair, Unattended, Memory test, Safe graphics (CPU rasteriser, V1). | M | 1 |
| AD01-F002 | Media self-check | Before the first page, hash every package and image on the media against the signed manifest (BLAKE3 tree, Ed25519). Result shown as a trust badge; failure blocks install with the failing file named. | M | 1 |
| AD01-F003 | Live mode | Run the desktop from RAM with a tmpfs overlay; no disk is opened for writing; an "Install" launcher stays on the panel. | M | 1 |
| AD01-F004 | Low-memory mode | Under 2 GiB RAM the live session starts the installer alone, full screen, with no shell or panel. | S | 2 |
| AD01-F005 | Resume after crash | Wizard state (never secrets) is saved to tmpfs after each page; a restarted installer returns to the same page. | M | 1 |
| AD01-F006 | No-write guarantee before commit | The install service refuses every write request until the plan has been confirmed on the summary page; the storage broker opens disks read-only until then. | M | 1 |
| AD01-F007 | Console fallback | If no display comes up in 20 s, a text front end of the same plan engine runs on the console and serial line. | S | 2 |
| AD01-F008 | Installer log | Every step and command is written to `/tmp/installer.log` and copied to `/var/log/installer/` on the target; passphrases and keys are never logged. | M | 1 |
| AD01-F009 | Debug shell | `Ctrl+Alt+F2` or Help ▸ Open terminal starts SY03 in the live session; a banner warns that manual changes can invalidate the plan. | S | 1 |
| AD01-F010 | Screen lock off, sleep inhibited | While a plan runs the session holds a `svc.power` inhibit. | M | 1 |

### 4.2 Language, keyboard, accessibility

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD01-F011 | Language list | Searchable list in each language's own name; choice applies at once to the whole wizard without restart. | M | 1 |
| AD01-F012 | Region and formats | Region drives time zone guess, date, number, paper and unit formats; each can be changed separately. | M | 1 |
| AD01-F013 | Keyboard layout and variant | Layout list filtered by language, variant list, test field, key-position diagram; multiple layouts with a switch shortcut. | M | 1 |
| AD01-F014 | Layout detection by key press | "Press the key to the right of left Shift" style sequence narrows the layout. | C | 3 |
| AD01-F015 | Passphrase layout warning | The layout active at boot unlock is shown on the encryption page; warns when the passphrase uses characters that differ between the chosen layout and US. | M | 1 |
| AD01-F016 | Accessibility bar | Always-visible buttons: screen reader (AX01), magnifier (AX02), on-screen keyboard (AX03), high contrast, large text, reduced motion. Choices carry into the installed system. | M | 1 |
| AD01-F017 | Screen reader hot key at boot | `Super+Alt+S` starts speech before the first page; the first page announces itself after 10 s of no input. | M | 2 |
| AD01-F018 | Time zone | Map picker and searchable city list; UTC hardware clock by default; "local time clock" switch for dual boot with Windows. | M | 1 |
| AD01-F019 | Time source | Manual, or NTP/NTS only if the network was enabled; time is never fetched silently. | S | 2 |

### 4.3 Hardware compatibility check

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD01-F020 | Requirements test | CPU features (64-bit, AES, RDRAND, SMEP/SMAP, virtualisation), RAM, disk size, firmware type (UEFI required for verified boot), TPM 2.0, Secure Boot state. Each row: pass, warn or fail with a plain reason. | M | 1 |
| AD01-F021 | Device inventory | PCI, USB, ACPI devices with driver state: supported in base, supported by UDA descriptor (`docs/specs/UDA_DESCRIPTOR.md`), needs firmware blob, unsupported. | M | 1 |
| AD01-F022 | Graphics test | Shows a test pattern through the Vulkan backend then the CPU rasteriser; records which one works. | S | 2 |
| AD01-F023 | Network device test | Lists wired and wireless adapters and whether firmware is on the media; does not connect. | S | 2 |
| AD01-F024 | Suspend and audio probes | Optional, with user consent: play a tone, record from the microphone, suspend for 10 s and resume. | C | 3 |
| AD01-F025 | Disk health pre-check | SMART or NVMe health summary per target disk (same reader as AD02); a failing disk needs an explicit override. | M | 1 |
| AD01-F026 | Memory quick test | 60 s pattern test of free RAM; full test is the loader entry. | C | 3 |
| AD01-F027 | Firmware settings advice | Lists firmware settings that weaken the install (CSM on, Secure Boot off, DMA protection off, firmware password not set) and how to change them. | S | 2 |
| AD01-F028 | Compatibility report | Save the result as Markdown and JSON to a removable disk; contains no serial numbers unless "include identifiers" is ticked. | S | 2 |
| AD01-F029 | Known-machine notes | A local, signed table of machine models with known issues is shipped on the media; matched by DMI strings. | C | 3 |

### 4.4 Network (optional)

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD01-F030 | Offline by default | The network is off when the installer starts. The page says so and has one "Stay offline" primary button. | M | 1 |
| AD01-F031 | Wired and Wi-Fi setup | DHCP or static IPv4/IPv6; Wi-Fi scan, WPA2/WPA3 passphrase, hidden network. | M | 1 |
| AD01-F032 | What the network is used for | A fixed list on the page: newer signed packages, extra languages, firmware, time. Each is a separate checkbox, all off. | M | 1 |
| AD01-F033 | Mirror choice | The update mirror and its signing key fingerprint are shown; a custom mirror needs its key from removable media. | S | 2 |
| AD01-F034 | Route choice | Clear, VPN profile file, or Tor for installer fetches (hands the route to the same mechanism SE03 uses). | C | 3 |
| AD01-F035 | MAC address randomisation | On by default in the live session and carried into the installed system. | M | 1 |
| AD01-F036 | Proxy | HTTP(S) or SOCKS proxy for installer fetches only. | C | 2 |
| AD01-F037 | Carry network settings | Checkbox to copy the connection (and its secret) into the installed system's keyring; off by default. | S | 2 |
| AD01-F038 | First-contact list | Every host the installer contacts is listed live on the page and written to the install log. | M | 2 |

### 4.5 Install type and disk selection

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD01-F039 | Install types | Erase a disk; Install beside another system; Replace an existing ParanoidBSD (keep `/home`); Manual layout; Add a boot environment to an existing pool. | M | 1 |
| AD01-F040 | Disk list | Every disk: model, size, bus (NVMe, SATA, USB), rotational or solid state, sector size, health, current contents summary (partition table, systems detected). The install medium is greyed out. | M | 1 |
| AD01-F041 | Existing system detection | Detects ParanoidBSD/FreeBSD pools, Windows (ESP + MSR + NTFS), Linux (ext4, btrfs, LUKS, LVM) by label readers; read-only probing only. | M | 1 |
| AD01-F042 | BitLocker and LUKS detection | Warns that shrinking is not possible while the volume is encrypted and names the steps to take in the other OS. | S | 2 |
| AD01-F043 | Multi-disk selection | Tick several disks; the layout page offers the vdev types that the count allows. | M | 1 |
| AD01-F044 | Erase confirmation | The summary lists each disk by model, size and serial tail; the commit button text is "Erase 2 disks and install". | M | 1 |
| AD01-F045 | Removable target | Install onto a USB disk as a portable system; boot loader goes only on that disk. | S | 2 |
| AD01-F046 | Disk identify | "Blink" a disk's activity light or show its bay where enclosure services exist, to match a row to a physical disk. | C | 3 |
| AD01-F047 | 4K alignment | Partitions aligned to 1 MiB; pool `ashift` set from the physical sector size, minimum 12. | M | 1 |
| AD01-F048 | Pre-existing GELI or ZFS-encrypted pools | Offer to unlock (to reuse `/home` or add a boot environment) with passphrase or key file. | S | 2 |

### 4.6 ZFS layout and partitions

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD01-F049 | Default partition plan | GPT: ESP 512 MiB (FAT32), swap (size rule below), ZFS partition for the rest. Reference defaults read from `hbsd/src/usr.sbin/bsdinstall/scripts/zfsboot` (pool `zroot`, BE root `ROOT`, boot fs `default`, `compression=on`, `atime=off`). | M | 1 |
| AD01-F050 | Pool topology | Stripe (1 disk), mirror (2+), RAID-Z1 (3+), RAID-Z2 (4+), RAID-Z3 (5+), striped mirrors (4+, even). Usable size and failure tolerance shown live. | M | 1 |
| AD01-F051 | Topology diagram | Small read-only diagram of pool ▸ vdev ▸ disk ▸ partition, updated as options change. | S | 2 |
| AD01-F052 | Dataset layout | Proposed tree: `ROOT/<be>`, `home`, `home/<user>` (own dataset per user), `var/log`, `var/audit`, `var/crash`, `var/tmp`, `tmp`, `usr/ports` off, `apps` (verified, read-only bundles), `vm`. Each with mount options `exec`, `setuid`, `devices` shown and editable. | M | 1 |
| AD01-F053 | Safe mount defaults | `/tmp`, `/var/tmp`, `/home`: `setuid=off`, `exec=off` in the hardened profile; `/var/log`, `/var/audit`: `exec=off`, `setuid=off`. | M | 1 |
| AD01-F054 | Pool properties | Compression (`zstd` default, `lz4`, off), checksum (`blake3` default where supported, `sha256`), `atime=off`, `xattr=sa`, `autotrim` on for SSDs, `copies` for single-disk installs (1 or 2 for `home`). | M | 1 |
| AD01-F055 | Swap | Size rule: RAM ≤ 8 GiB → equal to RAM, else 8 GiB; none allowed; always encrypted with a one-time key (`.eli` swap); mirrored when the pool is mirrored. Hibernation is not offered. | M | 1 |
| AD01-F056 | Reserved space | A `reserved` dataset with a 5 % `refreservation` so a full pool can still delete files. | S | 1 |
| AD01-F057 | Pool name and BE name | Editable; validated against ZFS naming rules; clash with an importable pool is refused. | M | 1 |
| AD01-F058 | Manual partition editor | Table and bar view of each disk: create, delete, resize, set type and label; same engine as AD02; pending-changes list, nothing written until commit. | M | 2 |
| AD01-F059 | Manual vdev editor | Build data, log, cache, special and spare vdevs by hand from partitions. | S | 2 |
| AD01-F060 | Dual boot: shrink | Shrink NTFS or ext4 only when a safe resizer is available; otherwise the page asks the user to shrink in the other OS and offers "use free space". Proposed: no NTFS resize in phase 1. | M | 2 |
| AD01-F061 | Dual boot: shared ESP | Reuse the existing ESP; add `\EFI\pbsd\`; never touch the other system's loader files; add a firmware boot entry and set the order (reference tool: `hbsd/src/usr.sbin/efibootmgr`). | M | 2 |
| AD01-F062 | Dual boot: clock and Fast Startup warnings | Explains the Windows Fast Startup and RTC issues; offers the local-time RTC switch. | S | 2 |
| AD01-F063 | Replace system, keep home | New boot environment and system datasets; `home` datasets kept untouched and snapshotted first. | S | 2 |
| AD01-F064 | Pre-install snapshot | When any existing dataset is reused, take `@pre-install-<date>` first. | M | 2 |
| AD01-F065 | Layout presets | Laptop (single disk), Workstation (mirror), Server-like (RAID-Z2), Portable USB. | S | 2 |
| AD01-F066 | Space forecast | Bar showing system, packages selected, swap, reserved, free. Refuses when the minimum (16 GiB) is not met. | M | 1 |

### 4.7 Encryption

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD01-F067 | Encryption on by default | The default plan encrypts everything except the ESP. Turning it off needs a typed confirmation and marks the install "unencrypted" in SE01. | M | 1 |
| AD01-F068 | Mode A: GELI under ZFS | Whole ZFS partition on a GELI provider: AES-XTS 256, 4096-byte sectors, as `bsdinstall` does (Verified: `geli init -bg -e … -l 256 -s 4096` in `hbsd/src/usr.sbin/bsdinstall/scripts/zfsboot:200-201`). Hides dataset names and all metadata. Default. | M | 1 |
| AD01-F069 | Mode B: ZFS native encryption | `encryption=aes-256-gcm`, `keyformat=passphrase` or `raw`, per dataset (Verified commands exist: `hbsd/src/sys/contrib/openzfs/man/man8/zfs-load-key.8`, `zfs-change-key.8`). Dataset names and sizes stay visible; allows raw encrypted `zfs send` for AD03. | M | 2 |
| AD01-F070 | Mode C: both | GELI for the disk, ZFS native keys per user home so one user's data stays locked while another is logged in. | S | 2 |
| AD01-F071 | Boot with GELI | Loader unlocks GELI (`-g` flag) or uses a small unencrypted boot pool (`bootpool`, Verified default name and 2 GiB size in `zfsboot:99-106`). Which path is used with verified boot is an open question (section 13). | M | 1 |
| AD01-F072 | Passphrase entry | Two fields, reveal on hold, caps-lock and layout indicator, offline strength estimate (length and word-list checks, no network), minimum 12 characters or 5 words. | M | 1 |
| AD01-F073 | Passphrase generator | Offline diceware-style generator from a bundled word list; number of words selectable (5–10). | S | 1 |
| AD01-F074 | Key derivation cost | GELI PBKDF2 iterations calibrated to about 2 s on this machine; ZFS `pbkdf2iters` set to at least 1 000 000. Shown and editable under Advanced. | S | 2 |
| AD01-F075 | Key slots | GELI has two user key slots (Verified: `geli.8` describes key index 0 and 1). Slot 0: passphrase. Slot 1: recovery key. TPM-sealed secret wraps a key file component, not a third slot. | M | 1 |
| AD01-F076 | Hardware token unlock | FIDO2 `hmac-secret` or smart card as a key component; needs the token at boot. Depends on loader support (Proposed). | C | 3 |
| AD01-F077 | Per-user home encryption | Each `home/<user>` dataset gets its own ZFS key unlocked by that user's login (hand-off to SH03). | S | 2 |
| AD01-F078 | Encrypted swap and dumps | Swap with one-time keys; crash dumps encrypted to a public key generated at install (private key in SE04). | M | 2 |
| AD01-F079 | Detached header backup | GELI metadata backup (`geli backup`) saved to removable media with the recovery sheet. | S | 2 |
| AD01-F080 | Wipe before install | Optional: TRIM/unmap whole device, ATA or NVMe secure erase where supported (same engine as AD02), or one random pass. | S | 2 |
| AD01-F081 | Plain-language comparison | Table comparing the three modes: what an attacker with the disk sees, boot behaviour, backup behaviour. | M | 1 |

### 4.8 Verified boot, measured boot, TPM

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD01-F082 | Boot chain page | Shows the chain as rows: firmware ▸ Secure Boot keys ▸ loader ▸ kernel and modules ▸ veriexec manifest ▸ root dataset ▸ app bundles; each row says what verifies it. | M | 1 |
| AD01-F083 | Secure Boot: signed loader | Install the loader signed by the PBSD release key; needs that key in firmware `db`. Depends on loader signing, which does not exist yet (Verified gap: `docs/ROADMAP.md` section 14, "Nothing found under `stand/`"). | M | 2 |
| AD01-F084 | Secure Boot: own keys | Generate PK, KEK and db keys on this machine, enrol them (firmware in setup mode), sign the loader locally; private keys go to SE04 or to a removable disk, never to the installed disk in clear. | S | 3 |
| AD01-F085 | Verified exec | Enable `mac_veriexec` with a signed manifest of base and app files (Verified in tree: `hbsd/src/sys/security/mac_veriexec/`, `hbsd/src/sbin/veriexec`; not enabled in `std.hardenedbsd` per `docs/ROADMAP.md` section 14). Enforce level chosen by the hardening profile. | M | 2 |
| AD01-F086 | Measured boot | Loader and kernel extend PCRs with what they load; event log kept for attestation in SE01. Kernel TPM driver is Verified (`hbsd/src/sys/dev/tpm/tpm20.c`); loader measurement is Proposed. | S | 2 |
| AD01-F087 | TPM enrolment | Seal a disk key component to PCRs 0, 2, 4, 7 (+ a PBSD PCR for kernel and manifest). Modes: TPM + PIN (default when a TPM exists), TPM only (warned), passphrase only. | M | 2 |
| AD01-F088 | TPM PIN | 6–20 characters; TPM dictionary-attack lockout parameters shown; PIN is not the disk passphrase. | M | 2 |
| AD01-F089 | No TPM path | Clear statement that unlock is passphrase only and that boot-chain tampering is detected only by Secure Boot and veriexec. | M | 1 |
| AD01-F090 | PCR policy preview | Lists which future events will require the recovery key (firmware update, Secure Boot key change, loader change) and that SY05 re-seals after signed updates. | S | 2 |
| AD01-F091 | Firmware password reminder | Detects where possible, otherwise asks; recorded as an advisory in SE01. | C | 3 |
| AD01-F092 | Kernel module lock | `HARDEN_KLD` / `hardening.forbid_kmod` after boot (named in `docs/ROADMAP.md` sections 14–15) set by profile; page lists modules that must then load early. | S | 2 |
| AD01-F093 | Boot environments from day one | First BE `default`; the loader menu lists BEs; a failed-boot counter falls back to the previous BE (plan item: `docs/plan/PBSD_PLAN.md` section 7 "Atomic A/B updates"). | M | 2 |
| AD01-F094 | Boot chain self-test | After install and before reboot: re-read loader, kernel and manifest from disk, verify signatures and hashes, show the result. | M | 2 |

### 4.9 Recovery key

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD01-F095 | Recovery key generation | 256 random bits from the kernel RNG, shown as 8 groups of 6 base-32 characters with a check character per group. | M | 1 |
| AD01-F096 | Ways to keep it | Write it down (large type view), print (only if a printer works offline), save as a text file or PDF to removable media (never to the target disk), show as a QR code. | M | 1 |
| AD01-F097 | Proof of recording | The user types two randomly chosen groups before "Next" is enabled. | M | 1 |
| AD01-F098 | Split recovery key | Shamir split, k of n (2 of 3 up to 5 of 8), one sheet per share. | C | 3 |
| AD01-F099 | Recovery sheet | One page: machine name, disk serial tail, date, key, GELI header backup hint, steps to use it. No passphrase on it. | M | 1 |
| AD01-F100 | Skip with consequence | Skipping needs a typed sentence and records "no recovery key" in SE01. | M | 1 |
| AD01-F101 | Screen-capture guard | The page asks the compositor to exclude the window from capture and recording. | S | 2 |
| AD01-F102 | Clipboard block | The key cannot be copied to the clipboard; selection is disabled. | M | 1 |
| AD01-F103 | Recovery key rotation hand-off | First run and SE04 can replace the recovery key later (re-keys slot 1). | S | 2 |

### 4.10 Users, host, hardening profile

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD01-F104 | First user | Full name, login name (validated), password with the same strength meter, optional picture. | M | 1 |
| AD01-F105 | Administrator model | The first user is an administrator who confirms privileged actions through SH03 prompts; no root password is set and direct root login is disabled by default. | M | 1 |
| AD01-F106 | Separate admin account | Option: create a distinct admin user so the daily account has no admin authority. Default in the Strict profile. | S | 1 |
| AD01-F107 | More users | Add further users now, each with own home dataset and quota. | S | 2 |
| AD01-F108 | Automatic login | Off; offered only when the disk is encrypted; never in the Strict profile. | S | 2 |
| AD01-F109 | Host name | Default is a random neutral name (`pbsd-xxxx`), not derived from the user or hardware. | M | 1 |
| AD01-F110 | Hardening profiles | Balanced (default) and Strict, with a per-item list. Items map to `hardening.*` sysctls (59 knobs counted in `docs/ROADMAP.md` section 15), the eight `bsdinstall` options (Verified list in `hbsd/src/usr.sbin/bsdinstall/scripts/hardening`: hide jails, lock shared memory, mlock limit, clean `/tmp`, no syslogd socket, console password, no destructive DTrace, firewall) and PBSD defaults. | M | 1 |
| AD01-F111 | Profile detail view | Table: setting, Balanced value, Strict value, what may break. Each row can be overridden; overrides are recorded in the plan. | M | 1 |
| AD01-F112 | App capability profile | Sets the default prompt policy that AD05 uses (Strict: no "always" grants at install, prompts every session; Balanced: remembered grants allowed). | M | 1 |
| AD01-F113 | Firewall default | Deny inbound, per-app outbound prompts (SE02) in both profiles; Strict also blocks all outbound until an app asks. | M | 1 |
| AD01-F114 | Services default | No listening network service enabled. SSH server off; enabling it requires key-only login. | M | 1 |
| AD01-F115 | USB policy default | New USB storage and input devices while locked are refused (policy owned by VM05). | S | 2 |
| AD01-F116 | Per-binary exceptions | Lists packages in the selection that need a PaX exception (`hbsdcontrol`, Verified tool: `hbsd/src/usr.sbin/hbsdcontrol`) and asks consent per package. | S | 2 |

### 4.11 Package selection and installation

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD01-F117 | Sets | Minimal (shell, Files, Settings, Terminal, Software, Permissions, Security Centre, Text Editor), Standard (all P0 and P1 apps), Full (everything on the media), Custom. | M | 1 |
| AD01-F118 | Custom tree | Category ▸ app tree with size, licence, capability summary and network badge per app; dependency closure computed locally. | M | 2 |
| AD01-F119 | Capability preview | Selecting an app shows the capabilities its manifest declares, before it is installed. | S | 2 |
| AD01-F120 | Language packs, dictionaries, fonts | Chosen from what is on the media; more only with network. | S | 2 |
| AD01-F121 | Firmware blobs | Listed separately with licence; only those matching detected devices are preselected. | M | 1 |
| AD01-F122 | Local models | Optional `.fpk` models for `svc.model` with size and RAM need; none in Minimal. | C | 3 |
| AD01-F123 | Offline repository | The media carries a complete signed repository; the installed system keeps it as an offline source for SY05. | M | 1 |
| AD01-F124 | Signature and SBOM check | Every package signature verified before extraction; SBOM recorded for SY05. | M | 1 |
| AD01-F125 | Summary page | Every choice in one scrollable list with "Change" links; destructive items in `--danger`; estimated time. | M | 1 |
| AD01-F126 | Plan diff | For "replace" and "dual boot": a before and after partition and dataset table. | S | 2 |
| AD01-F127 | Staged, journalled execution | Steps: partition, encrypt, create pool, create datasets, extract, configure, boot chain, verify. Each step idempotent with a journal so a failed step can retry. | M | 1 |
| AD01-F128 | Progress | Per-step list with state, overall bar, rate, time left, current file hidden behind "Details"; live log pane. | M | 1 |
| AD01-F129 | Cancel and rollback | Cancel before "create pool" restores the old partition table from the saved copy; after that, the page says what state the disk is in. | M | 1 |
| AD01-F130 | Post-install verification | Re-hash installed files against package manifests; `zpool status` must be clean; first snapshot `@install` taken on every dataset. | M | 1 |
| AD01-F131 | Finish page | Reboot, shut down, or keep using live session; reminder to remove media; install report saved. | M | 1 |
| AD01-F132 | Eject and boot order | Sets the firmware boot entry; offers to eject optical or USB media. | S | 1 |

### 4.12 Unattended install

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD01-F133 | Plan file | `install-plan.json`, schema-versioned, covers every wizard choice. Found on the media root, a second USB stick, or a URL (only with network enabled in the plan itself). | M | 2 |
| AD01-F134 | Plan signature | Detached Ed25519 `.sig` (same scheme as V14). Unsigned plans always stop at the summary page for confirmation. | M | 2 |
| AD01-F135 | Secrets in plans | Passphrases are never stored in clear: either prompted, or wrapped to a one-time age-style public key whose private half is typed at the console, or a salted hash for user passwords. | M | 2 |
| AD01-F136 | Disk matching rules | Select disks by size range, bus, model pattern or serial; refuse when zero or more than the expected number match. | M | 2 |
| AD01-F137 | Dry run | `--dry-run` produces the full step list and the partition diff without writing. | M | 2 |
| AD01-F138 | Export plan | Summary page ▸ "Save as plan" writes the current choices minus secrets. | S | 2 |
| AD01-F139 | Validation | Plans are checked against the schema and against this hardware before any step; errors point to a JSON path. | M | 2 |
| AD01-F140 | Post-install hooks | A plan may name signed ForgeScript hooks run in the target with declared capabilities; unsigned hooks are refused. | C | 3 |
| AD01-F141 | Result file | Machine-readable result and log written to the plan's source medium when writable. | S | 2 |
| AD01-F142 | Per-machine recovery keys | The recovery key is generated per machine and written to an escrow file encrypted to a public key named in the plan. | S | 3 |

### 4.13 Repair mode

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD01-F143 | Find installed systems | Scan for pools (`zpool import` listing), GELI providers and ESPs; show each system found with version and last boot. | M | 2 |
| AD01-F144 | Unlock | Passphrase, recovery key, or GELI header restore from a backup file. | M | 2 |
| AD01-F145 | Boot environment rollback | List BEs (reference: `hbsd/src/sbin/bectl`, `hbsd/src/lib/libbe`), activate an older one, or create a BE from a snapshot. | M | 2 |
| AD01-F146 | Reinstall boot loader | Rewrite ESP files and firmware entry; re-sign where local keys are used. | M | 2 |
| AD01-F147 | Re-seal TPM | Re-enrol the TPM after a firmware change, using the recovery key. | M | 2 |
| AD01-F148 | Pool check | Import read-only, show status, start a scrub, clear errors, replace a failed disk (uses the AD02 engine). | S | 2 |
| AD01-F149 | Reset password | Reset a user's login password after unlocking the disk; logged to the target's audit trail. | S | 2 |
| AD01-F150 | Reset capability grants | Clear all remembered grants or apply a profile, for a system locked out by its own policy. | S | 3 |
| AD01-F151 | Verify system files | Re-hash base and bundles against signed manifests; list changed files; restore from the media repository. | M | 2 |
| AD01-F152 | Copy data out | Mount datasets read-only and copy to another disk through SY02 in the live session. | S | 2 |
| AD01-F153 | Chroot terminal | SY03 opened inside the target with a clear banner. | S | 2 |
| AD01-F154 | Collect logs | Bundle boot logs and crash reports from the target (scrubbed by the AD04 rules) to removable media. | S | 3 |

### 4.14 First run

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AD01-F155 | Starts once per user | Runs at first login of each new user; can be reopened from Help. State in `svc.settings`. | M | 1 |
| AD01-F156 | Privacy choices | Each on its own row, all default to the private value: location off, crash reports stay local, search indexing scope, recent-files history, clipboard history, network time, update check frequency, MAC randomisation, per-app network prompts. | M | 1 |
| AD01-F157 | Nothing leaves statement | A fixed line with a "show me" link to the live connection list in SE02. | M | 1 |
| AD01-F158 | Theme | Dark, light, high contrast; accent colour; density; text size; wallpaper from the bundled set. Live preview. | M | 1 |
| AD01-F159 | Accounts (optional) | Add mail, calendar, contacts, chat endpoints into `svc.accounts`; a "Skip" primary button. No vendor sign-in. | S | 2 |
| AD01-F160 | Backup prompt | One page linking to AD03 with two presets (local snapshots on, external disk later). | S | 2 |
| AD01-F161 | Recovery key check | If the recovery key was skipped at install, ask again here. | M | 2 |
| AD01-F162 | Tour | Six short cards: launcher and search, permissions prompts, the network badge, snapshots and undo, boot environments, where help lives. Skippable; static images, no video. | S | 2 |
| AD01-F163 | Interactive permission demo | A harmless sample prompt showing once / session / always so the first real prompt is not a surprise. | C | 3 |
| AD01-F164 | Import sources | Another OS on an internal disk (read-only mount), an external disk, a migration archive, or another PBSD machine's backup (hands to AD03). | S | 2 |
| AD01-F165 | Import: files | Documents, pictures, music, videos, desktop; shows sizes; copies with hashes and a report; keeps timestamps; never executes anything. | S | 2 |
| AD01-F166 | Import: browser data | Bookmarks and (optional) history from common browser profile formats, handed to IN01. Saved passwords go only to SE04 and only when the source store can be read without the other OS's login secret. | S | 3 |
| AD01-F167 | Import: mail and contacts | mbox, Maildir, vCard, iCalendar handed to CO01, CO03, CO02 through `import` intents. | C | 3 |
| AD01-F168 | Import: keys | SSH, OpenPGP and age keys handed to SE04 with a per-key confirmation. | S | 3 |
| AD01-F169 | Import: Wi-Fi and settings | Not imported. The page says so. | M | 2 |
| AD01-F170 | Import sandbox | Foreign filesystems are mounted read-only, `noexec`, in a helper with no network; foreign file names are sanitised. | M | 2 |
| AD01-F171 | Quarantine flag | Imported files carry a provenance attribute ("imported from <volume>") that SY02 shows and that offers "open in a disposable sandbox". | S | 3 |
| AD01-F172 | Keyboard-only completion | Every first-run page completes with `Enter`; `Esc` asks to finish later. | M | 1 |
| AD01-F173 | Managed first run | A plan file can preset or hide first-run pages for fleet installs. | C | 3 |

## 5. GUI

Layout pattern: Wizard (DESIGN_SYSTEM.md section 4). One window, full screen on live media, 960×640 minimum. Step rail on the left, page in the middle, context help on the right, navigation bar at the bottom.

### 5.1 Window map

```
Installer window
├─ Step rail (all pages, state per page)
├─ Pages I1–I19 (install), R1–R4 (repair), U1 (unattended)
│   I1 Welcome and language        I8  Layout              I15 Hardening profile
│   I2 Keyboard and accessibility  I9  Datasets and mounts I16 Packages
│   I3 What to do (mode)           I10 Encryption          I17 Summary
│   I4 Hardware check              I11 Boot chain and TPM  I18 Progress
│   I5 Network (optional)          I12 Recovery key        I19 Finished
│   I6 Region and time             I13 Users
│   I7 Disks                       I14 Host name and services
├─ Dialogs: Manual partition editor · Vdev editor · Dataset properties · Wi-Fi join · Static IP
│           Profile detail · Custom package tree · Erase confirmation · Skip recovery key
│           Unlock existing volume · Save plan · Load plan · Cancel install · Error detail
├─ Sheets: Device inventory · Compatibility report · Install log · Plan diff
└─ Popovers: Passphrase generator · Strength detail · Topology diagram · First-contact list
First-run window (installed system)
├─ Pages W1 Welcome · W2 Privacy · W3 Appearance · W4 Accounts · W5 Import · W6 Safety net · W7 Tour · W8 Done
└─ Dialogs: Import source picker · Import selection · Import report · Finish later
```

### 5.2 Main window

Shown at 1280×800 on page I7 (Disks). States: empty (no disk found), loading (probing), error (probe failed), offline (always, unless I5 enabled), locked (existing encrypted volume), read-only (unattended run showing progress).

```
+----------------------------------------------------------------------------------------+
| [=] Install ParanoidBSD                 (1)[media ✓ verified] (2)[offline] [a11y] [✕]  |
+-------------------+--------------------------------------------------+-----------------+
| (3) STEPS         | (4) Where should ParanoidBSD go?                 | (7) HELP        |
|  ✓ Language       |                                                  | Erasing a disk  |
|  ✓ Keyboard       | ( ) Erase disks and install                      | removes all     |
|  ✓ What to do     | ( ) Install beside another system                | data on it.     |
|  ✓ Hardware       | ( ) Replace ParanoidBSD, keep home               | Nothing is      |
|  – Network (off)  | ( ) Manual layout…                               | written until   |
|  ✓ Region         |                                                  | you confirm on  |
| > Disks           | (5) +---+-----------------+--------+-----+------+| the Summary     |
|   Layout          |     | ✓ | Disk            | Size   | Bus |Health|| page.           |
|   Datasets        |     +---+-----------------+--------+-----+------+|                 |
|   Encryption      |     |[x]| nda0 Samsung 990| 1.0 TB |NVMe | ✓ OK ||                 |
|   Boot chain      |     |[ ]| ada0 WDC Red    | 4.0 TB |SATA | ✓ OK || [Learn more F1] |
|   Recovery key    |     |[ ]| ada1 WDC Red    | 4.0 TB |SATA | ! 12 ||                 |
|   Users           |     | – | da0  install USB| 16 GB  |USB  | media||                 |
|   Host            |     +---+-----------------+--------+-----+------+|                 |
|   Hardening       | (6) nda0 now: [ESP 260M][MSR][ Windows NTFS 930G    ][Rec 1G]      |
|   Packages        |     Detected: Windows 11 (BitLocker off)         |                 |
|   Summary         |     [Identify disk]  [Rescan F5]  [Unlock…]      |                 |
+-------------------+--------------------------------------------------+-----------------+
| (8) [Help F1] [Log]                        nothing written yet   [< Back]  (9)[Next >] |
+----------------------------------------------------------------------------------------+
```

1. Media trust badge: result of the media self-check; click opens the hash report.
2. Network badge: `offline` until page I5 enables it; then `[net]` in `--net`, click lists first contacts.
3. Step rail, 200 px, `listView`: done ✓, current >, skipped –, error !. Completed steps are clickable.
4. Page title (18 px) and install type `radioButton` group.
5. Disk table, `tableView`, multi-select by checkbox column; health cell shows a word and icon; `!` rows open the SMART summary.
6. Current-content bar for the focused disk (custom draw in `panel`), with detected systems as text.
7. Help pane, 240 px, `markdownView`, content keyed to the focused control.
8. Navigation bar: help, log sheet, a fixed status phrase that states whether anything has been written.
9. Primary button. Disabled with a tooltip reason until the page validates.

### 5.3 Other views and dialogs

Each page below is drawn at 720×420 (the centre region). All share the step rail, help pane and navigation bar from 5.2.

**I1 Welcome and language**
```
| Welcome                                                             |
| [ search language…        ]     (1) Media check: ✓ 412 files OK     |
| > English (Australia)           (2) [Install] [Try it] [Repair]     |
|   English (United Kingdom)          [Check this computer]           |
|   Deutsch · Français · 日本語 …  (3) Accessibility: [Reader][Zoom]  |
|                                     [Keyboard][Contrast][Large text]|
```
(1) trust badge; (2) mode shortcuts (also on I3); (3) accessibility bar, repeated in the header on every page.

**I2 Keyboard and accessibility**
```
| Keyboard                                                            |
| Layout  [English (US)        v]   Variant [Default        v]        |
| [+ Add layout]  Switch with [Super+Space]                           |
| Test: [______________________________]   (1) [key position diagram] |
| [ ] Screen reader  [ ] On-screen keyboard  [ ] High contrast        |
| [ ] Large text     [ ] Reduced motion      Zoom [100% v]            |
```
(1) drawn keyboard that lights the pressed key.

**I3 What to do**
```
| What would you like to do?                                          |
| (•) Install ParanoidBSD on this computer                            |
| ( ) Try it without changing anything                                |
| ( ) Repair an installed system                                      |
| ( ) Install from a plan file…   [Choose file]  sig: ✓ signed by Bo  |
```

**I4 Hardware check**
```
| Is this computer ready?                    [Run again] [Save report]|
| ✓ CPU: 64-bit, AES, RDRAND, SMEP, SMAP                              |
| ✓ Memory: 16 GiB            ✓ Firmware: UEFI                        |
| ! Secure Boot: off — verified boot will be weaker   [How to fix]    |
| ✓ TPM 2.0 present           ✓ Disks: 3, health OK (1 warning)       |
| ✕ Wi-Fi BCM4360: no driver — wired network works     [Devices 42 >] |
```
`listView` with icon, word and reason per row; "Devices" opens the inventory sheet (`treeView`).

**I5 Network (optional)**
```
| Network — not needed to install            State: OFF               |
| [ Stay offline ]  (primary)      [ Turn network on ]                |
| Use the network for:  [ ] newer packages  [ ] languages             |
|                       [ ] firmware        [ ] time                  |
| Wired em0: cable connected     Wi-Fi: [scan list            v]      |
| Route (•) clear ( ) VPN file ( ) Tor      Contacted hosts: none     |
```

**I6 Region and time**
```
| Region and time                                                     |
| Region [Australia v]  Time zone [Australia/Sydney v]  (map)         |
| Formats: date 12/11/2026 · number 1,234.5 · paper A4 · metric       |
| Hardware clock (•) UTC  ( ) local time (for dual boot with Windows) |
| Time [12:04] Date [2026-11-12]   [ ] Set from network (off)         |
```

**I8 Layout**
```
| How should the disks be arranged?        Preset [Workstation v]     |
| Pool name [zroot    ]   Type (•) Mirror ( ) Stripe ( ) RAID-Z1      |
| Usable 4.0 TB · survives 1 disk failure                             |
| (1) zroot ─ mirror-0 ─┬ ada0p3.eli                                  |
|                       └ ada1p3.eli        ESP on both · swap 8G ×2  |
| Swap [8 GiB v] [x] encrypted   Compression [zstd v]  [Advanced…]    |
| (2) [ESP][swap][        ZFS (encrypted)                    ]        |
```
(1) topology diagram (`nodeGraph`, read-only); (2) partition bar per disk. "Advanced…" opens the manual partition and vdev editors.

**I9 Datasets and mounts**
```
| Datasets                                         [+ Add] [Reset]    |
| Dataset            | Mount     | exec | setuid | Quota | Encrypt    |
| zroot/ROOT/default | /         | on   | on     | –     | inherit    |
| zroot/home         | /home     | off  | off    | –     | inherit    |
| zroot/home/<user>  | /home/…   | off  | off    | –     | own key    |
| zroot/var/log      | /var/log  | off  | off    | 8G    | inherit    |
| zroot/tmp          | /tmp      | off  | off    | 4G    | inherit    |
```
`treeView` with columns; cells edit in place; a changed cell shows a dot and can be reverted.

**I10 Encryption**
```
| Encrypt this computer                         State: ON (default)   |
| (•) Whole disk (GELI)  ( ) Per dataset (ZFS)  ( ) Both  [Compare]   |
| Passphrase [••••••••••••••••••] (hold to show)  [Generate…]         |
| Again      [••••••••••••••••••]   Strength: ████░ good · layout US  |
| [x] Encrypt swap   [x] Own key per user home   [ ] Wipe disks first |
| [ ] Do not encrypt (not recommended)…        [Advanced: KDF, cipher]|
```
Secret fields follow DESIGN_SYSTEM.md section 5. "Compare" opens the three-mode table.

**I11 Boot chain and TPM**
```
| Start-up protection                                                 |
| Firmware      ✓ UEFI          Secure Boot  ! off  [Use PBSD key v]  |
| Loader        signed by PBSD release key                            |
| Kernel+modules verified by loader · module loading locked after boot|
| System files  veriexec manifest: [enforce v]                        |
| Unlock with   (•) TPM + PIN ( ) TPM only ( ) Passphrase only        |
| PIN [••••••] Again [••••••]   [What will need the recovery key?]    |
```

**I12 Recovery key**
```
| Your recovery key — keep it away from this computer                 |
|   K7Q2M4  9XWT3A  B5HHPL  2C8DNE                                    |
|   R6VJ0F  M3YQ7S  T1KZ4G  W9PB2U                                    |
| [Show large] [Print sheet] [Save to USB…] [Show QR]                 |
| To continue, type group 3 [______] and group 6 [______]             |
| [Skip — I accept losing my data if I forget the passphrase…]        |
```

**I13 Users**
```
| Who will use this computer?                                         |
| Full name [Asha Rao        ]  Login [asha     ]  (picture)          |
| Password  [••••••••••]  Again [••••••••••]  Strength: ███░░         |
| (•) Asha can administer this computer                               |
| ( ) Create a separate administrator account  [admin    ] [••••••]   |
| [+ Add another user]        [ ] Log in automatically (not advised)  |
```

**I14 Host name and services**
```
| Name and services                                                   |
| Computer name [pbsd-7f3a      ] (random; change if you like)        |
| Listening services: none.   [ ] SSH server (keys only)              |
| Firewall: inbound denied · outbound asks per app                    |
```

**I15 Hardening profile**
```
| How strict?                                                         |
| (•) Balanced — recommended. Everything on that rarely breaks apps.  |
| ( ) Strict — every protection on; more prompts; some apps need help.|
| Setting                       | Balanced | Strict  | May affect     |
| W^X for all processes         | on       | on      | –              |
| Module loading after boot     | allowed  | blocked | late drivers   |
| Remembered app grants         | allowed  | session | convenience    |
| … 61 more                                   [Show all] [Overrides 0]|
```

**I16 Packages**
```
| What to install                     Source: this media (offline)    |
| ( ) Minimal 2.1G  (•) Standard 9.4G  ( ) Full 21G  ( ) Custom…      |
| [x] Firmware for detected devices (3)   [ ] Extra languages         |
| [ ] Local AI models (needs 8 GiB RAM)                               |
| Space: [system 9.4][swap 8][reserve 50][      free 930 GB        ]  |
```

**I17 Summary**
```
| Ready to install — nothing has been written yet     [Save as plan…] |
| Disks      ERASE nda0 Samsung 990 1.0 TB (…4F2K)          [Change]  |
| Layout     zroot, single disk, zstd, swap 8G encrypted    [Change]  |
| Encryption GELI AES-XTS-256, TPM + PIN, recovery key ✓    [Change]  |
| Users      asha (admin)   Host pbsd-7f3a                  [Change]  |
| Profile    Balanced, 0 overrides   Packages Standard      [Change]  |
| About 14 minutes.                      [ Erase 1 disk and install ] |
```

**I18 Progress**
```
| Installing                                           step 5 of 8    |
| ✓ Partition  ✓ Encrypt  ✓ Create pool  ✓ Datasets                   |
| > Copy system   ████████████░░░░░░░  61%   212 MB/s   4 min left    |
|   Configure · Boot chain · Verify                                   |
| [Details ▾]  log: extracting org.pbsd.files-0.1.0 …                 |
|                                                         [Cancel…]   |
```

**I19 Finished** — result badge, verification summary, [Restart] (primary), [Shut down], [Keep using live session], [Save report…].

**R1 Repair** (R2 unlock, R3 action, R4 result follow the same frame)
```
| Repair                                                              |
| Found: zroot on nda0 (encrypted) — ParanoidBSD 0.1, last boot 3 Nov |
| [Unlock…] then:                                                     |
|  [Roll back to an earlier boot environment]  [Reinstall boot loader]|
|  [Re-seal TPM]  [Verify system files]  [Check pool]  [Copy data out]|
|  [Reset a password]  [Open terminal in the system]                  |
```

**Manual partition editor (dialog, 900×560)**
```
| Disk [nda0 1.0 TB v]   Scheme GPT           [Undo] [Revert all]     |
| [ESP 512M][swap 8G][  zfs 921G                            ][free]   |
| # | Type        | Label   | Start   | Size   | Use                  |
| 1 | efi         | efiboot | 1 MiB   | 512M   | ESP                  |
| 2 | freebsd-swap| swap0   | 513 MiB | 8G     | swap (encrypted)     |
| 3 | freebsd-zfs | zfs0    | 8.5 GiB | 921G   | zroot                |
| [+ New] [Delete] [Resize…]     Pending: 3 changes   [Cancel] [Use]  |
```

**First-run W2 Privacy (720×480)**
```
| Privacy — everything starts private                       2 of 8    |
| Location services                              [ off ]              |
| Crash reports                                  stay on this computer|
| Search index covers                            [My files only   v]  |
| Recent files and clipboard history             [ on ] kept local    |
| Check for updates                              [Weekly v] via [clear v]|
| Nothing leaves this computer unless you allow it. [Show connections]|
|                                          [Skip tour] [Back] [Next]  |
```

**First-run W5 Import**
```
| Bring your things across (optional)                       5 of 8    |
| Source (•) Windows on nda0p3  ( ) External disk  ( ) Archive…       |
| [x] Documents 12 GB  [x] Pictures 48 GB  [ ] Music  [ ] Videos      |
| [x] Browser bookmarks → Web Browser   [ ] Keys → Passwords and Keys |
| Not imported: programs, Wi-Fi passwords, system settings.           |
| Mounted read-only, no programs are run.       [Skip] [Import 60 GB] |
```

W1 Welcome (name, one sentence, [Start]); W3 Appearance (three theme cards, accent swatches, density, text size, wallpaper `gridView`); W4 Accounts (list of endpoint types with [Add…], primary [Skip]); W6 Safety net (recovery key check, local snapshots toggle, [Open Backup]); W7 Tour (six cards in a `tabWidget` with dots); W8 Done (links to Permissions, Software, Help).

### 5.4 Menus

The wizard has no menu bar; `[=]` and `F10` open one menu.

| Item | Shortcut |
|---|---|
| Help for this page | `F1` |
| Show install log | `Ctrl+L` |
| Check this computer… | `Ctrl+H` |
| Load plan… / Save as plan… | `Ctrl+O` / `Ctrl+S` |
| Open terminal | `Ctrl+Alt+T` |
| Network: status and contacted hosts | `Ctrl+Shift+N` |
| Accessibility ▸ Screen reader / Magnifier / On-screen keyboard / High contrast / Large text | `Super+Alt+S` / `Super+Alt+Z` / `Super+Alt+K` / `Super+Alt+C` / `Ctrl++` |
| Keyboard layout ▸ (list) | `Super+Space` |
| Language ▸ (list) | — |
| About and licences | — |
| Restart / Shut down | — |
| Quit installer (asks if a plan is running) | `Ctrl+Q` |

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| `Enter` / `Alt+N` | Next (when valid) |
| `Alt+B` / `Alt+Left` | Back |
| `Ctrl+1` … `Ctrl+9` | Jump to a completed step |
| `F5` | Rescan disks or networks |
| `F6` | Cycle step rail, page, help pane |
| `Space` | Toggle the focused row or option |
| `Ctrl+D` | Toggle Details on the progress page |

Context menus: disk row (Identify, Show health, Show partitions, Unlock…, Exclude); dataset row (Add child, Properties…, Reset, Remove); partition row (Resize…, Delete, Set type, Set label). Drag and drop: disks between vdevs in the vdev editor; partition edges to resize. No file drops are accepted except a plan file on page I3.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | No disk found: page I7 shows "No disk found", [Rescan], [Check this computer], [Open terminal]. |
| Loading | Probes show a determinate bar where counts are known; pages stay navigable backwards. |
| Error | A failed step keeps the progress page, shows the step in `--danger`, [Details], [Retry step], [Cancel and restore]. |
| Offline | Normal state. No warning. Pages that would use the network say what is skipped. |
| Locked | Existing encrypted volumes show a lock and [Unlock…]; pages that need them are disabled until unlocked. |
| Read-only | Unattended runs show pages read-only with a countdown bar and [Take over]. |
| Narrow (< 640) | Step rail collapses to "Step 7 of 19" with a dropdown; help pane becomes an `F1` sheet. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Step rail | `listView` |
| Option groups | `radioButton`, `checkBox`, `toggleSwitch` |
| Disk, partition, dataset, profile tables | `tableView`, `treeView` |
| Language and package lists | `modelListView`, `treeView` with `lineEdit` filter |
| Topology diagram | `nodeGraph` (read-only use) |
| Partition bar, space forecast, key-position diagram | `panel` with custom draw (needs `Painter`, FUSE-WP-23) |
| Help pane, licences, tour text | `markdownView` |
| Progress | `progressBar`, `listView`, `codeEdit` read-only for the log |
| Time and date | `dateTimeEdit`, `calendar` |
| Dialogs and sheets | `dialog`, `scrollArea` |

Missing: `AD01-GAP-1` a wizard step-rail widget with validation state (can be composed from `listView`); `AD01-GAP-2` a secret field with reveal-on-hold and capture exclusion (DESIGN_SYSTEM.md section 5 names the pattern; no widget in V3); `AD01-GAP-3` a QR code draw routine (own code, about 400 lines). The live session needs G-01 or the X11 backend (V5).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Wizard logic (`main.fs`, `src/pages/*.fs`) | ForgeScript | app host | Page flow, validation, view models, plan assembly |
| Wizard UI (`ui/*.fuseui`, `style/*.css`) | FUSE UI | app host | All pages and dialogs |
| Plan engine (`src/plan.fs`) | ForgeScript | app host | Turns choices into an ordered, typed step list; computes diffs and space |
| Install service (`svc.install`, Proposed) | C++23 | privileged daemon on live media only | Executes confirmed steps; owns the journal; the only process that writes to target disks |
| Storage broker (`svc.storage`, Proposed, shared with AD02) | C++23 | privileged daemon | GEOM, GPT, GELI, ZFS, SMART operations behind a typed interface |
| Boot chain helper | C++23 | child of install service, capability mode | ESP files, firmware variables, signing, TPM seal, veriexec manifest |
| Probe helper | C++23 | sandbox, read-only device handles | Hardware inventory, foreign filesystem labels, existing-system detection |
| Import helper | C++23 | sandbox, read-only source handle, write handle to the user's home | Reads foreign filesystems and profile formats during first run |
| Console front end | ForgeScript | app host without UI context | Text-mode run of the same plan engine |
| First-run logic and UI | ForgeScript, FUSE UI | app host in the installed system | Welcome pages; writes through `svc.settings` |

```
 wizard (ForgeScript + FUSE UI, no disk access)
    │ plan (typed records over the session bus)
    ▼
 svc.install ──► svc.storage ──► GEOM / GELI / ZFS (kernel)
    │   │              ▲
    │   └─► boot chain helper ─► ESP, EFI variables, TPM
    │                  │
    └─► package extractor (signature check, then write)      probe helper ─► read-only handles
```

The wizard never holds a disk handle. It sends a plan; the service validates the plan again against its own schema and against the confirmation token issued on the summary page.

### 6.2 Data model

| Entity | Fields |
|---|---|
| `InstallPlan` | `schema: u32`, `locale: LocaleChoice`, `keyboard: KeyboardChoice[]`, `network: NetworkChoice?`, `mode: enum{erase,beside,replace,manual,add_be}`, `disks: DiskSelector[]`, `layout: PoolLayout`, `datasets: DatasetSpec[]`, `encryption: EncryptionSpec`, `boot: BootChainSpec`, `recovery: RecoverySpec`, `users: UserSpec[]`, `host: HostSpec`, `profile: ProfileSpec`, `packages: PackageSelection`, `confirm: bool`, `hooks: HookRef[]` |
| `DiskSelector` | `by: enum{name,serial,model,size_range,bus}`, `value: string`, `expect_count: u8` |
| `PoolLayout` | `pool_name: string`, `be_name: string`, `vdevs: VdevSpec[]`, `ashift: u8`, `properties: map<string,string>`, `swap_bytes: u64`, `esp_bytes: u64`, `reserve_percent: u8` |
| `VdevSpec` | `kind: enum{stripe,mirror,raidz1,raidz2,raidz3,log,cache,special,spare}`, `members: PartitionRef[]` |
| `DatasetSpec` | `name: string`, `mountpoint: string`, `exec: bool`, `setuid: bool`, `devices: bool`, `quota: u64?`, `reservation: u64?`, `encryption: enum{inherit,own_key,off}`, `properties: map<string,string>` |
| `EncryptionSpec` | `mode: enum{geli,zfs_native,both,none}`, `cipher: string`, `key_bits: u16`, `kdf_iterations: u64`, `secret_ref: SecretRef`, `encrypt_swap: bool`, `wipe: enum{none,trim,secure_erase,random}` |
| `BootChainSpec` | `secure_boot: enum{off,pbsd_key,own_keys}`, `veriexec: enum{off,audit,enforce}`, `measured: bool`, `tpm: enum{none,tpm_pin,tpm_only}`, `pcrs: u8[]`, `kmod_lock: bool` |
| `RecoverySpec` | `generate: bool`, `split_k: u8`, `split_n: u8`, `escrow_pubkey: string?`, `acknowledged: bool` |
| `UserSpec` | `login: string`, `full_name: string`, `admin: bool`, `password: SecretRef`, `home_quota: u64?`, `own_home_key: bool` |
| `ProfileSpec` | `base: enum{balanced,strict}`, `overrides: map<string,string>` |
| `SecretRef` | `kind: enum{prompt,wrapped,hash}`, `data: bytes?` — never plain text |
| `Step` | `id: u32`, `kind: string`, `args: record`, `state: enum{pending,running,done,failed,rolled_back}`, `undo: Step?`, `started_ns: i64`, `ended_ns: i64` |
| `InstallReport` | `plan_hash: bytes32`, `media_hash: bytes32`, `steps: Step[]`, `verification: VerifyResult`, `first_contacts: string[]` |

Stores: wizard state in tmpfs (`/tmp/installer/state.json`, no secrets); the journal in tmpfs and, after pool creation, also in `var/log/installer/journal.jsonl` on the target; first-run state under `svc.settings` namespace `org.pbsd.installer.firstrun`. Schema version in every file; an installer never reads a plan with a newer major version.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| `install-plan.json` + `.sig` | yes | yes | FORGE JSON and signing (V14) | app host (signature first), again in `svc.install` |
| GPT, MBR (detect only) | yes | GPT | kernel `geom_part` through `svc.storage` | kernel / broker |
| FAT32 (ESP) | yes | yes | base `newfs_msdos`, `msdosfs` | broker |
| ZFS pool and datasets | yes | yes | OpenZFS in tree (`hbsd/src/sys/contrib/openzfs`) via `libzfs_core` | broker |
| GELI metadata | yes | yes | `hbsd/src/lib/geom/eli` | broker |
| Package archives and repository manifest | yes | no | the SY05 package library | package extractor (capability mode) |
| NTFS, ext4, btrfs, APFS, HFS+ labels | detect | no | label readers in tree for NTFS and ext2 (`pbsd/geom/pbsd.geom.label_ntfs.cppm`, `label_ext2fs`); others own code | probe helper |
| ext2/3/4 file trees (import) | yes | no | kernel `ext2fs` (in tree: `pbsd/fs/pbsd.fs.ext2_*.cppm`), read-only | kernel, mounted `ro,noexec,nosuid` |
| NTFS file trees (import) | Proposed | no | no reader in tree was found; own read-only reader, about 4 000 lines | import helper |
| Browser bookmark stores (SQLite, JSON, HTML) | yes | no | SQLite (public domain), own JSON | import helper |
| EFI variables, boot entries | yes | yes | `libefivar` in base (used by `efibootmgr`) | boot chain helper |
| TPM 2.0 commands | yes | yes | Proposed own minimal command encoder over `/dev/tpm0`; no TPM userland tools found in base | boot chain helper |
| SMART / NVMe health log | yes | no | CAM and NVMe pass-through (reference: `hbsd/src/sbin/camcontrol`, `hbsd/src/sbin/nvmecontrol`) | broker |
| QR code, PDF recovery sheet | no | yes | own code; PDF through the G-16 pipeline | app host |

### 6.4 Integration

- Intents handled: `open` for `application/vnd.pbsd.install-plan+json`. Emitted: `import` hand-offs to IN01, CO01, CO02, CO03, SE04; `open` to AD03, AD05, SY05, UT05.
- Services used: `svc.settings`, `svc.notify`, `svc.power`, `svc.keyring` (first run only), `svc.accounts`, `svc.portal.file`, `svc.portal.print`, `svc.audit`, `svc.update` (boot environments), plus Proposed `svc.install` and `svc.storage`.
- AD02 shares the storage broker and the partition and vdev editor components. SY05 owns package verification code. SE04 receives the recovery-key rotation flow and any generated Secure Boot keys. SE01 reads the install report (encryption state, profile, skipped recovery key). SH03 uses the user records and the per-user home keys.
- Verified from the repo: `bsdinstall` script set and its defaults (`hbsd/src/usr.sbin/bsdinstall/scripts/{zfsboot,hardening,bootconfig,adduser,keymap,netconfig}`), the partition editor it uses (`hbsd/src/usr.sbin/bsdinstall/partedit/`), `bectl` and `libbe`. Everything about the graphical installer, `svc.install`, plan files, TPM sealing and Secure Boot enrolment is Proposed.

## 7. Security and privacy

Capabilities requested:

| Capability | Reason |
|---|---|
| `fs.app` | Wizard state, first-run state |
| `fs.portal` | Save recovery sheet, report and plan to removable media; choose a plan file |
| `net.none` | Default. The wizard itself never opens a socket |
| `svc.install` (Proposed) | Submit a confirmed plan; live media only |
| `svc.storage` (Proposed) | Read-only probing before commit; writes only by `svc.install` |
| `svc.settings`, `svc.notify`, `svc.power` | First-run choices; progress; sleep inhibit |
| `svc.keyring`, `svc.accounts` | First run: store carried Wi-Fi secret; optional accounts |
| `svc.portal.print`, `dev.print` | Print the recovery sheet |
| `svc.audit` | Record the install and first-run decisions |
| `svc.update` | Create and list boot environments |
| `proc.helper:probe-helper`, `proc.helper:import-helper` | Sandboxed parsing of foreign disks and profiles |
| `dev.usb:storage` | See removable disks as install targets and import sources |

Network fetches (packages, time) are made by `svc.update` under its own capability, not by the wizard.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| The installed system's integrity | Tampered or counterfeit media | Modified packages or installer | Media self-check against a signed manifest; signature check of every package before extraction; the release key fingerprint printed on the welcome page |
| Disk passphrase, recovery key, TPM PIN | Malware in the live session, shoulder surfer, screen capture | Read the secret from memory, logs or screen | Secrets held only in `svc.install` in locked, zeroed memory; never logged or in plans; capture exclusion; no clipboard; reveal on hold |
| Existing data on disks | Wrong disk chosen; hostile plan file | Erase the wrong disk | No-write guarantee before commit; disk named by model and serial tail; unsigned plans cannot skip confirmation; disk matching rules must match exactly |
| Boot chain | Evil-maid attacker after install | Replace loader or kernel | Secure Boot + signed loader, veriexec, TPM sealing to PCRs, boot chain self-test |
| The probe and import helpers | Malicious filesystem or profile on the old disk | Parser exploit | Helpers in capability mode with read-only handles and no network; kernel mounts `ro,noexec,nosuid`; fuzzed parsers |
| Privacy of the person | Network observer | Learn that a machine is installing PBSD, its MAC or model | Offline default; MAC randomisation; first-contact list; no identifiers in reports by default |
| Fleet plan secrets | Someone who finds the USB stick | Read passwords from the plan | Only wrapped or hashed secrets; per-machine recovery keys encrypted to an escrow key |

Stored: install report and log on the target (`/var/log/installer/`, admin-readable); first-run choices in `svc.settings`. Leaves the machine: nothing by default; with the network enabled, only requests to the shown mirror and time source.

## 8. Settings

The installer has no preferences page. These keys exist in plans and in `svc.settings` for first run.

| Key | Type | Default | Description |
|---|---|---|---|
| `installer.network.enabled` | bool | `false` | Whether the live session may use the network |
| `installer.network.uses` | set{packages,languages,firmware,time} | `{}` | What the network may be used for |
| `installer.encryption.mode` | enum | `geli` | `geli`, `zfs_native`, `both`, `none` |
| `installer.encryption.kdf_target_ms` | u32 | `2000` | Unlock time target for key derivation |
| `installer.pool.compression` | enum | `zstd` | Pool compression |
| `installer.pool.reserve_percent` | u8 | `5` | Reserved space |
| `installer.swap.rule` | enum | `auto` | `auto`, `none`, fixed size |
| `installer.boot.veriexec` | enum | profile | `off`, `audit`, `enforce` |
| `installer.boot.tpm` | enum | `tpm_pin` if present | TPM unlock mode |
| `installer.profile.base` | enum | `balanced` | Hardening profile |
| `installer.packages.set` | enum | `standard` | Package set |
| `installer.report.include_identifiers` | bool | `false` | Serial numbers in saved reports |
| `firstrun.completed` | bool | `false` | Per user |
| `firstrun.tour.seen` | bool | `false` | Per user |
| `firstrun.import.last_report` | path | — | Location of the last import report |

## 9. Accessibility and languages

- The whole wizard is usable with the screen reader from the first page; each page has one heading, a live region for validation messages, and progress announced every 10 % or 30 s.
- The recovery key is read group by group with a phonetic option; the confirmation fields accept it without the check characters being spoken twice.
- No colour-only states on the hardware check: icon, word and reason.
- Passphrase pages announce the active layout and caps lock.
- Right-to-left layouts mirror the step rail and navigation order. The live media carries FULL text shaping (G-03) or it cannot show many languages on page I1.
- Time-limited elements: none, except the unattended countdown, which can be paused.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Media self-check | Under 60 s for a 4 GiB image on USB 3 |
| First page after session start | Under 2 s |
| Page change | Under 50 ms; no page blocks on a probe |
| Disk probe | Under 3 s for 16 disks |
| Standard install to NVMe | Under 15 min from confirmation to finish page |
| Wizard memory | Under 150 MiB; live session usable in 2 GiB |
| First-run start | Under 400 ms |
| Import copy | At least 80 % of the slower disk's sequential rate |

## 11. Testing and proof

Test plan (nothing has run):

- Unit: plan engine (choices → steps), space forecast, disk matching rules, validation, recovery-key encoding and check characters, Shamir split and join.
- Golden renders: every page in 5.2 and 5.3 in dark, light, high contrast, at 100 % and 200 %, left-to-right and right-to-left.
- Scripted UI through the a11y tree: tasks T1–T13 against a virtual machine with virtual disks (VM01 / bhyve), including power cut at each step boundary and retry.
- Matrix: 1–6 disks × each topology × each encryption mode × TPM present or absent × Secure Boot on or off; each run must boot and unlock.
- Fuzz: plan parser, foreign label readers, NTFS reader, browser profile readers, package manifest reader.
- Differential: partition tables and pool layout against what `bsdinstall`'s `zfsboot` script produces for the same choices.
- Negative: tampered media, tampered plan, wrong recovery key, changed PCR, full disk, failing disk mid-install.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Plan engine | A plan never contains a write step for a disk not in `disks` | T1 | Lean model of plan generation; PRISM refinement | ForgeScript has no PRISM front end yet |
| `svc.install` commit gate | No write before a valid confirmation token | T2 | PRISM contract on the request handler | Service does not exist |
| Recovery key codec | Encode/decode round trip; check character detects any single error | T1 | Proof over the base-32 and check function | — |
| Shamir split | Any k shares rebuild; k−1 reveal nothing | T1 (algebra), T3 (code) | Lean proof of the scheme; tests on the code | Constant-time field code |
| Plan and label parsers | No runtime error on any input | T2 | PRISM absence-of-runtime-error + fuzz | Helpers not written |
| Boot chain helper | Signs and seals only what the plan names | T3 | Safe subset, sanitizers, VM tests with a software TPM | Firmware behaviour is outside the proof |
| Wizard UI | Next is disabled while a page is invalid | T3 | Scripted UI tests | UI layer is not in proof scope |
| Import helper | Never writes outside the granted destination handle | T2 | Capability mode + PRISM on path handling | Helper not written |

## 12. Build plan

Phase 1 — usable core (single-OS install, offline, GELI, no TPM):

| ID | Work item | Depends on |
|---|---|---|
| AD01-W01 | Live image with compositor, app host and the wizard as the session | G-01 or X11 backend, G-07, G-18, FUSE-WP-01, FORGE-WP-10 |
| AD01-W02 | `svc.storage` read-only probing (disks, partitions, pools, health) | AD02-W02 (shared broker), G-08 |
| AD01-W03 | Plan engine, plan schema, validation | G-02, FORGE-WP-26 |
| AD01-W04 | Pages I1–I3, I6, I7, I8 (presets only), I10 (GELI), I12, I13, I14, I15, I16 (sets), I17–I19 | AD01-W03, FUSE-WP-12, FUSE-WP-29 |
| AD01-W05 | `svc.install`: journalled steps for partition, GELI, pool, datasets, extract, configure, loader | AD01-W02, SY05 package library |
| AD01-W06 | Media self-check and package signature check | SY05, FORGE-WP-09 |
| AD01-W07 | Recovery key generation, sheet, confirmation | AD01-GAP-2, AD01-GAP-3 |
| AD01-W08 | Hardening profiles Balanced and Strict as data tables | SE01 (profile definitions), AD05 (prompt policy keys) |
| AD01-W09 | Hardware check page I4 (requirements + inventory) | VM05 probe code |
| AD01-W10 | First run W1–W3, W8 | G-09 (`svc.settings`) |
| AD01-W11 | Post-install verification and report | AD01-W05 |

Phase 2 — full feature:

| ID | Work item | Depends on |
|---|---|---|
| AD01-W12 | Network page I5 and fetch through `svc.update` | SE02 (first-contact log), G-09 |
| AD01-W13 | Manual partition, vdev and dataset editors (shared with AD02) | AD02-W05 |
| AD01-W14 | ZFS native encryption and per-user home keys | SH03 (unlock at login), loader support question |
| AD01-W15 | Signed loader, Secure Boot with the PBSD key, veriexec manifest | repo release signing (`docs/ROADMAP.md` §13–14), AD01-GAP-5 |
| AD01-W16 | Measured boot and TPM enrolment | AD01-GAP-4, SY05 (re-seal on update) |
| AD01-W17 | Dual boot: shared ESP, free-space install, firmware entries | AD01-W13 |
| AD01-W18 | Unattended install: plan file, signatures, dry run, export | AD01-W03, FORGE-WP-22 |
| AD01-W19 | Repair mode R1–R4 | AD01-W05, `svc.update` (boot environments) |
| AD01-W20 | Console front end | AD01-W03 |
| AD01-W21 | First run W4–W7 and file import from ext4 and external disks | CO01–CO03, IN01, SE04 import intents; AD03 |
| AD01-W22 | Accessibility from boot (reader hot key, announcements) | AX01, G-04 |

Phase 3 — advanced:

| ID | Work item | Depends on |
|---|---|---|
| AD01-W23 | Own Secure Boot keys, hardware token unlock | SE04, loader work |
| AD01-W24 | Split recovery key, fleet escrow, post-install hooks | AD01-W18 |
| AD01-W25 | NTFS reader and browser, mail and key import | import helper fuzzing in place |
| AD01-W26 | Known-machine notes, suspend and audio probes, memory quick test | VM05 |

## 13. Open questions and risks

1. **Boot from encrypted ZFS.** Whether the in-tree loader can read a GELI provider with verified boot on, and whether it can boot a natively encrypted root dataset, was not established here. The fallback is an unencrypted, signed boot pool as `bsdinstall` uses.
2. **Loader signing and Secure Boot do not exist in the tree** (`docs/ROADMAP.md` section 14). Verified boot features are blocked until the repo has a release signing key and a signed loader (`AD01-GAP-5`).
3. **No TPM userland in base** was found (`AD01-GAP-4`). A minimal TPM 2.0 command encoder is security-critical code that needs its own review.
4. **The storage plan may change under the installer.** `docs/plan/PBSD_PLAN.md` (P2-02) replaces GEOM with a block multiqueue layer and an adapter; the broker interface must not expose GEOM names as stable identifiers.
5. **NTFS shrink.** No safe resizer exists in the tree. Dual boot in phase 2 needs free space made in the other OS.
6. **Privileged services are not in PLATFORM.md section 8.** `svc.install` and `svc.storage` are new; how an app is authorised to use them (admin confirmation through SH03) needs a platform rule (`AD01-GAP-6`).
7. **Strict profile breakage** is unknown until the 59 hardening knobs have measured effects (`docs/ROADMAP.md` section 15).
8. **Live session display.** Without G-01 the live media depends on the X11 backend (V5), which the final system does not intend to ship.
9. **Recovery key on paper** relies on the user. The typed confirmation proves it was seen, not that it was kept.
