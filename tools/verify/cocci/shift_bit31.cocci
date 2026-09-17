/// SPDX-License-Identifier: AGPL-3.0-or-later
/// A left shift that reaches or passes bit 31 of a signed int.
///
/// docs/security/UB_FINDINGS.md fixed this class three times and then
/// found it again by hand: `lge' and `sis' write `(data << 16)' where
/// data carries BMCR_RESET = 0x8000, so the shift is 2^31 in an `int';
/// `if_dcreg.h:1050' does it for 30 of 74 device IDs; `pci.c:6096' does
/// it on a device-supplied runtime value in core PCI code.  The in-tree
/// correct idiom is already there in rt2860.c, if_run.c and if_mtw.c:
/// `(uint32_t)val << 16'.
///
/// Three shapes, all UB in C and none of them diagnosed by CBMC (the
/// function is never a checked entry point) or by clang --analyze (the
/// value comes from a caller it cannot see):
///   1 << 31            - the constant case, still UB
///   1 << i             - a loop to 31, which lge's siblings do
///   E << 16            - a 16-bit value in an int, shifted into the sign
//
// Confidence: Moderate
// Options: --no-includes
virtual report

@bit31const@
constant C;
position p;
@@
  1 << C@p

@shiftby16@
expression E;
position p;
@@
(
  E << 16@p
|
  E << 24@p
)

@script:python@
c << bit31const.C;
p << bit31const.p;
@@
try:
    n = int(c, 0)
except ValueError:
    n = -1
if n >= 31:
    print("PBSD\t%s\t%s\t%s\tshift-bit31\t1 << %s on a signed int is undefined; the operand needs (uint32_t) or 1U"
          % (p[0].file, p[0].line, p[0].column, c))

@script:python@
p << shiftby16.p;
@@
print("PBSD\t%s\t%s\t%s\tshift-into-sign\ta value shifted this far reaches bit 31 of an int if it has its top bits set; cast the operand"
      % (p[0].file, p[0].line, p[0].column))
