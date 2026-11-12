<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Apps

By Odin Loch

The application suite for ParanoidBSD: the desktop shell and 78 apps, planned in full. Each app has its own folder here. Today each folder holds the plan (`SPEC.md`) and a draft `manifest.json`; the app's code goes in the same folder when it is built.

**Status: plan only. Nothing in this folder has been built, run, tested or proved.**

Size of the plan: 78 app specs, 63,428 lines, 11,051 features, 2,234 work items, plus the two build plans. The counts come from the index tool, which also checks every spec's structure and IDs; it reports no problems.

## Start here

| File | What it is |
|---|---|
| [INDEX.md](INDEX.md) | Generated table of every app spec with line, feature and work-item counts, and the result of the consistency check |
| [00-foundation/CATALOGUE.md](00-foundation/CATALOGUE.md) | The 78 apps, their IDs, folders and priorities |
| [00-foundation/PLATFORM.md](00-foundation/PLATFORM.md) | How an app is built: ForgeScript + FUSE UI + C++23 helpers, bundles, capabilities, IPC, system services, platform gaps |
| [00-foundation/DESIGN_SYSTEM.md](00-foundation/DESIGN_SYSTEM.md) | The look and behaviour shared by every app |
| [00-foundation/SPEC_TEMPLATE.md](00-foundation/SPEC_TEMPLATE.md) | The 13 sections every spec has |
| [00-foundation/ROADMAP.md](00-foundation/ROADMAP.md) | Build order, milestones, shared engines |
| [00-foundation/GAPS.md](00-foundation/GAPS.md) | 133 further gaps found while planning the apps, grouped by who must build them (compositor, kernel, platform, FUSE, FORGE, other repos) |
| [00-foundation/DECISIONS.md](00-foundation/DECISIONS.md) | Every open choice and what was chosen, on the rule "functionality and simplicity". Wins over a spec where they differ |
| [00-foundation/build-plans/FUSE_BUILD_PLAN.md](00-foundation/build-plans/FUSE_BUILD_PLAN.md) | Everything to build in FUSE: 70 work packages, 526 tasks, 106 requirements |
| [00-foundation/build-plans/FORGE_BUILD_PLAN.md](00-foundation/build-plans/FORGE_BUILD_PLAN.md) | Everything to build in FORGE: 40 work packages, 215 tasks, 113 requirements |

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
- Reading the repos changed the roles first assumed for Cypha, NeuralVFX, SENTINEL and AEGIS. DECISIONS.md section 1 says what each really is and how the specs use it.
- Not read: `distro-build-plan-v4.md` (it is on the author's PC, not in a repo) and the private repo `Kodai`. If v4 names apps or decisions that differ from this plan, v4 wins and the catalogue should be reconciled against it.
- Nothing was built or run on HardenedBSD. Every statement about HardenedBSD behaviour is marked Unverified in the document that makes it.
- The repo's current plan (`docs/plan/PBSD_PLAN.md`, P5-02) still ships KDE Plasma. This suite is the Qt-free replacement; see ROADMAP.md section 5.

## Known loose ends

- Seven specs were written before a decision that overrides part of them: SH05 (window policy, D-61), SH01 (`Super+digit`, D-60), IN03 (SSH library, D-33), VM04 (RDP code, D-78), DV01 (`Ctrl+P`, D-67), and DV01, DV03 and DV05, which list SSH as open (D-33). DECISIONS.md wins until those sections are edited.
- Five manifests carry placeholder hosts (`net.host:<...>`) where the host is chosen by the user or not yet known: Maps (two), Data Studio, Anonymity, Firewall.
- The app-to-work-package tables in the two build plans were written while the specs were still arriving. They should be re-derived now that all 78 specs exist.
- Wireframe numbers and sample data are layout placeholders.

## Checking the plan

```sh
python3 Apps/00-foundation/tools/build_index.py          # checks every spec and rewrites INDEX.md
python3 Apps/00-foundation/tools/build_index.py --check  # check only
```
