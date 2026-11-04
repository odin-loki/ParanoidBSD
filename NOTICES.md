# PBSD third-party notices

Attribution ledger for every third-party work vendored into or ported inside this
repository. Referenced as `NOTICES` by
[`docs/plans/cxx23-port-master-plan.md`](docs/plans/cxx23-port-master-plan.md) §1
and §13. Licence policy: [LICENSING.md](LICENSING.md). Per-module trail:
[docs/PROVENANCE.md](docs/PROVENANCE.md).

Nothing listed here is relicensed by its presence in this repository.

---

## 1. Vendored upstream trees

| Tree | Project | Licence | Notice |
|---|---|---|---|
| `hbsd/` | [HardenedBSD](https://hardenedbsd.org/) 15-STABLE, itself derived from FreeBSD | Predominantly `BSD-2-Clause`; also `BSD-3-Clause`, `BSD-4-Clause`, `ISC`, `MIT`, `CDDL-1.0` (`src/cddl/`, `src/sys/contrib/openzfs/`), `GPL-2.0` (`src/gnu/`) and vendored third-party terms | `hbsd/src/COPYRIGHT`, plus the per-file headers. Vendored verbatim as the behavioural specification for the port |
| `kde/frameworks/` | [KDE Frameworks 6](https://invent.kde.org/frameworks) — kconfig, kcoreaddons, kdecoration, kio, kservice, kwayland, kwindowsystem, kxmlgui, layer-shell-qt, plasma-framework | Predominantly `LGPL-2.1-or-later`; some `GPL-2.0-or-later`, `BSD-2-Clause`, `MIT`, `CC0-1.0` | Each subproject's `LICENSES/` directory and its REUSE metadata |
| `kde/kwin/` | [KWin](https://invent.kde.org/plasma/kwin) | `GPL-2.0-or-later` | `kde/kwin/LICENSES/` |
| `kde/plasma-desktop/` | [Plasma Desktop](https://invent.kde.org/plasma/plasma-desktop) | `GPL-2.0-or-later`, `LGPL-2.1-or-later` | its `LICENSES/` directory |

---

## 2. Ported code inside `pbsd/`

Header counts are the SPDX identifiers actually present in the tree, not
estimates. Every one of these files retains the upstream copyright notice, the
list of conditions and the warranty disclaimer, unmodified.

| SPDX identifier | Files | Upstream | AGPL-combinable |
|---|---:|---|---|
| `BSD-2-Clause` | 557 | HardenedBSD / FreeBSD | Yes |
| `BSD-3-Clause` | 546 | HardenedBSD / FreeBSD, The Regents of the University of California | Yes |
| `BSD-4-Clause` | 22 | Assorted individual FreeBSD / NetBSD contributors | **No** — see §3.1 |
| `ISC` | 22 | Internet Software Consortium code in the `libc` resolver and `inet` | Yes |
| `MIT` | 8 | Wayland protocol headers and assorted userland | Yes |
| `BSD-1-Clause` | 5 | FreeBSD | Yes |
| `(BSD-3-Clause AND ISC)` | 4 | `libc` resolver (`res_*`) and `inet` | Yes |
| `GPL-2.0-or-later` | 3 | KWin effect API — `pbsd/theme/kwin-effects/pbsd_aero_blur/contents/code/` | Yes, via GPL-3.0 §13 |
| `0BSD` | 2 | FreeBSD | Yes |

A single ported file may merge several upstream sources and so carry more than
one identifier; the counts above are files, not headers, and the columns
therefore overlap. `python3 tools/check_licences.py --verbose` reprints them from
the tree, and treats each as a floor.

Modules with **no** upstream header are original PBSD design, licensed
`AGPL-3.0-or-later`: the handle / rights / lineage nucleus, UDA, the analyser,
BIFROST, the compositor, and the PBSD Aero theme assets. Their spec sources are
listed per module in [docs/PROVENANCE.md](docs/PROVENANCE.md), which requires a
public standard, a published datasheet, or an original design — never another
implementation's source.

---

## 3. Open compliance items

These are known and tracked rather than quietly carried. `tools/check_licences.py`
enforces each one.

### 3.1 `BSD-4-Clause` files cannot enter an AGPL binary — 22 files

The original four-clause BSD licence carries an advertising clause that the AGPL
does not permit as an added restriction. The 1999 University of California
rescission does not reach these files; their holders are individuals and
companies, not the Regents. Full path list and reasoning:
[LICENSING.md §4.2.1](LICENSING.md#421-bsd-4-clause-files-an-open-incompatibility-not-a-resolved-one).

Holders affected: Bill Paul; Jason R. Thorpe; HD Associates; Peter Wemm; Andrew
Moore (Talke Studio); David Greenman; Henrik Vestergaard Draboel; Gordon W. Ross;
Theo de Raadt; Berkeley Softworks; John Birrell (CIMlogic); David Xu; Sergey
Osokin; Garrett D'Amore; Nexenta Systems.

**Resolution options, per file:** take the current upstream FreeBSD revision if it
has since been relicensed; obtain a relicence from the holder; or reimplement the
module from its published interface. Until then the files stay excluded from the
AGPL grant and out of AGPL link targets. The list in `tools/check_licences.py` is
a ratchet — it may shrink, never grow.

### 3.2 `pbsd/zfs/` needs a per-file CDDL audit — 193 files

OpenZFS is `CDDL-1.0`, which has no compatibility route to the AGPL. The modules
under `pbsd/zfs/` are intended to be interface declarations — ABI constants, flag
enums, structure shapes — which are facts about a boundary rather than expression
copied across it. That intent has not yet been verified file by file.

Until the audit closes, `pbsd/zfs/` is treated as **CDDL-encumbered and excluded
from the AGPL grant**, ZFS is loaded as a separate unmodified module, and no AGPL
build target links the tree. See [LICENSING.md §4.3.2](LICENSING.md#432-openzfs-cddl-10-incompatible-kept-at-arms-length).

### 3.3 KDE-derived ports need their inherited headers — `pbsd/kde/`, 487 files

Only three files in the KDE-facing trees currently carry a KDE SPDX header. Where
a module under `pbsd/kde/` or `pbsd/compositor/pbsd.compositor.wayland.*` was
written from KDE's *source* rather than from its published API, it is a derivative
work and must carry the upstream `SPDX-License-Identifier` and
`SPDX-FileCopyrightText` lines. Where it was written from the published API or is
a PBSD original, it is AGPL and should say so explicitly.

Every file in those trees needs an explicit identifier either way. Until that pass
lands, treat unlabelled files in `pbsd/kde/` as potentially `GPL-2.0-or-later` —
the conservative reading, and compatible in any case per
[LICENSING.md §4.3.1](LICENSING.md#431-kde-derived-code-compatible-and-must-stay-so).

### 3.4 `GPL-2.0-only` must never enter the tree

The whole KDE compatibility argument rests on the "or (at your option) any later
version" clause. A GPL-2.0-only component would break it with no remedy short of
removal. `tools/check_licences.py` fails on the identifier appearing anywhere
under `pbsd/`.

---

## 4. Reporting

If your code appears here without the right notice, email
<odin.loch@outlook.com.au> with the path and the upstream source. Corrections are
made promptly.
