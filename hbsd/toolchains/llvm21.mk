# External LLVM 21 toolchain for PBSD.
#
# Used as CROSS_TOOLCHAIN=<path to this file>. Makefile.inc1 line 66 includes
# it before anything else, so every X* variable below is what the build uses.
#
# Why external at all: contrib/llvm-project is 14,337 files, 290 MB and 13% of
# the tracked tree, and PBSD already maintains it separately - it carries LLVM
# 21.1.8 while HardenedBSD 15-stable is on 19.x. Building a compiler in order
# to build the system is the largest single cost in the tree, and it buys
# self-hosting, which is worth having deliberately rather than by default.
#
# The version must match what lib/clang/include/llvm/Support/VCSRevision.h
# records, or the ports built against the in-tree headers and the ones built
# by this compiler disagree. tools/check_toolchain_version.py checks that.

LLVM_VERSION?=	21
LOCALBASE?=	/usr/local

XCC=		${LOCALBASE}/bin/clang${LLVM_VERSION}
XCXX=		${LOCALBASE}/bin/clang++${LLVM_VERSION}
XCPP=		${LOCALBASE}/bin/clang-cpp${LLVM_VERSION}
XLD=		${LOCALBASE}/bin/ld.lld${LLVM_VERSION}
XAS=		${LOCALBASE}/bin/clang${LLVM_VERSION}

XAR=		${LOCALBASE}/bin/llvm-ar${LLVM_VERSION}
XNM=		${LOCALBASE}/bin/llvm-nm${LLVM_VERSION}
XOBJCOPY=	${LOCALBASE}/bin/llvm-objcopy${LLVM_VERSION}
XOBJDUMP=	${LOCALBASE}/bin/llvm-objdump${LLVM_VERSION}
XRANLIB=	${LOCALBASE}/bin/llvm-ranlib${LLVM_VERSION}
XREADELF=	${LOCALBASE}/bin/llvm-readelf${LLVM_VERSION}
XSIZE=		${LOCALBASE}/bin/llvm-size${LLVM_VERSION}
XSTRINGS=	${LOCALBASE}/bin/llvm-strings${LLVM_VERSION}
XSTRIPBIN=	${LOCALBASE}/bin/llvm-strip${LLVM_VERSION}

CROSS_BINUTILS_PREFIX=	${LOCALBASE}/bin/
