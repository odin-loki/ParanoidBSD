/// SPDX-License-Identifier: AGPL-3.0-or-later
/// A signed index checked on one side only, then used to index an array.
///
/// docs/security/UB_FINDINGS.md, task #123, `es_vlangroup': eight
/// etherswitch drivers took an int straight out of an ioctl and indexed
/// `sc->vid[16]' with it.  Three never checked at all, two checked the
/// read path and not the write, and every `>' test that existed was
///
///   "wrong twice over: valid groups are 0 .. es_nvlangroups - 1, so `>'
///    admits es_nvlangroups itself, and against a signed index it admits
///    every negative value - es_vlangroup = -1 reaches sc->vid[-1]."
///
/// The fix in all eight is the same two-sided test.  This matches the
/// shape that was wrong: an upper-bound test with no `< 0' anywhere
/// between it and the subscript.  tools/verify/onesided_index.py is the
/// bespoke check for the same class; this is the semantic-patch form, and
/// disagreement between them is worth reading either way.
//
// Confidence: Low
// Options: --no-includes
virtual report

@upper@
expression E, C;
identifier arr;
position p;
@@
(
  if (E > C) { ... return ...; }
|
  if (E >= C) { ... return ...; }
|
  if (E > C) return ...;
|
  if (E >= C) return ...;
)
  ... when != E < 0
      when != E >= 0
      when != E = C
      when != E < C
  arr[E]@p

@script:python@
p << upper.p;
@@
print("PBSD\t%s\t%s\t%s\tonesided-index\tan index with an upper-bound test and no `< 0' test reaches arr[-1] if it is signed"
      % (p[0].file, p[0].line, p[0].column))
