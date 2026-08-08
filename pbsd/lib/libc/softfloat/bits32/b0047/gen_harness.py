#!/usr/bin/env python3
from pathlib import Path
OUT = Path(__file__).parent / "harness.cpp"
PUBLIC = [
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
decls = {
    "int32_to_float32": "u32 ref_int32_to_float32(i32);",
    "int32_to_float64": "u64 ref_int32_to_float64(i32);",
    "float32_to_int32": "i32 ref_float32_to_int32(u32);",
    "float32_to_int32_round_to_zero": "i32 ref_float32_to_int32_round_to_zero(u32);",
    "float32_to_float64": "u64 ref_float32_to_float64(u32);",
    "float32_round_to_int": "u32 ref_float32_round_to_int(u32);",
    "float32_add": "u32 ref_float32_add(u32,u32);",
    "float32_sub": "u32 ref_float32_sub(u32,u32);",
    "float32_mul": "u32 ref_float32_mul(u32,u32);",
    "float32_div": "u32 ref_float32_div(u32,u32);",
    "float32_rem": "u32 ref_float32_rem(u32,u32);",
    "float32_sqrt": "u32 ref_float32_sqrt(u32);",
    "float32_eq": "int ref_float32_eq(u32,u32);",
    "float32_le": "int ref_float32_le(u32,u32);",
    "float32_lt": "int ref_float32_lt(u32,u32);",
    "float32_eq_signaling": "int ref_float32_eq_signaling(u32,u32);",
    "float32_le_quiet": "int ref_float32_le_quiet(u32,u32);",
    "float32_lt_quiet": "int ref_float32_lt_quiet(u32,u32);",
    "float64_to_int32": "i32 ref_float64_to_int32(u64);",
    "float64_to_int32_round_to_zero": "i32 ref_float64_to_int32_round_to_zero(u64);",
    "float64_to_float32": "u32 ref_float64_to_float32(u64);",
    "float64_add": "u64 ref_float64_add(u64,u64);",
    "float64_sub": "u64 ref_float64_sub(u64,u64);",
    "float64_mul": "u64 ref_float64_mul(u64,u64);",
    "float64_div": "u64 ref_float64_div(u64,u64);",
    "float64_rem": "u64 ref_float64_rem(u64,u64);",
    "float64_sqrt": "u64 ref_float64_sqrt(u64);",
    "float64_eq": "int ref_float64_eq(u64,u64);",
    "float64_le": "int ref_float64_le(u64,u64);",
    "float64_lt": "int ref_float64_lt(u64,u64);",
    "float64_eq_signaling": "int ref_float64_eq_signaling(u64,u64);",
    "float64_le_quiet": "int ref_float64_le_quiet(u64,u64);",
    "float64_lt_quiet": "int ref_float64_lt_quiet(u64,u64);",
}
parts = ["""/*
 * harness.cpp -- differential test for PBSD batch b0047.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>

import pbsd.lib.libc.softfloat.bits32.b0047;

namespace port = pbsd::lib_libc_softfloat_bits32::b0047;

using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i32 = std::int32_t;

extern "C" {
"""]
for f in PUBLIC:
    parts.append("\t" + decls[f] + "\n")
parts.append("""extern int __oracle_float_rounding_mode;
extern int __oracle_float_exception_flags;
extern int __oracle_float_detect_tininess;
extern int __oracle_float_exception_mask;
}

struct Stat { const char *name; unsigned long long cases, failures; };

static Stat stats[] = {
""")
for f in PUBLIC:
    parts.append(f'\t{{ "{f}", 0, 0 }},\n')
parts.append("};\n\nenum FnIdx {\n")
for i, f in enumerate(PUBLIC):
    parts.append(f"\tFN_{f.upper()} = {i},\n")
parts.append("};\n\n")
parts.append(Path(__file__).parent.joinpath("harness_body.inc").read_text())
OUT.write_text("".join(parts))
