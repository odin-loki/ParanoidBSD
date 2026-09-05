SUBDIR_PARALLEL=	yes

PACKAGE=		bootloader

# Firmware may not be able to handle branch protection failures
MK_BRANCH_PROTECTION=	no
_HBSD_EXTRA_HARDENING_DISABLED=	1

.include "defs.mk"
