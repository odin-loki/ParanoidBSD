#!/usr/bin/env python3
"""Generate oracle.c and port.cppm for b0047."""

import re
from pathlib import Path

ROOT = Path("/home/odin/pbsd/hbsd/src/lib/libc/softfloat")
OUT = Path("/home/odin/pbsd/pbsd/lib/libc/softfloat/bits32/b0047")

SKIP_FUNCS = {"float64_round_to_int"}

COPYRIGHT = read_src = lambda p: (ROOT / p).read_text()

def read(p):
    return (ROOT / p).read_text()

def remove_ifdef_blocks(text, macro):
    out, i, lines = [], 0, text.splitlines()
    while i < len(lines):
        line = lines[i]
        if re.match(rf'^\s*#ifdef\s+{re.escape(macro)}\s*$', line):
            depth, i = 1, i + 1
            while i < len(lines) and depth:
                if re.match(r'^\s*#ifdef\b', lines[i]):
                    depth += 1
                elif re.match(r'^\s*#endif\b', lines[i]):
                    depth -= 1
                i += 1
            continue
        out.append(line)
        i += 1
    return "\n".join(out)

def remove_skipped_functions(text):
    lines = text.splitlines()
    out, i = [], 0
    while i < len(lines):
        m = re.match(r'^[\w\s\*]+\s+(\w+)\s*\(', lines[i])
        if m and m.group(1) in SKIP_FUNCS:
            depth = 0
            while i < len(lines):
                depth += lines[i].count('{') - lines[i].count('}')
                i += 1
                if depth <= 0 and '{' in ''.join(out[-5:] + lines[i-3:i]):
                    break
            continue
        out.append(lines[i])
        i += 1
    return "\n".join(out)

def strip_softfloat_c(text):
    text = remove_ifdef_blocks(text, "SOFTFLOAT_FOR_GCC")
    text = remove_skipped_functions(text)
    # drop includes and #include lines
    lines = [ln for ln in text.splitlines()
             if not re.match(r'^\s*#include\s', ln)]
    text = "\n".join(lines)
    # drop preamble through globals and macro/specialize includes
    marker = '#include "softfloat-specialize"'
    idx = text.find(marker)
    if idx != -1:
        text = text[idx + len(marker):]
    return text.lstrip("\n")

def strip_specialize(text):
    text = remove_ifdef_blocks(text, "SOFTFLOAT_FOR_GCC")
    text = remove_ifdef_blocks(text, "FLOATX80")
    text = remove_ifdef_blocks(text, "FLOAT128")
    # float_detect_tininess / float_exception_mask are defined in our globals block.
    text = re.sub(
        r'/\*[\s\S]*?Underflow tininess[\s\S]*?\*/\s*'
        r'(?:#ifdef __sparc64__[\s\S]*?#endif\s*)?',
        '', text, count=1)
    text = re.sub(r'^\s*int\s+float_exception_mask\s*=\s*0\s*;\s*$', '', text, flags=re.M)
    text = re.sub(r'^\s*#undef float_exception_mask\s*$', '', text, flags=re.M)
    text = re.sub(r'^\s*#define float32_default_nan\b.*$', '', text, flags=re.M)
    text = re.sub(r'^\s*#define float64_default_nan\b.*$', '', text, flags=re.M)
    return text

FUNC_RENAMES = [
    "normalizeFloat32Subnormal", "roundAndPackFloat32", "normalizeRoundAndPackFloat32",
    "extractFloat32Frac", "extractFloat32Exp", "extractFloat32Sign", "packFloat32",
    "normalizeFloat64Subnormal", "roundAndPackFloat64", "normalizeRoundAndPackFloat64",
    "extractFloat64Frac1", "extractFloat64Frac0", "extractFloat64Exp", "extractFloat64Sign",
    "packFloat64",
    "shift32RightJamming", "shift64Right", "shift64RightJamming", "shift64ExtraRightJamming",
    "shortShift64Left", "shortShift96Left", "add64", "add96", "sub64", "sub96",
    "mul32To64", "mul64By32To96", "mul64To128", "estimateDiv64To32", "estimateSqrt32",
    "countLeadingZeros32", "eq64", "le64", "lt64", "ne64",
    "float_raise", "float32_is_nan", "float32_is_signaling_nan", "float32ToCommonNaN",
    "commonNaNToFloat32", "propagateFloat32NaN",
    "float64_is_nan", "float64_is_signaling_nan", "float64ToCommonNaN",
    "commonNaNToFloat64", "propagateFloat64NaN",
    "addFloat32Sigs", "subFloat32Sigs", "addFloat64Sigs", "subFloat64Sigs",
    "int32_to_float32", "int32_to_float64",
    "float32_to_int32", "float32_to_int32_round_to_zero", "float32_to_float64",
    "float32_round_to_int", "float32_add", "float32_sub", "float32_mul", "float32_div",
    "float32_rem", "float32_sqrt",
    "float32_eq", "float32_le", "float32_lt",
    "float32_eq_signaling", "float32_le_quiet", "float32_lt_quiet",
    "float64_to_int32", "float64_to_int32_round_to_zero", "float64_to_float32",
    "float64_add", "float64_sub", "float64_mul", "float64_div",
    "float64_rem", "float64_sqrt",
    "float64_eq", "float64_le", "float64_lt",
    "float64_eq_signaling", "float64_le_quiet", "float64_lt_quiet",
]

def func_rename_block():
    return "\n".join(f"#define {n} ref_{n}" for n in FUNC_RENAMES)

TYPES = r'''
#include <signal.h>
#include <string.h>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

typedef int flag;
typedef int uint8;
typedef int int8;
typedef int uint16;
typedef int int16;
typedef unsigned int uint32;
typedef signed int int32;
typedef unsigned long long int uint64;
typedef signed long long int int64;
typedef unsigned char bits8;
typedef signed char sbits8;
typedef unsigned short int bits16;
typedef signed short int sbits16;
typedef unsigned int bits32;
typedef signed int sbits32;
typedef unsigned long long int bits64;
typedef signed long long int sbits64;

#define LIT64(a) a##ULL
#define INLINE static

enum { FALSE = 0, TRUE = 1 };

typedef bits32 float32;
typedef bits64 float64;

#ifndef FLOAT64_DEMANGLE
#define FLOAT64_DEMANGLE(a) (a)
#endif
#ifndef FLOAT64_MANGLE
#define FLOAT64_MANGLE(a) (a)
#endif

enum {
    float_tininess_after_rounding  = 0,
    float_tininess_before_rounding = 1
};
enum {
    float_round_nearest_even = 0,
    float_round_to_zero      = 1,
    float_round_down         = 2,
    float_round_up           = 3
};
enum {
    float_flag_inexact   =  1,
    float_flag_underflow =  2,
    float_flag_overflow  =  4,
    float_flag_divbyzero =  8,
    float_flag_invalid   = 16
};

#define float32_default_nan 0xFFFFFFFFu
#define float64_default_nan LIT64(0xFFFFFFFFFFFFFFFF)

'''

def gen_oracle():
    macros = read("bits32/softfloat-macros")
    specialize = strip_specialize(read("softfloat-specialize"))
    body = strip_softfloat_c(read("bits32/softfloat.c"))

    globals_ = r'''
int __oracle_float_rounding_mode = float_round_nearest_even;
int __oracle_float_exception_flags = 0;
int8 __oracle_float_detect_tininess = float_tininess_after_rounding;
int __oracle_float_exception_mask = 0;

#define float_rounding_mode __oracle_float_rounding_mode
#define float_exception_flags __oracle_float_exception_flags
#define float_detect_tininess __oracle_float_detect_tininess
#define float_exception_mask __oracle_float_exception_mask

'''

    oracle = (
        "/* oracle.c -- PBSD batch b0047 reference specification. */\n"
        + TYPES + globals_ + func_rename_block() + "\n\n"
        + macros + "\n" + specialize + "\n" + body
    )
    (OUT / "oracle.c").write_text(oracle)

def gen_port():
    macros = read("bits32/softfloat-macros")
    specialize = strip_specialize(read("softfloat-specialize"))
    body = strip_softfloat_c(read("bits32/softfloat.c"))

    header = '''/* $NetBSD: softfloat.c,v 1.1 2002/05/21 23:51:07 bjh21 Exp $ */
export module pbsd.lib.libc.softfloat.bits32.b0047;

export namespace pbsd::lib_libc_softfloat_bits32::b0047 {

#include <csignal>
#include <cstdint>
#include <cstring>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

using flag = int;
using uint8 = int;
using int8 = int;
using uint16 = int;
using int16 = int;
using uint32 = unsigned int;
using int32 = signed int;
using uint64 = unsigned long long int;
using int64 = signed long long int;
using bits8 = unsigned char;
using sbits8 = signed char;
using bits16 = unsigned short int;
using sbits16 = signed short int;
using bits32 = unsigned int;
using sbits32 = signed int;
using bits64 = unsigned long long int;
using sbits64 = signed long long int;

#define LIT64(a) a##ULL
#define INLINE static inline

enum { FALSE = 0, TRUE = 1 };

using float32 = bits32;
using float64 = bits64;

#ifndef FLOAT64_DEMANGLE
#define FLOAT64_DEMANGLE(a) (a)
#endif
#ifndef FLOAT64_MANGLE
#define FLOAT64_MANGLE(a) (a)
#endif

enum {
    float_tininess_after_rounding  = 0,
    float_tininess_before_rounding = 1
};
enum {
    float_round_nearest_even = 0,
    float_round_to_zero      = 1,
    float_round_down         = 2,
    float_round_up           = 3
};
enum {
    float_flag_inexact   =  1,
    float_flag_underflow =  2,
    float_flag_overflow  =  4,
    float_flag_divbyzero =  8,
    float_flag_invalid   = 16
};

#define float32_default_nan 0xFFFFFFFFu
#define float64_default_nan LIT64(0xFFFFFFFFFFFFFFFF)

int __port_float_rounding_mode = float_round_nearest_even;
int __port_float_exception_flags = 0;
int8 __port_float_detect_tininess = float_tininess_after_rounding;
int __port_float_exception_mask = 0;

#define float_rounding_mode __port_float_rounding_mode
#define float_exception_flags __port_float_exception_flags
#define float_detect_tininess __port_float_detect_tininess
#define float_exception_mask __port_float_exception_mask

'''

    port = header + macros + "\n" + specialize + "\n" + body + "\n} // namespace\n"
    (OUT / "port.cppm").write_text(port)

def gen_skipped():
    (OUT / "skipped.txt").write_text(
        "float64_round_to_int: uses .high/.low members on bits64 float64; source does not compile\n"
    )

if __name__ == "__main__":
    gen_oracle()
    gen_port()
    gen_skipped()
    print("done")
