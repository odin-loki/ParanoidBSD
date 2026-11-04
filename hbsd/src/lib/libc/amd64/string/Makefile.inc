# EMPTY, deliberately.
#
# This listed eight machine-dependent C files, and every one of them was a
# thin wrapper calling into assembly that PBSD's assembly removal deleted:
#
#   strcpy.c   -> __stpcpy      strlcat.c  -> __memchr, __strlcpy
#   strncat.c  -> __memccpy     strncpy.c  -> __stpncpy
#   strnlen.c  -> __memchr      strpbrk.c  -> __strcspn
#   strsep.c   -> __strcspn
#
# Each carries a PROTOTYPE for the symbol it calls, and nothing in the tree
# defines any of them any more. amd64_archlevel.c is the ifunc arch-level
# detector those wrappers dispatched through, and nothing else uses it.
#
# lib/libc/Makefile:159 reads
#
#   .if empty(MDSRCS) || ${MK_MACHDEP_OPTIMIZATIONS} == no
#   SRCS+=  ${MISRCS}
#
# so with hbsd/src.conf.pbsd's WITHOUT_MACHDEP_OPTIMIZATIONS=YES none of it
# was ever selected, and the breakage was invisible to every build PBSD
# does. Boot image run 46 asked for the vendor's defaults as a bisection
# control and buildworld died linking the first static binary:
#
#   ld.lld: error: undefined symbol: __stpcpy
#   >>> referenced by strcpy.c:35 (lib/libc/amd64/string/strcpy.c:35)
#
# The machine-independent C in lib/libc/string is present for all of them
# and is what the assembly removal was for. The .c files are left on disk
# rather than deleted: they are upstream's, and an empty MDSRCS is a
# reversible statement about what this tree builds.
#
# tools/check_machdep_symbols.py checks this does not come back.
