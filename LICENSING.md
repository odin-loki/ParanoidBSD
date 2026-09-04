# ParanoidBSD licensing

**Author and copyright holder:** Odin Loch (Imortek), <odin.loch@outlook.com.au>
**Default licence:** [GNU Affero General Public License v3.0 or later](LICENSE) (`AGPL-3.0-or-later`)
**Commercial alternative:** [COMMERCIAL-LICENCE.md](COMMERCIAL-LICENCE.md)
**Attribution ledger:** [NOTICES.md](NOTICES.md) · **Per-module trail:** [docs/PROVENANCE.md](docs/PROVENANCE.md)

This file is the authoritative statement of what licence applies to what in this
repository. Where any other document in the tree disagrees with this one, this
one wins.

---

## 1. The short version

| Question | Answer |
|---|---|
| What licence is PBSD's own code under? | AGPL-3.0-or-later |
| Is the vendored HardenedBSD tree relicensed? | **No.** `hbsd/` is untouched upstream and stays under its own terms |
| Is the vendored KDE tree relicensed? | **No.** `kde/` is untouched upstream and stays under its own terms |
| Can I use PBSD in a closed-source product? | Not under the AGPL. A commercial licence is available |
| If I run PBSD as a network service, do I owe source? | Yes — AGPL §13. See [§7](#7-running-pbsd-over-a-network-agpl-13) |
| Which licence governs a specific file? | Its own `SPDX-License-Identifier` header if it has one; otherwise the table in [§2](#2-what-covers-what) |

---

## 2. What covers what

Paths are relative to the repository root. Longest matching prefix wins.

| Path | Licence | Origin |
|---|---|---|
| `hbsd/` | **Excluded from the AGPL grant.** Upstream terms — predominantly `BSD-2-Clause`, with `BSD-3-Clause`, `ISC`, `CDDL-1.0` (`hbsd/src/cddl/`, `hbsd/src/sys/contrib/openzfs/`), `GPL-2.0` (`hbsd/src/gnu/`) and assorted vendored third-party terms. See `hbsd/src/COPYRIGHT` and the per-file headers | HardenedBSD 15-STABLE / FreeBSD. Vendored verbatim as the behavioural specification for the port |
| `kde/` | **Excluded from the AGPL grant.** Upstream terms — predominantly `GPL-2.0-or-later` and `LGPL-2.1-or-later`, per the `LICENSES/` directory and `.reuse` metadata inside each subproject | KDE Plasma 6, KWin, KDE Frameworks 6. Vendored verbatim as the behavioural specification for the wave-3 port |
| `secrets/` | Not licensed for redistribution. Local credentials only; never committed | — |
| `pbsd/` | `AGPL-3.0-or-later`, **except** files carrying their own `SPDX-License-Identifier` (see [§3](#3-precedence-a-per-file-spdx-header-always-wins) and [§4](#4-derived-ports-inside-pbsd)) | PBSD C++23 modules — original design plus ports |
| `tools/` | `AGPL-3.0-or-later` | Original |
| `scripts/` | `AGPL-3.0-or-later` | Original |
| `docs/` | `AGPL-3.0-or-later` | Original |
| `pbsd.py`, `*.bat`, `.clang-tidy`, `.github/`, root files | `AGPL-3.0-or-later` | Original |

Vendoring is not relicensing. `hbsd/` and `kde/` are present in this repository so
that ports can be diffed against the originals. Nothing about their presence here
changes their licence, and nothing in this repository asks you to treat them as
AGPL. If you redistribute this repository as a whole, you are redistributing those
two trees under *their* licences and must satisfy *their* conditions.

---

## 3. Precedence: a per-file SPDX header always wins

Every source file in `pbsd/` that carries upstream code carries the upstream
header verbatim — the `SPDX-License-Identifier`, the copyright line, the
conditions and the warranty disclaimer. For example, `pbsd/bin/rm/b0198/port.cppm`
opens with:

```
/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 ...
```

**That header governs that file.** The AGPL default in §2 applies only to files
with no `SPDX-License-Identifier` of their own. Removing or rewriting an upstream
header to "make a file AGPL" is a licence violation, not a cleanup pass; the
deterministic rewrite passes in `tools/` must preserve headers byte-for-byte, and
`tools/check_licences.py` fails the build if a known-derived file loses one.

Where PBSD modifications are added to a file that already carries an upstream
header, the modifications are additionally offered under `AGPL-3.0-or-later`, and
the file's effective terms are the union of both — meaning you must satisfy the
upstream conditions *and* the AGPL. Such files carry a second line:

```
 * SPDX-FileCopyrightText: 2026 Odin Loch
 * Modifications: AGPL-3.0-or-later
```

---

## 4. Derived ports inside `pbsd/`

`pbsd/` is not homogeneous. There are three provenance classes, tracked per module
in [`docs/PROVENANCE.md`](docs/PROVENANCE.md) and per file by the `/// PROVENANCE:`
comment the port passes emit.

### 4.1 Original design — `AGPL-3.0-or-later`

The capability/handle nucleus and everything built on it: `pbsd/handles/`,
`pbsd/rights/`, `pbsd/lineage/`, `pbsd/uda/`, `pbsd/analyser/`, `pbsd/bifrost/`,
`pbsd/core/`, `pbsd/compositor/` (the PBSD modules, not the Wayland-protocol
bridges), `pbsd/theme/` (the PBSD Aero artwork and theme packages), and the
PBSD-specific parts of `pbsd/kernel/`. No upstream code, no upstream conditions.

One exception inside `pbsd/theme/`: the three source files of the KWin effect
plugin — `pbsd/theme/kwin-effects/pbsd_aero_blur/contents/code/{main.cpp,
pbsd_aero_blur.cpp,pbsd_aero_blur.h}` — link KWin's effect API and are licensed
`GPL-2.0-or-later`, which is what their headers say. Compatible per §4.3.1; the
SVG, JSON and shader assets in the same package are original and AGPL.

### 4.2 Ports of `BSD-2-Clause` / `BSD-3-Clause` upstreams — AGPL, with notices retained

The bulk of `pbsd/bin/`, `pbsd/sbin/`, `pbsd/usr.bin/`, `pbsd/usr.sbin/`,
`pbsd/lib/`, `pbsd/userland/`, `pbsd/sys/`, `pbsd/net/`, `pbsd/fs/`, `pbsd/geom/`,
`pbsd/stand/`, `pbsd/arch/` and `pbsd/kernel/` — C→C++23 rewrites of HardenedBSD
and FreeBSD sources.

The BSD licences are permissive: a derivative work may be distributed under
stricter terms, **provided the original copyright notice, the list of conditions
and the disclaimer are retained**, and — for `BSD-3-Clause` — provided the
copyright holders' names are not used to endorse the derivative. Both conditions
are satisfied by keeping the upstream header in place, which is why §3 is a hard
rule and not a style preference. PBSD's rewrites of these files are licensed
`AGPL-3.0-or-later`; the retained upstream grant is unaffected and continues to
run to you from its original holders.

#### 4.2.1 `BSD-4-Clause` files: an open incompatibility, not a resolved one

Twenty-two files in the port tree inherit the **original four-clause BSD licence**,
whose clause 3 requires an acknowledgement in all advertising material. That is an
additional restriction in the sense of AGPL §7, and the FSF classifies the original
BSD licence as GPL-incompatible. The 1999 University of California rescission that
retired clause 3 applies only to code copyrighted by the Regents — it does **not**
reach these files, whose holders are individuals and companies (Bill Paul, Jason
R. Thorpe, HD Associates, Peter Wemm, Andrew Moore, David Greenman, Henrik
Vestergaard Draboel, Gordon W. Ross, Theo de Raadt, Berkeley Softworks, John
Birrell, David Xu, Sergey Osokin, Garrett D'Amore, Nexenta Systems).

Stated plainly: **these files are excluded from the AGPL grant and must not be
linked into an AGPL-licensed PBSD binary.** They remain available under
`BSD-4-Clause` from their own holders under the §3 precedence rule.

The affected paths:

```
pbsd/bin/chio/b0192/{oracle.c,port.cppm}
pbsd/lib/libc/amd64/gen/b0055/{oracle.c,port.cppm}
pbsd/lib/libc/locale/b0147/port.cppm
pbsd/lib/libc/rpc/b0196/{oracle.c,port.cppm}
pbsd/lib/libc/rpc/b0196s2/{oracle.c,port.cppm}
pbsd/lib/libcam/scsi_cmdparse_m/{oracle.c,port.cppm}
pbsd/lib/libthr/sys/b0234/{oracle.c,port.cppm}
pbsd/lib/libthr/thread/b0271/{oracle.c,port.cppm}
pbsd/lib/libthr/thread/b0278/{oracle.c,port.cppm}
pbsd/sbin/bsdlabel/bsdlabel_m/oracle.c
pbsd/sbin/rcorder/ealloc_m/{oracle.c,port.cppm}
pbsd/usr.sbin/rtprio/rtprio_m/{oracle.c,port.cppm}
```

Remediation, tracked in [NOTICES.md](NOTICES.md), is one of: take the current
upstream FreeBSD revision where the file has since been relicensed to two- or
three-clause; obtain a relicence from the holder; or reimplement the module from
its published interface. `tools/check_licences.py` holds this list as a ratchet —
the count may fall, never rise.

### 4.3 Ports and bridges against copyleft upstreams — case by case

| Tree | Upstream | Upstream licence | Status |
|---|---|---|---|
| `pbsd/kde/` | KDE Frameworks 6, KWin, Plasma 6 | `GPL-2.0-or-later`, `LGPL-2.1-or-later` | See §4.3.1 |
| `pbsd/compositor/pbsd.compositor.wayland.*` | KWin's Wayland layer + the Wayland protocol headers | `GPL-2.0-or-later`; protocol XML is `MIT` | See §4.3.1 |
| `pbsd/zfs/` | OpenZFS (via `hbsd/src/sys/contrib/openzfs/`) | `CDDL-1.0` | See §4.3.2 |

#### 4.3.1 KDE-derived code: compatible, and must stay so

`GPL-2.0-**or-later**` is forward-compatible with the AGPL. A recipient may elect
GPL-3.0 under the "or later" option, and GPL-3.0 §13 grants explicit permission to
combine a GPL-3.0 work with an AGPL-3.0 work into a single combined work conveyed
under the AGPL, with AGPL §13's network-source requirement applying to the part it
covers. The same route works from `LGPL-2.1-or-later`. So PBSD's KDE bridges,
its KWin effect plugin, and a PBSD desktop image that links Plasma may all be
conveyed as a whole under `AGPL-3.0-or-later`.

Two conditions on that, and they are not optional:

1. **The upstream files stay under their own terms.** Every file in `pbsd/kde/`
   or `pbsd/compositor/` that carries KDE code must carry the KDE
   `SPDX-License-Identifier` and `SPDX-FileCopyrightText` lines it inherited.
   Per §3, that header governs the file. A file that is a genuine original —
   PBSD constants, a bridge written from the published API rather than from
   KDE's source — is AGPL and says so.
2. **`GPL-2.0-only` is incompatible and must not enter the tree.** The
   or-later route is the whole basis of §4.3.1. A KDE or Qt component licensed
   GPL-2.0 *without* the "or later" clause cannot be combined with AGPL-3.0
   code at all. Before a new KDE dependency is ported, its licence must be
   confirmed to carry "or (at your option) any later version" and recorded in
   [NOTICES.md](NOTICES.md).

#### 4.3.2 OpenZFS: `CDDL-1.0`, incompatible, kept at arm's length

The CDDL is a file-level copyleft that the FSF and the SFLC both consider
incompatible with the GPL family, and therefore with the AGPL. This is not
resolvable by a compatibility clause; there is no §13 route from CDDL.

PBSD's rule, unchanged from the port plan's licensing ledger:

- ZFS is used as a **design reference** and loaded as a **separate, unmodified
  kernel module**. It is not statically combined with AGPL PBSD code, and AGPL
  PBSD code is not linked into it.
- The modules under `pbsd/zfs/` are ABI and constant declarations describing the
  OpenZFS interface so that PBSD can talk to it across that boundary. Where any
  of them contains expression copied from OpenZFS rather than an interface fact,
  that file is `CDDL-1.0`, must carry the CDDL header, and is excluded from the
  AGPL grant exactly like `hbsd/` is.
- No PBSD build target links `pbsd/zfs/` into an AGPL binary. If a future change
  needs that, the change is not permitted — use the module boundary.

Auditing every file under `pbsd/zfs/` against this rule is open work, tracked in
[NOTICES.md](NOTICES.md). Until it closes, treat the whole tree as
**CDDL-1.0-encumbered and excluded from the AGPL grant** — the conservative
reading, and the one that cannot become a violation.

---

## 5. Compatibility summary

| Combining AGPL-3.0-or-later PBSD code with… | Allowed? | Route |
|---|---|---|
| `BSD-2-Clause`, `BSD-3-Clause`, `MIT`, `ISC`, `0BSD`, `BSD-1-Clause` | Yes | Permissive; retain notices and disclaimers |
| `BSD-4-Clause` (original BSD) | **No** | The advertising clause is an added restriction under AGPL §7. See §4.2.1 |
| `LGPL-2.1-or-later`, `LGPL-3.0` | Yes | Elect the later version; GPL-3.0 §13 |
| `GPL-2.0-or-later`, `GPL-3.0-or-later` | Yes | Elect GPL-3.0; GPL-3.0 §13 permits the AGPL combination |
| `GPL-2.0-only`, `GPL-3.0-only` | **No** | No upgrade path; §13 is not available. Do not introduce |
| `CDDL-1.0` | **No** | File-level copyleft with no compatibility clause. Separate module only |
| `Apache-2.0` | Yes, one direction | Apache-2.0 code may be taken into an AGPL-3.0 work; the reverse is not permitted |
| Proprietary / closed source | **No** under the AGPL | Use the [commercial licence](COMMERCIAL-LICENCE.md) |

---

## 6. If you redistribute PBSD

Whether you ship source, a kernel, an installer image, or a running appliance,
all of the following apply:

1. **Ship the licence.** Include [`LICENSE`](LICENSE) (AGPL-3.0) and this file.
2. **Ship the corresponding source** for every AGPL-covered component, per AGPL
   §6 — including your modifications, and including the scripts used to control
   compilation and installation.
3. **Keep every upstream notice.** The BSD copyright headers in the ports, the
   KDE `SPDX-FileCopyrightText` lines, `hbsd/src/COPYRIGHT`, and the `LICENSES/`
   directories inside `kde/`'s subprojects travel with the code.
4. **Satisfy the excluded trees' own conditions.** `hbsd/` and `kde/` are not
   yours to relicense either. Shipping a binary built from `kde/` triggers that
   project's GPL source-offer obligations, independently of anything PBSD does.
5. **Do not use the copyright holders' names to endorse your build.** Required by
   `BSD-3-Clause`; also just the right thing.
6. **Keep ZFS separate.** Per §4.3.2, ship OpenZFS as its own unmodified,
   CDDL-licensed module with its own licence text.

---

## 7. Running PBSD over a network (AGPL §13)

This is the clause that distinguishes the AGPL from the GPL, and it is deliberate.

> If you modify PBSD and let users interact with it remotely over a network, you
> must offer those users the complete corresponding source of your modified
> version, at no charge, through a network server.

"Interact remotely" covers a PBSD-derived service, a hosted appliance, a
management plane, or a BIFROST endpoint reachable by someone who is not you.
It does **not** cover running an unmodified PBSD desktop on your own machine, and
it does not cover internal use with no remote users.

If this obligation is incompatible with your product, that is exactly the case the
[commercial licence](COMMERCIAL-LICENCE.md) exists for. Ask; don't quietly
violate it.

---

## 8. Contributing

Inbound licence equals outbound licence. By opening a pull request you certify
that:

- you wrote the contribution or have the right to submit it;
- you license it under `AGPL-3.0-or-later`, and additionally grant Odin Loch the
  right to offer it under the commercial terms in
  [COMMERCIAL-LICENCE.md](COMMERCIAL-LICENCE.md) — dual licensing only works if
  every contribution can be offered under both;
- any third-party code you bring in carries its own header, is compatible per §5,
  and is recorded in [NOTICES.md](NOTICES.md) and
  [`docs/PROVENANCE.md`](docs/PROVENANCE.md).

A contribution that strips an upstream header, or that introduces a `GPL-2.0-only`
or `CDDL-1.0` dependency into an AGPL build target, will be rejected.

---

## 9. Commercial licence

PBSD is dual-licensed. If the AGPL's copyleft or its §13 network clause does not
suit your deployment, a separate commercial licence removes both obligations.
Terms and contact: [COMMERCIAL-LICENCE.md](COMMERCIAL-LICENCE.md).

Note that a commercial PBSD licence covers **PBSD's own code only**. It cannot and
does not relicense HardenedBSD, KDE, or OpenZFS — those you take on their own
terms regardless.

---

## 10. How this is enforced

| Mechanism | What it checks |
|---|---|
| `tools/check_licences.py` | Root licence files present; no `SPDX-License-Identifier` stripped from a derived port; no `GPL-2.0-only` licence identifier anywhere in `pbsd/`; the `BSD-4-Clause` set of §4.2.1 held as a ratchet that may shrink but never grow; every SPDX identifier found in `pbsd/` known to §5 |
| `.github/workflows/pbsd-ci.yml` | Runs the above on every push and pull request |
| [`docs/PROVENANCE.md`](docs/PROVENANCE.md) | Per-module spec source, licence and author — required before a module is implemented |
| [NOTICES.md](NOTICES.md) | Third-party attribution ledger for anything vendored or ported |
| `/// PROVENANCE:` comments | Per-file trail back to the upstream path a port was derived from |

Run the check locally before pushing:

```bash
python3 tools/check_licences.py
```

---

## 11. Reporting a licensing problem

If you believe a file here carries your code without the right notice, or that
this repository misstates a licence, email <odin.loch@outlook.com.au> with the
path and the upstream source. Header and attribution errors will be corrected
promptly; that is the whole point of the provenance discipline. Please raise it
directly before filing anything formal.
