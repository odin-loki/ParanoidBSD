# bcopy, memcpy and memmove are gone from here. Their templates - bcopy.S,
# memcpy.S and memmove.S - were removed with the other 74 machine-dependent
# .S files in lib/libc (commit b1bd99b89), each having a machine-independent
# .c counterpart. What was left behind was everything built ON them: the
# three _vsx.S files, which are macro definitions ending in
# `#include "bcopy.S"`, and the three _resolver.c files, whose ifuncs choose
# between __bcopy_vsx and the __bcopy that bcopy.S defined. Neither half can
# build without the templates.
#
# strcpy and strncpy keep their POWER ISA 2.05 variants: those are complete
# on disk and include nothing that is missing.
MDSRCS+= \
	strncpy_arch_2_05.S \
	strncpy.c \
	strncpy_resolver.c \
	strcpy_arch_2_05.S \
	strcpy.c \
	strcpy_resolver.c
