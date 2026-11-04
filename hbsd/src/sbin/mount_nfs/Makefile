PACKAGE=nfs
PROG=	mount_nfs
SRCS=	mount_nfs.c mounttab.c
MAN=	mount_nfs.8
LIBADD=	util

UMNTALL= ${SRCTOP}/usr.sbin/rpc.umntall
CFLAGS+= -DNFS -I${UMNTALL}

CFI_OVERRIDE=	-fno-sanitize=cfi-icall

.PATH: ${UMNTALL}

.include <bsd.prog.mk>
