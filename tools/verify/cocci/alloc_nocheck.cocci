/// SPDX-License-Identifier: AGPL-3.0-or-later
/// An M_NOWAIT allocation dereferenced with no NULL test in between.
///
/// docs/security/UB_FINDINGS.md: "M_WAITOK cannot fail. M_NOWAIT can, and
/// returns NULL."  It also records why the token-based lint that exists
/// for this - tools/verify/nowait_check.py - could not see the bug it was
/// written to find: it tested `if "M_NOWAIT" not in stmt', and SCTP_MALLOC
/// puts the flag inside the MACRO BODY, so every SCTP allocation was
/// skipped.  IEEE80211_MALLOC spells it IEEE80211_M_NOWAIT and was skipped
/// the same way.
///
/// A semantic patch does not have that failure mode for the shapes it
/// names, because it matches the CALL and the DEREFERENCE and the absence
/// of a test between them, not a substring of a line.  The wrapper macros
/// are named here explicitly for the same reason the token test missed
/// them: the flag is not visible at the call site.
//
// Confidence: Moderate
// Options: --no-includes
virtual report

// The test written INTO the assignment.  `if ((cd = malloc(..)) == NULL)'
// is a NULL test, and the `when !=' below cannot see it because it is not
// a statement between the call and the use - it is the same node.  This
// rule collects those positions so the next one can exclude them; without
// it, sys/x86/x86/legacy.c:360 was reported, and it is correct code.
@checked@
expression E, sz, ty, fl;
position pa;
@@
(
  (E = malloc@pa(sz, ty, fl)) == NULL
|
  (E = malloc@pa(sz, ty, fl)) != NULL
|
  !(E = malloc@pa(sz, ty, fl))
|
  (E = mallocarray@pa(sz, fl, ty, ...)) == NULL
|
  (E = uma_zalloc@pa(sz, fl)) == NULL
|
  (E = uma_zalloc@pa(sz, fl)) != NULL
|
  (E = IEEE80211_MALLOC@pa(sz, ty, fl)) == NULL
)

@nowait@
expression E, sz, ty, fl;
identifier fld;
position p;
position pc != checked.pa;
@@
(
  E = malloc@pc(sz, ty, \(M_NOWAIT\|M_NOWAIT | M_ZERO\|M_ZERO | M_NOWAIT\))
|
  E = mallocarray@pc(sz, fl, ty, \(M_NOWAIT\|M_NOWAIT | M_ZERO\|M_ZERO | M_NOWAIT\))
|
  E = IEEE80211_MALLOC@pc(sz, ty, \(IEEE80211_M_NOWAIT\|IEEE80211_M_NOWAIT | IEEE80211_M_ZERO\))
|
  E = uma_zalloc@pc(sz, \(M_NOWAIT\|M_NOWAIT | M_ZERO\))
|
  E = uma_zalloc_arg@pc(sz, ty, \(M_NOWAIT\|M_NOWAIT | M_ZERO\))
)
... when != E == NULL
    when != E != NULL
    when != !E
    when != E ? sz : ty
    when != return ...;
    when != E = ty
(
  E@p->fld
|
  E@p[sz]
|
  *E@p
)

@script:python@
p << nowait.p;
@@
print("PBSD\t%s\t%s\t%s\tnowait-deref\tan M_NOWAIT allocation is dereferenced with no NULL test between the call and the use"
      % (p[0].file, p[0].line, p[0].column))
