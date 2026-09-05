# div.S and ldiv.S were removed with the other 74 machine-dependent .S files
# in lib/libc (commit b1bd99b89), each having a machine-independent .c
# counterpart in lib/libc/stdlib. Their MDSRCS line was not removed with them,
# so this file named two sources that were not on disk. See the note in
# amd64/stdlib/Makefile.inc.
