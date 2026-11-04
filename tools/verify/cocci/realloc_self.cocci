/// SPDX-License-Identifier: AGPL-3.0-or-later
/// realloc() written back over the only pointer to the block.
///
/// docs/security/UB_FINDINGS.md has this three times - usr.sbin/tftp's
/// main.c ("realloc into the only pointer to the buffer"), ppp's
/// datalink2iov(), and the ether_header realloc in :19266.  On failure
/// realloc returns NULL and does NOT free, so the assignment overwrites
/// the last reference to a still-allocated block: the old block leaks and
/// every later use of the pointer is a NULL dereference.
///
/// FreeBSD's reallocf() is the correct spelling and is deliberately NOT
/// matched - it frees on failure, which is the whole reason it exists.
///
/// The four-argument kernel realloc(9) has the SAME contract - "if the
/// requested memory cannot be allocated, NULL is returned and the memory
/// referenced by addr is unchanged" - so it is matched too, but ONLY with
/// M_NOWAIT.  Leaving the kernel form out entirely was how
/// sys/kern/subr_stats.c:395 was missed on the first tree sweep while its
/// userland twin two lines down was reported; matching it without looking
/// at the flags was worse, because M_WAITOK CANNOT FAIL and 26 of the 36
/// sites that produced were `realloc(p, n, M_X, M_WAITOK)' - correct code,
/// reported as a leak.
//
// Confidence: High
// Options: --no-includes
virtual report

@self@
expression P, SZ, ty;
type T;
position p;
@@
(
  P@p = realloc(P, SZ)
|
  P@p = (T)realloc(P, SZ)
|
  P@p = realloc(P, SZ, ty, \(M_NOWAIT\|M_NOWAIT | M_ZERO\|M_ZERO | M_NOWAIT\))
|
  P@p = (T)realloc(P, SZ, ty, \(M_NOWAIT\|M_NOWAIT | M_ZERO\))
)

@self2@
expression E, SZ, ty;
identifier fld;
position p;
@@
(
  E->fld@p = realloc(E->fld, SZ)
|
  E->fld@p = realloc(E->fld, SZ, ty, \(M_NOWAIT\|M_NOWAIT | M_ZERO\|M_ZERO | M_NOWAIT\))
)

// The kernel form whose flags are a VARIABLE.  Coccinelle cannot tell
// whether it is M_WAITOK (cannot fail, correct code) or M_NOWAIT (can,
// and this leaks), so it is reported under its own name and is advisory.
// sys/kern/subr_stats.c:395 is one: stats_realloc() DEFAULTS its flags to
// M_NOWAIT four lines up, so that one is a real leak - but the rule does
// not know that and must not claim to.
@kflags@
expression P, SZ, ty;
identifier fl != {M_WAITOK, M_NOWAIT};
position p;
@@
(
  P@p = realloc(P, SZ, ty, fl)
|
  P@p = realloc(P, SZ, ty, fl | M_ZERO)
)

@script:python@
p << kflags.p;
@@
print("PBSD\t%s\t%s\t%s\trealloc-self-kflags\tkernel realloc assigned back onto its own pointer with a variable flag word; a leak if that word can be M_NOWAIT"
      % (p[0].file, p[0].line, p[0].column))

@script:python@
p << self.p;
@@
print("PBSD\t%s\t%s\t%s\trealloc-self\trealloc assigned back onto its own pointer; on failure the block leaks and the pointer is NULL"
      % (p[0].file, p[0].line, p[0].column))

@script:python@
p << self2.p;
@@
print("PBSD\t%s\t%s\t%s\trealloc-self\trealloc assigned back onto its own field; on failure the block leaks and the field is NULL"
      % (p[0].file, p[0].line, p[0].column))
