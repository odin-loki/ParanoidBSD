# ParanoidBSD commercial licence

ParanoidBSD is dual-licensed. This file describes the paid alternative to the
[AGPL-3.0-or-later](LICENSE) default. For what the AGPL covers and what it does
not, read [LICENSING.md](LICENSING.md) first — particularly §2, which lists the
trees this licence **cannot** reach.

**Contact:** Odin Loch (Imortek) — <odin.loch@outlook.com.au>

---

## Who needs one

You do **not** need a commercial licence if you are an individual, a charity, a
school or university, or an organisation with annual income under **AUD 50,000**.
The AGPL costs you nothing and its obligations are the whole price.

You **do** need one if any of these is true:

- your organisation's annual income is AUD 50,000 or more;
- you want to ship PBSD-derived code without publishing your modifications;
- you run a PBSD-derived service that users reach over a network and do not want
  to make its source available to them (AGPL §13);
- your own product's licence cannot accept copyleft reach.

The threshold is measured on the organisation's annual income, not on revenue
attributable to the software. It is a deliberately low-friction test: if you are
small enough that working it out is annoying, you are small enough to be on the
free tier.

---

## What the commercial licence changes

| | AGPL-3.0-or-later | Commercial |
|---|---|---|
| Cost | Free | Tiered by organisation size, quoted per enquiry |
| Use in a product | Yes | Yes |
| Modify the source | Yes | Yes |
| Must publish your modifications | Yes, under the same dual licence | No |
| Network use triggers a source obligation | Yes — AGPL §13 | No |
| Research or products built on it | Must be open-sourced | No obligation |
| Attribution line required | Yes | Optional |
| Support | Best effort, via GitHub issues | Direct, by arrangement |

There is no published price list. The Imortek catalogue spans an operating
system, a decompiler, a numerics library and an anonymity transport, and what an
organisation needs from each differs enormously. Email with what you want to use
and roughly how big you are, and you will get a number rather than a discovery
call.

---

## What it does **not** change

A commercial PBSD licence covers **PBSD's own code** — the trees marked
`AGPL-3.0-or-later` in [LICENSING.md §2](LICENSING.md#2-what-covers-what). It is
not, and cannot be, a licence to the works PBSD is built on. Odin Loch does not
hold their copyright and does not claim to.

Regardless of any commercial agreement, you take these on their own terms:

| | Terms that still bind you |
|---|---|
| `hbsd/` — HardenedBSD and FreeBSD | `BSD-2-Clause` and friends. Retain the notices and disclaimers; do not use the holders' names to endorse your build |
| `kde/` — Plasma 6, KWin, KDE Frameworks | `GPL-2.0-or-later` / `LGPL-2.1-or-later`. If you ship binaries built from it, that project's source-offer obligations are yours, and no PBSD agreement waives them |
| OpenZFS | `CDDL-1.0`, shipped as a separate unmodified module. See [LICENSING.md §4.3.2](LICENSING.md#432-openzfs-cddl-10-incompatible-kept-at-arms-length) |
| The `BSD-4-Clause` files of [§4.2.1](LICENSING.md#421-bsd-4-clause-files-an-open-incompatibility-not-a-resolved-one) | The original four-clause BSD terms, advertising clause included |
| Ported files carrying an upstream `SPDX-License-Identifier` | That header, per [LICENSING.md §3](LICENSING.md#3-precedence-a-per-file-spdx-header-always-wins). A commercial licence covers the PBSD modifications, not the underlying grant |

If you need a PBSD-derived product with no BSD or GPL obligations at all, that is
not a licensing problem — it is a build problem, and the answer is a configuration
that omits those components. Ask and it can be scoped.

---

## The attribution line

AGPL users carry this in an about box, documentation, or a footer:

> Powered by ParanoidBSD, developed by Odin Loch. Licensed under AGPL-3.0+.

Commercial licensees may keep it or drop it, as they prefer.

---

## Making an enquiry

Email <odin.loch@outlook.com.au> with:

1. which components you want to use (`pbsd/` as a whole, the handle nucleus, UDA,
   BIFROST, the compositor, the tooling — whatever it actually is);
2. roughly how large your organisation is;
3. whether you will distribute binaries, run a network service, or both.

That is enough to quote from. You will get a number back.

---

*This file is a summary of the terms on offer, not the agreement itself and not
legal advice. A commercial licence is a signed document; where it and this file
differ, the signed document governs. Where this file and [LICENSE](LICENSE)
differ as to the AGPL, `LICENSE` governs.*
