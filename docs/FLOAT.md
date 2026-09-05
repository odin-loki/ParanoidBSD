# One float format, and bfloat as an instance of it

`pbsd/lib/fp/pbsd.fp.cppm`.

## What was asked for

A bfloat type at every data width, compatible with floating-point hardware,
done as a generic float rather than as a special case — and with no bugs.

## What bfloat actually is

bfloat16 is usually described as "float32 with fewer mantissa bits". That is
true but it hides the structure. The exact statement is:

> **bfloat16 is the high 16 bits of a binary32.**

Sign, the whole 8-bit exponent field, and the top 7 mantissa bits. Nothing is
rearranged. That single fact is where every useful property comes from:

* **The range.** It has binary32's exponent field verbatim, so it has
  binary32's range — about 10^38, against binary16's 10^4. This is the reason
  bfloat exists.
* **Hardware compatibility.** Widening is `bits << 16`. Not a conversion
  routine, a shift, and exact. Narrowing is an add and a shift.
* **Correctness.** Because the fields are contiguous, adding the round-to-
  nearest-even bias to the *whole* 32-bit pattern is the entire narrowing
  algorithm: a mantissa carry runs into the exponent, and an exponent carry
  runs into infinity, and both are what IEEE 754 asks for.

Generalise the sentence and the width disappears from it:

> **bfloat`W` is the high `W` bits of the IEEE interchange format of width
> `2W`.**

| type       | host format | layout      | range of |
|------------|-------------|-------------|----------|
| `bfloat16` | `binary32`  | 1 + 8 + 7   | binary32 |
| `bfloat32` | `binary64`  | 1 + 11 + 20 | binary64 |
| `bfloat64` | `binary128` | 1 + 15 + 48 | binary128 |

`bfloat64` is not declared yet: its host is binary128, whose storage needs a
128-bit integer, and `uint_for` stops at 64. That is a widening of one
template away and it is left undone rather than faked.

## Why this has no hand-written arithmetic

The one rule the module holds to:

> **It never implements arithmetic.**

Hand-written soft float is where floating-point bugs live — that is what the
five copies of NetBSD's `softfloat.h` in `lib/libc/<arch>/softfloat/` are, and
reimplementing them generically would generalise the bugs rather than remove
them. Instead:

* A format with a native type (`binary32` → `float`, `binary64` → `double`)
  computes in that type. The FPU rounds it.
* A truncation computes in the format it is the high half of, and rounds once
  on the way out.

The second one is only sound if computing in the wider format and rounding
once gives the same answer as computing in the narrow format directly. That is
Figueroa's condition: it holds for `+ - * /` when the wide format's precision
`q` is at least `2p + 2`.

| format     | p  | host      | q  | 2p+2 | holds |
|------------|----|-----------|----|------|-------|
| `bfloat16` | 8  | `binary32`| 24 | 18   | yes   |
| `bfloat32` | 21 | `binary64`| 53 | 44   | yes   |

`host_rounds_once<F>` is that inequality, and it is a `static_assert`. A format
whose host is too narrow does not double-round quietly; it fails to compile.

This is also, incidentally, what bfloat16 hardware does — AVX512-BF16 and the
ARM BF16 instructions compute in fp32 and round once — so the module agrees
with the silicon rather than merely with itself.

## What is tested, and how completely

`pbsd/lib/fp/tests/fp_test.cpp`, run by `pbsd-ci.yml` on every push.

Two of the tests are exhaustive, because the domains fit:

* **All 65,536 bfloat16 values** round-trip through binary32 unchanged.
  (Signalling NaNs come back quiet, which is checked separately and is what
  IEEE asks for.)
* **All 2^32 binary32 values** narrow to the same bfloat16 as the reference
  algorithm — the hand-written one everybody writes, kept in the test purely
  to disagree with.

There is no reason to sample a conversion whose whole domain can be
enumerated in six seconds.

The rest are the behaviours a format has to have: ties-to-even at 1 + eps/2,
1 + eps landing on the next representable value, overflow reaching infinity
rather than wrapping, NaN propagating and comparing unequal to itself, +0 and
-0 differing in bits and comparing equal, and 3e38 being finite in bfloat16
while the same value overflows binary32's mantissa — the range claim, checked
rather than asserted.

Both were verified to fail on a mutated module before being trusted: dropping
ties-to-even is caught at `0x00018000`, and dropping the NaN case at
`0x7f800001`.

## What it replaces

`docs/CONSOLIDATION.md` lists `lib/libc/<arch>/softfloat/softfloat.h` as 333
duplicated lines across five architectures — really two distinct texts, one
shared by arm and riscv and one by the three powerpc variants, plus a sixth
template. `basic_float<E, M>` is that description with the widths as
parameters, and it is 40 lines rather than 333.

Replacing the five in the kernel is a separate change and has not been made:
they are C headers included by C sources through `.PATH`, and a C++ module
cannot be dropped into that position without moving the consumers too.
