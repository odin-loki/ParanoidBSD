# div.S, ldiv.S and lldiv.S were removed with the other 74 machine-dependent
# .S files in lib/libc (commit b1bd99b89), each having a machine-independent
# .c counterpart in lib/libc/stdlib. Their MDSRCS line was not removed with
# them, so this file named three sources that were not on disk. Invisible
# under PBSD's WITHOUT_MACHDEP_OPTIMIZATIONS, which drops MDSRCS whole, and a
# build failure the moment anything asks for the assembly back.
#
# Left as an empty stub rather than deleted because lib/libc/Makefile
# includes ${LIBC_ARCH}/stdlib/Makefile.inc unconditionally.
