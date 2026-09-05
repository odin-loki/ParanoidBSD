SHLIB=		lattzfs
SHLIB_MAJOR=	0

.PATH:		${.CURDIR}/include
.PATH:		${.CURDIR}/src

SRCS+=		liblattzfs.c

INCS+=		liblattzfs.h

CFLAGS+=	-I${.CURDIR}/include
CFLAGS+=	-DIN_BASE -DHAVE_RPC_TYPES
CFLAGS+= 	-I${SRCTOP}/sys/contrib/openzfs/include
CFLAGS+= 	-I${SRCTOP}/sys/contrib/openzfs/include/os/freebsd
CFLAGS+= 	-I${SRCTOP}/sys/contrib/openzfs/lib/libspl/include
CFLAGS+= 	-I${SRCTOP}/sys/contrib/openzfs/lib/libspl/include/os/freebsd
CFLAGS+= 	-I${SRCTOP}/sys
CFLAGS+= 	-I${SRCTOP}/cddl/compat/opensolaris/include
CFLAGS+= 	-include ${SRCTOP}/sys/contrib/openzfs/include/os/freebsd/spl/sys/ccompile.h
CFLAGS+= 	-DHAVE_ISSETUGID

LDADD+=		-lzfs -lnvpair -lspl

.if defined(PREFIX)
INCLUDEDIR=	${PREFIX}/include
LIBDIR=		${PREFIX}/lib
.endif

.include <bsd.lib.mk>
