#!/usr/bin/env python3
"""Generate oracle.c, port.cppm, harness.cpp for b0047."""

import re
from pathlib import Path

ROOT = Path("/home/odin/pbsd/hbsd/src/lib/libc/softfloat")
OUT = Path("/home/odin/pbsd/pbsd/lib/libc/softfloat/bits32/b0047")
SKIP = {"float64_round_to_int": "uses .high/.low members on bits64 float64; source does not compile"}

# Functions the harness calls (non-static, not skipped).
PUBLIC_FUNCS = [
    "int32_to_float32", "int32_to_float64",
    "float32_to_int32", "float32_to_int32_round_to_zero", "float32_to_float64",
    "float32_round_to_int",
    "float32_add", "float32_sub", "float32_mul", "float32_div",
    "float32_rem", "float32_sqrt",
    "float32_eq", "float32_le", "float32_lt",
    "float32_eq_signaling", "float32_le_quiet", "float32_lt_quiet",
    "float64_to_int32", "float64_to_int32_round_to_zero", "float64_to_float32",
    "float64_add", "float64_sub", "float64_mul", "float64_div",
    "float64_rem", "float64_sqrt",
    "float64_eq", "float64_le", "float64_lt",
    "float64_eq_signaling", "float64_le_quiet", "float64_lt_quiet",
]

STATIC_FUNCS = [
    "normalizeFloat32Subnormal", "roundAndPackFloat32", "normalizeRoundAndPackFloat32",
    "normalizeFloat64Subnormal", "roundAndPackFloat64", "normalizeRoundAndPackFloat64",
    "addFloat32Sigs", "subFloat32Sigs", "addFloat64Sigs", "subFloat64Sigs",
    "shift32RightJamming", "shift64Right", "shift64RightJamming", "shift64ExtraRightJamming",
    "shortShift64Left", "shortShift96Left", "add64", "add96", "sub64", "sub96",
    "mul32To64", "mul64By32To96", "mul64To128", "estimateDiv64To32", "estimateSqrt32",
    "countLeadingZeros32", "eq64", "le64", "lt64", "ne64",
    "extractFloat32Frac", "extractFloat32Exp", "extractFloat32Sign", "packFloat32",
    "extractFloat64Frac1", "extractFloat64Frac0", "extractFloat64Exp", "extractFloat64Sign",
    "packFloat64",
    "float_raise", "float32_is_nan", "float32_is_signaling_nan", "float32ToCommonNaN",
    "commonNaNToFloat32", "propagateFloat32NaN",
    "float64_is_nan", "float64_is_signaling_nan", "float64ToCommonNaN",
    "commonNaNToFloat64", "propagateFloat64NaN",
]

RENAME_GLOBALS = {
    "float_rounding_mode": "__oracle_float_rounding_mode",
    "float_exception_flags": "__oracle_float_exception_flags",
    "float_detect_tininess": "__oracle_float_detect_tininess",
    "float_exception_mask": "__oracle_float_exception_mask",
}

RENAME_FUNCS = {f: f"ref_{f}" for f in PUBLIC_FUNCS + STATIC_FUNCS}
RENAME_FUNCS.update({k: f"ref_{v}" for k, v in [
    ("float_raise", "float_raise"),
]})

def read_src(name):
    return (ROOT / name).read_text()

def strip_includes(text):
    lines = []
    for line in text.splitlines():
        if re.match(r'^\s*#include\s', line):
            continue
        if re.match(r'^\s*#ifdef SOFTFLOAT_FOR_GCC', line):
            continue
        if line.strip() == '#endif' and 'SOFTFLOAT_FOR_GCC' in text[:text.find(line)]:
            pass
        lines.append(line)
    return "\n".join(lines)

def preprocess_softfloat_c(text):
    """Remove includes and SOFTFLOAT_FOR_GCC blocks; drop skipped functions."""
    out = []
    skip_depth = 0
    skip_func = False
    func_depth = 0
    i = 0
    lines = text.splitlines()
    while i < len(lines):
        line = lines[i]
        if re.match(r'^\s*#include\s', line):
            i += 1
            continue
        if '#ifdef SOFTFLOAT_FOR_GCC' in line:
            skip_depth += 1
            i += 1
            continue
        if skip_depth > 0 and line.strip() == '#endif':
            skip_depth -= 1
            i += 1
            continue
        if skip_depth > 0:
            i += 1
            continue
        m = re.match(r'^(\w[\w\s\*]*?)\s+(float64_round_to_int)\s*\(', line)
        if m and m.group(2) in SKIP:
            skip_func = True
            func_depth = 0
            i += 1
            continue
        if skip_func:
            func_depth += line.count('{') - line.count('}')
            if func_depth <= 0 and '{' in ''.join(lines[max(0,i-20):i+1]):
                if line.strip() == '}' or (func_depth == 0 and '}' in line):
                    skip_func = False
            i += 1
            continue
        out.append(line)
        i += 1
    return "\n".join(out)

def apply_renames(text, funcs, globals_):
    for old, new in sorted(globals_.items(), key=lambda x: -len(x[0])):
        text = re.sub(r'\b' + re.escape(old) + r'\b', new, text)
    for old, new in sorted(funcs.items(), key=lambda x: -len(x[0])):
        text = re.sub(r'\b' + re.escape(old) + r'\b', new, text)
    return text

ORACLE_HEADER = r'''/*
 * oracle.c -- reference specification for PBSD batch b0047.
 *
 * hbsd/src/lib/libc/softfloat/bits32/softfloat.c
 * plus softfloat-macros and softfloat-specialize fragments it includes.
 * Every function is renamed via the preprocessor; bodies are unmodified.
 */

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

#define float32_default_nan 0xFFFFFFFF
#define float64_default_nan LIT64(0xFFFFFFFFFFFFFFFF)

'''

def build_rename_block():
    lines = ["/* symbol renaming */"]
    for k, v in RENAME_GLOBALS.items():
        lines.append(f"#define {k} {v}")
    for k, v in sorted(RENAME_FUNCS.items()):
        lines.append(f"#define {k} {v}")
    return "\n".join(lines) + "\n"

def gen_oracle():
    macros = read_src("bits32/softfloat-macros")
    specialize = read_src("softfloat-specialize")
    # trim specialize at FLOATX80
    specialize = specialize.split("#ifdef FLOATX80")[0]
    softfloat = read_src("bits32/softfloat.c")
    softfloat = preprocess_softfloat_c(softfloat)
    body = macros + "\n" + specialize + "\n" + softfloat
    body = apply_renames(body, RENAME_FUNCS, RENAME_GLOBALS)
    # globals at end of specialize need initial values
    oracle = ORACLE_HEADER + build_rename_block() + body
    (OUT / "oracle.c").write_text(oracle)

PORT_HEADER = r'''/* $NetBSD: softfloat.c,v 1.1 2002/05/21 23:51:07 bjh21 Exp $ */
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

#define float32_default_nan 0xFFFFFFFF
#define float64_default_nan LIT64(0xFFFFFFFFFFFFFFFF)

'''

PORT_FOOTER = "\n} // namespace\n"

def gen_port():
    macros = read_src("bits32/softfloat-macros")
    specialize = read_src("softfloat-specialize").split("#ifdef FLOATX80")[0]
    softfloat = preprocess_softfloat_c(read_src("bits32/softfloat.c"))
    body = macros + "\n" + specialize + "\n" + softfloat
    # rename globals for port isolation
    body = body.replace("float_rounding_mode", "__port_float_rounding_mode")
    body = body.replace("float_exception_flags", "__port_float_exception_flags")
    body = body.replace("float_detect_tininess", "__port_float_detect_tininess")
    body = body.replace("float_exception_mask", "__port_float_exception_mask")
    port = PORT_HEADER + body + PORT_FOOTER
    (OUT / "port.cppm").write_text(port)

def gen_skipped():
    lines = [f"{k}: {v}" for k, v in SKIP.items()]
    (OUT / "skipped.txt").write_text("\n".join(lines) + "\n")

def gen_harness():
    funcs = [f for f in PUBLIC_FUNCS if f not in SKIP]
    h = r'''/*
 * harness.cpp -- differential test for PBSD batch b0047 (bits32 softfloat).
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

import pbsd.lib.libc.softfloat.bits32.b0047;

namespace port = pbsd::lib_libc_softfloat_bits32::b0047;

using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i32 = std::int32_t;

extern "C" {
'''
    for f in funcs:
        if f.startswith("float32") or f.startswith("int32_to_float"):
            h += f"u32 ref_{f}(u32 a);\n" if "to_float32" in f or (f.startswith("float32") and "_to_" not in f and f not in ("float32_eq","float32_le","float32_lt","float32_eq_signaling","float32_le_quiet","float32_lt_quiet")) else ""
    # simpler: declare all
    for f in funcs:
        if f in ("int32_to_float32", "int32_to_float64"):
            h += f"u32 ref_{f}(i32 a);\n" if f.endswith("float32") else f"u64 ref_{f}(i32 a);\n"
        elif f == "int32_to_float64":
            pass
        elif f.startswith("float32_to_int"):
            h += f"i32 ref_{f}(u32 a);\n"
        elif f == "float32_to_float64":
            h += f"u64 ref_{f}(u32 a);\n"
        elif f.startswith("float32_"):
            if f in ("float32_eq","float32_le","float32_lt","float32_eq_signaling","float32_le_quiet","float32_lt_quiet"):
                h += f"int ref_{f}(u32 a, u32 b);\n"
            else:
                h += f"u32 ref_{f}(u32 a, u32 b);\n" if "add" in f or "sub" in f or "mul" in f or "div" in f or "rem" in f else f"u32 ref_{f}(u32 a);\n"
        elif f.startswith("float64_to_int"):
            h += f"i32 ref_{f}(u64 a);\n"
        elif f == "float64_to_float32":
            h += f"u32 ref_{f}(u64 a);\n"
        elif f.startswith("float64_"):
            if f in ("float64_eq","float64_le","float64_lt","float64_eq_signaling","float64_le_quiet","float64_lt_quiet"):
                h += f"int ref_{f}(u64 a, u64 b);\n"
            else:
                h += f"u64 ref_{f}(u64 a, u64 b);\n" if any(x in f for x in ("add","sub","mul","div","rem")) else f"u64 ref_{f}(u64 a);\n"

    h += r'''
extern int __oracle_float_rounding_mode;
extern int __oracle_float_exception_flags;
extern int __port_float_rounding_mode;
extern int __port_float_exception_flags;
}

struct Stat { const char *name; unsigned long long cases, failures; };

static Stat stats[] = {
'''
    for i, f in enumerate(funcs):
        h += f'\t{{ "{f}", 0, 0 }},\n'
    h += r'''};

static int reported;
static unsigned long fail_total;

static void save_oracle() {
	/* touch globals */
}
'''

    (OUT / "harness.cpp").write_text(h)
    print("harness stub written - needs completion")

if __name__ == "__main__":
    gen_oracle()
    gen_port()
    gen_skipped()
    print("oracle, port, skipped generated")
