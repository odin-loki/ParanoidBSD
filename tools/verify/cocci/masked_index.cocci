/// SPDX-License-Identifier: AGPL-3.0-or-later
/// A masked hardware field used as an index or a loop bound.
///
/// docs/security/UB_FINDINGS.md, task #63: the PCI Enhanced Allocation
/// entry-size field PCIM_EA_ES is THREE BITS, so a device can claim up to
/// seven dwords follow an entry header, and `dw[]' is FOUR.
///
///     ent_size = (val & PCIM_EA_ES);
///     for (b = 0; b < ent_size; b++)
///             dw[b] = read_config(fd, &p->pc_sel, ptr, 4);
///
/// Three dwords past the end of a stack array, filled from PCI
/// configuration space, in a program run as root.  The kernel's copy was
/// fixed; usr.sbin/pciconf/cap.c carried the same code.
///
/// The mask is the whole tell: a value that has just been AND-ed with a
/// hardware field mask has the range the HARDWARE allows, which is a fact
/// about the device and not about the array.  This cannot decide whether
/// the mask fits - Coccinelle does not know sizeof - so it reports the
/// pairing and a person compares the two constants.  That is a candidate
/// list, not a verdict, and it belongs in the advisory tier.
//
// Confidence: Low
// Options: --no-includes
virtual report

@masked@
expression V, M;
identifier idx, arr;
position p;
@@
  idx = V & M;
  ... when != idx = V
      when != idx = M
  arr[idx]@p

@maskloop@
expression V, M;
identifier idx, arr, i;
position p;
@@
  idx = V & M;
  ... when != idx = V
  for (i = 0; i < idx; i++) { ... arr[i]@p ... }

@script:python@
p << masked.p;
@@
print("PBSD\t%s\t%s\t%s\tmasked-index\ta value masked from a hardware field indexes an array; compare the mask's width with the array's size"
      % (p[0].file, p[0].line, p[0].column))

@script:python@
p << maskloop.p;
@@
print("PBSD\t%s\t%s\t%s\tmasked-loop-bound\ta value masked from a hardware field bounds a loop that writes an array; compare the mask's width with the array's size"
      % (p[0].file, p[0].line, p[0].column))
