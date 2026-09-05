/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 Odin Loch <odin.loch@outlook.com.au>
 */

/*
 * PBSD: <machine/_inttypes.h>, once.
 *
 * Five copies, 1,071 lines, and every one is the same 154 conversion
 * macros with "l" and "ll" moved around according to whether int64_t is
 * long or long long on that target. The compiler answers that per type and
 * per conversion - __INT64_FMTd__ is "ld" on amd64 and "lld" on i386 - so
 * the table is written once here in terms of those predefines.
 *
 * The one thing that cannot be copied straight across is which predefine
 * goes where, and getting it wrong is not cosmetic:
 *
 *   PRI is for printf, where the argument has already been promoted. An
 *   int8_t arrives as an int, so PRId8 is "d". FreeBSD says so and it is
 *   right.
 *
 *   SCN is for scanf, which is handed a pointer and must be told the real
 *   width. SCNd8 is "hhd". "d" there would write four bytes through an
 *   int8_t *.
 *
 * So PRI for everything that promotes to int - 8, 16, 32 and their LEAST
 * and FAST forms - is the int conversion, and SCN follows the actual type.
 * There are no SCNX macros: %X is a printf conversion and C99 7.8.1 does
 * not give scanf one. The generator produced fourteen of them and the
 * checker reported them as macros no architecture defines, which is what
 * that column is for.
 * The first draft of this file made PRI and SCN identical from clang's
 * per-type predefines, which is how PRId8 came out "hhd";
 * tools/stdint_generic_check.py caught it against the five headers it is
 * meant to replace.
 *
 * FreeBSD's FAST types are int32_t and int64_t on all six architectures,
 * which is not what clang thinks - clang's int_fast8_t is a char. The FAST
 * rows below therefore use the 32- and 64-bit predefines rather than
 * clang's own FAST ones. Same reason, found the same way.
 */

#ifndef _SYS__INTTYPES_GENERIC_H_
#define	_SYS__INTTYPES_GENERIC_H_

/* fprintf(3): the argument is promoted, so 8 and 16 use the int conversion */
#define	PRId8           	__INT32_FMTd__
#define	PRId16          	__INT32_FMTd__
#define	PRId32          	__INT32_FMTd__
#define	PRId64          	__INT64_FMTd__
#define	PRIdLEAST8      	__INT32_FMTd__
#define	PRIdLEAST16     	__INT32_FMTd__
#define	PRIdLEAST32     	__INT32_FMTd__
#define	PRIdLEAST64     	__INT64_FMTd__
#define	PRIdFAST8       	__INT32_FMTd__
#define	PRIdFAST16      	__INT32_FMTd__
#define	PRIdFAST32      	__INT32_FMTd__
#define	PRIdFAST64      	__INT64_FMTd__
#define	PRIdMAX         	"jd"
#define	PRIdPTR         	__INTPTR_FMTd__

#define	PRIi8           	__INT32_FMTi__
#define	PRIi16          	__INT32_FMTi__
#define	PRIi32          	__INT32_FMTi__
#define	PRIi64          	__INT64_FMTi__
#define	PRIiLEAST8      	__INT32_FMTi__
#define	PRIiLEAST16     	__INT32_FMTi__
#define	PRIiLEAST32     	__INT32_FMTi__
#define	PRIiLEAST64     	__INT64_FMTi__
#define	PRIiFAST8       	__INT32_FMTi__
#define	PRIiFAST16      	__INT32_FMTi__
#define	PRIiFAST32      	__INT32_FMTi__
#define	PRIiFAST64      	__INT64_FMTi__
#define	PRIiMAX         	"ji"
#define	PRIiPTR         	__INTPTR_FMTi__

#define	PRIo8           	__UINT32_FMTo__
#define	PRIo16          	__UINT32_FMTo__
#define	PRIo32          	__UINT32_FMTo__
#define	PRIo64          	__UINT64_FMTo__
#define	PRIoLEAST8      	__UINT32_FMTo__
#define	PRIoLEAST16     	__UINT32_FMTo__
#define	PRIoLEAST32     	__UINT32_FMTo__
#define	PRIoLEAST64     	__UINT64_FMTo__
#define	PRIoFAST8       	__UINT32_FMTo__
#define	PRIoFAST16      	__UINT32_FMTo__
#define	PRIoFAST32      	__UINT32_FMTo__
#define	PRIoFAST64      	__UINT64_FMTo__
#define	PRIoMAX         	"jo"
#define	PRIoPTR         	__UINTPTR_FMTo__

#define	PRIu8           	__UINT32_FMTu__
#define	PRIu16          	__UINT32_FMTu__
#define	PRIu32          	__UINT32_FMTu__
#define	PRIu64          	__UINT64_FMTu__
#define	PRIuLEAST8      	__UINT32_FMTu__
#define	PRIuLEAST16     	__UINT32_FMTu__
#define	PRIuLEAST32     	__UINT32_FMTu__
#define	PRIuLEAST64     	__UINT64_FMTu__
#define	PRIuFAST8       	__UINT32_FMTu__
#define	PRIuFAST16      	__UINT32_FMTu__
#define	PRIuFAST32      	__UINT32_FMTu__
#define	PRIuFAST64      	__UINT64_FMTu__
#define	PRIuMAX         	"ju"
#define	PRIuPTR         	__UINTPTR_FMTu__

#define	PRIx8           	__UINT32_FMTx__
#define	PRIx16          	__UINT32_FMTx__
#define	PRIx32          	__UINT32_FMTx__
#define	PRIx64          	__UINT64_FMTx__
#define	PRIxLEAST8      	__UINT32_FMTx__
#define	PRIxLEAST16     	__UINT32_FMTx__
#define	PRIxLEAST32     	__UINT32_FMTx__
#define	PRIxLEAST64     	__UINT64_FMTx__
#define	PRIxFAST8       	__UINT32_FMTx__
#define	PRIxFAST16      	__UINT32_FMTx__
#define	PRIxFAST32      	__UINT32_FMTx__
#define	PRIxFAST64      	__UINT64_FMTx__
#define	PRIxMAX         	"jx"
#define	PRIxPTR         	__UINTPTR_FMTx__

#define	PRIX8           	__UINT32_FMTX__
#define	PRIX16          	__UINT32_FMTX__
#define	PRIX32          	__UINT32_FMTX__
#define	PRIX64          	__UINT64_FMTX__
#define	PRIXLEAST8      	__UINT32_FMTX__
#define	PRIXLEAST16     	__UINT32_FMTX__
#define	PRIXLEAST32     	__UINT32_FMTX__
#define	PRIXLEAST64     	__UINT64_FMTX__
#define	PRIXFAST8       	__UINT32_FMTX__
#define	PRIXFAST16      	__UINT32_FMTX__
#define	PRIXFAST32      	__UINT32_FMTX__
#define	PRIXFAST64      	__UINT64_FMTX__
#define	PRIXMAX         	"jX"
#define	PRIXPTR         	__UINTPTR_FMTX__

/* fscanf(3): the argument is a pointer, so the width is the real one */
#define	SCNd8           	__INT8_FMTd__
#define	SCNd16          	__INT16_FMTd__
#define	SCNd32          	__INT32_FMTd__
#define	SCNd64          	__INT64_FMTd__
#define	SCNdLEAST8      	__INT8_FMTd__
#define	SCNdLEAST16     	__INT16_FMTd__
#define	SCNdLEAST32     	__INT32_FMTd__
#define	SCNdLEAST64     	__INT64_FMTd__
#define	SCNdFAST8       	__INT32_FMTd__
#define	SCNdFAST16      	__INT32_FMTd__
#define	SCNdFAST32      	__INT32_FMTd__
#define	SCNdFAST64      	__INT64_FMTd__
#define	SCNdMAX         	"jd"
#define	SCNdPTR         	__INTPTR_FMTd__

#define	SCNi8           	__INT8_FMTi__
#define	SCNi16          	__INT16_FMTi__
#define	SCNi32          	__INT32_FMTi__
#define	SCNi64          	__INT64_FMTi__
#define	SCNiLEAST8      	__INT8_FMTi__
#define	SCNiLEAST16     	__INT16_FMTi__
#define	SCNiLEAST32     	__INT32_FMTi__
#define	SCNiLEAST64     	__INT64_FMTi__
#define	SCNiFAST8       	__INT32_FMTi__
#define	SCNiFAST16      	__INT32_FMTi__
#define	SCNiFAST32      	__INT32_FMTi__
#define	SCNiFAST64      	__INT64_FMTi__
#define	SCNiMAX         	"ji"
#define	SCNiPTR         	__INTPTR_FMTi__

#define	SCNo8           	__UINT8_FMTo__
#define	SCNo16          	__UINT16_FMTo__
#define	SCNo32          	__UINT32_FMTo__
#define	SCNo64          	__UINT64_FMTo__
#define	SCNoLEAST8      	__UINT8_FMTo__
#define	SCNoLEAST16     	__UINT16_FMTo__
#define	SCNoLEAST32     	__UINT32_FMTo__
#define	SCNoLEAST64     	__UINT64_FMTo__
#define	SCNoFAST8       	__UINT32_FMTo__
#define	SCNoFAST16      	__UINT32_FMTo__
#define	SCNoFAST32      	__UINT32_FMTo__
#define	SCNoFAST64      	__UINT64_FMTo__
#define	SCNoMAX         	"jo"
#define	SCNoPTR         	__UINTPTR_FMTo__

#define	SCNu8           	__UINT8_FMTu__
#define	SCNu16          	__UINT16_FMTu__
#define	SCNu32          	__UINT32_FMTu__
#define	SCNu64          	__UINT64_FMTu__
#define	SCNuLEAST8      	__UINT8_FMTu__
#define	SCNuLEAST16     	__UINT16_FMTu__
#define	SCNuLEAST32     	__UINT32_FMTu__
#define	SCNuLEAST64     	__UINT64_FMTu__
#define	SCNuFAST8       	__UINT32_FMTu__
#define	SCNuFAST16      	__UINT32_FMTu__
#define	SCNuFAST32      	__UINT32_FMTu__
#define	SCNuFAST64      	__UINT64_FMTu__
#define	SCNuMAX         	"ju"
#define	SCNuPTR         	__UINTPTR_FMTu__

#define	SCNx8           	__UINT8_FMTx__
#define	SCNx16          	__UINT16_FMTx__
#define	SCNx32          	__UINT32_FMTx__
#define	SCNx64          	__UINT64_FMTx__
#define	SCNxLEAST8      	__UINT8_FMTx__
#define	SCNxLEAST16     	__UINT16_FMTx__
#define	SCNxLEAST32     	__UINT32_FMTx__
#define	SCNxLEAST64     	__UINT64_FMTx__
#define	SCNxFAST8       	__UINT32_FMTx__
#define	SCNxFAST16      	__UINT32_FMTx__
#define	SCNxFAST32      	__UINT32_FMTx__
#define	SCNxFAST64      	__UINT64_FMTx__
#define	SCNxMAX         	"jx"
#define	SCNxPTR         	__UINTPTR_FMTx__


#endif /* _SYS__INTTYPES_GENERIC_H_ */
