<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Apps

By Odin Loch

The application suite for ParanoidBSD: the desktop shell and 78 apps, planned in full. Each app has its own folder here. Today each folder holds the plan (`SPEC.md`) and a draft `manifest.json`; the app's code goes in the same folder when it is built.

**Status: plan only. Nothing in this folder has been built, run, tested or proved.**

## Start here

| File | What it is |
|---|---|
| [INDEX.md](INDEX.md) | Generated table of every app spec with line, feature and work-item counts, and the result of the consistency check |
| [00-foundation/CATALOGUE.md](00-foundation/CATALOGUE.md) | The 78 apps, their IDs, folders and priorities |
| [00-foundation/PLATFORM.md](00-foundation/PLATFORM.md) | How an app is built: ForgeScript + FUSE UI + C++23 helpers, bundles, capabilities, IPC, system services, platform gaps |
| [00-foundation/DESIGN_SYSTEM.md](00-foundation/DESIGN_SYSTEM.md) | The look and behaviour shared by every app |
| [00-foundation/SPEC_TEMPLATE.md](00-foundation/SPEC_TEMPLATE.md) | The 13 sections every spec has |
| [00-foundation/ROADMAP.md](00-foundation/ROADMAP.md) | Build order, milestones, shared engines |
| [00-foundation/GAPS.md](00-foundation/GAPS.md) | Platform work found while planning the apps, beyond PLATFORM.md section 13 |

## Layout

```
Apps/
  00-foundation/   platform, design system, catalogue, roadmap, tools
  shell/           SH01–SH05  the desktop itself
  system/          SY01–SY05  Settings, Files, Terminal, System Monitor, Software
  admin/           AD01–AD05  Installer, Disks, Backup, Logs, Permissions
  security/        SE01–SE05  Security Centre, Firewall, Anonymity, Passwords and Keys, Vault
  virt/            VM01–VM05  Virtual Machines, Jails, Compatibility, Remote Desktop, Devices
  office/          OF01–OF09  Text Editor, Notes, Document Viewer, Writer, Sheets, Slides, Calculator, Draw, Data Studio
  comms/           CO01–CO05  Mail, Calendar, Contacts, Tasks, Messenger
  internet/        IN01–IN05  Web Browser, Feeds, Transfers, Maps, Weather
  media/           ME01–ME06  Photos, Music, Video, Camera, Screen Capture, Sound Recorder
  create/          CR01–CR04  Image Editor, Video Editor, Audio Editor, Studio
  dev/             DV01–DV07  IDE, UI Designer, Version Control, Hex Editor, Packet Analyser, Proof Workbench, Reverse Engineering
  ai/              AI01–AI04  Assistant, Model Manager, Forecast, Notebook
  utils/           UT01–UT06  Archives, Clocks, Fonts, Disk Usage, Help, Printers and Scanners
  access/          AX01–AX06  Screen Reader, Magnifier, On-Screen Keyboard, Input Methods, Clipboard, Voice
  games/           GM01       Games
```

## The rules every app follows

1. App logic is **ForgeScript**. The UI is **FUSE UI**. Native helpers are **memory-safe C++23** in their own sandboxed processes. No Qt.
2. No JIT and no writable-executable memory.
3. Default deny: an app reaches only what its manifest declares and the user granted. Every grant is visible and revocable in Permissions (AD05).
4. No telemetry, no vendor accounts, no network unless declared, granted and shown.
5. Offline first. Open formats first.
6. Anything that parses untrusted bytes runs in a helper with no capabilities.
7. No claim of a passing test or proof without the recorded output.

## What was and was not read

- Read: this repo; `odin-loki/FUSE` and `odin-loki/FORGE` at `main` (12 Nov 2026); and, for the apps built on them, `SENTINEL`, `ANONYMOUS`, `NeuralVFX`, `PRISM`, `RetDec-Decompiler`, `MathScript` and `Cypha`. Each spec cites `REPO:path` for what it states as existing.
- Not read: `distro-build-plan-v4.md` (it is on the author's PC, not in a repo) and the private repo `Kodai`. If v4 names apps or decisions that differ from this plan, v4 wins and the catalogue should be reconciled against it.
- The repo's current plan (`docs/plan/PBSD_PLAN.md`, P5-02) still ships KDE Plasma. This suite is the Qt-free replacement; see ROADMAP.md section 5.

## Checking the plan

```sh
python3 Apps/00-foundation/tools/build_index.py          # checks every spec and rewrites INDEX.md
python3 Apps/00-foundation/tools/build_index.py --check  # check only
```
