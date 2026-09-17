/// SPDX-License-Identifier: AGPL-3.0-or-later
/// A bounds assertion joined by || asserts nothing.
///
/// From docs/security/UB_FINDINGS.md, the isa_dma shape: "the contract
/// written six times and enforced nowhere".  The subtler version is the
/// contract that IS written and is a tautology:
///
///     MPASS(idx >= 0 || idx < IOMMU_MAX_MSI);
///
/// Every integer satisfies one disjunct or the other, so the assertion is
/// always true and the array index it was meant to bound is unbounded.
/// The author meant &&.  There is no reading of || here that is correct,
/// which is what makes this worth a gate: it needs no reachability
/// argument and no judgement about the caller.
///
/// `E < LO || E >= HI' is NOT matched and must not be.  That is the
/// EXCLUSION idiom - "va is outside [UPT_MIN, UPT_MAX)" - and || is
/// correct there.  An earlier version of this rule included it and
/// reported sys/amd64/amd64/pmap.c:7132 and sys/arm/arm/pmap-v6.c:3885,
/// which are both right; two of its four tree-wide hits were that one
/// alternative.  The shape that is always wrong is the INCLUSION test
/// written with ||.
//
// Confidence: High
// Options: --no-includes
virtual report

@r@
expression E, C1, C2;
position p;
@@
(
  KASSERT@p(E >= C1 || E < C2, ...)
|
  KASSERT@p(E > C1 || E < C2, ...)
|
  KASSERT@p(E >= C1 || E <= C2, ...)
|
  MPASS@p(E >= C1 || E < C2)
|
  MPASS@p(E > C1 || E < C2)
|
  MPASS@p(E >= C1 || E <= C2)
|
  VERIFY@p(E >= C1 || E < C2)
|
  ASSERT@p(E >= C1 || E < C2)
)

@script:python@
p << r.p;
@@
print("PBSD\t%s\t%s\t%s\ttautological-bound\ta bounds assertion joined by || is always true; && was meant"
      % (p[0].file, p[0].line, p[0].column))
