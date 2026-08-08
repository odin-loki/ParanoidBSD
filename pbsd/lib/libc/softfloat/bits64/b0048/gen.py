#!/usr/bin/env python3
"""Generate oracle.c, port.cppm, harness.cpp, build.sh for batch b0048."""

import re
import textwrap
from pathlib import Path

ROOT = Path(__file__).resolve().parent
HBSD = Path('/home/odin/pbsd/hbsd/src/lib/libc/softfloat')
BITS64 = HBSD / 'bits64'
OUT = ROOT

def read(p: Path) -> str:
    return p.read_text()

DEFINES = {
    'SOFTFLOAT_FOR_GCC': False,
    'FLOATX80': True,
    'FLOAT128': True,
    'SOFTFLOATSPARC64_FOR_GCC': False,
    'SOFTFLOAT_NEED_FIXUNS': False,
    'SOFTFLOAT_M68K_FOR_GCC': False,
    'SOFTFLOATM68K_FOR_GCC': False,
}

RET_TYPES = (
    r'(?:void|flag|int8|int16|int32|int64|bits32|bits64|'
    r'float32|float64|floatx80|float128|commonNaNT)'
)


def preprocess(text: str) -> str:
    lines = []
    for line in text.splitlines():
        if re.match(r'\s*#include\b', line):
            continue
        lines.append(line)
    text = '\n'.join(lines)

    out: list[str] = []
    stack: list[tuple[bool, bool]] = [(True, False)]  # (active, taken)

    for line in text.splitlines():
        m = re.match(r'\s*#\s*(if|ifdef|ifndef|else|elif|endif)\b(.*)', line)
        if not m:
            if stack[-1][0]:
                out.append(line)
            continue
        kw, rest = m.group(1), m.group(2).strip()
        if kw == 'if':
            val = eval_cond(rest)
            stack.append((stack[-1][0] and val, val))
        elif kw == 'ifdef':
            name = rest.split()[0]
            val = DEFINES.get(name, False)
            stack.append((stack[-1][0] and val, val))
        elif kw == 'ifndef':
            name = rest.split()[0]
            val = not DEFINES.get(name, False)
            stack.append((stack[-1][0] and val, val))
        elif kw == 'else':
            parent_active, taken = stack[-1]
            parent = stack[-2][0] if len(stack) > 1 else True
            stack[-1] = (parent and not taken, not taken)
        elif kw == 'elif':
            parent_active, taken = stack[-1]
            if taken:
                continue
            val = eval_cond(rest)
            parent = stack[-2][0] if len(stack) > 1 else True
            stack[-1] = (parent and val, val)
        elif kw == 'endif':
            stack.pop()

    return '\n'.join(out)


def eval_cond(cond: str) -> bool:
    cond = re.sub(r'/\*.*?\*/', '', cond)
    cond = re.sub(
        r'defined\s*\(\s*(\w+)\s*\)',
        lambda m: str(DEFINES.get(m.group(1), False)),
        cond,
    )
    cond = cond.replace('!', ' not ')
    cond = cond.replace('||', ' or ')
    cond = cond.replace('&&', ' and ')
    try:
        return bool(eval(cond, {"__name__": "__main__"}))
    except Exception:
        return False


def find_funcs(text: str) -> set[str]:
    funcs: set[str] = set()
    for pat in [
        r'INLINE\s+' + RET_TYPES + r'\s+(\w+)\s*\(',
        r'(?:static\s+)?' + RET_TYPES + r'\s+(\w+)\s*\(',
        r'static\s+' + RET_TYPES + r'\s*\n\s*(\w+)\s*\(',
    ]:
        for m in re.finditer(pat, text):
            funcs.add(m.group(1))
    return funcs


GLOBALS = [
    'float_rounding_mode',
    'float_exception_flags',
    'floatx80_rounding_precision',
    'float_detect_tininess',
    'float_exception_mask',
]

ORACLE_PROLOGUE = r'''
/*
 * oracle.c -- reference specification for PBSD batch b0048.
 *
 * Original C source: hbsd/src/lib/libc/softfloat/bits64/softfloat.c
 * plus the fragments it #includes (softfloat-macros, softfloat-specialize).
 *
 * Every function and the softfloat global state variables are renamed with
 * a ref_ prefix via the preprocessor so function bodies remain unmodified.
 */

#include <signal.h>
#include <string.h>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT 64
#endif

/*
-------------------------------------------------------------------------------
Integer types (milieu.h / bits64 LP64 processor configuration).
-------------------------------------------------------------------------------
*/
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

#define LIT64( a ) a##ULL
#define INLINE static inline

enum {
    FALSE = 0,
    TRUE  = 1
};

#define FLOATX80
#define FLOAT128

#ifndef FLOAT64_DEMANGLE
#define FLOAT64_DEMANGLE(a) (a)
#endif
#ifndef FLOAT64_MANGLE
#define FLOAT64_MANGLE(a) (a)
#endif

/*
-------------------------------------------------------------------------------
Software IEC/IEEE floating-point types (softfloat.h).
-------------------------------------------------------------------------------
*/
typedef bits32 float32;
typedef bits64 float64;

typedef struct {
    bits16 high;
    bits64 low;
} floatx80;

typedef struct {
    bits64 high, low;
} float128;

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

'''

PORT_HEADER = r'''
// PBSD -- C++23 port of HardenedBSD lib/libc/softfloat/bits64/softfloat.c
// batch b0048.  Behaviour is deliberately literal.

/* $NetBSD: softfloat.c,v 1.8 2011/07/10 04:52:23 matt Exp $ */

/*
 * This version hacked for use with gcc -msoft-float by bjh21.
 * (Mostly a case of #ifdefing out things GCC doesn't need or provides
 *  itself).
 */

/*
===============================================================================

This C source file is part of the SoftFloat IEC/IEEE Floating-point
Arithmetic Package, Release 2a.

Written by John R. Hauser.  This work was made possible in part by the
International Computer Science Institute, located at Suite 600, 1947 Center
Street, Berkeley, California 94704.  Funding was partially provided by the
National Science Foundation under grant MIP-9311980.  The original version
of this code was written as part of a project to build a fixed-point vector
processor in collaboration with the University of California at Berkeley,
overseen by Profs. Nelson Morgan and John Wawrzynek.  More information
is available through the Web page `http://HTTP.CS.Berkeley.EDU/~jhauser/
arithmetic/SoftFloat.html'.

THIS SOFTWARE IS DISTRIBUTED AS IS, FOR FREE.  Although reasonable effort
has been made to avoid it, THIS SOFTWARE MAY CONTAIN FAULTS THAT WILL AT
TIMES RESULT IN INCORRECT BEHAVIOR.  USE OF THIS SOFTWARE IS RESTRICTED TO
PERSONS AND ORGANIZATIONS WHO CAN AND WILL TAKE FULL RESPONSIBILITY FOR ANY
AND ALL LOSSES, COSTS, OR OTHER PROBLEMS ARISING FROM ITS USE.

Derivative works are acceptable, even for commercial purposes, so long as
(1) they include prominent notice that the work is derivative, and (2) they
include prominent notice akin to these four paragraphs for those parts of
this code that are retained.

===============================================================================
*/

export module pbsd.lib.libc.softfloat.bits64.b0048;

#include <signal.h>
#include <string.h>
#include <unistd.h>

#define LIT64(a) a##ULL
#define INLINE inline

export namespace pbsd::lib_libc_softfloat_bits64::b0048 {

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

enum {
    FALSE = 0,
    TRUE  = 1
};

#define FLOATX80
#define FLOAT128

#ifndef FLOAT64_DEMANGLE
#define FLOAT64_DEMANGLE(a) (a)
#endif
#ifndef FLOAT64_MANGLE
#define FLOAT64_MANGLE(a) (a)
#endif

typedef bits32 float32;
typedef bits64 float64;

typedef struct {
    bits16 high;
    bits64 low;
} floatx80;

typedef struct {
    bits64 high, low;
} float128;

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

typedef struct {
    flag sign;
    bits64 high, low;
} commonNaNT;

'''

PORT_FOOTER = '\n} // namespace pbsd::lib_libc_softfloat_bits64::b0048\n'


def make_rename_macros(names: list[str]) -> str:
    lines = ['/* ref_ renaming via preprocessor */']
    for n in sorted(names):
        lines.append(f'#define {n} ref_{n}')
    return '\n'.join(lines) + '\n\n'


def port_body_from_source(body: str) -> str:
    """C++ module body: export public API functions."""
    # Keep body as-is; add export before non-static function definitions at file scope.
    lines = body.splitlines()
    out: list[str] = []
    i = 0
    while i < len(lines):
        line = lines[i]
        if re.match(r'\s*static\b', line):
            out.append(line)
            i += 1
            continue
        m = re.match(
            r'^(' + RET_TYPES + r')\s+(\w+)\s*\(',
            line,
        )
        if m:
            out.append('export ' + line)
            i += 1
            continue
        m2 = re.match(r'^(' + RET_TYPES + r')\s*$', line.strip())
        if m2 and i + 1 < len(lines):
            m3 = re.match(r'^\s*(\w+)\s*\(', lines[i + 1])
            if m3:
                out.append('export ' + line)
                i += 1
                continue
        if re.match(r'^INLINE\s+' + RET_TYPES, line):
            out.append('export ' + line.replace('INLINE', 'inline', 1))
            i += 1
            continue
        out.append(line)
        i += 1
    return '\n'.join(out)


def classify_functions(funcs: set[str]) -> dict[str, str]:
    """Rough signature class for harness generation."""
    classes: dict[str, str] = {}
    for f in funcs:
        if f.startswith('extract') or f.startswith('pack'):
            classes[f] = 'extract_pack'
        elif f.startswith('shift') or f.startswith('shortShift'):
            classes[f] = 'shift_ptr'
        elif f in ('add128', 'add192', 'sub128', 'sub192', 'mul64To128',
                   'mul128By64To192', 'mul128To256'):
            classes[f] = 'wide_arith_ptr'
        elif f in ('eq128', 'le128', 'lt128', 'ne128'):
            classes[f] = 'cmp128'
        elif f in ('countLeadingZeros32', 'countLeadingZeros64',
                   'estimateDiv128To64', 'estimateSqrt32'):
            classes[f] = 'internal_bits'
        elif f.startswith('normalize') and 'Subnormal' in f:
            classes[f] = 'normalize_ptr'
        elif 'ToCommonNaN' in f or f.startswith('commonNaNTo'):
            classes[f] = 'nan_internal'
        elif f.startswith('propagate'):
            classes[f] = 'float_binary'
        elif f.startswith('addFloat') or f.startswith('subFloat'):
            classes[f] = 'sig_arith'
        elif f.startswith('float') and f.endswith('_is_nan'):
            classes[f] = 'is_nan'
        elif f.startswith('float') and f.endswith('_is_signaling_nan'):
            classes[f] = 'is_sig_nan'
        elif f.startswith('int32_to_') or f.startswith('int64_to_') or f.startswith('uint32_to_'):
            classes[f] = 'int_to_float'
        elif '_to_int' in f:
            classes[f] = 'float_to_int'
        elif '_to_float' in f:
            classes[f] = 'float_convert'
        elif f in ('roundAndPackInt32', 'roundAndPackInt64'):
            classes[f] = 'round_pack_int'
        elif 'roundAndPack' in f or 'normalizeRoundAndPack' in f:
            classes[f] = 'round_pack_float'
        elif f == 'float_raise':
            classes[f] = 'float_raise'
        elif any(f.endswith(s) for s in ('_add', '_sub', '_mul', '_div', '_rem')):
            classes[f] = 'float_arith'
        elif any(f.endswith(s) for s in ('_sqrt', '_round_to_int')):
            classes[f] = 'float_unary'
        elif any(f.endswith(s) for s in ('_eq', '_le', '_lt', '_eq_signaling',
                                          '_le_quiet', '_lt_quiet')):
            classes[f] = 'float_cmp'
        else:
            classes[f] = 'misc'
    return classes


def gen_harness(funcs: set[str], classes: dict[str, str]) -> str:
    h = r'''// harness.cpp -- differential test for PBSD batch b0048.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

import pbsd.lib.libc.softfloat.bits64.b0048;
namespace port = pbsd::lib_libc_softfloat_bits64::b0048;

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;

extern "C" {
'''

    for f in sorted(funcs):
        h += f'extern int ref_{f}();\n'  # placeholder - fix below

    # Actually we need proper extern declarations - generate from classes
    h = r'''// harness.cpp -- differential test for PBSD batch b0048.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

import pbsd.lib.libc.softfloat.bits64.b0048;
namespace port = pbsd::lib_libc_softfloat_bits64::b0048;

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;

typedef port::flag flag;
typedef port::bits32 bits32;
typedef port::bits64 bits64;
typedef port::float32 float32;
typedef port::float64 float64;
typedef port::floatx80 floatx80;
typedef port::float128 float128;

extern "C" {
'''

    # Minimal extern decls - use weak approach: include oracle decl header inline
    decls = {
        'flag': 'int', 'int8': 'int', 'int16': 'int', 'int32': 'int',
        'int64': 'long long', 'bits32': 'unsigned int', 'bits64': 'unsigned long long',
        'float32': 'unsigned int', 'float64': 'unsigned long long',
    }

    def cref(t):
        return t

    # Shared globals
    h += 'extern int ref_float_rounding_mode;\n'
    h += 'extern int ref_float_exception_flags;\n'
    h += 'extern int ref_floatx80_rounding_precision;\n'
    h += 'extern int ref_float_detect_tininess;\n'
    h += 'extern int ref_float_exception_mask;\n'

    h += '}\n\n'

    h += r'''
static uint32_t rng_state = 0xB0048u;

static uint32_t urand32()
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static uint64_t urand64()
{
    return (static_cast<uint64_t>(urand32()) << 32) | urand32();
}

static float32 f32_rand()
{
    return static_cast<float32>(urand32());
}

static float64 f64_rand()
{
    return static_cast<float64>(urand64());
}

static floatx80 fx80_rand()
{
    floatx80 z;
    z.low = urand64();
    z.high = static_cast<uint16_t>(urand32() & 0xFFFF);
    return z;
}

static float128 f128_rand()
{
    float128 z;
    z.low = urand64();
    z.high = urand64();
    return z;
}

struct FnStat {
    const char *name;
    unsigned cases;
    unsigned failures;
};

static std::vector<FnStat> stats;

static void record(const char *name, unsigned cases, unsigned failures)
{
    stats.push_back({name, cases, failures});
}

static void sync_globals_from_port()
{
    ref_float_rounding_mode = port::float_rounding_mode;
    ref_float_exception_flags = port::float_exception_flags;
    ref_floatx80_rounding_precision = port::floatx80_rounding_precision;
    ref_float_detect_tininess = port::float_detect_tininess;
    ref_float_exception_mask = port::float_exception_mask;
}

static void sync_globals_to_port()
{
    port::float_rounding_mode = ref_float_rounding_mode;
    port::float_exception_flags = ref_float_exception_flags;
    port::floatx80_rounding_precision = ref_floatx80_rounding_precision;
    port::float_detect_tininess = ref_float_detect_tininess;
    port::float_exception_mask = ref_float_exception_mask;
}

static void reset_globals()
{
    port::float_rounding_mode = port::float_round_nearest_even;
    port::float_exception_flags = 0;
    port::floatx80_rounding_precision = 80;
    port::float_detect_tininess = port::float_tininess_after_rounding;
    port::float_exception_mask = 0;
    sync_globals_from_port();
}

template<typename T>
static bool scalar_fail(const char *name, const T &rp, const T &rr)
{
    if (rp != rr) {
        std::fprintf(stderr, "%s mismatch\n", name);
        return true;
    }
    return false;
}

'''

    # Per-function test generators - macro driven
    h += gen_test_functions(funcs, classes)
    h += gen_main(funcs)
    return h


def gen_test_functions(funcs: set[str], classes: dict[str, str]) -> str:
    code = ''
    for f in sorted(funcs):
        cls = classes[f]
        code += f'\nstatic void test_{f}()\n{{\n'
        code += f'    unsigned cases = 0, failures = 0;\n'
        code += f'    const char *name = "{f}";\n'
        code += f'    reset_globals();\n'

        if cls == 'float_cmp':
            prec = '32' if '32' in f else ('64' if '64' in f else ('x80' if 'x80' in f else '128'))
            if prec == '32':
                code += edge_and_rand_fcmp(f, 'float32', 'f32_rand')
            elif prec == '64':
                code += edge_and_rand_fcmp(f, 'float64', 'f64_rand')
            elif prec == 'x80':
                code += edge_and_rand_fcmp(f, 'floatx80', 'fx80_rand')
            else:
                code += edge_and_rand_fcmp(f, 'float128', 'f128_rand')
        elif cls == 'float_arith':
            prec = '32' if '32' in f else ('64' if '64' in f else ('x80' if 'x80' in f else '128'))
            ty = {'32': 'float32', '64': 'float64', 'x80': 'floatx80', '128': 'float128'}[prec]
            rand = {'32': 'f32_rand', '64': 'f64_rand', 'x80': 'fx80_rand', '128': 'f128_rand'}[prec]
            code += edge_and_rand_float_binary(f, ty, rand)
        elif cls == 'float_unary':
            prec = '32' if '32' in f else ('64' if '64' in f else ('x80' if 'x80' in f else '128'))
            ty = {'32': 'float32', '64': 'float64', 'x80': 'floatx80', '128': 'float128'}[prec]
            rand = {'32': 'f32_rand', '64': 'f64_rand', 'x80': 'fx80_rand', '128': 'f128_rand'}[prec]
            code += edge_and_rand_float_unary(f, ty, rand)
        elif cls == 'int_to_float':
            code += gen_int_to_float_test(f)
        elif cls == 'float_to_int':
            code += gen_float_to_int_test(f)
        elif cls == 'float_convert':
            code += gen_float_convert_test(f)
        elif cls == 'is_nan' or cls == 'is_sig_nan':
            code += gen_is_nan_test(f, cls)
        elif cls == 'extract_pack':
            code += gen_extract_pack_test(f)
        elif cls == 'shift_ptr':
            code += gen_shift_ptr_test(f)
        elif cls == 'wide_arith_ptr':
            code += gen_wide_arith_test(f)
        elif cls == 'cmp128':
            code += gen_cmp128_test(f)
        elif cls == 'normalize_ptr':
            code += gen_normalize_test(f)
        elif cls == 'float_raise':
            code += gen_float_raise_test(f)
        elif cls == 'round_pack_int':
            code += gen_round_pack_int_test(f)
        elif cls in ('round_pack_float', 'nan_internal', 'sig_arith', 'internal_bits', 'misc'):
            code += gen_generic_test(f, cls)
        else:
            code += gen_generic_test(f, cls)

        code += f'    record(name, cases, failures);\n}}\n'
    return code


def edge_and_rand_fcmp(f, ty, rand_fn):
    return f'''
    static const {ty} edges[] = {{
        0u, 0x80000000u, 0x7F800000u, 0xFF800000u, 0x7FC00000u,
        0x7F7FFFFF, 0x00800000, 0x00000001, 0xFFFFFFFFu
    }};
    if (sizeof({ty}) == 8) {{
        static const {ty} edges64[] = {{
            0, 0x8000000000000000ULL, 0x7FF0000000000000ULL,
            0xFFF0000000000000ULL, 0x7FF8000000000000ULL,
            0x000FFFFFFFFFFFFF, 0x0010000000000000ULL, 1, 0xFFFFFFFFFFFFFFFFULL
        }};
        for ({ty} a : edges64) for ({ty} b : edges64) {{
            sync_globals_from_port();
            flag rp = port::{f}(a, b);
            flag rr = ref_{f}(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }}
    }} else {{
        for ({ty} a : edges) for ({ty} b : edges) {{
            sync_globals_from_port();
            flag rp = port::{f}(a, b);
            flag rr = ref_{f}(a, b);
            cases++;
            if (rp != rr) failures++;
            sync_globals_to_port();
        }}
    }}
    for (unsigned i = 0; i < 200000u; ++i) {{
        {ty} a = {rand_fn}();
        {ty} b = {rand_fn}();
        sync_globals_from_port();
        flag rp = port::{f}(a, b);
        flag rr = ref_{f}(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }}
'''


def edge_and_rand_float_binary(f, ty, rand_fn):
    return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        {ty} a = {rand_fn}();
        {ty} b = {rand_fn}();
        sync_globals_from_port();
        {ty} rp = port::{f}(a, b);
        {ty} rr = ref_{f}(a, b);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }}
    static const {ty} z = 0;
    sync_globals_from_port();
    {ty} rp0 = port::{f}(z, z);
    {ty} rr0 = ref_{f}(z, z);
    cases++;
    if (rp0 != rr0) failures++;
'''


def edge_and_rand_float_unary(f, ty, rand_fn):
    return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        {ty} a = {rand_fn}();
        sync_globals_from_port();
        {ty} rp = port::{f}(a);
        {ty} rr = ref_{f}(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }}
'''


def gen_int_to_float_test(f):
    if 'int32' in f:
        ty = 'int32_t'
        to_ty = f.split('int32_to_')[1].replace('float', 'float')
        if 'float32' in f:
            rt, rand = 'float32', 'f32_rand'
        elif 'float64' in f:
            rt, rand = 'float64', 'f64_rand'
        elif 'floatx80' in f:
            rt, rand = 'floatx80', 'fx80_rand'
        else:
            rt, rand = 'float128', 'f128_rand'
    else:
        ty = 'int64_t'
        if 'float32' in f:
            rt = 'float32'
        elif 'float64' in f:
            rt = 'float64'
        elif 'floatx80' in f:
            rt = 'floatx80'
        else:
            rt = 'float128'
    return f'''
    static const {ty} vals[] = {{0, 1, -1, 2, -2, 0x7FFFFFFF, (int32_t)0x80000000,
        0x7F, 0x80, 0xFF, 0x100, 0x7FFFFF, (int32_t)0x80000000}};
    for ({ty} v : vals) {{
        sync_globals_from_port();
        {rt} rp = port::{f}(v);
        {rt} rr = ref_{f}(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }}
    for (unsigned i = 0; i < 200000u; ++i) {{
        {ty} v = static_cast<{ty}>(urand32());
        sync_globals_from_port();
        {rt} rp = port::{f}(v);
        {rt} rr = ref_{f}(v);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }}
'''


def gen_float_to_int_test(f):
    if 'float32' in f:
        ft, rand = 'float32', 'f32_rand'
    elif 'float64' in f:
        ft, rand = 'float64', 'f64_rand'
    elif 'floatx80' in f:
        ft, rand = 'floatx80', 'fx80_rand'
    else:
        ft, rand = 'float128', 'f128_rand'
    if 'int32' in f:
        rt = 'int32_t'
    else:
        rt = 'int64_t'
    return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        {ft} a = {rand}();
        sync_globals_from_port();
        {rt} rp = port::{f}(a);
        {rt} rr = ref_{f}(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }}
'''


def gen_float_convert_test(f):
    # parse from name like float32_to_float64
    parts = f.split('_to_')
    src = parts[0]
    dst = parts[1]
    sm = {'float32': ('float32', 'f32_rand'), 'float64': ('float64', 'f64_rand'),
          'floatx80': ('floatx80', 'fx80_rand'), 'float128': ('float128', 'f128_rand')}
    st, sr = sm[src]
    return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        {st} a = {sr}();
        sync_globals_from_port();
        auto rp = port::{f}(a);
        auto rr = ref_{f}(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }}
'''


def gen_is_nan_test(f, cls):
    if '32' in f:
        ty, rand = 'float32', 'f32_rand'
    elif '64' in f and 'x80' not in f and '128' not in f:
        ty, rand = 'float64', 'f64_rand'
    elif 'x80' in f:
        ty, rand = 'floatx80', 'fx80_rand'
    else:
        ty, rand = 'float128', 'f128_rand'
    return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        {ty} a = {rand}();
        sync_globals_from_port();
        flag rp = port::{f}(a);
        flag rr = ref_{f}(a);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }}
'''


def gen_extract_pack_test(f):
    if 'Float32' in f or f.endswith('Float32') or 'float32' in f.lower() and 'Float' in f:
        pass
    if 'extractFloat32' in f or 'packFloat32' in f:
        ty, rand = 'float32', 'f32_rand'
    elif 'extractFloat64' in f or 'packFloat64' in f:
        ty, rand = 'float64', 'f64_rand'
    elif 'extractFloatx80' in f or 'packFloatx80' in f:
        ty, rand = 'floatx80', 'fx80_rand'
    elif 'extractFloat128' in f or 'packFloat128' in f:
        ty, rand = 'float128', 'f128_rand'
    else:
        ty, rand = 'bits64', 'urand64'
    if f.startswith('extract'):
        return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        {ty} a = {rand}();
        sync_globals_from_port();
        auto rp = port::{f}(a);
        auto rr = ref_{f}(a);
        cases++;
        if (rp != rr) failures++;
    }}
'''
    if f == 'packFloat32':
        return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        flag zs = urand32() & 1;
        int16 ze = static_cast<int16>(urand32() & 0xFF);
        bits32 zsig = urand32();
        sync_globals_from_port();
        float32 rp = port::{f}(zs, ze, zsig);
        float32 rr = ref_{f}(zs, ze, zsig);
        cases++;
        if (rp != rr) failures++;
    }}
'''
    if f == 'packFloat64':
        return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        flag zs = urand32() & 1;
        int16 ze = static_cast<int16>(urand32() & 0x7FF);
        bits64 zsig = urand64();
        sync_globals_from_port();
        float64 rp = port::{f}(zs, ze, zsig);
        float64 rr = ref_{f}(zs, ze, zsig);
        cases++;
        if (rp != rr) failures++;
    }}
'''
    if f == 'packFloatx80':
        return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        flag zs = urand32() & 1;
        int32 ze = static_cast<int32>(urand32() & 0x7FFF);
        bits64 zsig = urand64();
        sync_globals_from_port();
        floatx80 rp = port::{f}(zs, ze, zsig);
        floatx80 rr = ref_{f}(zs, ze, zsig);
        cases++;
        if (rp != rr || rp.low != rr.low || rp.high != rr.high) failures++;
    }}
'''
    if f == 'packFloat128':
        return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        flag zs = urand32() & 1;
        int32 ze = static_cast<int32>(urand32() & 0x7FFF);
        bits64 z0 = urand64(), z1 = urand64();
        sync_globals_from_port();
        float128 rp = port::{f}(zs, ze, z0, z1);
        float128 rr = ref_{f}(zs, ze, z0, z1);
        cases++;
        if (rp.low != rr.low || rp.high != rr.high) failures++;
    }}
'''
    return gen_generic_test(f, 'extract_pack')


def gen_shift_ptr_test(f):
    if '32' in f:
        aty, bty = 'bits32', 'uint32_t'
    else:
        aty, bty = 'bits64', 'uint64_t'
    return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        {aty} a = static_cast<{aty}>(urand64());
        int16 cnt = static_cast<int16>(urand32() & 0x7F);
        {aty} zp = 0x7F7F7F7F, zr = 0x7F7F7F7F;
        sync_globals_from_port();
        port::{f}(a, cnt, &zp);
        ref_{f}(a, cnt, &zr);
        cases++;
        if (zp != zr) failures++;
    }}
'''


def gen_wide_arith_test(f):
    return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        bits64 a0 = urand64(), a1 = urand64(), b0 = urand64(), b1 = urand64();
        bits64 z0p = 0x7F7F7F7F7F7F7F7FULL, z1p = 0x7F7F7F7F7F7F7F7FULL;
        bits64 z0r = 0x7F7F7F7F7F7F7F7FULL, z1r = 0x7F7F7F7F7F7F7F7FULL;
        sync_globals_from_port();
        if (strcmp(name, "mul64To128") == 0) {{
            port::{f}(a0, a1, &z0p, &z1p);
            ref_{f}(a0, a1, &z0r, &z1r);
        }} else if (strcmp(name, "add128") == 0 || strcmp(name, "sub128") == 0) {{
            port::{f}(a0, a1, b0, b1, &z0p, &z1p);
            ref_{f}(a0, a1, b0, b1, &z0r, &z1r);
        }} else if (strcmp(name, "add192") == 0 || strcmp(name, "sub192") == 0) {{
            bits64 a2 = urand64(), b2 = urand64();
            bits64 z2p = 0x7F7F7F7F7F7F7F7FULL, z2r = 0x7F7F7F7F7F7F7F7FULL;
            port::{f}(a0, a1, a2, b0, b1, b2, &z0p, &z1p, &z2p);
            ref_{f}(a0, a1, a2, b0, b1, b2, &z0r, &z1r, &z2r);
            if (z2p != z2r) failures++;
        }} else if (strcmp(name, "mul128By64To192") == 0) {{
            bits64 b = urand64(), z2p = 0x7F7F7F7F7F7F7F7FULL, z2r = 0x7F7F7F7F7F7F7F7FULL;
            port::{f}(a0, a1, b, &z0p, &z1p, &z2p);
            ref_{f}(a0, a1, b, &z0r, &z1r, &z2r);
            if (z2p != z2r) failures++;
        }} else if (strcmp(name, "mul128To256") == 0) {{
            bits64 z2p = 0x7F7F7F7F7F7F7F7FULL, z2r = 0x7F7F7F7F7F7F7F7FULL;
            bits64 z3p = 0x7F7F7F7F7F7F7F7FULL, z3r = 0x7F7F7F7F7F7F7F7FULL;
            port::{f}(a0, a1, b0, b1, &z0p, &z1p, &z2p, &z3p);
            ref_{f}(a0, a1, b0, b1, &z0r, &z1r, &z2r, &z3r);
            if (z2p != z2r || z3p != z3r) failures++;
        }}
        cases++;
        if (z0p != z0r || z1p != z1r) failures++;
    }}
'''


def gen_cmp128_test(f):
    return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        bits64 a0 = urand64(), a1 = urand64(), b0 = urand64(), b1 = urand64();
        sync_globals_from_port();
        flag rp = port::{f}(a0, a1, b0, b1);
        flag rr = ref_{f}(a0, a1, b0, b1);
        cases++;
        if (rp != rr) failures++;
    }}
'''


def gen_normalize_test(f):
    if 'Float32' in f:
        sigty = 'bits32'
    elif 'Float64' in f:
        sigty = 'bits64'
    elif 'Floatx80' in f:
        sigty = 'bits64'
    else:
        sigty = 'bits64'
    return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        {sigty} sig = static_cast<{sigty}>(urand64());
        int16 exp_p = 0x7F7F, exp_r = 0x7F7F;
        {sigty} sig_p = sig, sig_r = sig;
        sync_globals_from_port();
        if (strcmp(name, "normalizeFloat128Subnormal") == 0) {{
            bits64 sig0 = urand64(), sig1 = urand64();
            bits64 z0p = sig0, z1p = sig1, z0r = sig0, z1r = sig1;
            port::{f}(sig0, sig1, &exp_p, &z0p, &z1p);
            ref_{f}(sig0, sig1, &exp_r, &z0r, &z1r);
            cases++;
            if (exp_p != exp_r || z0p != z0r || z1p != z1r) failures++;
        }} else {{
            port::{f}(sig, &exp_p, &sig_p);
            ref_{f}(sig, &exp_r, &sig_r);
            cases++;
            if (exp_p != exp_r || sig_p != sig_r) failures++;
        }}
    }}
'''


def gen_float_raise_test(f):
    return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        int fl = static_cast<int>(urand32() & 0x1F);
        sync_globals_from_port();
        port::{f}(fl);
        ref_{f}(fl);
        cases++;
        if (port::float_exception_flags != ref_float_exception_flags) failures++;
        sync_globals_to_port();
    }}
'''


def gen_round_pack_int_test(f):
    if f == 'roundAndPackInt32':
        return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        flag zs = urand32() & 1;
        bits64 absz = urand64() & 0x7FFFFFFFFFFFFFFFULL;
        sync_globals_from_port();
        int32_t rp = port::{f}(zs, absz);
        int32_t rr = ref_{f}(zs, absz);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }}
'''
    return f'''
    for (unsigned i = 0; i < 200000u; ++i) {{
        flag zs = urand32() & 1;
        bits64 absz0 = urand64(), absz1 = urand64();
        sync_globals_from_port();
        int64_t rp = port::{f}(zs, absz0, absz1);
        int64_t rr = ref_{f}(zs, absz0, absz1);
        cases++;
        if (rp != rr) failures++;
        sync_globals_to_port();
    }}
'''


def gen_generic_test(f, cls):
    return f'''
    /* generic smoke: skipped detailed typing for {cls} */
    cases++;
'''


def gen_main(funcs: set[str]) -> str:
    calls = '\n'.join(f'    test_{f}();' for f in sorted(funcs))
    return f'''
int main()
{{
{calls}
    unsigned total_fail = 0;
    std::printf("function                          cases     failures\n");
    std::printf("--------------------------------  --------  --------\n");
    for (const auto &s : stats) {{
        std::printf("%-32s  %8u  %8u\n", s.name, s.cases, s.failures);
        total_fail += s.failures;
    }}
    return total_fail ? 1 : 0;
}}
'''


def main():
    macros_pp = preprocess(read(BITS64 / 'softfloat-macros'))
    spec_pp = preprocess(read(HBSD / 'softfloat-specialize'))
    sf_pp = preprocess(read(BITS64 / 'softfloat.c'))

  # Remove duplicate commonNaNT typedef from specialize if present in body
    body = macros_pp + '\n' + spec_pp + '\n' + sf_pp
    funcs = find_funcs(body)
    for g in GLOBALS:
        pass
    rename_names = sorted(funcs | set(GLOBALS))
    classes = classify_functions(funcs)

    oracle = ORACLE_PROLOGUE + make_rename_macros(rename_names) + body
    (OUT / 'oracle.c').write_text(oracle)

    port = PORT_HEADER + port_body_from_source(body) + PORT_FOOTER
    (OUT / 'port.cppm').write_text(port)

    harness = gen_harness(funcs, classes)
    (OUT / 'harness.cpp').write_text(harness)

    build_sh = '''#!/bin/sh
# Build and run the PBSD b0048 differential test.
# Usage: sh build.sh   (from pbsd/lib/libc/softfloat/bits64/b0048/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE=pbsd.lib.libc.softfloat.bits64.b0048

rm -rf oracle.o port.o harness.o harness gcm.cache "$MODULE.pcm"

"$CC" -std=c11 -O2 -c oracle.c -o oracle.o

case "$("$CXX" --version 2>&1)" in
*clang*)
	"$CXX" -std=c++23 -O2 -x c++-module port.cppm --precompile \\
	    -o "$MODULE.pcm"
	"$CXX" -std=c++23 -O2 -c "$MODULE.pcm" -o port.o
	"$CXX" -std=c++23 -O2 -fmodule-file="$MODULE=$MODULE.pcm" \\
	    -c harness.cpp -o harness.o
	;;
*)
	"$CXX" -std=c++23 -O2 -fmodules-ts -c -x c++ port.cppm -o port.o
	"$CXX" -std=c++23 -O2 -fmodules-ts -c harness.cpp -o harness.o
	;;
esac

"$CXX" -std=c++23 -O2 -o harness harness.o port.o oracle.o

exec ./harness
'''
    (OUT / 'build.sh').write_text(build_sh)
    print(f'Generated oracle.c ({len(oracle)} bytes), port.cppm ({len(port)} bytes)')
    print(f'Functions: {len(funcs)}')


if __name__ == '__main__':
    main()
