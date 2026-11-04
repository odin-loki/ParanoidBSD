# Wave 0 dual-link kmod — build on HARDENEDBSD/FreeBSD host:
#   make -C sys/modules/pbsd_cxx_hello
#
# Sources live in the PBSD tree; NORMAL_CXX / CXXSTD=c++23 come from kern.mk.

PBSD_KERNEL=	${.CURDIR}/../../../../../pbsd/kernel
.PATH:		${PBSD_KERNEL}

KMOD=	pbsd_cxx_hello
SRCS=	pbsd_cxx_hello.cpp pbsd_cxx_hello.c

CFLAGS+=	-I${PBSD_KERNEL}
CXXFLAGS+=	-I${PBSD_KERNEL}

.include <bsd.kmod.mk>
