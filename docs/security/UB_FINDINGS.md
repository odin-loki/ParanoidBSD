# Undefined behaviour and memory-safety defects found by verification

Two instruments over `hbsd/src`: CBMC function by function, and clang's
static analyser translation unit by translation unit.
`tools/verify/README.md` has the method and the five calibrations it took
to make the output mean anything; this is what it found.

**Nothing here rests on one tool's say-so.** Every arithmetic-UB entry was
confirmed with UBSan before anything was edited. Every memory-safety entry
was reproduced on the single file, read against the code that establishes
or fails to establish the precondition, and confirmed gone afterwards —
and the ones that did not survive that reading are in the last section,
with the reasoning that killed them.

---

## Fixed — the UB was in the *type* of an expression, not its value

For each of these the bit pattern the code computes is what it always
computed. The fix makes an operation defined that hardware was already
performing; nothing observable changes. That is why they were safe to
repair rather than to report.

### `lib/libc/stdbit` — six C23 functions

```c
unsigned int stdc_leading_ones_uc(unsigned char x) {
        const int offset = UINT_WIDTH - UCHAR_WIDTH;   /* 24 */
        return (__builtin_clz(~(x << offset)));
}
```

`x` promotes to `int` (C17 6.3.1.1p2). For `x >= 128`, `x << 24` needs
more than `INT_MAX` — C17 6.5.7p4, undefined. Half the domain of six
functions: `stdc_leading_ones_uc/us`, `stdc_leading_zeros_uc/us`,
`stdc_first_leading_one_uc/us`.

`stdc_first_leading_zero.c` — same author, same month, same directory —
has the `(unsigned int)` cast the other three files omit.

*Verified:* exhaustively against an independent reference that counts bits
in a loop — all 256 `unsigned char` and all 65,536 `unsigned short` values
for each of three operations, **197,376 cases, zero mismatches**, under
UBSan throughout. CBMC afterwards: 70 of 70 functions in the directory
`PROVED`.

### `lib/msun/src/s_rint.c`, `s_rintf.c`

```c
int32_t i0,j0,sx;
sx = (i0>>31)&1;                                  /* 0 or 1 */
SET_HIGH_WORD(t,(i0&0x7fffffff)|(sx<<31));        /* 1 << 31, signed */
```

`rint()` takes that branch for **every negative argument** with
`|x| < 2^52`.

### `lib/msun/src/s_ceil.c`, `s_floor.c`

```c
j = i1 + (1<<(52-j0));
```

`j0` is the unbiased exponent, and the branch runs for `j0` ∈ [21,51]. At
`j0 == 21` the shift is `1 << 31`. `ceil(3000000.5)` reaches it.

Casting the shift to `u_int32_t` also fixes the addition — `i1` is
`int32_t`, so the sum was signed and could overflow independently. The
next line is `if(j<i1) i0+=1;`, a carry check that only means anything
under wrapping, so unsigned is what the code already assumed.

*Verified:* 6,200,000 cases across every `j0` in [21,51], zero mismatches.

---

## Reported, not fixed — the UB *is* the overflow

For these there is no behaviour-preserving repair. The overflow is the
defect, so defining it means choosing new behaviour for an exported
function on arguments the standard does not bound. That is a product
decision and it is not made here.

| function | expression | reached by |
|---|---|---|
| `lib/libc/gen/nice.c` | `prio + incr` | `nice(INT_MAX)` — `incr` is the caller's `int`; POSIX does not bound it, and glibc clamps where FreeBSD does not |
| `lib/libc/compat-43/killpg.c` | `kill(-pgid, sig)` | `killpg(INT_MIN, sig)` |
| `lib/libc/gen/timezone.c` | `zone = -zone` | `timezone(INT_MIN, dst)` |
| `lib/libc/net/ip6opt.c` | `nbytes += 2` then `(nbytes+7)&~7` | `inet6_option_space(n)` for `n > INT_MAX-2` |

`inet6_option_space` is the one worth a second look: it computes a
**size** from a caller-supplied `int`, and an overflow there produces a
small value where a large one was asked for.

All four confirmed under UBSan.

---

## Fixed — memory safety, found by clang's static analyser

The model checker looks for UB in arithmetic. These came from the second
instrument, and each one was reproduced on the single file before the edit
and confirmed gone after it.

### `sys/netlink/netlink_snl.h` — `snl_free()` was not idempotent

`snl_init()` calls `snl_free()` on **every** failure after the socket
exists, and then returns false. `lib/libc/rpc/svc_nl.c:102` is a caller
that cleans up on failure:

```c
	if (!snl_init(&sc->snl, NETLINK_GENERIC) || ...)
		goto fail;
	...
fail:
	free(xprt);
	free(buf);
	snl_free(&sc->snl);
```

so the state was released twice. `snl_free()` cleared nothing, so:

- a `setsockopt(NETLINK_EXT_ACK)` an older kernel rejects closed `ss->fd`
  **twice**, and
- an `lb_init()` that could not allocate freed `ss->buf` **twice**.

The double close is the more reachable and the worse of the two: between
the two closes another thread can be handed that descriptor number, and
the second close then shuts down an unrelated connection.

Fixed by clearing `init_done`, `buf` and `lb` as each is released. A
correct caller cannot tell the difference.

### `lib/libc/rpc/svc_nl.c`, `usr.bin/genl/parser_rpc.c` — uninitialised parse target

`snl_parse_header()` writes only the attributes a message actually
carries; it does not zero the target. Both files declared

```c
	struct nl_request_parsed req;		/* uninitialised */
```

so a request without `RPCNL_REQUEST_GROUP` left `req.group` as stack
garbage, compared against `sc->group`; and a request without
`RPCNL_REQUEST_BODY` left `req.data` a garbage **pointer** that
`NLA_DATA_LEN(req.data)` then dereferences.

`sys/rpc/clnt_nl.c:395` — the kernel end of the same protocol — already
declares its target `= {}`. Both userland ends now do, plus an explicit
`req.data == NULL` check.

### `sys/dev/ntsync/ntsync.c` — uninitialised return on the fast path

```c
	for (;;) {
		ntsync_wait_check_ready(state);
		if (state->ready)
			break;			/* error never assigned */
		error = msleep_sbt(...);
```

`error` is assigned by `msleep_sbt()`, so breaking on the **first**
iteration returned whatever was on the stack — and that is the common
case, not an edge case: a wait on an object that is already signalled is
ready before the first sleep. The value goes to userland as the result of
the wait. The second readiness check below already spelled this out as
`error = 0`; the first now does too.

### `sys/netlink/route/nexthop.c` — a user-supplied weight of 0 panics the kernel

`newnhg()` took the weight straight out of a user-supplied `NHA_GROUP`
attribute:

```c
	wn[i].weight = grp[i].weight;
```

`nlattr_get_nhg()` validates that attribute's **length** and nothing else.
`calc_min_mpath_slots_fast()` then sorts the weights ascending, takes
`xmin = wn[0].storage`, and evaluates `total % xmin` — a kernel
divide-by-zero for weight 0. Its own comment says *"Assumes @wn is sorted
by weight ascending and each weight is > 0"* and nothing on this path made
that true.

Clamped rather than rejected, which is what `sys/netlink/route/rt.c:884`
does for `RTA_MULTIPATH` and `get_info_weight()` for `RTV_WEIGHT`. Linux
stores an `RTNH` weight biased by one, so a client sending 0 means the
default rather than an error.

### `sys/netinet/ip_reass.c` — an unvalidated boot tunable

`net.inet.ip.reass_hashsize` was fetched and used as given. Three things
require it to be a power of two greater than zero:

- `IPREASS_HMASK` is `V_ipq_hashsize - 1` and is used as a **mask**, so a
  non-power-of-two selects bucket indexes past the end of `V_ipq`;
- `ipq_reuse()` takes `% V_ipq_hashsize`;
- `ipreass_vnet_init()` divides by `V_ipq_hashsize / 2`.

Root-settable at boot only (`CTLFLAG_RDTUN`), so this is robustness rather
than a privilege boundary — but a typo in `loader.conf` should not corrupt
kernel memory. Now rejected with a message, and bounded above so a large
power of two cannot turn the `M_WAITOK` allocation into the failure.

`ipq_reuse()`'s `struct ipq *fp` is also initialised: it is assigned in a
loop that no longer has a zero-trip case, and every caller dereferences
what it returns.

### `sys/vm/vm_kern.c` — `kmem_back()` returns garbage for size 0

`rv` is assigned inside the loop, and the loop does not run when `size` is
0, so `return (rv)` returned stack contents that every caller compares
against `KERN_SUCCESS`. No caller in the tree passes 0 today
(`memguard_alloc()` returns early on it), so this changes no current
behaviour; it makes the function total rather than leaving the next caller
to find out.

---

## Not defects, and why they looked like defects

Kept because the reasoning is what stops them being re-reported.

| reported | why it is not a defect |
|---|---|
| `s_significand.c`: `-ilogb(x)` overflows | `math.h:45` defines `FP_ILOGB0` as `(-__INT_MAX)`, **not** `INT_MIN`, precisely so negation is safe. CBMC does not model `ilogb`, so its return was unconstrained. |
| `s_cosl`, `s_sinl`, `s_tanl`: `-n` overflows | `n` is written by `__ieee754_rem_pio2l`, also unmodelled. |
| `clock()`, `alarm()`, `svc_run()` | the values come from `getrusage`, `setitimer`, unmodelled externs. |
| `strcat`, `stpcpy`, `memrchr`, … | a nondeterministic `char *` includes NULL. These are the functions' *missing preconditions*, not bugs. |
| ~43 `lib/msun` "division by zero" | floating-point division by zero is **defined** by IEEE-754, and msun depends on it for `log(0)`, `logb`, `rsqrt` and `catrig`. |
| `nsap_addr.c:xtob`, `getopt_long.c:gcd` | `static`. Their callers constrain the domain; a modular check does not see callers. |
| `hash_buf.c:325`, `res_update.c:195`, `res_findzonecut.c:642` "use after free" | all three are `while ((p = HEAD(list))) { ...; UNLINK(list, p); free(p); }`. `UNLINK` updates `list.head` **before** the free — but only on the branch where `p->link.prev == NULL`, which the analyser cannot prove holds for a list head. |
| `gethostbyht.c:213`, `getnetbyht.c:182` "garbage returned" | the path needs `errno` to be non-zero at `return ((errno != 0) ? errno : -1)` and zero at the caller's `!= 0`. `errno` is `(*__error())`, a call the analyser re-evaluates opaquely, so it does not know the two reads agree. |
| `citrus_lookup_factory.c` "garbage returned" | same shape: `dump_db()` returns `errno` after a failed `malloc`, and the analyser does not model `malloc` setting `ENOMEM`. Reaching it needs `malloc` to return NULL with `errno == 0`, which FreeBSD's allocator does not do — `malloc(0)` returns a unique pointer rather than NULL. |
| `cpuset_alloc.c:32` `MallocSizeof` | `CPU_ALLOC_SIZE(n)` is `__BITSET_SIZE(n)`, a **byte count**, deliberately not `sizeof(cpuset_t)`. |
| `radixsort.c:109` `MallocSizeof` | `malloc(n * sizeof(a))` where `a` is `const u_char **` and the elements are `const u_char *`. Both are pointers, so the size is right on every supported target; `sizeof(*a)` would say so more clearly. |
| `g_stripe.c:111`, `g_shsec.c:107` "division by zero" | `lcm(a, b)` is `(a * b) / gcd(a, b)`, `static`, and every caller passes a sector size. |
