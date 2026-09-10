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

## Fixed — a NULL that nothing checked, five more

`M_WAITOK` cannot fail. `M_NOWAIT` can, and returns NULL. The difference is
one token, and these are the places where the second one is written and the
result is used anyway.

### `sys/geom/gate/g_gate.c` — a KASSERT is not a check

```c
	else if (unit == G_GATE_NAME_GIVEN) {
		KASSERT(name != NULL, ("name is NULL"));
		...
			if (strcmp(name, ...) != 0)
```

`unit` is `ggio->gctl_unit`, straight from an ioctl on `/dev/ggctl`, and
`G_GATE_CMD_MODIFY`, `G_GATE_CMD_START` and `G_GATE_CMD_DONE` all call this
as `g_gate_hold(ggio->gctl_unit, NULL)`. So `gctl_unit = G_GATE_NAME_GIVEN`
reached `strcmp(NULL, ...)`.

`KASSERT` is the wrong tool twice over: without `INVARIANTS` it compiles to
nothing and the kernel dereferences NULL, and *with* `INVARIANTS` it panics
on a userland argument, which is not what an assertion is for.

**Which of those PBSD gets, today, is the second one.** `HARDENEDBSD` and
`HARDENEDBSD-MINIMAL` both set `INVARIANTS` (via `sys/conf/std.debug`);
`GENERIC` does not. So on the kernels PBSD currently builds this is a panic
from an ioctl argument rather than a NULL dereference — and on the release
kernel PBSD will eventually ship, it becomes the NULL dereference. Both are
wrong and the fix answers both.
`g_gate_create()` validates both fields (`:490`, `:496`); the other three
commands did not. Now the loop is simply not entered, leaving `sc == NULL`,
which every caller already turns into `ENXIO` or `ENOENT`.

### `sys/netgraph/netflow/ng_netflow.c` — three of four

`NG_MKRESPONSE(..., M_NOWAIT)` leaves the pointer NULL on failure —
`ng_message.h:402` breaks out of the macro without touching it. Four call
sites in this file; `:539` checks, and `:388`, `:411` and `:556`
dereferenced it.

**clang's analyser reported exactly one of the three.** It explores paths
and stops at the first defect on each, so three instances of one mistake in
one file came back as one finding. That is what
`tools/verify/nowait_check.py` exists for.

### `sys/fs/p9fs/p9_protocol.c` — a length off the wire

```c
	nwname = *nwname_p;			/* uint16_t, read from the server */
	wnames = malloc(sizeof(char *) * nwname, M_TEMP, M_NOWAIT | M_ZERO);

	for (i = 0; i < nwname && (error == 0); i++)
		error = p9_buf_readf(buf, proto_version, "s", &wnames[i]);
```

Up to 512KB asked for with `M_NOWAIT`, and on failure the loop writes
through `&wnames[i]` for every `i` the *server* chose — and the error path
below then frees `wnames[i]` from the same pointer. A hostile or
compromised 9P server plus memory pressure is a kernel write through NULL.

### `sys/dev/usb/net/uhso.c` — device data into a NULL mbuf

```c
	m = m_getcl(M_NOWAIT, MT_DATA, M_PKTHDR);
	usbd_copy_out(pc, 0, mtod(m, uint8_t *), actlen);
```

`mtod(NULL, ...)` is a NULL dereference, and `usbd_copy_out()` writes
`actlen` bytes of device data into it. A USB device that keeps sending
while memory is tight panics the kernel. Now counted as `IQDROPS` and
dropped, like every other receive failure in the file.

### `sys/dev/enic/vnic_dev.c` — registers into an unchecked allocation

`ENIC_BUS_READ_REGION_4()` reads device registers straight into the result
of an unchecked `malloc(..., M_NOWAIT | M_ZERO)`, and the loop after it
dereferences the same pointer.

### Fixed — the nine that were reported and not fixed

All nine are device attach and setup paths, each a separate vendor edit,
and each was left listed rather than changed on the grounds that PBSD does
not boot those drivers. That reasoning does not survive the fact that they
are one-line fixes in code whose *own* error convention was sitting beside
them:

| file | what it already had |
|---|---|
| `sys/dev/bhnd/bhndb/bhndb_subr.c:899` | its own doc comment — `@retval NULL if allocation fails` — for a function that could not return NULL because it faulted first |
| `sys/dev/enic/vnic_dev.c:140` | a `return NULL` five lines up, for the other failure |
| `sys/dev/mxge/if_mxge.c:4450` | `err = ENXIO; goto abort_with_res;` immediately above |
| `sys/arm/freescale/imx/imx6_ssi.c:757`, `sys/arm/freescale/vybrid/vf_sai.c:712` | `return (ENXIO)` four lines up. The same driver twice — two SoCs, one copy |
| `sys/dev/enic/if_enic.c:558` | `return (error)` inside the loop below. Two allocations, neither checked |
| `sys/dev/sound/pci/hdsp-pcm.c:724` | `return (NULL)` below, and `sc->lock` held — so the fix has to unlock |
| `sys/dev/ufshci/ufshci_req_sdb.c:167` | `return (error)`. `&req_queue->hwq[0]` is not a dereference; `hwq->num_entries` two lines down is. Its second `M_NOWAIT` was unchecked too |
| `sys/arm/nvidia/tegra124/tegra124_cpufreq.c:264` | nothing — `build_speed_points()` returns `void` |

The tegra one is the only one that needed more than a check.
`get_speed_point()` reads `speed_points[0]` with no bound of its own, so
an empty table is no more survivable than a NULL one; the function gets a
return value (it has one caller) and attach fails.

**And the lint is a gate now.** It was report-only because it had false
positives. It has four documented classes it handles, the nine real sites
are fixed, and the tree is at zero — so `--gate` is about the *next* one.
Verified by reverting the `vnic_dev.c` fix alone and watching it name that
file and exit 1.

A fifth false-positive class gets fixed by teaching `sites()` about it,
never by an allowlist. An allowlist here would hide the next real one,
which is the exact failure this lint exists to prevent.

---

## Fixed — arm64, which nothing had ever actually checked

`arch_of()` mapped `lib/libc/<arch>` and `lib/msun/<arch>` to that
architecture's headers and said nothing about `sys/`, so 671 kernel files
under `sys/arm64`, `sys/arm`, `sys/powerpc` and `sys/riscv` were compiled
against **amd64's** `machine/`. `sys/arm64` went from 9 usable translation
units to 108 of 164 when that was fixed, and 13 findings appeared in code
that had never been looked at.

### `sys/arm64/arm64/identcpu.c` — the guard tests the index and uses the pointer

```c
	prev_desc = NULL;
	CPU_FOREACH(cpu) {
		desc = get_cpu_desc(cpu);
		if (cpu != 0) {
			check_cpu_regs(cpu, desc, prev_desc);
```

`prev_desc` is set at the end of each iteration, so `cpu != 0` and
`prev_desc != NULL` are the same question **only while CPU 0 is in
`all_cpus` and `CPU_FOREACH` reaches it first**. If it is not, the first
iteration passes NULL and `check_cpu_regs()` dereferences it five times,
during `SI_SUB_CPU`. The condition now tests what it means to test.

Twelve of the thirteen did not survive reading, and the reasoning is in the
last section — but they were only *available* to read because the
architecture fix made the files compile. PBSD's claim is that every
architecture is first class; a verifier checking four of the six against
the wrong headers is not that.

---

## Fixed — a peer's RPC reply could abort any client

`lib/libc/rpc/rpc_prot.c:159`, found by the model checker as
`line 159 assertion 0` — the only failure on that function, with valid
pointers supplied.

```c
	/* personalized union, rather than calling xdr_union */
	if (! xdr_enum(xdrs, (enum_t *) prj_stat))
		return (FALSE);
	switch (rr->rj_stat) {
	case RPC_MISMATCH: ...
	case AUTH_ERROR:   ...
	}
	/* NOTREACHED */
	assert(0);
	return (FALSE);
```

`enum reject_stat` has exactly two values, `RPC_MISMATCH = 0` and
`AUTH_ERROR = 1`. `rj_stat` was decoded from the **wire** one line earlier
by `xdr_enum()`, which reads a 32-bit integer and does not range check it.
A server replying `MSG_DENIED` with `rj_stat = 2` lands on the `assert`,
and **libc is not built `-DNDEBUG`** — so `assert(0)` calls `abort()`.

Any RPC client — NFS, NIS, rpcbind, anything linking libc's RPC — killed
by a value its peer chose. `/* NOTREACHED */` is true of a correct peer
and false of the network.

The fix is the line already beneath it. `FALSE` is what every other
malformed-input path in the file returns and what every caller of
`xdr_replymsg()` already handles. It is the only `assert(0)` in libc's RPC
and XDR code.

---

## Reported, not fixed — CAM's "insulate against a race" does not insulate

`sys/cam/cam_xpt.c`'s `xpt_done_process()` says exactly what it is doing:

```c
	/*
	 * Insulate against a race where the periph is destroyed but CCBs are
	 * still not all processed. This shouldn't happen, but allows us better
	 * bug diagnostic when it does.
	 */
	if (ccb_h->path->bus)
		sim = ccb_h->path->bus->sim;

	if (ccb_h->status & CAM_RELEASE_SIMQ) {
		KASSERT(sim, ("sim missing for CAM_RELEASE_SIMQ request"));
		xpt_release_simq(sim, /*run_queue*/FALSE);
```

The guard exists because `bus` can be NULL. Three lines later `sim` is
passed to `xpt_release_simq()`, which does `devq = sim->devq`. The same
function does it again at `:5325`:

```c
		if (sim)
			devq = sim->devq;
		KASSERT(devq, ("Periph disappeared with CCB %p ..."));

		mtx_lock(&devq->send_mtx);
```

`KASSERT` is not a check — without `INVARIANTS` it compiles to nothing —
so in a production kernel the insulation is defeated on the next line,
twice, in the completion path of every SCSI and ATA command. PBSD's current
configs do set `INVARIANTS`, so today it panics with a message instead;
`GENERIC` does not, and neither will a release kernel.

**Not fixed here**, and deliberately. The first one could be made
`if (sim != NULL)`, but the effect of skipping `xpt_release_simq()` is a
SIM queue left frozen: a hang instead of a panic. The second cannot be
skipped at all — `cam_ccbq_ccb_done()` is inside it, and not calling it
leaves the CCB accounted active forever. What the kernel *should* do when a
periph vanished mid-CCB is a design question CAM answered with "assert and
find out", and changing the failure semantics of the storage completion
path is not a change to make from a static analyser finding.

Same rule as the four integer overflows above: recorded with the reasoning,
because the reasoning is the part worth having.

`sys/cam/scsi/scsi_enc_ses.c:2762` and `:2792` are two more uninitialised
returns in the same subsystem and are not yet read.

---

## Fixed — a divisor that means "not running"

Three kernel divisions by zero, all of the same shape: a value that is
zero to mean *absent* is used as a divisor by code that reads it as
*small*.

### `sys/netinet/igmp.c:1199,1212` and `sys/netinet6/mld6.c:1005,1018` — a remote querier's divide by zero

`IGMP_RANDOM_DELAY(X)` is `random() % (X) + 1`. `igmp_input_v3_group_query()`
gets a `timer` the caller has already clamped to at least 1
(`igmp.c:1036`), and then does

```c
timer = min(inm->inm_timer, timer);
inm->inm_timer = IGMP_RANDOM_DELAY(timer);
```

`inm_timer == 0` means **the timer is not running** — that is exactly how
`igmp_v3_process_group_timers()` reads it at `:1867`, returning early
rather than treating 0 as expiry. Taking it as a minimum reads a stopped
timer as the soonest possible deadline, and the result is `random() % 0`
in the kernel, from a packet.

These are the only two of the file's four `IGMP_RANDOM_DELAY` call sites
that take a `min()` with that field, and they are the two clang's
analyser reported. The fix takes the minimum only when the timer is
running, which is what the `min()` was for.

Reachability was **not** established: every path that zeroes `inm_timer`
also moves the group out of `G_QUERY_PENDING_MEMBER`
(`igmp_v3_cancel_link_timers()`, `igmp_final_leave()`, and the fast
timeout itself). The invariant holds today across five functions and
nothing local to the divide enforces it.

`sys/netinet6/mld6.c` is the same code for IPv6 — `arc4random() % (X) + 1`,
the same `min()`, `in6m_timer == 0` meaning stopped at `mld6.c:1450`. The
analyser reported **nothing** for that file, and the file compiled
cleanly: a path-sensitive checker explores paths, not classes, so one of
two identical files came back clean. Same lesson as `ng_netflow.c`, where
three instances of one mistake in one file were reported as one finding.

### `sys/net/route/nhgrp_ctl.c:168` — an abort path the divide made unreachable

`calc_min_mpath_slots_fast()` sets `xmin = wn[0].storage` and evaluates
`total % xmin`. Its own comment says "Assumes @wn is sorted by weight
ascending and **each weight is > 0**", and `xmin` is 0 exactly when every
weight is 0.

`alloc_nhgrp()` already handles that case — its `if (nhgrp_size == 0)`
branch is commented *"Zero weights, abort"* — and could never reach it,
because the divide happens first. Returning 0 is this function's own
documented "precise calculation failed" and the caller
(`calc_min_mpath_slots()`) already handles it.

All four callers clamp the weight today: netlink `NHA_GROUP` (fixed
above), rtsock `RTA_MULTIPATH` at `rt.c:884`, `RTV_WEIGHT` via
`get_info_weight()`, and propagation from an existing group. The
precondition is four callers' responsibility and nothing checks it where
it is depended on — which is how a user-supplied weight of 0 reached this
line once already.

## Fixed — four uninitialised returns, three of them reachable

`core.uninitialized.UndefReturn`. All four are the same mistake: a
variable assigned only inside a loop or a `switch` arm that some path
does not take, then returned.

### `sys/vm/vm_mmap.c` — `mincore(addr, 0, vec)` returns stack garbage

`kern_mincore()` declares `int error` and assigns it only inside the scan
loop and the trailing zero-fill loop. `len == 0` is not rejected — `end <
addr` is the only bound above — so with `addr` page-aligned at the start
of a map entry:

* `end == addr`, so `while (entry->start < end)` is false;
* `vecindex = atop(end - first_addr)` is 0 and `lastvecindex` is -1, so
  `while ((lastvecindex + 1) < vecindex)` is false.

Both loops are skipped and `return (error)` hands an uninitialised `int`
back as the syscall's return value. **Unprivileged, one call, no setup.**

### `sys/net/if.c` — `SIOCGIFGROUP` on an interface with no groups

`if_getgroup()`'s `ifgr_len == 0` branch ends with an explicit
`error = 0;`. The other branch sets `error` only inside
`CK_STAILQ_FOREACH(ifgl, &ifp->if_groups, ...)`, and an interface with an
empty group list never enters it. The author was thinking about this — in
one of the two branches.

### `sys/kern/vfs_lookup.c` — two `continue`s above the only assignment

`vfs_lookup_cross_mount()` assigns `error` at its `VFS_ROOT()` call near
the end of the body. Two `continue`s sit above it: the `dp->v_mountedhere
!= mp` recheck and the `vfs_busy(mp, 0) != 0` failure. An iteration that
takes either and then finds `dp` is no longer a mountpoint leaves the
`do`/`while` with `error` never assigned, returning garbage into
`namei()`.

### `sys/fs/p9fs/p9fs_vnops.c` — a two-bit `switch` with three cases

`p9fs_uflags_mode()` switches on `uflags & 3` with `O_RDONLY`, `O_WRONLY`
and `O_RDWR`, and no `default`. `OFLAGS()` is `FFLAGS()` undone, so a
descriptor carrying neither `FREAD` nor `FWRITE` — `O_EXEC`, `O_PATH` —
gives `OFLAGS(0) == -1` and `(-1 & 3) == 3`. `ret` is then uninitialised
and goes on the 9P wire as the open mode. The function has no error
channel, so the unreadable-and-unwritable case now asks for the least
(`P9PROTO_OREAD`), and the `default:` makes the switch total.

### `sys/kern/kern_event.c` ×2 — not reachable, and fixed anyway

`kevent11_copyout()` and `kevent11_copyin()` assign `error` only inside
their loop. Their two non-compat siblings handle `count == 0` **by
construction** — `copyout(p, u, 0)` is a well-defined no-op returning 0 —
and no caller passes 0 today (`kqueue_scan` guards with `nkev != 0`,
`kern_kevent` with `n >= 1`). This is the four `kevent_copyops`
implementations agreeing on a contract, not a reachable bug, and it is
recorded that way.

All five files are registered in `tools/check_pbsd_marks.py`, verified by
reverting each fix in turn and watching the gate name that file. `FIXES`
now takes a list per file, because `sys/vm/vm_mmap.c` needed a second
entry alongside its `MAP_32BIT` one.

## Fixed — two GEOM tasters, which run on whatever is plugged in

`g_taste` is called for every provider that appears, so the parsers below
read attacker-supplied bytes with no authentication of any kind. That
raises the bar for "not reachable" here: an invariant that holds across
five functions today is not a bound on a removable medium.

### `sys/geom/part/g_part_ldm.c` — a validator that checks one field of two

`ldm_vmdbhdr_check()` reads both `dh.size` and `dh.last_seq` off the disk
and rejects only the first for being zero:

```c
if (error != 0 || db->dh.size == 0 ||
    pp->sectorsize % db->dh.size != 0 || ...
    db->dh.size * db->dh.last_seq > db->ph.conf_size * pp->sectorsize) {
```

`ldm_vmdb_parse()` then computes, on a `size_t`:

```c
size = howmany(db->dh.last_seq * db->dh.size, pp->sectorsize);
size -= 1;      /* one sector takes vmdb header */
```

`last_seq == 0` makes `howmany()` return 0 and the subtraction `SIZE_MAX`,
so the read loop's bound is nonsense — it walks off the end of the
provider until `g_read_data()` fails. It also passes the
`size * last_seq > conf_size * sectorsize` test trivially, by multiplying
to zero, which is why the one bound that looks like it covers both fields
does not. `last_seq == 0` is rejected beside `dh.size == 0` now.

`buf` in the same function is also initialised: `fail:` frees it
unconditionally and the loop that assigns it can have zero iterations.
That one is not reachable — with `last_seq` non-zero the xVBLK loop
between them cannot run either — and is a free of a stack value if it
ever becomes so.

### `sys/geom/linux_lvm/g_linux_lvm.c` — one of two locals zeroed

`g_llvm_taste()` declares `struct g_llvm_label ll;` and
`struct g_llvm_metadata md;` and zeroes only `md`.
`llvm_label_decode()` has four early `return (EINVAL)`s, two of them
above the assignment to `ll_md_offset` that `g_llvm_read_md()` then uses
as a disk offset. Every path returning 0 does set it, so this is not
reachable today; it is one line beside a line that already does it, in a
routine that runs on every medium that appears.

### Not a defect, in the same file

`g_llvm_taste():567` dereferences `md.md_vg` with no check.
`llvm_textconf_decode()` sets `md->md_vg` on both of its `return (0)`
paths and returns `EINVAL` or `-1` otherwise, and the caller checks
`error != 0` first. The analyser cannot correlate a callee's return value
with which of its assignments ran.

## Fixed — a two-bit switch with three arms, copied to userland

`sys/kern/kern_procctl.c`, `protmax_status()` at `:750` and
`aslr_status()` at `:799`:

```c
int d;

switch (p->p_flag2 & (P2_PROTMAX_ENABLE | P2_PROTMAX_DISABLE)) {
case 0:                   d = PROC_PROTMAX_NOFORCE;       break;
case P2_PROTMAX_ENABLE:   d = PROC_PROTMAX_FORCE_ENABLE;  break;
case P2_PROTMAX_DISABLE:  d = PROC_PROTMAX_FORCE_DISABLE; break;
}
if (kern_mmap_maxprot(p, PROT_READ) == PROT_READ)
        d |= PROC_PROTMAX_ACTIVE;
*(int *)data = d;
```

The mask has **two bits**, so four values; the switch has **three arms**.
`ENABLE|DISABLE` together falls through with `d` uninitialised, gets
`|=`'d, and `*(int *)data = d` copies it to the caller of `procctl(2)`.
That is a kernel stack disclosure, not a wrong answer.

Not reachable as the tree stands, and the reason is the point:
`protmax_ctl()` clears the opposite bit on all three of its arms,
`kern_fork.c:552` inherits a pair that already satisfies that, and
`imgact_elf.c:1182` clears both. The invariant is maintained in three
files and depended on in a fourth — in a tree that is actively adding
`PAX` variants to `p_flag2` handling in this very file (`wxmap_ctl`'s
`#ifndef PAX`). One new `p_flag2 |=` anywhere turns a masked-switch gap
into an infoleak.

`PROC_PROTMAX_NOFORCE` / `PROC_ASLR_NOFORCE` are what `case 0` already
says, and are the honest answer for a bit pair that forces nothing
coherent. Both sites, one token each, no behaviour change on any
reachable path.

These are the only two masked switches on `p_flag` or `p_flag2` in the
tree — `stackgap_status()` beside them uses ternaries and is total, and
the two `switch (state)` forms take user input and have `default: return
(EINVAL)`.

## Fixed — two header macros that shift into bit 31 of a signed `int`

Both were invisible until `report.py` stopped letting one pointer failure
decide a whole record (see below). Both are arm64 headers used across the
tree, and one of them is a functional bug and not only undefined
behaviour.

### `sys/arm64/include/cpu.h` — `CPU_MATCH()` cannot match implementer ≥ 0x80

```c
#define CPU_IMPL_MASK       (0xff << 24)
#define CPU_IMPL_TO_MIDR(v) (((v) & 0xff) << 24)
```

`0xff << 24` is 4278190080, which is not representable in `int`. UBSan
says so directly — *"left shift of 255 by 24 places cannot be represented
in type 'int'"* — but the value is wrong as well as undefined, and that
is the part that matters. As a signed `int`, `0xff000000` is **negative**,
so in

```c
#define CPU_MATCH(mask, impl, part, var, rev)   \
    (((mask) & PCPU_GET(midr)) ==               \
     ((mask) & CPU_ID_RAW((impl), (part), (var), (rev))))
```

the left side is `int & uint64_t` (the mask sign-extends, `midr`'s top
bits are RES0, result positive) and the right side is `int & int`
(stays negative, then sign-extends at the `==`). Measured, for an
implementer code of `0xC0`:

```
lhs = 0x00000000c0000000
rhs = 0xffffffffc0000000     CPU_MATCH -> 0
```

`CPU_IMPL_AMPERE` is `0xC0`. It appears only in `identcpu.c`'s name
table today, which compares the extracted field directly, so no erratum
is currently misapplied — the next `0x80`+ implementer needing a
workaround would have got one that silently never fired. With the `U`
suffixes both sides read `0xc0000000` and it matches; `APM` (`0x50`) is
unchanged, checked.

### `sys/dev/psci/smccc.h` — the SMC function ID is sign-extended into x0

```c
#define SMCCC_FUNC_ID(type, call_conv, range, func) \
        (((type) << 31) | ((call_conv) << 30) | ...)
```

`type` is `SMCCC_FAST_CALL`, which is 1, for **every** SMCCC call in the
tree. `1 << 31` is UB, and the negative `int` it produces is passed to
`psci_call()`, whose parameters are `register_t` — `int64_t` on arm64.
Measured:

```
old  fid as register_t = 0xffffffff80000000
new  fid as register_t = 0x0000000080000000
```

Arm DEN 0028 puts the function identifier in the low 32 bits as an
unsigned value. Monitors evidently ignore the high half, which is why
nothing has noticed.

### `sys/i386/pci/pci_cfgreg.c` — the same shift, executed

Three sites, `1 << slot` with `slot` a variable:

```c
if (domain == 0 && bus == 0 && (1 << slot & pcie_badslots) != 0)   /* :177, :223 */
...
for (slot = 0; slot <= PCI_SLOTMAX; slot++)
        ...
        pcie_badslots |= (1 << slot);                              /* :537 */
```

`PCI_SLOTMAX` is 31 (`sys/dev/pci/pcireg.h:47`), so `1 << 31` on a signed
`int` is **executed on every boot of a machine with PCIe** — and device 31
is where the LPC bridge lives on Intel chipsets, so it is not a corner
either.

This one is fixed and the 977 below are not, and the line is not
arbitrary: those are `#define` constants the compiler folds, and these are
a shift of a variable at run time — the thing UBSan traps and the thing a
compiler is entitled to assume cannot happen. The value is unchanged
either way (`1 << 31` is `INT_MIN`, which converts to `0x80000000` in the
`uint32_t` it is masked against), so `1U` is two characters and no
behaviour change.

It was found because `includes.py` started passing `--target`, so i386
was compiled as i386 rather than as x86-64 for the first time. It is the
first defect that fix produced.

### The rest of the class, and why it is not being fixed

Read the class, then grep for it — and this time the grep says don't.
A signed literal shifted past `INT_MAX` in a `#define`, excluding
`contrib/`, `cddl/` and `linuxkpi/`:

```
977 signed shift(s) past INT_MAX in a sys/ header #define
```

`(1 << 31)` for a bit-31 register flag is how every BSD and Linux driver
header in existence has always been written. All 977 are undefined
behaviour and essentially none of them matter: the value is assigned to a
`uint32_t` register field, the compiler produces the right bits, and
nothing widens or compares it.

The two that were fixed are different because each had **a consequence
beyond the undefined behaviour** — one made a comparison give the wrong
answer, the other put the wrong value in a 64-bit register. That is the
line: not "is it UB", but "does the sign change what the code does".

Fixing all 977 would be a 977-line vendor diff that conflicts on every
upstream merge and changes no behaviour. It is recorded here so that the
number is known and the decision is explicit rather than an omission
somebody has to rediscover.

### How they became visible

`report.py`'s `bucket()` returned one bucket per record, and *any*
failure matching `PTR_WORDS` sent the whole record to "pointer/memory (a
missing precondition, not a bug)". A translation unit reporting both

```
dereference failure: pointer NULL in ...
arithmetic overflow on signed shl in ...
```

was dismissed and its overflow was never printed. In the kernel sweep
that hid 9 signed `+`, 5 signed `shl`, 3 array upper-bound, 2 signed `-`,
2 signed `*` and 2 array lower-bound behind 176 NULL-pointer
preconditions — and an array bound is memory safety, not a missing
precondition.

The fix is the one the `msun` comment three lines below already
described and which had never been applied to this case: **strip** the
pointer failures, then classify what is left. "Worth a person's time"
went from 65 to 177 across the three sweeps, 4 to 23 in the kernel.

## Fixed — the one of four that does not check its index

`sys/kern/posix4_mib.c` has four functions that index
`facility[num - 1]`, and `P31B_VALID(num)` — `(num) >= 1 && (num) <
CTL_P1003_1B_MAXID` — is right there at `:106`:

| function | guard |
|---|---|
| `p31b_sysctl_proc()` `:114` | `if (!P31B_VALID(num)) return (EINVAL);` |
| `p31b_setcfg()` `:129` | `if (P31B_VALID(num)) { … }` |
| **`p31b_unsetcfg()` `:136`** | **none** |
| `p31b_getcfg()` `:147` | `if (P31B_VALID(num)) return …` |

It sits between two functions that use the guard, it is the only one of
the four that **writes without reading first**, it writes *two* static
arrays, and it is exported in `sys/sys/posix4.h`. `num == 0` gives
`facility[-1]`.

Both callers today are `sys/kern/uipc_sem.c:1065-1066` with
`CTL_P1003_1B_` constants, so it is not reachable as the tree stands.

This is the shape every real defect found this week has had: **the guard
exists on one of a pair, or on three of four.** `if_getgroup()` set
`error = 0` in one of its two branches; `ldm_vmdbhdr_check()` rejected a
zero `dh.size` and not a zero `last_seq`; `g_llvm_taste()` zeroed `md`
and not `ll`; `protmax_status()` had three arms for a two-bit mask. The
useful question about a finding is not "can I reach it" but "does the
code right beside it already do the thing this one does not".

### Ruled out from the same batch

| finding | why |
|---|---|
| `sys/arm/arm/identcpu-v6.c:260` `hw_buf_idx + len` | `hw_buf` is `char[81]`, the guard above resets at `hw_buf_idx + len + 2 >= 79`, and the longest string any caller passes is 17. CBMC cannot bound a `static int`. |
| `sys/dev/videomode/pickmode.c:77` `/(htotal * vtotal)` | `videomode_list[]` is a generated `const` table compiled into the kernel, not anything a monitor supplies. Counted exactly in the table at the end of this document — this row said 46 from an eyeball and it is 92. |
| `sys/i386/i386/machdep.c:1807` `md_spinlock_count - 1`, `sys/x86/x86/delay.c` `td_pinned + 1` | per-thread counters whose invariant is held by a paired enter/exit in another function. |
| `sys/ddb/db_access.c:69,72` | `size` is 1, 2, 4 or 8 and `value` accumulates that many bytes; the operand of `<< 8` is `db_expr_t`, which is signed by design because DDB expressions are. |
| `sys/cam/cam_queue.c:60,274` | `size` and `openings` are a driver's own queue depth. |

**A sixth rule is doing most of that work, and it is worth naming:
module state is as unconstrained to a modular checker as a parameter is.**
`hw_buf_idx`, `md_spinlock_count`, `td_pinned` and `facility` are a
`static int`, two struct fields and a file-scope array. Rule three says
`static` *linkage* means callers constrain the domain; this is the same
argument for *storage* — CBMC starts every function with every global and
every struct field nondeterministic, so an invariant maintained across
calls is invisible to it in exactly the way a caller's precondition is.

## Fixed — three defects in one twenty-five-line arm clock function

`sys/arm/ti/clk/ti_clk_dpll.c`, `ti_dpll_clk_find_best()`:

```c
uint64_t cur, best;                     /* never assigned before use */
...
min_p = ti_clk_factor_get_min(&sc->p);
for (p = min_p; p <= max_p; ) {
        for (n = min_n; n <= max_n; ) {
                cur = fparent * n / p;                    /* p can be 0 */
                if (abs(*fout - cur) < abs(*fout - best)) /* best is garbage */
                        best = cur;
                ...
        }
}
return (best);                          /* garbage if either loop is empty */
```

1. `best` is **read** at the first comparison, before anything assigns it.
2. `best` is **returned** uninitialised when either loop has zero
   iterations.
3. `p` is a divisor, and `ti_clk_factor_get_min()` returns **0** for a
   factor with `TI_CLK_FACTOR_ZERO_BASED` (`:100-101`) — so a zero-based
   `p` factor starts the loop at a divisor of zero.

`best = 0` matches `*factor_n` and `*factor_p`, which the function
already zeroes for the empty case, and on the first real iteration
`abs(*fout - cur)` beats `abs(*fout - 0)` for any candidate near the
target, so the first candidate still wins as intended. `p == 0` is
skipped, because zero is never a valid divisor.

This is `sys/arm`, which had **one** usable translation unit out of 322
before `--target` landed. Nothing here had ever been looked at.

### The gate caught an over-broad marker of mine

Registering the fix with `"uint64_t cur, best;"` as the must-not-appear
string failed immediately, and correctly: `ti_dpll_clk_set_freq()` forty
lines down has the identical declaration and assigns `best = cur = 0;` on
the next line. The marker was not unique; the code was fine. The `want`
string alone does the job, and a marker that matches a second function is
the same mistake as a lint that reads its own comment.

## Fixed — four unvalidated loader tunables in the NFS server

`sys/fs/nfsserver/nfs_nfsdstate.c:68-86` declares four hash sizes, all
`CTLFLAG_RDTUN` — taken from `loader.conf` and never looked at again —
and nothing validated any of them. Three are used as a **modulus**:

```c
nfsrvstate.h:59   nfsclienthash[(id).lval[1] % nfsrv_clienthashsize]
nfsrvstate.h:61   lc_stateid[(id).other[2]    % nfsrv_statehashsize]
nfsdport.h:95     nfslockhash[nfsrv_hashfh(f) % nfsrv_lockhashsize]
```

So `vfs.nfsd.clienthashsize=0` in `loader.conf` is a kernel division by
zero the first time a client connects, and `sessionhashsize=0` gives a
zero-length allocation that `nfsd_init()` then indexes. A **negative**
value is worse: `sizeof(x) * n` converts to `size_t`, so the three
`M_WAITOK` allocations ask for something near `SIZE_MAX`.

This is the same shape as `net.inet.ip.reass_hashsize`, fixed earlier in
`sys/netinet/ip_reass.c:691`, and it is clamped the same way — a `printf`
and the compiled default rather than a panic, so a typo in `loader.conf`
still boots. Unlike `ip_reass` these are not powers of two (20, 20, 20,
10), so only positivity is required.

All four are checked in `nfsd_init()`, which runs once per vnet before
any client is served, rather than at each use.

**The analyser still reports `nfs_nfsdstate.c:415` after the fix**, and
that is correct of it: `nfsd_init()` is in `nfs_nfsdsubs.c`, a different
translation unit. It is the same boundary described above for
`g_read_data()` — a finding in one file that is a fact about a callee in
another.

## Fixed — three in GEOM, two of them reachable without privilege

`sys/geom` is the part of the kernel that reads bytes off whatever is
plugged in and believes them. Its tasters run on every provider that
appears, before any policy, and `kern.geom.confxml` is `CTLFLAG_RD` — any
user can read it. Both of those are attacker-adjacent by construction, so
the divisor guards there are load-bearing in a way they are not in a
driver attach path.

### `sys/geom/raid/md_promise.c` — a disk count of zero, then two divisions by it

`promise_meta_read()` validated `meta->total_disks` at `:390`:

```c
	if (meta->total_disks > PROMISE_MAX_DISKS) {
```

One bound of two. `total_disks` is a `uint8_t` read straight off the
medium (`:929` copies it into `vol->v_disks_count`), and
`promise_meta_translate_disk()` is the consumer:

```c
	if (md_disk_pos >= 0 && vol->v_raid_level == G_RAID_VOLUME_RL_RAID1E) {
		width = vol->v_disks_count / 2;
		disk_pos = (md_disk_pos / width) +
		    (md_disk_pos % width) * width;
```

`width` is a divisor twice on one line and nothing above establishes it is
non-zero. RAID1E is selected at `:912-916` by `type == PROMISE_T_RAID1 &&
array_width != 1`, which does not mention `total_disks` at all, so a
volume claiming RAID1E with one disk halves to zero.

Both halves are fixed. `promise_meta_read()` now rejects `total_disks ==
0` — a volume with no disks in it is not a volume, and every other
consumer indexes by it — and the RAID1E branch is conditional on the
divisor it is about to use:

```c
	if (md_disk_pos >= 0 && vol->v_raid_level == G_RAID_VOLUME_RL_RAID1E &&
	    (width = vol->v_disks_count / 2) > 0) {
```

The `else` branch, which every other RAID level already takes, leaves the
position untranslated. That is the right answer for metadata that
describes a geometry it cannot have.

### `sys/geom/virstor/g_virstor.c` — three guards, all on the numerator

`g_virstor_dumpconf()` had this, twice in two different shapes:

```c
	sbuf_printf(sb, "%s<StorageFree>%u%%</StorageFree>\n", indent,
	    comp->chunk_next > 0 ? 100 -
	    ((comp->chunk_next + comp->chunk_reserved) * 100) /
	    comp->chunk_count : 100);
	...
	sbuf_printf(sb, "%s<State>%u%% physical free</State>\n",
	    indent, 100-(used * 100) / count);
	...
	sbuf_printf(sb, "%s<PhysicalFree>%u%%</PhysicalFree>\n",
	    indent, used > 0 ? 100 - (used * 100) / count : 100);
```

Three divisions, two guards, and neither guard is on a divisor. The third
line is the tell: the author reached for a guard, wrote `used > 0` — the
numerator — and the divisor `count` went unchecked one line above and one
line below. It is the same confusion three times, which is why the middle
one has no guard at all.

`count` is the sum of `chunk_count` over the components that are
**attached**, so it is zero for a virstor whose components have all gone
away — a state this same function prints two lines earlier as `Online=0`.
`g_virstor_dumpconf()` runs for `kern.geom.confxml`. That is an
unprivileged kernel division by zero, and the sweep found it: `analyze.jsonl`
from the run before this fix carries
`sys/geom/virstor/g_virstor.c:1502 core.DivideZero`. All three guards are
on `count` / `chunk_count` now, and it is gone from the re-run.

### `sys/geom/virstor/g_virstor.c:700` — `||` where `&` was meant

Found by reading, not by a tool, while confirming the finding above:

```c
		for (n = 0; n < sc->chunk_count; n++) {
			if (sc->map[n].flags || VIRSTOR_MAP_ALLOCATED != 0)
				count++;
		}
```

`VIRSTOR_MAP_ALLOCATED` is `1` (`g_virstor.h:34`), so
`VIRSTOR_MAP_ALLOCATED != 0` is the constant true, the `||` short-circuits
to it whatever `flags` holds, and `count` ends up as `chunk_count`. The
message two lines down — *"Device %s has %d allocated chunks"* — has
therefore been printing the total chunk count on every `INVARIANTS` kernel
since the code was written. Every other test of this bit in the same file
(`:1336`, `:1628`, `:1659`, `:1858`) is `&`.

It is diagnostic output inside `#ifdef INVARIANTS`, so nothing downstream
is wrong — but it is also why no instrument saw it: the analyser compiles
the default configuration, and this block is not in it. A precedence-shaped
typo in code nobody compiles is the least visible defect class there is.

## Fixed — an int from a userland ccb, bounded below and not above

`sys/cam/cam_queue.c`, `cam_ccbq_resize()`:

```c
	delta = new_size - (ccbq->dev_active + ccbq->dev_openings);
	ccbq->total_openings += delta;
	ccbq->dev_openings += delta;

	new_size = imax(64, 1 << fls(new_size + new_size / 2));
```

`new_size + new_size / 2` overflows a signed `int` above two thirds of
`INT_MAX`. Measured rather than reasoned:

```
new_size            = 2147483647
n + n/2 exact       = 3221225470  (INT_MAX = 2147483647)
n + n/2 as int      = -1073741826   <- signed overflow
fls(that)           = 32            <- shift count == width of int
```

So `1 << fls(...)` then shifts an `int` by its own width. Two counts of
undefined behaviour on one line, and `total_openings += delta` overflows
beside them.

### Where the number comes from

```
cam_xpt.c:2938   if ((crs->release_flags & RELSIM_ADJUST_OPENINGS) != 0) {
cam_xpt.c:2939           /* Don't ever go below one opening */
cam_xpt.c:2940           if (crs->openings > 0) {
cam_xpt.c:2941                   xpt_dev_ccbq_resize(path, crs->openings);
```

The shape this tree keeps finding, with the author's own comment naming
the half he checked. And `crs->openings` is userland's:

* `passdoioctl()`'s `CAMIOCOMMAND` rejects only `func_code &
  XPT_FC_XPT_ONLY` (`scsi_pass.c:1793`). `XPT_REL_SIMQ` is `0x05`
  (`cam_ccb.h:146`) and carries no such bit, so it passes.
* `passsendccb()` calls `xpt_merge_ccb()`, whose last statement is
  `bcopy(&(&src_ccb->ccb_h)[1], &(&dst_ccb->ccb_h)[1], sizeof(union ccb)
  - sizeof(struct ccb_hdr))` — the whole union body, `openings` and
  `release_flags` included, copied from the caller.
* `cam_periph_runccb()` → `xpt_action()` → the case above.

The other three callers of `xpt_dev_ccbq_resize()` pass
`sim->max_dev_openings`, `min(device->maxtags,
sim->max_tagged_dev_openings)` and `sim->max_dev_openings` — SIM
constants. This one is the only one that does not, and it is the only
one with a bound.

### What it is not

Not memory corruption, and the reason is worth writing down because the
first reading of it said otherwise. `camq_insert()` writes
`queue_array[++entries]` under a `KASSERT` — a no-op without
`INVARIANTS` — and on a benign wrap `new_size` comes out as
`imax(64, 1) == 64`, so the queue is *not* resized while `dev_openings`
has been raised to about two billion. That looks like a write past a
64-entry array.

It is not, because `cam_ccbq_insert_ccb()` (`cam_queue.h:170-180`)
checks `entries == array_size` first, tries `camq_resize()`, and on
failure spills the lowest-priority ccb to `queue_extra_head`. The
`KASSERT` asserts an invariant its caller maintains; it is not the only
check. What is left is the arithmetic, which is wrong on its own terms.

### The fix, and the twin it exposed

A clamp at the top of `cam_ccbq_resize()`, before anything is computed
from `new_size`, against a new `CAM_MAX_DEV_OPENINGS` of 65536 — above
what any real device can queue, since NVMe's maximum is 65535 entries
and SCSI's tag space is smaller.

The first version clamped the upper bound only, and CBMC still reported
the same two failures. It was right: `INT_MIN + INT_MIN / 2` underflows
the identical sum from the other side. Both bounds are there now.

Then the model checker found the twin. `cam_ccbq_init()` twenty lines
down has

```c
	if (camq_init(&ccbq->queue,
	    imax(64, 1 << fls(openings + openings / 2))) != 0)
```

— the same expression, exported beside the function I had just fixed,
with both of its callers passing a SIM constant. Fixing one of a pair is
the defect this document is mostly about, and I had just done it. It is
clamped too, and so is `camq_init()`'s own `size + 1`, which is UB at
`INT_MAX` for the same reason and in the same file.

`sys/cam/cam_queue.c` now proves every arithmetic property CBMC checks
in it. The two records that still say FAILED say it only for
`__CPROVER_memory_leak`, which is what a function that returns its
allocation looks like to a modular checker.

## Fixed — two more divisors nobody bounded, one of them in a validator

### `sys/geom/raid3/g_raid3.c:3162` — the check is `< 1` and the code needs `> 1`

```c
	/* One disk is minimum. */
	if (md->md_all < 1)
		return (NULL);
	...
	sc->sc_ndisks = md->md_all;
```

`md_all` is a `uint16_t` decoded straight off the medium
(`g_raid3.h:304`, `:333`, `:362`) and that is the only bound on it.
`sc_ndisks - 1` is the number of **data** disks, and it is a divisor in
fifteen places. Two of them are inside `g_raid3_check_metadata()` — the
function whose entire job is to reject bad metadata:

```c
	if ((md->md_mediasize % (sc->sc_ndisks - 1)) != 0) {
	...
	if ((sc->sc_mediasize / (sc->sc_ndisks - 1)) > pp->mediasize) {
```

So `md_all == 1` divides by zero in the validator, before any of the
other thirteen get a chance. `g_raid3_taste()` runs on every provider
that appears, so this is a panic from bytes on a disk somebody plugged
in — the same reach as the two GEOM RAID tasters above.

The comment is the interesting part. "One disk is minimum" is true of
the *array* and false of the arithmetic: a raid3 with one disk has zero
data disks. `graid3(8)` will not create fewer than three
(`lib/geom/raid3/geom_raid3.c:153` requires `nargs >= 4`, and `:157`
requires the data-disk count be a power of two), so `md_all < 2` cannot
reject an array anybody has.

### `sys/geom/eli/g_eli.h:677` — a divisor computed from the provider's sector size

```c
	sc->sc_data_per_sector  = sectorsize - sc->sc_alen;
	sc->sc_data_per_sector -= sc->sc_data_per_sector % 16;

	sc->sc_bytes_per_sector =
	    (md->md_sectorsize - 1) / sc->sc_data_per_sector + 1;
```

`sectorsize` is the **underlying provider's**, not the metadata's, and
`sc_alen` is 20, 32, 48 or 64 depending on the authentication algorithm
— which `eli_metadata_crypto_supported()` does validate. Nothing
validates their difference. Enumerated rather than argued:

```
  sectorsize   32  sha1/rmd160  alen=20 -> 12 -> 0   DIVISION BY ZERO
  sectorsize   32  sha256       alen=32 ->  0 -> 0   DIVISION BY ZERO
  sectorsize   64  sha512       alen=64 ->  0 -> 0   DIVISION BY ZERO
```

`md.c:1368` rejects a sector size that is not a power of two and
nothing else, so `mdconfig -a -t malloc -s 10m -S 64` followed by `geli
onetime -a hmac/sha512 /dev/md0` is a kernel division by zero from two
ordinary administrative commands. Root-only, like the four nfsd loader
tunables above, and rejected for the same reason. `gnop` cannot reach
it — `g_nop.c:376` requires the new sector size be a multiple of the
old — which is worth writing down because it is the near miss.

The fix is `eli_metadata_sectorsize_supported()`, shaped and named
after the `eli_metadata_crypto_supported()` it should have been
standing beside, called from `g_eli_create()` rather than from
`g_eli_create()`'s two callers — because the crypto check there is a
`KASSERT`, and a `KASSERT` is not a check without `INVARIANTS`. It runs
before `g_new_geomf()`, since the `failed:` label unwinds a mutex, a
consumer and two UMA zones that do not exist yet.

Refusing costs nothing that would have worked: `sc_data_per_sector ==
0` means the geom has no room for a single byte of payload.

### Both were reported, and both are still reported

`g_eli_integrity.c:232` and `g_raid3_ctl.c:479` are where the analyser
saw the division. The guards are in `g_eli.c`/`g_eli.h` and
`g_raid3.c`, so both findings survive the fix — the same
translation-unit boundary as `nfs_nfsdstate.c:415` and
`g_read_data()`. Chasing the warning rather than the defect would have
meant a check at each of the fifteen `sc_ndisks - 1` sites.

Every other finding under `sys/geom/raid3` and `sys/geom/eli` is
unchanged, finding for finding, against the pre-change sweep: the four
that moved are the same four, shifted by exactly the comment lengths
(19 lines in `g_eli.c`, 24 in `g_raid3.c`).

## Fixed — a `goto` that enters a guarded block past its guard

`sys/netinet/tcp_stacks/bbr.c`, `bbr_get_bw()`:

```c
	if (bbr->rc_bbr_state == BBR_STATE_STARTUP) {
		/* Attempt first to use rttProp */
		rtt = (uint64_t)get_filter_value_small(&bbr->r_ctl.rc_rttprop);
		if (rtt && (rtt < 0xffffffff)) {
measure:
			min_bw = (uint64_t)(bbr_initial_cwnd(bbr, bbr->rc_tp)) *
				((uint64_t)1000000);
			min_bw /= rtt;
			...
	} else if (bbr->rc_tp->t_srtt != 0) {
		/* No rttProp, use srtt? */
		rtt = bbr_get_rtt(bbr, BBR_SRTT);
		goto measure;
```

The label is **inside** the guard. The second path tests `t_srtt` and
then divides by `rtt`, and they are not the same number:
`bbr_get_rtt(BBR_SRTT)` returns `TICKS_2_USEC(t_srtt) >>
TCP_RTT_SHIFT`. The delayed-ack floor inside that function applies only
to `f_rtt`, on the no-rtt-at-all path — not to this one.

`TICKS_2_USEC` is `max(1, ...)`, so the shift by 5 takes it to zero
whenever `t_srtt * 1000000 / hz < 32`. At the 137kHz `HZ_MAXIMUM`
(`sys/time.h:614`) that is any `t_srtt` below 5 — an RTT under about a
microsecond. At the default `hz` of 1000 the same expression floors at
31, so this is not reachable on a stock kernel; it needs a raised
`kern.hz` and a very fast path.

Reachability aside, the structure is the defect: one entry into the
block tests the divisor and the other jumps over that test. The fix is
to test it on the second path too, and fall through to the initial
pacing bandwidth — which is exactly what the `t_srtt == 0` arm below
already does, "we have no usable rtt" being the same situation either
way.

### The other three `bbr.c` divisions, read and left alone

| reported | why it holds |
|---|---|
| `:3537` `(len + maxseg - 1) / maxseg` | `maxseg = t_maxseg - rc_last_options`. `t_maxseg` is floored at 64 in both places `tcp_mss()` sets it (`tcp_input.c:3891`, `:3955`), and `rc_last_options` is a TCP options length, at most 40 by the header's 4-bit data offset. So the divisor is at least 24. Worth noting that the identical expression 2200 lines down **is** guarded — `if (bbr->rc_tp->t_maxseg > bbr->rc_last_options)` at `:5739` — and this one is not. |
| `:5806` `tso_len / maxseg` | that same `:5739` guard, 66 lines up. |
| `:5817` `rounddown(tso_len, min_tso)` | `bbr_minseg()` is `rc_pace_min_segs - rc_last_options`, and `rc_pace_min_segs` has exactly one assignment in the file (`:5758`), from `t_maxseg`. Floored at 64 as above, minus at most 40. |

The floor that makes all three safe is `mss = max(mss, 64)`, not
`net.inet.tcp.minmss` — which is a bare `SYSCTL_INT` with no handler
and no validation (`tcp_subr.c:203`), the same shape as
`net.inet.ip.reass_hashsize` before it was fixed. It happens not to
matter here because `tcp_mss()` floors `t_maxseg` independently of it.

## Fixed — the PaX framework did not link without its features

Not a UB finding. It is here because it is the same defect shape as
everything above it, it was found by making a claim and having a linker
disprove it, and the claim was mine.

`sys/conf/files` gates each PaX feature separately:

```
hardenedbsd/hbsd_pax_common.c        optional pax
hardenedbsd/hbsd_pax_hardening.c     optional pax pax_hardening
hardenedbsd/hbsd_grsec_tpe.c         optional pax pax_hardening
hardenedbsd/hbsd_control_extattr.c   optional pax pax_control_extattr
```

which reads as a promise that `options PAX` alone is a configuration.
Boot run 52 tested it and the kernel did not link:

```
ld.lld: error: undefined symbol: pax_kmod_load_disabled
>>> referenced by link_elf_obj.c:227, link_elf.c:248
ld.lld: error: undefined symbol: pax_control_extattr_kmod
>>> referenced by link_elf_obj.c:240, link_elf.c:261
ld.lld: error: undefined symbol: pax_enforce_tpe
>>> referenced by vm_mmap.c:461 (kern_mmap)
ld.lld: error: undefined symbol: pax_harden_tty
>>> referenced by tty.c:2034, tty.c:614
```

I had written, in the config file that failed, "every call site outside
sys/hardenedbsd is #ifdef'd on its own feature, checked: five of them".
Five were. I read five and generalised to all of them. These four are
not: `vm_mmap.c:453` guards its block with a plain `#ifdef PAX`, and the
other three carry no guard at all.

### The idiom was already there

`sys/sys/pax.h` solves this correctly for exactly five functions, all of
them `*_init_prison`:

```c
#ifdef PAX_HARDENING
int pax_hardening_init_prison(struct prison *pr, struct vfsoptlist *opts);
#else
#define	pax_hardening_init_prison(pr, opts)	({ 0; })
#endif
```

The four that broke the link now have it too, with permit values —
`0`, `false`, `(pax_flag_t)0` — because a feature nobody compiled in has
to mean "allowed". Verified by preprocessing the declarations both ways:

```
=== -DPAX_HARDENING -DPAX_CONTROL_EXTATTR ===
CALL_tty:  pax_harden_tty(td)
CALL_kmod: pax_kmod_load_disabled()
CALL_tpe:  pax_enforce_tpe(td, vn, path)
CALL_ext:  pax_control_extattr_kmod(td, vp)
=== defines: none ===
CALL_tty:  ({ 0; })
CALL_kmod: ({ false; })
CALL_tpe:  ({ 0; })
CALL_ext:  ((pax_flag_t)0)
```

### And the 34 that were left alone, deliberately

Counting the whole class rather than the four the linker named: of the
43 functions in `pax.h` whose defining file `sys/conf/files` gates on a
feature option, **38 carry no `#ifdef`** and 5 do.

The other 34 are not being given stubs, and that is a decision rather
than an oversight. They are called only from sites that are themselves
`#ifdef`'d on the feature, so today a stub would change nothing — and
tomorrow it would convert a loud link error into a **silent no-op inside
a security feature**. That is the failure this entire document exists to
catch, and adding 34 of them to tidy up a warning nobody has hit would
be manufacturing it. A link error naming the symbol is the better
outcome for every one of those 34; the four here are different only
because their callers are compiled unconditionally and permit is the
right answer for them.

## Fixed — the one of four hardware-rate caps that checks nothing

`sys/netinet/tcp_stacks/rack.c`, `rack_get_output_bw()`. `fill_bw` is
the divisor eighty lines below:

```c
	lentim = (uint64_t)(len) * (uint64_t)HPTS_USEC_IN_SEC;
	lentim /= fill_bw;
```

and it is guarded on the way in — `if ((fill_bw < RACK_MIN_BW) || ...)
return (slot);` at `:17177`, with `RACK_MIN_BW` 8000. Between that guard
and the division it is reassigned three times, and this is the file's
four "cap it at the hardware rate" assignments side by side:

```c
:2195   if ((calcbw > 0) && (*bw > calcbw))                  *bw = calcbw;
:2205   if ((rack->r_ctl.bw_rate_cap > 0) && (*bw > ...))    *bw = ...;
:17212  if (high_rate) { if (fill_bw > high_rate)            fill_bw = high_rate; }
:17185                  if (fill_bw > high_rate)             fill_bw = high_rate;   <-- 
```

Three check the value they are about to assign. The fourth does not, and
`fill_bw > high_rate` is *true* when `high_rate` is zero, because
`fill_bw` is at least 8000 by the guard above. Nothing between there and
the division puts it back: `:17220`'s cap needs `fill_bw >
bw_rate_cap`, which zero fails.

The one at `:17212` is twenty-seven lines below, in the same function,
assigning the same variable from the same kind of source, with the check
present. That is the whole argument for the fix.

### Reachability, stated rather than assumed

`tcp_hw_highest_rate()` returns
`rle->ptbl->rs_rlt[rle->ptbl->rs_highest_valid].rate` — an entry in a
rate table the **NIC driver** supplies (`tcp_ratelimit.c:677` copies
`rate_table_act[i]` straight in), so a zero there is a driver's to
produce and nothing in `tcp_ratelimit.c` rejects one at the highest
valid index.

Without `options RATELIMIT` the branch cannot be entered at all:
`tcp_ratelimit.h:140-157` makes `tcp_set_pacing_rate()` and
`tcp_chg_pacing_rate()` return `NULL`, so `rack->r_ctl.crte` never
becomes non-NULL and `:17181`'s `crte != NULL` is false. Worth writing
down because the *other* definition of `tcp_hw_highest_rate()` in that
same header returns a literal `0` — so in the build where the function
is guaranteed to return zero, the code that would divide by it is
unreachable. Two halves of one `#ifdef`, each safe only because of the
other.

### The rest of rack.c's divisions, read and left alone

| reported | why it holds |
|---|---|
| ~~`:17384` `res = lentim / rate_wanted`~~ **WITHDRAWN** | This row was wrong, and it is left here struck rather than deleted. It quoted the guard as `if (((bw_est == 0) \|\| (rate_wanted == 0) \|\| ...))` and the elided half was `) && (rack->use_fixed_rate == 0)` — a fixed pacing rate turns the zero test off, and a fixed pacing rate is the only thing that can produce a zero. See *Withdrawn — a dismissal that read the guard with an ellipsis in it*, below. Fixed. |
| ~~`:2496` `bw_est = high_rate`~~ **WITHDRAWN** | Same reason: it leans on "its caller rejects zero at `:17352` before dividing", which is the sentence that was not true. The value is now rejected there unconditionally, so the row's conclusion holds again — by the fix, not by the reasoning. |

Verified: exactly one finding left `sys/netinet/tcp_stacks` — the
`:17231 core.DivideZero` this fixes. Every other finding in the file
moved by exactly the 25 lines of comment added, `:17384` to `:17409`
among them.

## Fixed — an allocation failure that jumps over an initialisation

`rack_output()` declares `int32_t len, error = 0;` — `error` gets an
initialiser and `len` does not — and its body starts

```c
19853  again:
19857	tso = 0;
	...
19900	while (rack->rc_free_cnt < rack_free_cache) {
19901		rsm = rack_alloc(rack);
19902		if (rsm == NULL) {
19903			if (hpts_calling)
19905				slot = (1 * HPTS_USEC_IN_MSEC);
19906			so = inp->inp_socket;
19907			sb = &so->so_snd;
19908			goto just_return_nolock;
		}
	}
19914	sack_rxmit = 0;
19915	len = 0;
```

The `goto` is **seven lines above** the first assignment to `len`, and
two sites past the label read it:

```c
20694	    (len == 0) &&
22188	    rack_log_queue_level(tp, rack, len, &tv, cts);
```

`rack_alloc()` returning NULL is memory pressure, so this is a real path
rather than a corner: the TCP output routine, out of send-map entries,
takes a branch that reads an uninitialised stack slot.

`int32_t len = 0` is the whole fix. Zero is what `:19915` sets eleven
lines later, so no path that already reaches that line changes, and
`again:` sits above it so a loop back-edge still resets it. Same shape
and same fix as the four uninitialised returns above.

Everything else the block reads was checked one at a time: `segsiz`
(`:19723`), `tot_len_this_send` (`:19516`) and `orig_len` (`:19551`) are
all assigned before the `goto`. `len` is the only one that is not.

Verified: **two** findings left `sys/netinet/tcp_stacks` for this one
initialiser — `:20694 core.UndefinedBinaryOperatorResult` and `:22188
core.CallAndMessage` ("3rd function call argument is an uninitialized
value"), which are the comparison and the log call reading the same
variable. Every remaining finding moved by exactly the 22 lines of
comment added.

### `if (tso)` at the EMSGSIZE arm is not one of them

`core.uninitialized.Branch` on `:22211` survives the fix and is a false
positive: `tso = 0;` is at `:19857`, directly under the `again:` label
and above every `goto` in the function, so it is set on entry and reset
on each back-edge. The analyser loses that across three thousand lines
and a `switch` on `error`.

## `sys/netinet/tcp_stacks` is fully read

This section used to name seven unread findings in `rack.c`, then
three. It is zero: every finding clang's analyser reports in `bbr.c` and
`rack.c` has been read, and each is either fixed above or in the
not-a-defect table below. Kept as a heading rather than deleted, because
"nothing listed here" and "nobody looked" are the two states this whole
document exists to tell apart.

### Four more, all one shape

| where | what |
|---|---|
| `bbr.c` `rtt_gain`/`delta` | `if (...) { delta = ...; rtt_gain = ...; } else rtt_gain = 0;` — the `else` initialises one of the two, `int delta, rtt_gain;` initialises neither, and four lines later `bbr_log_startup_event(bbr, cts, rtt_gain, delta, ...)` reads both. The most literal instance of the shape in the document: the two variables are assigned on the same line as each other in the `if`, and only one of them in the `else`. |
| `rack.c` `optval`/`loptval` | `rack_set_sockopt()`'s three-way branch sets both on two arms and **neither** on the `TCP_HYBRID_PACING` one. The deferred-option branch below knows it — its condition excludes `TCP_HYBRID_PACING` explicitly before passing `loptval` — and then `rack_process_option(tp, rack, sopt->sopt_name, optval, loptval, &hybrid)` two lines further on passes both, unguarded, on that same arm. `setsockopt(fd, IPPROTO_TCP, TCP_HYBRID_PACING, ...)` is how a user reaches it. Harmless as it stands: that case calls `process_hybrid_pacing(rack, hybrid)` and reads neither. |
| `rack.c:8296` `nrsm` in a `panic()` | `rack_log_output()` declares `nrsm` at `:8131` and does not assign it until *after* this panic, so the message printed stack garbage — in the one place a wrong value costs most. Six of the nine `Insert in tailq_hash` panics in the file pass an assigned `nrsm`; the two in `rack_init_outstanding()` use a shorter form with no `%p` for exactly this case. Now the same. |
| `rack.c:9797` `ret:%` | a conversion with no specifier, in one of the nine copies of that panic. The other eight say `ret:%d`. |

The first three were `core.CallAndMessage` — "Nth function call argument
is an uninitialized value" — which is the analyser's name for this
shape, and it names the argument position rather than the variable, so
they read as unrelated until the declarations are lined up.

### The gate caught a second over-broad marker

Registering the `:8296` fix with its own `panic(...)` line as the
must-appear string failed a check that the string occurs exactly once:
`rack_init_outstanding()` at `:14268` and `:14328` already use that
exact wording. A merge could have taken the fix and left the gate
passing on a vendor line. The marker is a sentence from the comment
instead. The same mistake as `ti_clk_dpll`, caught the same way.

## Asking "are there more?" by enumeration instead of by sampling

Boot run 54 got further than any before it — buildworld passed, so the
`g_eli.h` fix held — and died at the kernel link on a fifth symbol:

```
ld.lld: error: undefined symbol: pax_disallow_map32bit_active
>>> referenced by vm_mmap.c:321 (kern_mmap)
```

`hbsd_pax_aslr.c` is `optional pax pax_aslr`, and the call site is
guarded

```c
#if defined(__LP64__) && defined(PAX_HARDENING)
	if (pax_disallow_map32bit_active(td, flags))
```

on **the wrong option**. A kernel with hardening but not ASLR does not
link, and PBSD had already edited that very line once — the registered
fix changes `MAP_32BIT` to `__LP64__` in it — without noticing that the
other half named a feature the function does not live in.

### The mistake was mine, twice, in the same shape

Run 52 taught that `options PAX` alone does not link. Asked "are there
more?", I answered by reading and generalising, both times:

> "Every call site outside `sys/hardenedbsd` is `#ifdef`'d on its own
> feature, checked: five of them."

> "The other 34 are not getting stubs. They are called only from sites
> that are themselves `#ifdef`'d on the feature."

The first was wrong and cost run 52. The second was wrong *after* the
first had been proved wrong the same way, and cost run 54. Sampling call
sites and generalising is precisely the defect this document catalogues
in other people's code — the guard on five of nine — and I wrote it into
my own reasoning twice while cataloguing it.

### `tools/verify/check_pax_options.py`

So the question is now answered by enumeration. A call to a function
whose defining file needs option X is fine when any of:

1. an enclosing `#ifdef` in the caller mentions X;
2. the **caller's own file** is gated on X by `sys/conf/files`, so it is
   not compiled when X is off;
3. `pax.h` declares it under `#ifdef X` with an `#else` no-op.

It reports 63 feature-gated PaX functions and found exactly three sites
left, including the one run 54 died on:

```
FAIL  sys/kern/link_elf.c:268
      pax_insecure_kmod() needs PAX_HARDENING; guarded by: HARDEN_KLD
FAIL  sys/kern/link_elf_obj.c:247
      pax_insecure_kmod() needs PAX_HARDENING; guarded by: HARDEN_KLD
FAIL  sys/vm/vm_mmap.c:321
      pax_disallow_map32bit_active() needs PAX_ASLR; guarded by:
      defined(__LP64__) && defined(PAX_HARDENING)
```

`HARDEN_KLD` is a different option from `pax_hardening` — the same
class as `vm_mmap.c`'s, a guard naming a neighbour of the right option.
All three are fixed in `pax.h` rather than at the call sites: stubbing
the function fixes every site including future ones, correcting a guard
fixes one. Each verified to fail the gate when reverted alone.

### Why this gate exists and the `bool` one below does not

Both were written the same afternoon, for failures of the same kind —
something the analyser cannot see and only a forty-minute build reveals.
This one is decidable from the tree: the gate is in `sys/conf/files`,
the guard is in the preprocessor, the stub is in one header, and it
reports **zero** false positives. The `bool` rule was not a property of
the header at all and reported 159. The difference is not effort or
intent; it is whether the thing being checked is actually determined by
what is being read.

## A fix of mine that broke buildworld, and the gate I could not write

The `g_eli` divisor fix above put this next to `eli_metadata_softc()`:

```c
static __inline bool
eli_metadata_sectorsize_supported(const struct g_eli_metadata *md, ...)
```

Boot run 53 spent seventeen minutes to say what was wrong with it:

```
g_eli.h:671:17: error: unknown type name 'bool'
g_eli.h:678:11: error: use of undeclared identifier 'true'
g_eli.h:681:11: error: use of undeclared identifier 'false'
```

`lib/geom/eli/Makefile` has `.PATH: ${SRCTOP}/sys/geom/eli` and builds
`g_eli_crypto.c`, `g_eli_hmac.c`, `g_eli_key.c` and `pkcs5v2.c` **into
userland**, where that TU's include chain has no `<stdbool.h>`.
`usr.sbin/fstyp/geli.c` includes the header too. A kernel header that
userland compiles is not a kernel header.

The sibling this function was named after —
`eli_metadata_crypto_supported()` — is inside `#ifdef _KERNEL` and I did
not ask why. It is now too. Verified structurally: all eight
`bool`/`true`/`false` in `g_eli.h` are inside a `_KERNEL` region, and so
are all eleven in `pax.h`; `cam_queue.h` has none.

### The instruments could not have caught it, and one attempt made that worse

clang's analyser only ever compiles the kernel side of these headers,
so it reported nothing before the change and nothing after. The other
instrument is a forty-minute `buildworld`.

So I wrote a gate: *a `sys/` header that a userland Makefile pulls
sources from must not use `bool` outside `#ifdef _KERNEL`.* It found
**159 hits across the vendor tree** — `sys/sys/refcount.h`,
`sys/sys/runq.h`, `sys/crypto/curve25519.h`, `sys/ufs/ffs/ffs_extern.h`
and thirty more files of correct, shipping code.

The rule is simply false. `sys/sys/runq.h:41` says so out loud:

```c
#include <sys/types.h>		/* For bool. */
```

and `curve25519.h` and `ffs_extern.h` use `bool` with no provider of
their own at all, because every one of *their* includers has one. The
narrower hypothesis — "a header using `bool` must include something that
supplies it" — fails on exactly those two.

Whether a given `bool` compiles depends on the include chain of each
translation unit that reaches it, which is not a property of the header.
So the gate was deleted rather than allowlisted down to a passing set: a
checker with 159 false positives is worse than none, because the first
thing anybody does with it is stop reading its output. This class is
caught by `buildworld` and by nothing cheaper, and that is written here
instead of being papered over.
## What the sweep says the day's fixes did

Verify sweep 5 is the first full run over the tree with everything above
in it. Eleven of its twelve jobs are green; the twelfth is `report`,
which computed and printed the whole report and *then* died on the
apostrophe in run 5's own scope note — the injection bug written up
below, in the run that motivated writing it up. So the numbers in this
table are read out of the job log, and there is no `verify-report`
artifact for run 5 to check them against: `upload-artifact` never ran.
Worth saying rather than leaving as "sweep 5 said", because a number
whose artifact does not exist is a number somebody will fail to
reproduce.

Against the sweep taken before any of it (run 50 on the workstation),
clang's analyser:

| checker | before | after |
|---|---|---|
| `core.DivideZero` | 76 | **68** |
| `core.uninitialized.UndefReturn` | 55 | **48** |
| `core.uninitialized.Assign` | 133 | **128** |
| `core.CallAndMessage` | 278 | **275** |
| `core.UndefinedBinaryOperatorResult` | 246 | 244 |
| `core.NullDereference` | 583 | **586** |
| total | 1626 at 1489 sites | 1604 at 1467 sites |

Eight fewer divisions by zero and seven fewer uninitialised returns is
the shape of the day: `md_promise`, `g_virstor` ×2, `g_raid3`, `g_eli`,
`cam_ccbq_resize` and `cam_ccbq_init`, `bbr_get_bw`, `rack`'s hardware
rate cap; and `rack_output`'s `len`, `if_hw_tsomaxsegsize` ×3,
`rack_set_sockopt`'s pair, `bbr`'s `delta`.

**`core.NullDereference` went UP by three** — and then the comparison
itself turned out not to support that sentence. See below.

CBMC's exported-arithmetic list confirms the same fixes from the other
instrument. Gone since run 50: `posix4_mib.c:139`,
`arm64/cpu_errata.c`, `arm64/vmm/vmm.c:230`, `dev/psci/smccc.c:56,58`,
`cam_queue.c:60` and `:274`. `i386/pci/pci_cfgreg.c` now reads
`1u << slot` and the *"arithmetic overflow on signed shl"* complaint is
gone with it; what remains there is "shift distance too large", which is
CBMC not knowing `slot <= PCI_SLOTMAX` — rule three, an exported
function with a caller-constrained parameter.

### The comparison in that table is between two different machines

Run 50 was collected on a workstation and sweep 5 on a GitHub runner,
and **neither recorded which clang produced it**. clang's analyser
changes between releases: checkers are added, and existing ones get more
or less precise. So a three-finding difference across that pair cannot
be attributed to a source change at all, and "two accounted for, one
open" was a conclusion the data could not carry.

What *is* sound is a before-and-after on one machine with one compiler,
which is how every individual fix above was checked — the `+2` in
`rack.c` was established that way at the time, running the same local
clang either side of the change, and re-confirmed here: `sys/netinet*`
is `27 -> 29`, both in `rack.c`. `sys/kern` is unchanged at 123, which
disproved the hypothesis that the `pax.h` stubs had shifted anything
there.

The direction of the totals is still worth having — eight fewer
`DivideZero` is not a clang-version artefact — but the third
`NullDereference` is not a finding, it is a difference between two
measurements that were never comparable.

`analyze.py` now writes a `_meta` record naming the analyser as the
first line of every run, and `report.py` prints it — or says plainly
that a sweep predates the record and is not comparable across machines.
This is the third time today that evidence was dated or attributed by
the wrong instrument: artifact mtimes for collection times, interleaved
log gaps for per-function durations, and now two machines' finding
counts for one machine's diff.

### New in this sweep, now read

`sys/dev/firmware/arm/scmi_shmem.c` was one of them and was a real
defect — its own section above. The rest are not, and the reasoning is
in the table at the end. One item is still open: `core.NullDereference`
went from 583 to 586 and only two of the three are accounted for.

## Fixed — an error check that could not fire, on an arm attach path

The first thing read off the new sweep's list, and it was a real one.
`sys/dev/firmware/arm/scmi_shmem.c`:

```c
	size_t len;

	len = OF_getencprop_alloc_multi(node, "shmem", sizeof(*shmems),
	    (void **)&shmems);
	if (len <= 0) {
		device_printf(dev, "%s: Can't get shmem node.\n", __func__);
		return (NULL);
	}
	if (index >= len) { ... }
	shmem_dev = OF_device_from_xref(shmems[index]);
```

`OF_getencprop_alloc_multi()` returns **`ssize_t`**, and `-1` on
failure. Stored in a `size_t`, `-1` is `SIZE_MAX`, so `len <= 0` can
only ever catch zero: **the failure check is dead**. And
`OF_getprop_alloc_multi()` beneath it opens with `*buf = NULL;` and
leaves it NULL on every failing path.

So a device tree with no `shmem` property, or a malformed one, falls
through both guards — `index >= SIZE_MAX` is false too — and
dereferences a NULL `shmems`. Both callers (`scmi_smc.c:84`,
`scmi_mailbox.c:99`) are in device attach, so that is a panic at boot on
any arm platform whose SCMI firmware node is missing or wrong.

It is the only caller in the tree that got the type wrong:
`cpufreq_dt.c:352` declares `ssize_t n;` and `openfirm.c:500` declares
`ssize_t ret;`. One of four, again.

`index < 0` is now explicit as well. It had been caught *by accident* —
a negative `int` converted to `size_t` is huge, so `index >= len` was
true — and that accident disappears with the correct type. Both callers
pass a constant today, so this is closing a hole the fix would otherwise
have opened rather than one that was there.

### What the instruments say afterwards

clang's analyser reports **zero** findings across all seven translation
units in `sys/dev/firmware`. CBMC still reports `index * 4` and
`shmems + index` at that line, and is right to: `index` is a parameter
of an exported function and `len` comes from a call it cannot see
inside, so it has neither bound. The `g_read_data` boundary again — the
check that matters here is one no modular checker can make.

## `unix.Malloc` is 149 findings and a quarter of it is test files

The category had never been read. Reading it:

* **35 of the 149 are in `lib/libc/tests/`** — `fortify_string_test.c`
  alone has 21. A leak in a program that runs once and exits is not the
  finding a leak in libc is.
* **5 say "Use of memory after it is freed"** rather than "potential
  leak", which is a different class and the only part worth reading
  first. Four are the `UNLINK`-then-`free` list walk already in the
  table below; the fifth is `fmtmsg_test.c:206`, a test.
* The rest are leak reports on error paths, dominated by three
  variables: `st` (19), `dstvar` (15) and `np` (9).

Across the whole sweep, 73 of 1626 findings (4.5%) are in test files —
small overall, and 23% of this one category.

`report.py` now prints that count per checker and in total. **Not
dropped**: a test can have a real bug, and a finding nobody can see is
indistinguishable from one that is not there. The same reason the
`[triaged]` marker marks rather than hides. But "149 potential leaks"
and "114 potential leaks, plus 35 in tests" are different sentences, and
only one of them is true.

## Fixed — master keys carried between providers, and one scrub of fifteen

`g_eli_ctl_configure()` declares `struct g_eli_metadata md;` at function
scope and loops once per provider. Two things follow.

**The uninitialised read, which upstream documents.** ONETIME providers
have no on-disk metadata, so the read is skipped:

```c
	if (!(sc->sc_flags & G_ELI_FLAG_ONETIME)) {
		/*
		 * ONETIME providers don't write metadata to
		 * disk, so don't try reading it.  This means
		 * we're bit-flipping uninitialized memory in md
		 * below, but that's OK; we don't do anything
		 * with it later.
		 */
		...
		error = g_eli_read_metadata(mp, pp, &md);
```

and then ten `md.md_flags |= ...` / `&= ~...` follow. The comment is
right about the *result* — the ONETIME arm `continue`s before `md` is
encoded — and it is still ten reads of an indeterminate object.

**The one that matters.** `explicit_bzero(&md, sizeof(md))` sits at the
end of the loop body, on the single path that falls all the way through.
**There are fourteen `continue`s before it.** `struct g_eli_metadata`
contains `md_mkeys[G_ELI_MAXMKEYS * G_ELI_MKEYLEN]` — the master keys —
so one provider's decoded metadata can survive into the next iteration
and past the return, in a function that plainly means to scrub it.

That is not a guess about intent. `g_eli_ctl_attach()`, in the same
file, scrubs `md` on **every** early exit — six of them, at `:122`,
`:127`, `:134`, `:145`, `:149` and `:163`. `g_eli_ctl_configure()` does
it on one path out of fifteen.

One `explicit_bzero(&md, sizeof(md))` at the top of each iteration fixes
both: the reads become defined, and nothing carries between providers.
`explicit_bzero` rather than `bzero` for the same reason the existing
call uses it.

### A third instance of the same instrument behaviour

Four `core.uninitialized.Assign` gone, and **one new
`core.NullDereference` at `:651`** that is not a shift of anything:
`pp->sectorsize`, where `pp` is assigned only under `if (!(... ONETIME))`
and used only after the complementary `if (... ONETIME) continue;`.
Complementary tests of one flag on a function-scoped variable — the same
class as `udp`/`t_port` and `ip6`/`r_is_v6` in `rack.c`.

It appeared for the same reason two did there: while `md` was
indeterminate the analyser could not carry a path this far, and defining
it opened the road. **Fixing an uninitialised value raises the finding
count**, reliably, and three times today. A total that goes up after
that kind of fix is the instrument reaching further, not a regression —
which is worth knowing before somebody reads the next sweep's delta as
one.

## Fixed — a driver family that reads registers into whatever the stack held

`sys/dev/usb/wlan` had 22 findings across two files, `if_run.c` (16) and
`if_mtw.c` (6). They are one defect repeated, and the enumeration is the
argument rather than the sampling:

| helper | call sites | inspect the return |
|---|---|---|
| `run_rt3070_rf_read` | 70 | 0 |
| `run_bbp_read` | 28 | 1 |
| `run_efuse_read` | 17 (+ `run_efuse_read_2` → `sc_srom_read`) | 0 |
| `mtw_efuse_read_2` | via `mtw_srom_read`, 19 | 0 |
| `mtw_bbp_read` | 11 | 1 (+1 forwarded) |
| `mtw_rf_read` | 5 | 0 |

Every one of the six has the same shape. `*val` is written by the last
statement of the success path; two to four `return (error)` paths and a
KICK timeout above it leave the caller's object exactly as they found
it. And of 150 call sites, **two** look at the return value. The rest
read the object straight back — `rf & ~0x20` handed to the matching
`rf_write`, `val` handed to `run_bbp_write`, an eFUSE word assembled
into the MAC address and the per-rate Tx power tables. A USB transfer
error or a stuck KICK bit therefore programs a radio register, or
derives the interface's hardware address, from an uninitialised local.

The fix goes in the callee: `*val = 0;` before the first early return.
Zero is not a correct register value — the read failed, so there is no
correct one — but it is deterministic, which the previous behaviour was
not, and one line in each callee covers all 150 unchecked callers.
Changing 150 call sites to check a return they have never checked is a
different and much larger change, and not one to make blind.

### The seventh candidate, which was left alone

`run_rt3070_filter_calib()` matches the same mechanical test — more
`return`s than writes to its out-parameter, and neither of its two
callers checks. It is not the same defect, and patching it the same way
would have been a regression:

```c
	sc->rf24_20mhz = 0x1f;	/* default value */
	target = (sc->mac_ver < 0x3071) ? 0x16 : 0x13;
	run_rt3070_filter_calib(sc, 0x07, target, &sc->rf24_20mhz);
```

The caller seeds the documented default *before* the call, into a softc
field rather than a stack slot. An `*val = 0` at function entry would
destroy 0x1f (and 0x2f for the 40MHz call) on exactly the timeout path
the default exists for. The mechanical test found it; only reading the
callers said what to do about it.

### `if_mtw.c` is a copy of `if_run.c`, and the copy moved one line

The two `core.uninitialized.ArraySubscript` findings in `mtw_tx()` are a
different bug, and the parent driver holds the proof. `if_run.c`:

```c
	} else {
		if (tp->ucastrate != IEEE80211_FIXED_RATE_NONE)
			ridx = rn->fix_ridx;
		else
			ridx = rn->amrr_ridx;
		ctl_ridx = rt2860_rates[ridx].ctl_ridx;
	}
```

`if_mtw.c`, same function, same variables:

```c
	} else {
		if (tp->ucastrate != IEEE80211_FIXED_RATE_NONE) {
			ridx = rn->fix_ridx;

		} else {
			ridx = rn->amrr_ridx;
			ctl_ridx = rt2860_rates[ridx].ctl_ridx;
		}
	}
```

The assignment moved *inside* the else arm. A vap with a fixed unicast
rate — `ifconfig wlan0 ucastrate 54` — takes the other arm and reaches

```c
	dur = rt2860_rates[ctl_ridx].sp_ack_dur;
```

with `ctl_ridx` an indeterminate `uint8_t`, indexing a 44-entry table at
0..255 and writing the result into the frame's 802.11 duration field.
An out-of-bounds read of up to 211 entries past the end of a `const`
table, transmitted. Restoring the parent's shape fixes it.

This is the guard-on-one-of-a-pair shape with a provenance: not an
omission somebody made once, but a correct file copied and edited until
one assignment ended up one brace too deep.

### `if_urtw.c` — the complementary-condition shape that is *not* a false positive

The remaining finding in the scope was in a third driver. `data8` is
written only inside `if (sc->sc_flags & URTW_RTL8187B)` and read only
under the same flag, forty lines down, which is the exact pattern this
document's triage table dismisses six times over — value assigned under
`if (X)`, read under `if (X)`, function-scoped variable, analyser cannot
carry the correlation.

Except the `else` arm sets the flag:

```c
	} else {
		urtw_read32_m(sc, URTW_TX_CONF, &data);
		switch (data & URTW_TX_HWMASK) {
		case URTW_TX_R8187vD_B:
			sc->sc_flags |= URTW_RTL8187B;
```

So the two conditions are not complementary and never were. An RTL8187L
that reports `R8187vD_B` takes the arm that does not write `data8`,
leaves it with the flag set, and reads it. The consequence is small —
the hwrev letter in one `device_printf` at attach — but the reasoning
that would have filed it as a false positive is wrong, and it is wrong
in the direction that matters. Initialising to 0 gives "b", which is
what the switch above treats as `REV_B` and also its `default` arm, so
the string agrees with the flag that actually got set.

Scope afterwards: **22 findings → 0**, across nine translation units.

## Fixed — an Open Firmware output cell tested before it is copied back

`sys/powerpc/ofw/ofw_real.c` is the Open Firmware client interface for
real-mode PowerPC. Every call in it has the same six lines: fill the IN
cells, `ofw_real_map()` the argument struct into a physically-addressable
bounce page, `openfirmware()`, `ofw_real_unmap()` to copy the bounce page
back over the struct, then read the OUT cells.

Twenty call sites use that idiom. Nineteen test only IN cells inside the
`if` around `openfirmware()` — `args.service`, `args.propname`,
`args.buf`, `args.device`, each of them the return of `ofw_real_map()`,
so the test is "did the mapping succeed". `ofw_real_open()` tests one
more thing:

```c
	if (args.device == 0 || openfirmware((void *)argsptr) == -1
	    || args.instance == 0) {
```

`args.instance` is an OUT cell. At that point `ofw_real_unmap()` has not
run, the firmware wrote its answer into the bounce page, and the struct
member still holds whatever was on the stack. A successful open is
reported as a failure whenever that happens to be zero.

### The same line is correct in the other backend

This is not a line somebody wrote carelessly. `sys/dev/ofw/ofw_standard.c:570`:

```c
	if (openfirmware(&args) == -1 || args.instance == 0)
```

That backend hands the firmware `&args` itself, so the cell is written in
place and readable the moment the call returns. `ofw_real` is the same
code with a bounce buffer inserted underneath it, and the check did not
move. Identical to upstream FreeBSD, so this is not a PBSD divergence —
it is a bug PBSD inherits on an architecture it wants first-class.

The fix moves the test after `ofw_real_unmap()`, where the cell means
something.

### And a bound nothing was checking

`ofw_real_interpret()` has `cell_t slot[16]`, holding the command cell,
the status cell, and then `nreturns` results — so it fits only while
`nreturns + 2 <= 16`, that is, fourteen. `args.nreturns` is what tells
the *firmware* how many cells to write, and nothing bounded it. The
public entry point, `OF_interpret()` in `sys/dev/ofw/openfirm.c`, has its
own `cell_t slots[16]`, which makes fifteen and sixteen look legal from
the caller's side while the callee overflows its stack frame at both.

Every in-tree caller passes 0 — `sys/arm/arm/machdep.c:577`,
`OF_interpret("perform-fixup", 0)`, the only one — so this is a
precondition being stated rather than a live overflow. Two arrays sized
16 for capacities of 16 and 14 is the kind of disagreement that stays
harmless exactly until somebody adds the second caller.

## Fixed — the eleventh use, in a packet filter's route-to path

`pf_route()` takes `struct pf_kstate *s` and that pointer is optional.
Its own body says so eleven times:

```
9075   if (s) {
9100       if (s != NULL) {
9116       if (s)
9125       if (s)
9198   if (s != NULL) {
9234   if (s != NULL && s->kif == V_pfi_all && ...)
9236       MPASS(r->rt == PF_REPLYTO || (pd->af != pd->naf && s->direction == PF_IN));
9243   if (r->rt == PF_DUPTO || (pd->af != pd->naf && s->direction == PF_IN))
9249           MPASS(s != NULL);
9347       if (s && s->nat_rule != NULL) {
9400   if (s)
```

Ten guarded, one not. `:9243` sits between a test of `s != NULL` on the
line above and an `MPASS(s != NULL)` six lines below, inside the branch
it opens. Short-circuiting saves it whenever `r->rt == PF_DUPTO`; the
rest of the time it dereferences the pointer the previous statement
finished checking.

`pf_route6()` is the same function for IPv6 and has the same census —
eleven uses, the same one unguarded, at `:9574`. Identical in upstream,
so inherited.

This is the shape this document keeps naming, at its clearest: not a
guard nobody thought of, but a guard present on ten of eleven siblings.
Both twins now read `(s != NULL && pd->af != pd->naf && ...)`. The two
`core.NullDereference` findings for `direction` go with them; pf.c drops
from 10 to 8.

### And an ioctl out-parameter that is one line from being a leak

```c
	unsigned int old_limit;

	error = pf_ioctl_set_limit(pl->index, pl->limit, &old_limit);
	pl->limit = old_limit;
```

`pf_ioctl_set_limit()` returns `EINVAL` without touching `*old_limit`
when the index is out of range or the zone is NULL, and `pl` is the
`DIOCSETLIMIT` buffer. The obvious reading is a four-byte kernel stack
disclosure, and it is wrong: `kern_ioctl()` copies the buffer back only
on success —

```c
	if (error == 0 && (com & IOC_OUT))
		error = copyout(data, uap->data, (u_int)size);
```

— so on the `EINVAL` path nothing reaches userland. What is left is an
indeterminate read whose value is discarded. Guarded on `error == 0`
anyway: "out-parameter written only on success, copied unconditionally
into a userland-visible struct" is one moved `copyout` away from being a
disclosure bug, and the index argument comes from userland.

Checking the framework before writing that sentence is the point of the
entry. The first reading of it was the exciting one.

### The rest of `sys/netpfil/pf`, read and left alone

Twenty-five findings in the scope, three fixed, and the other twenty-two
are four things:

| reported | why it is not a defect |
|---|---|
| **ten** findings across `pf.c:229`, `pf_if.c:105`, `pf_ioctl.c:144`, `pf_norm.c:134,139`, `pf_table.c:178`, `pf_ruleset.c:77,78,79,81` | every one of them is an `RB_GENERATE(...)` line. Ten of the scope's twenty-five findings are one red-black-tree macro, the same class as `subr_stats.c`'s two `ARB_GENERATE_STATIC` lines carrying 94 between them. |
| `pf.c` `pf_change_ap`/`pf_change_a6`, **seven** `core.CallAndMessage` | `pf_addrcpy(&ao, a, pd->af)` is a visible `static inline` that copies **four** bytes for `AF_INET` and sixteen for `AF_INET6`, so after an IPv4 copy only `ao.addr16[0..1]` are defined. The author knew: the two address-family-translating arms pass literal `0` for words 2-7 rather than reading them, and only the two same-family arms read all eight — under a `switch (pd->af)` that selects the same value `pf_addrcpy` switched on. The analyser cannot correlate the two switches because the *next* statement, `pf_addrcpy(a, an, pd->af)`, writes sixteen bytes through `a`, and nothing rules out `a` aliasing `&pd->af`. All four arms check out; the guard is the literal `0`. |
| `pf_lb.c:1026` `ctx->nk->addr[idx]` | `switch (nat_action)` has three arms — `PF_NAT`, `PF_BINAT`, `PF_RDR` — out of an enum of fifteen, and `int idx;` is uninitialised. Both callers constrain it: `pf.c:5628` passes a local that is literally `PF_NAT` or `PF_RDR`, and `pf_lb.c:987` passes `r->action` for a rule that came out of the NAT, BINAT or RDR ruleset, having just excluded `PF_NONAT`, `PF_NOBINAT` and `PF_NORDR`. A rule cannot reach those rulesets with any other action: `pf_get_ruleset_number()` maps exactly ten actions to five rulesets and returns `PF_RULESET_MAX` for the rest, which `pf_ioctl.c:2296` rejects with `EINVAL`. The guarantee is real and it lives in a different translation unit, behind a mapping table. |
| `pf_lb.c:473,906`, `pf_lb.c:1238`, `if_pfsync.c:733`, `pflow.c:1666` | out-parameters and family-keyed pointers of the classes already above. |

## Fixed — NFSv4 sends the client stack bytes when a directory getattr fails

`sys/fs/nfsserver/nfs_nfsdserv.c` carried 23 findings, 18 of them one
`core.CallAndMessage` repeated. Seven NFSv4 handlers share this shape —
`nfsrvd_remove`, `nfsrvd_mknod`, `nfsrvd_rename`, `nfsrvd_link`,
`nfsrvd_symlink`, `nfsrvd_mkdir`, `nfsrvd_open`:

```c
	int error = 0, dirfor_ret = 1, diraft_ret = 1;
	struct nfsvattr dirfor, diraft;
	...
	if (dirp)
		diraft_ret = nfsvno_getattr(dirp, &diraft, nd, p, 0, NULL);
	...
	if (nd->nd_flag & ND_NFSV3) {
		nfsrv_wcc(nd, dirfor_ret, &dirfor, diraft_ret, &diraft);
	} else if ((nd->nd_flag & ND_NFSV4) && !nd->nd_repstat) {
		NFSM_BUILD(tl, u_int32_t *, 5 * NFSX_UNSIGNED);
		*tl++ = newnfs_false;
		txdr_hyper(dirfor.na_filerev, tl);
		tl += 2;
		txdr_hyper(diraft.na_filerev, tl);
	}
```

`dirfor` and `diraft` are stack `struct nfsvattr`. They are filled by
`nfsvno_getattr()` — `diraft` only `if (dirp)`, and neither of them if
the underlying `VOP_GETATTR` returns an error. `na_filerev` is
`na_vattr.va_filerev` (`nfsport.h:683`), which `VOP_GETATTR` writes on
success and leaves alone otherwise. `dirfor_ret` and `diraft_ret` record
exactly this and start at 1, meaning "not fetched".

**The NFSv3 path honours those flags. The NFSv4 path ignores both.**

`nfsrv_wcc()` emits a bare `newnfs_false` and no values when
`before_ret` is set, because the v3 wire format makes the attributes
optional. NFSv4's `change_info4` is `bool atomic; changeid4 before;
changeid4 after` and has **no encoding for "unknown"**, so the v4 code
writes the values unconditionally — and an operation that *succeeds*
while a directory getattr fails puts eight bytes of that stack frame on
the wire per value. `nfsrvd_rename` has four of them.

The guard exists, is computed, is passed to one of the two reply paths,
and is dropped by the other — seven times over, in an unauthenticated
network service.

Nothing can invent a `change_info4` the server does not have; that
format has no way to say so. What can be stopped is the value being
kernel memory. All seven now call `NFSVNO_ATTRINIT()` — this tree's own
initialiser for the type, already used thirteen times in these two files
— before any path can reach the emission.

### And a read-modify-write of a field one arm never wrote

`nfsrvd_opendowngrade()` decodes two client-supplied words in a row:

```c
	i = fxdr_unsigned(int, *tl++);
	switch (i) {
	case NFSV4OPEN_ACCESSREAD:  stp->ls_flags = (...); break;
	case NFSV4OPEN_ACCESSWRITE: stp->ls_flags = (...); break;
	case NFSV4OPEN_ACCESSBOTH:  stp->ls_flags = (...); break;
	default:
		nd->nd_repstat = NFSERR_INVAL;      /* and nothing else */
	}
	i = fxdr_unsigned(int, *tl);
	switch (i) {
	...
	case NFSV4OPEN_DENYREAD:    stp->ls_flags |= NFSLCK_READDENY; break;
```

`stp` points at a stack `struct nfsstate st`. Three arms of the first
switch assign `ls_flags`; the fourth sets an error and leaves it
indeterminate, and the second switch then `|=` it. Both selectors come
straight off the wire, so a client sends an invalid access mode with a
valid deny mode and reaches it.

It does not currently matter — `nfsrv_openupdate()` is called under `if
(!nd->nd_repstat)` and nothing else on that path reads `ls_flags`. It is
one moved guard from mattering, and the three arms above already assign.
Now all four do. `nfs_nfsdserv.c` goes from 23 findings to 7.

### `nfs_nfsdport.c` — one more of a pair, and the sibling is three lines away

`nfsrv_createiovecw()` fills `*ivpp` and `*iovcntp`, and returns
`EBADRPC` from its counting loop before writing either. It has two
callers. `nfsvno_write()`:

```c
	error = nfsrv_createiovecw(retlen, mp, cp, &iv, &cnt);
	if (error != 0)
		return (error);
	uiop->uio_iov = iv;
	uiop->uio_iovcnt = cnt;
```

and `nfsvno_setxattr()`:

```c
	error = nfsrv_createiovecw(len, m, cp, &iv, &cnt);
	uiop->uio_iov = iv;
	uiop->uio_iovcnt = cnt;
```

Two callers, the guard on one. Harmless so far because the uio is used
and `iv` freed under `if (error == 0)` further down, so the indeterminate
values are stored and never read — the same distance from mattering as
the `pf_ioctl` out-parameter above, and the same one-line fix.
`nfs_nfsdport.c` goes from 14 findings to 13.

The remaining thirteen are read. Five of them — `:5875`, `:6037`,
`:6220`, `:6406`, `:6554`, all `tdrpc->done` — are one pNFS idiom
repeated across the data-server fan-out functions:

```c
	drpc = NULL;
	if (mirrorcnt > 1)
		tdrpc = drpc = malloc(sizeof(*drpc) * (mirrorcnt - 1), M_TEMP,
		    M_WAITOK);
	...
	for (i = 0; i < mirrorcnt - 1; i++, tdrpc++)
		tdrpc->done = 0;
```

allocated under `mirrorcnt > 1` and used under `i < mirrorcnt - 1`, which
is zero iterations for every `mirrorcnt` that skips the allocation —
with `M_WAITOK` on top, which cannot return NULL. A maintained pair
across an arithmetic relation rather than a repeated test, which is the
version of that shape the analyser has no chance with.

### The gate for this found a second hole in the gate machinery

`check_pbsd_marks.py` tested `want in text`, and six of these seven
markers are the same sentence. Substring presence is satisfied by any
one of them, so a merge that ate five of the six would have passed
silently — the same guard-on-one-of-a-pair defect found in the `pf.c`
gate an hour earlier, at greater width.

A marker may now be `(text, n)`, meaning it must appear at least `n`
times. The first version of this entry counted `NFSVNO_ATTRINIT(&`,
which this file already uses thirteen times for unrelated reasons — so
the count was met by the pre-existing uses and the gate bit on nothing,
which the verification caught by deleting one site and watching it pass.
It counts the PBSD comment instead, which is unique to the fix, and
deleting one of the six now reports "found 5 time(s), needs 6".

## The sweep could not have found the boot bug, for three independent reasons

`libexec/rtld-elf/rtld.c` held a read of an uninitialised automatic that
clang deletes the enclosing function over. It cost thirty-seven boot
runs. It is a `core.uninitialized`-class defect in a C file in this tree
— which is exactly what this sweep is for — and the sweep never said a
word.

Three things had to be true for that silence, and **any one of them
alone was enough**:

**1. `libexec/` was in no shard.** The scope list is `sys/` directory by
directory, plus `lib/libc`, plus `lib/msun`. Nothing else in the tree is
analysed, and that was never written down as a decision anywhere — it
was just the list. The run-time linker is the first C code every
dynamically linked process on the system executes.

**2. The file would not compile even if it had been.** `rtld.c` does
`#include "notes.h"`, which lives in `lib/csu/common` and reaches the
real build through `libexec/rtld-elf/Makefile:45`. Without it the
translation unit comes back `ERROR`, and an `ERROR` file reports zero
findings — indistinguishable, in the totals, from a clean one. This is
the failure this repository keeps rediscovering, and it was sitting
inside the instrument.

**3. The defect was `#ifdef`'d out of the compilation.**
`share/mk/sys.mk:339` includes `bsd.hardenedbsd.mk`, whose entire
content is `CFLAGS+= -DHARDENEDBSD` and the `CXXFLAGS` line beside it.
`sys.mk` is the root of the build, so every shipped translation unit has
it. The sweep did not, so the `#ifdef HARDENEDBSD` block containing the
defect took its false branch here and its true branch in the binary.

That third one is the `__FreeBSD__`/`__linux__` mistake one macro over,
and `includes.py` already carries a long comment about how 607 files
"saw code that is not the code that ships". This is the same sentence
with a different macro — though the blast radius is far smaller: **three
files** tree-wide key on `HARDENEDBSD`, and one of them is the rtld.

### Verified rather than assumed

The defect was put back, exactly as it was, and the scope re-analysed at
each stage:

| state | `rtld.c` |
|---|---|
| scope added, nothing else | `ERROR`, 0 findings |
| + `-I lib/csu/common`, `-I` its arch dir, `-DIN_RTLD` | `ERROR`, 0 findings — now failing on `pax_flags` |
| + `-DHARDENEDBSD` | **`OK`, 8 findings**, first of them `rtld.c:635 core.UndefinedBinaryOperatorResult — The left operand of '!=' is a garbage value` |
| defect removed again | `OK`, **7** findings — and the one that disappears is that line |

`:635` is `if (aux_info[AT_PAXFLAGS] != NULL)`. The sweep finds it in
under a minute once it can see it.

### What changed

`libexec` is now in both matrices — the analyse shard and the model
shard, because checking one and not the other is the shape this document
is mostly about. `includes.py` learned the rtld's own build flags and
`-DHARDENEDBSD` for every translation unit.

And `check_shards.py` grew a second check. It asserted that every
directory under `sys/` is in a shard; it said nothing about anything
above `sys/`, which is how a whole top-level directory went unnoticed.
Now every top-level directory of the tree must be **either** in a shard
**or** in an `UNANALYSED` table with a reason — so a directory nobody
analyses is a decision on the record rather than an absence. Twenty-two
are listed: third-party trees PBSD does not maintain, build tooling, and
the honest "not yet" for `bin`, `sbin`, `usr.bin`, `usr.sbin`, `stand`
and the rest of `lib`.

Both arms verified: dropping `libexec` from one matrix alone fails that
matrix by name, and an `UNANALYSED` entry naming a directory that no
longer exists fails too, because a stale exemption is its own kind of
lie.

### Two more real ones in the rtld, from the first minute of coverage

With the file compiling, `rtld.c` reported seven findings besides the
boot bug. Two of them are defects of the same family, in the same file,
found immediately:

**`load_kpreload()` — three pointers, two initialised.** The vDSO
loader walks the preloaded object's program headers:

```c
	const Elf_Phdr *phdr, *phlimit, *phdyn, *seg0, *segn;
	...
	seg0 = segn = NULL;

	for (; phdr < phlimit; phdr++) {
		switch (phdr->p_type) {
		case PT_DYNAMIC:  phdyn = phdr;  break;
		case PT_LOAD:     ... seg0, segn ...
		}
	}

	obj->mapsize = segn->p_vaddr + segn->p_memsz;
	...
	obj->dynamic = (const Elf_Dyn *)(obj->relocbase + phdyn->p_vaddr);
```

`seg0` and `segn` are given NULL **because the author knew the loop
might not set them**. `phdyn` is filled by the same loop, under the same
conditions, and was left indeterminate. Then all three are dereferenced
with no test at all — `segn` at `:3296`, `phdyn` at `:3302`, `seg0`
inside the `dbg()` after it. An object with no `PT_LOAD` or no
`PT_DYNAMIC` gives a NULL dereference or a read through garbage, inside
the run-time linker, before the process has run a line of its own code.
`check_elf_headers()` validates the ELF header and says nothing about
which program headers exist. Now all three are NULL-initialised and
checked, taking the `return (-1)` that `check_elf_headers()` and
`digest_dynamic()` already use.

**`parse_rtld_phdr()` — an uninitialised `bool`.**

```c
	bool first_seg;
	...
		case PT_LOAD:
			if (first_seg) {
				obj->vaddrbase = rtld_trunc_page(ph->p_vaddr);
				first_seg = false;
			}
```

Declared, never initialised, **read** to decide whether this is the
first load segment and **assigned only inside the branch that read
selects**. So `obj->vaddrbase` was set or skipped according to a stack
byte, and `mapsize` computed against it on the next line. Reading an
indeterminate `bool` is worse than an `int`: a byte that is neither 0
nor 1 is not a valid representation of the type, and this file has
already demonstrated what a compiler may do with a value that cannot
exist.

`digest_phdr()`, which does the same walk sixty lines away, uses `int
nsegs = 0` and `if (nsegs == 0)`. Initialised there. One of a pair.

**`reloc_relr()` — a write through an indeterminate pointer, driven by
the file being loaded.** RELR alternates: an even entry is an address
and sets `where`, an odd entry is a bitmap of the words after the last
one.

```c
	Elf_Addr *where;
	...
		if ((entry & 1) == 0) {
			where = (Elf_Addr *)(obj->relocbase + entry);
			*where++ += (Elf_Addr)obj->relocbase;
		} else {
			for (long i = 0; (entry >>= 1) != 0; i++)
				if ((entry & 1) != 0)
					where[i] += (Elf_Addr)obj->relocbase;
```

`where` is assigned on the address branch and read on the bitmap branch.
A well-formed section always opens with an address, so it is always set
first — and `obj->relr` is `DT_RELR` out of **the object being loaded**,
so that well-formedness is a property of the file rather than of this
code. A section whose first entry is a bitmap leaves `where`
indeterminate, and the bitmap branch does not read it, it **writes
through it**. Now NULL-initialised, with a bitmap-before-address entry
refused.

"The linker would not emit that" is not a bound the run-time linker gets
to assume about every object it is asked to map.

`rtld.c` is at **2 findings, from 8**. Both remaining are read and left
alone:

| reported | why it is not a defect |
|---|---|
| `rtld.c:4845` `s_entry->dls_name = arg->strspace` | `arg` arrives as `void *param` from `fill_search_info()`'s three callers (`:2404`, `:2409`, `:4894`), and every one of them sets `strspace` to point just past the `dl_serpath` array before the fill pass runs. The `RTLD_DI_SERINFOSIZE` pass, which is the one that runs before `strspace` means anything, takes the other arm of the `if`. A `void *` callback parameter is about as far as an interprocedural analysis can be asked to see. |
| `rtld.c:6713` "potential leak of memory pointed to by `pe`" | real, and inconsequential. `open_binary_fd()` does `pathenv = strdup(pathenv)` and then `strsep(&pathenv, ":")`, which advances the pointer, so the original is lost. It is a few hundred bytes, once, at process start, on the direct-exec path (`rtld -p prog`) — and the function's own error paths call `rtld_die()`. A bounded one-time leak with defined behaviour is a different class from everything else in this file, and not worth a patch to vendor code. |

**Two live defects in the run-time linker, in the first minute after the
scope was added.** That is the argument for the `UNANALYSED` table, made
better than the table itself makes it.

## Fixed — a mesh action frame that reads the stack and puts it back on the air

`sys/net80211` is 802.11, so its inputs arrive from the radio and nobody
authenticated them. Eleven findings in the scope, five of them one chain.

### The parser reports success without having parsed anything

```c
mesh_parse_meshgate_action(..., struct ieee80211_meshgann_ie *ie,
    const uint8_t *frm, const uint8_t *efrm)
{
	while (efrm - frm > 1) {
		IEEE80211_VERIFY_LENGTH(efrm - frm, frm[1] + 2, return -1);
		switch (*frm) {
		case IEEE80211_ELEMID_MESHGANN:
			... fills *ie ...
			break;
		}
		frm += frm[1] + 2;
	}

	return 0;
}
```

The `switch` has one arm. A GANN action frame carrying any other element
— or none — walks the loop, matches nothing, and **returns 0**. The
caller:

```c
	struct ieee80211_meshgann_ie ie;		/* stack, uninitialised */

	if (mesh_parse_meshgate_action(ni, wh, &ie, frm + 2, efrm) != 0) {
		... "GANN parsing failed", is_rx_mgtdiscard++, return (0);
	}

	if (IEEE80211_ADDR_EQ(vap->iv_myaddr, ie.gann_addr))
		return 0;
	...
	IEEE80211_ADDR_COPY(gr->gr_addr, ie.gann_addr);
	...
	gr->gr_lastseq = ie.gann_seq;
```

So an unauthenticated peer sends a GANN frame with the element left out,
and the handler compares a stack-garbage MAC against the local address,
files a stack-garbage sequence number in the known-gates table, and
copies **six bytes of this kernel stack frame** into `gr->gr_addr` — a
table whose contents this node afterwards transmits in the GANN frames
it forwards. Kernel stack, onto the air, on request.

The failure path already existed and was never reachable. It is now:
the parser records whether it found the element and returns `-1` if it
did not, which lands in the caller's existing discard arm.

`mesh_parse_meshpeering_action()`, forty lines up, returns a **pointer**
and hands back NULL when the element it needs is absent, and its caller
checks. Same file, same job, same frame class.

### And two allocations of five that never checked

```c
	gr = IEEE80211_MALLOC(ALIGN(sizeof(struct ieee80211_mesh_gate_route)),
	    M_80211_MESH_GT_RT, IEEE80211_M_NOWAIT | IEEE80211_M_ZERO);
	IEEE80211_ADDR_COPY(gr->gr_addr, addr);
```

`IEEE80211_MALLOC` is `malloc` and `IEEE80211_M_NOWAIT` is `M_NOWAIT`
(`ieee80211_freebsd.h:745,749`), so it returns NULL under memory
pressure. Five sites in `ieee80211_mesh.c`: `:211`, `:665` and `:3458`
check the result; `:881` and `:2609` did not, and both allocate the same
struct on a path a received frame reaches.

The chain runs further. `ieee80211_mesh_mark_gate()` — the `:881` one —
*returns* `gr`, and both of its callers are in `ieee80211_hwmp.c`, in
the PREQ and RANN handlers, and both did

```c
	gr = ieee80211_mesh_mark_gate(vap, preq->preq_origaddr, rtorig);
	gr->gr_lastseq = 0; /* NOT GANN */
```

with no check either. Four sites, all reached from frames off the air,
all now guarded — the allocator's failure returns NULL up the chain and
the callers test it.

`sys/net80211`: **11 findings → 6.**

## The M_NOWAIT lint could not see the bug it exists to find

The two 802.11 mesh NULL dereferences above are exactly what
`tools/verify/nowait_check.py` was written for — an `M_NOWAIT`
allocation dereferenced without a NULL test — and it reported neither.
They were found by reading clang's output by hand.

Two reasons, and they are different from each other:

**The allocator was spelled differently.** `IEEE80211_MALLOC` is
`#define IEEE80211_MALLOC malloc` (`ieee80211_freebsd.h:745`). The call
sites do write the flag, as `IEEE80211_M_NOWAIT`, so the token test
passed — the lint's hard-coded list of allocator *names* was the only
thing keeping them out. `BITSET_ALLOC` is the same shape.

**Or the flag never appears at the call site at all.**

```c
#define SCTP_MALLOC(var, type, size, name) \
	do { var = (type)malloc(size, name, M_NOWAIT); } while (0)
```

`SCTP_MALLOC` is unconditionally `M_NOWAIT`, and the token lives in the
macro body. So `if "M_NOWAIT" not in stmt: continue` skipped **every one
of the tree's thirty-three SCTP_MALLOC calls** — a whole network
protocol invisible to the check, and not because the test was wrong but
because it was reading the wrong text. `R_Zalloc` in the routing code is
the same.

Both classes are covered now: the wrapper names joined the list, and the
baked-in allocators get their own pattern that skips the token test and
takes the variable from the macro's first argument rather than the left
of an `=`.

### What it then found

Eight SCTP sites on the first run, six of which already had their NULL
check — the checker had captured only the leading identifier of
`asoc->strmin`, looked for a test on `asoc`, and found none. Member
paths are the normal case for SCTP_MALLOC, so the capture takes the
whole path now, and six false positives went away.

The two that survived reading are real:

- `sctp_pcb.c:5543` — `sctp_cpuarry` allocated and then indexed,
  `sctp_cpuarry[i] = cpu`, with no test.
- `sctp_pcb.c:5589` — `SCTP_BASE_STATS` allocated and then `memset`.
  Live only under `SCTP_USE_PERCPU_STAT`, which nothing in this tree
  defines — a reason it was never noticed and not a reason to leave it.

Both are subsystem-init paths where `M_WAITOK` would have been the
correct flag in the first place; both are now guarded.

### And it was checked against the bugs it missed

Not "it looks right now" — run against `ieee80211_mesh.c` **as it stood
before today's fix**:

```
extended lint on the PRE-FIX ieee80211_mesh.c: 2 hit(s)
  line 882   gr = IEEE80211_MALLOC(...)  ->  IEEE80211_ADDR_COPY(gr->gr_addr, addr);
  line 2610  gr = IEEE80211_MALLOC(...)  ->  IEEE80211_ADDR_COPY(gr->gr_addr, ie.gann_addr);
```

Both of them, which is the whole point.

One thing that showed up while testing and is worth knowing: run against
the *fixed* file with only the `if (gr == NULL)` blocks deleted, it
reports one site and not two — because the explanatory comment left
behind is twelve lines long and pushes the dereference past the ten-line
lookahead. The lint measures distance in lines, and a comment is lines.

## Fixed — a POSIX timer re-armed from an indeterminate timespec

`sys/kern` was analysed in full: 230 translation units, 167 findings, and
**99 of them are `subr_stats.c`'s two `ARB_GENERATE_STATIC` lines**, so
the real pool is about sixty-eight spread thin. Two of them are defects.

`realtimer_expire_l()` (`kern_time.c:1705`) opens with

```c
	error = kern_clock_gettime(curthread, it->it_clockid, &cts);

	/* Only fire if time is reached. */
	if (error == 0 && timespeccmp(&cts, &it->it_time.it_value, >=)) {
```

and its `else if` arm — reached when the deadline has not arrived **and
when the gettime failed** — ends with

```c
			ts = it->it_time.it_value;
			timespecsub(&ts, &cts, &ts);
			TIMESPEC_TO_TIMEVAL(&tv, &ts);
			callout_reset(&it->it_callout, tvtohz(&tv), ...);
```

The function already treats `error` as something that can be non-zero —
it tests it three lines up — and then reads the same `cts` without it.
On that path the subtraction runs against an indeterminate timespec and
its result becomes `tvtohz()`'s argument: the timer is re-armed at an
arbitrary tick count.

The guard goes on the re-arm only. The `ITF_PSTOPPED` arm beside it does
not touch `cts` and keeps its behaviour exactly. A timer that fails to
re-arm is worse than one that re-arms correctly and better than one armed
from stack contents.

### An error check that could never fire, and the NULL behind it

`md_get_mbuf()` in `subr_mchain.c` was four lines:

```c
	rm = m_copym(m, mdp->md_pos - mtod(m, u_char*), size, M_WAITOK);
	md_get_mem(mdp, NULL, size, MB_MZERO);
	*ret = rm;
	return (0);
```

`m_copym()` returns NULL from its `nospace` path. This discarded that
and returned 0 **always** — so the error check at all four of its call
sites, in `netsmb/smb_rq.c`,

```c
	error = md_get_mbuf(&mbparam, txpcount, &m);
	if (error)
		goto freerq;
	mb_put_mbuf(mbp, m);
```

is dead code, and `mb_put_mbuf()` gets the NULL. That function
half-expects one: `mbp->mb_cur->m_next = m` is fine with NULL and
`while (m)` tests for it explicitly — and then the next statement is
`M_TRAILINGSPACE(m)`, which dereferences. Two NULL-safe statements and a
third that is not, two lines apart.

Fixed at the source: `md_get_mbuf()` returns `ENOBUFS` when `m_copym()`
fails, which makes the branch the callers already have do the job it was
written for. `mb_put_mbuf()` returns early on NULL as well, because it
is exported and its own first two statements make a promise the third
was breaking.

This is the shape worth naming separately from "missing NULL check": the
check was **present at every call site** and could not fire, because the
function it guarded had no way to say no.

### And one byte in the TLS record parser

`tls13_find_record_type()` finds the real record type behind TLS 1.3's
zero padding. `record_type` and `last_offset` are written **together**,
inside `if (m_len > 0)`, and the only thing between an all-zero record
and `*record_typep = record_type` with nothing ever written is

```c
	if (last_offset < tls->params.tls_hlen)
		return (EBADMSG);
```

— a test against a **session parameter**, not against the flag that
records whether the write happened. It holds because `tls_hlen` is 5 for
TLS 1.3, and that is the whole of what holds. A `tls_hlen` of zero turns
a record of nothing but zero bytes, which an attacker sends, into a byte
of that stack frame returned as the record type.

Not reachable today, and initialised anyway: zero is not a valid TLS
`ContentType`, so the poison is a value the caller's switch already
rejects, and the cost is a byte. A stack-disclosure boundary should not
rest on a parameter being non-zero when it can rest on an initialiser.

`uipc_ktls.c:2064` in the same file is **not** a defect: `wlocked` is set
inside `if (tls->tx)` and read inside `if (tls->tx)`, with nothing
between that touches it.

## `options mac_grantbylabel` has never compiled, in either tree

`sys/hardenedbsd` — PBSD's own PaX code, 74 translation units analysed
alongside `sys/security` — reports **zero findings**. Worth saying
plainly, because the rest of this document is the other kind of news.

The one thing that scope did turn up came from the ERROR inventory
rather than from a finding, on its first outing against a scope it had
not been seeded from:

```
FAIL  sys/hardenedbsd/hbsd_pax_SKEL.c does not compile and is not in EXPECTED
FAIL  sys/security/audit/audit_dtrace.c does not compile and is not in EXPECTED
FAIL  sys/security/mac_grantbylabel/mac_grantbylabel.c does not compile and is not in EXPECTED
```

Two are ordinary — a template in no `sys/conf/files` line, and a
DTrace-gated file wanting the opensolaris compat headers. The third is
not.

`sys/conf/files:5299` builds it:

```
security/mac_grantbylabel/mac_grantbylabel.c	optional mac_grantbylabel
```

and it fails with **one** error:

```
mac_grantbylabel.c:496:3: error: field designator 'mpo_proc_check_resource'
    does not refer to any field in type 'struct mac_policy_ops'
```

`struct mac_policy_ops` has `mpo_proc_check_debug`, `_sched`, `_signal`
and `_wait`. There is no `_resource`, and the only two occurrences of
that name in the entire tree are this file's own function definition at
`:344` and the initialiser at `:496`. Upstream HardenedBSD is identical.

**So `options mac_grantbylabel` cannot be enabled, and never could, in
either tree.**

### What the orphaned hook did, and why the fix is not mechanical

```c
static int
mac_grantbylabel_proc_check_resource(struct ucred *cred, struct proc *proc)
{
	if (!SLOT(proc->p_textvp->v_label)) {
		gbl = gbl_get_vlabel(proc->p_textvp, cred);
		if (gbl == 0)
			gbl = GBL_EMPTY;
		SLOT_SET(proc->p_textvp->v_label, gbl);
	}
	return 0;
}
```

It never denies anything. It is the **cache-population** step:
`gbl_get_vlabel()` reads the label and, if unset, fetches it from
`mac_veriexec` — but does not store it. This hook is the only caller of
`gbl_get_vlabel()` and the only writer of the vnode label slot, and
`mac_grantbylabel_priv_grant()` at `:234` reads that slot **raw**:

```c
	label = (gbl_label_t)(SLOT(curproc->p_textvp->v_label) |
	    SLOT(curproc->p_label));
```

Deleting the dead initialiser would make the option compile and produce
a policy whose grants read a cache nothing ever fills — it would fail
closed, granting nothing, silently. That converts a build error into a
security feature that appears to be enabled and does nothing, which is
worse than the error.

Moving the fetch to the point of use is the design-correct answer, and
it puts a `VOP_GETATTR()` inside `priv_grant()`, whose locking context
is not something to change on inference. Adding `mpo_proc_check_resource`
to the MAC KPI is a third option and a much larger one.

That is a decision about MAC entry points and locking, not a defect fix,
so it is written down here rather than guessed at. The inventory entry
says `BROKEN: registers a MAC entry point that does not exist` rather
than pretending it is option-gated, because an inventory that launders a
defect into an exemption is worse than no inventory.

| reported | why it is not a defect |
|---|---|
| `sys/security/mac_lomac/mac_lomac.c:461` | `lomac_copy_single()` does `labelto->ml_flags \|= MAC_LOMAC_FLAG_SINGLE` on a caller-supplied label. Every caller gets it from the MAC framework's label allocator, which zeroes. The `M_ZERO` class, in a `static` function whose callers the analyser does see but whose *label* provenance is three frames up. |

## Fixed — the run-time linker's symbol cache, freed on four architectures of six

`reloc_non_plt()` processes an object's non-PLT relocations. It
allocates a symbol cache once per object and hands it to `find_symdef()`
for every relocation:

```c
	if (obj == obj_rtld)
		cache = NULL;
	else
		cache = calloc(obj->dynsymcount, sizeof(SymCache));
		/* No need to check for NULL here */
```

amd64, i386, arm and powerpc end the function at a `done:` label that
every exit goes through:

```c
	r = 0;
done:
	free(cache);
	return (r);
```

aarch64 and riscv have no such label. Every `return (-1)` inside the
loop — five in aarch64, six in riscv — and the `return (0)` at the end
leave the allocation behind. `SymCache` is `{const Elf_Sym *; const
Obj_Entry *}`, sixteen bytes, times `dynsymcount`: tens of kilobytes per
shared object, leaked once at start-up and again on **every**
`dlopen(3)`, for the life of the process. A daemon that loads and
unloads nsswitch or PAM modules in a loop grows without bound.

The fix is amd64's `done:` label, copied verbatim into both. Not a new
idiom — the idiom this function already has on four of the six
architectures PBSD ships.

The guard on N of M, again. This document's most common finding, and
the first time it has been *across* architectures rather than within one
file.

### Only one of the two was reported, and the reason is the point

clang found riscv's. It did not find aarch64's:

```
  use of undeclared identifier '_IFUNC_ARG_HWCAP'
```

`_IFUNC_ARG_HWCAP` is in `sys/arm64/include/ifunc.h`, reached through
`<machine/ifunc.h>` from `aarch64/rtld_machdep.h`. `arch_of()` resolves
the `machine/` shim for `lib/libc/<arch>/`, `lib/msun/<arch>/` and
`sys/**/<arch>/` — and not for `libexec/rtld-elf/<arch>/`. So all seven
per-architecture relocation processors were analysed against **amd64's**
machine headers.

Five of the seven failed to compile and contributed nothing, which is
the harmless outcome and the one this file has now named often enough.
The interesting two are amd64's, right by coincidence of being amd64,
and riscv's, which **compiled** — a clean check of a program riscv does
not build. That its one finding is architecture-neutral C is luck, not
method.

Four lines in `arch_of()` and `libexec/rtld-elf` goes from 6 ERROR of 28
translation units to 2, both survivors being test fixtures whose
generated inputs are genuinely absent. Every relocation processor PBSD
ships is analysed now, for the first time.

Verified by putting both leaks back: `aarch64/reloc.c:615` and
`riscv/reloc.c:444` are reported, and neither is with the fixes in.

## The first check against a running kernel was red, and the instrument was wrong

Boot run 60 is the first PBSD system interrogated successfully — the
console-echo fix worked, and all five `--run` answers hold output rather
than the command. The step that compares its `sysctl hardening` dump
against the compiled-in defaults failed:

```
FAIL hardening.kmalloc_zero: source says 1, kernel says 0

17 knobs agree, 1 differ, 2 not present in the dump.
```

The kernel is right. `kern_malloc.c:265`:

```c
#ifdef PAX_HARDENING
#ifdef PAX_HARDEN_KMALLOC
static int kmalloc_zero = PAX_FEATURE_SIMPLE_ENABLED;
#else
static int kmalloc_zero = PAX_FEATURE_SIMPLE_DISABLED;
#endif
```

and `sys/conf/std.hardenedbsd:54` is `#options	PAX_HARDEN_KMALLOC`,
commented out. `hardening_sysctls.py` did one `re.search` for an
initialiser and took the first, so for **every** guarded pair in the
tree it reported the enabled arm regardless of configuration.

The first comparison this project has ever been able to make against a
running system, red because of the instrument, on the one knob the
kernel had exactly right. Two of those and the reader stops reading the
step — which is the real cost, and it is larger than the bug.

A default behind an `#ifdef` is not a property of the C file. It is a
property of the C file **and** the kernel config, and the tool had half
of that. `--kernconf` now names the config: its options are read the way
`config(8)` reads them, includes followed, `nooptions` honoured, a
commented-out `#options` line setting nothing. Where a guard cannot be
decided — no config, or a condition outside the
`defined()`/`&&`/`||`/`!` subset — every reachable arm is kept and a
disagreement reports `?`, so the knob is **skipped** rather than
asserted from whichever branch happens to be written first. Two arms
that agree are not a disagreement.

Worth passing: against run 60's dump, **18 knobs checked with
`--kernconf`, 10 without**.

### And two knobs that were never knobs

`hardening.pax.SKEL.status` came from `hardenedbsd/hbsd_pax_SKEL.c`, the
template a new PaX feature is copied from. It is in no
`sys/conf/files*`, so no kernel has ever compiled it.
`hardening.pax.aslr.compat.status` is real but sits inside `#ifdef
COMPAT_FREEBSD32`, which `std.hardenedbsd` removes with `nooptions`.

Both landed in `--check`'s "not present in the dump" pile — which is
also where a knob that really *did* disappear from the kernel would
land. That pile has to stay empty to mean anything. It is 0 now:

```
18 knobs agree, 0 differ, 0 not present in the dump,
1 not decided by the source.
```

Verified in both directions. Uncommenting `PAX_HARDEN_KMALLOC` flips the
expected default to 1 and the check goes red against the same dump;
putting it back makes it green. The resolution is live, not a fixed
second pick. `tools/test_hardening_sysctls.py` holds all of it, and
restoring the old first-initialiser-wins behaviour fails five of its
checks.

### What the running system said

The five answers from run 60, which are the first facts about PBSD read
off a booted machine rather than argued from `sys/conf/options`:

| asked | answered |
|---|---|
| `uname -a` | `FreeBSD freebsd 15.1-STABLE-HBSD FreeBSD 15.1-STABLE-HBSD  HARDENEDBSD amd64` |
| `sysctl hardening` | 30 knobs; every PaX feature at status **2**, enforcing |
| `sysctl -a \| grep -c ^hardening` | 30 — so the dump above is complete, not truncated |
| setuid/setgid binaries | **33**, from `/sbin/mksnap_ffs` to `/usr/sbin/traceroute6` |
| `kldstat -v` | one module, the kernel, with everything linked in |

The double space in the `uname` output is not a capture artefact — the
kernel's own boot banner has it (`boot.log:51`). `newvers.sh` took its
`include_metadata != yes` branch, so `VERINFO` is version, ident and
architecture with nothing between: no build host, user, path or date in
the running kernel's version string.

## Verify sweep 8 — the whole tree, after the coverage work

Run `34075626317`, head `83e625c16`, clang 18.1.3. Every analyse shard,
against sweep 6's numbers where they are comparable:

| shard | OK | ERROR | findings |
|---|---:|---:|---:|
| `libs` (`lib/libc`, `lib/msun`, `libexec`) | 1554 | **76** | 229 |
| `kern` (`sys/kern`, `vm`, `net`, `netinet`, `netinet6`) | 452 | **14** | 255 |
| `fs` (`sys/fs`, `ufs`, `geom`, `cam`, `security`, `cddl`) | 374 | **34** | 238 |
| `rest` (`sys/dev` + ~40 more) | 4783 | **912** | 934 |
| **total** | **7163** | **1036** | **1656** |

**8,199 translation units, 7,163 of them read.** The `rest` shard alone
went from 1,510 ERROR of 5,695 in sweep 6 to 912, and its findings from
474 to 934.

### Where the remaining 1,036 is

It is no longer spread across the tree. `rest`'s 912 breaks down as:

```
  359  sys/contrib/dev          vendored Linux wifi drivers (rtw88, mediatek)
  133  sys/dev
  100  sys/contrib/libsodium    its own test programs, not kernel TUs
   54  sys/contrib/zstd
   51  sys/contrib/ncsw         DPAA, wants sys/contrib/ncsw/inc
   28  sys/kgssapi
   24  sys/powerpc
   ... a tail
```

**Over 600 of the 912 are `sys/contrib/` — vendored third-party code.**
`sys/contrib/dev`'s 359 are one thing again: `page_pool_alloc_frag()`
and forty-odd linuxkpi headers those drivers want and this linuxkpi does
not have. libsodium's 100 are the library's own `test/` programs, which
are userland and belong in the not-a-kernel-TU list rather than in a
count of things the analyser could not read.

That is a good place for the next pass to start and a fair place for
this one to stop: what is left is overwhelmingly code PBSD vendors
rather than code PBSD owns, and the FreeBSD core — `sys/kern` at 14
ERROR of 466, `sys/fs` at 34 of 408 — is read.

## What the coverage work moved, in one table

Six changes to `tools/verify/includes.py`, none of them a new heuristic
and all of them reading something the build system already says. The
numbers are translation units the analyser could not compile, before and
after, measured on the same tree:

| scope | before | after | what was missing |
|---|---:|---:|---|
| `lib/libc` + `lib/msun` + `libexec` | 172 | 76 | the `-D` and `-I` on `CFLAGS` in each directory's own Makefile; its `.PATH`; `arch_of` for `libexec/rtld-elf/<arch>` and every other `<arch>` component |
| `sys/dev` | 552 | 155 | `sys/conf/files*`'s `compile-with`, `sys/modules/*/Makefile`'s `.PATH` + `CFLAGS`, and the `files.*` that says which architecture a driver belongs to |
| `sys/arm64` + `sys/arm` + `sys/powerpc` + `sys/riscv` + `sys/i386` | 124 | 55 | `conf/DEFAULTS`'s options, `contrib/device-tree/include`, and a per-architecture interface shim |

**848 translation units that reported nothing, and looked exactly like
848 clean ones, now report.** They produced 34 defects, of which the ones
above are written up.

Each of the six was found the same way: by reading the largest remaining
ERROR class instead of the findings. That is the whole method, and it is
worth stating plainly because the instinct is the opposite — a list of
findings looks like work and a list of compile errors looks like noise.
The compile errors were where the bugs were.

## Fixed — a compatibility layer that is not compatible on the NULL case

`sys/dev` came back **552 ERROR of 2,633** translation units — a fifth of
the driver tree contributing nothing and looking, in every total this
sweep prints, exactly like a fifth that is clean. The kernel states each
driver's include path in two places nobody was reading:

```
sys/conf/files*         compile-with "${NORMAL_C} -I$S/contrib/ck/include"
sys/modules/*/Makefile  .PATH on the source directory, plus its CFLAGS
```

348 source files are named by the first, 137 directories by the second.
Reading both takes `sys/dev` to **237 ERROR of 2,633** and produces
**127 findings from code that had never been analysed** — Intel QAT
(121 translation units, 119 of them ERROR before), bnxt, mlx4, mlx5,
qlnx, irdma, mthca, iser: the crypto accelerator and every Ethernet and
InfiniBand driver that uses linuxkpi.

**52 of those 127 are one thing.** `sys/dev/bnxt/bnxt_re/bnxt_re.h:705`:

```c
#define	rdev_to_dev(rdev)	((rdev) ? (&(rdev)->ibdev.dev) : NULL)
```

and `sys/compat/linuxkpi/common/include/linux/device.h:200`:

```c
#define	dev_err(dev, fmt, ...)	device_printf((dev)->bsddev, fmt, ##__VA_ARGS__)
```

so `dev_err(rdev_to_dev(rdev), ...)` — 49 sites in `ib_verbs.c`, three
more in `main.c` — is a macro that carefully yields NULL feeding a macro
that unconditionally writes through it.

The driver is not wrong. Linux's `_dev_printk()` opens with

```c
	if (dev)
		dev_printk_emit(...);
	else
		printk(...);
```

so `dev_err(NULL, ...)` is a supported call there, and a driver written
against Linux makes it. The ternary in `rdev_to_dev` exists because its
author believed `rdev` could be NULL; **the compatibility layer is what
turns that belief into a page fault.**

Fixed where it belongs — all eight `dev_*` macros, plus `dev_WARN` and
`dev_WARN_ONCE`, now route through one helper that logs a NULL device
the way Linux does instead of faulting. They stay expressions rather
than becoming `do/while` blocks, because `device_printf()` and
`printf()` both return `int` and some callers sit in a ternary.

`sys/dev/bnxt`: 63 findings → 11. 167 of 175 translation units across
`sys/compat/linuxkpi`, `mlx4`, `mlx5` and `irdma` still compile, so the
expression form did not break a consumer.

## Fixed — three in the driver the compat-layer fix uncovered

With the 52 `->bsddev` dereferences gone, `sys/dev/bnxt` had eleven
findings left. Three are defects.

**Two `int rc;` returned to iflib without being assigned.**
`bnxt_tx_queues_alloc()` assigns `rc` on its failure paths and inside the
`iflib_dma_alloc()` loop, then ends

```c
	softc->ntxqsets = ntxqsets;
	return rc;
```

which `ntxqsets == 0` reaches having run neither. `bnxt_msix_intr_assign()`
is the same shape one `goto` further: `if (BNXT_CHIP_P5_PLUS(softc)) goto
skip_default_cp;` jumps past the only assignment before its loop. iflib
reads the return as attach success or failure, so a zero-queue device
attached or did not according to a stack slot. `int rc = 0;` in both.

**And a media type the card chooses by not being recognised.**
`bnxt_add_media_types()`:

```c
	uint8_t phy_type = get_phy_type(softc), media_type;

	switch (phy_type) {
	...
	case HWRM_PORT_PHY_QCFG_OUTPUT_PHY_TYPE_UNKNOWN:
		/* Only Autoneg is supported for TYPE_UNKNOWN */
		break;

	default:
		/* Only Autoneg is supported for new phy type values */
		device_printf(softc->dev, "phy type %d not supported by driver\n", phy_type);
		break;
	}

	switch (link_info->sig_mode) {
	case BNXT_SIG_MODE_NRZ:
		...add_media(softc, media_type, ...);
```

Two arms of the first switch leave `media_type` unassigned and the second
switch passes it to `add_media()` on every path. `phy_type` is
`get_phy_type(softc)` — the PHY type the **card** reported over HWRM — so
a new or unrecognised card makes the driver advertise whichever of the
ten `BNXT_MEDIA_*` values a stack byte happens to name, one time in ten.
Both arms say in a comment that only autoneg is supported. That is
`BNXT_MEDIA_END`, which `add_media()`'s switch does not have an arm for,
so it adds nothing — which is what the comments describe.

`sys/dev/bnxt`: 63 findings → 4, and the four are read and left alone:
`bnxt_hwrm.c:198` writes `*data` from the caller's `void *msg`;
`qplib_res.c:137` frees `tmp_sg` on its only exit, through linuxkpi's
`vzalloc`/`vfree`, which clang does not model as an allocator pair;
`bnxt_auxbus_compat.c:93` is `list_for_each_entry` on a possibly-empty
list, whose `container_of` on the head is an offset computation and not
a load; and `linux/io.h:456` is the same caller's-buffer shape as the
first.

## Fixed — the CPU supply voltage, programmed from an indeterminate pointer

Reading the ~90 `files.*` under `sys/` for the architecture each driver
belongs to (`sys/arm/mv/files.arm7:24` is what says `dev/cesa/cesa.c` is
ARM) took `sys/dev` from 237 ERROR of 2,633 to 155, and produced sixteen
findings from arm, arm64, powerpc and riscv drivers analysed against
their own `<machine/*.h>` for the first time. Three of them are in
`sys/dev/cpufreq/cpufreq_dt.c`, the device-tree CPU frequency driver
every FDT platform uses, and all three are the same defect twice over.

**`copp` — the operating point to go back to.** `cpufreq_dt_set()`:

```c
	if (CPUFREQ_DT_HAVE_REGULATOR(sc)) {
		error = regulator_get_voltage(sc->reg, &uvolt);
		if (error != 0) {
			copp = cpufreq_dt_find_opp(sc->dev, freq);   /* the only assignment */
			...
			uvolt = copp->uvolt_target;
		}
	} else
		uvolt = 0;
	...
	error = clk_set_freq(sc->clk, opp->freq, CLK_SET_ROUND_DOWN);
	if (error != 0) {
		/* Restore previous voltage (best effort) */
		if (CPUFREQ_DT_HAVE_REGULATOR(sc))
			error = regulator_set_voltage(sc->reg,
			    copp->uvolt_min, copp->uvolt_max);
		return (ENXIO);
	}
```

`copp` is assigned inside `if (regulator_get_voltage(...) != 0)` — the
backup path, taken when the regulator will not report its voltage — and
read under `CPUFREQ_DT_HAVE_REGULATOR(sc)`, which is the **outer** test.
So on the ordinary path, a regulator that answers, `copp` is never
assigned; and both reads of it are in the `clk_set_freq()` failure
handler, which then programs the **CPU supply voltage** from
`copp->uvolt_min`/`uvolt_max` and, twenty lines down, the **CPU clock**
from `copp->freq`. The least-tested path in the driver doing the most
dangerous thing in it.

The guard on one of a pair, with the pair being an outer condition and
an inner one that looks like it. `copp = NULL;` and a test at both uses:
they are best-effort restores whose result is already discarded, so
skipping one is what was intended where the previous point is unknown.

**And `best_n`, one function up.**

```c
	ssize_t n, best_n;
	...
	for (n = 0; n < sc->nopp; n++) {
		if (diff < best_diff) { best_diff = diff; best_n = n; }
	}
	return (&sc->opp[best_n]);
```

`nopp == 0` runs the loop zero times and indexes `sc->opp[]` at an
indeterminate offset, returning that address to `cpufreq_dt_set()`. And
`nopp` counts device-tree nodes: `cpufreq_dt_oppv1_parse()` accepted a
zero-length `operating-points` property (`if (sc->nopp == -1)` was the
only rejection) and `cpufreq_dt_oppv2_parse()` accepted an
`operating-points-v2` node with no children. Both refuse an empty table
now, and `best_n = 0` makes the function defined regardless.

On an embedded board the device tree is firmware-supplied data. Same
class as the run-time linker's `DT_RELR` bitmap and `libc`'s hash
database header: well-formedness that is a property of the input, being
treated as a property of the code.

## Fixed — a performance counter one past the end, on six of eight bounds

`sys/dev/hwpmc/hwpmc_powerpc.c` bounds the PMC index eight times. Six of
them are

```c
	KASSERT(ri >= 0 && ri < ppc_max_pmcs, ...);	/* ×6 */
	for (i = 0; i < ppc_max_pmcs; i++)		/* ×2 */
```

and two are

```c
	if (pmc > ppc_max_pmcs)
		panic("Invalid PMC number: %d\n", pmc);
```

`>` where every other site says `<`. PMC indices run 0..`ppc_max_pmcs`-1
— 4 on E500, 6 on MPC7xxx and POWER8, 8 on PPC970 — so
`pmc == ppc_max_pmcs` passes the guard. On E500 that reaches `case 4:`
and reads `SPR_PMC5`, a counter the CPU does not have. On PPC970 it
matches no case at all: the `switch` covers 0..7, has no `default`, and
falls through to `return (val)` with `val` never assigned — an
indeterminate value handed back as a performance counter reading. The
write side is the same guard and falls through writing nothing, silently.

Both bounds are `>=` now, and both switches have a `default:` that says
the same thing the bound says. The switch arms are unreachable once the
bound is right; the point is that a function which panics on a bad index
must not also have a path that quietly does something else with one.

**And the same shape in `hwpmc_e500.c`.** `powerpc_pmc_config_pmc()`:

```c
	uint8_t pe_cpu_mask;
	...
	vers = mfpvr() >> 16;
	switch (vers) {
	case FSL_E500v1:  pe_cpu_mask = ev->pe_cpu & PMC_PPC_E500V1; break;
	case FSL_E500v2:  ...
	case FSL_E500mc:
	case FSL_E5500:   ...
	}
	if (pe_cpu_mask == 0)
		return (EINVAL);
```

Four cores named, no `default`, and the very next statement decides
whether the event is supported. `vers` is the Processor Version Register
read out of the CPU, so a core this driver did not expect admitted or
refused a performance event according to a stack byte. `= 0` is what the
test below means by "this core does not support this event".

## Fixed — the clock framework, and six drivers below it

`sys/dev/clk` is arm, arm64 and riscv's clock tree, analysed against its
own architecture for the first time. Six defects, one of them in the
framework everything else sits on.

**`sys/dev/clk/clk.c:980` — `int rv, done;`.** `done` is passed as
`&done` to `CLKNODE_SET_FREQ()`, where it is every driver's `*stop`
out-parameter, and it went in **indeterminate**. The default method
(`clknode_method_set_freq`, `:218`) opens with `*stop = 0`, which is what
every driver was relying on without the framework guaranteeing it. A
driver that sets it on only some paths inherits a stack slot:

```c
	/* rk_clk_mux_set_freq() */
	for (p_idx = 0; ...; p_idx++) {
		rv = clknode_set_freq(p_clk, *fout, flags | CLK_SET_DRYRUN, 0);
		if (rv == 0) {
			best_parent = p_idx;
			*stop = 1;			/* the only assignment */
		}
	}
	if (!*stop)
		return (0);
	...
	if (p_idx != best_parent)
		clknode_set_parent_by_idx(clk, best_parent);
```

A garbage non-zero there reparents a clock to an arbitrary index. Fixed
in the framework — one line, rather than in each of the fifty drivers —
and `rk_clk_mux_set_freq()` also says `*stop = 0` beside the loop that
depends on it, matching its own early return.

**`rk_clk_composite.c:243` — a divide the guard is ten lines late for.**
`rk_clk_composite_find_best()` starts `best_div = 0` and returns it
unchanged when no divisor beat the initial best, which is every divisor
when the parent is at 0 Hz — and `clknode_get_freq()`'s return is not
checked. The caller divides by it inside its parent loop; the
`if (best_div == 0) return (ERANGE)` for exactly that case is *after*
the loop. `continue` on a zero divisor.

**Two Allwinner `find_best()` functions of seven that never set `best`.**
`aw_clk_nm.c`, `aw_clk_m.c` and `aw_clk_frac.c` open with `best = 0`;
`aw_clk_nkmp.c` and `aw_clk_mipi.c` assign it on the next line;
`aw_clk_nmm.c` and `aw_clk_np.c` do neither. So the first
`abs(*fout - best)` reads a stack value and, if nothing beats it,
`return (best)` hands it back as the frequency the clock can produce —
with the divisors still 0, which is what then reaches the register.

**And the divisors themselves, in two more.** `aw_clk_nm_set_freq()`
assigns `best_n`/`best_m` only where a parent beat the running best, and
writes them into the clock control register unconditionally:

```c
	n = aw_clk_factor_get_value(&sc->n, best_n);
	m = aw_clk_factor_get_value(&sc->m, best_m);
	val |= n << sc->n.shift;
	val |= m << sc->m.shift;
```

With every parent failing, `best` stays 0 — which survives the two range
checks above whenever `CLK_SET_ROUND_DOWN` is set — and a stack value
goes into a live clock divider. Three of the five locals in that
function were already initialised on the two lines where these two now
are. `aw_clk_frac_set_freq()` is the same pair on its integer-mode path.

### The twenty-one Allwinner divide-by-zeros that are not defects

Every `aw_clk_*_find_best()` divides by factors from
`aw_clk_factor_get_min()`, whose third arm is

```c
	else if (factor->flags & AW_CLK_FACTOR_ZERO_BASED)
		min = 0;
```

so clang has a zero divisor on every one of them. `AW_CLK_FACTOR_ZERO_BASED`
appears **13 times in the tree and every one is an `/* n factor */` of an
`NKMP_CLK`**, where `n` is the PLL multiplier in
`cur = (fparent * n * k) / (m * p)` and never a divisor. The one
`AW_CLK_FACTOR_MIN_VALUE` in the tree (`ccu_a64.c:401`) is 2. Every
divisor factor in every SoC table therefore has a minimum of at least 1,
by `aw_clk_factor_get_min()`'s final `else min = 1`.

The tables are in `ccu_a10.c`, `ccu_a83t.c` and their siblings; the
accessor is in `aw_clk.h`; the divide is in `aw_clk_nkmp.c`. Caller-
constrained across a translation unit, the largest false-positive class
in this document — but measured here rather than assumed, because the
zero arm is real and one `NM_CLK` with a zero-based divisor would make
all twenty-one of these true.

## Fixed — a NULL check that logs and then dereferences anyway

`sys/<arch>/conf/DEFAULTS` is read by `config(8)` before any kernel
config, so its options hold for every kernel that architecture builds —
arm64's carries `INTRNG`, and without it nine translation units hit
`#error Need INTRNG for this file`. Adding those, plus
`-I$S/contrib/device-tree/include` for `<dt-bindings/…>` (which
`files.arm64` adds by `compile-with` for exactly one file while every
per-SoC clock driver wants it), took the architecture directories from
124 ERROR of 767 to 89, and produced eight more findings.

**`tegra210_xusbpadctl.c` — four sites, and its predecessor has the
answer.**

```c
	port = search_lane_port(sc, lane);
	if (port == NULL) {
		device_printf(sc->dev, "Cannot find port for lane: %s\n",
		    lane->name);
	}
	pad = lane->pad;

	if (port->supply_vbus != NULL) {
		rv = regulator_enable(port->supply_vbus);
```

The NULL check is there. It prints, and then falls through and
dereferences anyway — no `return`. This file is the Tegra210 copy of
`sys/arm/nvidia/tegra124/tegra124_xusbpadctl.c`, which writes the same
line as

```c
	if (port != NULL && port->supply_vbus != NULL) {
```

everywhere. The newer copy lost the first half of the test at four
sites, and at two more inside their error paths. Restored to match its
predecessor: six changes, and the older driver is the evidence for what
the newer one meant.

**And `ti_sysc_clock_enable()`, one of two.**

```c
	int err;					/* enable  */
	int err = 0;					/* disable */
```

The two functions are twenty lines apart and otherwise identical: a
`TAILQ_FOREACH_SAFE` over `sc->clk_list` assigning `err`, then
`return (err)`. An empty list returns the enable one's `err` unassigned
to a caller that reads it as success or failure. The disable twin
already had the initialiser.

### And a fifth way a gate marker can fail to bite

The count marker for the Tegra210 fix was registered as six, and the
comment the fix added **quoted the corrected line**. So the file held
seven copies of the string, deleting one of the six still left six, and
the marker passed. Found the same way as the other four: by reverting
one site and watching `check_pbsd_marks.py` not fail.

The rule is now written into the tool beside the others — do not quote
the marker text in the comment the fix adds; describe it. Which is the
same lesson as the previous four: a gate is worth exactly what its own
reversion test says it is worth, and nothing that it is *supposed* to
catch.

## Fixed — a DELAY() for as long as a stack word says

`REGNODE_SET_VOLTAGE(regnode, min, max, &udelay)` has three call sites
in the tree. Every driver implementing it writes `*udelay` **only on its
success path** — the default method (`regulator.c:291`) is `*udelay = 0;`
after its own checks, and Rockchip's returns `ENXIO` for a regulator with
no voltage step and `ERANGE` for a request it cannot meet, both before
reaching the assignment.

Of the three callers:

```c
	/* regulator.c:773 -- regnode_set_voltage() */
	rv = REGNODE_SET_VOLTAGE(regnode, min_uvolt, max_uvolt, &udelay);
	if (rv == 0)
		regnode_delay(udelay);			/* correct */

	/* regulator.c:829 -- the constraint path */
	rv = REGNODE_SET_VOLTAGE(regnode, min_uvolt, max_uvolt, &udelay);
	regnode_delay(udelay);				/* no check */

	/* rk8xx_regulators.c:80 */
	rv = rk8xx_regnode_set_voltage(regnode, param->min_uvolt,
	    param->max_uvolt, &udelay);
	if (udelay != 0)
		DELAY(udelay);				/* checks the wrong thing */
```

One of three, and the two that do not check spin the CPU for as many
microseconds as an uninitialised `int` names — at regulator-init time,
on every Rockchip and every other FDT board. `regnode_enable()`, forty
lines up from the first, gets it right a third way, by returning early
on error before its own `regnode_delay()`.

The out-parameter-written-only-on-success shape, which this document has
now recorded eight times, and the first where the missing check is in
the framework rather than a driver.

## Fixed — a macro whose only job is to hide the warning for the bug

Eighteen declarations across `mthca`, `mlx4` and `mlx5` read

```c
	int uninitialized_var(index);
	u32 uninitialized_var(f0);
	struct ib_uobject *uninitialized_var(xrcd_uobj);
```

and `sys/compat/linuxkpi/common/include/linux/compiler.h:65` was

```c
#define	uninitialized_var(x)		x = x
```

so each of them expands to `int index = index;` — **reading an
indeterminate object to initialise itself**, which is undefined at the
declaration, before any question of whether the variable is later read.

The macro exists because neither gcc nor clang warns on self-init. That
is its entire function: it silences `-Wuninitialized` and leaves the
undefined behaviour exactly where it was. It is why Linux deleted it in
2021 — *"compiler: remove uninitialized_var() macro"*, whose commit
message is that it papers over real bugs.

Every one of the eighteen is a scalar or a pointer, so `x = 0` is
assignable at all of them. Each carries a comment from its author
arguing the variable cannot be read before assignment, and those
arguments may all be right; this costs one store on a path they say is
unreachable. What it buys is that the declaration is defined whether
they are right or not.

The tree's other definition, `sys/dev/drm2/drm_os_freebsd.h:41`, is
`#define uninitialized_var(x) x` — it suppresses nothing and adds
nothing, which is the harmless form.

`mthca` + `mlx5_ib` + `mlx5_core`: 14 findings → 6.

## Fixed — the stack, written into a NIC's reserved register bits

`sys/dev/vnic` is the ThunderX / OcteonTX network driver, arm64's, and
its four queue-configuration functions all end the same way:

```c
	struct cq_cfg cq_cfg;			/* 64 bits of bitfields */
	...
	cq_cfg.ena = 1;
	cq_cfg.reset = 0;
	cq_cfg.caching = 0;
	cq_cfg.qsize = CMP_QSIZE;
	cq_cfg.avg_con = 0;
	nicvf_queue_reg_write(nic, NIC_QSET_CQ_0_7_CFG, qidx,
	    *(uint64_t *)&cq_cfg);
```

`struct cq_cfg` is `reserved_0_15:16`, `avg_con:9`, `reserved_25_31:7`,
`qsize:3`, `reserved_35_39:5`, `caching:1`, `reset:1`, `ena:1`,
`reserved_43_63:21` — **51 of its 64 bits are reserved fields, and not
one of them is ever assigned.** The struct is then written whole to a
hardware register, so the NIC received whatever the stack held in the
bits the datasheet reserves.

Linux's `thunder` driver, which this is a port of, declares these as a
`union` and opens each function with `.value = 0`. The port dropped the
union, and the zeroing with it. All four — `cq_cfg`, `rq_cfg`, `sq_cfg`,
`rbdr_cfg` — have the shape, and none had the initialiser.

## The setuid inventory said three things a real image disproves

`tools/setuid_inventory.py` is a gate: it reads every program Makefile's
`BINMODE` and fails if the setuid set changes. Its own docstring said
what it could not do:

> This reads Makefiles, not a staged tree. It is therefore a statement
> about what the tree declares, not about what a built image contains; a
> `find -perm -4000` over the staging directory is the other half and
> needs a build.

Boot run 60 is that build — the first PBSD image with a login, asked the
question directly. It has **33** setuid or setgid files. The tool
declared **30**, and every one of the differences was the tool's fault.

**`newgrp` was reported setuid root and ships 0555.**

```make
.if defined(ENABLE_SUID_NEWGRP)
BINMODE=	4555
PRECIOUSPROG=
.endif
```

`ENABLE_SUID_NEWGRP` appears nowhere in this tree except that line and a
**commented-out** example in `share/examples/etc/make.conf`. The tool
matched `BINMODE` anywhere in the file. A false entry in a security
inventory — and the same defect as `hardening_sysctls.py`'s, found the
same morning: a value read out of a conditional block without asking
whether the condition holds.

Getting that right turned up the harder half of the same bug: the setuid
`BINMODE` is usually in the **`.else`**.

```make
.if defined(NOSUID)
BINMODE=554
.else
BINMODE=4554
.endif
```

so a first attempt that kept the `.if` text attributed the setuid mode
to the condition that switches it *off*, and declared `mksnap_ffs`, `ppp`
and `login` not shipped setuid — which the image says they are. And
`sendmail` has both arms, `4555` under `.ifdef SENDMAIL_SET_USER_ID` and
`2555` in the `.else`; taking the highest mode and *then* asking about
its guard picked the arm nobody builds and lost the setgid one that is
on the image. The condition has to be evaluated per branch.

**`ksu` was reported with nothing that removes it.** `Makefile.inc1:436`
is `.if ${MK_KERBEROS} != "no"`, and the tool matched only the
`SUBDIR.${MK_X}` spelling — so both copies of a setuid-root program, the
most attractive one on the list, had an empty "removed by" column.
`WITHOUT_KERBEROS` removes both. (The inner `.if ${MK_MITKRB5}` only
chooses *which* Kerberos; `WITHOUT_MITKRB5` builds `kerberos5` with the
same `ksu` in it, so the answer is the outermost option, not the
innermost.)

**And `dmagent` is a directory, not a program.** `PROG= dma`. The
inventory named a path no system has.

**Plus the count itself: 30 declarations, 33 files.** The difference is
entirely hard links, which `LINKS` in each Makefile spells out —
`at`→`atq`/`atrm`/`batch`, `ping`→`ping6`, `shutdown`→`poweroff`,
`authpf`→`authpf-noip`. Six of them, and nothing said so.

The tool now declares **27 programs plus 6 hard links = 33 files**, and
`--check` against run 60's listing agrees in both directions. That
listing is committed as `tools/verify/testdata/setuid_boot_run60.txt`
and both CI and the boot workflow run against it, the latter against the
image it just built. Verified to bite: one fabricated
`/usr/local/bin/evil` in the listing and the check fails by name.

## What the model checker says about `sys/kern`, and why none of it is new

Sweep 8's `sys/kern` CBMC shard: **91 PROVED, 5 BOUNDED, 20 FAILED, 39
TIMEOUT, 4 ERROR**. All twenty failures are in classes this document
already carries, which is the useful negative result:

* **six** are `__CPROVER__start.memory-leak.1` on an allocator —
  `blist_create`, `int_alloc_resource`, `clock_register`, `stack_create`,
  `m_tag_alloc`, `nl_buf_alloc`. A function whose job is to allocate and
  return leaks its allocation from a single-function view by definition.
* **eight** are `dereference failure: pointer NULL in x->y` immediately
  after an `M_WAITOK` allocation, which cannot return NULL.
  `devctl_alloc_dei`, `rctl_rule_alloc`, `ng_alloc_item`,
  `livedump_start`, `nl_find_port` and friends.
* `subr_boot.c:boot_env_to_howto` is `strcasecmp` on an unconstrained
  `char *` — the missing-precondition class.
* `subr_blist.c:bitrange` is `static inline` with callers that pass
  `0 <= n`, `n + count <= BLIST_RADIX`; `subr_stats.c`'s
  `(sbintime_t)_bt.sec << 32` and `subr_autoconf.c`'s `warninterval *`
  are a clock reading and a tunable, both bounded in practice.

Nothing there is a defect, and nothing there is new. Worth recording
because a triage pass that finds nothing is evidence too — the earlier
passes over this shard held.

## Fixed — the DPAA ethernet, sixty-six files nobody had ever analysed

`sys/contrib` is over 600 of the 1,036 ERROR translation units sweep 8 left,
and the largest single reason in it was not a missing header at all. It was
one line of the build system nothing here had read:

```
# sys/powerpc/conf/dpaa/config.dpaa
makeoptions DPAA_COMPILE_CMD="${LINUXKPI_C} ${DPAAWARNFLAGS} \
	-include $S/contrib/ncsw/build/dflags.h \
	-I$S/contrib/ncsw/build/ \
	-I$S/contrib/ncsw/inc \
	...twenty-one -I in all
```

and `files.dpaa` builds every NCSW source with `compile-with
"${DPAA_COMPILE_CMD}"`. Three things had to be read to expand that, and
each was a place the tool had been guessing instead:

1. **`makeoptions` in `sys/*/conf/**`.** The variable table was seeded from
   `sys/conf/files*` only, so a `compile-with` naming a variable defined in
   an architecture's own conf directory expanded to nothing.
2. **`files.*` outside `sys/conf`.** `files.dpaa` lives in
   `sys/powerpc/conf/dpaa/`, and the glob stopped at `sys/conf`.
3. **`sys/conf/kern.pre.mk`.** `DPAA_COMPILE_CMD` opens with `${LINUXKPI_C}`,
   which is `${NORMAL_C} ${LINUXKPI_INCLUDES}` — and `LINUXKPI_INCLUDES` was
   represented in `includes.py` by a **hand-written copy of its three
   flags**, applied to module Makefiles and to nothing else. So five ncsw
   files that this tree ships a `<linux/math64.h>` for still failed on it.
   The copy is gone; `kern_pre_vars()` reads the file's seventy
   unconditional assignments.

There is also a `cpu` line to read, which `DEFAULTS` does not carry:
`sys/powerpc/include/tlb.h:33` is `#if defined(BOOKE_E500)`, and
`sys/dev/dpaa/portals_common.c` uses `_TLB_ENTRY_IO` from inside it. All
four configs that build the DPAA — MPC85XX, MPC85XXSPE, QORIQ64, dpaa/DPAA
— declare `cpu BOOKE_E500`, so `files_cpu_index()` takes the **intersection**
of the cpu sets of every config that names a file. Union would be inventing
a kernel nobody builds; it also gave `CPU_CORTEXA` to 136 arm sources and
`RISCV` to 25.

The result, for `sys/contrib/ncsw` + `sys/dev/dpaa`:

| | before | after the build-system reads | after the fixes |
|---|---|---|---|
| OK | 4 | 55 | 55 |
| ERROR | 62 | 11 | 11 |
| findings | 0 | 24 | 1 |

The eleven that remain are a fourth honest reason for `expected_errors.py`:
**not built** — the mEMAC MAC, the MACSEC block, the frame replicator and
the storage-profile helper are in NXP's drop and in no `files*`, no module
Makefile and no other source in the tree.

### One declaration was worth nine of the twenty-four

`ASSERT_COND` is live in this driver — `contrib/ncsw/build/dflags.h` does not
define `DISABLE_ASSERTIONS` — and it ends in `XX_Exit(1)`, whose FreeBSD
implementation is `panic()`. It is therefore this driver's null check, used
that way in hundreds of places. But `contrib/ncsw/inc/xx_ext.h` declared

```c
void    XX_Exit(int status);
```

with nothing saying it does not return. So every caller past a failed
assertion was a reachable path that dereferences the pointer the assertion
had just rejected — nine `core.NullDereference` findings across `fm_pcd.c`,
`fm_port.c`, `fm_port_im.c`, `fm_cc.c`, `qm_portal_fqr.c`, `fm_ncsw.c` and
`sys/powerpc/include/pio.h`, none of them a bug in the code reported. One
`__dead2` removed all nine, and it removes them from the compiler's view
too: a compiler reading that header was in exactly the same position.

### The IPC handlers reply with the stack when the getter fails

Six sites, one shape, in the two functions that answer inter-partition
messages on behalf of a guest:

```c
        case (FM_DMA_STAT):
        {
            t_FmDmaStatus       dmaStatus;      /* never written on error */
            t_FmIpcDmaStatus    ipcDmaStatus;

            FM_GetDmaStatus(h_Fm, &dmaStatus);
            ipcDmaStatus.boolCmqNotEmpty = (uint8_t)dmaStatus.cmqNotEmpty;
            ...
            memcpy(p_IpcReply->replyBody, (uint8_t*)&ipcDmaStatus, ...);
```

`FM_GetDmaStatus` returns `void` and `REPORT_ERROR(...); return;` on two IPC
failures without touching the caller's struct. The other five are the same
with a return value that is stored into `p_IpcReply->error` and then ignored
for the purpose of deciding whether the body is meaningful:
`FmIsPortStalled` (three error paths), `FM_GetFmanCtrlCodeRevision`,
`FmGetPhysicalMuramBase`, `BmGetRevision`, `QmGetRevision`. Each is a
garbage value the guest acts on **and** a kernel stack leak across the
partition boundary. All six now zero the local at its declaration; the
reply already carries the error, so a zeroed body is the honest thing to
send with it.

`FmIsPortStalled` is the guard-on-N-of-M shape again at its plainest: of its
two callers, `FmResumeStalledPort` at `fm_ncsw.c:2352` checks the error
before reading `isStalled` and the IPC handler at `:941` does not.

### A revision number that picks a register layout

```c
t_Error QmGetSetPortalParams(t_Handle h_Qm, ...)
{
    t_QmRevisionInfo    revInfo;
    ...
        QmGetRevision(p_Qm, &revInfo);

        if ((revInfo.majorRev == 1) && (revInfo.minorRev == 0))
```

Not a report — the comparison chooses which of two layouts this portal's
LIODN registers are programmed in. Read uninitialised it picks one at
random and the portal is configured wrong, silently. The function returns
`t_Error` and its caller checks it, so there was somewhere for the failure
to go; it now goes there.

### `FM_PORT_Free()` faulted on every initialised port

```c
    FmPortDriverParamFree(p_FmPort);     /* XX_Free, then = NULL */

    memset(&fmParams, 0, sizeof(fmParams));
    fmParams.hardwarePortId = p_FmPort->hardwarePortId;
    fmParams.portType = (e_FmPortType)p_FmPort->portType;
    fmParams.deqPipelineDepth =
            p_FmPort->p_FmPortDriverParam->dfltCfg.tx_fifo_deq_pipeline_depth;
```

Five lines after the free-and-null. And it is worse than the local ordering
suggests: `FM_PORT_Init` calls the same `FmPortDriverParamFree` at
`fm_port.c:2709` on success, so for any port that ever finished
initialising the pointer is **already** NULL on entry to `FM_PORT_Free`.
Tearing down a FMan port — interface detach, driver unload — dereferenced
NULL unconditionally.

Hoisting the read above the free would not have been right either. The
value `FmFreePortParams` subtracts from the FM's `accumulatedNumOfDeqTnums`
has to be the value `FmGetSetPortParams` added, and `FM_PORT_Init` overrides
it to 2 for the OH ports under `#ifndef FM_DEQ_PIPELINE_PARAMS_FOR_OP`. So
the port now records what it actually reserved with, after the override,
and the free gives back that.

### A frame descriptor built out of the stack

`sys/dev/dpaa/if_dtsec_rm.c:543` declared `t_DpaaFD fd;` and filled it with
the `DPAA_FD_SET_*` macros. Every one of the three that touches `fd.length`
is a read-modify-write clearing only its own mask —
`SET_LENGTH` 0x000fffff, `SET_FORMAT` 0xe0000000, `SET_OFFSET` 0x1ff00000
(`contrib/ncsw/inc/Peripherals/dpaa_ext.h:135-137`). The first of them reads
the descriptor before anything has written it. Between the three the word
does end up fully defined, which is why this survived; the read is
undefined behaviour all the same, and the descriptor is then enqueued to
the QMan.

## `sys/contrib`, continued — libsodium and zstd

Two more families, one more build-system read, and a distinction worth
keeping straight. `sys/contrib` holds whole upstream repositories, and a
translation unit in one is in exactly one of three states:

* **built** — named by a `files*` line or a module's `SRCS`. It must
  compile; if it does not, the tool is missing a flag.
* **not built** — vendored beside the built ones and named by nothing.
  It belongs in `expected_errors.py`, because an ERROR that is never
  compiled by anybody is a fact about the drop, not a hole.
* **not kernel** — a test program or a command-line tool with a `main()`.
  It belongs in `includes.NOT_KERNEL`, with OpenZFS's `tests/` and
  ACPICA's `compiler/`: "wants `<signal.h>` under `-D_KERNEL`" is not a
  finding about the kernel, and 700 of them once drowned the ones that
  were.

**libsodium** needed no flag at all. `sys/conf/files:5142-5200` names 77
sources — the stream ciphers, the one-time auth, ed25519, AEGIS — and
every one of the 77 compiles clean, with zero findings. Its other 100 are
28 library sources FreeBSD does not take (password hashing, generic hash,
secretbox, libsodium's own randomness) and 72 standalone test programs.
All 100 are now on the record or excluded as userland.

**zstd** did, and it was the same kind of miss as the DPAA's. The files*
reader required the compile line to be quoted:

```
FILES_COMPILE = re.compile(r'... compile-with\s+"(?P<cmd>[^"]*)"')
```

and `sys/conf/files:644-664` writes twenty-one lines of

```
contrib/zstd/lib/common/error_private.c  optional zstdio compile-with ${ZSTD_C}
```

with no quotes at all. `ZSTD_C` (`sys/conf/kern.pre.mk:160`) carries
`-I$S/contrib/zstd/lib/freebsd`, which is where `zstd_deps.h` lives — the
shim that maps zstd's `<stdlib.h>`, `<string.h>` and `assert()` onto the
kernel. Without it the **entire in-kernel zstd** — zstdio(9) and the ZFS
compressor — reported `missing header: stdlib.h` and contributed nothing.

| `sys/contrib/zstd` | before | after |
|---|---|---|
| OK | 7 | 27 |
| ERROR | 51 | 15 |

All 27 built sources compile; the 15 are upstream's legacy v0.1-v0.7
decoders, the dictionary trainer, the deprecated ZBUFF API and the
threaded compressor, none of them in `files*`. Its 16 `programs/` and
`zlibWrapper/` files are the zstd CLI and a zlib shim — userland.

Three misses, three different shapes, all in the same reader: a variable
defined in an architecture's own `conf/`, a `files.*` outside `sys/conf`,
and a `compile-with` without quotes. Each was a place the tool had a rule
where the build system had an answer.

## Fixed — a wireless monitor path that puts the stack in the radiotap header

The 359 vendored Linux wifi drivers are the last of `sys/contrib`'s six
hundred. Their include problem is real but different in kind: their module
Makefiles carry the flags (`.PATH` plus `${LINUXKPI_INCLUDES}`), so the
`-I` are already found — what is missing is the `-D`, and that is a
separate change with a conflict rule to settle first (see below). Even
without it, 163 of the 239 translation units in `iwlwifi`, `rtw88` and
`rtw89` analyse today, and they reported 16.

One is a defect, and it is the shape this document keeps finding.

`sys/contrib/dev/iwlwifi/mvm/rxmq.c` builds a `struct iwl_mvm_rx_phy_data`
in two places:

```c
2128:	struct iwl_mvm_rx_phy_data phy_data = {};   /* iwl_mvm_rx_mpdu_mq  */
2442:	struct iwl_mvm_rx_phy_data phy_data;        /* ..._rx_monitor_no_data */
```

The no-data path — the notification for a frame the radio saw and did not
receive, the one monitor mode exists to report — fills `d0`, `d1`,
`phy_info`, the energies, the channel and `rx_vec[0..3]`, and leaves `d2`,
`d3`, `d4`, `d5` and `eht_d4` as whatever was on the stack. It also sets

```c
	phy_data.phy_info = IWL_RX_MPDU_PHY_TSF_OVERLOAD;
```

**unconditionally**, and `iwl_mvm_rx_fill_status()` reads that as
permission to take the info type out of the firmware's own bytes:

```c
	if (phy_data->phy_info & IWL_RX_MPDU_PHY_TSF_OVERLOAD)
		phy_data->info_type =
			le32_get_bits(phy_data->d1, IWL_RX_PHY_DATA1_INFO_TYPE_MASK);
```

The EHT decoders guard on `phy_data->with_data`, which this path sets
false. `iwl_mvm_decode_he_phy_data()` does not: its `switch
(phy_data->info_type)` reads `d2` for `IWL_RX_PHY_INFO_TYPE_HE_TB_EXT`
(`:1169`) and `d4` for `HE_MU_EXT` (`:1242`), and encodes them into
`he->data4` and `he_mu->flags1` — the radiotap header that goes to
mac80211 and out to every monitor-mode listener.

So: uninitialised kernel stack, with **which** bytes selected by a value
derived from what a nearby radio transmitted, delivered to userland. The
guard on one of two, in a pair of functions forty lines apart in the same
file, one of which has the `= {}` and the other of which does not.

The fix is that `= {}`. Its marker in `check_pbsd_marks.py` is a **count**
of two, not a presence check, for a reason worth writing down: the
declaration it adds is character-for-character the one `iwl_mvm_rx_mpdu_mq`
already had, so a presence marker was satisfied by the twin that never
needed fixing. Found by reverting the fix and watching the check pass —
the fifth marker-failure mode, caught by the procedure that exists for it.

### The other fifteen, and what they have in common

None is a defect, and eleven of them are one idea: **a count the hardware
fixes**. `rtwdev->hal.rf_path_num` is assigned 1 or 2 in exactly one place
(`rtw88/main.c:1989-1996`, on a chip-version register bit), and every array
indexed by it — `rf_efuse_2g[2]`, `rf_efuse_5g[2][5]`, `thermal[2]`,
`dpk_gs[]` — is sized for that. The analyser sees a `u8` field and a loop
bound. `rtw8822c.c:4301`'s `cfo_path_sum / path_num` is the same field
again, as a divisor.

Two more are maintained pairs: `rtw88/mac.c:808` reads `ltecoex_bckp`
under `if (rtwdev->chip->ltecoex_addr && ...)` and writes it at `:789`
under the identical guard, returning `-EBUSY` if the read fails; and
`rtw89/efuse_be.c:530` reads a `buff[4]` that
`rtw89_dump_physical_efuse_map_be()` fills — its one `return 0` without
filling is `if (!map || dump_size == 0)`, and this caller passes a stack
array and 4.

`iwlwifi/mld/rx.c:592` is worth its own note because it looks like the
classic uninitialised-`switch` bug and is not:

```c
	u32 he_type = rate_n_flags & RATE_MCS_HE_TYPE_MSK;
	u32 nsts;

	switch (he_type) {          /* no default: */
```

`RATE_MCS_HE_TYPE_MSK` is `(3 << 23)`, two bits, and the four cases —
`SU` 0, `EXT_SU` 1, `MU` 2, `TRIG` 3 — exhaust it. The switch is total over
the masked value and `nsts` is always set. An exhaustive switch with no
`default` is invisible to a path-sensitive analysis, which is an argument
for writing the `default` rather than a bug.

And one is latent rather than absent. `rtw88/bf.h:117`:

```c
static inline void rtw_chip_cfg_csi_rate(..., u8 *new_rate)
{
	if (rtwdev->chip->ops->cfg_csi_rate)
		rtwdev->chip->ops->cfg_csi_rate(..., new_rate);
}
```

— a no-op when the op is NULL, and `rtw8703b.c:1874` and `rtw8723d.c:1415`
set it to NULL. `main.c:256` then reads `new_csi_rate_idx` unconditionally.
It is unreachable today only because `bfee->role` is set nowhere but
`rtw_bf_enable_bfee_su/mu`, called nowhere but 8821c, 8822b and 8822c —
the three chips that *do* supply the op. That is an invariant spread over
five files and stated in none of them, and the day a fourth chip grows
beamforming it becomes the bug it looks like. Left alone because it is
vendored and not reachable; recorded because "not reachable" here rests on
nothing anyone wrote down.

## The kernel is written in GNU C and was being read as ISO C

`sys/conf/kern.mk:376` and `share/mk/bsd.sys.mk:13` both say

```
CSTD?=		gnu17
```

and `tools/verify/includes.py` said `-std=c17`. Every translation unit
this repository has ever analysed — 8,199 of them, across nine sweeps —
was compiled in strict ISO mode against a tree that is not written in it.

The difference is not academic. In ISO mode `typeof` is not a keyword, so

```c
/* sys/compat/linuxkpi/common/include/linux/slab.h:54 */
#define	kzalloc_obj(_p, ...)  kzalloc(sizeof(typeof(_p)), default_gfp(__VA_ARGS__))
```

is `error: expected expression`, and **every vendored Linux driver that
allocates a struct** fails to compile on that one macro. Statement
expressions `({ ... })`, case ranges and the bare `asm` spelling are the
same story.

It was found from the far end. `sys/contrib/dev/iwlwifi/iwl-phy-db.c`
reported one error, thirty-odd lines of `-Wincompatible-library-redeclaration`
noise around it, and a `note: expanded from macro 'kzalloc'` — which reads
like a linuxkpi gap and is not. Twenty of iwlwifi's translation units were
that macro; another fifteen were the other GNU extensions.

| `iwlwifi` + `rtw88` + `rtw89` | OK | ERROR | findings |
|---|---:|---:|---:|
| before | 163 | 76 | 16 |
| after the module Makefiles' `-D` and decidable `.if` | 166 | 73 | 15 |
| after `-std=gnu17` | **201** | **38** | **25** |

The standard is read from those two makefiles now rather than named here,
along with `CXXSTD` — `c++23` for the kernel (`kern.mk:364`, which is what
this port exists for) and `gnu++17` for userland, where the analyser had
been applying the kernel's C++ standard to libc.

Every shard, re-run against the change. The FreeBSD core does not move at
all — it was already written in the subset both standards accept — and
the whole gain is in the vendored code, which is where the extensions are:

| shard | OK before | OK after | ERROR before | ERROR after | findings |
|---|---:|---:|---:|---:|---:|
| `kern` | 452 | 452 | 14 | 14 | 255 → 255 |
| `fs` | 374 | 374 | 34 | 34 | 238 → 240 |
| `libs` | 1554 | 1558 | 76 | 72 | 229 → 229 |
| `sys/dev` | 2514 | 2518 | 119 | 115 | 550 → 554 |
| the five architecture trees | 712 | 714 | 55 | 53 | 99 → 99 |
| `iwlwifi`+`rtw88`+`rtw89` | 163 | 201 | 76 | 38 | 16 → 25 |

Not one translation unit that compiled before stopped compiling, and not
one finding was lost. `sys/contrib` as a whole now reads 954 OK to 396
ERROR, against the six hundred-odd it could not read at sweep 8; 91 of
the 396 are on the record in `expected_errors.py` and the remaining 305
are the wifi drivers nobody has analysed yet — `mediatek` (135 files),
`athk` (119), `broadcom` (57) and `ath` (31).

### What the `-D` are for, and the one rule they needed

`kernel_flag_index()` collected `-I` and `-include` from a `compile-with`
and from a module's `CFLAGS` and dropped every `-D`. An `-I` makes a header
findable; a `-D` decides what is in it. `sys/modules/iwlwifi/Makefile`
passes nine, and without `-DCONFIG_IWLMLD=1` the driver's own headers
declare a different `struct iwl_mld` than its sources use — "no member
named `netdetect`".

Taking them needed one rule. A file can be named by a `sys/conf/files`
line **and** sit under a module Makefile's `.PATH`, and the two do not
agree, because a real build compiles it into the kernel **or** into the
module and never both: `sys/conf/kern.pre.mk:208` passes `-DWITH_NETDUMP`
for the in-kernel ZFS and `sys/modules/zfs/Makefile:38` passes
`-DWITHOUT_NETDUMP`. Merged, clang takes the last one on the command line,
so which of the two won depended on the order this file happens to
assemble them in. `-I` are additive and an unused one costs nothing, so
those still merge; a `-D` is exclusive per macro name, and the earlier
source wins — the analyser's own `-D_KERNEL` first, then the file's own
`compile-with`, then the module's. (`WITH_`/`WITHOUT_NETDUMP` are
different *names*, so that pair still both arrive; neither is tested
anywhere in the tree, and no rule short of modelling two separate builds
can separate them.)

And `.if` in a module Makefile is not always a question the tool cannot
answer:

```make
IWLWIFI_CONFIG_PM=	1
...
.if defined(IWLWIFI_CONFIG_PM) && ${IWLWIFI_CONFIG_PM} > 0
CFLAGS+=	-DCONFIG_PM
CFLAGS+=	-DCONFIG_PM_SLEEP
.endif
```

is a block the build always takes, from a variable set fifty lines up in
the same file. `_mk_cond()` is three-valued — `defined()`, a comparison
against a variable the Makefile itself assigned, `&&` and `||`, and
**None** for everything else, which skips exactly as before. An `.else` is
never taken: its `.if` was either taken (so the else is dead) or
undecidable (so the else is too).

That last part is also what puts the wifi drivers' remaining 38 on the
record honestly. `RTW88_SDIO= 0`, `RTW88_USB= 0`, `RTW88_LEDS= 0`,
`RTW89_CONFIG_PM= 0`, `IWLWIFI_DEBUGFS= 0` and three commented-out
`SRCS+=` lines are the Makefiles saying which files a FreeBSD kernel does
not take — 29 of them — plus eight Linux kunit tests named in no `SRCS`
at all.

One gap is left and it is deliberately **not** in that list.
`sys/contrib/dev/iwlwifi/fw/acpi.c` **is** built, on any kernel with
`DEV_ACPI`, and its 20 errors are one missing `-DCONFIG_ACPI` — verified
by adding it and watching them all go. The define sits inside
`.if ${KERN_OPTS:MDEV_ACPI}`, and `KERN_OPTS` is the kernel's own option
set, which this module index does not have because it is built once for
the whole tree rather than per architecture. Naming it here is the point:
an ERROR with a known cause and a known fix is a different thing from an
ERROR nobody has read.

### A submodule's variables are in its parent's `Makefile.inc`

`sys/modules/mt76/mt7615/Makefile` opens

```make
MT76_DRIVER_NAME=	mt7615
.include <kmod.opts.mk>
.PATH: ${DEVDIR}
```

and `DEVDIR` is defined in `sys/modules/mt76/Makefile.inc`, one directory
up — bmake pulls it in through `bsd.init.mk`, which that `.include` on
line 2 reaches. Reading each `Makefile` alone, the `.PATH` expanded to
nothing, and all 135 mt76 translation units found none of their own
headers. The same file carries `MT76_PCI`, `MT76_ACPI`, the shared
`CFLAGS` and the `.if` conditions the submodule Makefiles test.

The reader walks up to `sys/modules` collecting them now, and does it in
two passes: variables first (the Makefile's own line-1 assignment wins,
then the `.inc` chain nearest-first), then `.PATH` and `CFLAGS` with the
table complete — because a `.PATH` can name a variable defined in a file
read after it, and an `.if` can test one.

The first pass needed a rule of its own, and it is the one place this
tool assumes rather than reads: **a block it can prove false is skipped;
a block it cannot decide is taken.** That is the assumption the whole
sweep already runs on — a GENERIC-like kernel of this architecture — and
it is what supplies `IWLWIFI_CONFIG_ACPI= 1` from inside
`.if ${KERN_OPTS:MDEV_ACPI}`, which every amd64 and arm64 config
satisfies. With it, `fw/acpi.c`'s twenty errors go, which is the gap
named two sections above; `MT76_ACPI?= 0` and `RTW88_USB= 0` are at
depth 0 and stay off. Written down because the first version of this
took the assignment unconditionally and got the right answer by
accident.

### The readers are checked now, and the first check found a bug in one

Eight readers of the build system have accumulated in `includes.py`, and
none of them was tested. That is the wrong shape for this particular kind
of code: a reader that quietly stops reading gives back exactly the zero
it gave before it was written, which is the failure mode this whole
repository exists to catch.

`tools/verify/test_includes.py` checks each — the standard, the quoted and
unquoted `compile-with`, the `-D` dedupe, the three-valued `.if`, the
`Makefile.inc` chain, `kern.pre.mk`, the `cpu` intersection, the
relative-`-I` filter — against both synthetic input and the real tree,
and each check was confirmed to fail when its reader is broken. Twelve
deliberate breaks, twelve failures.

Three of them were silent on the first pass and are not any more: the
relative-`-I` filter had no check at all, and the `CSTD` reader could not
be told apart from its own fallback, because `STD_DEFAULT` happens to
agree with what the tree says today. Pointing it at a synthetic makefile
that says something else is what separated them — and that check failed
immediately, on a bug it had just found: `_std()` was taking the first
`CSTD?=` line in the file, including one inside

```make
.if ${COMPILER_TYPE} == "gcc"
CSTD?=		c89
.endif
```

which is not this compiler and not this build. `kern_pre_vars()` had
tracked conditional depth from the start; `_std()`, written an hour
later, did not. The guard on one of two, in the pair of functions that
read the same file.

## Fixed — two more in the wifi drivers, both the same shape

With the mt76 include set and the `-D` in place, `sys/contrib/dev` reads
467 of its 769 translation units and reports 44 findings where it
reported none. Two are defects, and both are the guard that exists on one
of two.

### A firmware field seven bits wide, indexing an array of 32

`iwl_mvm_sta_rx_agg()` starts an RX block-ack session, gets a BAID back
from the device and stores it:

```c
	if (baid < 0) {
		ret = baid;
		goto out_free;
	}
	...
	baid_data->rcu_ptr = &mvm->baid_map[baid];
	...
	rcu_assign_pointer(mvm->baid_map[baid], baid_data);
```

`mvm->baid_map` is `struct iwl_mvm_baid_data __rcu *baid_map[IWL_MAX_BAID]`
— **32** pointers. The only check on the index is that it is not
negative.

The BAID comes from `iwl_mvm_fw_baid_op()`, a two-line dispatcher over
two implementations:

```c
/* iwl_mvm_fw_baid_op_cmd(), the newer path */
	if (baid < 0 || baid >= ARRAY_SIZE(mvm->baid_map))
		return -EINVAL;
	return baid;

/* iwl_mvm_fw_baid_op_sta(), the older one */
	return u32_get_bits(status, IWL_ADD_STA_BAID_MASK);
```

`IWL_ADD_STA_BAID_MASK` is `0x7F00` — seven bits, **0..127** — lifted
straight out of the firmware's `ADD_STA` response. So the older path can
return 32..127, the caller's `baid < 0` lets it through, and the driver
writes a pointer up to 96 slots past the end of an array inside
`struct iwl_mvm`. A kernel heap out-of-bounds write, whose value is a
pointer to memory the driver just allocated, at an offset the device
chooses.

The fix is the line its twin already has, in the function that lacked it.
Its marker is a **count of two**, for the same reason as `rxmq.c`'s
above: a presence check would have been satisfied by the path that was
already right — which is to say, by the very asymmetry being fixed.

The reachability caveat is worth stating plainly, because it is the
difference between this and a remote hole. The value crosses the
device-to-host boundary, not the air-to-host one: it is what the Intel
adapter's firmware says, in response to a command the driver sent. This
repository already treats that boundary as untrusted — the same class as
the NIC reserved-register write and the medium-supplied GEOM metadata
fixed above — and the sibling function agrees, since bounding it is
exactly what the newer path does.

### A DTIM period from the air, used as a divisor, with a `WARN` on top

`rtw89_core_update_beacon_track()`:

```c
	beacon_int = bss_conf->beacon_int ?: 100;
	dtim = bss_conf->dtim_period;
	rcu_read_unlock();

#if defined(__FreeBSD__)
	WARN(beacon_int == 0 || dtim == 0, "period %u / beacon_int %u / dtim %u\n",
	    period, beacon_int, dtim);
#endif
	beacons_in_period = period / beacon_int / dtim;
```

`beacon_int` has the `?:`. `dtim` does not, and it is the second divisor
on the next line. `dtim_period` is parsed out of the AP's beacon, and
mac80211 leaves it **0** until one has been parsed — so this divides by
zero on the association path, and on anything an AP chooses to send.

The `#if defined(__FreeBSD__)` block is the part worth reading twice.
Somebody in this tree already hit it, added a FreeBSD-local warning that
names the exact condition, and left the division. The guard exists, and
it does not guard. `dtim = bss_conf->dtim_period ?: 1;` — the same
idiom as the line above it, and 1 is the conservative reading, a DTIM in
every beacon, which keeps `beacons_in_period` an upper bound.

### The other forty-two

The `rf_path_num` class from the last pass accounts for eleven of them
again. The rest are the shapes this document has been cataloguing all
along: `sta.c:1034`'s `find_first_bit(&tid_bitmap, IWL_MAX_TID_COUNT + 1)`
can return 9 and index a nine-element table, but every writer of that
bitmap sets a TID of 8 or less, so `WARN(!tid_bitmap)` does imply a bit
in range; `rtw89/core.c:5286`'s `highest[nss - 1]` needs `nss == 0` from
a hardware path count that is 1 or 2; ACPICA's five are its
`ACPI_DEBUGGER` and namespace-repair paths, where the caller establishes
what the callee cannot see. They are on the record as the previous
tables are, not fixed.

## `sys/contrib/dev`, all 302 of it, on the record

The wifi drivers were the last of `sys/contrib`'s six hundred unreadable
translation units, and the answer turned out not to be include paths at
all. `sys/modules/Makefile` — the file that says which modules a
`make buildkernel` actually builds — descends into `iwlwifi`, `rtw88` and
`rtw89`, and into **none** of `mt76`, `ath10k`, `ath11k`, `ath12k`,
`athk_common` or `brcm80211`. Their module directories exist, their
Makefiles are complete, and nothing enters them.

So of the 302 that will not compile:

```
   262  covered by a NOT_BUILT prefix
    40  named one at a time in EXPECTED
     0  unexpected
     0  stale
```

Three hundred hand-written entries saying the same sentence would go
stale on the next vendor import, so `expected_errors.py` grows a second
table: `NOT_BUILT`, prefix → reason, where the reason is a fact about the
build system you can grep for. `--check-errors` prints what each prefix
absorbed —

```
      135 of 135 under sys/contrib/dev/mediatek/ are NOT_BUILT
       79 of 119 under sys/contrib/dev/athk/ are NOT_BUILT
       29 of  57 under sys/contrib/dev/broadcom/ are NOT_BUILT
```

— and fails on a prefix that absorbed nothing, which is the file-entry
staleness check one level up.

That is a weaker signal than the per-file one, on purpose and with a
limit worth stating: a prefix keeps passing while **one** file under it
still errors. So the reason is checked separately, in
`tools/verify/test_expected_errors.py`, against `sys/modules/Makefile`
itself — the day somebody adds `mt76` to that SUBDIR list, the prefix
stops being true and the test says so rather than the prefix quietly
absorbing 135 ERRORs that have become real.

Writing that test found a bug in the test. `descends()` joined the
Makefile's line continuations before stripping comments, which turns the
whole SUBDIR list into one enormous line and makes every per-line
`^...$` match fail forever — so it answered "no, not built" to
everything, including the three modules that are. Found by adding `mt76`
to the list and watching the check pass; the fix is the ordering, and the
test now proves it can say **yes** (for `iwlwifi`, `rtw88`, `rtw89`)
before it is allowed to say no. A check that cannot fail is the same
zero this document keeps being about.

### And one finding underneath the not-built

`sys/contrib/dev/mediatek/mt76/mt76.h:2035` calls `page_pool_alloc_frag()`,
and `sys/compat/linuxkpi` does not define it — anywhere. The function is
called from a `static inline` in the header, so **every** translation
unit that includes `mt76.h` fails on it: all 66 of them. The tree carries
an mt76 newer than the linuxkpi that has to support it.

It is not the kind of thing to paper over with a stub, even though the
one-line stub is exactly what the rest of that file is:
`sys/compat/linuxkpi/common/include/net/page_pool/helpers.h` is eight
functions, every one of them `pr_debug("%s: TODO\n", __func__)` returning
NULL or 0. FreeBSD's linuxkpi does not implement page pools at all, and
adding a ninth stub would make a driver compile that cannot work. It is
on the record instead, in the prefix's own reason, where the next person
to try building mt76 will read it.

## The architecture can be stated by the option, not the directory

`sys/conf/files` — the architecture-neutral one — carries

```
contrib/alpine-hal/al_hal_iofic.c	optional al_iofic	\
	compile-with "${CC} -c -o ${.TARGET} ${CFLAGS} -I$S/contrib/alpine-hal ..."
```

and `device al_iofic` appears in exactly two places in the tree,
`sys/arm64/conf/std.al` and `sys/arm/conf/ALPINE`. Nothing in the path
says ARM, no `files.arm64` names it, and `arch_of()` therefore analysed
the Annapurna Alpine HAL — eleven files — as amd64, where it reports
twenty errors on `dsb`, `dmb` and the rest of ARM's barrier intrinsics.
As arm64 it compiles clean, zero errors.

So the option is a third thing the build system knows about a file's
architecture, after the directory and the `files.*`. Reading it took two
corrections, and the second is the interesting one.

**`optional miibus | e1000phy` is a disjunction.** The first version
intersected the option list, which for that line gives `armv7` — only
`sys/arm/conf` declares `e1000phy` — and turned a PHY driver every
architecture builds into ARM code. Union across the `|` alternatives,
intersection within each.

**And even corrected, it is a hint and not an answer.** `arch_of()`
deliberately does not use it, because "no amd64 config declares this
device" is not "amd64 cannot build this file": `sys/dev/nvmem/nvmem.c`
is `optional nvmem`, which only the three FDT architectures declare, and
it compiles clean as amd64. Wiring the hint into `arch_of()` would have
re-interpreted 272 translation units that were already being read
correctly — inventing a build rather than reading one, which is the
thing this file keeps warning about.

It is used in exactly one place instead: `analyze.py`, on the error
path. A file that does **not** compile under the default is retried
against the architecture the build system says can build it, and the
better of the two results is reported, tagged with the architecture that
produced it. A file that already compiled is never re-interpreted, and a
file that did not has nothing to lose. The check that it stays out of
`arch_of()` is in `test_includes.py`, alongside the one that a
disjunction is a union.

## The first `--check-errors` over `sys/kern` named six things

The CI `analyze` job does not pass `--check-errors`, so the ERROR
inventory has only ever been checked where it was checked by hand — lib,
`sys/dev`, `sys/contrib`. Running it over the `kern` shard for the first
time reported six, three in each direction, and every one of them was
worth having.

**Three exemptions had gone stale.** `sys/kern/subr_devmap.c`,
`subr_sfbuf.c` and `subr_intr.c` were on the record as "arch-private" and
"needs machine/intr.h, which amd64 has not" — and they compile now,
because a file that fails under the default is retried against the
architecture the build system names. All three were a missing flag
wearing an exemption's clothes. That is precisely what the staleness half
of `--check-errors` is for, and it had never been pointed at them.

**Two were honestly option-gated** — `tcp_stats.c` needs `STATS` and
`memguard.c` needs `DEBUG_MEMGUARD`, neither of which any config in this
tree sets — and are on the record now.

**And one was a character of regex.** `opt_shim()` synthesises the empty
`opt_*.h` that config(8) writes for an unset option, harvesting the names
from the tree so the list cannot go stale. It harvested
`#include "opt_foo.h"`. `sys/netinet/cc/cc.c:52` is the one file in the
tree that writes

```c
#include <opt_cc.h>
```

with angle brackets, and it is the congestion-control framework itself —
the file every `cc_*.c` algorithm registers with. It reported
`'opt_cc.h' file not found` and contributed nothing, in a shard that has
been reported as fully read for three sweeps. One file, one character,
and the same shape as everything else here: the guard existed for one of
two spellings.

## `${MACHINE_CPUARCH}`, and why the module index is per architecture now

`sys/modules/dtrace/dtrace/Makefile`:

```make
ARCHDIR=	${MACHINE_CPUARCH}
.PATH: ${SYSDIR}/cddl/dev/dtrace/${ARCHDIR}
.if ${MACHINE_CPUARCH} == "amd64" || ${MACHINE_CPUARCH} == "i386"
CFLAGS+=	-I${SYSDIR}/cddl/contrib/opensolaris/uts/intel \
		-I${SYSDIR}/cddl/dev/dtrace/x86
.endif
```

`kernel_flag_index()` was one table for the whole tree, so none of
`MACHINE_CPUARCH`, `MACHINE_ARCH` or `MACHINE` had a value: that `.PATH`
resolved to nothing and that `.if` was undecidable. DTrace, FBT, kinst,
SDT and the CTF reader all failed on their own architecture-private
headers — `regset.h`, `fbt_isa.h`, `kinst_isa.h`, `dis_tables.h` — in a
shard that reports its ERROR count every sweep.

The index takes an architecture now, and `include_flags` passes the one
it already resolved. 69 of a 1200-file sample gain flags and none loses
any; the additions are the openzfs SIMD set (`-DHAVE_AVX512F` and eleven
more, from `.if ${MACHINE_ARCH} == "amd64"` in the ZFS module),
`-DVMM_KEEP_STATS` and `-DCOMPAT_LINUX32`. Six of the ten failing files
compile, and `sys/cddl` goes from 29 ERROR to 24 — with **78 findings
where it had none**, because DTrace's own provider code had never been
read.

Seventy-three of those are one file, `sys/cddl/dev/dtrace/x86/dis_tables.c`,
the x86 instruction decoder — which is where `dtrace -n fbt:::` and
kinst find instruction boundaries in the running kernel. That is a large
enough single-file cluster to want its own pass rather than a paragraph
here.

Four files still did not compile and the reason was one this document
has met before, in `include_flags`'s own comment about ZFS: **flag
order**. That is the next section.

## `sys/conf/kmod.mk:128`, and 2,037 translation units compiled twice

```make
# Don't use any standard or source-relative include directories.
NOSTDINC=	-nostdinc
CFLAGS:=	${CFLAGS:N-I*} ${NOSTDINC} ${INCLMAGIC} ${CFLAGS:M-I*}
...
CFLAGS+=	-I. -I${SYSDIR} -I${SYSDIR}/contrib/ck/include
```

That reassignment exists for one purpose: to hold every `-I` a module
Makefile wrote, **in its own order**, and then append `-I${SYSDIR}`
after them. A module's include directories precede `-I$S` in the real
build, by explicit construction. `include_flags()` appended them last —
the reverse — and that is why `sdt.c`, `nfs_clkdtrace.c`,
`opensolaris_uio.c` and `ctf_mod.c` got FreeBSD's `<sys/types.h>` where
the module asks for Solaris's `uint_t`, `hrtime_t` and `uio_t`.

The fix is three lines of reordering and it took two wrong versions to
get right, both caught the same way: **compile every file whose flags
move, before and after.**

**Wrong version one** put all of it in front. Seventy-two files changed;
seventy stayed the same, and two — `sys/fs/nfsclient/nfs_clsubs.c` and
`nfs_clkrpc.c` — went from clean to seventeen errors on `rw_assert` and
`RA_WLOCKED`. `sys/modules/dtrace/dtnfscl` takes `.PATH` on
`sys/fs/nfsclient` and builds **one** file from it, so the whole
directory was inheriting its `${OPENZFS_CFLAGS}` — twelve `-I` that, in
front, put openzfs's SPL `<sys/rwlock.h>` ahead of FreeBSD's. Harmless
for as long as those flags were last; fatal the moment they were first.

**Wrong version two** fixed that by resolving each module's `SRCS`
against its `.PATH` set and dropping the directory entry wherever the
sources resolved. Tidier, and it cost six files their include set
entirely — the TX99 corner of the ath HAL, three brcmfmac ring files,
amd64's `linux32_genassym.c` — all siblings of files a module does
build, none of them named in any `SRCS`.

**What is right** is the distinction the two failures describe between
them. A `compile-with` line that names a file, or a module whose `SRCS`
names it, is *that file's own answer* and goes in front, as kmod.mk
says. A directory is a *guess* about a file nothing named, and a guess
does not get to decide order: it stays at the end, where it has always
been, and it is still there so nothing loses its headers.

The final measurement, over the whole tree rather than a sample: **1,553
kernel translation units change flags; 2 compile that did not, 0 stop
compiling.** The two are `sys/fs/nfsclient/nfs_clkdtrace.c` and
`sys/security/audit/audit_dtrace.c` — the second of which was on the
record as *"needs the opensolaris compat headers, i.e. option
KDTRACE_HOOKS"*, and was a missing flag all along. That is the third
exemption this week to turn out to be one.

`OPENZFS_CFLAGS` itself needed a second read to exist at all: it is
defined at `sys/conf/kmod.mk:576`, not in `kern.pre.mk`, and every
module Makefile reaches it through the closing
`.include <bsd.kmod.mk>`. Reading only `kern.pre.mk` left
`sys/modules/dtrace/sdt/Makefile`'s one `CFLAGS+= ${OPENZFS_CFLAGS}`
expanding to nothing — which then failed the reader's own `if not
flags` test and threw the module's `.PATH` away with it.

### And the four, one at a time

`nfs_clkdtrace.c` compiles. The other three do not, and none of them is
a coverage gap:

* **`sys/cddl/dev/sdt/sdt.c`** is the one file that cannot survive a
  decision made deliberately elsewhere in this tool. `opt_shim()` drops
  `KDTRACE_HOOKS` on purpose — `sys/sys/sdt.h:218` writes every probe as
  `asm goto(...)`, which clang's analyser gives up on, and it cost 85
  errors of 105 translation units in `sys/netinet` alone. The argument
  for dropping it is that a probe is a nop sled the kernel patches at
  run time, so it does not change what the surrounding code computes.
  It does change what `sdt.c` computes, because SDT is its whole
  subject: `struct sdt_tracepoint` is declared only under the option.
* **`opensolaris_uio.c`, `opensolaris_cmn_err.c`, `opensolaris_vm.c`**
  are named by nothing — not `sys/conf/files*`, not any module's `SRCS`,
  not the dtrace or zfs module, both of which take a `.PATH` on that
  directory and build their own files from it. `uio_t` has no definition
  left anywhere under `sys/` except ipfilter's. `opensolaris_atomic.c`,
  the file beside them that **is** built, is in `conf/files.powerpc` and
  `sys/modules/opensolaris/Makefile`.
* **`ctf_mod.c`, `ctf_subr.c`** live under `sys/` and are built by
  `cddl/lib/libctf/Makefile` — a userland library, for `ctfconvert(1)`
  and `ctfmerge(1)`. `ctf_impl.h`, the header they fail on, is under
  `src/cddl/` and not under `src/sys/` at all. They join ACPICA's
  `compiler/` and zlib's `test/` in `includes.NOT_KERNEL`.

## Fixed — DTrace frees the alias, not the buffer, on one path of three

`dtrace_dof_property()` reads a DOF blob the loader preloaded and decodes
it from hex. Its FreeBSD half — this is inside `#ifdef __FreeBSD__`, so
it is this tree's code and not Solaris's — has **five** `goto doferr`
sites and one label:

```c
	dof = NULL;
	...
	if (len % 2 != 0)      goto doferr;   /* dof NULL, nothing allocated */
	if (bytes < sizeof())  goto doferr;   /* dof NULL, nothing allocated */

	dofbuf = malloc(bytes, M_SOLARIS, M_WAITOK);
	for (i = 0; i < bytes; i++) {
		...
		if (c1 == UCHAR_MAX || c2 == UCHAR_MAX)
			goto doferr;          /* dof still NULL - and dofbuf is not */
	}

	dof = (dof_hdr_t *)dofbuf;
	if (bytes < dof->dofh_loadsz)     goto doferr;   /* dof == dofbuf */
	if (dof->dofh_loadsz >= max)      goto doferr;   /* dof == dofbuf */

doferr:
	free(dof, M_SOLARIS);
```

Two of the five are before the allocation, where `free(NULL)` is right.
Two are after `dof` has been pointed at the buffer, where it is also
right. The fifth is in the hex-decoding loop, **between** them, and there
`dof` is still `NULL`: the free does nothing and the whole buffer leaks.

The trigger is a single non-hex character in the preloaded blob, and the
size leaked is `bytes`, which the blob chooses — the length check
against `dtrace_dof_maxsize` happens two branches later, after
`dof = dofbuf`. Boot-time and root-supplied, so not a remote hole; a
leak of an attacker-chosen size all the same, and the kind that is
invisible because the error path *looks* like it frees.

`dofbuf` is what the label has to free, and it has to start `NULL` so
the two pre-allocation paths still work. One initialiser and one name.

### The five that came with it

Reading `dtrace.c` at all is new — it needed `${MACHINE_CPUARCH}` and
the module flag ordering before it would compile — and the other five
findings in it are classes this document already has:

* `dtrace_hash_remove()` at `:8232`, `:8236`, `:8245`: `ASSERT(b != NULL)`
  compiled out, over a bucket the caller just found in that same chain.
* `dtrace_difo_init()` at `:10733` and `dtrace_difo_destroy()` at
  `:10855`: `switch (v->dtdv_scope)` whose `default:` is `ASSERT(0)`, so
  `np` and `svarp` look unset. `dtdv_scope` is userland's, which is why
  it is worth checking rather than assuming — and
  `dtrace_difo_validate()` at `:10125-10131` rejects any scope that is
  not `GLOBAL`, `THREAD` or `LOCAL`, at `:13834`, one line before
  `dtrace_difo_init()` is called at `:13837`. Enforced one function
  earlier, in the same file, which is one function further than the
  analyser carries it.

### `pci.c:858`, `pci.c:859` — a three-bit length into a four-element array

The two `pci_ea_fill_info()` findings were on the same list as the four
above, in a function that names `M_ZERO`, and they are not that class
either. They are a defect.

`pci_read_extcap()` calls `pci_ea_fill_info()` once for every PCI
function that advertises the Enhanced Allocation capability, during bus
enumeration. Everything it reads comes out of the device's own
configuration space:

```c
	int a, b;
	uint32_t val;
	int ent_size;
	uint32_t dw[4];
	...
	for (a = 0; a < num_ent; a++) {
		...
		/* Read a number of dwords in the entry */
		val = REG(ptr, 4);
		ptr += 4;
		ent_size = (val & PCIM_EA_ES);

		for (b = 0; b < ent_size; b++) {
			dw[b] = REG(ptr, 4);
			ptr += 4;
		}

		...
		base = dw[0] & PCIM_EA_FIELD_MASK;          /* :858 */
		max_offset = dw[1] | ~PCIM_EA_FIELD_MASK;   /* :859 */
```

`PCIM_EA_ES` is `0x00000007` (`pcireg.h:633`). An EA entry is one header
dword — the `val` already consumed — followed by `ent_size` more, and
the layout tops out at four of them: base-low, max-offset-low, and a
high half for each when the entry is 64-bit. The array is sized for
exactly that. The *field* is three bits wide.

So a device that reports an entry size of 5, 6 or 7 makes the inner loop
write `dw[4]`, `dw[5]` and `dw[6]` — **up to twelve bytes past the end
of a stack array, with contents the device chooses**. Nothing between
the read and the store bounds `b` against `nitems(dw)`; nothing rejects
an out-of-range `ent_size` before or after the loop. The value is
attacker-controlled in every threat model where the device is: an
emulated function presented by a hostile hypervisor, an SR-IOV VF, or
anything arriving over Thunderbolt or PCIe hotplug.

The two findings clang reports are the *other* half of the same missing
check. With `ent_size` of 0 or 1 the loop leaves `dw[0]` or `dw[1]`
unwritten, and `:858` and `:859` read them anyway. On the first
iteration that is indeterminate stack; on a later one it is the previous
entry's dwords, because `dw` is declared outside the loop and never
reset. Either way `base` and `max_offset` are then stored into the
`pci_ea_entry` and used to program resource allocation.

Clang can only see the second half — reading an array element the loop
did not write is a path it can walk, while `b < ent_size` with `ent_size`
unbounded is a range it has no reason to think exceeds 4. Reading the
finding is what turns up the first half, which is the serious one. That
is twice now that the interesting bug was one line away from the one
that was reported: `svm.c`'s `errcode_valid` was the same, found while
reading a "1st function call argument" a few lines below it.

**Fixed.** The fix keeps `ptr` advancing by what the device claimed, or
every subsequent entry parses at the wrong offset — so the excess dwords
are still read, just not stored:

```c
		for (b = 0; b < ent_size; b++) {
			dwv = REG(ptr, 4);
			if (b < (int)nitems(dw))
				dw[b] = dwv;
			ptr += 4;
		}

		if (ent_size < 2 || ent_size > (int)nitems(dw)) {
			...
			free(eae, M_DEVBUF);
			continue;
		}
```

An entry with fewer than two dwords is dropped rather than built out of
whatever `dw` still holds from the previous one. Both findings go to 0 at
an unchanged flag digest.

### Ten locks released on some paths out of a function and not others

`sys/netipsec/ipsec.c` compiles clean and reports **nothing**, in every
sweep it has ever been in. It also contains this, in `ipsec_chkreplay()`:

```c
	SECREPLAY_LOCK(replay);
	...
	if (tl < window - 1 && seq >= bl) {
		if (th == 0)
			return (0);                    /* :1357 */
		*seqhigh = th - 1;
		...
		SECREPLAY_UNLOCK(replay);
		return (1);
	}
```

Eleven ways out of that function, ten of which unlock. `SECREPLAY_LOCK`
is `mtx_lock(&(_r)->lock)`, so `:1357` returns to the caller holding a
mutex. The sibling function twenty lines down, `ipsec_updatereplay()`,
has the same block written correctly:

```c
		if (th == 0) {
			SECREPLAY_UNLOCK(replay);
			return (1);
		}
```

Two functions from one template, one with the unlock and one without.

The default clang checkers have no model for `mtx_lock`, which is why
the sweep is silent on it: this was found by reading `xform_ah.c:666`,
in a different translation unit, and following the callee. Reading is
not a gate, so `tools/verify/lock_balance.py` is.

**What it looks for.** Not balance - plenty of functions in this tree
take a lock and hand it to their caller on purpose, and `..._locked()`
names a whole family of them. It looks for *inconsistency*: within one
function, a lock released before some returns and not others. It walks
the brace structure keeping one bit - "the lock is definitely held here"
- so the join for two arms is AND, an arm that cannot fall through does
not join at all, and a construct that may be skipped joins with its own
entry state.

Four rules kill the four false-positive families, each found by reading
what the tool reported and each written into `testdata/`:

* **A callee took it.** `vm_object_deallocate()` calls
  `vm_object_deallocate_vnode(object)` and returns; the callee owns the
  lock from there. So a call as the statement before the return, as the
  return's own expression, or in the condition of the block the return
  sits in, disqualifies it. Walking *every* line between the lock and
  the return instead - which reads better - was measured and was
  useless: a mutation test that deleted one unlock guarding an early
  return in 120 files caught **2** of them, because almost every
  function calls something after taking a lock.
* **Entered holding it.** A release above the first acquire means the
  function was called locked, dropped it for work it could not do
  holding it, and took it back. `udp_append()` says so in a comment,
  `passregister()` does it across `cam_periph_mapmem()`, and
  `ext2_nodealloccg()` across `bread()`.
* **Returns holding it by contract.** If control leaves the bottom of
  the function still holding, or the last return does, that is the
  point of the function.
* **The return is not reachable.** `siba.c` writes `panic(...); return
  (ENXIO);` in two switch arms.

**What it found.** Thirteen returns in 6,914 files. Ten are leaks:

| | |
|---|---|
| `sys/netipsec/ipsec.c:1357` | the one above |
| `sys/dev/drm2/drm_bufs.c:85`, `sys/dev/drm2/drm_bufs.c:102` | `drm_get_resource_start()` and `drm_get_resource_len()`, the same three lines copied: `mtx_lock(&dev->pcir_lock); if (drm_alloc_resource(dev, resource) != 0) return 0;`. `drm_alloc_resource()` is `static` in the same file and never touches `pcir_lock`. |
| `sys/arm/allwinner/aw_mmc.c:312` | `aw_mmc_cam_request()` takes `AW_MMC_LOCK` and returns `EBUSY` on "Controller still has an active command" without it. A CAM request arriving while one is in flight is not an unusual event. |
| `sys/arm64/nvidia/tegra210/max77620_gpio.c:563` | one of **five** identical `if (rv != 0) { device_printf(...); return (ENXIO); }` arms in one function. The other four unlock. |
| `sys/arm/nvidia/drm2/tegra_bo.c:165` | `if (vm_page_iter_insert(...) != 0) return (EINVAL);` between `VM_OBJECT_WLOCK` and the `VM_OBJECT_WUNLOCK` two lines below it |
| `sys/powerpc/mpc85xx/fsl_espi.c:350` | `if (plat_clk == 0) { ...; return (EINVAL); }` under `FSL_ESPI_LOCK` |
| `sys/powerpc/pseries/phyp_vscsi.c:341` | the `malloc(..., M_NOWAIT)` failure path in attach, under `sc->io_lock` |
| `sys/dev/sound/pci/ich.c:411` | `ichchan_init()`'s `default: return (NULL);`. Latent - the driver calls `pcm_addchan()` at most three times and the switch covers 0, 1, 2 - but the arm exists, and `ch = &sc->ch[num]` is dereferenced *before* it, which is the more interesting half if it ever fires. |
| `sys/netpfil/ipfw/ip_fw_table.c:1079` | `find_table_entry()`'s `if (ta->find_tentry == NULL) return (ENOTSUP);` under `IPFW_UH_RLOCK`. Also latent: all seven in-tree table algorithms set `find_tentry`. `ipfw_add_table_algo()` is exported for modules, which is presumably why the NULL test is there at all. |

Three are not, and are in the test by name so that a change which stops
reporting one of the ten has to account for it: `iw_cxgbe/cm.c:1146`
(`solisten_dequeue()` unlocks, and the comment two lines down says so),
`vfs_mount.c:2319` (`dounmount_cleanup()` ends with `MNT_IUNLOCK(mp)`),
and `kern_proc.c:454` (`_pfind()` returns the process locked on success
and NULL otherwise - genuinely ambiguous from the text).

**What it misses.** The same mutation test, after the four rules, catches
**33 of 120**. It gives up entirely on any function containing a `goto`,
because a label joins paths the walk does not model, and that is most of
the miss. Three quarters missed and ten found is the trade a reader
wants from a list of thirteen; a list of 833, which is what the first
version produced, is a list nobody opens.

Userland reports **zero**, in `lib`, `bin`, `sbin`, `usr.bin`,
`usr.sbin` and `libexec` together - 3,997 files - and that is a
measurement rather than a no-op: 165 functions under `lib` alone have a
lock/unlock pair the tool pairs up, and every one of them is consistent.
The kernel is where this defect lives.

**All ten are fixed.** Sweep 18 finished first — editing a source under
a sweep destroys the measurement — and the tool now reports the three
non-leaks and nothing else over the same 6,914 files. Each fix is
registered in `tools/check_pbsd_marks.py`, and each entry was checked by
reverting the fix and watching the check name it; `drm_bufs.c` carries a
count of **2**, so losing one of the two identical hunks fails as well as
losing both. `test_lock_balance.py` asserts each fixed file now reports
nothing, and that the three non-leaks are still reported — a tool that
quietly stops looking is the failure mode worth a test.

### The first sweep of `bin`, `sbin`, `usr.bin` and `usr.sbin`

1,862 translation units, **1,613 OK / 249 ERROR**, 600 findings. This is
the first time any sweep has looked at the tree's own utilities; every
number here is a first observation, with nothing to compare it against
yet.

The findings by checker:

```
  170  unix.Malloc                     43  core.uninitialized.Assign
  162  core.NullDereference            10  core.uninitialized.Branch
   73  core.UndefinedBinaryOperator     9  unix.MallocSizeof
   66  core.CallAndMessage              8  core.uninitialized.UndefReturn
   48  unix.cstring.NullArg             7  core.DivideZero
                                        2  core.StackAddressEscape
                                        1  core.NonNullParamChecker
```

`unix.Malloc` at the top is the shape of the difference: userland's
`malloc()` is the one clang models properly, so leaks and double frees
are visible here in a way they are not in the kernel.

**The 249 that do not compile are 212 missing headers, in classes.**
Almost all of them are the `device_if.h` story again - a header that
does not exist in a source tree because the build generates it:

| | |
|---|---|
| 57 | `curses.h`, `ncurses.h`, `term.h`, `termcap.h` - ncurses generates its own headers with `MKterm.h.awk` and friends |
| 31 | `nodes.h`, `syntax.h`, `parser.h` - `bin/sh` generates these with `mknodes.sh` and `mksyntax.c` |
| 21 | `hostres_oid.h`, `bridge_tree.h`, `wlan_tree.h` - bsnmp modules, generated by `gensnmptree` |
| 14 | `yp.h`, `ypxfrd.h`, `yppasswd.h`, `bootparam_prot.h`, `sm_inter.h`, `rpc/key_prot.h` - **rpcgen** output from the `.x` files beside them, exactly the `*_if.m` case |
| 22 | `opt_osname.h`, `osreldate.h`, `nl_defs.h`, `y.tab.h` - other build products |
| 13 | `type_traits` - C++ sources being handed to a C analyser; these should be excluded rather than expected |
| 12 | `ipf.h`, 10 `tip.h` - headers in a subdirectory the include walk does not reach |
| 10 | `samplerate.h` - libsamplerate, which is not in the tree |
| 4 | `sys/stream.h`, `sys/stropts.h`, `net/nit.h` - SunOS headers under `#ifdef`, in packet-capture code |

Five of those classes are recoverable by running the generator the build
runs, which is what `iface_shim()` already does for the kernel's
interface headers. Three are not, and belong in `expected_errors.py`
with the reason written down.

**`dhclient`'s divide, which is guarded by a table.**
`pretty_print_option()` (`sbin/dhclient/options.c:726`) does
`numhunk = len / hunksize`, and `hunksize` is accumulated from
`dhcp_options[code].format`, where `code` comes straight off the wire.
`numhunk` reaches 0 only through `case 'A'`, which adds nothing to
`hunksize`; so an entry whose format begins with `A` would divide by
zero on any option long enough to get past the `hunksize > len` check.
Every array format in `tables.c` is `IA`, `IIA`, `BA` or `SA` - the
sizing letter always comes first - so it cannot happen today. The guard
is the contents of a static table in another translation unit, which is
why the analyser reports it and why it is worth writing down rather than
dismissing: it holds by data, not by code, and nothing checks it.

**`ping6.c:1425`** is `core.StackAddressEscape` and is real. `pinger()`
sets `smsghdr.msg_iov = iov` where `iov` is a local array and `smsghdr` is
`static struct msghdr` at `:235`, so the global holds a pointer into a
dead frame from the moment the function returns. Nothing reads it before
the next call overwrites it, and `ping` is setuid root, which makes it
exactly the kind of thing to fix rather than argue about. **Fixed** by
making `iov` itself `static` — it was already cleared on every call.
`sbin/ping/ping.c` was checked for the same shape and does not have it:
there both the `msghdr` and the `iovec` are locals of one function.

**`rtadvd/config.c:1284`** is `unix.Malloc`, "Use of memory after it is
freed", and is also real. `invalidate_prefix()` calls `delete_prefix(pfx)`
— which ends in `free(pfx)` — on the arm where `rtadvd_add_timer()`
returned NULL, and then falls straight through into
`rtadvd_set_timer(&timo, pfx->pfx_timer)`. The arm was missing its
`return`. Reachable only under allocation failure, in a root daemon driven
by router solicitations off the network. **Fixed**; the file goes from 2
findings to 1, and the one that stays is `:307`, where the analyser has
aliased `pfx` and `rai` in `rm_rainfo()` — `delete_prefix()` frees the
prefix, not the rainfo.

### `bin/sh` compiles: 19 of 26 recovered, and three findings in the shell

The largest recoverable class from the first `bin`/`sbin`/`usr.bin`/`usr.sbin`
sweep, done. `bin/sh/Makefile` writes four headers that do not exist in a
source tree:

```make
builtins.c builtins.h: builtins.def     sh mkbuiltins
nodes.c nodes.h: nodetypes nodes.c.pat  mknodes nodetypes nodes.c.pat
syntax.c syntax.h:                      mksyntax
token.h: mktokens                       sh mktokens
```

`includes.py` runs those, the way `iface_shim()` already runs
`makeobjops.awk` for the kernel's `device_if.h`. Two of the four are C
programs the build compiles for the host and runs, so this compiles and
runs them; the other two are shell scripts.

Two things had to be dealt with, and both are the sort of thing that
would otherwise have produced a shim that looks right:

* `mknodes.c` and `mksyntax.c` use `__printf0like`, `__dead2` and
  `__unused`, which come from `<sys/cdefs.h>` — a *host* tool built by
  the host compiler does not get FreeBSD's. They are defined away, which
  is what `cdefs.h` itself defines them to on a compiler without the
  attribute.
* `mkbuiltins` and `mktokens` both open `temp=`` `mktemp -t ka` ``.
  BSD's `mktemp -t PREFIX` makes `/tmp/PREFIX.XXXXXXXX`; GNU coreutils
  deprecated that spelling and wants X's in the template, so both scripts
  die on Linux — and `mktokens` dies **after** writing a `token.h` whose
  arrays are empty, which is worse than writing none, because it
  compiles. A four-line `mktemp` on the front of `PATH` does the BSD
  spelling.

**19 of `bin/sh`'s 26 translation units go ERROR → OK**, and the
directory now compiles completely. A generator that fails leaves the
ERROR standing rather than returning a half-written directory: a compile
that succeeds and means nothing is the outcome this whole apparatus
exists to avoid.

Three findings came with them, in the system shell:

* **`eval.c:603`** — `evalpipe()` does `pip[1] = -1;` at the top of the
  loop and `prevfd = pip[0];` at the bottom. On the last element of the
  pipeline `pipe(pip)` is not called, so `pip[0]` was never written by
  that iteration. The parser builds an `NPIPE` only for two commands or
  more, so what is read is the *previous* iteration's read end — which
  the `close()` two lines up just shut — rather than an indeterminate
  value, and it is dead either way. **Fixed** as `pip[0] = pip[1] = -1;`,
  which is what `prevfd` started as; the finding goes to 0.
* **`jobs.c:1518`** (`cmdputs()`, `subtype = *p++`) and
  **`histedit.c:725`** (`sh_matches()`, `strndup(matches[1], ...)`) are
  both about the contents of a buffer filled elsewhere — the string
  `cmdputs()` walks and the array libedit's `el_filename_complete()`
  fills. The cross-translation-unit out-parameter class, one indirection
  out.

### Two in root daemons, from the userland use-after-free list

`unix.Malloc` is the largest bucket in the first `bin`/`sbin`/`usr.bin`/
`usr.sbin` sweep — 170 findings — because userland's `malloc()` is the
one clang models properly. Twenty-one of them are "Use of memory after
it is freed" or "Attempt to free released memory". Two read so far, both
in daemons that run as root and take their input off the network.

**`rpc.lockd`: a double free, because the callee does not clear what it
freed.** `split_nfslock()` (`usr.sbin/rpc.lockd/lockd_lock.c`) allocates
the left half of a split lock range, then the right:

```c
	if ((spstatus & SPL_LOCK2) != 0) {
		*right_lock = allocate_file_lock(...);
		if (*right_lock == NULL) {
			debuglog("Unable to allocate resource for split 1\n");
			if (*left_lock != NULL) {
				deallocate_file_lock(*left_lock);
			}
			return SPL_RESERR;
		}
```

and its only caller, `unlock_nfslock()`, does this with the answer:

```c
		if (spstatus == SPL_RESERR) {
			if (*left_lock != NULL) {
				deallocate_file_lock(*left_lock);
				*left_lock = NULL;
			}
```

The callee frees `*left_lock` and leaves the pointer where it was, so
the caller's `!= NULL` test passes and frees it again. Between the two
frees, `unlock_nfslock()` also passes it to `debuglog()` and
`dump_filelock()` — `lockd_lock.c:981` is what clang reports, and it is
the *read*, one line before the second free.

Reachable when the second `allocate_file_lock()` fails, on an NLM unlock
that splits an existing range. Allocation failure, in a root daemon
answering the network. `*left_lock = NULL;` after the callee's
`deallocate_file_lock()` is the whole fix, and it makes the callee's
contract match what its caller already assumes.

**`ppp`: `realloc()` moved the block and nothing told the owner.**
`datalink2iov()` (`usr.sbin/ppp/datalink.c`) writes the reallocated
pointer into the iovec and not back into the structure:

```c
  iov[*niov].iov_base = dl ? realloc(dl->name, DATALINK_MAXNAME) : NULL;
  iov[(*niov)++].iov_len = DATALINK_MAXNAME;

  link_fd = physical2iov(...);

  if (link_fd == -1 && dl) {
    free(dl->name);          /* :1418 */
    free(dl);
  }
```

`realloc()` is free to move the allocation, and when it does, the old
`dl->name` is freed and `dl->name` still points at it. The error path
then frees that stale pointer. `datalink.c:1418`.

**All three are fixed**, each A/B'd at an unchanged flag digest against
the record the include-path work produced. `lockd_lock.c` and
`datalink.c` go from one finding to none.

`patch(1)` went from **five to two**, not three to two: the missing
`noreturn` was costing findings in `inp.c` and `util.c` as well, which
is the point — the declaration is not local to the three sites that
made it visible. What is left is `pch.c:1316` and `:1320` in
`pch_swap()`, a different question. Neither of the first two is a shape
the kernel half of this document has seen: the kernel half is mostly
`M_ZERO` and locks, and this is what a modelled `malloc()` finds
instead.

**`patch(1)`: three findings, one missing `noreturn`.** `pch.c:998`,
`:1020` and `:1045` are all this arm, three times in `another_hunk()`:

```c
	if (fillold > p_ptrn_lines) {
		free(s);
		p_end = fillnew - 1;
		malformed();
	}
	p_char[fillold] = ch;
	p_line[fillold] = s;          /* clang: use after free */
```

`malformed()` is `static` and its body is one call to `fatal()`, and
`fatal()` ends in `my_exit(2)`. `util.h:47` declares
`my_exit(int) __attribute__((noreturn))` — and `util.h:35` declares
`fatal()` with a `format` attribute and nothing else. `fatal()` lives in
`util.c` and its callers are in `pch.c`, so there is no way for the
analyser to see through it.

The three findings are the visible cost. The larger one is that **every
path after a `fatal()` anywhere in `patch(1)` is explored as though the
program continued** — inventing findings and spending the analyser's
budget on unreachable code, in a program whose whole input is a file
somebody else wrote. The same class as the `atrun.c` `__dead2` fix
above; the fix is the same shape, on the declaration rather than at the
call sites.

Two more were read and put down; they are in the *Not defects* table
below, where the citation reader can see them.

**`gencat(1)`: a loop that does not advance, so it frees the same string
for ever.** `MCDelSet()` (`usr.bin/gencat/gencat.c:667`) is

```c
		msg = set->msghead.lh_first;
		while (msg) {
			free(msg->str);
			LIST_REMOVE(msg, entries);
		}
```

`LIST_REMOVE` unlinks `msg` from the list; it does not change `msg`. So
the loop never terminates, and its **second** iteration frees `msg->str`
a second time. `MCDelSet()` is what `$delset N` in a message-catalogue
source calls, and the set has to exist for the arm to be taken — so any
`.msg` file with a `$delset` for a set it defined hangs `gencat` and
double-frees. It has plainly never been run.

The fix is the loop the same file's `MCDelMsg()` would have needed if it
looped: take the head each time, unlink it, then free the string *and*
the node —

```c
		while ((msg = set->msghead.lh_first) != NULL) {
			LIST_REMOVE(msg, entries);
			free(msg->str);
			free(msg);
		}
```

which is also the only version that terminates. (`MCDelMsg()` leaks its
`_msgT` in the same way and is otherwise correct; that leak is left
alone rather than folded into this.) **Fixed**; `gencat.c` goes from
eleven findings to ten.

**`rtadvd`: a function that ends `return (ifi)` and three arms that free
it.** `update_ifinfo()` (`usr.sbin/rtadvd/if.c`) has three
`if (ifi_new) { free(ifi); } continue;` arms and finishes
`return (ifi);` at `:613`. A last loop iteration taking any of them
returns a dangling pointer. All seven call sites discard the value, so
nothing is dereferenced — but returning an indeterminate pointer is
itself the read, and it is the same shape as `ping6`'s escaping `iov`
above. **Fixed** with `ifi = NULL;` beside each free, which is safe
because the next iteration's `TAILQ_FOREACH` reassigns it; one finding
to none. Registered with a count of **3**, so losing one of the three
fails as well as losing all of them.

**`mountd`: the guard exists and runs after the code it guards.**
`get_net()` (`usr.sbin/mountd/mountd.c:3452`) does

```c
	p = prefp = NULL;
	if ((opt_flags & OP_MASKLEN) && !maskflg) {
		p = strchr(cp, '/');
		*p = '\0';
		prefp = p + 1;
	}
	...
	if (opt_flags & OP_MASKLEN) {
		preflen = strtol(prefp, NULL, 10);
		...
		*p = '/';               /* :3546 */
	}
```

An `/etc/exports` line reading `-network 1.2.3.0/24 -mask 255.255.255.0`
sets `OP_MASKLEN` from the `-network` argument — correctly, only when a
`/` is present — and then the `-mask` handler calls
`get_net(cpoptarg, ..., maskflg=1)`. With `maskflg` set the first block
is skipped and `p` and `prefp` stay NULL; the second block tests only
the **flag**, so it runs, passes NULL to `strtol()` and writes through
`p`.

`check_options()` **does** reject it — *"-mask and /masklen are mutually
exclusive"*, at `:3886` — and it is called from `get_exportlist_one()`
at `:1825`, after the whole option line has been parsed. The guard is
written, is correct, and runs after the code it was meant to guard.
Order matters too: `-mask` before `-network` does not crash, because
`OP_MASKLEN` is not set yet.

`/etc/exports` is root-written, so this is robustness rather than a
security boundary — but `mountd` dying at startup or on a `SIGHUP`
reload takes NFS down. **Fixed** by keying on `prefp`, which is the
thing actually needed, and by checking the `strchr` result two lines
up — that one is not reachable today and is the same premise, so it is
fixed and said so rather than left. Eight findings to seven.

Four more read and put down — two in `makefs`, two in the `preen`
framework `fsck` and `quotacheck` share — and all four are the same
thing: the analyser does not model `TAILQ_REMOVE`. Every one of them
removes the node before freeing it, and takes the next one off a list
the freed node is no longer in.

That is **sixteen** of the twenty-one read: five defects fixed, eleven
not defects — and **nine of the eleven are one macro**. `hastd`'s reload
path and `jail`'s config parser, the two the network can reach, are both
in that nine: every loop takes the head, `TAILQ_REMOVE`s it, and only
then frees it.

Five sites are left, and they are `gprof` and the two `patch` findings
in `pch_swap()` — programs that read a file a developer already chose to
trust. The interesting half of this list is done.

### 249 → 190: three include-path answers read out of the build

Three more classes of "does not compile", each answered by asking the
build rather than guessing. **59 translation units recovered, zero
regressions**, and the 59 carry 102 findings that no sweep had seen.

**A source is compiled by the Makefile whose `SRCS` names it, which is
not always an ancestor of it.** `usr.bin/tip/tip/Makefile` has

```make
CFLAGS+=-I${.CURDIR} -DDEFBR=9600 ...
.PATH:  ${.CURDIR}/../libacu
SRCS=   acu.c acutab.c ... biz22.c biz31.c ...
```

so `usr.bin/tip/libacu/biz22.c` is compiled in `usr.bin/tip/tip` and
finds `tip.h` there. The nearest ancestor holding a Makefile is
`usr.bin/tip`, whose Makefile is `SUBDIR=tip` and carries no flags at
all — and that is the directory the analyser was asking bmake about.
Ten ERRORs, every one on `'tip.h' file not found`.

`userland_names.build()` already resolves `SRCS` against `.PATH` for
every Makefile in the tree; it was throwing away *which* Makefile gave
each answer. It keeps them now, and `_component_dir()` prefers a naming
directory — but **only when the ancestor is not itself one of them**.
Preferring a namer unconditionally moved 265 sources, most of them onto
the wrong side of a pair: `sbin/fsdb` reaches into `sbin/fsck_ffs` and
`cddl/usr.bin/ctfconvert` into `cddl/contrib`, and in both the source's
own directory names it too and is the right answer. With the ancestor
preferred, 172 move, and they are the ones that should.

**A `-I` into the installed header tree means the tree's own headers.**

```make
CFLAGS+= -I${SYSROOT:U${DESTDIR}}/${INCLUDEDIR}/private/samplerate
```

is `usr.sbin/virtual_oss/virtual_oss/Makefile`, and with `SYSROOT` and
`DESTDIR` both empty it comes back as `-I//usr/include/private/samplerate`
— an absolute path on the *host*. Ten translation units failed on
`'samplerate.h' file not found` while
`incs_shim()/private/samplerate/samplerate.h` sat there the whole time,
staged from `lib/libsamplerate`'s own `INCS`. Any `-I` whose path ends
in `usr/include/...` is now rerooted into the shim.

**Two `INCS` that no source file backs.** `installed_headers()` maps an
installed path to a source path, so a header the build *generates* has
nothing to link to and is simply absent from the layout. Two are asked
for in scope, and both recipes are three lines in the Makefile that
installs them: `osreldate.h` is `__FreeBSD_version` out of
`sys/sys/param.h` formatted by `include/mk-osreldate.sh` (4 files), and
`bsdxml.h` is `contrib/expat/lib/expat.h` with three `sed`
substitutions (4 files). `usr.sbin/bsdinstall`'s `opt_osname.h` is one
`#define` its programs reach through `-I${.OBJDIR}/../include`, an
object-directory path with no answer here (10 files).

The reroot is the one of the three that could have reached outside the
scope it was written for, so it was measured: **zero** translation units
under `lib/libc`, `lib/msun` and `libexec` carry a `-I` into the
installed tree at all, so the libs shard cannot move. The other two are
bounded by construction — the naming map only differs where `.PATH`
reaches sideways, and the two staged headers are new files in a
directory that had neither.

### 190 → 133: ncurses generates its own headers, so generate them

The largest class left. 47 translation units failed on `curses.h`,
`ncurses.h`, `term.h` or `termcap.h`, and `lib/ncurses/tinfo/Makefile`
writes every one of them.

The recipes have a uniform shape —
`sed <${NCURSES_DIR}/include/<name>.in >$@` with a list of `-e`
substitutions in two forms:

```make
	    -e "/@NCURSES_MAJOR@/s%%${NCURSES_MAJOR}%"      # first, on matching lines
	    -e "s%@NCURSES_USE_DATABASE@%1%g"               # everywhere, literal value
```

(and a trailing `g` on the first form makes it every occurrence on the
matching line). The variable **names** are read out of the Makefile, so
one added upstream is picked up; the **values** come from bmake, so a
version bump is too. Only the shape of each rule is written down here,
each with the Makefile line it came from.

`curses.h` is then `curses.head` + `MKkey_defs.sh Caps Caps-ncurses` +
`curses.wide` + `curses.tail`; `ncurses.h` is the `INCSLINKS` copy of
it; and `term.h` is `MKterm.h.awk` over the capability tables followed
by `edit_cfg.sh`. That last one leaves `@HAVE_SGTTY_H@` standing —
`edit_cfg.sh` handles exactly four names and that is not one of them —
and **so does the real build**: it sits in an `#elif` the first arm
never lets the preprocessor reach. Left alone rather than "fixed", so
the header is the one the build produces.

**57 more translation units, ERROR → OK, zero regressions**, and zero
flag digests changed for anything that does not use curses.

So the utilities have gone **249 → 190 → 133**, and what remains is
seven classes, none of them large:

| | |
|---|---|
| 37 | not a missing header at all: `ipfilter`'s `ipsend`/`ipftest`/`ipresend`, which `sbin/ipf/Makefile:9` has **commented out** of `SUBDIR`, plus a handful of one-off compile errors |
| 13 | `type_traits` — C++ sources handed to a C analyser |
| 21 | `hostres_oid.h`, `bridge_tree.h`, `wlan_tree.h` — bsnmp modules, `gensnmptree` |
| — | **rpcgen output: done.** Each of those Makefiles says how — `RPCGEN= ... rpcgen -I -C` and a `name.h: name.x` rule — so a directory whose Makefile mentions `rpcgen` gets its headers generated from the `.x` beside it or in `include/rpcsvc`. 14 recovered, in two rounds, because the first missed two things the build does: `include/rpcsvc/Makefile` has `INCSGROUPS= INCS RPCHDRS` and installs `key_prot.h` to **both** `rpcsvc/` and `rpc/` (`RPCHDRSDIR= ${INCLUDEDIR}/rpc`), which is the spelling `rpc.ypupdated` uses; and `rpc.yppasswdd` has its own `yppasswd_private.x` beside its Makefile. **Two are left and stay left**: `rpc.statd`'s `statd.c` and `bootparamd`'s `main.c` reference the per-version *dispatcher* — `sm_prog_1`, `bootparamprog_1` — which FreeBSD's own rpcgen declares in the header (`usr.bin/rpcgen/rpc_hout.c:264`, `pdispatch()`) and the **host's** rpcgen does not. Building the tree's rpcgen for the host wants `<rpc/types.h>` and then `<sys/_null.h>`: the FreeBSD header universe inside a host tool, which is the compile-that-means-nothing hazard this whole apparatus exists to avoid. Recorded as an ERROR with the reason rather than papered over. |
| 9 | `parser.h` under `usr.bin/localedef` — yacc output |
| 6 | `nl_defs.h` under `usr.bin/netstat` — generated by its own Makefile |
| 6 | `ipf.h` — the same unbuilt ipfilter programs |

### `pf`'s ten, and a union member that C allows and C++23 does not

All ten uninitialised-value findings in `sys/netpfil/pf` are one
mechanism, and it is the first one in this document that is **not** a
false positive so much as a language change the port is walking into.

`struct pf_addr` (`sys/netpfil/pf/pf.h:302`) is a struct wrapping an
anonymous union:

```c
struct pf_addr {
	union {
		struct in_addr		v4;
		struct in6_addr		v6;
		u_int8_t		addr8[16];
		u_int16_t		addr16[8];
		u_int32_t		addr32[4];
	};		    /* 128-bit address */
};
```

and `pf_addrcpy()` (`sys/net/pfvar.h:577`, `static inline`, so the
analyser can see all of it) writes **one** member:

```c
	case AF_INET:
		memcpy(&dst->v4, &src->v4, sizeof(dst->v4));
```

Every caller then reads a **different** one. `pf_change_ap()` does
`pf_addrcpy(&ao, a, pd->af)` and then `pf_cksum_fixup(*pd->ip_sum,
ao.addr16[0], ...)`; `PF_AEQ`/`PF_ANEQ` (`pfvar.h:511`) are
`(a)->addr32[0] == (b)->addr32[0]`. Four findings in `pf_change_ap()`,
one in `pf_change_a6()`, two in `pf_change_icmp()`, one in
`pfsync_state_import()` and two in `pf_map_addr_sn()` — every one of
them "the 2nd function call argument is an uninitialized value" or "the
left operand of `==` is a garbage value", and every one of them the same
read.

The mechanism, probed rather than asserted — thirty lines, the union and
the copier and nothing else:

```c
static inline void
addrcpy(struct pf_addr *dst, const struct pf_addr *src, int af)
{
	if (af == 4)
		memcpy(&dst->v4, &src->v4, sizeof(dst->v4));
	else
		memcpy(&dst->v6, &src->v6, sizeof(dst->v6));
}

unsigned short
probe(struct pf_addr *a, int af)
{
	struct pf_addr ao;

	addrcpy(&ao, a, af);
	if (af == 4)
		return fixup(0, ao.addr16[0]);
	return 0;
}
```

```
union_probe.c:32:10: warning: 2nd function call argument is an
    uninitialized value [core.CallAndMessage]
union_probe.c:30:2: note: Calling 'addrcpy'
union_probe.c:17:6: note: Assuming 'af' is equal to 4
union_probe.c:17:2: note: Taking true branch
union_probe.c:30:2: note: Returning from 'addrcpy'
```

The note trail is the whole answer: clang **did** take the `AF_INET`
arm, **did** see the `memcpy` into `v4`, and still calls `addr16[0]`
uninitialised. The control — the same function reading `ao.v4.s_addr`,
the member that was written — reports nothing at all.

So the analyser is modelling member-wise initialisation and not
transferring it between union members. In **C** that is the analyser
being conservative: C11 6.5.2.3 and its footnote 99 permit reading a
union member other than the one last stored, and the bytes are exactly
the bytes `memcpy` wrote. Nothing is wrong with `pf` as it stands.

In **C++** it is not conservative, it is right. Reading a non-active
union member is undefined; P0137's object model made that explicit, and
C++23 has not relaxed it. Every one of these ten sites is a place where
a mechanical C→C++ translation keeps compiling and stops being defined,
and none of them will produce a diagnostic when it does — which is why
they are here rather than in the table below.

The fix, when the port reaches `pf`, is `std::bit_cast` or a
`memcpy`-through-`unsigned char` at the read as well as the write; the
`addr8`/`addr16`/`addr32` views are exactly what `bit_cast` is for.
Recorded now, while the finding that points at each one still exists.

## Not defects, and why they looked like defects

Kept because the reasoning is what stops them being re-reported.

| reported | why it is not a defect |
|---|---|
| `sys/fs/nfsclient/nfs_clrpcops.c:1793`, `sys/fs/nfsclient/nfs_clrpcops.c:3834` | One mount flag word, read twice, with a call in between. `nfsrpc_read()` sets `nfhp = np->n_fhp` under `NFSHASNFSV4(nmp)` and then re-tests `NFSHASNFSV4(nmp)` inside the retry loop; `nfsrpc_readdir()` sets `rderr` under `nd->nd_flag & ND_NFSV4` and reads it under the same test twenty lines down. Neither word changes: the only writes to `nm_flag` after mount are `sys/fs/nfsclient/nfs_clbio.c:1632` and `sys/fs/nfsclient/nfs_clvfsops.c:2098`, `sys/fs/nfsclient/nfs_clvfsops.c:2100`, and they touch only `NFSMNT_RDIRPLUS` and `NFSMNT_NOLOCKS` — the version bits are fixed for the life of the mount — and `nfsv4_loadattr()` never touches `nd_flag` (it is 1,250 lines of `nfs_commonsubs.c` with no write to it). The analyser has to assume a cross-TU call can change either. |
| `sys/fs/nfsclient/nfs_clrpcops.c:3032` | `nfsrvd_renamerpc()` reads `nd->nd_flag` after `if (ret == 0) NFSCL_REQSTART(...)`, so on `ret != 0` the descriptor looks unwritten. It is not: `nfscl_renamedeleg()` (`nfs_clstate.c:4960`) increments `retcnt` only on the two lines that also set `*gotfdp` or `*gottdp` — `:5070`/`:5071` and `:5112`/`:5113` — and every early return sets both to 0 and returns 0. So `ret > 0` implies one of the three `NFSCL_REQSTART` above ran. The correlation is between a return value and two out-parameters, in another translation unit. |
| `sys/fs/nfsclient/nfs_clrpcops.c:3740`, `sys/fs/nfsclient/nfs_clrpcops.c:4214` | `dp` is NULL until the first directory entry is written, and both sites are `dp->d_reclen += left` inside `if (_GENERIC_DIRLEN(len) + NFSX_HYPER > left)`. `left` is `DIRBLKSIZ - blksiz` with `DIRBLKSIZ` 512 - `sys/fs/nfs/nfsport.h:103` includes `<ufs/ufs/dir.h>`, whose `sys/ufs/ufs/dir.h:73` makes it `DEV_BSIZE`, and `blksiz` is only ever incremented after a `dp` has been set — so `dp == NULL` implies `blksiz == 0` implies `left == 512`. The largest the guard can be is `_GENERIC_DIRLEN(255) + 8`, and `offsetof(struct dirent, d_name)` is 24, so `(24 + 255 + 1 + 7) & ~7` is 280 and the sum is 288. The branch cannot be taken on the entry where `dp` is NULL. |
| `sys/fs/nfsclient/nfs_clrpcops.c:6176`, `sys/fs/nfsclient/nfs_clrpcops.c:6248`, `sys/fs/nfsclient/nfs_clrpcops.c:8802`, `sys/fs/nfsclient/nfs_clrpcops.c:8810`, `sys/fs/nfsclient/nfs_clrpcops.c:9973` | Five of the cross-function out-parameter class in one file. `retonclose` (`:6246`) is written by `nfsrv_parselayoutget()` at `:8009`/`:8011`, four lines into the function, and read by `nfsrpc_layoutgetres()` only under `if (laystat == 0)` — which is the caller's `error`, which is 0 only if that parse ran. `*nfhpp` (`:8800`, `:8808`) is set by `nfscl_mtofh()`, which sets `ND_NOMOREDATA` on every path that leaves it NULL, and the block is entered on `(nd->nd_flag & ND_NOMOREDATA) == 0`. `lease` (`:9971`) is filled only on the NFSv4 path of `nfsrpc_statfs()` and read only under `if (nmp->nm_clp != NULL)`, and `nm_clp` is set in exactly one place, `nfs_clstate.c:965`, on an NFSv4 mount. `devid` (`:6174`) is a parameter of an exported function. |
| `sys/fs/nfsserver/nfs_nfsdsocket.c:641`, `sys/fs/nfsserver/nfs_nfsdsocket.c:1359` | `vp` at `:641` and `md`/`dpos` at `:1359` are both dominated by their writes within one iteration: `md = nd->nd_md` at `:1219` runs unconditionally on every pass of the ops loop that does not `break` out of it, and `:1359` is inside that same pass. |
| `sys/fs/nfsserver/nfs_nfsdserv.c:3153`, `sys/fs/nfsserver/nfs_nfsdserv.c:3184` | `nfsquad_t` is `union nfs_quadconvert { u_int32_t lval[2]; u_quad_t qval; }` (`nfsproto.h:822`). The wire gives a clientid as two 32-bit words, so the code writes `clientid.lval[0]` and `clientid.lval[1]` and compares `clientid.qval`. Defined in C; the analyser does not transfer initialisation between union members. 94 writes through `lval` and 76 reads of `qval` across `sys/fs`, and every one of them is undefined in C++23 — which is why this is also in the port ledger, next to `pf_addr`. |
| `sys/fs/nfsclient/nfs_clbio.c:736` | `ncl_bioread()`'s `default:` arm sets `bp = NULL` and the read below is guarded by `if (n > 0)`. `n` is 0 at `:472` before the loop and can only become non-zero in the `VREG`, `VDIR` or `VLNK` arms — so reaching the guard with `n > 0` and `bp == NULL` needs `vp->v_type` to change between two iterations of one loop over one vnode. |
| `lib/libufs/cgroup.c:164` | The third program in the `validate_sblock()` row above. `ino_to_fsba()` divides by `fs->fs_ipg`, and `char block[MAXBSIZE]` is bzero'd to `fs->fs_bsize`; `ffs_subr.c:660` has `FCHK(fs->fs_ipg, <, fs->fs_inopb, ...)` and `:648`/`:649` bound `fs_bsize` to `[MINBSIZE, MAXBSIZE]`. libufs reads every superblock through `sbget()`, which is `ffs_sbget()`, which runs `validate_sblock()` at `ffs_subr.c:275`. |
| `sbin/hastd/hastd.c:702`, `usr.sbin/jail/config.c:268`, `usr.sbin/jail/config.c:274`, `usr.sbin/jail/config.c:276`, `usr.sbin/jail/config.c:914` | Five more of the same macro, and the two programs the list above singled out as worth reading first because they are the network-facing ones. `hastd_reload()`'s `failed:` block does `TAILQ_REMOVE(&newcfg->hc_listen, nlst, hl_next); free(nlst);` and loops back to `TAILQ_FIRST`; all three `goto failed` sites are above the one `yy_config_free(newcfg)` on the success path, so `newcfg` itself is not freed when the block reads it. `jail`'s `free_param()` is `free(p->name); free_param_strings(p); TAILQ_REMOVE(pp, p, tq); free(p);` — removed before freed — and `free_param_strings()` is the same take-head, remove, free loop one level down. Nothing here is out of order; the analyser is not modelling the macro. |
| `sbin/fsck/preen.c:324`, `sbin/quotacheck/preen.c:280` | The same `preen` framework in two programs, and the same order in both: the caller does `TAILQ_REMOVE(&d->d_part, p, p_entries)` **then** `free(p)`, and only then calls `startdisk(d)`, which takes `TAILQ_FIRST(&d->d_part)` — a node the freed one is no longer in front of, because it is no longer in the list at all. `fsck`'s `startdisk()` runs the check and `quotacheck`'s forks `chkquota()`, but the lifetime question is identical. The queue macros again: this is the fourth pair of findings in this sweep whose whole content is that the analyser does not model `TAILQ_REMOVE`. |
| `sbin/fsck_ffs/pass5.c:287` (`i / fs->fs_frag`), `sbin/quotacheck/quotacheck.c:620` (`inum % sblock.fs_ipg`) | Both divisors come out of the superblock of the filesystem being checked, which is exactly the attacker-supplied input if the threat model is "somebody hands you a disk image". Both are validated, in `sys/ufs/ffs/ffs_subr.c` — `validate_sblock()` has `FCHK(fs->fs_frag, <, 1, ...)` and `FCHK(fs->fs_frag, >, MAXFRAG, ...)` at `:652`, and `FCHK(fs->fs_ipg, <, fs->fs_inopb, ...)` at `:660`. Both programs read the superblock through `sbget()` (`fsck_ffs/setup.c:410`, `quotacheck.c:306`), which is libufs' entry into `ffs_sbget()`, and `validate_sblock()` runs at `ffs_subr.c:275` — **before** the check-hash comparison, so `UFS_NOHASHFAIL` (which `fsck_ffs` passes deliberately, to repair damaged filesystems) does not skip it. Neither caller passes `UFS_NOWARNFAIL`, so even the downgradeable checks are errors. A different translation unit, in the kernel tree, shared with userland through libufs — which is why the analyser cannot see it. |
| `usr.sbin/makefs/ffs.c:1219` | The same shape and not the same question: `makefs` **builds** the superblock it is dividing by, from its own command line, rather than reading one off a disk. |
| `usr.sbin/makefs/walk.c:376` | `apply_specdir()` saves `next = curfsnode->next` before `free_fsnodes(curfsnode)`, which is the `_SAFE` idiom written out by hand. `free_fsnodes()` frees a whole sibling chain — `for (cur = node; cur != NULL; cur = next)` — but only after unlinking `node` from that chain and setting `node->next = NULL`, and the unlink runs whenever `node->first != node`, which holds here because `first` is the `.` entry and `curfsnode` starts at `dirnode->next`. So only the one node and its children go. The analyser is not modelling the queue macros. |
| `usr.sbin/makefs/cd9660.c:1517` | `cd9660_generate_path_table()`'s breadth-first loop takes `n = TAILQ_FIRST(&pt_head)`, saves `dirNode = n->node`, `TAILQ_REMOVE`s and frees `n` — in that order — and the `n` it reuses later in the body comes from a fresh `PTQUEUE_NEW`. Same class as the row above. |
| `sys/netpfil/pf/pf.c:3391`, `sys/netpfil/pf/pf.c:3406`, `sys/netpfil/pf/pf.c:3430`, `sys/netpfil/pf/pf.c:3445`, `sys/netpfil/pf/pf.c:3510`, `sys/netpfil/pf/pf.c:3554`, `sys/netpfil/pf/pf.c:3568`, `sys/netpfil/pf/if_pfsync.c:733`, `sys/netpfil/pf/pf_lb.c:906` | `struct pf_addr` is a union; `pf_addrcpy()` writes `v4` and every caller reads `addr16[]` or `addr32[]`. **In C this is defined** (6.5.2.3 footnote 99) and the bytes are the bytes `memcpy` wrote, so these are not defects and are cited here. In C++ reading a non-active union member is undefined, and none of the ten will diagnose when the port reaches them — which is why they are also written up ABOVE this line, with the probe that shows the analyser taking the `AF_INET` arm, seeing the `memcpy`, and calling the read uninitialised anyway. Cited so they leave the unread bucket; read the section, not this row, before touching `pf`. |
| `usr.bin/tail/reverse.c:209` | `r_buf()`'s out-of-memory loop takes `first = TAILQ_FIRST(&head)` *before* testing `TAILQ_EMPTY(&head)`, which reads as a null dereference waiting to happen and is not one: the empty case calls `err(1, ...)` and exits before `first->len` is reached, and every `free(first)` is preceded by its `TAILQ_REMOVE`. The analyser's "use after free" is it re-entering the loop with the queue macros unmodelled. |
| `usr.sbin/rtadvd/config.c:307` | `rm_rainfo()`'s `while ((sol = TAILQ_FIRST(&rai->rai_soliciter)))`, after a loop of `delete_prefix(pfx)`. `delete_prefix()` frees the *prefix* and decrements `rai->rai_pfxs`; it does not free the rainfo. The analyser has aliased `pfx` and `rai`. Worth keeping beside the fix two hundred lines down at `:1284`, which is the same file, the same function family, and a real use-after-free. |
| `s_significand.c`: `-ilogb(x)` overflows | `math.h:45` defines `FP_ILOGB0` as `(-__INT_MAX)`, **not** `INT_MIN`, precisely so negation is safe. CBMC does not model `ilogb`, so its return was unconstrained. |
| `s_cosl`, `s_sinl`, `s_tanl`: `-n` overflows | `n` is written by `__ieee754_rem_pio2l`, also unmodelled. |
| `clock()`, `alarm()`, `svc_run()` | the values come from `getrusage`, `setitimer`, unmodelled externs. |
| `strcat`, `stpcpy`, `memrchr`, … | a nondeterministic `char *` includes NULL. These are the functions' *missing preconditions*, not bugs. |
| ~43 `lib/msun` "division by zero" | floating-point division by zero is **defined** by IEEE-754, and msun depends on it for `log(0)`, `logb`, `rsqrt` and `catrig`. |
| `nsap_addr.c:xtob`, `getopt_long.c:gcd` | `static`. Their callers constrain the domain; a modular check does not see callers. |
| `hash_buf.c:325`, `res_update.c:195`, `res_findzonecut.c:642` **and `:651`** "use after free" | all four are `while ((p = HEAD(list))) { ...; UNLINK(list, p); free(p); }`. `UNLINK` updates `list.head` **before** the free — but only on the branch where `p->link.prev == NULL`, which the analyser cannot prove holds for a list head. This row named **three of the four** until the `unix.Malloc` category was read: `free_nsrrset()` at `:641-642` and `free_nsrr()` at `:650-653` are the same idiom ten lines apart, and only the first was here. The guard on three of four, in this document's own triage table. |
| `gethostbyht.c:213`, `getnetbyht.c:182` "garbage returned" | the path needs `errno` to be non-zero at `return ((errno != 0) ? errno : -1)` and zero at the caller's `!= 0`. `errno` is `(*__error())`, a call the analyser re-evaluates opaquely, so it does not know the two reads agree. |
| `citrus_lookup_factory.c` "garbage returned" | same shape: `dump_db()` returns `errno` after a failed `malloc`, and the analyser does not model `malloc` setting `ENOMEM`. Reaching it needs `malloc` to return NULL with `errno == 0`, which FreeBSD's allocator does not do — `malloc(0)` returns a unique pointer rather than NULL. |
| `cpuset_alloc.c:32` `MallocSizeof` | `CPU_ALLOC_SIZE(n)` is `__BITSET_SIZE(n)`, a **byte count**, deliberately not `sizeof(cpuset_t)`. |
| `radixsort.c:109` `MallocSizeof` | `malloc(n * sizeof(a))` where `a` is `const u_char **` and the elements are `const u_char *`. Both are pointers, so the size is right on every supported target; `sizeof(*a)` would say so more clearly. |
| `g_stripe.c:111`, `g_shsec.c:107` "division by zero" | `lcm(a, b)` is `(a * b) / gcd(a, b)`, `static`, and every caller passes a sector size. |
| `fread.c:129`, `fnmatch.c:331`, `getdelim.c:103`, `fvwrite.c:182`, `fts*.c` "null passed to memcpy" | the pointer is the caller's buffer or the result of an allocation the analyser cannot see succeed. `fread(NULL, ...)` is the caller violating the contract, not libc having a bug. |
| `msgcat.c:241` "null passed to strlcpy" | the path needs `lang == NULL`, and `strdup(lang)` twenty lines earlier would have crashed first. |
| `uipc_mbuf.c:1199`, `igmp.c:2837` "unchecked M_NOWAIT" (my own lint) | `if (m && ...)` and `if (m)` are NULL tests. The first version of `nowait_check.py` did not know that. |
| `if_ptnet.c:1760` (same lint) | `mhead = mtail = m_getcl(...)` and the check is on `mhead`. |
| `uma_core.c:1252` (same lint) | `sizeof(hash->uh_slab_hash[0])` does not evaluate the pointer. |
| `sys/arm64/vmm/vmm_arm64.c:1054-1063` "garbage value" | `pte` and `pte_shift` are set in a `for (;levels > 0;)` loop that falls through to `done:`. `levels` is `howmany(ia_bits - granule_shift, granule_shift - 3)` and the code above rejects `tsz < 16 \|\| tsz > 39` and every granule but 4K and 16K, so `ia_bits - granule_shift >= 11` and the loop always runs. |
| `sys/arm64/vmm/vmm_arm64.c:415` "garbage value" | `vmm_base` is assigned inside one `if (!in_vhe())` block and used inside another. `in_vhe()` is a call, so the analyser explores false-then-true. Fragile, not wrong. |
| `sys/arm64/vmm/vmm_mmu.c:408` "undefined pointer" | `l3_list` is allocated under `if (invalidate)` and used under `if (invalidate)`, with `M_WAITOK`. |
| `sys/arm64/broadcom/genet/if_genet.c:1413`, `busdma_machdep.c:111` | driver attach paths where the analyser cannot see the device probe that establishes the field. |
| `sys/x86/isa/clock.c:200` `i8254_freq / freq` | all four callers checked: `sc_tone()` guards `if (herz)`, syscons guards `pitch != 0`, `vtterm_beep()` guards `(param & 0xffff) == 0`, and spkr(4)'s `SPKRTONE` ioctl routes frequency 0 to `rest()`. Exported, so rule three cannot see them. |
| `sys/net/altq/altq_subr.c:922` `machclk_freq / hz` | `hz` **is** range-checked — `subr_param.c:187` clamps it to `[HZ_MINIMUM, HZ_MAXIMUM]`. Worth contrasting with `net.inet.ip.reass_hashsize`, which was fetched with no check at all and is fixed above: the same shape, one validated and one not. |
| `lib/libc/locale/wcsftime.c:90` `SIZE_T_MAX / MB_CUR_MAX` | `MB_CUR_MAX` is at least 1 for every locale; the division IS the overflow guard. CBMC does not model the locale table. |
| six `xprintf_*.c` `assert(n > 0)` | `n` is the argument-type array length, and `parse_printf_format()` never passes 0. Exported, caller-constrained. |
| `lib/libc/stdio/_flock_stub.c` `fp->_fl_count + 1` | the recursion counter would need 2^31 nested `flockfile()` calls on one `FILE`, each holding a stack frame. |
| `lib/libc/iconv/citrus_mapper.c` "must hold lock upon unlock" | CBMC does not model the tree's rwlock macros. |
| `sys/dev/syscons/scvtb.c:114` `cols * rows` | video-mode dimensions, bounded by the hardware mode table. |
| `sys/dev/ath/ath_hal/ah.c:422` `streams * 4` | `streams` is `HT_RC_2_STREAMS(rc)` = `((rc & 0x78) >> 3) + 1`, so 1..16 by construction and 1..4 in practice. Exported, so rule three cannot see `ath_hal_computetxtime_ht()` three lines up computing it. |
| `sys/dev/dpaa2/dpaa2_swp.c:338` `sd << 5` and the eleven shifts beside it | both call sites (`:235`, `:254`) pass literal 0/1/2/3 for every `int` parameter. Exported, caller-constrained; the parameters would be better typed `uint8_t` like the six above them, which is a readability point and not a defect. |
| `sys/net/if.c:1763`, `sys/net/if.c:1764`, `sys/net/if.c:1765`, and every `fail:` label after an `M_ZERO` allocation | `ifa_alloc()` does `malloc(size, M_IFADDR, M_ZERO | flags)`, so all four counter fields are NULL before any of them is assigned, and its `fail:` path says so — `/* free(NULL) is okay */`. The analyser does not model `M_ZERO`, so every field of a zeroed allocation is an uninitialised value to it. Measured below rather than asserted: 30 of sweep 17's 481 uncited uninitialised-value findings sit in a function that names `M_ZERO`, which is an upper bound and not a count. This row previously cited a *range*, `:1757-1759`, which the citation reader does not match and which was the wrong three lines anyway. |
| `sys/i386/i386/sys_machdep.c:674` `pldt->ldt_base` | `i386_ldt_grow()` sets `mdp->md_ldt = pldt = new_ldt` at `:780` on exactly the path where it was NULL, so every `return (0)` leaves it non-NULL and the caller's re-read cannot be. Same translation unit, so the analyser could see it — this one is path-explosion rather than a boundary. |
| `sys/i386/i386/vm86.c:763,769`, `db_trace.c:130-132,414`, `db_disasm.c:1026,1038` | i386 debugger and vm86 BIOS-call support, reading a trapframe or a page table the caller established. `sys/i386` had 46 of 52 translation units compile for the first time this week; these are the first findings anybody has seen from it. |
| ~18 GEOM classes, "The left operand of `!=` is a garbage value" | one idiom, copied into 34 files: `buf = g_read_data(cp, off, len, &error); if (buf == NULL) return (error);` then a caller that checks `error != 0` before touching `md`. `g_read_data()` (`sys/geom/geom_io.c:878`) returns NULL **exactly when** it has set `*error` — `if (errorc) { g_free(ptr); ptr = NULL; }` is the last thing it does, with no earlier return — so `read_metadata()` cannot return 0 with the struct untouched. `geom_io.c` is a different translation unit, and the analyser is interprocedural *within* one and not *across* one, so it must assume the callee left `*error` alone. This is most of `sys/geom`'s 42 findings and it is one function's contract. The same shape, elsewhere: `sys/kern/sys_pipe.c:640,646` (`vm_map_find_locked()` fills `*addr` on `KERN_SUCCESS`) and `sys/kern/kern_jail.c:663` (`vfs_getopt()` fills `*buf` on 0, and the caller only proceeds when the length is positive). An out-parameter written across a translation-unit boundary is the general case. |
| `sys/dev/ata/ata-all.c:698` `ATA_ATAPI_MASTER << target` | `target` is a two-valued channel index — `ata-ite.c`, the only caller, branches on `target == 0` five lines from the call and shifts by `target << 2` elsewhere. Exported, caller-constrained. |
| `sys/kern/subr_blist.c:216` `blocks - 1` | signed `daddr_t`, so the subtraction overflows only at `DADDR_MIN`. `KASSERT(blocks > 0)` sits directly above — not a check without `INVARIANTS`, but every caller sizes a swap device. |
| `sys/dev/atkbdc/atkbdc.c:396` `q->tail + 1` (six exported functions) | one static queue with a fixed-size array and a `% KBDQ_BUFSIZE` on the next line; the counter would need 2^31 keystrokes between reads. |
| `sys/kern/subr_stats.c:351`, `:363` — 94 `core.NullDereference` | two `ARB_GENERATE_STATIC(...)` lines, 47 findings each. The macro generates an entire array-based red-black tree, and every finding inside the expansion carries the line that expanded it. That is 38% of every `core.NullDereference` under `sys/kern`, `sys/vm`, `sys/net`, `sys/fs` and `sys/ufs` put together, from two lines. `report.py` counts sites as well as findings now, and names any line reported eight or more times. |
| `sys/geom/linux_lvm/g_linux_lvm.c:567` `md.md_vg` | `llvm_textconf_decode()` sets `md->md_vg` on both of its `return (0)` paths and returns `EINVAL` or `-1` otherwise; the caller tests `error != 0` first. The analyser cannot correlate a callee's return value with which of its assignments ran. |
| `sys/net/rtsock.c:997` "stack address escapes" | `update_rtm_from_rc()` writes `&sa_dst.sa` and `&sa_mask.sa` — its own locals — into the caller's `info`. The caller says so: *"any pointer in @info CANNOT BE USED"*, and sets `rti_need_deembed = 0` in the same branch so the `#ifdef INET6` block forty lines down that would dereference them is skipped. Correct, and correct only because of a flag set in one branch guarding a use in another. |
| `sys/kern/kern_prot.c:646` "stack address escapes" | `user_setcred()` leaves `wcred->sc_label` pointing at its local `mac` and `sc_supp_groups` possibly at its local `smallgroups`. Its one caller is `sys_setcred()`, whose `return (user_setcred(td, uap->flags, &wcred));` reads neither again. |
| `sys/vm/vnode_pager.c:1022,1026` `trim * rbehind / sum` | `sum` is 0 only when `rbehind` and `rahead` are both 0, and the enclosing `if` then needs `count > atop(maxphys)`, which `vnode_pager.c:916` asserts against. A KASSERT, so not a check without INVARIANTS — but `count` comes from the VM, not from userland. |
| `sys/kern/kern_timeout.c:1451` `st / count` | `count` is the number of scheduled callouts in the whole callwheel, and `kern.callout_stat` needs a sysctl **write** to run at all. Zero callouts system-wide on a running kernel. |
| `sys/kern/kern_shutdown.c:1459` `length % di->blocksize` | `blocksize` is set by the dump driver at `dumper_insert()` and never validated, but every in-tree dumper passes a real sector size. Same class as the driver attach paths above. |
| `sys/geom/shsec/g_shsec.c:107`, `g_stripe.c:111` | already listed above — `lcm()` is `static` and every caller passes a sector size. |
| `sys/geom/virstor/g_virstor.c:1324` "left operand of `>=` is a garbage value" | `sc->map` is `malloc(..., M_WAITOK)` at `:1218` and filled by `bcopy(mapbuf, &sc->map[n], bs)` at `:1267`, where `mapbuf` came from `g_read_data()` in another translation unit. The `g_read_data()` boundary again, one indirection further out — the analyser cannot see the buffer written, so every field of the map is garbage to it. Reported before and after the divide-by-zero fix in the same function, at the same line, which is how the fix was confirmed not to have introduced it. |
| `sys/geom/raid/md_intel.c:2569` `mmap1->disk_idx[sdi]` | `mmap1` is NULL exactly when `mvol->migr_state == 0` (`:2557`), and the loop 70 lines up (`:2489`, same array, same bound, nothing in between mutates it) sets `migr_state = 1` if **any** subdisk is `REBUILD` or `RESYNC`. This line runs only under that condition, so it cannot be reached with `mmap1` NULL. Worth noting that every other `mmap1` use in the loop carries an explicit `if (mvol->migr_state)` and this one does not — the guard is genuinely absent, it is simply redundant. |
| `sys/dev/videomode/pickmode.c:77` division by zero and `dot_clock * 1000` | measured rather than argued: `videomode_list` is a generated `const` table of **92** entries (`videomode.c`, "THIS FILE AUTOMATICALLY GENERATED"), **none** with a zero `htotal` or `vtotal`, and its largest `dot_clock` is 297000, so `dot_clock * 1000` peaks at 297,000,000 against an `INT_MAX` of 2,147,483,647. The mode arguments are userland's; the divisor never is. |
| `sys/arm64/arm64/cpu_errata.c:59`, `sys/arm64/vmm/vmm.c:230`, `sys/dev/psci/smccc.c:56,58`, `sys/i386/pci/pci_cfgreg.c:177,223`, `sys/kern/posix4_mib.c:139` | stale, not false. All six describe the `CPU_IMPL_MASK` / `SMCCC_FUNC_ID` / `1 << slot` / `p31b_unsetcfg()` defects already fixed above, and the check is that the tree now reads `#define CPU_IMPL_MASK (0xffU << 24)`, `1U << slot`, and so on — a finding describing the unfixed form is from before the fix by definition. **Not** the argument first written here, which was that the sweep's `ksys.jsonl` is timestamped 09:57 and the header fix landed at 10:14: `/tmp/run50` is an *unpacked artifact*, so those mtimes are when the zip was extracted and say nothing about when the data was collected. A triage pass that re-reads a fixed finding as a live one wastes exactly as much time as one that misses a real one — and dating the evidence by the wrong clock is how you get there. |
| `sys/cam/scsi/scsi_enc_ses.c:2762,2792` "undefined value returned" | `ses_set_enc_status()` and `ses_set_elm_status()` return `req.result` from a stack `ses_control_request_t` they never assign. It is assigned by whoever wakes them: `ses_terminate_control_requests()` (`:133`) and the request loop in `ses_encode`'s caller (`:2228`) both set `req->result` **before** `wakeup(req)`, on every path, and `cam_periph_sleep(..., PUSER, ..., 0)` carries no `PCATCH` and no timeout, so there is no early return. The out-parameter class again, filled through a queue and a wakeup rather than a direct call. |
| `sys/cam/cam_xpt.c:5253` `xpt_path_mtx()` | `return (&path->device->device_mtx);` — taking the address of a member, of a field of a caller-supplied `struct cam_path`. A parameter precondition, and not even a dereference at run time. |
| `sys/netinet/tcp_stacks/rack.c:17528`, `:17547` `rack->r_ctl.crte->rate` | guarded on `rack->rack_hdrw_pacing` rather than on `crte != NULL`, which looks like the guard-on-one-of-a-pair shape and is its **opposite**: the two fields are set together at `:17510-17511` and cleared together at every one of the eight sites that clears either — `:14886/14887`, `:17420/17422`, `:17440/17442`, `:17451/17453`, `:17487/17489`, `:17559/17561`, `:17576/17577`, `:23394/23397`, never more than two lines apart. The invariant `rack_hdrw_pacing == 1` implies `crte != NULL` holds by construction; the analyser cannot carry a two-field invariant across four thousand lines. Worth the row as the counter-example: this file maintains its pair everywhere. |
| `sys/netinet/tcp_stacks/rack.c` `udp->uh_sum`, **four** sites | `rack->r_ctl.fsb.udp` is set to a real pointer inside `if (tp->t_port)` and to `NULL` in the `else`, at all four assignment sites (`:13887/13892` for v6, `:13909/13913` for v4). The three fast-output functions load it unconditionally and read it under `if (tp->t_port)`. So `fsb.udp != NULL` iff `t_port != 0`, an invariant held across a function boundary *and* a struct field — the same maintained-pair shape as `crte`/`rack_hdrw_pacing` above, and equally invisible. Two of these four only became reachable to the analyser after `if_hw_tsomaxsegsize` was initialised: an indeterminate value had been cutting the path short. |
| `sys/netinet/tcp_stacks/rack.c` `ip6->ip6_flow`, `ip6->ip6_hlim` | `struct ip6_hdr *ip6 = NULL;` at `:18913`, assigned at `:18916` under `if (rack->r_is_v6)`, read at `:19071` and `:19157` under the same `if (rack->r_is_v6)` in the same function. `r_is_v6` is a bitfield nothing between the two touches. |
| `lib/msun/ld128/s_cospil.c:76`, `s_sinpil.c:82`, `s_tanpil.c:100` | all three are `FFLOORL128()` (`math_private.h:825`), which shifts by `e + 1` where `e = u.bits.exp - 16383`, and by `e - 48` in its other arm. Each caller reaches the macro only past an early-return block — `if (ax <= 1)` in `cospil`, `if (ax < 1)` in the other two — so `ax >= 1`, `exp >= 16383`, `e >= 0`, and the first shift is 1..48; the enclosing `if (ax < 0x1p112)` puts the second at 0..63. Checked in all three rather than one, because a set of three where one lacks the guard is this document's most common finding — here all three have it, by two slightly different comparisons. CBMC sees an exported function taking an unconstrained `long double` and does not carry a floating-point comparison into the exponent field. |
| `lib/msun/src/e_sqrt.c:127` `ix1 >> (32 - i)` | fdlibm's subnormal path. `i` comes from `for (i = 0; (ix0 & 0x00100000) == 0; i++) ix0 <<= 1;`, entered only after `while (ix0 == 0)` has guaranteed `ix0 != 0`, so the loop terminates with `i` bounded by the position of bit 20 and `32 - i` stays in range. A loop-exit invariant, which is the shape CBMC is least able to establish and the one `--unwind` bounds rather than proves. |
| `lib/libcrypt/misc.c:42` `while (--n >= 0)` | `--n` is undefined only at `INT_MIN`. `_crypt_to64(s, v, n)` is called with small literal group counts by every crypt backend in the tree. Exported, so rule three. |
| `lib/libcalendar/easter.c:46` `29 / (i + 1)` | `i` is `(...) % 30`, and C's `%` yields a negative result for a negative dividend, so `i == -1` divides by zero. Reaching it needs a **negative year**: for `y >= 0` the numerator is `c - c/4 - (c-k)/3 + 19n + 15` with every term non-negative or small, and it stays positive. `easterg(int y, date *dt)` is libcalendar's public entry point and its domain is a calendar year. Rule three — worth the row because the failure is real arithmetic rather than a modelling artefact, and a caller that passes a computed year should know. |
| `sys/powerpc/ofw/ofw_real.c` — 19 findings, and `rtas.c:252` | one idiom. The OUT cells are written either by `ofw_real_unmap()`'s `memcpy(buf, ...)` — in this translation unit, but past two early returns — or, before the MMU is up, by the firmware writing straight into `args` at its own physical address, because `ofw_real_map()` returns `(uintptr_t)buf & ~DMAP_BASE_ADDRESS` when `!pmap_bootstrapped`. Which of the two happens depends on `pmap_bootstrapped` and on `of_bounce_virt`, and the second is a store the analyser cannot see at all. The one finding in this file that was **not** this — `ofw_real_open()` reading `args.instance` before the copy-back — is fixed above; the point of the row is that eighteen identical-looking findings hid one real one. |
| `sys/dev/mpr/mpr_config.c` (19) and `sys/dev/mps/mps_config.c` (7) | `error = mpr_wait_command(sc, &cm, 60, CAN_SLEEP); if (cm != NULL) reply = ...; if (error || (reply == NULL))` — so the analyser explores `error == 0` with `cm == NULL`, which makes `reply` indeterminate at the `==` and `cm` NULL at the later `cm->cm_length`. That state does not exist: `mpr_wait_command()` (`mpr.c`, and `mps.c` identically) writes `*cmp = NULL` **only** inside `if (error == EWOULDBLOCK)`, and the next statement is `error = ETIMEDOUT`. It cannot return 0 having freed the command, so `error == 0` implies `cm != NULL` and the guard binds. A cross-translation-unit out-parameter contract again. Worth the row for what is true underneath it: `mps_config.c` declares `reply = NULL` in **9 of 9** functions and `mpr_config.c`, which was copied from it, in **2 of 12** — so the newer driver's correctness rests entirely on that invariant while the older one does not need it. Nothing to fix, and not nothing to know. |
| `kern_condvar.c:162`, `kern_condvar.c:284`, `kern_condvar.c:352`, `kern_condvar.c:423`, `kern_synch.c:234`, `kern_exit.c:1576`, `uipc_sockbuf.c:747` — "2nd function call argument is an uninitialized value" | one idiom, seven times. `WITNESS_SAVE_DECL(lock_witness)` declares it, `WITNESS_SAVE(lock, lock_witness)` fills it inside `if (lock != &Giant.lock_object)`, and `WITNESS_RESTORE(lock, lock_witness)` reads it inside the same test with `lock` a parameter nothing between them touches. (`lock_state`, the argument it looks like at a glance, is initialised to 0 unconditionally.) |
| `sys_generic.c:1354,1355` — `copyout(obits[x], ...)` | `select()`'s `getbits` macro sets `ibits[x] = obits[x] = NULL` when the user passed a NULL fd_set and to real storage otherwise; `putbits` reads `obits[x]` only under `if (name && ...)`. The same `name` on both sides. |
| `tty_info.c:342` `thread_lock(td)` | `td = NULL` then `FOREACH_THREAD_IN_PROC(p, tdpick) if (thread_compare(td, tdpick)) td = tdpick;` — and `thread_compare()` opens with `if (td == NULL) return (1);`, so the first thread always takes it, and a process always has one. |

| `sys/fs/msdosfs/msdosfs_lookup.c`, **seven** `*vpp` dereferences | `msdosfs_lookup_ino()` has two modes and its callers pick one: six sites pass `vpp == NULL` with a non-NULL `scnp` (find the directory entry, produce no vnode), and `if (scnp != NULL) { *scnp = cluster; *blkoffp = blkoff; return (0); }` at `:488` returns before every unguarded `*vpp` write. The one write that IS guarded, `if (vpp != NULL) *vpp = NULL;` at `:178`, is the only one that happens *before* that return — which is why it needed the guard and why its lonely presence made the other eight look like omissions. Read the whole function before believing the count. |
| `sys/fs/msdosfs/msdosfs_fat.c`, five `bp->b_data` | `pcbmap()` walks the FAT with `struct buf *bp = NULL; u_long bp_bn = -1;` and reads through `bp` after `if (bn != bp_bn) { ...bread(&bp)... }`. The analyser's path needs `bn == bp_bn` on the first iteration, i.e. `bn == -1`, which `fatblock()` does not produce. Worth one note all the same: `bp_bn` is `u_long` and `bn` is a signed `daddr_t`, so the sentinel is compared across a signedness boundary. It is not a defect today and it is the kind of thing that becomes one. |
| `sys/dev/usb/serial/usb_serial.c`, **eight** "called function pointer is null" | `ucom_cfg_line_state()` builds `mask` from exactly which of the four optional callbacks are non-NULL, and then computes `notch_bits` and `any_bits` as `... & mask`. So `notch_bits & UCOM_LS_DTR` is non-zero only when `ucom_cfg_set_dtr` is not NULL. A maintained pair carried through a bitmask, which is further than a path-sensitive analysis will follow it. All eight are that one idiom, and it is sound. |
| `sys/dev/md/md.c`, eight `dst` | `mdstart_malloc()` sets `dst = NULL` in the `notmapped` arm and in the `vlist != NULL` arm, and `dst = bp->bio_data` only in the final `else` — then every use is inside a matching three-way `if (notmapped) ... else if (vlist != NULL) ... else`. Exactly complementary, three ways, and correct. |
| `sys/netinet/tcp_syncache.c` `syncache_respond()`, **seven** findings | `ip`, `ip6`, `udp` and `ulen` are one maintained set. `udp` and `ulen` are assigned together on exactly the branches where `sc->sc_port != 0` -- written `if (sc->sc_port != 0) {...}` in the v6 arm and `if (sc->sc_port == 0) {...} else {...}` in the v4 arm, inverted but the same condition -- and read only under `if (udp)` and `if (sc->sc_port)`. `ip` and `ip6` are the `INC_ISIPV6` pair. All four arms maintain it, checked one at a time because a set of four where one differs is this document's most common finding. Same class as `rack.c`'s `udp`/`t_port` and `ip6`/`r_is_v6`, in a function that answers unauthenticated SYNs. |
| `lib/libcalendar/calendar.c` `jdate`, `ndaysg`, `ndaysj`, `weekday`, and `easter.c` `easterog`/`easteroj:100` | the same domain argument: signed arithmetic on a year, day or day-number that overflows only for inputs no calendar produces. Exported, caller-constrained. |
| `libexec/phttpget/phttpget.c:471` `isdigit(hln[7])` | the path needs `readln()`'s `while (strnstr(resbuf + *resbufpos, "\r\n", *resbuflen - *resbufpos) == NULL)` to be **false on its first evaluation**, with `resbuflen` and `resbufpos` both 0 — that is `strnstr(s, "\r\n", 0)` returning non-NULL, which libc's `strnstr` cannot do (`if (slen-- < 1) return (NULL)` before any read). `strnstr` is in another translation unit, so the analyser models it as returning an arbitrary pointer into the buffer and then reads `malloc`'d bytes through it. In the real program `readln()` returns 0 only with a `\r\n` in range, `*eolp = '\0'` terminates the line, and `hln[7]` is reached only after `strncmp(hln, "HTTP/1.", 7) == 0` has proved seven non-NUL bytes precede it. Worth the row because `phttpget` is what `freebsd-update` runs as root against a network peer. |
| `lib/libc/gen/arc4random.c:211`, `:231` "null passed to memcpy" | `keystream = rsx->rs_buf + ...` where `rsx` is a global the analyser can see NULL. `rs` and `rsx` are one maintained pair: `_rs_allocate()` sets both or returns -1, and `_rs_stir()` — reached from `_rs_stir_if_needed()` on every path where `rs` is NULL — either allocates both or calls `abort()`. So `rs != NULL` implies `rsx != NULL` for the life of the process. The maintained-pair shape again, this time across two file-scope pointers in libc's CSPRNG. |
| `lib/libc/db/hash/hash.c:853`, `:922`, `:932` `unix.MallocSizeof` | `SEGMENT` is `BUFHEAD **`, so `sizeof(SEGMENT)` and `sizeof(BUFHEAD *)` agree on every supported target and the allocation is the right size; the spelling is one indirection off. Same as `radixsort.c:109` above. |
| `sys/contrib/ncsw/Peripherals/FM/Pcd/fm_cc.c:77` "1st function call argument is an uninitialized value" | `CcRootReleaseLock()` reads `p_FmPcdCcTree->p_Lock`, and the tree is `memset(p_FmPcdCcTree, 0, sizeof(t_FmPcdCcTree))` at its one allocation site (`:6026`), so the field is NULL and never garbage. The helper takes a `t_Handle` — an opaque `void *` — from API entry points, so analysed in isolation every field behind it is unknown. The last finding standing in the DPAA ethernet after the twenty-three above it were fixed. |
| `sys/contrib/zstd/lib/common/bitstream.h:173` "right operand of `&` is a garbage value due to array index out of bounds" | `BIT_getLowerBits()` does `bitContainer & BIT_mask[nbBits]` under `assert(nbBits < BIT_MASK_SIZE)`, and `debug.h:70` makes `assert` `((void)0)` at DEBUGLEVEL 0 — so the bound is upstream's contract with its callers, not a run-time check. The reporting caller, `ZSTD_encodeSequences_body()`, passes `LL_bits[]`, `ML_bits[]` and `ofCodeTable[]` entries; an offset code is `ZSTD_highbit32(offBase)`, so 0..31 against a `BIT_MASK_SIZE` of 32. The analyser cannot bound the contents of a byte table filled by the match finder. Worth noting which side this is: compression **encoding**, where the codes are zstd's own, not decoding, where they come off the medium. |
| `sys/cddl/dev/dtrace/x86/dis_tables.c`, **64** "uninitialized value" | `dtrace_disx86()` declares `uint_t mode = 0; uint_t reg; uint_t r_m;` — one of three initialised — and `dtrace_get_modrm()` writes all three **only** when `x->d86_got_modrm == 0`, returning without touching them otherwise. So every `STANDARD_MODRM`/`MIXED_MM` use of `reg` and `r_m` rests on the invariant *`d86_got_modrm == 1` implies this invocation already wrote them*, maintained at four sites: cleared on entry (`:3736`, `:4300`), set inside `dtrace_get_modrm()` itself, which writes (`:3038`), and set by hand at `:5175`/`:5183` under `if (vex_prefetch)` — where both of the two places that set `vex_prefetch` write `reg`, `reg` and `mode` first (`:3824` calls `dtrace_get_modrm`, `:3861-3863` assigns the three by hand). All four checked, one at a time. It holds. The analyser cannot carry a struct field's meaning across fifteen hundred lines, and the file is the x86 decoder `fbt` and `kinst` use to find instruction boundaries in live kernel text, so the four sites were worth reading rather than assuming. Two of the three declarations still lack the initialiser their sibling has. |
| `sys/cddl/dev/dtrace/x86/dis_tables.c:6419`, `:6447` — `dp->it_stackop`, `wbit ==` | latent, and not the same as the row above. `dtrace_disx86()` has exactly **one** `goto done`, at `:3763`, taken when `opcode1 == 0 && opcode2 == 0 && x->d86_check_func(x->d86_data)` — the caller's "this is data, not an instruction" hook. It is the first thing after the opcode bytes are read, before `dp` (`:3632`, `const instable_t *dp;`) and `wbit` (`:3644`) are assigned by the decode switch. `done:` then runs the `#ifdef DIS_MEM` block — and `DIS_MEM` **is** defined, by `sys/modules/dtrace/dtrace/Makefile:53` — whose first test is `x->d86_memsize != 0`, false because `:3737` zeroed it, and whose second **dereferences `dp`**. Unreachable today only because both in-tree callers set `d86_check_func = NULL` (`kinst_isa.c:290`, `instr_size.c:110`), so the `&&` short-circuits. The field exists to be set. Same treatment as `rtw88/bf.h` above and for the same reason: vendored code, an unreachable path, and a note is worth more than a change nobody can test. |
| `lib/libc/db/hash/hash.c:940` `store` leak | `alloc_segs()` writes `hashp->dir[i] = &store[i << hashp->SSHIFT]`, so `dir[0] == store`, and `hdestroy()` frees it as `free(*hashp->dir); /* Free initial segments */`. The analyser loses a pointer that escapes only through a computed address. |

## Sweep 9's newly readable files: nineteen findings, no defects

The module index learning `${MACHINE_CPUARCH}` and a module's own include
flags going ahead of `-I$S` made another nineteen findings visible across
`sys/cddl` and `sys/contrib`. All nineteen are read; none is a defect, and
they fall into four shapes, two of which this document already has names
for.

| site | why it is not a defect |
|---|---|
| `dtrace.c:8232`, `:8236`, `:8245` — `bucket->dthb_next`, `->dthb_chain` | `dtrace_hash_remove()` walks `hash->dth_tab[ndx]` for the bucket holding the probe and then states the result: `ASSERT(bucket != NULL)`. Without `DEBUG` that is `((void)0)`, so the analyser takes the exit-with-NULL path. It does not exist for a caller: `dtrace_hash_remove()` is only reached for a probe already in the hash — `dtrace_probe_remove()` and `dtrace_probe_provide()`'s failure unwind both take it from the table they just found it in. The compiled-out-`ASSERT` shape, the same one `dis_tables.c` produces sixty-four times. |
| `dtrace.c:10733` (`np`), `:10855` (`svarp`), `fasttrap.c:990` (`idp`) | `switch (v->dtdv_scope) { case THREAD: ... case LOCAL: ... case GLOBAL: ... default: ASSERT(0); }`, and the pointer the switch sets is read after it. `dtrace_difo_validate()` rejects any DIFO with a fourth scope at `:10125-10127` — `v->dtdv_scope != GLOBAL && != THREAD && != LOCAL` is an error — and it runs before `dtrace_difo_init()`, so the `default:` arm is unreachable by construction. `fasttrap.c:990` is the same over `tp->ftt_type`, which `fasttrap_tracepoint_init()` sets from the four `DTFTP_*` the switch names. |
| `fasttrap.c:1593` — `fprc->ftpc_next` | `fasttrap_proc_release()` walks its own hash bucket for the proc it was handed, then `ASSERT(fprc != NULL)` with the comment "Something strange has happened if we can't find the proc." Compiled out; the caller holds a reference to a proc that is in that bucket. |
| `fbt.c:792` — `ctf_list_append()` "potential memory leak" | `ctf_decl_push()` allocates `cdp` and hands it to `ctf_list_append(&cd->cd_nodes[prec], cdp)`, whose parameter is `void *new` and which stores it through a `ctf_list_t *` cast. The allocation escapes into the list; the checker loses it at the cast and reports the leak at the store. `ctf_decl_fini()` frees the list. The same escape-through-a-computed-address shape as `hash.c:940` above. |
| `sys/contrib/alpine-hal/al_hal_serdes_25g.c` — **eight** findings at `:504`, `:536`, `:938`, `:955`, `:1161`, `:1378`, `:1643` | one error path, ignored eight times. `al_serdes_25g_reg_read()` returns `-1` without writing `*data` for a `page` its switch does not name, and `al_serdes_25g_reg_masked_read()` propagates that. Every caller here ignores the return and reads the out-parameter — so `al_serdes_25g_cdr_is_locked()` would answer from a stack byte. It cannot happen in this tree: the only pages the switch omits are `AL_SRDS_REG_PAGE_2_LANE_2` and `_3_LANE_3`, which `al_hal_serdes_interface.h:69` marks "Relevant to Serdes hssp only" — the 25G SerDes has two lanes, and the callers cast `(enum al_serdes_reg_page)lane` from a lane the hardware has. Vendored HAL, an unreachable path, and no in-tree caller of the four affected entry points at all: recorded rather than changed, like `rtw88/bf.h` and `dis_tables.c`'s `done:`. Worth the row because a fifth lane would make all eight live at once. |
| `sys/contrib/dev/iwlwifi/mvm/sta.c:2973` — `baid_data->baid` | `baid_data` is allocated under `if (iwl_mvm_has_new_rx_api(mvm) && start)` and dereferenced under `if (start)` — three statements after `if (!iwl_mvm_has_new_rx_api(mvm)) return 0;`. The same predicate on both sides, tested twice because the allocation has to happen before the firmware command. `iwl_mvm_has_new_rx_api()` reads a capability bit fixed at device setup; the analyser cannot assume a function returns the same answer twice. |
| `sys/contrib/dev/rtw89/core.c:5286` — `highest[nss - 1]` | `nss` is `hal->rx_nss`, and `mac.c:3139` assigns it `rx_nss ? min_t(u8, rx_nss, chip->rx_nss) : chip->rx_nss` — the firmware's PHYCAP answer clamped by the chip table, and the chip table when the answer is zero. So `nss == 0` requires `chip->rx_nss == 0`, a compile-time constant that is 1 or 2 for every supported chip. `highest[-1]` needs a chip definition that does not exist. Not the same as the beacon-field divisor fixed above: that one took its divisor off the air. |

## Fixed — `ed(1)` calls `strlen()` on a function that returns NULL

The sweep has never covered `bin/`, `sbin/`, `usr.bin/` or `usr.sbin/` —
1,849 translation units of the userland that ships. A probe over `bin/`
alone: 111 sources, 77 of which compile with the flags as they stand, and
nine findings.

One is a defect. `bin/ed/main.c:1301`:

```c
char *
strip_escapes(char *s)
{
        static char *file = NULL;
        static int filesz = 0;
        int i = 0;

        REALLOC(file, filesz, PATH_MAX, NULL);
        ...
        return file;
}
```

`REALLOC` (`ed.h:113`) is a macro that **returns the caller's `err`
argument** when the allocation fails, and this caller passes `NULL`. So
`strip_escapes()` has a NULL return path — its comment does not mention
one — and all four call sites used the result directly:

| site | what happens |
|---|---|
| `main.c:995` `strlen(s = strip_escapes(old_filename))` | SIGSEGV |
| `main.c:519` `printf("%s\n", strip_escapes(...))` | undefined; FreeBSD's `vfprintf` prints `(null)` |
| `io.c:39` `fopen(strip_escapes(fn), "r")` | `EFAULT`, so ed reports "Bad address" for an out-of-memory |
| `io.c:146` `fopen(strip_escapes(fn), mode)` | the same |

Only the first crashes; the other three survive by accident, and two of
them print the wrong reason. All four check now, each returning `ERR` —
which is what the rest of the file does after a failed `REALLOC`, and
which leaves the `"out of memory"` `errmsg` the macro has already set.
Fixed on all four rather than on the one that crashes, for the reason
this document keeps giving.

`bin/ed`'s findings went 6 to 5. The three that remain are the
`buf`/`bufsz` and `ocmd`/`ocmdsz` maintained pairs: `REALLOC` assigns only
when `i > n`, so "a NULL buffer with a non-zero recorded size" is a state
the analyser explores and the program cannot reach.

### The rest of the `bin/` nine, and what a 21%-compiled corpus is worth

| site | why it is not a defect |
|---|---|
| `bin/ed/glbl.c:118`, `main.c:959`, `:1076` | the `REALLOC` maintained pair above, three more times |
| `bin/ed/glbl.c:186` `lp->q_forw` | `unset_active_nodes()` walks a circular list from `np` to `mp`, both caller-supplied |
| `bin/pax/tables.c:1261` `key += val` | `u_int val;` is filled byte-wise through `dest = (char *)&val` in a loop that runs exactly `sizeof(u_int)` times; the analyser does not track initialisation through a `char *` alias |
| `bin/pax/options.c:895` `incfiles` leak | `tar_options()` walks the pointer forward with `incfiles++` as it consumes entries, losing the base. Real, and it is an option parser in a program that then does its work and exits |
| `bin/pwait/pwait.c:64` `rbe_link` | the red-black tree macros again |

`sbin/` was probed the same way and is not yet worth triaging: **94 of
439** translation units compile, so its 35 findings are a sample of a
fifth of the corpus. The 345 that fail want the tree's own library
headers — `kvm.h`, `vis.h`, `libxo/xo.h`, `histedit.h`, `netinet/ip_compat.h`
— which live in `lib/libkvm`, `lib/libnetbsd`, `contrib/libxo`,
`contrib/libedit` and `sbin/ipf`. Chasing findings before that is the
mistake this document opens with, one directory over: an ERROR is not a
finding, and a fifth of a corpus is not the corpus.

## The severe checkers, swept for: five use-after-frees and four escapes,
 none of them real

`core.StackAddressEscape` and `unix.Malloc`'s "use of memory after it is
freed" are the two shapes worth interrupting other work for, so sweep 9's
1,750 findings were sorted by checker and every instance of both was
traced. There are nine, at six sites, and all nine are the analyser
failing to model a linked-list macro or a lifetime the caller documents.

| site | why it is not a defect |
|---|---|
| `lib/libc/db/hash/hash_buf.c:325` "use after free" | `__buf_free()` does `BUF_REMOVE(bp); free(bp); bp = LRU;`. `BUF_REMOVE` (`:70`) is `(B)->prev->next = (B)->next; (B)->next->prev = (B)->prev;` and `LRU` (`:84`) is `hashp->bufhead.prev`, so the re-read gets the next buffer, not the freed one. Reported because the checker does not follow the unlink through two pointer writes. |
| `lib/libc/resolv/res_findzonecut.c:642`, `:651`, `res_update.c:195` | `free_nsrrset()` is `while ((nsrr = HEAD(*nsrrsp)) != NULL) free_nsrr(nsrrsp, nsrr);` and `free_nsrr()` ends `UNLINK(*nsrrsp, nsrr, link); free(nsrr);`. Same shape as above, in BIND's list macros rather than hash's, in the DNS resolver. The `UNLINK` is before the `free` at every one of the three sites. |
| `lib/libc/tests/gen/fmtmsg_test.c:206` | a test, and the same list shape. |
| `sys/kern/kern_prot.c:646`, twice | `user_setcred()` stores `&mac` and possibly `smallgroups` — both its own locals — into the caller's `struct setcred *wcred`. Its only two callers, `sys_setcred()` (`:668`) and freebsd32's (`freebsd32_misc.c:4235`), each declare `struct setcred wcred;` on their own frame and `return (user_setcred(td, uap->flags, &wcred));` immediately. The pointers dangle into an object that dies in the same statement. |
| `sys/net/rtsock.c:997`, twice | `update_rtm_from_rc()` points `info->rti_info[RTAX_DST]` and `[RTAX_NETMASK]` at its locals `sa_dst` and `sa_mask`. The caller says so itself, at `:1216`: *"Note that some sockaddr pointers may have changed to point to memory outsize @rtm. Some may be pointing to the on-stack variables. Given that, any pointer in @info CANNOT BE USED."* — and it does not use them. |

### And the eleven undefined array subscripts

`core.uninitialized.ArraySubscript` is the third shape worth reading
whole, because an indeterminate index is a memory-safety question rather
than a correctness one. The three traced furthest:

| site | why it is not a defect |
|---|---|
| `sys/netpfil/pf/pf_lb.c:1026` — `ctx->nk->port[idx]` | `int idx;` is set by a `switch (nat_action)` with arms for `PF_NAT`, `PF_BINAT` and `PF_RDR` **and no `default:`**, then used to subscript a two-element array in the packet path. It holds on an invariant spread over two files: `pf_ioctl.c:2295` admits a rule only if `pf_get_ruleset_number(rule->action)` is below `PF_RULESET_MAX`, and that function maps exactly `PF_NAT`/`PF_NONAT` to the NAT ruleset, `PF_BINAT`/`PF_NOBINAT` to BINAT and `PF_RDR`/`PF_NORDR` to RDR; `pf_get_translation()` (`pf_lb.c:979`) then returns early for the three `PF_NO*`. So the three arms are the three reachable values. The other caller, `pf.c:5628`, passes a local set to `PF_NAT` or `PF_RDR`. Sound, and thin: an uninitialised index into the firewall's fast path, guarded by an admission check in another file and no assertion. |
| `sys/dev/evdev/evdev_mt.c:267` — `r2c[row]` | `row` is assigned inside `if (d < delta \|\| ...)` in a loop the analyser thinks may not take. `delta` starts at `INT_MAX` and the function's own comment states its preconditions — `m >= n`, and inputs in `0 .. INT_MAX / 2` — so the first iteration's `d` is below `INT_MAX` and assigns `row`. With `n == 0` the outer loop never runs and `row` is never read. |
| `sys/netgraph/ng_pptpgre.c:645` — `gre->data[gre->hasSeq]` | `hasSeq` is a bitfield in the GRE header being built, and `be32enc(gre, PPTP_INIT_VALUE)` at `:626` writes the whole flag word before anything reads it. The checker does not carry a bitfield's value through a four-byte store to the struct's first word. |

## `device vchiq` is declared where its sources are not, and listed where
 nothing declares it

Sweep 10's `sys/contrib` shard put twenty-nine ERROR translation units
outside the inventory, and two of them are the Raspberry Pi VCHIQ driver
failing under every architecture that was tried. Reading why gave a
build-system defect of the `mac_grantbylabel` family, in both directions
at once.

| | |
|---|---|
| `sys/conf/files.arm64` | lists all **20** VCHIQ sources, `optional vchiq soc_brcm_bcm2837 fdt` |
| any `sys/arm64/conf/*` | declares `device vchiq` **nowhere** — not `std.broadcom`, not `NOTES` |
| `sys/arm/conf/GENERIC:228` | `device vchiq` |
| `sys/conf/files.arm` | names **no** VCHIQ source |
| `sys/conf/files` (MI) | names none either |
| `sys/modules/vchiq` | does not exist |

So on 32-bit ARM the device is declared and no source is listed —
`config GENERIC` accepts the line and compiles nothing — and on arm64 the
sources are listed and no configuration asks for them. Twenty files that
no kernel in this tree can reach.

And they could not be built on the architecture that lists them anyway:
`vchiq_kmod.c:50` includes `<machine/fdt.h>` unconditionally, and that
header exists for amd64, arm, i386 and x86 and **not** for arm64.

Recorded rather than fixed, for the same reason `mac_grantbylabel` was:
making it build is a decision — move the twenty entries to
`sys/conf/files.arm`, or teach arm64 a `machine/fdt.h`, or drop `device
vchiq` from arm's GENERIC — and each of those says something different
about which hardware this tree intends to support. The inventory entry
says `VCHIQ_UNREACHABLE` and `test_expected_errors.py` checks all six
facts above, so the day any one of them changes the exemption fails
rather than quietly absorbing a driver that has started building.

## Sweep 10's `sys/dev`: 26 files that could not be read now can, and the
 two findings they carried

The `sys/dev` shard came back 2,563 OK / 70 ERROR against sweep 9's
2,537 / 96: twenty-six ERROR → OK, none the other way. Two findings are
new, and both are in files that had never compiled — which is the whole
point of chasing ERRORs, and also the reason to read new findings
carefully rather than count them.

Neither is a defect. One is the analyser's memory model and one is a
guard that means nothing.

### `sys/dev/bwn/if_bwn.c:5317` — a punned store leaves three of four bytes undefined

    uint8_t noise[4];
    *((uint32_t *)noise) = htole32(bwn_jssi_read(mac));
    if (noise[0] == 0x7f || noise[1] == 0x7f || noise[2] == 0x7f ||
        noise[3] == 0x7f)

`core.UndefinedBinaryOperatorResult`: *the left operand of `==` is a
garbage value* — on `noise[1]`, not `noise[0]`. The analyser binds a
store made through a wider type to the region's **first** element and
leaves the rest of the array undefined. Nine lines reproduce it with no
driver, no headers and no configuration:

    uint8_t noise[4];
    *((uint32_t *)noise) = __builtin_bswap32(src());
    if (noise[0] == 0x7f || noise[1] == 0x7f)   /* noise[1] is garbage */

so it is a property of the instrument, not of the code under it. The
pattern appears 71 times under `sys/`, so this class will recur and is
worth recognising on sight.

Passing `-fno-strict-aliasing` — which the kernel really is compiled
with, `sys/conf/kern.pre.mk:67-68` adds it whenever `COPTFLAGS` carries
`-O2`, and the default `COPTFLAGS?=-O2 -pipe` does — does **not** change
the result: checked on the reproducer, same warning with and without.
The analyser's store model is not the aliasing rules, so the flag would
be added for faithfulness and would silence nothing. It is left off, and
recorded here so the next person does not repeat the experiment.

### `sys/dev/sound/pcm/feeder_volume.c:279` — a NULL check after the dereference

`core.NullDereference` on `muted[j]`, reached from

    vol   = c->volume[SND_VOL_C_VAL(info->volume_class)];   /* :250 */
    muted = c->muted[SND_VOL_C_VAL(info->volume_class)];    /* :251 */
    ...
    d = (c != NULL) ? c->parentsnddev : NULL;               /* :272 */

The checker is reading the code's own claim: a function that tests `c !=
NULL` at `:272` is a function whose author thought `c` could be NULL,
and if it could, `:250` dereferenced it twenty-two lines earlier without
looking.

Only one of those two readings can be right, and it is not the guard's.
The two lines at `:272-273` are verbatim `chn_syncstate()`
(`sys/dev/sound/pcm/channel.c:2129-2131`), where the channel genuinely
can be NULL and the function returns if it is. Copied into
`feed_volume_feed()`, which is reached only through a channel's own
feeder chain, they guard nothing that `:250` has not already settled.

Left alone. Deleting a vestigial NULL test from upstream is churn with
no behaviour behind it, and the finding is now on the record with the
reason it is not a bug — which is what the next sweep needs.

## `libexec/ypxfr`: two `strlen(NULL)` that the two calls rule out

Generating the rpcsvc headers took eleven of the twelve translation
units under the six RPC `libexec` directories from ERROR to OK, and
`ypxfr_main.c` came with two `unix.cstring.NullArg` findings at `:448`
and `:458`. Both are infeasible, for different reasons, and both are
worth writing down because the second one is not obvious.

`:448`, `strlen(ypxfr_master)`. `ypxfr_master` is `strdup()`'d at `:318`
without a NULL check — but `:325` is `if (ypxfr_master == NULL)`, and
the `ypxfr_get_master()` it then calls **is** checked, with
`ypxfr_exit()` on failure. The unchecked `strdup` is caught by the test
that follows it.

`:458`, `strlen(ypxfr_dest_domain)`. This one turns on libc:

    if (!yp_get_default_domain(&ypxfr_local_domain) &&    /* :266 */
        _yp_check(&ypxfr_local_domain))
            ypxfr_use_yplib = 1;
    ...
    if (ypxfr_dest_domain == NULL) {
            if (ypxfr_use_yplib)
                    yp_get_default_domain(&ypxfr_dest_domain);   /* :277 */

The second call's return value is ignored, and on failure
`yp_get_default_domain()` sets `*domp = NULL` (`lib/libc/yp/yplib.c:747`)
— so on the face of it `ypxfr_dest_domain` can be NULL at `:458`,
`:478` and `:528`. It cannot, and the reason is not in this file:
`yp_get_default_domain_locked()` only calls `getdomainname()` when its
static `_yp_domain` is still empty, and `:277` is reached only when
`:266` already succeeded and filled it. The second call cannot fail
after the first succeeded.

That invariant lives in libc, is undocumented, and is the only thing
between this code and a NULL dereference. Recorded rather than
"fixed": adding a check would be reasonable defensive practice and
would also assert something about libc's caching that no comment
anywhere claims.

## `libexec/atrun`: `perr()` and `perrx()` never return, and two of three
 declarations did not say so

Reading a program's real `CFLAGS` out of bmake — rather than walking its
Makefile chain, which cannot follow `libexec/atrun/Makefile`'s
`.include "${SRCTOP}/usr.bin/at/Makefile.inc"` — took `atrun.c` from
"`use of undeclared identifier 'DAEMON_GID'`" to compiling for the first
time. It came with one finding:

    atrun.c:176  [core.NullDereference]
        Access to field 'pw_name' results in a dereference of a null
        pointer (loaded from variable 'pentry')

and the caller looks like it has already handled that:

    pentry = getpwuid(uid);
    if (pentry == NULL)
        perrx("Userid %lu not found - aborting job %s", ...);

    pam_err = pam_start(atrun, pentry->pw_name, &pamc, &pamh);  /* :176 */

`perrx()` ends in `exit(EXIT_FAILURE)` and never comes back — but nothing
in the file's declarations says so:

    void perr(const char *fmt, ...);            /* atrun.c:92  */
    void perrx(const char *fmt, ...);           /* atrun.c:93  */
    static void usage(void) __dead2;            /* atrun.c:94  */

One of the three carries the attribute, and it is the one that needed it
least. The same one-of-N shape as the mlx5 `SRCS` (6 of 7), the arm64
atomics (3 of 9) and `ed`'s `strip_escapes()` callers (1 of 4) — and
here the idiom is on the very next line. The program's other translation
unit repeats the omission (`gloadavg.c:44`), while upstream's own
`usr.bin/at/panic.h:32` has declared `void perr(const char *a) __dead2;`
all along.

Fixed: `__dead2` on both declarations in `atrun.c` and on `gloadavg.c`'s.
The finding goes away — checked, not assumed: `libexec/atrun` reports 1
finding before and 0 after, with both files still OK. `perrx` is
newly-visible only in the sense that nothing had ever compiled this file;
the missing attribute has been in the tree the whole time, costing a
compiler that cannot see unreachable code after either call.

`check_pbsd_marks.py` carries both, and both were checked by reverting
them: dropping `perrx`'s attribute reports "found 1 time(s), needs 2",
dropping `gloadavg.c`'s reports "found 0 time(s), needs 1".

## Sweep 10, whole: 8,109 translation units, 67 recovered, no regressions

| | sweep 9 | sweep 10 |
|---|---|---|
| OK | 7,348 | **7,415** |
| ERROR | 760 | **693** |
| findings | 1,612 | 1,622 |

`ERROR -> OK` 67, `OK -> ERROR` **0**, 11 findings new and 1 gone. Every
one of the eleven is in a file that had never compiled before, which is
the point of chasing ERRORs and also the reason to read each one rather
than count it. Two are written up above (`if_bwn.c`,
`feeder_volume.c`). The other nine:

### `sys/gnu/dev/bwn/phy_n/if_bwn_phy_n_core.c` — six, all one invariant

Five `core.NullDereference` (`:771`, `:1147`, `:1487`, `:4442`, `:4444`)
are the same shape. `bwn_radio_2057_chantab_upload()` takes two table
pointers and tests one of them:

    if (e_r7_2g) {
        ... e_r7_2g->radio_vcocal_countval0 ...
    } else {
        ... e_r7->radio_vcocal_countval0 ...      /* :771 */
    }

The `else` reaches `e_r7` with no test of its own. Both pointers start
NULL two calls up, in `bwn_nphy_set_channel()`:

    const struct bwn_nphy_chantabent_rev7    *tabent_r7 = NULL;
    const struct bwn_nphy_chantabent_rev7_2g *tabent_r7_2g = NULL;
    ...
    r2057_get_chantabent_rev7(mac, freq, &tabent_r7, &tabent_r7_2g);
    if (!tabent_r7 && !tabent_r7_2g)
            return -ESRCH;

so exactly-one-or-both is guaranteed — by a caller, two frames away,
with no assertion anywhere in between. Infeasible, and thin in the same
way `pf_lb.c:1026` is thin.

`:5497` is `core.uninitialized.Assign` and is **not** infeasible:

    uint16_t coef[4];                    /* :5473, never written here */
    ...
    for (i = 0; i < 4; i++) {
            if (mac->mac_phy.rev >= 3)
                    table[i] = coef[i];  /* :5497 */
            else
                    coef[i] = 0;
    }

On rev 3 and above the loop copies four uninitialised stack halfwords
into `nphy->cal_cache.txcal_coeffs_{2G,5G}`, the driver's TX
calibration cache; the `else` that would have zeroed them runs only on
the revisions that do not read them. The loop is verbatim Linux b43's
`b43_nphy_restore_cal()`, so this is upstream's, shared with Linux, and
on hardware neither tree can test here.

Left alone deliberately. `sys/gnu/` is the GPL-licensed vendored half of
this driver and a divergence here is a divergence from b43; the fix
(hoisting the read of `coef` that upstream evidently intended) changes
what reaches a radio's calibration registers, which is not a change to
make blind. Recorded so it is a decision and not an oversight.

### `sys/kgssapi/krb5/krb5_mech.c:661` — `M_WAITOK` cannot fail

    MGET(m, M_WAITOK, MT_DATA);     /* :656 -> m_get(M_WAITOK, ...) */
    M_ALIGN(m, tlen);
    m->m_len = tlen;
    p = (uint8_t *) m->m_data;
    *p++ = 0x60;                    /* :661 */

`m_get(M_WAITOK, ...)` blocks until it succeeds and never returns NULL.
The analyser does not know that, so every `M_WAITOK` allocation in the
tree is a potential NULL for it — which is a systemic false-positive
source, not a fact about Kerberos. Filed as its own piece of work:
teaching the analyser what `M_WAITOK` means is worth more than any
single triage.

### `sys/rpc/rpcsec_gss/rpcsec_gss_prot.c:191` — three guards and no assertion

    static uint32_t
    get_uint32(struct mbuf **mp)
    {
        struct mbuf *m = *mp;
        if (m->m_len < sizeof(uint32_t)) {      /* :191 */

`m` is `*mp` and `get_uint32()` itself sets `*mp = NULL` when its
`m_pullup()` fails. Its four call sites in this file check the result
three times and not the fourth (`:243`), which is the usual shape — but
`:243`'s omission is absorbed, because the `m_split(NULL, ...)` it feeds
returns NULL and the `if (!results)` on the next line catches it.

The path the checker actually found is different and better: at `:308`,
`get_uint32(&message)` where `message` is still the `NULL` it was
initialised to at `:235`, because `svc` matched neither
`rpc_gss_svc_integrity` nor `rpc_gss_svc_privacy`. There is a fourth
value — `rpc_gss_svc_default = 0`, `sys/rpc/rpcsec_gss.h:42` — and
`xdr_rpc_gss_unwrap_data()` asserts nothing about `svc` at all.

It is unreachable today, and it takes three separate facts to say so:

1. both callers return early on `rpc_gss_svc_none` before calling
   (`svc_rpcsec_gss.c:1702`, `rpcsec_gss.c:720`) — but neither names
   `rpc_gss_svc_default`;
2. the server rejects any other `gc_svc` with `AUTH_BADCRED`
   (`svc_rpcsec_gss.c:1492-1494`) — but **only** when `gc_proc` is
   neither `RPCSEC_GSS_INIT` nor `RPCSEC_GSS_CONTINUE_INIT`, and
   `cc->cc_service = gc.gc_svc` is assigned twenty lines *earlier*, at
   `:1472`, unvalidated;
3. and the INIT path ends in `RPCSEC_GSS_NODISPATCH`
   (`svc_rpcsec_gss.c:1543`), so the request whose service was never
   validated is never dispatched and never unwrapped.

Remove any one of the three and a remote peer chooses a service number
that reaches a NULL dereference in the kernel. Recorded here rather than
patched, because the patch is a design question — assert in the callee,
validate `gc_svc` before assigning it, or name `rpc_gss_svc_default` in
both callers' guards — and all three change an on-the-wire error path.

### `sys/rpc/rpcsec_tls/rpctls_impl.c:100`

`upsock_compare()` dereferences `a->so` and `b->so` as the RB-tree
comparator. The tree's own insert and lookup never pass NULL; the
checker reaches it through `RB_GENERATE_STATIC`'s generated body, where
the node under comparison is whatever the caller supplied. Same class as
the `phy_n` five: an invariant held by every caller and asserted by
none.

## Sweep 11's libs shard: 24 more files read, and one real leak the
 checker did not report

Reading userland's real `CFLAGS` out of bmake took the libs shard from
1,558 OK / 72 ERROR to **1,582 / 48** — 24 `ERROR -> OK`, none the other
way — and its unlisted ERRORs from 57 to 22. Eighteen findings are new,
and almost all of them are in libc's name-service switch: `getpwent.c`,
`getgrent.c`, `getservent.c`, `getprotoent.c`, `getrpcent.c`. Those
files need `-DNS_CACHING`, `-DYP` and `-DHESIOD`, which live in
`lib/libc/Makefile`'s chain, and until bmake was asked for them the NSS
code had never been analysed at all.

Twenty of the eighteen-plus-duplicates land on one line each, four at a
time:

    lib/libc/gen/getpwent.c:420  Potential leak of memory pointed to by
                                 'mp_state'  [unix.Malloc]   (x4)

and line 420 is `NSS_MP_CACHE_HANDLING(passwd);` — one macro,
instantiated five times across the five files, four functions deep each.

**What the checker is complaining about is not a leak.**
`nss_tls.h:67-70` does

    *p = calloc(1, sizeof(**p));
    if (*p == NULL)
            return (ENOMEM);
    rv = _pthread_setspecific(name##_state_key, *p);

and the analyser does not model `_pthread_setspecific()` as taking
ownership, so the allocation looks abandoned at the end of `getstate`.
Four reports per instantiation, five instantiations, in every NSS
consumer. That is the same class as `M_WAITOK` in the kernel: an
ownership transfer the instrument cannot see.

**Reading it led to one that is real, and that the checker never
mentions.** The pointer `_pthread_setspecific()` takes is freed by the
key's destructor when the thread exits — and `NSS_MP_CACHE_HANDLING`'s
destructor did not free it:

    static void
    name##_mp_endstate(void *s) {
        struct name##_mp_state *mp_state = (struct name##_mp_state *)s;
        if (mp_state->mp_write_session != INVALID_CACHED_MP_WRITE_SESSION)
                __abandon_cached_mp_write_session(...);
        if (mp_state->mp_read_session != INVALID_CACHED_MP_READ_SESSION)
                __close_cached_mp_read_session(...);
    }                                        /* and never free(s) */

Sixteen of the seventeen `_endstate` functions written out by hand in
`lib/libc` free the state itself — `files_endstate`, `dns_endstate`,
`nis_endstate` and `compat_endstate` in each of `getpwent.c`,
`getgrent.c`, `getservent.c` and `getrpcent.c`, plus `nsdispatch.c`'s
`fb_endstate`. The seventeenth, `getnetgrent.c`'s `netgr_endstate`,
deliberately does not — and is right not to, because it is *also*
called directly on a static (`getnetgrent.c:399`,
`netgr_endstate(&compat_state)`) and only frees the state's members.
The generated one is the eighteenth, is only ever a key destructor, and
had no reason.

So: one `struct <db>_mp_state` leaked per non-main thread, per NSS
database that went through nscd, for each of `passwd`, `group`,
`services`, `protocols` and `rpc`. Sixteen bytes each — small, and
unbounded in a long-lived threaded server that keeps creating threads.

Fixed with `free(mp_state);` in the macro's destructor. The static state
that `nss_tls.h` hands the main thread never reaches the destructor —
only values passed to `_pthread_setspecific()` do, and the static is not
one of them — so freeing there is safe; and when `setspecific` itself
fails, `nss_tls.h:71-73` already frees and clears, so there is no second
owner.

The four `unix.Malloc` reports are still there afterwards, because they
were never about this. That is worth stating plainly: the finding was a
false positive, and following it to its source found a defect it was not
reporting.

## A file that compiles to nothing is also indistinguishable from a clean one

`tools/verify/expected_errors.py` exists because a file that does not
compile reports zero findings and looks exactly like a clean one. Sweep
11's `kern` shard found the same failure one level down, where no
inventory was watching: a file that **does** compile, to nothing.

    netinet/tcp_ratelimit.c  optional ratelimit inet | ratelimit inet6

and `tcp_ratelimit.c:67` is `#ifdef RATELIMIT`, closed at the end of the
file. Without that macro the translation unit is a licence header and a
list of `#include`s. It compiled, it was `OK`, it reported nothing, and
it had reported nothing in every sweep since the first — 1,003 lines of
kernel code that no run has ever looked at.

Defining the options a file's own `optional` clause names — 2,141
sources that resolve to a file on disk — is what turned the body on.

How much code was dark? `tools/verify/check_empty_tus.py --measure`
counts only the lines of the **file's own** text that survive the
preprocessor — the linemarkers say which file each line came from, so
the headers do not drown the answer — for every source whose `#ifdef`
opens the file, names one of its own `optional` tokens, and **closes at
the last line**:

| off | on | source |
|---:|---:|---|
| 1 | 1,064 | `sys/netinet/tcp_ratelimit.c` |
| 0 | 638 | `sys/net80211/ieee80211_superg.c` |
| 0 | 452 | `sys/net80211/ieee80211_tdma.c` |
| 0 | 109 | `sys/arm/allwinner/a64/a64_padconf.c` |
| 0 | 19 | `sys/arm/allwinner/a64/a64_r_padconf.c` |
| 18,545 | 20,826 | all thirteen candidates |

Five were **empty**, or as near as makes no difference. Not "mostly
guarded" — a licence header, some `#include`s, and one `#ifdef` that was
false. They compiled, they were counted `OK`, and they contributed
nothing to any total in any sweep. 2,282 lines of kernel code, two of
them in the 802.11 stack.

The check is now a gate, and getting it to bite took two goes, both of
which are the point:

- **The first version could not fail.** It took its candidate list from
  `includes.files_option_defines()` — the map under test — so dropping
  `RATELIMIT` from that map dropped `tcp_ratelimit.c` from the
  candidates too, and it reported "every one of them has its option
  defined" while the file went dark. A check whose input is its own
  answer agrees with itself. The candidates now come from
  `sys/conf/files*` and the source text, read here.
- **The second version failed too loudly.** Twenty of its twenty-two
  complaints were files that merely *open* with a conditional include
  block — `tcp_syncache.c`, `ipsec.c`, `udp_usrreq.c` — and carry
  hundreds of lines after the `#endif`. Requiring the guard to close at
  the end of the file takes it from 22 wrong answers to 0, and leaves
  the thirteen that are really one macro from nothing.

Dropping `RATELIMIT` from the map now exits 1 and names the file.

The first thing the largest of them says:The first thing the largest of them says:

    tcp_ratelimit.c:747  The left operand of '&' is a garbage value
                         [core.UndefinedBinaryOperatorResult]

    if (rs->rs_rlt[(rs->rs_rate_cnt - 1)].flags & HDWRPACE_INITED)

`rs_rate_cnt` comes straight from the driver, at `:607` and `:618`:

    rs->rs_rate_cnt = rl.number_of_rates;

with no check for zero at either site. When it is zero: `:641` computes
`sz = 0`, `malloc(0, M_TCPPACE, M_NOWAIT)` hands back a valid non-NULL
pointer, the population loop at `:676` and the setup loop at `:682`
(`for (i = rs_rate_cnt - 1; i >= 0; i--)`) both have no iterations, and
`:747` reads `rs_rlt[-1]` — out of bounds, before the allocation, and
then decides whether to keep the table on what it finds there.

And the guard already exists in the same file, on the other path:
`rl_add_syctl_entries()` at `:349` is `if (rs->rs_rlt && rs->rs_rate_cnt
> 0)`. One of two — the shape this document keeps recording.

To be exact about what is and is not shown: no driver in this tree was
found returning `number_of_rates == 0`. The defect is that nothing
between the assignment and the read forbids it, on a value the kernel
takes from a device driver, in code the analyser had never been able to
read. The fix is an early return before the `malloc`, in the shape of
the `RT_IS_*` rejection twenty lines above it.

### And a note on method, from getting this one wrong first

The guard was written, and reverted within two minutes, because sweep
11's `kern` shard was part-way through `sys/netinet` at the time. It got
far enough: the shard recorded `tcp_ratelimit.c` as OK **with no
findings**, because it read the patched file. The finding then appeared
to have vanished between a standalone probe and the shard, which is a
much more alarming thing to believe than what had actually happened.

The record was re-measured against the reverted tree and replaced in the
shard, so `w11-kern` is now uniformly unpatched and the diff against
sweep 10 reads `1 new, 0 gone`. Two things are worth keeping from it:
editing a source under a running sweep destroys the measurement in a way
that looks like a result, and the accident did demonstrate — before the
fix was even committed — that the guard removes the finding.

## Sweep 11's `contrib`: twenty-one more read, and an invariant in a comment

965 OK / 385 ERROR becomes **986 / 364**. Twenty-one `ERROR -> OK`, none
the other way, and "ERRORs the inventory does not cover: 0". Two
findings are new, both in one file that had never compiled:

    sys/contrib/dev/acpica/components/disassembler/dmwalk.c:1250
    sys/contrib/dev/acpica/components/disassembler/dmwalk.c:1270
        [core.NullDereference]

    /*
     * The parent Op is guaranteed to be valid because of the flag
     * ACPI_PARSEOP_PARAMETER_LIST -- which means that this op is part of
     * a parameter list and thus has a valid parent.
     */
    ParentOp = Op->Common.Parent;              /* :1233 */
    ...
        ParentOp = ParentOp->Common.Parent;    /* :1250 */
        if (ParentOp && ...)                   /* :1251 */
    ...
        ParentOp->Common.DisasmFlags |= ...;   /* :1270 */

The invariant is stated, in a comment, four lines above the use — and
the very next line after the first dereference checks the *grandparent*
for NULL. So the file contemplates a short parent chain one level up and
asserts nothing at the level it relies on.

Left as it is. This is Intel's ACPICA, vendored whole; the claim is
upstream's, written down where a reader will find it, and an
`ACPI_ASSERT` added here diverges from the source tree PBSD re-imports
from. Recorded so the next sweep does not spend the same half hour.

## Three the coverage work found, fixed

Each was invisible until something the build system says was read, and
each is verified by the finding disappearing — not by argument.

### `sys/netinet/tcp_ratelimit.c:747` — `rs_rlt[-1]`

Written up above: `rs_rate_cnt` comes straight from the driver at `:607`
and `:618` with no zero check, and at zero `malloc(0)` succeeds, both
population loops have no iterations, and the "did we get at least 1
rate" test indexes `-1`. The guard is an early return before the
allocation, in the shape of the `RT_IS_*` rejection twenty lines above
it. The file's own `rl_add_syctl_entries()` has tested
`rs->rs_rlt && rs->rs_rate_cnt > 0` all along.

No driver in this tree was found returning `number_of_rates == 0`. The
defect is that nothing between the assignment and the read forbids it,
on a value the kernel takes from a device driver.

### `sys/dev/xilinx/xlnx_pcib.c` — two API drifts, in a driver nothing could build

`sys/conf/files.riscv:28` names it. It has not compiled in a long time:

    struct generic_pcie_core_softc  has no `bst' and no `bsh'

and three functions used them — `xlnx_pcib_req_valid()`,
`xlnx_pcib_read_config()` and `xlnx_pcib_write_config()`. The rest of
the same file already uses the modern idiom (`bus_read_4(sc->res, …)` at
`:113`, `:134`, `:215`), and so does the reference implementation it
wraps, `generic_pcie_read_config()` at `sys/dev/pci/pci_host_generic.c:304`.
The translation is mechanical and exact: `bus_read_4(r, o)` **is**
`bus_space_read_4(rman_get_bustag(r), rman_get_bushandle(r), o)`. The
driver's 32-bit-only quirk and the comment explaining it are untouched.

Then, one layer down, `sc->msi_page = kmem_alloc_contig(...)` at `:297`:
that function returns `void *` (`sys/vm/vm_extern.h:62`) and `msi_page`
is a `vm_offset_t` (`:87`). `sys/dev/xdma/controller/pl330.c:389` shows
the convention — it casts.

ERROR to **OK with no findings**. The same category as
`phyp_vscsi.c`'s missing semicolon: upstream has never compiled it.

### `sys/dev/pci/pci_host_generic_acpi.c:187` — three of four arms set `off`

Visible only with `-DDEV_ACPI`, which `optional pci acpi` implies and
the analyser does not yet supply — so this one was found by hand, with
the macro on the command line, while measuring what that change would be
worth. `pci_host_generic_acpi_parse_resource()` parses a PCI host
bridge's `_CRS`:

| arm | sets |
|---|---|
| `ACPI_RESOURCE_TYPE_ADDRESS16` (`:147`) | `restype`, `min`, `max` |
| `ACPI_RESOURCE_TYPE_ADDRESS32` (`:152`) | `restype`, `min`, `max`, **`off`** |
| `ACPI_RESOURCE_TYPE_ADDRESS64` (`:158`) | `restype`, `min`, `max`, **`off`** |
| `ACPI_RESOURCE_TYPE_FIXED_MEMORY32` (`:164`) | `restype`, `min`, `max`, **`off = 0`** |

and `:187` is `sc->base.ranges[r].phys_base = min + off;` for all four.
A 16-bit address descriptor in firmware's `_CRS` makes the kernel
program a PCI range's physical base from an uninitialised stack slot.
One line, the same one the 32- and 64-bit arms have, and the file goes
from one finding to none.

Three of four again — the shape this document has now recorded eleven
times.

## Sweep 13: 32 recovered, none regressed, and 61 findings with one cause

| | sweep 12 | sweep 13 |
|---|---|---|
| OK | 7,487 | **7,519** |
| ERROR | 622 | **590** |
| findings | 1,585 | 1,650 |

(Findings are counted the way `sweep_report.py` counts them — distinct
`(file, line, checker)`. The raw warning count is 1,728 and 1,803; one
line can carry several, as `linux_socket.c:1018` carries three, one per
argument.)

`ERROR -> OK` 32, `OK -> ERROR` **0**. Twenty-six of the thirty-two are
sweep 12's own regressions coming back — `sys/compat/linux` and
`sys/amd64/linux32`, which had been given `-DLOCORE` from a module rule
that belonged to one object and not to every source the module names.
Five more are `lib/msun/ld128`, and one, `sys/powerpc/powerpc/elf32_machdep.c`,
is the option retry finding `-DCOMPAT_FREEBSD32` — the third alternative
of `optional powerpc | powerpcspe | compat_freebsd32`, and the only one
that compiles.

Of the 65 new findings, **61 are in one file** —
`sys/compat/linux/linux_socket.c`, which had not compiled since the
regression — and all of them are one cause. The file produces 71
warnings at 61 distinct sites, and every one of the 71 traces passes
through `linux_socketcall()`:

    static const unsigned char lxs_args_cnt[] = {
        0 /* unused*/,      3 /* socket */,
        ...
        4 /* recv */,       6 /* sendto */,
        ...
    };

    if (args->what < LINUX_SOCKET || args->what > LINUX_ARGS_CNT)
            return (EINVAL);
    error = copyin(PTRIN(args->args), a, LINUX_ARG_SIZE(args->what));
    ...
    for (int i = 0; i < lxs_args_cnt[args->what]; ++i)
            l_args[i] = a[i];               /* :2761 */
    arg = l_args;
    switch (args->what) {
    ...
    case LINUX_SENDTO:
            return (linux_sendto(td, arg));  /* :2789 */

The handler reads all six words of what the loop filled. `what` is
`LINUX_SENDTO`, which is 11, so `lxs_args_cnt[11]` is 6 and the loop
fills all six — but the analyser loads `lxs_args_cnt[args->what]`
symbolically, does not fold it against the `switch`, and explores
"loop runs once" together with "dispatch to `case 11:`". That path does
not exist in the program.

Thirty lines reproduce it exactly:

    static const unsigned char cnt[] = { 0, 3, 6 };
    struct six { unsigned long a, b, c, d, e, f; };
    int sink(unsigned long);
    static int handler(struct six *s) { return sink(s->a) + sink(s->f); }

    int dispatch(int what, const unsigned long *in) {
            unsigned long out[6];
            if (what < 1 || what > 2) return 22;
            for (int i = 0; i < cnt[what]; ++i) out[i] = in[i];
            switch (what) {
            case 2: return handler((struct six *)out);
            }
            return 0;
    }

— same checker, same two notes in the trace: *"Loop condition is false.
Execution continues"* immediately followed by *"Control jumps to
`case 2:`"*.

So: an analyser limitation, recorded as a decision rather than counted
as 61 defects. Worth stating plainly because the alternative reading —
1,650 findings, up 65 — is the one a total gives you, and 61 of those 65
are one symbolic array load. The other four are one each in
`linux_ioctl.c`, `linux_misc.c`, `linux_signal.c` and
`nfs_nfsdport.c`, and nothing went away: `gone` is zero.

The 72nd warning the file produces is a `deadcode` note at `:1427`
(`Value stored to 'error' is never read`), which the checker list does
not run and this sweep does not count.

### `lib/msun/src/k_rem_pio2.c` — eleven, and `prec` is not an input

Eleven findings, every one of them downstream of `:296`:

    static const int init_jk[] = {3,4,4,6};   /* :133 */
    ...
    int
    __kernel_rem_pio2(double *x, double *y, int e0, int nx, int prec)
    {
            int32_t jz,jx,jv,jp,jk,carry,n,iq[20],i,j,k,m,q0,ih;
            double z,fw,f[20],fq[20],q[20];

            jk = init_jk[prec];               /* :296 */

`init_jk[]` has four entries. With `prec` unconstrained the read is out
of bounds, `jk` is garbage, `m = jx+jk` is garbage, the loop at `:306`
fills none of `f[]`, and everything that touches `f`, `q` or `iq`
afterwards is reported: `:310`, `:329`, `:333`, `:365`, `:380`, `:394`,
`:416`, and four assignments at `:433` and `:435`.

The function is not static, so the analyser must treat `prec` as
arbitrary — the same rule that makes `nice(int incr)` worth reading.
Here it is not, and the test is the one that section states: is this a
parameter of an **exported** function? It is not. `__kernel_rem_pio2`
appears in no `Symbol.map` under `lib/msun`; it is declared in
`math_private.h:911` and called from exactly four places, each with a
literal:

| caller | `prec` |
|---|---:|
| `src/e_rem_pio2f.c:72` | 0 |
| `src/e_rem_pio2.c:173` | 1 |
| `ld80/e_rem_pio2l.h:134` | 2 |
| `ld128/e_rem_pio2l.h:125` | 3 |

Four callers, four literals, four table entries — the domain is covered
exactly and nothing outside libm can reach the function.

Sweep 16 reports nine of them, cited so they can be counted:
`k_rem_pio2.c:310`, `k_rem_pio2.c:329`, `k_rem_pio2.c:333`,
`k_rem_pio2.c:365`, `k_rem_pio2.c:380`, `k_rem_pio2.c:394`,
`k_rem_pio2.c:416`, `k_rem_pio2.c:433`, `k_rem_pio2.c:435`.

Recorded rather than changed: `static` would be the honest annotation,
and fdlibm's `__kernel_*` functions are shared across `src/`, `ld80/`
and `ld128/` translation units, so it is not available.

### `ufs_lookup.c` and `ext2_lookup.c` — twelve findings, one copied idiom

Six each, every one *"Dereference of null pointer (loaded from variable
`vpp`)"*, and both functions have the same shape because one is a copy of
the other:

    ufs_lookup_ino(struct vnode *vdp, struct vnode **vpp,
        struct componentname *cnp, ino_t *dd_ino)
    {
            ...
            if (vpp != NULL)          /* :216 - so vpp MAY be NULL */
                    *vpp = NULL;
            ...
            *vpp = tdp;               /* :606, :662, :708, :730, :740 */
            ...
            cache_enter(vdp, *vpp, cnp);  /* :747 */

The function states in its own third statement that `vpp` may be NULL and
then dereferences it six times without a test. The analyser is following
a branch the code creates.

**UFS**: reachable, and guarded — by the *other* argument. Every store is
behind an `if (dd_ino != NULL) return (0);`: `:584` covers `:601` and
`:606`, `:630` covers `:662`, and `:665` covers `:708`, `:730`, `:740`
and the `cache_enter(vdp, *vpp, cnp)` at `:747`. The four callers that
pass `vpp == NULL` — `ufs_vnops.c:1346`, `:1372`, `:1714` and
`ffs_softdep.c:2891` — all pass a non-NULL `dd_ino`, and the one that
passes a real `vpp`, `ufs_lookup()` at `:184`, passes `dd_ino = NULL`.
So the invariant is *`vpp == NULL` implies `dd_ino != NULL`*, maintained
at five call sites and asserted at none. Thin, in the same way
`pf_lb.c:1026` is thin: it holds, and nothing in the function says so.

**ext2fs**: not reachable at all. `ext2_lookup_ino()` is `static`
(`:125`) with exactly one caller, `ext2_lookup()` at `:311`, which passes
`ap->a_vpp` and `dd_ino = NULL`. A VFS lookup's `a_vpp` is never NULL. So
`:342`'s `if (vpp != NULL)` and the `dd_ino != NULL` returns at `:579`,
`:622` and `:630` are all vestigial — copied from UFS, where they carry
weight, into a file where no caller can exercise them. The analyser
reports them because `a_vpp` is a field of a parameter and so
unconstrained.

Cited so the reader can count them: `ufs_lookup.c:601`,
`ufs_lookup.c:606`, `ufs_lookup.c:662`, `ufs_lookup.c:708`,
`ufs_lookup.c:730`, `ufs_lookup.c:740`; `ext2_lookup.c:583`,
`ext2_lookup.c:602`, `ext2_lookup.c:627`, `ext2_lookup.c:676`,
`ext2_lookup.c:690`, `ext2_lookup.c:695`.

(The ext2fs six are the findings' own lines. An earlier revision of this
paragraph listed `:579`, `:584`, `:622`, `:630`, `:664` and `:674`, which
are the GUARDS the prose above names — the `dd_ino != NULL` returns — and
not the reports. Citing the guard instead of the finding is the same
mistake as citing nothing.)

Twelve findings, no defect in either, and the reason they read the same
is that the second file inherited the first file's contract without
inheriting its callers. Recorded rather than changed: removing ext2fs's
dead half would silence six findings and buy nothing, and the divergence
from the UFS shape it was deliberately copied from is worth more than the
six.

## Sweep 14: the twelve powerpc files, and one guard that was on one of two

| | sweep 13 | sweep 14 |
|---|---|---|
| OK | 7,519 | **7,531** |
| ERROR | 590 | **578** |
| findings | 1,650 | 1,652 |

`ERROR -> OK` 12, `OK -> ERROR` **0**. All twelve are what the cpu index
was for — `sys/powerpc/{booke,mpc85xx,powernv,ps3,pseries,powerpc,ofw}`,
each of which had been failing on an identifier inside an `AIM` or a
`BOOKE` block in `sys/powerpc/include/{spr,pte,tlb}.h`. Three findings
are new and one is gone.

### `sys/i386/i386/trap.c:809` — the F00F arm writes before the guard

Not a powerpc file at all. Every i386 configuration in this tree says
`cpu I486_CPU`, `cpu I586_CPU` and `cpu I686_CPU`, so the cpu index
supplies all three — and `-DI586_CPU` opened a block the analyser had
never compiled:

    trap_pfault(struct trapframe *frame, bool usermode, vm_offset_t eva,
        int *signo, int *ucode)
    {
            MPASS(!usermode || (signo != NULL && ucode != NULL));   /* :750 */
            ...
            if (eva >= PMAP_TRM_MIN_ADDRESS) {
    #if defined(I586_CPU) && !defined(NO_F00F_HACK)
                    if ((eva == (unsigned int)&idt[6]) && has_f00f_bug) {
                            *ucode = ILL_PRVOPC;                   /* :810 */
                            *signo = SIGILL;
                            return (-2);
                    }
    #endif
                    if (usermode) {
                            *signo = SIGSEGV;                      /* :816 */
                            *ucode = SEGV_MAPERR;
                            return (1);
                    }
                    trap_fatal(frame, eva);
                    return (-1);

The function has two callers. `:396` passes `&signo, &ucode` with
`usermode = true`; `:481` passes `NULL, NULL` with `usermode = false`,
and the assertion on `:750` is what says that is allowed — `MPASS`,
which is `((void)0)` without `INVARIANTS`.

Every write to those pointers in the function is behind a test of
`usermode` — every one except this arm, which is reached first. The
guard exists on one of two, the twelfth time this document has recorded
that shape.

Reachability is narrow: a Pentium with the F00F erratum, the workaround
active, and a fault at `&idt[6]` taken in kernel mode. The bug delivers
that fault while the CPU is fetching the descriptor for `#UD`, so the
frame's CPL is the offending instruction's and a user program's F00F
sequence arrives with `usermode` true. What is left is a kernel-mode
access to the read-only IDT page — not something the kernel does, and
not something anyone can test here.

The fix is the one the rest of the function already makes: test
`usermode` first, and let a kernel-mode fault in that range fall through
to `trap_fatal()` like every other kernel fault in it. One line, and the
file goes from two findings to one.

Recorded in `check_pbsd_marks.py` as the 150th vendor-tree fix, and
checked by reverting it.

### The other two are the unconstrained-input class

`sys/powerpc/booke/pmap.c:2630` reads `args->e->virt` where `args` is
`tlb1_write_entry_int(void *arg)`'s parameter — an `smp_rendezvous`
callback, so the analyser has no caller and `args->e` is arbitrary.
`sys/fs/nfsserver/nfs_nfsdport.c:2683` is the same shape one field
deeper. Both belong to the class `k_rem_pio2.c` is the worked example
of, and neither is a defect.

## Sweep 15: the userland flags, and a dead file with a duplicate symbol

| | sweep 14 | sweep 15 |
|---|---|---|
| OK | 7,531 | **7,539** |
| ERROR | 578 | **570** |
| findings | 1,652 | 1,650 |

`ERROR -> OK` 8, `OK -> ERROR` **0**, no new findings and two gone. All
eight are `lib/libc` and `lib/msun`, and they are what asking bmake for
a file's own `CFLAGS.<file>` — and asking it as clang — buys:

    dlfcn.c tls.c                        CFLAGS.<file> is ${RTLD_HDRS}
    fts_blocks_test.c glob_blocks_test.c
    scandir_blocks_test.c qsort_b_test.c -fblocks, and NAMED at all
    detect_tz_changes_test.c             -I${SRCTOP}/contrib/tzcode
    i387/fenv.c                          i387 IS i386

The two findings that went away are `sys/i386/i386/trap.c:810`, which is
the F00F guard fixed above, and `sys/fs/nfsserver/nfs_nfsdport.c:2683`,
which is not explained. That one is absent in sweep 13, present in 14,
absent in 15, and every hypothesis has been ruled out — the flag list is
deterministic across processes, the analyser is deterministic on it, a
no-op macro changes nothing, and the module route does not reach the
file. What is left is a flag list that differed in sweep 14, and a sweep
records the finding and not the command that produced it. That is worth
fixing before the next one.

### `sys/amd64/vmm/amd/amdv.c` — a second `iommu_ops_amd`, without the const

Six of bhyve's seven unreadable sources want a header in
`sys/amd64/vmm/io/`, which `sys/modules/vmm/Makefile` puts on the path
with two `-I` and the kernel reader does not; supplying them by hand
compiles all six, three of them with a finding. The seventh does not,
and reading why says something about the file:

    io/iommu.h:62          extern const struct iommu_ops iommu_ops_amd;
    amd/amdvi_hw.c:1367          const struct iommu_ops iommu_ops_amd = {
    amd/amdv.c:118                     struct iommu_ops iommu_ops_amd = {

Two definitions of the same object, one of them without the `const` that
the header and the other definition both carry. `amdv.c` is the "not
implemented" stub — eleven functions whose bodies are `printf("...: not
implemented\n")` — that `amdvi_hw.c` replaced, and it is named by no
`files*` line and by no module: `sys/modules/vmm/Makefile:124` builds
`amdvi_hw.c` and not this. Were it built, the link would fail on a
duplicate symbol.

So it is on the record as not built, and the reason says which of the
two it is: not an include path, a file the tree stopped compiling and
never removed.

## Sorting the 1,569 nobody has read, and the first thing it found

Fifteen sweeps in, the pile that is not written up here is larger than
the pile that is: 1,569 of sweep 15's 1,638 deduplicated findings are not
cited by line in this document. That is past the point where reading them
one at a time is a plan.

"Not cited" overstates "not read", and by a lot. `report.is_triaged()`
looks for the finding written as `` `ufs_lookup.c:601` ``, and this
document also writes `:601` inside a section whose heading already named
the file — the `ufs_lookup.c` and `ext2_lookup.c` section is twelve
findings written up exactly that way. 542 of the 1,569 are in a file this
document discusses somewhere. That is a weaker claim than a citation and
`param_premise.py` prints it as its own number rather than folding it
into the first; the real figure for "nobody has looked at this" is
somewhere between 1,022 and 1,569, and closing the gap means citing lines
in the prose, not loosening the reader.

`tools/verify/param_premise.py` sorts them by **whose precondition they
are** — the rule `tools/verify/report.py` has applied to the CBMC runs
since the beginning and the clang sweep never had:

|  | findings | names a param | param on the line |
|---|---:|---:|---:|
| exported | 621 | 48 | 246 |
| static, address taken | 235 | 0 | 44 |
| static, called here | 611 | 60 | 244 |
| static, unreferenced | 8 | 1 | 4 |
| in a header | 35 | 10 | 21 |
| in a generated header | 7 | 0 | 0 |
| **attributed to no function** | **52** | | |

Four of those categories are new information rather than a restatement.

**static, address taken** is a `static` function with no call site in its
own file and a mention that is not a call — a `DEVMETHOD` entry, a kobj
method, a callout, an ioctl table. It is reached through a pointer by
code that is not in the translation unit, so as far as a precondition is
concerned it is exported, and the 235 findings in that class are not
deferrable on the "the callers are all here" argument.

**in a header** is a `static __inline` in a `.h`: `sys/sys/refcount.h:69`,
`sys/vm/vm_page.h:959`, `sys/sys/time.h:87`. It has no caller in its own
file and a caller in every unit that includes it, and the analyser starts
at it as a top-level entry with its parameters unconstrained in each one.

**in a generated header** is `config(8)`'s `vnode_if.h` and the other
`*_if.h` interfaces, which the sweep writes into a `mkdtemp` directory
that does not outlive the run. Seven findings live at a path that belongs
to one run; `sweep_report.py` already normalises it away, which is why
they have never shown up as fourteen in a delta.

**attributed to no function** is the tool declining. All of it is code a
macro generated at file scope — `RB_GENERATE_STATIC` is most of it — and
saying nothing there is better than charging it to whichever function
happens to be above it.

### `lib/libc/gen/sysctl.c` — the guard was on one write of three

The first thing read out of the exported-and-names-a-parameter column is
a null store from a public libc entry point.

`sysctl(3)` documents `oldp` and `oldlenp` both being `NULL` when the old
value is not wanted, and `sysctl()` itself knows it — line 71 is

```c
	orig_oldlen = oldlenp != NULL ? *oldlenp : 0;
```

There are three writes through `oldlenp` in the `CTL_USER` post-processing
that follows, and **one of them tested for it**:

| | |
|---|---|
| `:92` `USER_LOCALBASE` | `if (oldlenp == NULL \|\| *oldlenp > sizeof("")) return (0);` |
| `:106` `USER_CS_PATH` | `set_user_str(oldp, oldlenp, ...)`, whose `:60` is an unconditional `*dstlenp = len;` |
| `:114` every `USER_*` int | `*oldlenp = sizeof(int);`, reached with `oldp == NULL` |

So

```c
	sysctl((int []){CTL_USER, USER_BC_BASE_MAX}, 2, NULL, NULL, NULL, 0);
```

stores through a null pointer. The path is short and every step of it is
in the tree:

1. `sys___sysctl()` (`kern_sysctl.c:2455`) handles a null `oldlenp`
   explicitly — `if (uap->oldlenp)` guards the `copyout` at `:2472` — and
   `userland_sysctl()` does too, at `:2560`, leaving `req.oldlen` zero.
2. `kern_mib.c:688` really does register `_user.bc_base_max` as a
   `CTLFLAG_RD` int with `SYSCTL_NULL_INT_PTR`, so the lookup succeeds and
   `SYSCTL_OUT` with a null `oldptr` returns 0. `retval` is 0.
3. libc's `:81` `if (retval != 0 || name[0] != CTL_USER) return (retval);`
   therefore does not return, `namelen` is 2, `newp` is `NULL`, `name[1]`
   is neither `USER_LOCALBASE` nor `USER_CS_PATH`, and `:110`'s
   `oldp != NULL &&` short-circuits past the read.
4. `:114` writes.

`oldp != NULL` with `oldlenp == NULL` does *not* reach it: `SYSCTL_OUT`
against a zero `validlen` returns `ENOMEM`, `retval` is non-zero and libc
returns at `:81`. The reachable case is exactly the documented one.

The fix is the guard the other name already has, moved to where it covers
all three:

```c
	if (oldlenp == NULL)
		return (0);
```

placed after the `newp != NULL` check, so `USER_CS_PATH` is covered as
well as the integers. Three findings to none on that file, at the same
flag digest `3e06ab59bf8b` — the digest sweep 15 added exists to say that
the change was the code and not the command, and this is the first time
it has been used to say it.

That is the twelfth time in this document that the shape has been "the
guard exists on N of M".

### `lib/libc/iconv/citrus_stdenc.c:137` — the close path takes the malloc failure

`_citrus_stdenc_open()` allocates the handle and, when that fails, goes to
the same label every other failure goes to:

```c
	ce = malloc(sizeof(*ce));
	if (ce == NULL) {
		ret = errno;
		goto bad;
	}
	...
bad:
	_citrus_stdenc_close(ce);
```

and `_citrus_stdenc_close()`'s first test is `ce == &_citrus_stdenc_default`,
not `ce == NULL`, so the next line dereferences it. Every other `goto bad`
in the function reaches the label with a valid `ce`; this one is the
allocation that produced it.

Fixed by taking a null handle the way `free(3)` does. One finding to none
at the same digest `1081cf899313`. The four call sites outside this file —
`libiconv_modules/iconv_std/citrus_iconv_std.c:393,395,414,415` — all pass
handles that `_stdenc_open()` filled, so nothing else changes.

### `lib/libc/rpc/pmap_prot2.c:105` — a use-after-free the tree had already fixed three times

The analyser's complaint here is a false positive of a class this document
already knows: `xdr_bool()` is in another translation unit, so the checker
must assume it can set `more_elements` true while `*rp` is null, and the
comment four lines above says it cannot —

```c
	 * more_elements is pre-computed in case the direction is
	 * XDR_ENCODE or XDR_FREE.  more_elements is overwritten by
	 * xdr_bool when the direction is XDR_DECODE.
```

Reading the loop to establish that found something else.

```c
		if (freeing)
			next = &((*rp)->pml_next);
		if (! xdr_reference(xdrs, (caddr_t *)rp,
		    (u_int)sizeof(struct pmaplist), (xdrproc_t)xdr_pmap))
			return (FALSE);
		rp = (freeing) ? next : &((*rp)->pml_next);
```

The comment above it says "in the case of freeing we must remember the
next object before we free the current object", and the code remembers the
**address of a field inside the object about to be freed**.
`xdr_reference()` under `XDR_FREE` (`xdr_reference.c`) ends with

```c
	if (xdrs->x_op == XDR_FREE) {
		mem_free(loc, size);
		*pp = NULL;
	}
```

and `sys/rpc/types.h:69` is `#define mem_free(ptr, bsize) free(ptr)`. So
`rp` is left pointing into freed memory, the next iteration reads `*rp`
out of it, and — if the value read is not null — passes it to
`xdr_reference()` to be freed in turn. A freed-memory-driven `free()`.

What settles it is that **this tree already carries the fix, three times
over.** `xdr_rpcblist_ptr()` and `xdr_rpcb_entry_list_ptr()`, in
`lib/libc/rpc/rpcb_prot.c` and again in the kernel's `sys/rpc/rpcb_prot.c`,
are this same loop with this same comment, and all four copies read

```c
		if (freeing && *rp)
			next = (*rp)->rpcb_next;		/* the VALUE */
		...
		if (freeing) {
			next_copy = next;
			rp = &next_copy;
			/*
			 * Note that in the subsequent iteration, next_copy
			 * gets nulled out by the xdr_reference
			 * but next itself survives.
			 */
		} else if (*rp) {
			rp = &((*rp)->rpcb_next);
		}
```

`pmap_prot2.c` kept the original. The fix is that shape, transcribed:
`next` becomes a `struct pmaplist *`, `next_copy` appears, and the
non-freeing arm gains the `*rp` test that also disposes of the analyser's
complaint. One finding to none at the same digest `bf5404f30c2d`.

Reachable: `usr.sbin/rpcbind/warmstart.c:175` is
`xdr_free((xdrproc_t)xdr_pmaplist_ptr, &list_pml)` on the list read back
out of `/tmp/portmap.xdr` at warm start, and `xdr_pmaplist_ptr()` is one
line — `return xdr_pmaplist(xdrs, (struct pmaplist **)(void *)rp);`.
`lib/libc/rpc/pmap_getmaps.c:85` reaches it through `CLNT_CALL`'s result
free. The kernel does not: `sys/rpc/pmap_prot.h` declares `xdr_pmaplist()`
and nothing under `sys/` defines or calls it.

One copy is left unfixed, `crypto/krb5/src/lib/rpc/pmap_prot2.c:110`, and
it is vendor code PBSD does not own — `SKIP-VENDOR` in the ledger. It is
recorded here rather than edited.

The shape, for the thirteenth time: the guard exists on N of M. Here N and
M are whole files.

### Ten more out of the same column, all preconditions, all cited

The rest of the exported-and-names-a-parameter column in the shards that
had finished. None is a defect; each is written up with its line so it
stops being counted as unread.

**`msdosfs_lookup.c:518`, `msdosfs_lookup.c:575`,
`msdosfs_lookup.c:584`, `msdosfs_lookup.c:591`,
`msdosfs_lookup.c:605`** — the third
copy of the `ufs_lookup_ino()` shape above, and the best-built of the
three. `msdosfs_lookup_ino(vdp, vpp, cnp, scnp, blkoffp)` says at `:178`
that `vpp` may be null and stores through it five times, but where UFS
scatters four `if (dd_ino != NULL) return` guards through the body,
msdosfs has exactly one — `:488`

```c
	if (scnp != NULL) {
		*scnp = cluster;
		*blkoffp = blkoff;
		return (0);
	}
```

— and it dominates all five stores. The six call sites keep the same
invariant UFS's five do: `msdosfs_vnops.c:1009`, `msdosfs_vnops.c:1033`,
`msdosfs_vnops.c:1157`, `msdosfs_vnops.c:1186` and
`msdosfs_vnops.c:1208` pass `vpp == NULL` with `&scn`, and
`msdosfs_lookup.c:92`, the VFS `lookup` entry, passes a real `a_vpp` with
`scnp == NULL`. One guard instead of four, in the file that copied the
idiom last.

Two more in the same file are the same `vpp`: `msdosfs_lookup.c:80` and
`msdosfs_lookup.c:84`, in `msdosfs_lookup_checker()`, whose only caller
is `msdosfs_lookup.c:595` — inside the region the `:488` guard already
dominates.

**`nfs_nfsdstate.c:700`, `nfs_nfsdstate.c:716`,
`nfs_nfsdstate.c:877`** — `nfsrv_getclient()` tests
`nd == NULL` in its own second statement (`:679`) and again at `:727`,
and dereferences `nd` without a test at three places. The three
dereferences are all inside `if (opflags & CLOPS_CONFIRM)` (`:696`) or
`if (opflags & CLOPS_RENEWOP)` (`:871`), and **every one of the seven
call sites in the tree passes a real `nd`** —
`nfs_nfsdserv.c:4020`, `:4500`, `:4926` and `nfs_nfsdstate.c:1943`,
`:1956`, `:2602`, `:2871`. So the `nd == NULL` arm at `:679` is
vestigial, in the same way `ext2_lookup_ino()`'s guards are, and it is
what makes the analyser explore a null `nd` at all.

**`nfs_nfsdport.c:3566`** — `nfsd_excred()` uses `credanon` three times
without testing it, and its caller tests it five lines later:
`:3776` is `if (credanon != NULL) crfree(credanon);`. The guard that
matters is neither: `vfs_stdcheckexp()` (`vfs_export.c:683`) really can
return 0 with `*credanonp == NULL`, and every path that does so also
leaves `exp->nes_exflag = 0` — `nfsvno_fhtovp()` sets it explicitly at
`nfs_nfsdport.c:3608` and `:3649` on the CHECKEXP error it then converts
back to success — so `NFSVNO_EXPORTED(exp)` at `:3562` is false and the
three uses are not reached. The correlation is between an out-parameter
and a flag word filled by an indirect call through the VFS vector, which
is exactly what an analyser confined to one translation unit cannot
follow.

**`kern_jail.c:4050`** — `prison_isalive(const struct prison *pr)` is
two lines and dereferences its parameter. A precondition, not a defect,
and the same class as `cam_xpt.c:5253` above.

### 29 "potential leak"s in libc, and all of them are thread-local storage

`param_premise.py`'s static-with-its-address-taken column is 235
findings, and the first thing visible in it is that libc's name-service
backends account for 29 of them with one cause. Every one is
`unix.Malloc`, *"Potential leak of memory pointed to by …"*, in a
`static` function that appears in an `ns_dtab[]` rather than at a call
site.

Two macros generate the same thing. `lib/libc/include/nss_tls.h:39`'s
`NSS_TLS_HANDLING(name)` writes a `name_getstate()`:

```c
	if (!__isthreaded || _pthread_main_np() != 0) {
		*p = &st;			/* a static - must NOT be freed */
		return (0);
	}
	rv = _pthread_once(&keyinit, name##_keyinit);
	...
	*p = calloc(1, sizeof(**p));
	...
	rv = _pthread_setspecific(name##_state_key, *p);
```

and `keyinit` is `_pthread_key_create(&key, name##_endstate)`.
`lib/libc/net/netdb_private.h:33`'s `NETDB_THREAD_ALLOC(name)` is the
same shape with `name##_free` as the destructor and `__name_init()` as
the accessor. Ownership goes to the thread-specific-data key at the
moment of allocation, and the destructor runs at thread exit. There is
nothing for the backend to free — and on the single-threaded path
freeing would be a bug, because what came back is the file-scope `st`.

The analyser models neither `pthread_setspecific()` nor the destructor
registered next to it, so every backend that takes the state and returns
looks like a leak.

`getgrent.c:834`, `getgrent.c:877`, `getgrent.c:1086`,
`getgrent.c:1291`, `getgrent.c:1482`;
`getnetgrent.c:243`, `getnetgrent.c:280`;
`getpwent.c:788`, `getpwent.c:915`, `getpwent.c:1282`,
`getpwent.c:1723`, `getpwent.c:1939`;
`gethostnamadr.c:672`, `gethostnamadr.c:687`, `gethostnamadr.c:702`;
`getprotoent.c:428`, `getprotoent.c:452`, `getprotoent.c:464`;
`getservent.c:334`, `getservent.c:464`, `getservent.c:537`,
`getservent.c:641`, `getservent.c:822`, `getservent.c:1320`;
`getrpcent.c:249`, `getrpcent.c:365`, `getrpcent.c:593`,
`getrpcent.c:952`; `nsdispatch.c:667`.

Nothing to change. Recorded because 29 findings that read as memory
leaks in a library every program links are worth being able to dismiss
by name rather than one at a time — which is the whole point of sorting
the sweep by who can call the function.

## Sweep 16: the six bhyve files, and the one finding they were hiding

| | sweep 15 | sweep 16 |
|---|---|---|
| OK | 7,539 | **7,545** |
| ERROR | 570 | **564** |
| findings (deduplicated) | 1,650 | **1,651** |

Six `ERROR -> OK`, none the other way, and the six are the ones the
`.elif` fix predicted: `sys/amd64/vmm/vmm.c`, `vmm_ioport.c`,
`vmm_lapic.c`, `amd/svm.c`, `amd/vmcb.c` and `intel/vmx.c`. Five of them
compile clean. The sixth is the point of the exercise.

### `sys/amd64/vmm/amd/svm.c:1548` — `errcode_valid` on the arm that says nothing

```c
	case 0x40 ... 0x5F:
		vmm_stat_incr(vcpu->vcpu, VMEXIT_EXCEPTION, 1);
		reflect = 1;
		idtvec = code - 0x40;
		switch (idtvec) {
		...
		}

		if (reflect) {
			...
			error = vm_inject_exception(vcpu->vcpu, idtvec,
			    errcode_valid, info1, 0);
```

`errcode_valid` is declared at `:1358` and never initialised. Of the arms
of the inner switch, `IDT_NP`/`SS`/`GP`/`AC`/`TS` set it to 1 (`:1441`),
`IDT_DF` sets it to 1 (`:1445`), the `default` sets it to 0 (`:1535`),
and `IDT_MC` (`:1427`) and `IDT_BP` (`:1516`) clear `reflect` so they
never reach the read.

`IDT_DB` does neither. `:1507`'s `reflect = 0` is **inside**

```c
		if (stepped && (vcpu->caps & (1 << VM_CAP_RFLAGS_TF))) {
```

so it applies to a TF single-step the hypervisor is expecting. A #DB the
hypervisor is *not* expecting — a guest's own hardware breakpoint through
DR0–DR3, or DR6.BD — leaves `stepped` false, `reflect` 1, and
`errcode_valid` never written. The value handed to
`vm_inject_exception()` is whatever was in that stack slot, and it
decides whether an error code is pushed onto the guest's stack for a
vector that has none.

Guest-triggerable: setting a debug register and hitting it is an
unprivileged operation inside the guest.

The fix is the idiom the Intel side already uses. `vmx.c:2740` is

```c
		/* Reflect all other exceptions back into the guest */
		errcode_valid = errcode = 0;
		if (intr_info & VMCS_INTR_DEL_ERRCODE) {
			errcode_valid = 1;
```

— cleared unconditionally on the line above the test that can raise it —
so `svm.c` now clears it next to `reflect = 1`, before the switch that
may or may not set it. One finding to none at the same flag digest
`4552621976ec`.

This one is worth noting for how it was found: the file has been in the
tree the whole time and in `expected_errors.py` since sweep 6, because
`sys/modules/vmm/Makefile`'s `.elif` arm was being skipped and bhyve was
compiled with one include directory of four. **A file that does not
compile reports zero findings and is indistinguishable from a clean one**
— fourteen sweeps of that, in the hypervisor.

### `dis_tables.c` — all 73, cited

`dis_tables.c` is 73 findings, the largest single file in the sweep, and
the two rows above account for 66 of them without naming a line the
reader can count. All three causes are the same one seen from three
angles: **the decoder's state machine decides which local is live in
which arm, and the analyser switches on a table entry it cannot
correlate with how that entry was reached.**

The 64 `core.CallAndMessage` "uninitialized value" findings are the
`d86_got_modrm` invariant, walked site by site in the row above:

`dis_tables.c:4654`, `dis_tables.c:4673`, `dis_tables.c:4686`,
`dis_tables.c:4720`, `dis_tables.c:4728`, `dis_tables.c:4738`,
`dis_tables.c:4744`, `dis_tables.c:4755`, `dis_tables.c:4772`,
`dis_tables.c:4782`, `dis_tables.c:4789`, `dis_tables.c:4802`,
`dis_tables.c:4861`, `dis_tables.c:4874`, `dis_tables.c:4892`,
`dis_tables.c:4907`, `dis_tables.c:5089`, `dis_tables.c:5177`,
`dis_tables.c:5185`, `dis_tables.c:5196`, `dis_tables.c:5232`,
`dis_tables.c:5246`, `dis_tables.c:5272`, `dis_tables.c:5308`,
`dis_tables.c:5325`, `dis_tables.c:5331`, `dis_tables.c:5336`,
`dis_tables.c:5344`, `dis_tables.c:5352`, `dis_tables.c:5359`,
`dis_tables.c:5380`, `dis_tables.c:5451`, `dis_tables.c:5457`,
`dis_tables.c:5473`, `dis_tables.c:5554`, `dis_tables.c:5719`,
`dis_tables.c:5737`, `dis_tables.c:5790`, `dis_tables.c:5829`,
`dis_tables.c:5867`, `dis_tables.c:5901`, `dis_tables.c:5959`,
`dis_tables.c:5970`, `dis_tables.c:6028`, `dis_tables.c:6042`,
`dis_tables.c:6050`, `dis_tables.c:6053`, `dis_tables.c:6066`,
`dis_tables.c:6084`, `dis_tables.c:6105`, `dis_tables.c:6118`,
`dis_tables.c:6126`, `dis_tables.c:6139`, `dis_tables.c:6149`,
`dis_tables.c:6164`, `dis_tables.c:6175`, `dis_tables.c:6194`,
`dis_tables.c:6196`, `dis_tables.c:6214`, `dis_tables.c:6227`,
`dis_tables.c:6241`, `dis_tables.c:6252`, `dis_tables.c:6263`,
`dis_tables.c:6275`.

Three more are the same invariant with a different checker —
`dis_tables.c:3073` and `dis_tables.c:3132` are `*reg += 8` inside
`dtrace_vex_adjust()` and `dtrace_evex_adjust_reg()`, which take the
address of that same `reg`, and `dis_tables.c:4461` is `reg == 4`.

Four are a second invariant of the same kind, on the opcode nibbles
rather than the modrm byte. `dis_tables.c:4675` is `MOVZ`'s
`WBIT(opcode5)`; `dis_tables.c:4687` and `dis_tables.c:4700` are
`CRC32`'s and `MOVBE`'s `WBIT(opcode7)`; `dis_tables.c:4829` is
`XMMSH`/`mm_shift`'s `REGNO(opcode7)`. Each of the four is reachable
only through the table walk that read the byte its nibble came from —
`:4141`'s `dtrace_get_opcode(x, &opcode4, &opcode5)` fills `opcode5`
before `dp = &dis_op0F[...]`, `:4147`'s fills `opcode7` before
`dp = &dis_op0F7123[opcode5][subcode]` (the `0F 71/72/73` SIMD shift
group, which is where `mm_shift` comes from), and `:4194`'s fills it
before the `dis_op0F38` lookup that yields `CRC32` and `MOVBE`. Reading
the byte and selecting the arm are the same step; a failed read is
`goto error`, not a fallthrough.

The last two, `dis_tables.c:6419` and `dis_tables.c:6447`, are the
single `goto done` in the row above.

Nothing to change in a vendored CDDL decoder. Cited so that the largest
file in the sweep stops being 72 uncounted findings and becomes one that
has been read.

### `linux_socket.c` — the same 61, cited

The sweep-13 section above establishes that every one of this file's
findings comes from one path the program does not have:
`linux_socketcall()` fills `l_args[i]` for `i < lxs_args_cnt[args->what]`
and then dispatches on `args->what`, and the analyser explores "the loop
ran once" together with "the `switch` went to `case LINUX_SENDTO`",
because it loads the count symbolically and does not fold it against the
same `what` the `switch` uses. Thirty lines reproduce it.

Sweep 16 still reports 61 sites — 33 `core.CallAndMessage`, 13
`core.UndefinedBinaryOperatorResult`, 13 `core.uninitialized.Assign`, 2
`core.uninitialized.Branch` — and only `:1018` was ever written where a
reader could count it. All 61:

`linux_socket.c:832`, `linux_socket.c:848`, `linux_socket.c:880`,
`linux_socket.c:892`, `linux_socket.c:911`, `linux_socket.c:918`,
`linux_socket.c:959`, `linux_socket.c:981`, `linux_socket.c:1018`,
`linux_socket.c:1099`, `linux_socket.c:1107`, `linux_socket.c:1118`,
`linux_socket.c:1143`, `linux_socket.c:1166`, `linux_socket.c:1169`,
`linux_socket.c:1176`, `linux_socket.c:1188`, `linux_socket.c:1218`,
`linux_socket.c:1219`, `linux_socket.c:1220`, `linux_socket.c:1221`,
`linux_socket.c:1259`, `linux_socket.c:1260`, `linux_socket.c:1261`,
`linux_socket.c:1262`, `linux_socket.c:1278`, `linux_socket.c:1288`,
`linux_socket.c:1293`, `linux_socket.c:1294`, `linux_socket.c:1296`,
`linux_socket.c:1308`, `linux_socket.c:1531`, `linux_socket.c:1542`,
`linux_socket.c:1548`, `linux_socket.c:1560`, `linux_socket.c:1943`,
`linux_socket.c:1947`, `linux_socket.c:2009`, `linux_socket.c:2049`,
`linux_socket.c:2063`, `linux_socket.c:2066`, `linux_socket.c:2074`,
`linux_socket.c:2108`, `linux_socket.c:2119`, `linux_socket.c:2130`,
`linux_socket.c:2133`, `linux_socket.c:2141`, `linux_socket.c:2148`,
`linux_socket.c:2157`, `linux_socket.c:2172`, `linux_socket.c:2191`,
`linux_socket.c:2232`, `linux_socket.c:2307`, `linux_socket.c:2338`,
`linux_socket.c:2386`, `linux_socket.c:2389`, `linux_socket.c:2407`,
`linux_socket.c:2414`, `linux_socket.c:2431`, `linux_socket.c:2683`,
`linux_socket.c:2690`.

Nothing to change: the table and the `switch` agree, and making the
analyser see it would mean writing the count twice.

### `ofw_real.c` — 19 OUT cells, read at the right moment this time

The section above fixed the one site that read an OUT cell *before*
`ofw_real_unmap()`. The other nineteen read it after, which is correct,
and the analyser cannot tell — for a reason worth writing down, because
it is the same bounce buffer seen from the other side.

An OUT cell is never assigned in C. Two things can write it, and the
analyser can follow neither:

* `ofw_real_unmap()` (`:354`) ends in
  `memcpy(buf, of_bounce_virt + (physaddr - of_bounce_phys), len)` — a
  copy out of a pointer nothing relates to `&args` — and it returns
  early, without copying, when `of_bounce_virt == NULL` (`:358`) or
  `physaddr == 0` (`:361`). That early return is the path the analyser
  explores.
* `openfirmware()` is an indirect call through a function pointer
  assigned at `ofw_real_init()`. When there is no bounce page it is
  handed the struct's own address, and it writes the cell in place.

Those two cases are exhaustive and they interlock. `ofw_real_map()`
(`:301`) returns `(cell_t)((uintptr_t)buf & ~DMAP_BASE_ADDRESS)` — the
1:1 physical address of the caller's own storage — when
`of_bounce_virt == NULL && !pmap_bootstrapped`, and *does not set*
`of_bounce_virt`; so the very `ofw_real_unmap()` early return the
analyser takes is the case where no copy back is needed, because the
firmware wrote `args` directly. Otherwise `ofw_real_map()` has a bounce
page (it installs `emergency_buffer` at `:323` if it must), so the
`memcpy` runs.

Either the firmware wrote the struct or `ofw_real_unmap()` copied it
back. Never neither, and never something the analyser can see.

`ofw_real.c:425`, `ofw_real.c:458`, `ofw_real.c:487`, `ofw_real.c:516`,
`ofw_real.c:545`, `ofw_real.c:577`, `ofw_real.c:617`, `ofw_real.c:654`,
`ofw_real.c:693`, `ofw_real.c:730`, `ofw_real.c:761`, `ofw_real.c:798`,
`ofw_real.c:835`, `ofw_real.c:926`, `ofw_real.c:978`,
`ofw_real.c:1038`, `ofw_real.c:1072`, `ofw_real.c:1105`,
`ofw_real.c:1142`.

Two of the nineteen are not `UndefReturn`: `ofw_real.c:926` is
`ofw_real_interpret()`'s `status = OUT(args.slot[i++])` and
`ofw_real.c:978` is the `args.instance == 0` test the fix above moved to
after the unmap. Same cell, same reason.

### The fortify tests, which are supposed to look like this

45 findings across four files under `lib/libc/tests/secure/`, and every
one of those files begins

```c
/* @generated by `generate-fortify-tests.lua "select"` */
```

The generator emits one ATF test body per call it checks, and the body is
deliberately the shape a fortify check is meant to catch:

```c
ATF_TC_BODY(FD_SET_end, tc)
{
	struct {
		uint8_t padding_l;
		fd_set __buf;
		uint8_t padding_r;
	} __stack;
	const size_t __len = FD_SETSIZE;
	const size_t __idx __unused = __len - 1;

	FD_SET(__idx, &__stack.__buf);
}
```

`__stack` is never initialised, and `FD_SET` is `__fds_bits[i] |= mask` —
a compound assignment on uninitialised storage, which is exactly the 12
`core.uninitialized.Assign` and `core.UndefinedBinaryOperatorResult`
findings in `fortify_select_test.c`. The test is about the *index*, not
the contents; `_FORTIFY_SOURCE 2` is set on line 3 of the file and what
is under test is whether the bounds check fires. The other 33 are
`unix.Malloc` "potential leak" in the heap variants, where the body
allocates the buffer and does not free it because the process is
expected to abort inside the call.

`fortify_select_test.c:182`, `fortify_select_test.c:203`,
`fortify_select_test.c:278`, `fortify_select_test.c:301`,
`fortify_select_test.c:376`, `fortify_select_test.c:397`,
`fortify_select_test.c:472`, `fortify_select_test.c:495`,
`fortify_select_test.c:570`, `fortify_select_test.c:591`,
`fortify_select_test.c:666`, `fortify_select_test.c:689`.

`fortify_string_test.c:232`, `fortify_string_test.c:256`,
`fortify_string_test.c:379`, `fortify_string_test.c:403`,
`fortify_string_test.c:526`, `fortify_string_test.c:550`,
`fortify_string_test.c:670`, `fortify_string_test.c:693`,
`fortify_string_test.c:968`, `fortify_string_test.c:995`,
`fortify_string_test.c:1296`, `fortify_string_test.c:1323`,
`fortify_string_test.c:1460`, `fortify_string_test.c:1487`,
`fortify_string_test.c:1624`, `fortify_string_test.c:1788`,
`fortify_string_test.c:1815`, `fortify_string_test.c:1952`,
`fortify_string_test.c:1979`, `fortify_string_test.c:2116`,
`fortify_string_test.c:2143`.

`fortify_strings_test.c:232`, `fortify_strings_test.c:256`,
`fortify_strings_test.c:376`, `fortify_strings_test.c:399`,
`fortify_strings_test.c:518`, `fortify_strings_test.c:541`.

`fortify_wchar_test.c:232`, `fortify_wchar_test.c:256`,
`fortify_wchar_test.c:379`, `fortify_wchar_test.c:403`,
`fortify_wchar_test.c:526`, `fortify_wchar_test.c:550`.

Not a defect, not editable — the files are generated, and a change would
be to `generate-fortify-tests.lua` and would make the tests test less.
Fourteen more findings are in hand-written tests under other `tests/`
directories and are not covered here.

### `lib/libc/nls/msgcat.c` — a lock that returns out of the middle of an ownership transfer

Nine findings: eight `unix.Malloc` "potential leak" and one
`unix.cstring.NullArg`. The eight are real, and they are one line.

```c
#define WLOCK(fail)	{ int ret;						\
			  if (__isthreaded &&					\
			      ((ret = _pthread_rwlock_wrlock(&rwlock)) != 0)) {	\
				  errno = ret;					\
				  return (fail);				\
			  }}
```

`WLOCK()` returns **from the caller**, and both places that use it are
one statement away from handing something to `cache`, which is where the
ownership of that something lives:

* `SAVEFAIL()` builds a `struct catentry` with two `strdup()`s and inserts
  it under `WLOCK(NLERR)`. Seven call sites — `msgcat.c:246`,
  `msgcat.c:270`, `msgcat.c:412`, `msgcat.c:419`, `msgcat.c:426`,
  `msgcat.c:445`, `msgcat.c:453`, `msgcat.c:472`.
* `load_msgcat()`'s success path, `msgcat.c:485`, is `WLOCK(NLERR)`
  immediately after `mmap()`ing the catalogue and allocating `catd`, `np`
  and three copied strings. On the failure path all of it goes, including
  the mapping.

The lock can fail. `rwlock` is `PTHREAD_RWLOCK_INITIALIZER`; libthr
initialises a statically-initialised rwlock on first use in
`rwlock_init()` (`lib/libthr/thread/thr_rwlock.c:97`), and that is an
`aligned_alloc()` which returns `ENOMEM` at `:106`. So the leak happens
exactly when the process is already short of memory — which is when a
message catalogue is most likely to be opened by an error path.

`TRY_WLOCK()` reports the failure instead of returning through it.
`SAVEFAIL()` then frees the entry it could not cache, which is the same
thing it already does when a `strdup()` fails, and `errno` ends up as the
`catopen()` failure the caller asked about rather than the lock's. In
`load_msgcat()` the cache entry cannot simply be dropped — `catclose()`
(`:369`) finds the catalogue *through* the cache, so an uncached `catd`
would leak the mapping instead of the entry — so that site undoes the
load the same way the `ENOMEM` arm eleven lines above it does. Eight
findings to none, at the same flag digest.

The ninth, `msgcat.c:253`, is not a defect: `strlcpy(pathP, tmpptr,
spcleft)` with `tmpptr` from `%l`/`%t`/`%c`/`%L`, and the analyser has
`lang == NULL` in hand from `:155`. That arm is taken only when `name`
contains a `/`, and `:190` returns for exactly that case before any of
this runs, so by `:194` `lang` is non-null — `"C"` at worst, `:166`. The
same test written twice, thirty-five lines apart, which the analyser does
not fold.

### `mpr_config.c` and `mps_config.c` — the 19 and the 7, cited

The row above works out why `error == 0` cannot coexist with `cm == NULL`
— `mpr_wait_command()` writes `*cmp = NULL` only inside
`if (error == EWOULDBLOCK)` and the next statement is
`error = ETIMEDOUT` — and names no line. Sweep 16's 26:

`mpr_config.c:100`, `mpr_config.c:183`, `mpr_config.c:232`,
`mpr_config.c:315`, `mpr_config.c:557`, `mpr_config.c:642`,
`mpr_config.c:694`, `mpr_config.c:831`, `mpr_config.c:916`,
`mpr_config.c:968`, `mpr_config.c:1053`, `mpr_config.c:1105`,
`mpr_config.c:1190`, `mpr_config.c:1239`, `mpr_config.c:1322`,
`mpr_config.c:1452`, `mpr_config.c:1502`, `mpr_config.c:1586`,
`mpr_config.c:1742`.

`mps_config.c:183`, `mps_config.c:681`, `mps_config.c:956`,
`mps_config.c:1088`, `mps_config.c:1218`, `mps_config.c:1352`,
`mps_config.c:1508`.

`mpr_config.c` is 19: ten `core.NullDereference` at the later
`cm->cm_length` and nine `core.UndefinedBinaryOperatorResult` at the
`reply == NULL` test. `mps_config.c` is 7, all dereferences, because
it declares `reply = NULL` in 9 of 9 functions and so never reaches the
indeterminate `==`. That difference is the whole point of the row: the
newer driver, copied from the older, dropped the initialiser in 10 of its
12 functions and rests entirely on the invariant instead.

### `nfs_nfsdport.c` — five pNFS mirror loops, one `M_WAITOK`

`nfs_nfsdport.c:5875`, `nfs_nfsdport.c:6037`, `nfs_nfsdport.c:6220`,
`nfs_nfsdport.c:6406`, `nfs_nfsdport.c:6554` — *"Access to field `done`
results in a dereference of a null pointer"*, five times, one idiom
copied into `nfsrv_writedsrpc()`, `nfsrv_allocatedsrpc()`,
`nfsrv_deallocatedsrpc()`, `nfsrv_setattrdsrpc()` and
`nfsrv_setacldsrpc()`:

```c
	drpc = NULL;
	if (mirrorcnt > 1)
		tdrpc = drpc = malloc(sizeof(*drpc) * (mirrorcnt - 1), M_TEMP,
		    M_WAITOK);
	...
	for (i = 0; i < mirrorcnt - 1; i++, tdrpc++) {
		tdrpc->done = 0;
```

The loop runs exactly when the allocation ran — both are
`mirrorcnt > 1` — so the only way to reach `tdrpc->done` with `tdrpc`
null is for the `malloc()` to have returned null. It cannot:
`M_WAITOK` sleeps until it can satisfy the request, which this document
established at the `nowait_check.py` work and states again at
`kern_malloc.c`'s row. The analyser models the kernel's `malloc()` as
the libc one.

Five findings, no defect, and the reason all five read the same is that
the pNFS DS-mirror path is one function written five times.

### The Allwinner clock drivers — 15 divisions by zero, held off by data

Eight files under `sys/dev/clk/allwinner/` produce 15 `core.DivideZero`
findings, and they are one flag.

A clock factor is a register field plus a rule for turning it into a
number, and `aw_clk.h` has three functions that read that rule.
`aw_clk_factor_get_factor()` (`:119`) converts a raw field —
`1 << raw` for `POWER_OF_TWO`, `raw + 1` normally, and **`raw` itself for
`AW_CLK_FACTOR_ZERO_BASED`**. `aw_clk_factor_get_min()` (`:144`) mirrors
that: `1` normally, and **`0` for `AW_CLK_FACTOR_ZERO_BASED`** (`:151`).

So a factor with that flag has a minimum of zero and can read zero out of
the hardware, and the search loops divide by it:

```c
	for (m = min_m; m <= max_m; ) {
		for (n = min_n; n <= max_n; ) {
			cur = fparent / n / m;		/* aw_clk_nm.c:150 */
```

```c
	div = aw_clk_get_factor(val, &sc->div);
	prediv = aw_clk_get_factor(val, &sc->prediv);

	*freq = *freq / prediv / div;		/* aw_clk_prediv_mux.c:121 */
```

A kernel division by zero on arm or arm64 is not a signal, it is a panic.

What keeps it latent is the clock definitions. `AW_CLK_FACTOR_ZERO_BASED`
appears **13 times in the whole tree** — three in `ccu_a10.c` and ten in
`ccu_a83t.c` — and every single one is the `n` factor of an `NKMP_CLK`:

    NKMP_CLK(pll_ddr_clk, ...
        8, 5, 0, AW_CLK_FACTOR_ZERO_BASED,	/* n factor */

In NKMP, `n` multiplies —
`cur = (fparent * n * k) / (m * p)`, `aw_clk_nkmp.c:155` — so `n == 0`
gives a candidate frequency of zero, which the search rejects, and never
a division. Not one divisor in the tree is zero-based: the `m` and `p`
factors of those same PLLs are `FIXED 1`, plain width fields (minimum 1)
or `POWER_OF_TWO` (minimum 1, since `get_min()` has no `POWER_OF_TWO`
arm and falls through to `1`).

Recorded rather than changed, and worth the section for what it depends
on: the guard is not in the code, it is in thirteen driver-data lines,
and a fourteenth that put `AW_CLK_FACTOR_ZERO_BASED` on an `m`, `p`,
`div` or `prediv` field would make all fifteen live at once. The same
treatment, and the same reason, as `al_hal_serdes_25g.c`'s eight above.

`aw_clk_frac.c:161`, `aw_clk_frac.c:332`, `aw_clk_m.c:146`,
`aw_clk_m.c:236`, `aw_clk_mipi.c:127`, `aw_clk_mipi.c:224`,
`aw_clk_nkmp.c:155`, `aw_clk_nkmp.c:339`, `aw_clk_nm.c:150`,
`aw_clk_nm.c:303`, `aw_clk_nmm.c:131`, `aw_clk_nmm.c:228`,
`aw_clk_np.c:125`, `aw_clk_np.c:216`, `aw_clk_prediv_mux.c:121`.

### `RB_GENERATE` — 31 findings, one macro, and the invariant is in its comment

31 findings across 25 files, every one *"Array access (via field
`rbe_link`) results in an undefined pointer dereference"*, every one at
the line where a file says `RB_GENERATE(...)`. They are not 31 things.
They are `sys/sys/tree.h:524`, `RB_GENERATE_INSERT_COLOR`, reported once
per instantiation — which is what made them hard to see as one until the
sweep started recording the function clang names, and the name came back
`<something>_RB_INSERT_COLOR` every time.

The variable is `child`, declared at `:540` and assigned only at `:557`
and `:596`, both on paths that loop; the rotations at `:583` and `:612`
read it. The macro says why that is safe, in its own words, at `:529`:

```c
	/*
	 * Initially, elm is a leaf.  Either its parent was previously
	 * a leaf, with two black null children, or an interior node
	 * with a black non-null child and a red null child. The
	 * balance criterion "the rank of any leaf is 1" precludes the
	 * possibility of two red null children for the initial parent.
	 * So the first loop iteration cannot lead to accessing an
	 * uninitialized 'child', and a later iteration can only happen
	 * when a value has been assigned to 'child' in the previous
	 * one.
	 */
```

A rank invariant over a red-black tree, maintained by the other half of
the same header, is not something a path-sensitive checker is going to
reconstruct from one function. The comment exists because somebody
already had this argument.

Nothing to change, and the reason to record it is arithmetic: 31 of the
sweep's findings — more than any single file except `dis_tables.c` and
`linux_socket.c` — are one macro, and every future `RB_GENERATE` added to
the tree will add one more.

`clnt_dg.c:118`, `clnt_vc.c:147`, `linux_compat.c:179`,
`ttm_bo_vm.c:54`, `iommu_gas.c:187`, `evtchn_dev.c:94`, `gntdev.c:112`,
`gntdev.c:246`, `gntdev.c:491`, `autofs.c:157`, `tmpfs_subr.c:2410`,
`g_eli_key_cache.c:98`, `kern_sysctl.c:84`, `vfs_inotify.c:162`,
`if_ovpn.c:223`, `in_mcast.c:3014`, `tcp_log_buf.c:357`,
`in6_mcast.c:100`, `pf.c:229`, `pf_if.c:105`, `pf_ioctl.c:144`,
`pf_norm.c:134`, `pf_norm.c:139`, `pf_ruleset.c:77`, `pf_ruleset.c:78`,
`pf_ruleset.c:79`, `pf_ruleset.c:81`, `pf_table.c:178`,
`clnt_nl.c:157`, `rpctls_impl.c:100`, `vm_phys.c:113`.

(`pf_ruleset.c` has four, `gntdev.c` three and `pf_norm.c` two, because a
file may instantiate the macro more than once.)

### `al_hal_serdes_25g.c` — the eight, cited

The row above works out that `al_serdes_25g_reg_read()` returns `-1`
without writing `*data` for a page its switch does not name, that every
caller here ignores the return and reads the out-parameter, and that the
only unnamed pages are `AL_SRDS_REG_PAGE_2_LANE_2` and `_3_LANE_3` on a
SerDes with two lanes. Sweep 16 reports seven of the eight sites (`:955`
and one neighbour share a line): `al_hal_serdes_25g.c:504`,
`al_hal_serdes_25g.c:536`, `al_hal_serdes_25g.c:938`,
`al_hal_serdes_25g.c:955`, `al_hal_serdes_25g.c:1161`,
`al_hal_serdes_25g.c:1378`, `al_hal_serdes_25g.c:1643`.

## Sweep 17: five fixes land, and the output format changes where a finding sits

| | sweep 16 | sweep 17 |
|---|---|---|
| OK | 7,545 | **7,545** |
| ERROR | 564 | **564** |
| findings (deduplicated) | 1,651 | **1,635** |

`ERROR -> OK` 0 and `OK -> ERROR` 0, which is the first thing this sweep
had to show: it is the first run with `-analyzer-output=plist-multi-file`
instead of `text`, and a change to how the analyser is asked must not
change what compiles. It did not, over 8,109 translation units.

34 findings gone and 18 new, and every one of the 52 is accounted for.

**Fifteen are gone for a reason.** Fourteen are the five fixes above —
`sysctl.c:60`, `:110`, `:114`; `citrus_stdenc.c:137`;
`msgcat.c:246`, `:270`, `:412`, `:419`, `:426`, `:445`, `:453`, `:472`,
`:485`; `pmap_prot2.c:105`; `svm.c:1548`. The fifteenth is
`subr_stats.c`, where the two reports at `:3032` and `:3033` became one
at `:3031`: they are the two arms of one `KASSERT`'s ternary, and the
plist writer anchors a diagnostic at the statement rather than the
sub-expression. One assertion, one finding, which is the better answer.

**Nineteen moved, by one to four lines, and are the same findings.**
Every one pairs with a new report of the same checker in the same file:

| | |
|---|---|
| `umass.c` | `:1440`, `:1495`, `:1951`, `:2007` → `:1439`, `:1494`, `:1950`, `:2006` |
| `icrdma.c` | `:423`, `:447` → `:421`, `:445` |
| `mpi3mr.c` | `:4933` → `:4929` |
| `mpi3mr_cam.c` | `:1845` → `:1843` |
| `t4_sge.c` | `:4366` → `:4365` |
| `hw_channel.c` | `:289` → `:288` |
| `mlx5_en_main.c` | `:2404` → `:2403` |
| `drm_sysctl.c` | `:365` → `:364` |
| `dmu_traverse.c` | `:127` → `:126` |
| `dsl_scan.c` | `:1826` → `:1824` |
| `vdev.c` | `:3563` → `:3562` |
| `zil.c` | `:350` → `:349` |
| `msgcat.c` | `:241` → `:253`, which is this file's own fix moving twelve lines in |

None of the nineteen had been cited, so no citation in this document
broke. `param_premise.py --audit` went 192 → 204 unmatched citations, and
all thirteen of the difference are the fixed findings, cited by the
sections that fixed them.

The ERROR inventory needed no reconciliation: all six shards report
"every ERROR translation unit is on the record", 5 + 24 + 60 + 76 + 35 +
364 = 564.

### What the format bought

Every record now carries the function clang says the finding is in, and
the first thing that made visible was that 31 findings across 25 files
are one `RB_GENERATE` — the section above. `param_premise.py`'s
"attributed to no function" went from 52 to **zero**: the 25 it cannot
find in the text are now a named class, `macro`, rather than a residue.

### `usb_serial.c` — eight calls through a pointer, guarded by a bitmask

`ucom_cfg_line_state()` calls four optional callbacks, twice each:

```c
	if (notch_bits & UCOM_LS_DTR)
		sc->sc_callback->ucom_cfg_set_dtr(sc, ...);
```

and the analyser is right that `ucom_cfg_set_dtr` may be null — a
`struct ucom_callback` fills in only what its driver implements. The
guard is nine lines above, at `usb_serial.c:1065`:

```c
	mask = 0;
	/* compute callback mask */
	if (sc->sc_callback->ucom_cfg_set_dtr)
		mask |= UCOM_LS_DTR;
	if (sc->sc_callback->ucom_cfg_set_rts)
		mask |= UCOM_LS_RTS;
	if (sc->sc_callback->ucom_cfg_set_break)
		mask |= UCOM_LS_BREAK;
	if (sc->sc_callback->ucom_cfg_set_ring)
		mask |= UCOM_LS_RING;

	notch_bits = (sc->sc_pls_set & sc->sc_pls_clr) & mask;
	any_bits = (sc->sc_pls_set | sc->sc_pls_clr) & mask;
```

so `notch_bits & UCOM_LS_DTR` is true only when the pointer is not null.
The invariant is *bit i of `mask` is set exactly when pointer i is
non-null*, carried through two `&` and an `|`, and a path-sensitive
checker does not track a correspondence between a bit position and a
struct member. Eight findings, one bitmask: `usb_serial.c:1089`,
`usb_serial.c:1092`, `usb_serial.c:1095`, `usb_serial.c:1098`,
`usb_serial.c:1103`, `usb_serial.c:1106`, `usb_serial.c:1109`,
`usb_serial.c:1112`.

### `init_main.c:328` — the SYSINIT loop

`(*(sip->func))(sip->udata)` in `mi_startup()`. `sip` walks the `sysinit`
linker set, and every entry in it was written by the `SYSINIT()` macro,
which takes the function as an argument — there is no way to put a null
one in. The analyser sees a pointer loaded out of an array whose contents
the linker supplies, which is exactly the kind of thing it cannot see.

Twelve more findings of the same checker are not covered here:
`citrus_iconv.c:100`, `citrus_mapper.c:188`, `clnt_vc.c:424`,
`ng_parse.c:148`, `ng_parse.c:1445`, `drm_crtc_helper.c:302`,
`subr_scanf.c:531`, `linux_80211_macops.c:586`, `ib_cache.c:599` and two
in a test.

### `md.c` — six, and a three-way branch the loop forgets

`mdstart_malloc()` decides once, before the loop, which of three ways it
will move data (`md.c:677`):

```c
	if (notmapped) {
		m = bp->bio_ma;
		ma_offs = bp->bio_ma_offset;
		dst = NULL;
	} else if (vlist != NULL) {
		ma_offs = bp->bio_ma_offset;
		dst = NULL;
	} else {
		dst = bp->bio_data;
	}
```

and then, inside `while (nsec--)`, every use of `dst` is in the `else` of
the same two tests — `if (notmapped) ... else if (vlist != NULL) ...
else bzero(dst, sc->sectorsize);`. The correspondence is exact and both
tests are on locals the function set itself, which is a correlation the
checker holds at the top of the loop and drops when it widens the loop
state. All six findings are inside it: `md.c:708`, `md.c:719`,
`md.c:731`, `md.c:749`, `md.c:784`, `md.c:801`.

Worth the entry for the shape rather than the file: a guard evaluated
*before* a loop and re-tested *inside* it is not a guard the analyser can
keep, and this tree does it wherever a bio can be unmapped.

### `siena_phy.c` — six, and the short-circuit is the guard

`siena_phy_decode_stats(enp, vmask, esmp, smaskp, stat)` is called twice.
`siena_nic.c:232` passes `esmp` **and** `stat` as `NULL` — it wants only
the mask conversion — and `siena_phy.c:584` passes both for real. The
parameter is even annotated `__in_opt`.

`SIENA_SIMPLE_STAT_SET` (`siena_phy.c:443`) is expanded eleven times and
its guard is

```c
		if ((_stat) != NULL && !EFSYS_MEM_IS_NULL(_esmp)) {
```

with `EFSYS_MEM_IS_NULL(_esmp)` being `((_esmp)->esm_base == NULL)`
(`efsys.h:330`) — a dereference. So the second operand is only evaluated
when `stat != NULL`, and the invariant that makes that safe is
*`esmp == NULL` implies `stat == NULL`*, held at both call sites and
written down nowhere. The analyser is following the order the code
gives it and asking what happens if only one of the two is null.

Six findings, one macro, one short-circuit: `siena_phy.c:495`,
`siena_phy.c:497`, `siena_phy.c:499`, `siena_phy.c:501`,
`siena_phy.c:504`, `siena_phy.c:505`.

Worth noting that the function's own hand-written block at `:482` spells
the test the other way round —
`if (stat != NULL && esmp != NULL && !EFSYS_MEM_IS_NULL(esmp))` — which
does not need the invariant at all. The macro is the one place in the
function that does.

### `ip_fw_sockopt.c:1886` — allocated on a wider condition than it is used

`ipfw_mark_object_kidx(uint32_t *bmask, ...)` indexes `bmask[bidx]` with
no test, and its caller passes `da->bmask`, which `dump_config()`
(`ip_fw_sockopt.c:1956`) leaves `NULL` unless

```c
	if (hdr->flags & (IPFW_CFG_GET_STATIC | IPFW_CFG_GET_STATES))
		da.bmask = bmask = malloc(...);
```

The walk that reaches it runs under `if (hdr->flags &
IPFW_CFG_GET_STATIC)` at `:1985`, and `ip_fw_dynamic.c:2924`'s runs under
`IPFW_CFG_GET_STATES` at `:2001`. Both are members of the pair the
allocation tests, so each use's condition implies the allocation's — the
allocation is guarded by a **superset** of the use's condition, which is
correct and is not a folding a path-sensitive checker does across
`flags & (A|B)` and `flags & A`. A `setsockopt` path with neither flag
allocates nothing and walks nothing.

### `linux_emul.c:167` — a precondition on the first parameter

`linux_proc_init(struct thread *td, struct thread *newtd, bool
init_thread)` is declared in `linux_emul.h` and dereferences `td` at
`:167`. Eleven call sites — `linux_emul.c:279`, `:284`, `:295`,
`linux_fork.c:80`, `:110`, `:177`, `:286` and the rest — every one passes
a real thread; the parameter that is ever `NULL` is the *second*
(`:279`). Exported and caller-constrained, the class `report.py` names in
its header.

### `tcp_syncache.c` — seven, and one opaque call between two asks

`syncache_respond()` declares `struct ip *ip = NULL` (`:1801`) and
`struct ip6_hdr *ip6 = NULL` (`:1809`), fills exactly one of them at
`:1848`/`:1877` under

```c
	if (sc->sc_inc.inc_flags & INC_ISIPV6) {
		ip6 = mtod(m, struct ip6_hdr *);
```

and then, a hundred lines later, asks the same question again to decide
which one to touch — `:1963`, `:1999` and the rest. The two asks are the
same expression and nothing in the function changes it.

Between them is `optlen = tcp_addoptions(&to, (u_char *)(th + 1))`
(`:1956`), a call into another translation unit taking a pointer into
the mbuf. The analyser has to assume it may write anything reachable,
`sc->sc_inc.inc_flags` included, so the second ask is unconstrained by
the first and it explores "IPv6 the first time, IPv4 the second" — where
`ip` is still the `NULL` it was declared as.

Seven findings, one call: `tcp_syncache.c:1964`, `tcp_syncache.c:1967`,
`tcp_syncache.c:2003`, `tcp_syncache.c:2012`, `tcp_syncache.c:2034`,
`tcp_syncache.c:2035`, `tcp_syncache.c:2040`.

The shape is the one to remember, because it is not about this file: a
branch variable read before an opaque call and re-read after it is two
different questions to a path-sensitive checker, and every long function
in this tree that dispatches on an address family does exactly that.

### `pfil.c` — three, and one masked equality

`pfil_link()` allocates its two link records conditionally
(`pfil.c:381`):

```c
	if ((pa->pa_flags & (PFIL_IN | PFIL_UNLINK)) == PFIL_IN)
		in = malloc(sizeof(*in), M_PFIL, M_WAITOK | M_ZERO);
	else
		in = NULL;
	if ((pa->pa_flags & (PFIL_OUT | PFIL_UNLINK)) == PFIL_OUT)
		out = malloc(sizeof(*out), M_PFIL, M_WAITOK | M_ZERO);
	else
		out = NULL;
```

— allocate exactly when this direction is asked for **and** the request
is not an unlink. Then:

* `pfil.c:410` is `if (pa->pa_flags & PFIL_UNLINK) return
  (pfil_unlink(...));`, which returns without freeing either. It cannot
  leak: `PFIL_UNLINK` set means both were `NULL`.
* `pfil.c:432` and `pfil.c:445` dereference `in` and `out` under
  `if (pa->pa_flags & PFIL_IN)` and `if (pa->pa_flags & PFIL_OUT)`,
  which is the first half of each allocation's test; the second half —
  `PFIL_UNLINK` clear — is established by the early return above.

Three findings, one idiom: an allocation guarded by `(flags & (A|B)) ==
A` and a use guarded by `flags & A` with `B` ruled out somewhere else.
Folding those is not something a path-sensitive checker does, and this
is the second file today where the guard is arithmetic on a flag word
rather than a test of the pointer — `ip_fw_sockopt.c:1886` is the other,
and `usb_serial.c`'s eight are a third variant of the same thing.

### `kern_lock.c` — three, and the contract is in the comment

`LOCK_CLASS(ilk)` is `lock_classes[LO_CLASSINDEX(ilk)]`, which reads
`ilk->lo_flags`. All three sites reach it only under `flags &
LK_INTERLOCK`:

* `kern_lock.c:194` — `lockmgr_exit()`, `if (flags & LK_INTERLOCK) {
  class = LOCK_CLASS(ilk); class->lc_unlock(ilk); }`
* `kern_lock.c:272` — `sleeplk()`, `class = (flags & LK_INTERLOCK) ?
  LOCK_CLASS(ilk) : NULL;`
* `kern_lock.c:809` — `lockmgr_xlock_hard()`, the same three lines as
  the first, on the path to a `panic()`

and `sleeplk()`'s own comment (`:258`) states the contract the whole file
runs on:

```c
 * It assumes sleepq_lock held and returns with this one unheld.
 * It also assumes the generic interlock is sane and previously checked.
 * If LK_INTERLOCK is specified the interlock is not reacquired after the
 * sleep.
```

*`LK_INTERLOCK` in the flags means the caller supplied an interlock* is
a `lockmgr()` API contract held at every call site in the tree and
checked at none. Three findings, and the interesting part is that the
invariant is written down — in prose, in the file, ten lines above one
of them.

### `rtsock.c:2497`, `rtsock.c:2548` — an assertion that is not compiled

`sysctl_iflist()` and `sysctl_ifmalist()` reach
`w->w_req->td->td_ucred`, and `w_req` is the `struct sysctl_req *` the
sysctl framework hands a handler. `req->td` is set from a thread
argument in both entry points — `kern_sysctl.c:252` is `req.td =
curthread` and `:2138` is `req.td = td` — and `sysctl_root()`, which
every handler is reached through, says so at `:2364`:

```c
	KASSERT(req->td != NULL, ("sysctl_root(): req->td == NULL"));
```

Without `INVARIANTS` that is `((void)0)`, so the analyser takes the path
the assertion exists to deny. The same shape as `dtrace.c:8232`'s three
and `dis_tables.c`'s sixty-four: an invariant asserted in a form that
compiles to nothing in the configuration the sweep uses.

### `if.c:1763-1765` — `M_ZERO`, and a thirty-line probe that settles it

The table above has carried a row asserting that the analyser does not
model `M_ZERO` since the first sweep that reached `sys/net`. It cited
`sys/net/if.c:1757-1759`, which is a *range*, and the citation reader
matches `file.c:NNN` and `file.c:NNN, NNN` and nothing else — so the row
cited no finding at all, and the three findings it was written about
have been sitting in the unread bucket the whole time under their real
lines. Both halves are now fixed: the lines are `:1763`, `:1764`,
`:1765`, and the claim has been measured rather than asserted.

`ifa_alloc()` is the shape:

```c
	ifa = malloc(size, M_IFADDR, M_ZERO | flags);
	if (ifa == NULL)
		return (NULL);

	if ((ifa->ifa_opackets = counter_u64_alloc(flags)) == NULL)
		goto fail;
	if ((ifa->ifa_ipackets = counter_u64_alloc(flags)) == NULL)
		goto fail;
	...
fail:
	/* free(NULL) is okay */
	counter_u64_free(ifa->ifa_opackets);
	counter_u64_free(ifa->ifa_ipackets);   /* :1763 */
	counter_u64_free(ifa->ifa_obytes);     /* :1764 */
	counter_u64_free(ifa->ifa_ibytes);     /* :1765 */
```

The reported set is the tell. `ifa_opackets` is assigned on *every* path
that reaches `fail:`, and it is the one field of the four that is **not**
reported. The other three are reported at exactly the lines where a
field that may not have been assigned yet is read. The analyser is being
perfectly consistent; it simply does not believe the memory was zeroed.

Asserting that is not the same as knowing it, so here is the probe —
thirty lines, no kernel headers, the tree's `malloc()` prototype and
nothing else:

```c
typedef unsigned long size_t;
struct malloc_type;
#define M_ZERO 0x0100
#define M_NOWAIT 0x0001
void *malloc(size_t size, struct malloc_type *type, int flags);
void sink(unsigned long);

struct s { unsigned long a, b; };
extern struct malloc_type *M_X;

int probe(void)
{
	struct s *p = malloc(sizeof(*p), M_X, M_NOWAIT | M_ZERO);
	if (p == 0)
		return 1;
	sink(p->a);          /* zeroed by M_ZERO; is clang told? */
	return 0;
}
```

```
mzero_probe.c:16:2: warning: 1st function call argument is an
    uninitialized value [core.CallAndMessage]
mzero_probe.c:13:16: note: Uninitialized value stored to field 'a'
```

and the control, which is the same function with the flag removed and an
explicit `memset()` in its place:

```c
	struct s *p = malloc(sizeof(*p), M_X, M_NOWAIT);
	if (p == 0)
		return 1;
	memset(p, 0, sizeof(*p));
	sink(p->a);
```

```
mzero_control.c:18:2: warning: Potential leak of memory pointed to by 'p'
```

One warning in the control, and it is a *different* one. So the analyser
does recognise the name `malloc` as an allocator — it tracks the leak —
and it does honour `memset`. What it has no model for is the third
argument. `M_ZERO` is an ordinary `int` flag to a function whose name it
knows and whose body it has never seen, and there is no reason it would
be anything else.

That is the whole mechanism, and it is worth being precise about how
large the class actually is, because the old row's "this is a large
class in a kernel that zeroes most of what it allocates" was a guess.

Sweep 17 has **481** uncited findings whose message is an uninitialised
or garbage value. Of those, **30**, across **20** files, sit in a
function whose own body names `M_ZERO`. That is the honest upper bound
on this class, and it is an upper bound and not a count, because "the
enclosing function contains an `M_ZERO` allocation" is a proxy for "the
reported value came from an `M_ZERO` allocation" and the two are not the
same question. The next section is the file where they come apart.

### `cardbus_cis.c` — four in an `M_ZERO` function, and not this class

`cardbus_parse_cis()` allocates `tupledata` with `M_WAITOK | M_ZERO`, so
the proxy above claims its four findings. Reading them says otherwise.
All four are `3rd function call argument is an uninitialized value` on

```c
	cardbus_read_tuple_finish(cbdev, child, rid, res);
```

and `rid` has nothing to do with `tupledata`. It is an out-parameter:

```c
	res = cardbus_read_tuple_init(cbdev, child, &start, &rid);
	if (res == NULL) { ... return (ENXIO); }
```

`cardbus_read_tuple_init()` writes `*rid` on its `PCIM_CIS_ASI_BAR*` and
`PCIM_CIS_ASI_ROM` arms, and on the `PCIM_CIS_ASI_CONFIG` arm it does

```c
		/* CIS in PCI config space need no initialization */
		return (CIS_CONFIG_SPACE);
```

which is `(struct resource *)~0UL` — a non-NULL sentinel. So there is a
real path on which the function returns success and leaves `*rid`
untouched, and the caller then passes `rid` by value four times.

The callee is safe: `cardbus_read_tuple_finish()` opens with
`if (res != CIS_CONFIG_SPACE)`, which is exactly the path on which `rid`
was set. But *passing* an indeterminate `int` is itself the read — the
argument is evaluated before the callee's guard is anywhere near
running, and `core.CallAndMessage` is right to flag the call site rather
than the use. No supported architecture gives `int` a trap
representation, so nothing goes wrong today; it is still an indeterminate
read in a language that is about to become C++23, where the answer to
"what does reading an uninitialised `int` do" is not one anybody should
want to rely on. `int rid = 0;` costs nothing, and is **fixed**: all four
findings go to 0 at an unchanged flag digest.

Four findings, in a function the proxy picked out, that are the
cross-function out-parameter class and not the `M_ZERO` class at all.
Which is the point of writing the caveat down rather than reporting 30
as though it were a count.

### One from that list is a defect, not a class

The other two findings on that list — `pci_ea_fill_info()`'s pair — are
neither the `M_ZERO` class nor a false positive. They are written up
above the *Not defects* line, where defects go: a three-bit
device-supplied length indexing a four-element stack array.

## The NFS client, read as a parser of a server's replies

`sys/fs/nfsclient/nfs_clrpcops.c` had fifteen findings and is the file
that turns whatever an NFS server sends into kernel data structures, so
it is worth reading rather than counting. Three of the fifteen are
defects, one of them a NULL dereference; reading the two files beside it
turned up three more, including an uninitialised pointer that gets
*stored* rather than merely read.

The threat model is not exotic. A client mounts a server; from then on
every reply is input, and "the server is malicious" is only the strongest
form of "the server is buggy, or another client raced you".

### `nfsrpc_openrpc()`: the Getattr that did not happen

```c
	NFSM_DISSECT(tl, u_int32_t *, 2 * NFSX_UNSIGNED);
	/* If the 2nd element == NFS_OK, the Getattr succeeded. */
	if (*++tl == 0) {
		KASSERT(nd->nd_repstat == 0,
		    ("nfsrpc_openrpc: Getattr repstat"));
		error = nfsv4_loadattr(nd, NULL, &nfsva, ...);
		...
	}
	if (ndp != NULL) {
		if (reclaim != 0 && dp != NULL) {
			...
		} else if (nd->nd_repstat == 0) {
			ndp->nfsdl_change = nfsva.na_filerev;
			ndp->nfsdl_modtime = nfsva.na_mtime;
```

The `KASSERT` states one direction: Getattr status OK implies
`nd_repstat == 0`. Fourteen lines later the code takes the *converse* as
though it had been asserted too. It has not been. The enclosing block is
entered on `nd_repstat == 0`, so on a reply that grants a delegation and
gives the trailing Getattr a non-zero status, `nfsva` — a `struct
nfsvattr` on the stack, written nowhere else in the function — is copied
into the new delegation's change ID and modify time.

The `else` two lines down already sets `NFSCLDL_RECALL`, which is the
right answer when the attributes are unknown. A `gotattr` flag routes
the no-attributes case there. **Fixed.**

### `nfsrpc_statfs()`: one arm has the version guard and the other does not

```c
	} else {
		...
		NFSM_DISSECT(tl, u_int32_t *,
		    NFSX_STATFS(nd->nd_flag & ND_NFSV3));
	}
	if (NFSHASNFSV3(nmp)) {
		sbp->sf_tbytes = fxdr_hyper(tl); tl += 2;
		...
	} else if (NFSHASNFSV4(nmp) == 0) {
		sbp->sf_tsize = fxdr_unsigned(u_int32_t, *tl++);
```

`tl` is `NULL` at the top of the function and is only dissected in the
`else` — the non-NFSv4 arm. The second reader carries
`NFSHASNFSV4(nmp) == 0`, so the author knew NFSv4 had to be excluded;
the first reader does not carry it.

`NFSHASNFSV3` and `NFSHASNFSV4` are two bits of one word, not two values
of one enum:

```c
#define	NFSHASNFSV3(n)		((n)->nm_flag & NFSMNT_NFSV3)
#define	NFSHASNFSV4(n)		((n)->nm_flag & NFSMNT_NFSV4)
```

and `nfs_clvfsops.c` sets each from its own mount option — `:1017` and
`:1019` — with nothing anywhere rejecting the pair. `mount -t nfs -o
nfsv3,nfsv4` therefore produces a mount that takes the NFSv4 branch,
leaves `tl` NULL, and dereferences it. (The `mount -u` path cannot do
it: `:1279` strips both version bits out of the update's arguments and
takes them from the existing mount.) Root-only, so a robustness bug
rather than a privilege boundary — and a NULL dereference in the kernel,
fixed by giving the first arm the guard the second one already has.
The NFSv4 path has already filled `sbp` through `nfsv4_loadattr()`, so
skipping both arms is not just safe but correct. **Fixed.**

### `nfsrv_parselayoutget()`: a debug line one argument out

```c
	error = nfsrv_parseug(nd, 0, &user, &grp, curthread);
	NFSCL_DEBUG(4, "after parseu=%d\n", error);
	if (error == 0)
		error = nfsrv_parseug(nd, 1, &user, &grp, curthread);
	NFSCL_DEBUG(4, "aft parseg=%d\n", grp);
	if (error != 0)
		goto nfsmout;
	NFSCL_DEBUG(4, "user=%d group=%d\n", user, grp);
```

`grp` is `gid_t grp;` with no initialiser, and only the *second*
`nfsrv_parseug()` writes it. The line above prints `error`, the line
below prints `grp` after the error check; this one prints `grp` before
it. It is a typo for `error`, and on the failure path the kernel
formats an uninitialised stack word into `dmesg` — only when
`vfs.nfs.debuglevel` has been raised to 4, but that is a sysctl and not
a compile-time switch. **Fixed.**

### `nfs_lookitup()`: three arms, two of which set `np`

This one the sweep found in `nfs_clvnops.c`, and it is the worst of the
six.

```c
	if (npp && !error) {
		if (*npp != NULL) {
		    np = *npp;
		    ...
		    newvp = NFSTOV(np);
		} else if (NFS_CMPFH(dnp, nfhp->nfh_fh, nfhp->nfh_len)) {
		    free(nfhp, M_NFSFH);
		    VREF(dvp);
		    newvp = dvp;
		} else {
		    error = nfscl_nget(dvp->v_mount, dvp, nfhp, &cn, td,
			&np, LK_EXCLUSIVE);
		    ...
		}
		...
	}
	if (npp && *npp == NULL) {
		if (error) {
			...
		} else
			*npp = np;
	}
```

The middle arm — the server answered the LOOKUP with the directory's
*own* filehandle — sets `newvp` and never touches `np`. Reaching it
requires `*npp == NULL`, which is exactly the condition the tail then
acts on: with `error == 0` it stores `np`, an uninitialised stack
pointer, into the caller's out-parameter. `nfs_symlink()` at `:2436`
and `nfs_mkdir()` at `:2525` immediately do `newvp = NFSTOV(np)` on
it.

That is not a NULL dereference, which faults predictably at address
zero; it is whatever the stack slot last held, dereferenced at
`offsetof(struct nfsnode, n_vnode)`. On that arm `newvp` is `dvp`, so
the node is `dnp` — the value the other two arms would have produced —
and `np = dnp;` is the fix. **Fixed.**

### `nfs_mknodrpc()` and `nfs_createrpc()`: `NFSTOV(NULL)`

```c
	if (!error) {
		if (!nfhp)
			(void) nfsrpc_lookup(dvp, cnp->cn_nameptr, ...);
		if (nfhp)
			error = nfscl_nget(dvp->v_mount, dvp, nfhp, cnp,
			    curthread, &np, LK_EXCLUSIVE);
	}
	...
	if (!error) {
		newvp = NFSTOV(np);
```

`np` is `NULL`-initialised and `NFSTOV(np)` is `(np)->n_vnode`. The
guard on the `nfscl_nget()` is `if (nfhp)`; the guard on the use is
`if (!error)`, and the two are not the same condition, because the
fallback lookup's return is discarded.

The fallback exists because `nfscl_mtofh()` returns 0 with `*nfhpp`
still NULL whenever an NFSv3 server answers "no file handle follows" —
`flag` there, at `nfs_clcomsubs.c:352`, is read straight off the wire:

```c
	if (nd->nd_flag & ND_NFSV3) {
		NFSM_DISSECT(tl, u_int32_t *, NFSX_UNSIGNED);
		flag = fxdr_unsigned(int, *tl);
	}
	...
	if (flag) {
		error = nfsm_getfh(nd, nfhpp);
```

So the sequence is ordinary: NFSv3 CREATE or MKNOD succeeds without a
post-op filehandle, the follow-up LOOKUP fails — another client removing
the entry in between is enough, no malice required — and the client
dereferences NULL. Both functions get `else error = ENOENT;`, which is
what `nfs_symlinkrpc()`'s and `nfs_mkdirrpc()`'s fallbacks already
produce by propagating theirs. **Fixed.**

### `nfs_mount()`: the two allocations `out:` does not free

```c
	tlscertname = malloc(len, M_NEWNFSMNT, M_WAITOK);
	strlcpy(tlscertname, opt, len);
```

`mountnfs()` is the only consumer of `tlscertname` and of `nam`: it
frees them at `:1540` and `:1555`, hangs them off the new `nfsmount`
otherwise, and frees them again on its own `bad:` path at `:1861` and
`:1863`. Between the `malloc` and that call there are **37** `goto out`
for `tlscertname` and **9** for `nam`, and `out:` frees only `hst` and
`dirpath`. Every rejected mount that named a certificate and then
tripped a later option check leaks up to `NAME_MAX - 6` bytes of
`M_NEWNFSMNT` and a `struct sockaddr` of `M_SONAME`.

Clearing both locals immediately after the `mountnfs()` call — the one
point where ownership transfers — and freeing both at `out:` fixes all
46 exits at once without any risk of double-freeing the paths that do
reach `mountnfs()`; `free(NULL, ...)` is a no-op. **Fixed.**

### What the measurement said

An A/B over `sys/fs/nfsclient`, `sys/fs/nfsserver` and `sys/fs/nfs` —
24 translation units, the six edits above and nothing else, flag
digests unchanged:

```
  nfs_clrpcops.c:   OK->OK  findings 15->13
  nfs_clvfsops.c:   OK->OK  findings  1->0
  nfs_clvnops.c:    OK->OK  findings  5->2
  nfs_nfsdsocket.c: OK->OK  findings  5->4
  before (24, 0, 59)  after (24, 0, 52)
  24 units; 4 changed; 0 flag digests changed
```

Seven findings gone, no status moved, and eleven of `nfs_clrpcops.c`'s
fifteen simply shifted by the two lines the `gotattr` declaration adds —
which is why the comparison is by `(file, line, checker)` and prints
both sides.

Two of the seven wanted checking rather than counting.

`nfs_clvnops.c:2432` and `:2521` — `nfs_symlink()` and `nfs_mkdir()`
doing `newvp = NFSTOV(np)` on `nfs_lookitup()`'s out-parameter — are
still reported, at `:2436` and `:2525`, and the reason is the fix
itself. `np = dnp` sets it to `VTONFS(dvp)`, which is
`((struct nfsnode *)(dvp)->v_data)` — a cast of a field the analyser
has no reason to believe is non-NULL. It was an *uninitialised* pointer
being published before; it is a pointer clang cannot prove non-NULL
now. The VFS invariant that a live vnode has `v_data` is not something
a path-sensitive checker can carry.

`nfsrpc_statfs`'s pair is still reported too, at `:5020` and `:5028`.
That one is explainable and worth writing down: the guard now reads
`NFSHASNFSV3(nmp) && NFSHASNFSV4(nmp) == 0`, and the branch that
dissected `tl` was taken on `NFSHASNFSV4(nmp)` being false — the *same*
expression. What sits between the two reads is `nfscl_request()`, a
call into another translation unit that takes `nmp`, so the analyser
must assume `nm_flag` changed. The fix closes the reachable path
(`mount -o nfsv3,nfsv4`); it does not, and cannot, close the analyser's.

**A finding that does not move is not a fix that did not work.** It is
the difference between what the code guarantees and what a
path-sensitive analyser can carry across a call boundary, and the two
were never the same thing.

### And one alignment, which is not a bug today

`nfsrvd_compound()` builds its reply-op-count slot only after

```c
	/* If taglen < 0, there was a parsing error in nfsd_getminorvers(). */
	if (taglen < 0) {
		error = EBADRPC;
		goto nfsmout;
	}
	...
	NFSM_BUILD(retopsp, u_int32_t *, NFSX_UNSIGNED);
```

and writes through `retopsp` at the end under `if (taglen == -1)`'s
`else`. Two spellings of one condition, five hundred lines apart, with
`retopsp = NULL` in between if they ever disagree. They do not:
`nfsd_getminorvers()` forces `taglen = -1` on every error it has. But
it is in another translation unit, and the guard that keeps a remote
NFSv4 client from reaching `*NULL` should not depend on that. The
second spelling is now `taglen < 0`, the same as the first.
## Withdrawn — a dismissal that read the guard with an ellipsis in it

`sys/netinet/tcp_stacks/rack.c`'s `res = lentim / rate_wanted` is in
this document already, in *The rest of rack.c's divisions, read and left
alone*, and the row is wrong. It says:

> `:17352` does `if (((bw_est == 0) || (rate_wanted == 0) || ...)) goto old_method;`

The `...` is not "and some more disjuncts". It is

```c
		if (((bw_est == 0) || (rate_wanted == 0) || (rack->gp_ready == 0)) &&
		    (rack->use_fixed_rate == 0)) {
			goto old_method;
		}
```

— the whole disjunction is **conjoined with `use_fixed_rate == 0`**. A
fixed pacing rate turns the check for a zero divisor off. And a fixed
pacing rate is the one thing that can hand this function a zero:

```c
		if (rack->use_fixed_rate) {
			rate_wanted = bw_est = rack_get_fixed_pacing_bw(rack);
```

`rack_get_fixed_pacing_bw()` returns one of
`rc_fixed_pacing_rate_rec`, `_ss` or `_ca`, and the three
`setsockopt` cases that write them —
`TCP_RACK_PACE_RATE_REC`, `TCP_RACK_PACE_RATE_SS`,
`TCP_RACK_PACE_RATE_CA` — store `optval` with no lower bound at all.
Each also propagates the value to whichever of the other two is still
zero, so one call sets all three.

So, from an unprivileged process:

```c
	setsockopt(fd, IPPROTO_TCP, TCP_FUNCTION_BLK,      "rack", ...);
	setsockopt(fd, IPPROTO_TCP, TCP_RACK_PACE_ALWAYS,  &one,   4);
	setsockopt(fd, IPPROTO_TCP, TCP_RACK_PACE_RATE_CA, &zero,  4);
	write(fd, buf, len);
```

`rc_always_pace` takes `rack_get_pacing_delay()` into the paced arm,
`use_fixed_rate` takes the zero test off, `rack_rate_cap_bw()` cannot
put it back — both of its writes to `*bw` are guarded on `*bw` being
*greater* than the cap — and the division faults. There is no
`priv_check()` anywhere in `rack.c`; `tcp_can_enable_pacing()` is a
global count of pacing sockets, not a check on the caller.

The fix splits the test so the zero is unconditional:

```c
		if (rate_wanted == 0)
			goto old_method;
		if (((bw_est == 0) || (rack->gp_ready == 0)) &&
		    (rack->use_fixed_rate == 0))
			goto old_method;
```

which changes nothing for any non-zero rate.

The row `:2496` in the same table leans on the same sentence — "its
caller rejects zero at `:17352` before dividing" — and inherits the same
error. Both rows are struck rather than edited: the point of the table
is the reasoning, and reasoning that quoted a guard with the operative
half inside an ellipsis is the thing to record, not to tidy away.

**The lesson is mechanical and worth stating as a rule: never elide part
of a condition when writing down why a finding is not a defect.** Quote
the guard entire, or do not quote it. Three other rows in this document
put a `...` inside a quoted `if`; all three were re-read against the
tree and all three hold — `rack.c`'s own `fill_bw` guard (complete at
`:17195`), `rtw88/mac.c`'s `ltecoex_bckp` pair (`:788` and `:807`, the
identical two-term guard on a field of the const chip descriptor), and
`msdosfs`'s complementary `ONETIME` tests. One in four was wrong, which
is exactly the rate at which a shortcut like that should be expected to
be.

## Fixed — `rtprio_thread(2)` hands out two bytes of kernel stack

`sys/kern/kern_resource.c`:

```c
pri_to_rtp(struct thread *td, struct rtprio *rtp)
{

	thread_lock(td);
	switch (PRI_BASE(td->td_pri_class)) {
	case PRI_REALTIME:
		rtp->prio = td->td_base_user_pri - PRI_MIN_REALTIME;
		break;
	case PRI_TIMESHARE:
		rtp->prio = td->td_base_user_pri - PRI_MIN_TIMESHARE;
		break;
	case PRI_IDLE:
		rtp->prio = td->td_base_user_pri - PRI_MIN_IDLE;
		break;
	default:
		break;
	}
	rtp->type = td->td_pri_class;
	thread_unlock(td);
}
```

`type` is written on every path. `prio` is written on three of four.
The classes are `PRI_ITHD` 1, `PRI_REALTIME` 2, `PRI_TIMESHARE` 3,
`PRI_IDLE` 4 (`sys/sys/priority.h:45-48`), so `default:` is exactly one
case: the interrupt thread.

Both callers copy the struct straight out.

`sys_rtprio_thread()`'s `RTP_LOOKUP` declares `struct rtprio rtp;` with
no initialiser, takes `td1 = tdfind(uap->lwpid, -1)` — which resolves a
tid in **any** process — checks `p_cansee()`, and then

```c
		pri_to_rtp(td1, &rtp);
		PROC_UNLOCK(p);
		return (copyout(&rtp, uap->rtp, sizeof(struct rtprio)));
```

`struct rtprio` is two `u_short` (`sys/sys/rtprio.h:73`), so `rtp.prio`
is two bytes of this frame's kernel stack, per call, repeatable.

**Who reaches it depends on `security.bsd.see_other_uids`, and this is
the hardening earning its keep.** `p_cansee()` is what stands between an
unprivileged caller and the kernel's interrupt process, whose tids
`procstat -at` prints for anyone. `kern_prot.c:1870`:

```c
#ifdef PAX_HARDENING
static int	see_other_uids = 0;
#else
static int	see_other_uids = 1;
#endif
```

and `sys/conf/std.hardenedbsd:53` is `options PAX_HARDENING`, which
every HARDENEDBSD config includes. So on a stock FreeBSD kernel this is
an unprivileged kernel-memory disclosure; on a PBSD kernel the default
closes it, and what is left is root reading two bytes of its own
kernel's stack — plus whatever an administrator opens by setting the
sysctl back to 1, which is `CTLFLAG_RW` and is a common compatibility
change.

That is worth stating precisely rather than either way round. The
hardening is not the fix: it is a second door in front of a first one
that should not have been open, and `pri_to_rtp()` writing every field
it is documented to write is the first door.

`sys_rtprio()`'s `RTP_LOOKUP` reaches the same bytes without needing a
tid: it walks `FOREACH_THREAD_IN_PROC` with an uninitialised `struct
rtprio rtp2`, and

```c
			rtp.type = RTP_PRIO_IDLE;	/* 4 */
			rtp.prio = RTP_PRIO_MAX;
			FOREACH_THREAD_IN_PROC(p, tdp) {
				pri_to_rtp(tdp, &rtp2);
				if (rtp2.type <  rtp.type || ...) {
					rtp.type = rtp2.type;
					rtp.prio = rtp2.prio;
```

An interrupt thread reports `type` 1, `1 < 4` is true, and the unwritten
`rtp2.prio` is copied into the struct that `:451` copies out.

The fix is `rtp->prio = 0;` before the switch: every arm that sets it
overwrites it, and the class that has no user priority now says so
rather than saying whatever was on the stack. The other direction of the
same conversion, `rtp_to_pri()`, ends its identical switch with
`default: return (EINVAL);` — the asymmetry is what makes this an
oversight rather than a design.

Found from `core.uninitialized.Assign` at `kern_resource.c:446`. The
analyser named the comparison; the `copyout` two lines further on is
what makes it a disclosure. That distance is the argument for reading
the findings.

## Fixed — the SPD dump's lifetime extension, and two multicast rollbacks

Three more from the same reading, each one `core.uninitialized` and each
one a value that leaves the kernel or steers a tree walk.

`sys/netipsec/key.c`'s `key_setdumpsp()` declares `struct seclifetime
lt;` with no initialiser and fills two of its four fields:

```c
	if (sp->lifetime) {
		lt.addtime = sp->created;
		lt.usetime = sp->lastused;
		m = key_setlifetime(&lt, SADB_EXT_LIFETIME_CURRENT);
```

`key_setlifetime()` copies all four into the `sadb_lifetime` extension,
`allocations` and `bytes` among them — twelve bytes of stack
(`keydb.h:89` is a `u_int32_t` and three `u_int64_t`), twice per policy,
into a message that goes to every PF_KEY listener. Not a privilege
boundary on a stock system, since `key_attach()` requires
`PRIV_NET_RAW`, and not something the sending path should be doing
either way. The same file already writes `memset(&lft_c, 0,
sizeof(lft_c))` at another call site, so the shape was known.

`inm_merge()` and `in6m_merge()` are the same function twice:

```c
	struct ip_msource	*ims, *nims;		/* neither initialised */
	...
	RB_FOREACH(ims, ip_msource_tree, &imf->imf_sources) {
		error = inm_get_source(inm, lims->ims_haddr, 0, &nims);
		++schanged;
		if (error)
			break;
		ims_merge(nims, lims, 0);
	}
	if (error) {
		RB_FOREACH_REVERSE_FROM(ims, ip_msource_tree, nims) {
```

`inm_get_source()` writes `*pims` only on its success path — its
`return (ENOSPC);` at the `in_mcast_maxgrpsrc` limit and its
`return (ENOMEM);` from an `M_NOWAIT` malloc leave it untouched. On any
iteration but the first, `nims` therefore still holds the previous
node, which is exactly what the rollback wants. On the *first*, it
holds the uninitialised stack word, and the red-black tree walk starts
from it. Filling a group to `net.inet.ip.mcast.maxgrpsrc` (512 by
default) and adding one more source is an unprivileged
`setsourcefilter(3)` away.

`nims = NULL` at the declaration and `if (nims == NULL) goto out_reap;`
before the walk. When the first call failed nothing was merged, so
there is nothing to roll back, and `out_reap` is where the loop falls
through to anyway.

### What the measurement said

An A/B over `sys/kern`, `sys/netinet`, `sys/netinet6` and `sys/netipsec`
— 384 translation units, these five edits and nothing else:

```
  kern_resource.c        OK->OK  findings  1->0
  in_mcast.c             OK->OK  findings  2->1
  in6_mcast.c            OK->OK  findings  4->3
  key.c                  OK->OK  findings  2->1
  tcp_stacks/rack.c      OK->OK  findings 10->9
  before (379, 5, 221)  after (379, 5, 216)
  384 units; 5 changed; 0 flag digests changed
```

Five findings, five fixes, one each — and every other movement in the
list is a line shift from the comments the fixes carry: `rack.c`'s nine
by twenty-six lines, `in_mcast.c`'s one by thirteen, `in6_mcast.c`'s one
by four. `rack.c:17427 core.DivideZero`, the one this document had
already dismissed, is gone.
## Fixed — `bt_devinquiry()` frees the cursor, not the array

`lib/libbluetooth/hci.c`. The analyser's wording is the whole finding:

```
Argument to free() is offset by 256 bytes from the start of memory
allocated by calloc()
```

reported three times at each of two sites, for one, two and three
devices. `sizeof(struct bt_devinquiry)` is 256 — `bluetooth.h:157` is a
`bdaddr_t`, four small fields and a `uint8_t data[240]`, padded — so the
offsets are one, two and three of them.

```c
	i = *ii = calloc(num_rsp, sizeof(struct bt_devinquiry));
	...
wait_for_more:
	n = bt_devrecv(s, buf, sizeof(buf), length);
	if (n < 0) {
		free(i);
```

`i` is the cursor. It walks forward once per device the controller
reports (`i ++`, in the `NG_HCI_EVENT_INQUIRY_RESULT` arm) and the
function's own `return (i - *ii)` is what it is for. Control comes back
to the label by `goto wait_for_more`, so after a single device has
answered, both error returns hand `free()` a pointer into the middle of
the block.

It needs no hostile device. `length` is the inquiry timeout passed to
`bt_devrecv()`, and "one device answered, then the read timed out" takes
the first arm. `free()` on a pointer that is not the start of an
allocation is undefined; under jemalloc it aborts or corrupts the arena.

`free(*ii)` at both, and `*ii = NULL` with it, because the function has
already published the pointer to its caller and would otherwise leave it
dangling on an error return. The two `free(i)` *before* the label are
correct and stay: `i` still equals `*ii` there.

## Fixed — three tables, two of which were writable by accident

The analyser cannot fold a lookup table it must assume something else
can write, and two tables in `lib` are non-`const` for no reason anybody
would defend.

`lib/libc/resolv/res_debug.c:747` is `static unsigned int
poweroften[10] = {1, 10, 100, ...}`. Nothing writes it — its three uses
are reads. Because it is not `const`, `core.DivideZero` fires on
`cmval / poweroften[exponent]` for a divisor that is 1 at index 0.

`lib/libdevstat/devstat.c:96` is the other one: `devstat_arg_list[]` is
at file scope, not `static`, not `const`, and named in no header —
`devstat.h` does not mention it, there is no `Symbol.map`, and it
appears nowhere else in the tree. `devstat_compute_statistics()`
switches on `devstat_arg_list[metric].argtype` to choose between
`destu64` and `destld`, then switches on `metric` itself to write
through one of them. That correlation is 53 `core.NullDereference` in
one function, the second densest cluster in the sweep after a
disassembler's opcode tables.

`const` on both, and here the measurement says something I did not
expect.

`poweroften` moved: `res_debug.c:796 core.DivideZero` is gone.
`devstat_arg_list` did not — 54 findings before, 54 after. **The
prediction was wrong, and the reason it was wrong is worth more than the
prediction.** `poweroften[exponent]` is indexed by a value the analyser
has bounded to `0..9` by the loop above it, so once the table is
constant the load folds and the divisor is known. `metric` is a
`va_arg`: it is symbolic, and a load from a *fully constant* table at an
unknown index is still an unknown value. Constness was never what
stood in the way.

The change stays, because it was never only about the findings: a
dispatch table that decides which of two pointers a function writes
through belongs in `.rodata`, where a memory-corruption bug cannot
redirect it. But the 53 remain, and they remain for a reason this
document now records rather than one it guessed at.

## Fixed — four more in `lib`, each one line

`lib/lib80211/lib80211_regdomain.c:583` — `lib80211_regdomain_cleanup()`
has three unlink-and-free loops. The second and third end in `free(cp)`
and `free(fp)`. The first frees the domain's seven band lists and its
name and then drops `dp`; `free(dp)` appears nowhere in the file. Every
domain in `/etc/regdomain.xml` leaked.

The analyser's own finding here is *not* that. It reports "Attempt to
free released memory" at the `free(dp->name)`, because it does not model
`LIST_REMOVE` and so walks the same node twice. The leak is what a
person sees reading the same six lines — which is the argument for
reading them.

`lib/libradius/radlib.c:1528` — `rad_demangle_mppe_key()` checks
`mlen % 16 != SALT_LEN` with `SALT_LEN` 2, and `mlen == 2` passes it.
Then `Clen` is 0, `P = alloca(Clen)` is a zero-sized object, the decrypt
loop never runs, and `*len = *P` reads past it; if that byte is 0 or 1
the two range checks below pass and the `memcpy` reads one byte further
out again and returns it as the decrypted key. `mlen` is the length of
an MS-MPPE-Send-Key attribute in the RADIUS server's reply. The
condition the loop already assumes — at least one cipher block — is now
in the same `if`.

`lib/libc/gen/getpwent.c` (reported at `:1934`, now `:1942`) —
`compat_passwd()` ends with
`if (how == nss_lt_all) st->keynum = keynum;` and three `goto fin` jump
over the only place `keynum` is set. One of them is the ordinary case:
the `getpwent()` call after the last one, where `st->keynum` is already
negative. It was overwriting the enumeration cursor in the thread state
with a stack value, and a non-negative one restarts the walk at an
arbitrary key. The read moves up to just after `compat_getstate()`, so
every early return writes back the value it read.

And then the A/B found a second one under it. With `keynum` defined the
analyser walked one line further and reported
`core.uninitialized.Branch` at the *next* statement:

```c
fin:
	if (how == nss_lt_all)
		st->keynum = keynum;
	if (st->db != NULL && !stayopen) {
```

`stayopen` is set in the same `if (how == nss_lt_all) ... else ...` the
three `goto fin` jump over, so it had the identical bug — deciding
whether to close the password database on an indeterminate value. The
answer was already in the file: `files_passwd()`, the same function one
backend along, declares `int rv, stayopen = 0, *errnop;` at `:805` and
reads it at `:911` in the identical `if (st->db != NULL && !stayopen)`.
One initialised, one not.

This is the third time in this document that **fixing an uninitialised
value has raised what the analyser can see**, and the first time the
thing it then saw was a second defect in the same eight lines rather
than a false positive. A finding count that goes up after that kind of
fix is the instrument reaching further.

`lib/libc/db/btree/bt_delete.c` (reported at `:189` and `:244`, now
`:195` and `:253`) — `__bt_stkacq()`'s two
identical loops set `idx` only on the arm that `break`s. Running off the
top of the stack instead leaves `idx` unset and `h` already
`mpool_put()`, and the restore loop then subscripts that page at an
undefined index. On a consistent tree it cannot happen — popping every
level means the leaf is the rightmost, and `h->nextpg == P_INVALID`
broke the outer loop before this one was entered — but `db(3)` is handed
files it did not write. `if (parent == NULL) return (1);` after each,
which is the error return the two `mpool_get` failures above already
use.

## Fixed — `build_iovec_argf()` takes a printf format and never said so

`lib/libutil/mntopts.h:111`:

```c
void build_iovec_argf(struct iovec **iov, int *iovlen, const char *name,
	const char *fmt, ...);
```

no `__printflike`, and `mntopts.c` hands `fmt` straight to `vsnprintf()`.
The declaration two lines above it, `chkdoreload()`, carries
`__printflike(1,2)` on its callback — so the file knows the annotation
exists.

Three call sites pass a runtime string in that position, all in
`mount_msdosfs(8)`:

```c
:116	build_iovec_argf(&iov, &iovlen, "cs_local", quirk);
:122	build_iovec_argf(&iov, &iovlen, "cs_dos", cs_dos, (size_t)-1);
:313	build_iovec_argf(iov, iovlen, "cs_dos", cs_local);
```

`cs_dos` is `strdup(optarg)` from `-D`, with nothing between the getopt
case and the call, so `mount_msdosfs -D '%n' ...` formats the user's
string against an empty `va_list`. `quirk` is
`kiconv_quirkcs(csp + 1, ...)`, and `lib/libkiconv/quirks.c:132` returns
`base` unchanged when no quirk matches — the text after the `.` in `-L`
— though that path needs `setlocale()` to accept the string first, which
for an arbitrary one means pointing `PATH_LOCALE` at a directory of your
own. `cs_local` at `:313` is `strdup(quirk)`.

`mount_msdosfs` is not installed setuid, so the immediate exposure is a
user corrupting their own process. Which is why the annotation matters
more than the three call sites: it is what stops the next caller being
one that runs with privilege. (The `(size_t)-1` at `:122` and the one at
`:90` are arguments the varargs never reads; both go.)

While in the file: `free_iovec()` is documented as "Free the iovec and
reset to NULL with zero length. Useful for calling nmount in a loop" and
does not do the reset, so the documented loop hands the next
`build_iovec()` a freed pointer to `realloc()`. Every in-tree caller
happens to reset the variables itself. Two lines make the code match its
own comment.

That one **relocated** a finding rather than removing it, and the new
place is the truthful one. `mntopts.c:259` — a leak of `iov` reported in
`chkdoreload()` — is gone, because the pointer is nulled now. In its
place is `mntopts.c:328`, in `free_iovec()` itself, and that is a real
leak the reset made visible: the loop frees `(*iov)[i].iov_base` for
**even** `i` only, so the option *names* `build_iovec()` strdup'd are
freed and the *values* are not. For every caller that is right — the
values are borrowed, `mntp->f_mntonname` and an `errmsg[]` on the stack
— and for `build_iovec_argf()`, which passes `strdup(val)`, it is a leak
by construction. Every caller of that is a `mount_*` helper that exits
seconds later, so nothing is going to be restructured for it; the
finding is the honest record of an API whose ownership rule is "the
caller owns the value, except when it doesn't".

## Four generated headers, and the seventeen files that could not be read

Four userland directories were entirely dark. Every translation unit in
them was ERROR on a header the build makes and the sweep did not, and an
ERROR unit reports zero findings and is indistinguishable from a clean
one — which is the failure mode this document keeps coming back to.

The four headers, each recipe taken from the directory's own Makefile
rather than reconstructed from what the header looks like:

| header | who makes it | how |
|---|---|---|
| `usr.bin/localedef/parser.h` | `bsd.prog.mk`'s `.y.c` rule | `yacc -d`, which writes the header beside the `.c`; the Makefile then says `${SRCS:M*.c}: parser.h`, and nine of the ten sources include it |
| `usr.bin/netstat/nl_defs.h` | its own Makefile | an `awk` over `nlist_symbols` emitting `#define N<SYMBOL> <i>` |
| `sbin/route/keywords.h` | its own Makefile | `LC_ALL=C awk` over `keywords`, emitting *both* a `#define K_<NAME>` and a table row per line, which is why `route.c` includes it twice under different macros |
| `usr.bin/getaddrinfo/tables.h` | its own Makefile | `LC_ALL=C awk -f tables.awk` over `sys/sys/socket.h` |

Measured in one run over `bin`, `sbin`, `usr.bin` and `usr.sbin`:

    before (1754 OK, 108 ERROR, 716 findings)
    after  (1771 OK,  91 ERROR, 739 findings)
    1862 units; 17 changed, every one ERROR->OK
    32 flag digests changed, all inside the four directories

Nothing regressed and no finding moved in a file outside the four. What
the seventeen newly-readable units say, read one by one:

**Twenty of the twenty-three are the red-black tree class already on the
record.** `usr.bin/localedef/collate.c` has eight `core.NullDereference`
at `:365`, `:378`, `:391`, `:404`, `:417`, `:428`, `:439` and `:452`, and
every one of those lines is a bare

    RB_GENERATE_STATIC(weights, weight, entry, weight_compare);

— the finding is inside the tree code the macro pastes in, not in
anything `collate.c` wrote. Its four `unix.Malloc` at `:960`, `:998`,
`:1019` and `:1048` are the other half of the same gap: a node is
`calloc`'d and handed to `RB_INSERT`, the checker does not model
`RB_INSERT` as taking ownership, and the `return` after it reads as a
leak. `charmap.c` (`:61`, `:62`, `:274`, `:288`) and `ctype.c` (`:88`,
`:290`) are the same two shapes in the same proportions. Nothing to fix
in any of them; they are what the checker says about a macro it cannot
see through.

**`sys/netlink/netlink_snl.h:457`, reached through
`usr.bin/netstat/route_netlink.c`,** is `find_parser()`'s

    if (key < ps[0].type || key > ps[pslen - 1].type)

with `ps` NULL. `find_parser` is a `static` in a header, so the analyser
takes it as an entry point of its own and gets to choose the arguments;
every in-tree caller passes `&parser->np[0]` from a statically
initialised table. An artefact of analysing a static function as a root.

**Two are real, and both are in files nothing had ever read.** They get
their own sections below.

## `netstat -c` on a non-TCP protocol prints a column width nobody set

`usr.bin/netstat/inet.c` has never been read by this sweep — every unit
in the directory was ERROR on `nl_defs.h` — and the first thing it says
is `core.CallAndMessage` at `inet.c:363` and `:367`.

`protopr()` declares the two column widths uninitialised at `:200`

    int fnamelen, cnamelen;

and computes them at `:241`

    if (istcp && (cflag || Cflag)) {
        fnamelen = strlen("Stack");
        cnamelen = strlen("CC");
        ...
            fnamelen = max(fnamelen, (int)strlen(tp->xt_stack));
            cnamelen = max(cnamelen, (int)strlen(tp->xt_cc));
    }

`istcp` is set only in the `case IPPROTO_TCP:` arm of the switch at the
top. So on a UDP, UDP-Lite or divert pass the two stay uninitialised.

The **per-connection** uses know that. At `:542` they sit inside

    if (istcp) {
        if (cflag)
            xo_emit(" {t:stack/%-*.*s}", fnamelen, fnamelen, tp->xt_stack);
        if (Cflag)
            xo_emit(" {t:cc/%-*.*s} ...", cnamelen, cnamelen, tp->xt_cc, ...);

The **header** uses do not:

    362:  if (cflag) {
    363:      xo_emit(" {T:/%-*.*s}",
    364:          fnamelen, fnamelen, "Stack");
    365:  }
    366:  if (Cflag)
    367:      xo_emit(" {T:/%-*.*s} {T:/%10.10s}" ... , cnamelen,
    370:          cnamelen, "CC", ...

and that block is under `if (first)`, guarded by nothing but the flag.
`cflag` and `Cflag` are plain globals set by `-c` and `-C` in `main.c`
with no protocol test, and `netstat -a` walks the protocol table calling
`protopr()` for TCP *and* UDP. So `netstat -a -c` prints its UDP header
with an indeterminate `int` as both the field width and the precision of
a `%-*.*s`. What comes out is whatever was on the stack: a huge width
makes libxo pad a five-character string out to it, a negative one flips
the justification and drops the precision. It is not a memory-safety
bug and `netstat` is not setuid, but it is a read of an indeterminate
value passed straight to a formatter, and the output is garbage.

The fix is to make the header agree with the data it is a header for.
The columns exist only when `istcp`, because only the `istcp` branch
ever fills a cell under them:

    -			if (cflag) {
    +			if (istcp && cflag) {
     				xo_emit(" {T:/%-*.*s}",
     					fnamelen, fnamelen, "Stack");
     			}
    -			if (Cflag)
    +			if (istcp && Cflag)

which is the same predicate `:542` already uses, one screen up.

## `fiboptlist_range()` reads `fib[1]` if its argument has no range in it

`sbin/route/route.c` was ERROR on `keywords.h`, so this is also a first
reading. `core.UndefinedBinaryOperatorResult` at `route.c:339`:

    int fib[2], i, error;
    ...
    while ((token = strsep(&str, "-")) != NULL) {
        switch (i) {
        case 0:
        case 1:
            ... fib[i] = strtol(token, &endptr, 0); ...
        default:
            error = 1;
        }
        ...
        i++;
    }
    if (fib[0] >= fib[1]) {

One iteration of the loop leaves `fib[1]` unwritten and the test at
`:339` reads it.

This one does **not** reach a shipped path. `fiboptlist_range` is
`static`, declared at `:150`, and called from exactly one place —
`fiboptlist_csv` at `:389`, under

    if (*token != '-' && strchr(token, '-') != NULL)

so the token is guaranteed to contain a `-` that is not its first
character, `strsep` on `"-"` therefore yields at least two fields, and
`fib[1]` is always written. The finding is the analyser doing what it
does with a `static` function: taking it as an entry point of its own
and choosing `arg` freely. `"5"` is a legal argument to the function's
*signature* and an illegal one to its *contract*, and nothing in the
function says so.

That is worth closing anyway, because the contract lives in the caller
and the next caller will not know it. The loop already counts its
fields; requiring two is one line and turns an indeterminate read into
the error the caller would want:

    	if (error)
    		goto fiboptlist_range_ret;
    	i++;
    }
    +	if (i != 2) {
    +		/* Not "lo-hi": one end of the range is missing. */
    +		error = 1;
    +		goto fiboptlist_range_ret;
    +	}
    if (fib[0] >= fib[1]) {

Left as a finding until it is measured; both fixes go in one batch.

## The C++ standard library was never on the sweep's include path, and five landed ports do not compile without it

`-nostdinc` leaves a `.cpp` with no C++ standard library at all. Thirteen
translation units under `usr.bin/clang` were ERROR on

    llvm/ADT/ADL.h:12:10: fatal error: 'type_traits' file not found

— the shape this document keeps naming: a unit that reports zero findings
because it never compiled.

`lib/libc++/Makefile` says where the headers are. It installs
`${SRCTOP}/contrib/llvm-project/libcxx/include` into
`${INCLUDEDIR}/c++/v${SHLIB_MAJOR}`, and four files CMake generates
upstream are checked in beside the Makefile and installed into the same
directory:

    STD+=  ${.CURDIR}/__assertion_handler   # as of libc++ 18
    STD+=  ${.CURDIR}/__config_site         # as of libc++ 13
    STD+=  ${.CURDIR}/libcxx.imp            # as of libc++ 19
    STD+=  ${.CURDIR}/module.modulemap      # as of libc++ 21

So a `.cpp` now gets a shim directory of those four plus
`libcxx/include`, and gets them **before** every C header directory, not
after. That ordering is not a preference; libc++ checks it and says so:

    <cstddef> tried including <stddef.h> but didn't find libc++'s
    <stddef.h> header. ... The header search paths should contain the
    C++ Standard Library headers before any C Standard Library

`tools/verify/test_includes.py` gains five checks and a sentinel that
compiles a real driver stub. All six were confirmed to fail with
`libcxx_shim()` returning nothing, and the ordering check plus the
sentinel were confirmed to fail again with the pair present but appended
last instead of first — the subtler of the two mistakes, and the one a
presence-only check would have waved through.

### What it found on the way in

Measured over `lib`, 2,169 units:

    before (2089 OK, 80 ERROR, 392 findings)
    after  (2085 OK, 84 ERROR, 436 findings)

`lib/clang/liblldb/LLDBWrapLua.cpp` went ERROR->OK and brought 45
findings with it — a 90,000-line SWIG-generated Lua binding nothing had
ever read. **And five units went the other way**, which is the thing
worth writing down:

    lib/msun/src/s_llround.cpp:  OK->ERROR
    lib/msun/src/s_llroundf.cpp: OK->ERROR
    lib/msun/src/s_llroundl.cpp: OK->ERROR
    lib/msun/src/s_lroundf.cpp:  OK->ERROR
    lib/msun/src/s_lroundl.cpp:  OK->ERROR

Each of the five is a five-line wrapper that parameterises a shared body
by macro and includes it:

    #define type		long double
    #define roundit		roundl
    #define dtype		long
    ...
    #include "s_lround.c"

`s_lround.c:31` then does `#include <math.h>`. Under C that is FreeBSD's
`math.h` and `type` collides with nothing. Under C++ it is libc++'s
`math.h`, which reaches `<__math/abs.h>` and `<__type_traits/enable_if.h>`,
where the line is

    typedef _Tp type;

and the macro rewrites it:

    enable_if.h:29:15: error: cannot combine with previous 'type-name'
        declaration specifier
       29 |   typedef _Tp type;
    s_llround.cpp:1:15: note: expanded from macro 'type'
        1 | #define type            double

**This is not an artefact of the sweep. These five files do not compile
in the shipped build either.** `lib/msun/Makefile:86` and `:88` name
`s_llround.cpp s_llroundf.cpp s_llroundl.cpp` and `s_lroundf.cpp
s_lroundl.cpp` in `COMMON_SRCS`; `bsd.lib.mk` compiles a `.cpp` with
`${CXX} ${CXXFLAGS}`, and `CXXFLAGS+= -D_LIBCPP_HARDENING_MODE=...` two
lines up settles which C++ library that is. The port landed in
`be4ec29f4` ("A hundred more of lib/msun compiled as C++") three days
ago and nothing in the tree could see it: the two gates that compile
every port as C++ — `tools/check_port_symbols.py` and
`tools/check_port_cxx_warnings.py` — both build their command line from
`include_flags()`, so they had no libc++ either and were comparing a
C++ compile that had never included a C++ header.

The fix is the macro's name. `type` is a plain lowercase identifier in
the C++ standard library's own namespace; `dtype` beside it is not, and
neither is `roundit` or `fn`. Renaming `type` to `ftype` across
`s_lround.c` and its five includers — the complete set, since nothing
else includes that body — leaves the generated code identical and the
collision gone. Measured over `lib/msun`, 321 units:

    before (311 OK, 10 ERROR, 20 findings)
    after  (316 OK,  5 ERROR, 20 findings)
    5 changed, every one ERROR->OK; 0 flag digests changed

The `lrint` family beside it — `s_lrint.c`, `s_lrintf.c`, `s_lrintl.c`,
`s_llrint.c`, `s_llrintf.c`, `s_llrintl.c` — has the same
`#define type` shape and is still C, so it compiles today and will hit
this wall the moment it is ported. Nothing else in the tree defines a
bare `type` macro in a `.cpp`.

### The gate that should have caught it, and now does

`tools/check_port_cxx_warnings.py` compiles every landed port twice — as
C17 and as C++23 — and reports any warning flag the C++ side raises that
the C side does not. It is the right differential and it was blind to
this, for a reason worth stating plainly:

    FLAG_RE = re.compile(r"\[-W([a-z0-9-]+)\]")
    ...
    return set(FLAG_RE.findall(p.stderr)), p.stderr

It reads the compiler's **stderr for warning tags** and throws away the
compiler's **exit status**. A hard error carries no `[-W...]` tag, so
`s_lroundl.cpp` raised the empty set as C++, matched its C original's
empty set, and passed. The check printed

    122 ported .cpp compared against the same source as C

      no port raises a diagnostic its C original does not.

for three days about a file that does not build. Same shape as the
sweep's own ERROR problem, one tool over.

`flags_raised()` now returns a third value — whether the compile
succeeded — and a port that compiles as C and not as C++ is a finding of
its own, reported before the warning differential and failing `--gate`
on its own line. Verified by reverting one of the five renames with the
new check in place:

    lib/msun/src/s_lroundl.cpp
        does NOT COMPILE as C++, and does as C:
        error: cannot combine with previous 'type-name' declaration specifier

    FAIL 1 port(s) do not compile as C++ at all.
    exit=1

and green again with the rename restored. The check was also not run by
any workflow; it is now a step in the `lints` job, which gains `clang`
alongside `bmake` for it.

### `LLDBWrapLua.cpp`, read for the first time

The one unit the change turned ERROR->OK in `lib` is
`lib/clang/liblldb/LLDBWrapLua.cpp`: 2.8 MB of SWIG output, checked into
the tree, named by `lib/clang/liblldb/Makefile:343` in `SRCS` — so it
ships — and regenerated by the rule at `:830`:

    # this directory to generate generate LLDBWrapLua.cpp, and commit the
    # result.
        ... -o ${.CURDIR}/LLDBWrapLua.cpp ${LLDB_SRCS}/bindings/lua.swig

It reports 45 findings in two shapes, and both are upstream's, not this
tree's — a fix here would be overwritten by the next `swig` run, so what
follows is the record rather than a patch.

**Twenty-eight `unix.Malloc`**, all the same typemap. The list-of-numbers
argument converter at `:19481` and its twenty-seven copies do

    arg3 = (uint64_t *)malloc((arg4) * sizeof(uint64_t));
    int i = 0, j = 0;
    while (i++ < arg4) {
      lua_rawgeti(L, 3, i);
      if (!lua_isnumber(L, -1)) {
        lua_pop(L, 1);
        return luaL_error(L, "List should only contain numbers");
      }

`luaL_error` does not return — it `longjmp`s out — so `arg3` leaks once
per non-numeric element in a list passed to any of those twenty-eight
entry points from an lldb Lua script. It belongs to the typemap in
`lldb/bindings/lua/lua-typemaps.swig`.

**Seventeen in the `__repr__` extensions and the argument converters.**
`:3720` is the shape:

    SWIGINTERN std::string lldb_SBWatchpoint___repr__(lldb::SBWatchpoint *self){
      lldb::SBStream stream;
      self->GetDescription (stream, lldb::eDescriptionLevelVerbose);
      const char *desc = stream.GetData();
      size_t desc_len = stream.GetSize();
      ...
      return std::string(desc, desc_len);

`SBStream::GetData()` returns `NULL` for an empty stream, and
`std::string(nullptr, 0)` is undefined however harmless it looks. The
rest of the cluster at `:81831`-`:82358` is the `SWIG_ConvertPtr` /
`SWIG_fail_ptr` pattern, where the analyser cannot see that the failure
macro leaves the block.

Nothing here is PBSD's to change. It is on the record because 2.8 MB of
shipped code had never been compiled by this sweep at all.

## The bsnmpd modules, whose headers come out of a tool the build builds first

Twenty-five of the remaining progs ERRORs were seven directories under
`usr.sbin/bsnmpd/modules`, each on the same shape:

    hostres_begemot.c:34:10: fatal error: 'hostres_oid.h' file not found
    bridge_addrs.c:51:10:    fatal error: 'bridge_tree.h' file not found

`share/mk/bsd.snmpmod.mk` is the whole recipe, and it is not a header
substitution but a *program*:

    SRCS+=              ${MOD}_oid.h ${MOD}_tree.c ${MOD}_tree.h
    CFLAGS+=            -I.
    GENSNMPTREEFLAGS+=  -I${SHAREDIR}/snmpdefs

    ${MOD}_oid.h: ${MOD}_tree.def ${EXTRAMIBDEFS} ${EXTRAMIBSYMS}
        cat ${.ALLSRC} | gensnmptree ${GENSNMPTREEFLAGS} -e ${XSYM} \
            > ${.TARGET}
    ${MOD}_tree.c: ${MOD}_tree.def ${EXTRAMIBDEFS}
        cat ${.ALLSRC} | gensnmptree -f ${GENSNMPTREEFLAGS} -p ${MOD}_

`gensnmptree` is `usr.sbin/bsnmpd/gensnmptree`, built from one contrib
source, so the sweep builds it too — with the host's `cc`, plus the two
things a FreeBSD host would have supplied and this one does not:
`-DHAVE_ERR_H`, because `contrib/bsnmp/lib/support.h:39` declares its own
`err()`/`warn()` with `__printflike` when the host has no `<err.h>`, and
a `-include` shim defining `__dead2`, `__unused` and `__printflike` away,
because those come from FreeBSD's `<sys/cdefs.h>` and this compile uses
the host's. `@functools.lru_cache` keeps it to one build per process.

Three details the Makefiles settle, each of which is a wrong answer if
guessed:

**`XSYM` spans continuations.** `snmp_hostres/Makefile:51` is fifty-two
symbol names over eleven backslash-continued lines. A reader that takes
only the first line produces an `oid.h` with four `#define`s instead of
fifty-two — 510 bytes where the right answer is 6,876 — and every file
that wanted one of the missing forty-eight still fails, on a *different*
missing name. `_make_var()` joins continuations, and the output is
byte-identical to the reference generated by hand.

**`EXTRAMIBDEFS` and `EXTRAMIBSYMS` are set by nothing.** They appear in
the rules and in no Makefile in the tree, so `${.ALLSRC}` is the single
`.def` file in both cases.

**The `-I` is for a real `include`, and only in three modules.**
`contrib/bsnmp/snmp_{target,usm,vacm}/*_tree.def:31` is

    include "tc.def"

and those three Makefiles add `-I${CONTRIB}/lib`, where `tc.def` is.
`snmp_bridge` and `snmp_wlan` have

    #include "tc.def"

with a `#`, and `gensnmptree.c:538` treats `#` as a comment to end of
line — so that line is a comment and those two need nothing. The `-I` is
passed to all eleven because `contrib/bsnmp/lib` is the only place
`tc.def` lives, and passing it where it is unused changes no output.

All eleven modules generate: the seven in `usr.sbin/bsnmpd/modules` and
the four whose sources live in `contrib/bsnmp/snmp_*` and are reached
through `.PATH`, which is why the `.def` is looked for in both places.

### What the twenty-five newly-readable bsnmpd files said

Seven findings, in a daemon that runs as root and answers the network.
Five are real; two are the checker being wrong about `sizeof`.

**`hostres_fs_tbl.c:353` — the one table in the module that forgot to
zero its entry.** `fs_tbl_process_statfs_entry()` does

    entry->flags |= HR_FS_FOUND;

on what `fs_entry_create()` returns, and `fs_entry_create()` at `:198`
is

    if ((entry = malloc(sizeof(*entry))) == NULL) {
        syslog(LOG_WARNING, "%s: %m", __func__);
        return (NULL);
    }

    if ((entry->mountPoint = strdup(name)) == NULL) {

`flags` is never assigned before that `|=`, so the compound assignment
reads indeterminate heap. That matters beyond the read itself:
`HR_FS_FOUND` is the bit `fs_tbl` uses to decide which entries survive a
refresh, so a garbage `flags` decides whether a filesystem stays in the
table or is deleted from it.

This is not a judgement call about house style. Every one of the other
**nine** `*_entry_create()` functions in the same module — device,
diskstorage, network, partition, printer, processor, storage,
swinstalled, swrun — has exactly the line this one is missing:

    if ((entry = malloc(sizeof(*entry))) == NULL) { ... }
    memset(entry, 0, sizeof(*entry));

Nine of ten. The fix is the tenth copy of that line.

**`hostres_processor_tbl.c:300` — a zero-length VLA.**
`refresh_processor_tbl()` opens with

    long pcpu_cp_times[cplen];
    memset(pcpu_cp_times, 0, sizeof(pcpu_cp_times));

and `cplen` is a file-scope `size_t` that `init_processor_tbl()` sets to
`0` on **both** failure arms of its `kern.cp_times` lookup — the
`sysctlnametomib` failure at `:261` and the length-query failure at
`:264`. A VLA whose size is zero is undefined (C11 6.7.6.2p5). Nothing
dangerous follows it here, because `cpmib` is `{0, 0}` on that path and
the `sysctl` call fails with `EINVAL` rather than `ENOMEM`, so the
function logs and returns — but it declares the array first. `cplen ==
0` means there is nothing to refresh, and saying so before the
declaration is one `if`.

**`hast_snmp.c:364` — a leak per resource in an error state, per
refresh, forever.** `update_resources()` allocates one
`struct hast_snmp_resource` per resource and links it into the list at
the *bottom* of the loop:

    res = calloc(1, sizeof(*res));
    ...
    error = nv_get_int16(nvout, "error%u", i);
    if (error != 0)
        continue;              /* res is dropped here */
    ...
    TAILQ_INSERT_TAIL(&resources, res, link);

So a resource reporting an error costs one `res` on every call, and
`update_resources()` is called from the table handler on every SNMP poll
past `UPDATE_INTERVAL`. `bsnmpd` is a long-running daemon; this is
unbounded. While in the function: the `calloc`-failure arm two lines
above returns `-1` without `nv_free(nvout)`, which is the same shape on
the OOM path. Both closed.

**`pf_snmp.c:1267` — a leak per non-leaf ALTQ queue, per refresh.**
`pfq_refresh()` allocates before it knows whether it will keep:

    e = malloc(sizeof(struct pfq_entry));
    ...
    if (pa.altq.qid > 0) {
        ...
        INSERT_OBJECT_INT_LINK_INDEX(e, &pfq_table, link, index);
    }
    }

Only leaf queues carry a `qid`; the parent disciplines have `qid == 0`,
are not inserted, and are overwritten by the next iteration's `malloc`.
Any system with an ALTQ hierarchy leaks one entry per parent per
refresh. An `else free(e);` closes it.

**`wlan_snmp.c:4064` — an out-parameter that is not written on every
return.** `wlan_get_acl_mac()` writes `*wif` only once it reaches
`wlan_find_interface()`:

    if (wlan_mac_index_decode(oid, sub, wname, mac) < 0)
        return (NULL);
    if ((*wif = wlan_find_interface(wname)) == NULL)
        return (NULL);

and `wlan_acl_mac_set_status()` is the one caller that keeps going after
a `NULL` result — `RowStatus_createAndGo` *wants* `macl == NULL` — and
then reads `wif == NULL`. Today the caller happens to have called
`wlan_mac_index_decode` itself, with the same arguments, and returned on
failure, so the unwritten path is unreachable. That is the
`fiboptlist_range()` shape again: a contract that lives in the caller
and nowhere in the callee. `*wif = NULL` as the first statement of
`wlan_get_acl_mac()` — and of `wlan_get_next_acl_mac()`, which
short-circuits the same way at `:4021` — makes it true of the function
rather than of one of its callers.

**Two are the checker, not the code.** `hast_snmp.c:125` and
`hostres_swinstalled_tbl.c:426` are `unix.MallocSizeof` on

    cfgpath = malloc(sizeof(HAST_CONFIG));
    ...
    strcpy(cfgpath, HAST_CONFIG);

`sizeof` of a string literal is its length plus the NUL, which is
exactly what the following `strcpy` needs. The checker compares the
pointee type (`char`, size 1) against the `sizeof` operand's type
(`char[N]`) and calls the mismatch suspicious. Here it is the correct
allocation, written the way that cannot drift if the macro changes.
Left alone.

## rpcgen was the host's, and the host's writes a different header

`rpc.statd`, `rpc.lockd` and `bootparamd` were ERROR on an identifier
that nothing in the tree declares:

    statd.c:565:  use of undeclared identifier 'sm_prog_1'
    lockd.c:296:  use of undeclared identifier 'nlm_prog_4'
    main.c:103:   use of undeclared identifier 'bootparamprog_1'

`sm_prog_1` is the RPC **server dispatch** function. `rpc.statd`'s
Makefile generates it into `sm_inter_svc.c` with `rpcgen -m`, and
`statd.c` takes its address:

    svc_register(transp, SM_PROG, SM_VERS, sm_prog_1, 0)

without declaring it anywhere. It does not need to, because
`usr.bin/rpcgen/rpc_hout.c` declares it for them:

    static void
    pdispatch(const char * name, const char *vers)
    {
        f_print(fout, "void ");
        pvname(name, vers);
        f_print(fout, "(struct svc_req *rqstp, SVCXPRT *transp);\n");
    }

called from `pprogramdef()` once per version whenever a header is being
generated. **The host's rpcgen does not emit it.** Diffing the two over
`include/rpcsvc/sm_inter.x`:

    -  #define SM_PROG 100024
    +  #define	SM_PROG ((unsigned long)(100024))
    +  extern  void sm_prog_1(struct svc_req *rqstp, SVCXPRT *transp);
    -  stat_fail = 1,
    +  stat_fail = 1
    -  #if defined(__STDC__) || defined(__cplusplus)   (a K&R fallback)

Three differences, and the middle one is why three daemons could not be
read. The other two matter less but matter: the program number's type is
`unsigned long` in one and `int` in the other, and `svc_register()`
takes it as `rpcprog_t`.

The point is larger than the three files. **Every** rpcgen consumer in
the sweep — the twenty-two interfaces `include/rpcsvc/Makefile` names,
and every directory with its own `.x` — was being compiled against a
header a different implementation wrote. The lesson is the one the
gensnmptree work had just made: the build runs a tool the build builds,
so build that tool.

`rpcgen_tool()` compiles `usr.bin/rpcgen`'s ten sources with the host's
`cc`. Three things the host does not supply: `-include <stdint.h>`,
because `sys/rpc/types.h` uses `uint32_t` and reaches it through
FreeBSD's `<sys/types.h>`; a `-D` for `nitems()`, which is FreeBSD's
`<sys/param.h>`; and the `__dead2`/`__printflike` shim `gensnmptree`
also needed. The only headers it reads are the tree's own —
`sys/rpc/*.h` symlinked as `rpc/`, `include/netconfig.h`, and
`sys/sys/_null.h`, which is pure preprocessor. Nothing is
reimplemented, and a failed build returns `None` so the host's rpcgen
remains the fallback rather than the generator silently writing nothing.

Two more things the directory's own Makefile settles, both of which
were wrong before:

**The flags.** `rpc.statd` has `RPCGEN= ... rpcgen -L -C`;
`rpc.tlsclntd` and `rpc.tlsservd` have `-L -C -M`. `-M` is not
cosmetic — `rpc_hout.c`'s `pprocdef()` writes the multithread-safe
prototypes under it, `bool_t f_2_svc(args, result, struct svc_req *)`
rather than `result *f_2_svc(args, struct svc_req *)`, and both daemons
*define* the `-M` form. Generated without it they were

    error: conflicting types for 'rpctlscd_null_2_svc'

`_rpcgen_flags()` reads the `RPCGEN=` line and takes what it fixes for
every target in the directory, not the per-target `-h -c -m -o`.

**Where the `.x` is.** The generator looked beside the Makefile and in
`include/rpcsvc`. `rpc.tlsclntd`'s is

    RPCSRC=	${SRCTOP}/sys/rpc/rpcsec_tls/rpctlscd.x

which is neither. `_rpcsrc_of()` reads the `RPCSRC=` line, expands
`${SRCTOP}` and the `${SYSROOT:U${DESTDIR}}/usr/include/` form, and only
returns a path whose basename matches the header being generated, so a
Makefile with several is not matched to the wrong one.

Six checks in `tools/verify/test_includes.py`, each confirmed failing:
with `rpcgen_tool()` returning `None`, and with `_rpcgen_flags()`
ignoring the Makefile.

## A flag the component's CFLAGS carried, and the sweep dropped on principle

`usr.sbin/wlanstat/wlanstat.c` was ERROR on

    fatal error: bracket nesting level exceeded maximum of 256
      653 |         case S_RATE:
    wlanstat.c:369:24: note: expanded from macro 'S_RATE'
      369 | #define S_RATE                  AFTER(S_TX_MCAST)
    wlanstat.c:367:27: note: expanded from macro 'S_TX_MCAST'
      367 | #define S_TX_MCAST              AFTER(S_TX_UCAST)

The file numbers about three hundred wireless statistics with a chain of

    #define AFTER(prev)  ((prev)+1)
    #define S_RX_BADVERSION      0
    #define S_RX_ELEM_UNKNOWN    AFTER(S_RX_ELEM_TOOSMALL)
    #define S_RX_BADCHAN         AFTER(S_RX_ELEM_UNKNOWN)
    ...

so the last one expands to three hundred nested parentheses, and clang's
default limit is 256. Its Makefile says so:

    usr.sbin/wlanstat/Makefile:10
        CFLAGS.clang+= -fbracket-depth=512 -Wno-cast-align

`bsd.sys.mk` folds `CFLAGS.${COMPILER_TYPE}` into `CFLAGS`, so bmake was
already handing this flag to `ask_cflags()`. It was thrown away there,
by a rule that is right in general:

    # -f and -m only from the file's own flags, and never the
    # two the analyser cannot accept.
    if w[:2] in ("-I", "-D", "-U") or (
            i >= 2 and w[:2] in ("-f", "-m")
            and not w.startswith(("-fsanitize", "-flto"))):

A component's `CFLAGS` is where `-flto`, `-fsanitize=cfi` and
`-mretpoline` live, and the analyser does not want them; the per-file
`CFLAGS.<name>` is where `-fblocks` lives, and without that a file does
not compile. The rule separates the two by where the flag was written.

That is the wrong axis for two flags. `-fbracket-depth` and `-fblocks`
decide whether the source **parses** — dropping them does not produce a
differently-optimised compile, it produces an ERROR — and both appear in
component `CFLAGS` as well as per-file ones (`-fblocks` four times in
userland). So they are taken from either, by name:

    PARSE_AFFECTING = ("-fbracket-depth", "-fblocks")

Deliberately two entries. Anything that only changes code generation
stays behind the per-file rule, where `-flto` and `-fsanitize` can go on
being excluded by name.

## Two more shapes of "the build does not compile this", and gates for both

`expected_errors.py` could say three things about a file that will not
compile: `NOT_NAMED` (no `SRCS`, no `files*` entry names it),
`NOT_SUBDIR` (its directory has a Makefile but no parent descends into
it), and `INCLUDED_BY:<path>` (it is `#include`d, not compiled). Four of
the remaining nineteen progs ERRORs fit the first:

| file | why |
|---|---|
| `usr.bin/tip/libacu/biz31.c` | `usr.bin/tip/tip/Makefile:41` reaches `../libacu` by `.PATH` and names **nine** of its ten drivers. This is the tenth. |
| `usr.sbin/traceroute/findsaddr-socket.c` | `SRCS` names `findsaddr-udp.c`. |
| `usr.sbin/bhyve/snapshot.c` | `SRCS+=` only inside `.if ${MK_BHYVE_SNAPSHOT} != "no"`, and that option is `__DEFAULT_NO`, so bmake asked with the tree's defaults names nothing. |
| `sbin/veriexec/veriexec.c` | `MK_VERIEXEC` depends on `BEARSSL`, `__DEFAULT_NO`. |

Two did not fit anything, and each needed its own marker.

**`DEFAULT_OFF:<option>`.** `usr.bin/dpv/dpv.c` is named by its own
Makefile — bmake in that directory says so — so `NOT_NAMED` is false.
And `usr.bin/Makefile:194` is

    SUBDIR.${MK_DIALOG}+=	dpv

which the `NOT_SUBDIR` reader counts as a `SUBDIR` line, correctly: it
was written to catch a *commented-out* one. What is true is that
`DIALOG` is in `__DEFAULT_NO_OPTIONS`, so nothing descends into `dpv`
and `contrib/dialog/dialog.h` is on no include path. The marker carries
the option's name so the gate can check both halves: that the parent
really gates that directory on that option, and that `src.opts.mk`
really defaults it to no. Verified by planting `DEFAULT_OFF:KERBEROS`
(rejected — `KERBEROS` is `__DEFAULT_YES`) and by claiming it for
`bin/cat/cat.c` (rejected — `bin/Makefile` does not gate `cat`). The
option reader has its own sentinel: a reader that answered "off" to
everything would make every one of these claims pass.

**`NEEDS_LOCALBASE`.** `usr.sbin/virtual_oss/virtual_equalizer` **is**
built — `MK_CUSE` and `MK_SOUND` are both `__DEFAULT_YES` — and its own
Makefile says where its header comes from:

    CFLAGS+=  -I${SRCTOP}/usr.sbin/virtual_oss/virtual_oss \
              -I/usr/local/include
    LDFLAGS+= -L/usr/local/lib -lm -lfftw3

`/usr/local` is the ports prefix; `<fftw3.h>` is `math/fftw3` and is in
no part of this tree. The gate reads the same Makefile the claim is
about. Verified by claiming it for `bin/cat/cat.c`, which names no port
and is rejected.

## Three more things bmake was already saying and nothing was listening to

Chasing the last nineteen unexplained progs ERRORs turned up three gaps
in how the sweep reads a userland file's flags. None is a guess: in each
case bmake had the answer and the reader threw it away.

### `-include`, and a deduplication that turned a flag into a source file

`ask_cflags()` took `-I`, `-D`, `-U` and — from a file's own
`CFLAGS.<name>` — `-f` and `-m`. `-include <path>` is two words, so it
matched neither test and both halves were dropped.
`usr.sbin/fstyp/Makefile` needs two of them:

    CFLAGS.zfs.c+= -include ${ZFSTOP}/include/os/freebsd/spl/sys/ccompile.h
    CFLAGS.zfs.c+= -include ${SRCTOP}/sys/modules/zfs/zfs_config.h

and without them `zfs.c` is fourteen errors starting inside libspl's own
`<string.h>`, where `extern size_t strlcat(...)` collides with FreeBSD's
`ssp/string.h` macro of the same name. Thirty-two userland Makefiles use
`-include`, in the component form and the per-file form both.

Taking the pair naively made it worse in a way worth recording.
`includes.py` merges bmake's answer into the flag list with

    if f not in seen:

so the *second* `-include` word — identical to the first — was dropped
and its path appended alone. clang reads a bare path as another **source
file**. The fix is to emit the flag joined to its path,
`-include/path/x.h`, which clang accepts and which is one word, so
deduplication does the right thing by construction.

### A file's flags can be written on its program's name, not its own

`sbin/dhclient/tests/fake.c` was ERROR on `'dhcpd.h' file not found`.
The `-I` that finds it exists, and it is spelled the way `bsd.progs.mk`
and `bsd.test.mk` spell things:

    PLAIN_TESTS_C=                     option-domain-search_test
    SRCS.option-domain-search_test=    alloc.c ... fake.c \
                                       option-domain-search.c
    CFLAGS.option-domain-search_test+= -I${.CURDIR:H}

`fake.c` has no `CFLAGS.fake.c`; the flags are on the *program* that
names it. `ask_cflags()` now asks bmake for `PROGS`, `PROGS_CXX`,
`PLAIN_TESTS_C`, `PLAIN_TESTS_CXX`, `ATF_TESTS_C`, `ATF_TESTS_CXX` and
`GTESTS`, then for each one's `SRCS.<prog>`, and adds `CFLAGS.<prog>`
for whichever names this file. Cached on the **directory**: asked per
file that would be two extra bmake runs for each of 1,862 progs units.

### The sweep was reading the Kerberos tree the build does not build

`usr.sbin/gssd/gssd.c` was ERROR on `'krb5.h' file not found`, and
`installed_headers()` — 2,392 entries — had no `krb5.h` in it.
`Makefile.inc1:436` is

    .if ${MK_KERBEROS} != "no"
    .if ${MK_MITKRB5} != "no"
    SUBDIR+=krb5
    .else
    SUBDIR+=kerberos5
    .endif

and `MITKRB5` is `__DEFAULT_YES`. The build descends into the top-level
`krb5/` (MIT) and never into `kerberos5/` (Heimdal). `SCOPES` had it the
other way round: `makefile_dirs()` returned **54** `kerberos5`
directories and **zero** `krb5` ones.

Asking bmake in the Heimdal tree does not even work, which is why this
was silent rather than wrong:

    src.libnames.mk line 952: kerberos5/lib/libkrb5: Missing or incorrect
    _DP_krb5 entry in src.libnames.mk. Should match LIBADD for krb5
    ('asn1 com_err crypt crypto hx509 roken wind heimbase heimipcc' vs
     'krb5profile k5crypto com_err krb5support')

`_DP_krb5` is the MIT dependency set under the default options while
that Makefile's `LIBADD` is the Heimdal one, so bmake `.error`s and
`ask_incs()` returns the empty dict — the shape this document keeps
naming, one level up: a reader that answers "nothing" is
indistinguishable from a directory that installs nothing.

With `krb5` in `SCOPES`, `krb5/include` contributes `krb5.h`,
`gssapi.h`, `kdb.h`, `krad.h` and fourteen more under `krb5/`.
`kerberos5` stays in the list, because a tree built `MK_MITKRB5=no` is a
legal configuration whose headers are then the right ones; it simply
contributes nothing under the defaults.

**`gssd.c` still does not compile,** and now for a reason that is
precisely located rather than "file not found":
`crypto/krb5/src/include/krb5.h` is MIT's compatibility stub, whose
whole body is `#include <krb5/krb5.h>`, and the real
`krb5/krb5.h` is *generated* — `krb5/include/krb5/Makefile` builds it by
`cat`-ing `krb5.hin` together with error-table headers that
`compile_et` makes first. That is a third host tool, after
`gensnmptree` and `rpcgen`, and it is on the list rather than in this
commit.

## Two globals consulted twice, with a call in between

The five `rpc.lockd` findings and the one in `rpc.tlsservd` — the files
the tree's own rpcgen made readable — are one shape.

`lockd.c` assigns `fd` only inside `if (!kernel_lockd)` at `:549`, and
every use of it is inside the same test:

    if (!kernel_lockd)
            close(fd);

`kernel_lockd` is a file-scope `int` at `:85`, written once in `main()`
and never again — but between the two tests are `syslog()`,
`inet_pton()` and `getaddrinfo()`, and nothing tells the analyser a
global survives an opaque call. `rpc.tlsservd.c` is the same with
`gethostret`, assigned under `if (rpctls_verbose)` at `:752` and under
`if (!rpctls_verbose)` at `:767` — every real path writes it — with
`SSL_get_version()`, `SSL_get_cipher()` and
`SSL_get1_peer_certificate()` between them.

Neither is reachable. Both get a one-word initialiser anyway, and in
both cases the value is the one the file's own conventions already use:

  - `int fd = -1`, which is exactly what `lockd.c` stores into `sock_fd[]`
    twenty lines earlier as "invalid for now" and what
    `create_service()` tests with `if (fd < 0) continue`. `close(-1)`
    fails `EBADF`.
  - `int gethostret = 0`, which means "no hostname", leaves `ret` at 0
    at `:803`, and takes the `ret != 1` arm — `RPCTLS_FLAGS_DISABLED`.
    A TLS server that cannot tell whether it got the peer's name should
    fail closed.

## The last two easy generators, and where the ERROR set stands

`bin/csh/iconv_stub.c` was `use of undeclared identifier
'dl_iconv_close_t'` — an error one step past the real problem. It opens
with `#include "iconv.h"`, and `bin/csh/Makefile:103` is

    iconv.h: ${.CURDIR}/iconv_stub.h
            ${CP} ${.CURDIR}/iconv_stub.h ${.TARGET}

so in a real build that quoted include finds a **copy of
`iconv_stub.h`**, which declares `dl_iconv_t` and `dl_iconv_close_t`.
Without the copy it finds the tree's own `include/iconv.h` — a real
header, for the real `iconv(3)`, which declares neither. A wrong header
found is worse than a missing one: the file gets further before it
fails, and the error names a symbol rather than a file.

`usr.sbin/config/mkheaders.c` wanted `y.tab.h`.
`usr.sbin/config/Makefile:6` puts `config.y` **and** `y.tab.h` in
`SRCS`, and `share/mk/bsd.suffixes.mk:74` is

    .y.c:
            ${YACC} ${YFLAGS} ${.IMPSRC}
            mv y.tab.c ${.TARGET}

with `YFLAGS ?= -d` from `sys.mk:273` — so yacc runs in the objdir and
leaves `y.tab.h` beside the `.c` it renames away. No `-o` here, unlike
`localedef`'s generator: `-o` renames the header too, and this one is
wanted under the name yacc gives it by default.

That leaves **two** unexplained progs ERRORs, both waiting on a third
and fourth host tool:

| file | needs |
|---|---|
| `sbin/ifconfig/sfp.c` | `libifconfig_sfp_tables.h`, which `lib/libifconfig/Makefile:31` makes with `${LUA} sfp.lua` over a `.tpl.h` — `flua`, the tree's own Lua 5.3 |
| `usr.sbin/gssd/gssd.c` | `krb5/krb5.h`, which `krb5/include/krb5/Makefile` makes by `cat`-ing `krb5.hin` with error-table headers `compile_et` builds first |

Both are the same shape as `gensnmptree` and `rpcgen`: the build runs a
tool the build builds. Neither is a guess about what the header should
contain.

Where the number has gone, over this run of work:

| | ERROR | unexplained |
|---|---|---|
| at the start | 108 | 83 |
| four generated headers | 91 | 66 |
| the ERROR inventory, twice | 91 | 57 |
| the C++ standard library | 78 | 44 |
| gensnmptree | 53 | 19 |
| the tree's rpcgen, and four flag readings | 44 | 4 |
| these two generators | 42 | 2 |

---

## The premise 72 findings rested on, which nobody had checked

Task #90 had been on the list since sweep 14 with this description:

> Model copyin() for the analyser: it explains 71 findings in one file.

The one file is `sys/compat/linux/linux_socket.c`, and the reasoning
looked sound: `copyin()` has no body the analyser can see, so the buffer
it fills stays undefined, so everything read out of that buffer is an
uninitialised read. clang-18 even has the machinery — `model-path=DIR`
and `faux-bodies` — to hand the analyser a body for exactly this.

**The premise is wrong.** CSA already invalidates a by-pointer argument
passed to an unknown function: it has to, because the callee may have
written through it. A caller that checks `copyin`'s return is clean with
or without a model, and a test with a hand-written model confirmed the
finding set does not move. Four hours of work were queued against a
sentence that had never been tested.

So where do the findings come from? Here, and only here:

```c
	l_ulong a[6];
#if defined(__amd64__) && defined(COMPAT_LINUX32)
	register_t l_args[6];
#endif
	...
	error = copyin(PTRIN(args->args), a, LINUX_ARG_SIZE(args->what));
	if (error != 0)
		return (error);

#if defined(__amd64__) && defined(COMPAT_LINUX32)
	for (int i = 0; i < lxs_args_cnt[args->what]; ++i)
		l_args[i] = a[i];                    /* linux_socket.c:2761 */
	arg = l_args;
#endif
```

The analyser will not fold `lxs_args_cnt[args->what]`, so it cannot
establish that the loop writes as many words of `l_args` as the handler
downstream reads, and it reports every one of those reads. 72 findings,
one loop.

### They are false only because a table is right

`LINUX_ARG_SIZE(x)` is `lxs_args_cnt[x] * sizeof(l_ulong)`, so that
table decides how much of the user's array is copied in — and the
handler it dispatches to decides how much is read back out:

```c
static const unsigned char lxs_args_cnt[] = {
	0 /* unused*/,		3 /* socket */,
	...
	6 /* sendto */,		6 /* recvfrom */,
```

```c
struct linux_sendto_args {
	char s_l_[PADL_(l_int)]; l_int s; char s_r_[PADR_(l_int)];
	char msg_l_[PADL_(l_uintptr_t)]; ...
	char len_l_[PADL_(l_int)]; ...
	char flags_l_[PADL_(l_int)]; ...
	char to_l_[PADL_(l_uintptr_t)]; ...
	char tolen_l_[PADL_(l_int)]; ...
};
```

Nothing in the language connects those two. The table is 21
hand-maintained integers in `linux_socket.c`; eighteen of the structs it
describes are generated into `linux32_proto.h` from `syscalls.master`,
and the other three (`accept`, `send`, `recv`) are written by hand in
`linux_socket.h:129` and `linux_socket.c:1197`/`:1240`. Nobody who edits
one is told about the other.

The failure is quiet and it is one-directional:

| the table says | what happens |
|---|---|
| **more** than the handler reads | more of the user's array is copied than is used — harmless |
| **fewer** than the handler reads | the tail of `a[]` holds whatever the previous frame left on the kernel stack, and the handler reads those words as its last arguments |
| **more than 6** | the copyin overruns a six-word stack buffer outright |

For `linux_getsockopt` the last argument is `optlen`, an out-pointer the
kernel then writes through. A count one too small there is an
unprivileged write to an address the caller never supplied.

### Verified, then made checkable

All 21 were read by hand: **0 mismatches.** `accept`, `send` and `recv`
declare their own `register_t` structs with 3, 4 and 4 fields, matching
their table entries; the other 18 match the generated structs field for
field.

But "somebody read it once" is not a property of a tree — it is a
property of an afternoon. `tools/verify/socketcall_args.py` counts it on
every run:

```
   LINUX_SOCKET           copies 3  struct linux_socket_args has 3  [domain, type, protocol]
   LINUX_LISTEN           copies 2  struct linux_listen_args has 2  [s, backlog]
   LINUX_SENDTO           copies 6  struct linux_sendto_args has 6  [s, msg, len, flags, to, tolen]
   ...
21 socketcall opcodes: every lxs_args_cnt[] entry matches the field count
of the struct its handler receives.
```

It reads the opcode defines out of the `/* Operations for socketcall */`
block only — `linux_socket.h` names hundreds of other `LINUX_*`
constants and their values collide with the opcodes freely
(`LINUX_TCP_NODELAY` is also 1). The first draft did not scope that, and
reported 43 faults over a tree with none.

Field counting handles both spellings. A generated struct pads every
argument (`char s_l_[PADL_(l_int)]; l_int s; char s_r_[PADR_(l_int)];`),
where the padding members are not arguments; counting `PADL_(` counts
each argument exactly once. A hand-written struct is counted by
declaration.

### It refuses to pass on absence

This is the same rule the ERROR inventory exists for, one level up
again: **a reader that finds nothing and a tree with nothing wrong
produce the same silence.** So the check distinguishes two kinds of bad
news, and only one of them is a finding:

- a **fault** is a tree that is wrong — a count that disagrees, a
  handler with no struct, an opcode with no table entry. Reported; fails
  `--gate`.
- a **fault to read** is the check going blind — the table renamed, the
  opcode block comment gone, a proto header not in the tree, a dispatch
  switch with no `case` labels. It exits 1 **whether or not `--gate` was
  asked for**, because there is no such thing as a passing run that read
  nothing.

Eight edits were made to the real tree while it was written, run, and
reverted:

| edit | what it said |
|---|---|
| `6 /* sendto */` → `5` | `lxs_args_cnt says 5, struct linux_sendto_args declares 6 — the handler reads 1 word(s) of kernel stack the copyin never wrote` |
| `6 /* sendto */` → `7` | `copies 7 words into a 6-word buffer`, and separately `copies more of the user array than the handler reads` |
| `linux_sendto` → `linux_sendto_nosuch` | `struct linux_sendto_nosuch_args is not declared anywhere searched` |
| a field added to `linux_sendto_args` in `linux32_proto.h` | `6 fields in .../linux_proto.h, 7 fields in .../linux32_proto.h` |
| `lxs_args_cnt[]` renamed | `cannot check: lxs_args_cnt[] not found` |
| the opcode block comment deleted | `cannot check: expected exactly one '/* Operations for socketcall */' block, found 0` |
| the `LINUX_SENDTO` case deleted | `copied 6 words, dispatched by nothing` |
| `LINUX_SENDFILE` renumbered to 99 | `lxs_args_cnt[21] (sendfile) has no LINUX_* opcode`, and `dispatched to linux_sendfile, but not in lxs_args_cnt[]` |

Every one of those is a case in `tools/verify/test_socketcall_args.py`
(25 tests), so the gate is never again only ever observed printing "ok".
Two of the tests are there for the reader rather than the tree: a struct
that is not declared must return `None` and not an empty field list —
otherwise "no struct" would silently agree with a table entry of 0 — and
the miniature fixture every other case is built from is asserted to pass
first, since a baseline that fails proves nothing about the cases that
subtract from it.

Both are in the `lints` job of `.github/workflows/pbsd-verify.yml`.

The 72 findings stay in the sweep. They are false, and now they are
false for a reason the tree states rather than a reason somebody
remembers.

---

## The other invariant that was only ever read once

The Allwinner clock section above ends on a sentence that is a liability
as written:

> the guard is not in the code, it is in thirteen driver-data lines, and
> a fourteenth that put `AW_CLK_FACTOR_ZERO_BASED` on an `m`, `p`, `div`
> or `prediv` field would make all fifteen live at once.

That is a correct reading of the tree on the day it was made, recorded
in a document. It is not a property of the tree, and nothing would have
told anybody when it stopped being true. Same defect as the socketcall
table: a fact established by an afternoon's reading, load-bearing for
fifteen findings, checked by nobody thereafter.

`tools/verify/aw_clk_zero_based.py` checks it by counting:

```
FRAC_CLK         aw_clk_frac.c divides by m
MIPI_CLK         aw_clk_mipi.c divides by m
M_CLK            aw_clk_m.c divides by m
NKMP_CLK         aw_clk_nkmp.c divides by m, p
NMM_CLK          aw_clk_nmm.c divides by m0, m1
NM_CLK           aw_clk_nm.c divides by m, n, prediv
NP_CLK           aw_clk_np.c divides by p
PREDIV_CLK       aw_clk_prediv_mux.c divides by div, prediv

   sys/dev/clk/allwinner/ccu_a10.c:302 NKMP_CLK n factor  (multiplies)
   ... thirteen of these ...

13 zero-based factor(s) in the tree
no zero-based factor is divided by; the 15 core.DivideZero findings
under sys/dev/clk/allwinner/ stay unreachable.
```

Three readings, none of them a re-implementation of anything:

1. **Which argument is which factor's flags.** The definition macros are
   positional — `NKMP_CLK(_clkname, _id, _name, _pnames, _offset,
   _n_shift, _n_width, _n_value, _n_flags, ...)` — so nothing in an
   invocation says which of its twenty-odd arguments is the `m` factor's
   flags word. The macro *body* does: `.m.flags = _m_flags,`. That is
   read out of `aw_clk.h`, per macro, and gives argument index → factor
   field.
2. **Which factors a driver divides by.** From the frequency arithmetic
   itself: `cur = (fparent * n * k) / (m * p);` and
   `*freq = *freq / prediv / div;`. An identifier is a divisor if an odd
   number of `/` operators governs it, which needs a stack rather than a
   flag only because `/ (m * p)` inverts a whole parenthesised group.
   The locals are then traced back to fields through
   `m = aw_clk_get_factor(val, &sc->m)`.
3. **Every invocation in the tree**, matched positionally against (1).

### `n` multiplies in NKMP and divides in NM

This is why the check is per-driver and a rule about the letter would be
wrong:

```c
	cur = (fparent * n * k) / (m * p);	/* aw_clk_nkmp.c:155 */
	cur = fparent / n / m;			/* aw_clk_nm.c:150  */
```

Thirteen `AW_CLK_FACTOR_ZERO_BASED` on NKMP `n` are safe. A fourteenth
on an `NM_CLK`'s `n` — the same letter, in the same position of a
similar-looking macro — is a division by zero. The check says so:

```
sys/dev/clk/allwinner/ccu_a83t.c:397 NM_CLK n factor: aw_clk_nm.c
DIVIDES by n, and AW_CLK_FACTOR_ZERO_BASED gives it a minimum of 0
```

### What breaking it found

Six edits, made to the real tree, run, reverted. Four of them the check
handled; **two found defects in the check itself**, which is the only
reason they are worth listing:

- **The line number was wrong and looked right.** Comments were stripped
  with `re.sub(r"/\*.*?\*/", " ", ..., re.S)`, and every clock definition
  line carries a `/* n factor */`, several of them multi-line. Collapsing
  those to one space moved every line after them: a definition at
  `ccu_a83t.c:397` was reported at `:368`. Now the comment is replaced by
  its own newlines, and the offset reported is the argument's, not the
  macro name's — those differ by four lines in a twelve-line definition.
- **An empty divisor set read as a clean bill of health.**
  `aw_clk_nm.c:214` is `cur = aw_clk_nm_find_best(sc, fparent, fout, &n,
  &m);` — it matches the frequency-expression pattern and contains no
  division at all. So a driver whose arithmetic moved into a helper would
  have reported *"divides by (nothing)"* and waved through every
  zero-based factor it had. Every one of these drivers divides a
  frequency by at least one factor — that is what makes the findings
  reachable — so a kind that comes back with none is now a refusal to
  answer, not a pass.

The other four: the flag moved from NKMP `n` to NKMP `m` (reported); the
flag put on an `NM_CLK`'s `n` (reported, with the right line); the
`min = 0` arm of `aw_clk_factor_get_min()` changed to `min = 1` (refuses
— the premise it enforces would no longer exist); and every use of the
flag deleted (refuses — *"either the flag is gone or this reader stopped
finding it"*).

All six are cases in `tools/verify/test_aw_clk_zero_based.py` (21
tests), which redirects the tool's single `_read()` rather than editing
files, so a failing test cannot leave the tree dirty. Writing those
tests found a third defect: `check()` was globbing and reading the
`ccu_*.c` files directly while everything else went through `_read()`,
so the tool had two readers and only one of them could be observed.

Both lints are in the `lints` job of `.github/workflows/pbsd-verify.yml`.

Two findings sets, 87 findings between them, that are false for reasons
the tree now states.

---

## Three Allwinner attach paths, from the static-taken class

`param_premise.py` sorts a sweep's findings by what constrains the
function they are in. Its `static-taken` bucket is the one with no
deferral available: a `static` function with **no call site in its own
translation unit** and a mention that is not a call — a `DEVMETHOD`
entry, a kobj method, a callout. Whatever calls it is outside the unit
and reaches it through a pointer, so "the callers are all here and they
constrain the parameters" is not an argument anybody can make about it.

175 of those are uncited in the current kernel sweep, 32 with a parameter
on the finding's own line. Reading the Allwinner run of them found three
defects and one false positive. All four are `device_attach` or a kobj
method — code that runs once, at boot, before anything can be logged.

### `awusb3phy_attach` returns a stack value as its attach status

```c
	int error, i;
	...
	for (i = 0; clk_get_by_ofw_index(dev, 0, i, &clk) == 0; i++) {
		error = clk_enable(clk);
		...
	}
	for (i = 0; hwreset_get_by_ofw_idx(dev, 0, i, &rst) == 0; i++) {
		error = hwreset_deassert(rst);
		...
	}
	...
	return (error);
```

Both loops have the lookup **as the loop condition**, so both run zero
times for a device-tree node that names neither clocks nor resets, and
`error` is never written. The last statement of the function returns it.
newbus reads that as the attach status: a garbage nonzero fails the
attach of a USB3 PHY that in fact came up, and a garbage zero claims a
success the driver never had.

Fixed by initialising it. Not by adding a `return (0)` — the two loops
genuinely do want to report the last failure they saw.

### `axp8xx_regnode_init` busy-waits on the one path nothing set

```c
	rv = axp8xx_regnode_set_voltage(regnode, param->min_uvolt,
	    param->max_uvolt, &udelay);
	if (rv != 0)
		DELAY(udelay);
```

and the callee:

```c
	if (sc->def->voltage_step1 == 0)
		return (ENXIO);
	if (axp8xx_regnode_voltage_to_reg(sc, min_uvolt, max_uvolt, &val) != 0)
		return (ERANGE);
	axp8xx_write(sc->base_dev, sc->def->voltage_reg, val);
	*udelay = 0;
	return (0);
```

`*udelay` is written on the success path and on no other. The caller
reads it on `rv != 0` and on no other. The two conditions are exact
complements: this `DELAY()` runs **if and only if** `udelay` is
uninitialised, never otherwise. `DELAY()` busy-waits for the number of
microseconds it is handed; an `int` of the wrong size is minutes of a
boot spent spinning in a regulator init.

The same defect was fixed in `rk8xx_regnode_init()` in this tree
already — same method, same out-parameter contract, same two early
returns — and the correction here is that one verbatim:
`if (rv == 0 && udelay != 0)`. That reads the cell only where the callee
wrote it, and restores the intent, which is to let a voltage settle
*after* it has been set.

### `aw_gmacclk_attach` leaks its clock definition on every path

```c
	def.parent_names = malloc(sizeof(char *) * ncells, M_OFWPROP, M_WAITOK);
	...
	clk = clknode_create(clkdom, &aw_gmacclk_clknode_class, &def);
	...
	return (0);

fail:
	return (error);
```

`clknode_create()` **copies** what it is given —
`strdup(def->name, M_CLOCK)` and
`strdup_list(def->parent_names, def->parent_cnt)` at `clk.c:89` — so the
caller still owns both after it returns. This owned them on every path
out, the success path included, and freed neither. `clk_fixed_attach()`
is the shape the tree already uses: `OF_prop_free()` on both, at the
return and again at the fail label.

Fixed by making the success path fall into the label rather than return
past it, and freeing both there. `free(NULL, ...)` is a no-op in the
kernel (`kern_malloc.c:1004`), which is what makes one label correct for
the early `goto fail` where neither has been allocated yet.

### `aw_gpio_pic_setup_intr` is a false positive, and the reason is one function away

```c
	switch (mode) {
	case GPIO_INTR_LEVEL_LOW:  irqcfg = ...; break;
	...five arms, no default...
	}
	...
	reg |= irqcfg;
```

`uint32_t irqcfg;` with no default arm, then written into the GPIO
interrupt-configuration register. But `mode` is an out-parameter of
`aw_gpio_pic_map_gpio()`, which switches on the same five values and
returns `EINVAL` for anything else *before* writing it — and the caller
returns on that error. The analyser does not carry the callee's
validation into the caller's switch. Recorded, not changed.

### Measured

`--scope sys/arm/allwinner`, 48 translation units, all OK before and
after:

```
before  8 findings
after   5 findings
  aw_gmacclk.c: 1->0
  aw_usb3phy.c: 1->0
  axp81x.c:     1->0
```

Nothing else moved. The five that remain are `aw_cir.c:235`,
`aw_gpio.c:1452` (above), `aw_rsb.c:362`, `aw_sid.c:408` and
`axp209.c:1360`.

Each fix is a `check_pbsd_marks.py` entry, and each marker was verified
by reverting its file and watching the gate fail — the check that
matters, since a marker whose text also occurs somewhere else in the
file passes over a fix that is gone.

### Still open, from the same reading

Four drivers `malloc()` a `parent_names` array for a clock definition and
never free it: `aw_gmacclk.c` (fixed above), `sdhci_fdt.c:157`,
`rk_usb2phy.c:288` and `sifive_prci.c:478`. `clk_fixed.c` and
`clock_common.c` are the two that get it right. Only the first was in
the sweep's findings, so the other three are recorded rather than
changed — a leak read out of the source is not the same evidence as a
leak an analyser walked to.

---

## The rest of the Allwinner batch, and what the other four rest on

The three fixes above left five findings in `sys/arm/allwinner/`. One
more was real; the other four are false, and naming *why* each is false
is the point — an unread finding and a read one that turns out fine look
identical in a count.

### `aw_ir_decode_packets` branches on an unread FIFO byte

```c
	unsigned char val, last;
	...
	for (i = 0;  i < sc->dcnt; i++) {
		val = sc->buf[i];
		...
	}
	if ((val & VAL_MASK) || (len <= AW_IR_L1_MIN))
		goto error_code;
```

Three loops assign `val`, and all three are bounded by `sc->dcnt`. Two
guards branch on it, outside every loop.

`aw_ir_intr()` reaches this on `AW_IR_RXINT_RPEI_EN` — RX packet end —
whether or not the same interrupt also carried FIFO data. The FIFO drain
just above it is a separate condition:

```c
	if (val & (AW_IR_RXINT_RAI_EN | AW_IR_RXINT_RPEI_EN)) {
		dcnt = AW_IR_RXSTA_COUNTER(val);
		for (i = 0; i < dcnt; i++) { ... }
	}
	if (val & AW_IR_RXINT_RPEI_EN) {
		ir_code = aw_ir_decode_packets(sc);
```

so a packet-end interrupt whose counter reads zero fills nothing, and
`aw_ir_buf_reset()` at the end of the previous packet has already put
`sc->dcnt` back to 0. The decode then runs no loop body at all.

Both outcomes of the garbage read happen to end in a code
`aw_ir_validate_code()` rejects, so no bogus scancode reaches evdev
today. That is a fact about this compilation, not a property of the
program: the read is undefined, and the second guard branches on the
same byte again after the first has let it through. Fixed with an early
return on an empty buffer.

**The first version of that fix did not work, and the measurement is
what said so.** It was `if (sc->dcnt == 0)`, and the finding came back
at `:257` — the same finding, moved down by the 22 lines of comment. The
reason is in the declaration: `int dcnt;`. Nothing in the driver can
make it negative — `aw_ir_buf_reset()` sets 0 and `aw_ir_buf_write()`
only increments — but `== 0` leaves the loops provably skippable for a
negative value, and the analyser was right about the type it had been
given. `<= 0` clears it.

### The four that stay

| finding | why it is false |
|---|---|
| `aw_gpio.c:1452` | `irqcfg`'s switch on `mode` has five arms and no default — but `mode` is an out-parameter of `aw_gpio_pic_map_gpio()`, which switches on the same five values and returns `EINVAL` for anything else *before* writing it, and the caller returns on that error. The validation is one function away. |
| `aw_rsb.c:362` | `cmd` is assigned only under `if (sc->type == A23_RSB)` and read only under `if (sc->type == A23_RSB)`. Between them are `mtx_sleep()` and register I/O, so the analyser drops what it knew about `sc->type`. |
| `aw_sid.c:408` | `data[i]` is written by `aw_sid_get_fuse()` for `i < size`, and `size` comes back from the same call. The bound is a table the analyser will not fold. |
| `axp209.c:1360` | `regdefs` is set by a switch on `sc->type` with cases for `AXP209` and `AXP221` and no default — and `axp2xx_probe()` returns `ENXIO` for anything else, so attach never runs with a third value. probe gating attach is not something the analyser models. |

Two of those four are the same shape as each other and as one more,
below.

### One cause, seven findings: a value read twice across an opaque call

`sys/amd64/amd64/fpu.c:466` is the clearest instance, because the thing
being read twice is a global:

```c
	int cp[4], i, max_ext_n;

	if (use_xsave) {
		max_ext_n = flsl(xsave_mask | xsave_mask_supervisor);
		xsave_area_desc = malloc(...);
	}
	cpu_thread_alloc(&thread0);
	saveintr = intr_disable();
	fpu_enable();
	fpusave_fxsave(fpu_initialstate);
	...
	if (use_xsave) {
		...
		for (i = 2; i < max_ext_n; i++) {
```

Both guards read `use_xsave`, which is written in exactly one place in
the whole tree — `hammer_time()`, `amd64/machdep.c:1359` — long before
this SYSINIT runs. But `cpu_thread_alloc()` and `fpusave_fxsave()` are
opaque to the analyser, and an unknown function may write a global, so
it allows false-then-true and reports `max_ext_n` unread.

The four `*_marshal_func` findings in libc are the same thing with a
struct field instead of a global, and the opaque step is a `memcpy`:

```c
	if (grp->gr_mem != NULL) {
		mem_size = 0;
		for (mem = grp->gr_mem; *mem; ++mem) { ...; ++mem_size; }
	}
	...
	memcpy(&new_grp, grp, sizeof(struct group));
	...
	if (new_grp.gr_mem != NULL)
		memcpy(p, new_grp.gr_mem, sizeof(char *) * mem_size);
```

`new_grp` is a byte copy of `*grp`, so `new_grp.gr_mem` and
`grp->gr_mem` are the same pointer and the two guards are the same
question — which is not a relation `memcpy` preserves for the analyser.

`getgrent.c:284`, `gethostnamadr.c:331` and `:345` (the same shape twice
in one function, for `aliases_size` and `addr_size`), `getnetnamadr.c:190`
and `getrpcent.c:721`. Five findings, four files, one nsswitch marshalling
idiom copied five times.

With `aw_rsb.c` and `axp209.c`, that is **seven findings on one cause**,
and none of them is a defect.

### Where the Allwinner directory stands

48 translation units, all OK:

```
at the start of this reading   8 findings
after the three attach fixes   5
after aw_cir                   4
```

and every one of the four is named above.

Of the 32 `static-taken` findings with a parameter on their own line,
this reading accounted for 12: four defects fixed, eight explained.

---

## An ioctl that returns a stack word, and a driver that ignored nine returns

Continuing the `static-taken` reading past the Allwinner run. Both of
these are on hardware that ships in ordinary machines rather than on one
SoC, and one of them is reachable from userspace without being root.

### `backlight_ioctl` had no default arm

```c
	int error;

	switch (cmd) {
	case BACKLIGHTGETSTATUS:  ...  break;
	case BACKLIGHTUPDATESTATUS: ... break;
	case BACKLIGHTGETINFO:    ...  break;
	}

	return (error);
```

`d_ioctl` is handed whatever number the caller passed — the cdev layer
does not filter unknown commands — so any `cmd` outside those three
falls through the switch and returns an uninitialised `int`.

The node is not root-only. `backlight_attach()` makes it
`UID_ROOT:GID_VIDEO`, so any member of the `video` group can issue
`ioctl(fd, <anything>, ...)` and get back a word of kernel stack as
`errno`, or — when the word happens to be zero — a reported success for
a request nothing served.

`default: error = ENOTTY;`, which is what the rest of the tree returns
in this position (`spigen.c:274`, `evtchn_dev.c:517`).

### `amdsmb`: one function's ignored return, six findings

```c
static int
amdsmb_ec_read(struct amdsmb_softc *sc, u_char addr, u_char *data)
{
	if (amdsmb_ec_wait_write(sc))
		return (1);
	...
	if (amdsmb_ec_wait_read(sc))
		return (1);
	*data = AMDSMB_ECINB(sc, EC_DATA);
	return (0);
}
```

Three timeout paths, each returning 1 **without writing `*data`**. Nine
call sites in the file, and every one of them discarded that return:

```c
	amdsmb_ec_read(sc, SMB_PRTCL, &temp);
	if (temp != 0)                             /* :283 */
	...
	amdsmb_ec_read(sc, SMB_STS, &sts);
	sts &= SMB_STS_STATUS;                     /* :295 */
	switch (sts) { ... }
```

```c
	if ((error = amdsmb_wait(sc)) == SMB_ENOERR) {
		amdsmb_ec_read(sc, SMB_BCNT, &len);
		for (i = 0; i < len; i++) {            /* :531 */
			amdsmb_ec_read(sc, SMB_DATA + i, &data);
			if (i < *count)
				buf[i] = data;
		}
		*count = len;                          /* :534 */
	}
```

So when the embedded controller does not answer in time — which is the
entire reason `amdsmb_ec_read` has a return value — the driver spins on
a stack slot as the protocol register, decodes a stack slot as the
status byte, composes a word out of two of them, and in `amdsmb_bread`
uses one as both the loop bound and the byte count it hands back to its
caller. `buf` itself is safe (`if (i < *count)` guards the write), but a
consumer that trusts the returned `*count` reads bytes of its own buffer
that were never filled.

Every call site whose value is then used now checks, and returns
`SMB_ETIMEOUT` — the code `amdsmb_wait()` already returns for exactly
this condition.

### Measured

```
sys/dev/amdsmb/amdsmb.c        6 findings -> 0   (:283 :295 :477 :477 :531 :534)
sys/dev/backlight/backlight.c  1 finding  -> 0   (:94)
```

Both still compile; both are one translation unit and the analyser
reports nothing else in either.

`amdsmb.c:477` was reported twice at the same line — the two bytes of
`*word = temp[0] | (temp[1] << 8)`, each its own unwritten value. That
is why the finding list has six entries and the file has five distinct
sites.

---

## A dead assignment the analyser did not report

`imx6_ssi.c` and `vf_sai.c` — the same audio driver, one Freescale SoC
apart — each carry two findings: one false, one a leak that cannot be
fixed without an answer this reading did not have. What the reading
found instead was in neither.

```c
	err = bus_dma_tag_create(
	    bus_get_dma_tag(sc->dev),
	    ...
	    &sc->dma_tag);

	err = bus_dmamem_alloc(sc->dma_tag, (void **)&sc->buf_base,
	    BUS_DMA_NOWAIT | BUS_DMA_COHERENT, &sc->dma_map);
	if (err) {
```

The first `err` is overwritten by the second before anything reads it.
And `bus_dma_tag_create()` opens with

```c
	*dmat = NULL;                            /* busdma_bounce.c:179 */
```

so a failed tag create leaves `sc->dma_tag` NULL and the next line hands
that to `bus_dmamem_alloc()`, which dereferences it. A NULL kernel
dereference at attach, on precisely the path the discarded return value
existed to report.

**The finding count does not move.** This is not one of the four things
the analyser reported in these two files; the leak finding shifts down
by the seven lines of comment and that is all. It is recorded here as
something a reading found while checking something else, which is a
different kind of evidence from a finding, and worth saying so rather
than letting a commit imply the sweep caught it.

### The two findings that stay in those files

`imx6_ssi.c:385` and `vf_sai.c:357` are false, and they are the `aw_sid`
shape again — a table the analyser will not fold:

```c
	sr = NULL;
	for (i = 0; rate_map[i].speed != 0; i++)          /* exact match */
		if (rate_map[i].speed == speed) sr = &rate_map[i];
	if (sr == NULL) {
		for (i = 0; rate_map[i].speed != 0; i++) {    /* nearest */
			sr = &rate_map[i];
			...
		}
	}
	sc->sr = sr;
	...
	pll4_configure_output(sr->mfi, sr->mfn, sr->mfd);
```

`sr` stays NULL only if `rate_map[0].speed == 0`. Both tables have real
entries before their sentinel — one in `imx6_ssi.c:143`, three in
`vf_sai.c:129` — so the second loop always runs at least once.

`imx6_ssi.c:790` and `vf_sai.c:742` are the `scp` leak, and they are
**real**: `ssi_attach()` mallocs `sc`, `sc->conf` and `scp`, and four
later `return (ENXIO)` paths free none of them. It is not fixed here,
and the reason is specific rather than a shrug. Two of those returns are
after `pcm_init(dev, scp)`, at which point the sound(4) framework holds
`scp` — so freeing it there would turn a boot-time leak of a few hundred
bytes into a use-after-free. Which of `pcm_init`, `pcm_addchan` and
`pcm_register` takes ownership, and what each one's failure contract is,
has to be established before the unwind can be written. Recorded as a
task with that named as the blocker.

---

## Three bus drivers whose `timeout` is only assigned inside the loop

`mv_spi.c:373`, `a37x0_spi.c:459` and `ti_i2c.c:432` are one shape, and
it is the `awusb3phy_attach` shape from earlier in this reading: a
variable assigned only inside a loop, read after it, where the loop's
own condition can be false on entry.

The two SPI drivers are the same file one Marvell SoC apart:

```c
	int resid, timeout;
	...
	while ((resid = sc->sc_len - sc->sc_written) > 0) {
		...
		timeout = 1000;
		while (--timeout > 0) { ... }
		if (timeout == 0)
			break;
		...
	}
	...
	/*
	 * Check for transfer timeout.  The SPI controller doesn't
	 * return errors.
	 */
	return ((timeout == 0) ? EIO : 0);
```

`sc->sc_len` is `cmd->tx_cmd_sz + cmd->tx_data_sz`, so a command of zero
length runs no body and the return reads a stack slot to decide whether
a transfer that had nothing to do failed. The driver's own comment says
what that value means — this controller reports nothing, so `timeout` is
the *only* evidence of success — which is what makes reading an
unwritten one worse than usual.

Not reachable from userspace: `spigen_transfer()` rejects a zero-length
command at `spigen.c:191`. Any in-kernel `SPIBUS_TRANSFER` consumer can
reach it, and nothing between it and the driver checks.

Initialised nonzero, which is what "did not time out" reads as at that
return.

`ti_i2c_transfer()` is the same with a different loop:

```c
	int err, i, repstart, timeout;
	...
	for (i = 0; i < nmsgs; i++) {
		...
		if (repstart == 0) {
			timeout = 0;
			while (ti_i2c_read_2(sc, I2C_REG_STATUS_RAW) & I2C_STAT_BB) {
				if (timeout++ > 100) { err = EBUSY; goto out; }
				DELAY(1000);
			}
			timeout = 0;
		} else
			repstart = 0;
	...
out:
	if (timeout == 0) {
		while (ti_i2c_read_2(sc, I2C_REG_STATUS_RAW) & I2C_STAT_BB) {
```

A transfer of no messages runs no body, and the `out:` label reads the
stack slot to decide whether to wait for the bus to go idle. Initialised
to 0, the value the normal flow leaves there.

### Measured

```
--scope sys/arm/mv   4 findings -> 2   (a37x0_spi.c:459, mv_spi.c:373 gone)
--scope sys/arm/ti   6 findings -> 5   (ti_i2c.c:432 gone)
```

All units still OK. What remains in those two directories is
`mv/gpio.c:416`, `mvebu_gpio.c:296`, `am335x_pwmss.c:140`,
`ti_divider_clock.c:170` and `:172`, and `ti_adc.c:460` and `:464` —
unread, and named here so the count is not mistaken for a clean bill.

### Running total for this reading

Of the 32 `static-taken` findings with a parameter on their own line:

| | |
|---|---|
| defects fixed | 10 |
| explained as false, with a named cause | 12 |
| real, deferred with the blocker named | 2 |
| not yet read | 8 |

Plus one defect found by reading rather than by the analyser
(`bus_dma_tag_create` in two drivers), which moves no count at all.

---

## A panic the analyser did not report, next to a finding that it did

Reading the seven findings named as unread above turned up three more
defects, and the largest of them is not what the analyser was pointing
at.

### `mvebu_gpio_pin_toggle` panics on every toggle

The finding is `mvebu_gpio.c:296`, *"The left operand of `!=` is a
garbage value"*:

```c
	GPIO_LOCK(sc);
	mvebu_gpio_pin_get(sc->dev, pin, &val);
	if (val != 0)
```

`mvebu_gpio_pin_get()` returns `EINVAL` without writing `*val`, and the
return is discarded — which is what the analyser saw. That path is not
reachable here, because the caller already made the same `pin >=
sc->gpio_npins` check.

What *is* reachable is the next line down:

```c
static int
mvebu_gpio_pin_get(device_t dev, uint32_t pin, unsigned int *val)
{
	...
	GPIO_LOCK(sc);
```

and

```c
#define	GPIO_LOCK_INIT(_sc)	mtx_init(&_sc->mtx, 			\
	    device_get_nameunit(_sc->dev), "mvebu_gpio", MTX_DEF)
```

`MTX_DEF`, not `MTX_RECURSE`. So `pin_toggle` takes the mutex and then
calls a function that takes the same mutex: **`panic:
_mtx_lock_sleep: recursed on non-recursive mutex`**, on every
`GPIOTOGGLE` ioctl to any Marvell Armada 3700/7K/8K GPIO controller,
from anyone who can open `/dev/gpioc*`.

Fixed by doing the two reads inline under the lock that is already held.
That removes the recursion, removes the uninitialised read with the call
that caused it, and makes the toggle the atomic read-modify-write it was
always meant to be — the old form dropped the lock between the read and
the write.

This is the second time in this reading that a finding was a false
positive sitting immediately next to a real defect it did not name. It
is an argument for reading them rather than counting them.

### `am335x_pwmss_attach` shifts by a stack value

```c
	rev_address = ti_sysc_get_rev_address(device_get_parent(dev));
	switch (rev_address) {
	case PWMSS_REV_0: id = 0; break;
	case PWMSS_REV_1: id = 1; break;
	case PWMSS_REV_2: id = 2; break;
	}

	reg = SYSCON_READ_4(sc->syscon, SCM_PWMSS_CTRL);
	reg |= (1 << id);
	SYSCON_WRITE_4(sc->syscon, SCM_PWMSS_CTRL, reg);
```

Three arms, no default, and `rev_address` is whatever the device tree
says. `1 << id` for an unset `id` is undefined at or above the width of
`int` and an arbitrary bit in a system-control register below it. A
`default:` that reports the revision and refuses to attach.

### `ti_divider_attach` takes a stack slot as a register field width

```c
	uint32_t	ti_max_div;
	...
	if (OF_hasprop(node, "ti,max-div")) {
		OF_getencprop(node, "ti,max-div", &value, sizeof(value));
		ti_max_div = value;
	}
	...
	if (sc->div_def.div_flags)
		sc->div_def.i_width = fls(ti_max_div-1);
	else
		sc->div_def.i_width = fls(ti_max_div);
```

Assigned in one place, read unconditionally. `i_width` is the *bit width
of the divider field in a clock register*, so a node without
`ti,max-div` configures a clock from a stack value and every consumer of
that clock gets a rate computed from the wrong bits.

Not hypothetical. Counting the device trees this tree ships:

```
ti,divider-clock nodes in the shipped TI device trees: 200
  without ti,max-div: 16
```

on `dra7xx`, `omap44xx`, `omap54xx`, `am43xx`, `dm816x` and `omap446x`.
Every one of the 16 uses `ti,dividers` — an explicit divider table this
driver does not implement and only prints a line about — so there is no
width to compute from anything.

**This changes behaviour, deliberately.** Those 16 nodes go from
attaching with a garbage width to not attaching, with a message saying
why. Configuring a clock divider from a stack slot is not a behaviour
worth preserving, and a driver that cannot do its job should say so.

### Measured

```
--scope sys/arm/mv   2 findings -> 1   (mvebu_gpio.c:296)
--scope sys/arm/ti   5 findings -> 2   (am335x_pwmss.c:140,
                                        ti_divider_clock.c:170 and :172)
```

All 44 `sys/arm/ti` units still OK. Over this reading those two
directories have gone `mv 4 -> 1` and `ti 6 -> 2`. What is left is
`mv/gpio.c:416` and `ti_adc.c:460` and `:464`, still unread.

---

## An unbounded FIFO drain into a 16-word stack array

The last two findings in `sys/arm/mv` and `sys/arm/ti`. The second is
the most serious thing this reading found.

### `ti_adc_tsc_read_data` has no bound on either side

```c
	int count;
	uint32_t data[16];
	...
	count = ADC_READ4(sc, ADC_FIFO1COUNT) & ADC_FIFO_COUNT_MSK;
	if (count == 0)
		return;

	i = 0;
	while (count > 0) {
		data[i++] = ADC_READ4(sc, ADC_FIFO1DATA) & ADC_FIFO_DATA_MSK;
		count = ADC_READ4(sc, ADC_FIFO1COUNT) & ADC_FIFO_COUNT_MSK;
	}
```

`i` has **no bound**. The loop runs until the hardware's own count
register reads zero, and that register is masked with

```c
#define	ADC_FIFO_COUNT_MSK		0x0000007f
```

— up to **127 words into a 16-word array on the kernel stack**. The step
configuration normally queues `2n + 2` samples, but the AM335x FIFO is
64 entries deep and nothing stops it holding more than one sequencer run
when an interrupt is late.

Bounded, with the reads past the array continuing so the FIFO is still
drained — leaving entries in it leaves the controller stuck.

The analyser's actual finding was the *other* direction, at the two
summing lines: a run that delivered **fewer** than a full coordinate set
leaves the tail of `data[]` unwritten and the sums read it. Guarded by
returning when `i < sc->sc_coord_readouts * 2 + 2` — an incomplete set
is not a coordinate.

### And the index base came from the device tree unchecked

```c
		if ((OF_getencprop(child, "ti,coordinate-readouts", &cell,
		    sizeof(cell))) > 0)
			sc->sc_coord_readouts = cell;
```

`sc_coord_readouts` indexes that same 16-word buffer over
`[n + 2, 2n + 2)` and is a `qsort()` length there, and at
`ti_adc_setup()` it is

```c
	start_step = ADC_STEPS - (sc->sc_coord_readouts*2 + 2) + 1;
```

which goes negative past 7. So the buffer holds only for `2n + 2 <= 16`,
i.e. `n <= 7`, and nothing said so. Named the bound
(`TI_ADC_MAX_READOUTS`), derived it from the buffer size rather than
writing 7, and rejected an out-of-range property with a message.

One of the two findings stays, at the `x` sum. The guard makes it
unreachable — with `i >= 2n + 2`, entries `0..2n+1` are written, and the
largest index either loop touches is `2n`  when `n > 3` and `2n + 1`
otherwise — but the analyser does not connect the count of words written
to the range of indices read. It cleared the `y` sum and not the `x` one,
which is the difference between low indices and high ones, not between
two different arguments.

### `mv_gpio_setup_intrhandler` leaked on the success path

```c
	sc = (struct mv_gpio_softc *)device_get_softc(dev);
	s = malloc(sizeof(struct mv_gpio_pindev), M_DEVBUF, M_NOWAIT | M_ZERO);

	if (pin < 0 || pin >= sc->pin_num)
		return (ENXIO);
	event = sc->gpio_events[pin];
	if (event == NULL) {
		...
		error = intr_event_create(&event, (void *)s, ...);
```

`intr_event_create()` is the *only* consumer of `s`, and it is inside
`if (event == NULL)`. So the allocation leaked on the bounds check, on
both failing returns — and on the ordinary success path every time the
pin already had an event, which is every call after the first for that
pin. The `M_NOWAIT` return was also never checked, and a NULL would go
through as the cookie `mv_gpio_intr_mask()`, `mv_gpio_intr_unmask()` and
`mv_gpio_int_ack()` all dereference.

Moved to its point of use, checked, and freed on the two paths that do
not hand it over.

Worth saying: this function has **no callers in the tree** — only its
own declaration and the comment in `mv_gpio_finish_intrhandler()` about
"when we achieve full interrupt support" — and nothing ever fills in
`s->dev` or `s->pin`, so a future first caller would NULL-dereference in
the mask callback before the leak mattered. The leak is the smallest of
its problems, and is fixed because it was reported, not because it is
the interesting thing about this function.

### Measured

```
--scope sys/arm/mv   1 finding -> 0    44 units, all OK
--scope sys/arm/ti   2 findings -> 1   44 units, all OK
```

`sys/arm/mv` is clean. Over this reading:

| directory | at the start | now |
|---|---|---|
| `sys/arm/allwinner` | 8 | 4 |
| `sys/arm/mv` | 4 | 0 |
| `sys/arm/ti` | 6 | 1 |
| `sys/dev/amdsmb` | 6 | 0 |
| `sys/dev/backlight` | 1 | 0 |

---

## Three more stack words returned to userspace

Same class as `backlight_ioctl`, and the first of these is on every
amd64 machine.

### `pci_ioctl`'s `PCIOCGETCONF` returns an unassigned `error`

This one has a `default: error = ENOTTY;`, so the obvious reading of the
`core.uninitialized.UndefReturn` at `pci_user.c:1566` is wrong. The
uncovered path is inside an arm:

```c
	case PCIOCGETCONF:
		...
		cio->num_matches = 0;
		...
		for (cio->num_matches = 0, i = 0,
			 dinfo = STAILQ_FIRST(devlist_head);
		     dinfo != NULL;
		     dinfo = STAILQ_NEXT(dinfo, pci_links), i++) {
			if (i < cio->offset)
				continue;
			...
			error = copyout(&pcu, ...);
			...
		}
		...
getconfexit:
		pci_conf_io_update_data(cio, data, cmd);
		free(cio, M_TEMP);
		free(pattern_buf, M_TEMP);

		break;
```

`error` is assigned inside the walk — at the `copyout`, and at the
`num_matches >= ionum` limit — and on the several `goto getconfexit`
error paths. A walk that **matches nothing** does neither, falls into
`getconfexit:`, breaks, and returns a stack word.

`pciconf -l` with a selector that matches no device is exactly that
call.

### `u2f_read` on a zero-length read

```c
	size_t length = 0;
	int error;
	...
	while (!sc->sc_state.data) {
		...                       /* every assignment to error is here */
	}
	if (sc->sc_state.data && uio->uio_resid > 0) {
		length = min(uio->uio_resid, sc->sc_isize);
		...
	}
exit:
	mtx_unlock(&sc->sc_mtx);
	if (length != 0)
		error = uiomove(buf, length, uio);

	return (error);
```

`length` is initialised; `error` is not, one line apart. A read issued
when a report is **already** buffered runs the wait loop zero times, and
asking for zero bytes leaves `length` at 0, so the `uiomove` is skipped
too. `read(fd, buf, 0)` on `/dev/u2f/N`.

### `cp2112iic_transfer` with no messages

Both of that function's loops — the validation pass and the transfer
pass — are `for (i = 0; i < nmsgs; i++)`. `nmsgs == 0` runs neither, so
`reason` stays NULL and the validation is considered passed, and
`return (err)` reads a slot nothing wrote. The same shape as
`ti_i2c_transfer` above, in a USB-attached I²C bridge.

### Measured

```
--scope sys/dev/pci        1 finding  -> 0   19 units, all OK
--scope sys/dev/hid        2 findings -> 1   18 units, all OK
--scope sys/dev/usb/misc   1 finding  -> 0    5 units, all OK
```

`sys/dev/pci` and `sys/dev/usb/misc` are clean. `sys/dev/hid/hmt.c:348`
remains, unread.

### And one that is false by its registration

`lio_get_ringparam()` (`lio_sysctl.c:979`) switches on `arg2` with two
cases and no default, then returns an `int err` only those two assign.
`arg2` is fixed when the sysctl is registered, and the only two
registrations of this handler pass `LIO_SET_RING_RX` and
`LIO_SET_RING_TX` (`lio_sysctl.c:186` and `:189`). The analyser cannot
see a `SYSCTL_PROC` registration, so it treats `arg2` as free.
Recorded, not changed — the same shape as `axp2xx_attach`, where
`probe` gates what reaches `attach`.

---

## The inverse of the `copyin` premise, and it is measured

Three findings in `sys/dev/usb/controller/` are the same line in three
drivers:

```c
	/* receive data */
	bus_space_read_multi_1(sc->sc_io_tag, sc->sc_io_hdl,
	    MUSB2_REG_EPFIFO(0), (void *)&req, sizeof(req));
	...
	if ((req.bmRequestType == UT_WRITE_DEVICE) &&      /* <- reported */
```

`atmegadci.c:295`, `musb_otg.c:503` and `uss820dci.c:326` — each reading
a USB SETUP packet off the controller's FIFO into a local `struct
usb_device_request` and then looking at it. All three are
`core.UndefinedBinaryOperatorResult` on `req.bmRequestType`.

Earlier in this document, the `copyin` premise was disproved with the
observation that **CSA invalidates a by-pointer argument passed to an
unknown function** — it must, because the callee may have written
through it. So why is `req` still undefined here?

Because `bus_space_read_multi_1` is not unknown. It is
`static __inline`, in `sys/x86/include/bus.h:275`, and its body is
inline assembly:

```c
static __inline void
bus_space_read_multi_1(bus_space_tag_t tag, bus_space_handle_t bsh,
		       bus_size_t offset, u_int8_t *addr, size_t count)
{
	if (tag == X86_BUS_SPACE_IO)
		insb(bsh + offset, addr, count);
	else {
		__asm __volatile("				\n\
		1:	movb (%2),%%al				\n\
			stosb					\n\
			loop 1b"				:
		    "=D" (addr), "=c" (count)			:
		    "r" (bsh + offset), "0" (addr), "1" (count)	:
		    "%eax", "memory");
	}
}
```

The analyser steps *into* it, finds no C store through `addr`, and comes
back out with the buffer still undefined. The `"memory"` clobber is a
constraint on the optimiser, not something CSA folds into its value
model for the pointee.

So the rule is the exact inverse of the one that disproved `copyin`:
**an unknown function invalidates what it is given; a visible inline
function that writes only through inline assembly does not.** Being able
to see the callee is what loses the information.

### Probed, not asserted

`tools/verify/probes/inline_asm_write.c` makes all three predictions at
once and clang answers all three:

| | expected | reported? |
|---|---|---|
| read through a visible inline function with an asm body | reported | **yes**, with `note: Calling 'read_multi'` / `Returning from 'read_multi'` |
| read after an **unknown** function was handed the pointer | not reported | **no** — the control |
| read of something nothing wrote at all | reported | **yes** — the sanity control |

```
asm_probe.c:28:13: warning: The left operand of '==' is a garbage value
asm_probe.c:27:2: note: Calling 'read_multi'
asm_probe.c:27:2: note: Returning from 'read_multi'
asm_probe.c:48:13: warning: The left operand of '==' is a garbage value
2 warnings generated.
```

Line 38 — the unknown-function control — is absent, which is the half
that makes the other two mean something. The note trail is the whole
answer: it says clang went in and came back.

### How much this explains: 3, and the proxy said 13

The tempting next step is to count findings in files that use a
multi-word bus-space read. That gives **13 findings in 5 files**. It is
an upper bound and not a count, and reading the largest cluster shows
why: `fdc.c`'s eight are `fdc_sense_int()` writing `*st0p` and `*cylp`
on some paths and not others — a plain out-parameter shape that has
nothing to do with inline assembly, in a file that happens to also
contain a `bus_space_read_multi_1` elsewhere.

"The file contains X" is a proxy for "the finding rests on X", and the
same substitution was already caught once in this document over
`M_ZERO`. So the honest number is the three that were read: the USB
SETUP-packet trio. The other ten stay in the unread bucket where they
belong.

Nothing to change in any of the three. Recorded so they can be
dismissed by name.

---

## Two detach paths that test a pointer and then dereference it

`es137x.c:1913` and `via8233.c:1412` are the same function in two sound
drivers, and each contradicts itself in three lines:

```c
	es = pcm_getdevinfo(dev);

	if (es != NULL && es->num != 0) {
		...
	}

	bus_teardown_intr(dev, es->irq, es->ih);
	bus_release_resource(dev, SYS_RES_IRQ, es->irqid, es->irq);
	bus_release_resource(dev, es->regtype, es->regid, es->reg);
	bus_dma_tag_destroy(es->parent_dmat);
	mtx_destroy(&es->lock);
	free(es, M_DEVBUF);
```

The `!= NULL` is the author's own statement that NULL is possible here —
`pcm_getdevinfo()` returns `d->devinfo`, which only `pcm_init()` sets —
and everything after it dereferences `es` six times and then frees it.
Either the check is unnecessary and should not be there, or it is not
and this function walks straight past it.

Fixed by returning on NULL, which is the reading that keeps the check
meaning what it says. `via8233.c` is the identical shape with three more
dereferences.

## `ichwd_identify`: a KASSERT decided by a stack slot

```c
	device_t ich, smb;

	ich = ichwd_find_ich_lpc_bridge(parent, &id_p);
	if (ich == NULL) {
		smb = ichwd_find_smb_dev(parent, &id_p);
		if (smb == NULL)
			return;
	}

	KASSERT(id_p->tco_version >= 1, ...);
	KASSERT(id_p->tco_version != 4 || smb != NULL,
	    ("could not find PCI SMBus device for TCOv4"));
```

`smb` is assigned only on the path where the LPC lookup failed, and read
whenever `tco_version == 4`. On an `INVARIANTS` kernel that would be a
boot-time panic decided by a stack word.

It cannot happen today, and the reason is in the two device tables, not
in the code:

```
ichwd_devices (LPC):     209 entries, tco_version counts {1: 12, 2: 191, 3: 6}
ichwd_smb_devices (SMB):   6 entries, tco_version counts {4: 6}
```

No LPC row is TCOv4, so `!= 4` short-circuits before `smb` is read. That
is the `AW_CLK_FACTOR_ZERO_BASED` shape a third time — an invariant held
by driver data, one table row from being false. Here it costs nothing to
put in the code: `smb = NULL` is what the assertion already claims to be
testing for, so a 210th LPC row with TCOv4 would fire the assertion
rather than read the stack.

## `mv88e151x_attach` restores a stack word as a PHY capability

```c
	uint32_t cop_cap, cop_extcap;
	...
	cop_cap = sc->mii_capabilities;
	if (sc->mii_capabilities & BMSR_EXTSTAT) {
		sc->mii_extcapabilities = PHY_READ(sc, MII_EXTSR);
		cop_extcap = sc->mii_extcapabilities;
	}
	...
	if (MII_MODEL(ma->mii_id2) == MII_MODEL_xxMARVELL_E1512) {
		...                                  /* switch to fiber */
		sc->mii_capabilities = cop_cap;
		sc->mii_extcapabilities = cop_extcap;
	}
```

`cop_cap` is assigned unconditionally; `cop_extcap` is not. An E1512
whose BMSR does not report `BMSR_EXTSTAT` writes a stack word into the
PHY's advertised extended capabilities, which `mii_phy_add_media()` and
`mii_phy_setmedia()` then read to choose media.

Zero is the correct save: when that branch is not taken
`sc->mii_extcapabilities` was never assigned either, and the softc is
allocated zeroed, so restoring 0 puts back what was there.

### Measured

```
--scope sys/dev/ichwd        1 finding   -> 0    2 units, all OK
--scope sys/dev/mii          2 findings  -> 1   39 units, all OK
--scope sys/dev/sound/pci   10 findings  -> 8   32 units, all OK
```

### Two more read and left alone

`micphy.c:299` reads `reg` under
`if (sc->mii_mpd_model == MII_MODEL_MICREL_KSZ8081)` and assigns it
under the same test, with `mii_phy_reset(sc)` between them — the
`fpu.c` / `aw_rsb.c` shape, a value guarded twice across an opaque call.

`kbdmux.c:493`'s "potential leak of `fkeymap`" is the **M_ZERO** class
this document already established. `kbd_set_maps()` — which stores
`fkeymap` into `kbd->kb_fkeytab`, and `kbd` escapes to the caller
through `*kbdp` on the line it is allocated — is called only inside
`if (!KBD_IS_PROBED(kbd))`. `KBD_IS_PROBED` reads `kbd->kb_flags`, which
is zero only because the `malloc` two lines up passed `M_ZERO`, and
clang has no model for `malloc`'s third argument. So the analyser
allows the branch to be skipped and the allocation to escape nothing.

### Addendum: the class is wider than the three, and the proxy misses both ways

Two more members read after the section above was written:

- `sys/dev/mgb/if_mgb.c:434` — `mgb_get_ethaddr()` fills a `struct
  ether_addr` with `CSR_READ_REG_BYTES`, which is
  `bus_read_region_1((sc)->regs, reg, dest, cnt)`, and the caller then
  tests `ETHER_IS_BROADCAST(hwaddr.octet)`.
- `sys/x86/isa/orm.c:131` — `bus_space_read_region_1(bt, bh, 0, buf,
  sizeof(buf))` then `if (buf[0] != 0x55 || buf[1] != 0xAA ...)`.

So `read_region` belongs beside `read_multi`: both are `static __inline`
in `sys/x86/include/bus.h` with `insb`/`rep movsb` bodies, and both lose
the write.

Widening the proxy to `bus_(space_)?read_(multi|region)_[1248]` gives
**22 uninitialised-value findings in 8 files** — and it is wrong in
*both* directions, which is the useful part:

- it **over**-counts: `fdc.c`'s eight are `fdc_sense_int()`'s
  out-parameters, as established above;
- it **under**-counts: neither `atmegadci.c` nor `if_mgb.c` matches it,
  because they call the primitive through a driver macro
  (`ATMEGA_READ_MULTI_1`, `CSR_READ_REG_BYTES`) whose expansion lives in
  a header the pattern never opens.

A textual proxy for "the finding rests on X" fails on macros in one
direction and on coincidence in the other.

Three more read since: `hptiop.c:477`, `:813` and `:1110`, all
`core.uninitialized.Branch`, all

```c
	bus_space_read_region_4(hba->bar0t, hba->bar0h, req32 +
		offsetof(struct hpt_iop_request_header, context),
		(u_int32_t *)&temp64, 2);
	if (temp64)
```

— a 64-bit value assembled out of two 32-bit register reads and then
branched on.

The verified members are the ones that were read: `atmegadci.c:295`,
`musb_otg.c:503`, `uss820dci.c:326`, `if_mgb.c:434`, `orm.c:131`,
`hptiop.c:477`, `:813`, `:1110` — **eight**, across six files and three
subsystems (USB device controllers, an Ethernet MAC, an ISA option-ROM
probe, a RAID controller). The rest of what the proxy names is not
claimed.

---

## `ti_copy_scratch`: the loop condition itself reads the uninitialised value

`if_ti.c:788` is `core.uninitialized.UndefReturn`, and the defect is
worse than that:

```c
	uint32_t segptr;
	int cnt, error;
	...
	segptr = tigon_addr;
	cnt = len;
	ptr = buf;

	while (cnt && error == 0) {
		...
	}

	return (error);
```

`error` is never initialised, and it is not only returned — it is in the
**loop condition**. The very first evaluation of `while (cnt && error ==
0)` tests a stack slot, so the loop may not run at all on a call that
had work to do. And when it does not run, whether for that reason or
because `len` was zero, the `return (error)` hands the slot back.
`ti_ioctl2()` — the driver's private ioctl — reaches this with a length
from userspace.

The evidence for what it should be is in the same file:
`ti_copy_mem()`, the sibling this function was copied from, has the same
`while (cnt != 0 && error == 0)` and sets `error = 0` explicitly before
it.

### Measured, and a lesson about how

```
--scope sys/dev/ti   9 findings -> 7   1 unit, OK on both sides
  gone: if_ti.c:726 (UndefinedBinaryOperatorResult, the loop condition)
        if_ti.c:788 (UndefReturn, the return)
```

The first comparison of this fix looked like a **regression** — three
findings that were not in the kernel sweep's row for this file appeared,
including one in `if_tireg.h`. They were not new. The kernel sweep
reports 6 findings for `if_ti.c` and a `--scope sys/dev/ti` run reports
9, because the two shards do not compile the file with the same flags.

So a before/after has to put both sides at the same scope, and the
number in a full sweep's row is not a baseline for a scoped run. Every
other measurement in this session's driver work was a scoped run against
the kernel sweep's row — which was safe only because those were all
*disappearances*, and a finding that vanishes cannot be an artefact of
the baseline having fewer. This one had a shift and an apparent
addition, and needed the real A/B.

---

## Read and left alone, with the reason each time

Findings from the `static-taken` list that were read in this pass and
are not defects. Each is here so it leaves the unread bucket by name
rather than by a count going down.

### `mpr_sas.c:2583` and `mps_sas.c:2118` — a guard 40 lines from its alias

```c
	rep = (MPI2_SCSI_IO_REPLY *)cm->cm_reply;        /* :2436 */
	...
	if (cm->cm_reply == NULL) {                       /* :2544 */
		...
		return;                                   /* :2576 */
	}
	...
	rep->SCSIStatus = mprsas_complete_nvme_unmap(sc, cm);   /* :2583 */
```

The NULL case is handled and returns. What the analyser will not do is
carry a guard on `cm->cm_reply` across to `rep`, which was aliased to it
147 lines earlier. Two intermediate blocks test `cm->cm_reply != NULL`
before logging through `rep`, which is the same author making the same
connection by hand.

Worth stating plainly: this is a heavily-deployed SAS driver, and
"fixing" it on the first reading — before finding the `return` at
`:2576` — would have added a redundant branch to a hot completion path
on the strength of a false positive.

### `rk_pinctrl.c:692` and `:694` — a bank validated by a table lookup

`rk3399_parse_bias()` switches on `bank` with arms for 0-4 and no
default, then returns `pullup`/`pulldown`. Its only caller checks the
`(bank, subbank)` pair against `sc->conf->iomux_conf[]` and returns
before calling if there is no match — and `rk3399_iomux_bank[]` has
exactly banks 0 through 4. The same shape as `aw_gpio_pic_setup_intr()`:
validation one function up, through a table.

### `micphy.c:299` — a softc field read twice across an opaque call

```c
	if (sc->mii_mpd_model == MII_MODEL_MICREL_KSZ8081)
		reg = PHY_READ(sc, MII_KSZ8081_PHYCTL2);
	mii_phy_reset(sc);
	if (sc->mii_mpd_model == MII_MODEL_MICREL_KSZ8081)
		PHY_WRITE(sc, MII_KSZ8081_PHYCTL2, reg);
```

Assigned and read under the same test, with an unknown call between
them. The `fpu.c` / `aw_rsb.c` shape.

### `kbdmux.c:493` — the M_ZERO class

`kbd_set_maps()`, which stores `fkeymap` into a structure that escapes
to the caller, is called only inside `if (!KBD_IS_PROBED(kbd))`.
`KBD_IS_PROBED` reads `kbd->kb_flags`, which is zero only because the
`malloc` two lines above passed `M_ZERO` — and clang has no model for
`malloc`'s third argument, as this document established with a probe.

### `lio_sysctl.c:979` — false by its registration

`switch (arg2)` with two cases and no default, returning an `int` only
those two assign. `arg2` is fixed when the sysctl is registered, and the
only two registrations of this handler pass `LIO_SET_RING_RX` and
`LIO_SET_RING_TX`. `SYSCTL_PROC` is not something the analyser reads.

### `fdc.c` ×8 — out-parameters, not the bus-space class

Named here because they were nearly miscounted into the inline-assembly
class above: `fdc_sense_int()` writes `*st0p` and `*cylp` on some paths
and not others, in a file that also contains a `bus_space_read_multi_1`
somewhere else entirely.

### `busdma_iommu.c:393` — recorded as a task, not fixed

```c
	error = common_bus_dma_tag_create(parent != NULL ?
	    &((struct bus_dma_tag_iommu *)parent)->common : NULL, ...);
	...
	oldtag = (struct bus_dma_tag_iommu *)parent;
	newtag->ctx = oldtag->ctx;
```

The same self-contradiction as the two sound detach paths — a ternary
that contemplates NULL, then an unconditional dereference — but without
their obvious resolution. A detach with no devinfo has nothing to tear
down, so returning is right; a *tag* with no parent has no `ctx` to
inherit, and one built with a NULL `ctx` would fault later in the map
path rather than here. Which of the two lines is wrong depends on
whether `parent` can be NULL at all, given that this implementation is
only selected when the parent already carries it. Left for a reading
that settles that.

---

## `iwn5000_read_eeprom`: a failed EEPROM read becomes the address of every later one

`if_iwn.c:2351` is `core.uninitialized.Assign`, and it is the one place
in that file where the shape matters most.

```c
static int
iwn_read_prom_data(struct iwn_softc *sc, uint32_t addr, void *data, int count)
{
	uint8_t *out = data;
	...
		if (ntries == 20) {
			device_printf(sc->sc_dev,
			    "timeout reading ROM at 0x%x\n", addr);
			return ETIMEDOUT;
		}
		if (sc->sc_flags & IWN_FLAG_HAS_OTPROM) {
			...
			if (tmp & IWN_OTP_GP_ECC_UNCORR_STTS) {
				device_printf(sc->sc_dev,
				    "OTPROM ECC error at 0x%x\n", addr);
				return EIO;
			}
		}
		*out++ = val >> 16;
```

Both error returns are **before** the first write to `*out`, so the
caller's buffer is untouched. And:

```c
	/* Read regulatory domain (4 ASCII characters). */
	iwn_read_prom_data(sc, IWN5000_EEPROM_REG, &val, 2);
	base = le16toh(val);
```

`base` is not a value. It is the **address** every subsequent read in
that function uses — the regulatory domain, the channel list for each
band, the calibration header, the temperature and voltage calibration
points, the crystal calibration. A stack word here sends all of them
somewhere else in the EEPROM, and the driver then configures the
regulatory domain and channel list from whatever it finds.

The device prints "timeout reading ROM" or "OTPROM ECC error" first, so
the failure is not silent — but nothing acts on it.

Fixed for this one call. **Not** fixed for the other sixteen: eighteen
call sites, one of which (`:1657`) checks. The rest are one wrong value
each rather than a wrong offset for everything, and the `read_eeprom`
method they sit in is `void`, so propagating an error means changing the
method's signature and both implementations. That is a different change
from a line, and half-doing it would leave the file looking checked
where it is not.

```
--scope sys/dev/iwn   1 finding -> 0   1 unit, OK on both sides
```

Both sides at the same scope, per the `ti_copy_scratch` note above.

---

## `intel_ntb_exchange_msix`: the peer's interrupt address, from a stack word

`ntb_hw_intel.c:3413` and `:3430`.

```c
static int
intel_ntb_spad_read(device_t dev, unsigned int idx, uint32_t *val)
{
	struct ntb_softc *ntb = device_get_softc(dev);

	if (idx >= ntb->spad_count)
		return (EINVAL);

	*val = intel_ntb_reg_read(4, ntb->self_reg->spad + idx * 4);

	return (0);
}
```

The bounds check returns **before** the write. All four calls in
`intel_ntb_exchange_msix()` discarded the return:

```c
	intel_ntb_spad_read(ntb->device, NTB_MSIX_GUARD, &val);
	if (val != NTB_MSIX_VER_GUARD)                      /* :3413 */
		goto reschedule;

	for (i = 0; i < XEON_NONLINK_DB_MSIX_BITS; i++) {
		intel_ntb_spad_read(ntb->device, NTB_MSIX_DATA0 + i, &val);
		ntb->peer_msix_data[i].nmd_data = val;
		intel_ntb_spad_read(ntb->device, NTB_MSIX_OFS0 + i, &val);
		ntb->peer_msix_data[i].nmd_ofs = val;
	}
	...
	intel_ntb_spad_read(ntb->device, NTB_MSIX_DONE, &val);
	if (val != NTB_MSIX_RECEIVED)                       /* :3430 */
		goto reschedule;
```

The indices are fixed by the MSI-X handshake protocol; `spad_count`
comes from the hardware. On a device with fewer scratchpad registers
than the protocol uses, every one of these fails, and:

- the two guard comparisons decide the handshake on a stack word — the
  two findings the analyser reports;
- and the two reads in the loop go straight into
  `ntb->peer_msix_data[i]`, which is the **address and data this driver
  later writes to signal the peer**. Those two are not reported at all,
  because their destination is a struct field rather than a comparison,
  and they are the reason this is worth more than tidiness.

Every call now checks and takes `reschedule` — the path the function
already uses when the peer is not ready, which is the right answer for a
read that did not happen.

```
--scope sys/dev/ntb   8 findings -> 6   7 units, OK on both sides
  gone: ntb_hw_intel.c:3413, :3430
```

Both sides at the same scope. What remains is `ntb_hw_amd.c:697`,
`ntb_transport.c:467`, `:604`, `:1337` and two in `ntb_tool.c`, all
unread.

---

## `hmt_attach`: where `= 0` would have made it worse

`hmt.c:348` was named as unread in an earlier commit here; reading it
found a defect whose obvious fix is wrong.

```c
	hid_size_t d_len, fsize, rsize;
	...
	if (sc->cont_max_rlen > 1) {
		err = hid_get_report(dev, fbuf, sc->cont_max_rlen, &rsize, ...);
		if (err == 0 && (rsize - 1) * 8 >= ...) { ... }
	}
	...
	if (sc->btn_type_rlen > 1 && sc->btn_type_rid != sc->cont_max_rid) {
		bzero(fbuf, fsize);
		err = hid_get_report(dev, fbuf, sc->btn_type_rlen, &rsize, ...);
		...
	}
	if (sc->btn_type_rlen > 1 && err == 0 && (rsize - 1) * 8 >=
	    sc->btn_type_loc.pos + sc->btn_type_loc.size)          /* :348 */
```

The second block is skipped when `btn_type_rid == cont_max_rid`
**deliberately** — the two feature reports are the same report, so
`fbuf` and `rsize` from the first fetch are meant to be reused. That is
not the bug. The bug is that `cont_max_rlen <= 1` skips the *first*
fetch as well, and then nothing has ever written `rsize`.

### The obvious fix is a worse bug

`hid_location`'s `pos` and `size` are `uint32_t` (`hid.h:238`). So

```c
	(rsize - 1) * 8 >= sc->btn_type_loc.pos + sc->btn_type_loc.size
```

promotes `rsize` to `int`, gives **-8** for `rsize == 0`, and then
converts that to `unsigned int` for the comparison against an unsigned
right-hand side: **4294967288**, which is `>=` almost anything. Writing
`hid_size_t ... rsize = 0;` and stopping there would turn an
indeterminate read into a guard that reliably *passes*, and the driver
would decide `is_clickpad` from a buffer nothing filled.

So the fix is the initialiser **and** `rsize > 0` in the guard — which
also closes the same conversion trap for a report that genuinely comes
back empty, and preserves the intentional reuse, since in that case the
first fetch left `rsize` non-zero.

The same `rsize > 0` was added to the contact-count guard at `:324`,
where a successful `hid_get_report` returning zero bytes has exactly the
same effect.

```
--scope sys/dev/hid   1 finding -> 0   18 units, OK on both sides
```

`sys/dev/hid` is clean.

---

## `cyapawrite`: two findings, one initialiser

`cyapa.c:888` and `:1113`, both `core.UndefinedBinaryOperatorResult`, in
a Cypress trackpad's `d_write`.

```c
	int error;
	...
	while ((n = fifo_space(sc, &sc->wfifo)) > 0 && uio->uio_resid) {
		...
		error = uiomove(ptr, n, uio);
		...
	}

	/* Handle commands */
	cmd_completed = (fifo_ready(sc, &sc->wfifo) != 0);
	while (fifo_ready(sc, &sc->wfifo) && cmd_completed && error == 0) {
```

`error` is assigned only inside the copy-in loop, whose condition is
"there is FIFO space **and** the caller has bytes left". A write issued
when the FIFO is already full, or a zero-length write, runs no body —
and the command loop's condition, and the function's return, are then
decided by a stack slot. `write(fd, buf, 0)` on `/dev/cyapa*` is the
second of those.

The same shape as `u2f_read` and `cp2112iic_transfer` above: a value set
only inside a loop whose guard can be false on entry. One `= 0` clears
both findings.

```
--scope sys/dev/cyapa   2 findings -> 0   1 unit, OK on both sides
```

### And the `ntb_transport` divisions, read and left alone

`ntb_transport.c:604` and `:1337` are `core.DivideZero` on
`QP_TO_MW(nt, qp)`, which is `((qp) % nt->mw_count)`. `mw_count` cannot
be zero by the time either runs, but only through a chain of three
guards a hundred lines earlier in `ntb_transport_attach()`: zero is
rejected outright with `ENXIO`; the compact path requires
`spad_count >= 3` before it can assign `spad_count - 2`; the other path
requires `spad_count >= 6` before assigning `(spad_count - 4) / 2`. Both
floors give at least 1. The analyser follows none of it.

### The cyapa marker was red when it was committed, and the test that "passed" was vacuous

Two mistakes in one commit, both worth writing down because each has
bitten this document before in a different disguise.

**The marker was not unique.** It registered

```
	struct cyapa_softc *sc;
	int error;
```

as the text that must be *absent* once the fix is in. That opening also
begins `cyaparead()` at `:696` and `cyapaioctl()` at `:1215`, so
`check_pbsd_marks.py` reported *"bug is back"* the moment the entry was
added — correctly. It is the same defect as the `pf_snmp.c` and
`lib80211` markers before it: a marker whose text is not unique to the
site it guards. `int cmd_completed;` is what makes it `cyapawrite()`'s.

**And the failure was committed**, because only `| tail -1` of the
check's output was read, and the last line of a *failing* run —
"Neither says anything. Recover from the commit before it." — is prose,
not a verdict. `tail -1` was chosen when the check passes, where the
last line is the summary. Read the exit status, or the whole thing.

**Then the verification was vacuous.** The revert test in that same
command was

```sh
	git checkout -- hbsd/src/sys/dev/cyapa/cyapa.c
```

which restores the file from HEAD — and by then the fix was *in* HEAD,
so nothing was reverted and the check was re-run against the fixed file.
It printed a FAIL, which looked like the marker biting; it was the
non-unique marker failing for the other reason.

Once a fix is committed, the pre-fix file has to come from the commit
before it:

```sh
	git show HEAD~1:<path> > <path>
	python3 tools/check_pbsd_marks.py        # must FAIL
	git checkout -- <path>
```

Checked that way, the narrowed marker does bite. Every earlier fix in
this session was revert-tested *before* its commit, where
`git checkout --` does restore the vendor file, so those tests were
real.

---

## The fold, confirmed over all 1,862 units

The `ask_cflags` change — folding the seven program-list variables into
the bmake run the function was already making — was justified on a
sample: 37 files in 36 directories, old and new answers identical. A
sample is not the tree.

Re-run over everything, both sides at the same scope:

```
before (1820, 42, 737)  after (1820, 42, 737)
1862 units; 0 changed; 0 flag digests changed
wall 895s
```

**Zero changed. Zero flag digests changed.** Not one translation unit
got different flags, so not one could have got a different result. That
is the claim the sample could only suggest.

### And a wall-clock number worth keeping

895 seconds for the progs shard, with the build-authority caches warm.

The first attempt at this same measurement ran for over half an hour and
produced 1,104 of 1,862 rows before it was killed, because the caches
had been deleted immediately before starting it — which puts every
worker process independently into a whole-tree bmake walk, one per
architecture, forty-one of them running at once. `for_arch()` with a
cold cache is exactly the pathology recorded when the libs shard went
from twenty minutes to a projected five hours at load average 45.

The fix is one line, and it is already written down in this repository's
history: warm the caches once, serially, before the sweep.

```
python3 tools/verify/userland_names.py --refresh     # 521s, six architectures
installed_headers(a, refresh=True) for each a        # ~145s each
                                                     # 1396s total
```

Twenty-three minutes of warming, once, against a sweep that then takes
fifteen. Deleting the caches to be sure they were fresh cost more than
it could ever have saved, and the reason it looked like progress at the
time is that a stalled sweep and a slow one produce the same output:
nothing.

---

## `iwn`: all eighteen, and the lock the failure path never released

The earlier commit fixed one of `iwn_read_prom_data()`'s eighteen call
sites — the one producing `base`, the address every later read uses —
and left the other seventeen with the reason stated: the `read_eeprom`
method is `void`, so propagating an error means changing the ops
signature and both implementations, and half-doing it would leave the
file looking checked where it is not.

Done properly now. `int (*read_eeprom)(struct iwn_softc *)` in
`if_iwnvar.h:206`, and `iwn4965_read_eeprom`, `iwn5000_read_eeprom`,
`iwn_read_eeprom_channels` and `iwn_read_eeprom_enhinfo` all return
`int` and check every read. **20 of 20 calls checked**, up from 1.

What each unchecked read had been feeding, when the EEPROM timed out or
the OTPROM reported an uncorrectable ECC error:

| | |
|---|---|
| `IWN_EEPROM_SKU_CAP` | whether 11n is bonded out |
| `IWN_EEPROM_RFCFG` | the transmit and receive chain masks |
| `IWN_EEPROM_MAC` | the MAC address |
| `IWN4965_EEPROM_DOMAIN`, `IWN5000_EEPROM_DOMAIN` | the regulatory domain |
| `iwn_read_eeprom_channels` | the channel list — and note the array is indexed **per band**, so a failure on band *n* leaves band *n−1*'s channels there, not zeroes |
| `IWN4965_EEPROM_MAXPOW` | maximum transmit power for 2GHz and 5GHz |
| `IWN4965_EEPROM_BANDS` | the per-group power samples |
| `IWN5000_EEPROM_CAL` and the header at `base` | the calibration version and voltage |
| `IWN5000_EEPROM_TEMP`, `_VOLT`, `_CRYSTAL` | temperature offset and crystal calibration |
| `iwn_read_eeprom_enhinfo` | **35 structures on the kernel stack**, every one walked for per-channel transmit power |

### And a lock leak the reading found

`iwn_read_eeprom()` acquires two things and released neither on failure:

```c
	if ((error = iwn_apm_init(sc)) != 0)
		return error;                    /* adapter now powered ON */

	if ((IWN_READ(sc, IWN_EEPROM_GP) & 0x7) == 0)
		return EIO;                      /* ...and left powered on */

	if ((error = iwn_eeprom_lock(sc)) != 0)
		return error;                    /* ...still powered on */

	if (sc->sc_flags & IWN_FLAG_HAS_OTPROM) {
		if ((error = iwn_init_otprom(sc)) != 0)
			return error;            /* ROM LOCKED, adapter on */
	}
```

The success path ends with `iwn_apm_stop(sc); iwn_eeprom_unlock(sc);`.
Every failure path skipped both. Now there are two labels — `unlock:`
after the lock is taken, `poweroff:` after the power-on — and every exit
goes through the one that matches what it holds.

That was not a finding. It was visible only because propagating the
error meant reading every exit from the function.

```
--scope sys/dev/iwn   0 findings -> 0   1 unit, OK on both sides
```

The count does not move: the one finding was cleared by the earlier
commit, and the other seventeen were never reported — the analyser flags
a discarded return only when the unwritten value then reaches a
comparison or an assignment it tracks, and most of these land in softc
fields.

## Three more clknode `parent_names` arrays, and the one that never fails

`clknode_create()` copies its `struct clknode_init_def` — unless the
caller sets `CLK_NODE_STATIC_STRINGS`, both the name and the parent name
array are `strdup`'d into the node:

```c
	/* Copy all strings unless they're flagged as static. */
	if (def->flags & CLK_NODE_STATIC_STRINGS) {
		clknode->name = def->name;
		clknode->parent_names = def->parent_names;
	} else {
		clknode->name = strdup(def->name, M_CLOCK);
		clknode->parent_names =
		    strdup_list(def->parent_names, def->parent_cnt);
	}
```

Every one of the three drivers below `memset`s or `bzero`s the def, so
`flags` is 0 and the copy is always taken. The array the driver built is
therefore the driver's to free the moment `clknode_create()` returns —
and none of the three gave it back.

There is a second allocation at each site that is easier to miss.
`ofw_bus_string_list_to_array()` returns a *single* `M_OFWPROP` block
holding the pointer array and all the strings it points at:

```c
	array = malloc((cnt + 1) * sizeof(char *) + nelems, M_OFWPROP,
	    M_WAITOK);
	/* Get address of first string. */
	tptr = (char *)(array + cnt + 1);
```

so one `OF_prop_free()` releases the lot — which is exactly what
`clk_parse_ofw_clk_name()` in `sys/dev/clk/clk.c:1648` does, on both its
success and its failure return. Two of the three drivers never called it
at all.

### `sys/dev/sdhci/sdhci_fdt.c`

`sdhci_export_clocks()` allocates **inside the loop**:

```c
	for (i = 0; i < nclocks; i++) {
		...
		def.parent_names = malloc(sizeof(char *) * 1, M_OFWPROP, M_WAITOK);
```

one array per exported clock, none of them freed — plus `clknames`,
leaked on all three returns. The function returns `void`, so there was
nowhere for an unwind to hide; there now is an `out:` label, and the
per-iteration array is released immediately after `clknode_create()`,
before the `NULL` test, because it is dead either way.

### `sys/arm64/rockchip/rk_usb2phy.c`

The same pair, on more paths. `def.parent_names` leaked on the
`clk_get_by_ofw_index` failure, the `clknode_create` failure, the
`clkdom_finit` failure and the success return; `clknames` leaked on
those four *and* on

```c
	nclocks = ofw_bus_string_list_to_array(node, "clock-output-names",
	    &clknames);
	if (nclocks != 1)
		return (ENXIO);
```

— a device tree naming two output clocks where the driver wants one
returns here with the array already allocated. That return is the reason
the new `out:` label cannot simply absorb it: at that point `def` has not
been `memset` yet, so the early arm frees `clknames` on its own and the
label handles everything after.

### `sys/riscv/sifive/sifive_prci.c`

This one already had the unwind, and a comment saying where the point of
no return is:

```c
	/* We can't free a clkdom, so from now on we cannot fail. */
```

`fail1:` frees `clkdef.parent_names` and falls into `fail:`. Two arms
jumped straight to `fail:`, past it — the `clkdom_create()` NULL test and
the `clknode_gate_register()` error — and the success path, `return (0)`
after `clkdom_finit()`, never freed it either.

The `clkdom_create()` arm carried a second defect that only showed up
because the label was wrong:

```c
	sc->clkdom = clkdom_create(dev);
	if (sc->clkdom == NULL) {
		device_printf(dev, "Couldn't create clock domain\n");
		goto fail;			/* ...to `return (error)' */
	}
```

`error` at that point holds the 0 that the last successful
`clk_get_by_ofw_index()` left in it. `fail:` ends `return (error)`, so a
clock domain that could not be created was reported to newbus as a
successful attach — resources released, mutex destroyed, and a device
the rest of the system believes is there. It now sets `ENXIO` and goes
to `fail1:` like the rest.

```
--scope sys/arm64/rockchip   4 findings -> 3   19 units, OK on both sides
--scope sys/dev/sdhci        1 finding  -> 1   13 units, OK on both sides
--scope sys/riscv/sifive     0 findings -> 0    7 units, OK on both sides
```

Only the rockchip one was ever a finding
(`rk_usb2phy.c:292 unix.Malloc`). The other two are leaks the analyser
does not report: in `sdhci_fdt.c` the allocation is handed to
`clknode_create()`, an unknown function as far as the checker is
concerned, which is enough for it to stop tracking; in `sifive_prci.c`
there *is* a `free()` of the pointer in the function, on the path the
analyser happens to walk. Neither absence says anything about the leak.

## Four sound drivers that leak their softc, and where the line is

Task #109 was parked on a question, not on a defect: `ssi_attach()` and
`sai_attach()` leak three allocations on seven returns each, but two of
those returns are *after* `pcm_init(dev, scp)`, and freeing there would
turn a leak into a use-after-free. The unwind could not be written until
the sound(4) ownership contract was established rather than guessed.

### The contract

`sys/dev/sound/pcm/sound.c` answers it in three places.

`pcm_init()` stores the pointer and nothing else:

```c
	d = device_get_softc(dev);
	...
	d->devinfo = devinfo;
```

`pcm_register()` never touches `devinfo`; it sets `SD_F_REGISTERED`,
creates the sysctl trees and the dsp device. `pcm_unregister()` tears all
of that down — `pcm_killchans`, both sysctl contexts, `sndstat_unregister`,
`mixer_uninit`, `dsp_destroy_dev`, `cv_destroy`, `mtx_destroy` — and
frees **nothing** the driver allocated. There is no `free(d->devinfo)`
anywhere in the sound layer.

So the devinfo is the driver's memory for its whole life: sound(4)
borrows the pointer and hands it back to every method. That is why the
drivers in the tree that malloc a softc free it in their own `detach`
(`es137x.c`'s `free(es, M_DEVBUF)` is the pattern).

What it does *not* license is freeing on an attach failure that happens
after `pcm_init()`. By then three other things hold the pointer:
`d->devinfo`, the channel `pcm_addchan()` created, and — in all four
drivers here — the interrupt handler installed a few lines earlier. The
line is `pcm_init()`. Before it, the allocation is the attach path's
alone and must be unwound; after it, the leak stays, because the
alternative is worse. Each of the four now says so in a comment at
exactly that point.

### `sys/arm/freescale/imx/imx6_ssi.c`, `sys/arm/freescale/vybrid/vf_sai.c`

Seven returns each, before `pcm_init()`, dropping in turn: `sc`
(`M_WAITOK`), `sc->conf` (`M_WAITOK`, imx only), `scp` (`M_NOWAIT`), the
softc mutex, the `bus_alloc_resources()` set, the DMA tag, the DMA
memory and the DMA map. The analyser named one of the eight —

```
sys/arm/freescale/imx/imx6_ssi.c:797  [unix.Malloc]
    Potential leak of memory pointed to by 'scp'
```

— because `scp` is the only one whose `malloc` it can follow to a return
without an intervening store into a structure it has stopped tracking.
Fixing what it named and stopping there would have left the function
half-unwound, which is the whole reason a finding is a starting point
rather than a work item.

### `sys/dev/sound/macio/i2s.c`, `sys/dev/sound/macio/davbus.c`

The same shape on PowerPC. `i2s_attach()` has seven such returns and
`davbus_attach()` four, and the stage order differs between them:
`i2s_attach()` does `mtx_init()` *before* mapping its resources, so its
unwind has a `fail_mtx:` stage that `davbus_attach()`'s does not need —
one more reason to read each rather than copy the first.

`i2s_attach()` also establishes a `config_intrhook` holding the softc.
That hook is disestablished and freed by `i2s_postattach()` when it runs,
so the only path that has to undo it is the one where
`config_intrhook_establish()` itself fails — which cannot have
established anything.

```
--scope sys/arm/freescale/imx      2 findings -> 1   23 units, OK both sides
--scope sys/arm/freescale/vybrid   2 findings -> 1   17 units, OK both sides
--scope sys/dev/sound/macio        2 findings -> 0    6 units, OK both sides
```

The two that remain are the unrelated `sr` null-dereference in each
Freescale driver's rate lookup.

### Two things read on the way that are not fixed here

`davbus_attach()` passes the same `void *cookie` to `snd_setup_intr()`
and then to `bus_setup_intr()` for the control interrupt, so the first
cookie is overwritten and lost. It costs nothing today because the driver
has no `detach` method at all and never tears either handler down; it
would matter the moment one is written.

`i2s_delayed_attach` is a single file-scope `struct intr_config_hook *`,
not a per-softc field. A second i2s device would overwrite the first
one's hook pointer between `config_intrhook_establish()` and
`i2s_postattach()`. Every Apple machine this driver runs on has one, so
it is a latent bug rather than a live one.

## `iommu_bus_dma_tag_create`: the test and the dereference disagreed

```c
	*dmat = NULL;
	error = common_bus_dma_tag_create(parent != NULL ?
	    &((struct bus_dma_tag_iommu *)parent)->common : NULL, alignment,
	    ...);
	if (error != 0)
		goto out;

	oldtag = (struct bus_dma_tag_iommu *)parent;
	newtag->common.impl = &bus_dma_iommu_impl;
	newtag->ctx = oldtag->ctx;
	newtag->owner = oldtag->owner;
```

The first statement says `parent` may be NULL. Three lines later
`oldtag->ctx` says it may not. Both cannot be right, and the finding —

```
sys/dev/iommu/busdma_iommu.c:393  [core.NullDereference]
    Access to field 'ctx' results in a dereference of a null pointer
    (loaded from variable 'oldtag')
```

— is the analyser reading the first as a claim about the second.

Which half is wrong is answerable without judgement. This function is
reachable at exactly one call site, `bus_dma_iommu_impl.tag_create`, and
`bus_dma_tag_create()` reaches a tag's `impl` only in its non-NULL arm:

```c
	if (parent == NULL) {
		error = bus_dma_bounce_impl.tag_create(parent, ...);
	} else {
		tc = (struct bus_dma_tag_common *)parent;
		error = tc->impl->tag_create(parent, ...);
	}
```

— `sys/x86/x86/busdma_machdep.c:151`, `sys/arm64/arm64/busdma_machdep.c:134`,
`sys/riscv/riscv/busdma_machdep.c:128`; those are the three architectures
that build this file. A NULL parent never gets an IOMMU tag; it gets a
bounce tag. So the conditional is the dead half, and the dereference is
correct.

Removing the conditional and stating the invariant as a `KASSERT` is the
fix. It is not a behaviour change in either direction: without
`INVARIANTS`, a NULL parent would have faulted on `oldtag->ctx` exactly
as before; with it, the panic now names the reason.

```
--scope sys/dev/iommu   2 findings -> 1   2 units, OK on both sides
```

The one that remains is `iommu_gas.c:187`, which is the `RB_GENERATE()`
line — a finding inside the generated red-black tree code, of the class
already triaged in sweep 17, not a defect in this file.

## Task #61, first batch: six drivers, and one finding that is the instrument

The remaining findings from the static-taken bucket were read one
directory at a time, at `--scope` granularity, rather than from a full
sweep's row.

### `sys/dev/adlink/adlink.c` — an empty ring, from a 0444 device

`ADLINK_SETRINGSIZE` rejects a size that is not a multiple of the chunk
size — *when a chunk size has already been set*:

```c
	case ADLINK_SETRINGSIZE:
		...
		if (sc->p0->chunksize != 0 && u % sc->p0->chunksize)
			return (EINVAL);
		sc->p0->ringsize = u;
```

`ADLINK_START` then applies its defaults, after that check can no longer
run:

```c
			if (sc->p0->chunksize == 0)
				sc->p0->chunksize = 4 * PAGE_SIZE;
			...
			sc->nchunks = sc->p0->ringsize / sc->p0->chunksize;
```

So: set the ring to one page, never set a chunk size, START. The chunk
becomes four pages and `nchunks` is **0**. `malloc(0, M_DEVBUF, M_WAITOK
| M_ZERO)` returns a valid pointer, the loop that fills `pg->sample`
never runs, and the DMA setup a few lines later does

```c
		pg = sc->next = sc->chunks;
		*(pg->sample) = 0;
```

on the NULL that `M_ZERO` left there. `make_dev(..., UID_ROOT, GID_WHEEL,
0444, "adlink%d")` and an ioctl handler that never looks at `fflag` make
that any local user. One `if (sc->nchunks == 0) return (EINVAL);`.

### `sys/dev/acpi_support/acpi_asus_wmi.c` — a read that failed, written back

```c
	if (ACPI_FAILURE(ACPI_WMI_EVALUATE_CALL(...))) {
		acpi_asus_wmi_free_buffer(&out);
		return (-EINVAL);		/* *retval not written */
	}
```

Two hotkey handlers discarded that return and used the uninitialised
`val` immediately — `val &= 0x3` for the keyboard backlight, `val = !(val
& 1)` for the touchpad — and then wrote the result **back to the
firmware** with `acpi_wpi_asus_set_devstate()`, caching it in
`sc->kbd_bkl_level` in the backlight case. Not reading a value is a
reason to do nothing, not a reason to write one. The third call site,
in `acpi_asus_wmi_sysctl_get()`, was already safe: its `val` is
initialised to 0.

### `sys/dev/acpi_support/acpi_asus.c` — the union that makes the P30 work

```c
	AcpiEvaluateObject(sc->handle, "INIT", &Args, &Buf);
	Obj = Buf.Pointer;

	/*
	 * The Samsung P30 returns a null-pointer from INIT, we
	 * can identify it from the 'ODEM' string in the DSDT.
	 */
	if (Obj->String.Pointer == NULL) {
```

Two separate things. A failed evaluation leaves `Buf.Pointer` NULL with
`ACPI_ALLOCATE_BUFFER`, and the next statement reads through it — that
is not the Samsung case, which works because the method returns an
*integer* 0 whose `Integer.Value` shares the union offset with
`String.Pointer`. The new guard covers the machine whose INIT is missing
or fails and leaves the P30 path exactly where it was.

The finding itself is the other one: when neither `ODEM` nor `ASUS010`
matches, the block falls through into

```c
	for (model = acpi_asus_models; model->name != NULL; model++) {
		if (strncmp(Obj->String.Pointer, model->name, 3) == 0) {
```

with the NULL the block exists because of. It now returns `ENXIO`.

### `sys/dev/usb/input/wmt.c` — hmt.c's `rsize`, one driver over

`int err;` assigned only inside two guarded feature-report fetches and
read afterwards as `if (err == 0)`. A device whose Contact Count Maximum
report is absent and whose Button Type report shares that report's id
skips both, and `sc->is_clickpad` was decided by calling
`hid_get_udata()` over a buffer nothing had filled.

`= 0` would be worse than nothing — it makes the guard reliably *pass*.
The initialiser is `USB_ERR_INVAL`, which preserves the intentional
reuse the second fetch's `btn_type_rid != cont_max_rid` condition exists
for (there, the first fetch ran and left its own status) and closes the
case where neither ran.

### `sys/contrib/vchiq/.../vchiq_arm.c` — `service` where `service1` was meant

```c
		while (instance->completion_remove !=
			instance->completion_insert) {
			...
			service1 = completion->service_userdata;
			if (completion->reason == VCHIQ_SERVICE_CLOSED)
			{
				USER_SERVICE_T *user_service =
					service->base.userdata;
```

`service` is the variable of the loop *directly above*, which ended
because `next_service_by_instance()` returned NULL — so it is NULL on
every path that reaches here. Everything else in the block already uses
`service1`, including the `unlock_service(service1)` two lines down.

### `sys/dev/sbni/if_sbni_isa.c` — a type pun the compiler may delete

```c
	*(u_int32_t*)&flags = device_get_flags(dev);

	sbni_attach(sc, device_get_unit(dev) * 2, flags);
```

A `struct sbni_flags` object written through a `u_int32_t` lvalue: a
strict-aliasing violation, which is why the analyser saw `flags` passed
by value with every field indeterminate. A union keeps the bit layout
exactly as it was on every platform and is defined behaviour. The PCI
attach never had the problem — it does `memset(&flags, 0, sizeof(flags))`.

### `sys/dev/mgb/if_mgb.c:434` — not a defect, the instrument

```c
	mgb_get_ethaddr(sc, &hwaddr);
	if (ETHER_IS_BROADCAST(hwaddr.octet) || ...
```

`mgb_get_ethaddr()` fills the address with `bus_read_region_1()`, and on
x86 `bus_space_read_region_1()` is a `static __inline` whose entire body
is `__asm __volatile` — `rep movsb`, or `inb`/`stosb` for I/O space.
This is the class the `inline_asm_write.c` probe established earlier in
this session: the analyser steps *into* a visible inline function,
finds no C store to `*addr`, and comes back with the buffer still
undefined. It does invalidate a by-pointer argument to an *unknown*
function; a visible one whose body it cannot interpret is the exact
inverse. Left alone, and recorded here so it is not read again.

```
--scope sys/dev/adlink         1 finding  -> 0    1 unit,  OK both sides
--scope sys/dev/acpi_support   3 findings -> 0   12 units, OK both sides
--scope sys/dev/usb/input      1 finding  -> 0    9 units, OK both sides
--scope sys/contrib/vchiq      1 finding  -> 0   11 units, 4 ERROR both sides
--scope sys/dev/sbni           1 finding  -> 0    3 units, OK both sides
--scope sys/dev/mgb            1 finding  -> 1    1 unit,  OK both sides
```

### A method note

Six markers were "verified" in one shell loop that printed `exit=0` for
every one of them — and they all bite. The loop said

```sh
    echo "$(basename $p) reverted -> exit=$?"
```

and `$?` there is the status of `basename`, not of the check. The same
lesson as `| tail -1` two commits ago, in a new disguise: the verdict has
to be read from the thing that produced it. Re-run with the status
captured into a variable first, all six failed as they should.

## Task #61, second batch: five drivers, and eight findings that are guards

### `sys/dev/aic7xxx/aic7xxx_pci.c` — a resume that drives the SEEPROM from stack

`ahc_parse_pci_eeprom()` builds its `struct seeprom_descriptor` in full:

```c
	sd.sd_ahc = ahc;
	sd.sd_control_offset = SEECTL;
	sd.sd_status_offset = SEECTL;
	sd.sd_dataout_offset = SEECTL;
	...
	sd.sd_MS = SEEMS;
	sd.sd_RDY = SEERDY;
	sd.sd_CS = SEECS;
	sd.sd_CK = SEECK;
	sd.sd_DO = SEEDO;
	sd.sd_DI = SEEDI;
```

`ahc_pci_resume()` declares its own and sets **only the four offsets**.
It then calls `ahc_acquire_seeprom()`, which does `SEEPROM_OUTB(sd,
sd->sd_MS)` — a write to the SEECTL register — and spins on
`SEEPROM_STATUS_INB(sd) & sd->sd_RDY`, and `configure_termination()`,
which writes `sd->sd_MS | sd->sd_CS`. So a resume on any controller with
`AHC_HAS_TERM_LOGIC` drove the SEEPROM control register from
uninitialised stack and decided the SCSI bus termination from what came
back. Both findings in that file were this one bug, seen from two
functions.

### `sys/dev/mana/hw_channel.c` — the NULL arm printed the NULL

```c
	if (!hwc_txq || hwc_txq->gdma_wq->id != gdma_txq_id) {
		mana_warn(NULL, "unmatched tx queue %u != %u\n",
		    hwc_txq->gdma_wq->id, gdma_txq_id);
	}

	bus_dmamap_sync(hwc_txq->gdma_wq->mem_info.dma_tag, ...);
```

`!hwc_txq` short-circuits *into* the body, which reads
`hwc_txq->gdma_wq->id`. And there is no `return`, so a queue id that did
not match was warned about and then acted on anyway. The rx handler ten
lines up is the shape this was meant to have: warn, and return.

### `sys/dev/axgbe` — an unvalidated device tree property, twice over

```c
	} else if (ad_reg & 0x20) {
		switch (pdata->speed_set) {
		case XGBE_SPEEDSET_1000_10000: ... break;
		case XGBE_SPEEDSET_2500_10000: ... break;
		}
	} else {
		mode = XGBE_MODE_UNKNOWN;
```

No `default`, and `speed_set` is `amd,speed-set` taken verbatim from the
device tree — `if_axgbe.c` checks only that `OF_getencprop()` returned
something. A third value falls out of the switch and
`xgbe_an73_outcome()` returns an unwritten `mode` to the caller that
programs the PHY. Fixed at both ends: a `default` that gives the same
answer the `else` arm gives, and a range check where the property is
read, since two more switches elsewhere in the driver read the same
field.

`xgbe_phy_redrv_write()` builds a five-byte redriver command and writes
two of the bytes through `(__be16 *)&redrv_data[2]` — the same
aliasing violation as `if_bwn.c` and `if_sbni_isa.c` below — then
computes a checksum over all four. `memcpy` writes the same bytes and is
defined.

`xgbe_rx()` assigns `buf2_len` only inside its `pdata->sph_enable` arm
and prints it as the eighth argument of an `axgbe_printf()` that fires
whenever `packet->errors` is set. With split-header receive off, that is
an unwritten stack slot.

### `sys/dev/bwn/if_bwn.c` — the third aliasing pun this session

```c
	uint8_t noise[4];
	...
	*((uint32_t *)noise) = htole32(bwn_jssi_read(mac));
```

A `uint32_t` stored into an object whose declared type is `uint8_t[4]`.
`memcpy` of the same four bytes.

### `sys/dev/irdma` — eight findings, three shapes, no defect

Read in full and left alone:

| Where | Why it is not a defect |
|---|---|
| `icrdma.c:421`, `:445` | the `irdma_debug(h, ...)` macro's own first branch is `if (!(h))`, and `h` is `&rf->sc_dev`. Under that branch the analyser has `rf == NULL`, and the *arguments* it then evaluates include `rf->peer_info->pf_id`. The macro's defensiveness is what produces the finding. |
| `irdma_hw.c:615`, `:681`, `:748` | `iw_msixtbl` is tested for NULL where it is allocated, and the analyser carries that into the teardown paths. `irdma_del_ceq_0()` runs only in state `CEQ0_CREATED` and `irdma_destroy_aeq()` only when `rf->rsrc_created` — both reached only after the allocation succeeded. |
| `irdma_puda.c:495`, `:507` | `irdma_puda_send_buf()` sets `info.ah_id` on the `hw_rev >= IRDMA_GEN_2` branch and `info.maclen`/`do_lpb` on the other; `irdma_puda_send()` tests the same thing again and reads each on the matching arm. `qp->dev = rsrc->dev` (`irdma_puda.c:720`), so it is one predicate, tested twice across a call. |
| `irdma_cm.c:3965` | `irdma_create_cm_node()` writes `*caller_cm_node` on every path that returns 0; the only early return is `if (IS_ERR(cm_node)) return PTR_ERR(cm_node);`, and `IS_ERR` true implies `PTR_ERR` in `[-MAX_ERRNO, -1]`. The analyser does not model that pair. |

### `sys/dev/aic7xxx` — the five that remain

`aic7xxx_osm.c:853` and `aic79xx_osm.c:872` are `scsi->flags &=
~CTS_SCSI_FLAGS_TAG_ENB` on a `struct ccb_trans_settings` the CAM caller
supplies. Reading the caller's bits and clearing one is the
XPT_GET_TRAN_SETTINGS contract, not a defect in this driver.

`aic7xxx_osm.c:552` is `ahc->pending_device == lstate` inside `if
(ccb->ccb_h.func_code == XPT_CONT_TARGET_IO)`. `lstate` is assigned on
exactly that path, in the target-mode case that falls through into this
one — but `ahc_find_tmode_devs()` writes through `ccb`, so the analyser
loses the earlier `func_code` equality and reaches the second test with
`lstate` unset.

`aic79xx_osm.h:193` and `aic79xx.c:7254` are the allocation-check class
again: `platform_data` and `scb->hscb` are each tested for NULL where
they are allocated.

```
--scope sys/dev/aic7xxx   7 findings -> 5   16 units, 3 ERROR both sides
--scope sys/dev/mana      2 findings -> 0    6 units, OK both sides
--scope sys/dev/axgbe     3 findings -> 0   14 units, 1 ERROR both sides
--scope sys/dev/bwn       1 finding  -> 0    7 units, OK both sides
--scope sys/dev/irdma     8 findings -> 8   15 units, OK both sides
```

Three aliasing puns in one session — `if_sbni_isa.c`, `if_bwn.c`,
`xgbe-phy-v2.c` — all of the same shape: a wide store through a cast
pointer into a narrow array, and every later read of that array reported
as a garbage value. The analyser is right to refuse to model them: the
compiler is entitled not to perform the store at all.

## Task #61, third batch: the five Intel network drivers

### `sys/dev/ice/ice_nvm.c` — an NVM checksum decided on a stack word

```c
	ice_read_sr_word(hw, ICE_SR_SW_CHECKSUM_WORD, &checksum_sr);

	/* Verify read checksum from EEPROM is the same as
	 * calculated checksum
	 */
	if (checksum_local != checksum_sr)
		status = ICE_ERR_NVM_CHECKSUM;
```

`ice_read_sr_word()` leaves `checksum_sr` unwritten when it fails, and
its return here is discarded. So `ice_nvm_validate_checksum()` could
compare the checksum it computed against whatever was on the stack — and
report the NVM valid when it never read the stored checksum at all. The
function above it, `ice_calc_sr_checksum()`, is checked; this one was
not. A checksum that cannot be read is not a checksum that matches.

### `sys/dev/igc/igc_phy.c` — a "success" that says nothing

```c
	if (!hw->phy.ops.read_reg)
		return IGC_SUCCESS;
```

`*success` is the whole output of `igc_phy_has_link_generic()`, and this
arm returns without writing it. All four callers declare `bool link;` on
the stack and branch on it immediately — `igc_phy.c:568` does `if
(link)` and, when it is set, calls `config_collision_dist()` and
`igc_config_fc_after_link_up_generic()`. A PHY with no `read_reg` method
is a PHY whose link state is unknown, which is not a link.

### `sys/dev/igc/if_igc.c` — two of three accumulators zeroed

```c
	bytes = bytes_per_packet = 0;
```

`packets` is the third, and it is assigned only inside `if (txpackets !=
0)` and `if (rxpackets != 0)`. The early return above tests the *byte*
counters, and all four counters are read with separate
`atomic_load_long()`, so a ring whose bytes have been accounted before
its packets reaches `lmax(packets, rxpackets)` and the latency state
machine with the word never written.

### `sys/dev/ixl/if_ixl.c` — an out-parameter left behind on two paths

`ixl_process_adminq(pf, &pending)` returns `ENOMEM` without writing
`*pending` when its buffer allocation fails, and `break`s out of its loop
without writing it if `i40e_clean_arq_element()` fails on the first pass.
`ixl_if_update_admin_status()` declares `u16 pending;`, discards the
return, and then chooses between `iflib_admin_intr_deferred()` and going
back to sleep on `pending > 0`. `*pending = 0` at the top: nothing
processed means nothing pending.

### `sys/dev/ice/if_ice_iflib.c` — a resource id from the stack

```c
	int rid;
	for (i = 0, vector = 1; i < vsi->num_rx_queues; i++, vector++) {
		...
		rid = vector + 1;
	...
	/* For future interrupt assignments */
	sc->last_rid = rid + sc->irdma_vectors;
```

A VSI with no receive queues publishes `sc->last_rid` from an unwritten
`rid`, and every later allocation counts from it. The right value there
is 1 — the administrative vector's, allocated above the loop.

### `sys/dev/ice/ice_common.c` — a callee that ORs into its out-parameter

`ice_get_link_default_override()` assigns `options`, `phy_config` and
`fec_options`, but builds `phy_type_low` and `phy_type_high` with `|=`,
one 16-bit word at a time. `ice_lib.c:9880` declares its tlv as `= { 0
}`; `ice_common.c:4082` does not. A callee that ORs into its
out-parameter has to own the zero, so it now clears both before the
loops.

### The five that are one idiom

`ixl_txrx.c:399`, `iavf_txrx_iflib.c:386`, `ice_iflib_txrx.c:181`,
`igc_txrx.c:319` and `ix_txrx.c:240` are the same iflib transmit-encap
shape in five drivers:

```c
	struct i40e_tx_desc *txd = NULL;
	...
	for (j = 0; j < nsegs; j++) {
		txd = &txr->tx_base[i];
		...
	}
	/* Set the last descriptor for report */
	txd->cmd_type_offset_bsz |= ...
```

`nsegs` is `pi->ipi_nsegs`, and iflib never calls `isc_txd_encap` with
zero segments. The analyser has no way to know that, and the `= NULL`
initialiser — which is what makes the code safe to read — is what turns
the unreachable path into a null dereference it can name.

### And two more that are invariants

`ice_sched.c:279` passes `node->parent` to `ice_sched_remove_elems()`,
which dereferences it. Four lines below the call the same function says
`/* root has no parent */`, which is what the analyser follows — but the
call is guarded by `elem_type != ICE_AQC_ELEM_TYPE_ROOT_PORT`, so the one
node with no parent never reaches it.

`ice_bitops.h:230` is `dst[i] = (dst[i] & ~mask) | ...`, and the comment
directly above it says why the old `dst[i]` is read: "we won't directly
assign the last bitmap, but instead use a bitmask to ensure we only
modify bits which are within the size, and leave any bits above the size
value alone." Reading the caller's bits is the point of the line.

```
--scope sys/dev/ice       6 findings -> 3   22 units, OK both sides
--scope sys/dev/igc       5 findings -> 1    8 units, OK both sides
--scope sys/dev/ixl       2 findings -> 1   15 units, OK both sides
--scope sys/dev/ixgbe     1 finding  -> 1   21 units, OK both sides
--scope sys/dev/iavf      1 finding  -> 1    8 units, OK both sides
```

## Task #61, fourth batch: the linuxulator and Hyper-V

### `sys/compat/linux/linux_ioctl.c` — the bound tested before the value arrives

```c
	bp->format = lp->type;
	switch (bp->format) {
	case DVD_STRUCT_PHYSICAL:
		if (bp->layer_num >= 4)
			return (EINVAL);
		bp->layer_num = lp->physical.layer_num;
```

`bp` is the caller's `struct dvd_struct` on the stack. The check reads
`bp->layer_num` before anything has written it, and then stores
`lp->physical.layer_num` — which came from userland — **unchecked**. The
guard was doing nothing except reading indeterminate memory. Store,
then check.

### `sys/compat/linux/linux_misc.c` — an errno from the stack

`linux_prlimit64()` with `args->pid == 0` and both `args->new` and
`args->old` NULL assigns `error` nowhere, and ends `return (error)`. The
syscall layer takes that as the errno, so `prlimit64(0, resource, NULL,
NULL)` from any Linux binary returned a stack word.

### `sys/compat/linux/linux_signal.c` — td_retval from an osa nobody wrote

```c
	error = linux_do_sigaction(td, args->sig, &nsa, &osa);
	td->td_retval[0] = (int)(intptr_t)osa.lsa_handler;
```

Every failing return in `linux_do_sigaction()` is before the
`bsd_to_linux_sigaction(osa, linux_osa)` that fills it. The syscall
layer discards `td_retval` when `error` is set, which is why nothing was
ever seen — the read is still undefined, and the guard is one line.

### `sys/dev/hyperv/pcib/vmbus_pcib.c` — a length from the stack, to the hypervisor

`hv_pci_map_msi()` builds an interrupt-creation packet in a `switch
(hpdev->hbus->protocol_version)` with cases for 1.1 and 1.4, no
`default`, and then

```c
	ret = vmbus_chan_send(sc->chan, VMBUS_CHANPKT_TYPE_INBAND,
	    VMBUS_CHANPKT_FLAG_RC, &ctxt.int_pkts, size,
```

`size` is the byte count. The switch is exhaustive only as long as
`pci_protocol_versions[]`, two hundred lines above, holds exactly those
two entries. It does today. A third would send an unwritten stack word's
worth of `ctxt.int_pkts` to the host. The `default` makes the invariant
local and fails where it is broken — the same move as the IOMMU
`KASSERT` earlier in this document.

### `sys/dev/hyperv/netvsc/hn_rndis.c` — a success that copies nothing

```c
	if (comp->rm_infobuflen == 0 || comp->rm_infobufoffset == 0) {
		/* No output data! */
		if_printf(sc->hn_ifp, "RNDIS query 0x%08x, no data\n", oid);
		*odlen0 = 0;
		error = 0;
		goto done;
	}
```

`hn_rndis_query2()` returns 0 with the caller's buffer untouched, and
also returns 0 after a *short* copy when `comp->rm_infobuflen < odlen`.
The three `hn_rndis_query()` callers all check the returned length —
`hn_rndis_get_eaddr()`, `_linkstatus()` and `_mtu()` each reject a
length that is not exactly what they asked for. The two that call
`hn_rndis_query2()` directly, for the RSS and offload capability
structures, read the struct immediately. Both now check the length
first, which is what the finding at `:433` and `:973` was pointing at.

### `sys/dev/hyperv/netvsc/if_hn.c` — the fifth member

```c
	info.vlan_info = NULL;
	info.csum_info = NULL;
	info.hash_info = NULL;
	info.pktinfo_id = NULL;
```

`struct hn_rxinfo` has five members. `hash_value` is the one not
cleared, and `hn_rsc_add_data()` copies it into `rxr->rsc.hash_value`
unconditionally. `hn_rndis_rxinfo()` writes it only when the host sends
an `NDIS_PKTINFO_TYPE_HASHVAL`, and signals its absence by setting
`hash_info` to NULL instead — so the pointer copied on every other
packet was whatever was on the stack.

### What is left, and why

`linux_mib.c:383` and `:549` are the M_WAITOK class: `linux_alloc_prison()`
does `nlpr = malloc(sizeof(struct linux_prison), M_PRISON, M_WAITOK)` and
hands `nlpr` back through `*lprp`, and the analyser models `malloc` as
possibly-NULL. Task #36 exists to settle that premise before anything is
built on it, so these two wait for it.

`linux_emul.c:167` and `hv_sock.c:1401` are the defensive-check class —
a `td` and a `so` tested for NULL somewhere in the same translation unit.

The 68 remaining in `linux_socket.c` are the socketcall bucket, read in
full under task #90 and covered by `tools/verify/socketcall_args.py`.

```
--scope sys/compat/linux   77 findings -> 74   31 units, OK both sides
--scope sys/dev/hyperv      5 findings ->  1   31 units, OK both sides
--scope sys/dev/athk        0 translation units - the driver's sources are
                            not in a directory the sweep reaches
```

## Task #61, fifth batch: mlx5

### `mlx5_fs_core.c` — a function that tests `dest` twice and derefs it between

```c
	if (dest && (dest->type == MLX5_FLOW_DESTINATION_TYPE_COUNTER))
		return counter_is_valid(action);
	...
	if (ignore_level) {
		...
		if (dest->type == MLX5_FLOW_DESTINATION_TYPE_FLOW_TABLE &&
		    ft->type != dest->ft->type)
			return false;
	}

	if (!dest || ((dest->type == ...
```

`dest_is_valid()` guards `dest` above and below and dereferences it in
between. A flow rule with `FLOW_ACT_IGNORE_FLOW_LEVEL` and
`MLX5_FLOW_CONTEXT_ACTION_FWD_DEST` but no destination reaches that line.
The added `dest != NULL` changes no outcome — a NULL `dest` falls through
to the `!dest ||` below and gets the same `false`.

### `mlx5_vsc.c` — a config-space write from the stack

`MLX5_VSC_SET()` is a read-modify-write of one bitfield:

```c
	*((__le32 *)(p) + __mlx5_dw_off(typ, fld)) =
	cpu_to_le32((le32_to_cpu(*((__le32 *)(p) + __mlx5_dw_off(typ, fld))) &
		     (~__mlx5_dw_mask(typ, fld))) | ...
```

`mlx5_vsc_write()` and `mlx5_vsc_set_space()` both declare their word
`= 0`. `mlx5_vsc_read()` declared `u32 in;` — so every bit outside
`address`, the flag bit among them, was whatever the stack held, and the
whole word went to the device's VSC address register.

### `mlx5_port.c` — a link toggled from an unread status

```c
	mlx5_query_port_admin_status(dev, &ps);
	mlx5_set_port_status(dev, MLX5_PORT_DOWN);
	if (ps == MLX5_PORT_UP)
		mlx5_set_port_status(dev, MLX5_PORT_UP);
```

A failed firmware command leaves `ps` unwritten, and the test decides
whether to bring the port back up. Either outcome is wrong: an
administratively-up port left down, or one the administrator had put
down raised. Toggling a port whose state cannot be read is worse than
not toggling it.

### `mlx5_fwdump.c` — the wrong struct member

```c
		fw_data = kmem_malloc(fu->img_fw_data_len, M_WAITOK);
		if (fake_fw.data == NULL) {
```

`fake_fw` is a local `struct firmware` whose `bzero()` is four lines
below, inside the `error == 0` arm. The check is for the allocation on
the line above it.

### `mlx5_en_main.c` — a void function that discards an error

`mlx5e_get_wqe_sz()` returns `-ENOMEM` without writing either output when
the segment count would exceed `MLX5E_MAX_BUSDMA_RX_SEGS`. Its other two
call sites check; `mlx5e_build_rq_param()` is `void` and did not, so a
failure built the receive queue's stride and size out of two unwritten
stack words. The fallback is that function's own answer for an MTU that
fits in one cluster — `maxs <= MCLBYTES` gives `r = MCLBYTES` and
`n = roundup_pow_of_two(2) - 1 = 1` — not an invented pair.

```
--scope sys/dev/mlx5   6 findings -> 1   74 units, 8 ERROR both sides
```

The one that remains, `mlx5_ib_cq.c:1290`, is the `IS_ERR`/`PTR_ERR`
pair again: `resize_user()` returns `PTR_ERR(umem)` on the only path that
does not write `*npas`, and `IS_ERR` true implies that value is in
`[-MAX_ERRNO, -1]`. Same shape as `irdma_cm.c:3965`.

## Task #61, sixth batch: cxgbe, and a warning about counting from a `head`

### `sys/dev/cxgbe/t4_sched.c` — a rate mode never chosen

```c
		} else if (p->rateunit == SCHED_CLASS_RATEUNIT_PKTS) {
			/* maxrate is the absolute value in pps. */
			check_pktsize = true;
			fw_rateunit = FW_SCHED_PARAMS_UNIT_PKTRATE;
		} else
			return (EINVAL);
```

Every other arm of this decision sets both `fw_rateunit` and
`fw_ratemode`; the packet-rate one sets the unit only. `fw_ratemode`
then reaches `tc->ratemode = fw_ratemode` and the sixth argument of
`t4_sched_params()` — the two findings — unwritten. `ABS` is what the
comment on that branch says it means and what the other absolute arm
sets.

```
--scope sys/dev/cxgbe   21 findings -> 19   47 units, OK both sides
```

### A counting mistake worth writing down

The first look at this scope was `... | head -30`, which showed fourteen
findings, and the "after" run reported nineteen. That looked like a
regression caused by the fix. It was not: the scope had **21**, and
`head -30` had cut the list at fourteen because each finding takes two
lines. The saved output file had the real total on its summary line all
along.

The same shape as `| tail -1` and `$(basename $p)` earlier in this
session: a number read from a truncated view of the thing that produced
it. Read the summary line, not the part of the list that fitted.

### The nineteen that remain, by shape

Six are in `t4_sge.c` (`:3643`, `:3720`, `:4189`, `:4365`, `:4420`,
`:4446`) and are the iflib/mp_ring descriptor idioms — the same family as
`ixl_txrx.c:399`. Three are in `iw_cxgbe` (`cm.c:186`, `cq.c:791`,
`qp.c:1846`), all NULL tests on a `cm_id`, a `qhp` and a `ucontext` that
the RDMA layer guarantees. `t4_cpl_io.c:583` is
`struct sglist_seg segs[n];`, a VLA whose `n` is "the maximum segments in
any one mbuf" and is at least 1 whenever the `KASSERT(nsegs > 0)` on the
next line holds — an invariant one function up. The rest —
`fastlz_api.c:386`, `cudbg_lib.c:1837`, `t4_hw.c:3550`,
`t4_mp_ring.c:318`, `t4_filter.c:1259`, `t4_listen.c:1519`,
`t4_main.c:10240`, `cxgbei.c:630`, `t4_tls.c:1037` — are unread and stay
on task #61.

## `sys/contrib/openzfs`: fifty findings, and what they are not

```
--scope sys/contrib/openzfs/module/zfs   50 findings, 137 units, all OK
```

Unread, and now task #112. One thing was established first, because it
decides whether reading them is worth anything.

The obvious guess is that they are the assertion class — ZFS is written
with `ASSERT` everywhere, and an assertion the analyser cannot see is a
guard it cannot honour. In
`sys/contrib/openzfs/include/os/freebsd/spl/sys/debug.h`:

```c
#ifdef NDEBUG
#define	ASSERT(x)		((void) sizeof ((uintptr_t)(x)))
...
#else
#define	ASSERT		VERIFY
#endif
```

The sweep does **not** pass `-DNDEBUG` for this tree —
`include_flags()` for `module/zfs/vdev_raidz.c` gives `-DBUILDING_ZFS`
and `-DZFS` and nothing else of the kind. So every `ASSERT` is a
`VERIFY`, a real check, and the analyser is following all of them. The
fifty are not assertion artefacts.

The three in `vdev_raidz.c` bear that out. They are
`parity_valid[VDEV_RAIDZ_P]` and `[VDEV_RAIDZ_Q]` reads in
`vdev_raidz_reconstruct()`, each preceded by
`ASSERT(rr->rr_firstdatacol > 1)`, and the analyser does honour that
constraint. What it cannot do is relate the loop above —

```c
		if (c < rr->rr_firstdatacol)
			parity_valid[c] = B_FALSE;
```

— to the two constant indices, because it does not unroll far enough to
know that `c` took the values 0 and 1. A loop-coverage limit, not an
assertion one, and a different class from anything else read this
session.

## cxgbe, second pass: the idempotent allocator that was not

### `t4_sge.c` — one of five that ends differently

`alloc_ctrlq()`, `alloc_rxq()`, `alloc_txq()` and `alloc_ofld_txq()` are
all documented `/* Idempotent. */` and all end `return (0);`.
`alloc_ofld_rxq()` is documented the same way and ends

```c
	return (rc);
```

with `rc` assigned only inside its two `if (!(ofld_rxq->iq.flags &
IQ_..._ALLOCATED))` blocks. So the case the word "idempotent" exists to
name — called again with both flags already set, nothing to do —
returned an unwritten stack word to a caller that reads it as an error
code. Its four siblings say what the value should be.

### `common/t4_hw.c` — a completion poll on a stack word

```c
	max_poll = EEPROM_MAX_POLL;
	do {
		udelay(EEPROM_DELAY);
		t4_seeprom_read(adapter, EEPROM_STAT_ADDR, &stats_reg);
	} while ((stats_reg & 0x1) && --max_poll);
	if (!max_poll)
		return -ETIMEDOUT;

	/* Return success! */
	return 0;
```

`t4_seeprom_read()` returns without writing `*data` on three paths — a
misaligned or out-of-range address, and either `t4_seeprom_wait()`
failing — and `u32 stats_reg;` is an uninitialised local. With the
return discarded, the loop that decides whether the VPD write finished
reads whatever was on the stack: it can exit on the first pass and
`return 0` — "Return success!" — having read nothing that says so.

```
--scope sys/dev/cxgbe   19 findings -> 16   47 units, OK both sides
```

### The rest of cxgbe, read

| Where | Shape |
|---|---|
| `t4_sge.c:3720` | `cong_map`, assigned under `iq->cong_drop != -1` and read under the same test, with `refill_fl()` and register writes in between that the analyser must assume can change `iq->cong_drop`. |
| `t4_sge.c:4374`, `:4429`, `:4455` | `vi->viid`, `vi->nofldtxq`, `vi->dev` in `eth_eq_alloc()` and friends. `alloc_eq_hwq()` dispatches on `eq->type`: `EQ_CTRL` (where `vi` is NULL — `free_ctrlq()` passes it literally) goes to `ctrl_eq_alloc()`, the others to the `vi`-using ones. The analyser cannot relate one argument's type tag to another argument's nullness. |
| `t4_mp_ring.c:318` | `r->consumer[i]` on the `failed:` path of `mp_ring_alloc()`, where `r = malloc(__offsetof(struct mp_ring, items[size]), mt, flags \| M_ZERO)`. The `M_ZERO` is unmodelled — through a flexible-array `__offsetof` size in particular. |
| `t4_main.c:10240` | `stats[2]` and `stats[3]` under `if (nchan > 2)`, filled by a loop to `nchan`. `nchan` is `chip_params->nchan`, which the four chip tables set to `NCHAN` (4) or `T6_NCHAN` (2) — never 3, so `> 2` means 4. Correct, and fragile: it is right only because 3 is not a value the table can hold. |
| `t4_cpl_io.c:583` | `struct sglist_seg segs[n];`, `n` at least 1 whenever the `KASSERT(nsegs > 0)` on the next line holds — an invariant one function up. |
| `iw_cxgbe/cm.c:186`, `cq.c:791`, `qp.c:1846` | NULL tests on a `cm_id`, a `qhp` and a `ucontext` that the RDMA layer guarantees. |
| `t4_filter.c:1259`, `t4_listen.c:1519`, `cxgbei.c:630`, `t4_tls.c:1037`, `fastlz_api.c:386`, `cudbg_lib.c:1837` | Still unread; they stay on task #61. |

## cxgbe, third pass: the debug-dump path

### `cudbg/fastlz_api.c` — a header that could not be read, reported as read

```c
	int byte_r = read_from_buf(pc_buff->data, pc_buff->size,
				   &pc_buff->offset, buffer, 16);
	if (byte_r == 0)
		return 0;

	*pid = readU16(buffer) & 0xffff;
	...
```

`read_chunk_header()` returns **0** — success — having written none of
its five out-parameters. `decompress_buffer()`'s loop reads every one of
them when it gets 0 back:

```c
		rc =  read_chunk_header(pc_buff, &chunk_id, &chunk_options,
					&chunk_size, &chunk_checksum, &chunk_extra);
		if (rc != 0)
			break;

		if ((chunk_id == 1) && (chunk_size > 10) && ...
```

and `chunk_size` goes on to size a `get_scratch_buff()` allocation and
to bound an `update_adler32()` over the buffer. `CUDBG_STATUS_BUFFER_SHORT`
is what a 16-byte header that would not come out of the buffer means.

### `cudbg/cudbg_lib.c` — an out-parameter filled only where a region was found

`get_max_ctxt_qid()` writes `max_ctx_qid[idx]` for each memory region it
recognises and skips the rest (`continue` on `idx >= ARRAY_SIZE(region)`).
`collect_dump_context()` declares `u32 max_ctx_qid[CTXT_CNM + 1];` with
no initialiser. So a queue type whose region is absent left its entry
unwritten; the block the source calls its "Sanity check" only clamps that
stack word to the type's maximum; and the caller then uses it as a
**count**:

```c
	for (i = CTXT_EGRESS; i <= CTXT_CNM; i++)
		size += sizeof(struct cudbg_ch_cntxt) * max_ctx_qid[i];
```

The function already validates `nelem`, so it can zero exactly the array
it was handed. Not finding a region means no queues of that type.

### `cxgbei/cxgbei.c` — two of three combinations

`do_rx_iscsi_cmp()` sets `ip` in two places: the `(val & F_DDP_PDU) == 0`
block, and the `icp == NULL` allocation. A DDP-placed PDU arriving while
`toep->ulpcb2` already holds one is neither, and reaches

```c
	m_copydata(m, sizeof(*cpl), ISCSI_BHS_SIZE, (caddr_t)ip->ip_bhs);
```

with `ip` never assigned. `struct icl_cxgbei_pdu` opens with its
`struct icl_pdu` (`cxgbei.h:88`) and `ip_to_icp()` is exactly that
inverse, so `ip = &icp->ip` is what the other two cases already compute.

```
--scope sys/dev/cxgbe   16 findings -> 13   47 units, OK both sides
```

Across the three passes: **21 -> 13**, with `t4_filter.c:1259` joining the
read-and-explained list — `ftuple` is assigned under `if (t->fs.hash)`
and read under `if (t->fs.hash)`, with `t4_l2t_alloc_switching()` and
`t4_smt_alloc_switching()` in between, which the analyser must assume can
write through `t`.

`tom/t4_tls.c:1037` is the last one in this scope still unread.

## `cxgbe/tom/t4_tls.c` — a zero-length TLS record, from the wire

The last unread finding in the cxgbe scope, and the one worth the most.

`do_rx_tls_cmp()` ends by deciding how to hand the decrypted record to
the socket. In the branch that has a control mbuf:

```c
		if (tls_data != NULL) {
			m_last(tls_data)->m_flags |= M_EOR;
			tgr->tls_length = htobe16(tls_data->m_pkthdr.len);
		} else
			tgr->tls_length = 0;

		m_freem(m);
		m = tls_data;
```

The `else` is the code stating, in its own words, that `tls_data` can be
NULL — a TLS record with no payload. Two lines later `m` becomes that
NULL. Then:

```c
	if (sb->sb_flags & SB_AUTOSIZE &&
	    V_tcp_do_autorcvbuf &&
	    sb->sb_hiwat < V_tcp_autorcvbuf_max &&
	    m->m_pkthdr.len > (sbspace(sb) / 8 * 7)) {
```

A zero-length application-data record is something a TLS peer may send,
so this is a remote null dereference on any T6 connection using TLS
receive offload.

Everything *after* the block is already NULL-safe, which is what makes
the omission clear rather than ambiguous: `sbappendcontrol_locked()`
opens `if (m0 != NULL)` and `sbm_clrprotoflags()` walks `while (m)`. The
append path was written for a record with no data mbuf; the autosize
heuristic was not. One `m != NULL &&` in front of a condition whose whole
purpose is to compare a length that does not exist.

```
--scope sys/dev/cxgbe   13 findings -> 12   47 units, OK both sides
```

**The cxgbe scope is now fully read: 21 -> 12**, and every one of the
twelve that remain is written up above with the invariant it rests on.

## Task #61 closed: what the bucket contained

The static-taken bucket's remaining drivers have all been read, one
`--scope` at a time, both sides measured at the same scope.

```
  sys/dev/adlink          1 ->  0      sys/dev/aic7xxx       7 ->  5
  sys/dev/acpi_support    3 ->  0      sys/dev/mana          2 ->  0
  sys/dev/usb/input       1 ->  0      sys/dev/axgbe         3 ->  0
  sys/contrib/vchiq       1 ->  0      sys/dev/bwn           1 ->  0
  sys/dev/sbni            1 ->  0      sys/dev/irdma         8 ->  8
  sys/dev/mgb             1 ->  1      sys/dev/ice           6 ->  3
  sys/dev/igc             5 ->  1      sys/dev/ixl           2 ->  1
  sys/dev/ixgbe           1 ->  1      sys/dev/iavf          1 ->  1
  sys/compat/linux       77 -> 74      sys/dev/hyperv        5 ->  1
  sys/dev/mlx5            6 ->  1      sys/dev/cxgbe        21 -> 12
```

Thirty-one defects fixed. Every finding that remains is written up above
with the invariant it rests on, and they fall into eight shapes:

1. **an iflib or descriptor-ring idiom** — a loop that always runs at
   least once, and a `= NULL` initialiser that turns the zero-iteration
   path into a nameable dereference (5 drivers);
2. **an allocation's own NULL test** carried into code that only runs
   after it succeeded (aic7xxx, irdma);
3. **a defensive NULL test in a macro** — `irdma_debug(h, ...)`'s
   `if (!(h))` makes every argument evaluated under `rf == NULL`;
4. **one predicate tested twice across a call** that may write through
   the structure holding it (irdma, aic7xxx, cxgbe, mlx5);
5. **`IS_ERR`/`PTR_ERR`**, whose pairing the analyser does not model
   (irdma_cm, mlx5_ib_cq);
6. **`M_ZERO`**, unmodelled — especially through a flexible-array
   `__offsetof` size (t4_mp_ring);
7. **a caller contract** — CAM's `XPT_GET_TRAN_SETTINGS`, `ice_bitops.h`'s
   deliberate read of bits above the size;
8. **an inline function whose body is inline assembly** — the class the
   `inline_asm_write.c` probe established (mgb).

Two things did not fit and left with tasks of their own: the fifty in
`sys/contrib/openzfs/module/zfs` (#112, with the assertion hypothesis
already ruled out), and `linux_mib.c:383`/`:549`, which rest on
`malloc(..., M_WAITOK)` being able to return NULL — the premise #36
exists to settle before anything is built on it.

## `sbin/ifconfig/sfp.c`: a translation unit reporting zero findings

Not a defect in the tree — a hole in the instrument, and the last of the
unexplained `progs` ERRORs.

```
sbin/ifconfig/sfp.c   missing header: libifconfig_sfp_tables.h
```

`lib/libifconfig/Makefile:23-33`:

```make
GEN=	libifconfig_sfp_tables.h \
	libifconfig_sfp_tables.c \
	libifconfig_sfp_tables_internal.h

.SUFFIXES: .tpl.c .tpl.h
.tpl.c.c .tpl.h.h: sfp.lua
	${LUA} ${.CURDIR}/sfp.lua ${.IMPSRC} >${.TARGET}
```

`LUA` is the tree's own flua, and there is no Lua of any kind on this
host, so the header did not exist and `sfp.c` came back ERROR — zero
findings, indistinguishable from clean.

Building all of flua is a large job: `libexec/flua` links `contrib/lua`
with `lfbsd`, `lfs`, `libhash`, `libucl` and `liblyaml`. None of it is
needed. `sfp.lua` does one thing —

```lua
package.path = (os.getenv("SRCTOP") or "/usr/src").."/tools/lua/?.lua"
require("template").render(arg[1], { ... })
```

— and `tools/lua/template.lua` is plain Lua 5.4 whose only other
`require` is a `pcall(require, "table.new")` that is *meant* to fail off
LuaJIT. So the interpreter is `contrib/lua`'s own sources, one `cc`,
compiled against the tree's `lib/liblua/luaconf.local.h` — the
configuration flua itself uses — with `-DBOOTSTRAPPING`, which is the
tree's own name for the variant without `LUA_USE_DLOPEN`.

Not the host's lua, for the same reason `usr.bin/rpcgen` is built rather
than borrowed: a different interpreter is a different answer, and the
answer is what the analysed source compiles against. There was no host
lua to borrow in any case.

```
--scope sbin/ifconfig --check-errors
  before   14 findings, 22 OK, 1 ERROR    FAIL: sfp.c is not in EXPECTED
  after    14 findings, 23 OK, 0 ERROR    ok

--scope lib/libifconfig
  before    1 finding, 9 OK, 2 ERROR
  after     1 finding, 10 OK, 1 ERROR
```

`sfp.c` and `libifconfig_sfp.c` both compile now and both are clean — the
finding count does not move. That is the outcome an ERROR cannot tell you
from a defect.

The one ERROR that remains in `lib/libifconfig` was there before and is
not a translation unit at all:
`libifconfig_sfp_tables.tpl.c` is the *template*, whose body is Lua
inside `{% %}` and `{* *}` markers, so clang stops at `expected
identifier` on the first `{%`. `for_arch('amd64')` names ten sources in
that directory and this is not one of them. It is now in
`expected_errors.py`, in the same family as `sbin/ipf/common/lexer.c` —
a sed template, likewise not a source.

## `-include opt_global.h` was emitted last, so every ZFS assertion was compiled out

Task #112 opened with a premise, and the premise was checked the wrong
way. It said the sweep does not pass `-DNDEBUG` for the openzfs tree —
which is true — and concluded that the analyser therefore sees every
`ASSERT` as a real check. It does not. `NDEBUG` is not a flag here. It is
decided by a *header*, and by the order the headers are read in.

```
sys/contrib/openzfs/include/os/freebsd/spl/sys/ccompile.h:39
    #if defined(INVARIANTS) && !defined(ZFS_DEBUG)
    #define ZFS_DEBUG
    #undef  NDEBUG
    #endif
ccompile.h:51
    #if !defined(ZFS_DEBUG) && !defined(NDEBUG)
    #define NDEBUG
    #endif
```

`sys/modules/zfs/static_ccompile.h:7` does the same `#ifdef INVARIANTS`.
Both headers are force-included by the module's `CFLAGS`, and both read
an option that arrives from `opt_global.h`. So the answer depends
entirely on whether `opt_global.h` has been read yet when they run.

In the build it has. `sys/conf/kern.pre.mk:78` puts `-include
opt_global.h` in the **base** kernel `CFLAGS`, and
`sys/modules/zfs/Makefile:391-393` appends its three `-include` *after*
`.include <bsd.kmod.mk>` — below that line, therefore after it.

`includes.py` appended `-include opt_global.h` at the **end**, after the
module `CFLAGS`. The reverse. Measured on `vdev.c` with the sweep's own
flags: `INVARIANTS` defined, `ZFS_DEBUG` **not**, and

```
ASSERT(string != NULL)
  -> ((void) sizeof ((uintptr_t)(string != ((void *)0))))
```

which is `debug.h:260`, the `NDEBUG` form — the assertion evaluated for
its type and discarded.

This is not an academic configuration in this tree.
`sys/conf/std.hardenedbsd:29-41` turns `INVARIANTS` on as a **hardening**
option — the file says so in its own words, "INVARIANTS is a hardening
option here, not a debugging one" — and `amd64/conf/HARDENEDBSD` includes
it. The kernel ParanoidBSD ships has every one of those assertions live.
The sweep was reading a kernel this tree does not build, and an analysis
of a configuration you do not ship is not a weaker reading of yours. It
is a reading of a different one.

```
--scope sys/contrib/openzfs/module/zfs
  before   50 findings, 137 OK, 0 ERROR
  after    22 findings, 137 OK, 0 ERROR

--scope sys/cddl
  before   84 findings, 48 OK, 22 ERROR
  after    76 findings, 48 OK, 22 ERROR
```

Nothing stopped compiling on either side: the OK and ERROR counts are
unchanged, so the twenty-eight that went are answers, not silence.

The direction is worth naming, because it is the opposite of what a
"stricter flag" intuition expects. Turning the assertions **on** removed
twenty-eight findings and added three new ones. Removed, because an
`ASSERT` the analyser can see is a constraint it can use:
`vdev.c:3562`'s `strlen(NULL)` was reachable only past
`ASSERT(string != NULL)`, `zil.c:4582`'s `*cookiep` only past
`ASSERT(cookiep != NULL)`. Added, because `ZFS_DEBUG` also compiles code
**in** — `vdev_raidz.c:1892` is an assertion that indexes `missing[0]`,
and it did not exist to be read before.

Blast radius is bounded and was checked rather than assumed: the only
`-include` any module flag set carries are linuxkpi's `kconfig.h`, whose
conditionals are all architecture tests and no option, and these two ZFS
headers. `sys/contrib/openzfs` and `sys/cddl` are the scopes that can
move, and both were measured.

Three checks in `test_includes.py`, made to fail before being trusted:
with the fix reverted, "...before the module headers that read the
options it defines" reports `opt_global.h at [37], module -include at
[27, 30]` and the `#error assertions are compiled out` probe goes red.
The third check asks the *preprocessor* whether `ZFS_DEBUG` is set,
rather than asking the flag list — because the flag list is not where
`NDEBUG` is decided, which is the whole of what went wrong here.

## openzfs: eight uninitialised reads, and the fourteen that are premises

With the assertions restored (above), `--scope
sys/contrib/openzfs/module/zfs` reports 22. Eight of them are defects and
all eight are the same shape — a local that one path writes and another
path reads — which is what the `core.uninitialized.*` and
`core.CallAndMessage` checkers are for.

**`zvol.c`, three functions.**

`zvol_create_minors_impl()` declares `int total = 0, done = 0, last_error,
error;`. `last_error` is assigned only when `zvol_os_create_minor()`
fails, or when a prefetch job carries an error. A name containing `@`
whose `snapdev` is not `visible` — or whose `dsl_prop_get_integer()`
fails — skips both, the prefetch list is empty, and
`zvol_task_update_status(task, total, done, last_error)` reads a stack
word. It lands in `task->zt_error`, which `zvol_remove_minors()` returns
to its caller.

`zvol_rename_minors_impl()` is worse: `error` is uninitialised too, and
it is assigned inside the loop **only** for a zvol whose name matches
`oldname` or lies under it. Every *other* zvol in `zvol_state_list` — one
per unrelated volume on the system, so the common case — falls through to
`if (error)` having never written it. A non-zero stack word counts the
rename as failed and becomes the task's error.

`zvol_set_volmode_impl()` switches on `task->zt_value` with `case
ZFS_VOLMODE_NONE/GEOM/DEV/DEFAULT` and **no `default`**. That value is
the `volmode` property as `zvol_set_common_sync_cb()` read it with
`dsl_prop_get_int_ds()` — a number off disk, checked against the four
`zfs_volmode_t` values nowhere in between. A dataset carrying any other
value leaves `error` unwritten, and the next line reads it twice, as the
done count and as the error. Same shape as `hyperv/pcib`'s
protocol-version switch.

**`zil.c`.** `zil_lwb_write_issue()` declares `boolean_t slog;` and
writes it only through `zio_alloc_zil(..., &slog, ...)`, which is inside
`if (error == 0)` where `error = lwb->lwb_error`. An lwb that already
carries an allocation failure skips that whole block, and `if (slog)`
near the end of the function then sets `LWB_FLAG_SLOG` on the next lwb
from a stack word.

**`vdev.c`.** `vdev_prop_get_bool()` calls `vdev_prop_get_int()` and
converts. But `vdev_prop_get_int()` writes `*value` on exactly two
paths — a successful `zap_lookup()`, and `ENOENT`, where it writes the
default — and returns `EINVAL` **without writing anything** when
`vdev_prop_get_objid()` finds no ZAP object. `vdev_load()` stores the
result straight into `vd->vdev_slow_io_events` and only `vdev_dbgmsg()`s
the error, so a vdev with no ZAP got a random answer to "should this vdev
post slow-IO events" — and the property's default is **on**
(`zpool_prop.c:484` registers `slow_io_events` with `B_TRUE`), so the
half of the time it lands zero is a vdev silently not reporting slow IO.
Seeded with `vdev_prop_default_numeric(prop)`, which is what
`vdev_prop_get_int()` itself writes on `ENOENT`.

**`vdev_raidz.c`, two.** `raidz_reconstruct()`'s debug line prints
`ltgts[0], ltgts[1], ltgts[2]` unconditionally; the caller writes
`tstore[]` for indices `-1` through `num_failures`, and `num_failures` is
this function's `ntgts`. With one or two targets the other two are
untouched stack, printed into the debug ring. And
`vdev_raidz_matrix_reconstruct()` asserts
`ccount >= rr->rr_col[missing[0]].rc_size || i > 0` — indexing
`missing[0]` without first asking whether `nmissing` is 0.
`vdev_raidz_reconstruct()`'s `switch (nbaddata)` has arms for 1 and 2 and
falls through to `vdev_raidz_reconstruct_general()` for 0, which is the
all-parity-targets case; there `nmissing_rows` is 0 and `missing_rows[]`
is a stack array nothing wrote. The loops either side of that assertion
are bounded by `nmissing` and do nothing; only the assertion read it —
and this tree ships with assertions on, which is the whole reason it is
visible at all.

```
--scope sys/contrib/openzfs/module/zfs
  22 -> 14 findings, 137 OK, 0 ERROR on both sides
```

Each of the eight has a `check_pbsd_marks.py` entry, and each was
verified by restoring the file from `HEAD` and reading the exit status:
all four files go to `exit=1`, naming the individual fix that went.
That matters more here than in tree-owned code — this is a vendor
directory that gets re-imported, and a fix with no marker is a fix with a
deletion date.

### The fourteen that remain, and what each rests on

None is a defect; each is a premise the analyser cannot reach, and the
premise is what is worth recording.

| where | rests on |
|---|---|
| `dbuf.c:3869` | `dbuf_prefetch_impl()` returns early on `level >= nlevels`, so `curlevel <= nlevels-1` and `bp` is written either by the loop's `break` or by the `curlevel == nlevels-1` arm. A loop bound the analyser will not relate to a guard on the entry value. |
| `dmu.c:2460` | `dn == NULL` implies `type = DMU_OT_OBJSET`, whose `dmu_ot[]` entry has `ot_metadata` TRUE, so `ismd` is true and the `else` is not reached. `dmu_ot[]` is `const` — the same family as the dispatch tables in task #88. |
| `dmu.c:2540` | the one caller passing `dn == NULL` (`dmu_objset_sync()`) passes `wp = 0`, so `(wp & WP_SPILL)` picks `type`. |
| `dmu_send.c:2490` | `redact_rl` is set exactly when `dspp->redactbook != NULL`, and the deref is under the same test — one predicate read twice across intervening calls on a struct the analyser must assume they can write. |
| `dsl_dataset.h:276` (from `dsl_destroy.c:802`) | a clone has an origin snapshot, so `dd_origin_obj != 0` implies `ds_prev_snap_obj != 0` implies `ds->ds_prev != NULL` — `dsl_dataset.c:1794` asserts the second half. Worth noting that the same function tests `ds->ds_prev != NULL` fourteen lines earlier, at `:788`. |
| `metaslab.c:3416` | `ms_sm == NULL` implies `ms_allocated_space == 0` (`metaslab_init()` sets one from the other, and `metaslab_sync()` creates the space map before adding), so the `allocated == 0` early return covers it. The caller at `:3528` explicitly permits `ms_sm == NULL`, which is what makes this one worth a second look rather than a shrug. |
| `spa.c:1242` | `boot_ncpus >= 1`. With it, `cpus >= 1` and the `while (count * count > cpus) count--` stops at 1. |
| `vdev_raidz.c:2146,2153,2164,2165` | `parity_valid[c]` is written for every `c < rr->rr_firstdatacol` by the loop above, and the reads are the constant indices `VDEV_RAIDZ_P` and `VDEV_RAIDZ_Q` under `ASSERT(rr_firstdatacol > 1)`. The analyser honours the assertion — it does not unroll the loop far enough to know it covered indices 0 and 1. A loop-coverage limit, not an assertion one. |
| `vdev_raidz.c:2856` | `orig[c]` is written and read under the identical `!rc_tried \|\| rc_error != 0` test; `vdev_draid_map_verify_empty()` in between writes `rc_error` only for `c >= rr_bigcols`, which are data columns, not the parity columns this loop walks. |
| `zfs_log.c:355,360` | `fuidp` is non-NULL whenever an id is ephemeral, because the same `zfs_acl_ids_create()` that made the id allocated the fuid info. On FreeBSD `IS_EPHEMERAL(x)` is `x > UID_MAX` and `z_uid` is a `uint64_t`, so the branch is not folded away as it is on a 32-bit uid — but the value is the creating process's uid, and no such uid exists. |

## dtrace's x86 disassembler: three uninitialised reads, and a table premise for the rest

`--scope sys/cddl` reported 76 findings, 73 of them in one file —
`sys/cddl/dev/dtrace/x86/dis_tables.c`, and 71 of those in one function,
`dtrace_disx86()`. A count like that is a hypothesis before it is an
inventory: seventy-one separate defects in one function is not what
seventy-one findings in one function usually means.

It was three.

**`dtrace_get_SIB()` returns without writing its outputs.** Both of its
early returns — `x->d86_error` already set, and `d86_get_byte()`
returning `< 0` — leave `*ss`, `*index` and `*base` as the caller left
them. And `dtrace_get_modrm()` passes the ModRM byte's three fields
straight through it:

```c
	if (x->d86_got_modrm == 0) {
		...
		dtrace_get_SIB(x, mode, reg, r_m);
		x->d86_got_modrm = 1;
	}
```

so a failed read leaves `mode`, `reg` and `r_m` unwritten *and* sets
`d86_got_modrm`, which stops anything from retrying. `d86_get_byte()`
returns `< 0` when the instruction stream ends — for fasttrap that is a
**user** address, so an unmapped page after the last byte of a probed
instruction is a way to reach it. The values are register numbers that
index `dis_REG[][]` and are then printed. Writing the outputs before
either return fixes every caller in one place: 76 -> 40.

**`reg` and `r_m` were declared uninitialised beside a `mode` that was
not.** `uint_t mode = 0;` on one line, `uint_t reg;` and `uint_t r_m;` on
the next two — someone had already met this and fixed a third of it. The
remaining path is the table indirection at `:4320`, which calls
`dtrace_get_modrm(x, &mode, &opcode3, &r_m)`, putting the ModRM *reg*
field in `opcode3` and setting `d86_got_modrm`; an arm reached after that
indirection calls `dtrace_get_modrm(x, &mode, &reg, &r_m)`, gets nothing,
and uses `reg` as a register number. 40 -> 35.

**`goto done` jumps over the only assignment of `dp`.** The zero-padding
check runs *before* the prefix loop that first writes `dp`:

```c
	if (opcode1 == 0 && opcode2 == 0 &&
	    x->d86_check_func != NULL && x->d86_check_func(x->d86_data)) {
		(void) strncpy(x->d86_mnem, ".byte\t0", OPLEN);
		goto done;
	}
```

and the `DIS_MEM` block at `done:` reads `dp->it_stackop`, `dp->it_size`
and `dp->it_adrmode`. `-DDIS_MEM` is in this tree's flags for this file,
so that block is compiled. It is **latent**, not reachable: every caller
in the tree sets `d86_check_func` to NULL — `kinst_isa.c:290`,
`instr_size.c:108`, and libdtrace's `dt_isadep.c:503` — so the arm cannot
be taken today. Which is a reason to name it rather than a reason to
leave it: the field exists precisely so a caller can supply one.
35 -> 34.

```
--scope sys/cddl --check-errors
  76 -> 34 findings, 48 OK, 22 ERROR on both sides, all 22 on the record
```

### The 31 that remain in `dtrace_disx86()`, and the one premise under them

27 of them are `wbit`, 3 are `opcode7` and 1 is `opcode5` — all
"uninitialised" in a path where the analyser has `dp` pointing at an
opcode table entry whose `it_adrmode` selects a `VEX_*` arm of the big
switch, while `vex_prefix` is 0. `wbit` is set for the VEX case at
`:4106`:

```c
	if (vex_prefix) {
		if (dp->it_vexwoxmm) { wbit = LONG_OPND; }
		else if (dp->it_vexopmask) { wbit = KOPMASK_OPND; }
		else { wbit = vex_L ? YMM_OPND : XMM_OPND; }
	}
```

so the arm is unreachable if a `VEX_*` `it_adrmode` implies a VEX prefix
was parsed. That is a claim about table *contents*, and it is checkable
rather than arguable — scanning every `instable_t` array in the file for
`VEX_*` adrmode values gives exactly nine tables:

```
dis_opAVX0F  dis_opAVX660F  dis_opAVX660F38  dis_opAVX660F3A
dis_opAVXF20F  dis_opAVXF20F38  dis_opAVXF20F3A
dis_opAVXF30F  dis_opAVXF30F38
```

(`dis_distable` matches the regex too, on `VEX_B`/`VEX_L`/`VEX_R`/`VEX_W`
/`VEX_X`/`VEX_m` — those are prefix *bit* masks, not adrmodes.) Every one
of the nine is assigned to `dp` only inside the `vex_prefix ==
VEX_2bytes` and `VEX_3bytes` blocks. So `it_adrmode` being a `VEX_*`
value implies `vex_prefix != 0` implies `wbit` was set. The same
reasoning covers `opcode5` and `opcode7`: the arms that read them are
reached only through a `dp` in a table that a second or third opcode byte
selected.

This is the `const` dispatch-table family from task #88 with an extra
step: not just "the table is `const` so the value is one of these", but
"the value being one of *these* says which table `dp` points into, and
therefore what was parsed to get there". The analyser reads
`dp->it_adrmode` as an unconstrained symbol and has no way to run that
inference backwards.

The other three findings in the scope:

| where | rests on |
|---|---|
| `fasttrap_isa.c:1621` | `ftt_ripmode` is written only at `:609` and `:614`, as `FASTTRAP_RIP_1\|(FASTTRAP_RIP_X * FASTTRAP_REX_B(rex))` and the `RIP_2` form; `FASTTRAP_REX_B(rex)` is `((rex) & 1)`, so the value is one of 1, 2, 5, 6 — exactly the switch's four arms, and `reg` is always assigned. |
| `fbt/riscv/fbt_isa.c:153` | `rval` and `patchval` are written only inside the loop, which `break`s when it writes them; reaching `:153` needs `instr < limit`, and the loop exits only on `break` or on `instr >= limit`. Another loop bound the analyser will not relate to a guard after it. |
| `fbt.c:792` | `ctf_list_append()` stores the pointer into the list via `lp->l_prev = q`, which `unix.Malloc` does not count as an escape. |

## The armv7 target said soft float; the build says `gnueabihf`

Found while measuring task #47, which is the point: a measurement that
does not move the number can still tell you where you are standing.

`includes.py` passed `--target=armv7-unknown-freebsd15.0`.
`Makefile.inc1:136-142` is

```
.if ${TARGET} == "arm"
.if ${TARGET_CPUTYPE:M*soft*} == ""
TARGET_TRIPLE_ABI=	gnueabihf
.else
TARGET_TRIPLE_ABI=	gnueabi
.endif
.endif
```

and `:893` passes the result as `-target ${TARGET_TRIPLE}`. Nothing in
this tree sets a soft-float `CPUTYPE` — the only matches for
`CPUTYPE:M*soft*` are in `share/mk/bsd.cpu.mk`, the machinery that
*reads* it — so the build's arm triple carries the hard-float ABI.

`unknown` is not a neutral spelling of that. It is the other ABI:

```
armv7-unknown-freebsd15.0     __ARM_PCS 1  __SOFTFP__ 1
armv7-gnueabihf-freebsd15.0   __ARM_PCS 1  __ARM_PCS_VFP 1  __ARM_FP 0xc
```

So the whole of armv7 was analysed as soft float. That decides what is
compiled, not just how it is called: `lib/libc/arm/gen/flt_rounds.c`
wraps its three softfloat `#include`s and half its body in
`#ifndef __ARM_PCS_VFP`.

```
--scope lib/libc --scope lib/msun --scope libexec --check-errors
  before  239 findings, 1599 OK, 31 ERROR
  after   239 findings, 1600 OK, 30 ERROR

--scope sys/arm --check-errors
  before   15 findings, 315 OK, 8 ERROR
  after    15 findings, 315 OK, 8 ERROR   (byte-identical, per file)
```

One translation unit comes back, and it is exactly the one task #47 was
opened about: `lib/msun/arm/fenv-softfp.c`, whose `expected_errors.py`
entry blamed `lib/msun/Makefile:18` putting `-I${.CURDIR}/x86` on the
path for an architecture whose `.if` does not hold. The entry was right
about the flag walk and wrong about the cause of *this* failure; under
the correct ABI the file compiles with the extra `-I` still there. The
entry is gone.

`sys/arm` does not move at all — the kernel has no floating point in it —
which is the useful half of the negative result: this was a userland
misreading, and now it is not one.

Two checks in `test_includes.py`, made to fail first: with the triple put
back to `unknown`, "armv7 carries the hard-float ABI" reports
`armv7-unknown-freebsd15.0` and the second reports `__ARM_PCS __SOFTFP__`.
The second asks *clang* what the triple means rather than checking the
string, because the string is not the thing that decides.

`Makefile.inc1` special-cases no other architecture, so the remaining
five triples are `unknown` in the build too.

## bmake decides a component's include path, for the files bmake builds

`include_flags()` had three sources for a userland component's `-I`: a
table written in the file, a textual walk of the component's Makefiles,
and `ask_cflags()`, which asks bmake. The first two cannot evaluate a
conditional, so both contribute directories from branches that do not
hold. Analysing `lib/msun/arm/fenv.c` as armv7:

```
table + walk + bmake   msun/arm  msun/src  msun/ld80  msun/ld128
                       msun/x86  msun/bsdsrc  msun/man
bmake alone            msun/arm  msun/src
```

`lib/msun/Makefile:18` puts `x86/` on the path only for i386 and amd64;
`:24` picks `ld80/` or `ld128/` from `LDBL_PREC`; `bsdsrc/` and `man/`
come from the walk. Four of the seven are flags no armv7 build passes.

So: when bmake answered with at least one `-I` inside the component, that
answer is the component's include path, and a `-I` into the component
bmake did not name comes off. Narrowly — only directories under the
component root, so the `-I` into libc, contrib and the generated shims
that the table adds for *other* components are untouched.

The first attempt applied that to every file, and the measurement said
no:

```
--scope lib/libc --scope lib/msun --scope libexec --check-errors
  before   239 findings, 1599 OK, 31 ERROR
  first    239 findings, 1560 OK, 70 ERROR
```

Thirty-nine translation units stopped compiling and no finding moved.
They are `lib/libc/softfloat` and `lib/libc/arm/aeabi`, and bmake is
right about them: `lib/libc/Makefile:131` reads `softfloat/Makefile.inc`
only under `LIBC_ARCH == "arm" && CPUTYPE:M*soft*`, so an armv7 build
with no `CPUTYPE` compiles none of them and its `CFLAGS` rightly carry
none of their `-I`. The sweep analyses those files anyway, deliberately,
on the `-I` the table supplies. Taking bmake's answer for them buys
nothing and costs 39 files their voice.

So the rule is gated on the build **naming** the file at that
architecture — `for_arch(arch)`, which is bmake's own answer to "what
does this tree build". A file the build does not build keeps the reading
it had.

```
--scope lib/libc --scope lib/msun --scope libexec --check-errors
  gated    239 findings, 1600 OK, 30 ERROR
```

— identical to not applying it at all, over the same 1630 units, with the
armv7 triple already corrected. No finding moves either way. What changes
is that four `-I` per armv7 msun file are no longer flags the build never
passes, and the next time one of them shadows a header the answer will
not be wrong for that reason.

`bin`, `sbin`, `usr.bin` and `usr.sbin` are unaffected by construction:
the walk this narrows exists only for `lib` and `libexec`.

Two checks in `test_includes.py`, made to fail first: with the change
reverted, "a built armv7 msun file gets arm/ and src/" reports all seven
directories. The second check holds the other half of the rule — that
`lib/libc/softfloat/eqdf2.c`, which `for_arch('armv7')` does not name,
still gets `-I lib/libc/softfloat`.

## The rest of openzfs: a boot-time uninitialised pointer, and a `noreturn` a Linux tool took away

`module/zfs` is 137 of the 288 translation units under
`sys/contrib/openzfs`. The other 151 — the FreeBSD SPL and VFS layer,
`zcommon`, `icp`, `nvpair`, `lua`, `zstd`, `unicode` — had never been
read, and until the `-include` order was fixed they were being read with
every `ASSERT` compiled out. 18 findings live outside `module/zfs`. Two
are defects.

**`os/freebsd/zfs/spa_os.c:108`.** `spa_generate_rootconf()` declares
`nvlist_t *best_cfg` uninitialised and assigns it only inside

```c
	best_txg = 0;
	for (i = 0; i < count; i++) {
		if (configs[i] == NULL)
			continue;
		txg = fnvlist_lookup_uint64(configs[i], ZPOOL_CONFIG_POOL_TXG);
		if (txg > best_txg) {
			best_txg = txg;
			best_cfg = configs[i];
		}
	}
	nchildren = 1;
	nvlist_lookup_uint64(best_cfg, ZPOOL_CONFIG_VDEV_CHILDREN, &nchildren);
```

A set of labels whose `POOL_TXG` is 0 — or whose `configs[]` entries are
all NULL, which the loop's own `continue` says is a case — leaves it a
stack word, and the very next line hands that word to
`nvlist_lookup_uint64()`, which walks it. This runs at **boot**, on the
labels read off the root pool's disks by
`vdev_geom_read_pool_label()`, so the deciding value comes off disk. Now
initialised to NULL, with the "nothing was best" case freeing `configs`
and returning NULL — which is what the function's other failure path
already does.

**`module/lua/llimits.h:104`.** Lua's `l_noret` is the return type of
`luaX_syntaxerror()`, `luaG_runerror()` and the rest of the interpreter's
error paths, all of which `longjmp`. The vendor tree has

```c
/* Suppress noreturn attribute in kernel builds to avoid objtool
   check warnings */
#if defined(__GNUC__) && !defined(_KERNEL)
#define	l_noret		void __attribute__((noreturn))
```

so in every kernel build the attribute is gone. objtool is Linux's, and
it is looking at *code*; a static analyser is looking at *paths*, and
without the attribute it walks out of `luaX_syntaxerror()` and reads what
the caller never initialised — `lparser.c:858`'s `args.k`, after the
`default:` arm that is the only path not to set it.

openzfs already knows this shape and already makes the exception:
`include/os/freebsd/spl/sys/debug.h:85` gives `spl_panic()` the attribute
under `__COVERITY__ || __clang_analyzer__`, with a comment saying exactly
why the general case is suppressed. The same two macros, on the same
argument: neither is defined when the kernel is built, so no object code
changes.

```
--scope sys/contrib/openzfs --check-errors
  32 -> 30 findings, 280 OK, 8 ERROR on both sides, all 8 on the record
```

Both markers verified by restoring the file from `HEAD`: `exit=1`.

### The sixteen that remain outside `module/zfs`

| where | rests on |
|---|---|
| `icp/algs/modes/ccm.c:260,267,279` | `macp` is set under `ccm_remainder_len > 0` and used under the same test, with `calculate_ccm_mac()` and `crypto_{init,get}_ptrs()` in between — one predicate read twice across calls that take the same `ctx`. |
| `lua/ldebug.c:42`, `lua/lstrlib.c:780` | the interpreter's own invariants (`ci->func` points at a Lua closure when `isLua(ci)`; `str_gsub`'s match state). Reachable only through `zfs program`, which needs pool-owner privilege. Not discharged in detail — named here so the next reader starts from that rather than from zero. |
| `os/freebsd/zfs/zvol_os.c:816,864` | `zfs_uio_init()` tolerates a NULL `struct uio *` (`if (uio_s != NULL)`) and every caller then dereferences it; the cdev `d_read`/`d_write` entry points are never called with one. The defensive test inside the helper, carried forward. |
| `os/freebsd/zfs/zfs_acl.c:830` | `zfs_acl_node_alloc(n * sizeof (zfs_object_ace_t))` leaves `z_acldata` NULL when `n` is 0, and a v0 ACL being transformed has at least one ACE. |
| `unicode/u8_textprep.c:1370,1524,1703` | three counting invariants, discharged in the section below. |
| `zstd/lib/decompress/zstd_decompress_block.c:1388,1394` | `ZSTD_decodeSeqHeaders()` writes `*nbSeqPtr` on both of its non-error returns (`:496` and `:509`), and every earlier return is a `RETURN_ERROR_IF` the caller catches with `ZSTD_isError()`. The analyser loses the `size_t` wraparound arithmetic that makes `ZSTD_isError` true for those codes. |
| `zstd/lib/common/bitstream.h:205` | `assert(nbBits < BIT_MASK_SIZE)` two lines above the `BIT_mask[nbBits]` it guards — compiled out here, and a caller contract besides. Compression side, not decompression. |
| `zstd/lib/compress/fse_compress.c:251` | `normalizedCounter[symbol++]` bounded by the `maxSymbolValue` the same function validated. Compression side. |

## u8_textprep: three counting invariants, and why one copy loop is bounded and its twin is not

The Unicode normalisation that the `utf8only` and `normalization` dataset
properties run over **filenames** carried the three findings the table
above deferred. All three hold, and the invariants are worth writing
down, because two of the three sites look careless until you have them.

**`do_composition():1370`** — `for (i = 0; i < l; i++) s[i] = t[i];` over
a `uchar_t t[U8_STREAM_SAFE_TEXT_MAX + 1]` that nothing initialises, with
`l` advanced by four separate copy loops and *rewound* by `l = saved_l`
on a failed match at `:1202` and `:1327`. The analyser cannot follow an
index across a restore, so it reports a read of a `t[]` element it never
saw written.

The bound:

* `last` is clamped to `U8_UPPER_LIMIT_IN_A_SEQ` (31) at `:1094`, so the
  sequence is at most 32 characters;
* `disp[i]` is `u8_number_of_bytes[...]`, whose largest entry in the
  table at `:237` is 4 — `U8_MB_CUR_MAX`;
* composition never grows the byte count (the comment at `:1364` says so,
  and the table entry replaces two or more characters with one);
* so `l <= 32 * 4 = 128 = U8_STREAM_SAFE_TEXT_MAX`, and `t[l] = '\0'` at
  `:1371` lands on exactly the last element of a `[129]` array.

That also settles what otherwise reads as an oversight: the copy loop at
`:1204`

```c
	l = saved_l;
	while (*++p != U8_TBL_ELEMENT_FILLER)
		t[l++] = *p;
```

has no bound check, while its near-identical twin at `:1329` does

```c
	if (l >= U8_STREAM_SAFE_TEXT_MAX) {
		p = saved_p;
		goto SAFE_RETURN;
	}
	t[l++] = *q;
```

The first writes a composite of characters already counted in the 32.
The second is in the block from `:1292` that pulls **further** characters
off the remaining stream `*os`, which is not part of the counted
sequence — so it is the only one that can exceed the bound, and it is the
only one that checks.

**`collect_a_seq():1524`** — `comb_class[last - 1]` on a `size_t last`.
Both arms of the `:1488` decomposition test set it first: the `sz == 1`
arm assigns `last = 1` outright, and the other fills from
`saved_sz = do_decomp(...)`, which returns at least the `sz >= 2` it was
given. `last >= 1`.

**`collect_a_seq():1703`** — the sharpest of the three, and the reason
`last >= 1` is worth stating rather than assuming:

```c
	last--;
	if (last >= saved_last) {
		for (i = 0; i < last; i++)
			for (j = last; j > i; j--)
				if (comb_class[j] && ...)
					U8_SWAP_COMB_MARKS(j - 1, j);
	}
```

`last` is a `size_t`. At zero, `last--` is `SIZE_MAX`, `last >=
saved_last` is then true for any value, and the doubly-nested loop walks
`comb_class[]`, `start[]` and `disp[]` — three 32-byte arrays — to
`SIZE_MAX`, swapping as it goes. The guard against that is not in this
code; it is the same `last >= 1` above.

Nothing is patched here. That is a different verdict from
`dis_tables.c`'s `goto done` over the assignment of `dp`, which was also
unreachable today: there the code was wrong and the invariant accidental,
and the fix restored an intent the file had lost. Here the code is right
and maintains what it needs. What it does not do is say so, which is what
this section is for.

## A module's explicit rules, and whose flags they are

`ask_module()` asked bmake for `SRCS`, `OBJS` and `.PATH`, and mapped
every `.o` that was in `OBJS` but not in `SRCS` back to a source. A rule
whose target is in **neither** was invisible. `sys/modules/vmm`:

```
CLEANFILES+=  vmm_nvhe_exception.o vmm_nvhe.o

vmm_nvhe.o: vmm_nvhe.c vmm_hyp.c
	${CC} -c ${NOSAN_CFLAGS:N-mbranch-protection*:N-fstack-protector*} \
	    ${.IMPSRC} -o ${.TARGET} -fpie

vmm_hyp_blob.elf.full:	vmm_nvhe_exception.o vmm_nvhe.o
```

`vmm_nvhe.o` is built — the hypervisor blob the module links depends on
it — and it appears nowhere but `CLEANFILES` and its own rule. bmake's
`.ALLTARGETS` is the list of every target it has a rule for, and it names
both. Six Makefiles in the tree have a `.o:` rule and no `OBJS` line, so
the whole cost is six more bmake runs.

The other half of the change matters more than the first, and it is the
half the task warned about. A rule's `-D` belong to **its** target. The
old code harvested every tab-indented line in the Makefile into one list
and gave it to every rule-built source. `sys/modules/vmm` has two rules
side by side:

```
vmm_nvhe_exception.o: ...
	${CC} -c -x assembler-with-cpp -DLOCORE ...
vmm_nvhe.o: vmm_nvhe.c vmm_hyp.c
	${CC} -c ... (no -DLOCORE)
```

and `sys/modules/linux` has three whose targets are `$`-expanded, of
which `linux${SFX}_locore.o` and `linux${SFX}_support.o` carry `-DLOCORE`
and `linux${SFX}_vdso_gtod.o` does not. Handing `-DLOCORE` to a C file
tells every header it is assembly — the twenty-six-regression mistake of
sweep 12, one rule further in, and it was already happening.

So the rules are split by target. A literal target gets its own body and
nothing else; a `$`-expanded target is matched on the literal tail after
the last `}` — `_vdso_gtod.o` — which is unique among that Makefile's
three; and a target with no tail at all, blake2's `${src:S/.c/.o/}:`,
stays in the generic set, which is right, because that rule *is* every
source it builds.

```
--scope sys/amd64 --scope sys/arm64 --scope sys/i386 --scope sys/crypto
--scope sys/dev/hyperv --scope sys/cddl --check-errors
  before  75 findings, 420 OK, 47 ERROR   FAIL x2, not in EXPECTED
  after   75 findings, 422 OK, 45 ERROR   ok
```

Two translation units come back — `sys/amd64/linux32/linux32_genassym.c`
and `linux32_vdso_gtod.c`, both of which had been compiled as assembly —
and the finding count does not move. Every other file is byte-identical
across the pair.

Four checks in `test_includes.py`, made to fail first: with the change
reverted, "a rule-only target is named" reports `not in by_src` and
"...nor does linux32_vdso_gtod.c, whose rule is $-expanded" reports
`-DCOMPAT_FREEBSD32 -DCOMPAT_LINUX32 -DLOCORE`. The two that stay green
under the revert are the ones holding the *other* direction — that
blake2's generic `.for` rule keeps its flags, and that `vmm_nvhe.c` does
not acquire its neighbour's — which is what a check for a split like this
needs: one half proves the new behaviour, the other proves the old one
was not broken to get it.

## The M_WAITOK premise was wrong twice, and the answer was in `M_START()`

Task #36 was filed on the claim that every `M_WAITOK` allocation is a
potential NULL to the analyser, with
`sys/kgssapi/krb5/krb5_mech.c:661` as the instance. A control test had
already refuted the general claim — clang's analyser says nothing about a
bare `extern` whose return is unconstrained; it warns when it can
*constrain* a value to NULL — and a modelling layer built on it was
built, tested and found to change nothing.

The instance was then re-read as "the NULL is `m->m_data`, a field of the
mbuf, not the mbuf pointer". That was the right direction and still not
the cause. The cause is one macro:

```c
#define	M_START(m)							\
	(((m)->m_flags & M_EXTPG) ? NULL :				\
	 ((m)->m_flags & M_EXT) ? (m)->m_ext.ext_buf :			\
	 ((m)->m_flags & M_PKTHDR) ? &(m)->m_pktdat[0] :		\
	 &(m)->m_dat[0])
```

Three of its four arms are addresses. One is a literal `NULL` — the
M_EXTPG case, an mbuf whose payload is unmapped pages and which therefore
has no linear data area at all. And `m_align()`, which `M_ALIGN()`,
`MH_ALIGN()` and `MEXT_ALIGN()` all are, opens with

```c
	KASSERT(m->m_data == M_START(m),
	    ("%s: not a virgin mbuf %p", __func__, m));
```

On a freshly allocated mbuf the analyser has no constraint on `m_flags`:
`m_get()` takes it from `uma_zalloc_arg()`, whose zone constructor is in
another translation unit. So it explores the path where M_EXTPG is set,
`M_START(m)` is `NULL`, and the assertion — which this tree compiles in,
because `INVARIANTS` is a hardening option here — is read as **binding**
`m_data` to NULL. Every caller that then does `p = m->m_data` and writes
through `p` reports a null dereference. There are 61
`M_ALIGN`/`MH_ALIGN`/`MEXT_ALIGN` call sites in this tree.

Proved rather than argued: adding `if (m == NULL) return (NULL);`
straight after the `MGET()` in `krb5_make_token()` leaves the finding
exactly where it was (and adds two more, at the two callers that then
have a NULL return to dereference). The mbuf is not the NULL. Adding

```c
	KASSERT((m->m_flags & M_EXTPG) == 0,
	    ("%s: M_EXTPG mbuf %p has no linear data area", __func__, m));
```

as the first line of `m_align()` removes it.

That assertion is not a silencer. It says what `m_align()` already
requires: on an M_EXTPG mbuf the existing assertion would fire anyway, by
way of `m->m_data == NULL`, and the arithmetic two lines down would be
NULL plus an offset. The requirement was there to be deduced from a
comparison against a macro whose arms disagree about what they are; now
it is written.

```
--scope sys/kern --scope sys/net --scope sys/netinet --scope sys/netinet6
--scope sys/netipsec --scope sys/kgssapi --scope sys/net80211
--scope sys/netlink --scope sys/netpfil --check-errors
  before  322 findings, 609 OK, 19 ERROR
  after   319 findings, 609 OK, 19 ERROR
```

Three go and none arrives: `kern/uipc_mbuf.c:1154`,
`kgssapi/krb5/krb5_mech.c:661` and `netinet6/ip6_output.c:3315`. Fewer
than the 61 call sites, because most of them do not read `m->m_data`
back into a pointer they write through inside the same function — which
is the other half of what this class needed, and the reason it never
looked like a class.

The general lesson is the one the control test gave and this confirms:
the analyser does not invent NULLs from unconstrained returns. When it
reports one, something in the code said NULL — here, a macro arm nobody
was reading.

## `*ap->a_bnp = blkno` is unconditional, and `bmaparray` has returns that write no block

Two of the fs shard's uninitialised reads are the same three lines, once
in ext2 and once in UFS:

```c
	error = ext2_bmaparray(ap->a_vp, ap->a_bn, &blkno,
	    ap->a_runp, ap->a_runb);
	*ap->a_bnp = blkno;
	return (error);
```

`blkno` is a bare local. `ext2_bmaparray()` returns `ext2_getlbns()`'s
error at `:240` having written nothing through `bnp`; UFS's does the same
at `:227` and has two `return (EINVAL)` at `:239` and `:245` besides. The
store is not under the error test, so `VOP_BMAP`'s caller gets a stack
word as a disk block number on every one of those paths.

Both are now `= -1`, which is not an invention: it is this API's own
spelling of "not mapped", written by `*bnp = -1` at `ext2_bmap.c:246` and
`ufs_bmap.c:236` for a zero block pointer. A caller that reads the block
number before the error now sees a hole.

The analyser reported the ext2 one and not the UFS one — the same code,
in the tree's primary filesystem, differing only in which paths the path
budget reached. It was found by reading the sibling, which is what one
finding in a copied function is for.

## `g_concat`: the "we could not ask" fallback is inside the arm that could

```c
	error = g_access(cp, 1, 0, 0);
	if (error == 0) {
		error = g_getattr("GEOM::candelete", cp, &disk_candelete);
		if (error != 0)
			disk_candelete = 0;
		(void)g_access(cp, -1, 0, 0);
	} else
		G_CONCAT_DEBUG(1, "Failed to access disk %s, error %d.", ...);
	...
	disk->d_candelete = disk_candelete;
```

The `disk_candelete = 0` that handles "the attribute could not be read"
is nested inside the arm where the consumer *was* opened. A consumer that
could not be opened at all takes the `else`, says so in a debug message,
and falls through to store an unwritten local into the disk record.
`d_candelete` is what `g_concat_candelete()` reads to decide whether the
concat device advertises `GEOM::candelete`, and therefore whether
`BIO_DELETE` is passed through to that member — so a stack word decides
whether TRIM reaches a disk. Initialised to 0, which is what the
fallback beside it already uses and what "we could not ask" means.

```
--scope sys/fs --scope sys/ufs --scope sys/geom --scope sys/cam
--scope sys/security --check-errors
  156 -> 154 findings, 334 OK, 2 ERROR on both sides
```

Two, not three: `ufs_bmap.c` was never reported, so fixing it moves no
number. All three markers verified by restoring from `HEAD`: `exit=1`.

### Three from the same shard that are not defects

| where | rests on |
|---|---|
| `cam/scsi/scsi_enc_ses.c:2762,2792` | `req.result` is a stack field of a request queued on `ses->ses_requests` and read after `cam_periph_sleep()`. Every write is immediately before the `wakeup(req)` that ends that sleep: `ses_encode()`'s result at `:2228`, and `ses_terminate_control_requests()` at `:133`, reached from `:1939`, `:2208` and `:2728`. `cam_periph_sleep` is `xpt_path_sleep` is `msleep` at `PUSER` — no `PCATCH` — with no timeout, so it returns no other way. |
| `geom/eli/g_eli.c:1202` | `dcw` is assigned at `:1092`, after the `goto failed` at `:1083` that `g_attach()` failure takes. The `failed:` label reads it only inside `if (cp->provider != NULL)`, and `g_attach()` leaves `cp->provider` NULL exactly when it fails. Worth noting that initialising `dcw` would be the *wrong* fix: `g_access(cp, -1, 0, -1)` on a consumer opened with `dcw == 1` would leak a write count, so 0 is not a safe default here — the guard is. |
| `geom/eli/g_eli.c:1509` | `g_eli_mkey_decrypt_any()` writes `*nkeyp = -1` on entry (`g_eli_key.c:158`), before any loop, so `nkey` is written whether or not a key is found. The callee is in another translation unit, which is the whole of why the analyser cannot see it. |

## Four in userland, three of them reachable by someone else

`--scope bin --scope sbin --scope usr.bin --scope usr.sbin` reports 737
findings across 1862 translation units. 68 are `core.uninitialized.*`,
which is where every defect this session has come from, and the four
below are the ones read first, chosen by what supplies the input.

**`usr.sbin/rtsold/rtsol.c:499` — two defects in three lines.** The DNSSL
option handler, processing a router advertisement, does

```c
	if (rao->rao_msg == NULL) {
		warnmsg(LOG_ERR, __func__, "strdup failed: %s", ...);
		free(rao);
		addr++;
		continue;
	}
```

`addr` is the **RDNSS** cursor, assigned at `:396` inside the other
option's branch; here it has never been written, which is the finding.
The larger problem is the one the finding leads to: this loop walks the
DNSSL names with `p`, which the `p += len` at the bottom of the body
advances, and this `continue` skips it. A second `strdup()` failure
decodes the same name again — and a third, and a fourth, for as long as
the allocation failures last. `rtsold` runs as root and the option came
off the wire. The block is a byte-for-byte copy of the RDNSS block at
`:435-441`, where `addr++` is the right cursor and correct.

**`usr.bin/mdo/mdo.c:439,443,450` — one past the end, four times, in a
setuid program.** `remove_groups()` walks two sorted `gid_t` arrays:

```c
	cand = set->groups[++from];
	if (from == set->nb)
		break;
```

Every arm indexes and *then* asks whether the index has reached `nb`, so
the last iteration reads one element past the end of a heap array. The
value is discarded, which is not the same as the read being legal: an
allocation ending on a page boundary has no obligation to have the next
page mapped, and `mdo` is setuid with its group arrays coming from the
target user's group list. The test now precedes the load at all four
sites; the four breaks are in the same four places.

**`usr.sbin/ppp/mp.c:508,534` — a peer decides which fragments are
dropped.** `mp_ReadHeader()` rejects a multilink header whose reserved
bits are set:

```c
	if (val & 0x3000) {
		log_Printf(LogWARN, "Oops - MP header without required zero bits\n");
		return 0;
	}
```

and returns without writing `header->begin` or `header->end` (the 24-bit
arm writes `seq` at `:151` before it looks at it; the 12-bit arm writes
nothing). Four of the six callers ignore the return value — `:472`,
`:507`, `:543`, `:598` — and read those fields immediately. Setting the
reserved bits is exactly what an attacker does, so a malformed fragment
had `ppp` choosing which fragments to drop and what to set
`mp->seq.next_in` to from the frame. Both rejects now zero the header, so
"returned 0" means something about `*header`. That is the smaller half of
the fix; the larger is that those four callers should test the return,
and that is a change to the reassembly logic rather than to a
postcondition.

**`usr.sbin/setaudit/setaudit.c:151` — the ordinary invocation.**
`term_port` is assigned only under `-p`, and

```c
	if (!Uflag || sflag) {
		aia.ai_termid.at_port = term_port;
		aia.ai_termid.at_type = term_type;
	}
```

runs whenever `-U` was *not* given. `setaudit -a user` — no `-p`, no
`-U` — therefore set the audit terminal ID's port from this frame and
handed it to `setaudit_addr(2)`, where it becomes part of the process's
audit state and of every record written for it. `term_type` on the line
below already had a default at `:72`; `term_port` now has one too.

```
--scope bin --scope sbin --scope usr.bin --scope usr.sbin
  before  737 findings, 1821 OK, 41 ERROR
  after   729 findings, 1821 OK, 41 ERROR
```

Eight go and none arrives — the six named above plus `mp.c:487`, an
`UndefinedBinaryOperatorResult` on the same `h.seq` the zeroing defines.
All four markers verified by restoring from `HEAD`: `exit=1`.

`rtsold`'s marker carries no "must be absent" string, and the reason is
worth recording: the block it fixes was copied from the RDNSS block
verbatim, so every string short enough to be worth writing matches the
site the marker does *not* guard — where `addr++` is correct. A marker
that matches the wrong site is worse than none.

## Three more in userland, and one the number cannot show

**`usr.sbin/ppp/radius.c:209` — the copy of a function already fixed.**
`demangle()` decrypts an MPPE key attribute:

```c
	if (mlen % 16 != SALT_LEN) { ...reject... }
	...
	Clen = mlen - SALT_LEN;
	P = alloca(Clen);
	while (Clen) { ...16 bytes at a time... }
	*len = *P;
```

`SALT_LEN` is 2, so `mlen == 2` satisfies `mlen % 16 == SALT_LEN`, `Clen`
is 0, `alloca(0)` returns a zero-sized object, the decrypt loop never
runs, and `*P` reads past it. This is `lib/libradius/radlib.c`'s
`demangle()`, copied into ppp — and that copy was fixed under task #97
while this one was not. The guard is now the same
`mlen < SALT_LEN + 16 || mlen % 16 != SALT_LEN`. The attribute's length
comes from the RADIUS server.

That is the second time today one finding named one of two copies:
`ufs_bmap.c` was the sibling of `ext2_bmap.c`, and this is the sibling of
`radlib.c`. Both were found by looking for the other copy rather than by
the tool.

**`usr.sbin/ppp/async.c:215` — a wild mbuf returned to the layer above.**
`async_LayerPull()` declares `struct mbuf *nbp, **last;`, sets
`last = &nbp` and writes through `last` for every byte it decodes. A `bp`
that is NULL on entry, or whose mbufs are all `m_len == 0`, decodes no
bytes, and `return nbp` hands the caller a pointer to walk and free.
NULL is what "nothing was reassembled" means, and what the loop itself
stores when `async_Decode()` returns it.

**`sbin/ping/ping.c:696` — a guard that names the wrong flag.** On the
`F_HDRINCL` path, `ip.ip_ttl = ttl` is unconditional, and the one thing
that writes `ttl` — a `net.inet.ip.ttl` sysctl — is under

```c
	if (!(options & (F_TTL | F_MTTL))) {
```

`F_MTTL` is set by `-T`, which assigns `mttl`: a different variable, used
only for `IP_MULTICAST_TTL` at `:748`. So `-T` without `-m` skipped the
initialisation and kept the use, and every packet ping built its own
header for carried a TTL read off the frame. The guard is now `F_TTL`
alone.

**And the number does not move for that one.** The finding is still
reported, nine lines further down, because the analyser cannot relate
`options & F_TTL` at the guard to the `ttl = (int)ltmp` in the `getopt`
loop three hundred lines earlier — a flag and the variable it stands for,
coupled only by the programmer. What the fix removes is the `-T` case,
which the same finding was covering and which is decidable by reading.
A change that is right and does not move the count is worth making and
worth saying so about; the alternative is to let the measurement decide
what is true.

```
--scope sbin/ping --scope usr.sbin/ppp
  before  63 findings
  after   61 findings
```

Two go — `async.c:215` and `radius.c:209` — and the two that "arrive" are
`ping.c:1641` and `radius.c:661` reappearing at `:1650` and `:671`, moved
by the comments. All three markers verified by restoring from `HEAD`:
`exit=1`.

### Two more from the same list that are premises

| where | rests on |
|---|---|
| `usr.sbin/traceroute/traceroute.c:798,818` | `sockerrno` is assigned only inside `if (pe)` at `:520-525`. `:793` exits when `pe == NULL`, so the block ran; and `:817`'s read is reached only when `s >= 0`, which is exactly when the `else if` that assigns it ran. Two guards, three hundred lines apart from the assignment. |
| `usr.sbin/bhyve/pci_virtio_net.c:374` | `info[i].len` for `i` up to the chain count `vq_getchain()` returned, which is also what filled `info[]`. |

### gzip(1) reports a stack word as the compressed size of a pack(1) file

`usr.bin/gzip/unpack.c:139`, `core.uninitialized.Assign`, in
`accepted_bytes()`. The line is

```c
	if (bytes_in != NULL)
		(*bytes_in) += newbytes;
```

`+=`, not `=`. That makes `unpack()` the one decompressor in gzip(1)
whose byte count *accumulates* into the caller's variable instead of
assigning it — `gz_uncompress()` ends `*gsizep = in_tot` (`gzip.c:1052`)
and `cat_fd()` ends `*gsizep = in_tot` (`gzip.c:1739`), so neither ever
needed the caller to start the sum. `unpack()` does, and no caller does
it: `handle_stdin()` declares

```c
	off_t usize, gsize;
```

at `gzip.c:1750` with no initialiser and passes `&gsize` straight into
`unpack()` at `:1819`. So `gzip -d` on a `pack(1)` stream added the
compressed length to whatever that stack slot held and printed the
result under `-v` and under `-l`.

The fix is one store at the top of `unpack()`, where the other two
decompressors' equivalent already effectively is:

```c
	if (bytes_in != NULL)
		*bytes_in = 0;
```

It goes in the callee rather than in `handle_stdin()` because the
callee is where the contract is wrong — a function that accumulates
into an out-parameter it never initialises is asking every present and
future caller to know that about it.

```
--scope usr.bin/gzip
  before  2 findings
  after   1 finding
```

The survivor is `unix.Malloc` at `gzip.c:554`. Note that the finding is
reported *through* `gzip.c`: `unpack.c` is `#include`d, not compiled, so
its own translation unit is an `ERROR` in both runs and contributes
nothing either way — the count that moved is `gzip.c`'s.

### kdump(1) dispatches a zero-length record to a handler that reads a struct

Four of the sweep's `core.uninitialized.*` findings in `usr.bin/kdump`
name four different functions — `fetchprocinfo`, `ktrsyscall`,
`ktrsysret`, `ktrcsw` — and six more of `core.CallAndMessage` and
`core.UndefinedBinaryOperatorResult` name six others. They are one bug.

`main()` reads a `struct ktr_header`, takes `ktrlen = ktr_header.ktr_len`
from it, and then

```c
		if (ktrlen && fread_tail(m, ktrlen, 1) == 0)
			errx(1, "data too short");
```

— note the `if (ktrlen)`. `m` is `malloc(1025)`, grown by `realloc` only
when a record is larger. Then the dispatch casts `m` to whatever struct
the record's *type* implies and hands it to a handler that reads the
whole struct. Nothing checked that the length and the type agree. A
record whose `ktr_len` is zero skips the `fread` entirely and reaches its
handler over 1025 bytes of heap this run never wrote.

That is not a guess about which path the analyser is on. Making the
`fread_tail` unconditional — an experiment, not a fix — took the file
from eleven findings to one:

```
--scope usr.bin/kdump, fread_tail made unconditional
  11 findings -> 1
```

The one that stays is `unix.Malloc` on `m` at the end of `main`.

`KTR_SYSCALL` is worse than the rest. `ktr_narg` is a `short` that comes
from the file too, and `ktrsyscall()` does

```c
	ip = &ktr->ktr_args[0];
	...
	while (narg > 0)
		print_number(ip, narg, c);
```

so a header claiming 32767 arguments walks that many `register_t` off the
end of the buffer and prints every one. A `ktrace.out` is a file, and a
file comes from wherever the person running kdump(1) got it — a bug
report, a shared machine, a tarball. The kernel never writes a record
like this; that is not the same as never having to read one.

The fix is a length table in `main()`, checked before the dispatch: each
fixed-layout type's minimum, plus `ktr_narg * sizeof(register_t)` for
`KTR_SYSCALL`. A short record is warned about and skipped.

**Two attempts did not work, and the reason is worth recording.**

The first put the check in a helper, `ktr_lenok()`. The analyser never
inlines it — `grep -c "Calling 'ktr_lenok'"` over the text-output path is
`0` — so the guard was opaque and all ten findings stood. The count did
move, 11 to 10, which was worse than not moving: `fetchprocinfo`'s
finding vanished only because passing `m` to a function the analyser
treats as opaque invalidates what it knows about that memory. A count
that moves for the wrong reason is a measurement that has stopped
measuring.

The second spelled the `switch` out in `main()` and still reported all
ten. The path says why:

```
kdump.c:503:3: note: Control jumps to the 'default' case at line 539
...
kdump.c:570:3: note: Control jumps to 'case 2:'  at line 574
```

Two switches on `ktr_header.ktr_type`, one taking `default` and the other
taking `KTR_SYSRET`. Between them are `fetchprocinfo(&ktr_header, ...)`,
`findabi(&ktr_header)` and `dumpheader(&ktr_header, ...)` — three calls
that take the header by non-`const` pointer, after which the analyser no
longer knows which case the guard took. Reading the type into a local
once, and switching on the local in both places, is what made the guard
visible. None of the three writes to the header, so the local says
something true that the pointer did not.

```
--scope sbin/init --scope usr.bin/kdump
  before  12 findings
  after    1 finding
```

### init(8): `replace_init()` fills two of three argv entries, and the copy loop reads all three

`sbin/init/init.c:1097`. `execute_script()` ends

```c
	for (i = 0; i != SCRIPT_ARGV_SIZE; ++i)
		sh_argv[i + sh_argv_len] = argv[i];
	execv(shell, sh_argv);
```

A fixed count, not a walk to the terminator. Two of the three callers
fill all three entries; `replace_init()` fills `argv[0]` and `argv[1] =
NULL` and leaves `argv[2]`. So pid 1 loads an indeterminate `char *` and
stores it one past the NULL that `execv()` stops at — which is why
nothing ever went wrong, and why it is still a load of an indeterminate
pointer value in the one process the machine cannot restart.

The loop now copies the NULL and stops, which is what `argv` means and
what the `execv(script, argv)` twenty lines above already assumed;
`replace_init()` also fills `argv[2]`, so the object is complete either
way.

### mptutil(8): a lookup that returns errno values, and three callers that test `< 0`

`usr.sbin/mptutil/mpt_config.c:974`, `core.uninitialized.ArraySubscript`,
in `add_spare()`. The subscript is `i`, and `i` is written by

```c
	if (mpt_lookup_standalone_disk(av[1], sdisks, nsdisks, &i) < 0) {
		error = errno;
		warn("Unable to lookup drive %s", av[1]);
```

`mpt_lookup_standalone_disk()` returns `0`, `EINVAL`, or `ENOENT` — never
`-1`. `EINVAL` is 22 and `ENOENT` is 2, so a name that is neither
`<bus>:<id>` nor `daN` does not take that branch: it falls through to
`sdisks[i]` with `i` never written, and hands the result to
`mpt_lock_physdisk()` and `mpt_create_physdisk()`, which write a RAID
physical-disk page for whatever bus and target the stack word named.
mptutil(8) runs as root.

There are three call sites and all three are the same. The other two are
`build_raid_volume()` at `:421` and the `create physdisk` path at
`:1112`; the analyser reported only the first because the other two index
through a struct field and a second local. **One finding in a shape
repeated three times is a reason to grep for the shape.** All three now
take the returned value as the error and use `warnc()`, which is what the
rest of this file already does with `mpt_lookup_volume()` and
`mpt_lookup_drive()` — both of which return errno values too, and both of
which are called correctly.

### mptutil(8): a failed page read that returns success

`find_volume_spare_pool()` ends

```c
	/* Add this pool to the volume. */
	info = mpt_vol_info(fd, VolumeBus, VolumeID, NULL);
	if (info == NULL)
		return (error);
```

The two earlier `mpt_vol_info() == NULL` arms in the same function return
`errno`. This one returns `error` — and on every path that reaches this
line `error` is 0, because it was last written by `mpt_lookup_volume()`
at the top and that call's failure returned. So a volume page that reads
successfully once and fails the second time reports success to
`add_spare()` with `*pool` never written, and `add_spare()` writes the
stack word into the physical disk page as the hot spare pool bitmap.

```
--scope usr.sbin/mptutil
  before  10 findings
  after    9 findings
```

Only one moves. The `add_spare` finding that stays is now a different
path: `mpt_vol_info()` returning NULL at `:860` with `errno == 0`.
Reading `mpt_read_config_page()` says that cannot happen — every `NULL`
return there either comes from a failed `ioctl(2)`, which sets `errno`,
or sets `errno = EIO` itself — so that one is a premise, and the fix
above is not the thing it was reporting. Both fixes are right on reading;
one of them is also measurable.

### Three premises in the same directory

`mpt_cam.c:162`, `:427` and `:539` all trace to `fetch_path_id()`, which
writes `*path_id` from `ccb.cdm.matches[0]` after
`ioctl(xptfd, CAMIOCOMMAND, &ccb)`. The `ccb` is passed by address, so
the analyser gives up what it knew about it — including that
`ccb.cdm.matches` points at `calloc`'d memory — and every read through
that pointer is garbage from there on. The same shape as `ifgif.c:83` and
`ifgre.c:105`: an ioctl fills a buffer the analyser cannot see written.

### bhyve e82545: the VLAN correction adjusts checksum offsets that were never set

`usr.sbin/bhyve/pci_e82545.c:1349`, `:1350`, `:1353`. `e82545_transmit()`
declares `struct ck_info ckinfo[2]` and initialises exactly one field of
each:

```c
	ckinfo[0].ck_valid = ckinfo[1].ck_valid = 0;
```

`ck_start`, `ck_off` and `ck_len` are written only inside the arms that
set `ck_valid`, or under `IXSM || tso` and `TXSM || tso`. A **legacy**
descriptor without the IC bit leaves `ckinfo[0].ck_off`, `ckinfo[0].ck_len`
and the whole of `ckinfo[1]` unwritten — and the VLAN insertion block
adds `ETHER_VLAN_ENCAP_LEN` to all six fields with no `ck_valid` test.

The obvious fix — guard the correction on `ck_valid` — is wrong, and
worth writing down as the second time this shard has offered one. A TSO
packet without `IXSM` has `ckinfo[0].ck_valid == 0` and still *uses*
`ck_start` and `ck_off`, at

```c
	ipid = ntohs(*(uint16_t *)&hdr[ckinfo[0].ck_start + 4]);
	...
	ipcs = *(uint16_t *)&hdr[ckinfo[0].ck_off];
```

where `hdr` is `__builtin_alloca(hdrlen + vlen)`. Guarding on `ck_valid`
would skip the correction for exactly those packets and index a stack
buffer four bytes short. `memset(ckinfo, 0, sizeof(ckinfo))` is the exact
change: every path that *reads* these fields writes them first, so the
only values it changes are the indeterminate ones.

### bhyve TPM CRB: the guest picks how much of the register it writes

`usr.sbin/bhyve/tpm_intf_crb.c:346`, `:365`, `:396`. Three cases of
`tpm_crb_mem_handler()`'s write switch have the same shape:

```c
	union tpm_crb_reg_ctrl_start start;

	if ((size_t)size > sizeof(start))
		goto err_out;

	*val = *val << shift;

	pthread_mutex_lock(&crb->mutex);
	tpm_crb_mmiocpy(&start, val, size);

	if (!start.start || crb->regs.ctrl_start.start) {
```

`size` is the guest's MMIO access width. The check rejects *too large*;
it says nothing about too small. `tpm_crb_mmiocpy()` copies exactly
`size` bytes into a union of `uint32_t` bitfields, so a one- or two-byte
write leaves two or three bytes of the object indeterminate, and the
bitfields are read out of it — for `ctrl_start`, to decide whether a TPM
command runs.

**It works today, and the reason it works is not the contract.** The bits
these three read are the low ones, which on a little-endian target live
in byte 0, which every `size >= 1` copy writes. Bitfield allocation is
implementation-defined and PBSD builds big-endian targets. Zeroing each
union at its declaration costs nothing and makes the layout irrelevant.

### Two more, and a same-tree baseline

`pci_emul.c:2749`: `pci_emul_dior()` sets `value = 0` inside the
`baridx == 0` branch. The `baridx == 1 || baridx == 2` branch has an
`unknown size` arm of its own, and takes it for any size that is not 1,
2, 4 or 8 — then returns a `uint32_t` nothing wrote. The initialiser
moves to the declaration.

`usr.bin/sdiotool/cam_sdio.c`: `sdio_read_1`, `_2` and `_4` return `val`
whatever `*ret` says, and the CAM transfer that fills it does not run
when the ccb fails. `sdio_func_read_cis()` fills `cis1_info[0..count-1]`
and stops at the first `0xff` byte the card returns, then prints all four
slots with `%s`. The card supplies the CIS.

```
--scope usr.sbin/bhyve --scope usr.bin/sdiotool
  before  35 findings, 93 OK, 2 ERROR
  after   24 findings, 93 OK, 2 ERROR
```

Thirteen go and two "arrive" — `pci_e82545.c:1237` and `:1321`
reappearing at `:1253` and `:1337`, moved by the sixteen-line comment.
Both are a different premise: `iov[0].iov_len` where no descriptor
contributed a segment.

**The first attempt at this measurement was against the wrong baseline.**
`w18-progs.jsonl` gave 35 findings, 91 OK and **4** ERROR for this scope,
and two of those ERRORs are now OK — the sweep predates the
`-include opt_global.h` reorder and the userland `-I` filter. Two files
that did not compile then do now, and a file that does not compile
reports zero findings and reads as clean. Reverting the four files in the
current tree and re-running gave 35/93/2, which is the number the "after"
is comparable to. A before and an after have to be the same tree with one
change between them, and a stored baseline is only the same tree until
something else lands.

### col(1): a memset three bytes short of its allocation

`usr.bin/col/col.c:448`. `flush_line()` grows two static buffers and then
clears the counting one:

```c
	if (l->l_max_col >= count_size) {
		count_size = l->l_max_col + 1;
		if ((count = realloc(count,
		    (unsigned)sizeof(int) * count_size)) == NULL)
			err(1, NULL);
	}
	memset(count, 0, sizeof(int) * l->l_max_col + 1);
```

`*` binds tighter than `+`, so the memset clears
`(sizeof(int) * l_max_col) + 1` bytes — one byte of the last entry, where
the allocation is `sizeof(int) * (l_max_col + 1)`. The top three bytes of
`count[l_max_col]` survive from whatever a previous, longer line left
there; `count` is `realloc`'d and never `calloc`'d, so on the first line
they are the allocator's.

Those three bytes go into the running total:

```c
	for (tot = 0, i = 0; i <= l->l_max_col; i++) {
		save = count[i];
		count[i] = tot;
		tot += save;
	}
	for (i = nchars, c = l->l_line; --i >= 0; c++)
		sorted[count[c->c_column]++] = *c;
```

and `sorted` holds `l->l_lsize` entries. col(1) is a filter; the input
stream decides how far past the end that writes.

The clear is now `sizeof(int) * count_size` rather than the
parenthesised `l_max_col + 1`, because `count_size` *is* the allocated
length — it is what the `realloc` above was given and it never shrinks.
Tying the clear to the allocation is what stops the two drifting apart
again.

**And the number does not move.** col.c reports three findings before and
three after: the analyser was never reporting the memset length. It was
reporting `count` and `sorted` being NULL — reachable only if
`l->l_lsize` is 0 while `l->l_line_len` is not, which is a premise about
`l_lsize` being the allocated length of `l_line`. The bug that is real
here is the one it was silent about.

### Seven more, and one that made the next layer visible

| where | what |
|---|---|
| `fmt/fmt.c:556` | `get_line()` returns the length out of band and never writes a NUL. Every caller respects that except `might_be_header()`, which walks the line as a wide *string* looking for the colon — so a line shorter than the longest seen so far read into what the previous one left there. It now terminates the buffer, growing by one first, since the loop only guarantees room for `len`. |
| `gprof/arcs.c:699` | `compresslist()` writes `maxexitarcp`, `maxwithparentarcp` and `maxnoparentarcp` only when the matching count rises above 0, and the third arm was an unconditional `else`. A list whose arcs all have `arc_cyclecnt == 0` fell into it and did `maxarcp->arc_flags |= DEADARC` — a *write* through a pointer nothing had set. Three counts at 0 is exactly "there is no edge to break", so that arm now returns. |
| `patch/pch.c:1320` | `pch_swap()`'s copy loop runs from `p_ptrn_lines + 1` to `p_end`. A hunk with no replacement half leaves `n == 0`, and `p_line[0]`/`p_char[0]` fresh from `set_hunkmax()`'s malloc — and if that byte happened to be `'='` the sanity check passed and `for (s = p_line[0]; *s; s++)` walked an unwritten `char *`. The patch file decides. |
| `top/commands.c:486` | `scanint()` returns `-1` without writing `*prio`, and `prio = -prio` ran before the `procnum == -1` test. Reject, then negate, then range-check. |
| `rtlbtfw/rtlbt_hw.c:269` | `frag_num` is a `size_t` expression converted to `int`; `ret` is returned whether the loop ran or not. `-1` at the declaration, so nothing-sent is not success. |
| `fdread/fdread.c:243` | `fdopts |= FDOPT_NOERROR` and then `ioctl(fd, FD_SOPTS, &fdopts)`, which writes the *whole* option word — and `FD_GOPTS` appears nowhere in the program. `sys/sys/fdcio.h` says these options are "cleared on device close" and fdread(8) opens the device itself, so 0 is the driver's actual state and every other bit was the frame's. |
| `sa/main.c:341` | `ci_flags` is only ever `|=`'d. `ci` is reused for every record and handed whole to `pacct_add()` and `usracct_add()`, so the flag word entering the accounting databases — `CI_UNPRINTABLE` among them — came off the frame. Reset per record, not per call. |

`rpcgen`'s pair is the interesting one. `get_declaration()` leaves
`dec->name` unwritten on its `void` early return and `dec->array_max`
unwritten for every declaration that is not an array or vector — which is
most of them — and `def_typedef()` copies both out unconditionally.
Starting them at `NULL` fixed the two reported findings and produced two
new ones: `check_type_name()` now visibly `strcmp`s a NULL for
`typedef void x;`. That is not the fix breaking something. It is the fix
turning a wild pointer into a NULL one and the analyser being able to say
so — the same defect, one layer up, where it can be handled:
`def_typedef()` now rejects a declaration with no name.

Which needed one more thing. The guard only protects the line below it if
`error()` is known not to return, and `error()` — which ends in
`crash()`, which is already `__dead2` — was declared plainly. So were
`expected1()`, `expected2()` and `expected3()`, which are three wrappers
around it. All four are now `__dead2`. **This is the third time in this
tree**: `patch(1)`'s `fatal()` and `pfatal()` under task #70, and now
rpcgen's four.

```
--scope usr.bin/col --scope usr.bin/fmt --scope usr.bin/gprof
--scope usr.bin/patch --scope usr.bin/rpcgen --scope usr.bin/top
--scope usr.sbin/bluetooth/rtlbtfw --scope usr.sbin/fdread --scope usr.sbin/sa
  before  22 findings, 43 OK, 0 ERROR
  after   12 findings, 43 OK, 0 ERROR
```

Fourteen go and four arrive, all four relocations: col.c's three moved by
its nineteen-line comment and gprof's `unix.Malloc` by its twelve.
`usr.bin/rpcgen` is now clean apart from `rpc_main.c:319`.

### Premises read and dismissed

| where | rests on |
|---|---|
| `usr.bin/pr/pr.c` ×8 | `lines`, a global. `-l` rejects anything below 1 at `:1688`; `:1819` defaults it; and the `lines -= HEADLEN + TAILLEN` at `:1828` is the `else` of `lines <= HEADLEN + TAILLEN`. So `lines >= 1` at `vertcol()`'s entry, and `indy[]`, `lindy[]` and `lstdat[]` are filled for exactly `0..lines-1`. Eight findings, one predicate, three hundred lines and one function away. |
| `usr.sbin/watchdogd/watchdogd.c:280,281,311` | `do_timedog`, a static set once at `:698`. `watchdog_getuptime()` writes `*tp` only under it and `watchdog_check_dogfunction_time()` returns early under it — the same predicate at the write and at the read, with a call in between. |
| `bin/pax/tables.c:1261` | `val` is filled a byte at a time through a `char *` alias, `sizeof(u_int)` of them, immediately above. |
| `usr.sbin/mptutil/mpt_cam.c:162,427,539` | `fetch_path_id()` writes `*path_id` from `ccb.cdm.matches[0]` after an ioctl that took `&ccb`; the analyser discards what it knew about `ccb.cdm.matches` at that call. |

### The iSCSI target trusts the initiator's declared transfer length

`sys/cam/ctl/ctl_frontend_iscsi.c:2776`, in `cfiscsi_datamove_out()`.
The function computes

```c
	expected_len = ntohl(bhssc->bhssc_expected_data_transfer_length);
	...
	datamove_len = MIN(io->scsiio.kern_data_len,
	    expected_len - io->scsiio.kern_rel_offset);
```

— `expected_len` straight off the wire — and then everything after it
assumes `io->scsiio.ext_data_filled` is within `datamove_len`. Two things
rest on that, and nothing checked it.

The scatter-gather walk skips the already-filled bytes:

```c
	while (r2t_off > 0) {
		if (r2t_off >= cdw->cdw_sg_len) {
			r2t_off -= cdw->cdw_sg_len;
			cdw->cdw_sg_index++;
			cdw->cdw_sg_addr = ctl_sglist[cdw->cdw_sg_index].addr;
			cdw->cdw_sg_len = ctl_sglist[cdw->cdw_sg_index].len;
			continue;
		}
```

The index is advanced and the entry **loaded** before `r2t_off > 0` is
tested again — so consuming the list exactly reads the entry after the
last one. The same shape as `mdo(1)`'s `remove_groups()`, and here it is
worse: on the `kern_sg_entries == 0` path `ctl_sglist` is `&ctl_sg_entry`,
a single `struct ctl_sg_entry` on this frame, and what comes back becomes
`cdw_sg_addr` — the address the next Data-Out PDU is copied to.

And forty lines further down,

```c
	r2t_len = MIN(datamove_len - io->scsiio.ext_data_filled,
	    cs->cs_max_burst_length);
```

is `uint32_t` arithmetic, so the same excess underflows it and the R2T
asks the initiator for a full `MaxBurstLength` into a buffer with no room
for it.

Both are fixed, separately, because they are two different mistakes.
The invariant is now checked once where `datamove_len` is computed —
before `cfiscsi_data_wait_new()`, so there is nothing to unwind, the same
shape as the write-underflow return above it. And the loop tests before
it loads:

```c
	while (r2t_off > 0) {
		if (r2t_off < cdw->cdw_sg_len) {
			cdw->cdw_sg_addr += r2t_off;
			cdw->cdw_sg_len -= r2t_off;
			break;
		}
		r2t_off -= cdw->cdw_sg_len;
		cdw->cdw_sg_index++;
		cdw->cdw_sg_len = 0;
		if (r2t_off == 0)
			break;
		cdw->cdw_sg_addr = ctl_sglist[cdw->cdw_sg_index].addr;
		cdw->cdw_sg_len = ctl_sglist[cdw->cdw_sg_index].len;
	}
```

That is behaviour-identical. Where the old loop consumed a segment
exactly and loaded the next entry, the new one leaves `cdw_sg_len` at 0
and `cdw_sg_index` on that entry — and `cfiscsi_handle_data_segment()`
already reads `cdw_sg_len == 0` as "load `ctl_sglist[cdw_sg_index]`",
behind its own `KASSERT(cdw->cdw_sg_index < ctl_sg_count)`. Same cursor,
by the consumer's own path, without touching the list.

### nfsd writes a whole stack struct into a file's extended attributes

`nfsrv_setextattr()` fills five named members of a `struct pnfsdsattr` on
the stack and then hands the object to

```c
	vn_extattr_set(vp, IO_NODELOCKED, EXTATTR_NAMESPACE_SYSTEM,
	    "pnfsd.dsattr", sizeof(dsattr), (char *)&dsattr, p);
```

`sizeof(dsattr)`, not "the five members". Any padding the ABI puts inside
the struct — or inside the two `struct timespec` it copies wholesale — is
kernel stack that lands in the `pnfsd.dsattr` extended attribute and
comes back out of it to any pNFS client that reads the file's attributes.
PBSD builds six architectures and the layout is not the same on all of
them. `nfsrv_pnfscreate()` writes the same struct to the same attribute
with the same `sizeof`; both are zeroed now.

### Four more, and four that are right and do not move the number

| where | what |
|---|---|
| `sbin/routed/if.c:745` | `ifs0` is filled only in the `RTM_IFINFO` arm, which then `continue`s. An `RTM_NEWADDR` not preceded by one reaches `memcpy(&ifs, &ifs0, sizeof(ifs))` and copies the frame into the interface record, then ORs alias flags into it. The kernel emits IFINFO first — and the `ifinit: out of sync` arm ten lines down is this function already saying it does not assume the stream is as expected. |
| `usr.bin/env/envopts.c:365` | `*nextarg = NULL` writes `newargv[1]` when the `-S` string produced no arguments, and the `-v -v` dump printed that NULL and then stepped past it. `newargv` is `malloc`'d, so the loop ran until the heap happened to hold a zero word, printing each word before it as a string. |
| `usr.bin/mkimg/mkimg.c:522` | Neither switch in the per-partition loop assigns `error` on its success paths and neither has a `default`, so a `PART_KIND_SIZE` partition falls through both untouched and `if (error)` tested what the *previous* partition left. Correct only because a nonzero one would already have exited — a chain nobody wrote down. |
| `usr.sbin/bhyve/amd64/fwctl.c:512` | `fwctl_response()`'s `default` arm writes `*retval` only when `remlen` is positive, and returns either way. That value goes straight out of the fwctl I/O port, so an unwritten one is four bytes of the host's stack handed to the guest. `0xffffffff` at the declaration is what the switch's own `default` already means by "nothing to say". |

```
--scope bin/ed --scope sbin/routed --scope usr.bin/env --scope usr.bin/mkimg
--scope usr.sbin/bhyve --scope usr.sbin/pmcstudy
  before  39 findings, 126 OK, 2 ERROR
  after   35 findings, 126 OK, 2 ERROR

--scope sys/cam/ctl --scope sys/fs/nfsserver
  before  36 findings, 26 OK, 1 ERROR
  after   36 findings, 26 OK, 1 ERROR
```

Seven go in the first and three arrive, all three relocations. The
second moves nothing at all, and neither do two of the six in the first.
**Four right-and-unmeasurable fixes in one batch is worth naming**, with
the reason for each, because they are not the same reason:

- `bin/ed/main.c` — `strip_escapes()` reserves its last byte ("Worry
  about a possible trailing escape") and never writes a NUL into it, so
  an `old_filename` that fills the buffer leaves the caller's `strlen()`
  running off a `PATH_MAX` allocation. The analyser's path reads
  `Calling 'strip_escapes'` / `Returning from 'strip_escapes'` with
  nothing in between — it did not walk the body, so the buffer is
  symbolic and every byte is garbage whatever the code does.
- `usr.sbin/pmcstudy/eval_expr.c` — `run_expr()`'s `op == NULL` return
  was the one that left `*lastone` unwritten, and
  `gather_exp_to_paren_close()` returns it for the caller to walk as a
  `struct expression *`. `run_expr()` is mutually recursive with
  `gather_exp_to_paren_close()`, and the analyser does not inline
  recursion.
- `ctl_frontend_iscsi.c` — the invariant check is 80 lines above the
  loop, and relating "`ext_data_filled <= datamove_len`" to "the index
  stays inside the list" needs the sum of the segment lengths, which the
  analyser does not have.
- `nfs_nfsdport.c` — the finding at `:5589` is `nap->na_filerev` being
  garbage, a premise about the *caller's* `nfsvattr`. The padding going
  to disk is something it never reported at all.

The last one is the general case of what col(1) showed earlier in this
document: **the bug you can measure and the bug that is there are not
always the same bug.**

## A file that does not compile, and what was inside it

`usr.sbin/gssd/gssd.c` had never compiled. Sweep 19 was the first run
collected with `--check-errors`, and it said so:

```
FAIL  usr.sbin/gssd/gssd.c does not compile and is not in EXPECTED
```

The immediate error was `KRB5_CALLCONV` undefined in
`gssapi/gssapi_ext.h`. The cause was two directories deeper.

`installed_headers()` builds the include path by asking each Makefile
what it installs and where, and resolving each name through that
Makefile's `.PATH`. `krb5/include/krb5/Makefile` installs `krb5.h` into
`${INCLUDEDIR}/krb5`, and its `.PATH` ends at `${KRB5_DIR}/include` —
where there is a file called `krb5.h`. It is the 1.5-era compatibility
stub, and its whole body is

```c
   /* The MIT Kerberos header file krb5.h used to live here.
      ...  Please update your code to use the new path ... */
#include <krb5/krb5.h>
```

So `krb5/krb5.h` resolved to a file that includes `krb5/krb5.h`. clang
gave up at its depth limit, nothing was defined, and every translation
unit that reaches it failed — silently, because a file that does not
compile reports zero findings and is indistinguishable from a clean one.
**A wrong header is worse than a missing one: a missing one says so.**

The real header is generated, and the recipe is a concatenation —
`krb5/include/krb5/Makefile:41` is a guard around `krb5.hin`, no
configure substitution — so `_installed_generated()` now carries it out,
beside the osreldate.h and bsdxml.h it already did.

That was half. `gssd.c` then failed differently: MIT's `gssapi_ext.h` and
`gssapi_krb5.h` beside **Heimdal's** `gssapi/gssapi.h`, which is what
`include/gssapi/gssapi.h` is. `MK_MITKRB5` is in `src.opts.mk`'s
`__DEFAULT_YES_OPTIONS`, so this tree's GSS-API is MIT's, and the sweep
was reading a configuration nobody builds.

**Generating MIT's `gssapi/gssapi.h` into the general farm was wrong, and
measuring said so:**

```
--scope lib/libgssapi --scope lib/librpcsec_gss
  before   2 findings, 58 OK,  1 ERROR
  after    0 findings, 17 OK, 42 ERROR
```

`lib/libgssapi` is FreeBSD's *own* GSS-API mechanism switch and compiles
against `include/gssapi/gssapi.h`. Two implementations install a header
of the same name, and which one a file wants is not a property of the
header.

It is a property the build states. `usr.sbin/gssd/Makefile:15` and
`lib/libpam/modules/pam_ksu/Makefile:44` are the only two places in the
tree that say `CFLAGS+= -DMK_MITKRB5=yes` — and that flag already arrives
through `ask_cflags()`. So MIT's `gssapi.h` lives in its own shim,
reached only by a file whose own flags carry it: read out of the
Makefile rather than guessed from a path.

```
--scope lib/libgssapi --scope lib/librpcsec_gss --scope usr.sbin/gssd
--scope lib/libpam/modules/pam_ksu
  2 findings, 59 OK, 2 ERROR
```

`lib/libgssapi` is back where it was, `gssd.c` compiles, and
`pam_ksu.c` still fails on a different missing header (`profile.h`,
MIT's, from `crypto/krb5/src/util/profile`) — as it did before, with six
errors rather than one.

The gate for this is in `test_includes.py` and walks the farm rather than
the mapping, because the farm is what goes on `-I`: a bad entry the
generation writes over is harmless, and one it does not is the bug.
Verified by removing the generation: three checks fail, and pass again on
restore.

### getgrouplist() reports the groups it FOUND, not the groups it stored

And inside the file, once it compiled: one finding, and a class.

```c
	int len = NGROUPS;
	int groups[NGROUPS];
	getgrouplist(pw->pw_name, pw->pw_gid, groups, &len);
	result->gidlist.gidlist_len = len;
	result->gidlist.gidlist_val = mem_alloc(len * sizeof(int));
	memcpy(result->gidlist.gidlist_val, groups, len * sizeof(int));
```

`lib/libc/gen/getgrouplist.c` is a one-line wrapper around
`__getgroupmembership()`, which ends

```c
	/* too many groups found? */
	return (*grpcnt > maxgrp ? -1 : 0);
```

— `*grpcnt` is the number of groups **found**, which can exceed the array,
and the `-1` says so. Nobody was reading it. A user in more groups than
`NGROUPS` makes `len > NGROUPS`, and the `memcpy` reads that far past a
stack array in **gssd(8), which runs as root**, and sends the result to
the kernel GSS layer as that credential's supplementary group list.

There are five call sites in the tree that ignore the return and then use
the count as a bound:

| where | what it walks |
|---|---|
| `usr.sbin/gssd/gssd.c` `_gss_get_unix_cred()` | its caller's `gid_t groups[NGROUPS]`, in a `for (i = 0; i < len; i++)` |
| `usr.sbin/gssd/gssd.c` `gssd_pname_to_uid_1_svc()` | its own stack array, by `memcpy` |
| `lib/librpcsec_gss/svc_rpcsec_gss.c` | `uc->gidlen = len` with no bound at all — **this is the library every RPCSEC_GSS server links** |
| `usr.bin/id/id.c` ×2 | a `malloc` of `_SC_NGROUPS_MAX + 1`, which the group database can exceed |

`usr.bin/mdo/mdo.c:753` is the one that gets it right —
`if (ngroups > ngroups_alloc) err(...)` — and it is the model. All five
now clamp to what they asked for, which is exactly the number
`getgrouplist()` stored.

`gssd_pname_to_uid_1_svc()`'s array also becomes `gid_t` and the copy
element-by-element, which is what the same file already does in
`gssd_accept_sec_context()` for the same question, and the reason the
analyser was calling the `mem_alloc()` size wrong.

```
--scope usr.sbin/gssd --scope lib/librpcsec_gss --scope usr.bin/id
  before  1 finding, 7 OK, 0 ERROR
  after   0 findings, 7 OK, 0 ERROR
```

The one finding was `unix.MallocSizeof` on the `sizeof(int)`. The
out-of-bounds read it sits next to is not something the analyser
reported — it could not, because `getgrouplist()`'s contract lives in
another translation unit. **The gate that found this was `--check-errors`,
not the checkers.**

### Three the first pass missed, and the sweep that found them

Sweep 20 is the first `bin`/`sbin`/`usr.bin`/`usr.sbin` sweep collected
on a clean tree with `--check-errors`, and it passes:

```
sweep 19  689 findings, 1821 OK, 41 ERROR   FAIL usr.sbin/gssd/gssd.c
sweep 20  689 findings, 1822 OK, 40 ERROR   ok, all 40 on the record
```

29 `core.uninitialized.*` remain across 1,862 translation units, and
reading the list against what this document already records turned up
**six entries in five files that had never been read.** Three are real.

`usr.sbin/ctladm/ctladm.c:896` and `:902`, in `cctl_error_inject()`:

```c
	optret = getoption(cctl_err_types, optarg,
			   &err_type, &argnum, &subopt);
	err_desc.lun_error = err_type;
```

`getoption()` sets `*cmdnum`, `*argnum` and `*subopt` inside its match
branch only, so `CC_OR_NOT_FOUND` writes none of them — and the store
into `err_desc` happens **before** the `CC_OR_NOT_FOUND` test fourteen
lines down. The `goto bailout` there means the value never reaches the
`CTL_ERROR_INJECT` ioctl, so this is a read whose result is discarded;
the ordering is still wrong, and the file's own two other `getoption()`
call sites, at `:508` and `:4341`, both check before using. The same
shape as `top(1)`'s `renice_procs()` earlier in this document.

`usr.bin/systat/netstat.c:321` is not discarded:

```c
		if (istcp) {
			KREAD(inpcb->inp_socket, &sockb, sizeof (sockb));
			enter_kvm(inpcb, &sockb, tcpcb.t_state, "tcp");
		} else
			enter_kvm(inpcb, &sockb, 0, "udp");
```

The `KREAD` is inside the TCP arm. The UDP arm passes the same `&sockb`
without ever reading into it, and `enter_kvm()` takes
`so->so_rcv.sb_ccc` and `so->so_snd.sb_ccc` out of it. UDP is the
*second* pass — the `goto again` twenty lines down sets `istcp = 0` —
so every UDP socket was displayed with the last TCP socket's queue
counts, and with the frame's bytes when there were no TCP sockets at all.

`usr.sbin/bsdinstall/partedit/gpart_ops.c:899` decides where the
installer offers to write:

```c
		LIST_FOREACH(gc, &pp->lg_config, lg_config) {
			if (strcmp(gc->lg_name, "start") == 0)
				partstart = strtoimax(gc->lg_val, NULL, 0);
			if (strcmp(gc->lg_name, "end") == 0)
				partend = strtoimax(gc->lg_val, NULL, 0);
		}
```

Neither is initialised, so a provider whose config names neither
silently reused the *previous* partition's extent, and the first one
reused the frame. The tell is twelve lines up: the same author, the same
shape, over the geom's own config — and there it reads `start = end = 0;`
before the loop. `maxstart` had no initialiser either and is returned
through `*npartstart` whether or not the two `if`s that write it fired.
A provider that cannot be placed now makes the function report no free
space at all, because guessing its extent would offer its space to the
installer.

```
--scope usr.sbin/ctladm --scope usr.bin/systat --scope usr.sbin/bsdinstall
  before  92 findings, 42 OK, 0 ERROR
  after   86 findings, 42 OK, 0 ERROR
```

Six go — the three `core.uninitialized.Assign` plus `gpart_ops.c`'s two
`core.UndefinedBinaryOperatorResult` on the same variables — and
forty-seven "arrive", every one of them a finding in the same function
shifted by the new comments.

The other three are premises: `usr.sbin/gstat/gstat.c:280`
(`head_printed` is written only in the `case 'C':` arm that also sets
`flag_C`, and the read is under `flag_C`), `bin/sh/jobs.c:1518`
(`cmdputs()`'s parameter, and both callers that pass a stack buffer write
both of its bytes first), and `usr.bin/fmt/fmt.c:558` — the residue of a
fix that did land at `:556`, where the analyser knows `buf[len]` is now
NUL but cannot relate that to index 1.

**Task #119 was closed a step early.** Its list came from sweep 18, and
sweep 18 was collected before the `-include opt_global.h` reorder and the
userland `-I` filter changed which files compile. Six findings existed in
the tree that the list it was closed against did not contain.

## The fs shard, re-read against a fresh inventory

Sweep 20's fs shard passes `--check-errors` and reports 154 findings over
336 translation units, 11 of them `core.uninitialized.*` — down from 18 in
sweep 18, the difference being the fixes already in this document.

Seven of the eleven are already recorded here. The four that were not are
all NFS.

### nfsd's ERELOOKUP rollback reads cursors five of six paths never save

`sys/fs/nfsserver/nfs_nfsdsocket.c:1359`, in `nfsrvd_compound()`. Each
operation of a compound saves six mbuf cursors so an `ERELOOKUP` return
can roll back and redo it:

```c
		    md = nd->nd_md;
		    dpos = nd->nd_dpos;
tryagain:
```

and the rollback reads them:

```c
		if (nd->nd_repstat == ERELOOKUP) {
			nd->nd_md = md;
			nd->nd_dpos = dpos;
```

The save is inside the operation switch's `default:` case at `:1175`. The
rollback is **after** the switch. So `PUTFH`, `PUTPUBFH`, `PUTROOTFH`,
`SAVEFH` and `RESTOREFH` — the five operations with explicit cases — run
their bodies, `break` out of the switch, and reach `nd->nd_md = md` with
nothing in this function having written `md`. Each of them calls
`nfsd_fhtovp()`, which sets `nd->nd_repstat` from `nfsvno_fhtovp()`, and
`ERELOOKUP` is a value the VFS layer returns.

The six are now also saved at the top of the loop body, right after the
operation number is decoded. This is an **addition, not a move**: the
`default:` case still re-saves where it always did, so the path that
works today is byte for byte unchanged, and the five explicit cases get
the cursors their own reply started at — which is what
`nfsm_trimtrailing()` below wants.

### nfsrvd_readdirplus: an initialiser that covered one of two

`sys/fs/nfsserver/nfs_nfsdport.c:2815`. `savbits` is filled under
`if (nd->nd_flag & ND_NFSV4)`, and the `else` arm reads

```c
	} else {
		NFSZERO_ATTRBIT(&attrbits);
	}
```

— one of the two that are read outside a V4 guard. The entry loop tests

```c
			if ((nd->nd_flag & ND_NFSV3) ||
			    NFSNONZERO_ATTRBIT(&savbits) || ...
```

which short-circuits on `ND_NFSV3`, so `savbits` is reached only for a
request that is neither V3 nor V4. The RPC procedure numbering does not
allow that — READDIRPLUS is a V3 procedure — so this is not reachable;
it is also not a premise anyone reasoned about. It is an initialising
block that exists and covers half of what it should. Zero is exactly
what the V4 arm computes when the client asks for only the four
attributes it clears.

```
--scope sys/fs/nfsserver
  before  33 findings
  after   30 findings
```

Three go: the two above and — unlooked for —
`nfs_nfsdport.c:2166`, a `core.UndefinedBinaryOperatorResult` in
`nfsvno_fillattr()`, which is downstream of the same `savbits`. Nine
"arrive", every one a finding in the same function shifted by the new
comments.

### The other two, and why they are premises

`nfs_clrpcops.c:3834` reads `rderr` under `if (nd->nd_flag & ND_NFSV4)`,
and `rderr = 0` sits under the same test twenty lines up, with
`nfsv4_loadattr()` and an `NFSM_DISSECT` in between — both of which take
`nd` by pointer, so the analyser stops knowing what `nd_flag` holds.
`nfsv4_loadattr()` never writes `nd_flag`; there are 38 references to it
in `nfs_commonsubs.c` and none inside that function assigns it. The same
shape as `kdump`'s `ktr_header.ktr_type` earlier in this document, where
the fix was a local — here there is nothing to fix, because the two
tests genuinely agree.

`nfs_nfsdsocket.c:641` reads `vp`, which the `!(nd->nd_flag & ND_NFSV4)`
block above sets on both of its arms — `vp = NULL` explicitly, or through
`nfsd_fhtovp()`, whose first statement is `*vpp = NULL`. The read at
`:641` is in the `else` of `if (nd->nd_flag & ND_NFSV4)`, the same
predicate. Same reason, same verdict.

### The shard without the gate

`progs` — `bin`, `sbin`, `usr.bin`, `usr.sbin` — was the one analyse
shard in `.github/workflows/pbsd-verify.yml` that ran without
`--check-errors`. The comment beside it said why, and set a condition:

> 133 of its 1,862 translation units still do not compile, in seven
> named classes, and an inventory of a hole that is still being filled
> goes stale faster than it is written. The shard runs so that the
> FINDINGS are collected; the ERROR set joins the gate when the classes
> are gone.

They are gone. Sweep 20 reports 1,822 OK and 40 ERROR, every one of the
forty on the record in `expected_errors.py`. The flag is on.

It is not a formality, and the evidence is `usr.sbin/gssd/gssd.c`: the
first run of this shard collected *with* the flag — sweep 19 — failed on
it, and it had never compiled in eighteen sweeps. Nothing else had
noticed, because a file that does not compile reports zero findings and
is indistinguishable from a clean one. **This was the one shard where
that could happen.**

So the exemption gets a gate of its own. `check_shards.py` now reads the
analyse matrix and fails when a shard runs without `--check-errors` and
is not in an `UNGATED` table — which is empty. It also fails when an
`UNGATED` entry names a shard that *does* carry the flag (a stale
exemption, the same shape the rest of that script was written to catch),
and when the matrix regex matches nothing at all, because a check reading
zero shards passes silently.

All three verified by deliberate breakage: removing the flag from `progs`
fails the first, adding `progs` to `UNGATED` fails the second, renaming
the matrix key from `name:` to `nom:` fails the third, and the file
restored passes all of them.

## Sweep 20, the kern shard

`sys/kern`, `sys/vm`, `sys/net`, `sys/netinet`, `sys/netinet6` — 466
translation units, 461 OK, 5 ERROR, 249 findings. The five ERROR are all
on the record in `expected_errors.py`, so `--check-errors` passes; this
is the first time the flag has been run against a freshly-triaged kern
shard, and unlike `progs` it had nothing to say.

169 of the 249 are `core.NullDereference`, the premise-heavy class task
#51 characterised. The other 80 are where the defects were. Nineteen
fixes came out of them, in eighteen files.

### Two that hand the caller a garbage pointer

`vm_page_alloc_noobj_domain()` declares `vm_page_t m;` and writes it only
inside its three allocation arms: the `VM_ALLOC_NOFREE` arm, the per-CPU
page cache arm, and the arm guarded by `vm_domain_allocate()`. The
ordinary out-of-memory path takes **none** of them — a request without
`VM_ALLOC_NOFREE`, a domain whose `vmd_pgcache[VM_FREEPOOL_DIRECT].zone`
is NULL, and a `vm_domain_allocate()` that refuses because the domain is
short. `if (m == NULL)` then read an uninitialised local, and a garbage
non-zero fell through to `found:`, where the page is dressed up with
flags and returned to the caller as a real page. `m = NULL` now starts
every attempt, at the `again:` label rather than the declaration, so a
retry cannot carry a stale value either.

`link_elf_lookup_set()` called `link_elf_symbol_values()` twice and threw
away the return both times, then read `symval.value`. That return is not
decorative: `link_elf_debug_symbol_values()` — the default, because
`debug.link_elf_leak_locals` defaults to true — returns ENOENT without
writing `*symval` when the symbol is outside `symtab` and `symtab` is the
ddb table. `start` and `stop` then became garbage kernel pointers, and
`count = stop - start` a garbage length, which the caller walks as a
linker set.

### A NULL that a `== 0` test was never going to catch

`link_elf_search_symbol()` **always returns 0**. When it matches nothing
it sets `*sym = NULL` and `*diffp` to the raw address. Its one caller,
`link_elf_ifunc_symbol_value()`, tests `... == 0 && off == 0` — meaning
"an exact match" — and that is also true when the resolver returned NULL
and nothing matched it, at which point `es->st_value` dereferences NULL.
The same function, with the same shape, exists in `link_elf_obj.c`. Both
now test `sym != NULL`.

### Four leaks an unprivileged process can drive

`fsetown()` allocates a `struct sigio` and `crhold()`s a credential
before it knows whether the operation will succeed. Every failing path —
`pget()` not finding the pid, `pgfind()` not finding the group, and both
of the same-session policy checks — leaves it neither stored in `*sigiop`
nor linked onto a list, and nothing freed it. `fcntl(fd, F_SETOWN, pid)`
against a process in another session leaks the allocation *and* a ucred
reference, once per call, without bound.

`sysctl_kern_proc_kstack()` allocates a `struct kinfo_kstack` and a
`struct stack` before `p_candebug()`. That check's early return is the
only exit past the two allocations that did not take them along; the tail
of the function frees both. Reading `kern.proc.kstack.<pid>` for a
process you may not debug leaked both, per call.

`uipc_sosend_stream_or_seqpacket()` — the AF_LOCAL stream and seqpacket
send — sets `uio` and `resid` only on the `uio0` path. On the kernel
mbuf path (`m != NULL`, which `sys/rpc` uses over local sockets) both
were left undefined, and the send loop then tests `uio != NULL` to decide
whether to copy in more, and the tail *writes* `uio->uio_resid` through
it. Not a leak but the worst of the four: a read and then a write
through an uninitialised stack pointer.

`accept_filt_generic_mod_event()` copies its argument into a fresh
allocation and hands it to `accept_filt_add()`, which frees it on the one
success path that does not keep it and returns EEXIST without freeing
anything. Neither did the caller.

### One a remote peer can drive

`Add_Global_Address_to_List()` in the SCTP NAT returns 0 — "already
exists, so don't add" — without inserting the address and without freeing
it. All three callers in `AddGlobalIPAddresses()` log the refusal and
move on. The loop above them is bounded by `sysctl_track_global_addresses`
against `assoc->num_Gaddr`, and `num_Gaddr` is only incremented on
success, so a duplicate never raises it and that limit never trips. A
peer that repeats one IPv4 address parameter across packets leaks a
`struct sctp_GlobalAddress` per repeat, in the kernel, through an
ipfw/natd SCTP NAT. The helper now frees what it declines.

`bw_meter_prepare_upcall()` is the same shape with a different cause: a
full `buf_ring` does not take the pointer, the code logged and carried
on, and the ring fills exactly when upcalls outrun the daemon draining
them — so the leak is worst under the load that causes it.

### An IPv6 netmask that was never fully written

`ip6_writemask()` stops as soon as the prefix runs out, leaving the
trailing words of the `struct in6_addr` at whatever the caller had there.
Two of its three callers zero the whole sockaddr first and were fine.
`rt_get_inet6_parent()` does neither. Its `struct in6_addr mask6` is an
uninitialised local, so the first pass masks the lookup key with stack
garbage; and it reuses the same buffer for every pass of a loop that
walks the prefix length *down*, so each later mask still carries the
previous, longer prefix in the words it does not reach. Going from /33
to /32, word 1 keeps `0x80000000` and bit 32 survives a mask that should
have cleared it — the covering route is then looked up under the wrong
key. Both halves are fixed in the function itself, which now writes all
four words and never more than four; `mask` is a `uint8_t`, and all three
callers range-check it against 128 today, which this stops being the only
thing between a bad one and `0xFFFFFFFF` written past a 16-byte address.

### Big-endian only, and a write

`kern_select()`'s `getbits()` macro goes straight to `done:` when its
copyin fails, and `done:` runs `swizzle_fdset()` over `obits[0..2]`
regardless of how far `getbits()` got. On a little-endian kernel that
macro is empty and nothing happens. On a big-endian LP64 kernel — which
PBSD targets — it is not: it tests the pointer and, when it is not NULL,
byte-swaps in place through it. A failed copyin of the read set wrote
through two uninitialised stack pointers. All six are now NULL before
the first `getbits()`.

### Four switches and a loop bound that were never total

`sbsetopt()` has two switches on `sopt->sopt_name` and neither had a
`default`. An unmatched name left the listening arm's `lowat`/`hiwat`/
`flags` dangling and the connected arm's `wh` undefined — which
`SOCK_BUF_LOCK()` switches on — with `sb` still NULL at the three loads
above that lock. Only `sosetopt()`'s own four-case arm and
`nl_setsbopt()` reach it, so the four names are validated once, up front,
before either switch.

`lagg_port_create()`'s switch on the lagg's own `if_type` ends
`default: break;`, falling through with `if_type` unset — and the store
at the bottom of the function puts it in the *member* interface's
`if_type`, which the protocol dispatch reads. `lagg_clone_create()` only
ever builds an IFT_ETHER or IFT_INFINIBAND lagg, so nothing reaches that
arm; refusing, which is what the two arms above it do, costs nothing.

`iflib_dma_alloc_multi()` returns `err` without initialising it when
`count <= 0`, because the only assignment is inside the loop.

`corefile_open_last()` reads `error` uninitialised when `debug.ncores` is
zero — and it can be, because `sysctl_debug_num_cores_check()` explicitly
clamps a negative value to 0. With a `%I` in the core format the loop
then never runs, neither store to `error` happens, and both arms of the
tail read it: a garbage non-zero comes back as an errno, a garbage zero
publishes a NULL `*vpp` as a successful open.

`dxr_build()` allocates its aux struct with `M_NOWAIT` and no `M_ZERO`,
and the initialisation block that follows names ten fields but not
`updates_low`, `updates_high` or `updates_mask`. `updates_low >
updates_high` is then read from uninitialised heap. It is benign on the
first build, where the range table is NULL and the rebuild is forced
anyway — but a build that returns early because the `M_NOWAIT` extension
table did not come back leaves the aux struct behind with the range table
allocated, and the *next* build reaches that test with the rebuild flag
clear. Garbage that reads as a valid range then drives both the chunk
walk and the `bzero` of `updates_mask`, which are indexed by it. The
allocation gets `M_ZERO` and the block states the file's own
empty-range convention.

### Two divisors nothing established

`dumper_create()` copies `blocksize` out of the driver's template and
never checked it. `dump_check_bounds()` takes `length % di->blocksize`
and `offset % di->blocksize`, `_dump_append()` rounds down by it, and the
`malloc()` in `dumper_create()` itself would hand out a zero-length
`blockbuf` that those same paths write through. A zero divides by zero in
the middle of a kernel dump — the one moment there is nothing left to
report it with.

`sysctl_kern_callout_stat()` prints `st / count` and `spr / count` where
`count` is just a sum over every call wheel. Nothing in the function
establishes that it is non-zero. It now says so and stops.

### What the other sixty-one are

Six `core.CallAndMessage` in `kern_condvar.c`, `kern_synch.c` and
`kern_exit.c` all say "2nd function call argument is an uninitialized
value" at a `WITNESS_RESTORE()`. `WITNESS_SAVE_DECL(n)` declares
`n__wfile` and `n__wline`, and `witness_save(lock, &n__wfile, &n__wline)`
fills them — in `subr_witness.c`, another translation unit. The analyser
cannot see the write, so the pair stays "uninitialised" and
`witness_restore(lock, n__wfile, n__wline)` reports its second argument.
The recurring out-parameter shape, six times over.

`tcp_output.c:1380` and `tcp_syncache.c:2003`/`:2035` read `ulen`, which
is written under `tp->t_port` / `sc->sc_port` and read under the same
test, with `sleepq`- and mbuf-level calls in between that take the
struct by non-const pointer. `uipc_ktls.c:2064` reads `wlocked` under
`tls->tx`, written under `tls->tx`. `pfil_link()`'s two are the same:
`in` and `out` are allocated only when `PFIL_UNLINK` is clear, and the
early return that "leaks" them is taken only when it is set. The
predicate-tested-twice shape, five times over.

`kern_cpu.c:784` needs the two list walks in `cpufreq_insert_abs()`
related: the reverse walk fails only when the new frequency exceeds every
entry, and the forward walk's `>=` then matches the first entry, so
`level` is always inserted. `vfs_cache.c:2923`, `sysv_shm.c:419` and
`if_vlan.c:445`/`:508` all rest on a `malloc()` followed by a loop that
initialises every element, which the analyser does not unroll.
`subr_sfbuf.c:103`, `vm_phys.c:1160`, `in6_mcast.c:730` and `:2137`,
`sctp_auth.c:623` and `vfs_mountroot.c:1164` are ownership transfers into
a list, an RB tree or a callee's out-parameter.

`ifa_alloc()`'s three rest on `M_ZERO`, which the kernel `malloc()` model
does not carry. `kern_jail.c:663` and `subr_scanf.c:531` rest on
`M_WAITOK` and on a switch arm respectively — the latter with the
original author's `/* XXX just to keep gcc happy */` still beside the
`ccfn = NULL`. `kern_exec.c:1107` needs `namei()`'s `ni_vp` to be
non-NULL on success; `sys_pipe.c:640`/`:646` need `vm_map_find_locked()`
to have written its out-parameter on `KERN_SUCCESS`.

`user_setcred()`'s two `core.StackAddressEscape` are real observations
about a struct that dies with the caller's frame — both callers `return
(user_setcred(...))` on their own stack copy. `update_rtm_from_rc()`'s
two are answered by the comment its own caller already carries: *"any
pointer in @info CANNOT BE USED."*

`sched_ule.c:748`/`:819`, `subr_blist.c:850`, `nhop.c:207`,
`subr_witness.c:2964`, `tcp_subr.c:1645`, `ip_carp.c:2314`,
`alias_nbt.c:516`/`:587`, `if_me.c:461`, `kern_linker.c:2207` and
`ck_pr.h:83` are unconstrained parameters of unexported functions —
task #51's class, unchanged. `bbr.c`'s three divisors rest on
`t_maxseg >= V_tcp_minmss` and `rc_last_options <= TCP_MAXOLEN`; note
that `tcp_bbr_tso_size_check()` guards its subtraction and
`bbr_get_pacing_delay()`, twenty lines away, does not — an asymmetry
worth a second look if `t_maxseg`'s floor ever moves.
`vnode_pager_generic_getpages()`'s two rest on the caller's `count`
being no more than `atop(maxphys)`, which its own KASSERT asserts.
`init_main.c:328` is a sysinit's function pointer; `rack.c:22312` reads
`tso`, which `rack_output()` sets four lines below the `again:` label
every path to that point passes through.

`rtsock.c:898` is the one premise worth naming: `init_sockaddrs_family()`
does nothing at all for a family that is neither AF_INET nor AF_INET6,
and `export_rtaddrs()` then reads `dst->sa_family` from the untouched
stack, after which `update_rtm_from_rc()` publishes the whole sockaddr
into a routing message bound for userland. It is dead today — only
`in_proto.c` and `in6_proto.c` set `dom_rtattach`, so the rib holds no
other family — but it is a fail-open, and it is on the record here
rather than fixed because closing it means deciding what a third family
should do.

### The measurement

Same scope, same tree, the nineteen fixes the only difference:

```
              before   after
OK               461     461
ERROR              5       5     (the same five, all on the record)
findings         249     229
```

Sixteen of the eighteen edited files went to the count the reading
predicted. The two that did not are worth naming, because in both the
change is right and the number could not have moved:

`kern_shutdown.c:1459` is `dump_check_bounds()`, a **static** function
whose `struct dumperinfo *` parameter the analyser has no constraint on
— task #51's class. The fix belongs in `dumper_create()`, where the
blocksize enters the kernel, and putting it there cannot make an
unconstrained parameter constrained. It moved from `:1459` to `:1468`
and stayed.

`route_helpers.c:610` is `IN6_MASK_ADDR(&addr6, &mask6)` in
`rt_get_inet6_parent()`. `ip6_writemask()` now defines all sixteen bytes
of `mask6`, but it is a separate function in the same file and the
analyser declines to inline it — the same reason `kdump`'s `ktr_lenok()`
helper was invisible earlier in this document. `:610` became `:634` and
stayed.

Neither is a reason to write the fix differently. A finding that
survives a correct fix is a statement about the instrument.

### Three the NullDereference class had after all

The first reading of the kern shard treated `core.NullDereference` as
task #51's premise-heavy class and went at the other 80 findings. That
was the right order — but "premise-heavy" is not "empty", and reading
the class properly afterwards found three.

The class breaks down: 90 of the 169 are two `ARB_GENERATE_STATIC` lines
in `subr_stats.c` and seven more are `<something>_RB_INSERT_COLOR`, all
red-black rebalance macro expansions, the class this document has
described twice already. Five are inline functions in headers
(`refcount.h`, `atomic.h`, `vm_page.h`, `vm_pager.h`, `sf_buf.h`) reached
from a caller whose pointer the analyser cannot constrain. Six are
`LOCK_CLASS()` dispatch tables that are not `const` — task #88. Thirteen
are the `t_port`/`sc_port`/`crte` predicate in `rack.c` and
`tcp_syncache.c`. That leaves 48 read one at a time.

`bridge_input()` declares `struct bridge_softc *sc = NULL` and does not
assign it until after the header-pullup block — but that block's failure
path does `if_inc_counter(sc->sc_ifp, IFCOUNTER_IERRORS, 1)`. `sc` is
provably NULL there. `m_pullup()` returns NULL when it cannot get an
mbuf or when the whole chain is shorter than an Ethernet header, so a
runt frame or mbuf exhaustion is a NULL dereference on the bridge
receive path. The counter now goes on the member interface, which is
what the function actually has at that point.

`newreno_cong_signal()` opens by computing `beta`, `beta_ecn` and
`factor`, and all three lines test `nreno == NULL` — because
`newreno_cb_init()` returns ENOMEM with `ccv->cc_data` left NULL when its
`M_NOWAIT` allocation fails. Twenty lines later the `CC_NDUPACK` and
`CC_ECN` arms dereference `nreno->newreno_flags` with no guard at all.
A function that establishes its own premise and then ignores it.

`lookup()` in `subr_firmware.c` tests `fp->fw.name != NULL &&
strcasecmp(...)`, and then, a few lines below — on exactly the path where
that first conjunct was false — dereferences `*fp->fw.name` to see
whether the name looks like an absolute path. The guard-on-one-of-a-pair
shape, in a file whose own header comment and whose `MOD_UNLOAD` arm both
treat a NULL name as a thing that happens. One check now covers both
uses.

```
              before   after
findings         215     211      (sys/kern, sys/net, sys/netinet)
```

All four from the three files, nothing else moved, `--check-errors` still
passes.

The rest of the 48 are premises, and two are worth naming because they
are the same fail-open shape as `sbsetopt()` above and were left alone
rather than fixed. `unp_dispose()` switches on `so->so_type` over
SOCK_DGRAM, SOCK_STREAM and SOCK_SEQPACKET with no `default`, leaving
`sb` and `m` undefined for anything else — dead, because AF_UNIX creates
no other type, and there is no fail-closed action for a dispose path.
`init_sockaddrs_family()` is the other, described above. The remainder
are unconstrained parameters (`m_collapse()`, `sl_uncompress_tcp_core()`,
`pim_register_send()`, `slab_free_item()`, `HandleCFIBadType()`),
out-parameters written in another translation unit (`kern_idle.c`'s
`kproc_kthread_add()`), and predicates the analyser lost across an
intervening call (`in6_control_ioctl()`'s `if_afdata[AF_INET6]` guard,
`tcp_subr.c`'s two switches on `addrs[0].ss_family`, `pfil_link()`'s
`PFIL_IN`). Two carry the original author's own note that they knew:
`rn_walktree_from()`'s `last = NULL; /* shut up gcc */`, safe because a
radix head's top node is always internal, and `vsscanf()`'s `ccfn = NULL;
/* XXX just to keep gcc happy */`.

## Sweep 23, the dev shard

`sys/dev` — 2,633 translation units, 2,573 OK, 60 ERROR, 485 findings,
`--check-errors` passes. Twelve fixes, in twelve files, all from the
`core.uninitialized.*` classes: 31 `Assign`, 8 `Branch`, 7 `UndefReturn`,
6 `ArraySubscript` and one `VLASize`, 53 in all.

### A loop whose condition was a constant

```c
	for (i = 0; nitems(res_types); i++) {
```

`vtpci_legacy_alloc_resources()`. `nitems(res_types)` is 2, so the
condition is always true and the loop's only exit is the `break` taken
when `bus_alloc_resource_any()` returns something. A virtio device that
offers BAR0 as neither I/O nor memory space walks `i` past the end of a
two-element array, reads out of bounds, and hands whatever it finds to
`bus_alloc_resource_any()` — forever. It is the only instance of that
shape in the tree; `grep -rn "for (.*; nitems("` finds nothing else.

### Two I²C transfers that reported a random errno

`ig4iic_read()` and `ig4iic_write()` both declare `int error;` and write
it only through `wait_intr()`, which runs when a FIFO is not ready. A
read whose bursts were all satisfied by the RX FIFO, or a write that
fits the TX FIFO in one pass — the common case for an I²C HID touchpad —
falls out of the loop and returns an uninitialised local as the
transfer's status. With a garbage non-zero the transfer looks failed; with
a garbage zero a failed one looks fine.

### Three descriptors and a MAC built from stack

`fwohci_add_rx_buf()` fills a two-element `bus_addr_t dbuf[2]` and then
programs OHCI DMA descriptors from it. Two of its paths do not fill what
they program:

- the `ir->buf == NULL` case of the second branch — reached when the
  queue is marked `FWXFERQ_EXTBUF` but has no buffer attached, the exact
  pair the branch above tests — sets `dsiz` and bumps `dbcnt` but leaves
  the `dbuf` slot alone;
- the first branch skips `fwdma_malloc_size()`, the only writer of
  `dbuf[0]`, whenever `db_tr->buf` survived from a previous arm — and
  stopping an isochronous channel clears `FWXFERQ_RUNNING` without
  calling `fwohci_db_free()`, so a restart takes exactly that path.

In both the controller is handed a bus address from an unwritten stack
slot. The second one only became visible after the first was fixed.

`ql_read_mac_addr()` drops the return value of `ql_rd_flash32()` twice.
That function has three failure paths — the flash semaphore and either
of two indirect register accesses — and none of them writes `*data`. A
card whose flash read fails comes up with an interface MAC address made
of stack bytes.

### A receive path that decodes nothing and uses it anyway

`qcom_ess_edma_rx_ring_complete()` pulls `len`, `num_rfds`, `port_id`,
`priority`, `hash_type`, `hash_val`, `flow_cookie` and `vlan` out of the
return descriptor — but only inside `if (rrd->rrd7 & EDMA_RRD_DESC_VALID)`.
The `else` set `len = 0` and nothing else. Downstream, `port_id` indexes
`sc_gmac_port_map[]` and then `sc_gmac[]`; `vlan` and `priority` go into
`m_pkthdr.ether_vtag`; `hash_val` goes into `m_pkthdr.flowid`. All from
uninitialised stack, on the path the hardware takes when it hands back a
descriptor it has not filled.

The first fix set `port_id` and added a bound check, and the sweep that
measured it reported **two new findings** at the VLAN and hash blocks —
the other four locals, which the array-subscript report had been masking.
The bug you can measure and the bug that is there are not always the same
bug, and here the first fix is what made the second visible.

While reading it: line 470 builds the VLAN tag as
`(vlan & 0xfff) | ((priority < 1) & 0xf)`. `priority < 1` is a
comparison, so that disjunct is 0 or 1 whatever the priority is. It is
plainly not what was meant — but `<` for `<<` still leaves the shift
wrong for an 802.1Q TCI, where the PCP sits at bit 13, so correcting it
means deciding what the author intended rather than what they typed. It
is on the record here and not changed.

### Four smaller ones

`gve_prep_tso()` leaves `csum` undefined for a TSO frame that is neither
IPv4 nor IPv6 — `l4_off` is initialised to 0 and `csum` is not — and then
writes it into `th->th_sum`, which with `l4_off` at 0 lands sixteen bytes
into the Ethernet header. The caller already counts and drops on a
non-zero return, so it now gets one.

`lio_get_ringparam()` returns `err` from a switch with no `default`.

`sdio_func_read_cis()` declares `char *cis1_info[4]`, fills as many as it
finds NUL-terminated strings for, and prints all four. This is the same
code, with the same defect, as `usr.bin/sdiotool/cam_sdio.c` — which
already carries this fix. The kernel twin had gone unnoticed because
nothing had swept `sys/dev` at this level before.

`ar9280ChangeGainBoundarySettings()` returns `*diff`, and writes it only
when the chip is Merlin 2.0 or later *and* the board's power table offset
differs from the default. Its one caller passes `&diff` and assigns the
result back to `diff` — so on any other path the function hands the
variable straight back unwritten, and `NUM_PDADC(diff)` bounds two loops
with it. Zero is what the written path computes when the offsets are
equal.

`mlx4_counter_alloc()` returns `-ENOSPC` from its multifunction path
without writing `*idx`. `__mlx4_counter_alloc()`, the other path, sets it
to the sink counter first — and `mlx4_allocate_default_counters()` stores
`idx` for `!err || err == -ENOSPC`, so on the virtualised path it was
storing an uninitialised local as a port's default counter index.

### The measurement

Same scope, same tree, the twelve fixes the only difference:

```
              before   after
OK             2,573   2,573
ERROR             60      60     (the same sixty)
findings         485     473
```

Every edited file moved. The four `core.uninitialized.*` classes account
for all twelve:

```
  Assign          31 -> 25
  UndefReturn      7 ->  3
  Branch           8 ->  7
  ArraySubscript   6 ->  5
```

`fwohci.c` went 3 -> 2: the uninitialised-descriptor finding is gone and
the two that remain are a NULL dereference and a leak in
`fwohci_db_init()`, a different function, unread.

The other 420 findings in this shard have not been read yet, and this
document does not claim otherwise. Twenty of the 37 `core.DivideZero` are
the allwinner clock class task #87 already characterised and put a lint
behind. Twenty-six of the `core.NullDereference` and
`core.UndefinedBinaryOperatorResult` are two files, `mpr_config.c` and
`mps_config.c`, in one repeated shape — that one is read and named in the
next section.

### The stale `reply`, forty-two times

`mpr_config.c` and `mps_config.c` — the LSI/Broadcom SAS controllers'
configuration-page readers — carried 26 of the dev shard's findings
between them, all one shape. Each function does this twice, once for the
page header and once for the page:

```c
	error = mpr_wait_command(sc, &cm, 60, CAN_SLEEP);
	if (cm != NULL)
		reply = (MPI2_CONFIG_REPLY *)cm->cm_reply;
	if (error || (reply == NULL)) {
		...
		goto out;
	}
```

The guard on the assignment is there because `mpr_wait_command()` writes
`*cmp = NULL` when it reclaims a timed-out command. But `reply` is a
**function-scope local**, so on the second command it still holds the
first command's reply — the `reply == NULL` test passes, and the function
falls through to `bcopy(page, config_page, MIN(cm->cm_length, ...))`,
dereferencing the very NULL it had just declined to take a reply from.

And `reply` is declared without an initialiser, so on the *first* command
the same test reads garbage — which is what the ten
`core.UndefinedBinaryOperatorResult` in these two files were saying, at
the `reply == NULL` line, while the seventeen `core.NullDereference` were
saying the other half at `cm->cm_length`. Two checkers, two symptoms, one
missing conjunct.

The fix is `cm == NULL` in the test, at all 42 sites. Short-circuit
order does the rest: `cm` is tested before `reply`, so the uninitialised
read is gone too and no declaration had to change. Two of the 24 sites in
`mpr_config.c` had no `cm != NULL` guard on the assignment at all; they
now have the same one as the other 22.

`aac_define_int_mode()` came along for the ride: its legacy-interrupt arm
sets `aac_max_msix = 1` and its MSI-X arm only clamps the firmware's
value *down*, against `msi_count`, never up — so a controller reporting
zero vectors reaches `aac_max_fibs / aac_max_msix` with a zero divisor.
The floor one arm already establishes now applies to both.

```
              before   after     (sys/dev/mpr, sys/dev/mps, sys/dev/aacraid)
OK                22      22
findings          30       3
```

Twenty-seven of thirty, from forty-four one-line changes.

### The dev shard's `core.CallAndMessage`, and what it uncovered

`core.CallAndMessage` — "an uninitialised value passed to a function" —
is the same defect as `core.uninitialized.*` seen from the call site
rather than the assignment, and it was the next class to read: 79 in the
dev shard, 70 of them argument reports and 9 "called function pointer is
null". Six were real.

`usie_if_rx_callback()` — the Sierra Wireless USB modem's receive path —
switches on the ether type of each frame in an aggregated USB transfer:

```c
		default:
			DPRINTF("unsupported ether type\n");
			err++;
			break;
		}
```

That `break` leaves the **switch**, not the loop, and the two
`netisr_dispatch(ipv, ...)` calls twelve lines down take `ipv` — a
`uint8_t` local with no initialiser. `netisr_dispatch()` indexes
`netisr_proto[]` with its first argument and calls through what it
finds, so a device sending a frame with any ether type but IP or IPv6
picked a protocol handler out of the stack. The arm now does what the
"received wrong type of packet" block above it does: advance past the
frame and go round again, or free and stop.

`struct ufshci_query_param` has seven fields and is passed **by value**
to `ufshci_ctrlr_cmd_send_query_request()`, which does
`upiu->length = param.desc_size`. Five of the six functions that build
one set six fields and leave `desc_size` alone — only
`ufshci_dev_read_descriptor()` sets it — so every UFS flag read, flag
set, flag clear, attribute read and attribute write sent a query UPIU
whose length field was an uninitialised local.

Fixing that made a sixth visible in the same file:
`ufshci_dev_config_write_booster()` declares `uint32_t alloc_units;` and
writes it, on the LU-dedicated path, only inside a loop iteration whose
unit-descriptor read succeeded — every iteration may `continue` past it.
The `if (alloc_units == 0)` after the loop then read it either way, and
a garbage non-zero went on to size the WriteBooster buffer.

`sdio_get_common_cis_addr()` has two:

- its `err:` label sits **inside the body** of the
  `if (a < SD_IO_CIS_START || a > ...)` that follows it, so the three
  `goto err` above jump past one, two or all three of the assignments
  that build `a` — which the `CAM_DEBUG` at that label then prints;
- that same range check leaves `*addr` unwritten and returns `error`,
  which on that path is **0**. A card reporting a CIS pointer outside the
  valid range therefore had `sdiob_get_card_info()` read the CIS from an
  uninitialised local. It now returns EINVAL.

`gve_xmit()` is `gve_prep_tso()`'s neighbour with the same shape:
`l4_off` is written only by the IPv6 and IPv4 arms and `csum_offset` only
under `has_csum_flag`, and both go to `gve_tx_fill_pkt_desc()`
unconditionally.

```
              before   after     (sys/dev/gve, sdio, ufshci, usb)
OK               164     164
findings          46      35
```

`ufshci_dev.c` and `sdiob.c` are now clean. What remains in those four
directories is nine `core.CallAndMessage` on out-parameters written in
another translation unit — `bus_space_read_region_4()`,
`HYPERVISOR_event_channel_op()`, `usbd_*` — and one `unix.Malloc` pair
in `ufshci_sim.c`, unread.

### Three where the same file already had the right idiom

`core.UndefinedBinaryOperatorResult` in the dev shard is 55 findings
after the batches above. Three of them are the shape this document keeps
returning to: a file that tests something correctly in one place and not
in another.

`fdc_sense_int()` has **three** failure returns — `fdc_cmd()` failing,
`FD_NOT_VALID`, and `fdc_err()` after the cylinder read — and writes
`*st0p` only after the first command succeeds and `*cylp` only after the
second. Four call sites test `== FD_NOT_VALID` and then read `st0` and
`cyl`; a fifth, at `:1962` in the same file, tests `== 0`. The four now
test `!= 0`, which for the FD_NOT_VALID case is exactly what they did
before.

`em_newitr()` writes `bytes = bytes_per_packet = 0;` on one line and
leaves `packets` out of it. `packets` is set only inside the two
`if (txpackets != 0)` / `if (rxpackets != 0)` blocks, and the early
return above covers only both **byte** counters being zero — a ring's
byte and packet counters are loaded by separate `atomic_load_long()`
calls, so a packet counter can still read zero while its byte counter
does not. The AIM latency state machine then branches on an
uninitialised local four times.

`doopen()` in `psm.c` calls `get_mouse_status(sc->kbdc, stat, 0, 3)`
twice. The second checks `< 3`; the first drops the count and tests
`stat[1]` and `stat[2]` regardless.

```
              before   after     (sys/dev/fdc, e1000, atkbdc)
OK                30      30
findings          18       7
```

`if_em.c` and `psm.c` are clean; `fdc.c` keeps two, both
`bus_space_read_*` out-parameters.

### Four leaks and two divisors a chip register chooses

Reading the dev shard's `unix.Malloc` (19), `unix.cstring.NullArg` (15)
and the non-allwinner `core.DivideZero` (16) turned up six more.

Three of the leaks are the same shape — a second allocation, or a second
step, fails and the first is left neither owned nor freed:

- `gpioc_attach_priv_pin()` returns ENOMEM from its second `malloc()`
  without freeing the first, which is linked onto the two lists only
  further down, once both have succeeded;
- `ntb_transport_attach()` breaks out of its child loop when
  `device_add_child()` returns NULL, one line before
  `device_set_ivars()` would have handed the softc over;
- `sndstat_add_user_devs()` `goto done`s when the unpack fails, with the
  `struct sndstat_userdev` neither on the list nor freed. That one is an
  ioctl a user drives with a malformed nvlist, so it repeats.
  `sndstat_dsp_unpack_nvlist()` has exactly one failure return — an
  `nvlist_clone()` — taken before it writes any of the struct's fields,
  so a bare `free()` is right and freeing the members would not be.

`nvmf_allocate_qpair()` declares `struct nvmf_qpair *qp;` and writes it
only inside `SLIST_FOREACH`. `nvmf_supported_trtype()` above it only
says the transport type is in range — not that any transport has
registered for it — so with an empty list the `qp == NULL` test read an
uninitialised local, and a garbage non-zero went on to
`qp->nq_transport = nt` with `nt` NULL.

The two divisors are both in Broadcom's power control:

`bhnd_pwrctl_factor6()` maps a chip register field to a divisor through
a six-case switch and returns **0** for everything else. Four of the
five arms of `bhnd_pwrctl_clock_rate()`'s inner switch then divide by
`m1`, or by a product containing it. Each arm now computes the same
product into one variable and the division happens once, after a check
that refuses zero the way the `default:` arm already refuses an `mc` it
cannot use — the only new behaviour.

`bhnd_pwrctl_slowclk_freq()` returns 0 on two paths it reports with a
`device_printf()` — an unknown device type and an unknown slow-clock
source — and `bhnd_pwrctl_fast_pwrup_delay()` used it as a divisor with
no check. `fpdelay` is already 0 there, which is the answer when there is
no clock to measure a delay against.

```
              before   after     (sys/dev/gpio, ntb, sound, nvmf, bhnd)
OK               164     164
findings          30      21
```

The rest of those three classes are premises the write-ups above have
already named: ownership transfers into an RB tree or a list
(`put_file_offset()`, `gntdev_map_grant_ref()`, `kbdmux_init()`'s three),
`M_ZERO` the kernel `malloc()` model does not carry (`mp_ring_free()`
walking a ring its allocator zeroed), and predicates tested twice across
an intervening call — `mdstart_malloc()`'s five are all `notmapped` and
`vlist`, tested where `dst` is set and again at each use, with the
author's own explicit `dst = NULL` in the two branches that do not use
it.

One more, read and left alone: `scgetc()` in `syscons.c` does
`(void)kbdd_ioctl(sc->kbd, KDGKBSTATE, (caddr_t)&f)` and then tests
`f & SLKED`. The cast to void discards the one thing that says whether
`f` was written. It is a keyboard LED state and the fix is a behaviour
decision — what a scroll-lock key should do when the keyboard driver
will not say — so it is on the record rather than changed.

### Two macros and one interrupt handler, twelve findings

The dev shard's `core.NullDereference` is 189 findings across 111 files
once `mpr_config.c`/`mps_config.c` are out of it — a long tail with a few
clusters. Two of the clusters are one defect each.

`SIENA_SIMPLE_STAT_SET` in `siena_phy.c` guards its memory read with

```c
	if ((_stat) != NULL && !EFSYS_MEM_IS_NULL(_esmp)) {
```

and `EFSYS_MEM_IS_NULL(_esmp)` is `((_esmp)->esm_base == NULL)` — it
dereferences the pointer it is asked about. Twenty lines below the macro,
in the same function, the hand-written version of the same check reads
`stat != NULL && esmp != NULL && !EFSYS_MEM_IS_NULL(esmp)`. The macro now
does too. Six findings, one token.

`sk_intr()` opens by testing whether each of the card's two ports is
present:

```c
	sc_if0 = sc->sk_if[SK_PORT_A];
	sc_if1 = sc->sk_if[SK_PORT_B];

	if (sc_if0 != NULL)
		ifp0 = sc_if0->sk_ifp;
	if (sc_if1 != NULL)
		ifp1 = sc_if1->sk_ifp;
```

— and then six of its per-port arms use those pointers without the test.
`SK_ISR_RX1_EOF` calls `if_getmtu(ifp0)`; `SK_ISR_TX1_S_EOF` calls
`sk_txeof(sc_if0)`, whose first statement is `ifp = sc_if->sk_ifp`;
`SK_ISR_MAC1` calls `if_getdrvflags(ifp0)` and then `sk_intr_xmac(sc_if0)`,
whose first statement is `sc = sc_if->sk_softc`. The `SK_ISR_EXTERNAL_REG`
block eight lines further down, and the two `if_sendq_empty()` calls at
the end of the same function, do make the test. A one-port card — or one
whose second port failed to attach — takes an interrupt whose status word
has a bit for the absent port, and the handler dereferences NULL. Six
findings, six added conjuncts, written the way the three that were
already right are written.

```
              before   after     (sys/dev/sfxge, sys/dev/sk)
OK                52      52
findings          12       1
```

The one left in `if_sk.c` is `sk_txcksum()`'s `thirtytwo` array access,
an unconstrained parameter of a static function.

### A line outside the branch that assigns its operand, five times

`smsatcb.c` — the SAT (SCSI-to-ATA translation) callbacks in the PMC
Sierra driver — has this shape in five functions:

```c
	if (satIntIo == agNULL) {
		satOrgIOContext = satIOContext;
		smOrgIORequest  = smIORequestBody->smIORequest;
		smIORequest     = smOrgIORequest;
	} else {
		satOrgIOContext    = satIOContext->satOrgIOContext;
		smOrgIORequestBody = satOrgIOContext->smRequestBody;
		smOrgIORequest     = smOrgIORequestBody->smIORequest;
	}
	smIORequest  = smOrgIORequestBody->smIORequest;
```

The last line is outside the `if`. `smOrgIORequestBody` is declared
`= agNULL` and assigned only in the `else` arm, so on the other one that
line is `agNULL->smIORequest` — a hard NULL dereference, not a maybe.
And it is the wrong source either way: that arm has already computed
`smOrgIORequest` from `smIORequestBody`, and two of the five had already
assigned `smIORequest` from it — this line then overwrote the right
value with a dereference of NULL. The other three never assigned
`smIORequest` at all on that arm, so it was their only source for it.

`smIORequest = smOrgIORequest;` is correct on both arms and byte-for-byte
what the line already did on the `else` one, where `smOrgIORequest` is
`smOrgIORequestBody->smIORequest` assigned two lines up.

```
              before   after     (sys/dev/pms)
OK                48      48
ERROR             10      10
findings          36      31
```

The three that remain in `smsatcb.c` are `agFirstDword->D2H` reads — an
out-parameter the firmware fills, in another translation unit — and the
rest of `sys/dev/pms` is unread.

## A guard the comment already told us was needed

`ar5212AniControl()` opens with

```c
	const struct ar5212AniParams *params = AH_NULL;
	...
	if (aniState != AH_NULL)
		params = aniState->params;
```

and the function's own comment a few lines above says *"This function
may be called before there's a current channel (eg to disable ANI.)"* —
which is precisely the case where `ah_curani`, and therefore `params`,
is `AH_NULL`. Five of the commands in the switch that follows then use
one or the other unconditionally: `HAL_ANI_NOISE_IMMUNITY_LEVEL`,
`HAL_ANI_FIRSTEP_LEVEL` and `HAL_ANI_SPUR_IMMUNITY_LEVEL` bound-check
the caller's level against `params->maxNoiseImmunityLevel` and friends
and then index a table off `params`; the two weak-signal commands write
`aniState->ofdmWeakSigDetectOff` and `aniState->cckWeakSigThreshold`.
`ar5416AniControl()` in the AR5416 HAL is the same function with the
same hole.

Both now refuse those five commands up front when there is no ANI
state:

```c
	if (aniState == AH_NULL) {
		switch (cmd) {
		case HAL_ANI_NOISE_IMMUNITY_LEVEL:
		case HAL_ANI_OFDM_WEAK_SIGNAL_DETECTION:
		case HAL_ANI_CCK_WEAK_SIGNAL_THR:
		case HAL_ANI_FIRSTEP_LEVEL:
		case HAL_ANI_SPUR_IMMUNITY_LEVEL:
			HALDEBUG(ah, HAL_DEBUG_ANY,
			    "%s: no ANI state, cmd %u ignored\n",
			    __func__, cmd);
			return AH_FALSE;
		default:
			break;
		}
	}
```

`HAL_ANI_PRESENT`, `HAL_ANI_MODE` and `HAL_ANI_PHYERR_RESET` touch
neither `params` nor `aniState`, so the disable-ANI call the comment
describes still works — which is the point of scoping the guard to the
five rather than returning early for everything.

## Two more guard-on-one-of-a-pair

`msk_intr()` in the Marvell Yukon II driver ends its handler with

```c
	if (rxput[MSK_PORT_A] > 0)
		msk_rxput(sc->msk_if[MSK_PORT_A]);
	if (rxput[MSK_PORT_B] > 0)
		msk_rxput(sc->msk_if[MSK_PORT_B]);
```

`msk_rxput()`'s first statement is `sc = sc_if->msk_softc`. Twenty
lines up, the same function's two `msk_txeof()` calls and both arms of
`msk_intr_hwerr()` test `sc->msk_if[port] != NULL` first — a
single-port card leaves the other slot NULL. Both `msk_rxput()` calls
now make the same test.

`fdc.c`'s bailout path had the mirror of it:

```c
	/* Disable ISADMA if we bailed while it was active */
	if (fd != NULL && (fd->flags & FD_ISADMA)) {
		...
		isa_dmadone(bp->bio_cmd == BIO_READ ? ISADMA_READ : ISADMA_WRITE, ...
```

It reads `bp->bio_cmd` for the DMA direction while testing only `fd`.
Twelve lines above, the retry check tests `bp` — because `fdc_biodone()`
sets `fdc->bp = NULL`, so reaching this block with `fd` live and `bp`
already NULL is the ordinary shape of a completed-then-bailed transfer.
The test is now `fd != NULL && bp != NULL && (fd->flags & FD_ISADMA)`.

```
              before   after     (sys/dev/ath + msk + fdc)
OK               125     125
ERROR              5       5
findings          33      21
```

Per file: `ar5212_ani.c` 2 → 0, `ar5416_ani.c` 2 → 0, `if_msk.c`
1 → 0, `fdc.c` 8 → 1 (the seven that went were the `fdc_sense_int()`
fix from the previous batch plus this one; the survivor is a
`device_get_ivars()` read).

## The Atheros HAL's two list searches, in eight copies

`GetLowerUpperIndex()` brackets a target value between two entries of a
sorted list. Six files carry byte-identical or near-identical copies of
it — `ar2413.c`, `ar2425.c`, `ar5413.c`, `ar2316.c`, `ar2317.c` and, under
the name `ar5212GetLowerUpperIndex()`, `ar5112.c` — and all six end the
same way:

```c
	for (tp = lp; tp < ep; tp++) {
		if (*tp == target) { *vlo = *vhi = tp - lp; return; }
		if (target < tp[1]) { *vlo = tp - lp; *vhi = *vlo + 1; return; }
	}
}
```

`tp[1]` on the last iteration is one element past the list. And when the
loop falls out — which it does whenever the list is not sorted ascending,
the thing the comment above it says the caller must guarantee — neither
output is written at all, and the caller uses both as indices into
`pwrList[]` and `VpdList[]`.

The loop now stops one short, which is where it always returned anyway on
a sorted list (at `tp == ep - 2` the second test is `target < ep[-1]`,
which the bounds check above has already established), and the
fall-through answers with the last index rather than leaving the caller's
variables as it found them. An empty list is refused up front: `lp[0]`
and `ep[-1]` in the bounds checks were both out of bounds for it.

`ar5212GetLowerUpperValues()` in `ar5212_reset.c` and
`ar5211GetLowerUpperValues()` in `ar5211_reset.c` are the same search
over values rather than indices, with the same `lp[1]` /
`pList[i + 1]` read one past the end. The ar5212 copy ends its loop with
`HALASSERT(AH_FALSE); /* should not reach here */`, which is nothing at
all in a production kernel; the ar5211 copy has no assert. Both now
write the last element on the fall-through.

The ANI values these tables produce become transmit power and PDADC
settings, so the failure mode is a radio configured from whatever was on
the stack.

## Three unchecked returns that leave the out-parameter unwritten

`xl_read_eeprom()` returns 1 without touching `dest` when the EEPROM does
not come ready. `xl_attach()` checks that for the station address —

```c
	if (xl_read_eeprom(sc, (caddr_t)&eaddr, XL_EE_OEM_ADR0, 3, 1)) {
		device_printf(dev, "failed to read station address\n");
		error = ENXIO;
		goto fail;
	}
```

— and not for the transceiver configuration a hundred and sixty lines
later, which the entire media setup hangs off:

```c
	xl_read_eeprom(sc, (char *)&xcvr, XL_EE_ICFG_0, 2, 0);
	sc->xl_xcvr = xcvr[0] | xcvr[1] << 16;
```

`xcvr` is a two-element stack array. The `sinfo2` read between them
already sets `sinfo2 = 0` first, and `sc->xl_caps` lands in the zeroed
softc, so this was the one of the four that could pick a transceiver type
out of the stack. It now takes the same branch as the station address.

`cqspi_wait_ready()` spins on a status byte a failed read never wrote:

```c
	do {
		cqspi_cmd_read(sc, CMD_READ_STATUS, &data, 1);
	} while (data & STATUS_WIP);
```

`cqspi_cmd_read()` returns before its `data = READ4(...)` when the
controller reports an error, so on that path the loop tests an
uninitialised `uint8_t` — and if it happens to have STATUS_WIP set, does
so forever. It now propagates the error.

`rtl_getport()`'s CPU-port arm reads the link status the same way:

```c
	smi_read(dev, RTL8366_PLSR_BASE + (RTL8366_NUM_PHYS)/2, &v, RTL_WAITOK);
	v = v >> (8 * ((RTL8366_NUM_PHYS) % 2));
```

`smi_read()` returns EBUSY without writing `v` when it cannot take the
bus. The value goes out through the SIOCETHERSWITCHGETPORT ioctl.

`ecore_mcp_trans_speed_mask()` is the fourth of the shape and the
starkest: two of `ecore_mcp_get_transceiver_data()`'s four exits — the
`IS_VF()` one and the MFW-not-initialised one — return before writing
`*p_tranceiver_type`, and the call ignored the return entirely, decoding
the transceiver state and type out of the stack.

## Two functions whose only assignment is compiled out on this platform

`tdsaSendTMFIoctl()` in the PMC Sierra driver reads:

```c
	bit32		status;
	tmf_pass_through_req_t  *tmf_req = ...;
#if !(defined(__FreeBSD__))
	status = ostiSendResetDeviceIoctl(...);
#endif
	TI_DBG3(("Status returned from ostiSendResetDeviceIoctl is %d\n",status));
	if(status !=  IOCTL_CALL_SUCCESS)
	{
		agIOCTLPayload->Status = status;
		return status;
	}
```

On FreeBSD — the only platform this tree builds — the `#if` excludes the
one assignment, so both the test and the value handed back to the caller
of the ioctl are stack contents. The `#else` now says `IOCTL_CALL_FAIL`,
which is what an unimplemented call did in effect anyway, only reliably.

`mpi3mr_pel_enable()` tests a field of a struct it has not filled yet:

```c
	if ((data_out_sz != sizeof(pel_enable) ||
	    (pel_enable.pel_class > MPI3_PEL_CLASS_FAULT))) {
		...
		goto out;
	}
	memset(&pel_enable, 0, sizeof(pel_enable));
	if (copyin(data_out_buf, &pel_enable, sizeof(pel_enable))) {
```

The class range check is repeated — correctly — six lines below, after
the copyin. Here it read the stack, so an ioctl could be rejected or
accepted on the strength of it. Only the size test belongs before the
copy.

## Loops whose zero-trip case the code after them does not expect

`gve_unregister_qpls()`, `gve_adminq_destroy_rx_queues()` and
`gve_adminq_destroy_tx_queues()` share this:

```c
	int err;
	int i;

	for (i = 0; i < num_queues; i++) {
		err = gve_adminq_destroy_rx_queue(priv, i);
		if (err != 0)
			device_printf(priv->dev, "Failed to destroy rxq ...");
	}

	if (err != 0)
		return (err);
```

Two defects in six lines. `err` is never written when the queue count is
zero, and the test after the loop reads it. And each iteration overwrites
the previous one's failure, so the test cannot see anything but the last
queue's result — which defeats its whole purpose. All three now keep the
first error in an `err` that starts at zero, with the per-call result in
its own `rc`. The `create` counterparts of these functions are not
affected: they `goto abort` on the first failure, so their `err` is
always written before it is used.

`psci_fdt_callfn()` reads `node` after a loop over `compat_data` that
does not run if the table is empty; `dsp_oss_audioinfo()` tests `d`
against NULL after a loop that does not run when `pcm_devclass` has no
units; `elink_link_update()` computes

```c
	vars->link_up = (vars->phy_link_up && ... &&
			 (phy_vars[active_external_phy].fault_detected == 0));
```

with `active_external_phy` still at its `ELINK_INT_PHY` default when no
external phy came up — and the loop that initialises `phy_vars` is
bounded by `params->num_phys`, so on a board reporting no phys that entry
was never written. It now clears the whole `ELINK_MAX_PHYS` array.

`vt_allocate_keyboard()` is the odd one of the four: `grabbed` is assigned
under `vd->vd_curwindow == &vt_conswindow` at the top and read under the
same test at the bottom, with `kbd_allocate()` and `kbdd_ioctl()` — both
handed `vd` — in between. Nothing in the code holds `vd_curwindow` still
across those, so `grabbed = 0` is the honest declaration.

## Four more, each its own shape

`bhnd_nvstore_path_new()` frees a pointer it never set. `bhnd_nv_malloc()`
does not zero; the first `goto failed` is taken when
`bhnd_nvram_plist_new()` fails, which is above the only assignment to
`path->path_str`; and the label does

```c
	if (path->path_str != NULL)
		bhnd_nv_free(path->path_str);
```

on whatever the allocator handed back.

`cfumass_t_data_callback()` walks off its scatter-gather list:

```c
	while (sumlen >= sglist->len && sg_count > 0) {
		sumlen -= sglist->len;
		sglist++;
		sg_count--;
	}
```

`sglist` is advanced past the end on the iteration that takes `sg_count`
to zero, and `sglist->len` is evaluated before the count test on the next
one. The two tests are now the other way round.

`read_timeregs()` in the NXP RTC driver breaks out of its retry loop on a
failed register read with `tmr1` unwritten, and the test after the loop —
`if (!sc->use_timer || tmr1 > TMR_TICKS_SEC)` — does not short-circuit
that read when the timer *is* in use. Zero is what the comment beneath it
calls for when the timer is not usable, so the declaration starts there.

`scgetc()`'s scroll-lock key does this:

```c
	case SLK:
		(void)kbdd_ioctl(sc->kbd, KDGKBSTATE, (caddr_t)&f);
		if (f & SLKED) {
```

`save_kbd_state()` and `update_kbd_state()`, two hundred lines down the
same file, both check that return. Here a keyboard driver without
KDGKBSTATE leaves `f` on the stack, and a stack bit that happens to be
set latches SLKED — which stops console output. It now falls back to the
state the console already holds.

## An invariant the switches never had a default for

Every switch on `sc->sc_width` in `cfi_core.c` has cases 1, 2 and 4 and no
default — eight of them, covering the reads, the writes and the program
verify. `cfi_write_block()`'s verify loop is the one the analyser catches:

```c
	switch (sc->sc_width) {
	case 1: val = *(ptr.x8 + i); break;
	case 2: val = *(ptr.x16 + i / 2); break;
	case 4: val = *(ptr.x32 + i / 4); break;
	}

	if (cfi_read(sc, sc->sc_wrofs + i) == val)
```

Attach probed for the width by doubling from 1 and rejected anything
`> 4`, which catches the probe's own overshoot to 8 but not a width of 3
set by a device hint — that path takes the `else if` and only ever meets
the `> 4` test. The check is now `!= 1 && != 2 && != 4`, which is the
invariant all eight switches were written against. The finding itself
stays in the sweep: it rests on a softc field the analyser has no reason
to believe anything about, exactly like the allwinner clock divisors.

### The measurement

Twenty-five files, one tree, one scope, `--check-errors` passing on both
sides:

```
              before   after
OK               632     632
ERROR             28      28
findings         140     111
```

```
  sys/dev/ath/ath_hal/ar5211/ar5211_reset.c        6 ->  2
  sys/dev/ath/ath_hal/ar5212/ar2413.c              2 ->  0
  sys/dev/ath/ath_hal/ar5212/ar2425.c              2 ->  0
  sys/dev/ath/ath_hal/ar5212/ar5212_reset.c        2 ->  0
  sys/dev/ath/ath_hal/ar5212/ar5413.c              2 ->  0
  sys/dev/bhnd/nvram/bhnd_nvram_store_subr.c       1 ->  0
  sys/dev/bxe/bxe_elink.c                          3 ->  2
  sys/dev/etherswitch/rtl8366/rtl8366rb.c          1 ->  0
  sys/dev/flash/cqspi.c                            1 ->  0
  sys/dev/gve/gve_adminq.c                         2 ->  0
  sys/dev/gve/gve_qpl.c                            1 ->  0
  sys/dev/iicbus/rtc/nxprtc.c                      2 ->  1
  sys/dev/mpi3mr/mpi3mr_app.c                      1 ->  0
  sys/dev/pms/.../tdioctl.c                        4 ->  3
  sys/dev/psci/psci.c                              1 ->  0
  sys/dev/qlnx/qlnxe/ecore_mcp.c                   4 ->  3
  sys/dev/sound/pcm/dsp.c                          1 ->  0
  sys/dev/syscons/syscons.c                        6 ->  5
  sys/dev/usb/storage/cfumass.c                    1 ->  0
  sys/dev/vt/vt_core.c                             2 ->  1
  sys/dev/xl/if_xl.c                               1 ->  0
```

Three of the twenty-five moved by more than the finding that led to them:
`ar5211_reset.c` lost four, not one, because the empty-list guard closed
the `lp[0]` and `ep[-1]` reads as well. Three moved not at all and are
absent from the table: `ar2316.c`, `ar2317.c` and `ar5112.c` carry the
same `GetLowerUpperIndex()` hole as the three ath copies that did move,
but the analyser had never reported it in them — the fix went in because
the shape did, not because a finding did. `cfi_core.c` is the fourth: its
`val` finding rests on a softc field, and tightening the attach-time
check does not tell the analyser anything, as expected.

## An ioctl argument indexing a fixed array, in eight drivers

This one did not come from a finding. `rtl_getport()`'s unchecked
`smi_read()` (above) is in `rtl8366rb.c`, and reading the rest of the
file to write that fix turned up `rtl_setvgroup()`:

```c
	sc = device_get_softc(dev);

	g = vg->es_vlangroup;

	sc->vid[g] = vg->es_vid;
```

`sc->vid[]` is `int vid[RTL8366_NUM_VLANS]` — sixteen entries.
`es_vlangroup` is a signed `int` in `etherswitch_vlangroup_t`, which
`IOETHERSWITCHSETVLANGROUP` copies in from userland and
`etherswitch.c`'s `ioctl()` hands to the driver's SETVGROUP method
without looking at:

```c
	case IOETHERSWITCHSETVLANGROUP:
		error = ETHERSWITCH_SETVGROUP(etherswitch,
		    (etherswitch_vlangroup_t *)data);
		break;
```

The bound is each driver's to enforce, and the survey of all of them
found three states:

| driver | GETVGROUP | SETVGROUP | indexed |
|---|---|---|---|
| `rtl8366rb` | none | none | `sc->vid[16]` |
| `ip17x` | none | none | `sc->vlan[16]` |
| `felix` | none | none | `sc->vlans[4096]` |
| `arswitch` | `> es_nvlangroups` | none | `sc->vid[16]` |
| `ar40xx` | `> es_nvlangroups` | none | `vlan_id[64]`, `vlan_ports[]`, `vlan_untagged[]` |
| `e6000sw` | `> num_ports` | `> num_ports` | `sc->vlans[]` |
| `mtkswitch` ×2 | `> es_nvlangroups` | `> es_nvlangroups` | VTIM/VLANI register index |

Three drivers never checked at all, on either side. Two checked the read
and not the write. And every `>` test is wrong twice over: valid groups
are `0 .. es_nvlangroups - 1`, so `>` admits `es_nvlangroups` itself, and
against a signed index it admits every negative value — `es_vlangroup =
-1` reaches `sc->vid[-1]`.

All eight now open both methods with

```c
	if (vg->es_vlangroup < 0 ||
	    vg->es_vlangroup >= sc->info.es_nvlangroups)
		return (EINVAL);
```

(`es_nvlangroups` rather than the array constant on purpose: the
arswitch 8216 and 8226 back-ends set it to 0 because those parts have no
VLAN support, and the same test then refuses every group on them, which
is right. `ip17x`, `felix` and `rtl8366rb` use their array constants
where `es_nvlangroups` is not set before the call.) `e6000sw` already
casts through a `uint32_t`, which turns a negative into a large positive
its test rejects, so only its off-by-one needed correcting.

```
              before   after     (sys/dev/etherswitch)
OK                25      25
ERROR             10      10
findings           1       1
```

The measurement does not move, and cannot: none of the enabled checkers
models an array bound, and the index is unconstrained data from an ioctl
the analyser cannot see the source of. The sweep's contribution here was
to put a person in the file. The number that matters is that the ERROR
set is unchanged — all eight still compile.

## The fs shard: a permission decision made on stack contents

`fuse_internal_getattr()` returns from `fuse_internal_do_getattr()`,
whose only failure path is

```c
	if ((err = fdisp_wait_answ(&fdi))) {
		if (err == ENOENT)
			fuse_internal_vnode_disappear(vp);
		goto out;
	}
```

— straight to `out:`, without touching `*vap`. Nine call sites in the
tree; six check the return. The three that did not:

**`fuse_internal_access()`.** Under `FSESS_DEFAULT_PERMISSIONS` — the
mount option that tells the kernel to do its own permission checking
rather than asking the daemon — this *is* the permission decision:

```c
	if (dataflags & FSESS_DEFAULT_PERMISSIONS) {
		struct vattr va;

		fuse_internal_getattr(vp, &va, cred, td);
		return vaccess(vp->v_type, va.va_mode, va.va_uid,
		    va.va_gid, mode, cred);
	}
```

A failed getattr — an aborted connection, a daemon that returns an
error, a `FUSE_GETATTR` reply the transport drops — leaves `va` as the
stack found it, and `vaccess()` grants or denies on that. A stack
`va_uid` that happens to match the caller's uid grants everything the
owner can do. It now propagates the error, which denies.

**`fuse_internal_clear_suid_on_write()`.** Whether a setuid bit survives
a write by a caller without `PRIV_VFS_RETAINSUGID` was decided by
`va.va_mode & (S_ISUID | S_ISGID)` on the same unwritten struct. It now
acts only on an answer it actually got.

**`fuse_close()`'s atime update**, which ran its `VWRITE` check the same
way. Less consequential — the worst case is an atime write — but the
same shape, and now the same fix.

`tarfs_io_init()` is the fourth in this batch and the plainest: every
exit but one goes through a `bad:` label that frees the `iosize`-sized
block it allocated up front, and the one that does not is the failed
first read.

```
              before   after     (sys/fs/fuse, sys/fs/tarfs)
OK                13      13
ERROR              0       0
findings           5       2
```

`fuse_internal.c` 2 → 0 and `tarfs_io.c` 1 → 0. The two that remain are
in `fuse_vnops.c` and are the RB-tree and dispatch-table classes already
on the record.

### What the rest of the fs shard is

The shard's other 149 findings were read and are classes already on the
record, not defects. The four largest:

**`vpp` and `dd_ino` are independent parameters** (19 findings).
`ufs_lookup_ino()`, `msdosfs_lookup_ino()` and `ext2_lookup_ino()` are
each called once with `vpp == NULL` and a non-NULL `dd_ino`, and each
guards every `*vpp` store with an `if (dd_ino != NULL) return (0);`
above it — three or four such guards per function. The analyser follows
the NULL call in and reports each store, because nothing in the
signature ties the two parameters together; the exclusivity is a caller
contract. Correct as written.

**The allocation condition and the loop bound are the same predicate**
(5 findings). `nfsrv_writedsrpc()` and its four siblings in
`nfs_nfsdport.c` do

```c
	if (mirrorcnt > 1)
		tdrpc = drpc = malloc(sizeof(*drpc) * (mirrorcnt - 1), ...);
	...
	for (i = 0; i < mirrorcnt - 1; i++, tdrpc++)
```

so the loop that would dereference a NULL `tdrpc` cannot run when the
allocation did not happen.

**A `g_*_read_metadata()` out-parameter** (11 findings). Every geom
class's `taste` and `add_disk` fills a stack `struct g_*_metadata` from
`g_read_data()` in another translation unit and then reads its fields.

**Dispatch tables that are not `const`** (task #88's class), which is
what `nfsrvd_dorpc()`'s `nfs_retfh[]` and `nfsrvd_compound()`'s
`nfsv4_opflag[]` findings are.

Two were chased to the bottom and are not bugs. `nfsd_excred()`
dereferences `credanon` where the caller two lines below tests it
against NULL — but `vfs_stdcheckexp()` only returns 0 with a NULL
`credanon` if `netc_anon` is NULL, and both `vfs_hang_addrlist()` paths
`crget()` it before setting `MNT_DEFEXPORTED`, with the clear and the
`crfree()` both under `mnt_explock`, which `vfs_stdcheckexp()` holds
shared. And `sctty_ioctl()`'s `scp->history->vtb_buffer` is reachable
only when `ptr->y + ptr->ysize > scp->ysize`, which the bounds check
above it allows only when `hist_rsz` is non-zero, which requires
`scp->history != NULL`.

`ses_set_enc_status()` and `ses_set_elm_status()` return a `req.result`
their own thread never writes — the SES worker fills it before
`cam_periph_sleep()` returns — and `elm_idx` is bounded by
`scsi_enc.c`'s ioctl layer at all four entry points.

## The rest shard, first read

The `rest` shard — everything under `sys/` that is not `kern`, `dev` or
`fs`: nine architectures, `compat`, `netpfil`, `netgraph`, `crypto` and
two dozen more — had never been triaged at the uninitialised level. A
fresh sweep first, because the stale numbers were three weeks and a
dozen fixes old:

```
              w18      w38      (sys/arm64 .. sys/xdr, 39 scopes)
OK            1554     1554
ERROR           70       70
findings       354      333
```

Twenty-one of the twenty-one findings that went belong to fixes already
committed — `linux_prlimit64`, `linux_signal`, `key.c`, the allwinner
and TI clock leaks, `imx6_ssi`, `vf_sai`.

### The AMD microcode loader returns a stack pointer

`ucode_amd_find()` declares

```c
	const amd_10h_fw_header_t *selected_fw;
	...
	size_t selected_size;
```

and never initialises either. The container walk then does

```c
		if (section_header->type == AMD_10H_MAGIC) {
			if (selected_fw != NULL)
				goto found;
			else
				goto nextfile;
		}
```

on the *first* container, before any assignment, and

```c
found:
	*selected_sizep = selected_size;
	return (selected_fw);
```

returns both whether or not the loop ever assigned them. A truncated or
malformed microcode container — the file comes from the loader — hands
the caller a stack word as the address of the microcode to apply and
another as its length. Both now start NULL and 0, which is the "no
update found" answer the caller already handles.

### Three more of shapes already on the record

`freebsd32_ptrace()`'s `PT_COREDUMP` arm:

```c
	case PT_COREDUMP:
		if (uap->data != sizeof(r32.pc))
			error = EINVAL;
		else
			error = copyin(uap->addr, &r32.pc, uap->data);
		CP(r32.pc, r.pc, pc_fd);
		CP(r32.pc, r.pc, pc_flags);
		r.pc.pc_limit = PAIR32TO64(off_t, r32.pc.pc_limit);
```

`PT_VM_ENTRY` twenty lines above and `PT_SC_REMOTE` ten lines below both
`break` on a failed copyin; this one ran the `CP` macros over the stack
union either way. Nothing downstream sees the result — the switch is
followed by `if (error) return (error);` — so it is undefined behaviour
on a syscall path rather than an exploitable one, but it is the same
guard-on-one-of-a-pair as everything else in this document.

`rk3399_parse_bias()` switches on a device-tree `bank` with cases 0 to 4
and no default, then returns one of two locals the switch was the only
writer of. RK3399 has five GPIO banks, so a sixth is a bad DTB — and the
caller writes what comes back into a pull-up/pull-down register. The
`default:` now returns `-1`, which is already this function's answer for
"no bias" and which the caller tests for.

`ipf_sync_nat()`'s `SMC_CREATE` arm is the subtlest. The state arm above
it does

```c
		bcopy(sp, &sl->sl_hdr, sizeof(struct synchdr));
```

after its `KMALLOC(sl, synclist_t *)`. The NAT arm does not — it sets
`sl_idx`, `sl_ipn` and `sl_num` by hand and leaves the rest of `sl_hdr`
as the allocator found it. `sl_rev` *is* `sl_hdr.sm_rev`, so

```c
		n->nat_rev = sl->sl_rev;
```

takes a NAT entry's direction from unzeroed heap, and the `SMC_UPDATE`
arm below reads the same field again later. It now takes it from the
message, which is what the state arm does through
`ipf_state_insert(softc, is, sp->sm_rev)`.

### And one that is not a bug

`pf_get_transaddr()` picks the pf state-key index with

```c
	switch (nat_action) {
	case PF_NAT:   idx = pd->sidx; break;
	case PF_BINAT: idx = 1;        break;
	case PF_RDR:   idx = pd->didx; break;
	}
	naddr = &ctx->nk->addr[idx];
```

— no default, and `addr[]` and `port[]` are two elements each. Chasing
it: `pf.c` passes `PF_NAT` or `PF_RDR`; `pf_get_translation()` passes
`r->action` after excluding `PF_NONAT`, `PF_NOBINAT` and `PF_NORDR`; and
a rule can only have entered a translation ruleset at all if
`pf_get_ruleset_number()` — which is total — mapped its action to one of
those six. So `idx` is always assigned. The `default: return
(PFRES_MAX);` that is now there is defence in depth for a hardened
kernel, and says so: nothing inside the function establishes the
invariant, and it indexes a two-element array in the packet path.

```
              before   after     (sys/x86, compat/freebsd32,
OK               170     170      arm64/rockchip, netpfil)
ERROR             14      14
findings          79      72
```

```
  sys/arm64/rockchip/rk_pinctrl.c            3 -> 1
  sys/compat/freebsd32/freebsd32_misc.c      1 -> 0
  sys/netpfil/ipfilter/netinet/ip_sync.c     3 -> 2
  sys/netpfil/pf/pf_lb.c                     5 -> 4
  sys/x86/x86/ucode_subr.c                   2 -> 0
```

`rk_pinctrl.c` moved by two, not one: the `default:` closed both
`UndefReturn`s the switch produced.

### What the rest of the shard is

The remaining 138 uninitialised-class findings are classes already
characterised. `ofw_real.c` alone carries eighteen: every OpenFirmware
client call fills a stack `args` struct, maps it, calls the firmware,
and reads the result back with

```c
	memcpy(buf, of_bounce_virt + (physaddr - of_bounce_phys), len);
```

inside `ofw_real_unmap()` — an out-parameter written through a `void *`,
which the analyser cannot follow. `linux_socket.c` carries fifteen of
task #90's `lxs_args_cnt[]` class. `mmu_oea64`'s three `sp_*` walkers
return a `prev` the loop's third clause is the only writer of, and the
loop cannot run zero times for the non-NULL `sp` its callers pass.
`ng_pptpgre_xmit()` reads `gre->hasSeq` as an array index — but
`be32enc(gre, PPTP_INIT_VALUE)` four lines up writes the byte that
bitfield lives in, which the analyser loses through the punned pointer.

The four Atheros-lineage ethernet drivers — `age`, `ale`, `alc`, `mxge`
— set `ip` and `tcp` under `csum_flags & (CSUM_FEATURES | CSUM_TSO)` and
read them under `csum_flags & CSUM_TSO`, a subset, with a
`bus_dmamap_load_mbuf_sg()` and possibly an `m_collapse()` in between.
Worth chasing to the bottom, because a stale pointer there would be a
use-after-free rather than the NULL dereference reported: `m_collapse()`
collapses *towards the front*, keeping `m0` and freeing only the mbufs
after it, and the headers those pointers address were pulled up into
`m0`. Not a bug.

## The libs shard's malloc set: two leaks under 117 false positives

`unix.Malloc` is the largest single bucket anywhere in the sweep — 112
findings in the `libs` shard, plus seven `unix.MallocSizeof`. Reading all
119 produced two fixes, and the ratio is the interesting part.

Thirty-three are in `lib/libc/tests/secure/fortify_*_test.c`, which
deliberately allocates and abandons buffers to check the fortified
string functions trap. Forty-three more are the NSS `get*ent` family —
`getservent`, `getgrent`, `getpwent`, `getrpcent`, `getprotoent` — where
every allocation is handed to `thr_setspecific()` and freed by the
thread's destructor. `ttyname()`, `rpc_call()`, `__rpc_getconfip()` and
`files_getnetgrent_r()` are the same shape one function at a time.
`_citrus_db_factory_add*_by_string()` ×5 pass their allocation to
`_citrus_db_factory_add(df, &r, 1, ...)` — the `1` is `keyfree`, telling
the callee to take ownership. `alloc_segs()` stores its `store` in
`hashp->dir[]`; `nss_load_module()` appends `mod.name` into the module
vector; `_citrus_esdb_get_list()` frees on every error path through
`quit3` and transfers on success. All correct.

### getipv4sourcefilter() frees only when there was something to copy

```c
	if (tmpslist != NULL && *numsrc != 0) {
		pina = slist;
		psu = tmpslist;
		for (i = 0; i < MIN(onumsrc, *numsrc); i++, psu++) {
			...
		}
		free(tmpslist);
	}
```

The `free()` is inside the copy, and the copy is conditional on the
kernel having returned at least one source. A multicast group with no
source filters — the ordinary case — leaks the whole `onumsrc *
sizeof(sockunion_t)` array, and so does every `getsourcefilter()`
failure that leaves `*numsrc` at zero. `setipv4sourcefilter()` fifty
lines above ends with

```c
	if (tmpslist != NULL)
		free(tmpslist);
```

which is the same function's own answer. The copy is now nested inside
an unconditional free.

### __rec_put() and a finding that was pointing at something else

The sweep put a `unix.Malloc` finding on `rec_put.c:150`. Reading the
function to write the fix found a different leak entirely:

```c
			if (F_ISSET(t, R_FIXLEN)) {
				if ((tdata.data = malloc(t->bt_reclen)) == NULL)
					return (RET_ERROR);
				...
			}
			while (nrec > t->bt_nrecs + 1)
				if (__rec_iput(t,
				    t->bt_nrecs, &tdata, 0) != RET_SUCCESS)
					return (RET_ERROR);
			if (F_ISSET(t, R_FIXLEN))
				free(tdata.data);
```

Putting a record past the end of a fixed-length recno database creates
the intervening records from a malloc'd pad buffer. The `free()` is on
the success path only, so a failure part-way through filling the gap
leaks it. That is now freed on the error return too.

The reported finding did not move — it relocated from 150 to 158,
tracking the eight lines I inserted, which is the same line. Chasing it:
the variable it names, `dstvar`, does not exist in `rec_put.c`. It comes
from `include/ssp/strings.h`:

```c
#define _ssp_bcopy(srcvar, src, dstvar, dst, lenvar,  len) __extension__ ({ \
    const void *srcvar = (src);			\
    void *dstvar = (dst);			\
```

— the FORTIFY_SOURCE `memmove` wrapper, expanded over
`memmove(t->bt_rdata.data, data->data, data->size)` seventy lines
earlier, where `t->bt_rdata.data` came from a `reallocf()` the BTREE
owns and `__bt_close()` frees. The finding is an artifact of a statement
expression in a fortification macro. It was still worth having: it is
why anybody read the function.

```
              before   after     (lib/libc/net, lib/libc/db)
OK                72      72
ERROR              8       8
findings          48      47
```

One of the two fixes moves the number. The other fixes a leak the
analyser never saw, at a line where it was reporting something else.

## The dev shard's malloc set: one clock driver and one RAID poll

Thirty-four `unix.Malloc` and `unix.cstring.NullArg` findings in
`sys/dev`, read. Most are the ownership-transfer class this document has
now catalogued four times: `fw_phy_config()`'s xfer goes to
`fw_asyreq()`, whose handler is `fw_asy_callback_free`;
`put_file_offset()`'s and `gntdev_map_grant_ref()`'s allocations go into
RB trees; `bhnd_nvram_val_copy()` returns its `result`;
`kbdmux_init()`'s three keymaps go to `kbd_set_maps()`; `alloc_segs()`
stores into `hashp->dir[]`. `mdstart_malloc()`'s five `NullArg` findings
are the one-predicate-tested-twice class — `dst` is NULL exactly on the
`notmapped` and `vlist` arms, and non-NULL exactly on the `else` that
uses it, but the two tests are in different `switch` statements.

Two files were not that.

### zynqmp_clock.c: three leaks, a fault, and an ignored return

`zynqmp_clk_register()` walks a clock's topology, registering one node
per entry and remembering each node's name to be the next one's parent:

```c
		if (clkname != NULL)
			prev_clock_name = strdup(clkname, M_DEVBUF);
		free(clkname, M_DEVBUF);
```

Nothing frees the previous iteration's `strdup()`. Every topology node
past the first leaks one clock name. After the loop:

```c
	clkdef->clkdef.parent_names[0] = strdup(prev_clock_name, M_DEVBUF);
	clknode = clknode_create(clkdom, &zynqmp_clk_clknode_class, &clkdef->clkdef);
	if (clknode == NULL)
		return (1);
```

— the last `prev_clock_name` is copied and then abandoned, and the
`clknode_create()` failure return leaves both the parent-name array and
the string inside it behind. That last one is the same leak three other
clock drivers carried.

And `strdup(prev_clock_name, ...)` faults outright when the loop
registered nothing: the loop `break`s immediately on a
`CLK_NODE_TYPE_NULL` first entry, and its `default:` arm sets `clkname =
NULL` for a type this driver does not know, so `prev_clock_name` can
still be NULL here. There is no parent to name in that case and nothing
to register, so it now returns.

The `NullArg` in the same file is a fifth, separate defect.
`zynqmp_fw_clk_get_name()` returns non-zero without setting
`clk->clkdef.name` — the firmware refused the query, or handed back an
empty string — and `zynqmp_fw_clk_get_all()` ignored the return:

```c
		zynqmp_fw_clk_get_name(sc, clk, i);
		zynqmp_fw_clk_get_attributes(sc, clk, i);
```

`clk` is `M_ZERO`, so the name stays NULL rather than becoming garbage,
and the clock goes on the list. The registration loop then does
`strcmp(clk->clkdef.name, "dummy")`. The two firmware queries
immediately below this call already `free(clk)` and `continue` on
failure; this one now does too.

### mlx_periodic_eventlog_poll() frees on the wrong condition

```c
 out:
    if (error != 0) {
	if (mc != NULL)
	    mlx_releasecmd(mc);
	if ((result != NULL) && (mc->mc_data != NULL))
	    free(result, M_DEVBUF);
    }
```

Three things wrong in two lines. `mc->mc_data` is assigned only after
`mlx_getslot()` succeeds, so on the `mlx_getslot()` failure path — with
a command that came off the free list with `mc_data` already NULL — the
1024-byte response buffer is not freed. This poll runs periodically.
`mc` is read after `mlx_releasecmd()` has returned it to the free list.
And the line above establishes that `mc` can be NULL here, which the
dereference does not allow for; only C's left-to-right short-circuit on
`result != NULL` — which is also NULL on that path — keeps it from
faulting. `result` is the thing being freed, so `result` is the thing to
test.

```
              before   after     (sys/dev/clk, sys/dev/mlx)
OK                56      56
ERROR              0       0
findings          26      23
```

`zynqmp_clock.c` 2 → 0, `mlx.c` 1 → 0.

## The progs shard, opened: two in bhyve's guest-facing emulation

`bin`, `sbin`, `usr.bin` and `usr.sbin` carry 689 findings — the largest
untriaged pool left, and the only shard where `unix.Malloc` (206) rivals
`core.NullDereference` (220). Starting at the
`core.UndefinedBinaryOperatorResult` set, which is where the real defects
have been in every other shard.

### pci_vtcon_sock_tx()

```c
	int i, ret;
	...
	for (i = 0; i < niov; i++) {
		ret = stream_write(sock->vss_conn_fd, iov[i].iov_base,
		    iov[i].iov_len);
		if (ret <= 0)
			break;
	}

	if (ret <= 0) {
		mevent_delete_close(sock->vss_conn_evp);
		sock->vss_conn_fd = -1;
		sock->vss_open = false;
	}
```

The loop is the only writer of `ret` and the test after it runs whether
or not the loop did. `niov` is the descriptor count on a virtio-console
transmit buffer, so a guest that queues a zero-descriptor buffer makes
bhyve decide from a stack word whether to tear the console connection
down. `ret = 1` at the declaration makes an empty chain a no-op, which
is what it is.

### e82545_transmit()

```c
		if (hdrlen != 0 && iov[0].iov_len > hdrlen &&
		    iov[0].iov_len < hdrlen + 100)
			hdrlen = iov[0].iov_len;
```

`iov[]` is filled by the transmit-descriptor walk, which skips any
descriptor whose length is zero:

```c
		if (len > 0 && iovcnt < I82545_MAX_TXSEGS) {
			iov[iovcnt].iov_base = paddr_guest2host(...);
```

so a guest TX chain whose descriptors are all zero-length leaves
`iovcnt` at 0 and `iov[0]` untouched — and `hdrlen` is non-zero as soon
as the guest asks for a checksum offload, because the arms above set it
from `ckinfo[].ck_off`. The read is of bhyve's stack.

It is not exploitable as it stands, and the reason is worth writing
down: `hdrlen` only ever grows here, and forty lines later

```c
	if (pktlen < hdrlen + vlen) {
		WPRINTF("packet too small for writable header");
		goto done;
	}
```

with `pktlen` also 0 stops the copy loop from acting on it. The guard is
now `iovcnt > 0`, which is what the line means.

### Two copies of an ignored sscanf(), and a finding that meant something else

`StrToPortRange()` exists twice — `sbin/ipfw/nat.c` and
`sbin/natd/natd.c`, byte for byte:

```c
	sscanf (str, "%hu-%hu", &loPort, &hiPort);
	SETLOPORT(*portRange, loPort);
	SETNUMPORTS(*portRange, 0);	/* Error by default */
	if (loPort <= hiPort)
		SETNUMPORTS(*portRange, hiPort - loPort + 1);

	if (GETNUMPORTS(*portRange) == 0)
		errx (EX_DATAERR, "invalid port range %s", str);
```

`sscanf()` assigns fewer than two values for anything that is not two
decimal numbers around a `-` — `"-5"`, `"a-b"`, `"5-"` — and the return
is ignored, so `loPort` and `hiPort` come off the stack. The
`numports == 0` test catches only the subset where the garbage happens
to come out `lo > hi`; anything else silently installs a NAT redirect
over a port range nobody asked for. Both now check, using each file's
own `errx` two lines below.

The three findings in each copy did not move. They are on lines 227 and
235 as well as the one I edited, and all three name `*portRange` — the
caller's out-parameter struct that the `SETLOPORT` macro
read-modify-writes, which is the unconstrained-parameter class. This is
`__rec_put()` again: the sweep put a person in the function, and the
defect there was not the one it named.

```
              before   after     (sbin/ipfw, sbin/natd, usr.sbin/bhyve)
OK               103     103
ERROR              2       2
findings          47      45
```

`pci_e82545.c` 2 → 1 — the surviving one is the `iov->iov_len` read
inside the header-copy loop, which is past the `pktlen` check —
and `pci_virtio_console.c` 1 → 0.

## The same PCI overflow, in the userland twin

Task #63 fixed `pci_ea_fill_info()` in `sys/dev/pci/pci.c`: the PCI
Enhanced Allocation entry-size field `PCIM_EA_ES` is three bits, so a
device can claim up to seven dwords follow an entry header, while
`dw[]` is four. `usr.sbin/pciconf/cap.c` carries the same code:

```c
	uint32_t dw[4];
	...
		ent_size = (val & PCIM_EA_ES);

		for (b = 0; b < ent_size; b++) {
			dw[b] = read_config(fd, &p->pc_sel, ptr, 4);
			ptr += 4;
		}
```

Up to three dwords past the end of a stack array, filled from PCI
configuration space, in a program run as root. The fix is the kernel's,
ported verbatim: read what the device claims so `ptr` still lands on the
next entry, store only what fits, and refuse an entry that does not
carry the two mandatory dwords `base` and `max_offset` are read from.

```
              before   after     (usr.sbin/pciconf)
OK                 3       3
ERROR              0       0
findings           5       3
```

`cap.c` 2 → 0 — both the `dw[0]` and `dw[1]` reads the entry-size guard
now covers.

Found by grepping for the shape after the kernel fix, not by a new
finding. It is the first case in this document where a defect fixed on
one side of the kernel/userland boundary was still live on the other.

### And four shapes in the progs shard that are not defects

`makefs` carries nine `The right operand of '-' is a garbage value`
findings across four files, all from one macro pair:

```c
#define	TIMER_START(x)				\
	if (debug & DEBUG_TIME)			\
		gettimeofday(&(x), NULL)

#define	TIMER_RESULTS(x,d)			\
	if (debug & DEBUG_TIME) {		\
		...				\
		timersub(&end, &(x), &td);	\
```

— the same predicate on both sides, which is the class this document
has now met in `vt_allocate_keyboard()`, `mdstart_malloc()`,
`cfginitmsi()` and `nfsrv_writedsrpc()`.

`usr.sbin/pkg/ecc.c`'s division is on a dead statement: `keysz = MIN(...)`
is immediately overwritten by `keysz = sizeof(keybuf)` on the next line,
so the `cbdata->keylen / 2` it divides is never used.

`chat(8)`'s `StackAddressEscape` follows `arg` — a pointer into
`do_file()`'s local `buf` — into `chat_send()`, and loses that
everything stored past the call goes through `clean()`, which ends in
`dup_mem()` → `malloc`.

`ahci_handle_next_trim()` reads `elba` and `elen` after a loop that can
be skipped, but only when `done > len` on entry, and `len` is
`(guest count) * 512` capped at 512 — always a multiple of the 8 that
`done` advances by, so `done` lands exactly on `len` and the
`done == len` test catches it.

## The twin hunt: grepping for a fix's shape, not for a finding

`pciconf/cap.c` was found by hand — the kernel's `pci_ea_fill_info()`
fix was fresh, and the userland copy of the same parser was the obvious
next place to look. That is worth doing systematically, so it was:
`check_pbsd_marks.py`'s FIXES table already stores, for 380 of its
entries, the exact text a fix *replaced*. 308 of those strings are
long enough to be worth grepping. All 19,799 `.c` and `.h` files in the
tree were searched for each.

Fifteen shapes turned up somewhere else. Most are generic C — a
`free(x)` here, a two-line `if` there — and mean nothing. Two were
precise enough to be the same defect twice.

### `1 << slot` on amd64, five years after i386 was fixed

`sys/i386/pci/pci_cfgreg.c` carries a PBSD fix turning three
`1 << slot` into `1U << slot`. `PCI_SLOTMAX` is 31, and
`pcie_init_badslots()` walks `0..PCI_SLOTMAX`, so `1 << 31` on a signed
`int` is executed on every boot of a PCIe machine — not a corner case,
since slot 31 is where the LPC bridge sits on Intel chipsets.

`sys/amd64/pci/pci_cfgreg.c` is the same file, with the same three
shifts, and had never been touched. The grep found it in the one place
a finding never would: the analyser does not check for signed shift
overflow at all, and no sweep of `sys/amd64` has ever reported this
line.

### mpr and mps: a null command, and the label that would have caught the fault

`mprsas_get_sata_identify()` calls `mpr_wait_command(sc, &cm, ...)` and
then reads `cm->cm_reply` — guarded only by

```c
	/* mprsas_ata_id_timeout does not reset controller */
	KASSERT(cm != NULL, ("%s: surprise command freed", __func__));
```

`KASSERT` compiles to nothing without `INVARIANTS`, which is how the
release kernel is built. And the comment's argument is about the wrong
thing: `mpr_wait_command()` clears `*cmp` whenever `MPR_FLAGS_REALLOCATED`
is set, and that is a *softc* flag set by any reinit that reallocated
the command pool — not a statement about which timeout handler ran.

The same file's `MPI2_EVENT_IR_CONFIGURATION_CHANGE_LIST` handler
already writes it the way it should be written:

```c
	error = mpr_request_polled(sc, &cm);
	if (cm != NULL)
		reply = (Mpi2RaidActionReply_t *)cm->cm_reply;
	if (error || (reply == NULL)) {
```

so the fix is the driver's own idiom, not an invention. `mpr_config.c`
carries it at forty-two more sites.

The first attempt at this fix was wrong, and the way it was wrong is
the interesting part. Guarding the `cm->cm_reply` load alone made the
`error` path `goto out` — and `out:` reads

```c
	if ((cm->cm_flags & MPR_CM_FLAGS_SATA_ID_TIMEOUT) == 0) {
		mpr_free_command(sc, cm);
		free(buffer, M_MPR);
	}
```

unconditionally. With `cm == NULL` that faults one line later than it
used to. A guard that relocates a fault is not a fix. `out:` had to be
split:

```c
	if (cm == NULL) {
		free(buffer, M_MPR);
	} else if ((cm->cm_flags & MPR_CM_FLAGS_SATA_ID_TIMEOUT) == 0) {
		mpr_free_command(sc, cm);
		free(buffer, M_MPR);
	}
```

The `free(buffer, ...)` on the null arm is not symmetry for its own
sake: `buffer` is `malloc()`ed by this function and only ever reachable
through `cm->cm_data`, and nothing on the reinit path frees `cm_data` —
`mpr.c` never calls `free()` on it at all. Dropping the command without
freeing the buffer leaks it.

`sys/dev/mps/mps_sas_lsi.c` is the same function, same defect, same
`out:` label.

### And the finding that was next to it

Putting the two `_sas_lsi.c` files under the analyser for a before/after
turned up something the twin grep could not have found, in the caller:

```
sys/dev/mps/mps_sas_lsi.c:831  [core.UndefinedBinaryOperatorResult]
    The left operand of '&' is a garbage value
```

`mpssas_get_sas_address_for_sata_disk()` declares `Mpi2SataPassthroughReply_t
mpi_reply;` on the stack, passes it to `mpssas_get_sata_identify()`, and
then decides whether to retry from

```c
		ioc_status = le16toh(mpi_reply.IOCStatus) & MPI2_IOCSTATUS_MASK;
		sas_status = mpi_reply.SASStatus;
```

`mpssas_get_sata_identify()` writes `*mpi_reply` only on its success
path, so every error return leaves both reads on stack garbage. The mpr
copy of this function carries `memset(&mpi_reply, 0, sizeof(mpi_reply));`
one line after the `ata_identify` memset it already had. mps does not.
Guard-on-one-of-a-pair again, and this time the pair is two drivers.

```
                mpr+mps+amd64/pci
                before  after
OK                 17      17
ERROR               0       0
findings            3       2
```

The two `_sas_lsi.c` fixes move no number — the analyser never reported
them, because `mpr_wait_command()` is in another translation unit and
its effect on `*cmp` is invisible. They are on this page because the
grep found them and the code says they are real, which is a different
kind of evidence from a finding and worth naming as such.

The two survivors are both `mprsas_scsiio_complete()` / `mpssas_scsiio_complete()`
reading `rep->SCSIStatus` after the function's `if (cm->cm_reply == NULL)`
fast path has already returned. The analyser constrains `rep == NULL`
from an earlier `if (cm->cm_reply != NULL)` test, then loses the
relation across `xpt_freeze_simq()` and friends, so the second test
tells it nothing. That is the "one predicate tested twice across an
intervening call" class this document has met eight times now.

## The dev shard's NullDereference tail, part one: the branch that proved it null

The dev shard was re-swept whole after the UBO and uninitialised work
landed (2633 translation units, 60 ERROR all on the record, 361
findings). What is left is dominated by `core.NullDereference` — 169 of
the 361, spread over 108 files, most of them singletons. Reading them
one at a time turned up a shape that had not been named yet, and it is
the least ambiguous kind of defect there is:

**a pointer dereferenced inside the branch that tested it for NULL.**

Not "may be null on some path". The branch condition *is* `p == NULL`,
and the body reads `p->field`. Such a line is either dead code or a
guaranteed fault; there is no third reading. Six of them, in four
drivers:

```c
	/* sys/dev/ntb/ntb_hw/ntb_hw_amd.c:696 */
	sb = sbuf_new_for_sysctl(NULL, NULL, 4096, req);
	if (sb == NULL)
		return (sb->s_error);
```

The intent is clear — report why the sbuf could not be made — but the
only place that reason is stored is inside the sbuf that does not exist.
`ENOMEM` is what there is to say. Grepping for the shape found it twice
more, in `tool_mw_read_fn()` and `tool_mw_trans_read()` in
`sys/dev/ntb/test/ntb_tool.c`, both reachable from a sysctl.

```c
	/* sys/dev/mpi3mr/mpi3mr_cam.c:1842 */
	if (!target) {
		mpi3mr_dprint(sc, MPI3MR_INFO,
			"Device (persistent_id: %d dev_handle: %d) is already "
			"removed from driver's list\n",
			target->per_id, handle);
```

`handle` is the only identifier that still exists on that path, and it
was already an argument.

And four in one function, `sbp_action()` in `sys/dev/firewire/sbp.c`:

```c
		if (sbp == NULL &&
			ccb->ccb_h.target_id != CAM_TARGET_WILDCARD) {
SBP_DEBUG(0)
			printf("%s:%d:%jx func_code 0x%04x: "
				"Invalid target (no wildcard)\n",
				device_get_nameunit(sbp->fd.dev),
```

`SBP_DEBUG(x)` is `if (debug > x) {`, and `debug = bootverbose`, so
`boot -v` is enough. The function knows perfectly well that `sbp` can be
NULL — it opens with `if (sbp != NULL) SBP_LOCK_ASSERT(sbp);`, the
mapping block is wrapped in `if (sbp != NULL ...)`, and the comment
above the very branch quoted says "sometimes aimed at the SIM (sc is
invalid and target is CAM_TARGET_WILDCARD)". Three more of its printfs
use `sbp->fd.dev` safely, because their func_codes are in the set the
`sdev == NULL` guard returns for, and `sdev != NULL` implies
`sbp != NULL`. Those three are left alone; the four that are not in that
set go through a `sbp_nameunit()` helper that yields `"sbp?"` for a NULL
softc.

### mpi3mr: a reply frame that is not always there

The same file's `mpi3mr_process_op_reply_desc()` has the more
interesting version. It handles three reply-descriptor types:

```c
	U16 ioc_status = MPI3_IOCSTATUS_SUCCESS;
	Mpi3SCSIIOReply_t *scsi_reply = NULL;
	...
	case MPI3_REPLY_DESCRIPT_FLAGS_TYPE_STATUS:
		ioc_status = status_desc->IOCStatus;	/* no reply frame */
		break;
	case MPI3_REPLY_DESCRIPT_FLAGS_TYPE_ADDRESS_REPLY:
		scsi_reply = mpi3mr_get_reply_virt_addr(sc, *reply_dma);
		...
	case MPI3_REPLY_DESCRIPT_FLAGS_TYPE_SUCCESS:
```

A *status* descriptor is the compact form: it carries an IOCStatus and
no frame. Only the *success* descriptor is short-circuited before the
big `switch (ioc_status)` (`if (success_desc) goto out_success;`). A
status descriptor falls straight through, with `scsi_reply == NULL` and
an `ioc_status` the controller chose — and every arm of that switch
except BUSY, INSUFFICIENT_RESOURCES, DEVICE_NOT_THERE and DATA_OVERRUN
reads the frame. Including `case MPI3_IOCSTATUS_SUCCESS`, which is also
the initial value of `ioc_status`.

The third one in the same function is the plain pair shape: the throttle
block does

```c
		if (target) {
			tg = target->throttle_group;
			throttle_enabled_dev = target->io_throttle_enabled;
		}
		if ((data_len_blks >= sc->io_throttle_data_length) &&
		     throttle_enabled_dev) {
			...
		} else if (target->io_divert) {
```

— the `if (target)` that gates `tg` and `throttle_enabled_dev` is the
same `target`, from the same `mpi3mr_find_target_by_per_id()` that
returns NULL for a device already gone from the list.

### And one zero-trip loop that is not the usual false positive

`mmc_wait_for_request()`:

```c
			for (i = 0; i < sc->child_count; i++) {
				ivar = device_get_ivars(sc->child_list[i]);
				if (ivar->rca == sc->last_rca)
					break;
			}
			if (ivar->rca != sc->last_rca)
				return (EINVAL);
```

`ivar` is declared uninitialised. The test after the loop is correct
when the loop ran and found nothing — it is the "no card matched"
answer. When `child_count` is zero the loop never runs and the test
reads through an uninitialised stack pointer instead. `ivar = NULL` plus
`ivar == NULL ||` gives the same EINVAL for the same reason, from a
value that exists.

```
                firewire+mmc+mpi3mr+ntb
                before  after
OK                 35      35
ERROR               0       0
findings           22      10
```

All twelve targeted findings closed. (Three `mmc_calculate_clock`
`CallAndMessage` findings appear at new line numbers on the after side —
the same three findings, moved down nine lines by the comment added
above them.)

### The false positives read on the way, and why each is one

Everything else in this part of the tail is one of the classes already
catalogued, but three earned their reading:

`evdev_mt_push_slot()` looks like the pair shape — it writes
`mt != NULL && mt->type_a` in one test and bare `mt->type_a` in the
next. It is not. `evdev->ev_mt` is set by `evdev_mt_init()`, which
`evdev_register()` calls whenever `ABS_MT_SLOT` is supported **or**
`EVDEV_FLAG_MT_TRACK` is set (`evdev.c:315`), and the second test is
under `EVDEV_FLAG_MT_TRACK`. The first test is reached without either.

`probe_adapters()` in `sys/dev/fb/vga.c` indexes `mp[1]` in two switch
arms after `if (mp != NULL)` guards only the `bcopy`. The switch is on
`comp_adpregs(adpstate.regs, mp)`, whose first statement is
`if ((buf1 == NULL) || (buf2 == NULL)) return COMP_DIFFERENT;` — so
`mp == NULL` reaches only the `default` arm. The analyser is
interprocedural within a translation unit but budget-limited, and
`comp_adpregs()` carries a 64-entry static table and a loop.

`netmap_monitor_del()` guards `if (kring != NULL)` before
`nm_kr_stop()`, then uses `kring->tx` unguarded in the copy-monitor
branch. The guard exists for the *zmon* branch, where `kring` is
reassigned from `nm_zmon_list_head()`; on the copy path it is still the
caller's argument, and the one caller passes `NMR(pna, s)[i]` under
`if (pna == NULL) continue;`. A guard placed for one branch reading as
a warning about both is a legitimate complaint about the code, but it
is not a defect.

## An instrument for the shape: tools/verify/null_branch.py

The dereference-inside-the-null-branch shape is worth a pass of its own,
for the reason that makes it unambiguous in the first place: it needs no
premise about reachability. `if (p == NULL) { ... p->x ... }` is wrong
on inspection. That makes it findable by a grep with a brace matcher,
over the whole tree, in a second — including the files the clang sweep
cannot build, which is most of what a sweep's ERROR column is.

The sweep did find the shape; it is where it was noticed. But the second
copy in `ntb_tool.c` came from a grep, not a finding, and the
`sbp_action()` site the analyser reported was one of four.

Writing it took four rounds of running it against the tree and reading
what came back, and each round was a real false-positive class rather
than a tuning knob:

| round | tree-wide | what the round taught |
|---|---|---|
| 1 | 81 | comments quoting the bug are not the bug; `p = malloc()` at line start is a reassignment |
| 2 | 48 | `if ((p = malloc(n)) == NULL)` assigns `p` from inside a condition, and `sizeof(p[0])` names a type |
| 3 | 3 | C macros assign through bare arguments — `TAILQ_FOREACH(p, ...)`, `ELM_MALLOC(p, ...)`, `sctp_alloc_a_chunk(stcb, p)` — and that assignment is not in the file's text at all |
| 4 | 0 | `&p` passed to anything is the out-parameter idiom; `#if 0` bodies are not shipped; and a *re-test* of `p` inside the block re-establishes the guard (`if (p == NULL \|\| p->x > y)` is safe by short circuit) |

Round 4's last case was the only survivor of round 3 that was not a
mechanism the lint could not see: `spa_raidz_expand_get_stats()` in
vdev_raidz.c re-tests `vre == NULL ||` inside the block before reading
`vre->vre_end_time`. Round 3 also found one in `#if 0` — an
`ip_fw_dynamic.c` log line that would fault the moment anyone re-enabled
the block it sits in. It is disabled, so it is not a defect, and it is
recorded here rather than fixed.

Calibrated both ways: five sites on the tree before the fixes in this
document, zero after, across `sys`, `lib`, `bin`, `sbin`, `usr.bin`,
`usr.sbin`, `libexec`, `stand` and `share` — 20,324 files. Zero is what
lets it be a gate rather than a report, so it runs in `pbsd-ci.yml` with
`--gate` beside `nowait_check.py` and `masked_switch_check.py`.

It deliberately claims only the narrow half. The other three
`sbp_action()` sites are not of this shape: there `sbp` is merely
*reachable* as NULL rather than proven NULL by the branch, and
establishing that took reading the function and its callers. No grep
was going to do that, and the lint does not pretend otherwise.

## sys/dev/pms: an assertion macro that asserts nothing

The PMC-Sierra RefTisa driver writes its preconditions like this, and
does it dozens of times:

```c
	SA_ASSERT(NULL != circularQ, "circularQ argument cannot be null");
	SA_ASSERT(NULL != messagePtr, "messagePtr argument cannot be null");
	SA_ASSERT(0 != circularQ->numElements, "The number of elements ...");
```

`SA_ASSERT`, `SM_ASSERT` and `DM_ASSERT` are all `OS_ASSERT`, and
`OS_ASSERT` in the shipping build is:

```c
#define OS_ASSERT(expr, message)                                  \
do {                                                              \
          if (!(expr))                                            \
          {                                                       \
            printf("ASSERT: %s", message);                        \
            printf(" - file %s, line %d\n", __FILE__, __LINE__);  \
          }                                                       \
} while (0)
```

It prints and returns. So the third line above dereferences the pointer
the first line just announced was NULL — one line later, after saying so
out loud. The assertion detects the internal error and does not stop it.
(The `AGTIAPI_KDB_ENABLE` arm calls `BUG_ON` and `KDB_ENTER`, but that
arm is Linux-only: it `#include <linux/kdb.h>`.)

This is the whole reason eleven of the shard's `core.NullDereference`
findings exist, in seven files across all four of the driver's layers —
`dminit.c`, `sminit.c`, `sainit.c`, `mpi.c` (three), `sampirsp.c` (two),
`saport.c` (two), `itdio.c`. The analyser is right about the code and
would be right about a NULL pointer; whether any of these pointers is
ever actually NULL is a separate question the assertions were supposed
to answer and cannot.

The fix is at the macro:

```c
            printf(" - file %s, line %d\n", __FILE__, __LINE__);   \
            KASSERT(0, ("%s: %s", __func__, message));             \
```

A kernel with INVARIANTS now stops at the assertion, with the driver's
own message, instead of faulting one line later with no context. A
kernel without INVARIANTS is byte-for-byte what it was, print included.
Making this halt a production kernel would turn every benign assertion
in a third-party driver into a panic, and that is not this change's to
make — so it does not, and the fall-through in a non-INVARIANTS build is
still there. What has changed is that it is now findable.

### Three in the same driver that are not the macro

`smsatSetFeaturesAACB()` and `smsatSetFeaturesVolatileWriteCacheCB()`
both open with

```c
    if (agFirstDword == agNULL && agIOStatus != OSSA_IO_SUCCESS)
      SM_DBG1(("...: fail, case 1 agFirstDword is NULL when error ..."));
```

— contemplating a NULL `agFirstDword`, but only when the status is *not*
success — and then read the response frame under

```c
    if (agIOInfoLen != 0 && agIOStatus == OSSA_IO_SUCCESS)
      statDevToHostFisHeader = (agsaFisRegD2HHeader_t *)&(agFirstDword->D2H);
      ataStatus = statDevToHostFisHeader->status;
```

which is exactly the case the warning does not cover. `agFirstDword` is
now in the guard, which is what `smsatPassthroughCB()` in the same file
already does on its own success path (`/* prcessing the success case */
if(agFirstDword != NULL)`).

`smsatIDStartCB()`'s internal-IO arm:

```c
    satOrgIOContext = satIOContext->satOrgIOContext;
    if (satOrgIOContext == agNULL)
    {
      SM_DBG5(("smsatIDStartCB: satOrgIOContext is NULL\n"));
    }
    else
    {
      smOrgIORequestBody = (smIORequestBody_t *)satOrgIOContext->smRequestBody;
      if (smOrgIORequestBody == agNULL)
      {
        ... free, return;
      }
    }
    ...
  smIORequest = smOrgIORequestBody->smIORequest;
```

The outer arm prints and falls through, leaving `smOrgIORequestBody` at
its `= agNULL` initialiser for the read at the bottom. The nested arm
three lines down already unwinds exactly the way this one needs to, so
that is what it now does.

```
                sys/dev/pms
                before  after
OK                 48      48
ERROR              10      10
findings           30      16
```

Fourteen closed, none new. Eleven of them by the macro alone.

### The four survivors that are worth naming

`itdcb.c` reports the same shape four times —
`tiDeviceHandle = tdIORequestBody->tiDevHandle;` then
`tiDeviceHandle->tdData` on the next line. `tiDeviceHandle` is declared
`= agNULL` and that initialiser is the analyser's whole basis; the value
read is a struct field it cannot constrain. Unconstrained-field class.

`smEnqueueIO()` and `dmCleanAllExp()` are the "tested and only warned"
shape without a macro:

```c
    if (smIORequestBody->satIoBodyLink.blink == agNULL)
    {
      SM_DBG1(("smEnqueueIO: internal command!!!, io ID %d, blink is NULL!!!\n", ...));
    }
    ...
    SMLIST_DEQUEUE_THIS(&(smIORequestBody->satIoBodyLink));
```

— and the test is labelled `/* debugging only */` by its author. These
are left alone deliberately. The correct fix is a `return`, and a
`return` here skips the dequeue, which leaks the entry off the in-use
list; picking the right unwind in a vendor driver on a path its author
documents as impossible is guessing, and guessing is worse than
recording.

## The lint learns to read aliases, and finds seven more

`null_branch.py` shipped reporting zero across the tree. Reading the
`sys/dev` findings by hand then turned up `sk_txcksum()` and four
`ecore_*` sites of exactly the shape it exists for — and it had found
none of them. Each miss was a rule that was too narrow, and each is now
closed:

**A pointer aliased to the tested one is the same pointer.**

```c
	struct ecore_hwfn *p_hwfn = (struct ecore_hwfn *)rdma_cxt;
	...
	if (!rdma_cxt) {
		DP_ERR(p_hwfn->p_dev, "destroy ud qp failed due to NULL rdma_cxt\n");
```

`DP_ERR(p_dev, ...)` expands to `(p_dev)->dp_ctx` and `(p_dev)->name`,
so this reports a NULL `rdma_cxt` by dereferencing it — twice. The pass
was reading the test and the dereference as being about different
pointers. It now tracks `T *alias = (T *)base;` and `T *alias = base;`,
resets the map at every column-0 `}`, drops a pair the moment either
name is assigned again, and does not treat `T *p = NULL;` as an alias.
All three of those refinements were false positives the tree handed
back on the next run.

**A disjunction proves it too.** The pass accepted only `&&` chains, on
the reasoning that `if (A && p == NULL)` proves `p` NULL in the body.
But `if (A || p == NULL)` means the body is reached when *any* operand
held — so there is a path into it on which `p` was NULL, and a
dereference there faults on that path. Exactly as certain; three of the
four ecore sites are written that way. (A test in the *condition* is a
different matter — `if (p == NULL || p->x > y)` is safe by short
circuit — and never reached this rule, because only the body is
scanned.)

**And three narrower ones**, each from a real hit: an assignment whose
value is on the next line (`AcpiGbl_CommentAddrListHead =\n    AcpiOs...`)
is still an assignment; a re-test may be written on any name in the
alias group (`sli_res_sli_config()` tests `buf ?` to guard a read of
`sli_config`, which is `buf`); and the ternary `p ? p->x : NULL` that
`sys/dev/ocs_fc` writes at two dozen sites is a re-test like any other.

Tree-wide, across the same 20,324 files: 81 → 48 → 3 → 0 in the first
pass, then 30 → 11 → 9 → **2** as the alias and disjunction rules went
in. The two survivors were both real.

### The seven

`sk_txcksum()` — a `goto` into a label that dereferences it:

```c
	if (m == NULL) {
		offset = sizeof(struct ip) + ETHER_HDR_LEN;
		goto sendit;
	}
	...
sendit:
	f->sk_csum_start = htole32(((offset + m->m_pkthdr.csum_data) & 0xffff) | ...);
```

Two of the three `goto sendit` sites reach the label with `m` NULL, and
one of them tests for exactly that and nothing else. It is also wrong on
the path that does *not* fault: `m` has been walked forward off the head
of the chain by then, and `m_pkthdr` only exists on the head. Both are
fixed by taking `csum_data` from the head once, before the walk.

(The local could not be called `csum_data`: that is itself a macro for
`PH_per.thirtytwo[1]`, so the declaration expanded into nonsense. The
sweep's `--check-errors` gate caught it as an ERROR on the after side —
which is the whole reason that gate exists, since a file that does not
compile reports zero findings and reads as clean.)

The four `ecore_*` sites above. `hvsock_canread_check()`:

```c
	if (pcb == NULL || pcb->chan == NULL) {
		pcb->so->so_error = EIO;
```

And `sdp_get_lcaddr()` in libsdp, which is the worst of them because
there was no way out:

```c
	if (l == NULL || ss == NULL || ss->flags & SDP_SESSION_LOCAL) {
		ss->error = EINVAL;
		goto fail;
	}
	...
fail:
	return ((ss->error == 0) ? 0 : -1);
```

A NULL session faults in the body, and would fault again at the label,
and again in the return. `sdp_error()` eight lines above gets it right
as a ternary, and `service.c` and `search.c` open with the standalone
`if (ss == NULL) return (-1);` this now uses.

```
                sk+qlnx+hyperv+libsdp
                before  after
OK                 62      62
ERROR               0       0
findings           17      11
```

Seven closed. The one addition is `hvsock_open_channel()`'s existing
finding at a line number eight further down, moved by the comment added
above it.

### What it still does not claim

`ecore_rdma_destroy_qp()` writes `DP_ERR(p_hwfn, ...)` — the pointer
passed bare, with the dereference inside the macro. The pass suppresses
a block that passes the name bare to anything, because that is how
`TAILQ_FOREACH(p, ...)` and `ELM_MALLOC(p, ...)` assign it, and the
assignment is not in the file's text. Keeping that rule costs this one
site. It was found by reading, and the trade is the right way round.

`sk_txcksum()` is missed for a different reason: the dereference is at a
`goto` label, not in the branch. Following gotos is a control-flow
graph, not a brace matcher, and the moment this pass needs one it stops
being the thing it is — a grep that runs over the whole tree in a second
and is right every time it speaks.

## Two more of the shape, and the parenthesis that hid one

Finishing the dev shard's `core.NullDereference` tail by hand turned up
two more, and one of them the lint had been standing right next to.

`tws_intr()`:

```c
	struct tws_softc *sc = (struct tws_softc *)arg;
	...
	if (!(sc)) {
		device_printf(sc->tws_dev, "null softc!!!\n");
		return;
	}
```

The pass tests for `!p`, and this is `!(p)`. One parenthesis. That is
the honest character of a lint like this: it is exactly as good as the
forms it happens to have been shown, and the tree is where you find out
which those are. `!(p)` and `(p) == NULL` are both accepted now. With no
softc there is no device to name the message with, so it goes out on
plain `printf`.

`wdatwd_action()` is the one worth reading twice:

```c
static int
wdatwd_action(const struct wdatwd_softc *sc, const u_int action,
    const uint64_t val, uint64_t *ret)
{
	STAILQ_FOREACH(wdat, &sc->action[action], next) {
		switch (wdat->entry.Instruction & ~ACPI_WDAT_PRESERVE_REGISTER) {
		    case ACPI_WDAT_READ_VALUE:
			...
			*ret = (x == wdat->entry.Value) ? 1 : 0;
```

Six of its nine callers pass `ret = NULL` —
`ACPI_WDAT_RESET`, `SET_COUNTDOWN`, `SET_RUNNING_STATE`,
`SET_STOPPED_STATE`, `SET_STATUS`, `SET_REBOOT`. Which instructions an
action runs is not the driver's decision: it is read from the firmware's
WDAT table, and the table is a flat list of (action, instruction)
entries whose sequences the specification leaves to the vendor. A
firmware that lists a `READ_VALUE` under `SET_RUNNING_STATE` — legal,
and nothing in this driver rejects it — makes the kernel write to NULL
from a watchdog action. `if (ret != NULL)` is all it takes; the caller
that did not ask for a value does not get one.

```
                tws+wdatwd
                before  after
OK                  6       6
ERROR               0       0
findings            5       2
```

Three closed. The addition at `wdatwd.c:373` is the existing
`wdatwd_event()` `CallAndMessage` finding, ten lines lower.

The survivor, `tws_q_remove_head()`, is on the record as read:

```c
	if (r->next == NULL && r->prev == NULL) {
		/* last element  */
		sc->q_head[q_type] = sc->q_tail[q_type] = NULL;
	} else {
		sc->q_head[q_type] = r->next;
		r->next->prev = NULL;
```

Taking the `else` with `r->next == NULL` faults, and the condition
admits it — but only if `r->prev != NULL`, and `r` is the head, whose
`prev` is NULL in any well-formed queue. The second conjunct is
redundant, and its redundancy is the whole finding. Not a defect; a test
that says less than it means.

## The gate that reported clean, and the six it could not see

`nowait_check.py` runs in CI with `--gate` and reported **0 M_NOWAIT
allocations used without a NULL check**. `sys/dev/wtap` had five, in
four functions, and the sweep had reported them as
`core.NullDereference` in the same shard this document has been working
through.

Three things sit between `=` and the allocator, and that directory had
all three at once:

```c
	hal->hal_devs[id] = (struct wtap_softc *)malloc(       /* a CAST, and */
	    sizeof(struct wtap_softc), M_WTAP, M_NOWAIT | M_ZERO);
	hal->hal_devs[id]->sc_md = hal->hal_md;                /* a member PATH */

	struct eventhandler *eh = (struct eventhandler *)
	    malloc(sizeof(struct eventhandler), M_WTAP, M_NOWAIT | M_ZERO);
	eh->tq = taskqueue_create(...);                        /* and a LINE BREAK */

	plugin = (struct visibility_plugin *)malloc
	    (sizeof(struct visibility_plugin), M_WTAP_PLUGIN,   /* the break even
	    M_NOWAIT | M_ZERO);                                    between malloc
	plugin->base.wp_hal  = hal;                                and its ( */
```

The pattern wanted `var = fn(` with `var` a bare identifier. A cast
between the two, a member path on the left, and the allocator on the
next line each broke it independently. All three are handled now — the
line break by searching a two-line window when the current line ends
mid-assignment or on an allocator name.

Widening it turned up a sixth the analyser sweep had never reported at
all:

```c
	/* ata_promise_queue_hpkt(), with hpktp->mtx held */
	struct host_packet *hp = 
	    malloc(sizeof(struct host_packet), M_TEMP, M_NOWAIT | M_ZERO);
	hp->addr = hpkt;
	TAILQ_INSERT_TAIL(&hpktp->queue, hp, chain);
```

### The fix depends on whether a lock is held

Four of the six are in sleepable context — the `MOD_LOAD` handler
(`event_handler`, twice), `init_hal()` and `init_medium()` reached from
it, and `new_wtap()` from the `wtapctl` cdev ioctl. None holds a lock,
so `M_WAITOK` is available, cannot fail, and removes the question rather
than answering it.

Two hold a mutex and cannot sleep: `medium_transmit()` under `md_mtx`
and `ata_promise_queue_hpkt()` under `hpktp->mtx`. Those stay `M_NOWAIT`
and get the check. `medium_transmit()` drops the frame, which is exactly
what its own `md->open == 0` arm does four lines above; the ATA one
drops one queued host packet, which loses a command where dereferencing
NULL under a lock loses the machine.

```
                wtap+ata
                before  after
OK                 32      32
ERROR               0       0
findings            8       3
```

Five closed, none new. The sixth does not appear in that delta because
it was never in the before set: no sweep has ever reported
`ata-promise.c:1237`. That is the argument for having both instruments.
The sweep found the shape in `wtap` and could not build or reach the ATA
file; the lint could not see the shape at all until it was widened, and
then found it in a second.

## A third instrument: FuSeBMC's method, on CBMC and AFL++

FuSeBMC (Alshmrany et al., SV-COMP) is a hybrid — a bounded model
checker produces counterexample inputs, a fuzzer takes them as "smart
seeds", and coverage the fuzzer finds comes back as new goals. The two
halves cover each other: BMC reasons about every path to a shallow
depth, fuzzing reaches deep paths it can stumble into and proves
nothing.

The published FuSeBMC is ESBMC + Map2Check + AFL, distributed from
GitHub. This container's network policy scopes GitHub to one owner and
ESBMC is in no distribution's package set, so neither is reachable here.
What is here is CBMC 5.95.1 with `goto-cc` — the model checker this tree
already uses — and AFL++ 4.09c from the archive. That is the same
architecture with a different BMC engine, so `tools/verify/fusebmc.py`
is the method, not the program, and is named for the method.

**It proves nothing.** CBMC with `--unwind K` and
`--unwinding-assertions` can prove a bounded property, and
`cbmc_driver.py` says PROVED when it has. A fuzzer that finds nothing
has told you that it found nothing. The status vocabulary keeps them
apart: CRASH, CLEAN ("found nothing in this budget; not a proof"),
NOSEED, ERROR, NOFUNC, TIMEOUT.

Its reach is a strict subset of CBMC's, and CBMC's is 749 of 4737
translation units. CBMC needs to *parse* a unit; this needs to compile,
link and **run** it. That ratio is the result.

### Three ways this reported CLEAN on code it had not tested

Every one was found by planting a bug and watching the engine miss it,
and every one would have made the tool a liar.

**1. The seed that crashes stops the fuzzer.** The calibration target
was an out-of-bounds array read. CBMC found it and handed over a
counterexample — and AFL++ refuses to start when every seed crashes
("We need at least one valid input seed that does not crash!"). The
first version read that abort as CLEAN. A fuzzer that would not start
and a fuzzer that found nothing are not the same answer. The engine now
*replays the BMC seed first*: if it dies, that is the finding, recorded
as `found_by: bmc-seed`, and no fuzzing is needed. That is not a
workaround — it is the point of the hybrid, and it turned a 20-second
budget into 0.27 seconds.

**2. `-fno-builtin`, or you are testing the compiler.** `abs()` is a
compiler builtin. Without `-fno-builtin`, clang recognised the call in
the generated harness, lowered it to `llvm.abs` with
`is_int_min_poison=false`, and **never called the definition in
`abs.c`**. The engine ran a full budget against clang's implementation
and reported CLEAN on a function it had not executed. The same holds for
`memcpy`, `strlen`, `memset` and every other libc name clang knows —
which is most of what a userland harness would ever point at.

**3. A discarded result lets the optimiser delete the call.** The
harness and the source are compiled as one translation unit, so LLVM can
see the function is pure; with the result thrown away, the call goes.
The result now lands in a `static volatile` sink of the function's own
return type, which meant parsing the return type as well as the
parameters.

And one that made the seed worthless rather than absent: CBMC prints
`j=-2147483648 (10000000 ...)`, and the first packer truncated every
counterexample value to one byte — discarding exactly the extreme values
a counterexample is made of. Seeds are now packed at each parameter's
real width, at the same offset the harness reads it from, from a
`layout()` the two share.

### What it found

```
                lib/libc scalar functions
CLEAN      5     ffs, ffsl, fls, flsl, flsll   (20s budget each, real runs)
CRASH      1     abs
```

`abs(INT_MIN)` is `-j` on `INT_MIN` — signed overflow, undefined
behaviour, and UBSan says so at `abs.c:37`. CBMC found the input, the
replay confirmed it in a quarter of a second.

It is **not a defect in `abs.c`**. C17 7.22.6.1p2 says outright that if
the result cannot be represented the behaviour is undefined; the value
is the caller's to keep in range, and every libc on earth is written
this way. Recording it as a finding would be the same mistake the
`--conversion-check` tier exists to avoid.

So the honest summary of this engine's first run is: it reproduced a
known-by-construction property of the C standard, and it did so through
four layers that each had to be right. That is what a calibrated
instrument looks like before it is pointed at anything.

### The harness's limit, stated rather than papered over

A parameter that is a pointer is not synthesised. `cbmc_driver.py` runs
those under an explicit stated precondition (`--min-null-tree-depth`)
and records the assumption in the result. There is no honest equivalent
for a fuzzer: passing NULL reports the absence of a caller's contract as
a crash, and passing a buffer of an invented size reports the invention.
Those functions come back ERROR with that sentence as the reason, which
is true and is the number that matters.

## sort_modes(): a monitor whose EDID decodes to nothing

`sys/dev/videomode/pickmode.c`, the "no preferred mode" arm:

```c
	struct videomode *mtemp = NULL;
	...
		hbest = 0;
		vbest = 0;
		for (i = 0; i < nmodes; i++) {
			if (modes[i].hdisplay > hbest) {
				hbest = modes[i].hdisplay;
				vbest = modes[i].vdisplay;
				mtemp = &modes[i];
			} else if (...) { ... }
		}
		aspect = mtemp->hdisplay * 100 / mtemp->vdisplay;
```

The loop runs — `if (nmodes < 2) return;` is above it — but `mtemp` is
assigned only by a mode whose `hdisplay` is greater than `hbest`, and
`hbest` starts at zero with a strict `>`. A list in which every
`hdisplay` decodes to zero never assigns it, and the next line reads
through the `NULL` initialiser.

These modes come from a parsed EDID, which is what the monitor says
about itself. There is also nothing this arm could compute from such a
list even if `mtemp` were set: the very next expression divides by
`vdisplay`. So the guard returns rather than substituting a mode.

```
                sys/dev/videomode
                before  after
findings            2       1
```


## Closing the dev shard

The shard was swept whole at the start of this work and again at the
end, on the same scope with `--check-errors` passing both times.

```
                sys/dev, whole shard
                        before   after
OK                        2573    2573
ERROR                       60      60
findings                   361     320

  core.NullDereference       169     129   -40
  core.CallAndMessage         65      65
  core.DivideZero             31      31
  core.UndefinedBinaryOperatorResult
                              31      30    -1
  core.uninitialized.Assign   24      24
  unix.cstring.NullArg        14      14
  unix.Malloc                 14      14
  core.uninitialized.Branch    7       7
  core.uninitialized.UndefReturn
                               3       3
  core.uninitialized.ArraySubscript
                               2       2
  core.VLASize                 1       1
```

The ERROR set is unchanged, which is the measurement that says the
edited files still compile. The 60 are on the record in
`expected_errors.py` and every one is accounted for by the sweep's own
NOT_BUILT report.

All 167 `core.NullDereference` findings were read. Forty closed; the
`UndefinedBinaryOperatorResult` that went with them is the `mps`
`memset`. What is left is 129 across 86 files, and the concentrations
are the classes this document has been naming:

```
   16  pms      the RefTisa assertion sites the macro fix did not
                cover, plus itdcb.c's unconstrained struct field
   11  usb      the chain-building loops the analyser does not unroll
    9  mlx4     `pd' as an unexported function's parameter
    6  cxgbe    the same, `vi'
    6  irdma    out-parameters written by a callee in another TU
    6  sound    zero-trip loops over static tables with a sentinel
    5  syscons  a switch over two mask bits with all four arms, which
                the constraint solver cannot see is exhaustive
    4  xen      RB_INSERT_COLOR macro expansions
```

None of these is a defect on the reading each was given, and each
reading is in this document rather than in a suppression list. That
distinction is the whole point: a finding that has been read and
explained is closed; a finding that has been silenced is a finding
nobody will ever look at again.

## Following the goto, and why that pass cannot gate

`sk_txcksum()` and the `mpr` `out:` label were both cases where the
dereference sits at a `goto` target rather than in the guarded block,
and the first version of `null_branch.py` recorded that as a limitation
it would not chase — "the moment this pass needs a control-flow graph it
stops being the thing it is."

That was too pessimistic by one step. The argument the disjunction rule
rests on carries straight through a jump: a `goto L` out of a block that
proved `p` NULL **arrives at L with p NULL**, so a dereference at L
faults on that path whatever other paths also reach L. Nothing needs to
be known about those other paths, so no graph is needed — only the
label's own text, found by name within the enclosing function.

It found two more, in code the analyser sweep reports nothing about:

```c
	/* isp_freebsd.c, the target-mode SRR handler */
	ccb = atp->srr_ccb;
	atp->srr_ccb = NULL;
	if (ccb == NULL) {
		isp_prt(isp, ISP_LOGWARN, "SRR[0x%x] null ccb", atp->tag);
		goto fail;
	}
	...
fail:
	inot->in_reserved = 1;
	isp_async(isp, ISPASYNC_TARGET_NOTIFY_ACK, inot);
	ccb->ccb_h.status &= ~CAM_STATUS_MASK;
```

It logs that the ccb is null and then faults on it three lines later.
The notify-ack is the part that has to happen either way; there is no
ccb to complete when there was no ccb.

`netmap_mem_pt_guest_create()` is the quieter one. Its allocation
failure jumps to a label calling
`netmap_mem_pt_guest_delete(&ptnmd->up)`, and that callee *does* test
its argument for NULL — so it never faults. But forming `&ptnmd->up` on
a null `ptnmd` is undefined (C17 6.5.3.2), and it arrives at the guard
as NULL only because `up` happens to be the first member of the struct.
Nothing states that invariant. The allocation-failure path has nothing
to delete, so it returns.

### And then it would not go to zero

Six sites remain, and none is fixable by a grep:

| site | why it is not a defect |
|---|---|
| `scandir-compat11.c`, `efi_variables.c` | the label's loop is bounded by a counter that is zero on exactly the path where the pointer is null |
| `mpi3mr.c`, `sym_hipd.c` | the guard at the label is on a *different* variable, non-null only when this one is (`sense_buf`/`scsi_reply`, `vaddr`/`vbp`) |
| `bsd_nvpair.c` | the null test is a redundant disjunct that an early return already excluded |
| `stand/libsa/nfs.c` | the label and the dereference are in different `#if` arms |

Each needs to know what a counter holds, or which of two variables
implies the other, or what the preprocessor kept. A pass that needs any
of those is not this pass.

So the tool now has two modes and refuses to combine them. The default
reads the guarded block, reaches zero across 20,324 files, and gates CI.
`--follow-goto` reads the labels too, finds what the block alone cannot,
and **reports** — because a lint with a false-positive floor cannot
carry a build, which is the same conclusion `nowait_check.py`'s
docstring reached about itself.

```
                isp+netmap
                before  after
OK                 19      19
ERROR               0       0
findings            5       5
```

Zero delta, and that is the honest number: the sweep never reported
either site. Like `ata-promise.c`, they exist in this document because a
second instrument found what the first could not see, and the unchanged
OK/ERROR columns are what says the edited files still compile.

## The progs shard, and a command mailbox filled from the stack

```
                bin + sbin + usr.bin + usr.sbin
OK                      1822
ERROR                     40   (all on the record)
findings                 679

  core.NullDereference            220
  unix.Malloc                     206
  core.CallAndMessage              73
  core.UndefinedBinaryOperatorResult
                                   71
  unix.cstring.NullArg             49
  core.uninitialized.Assign        22
  core.DivideZero                  21
  unix.MallocSizeof                12
  (the rest)                        5
```

### mlxcontrol: four commands built on uninitialised stack

`usr.sbin/mlxcontrol/interface.c` has four functions of one shape, and
the sweep reports each of them:

```c
int
mlx_enquiry(int unit, struct mlx_enquiry2 *enq)
{
    struct mlx_usercommand	cmd;

    /* build the command */
    cmd.mu_datasize = sizeof(*enq);
    cmd.mu_buf = enq;
    cmd.mu_bufptr = 8;
    cmd.mu_command[0] = MLX_CMD_ENQUIRY2;

    mlx_perform(unit, mlx_command, (void *)&cmd);

    return(cmd.mu_status != 0);
}
```

`mlx_perform()` is:

```c
    if ((fd = open(ctrlrpath(unit), 0)) >= 0) {
	func(fd, arg);
	close(fd);
    }
```

with no `else`. When the control device cannot be opened — no controller
present, or not running as root — the command function is never called,
`cmd` is never written, and `cmd.mu_status != 0` is a read of stack
garbage. The caller learns whether the command succeeded from whatever
happened to be on the stack.

The second half is worse and the analyser cannot see it at all. `cmd`
goes to the driver whole, through `_IOWR('M', 4, struct
mlx_usercommand)`, and the struct is:

```c
    u_int16_t	mu_status;	/* command status returned */
    u_int8_t	mu_command[16];	/* command mailbox contents */
    int		mu_error;
```

No caller here sets more than three of those sixteen mailbox bytes —
`mlx_get_device_state()` sets `[0]`, `[2]` and `[3]`, leaving `[1]` and
`[4]` through `[15]` as stack — and `mlx_scsi_inquiry()` never sets
`mu_bufptr` at all, which is the offset at which the driver places the
data buffer address. A DAC960 controller is handed a command mailbox
built partly from this process's stack.

All four now `bzero(&cmd, sizeof(cmd))` and set `mu_status = 0xffff`, so
"the command did not run" reports as failure rather than as whatever the
stack held.

```
                usr.sbin/mlxcontrol
                before  after
OK                  4       4
ERROR               0       0
findings            4       0
```

### And 36 findings that are one guard's exemption

`ctladm` has 60 findings, a tenth of the shard, and 36 of them are four
XML end-element callbacks writing the same guard:

```c
	cur_conn = islist->cur_conn;

	if ((cur_conn == NULL)
	 && (strcmp(name, "ctlislist") != 0))
		errx(1, "%s: cur_conn == NULL! (name = %s)", __func__, name);
	...
	if (strcmp(name, "initiator") == 0) {
		cur_conn->initiator = str;
```

`errx` is `__dead2` and the analyser knows it, so the only surviving
path with `cur_conn == NULL` is the one where `name` **is**
`"ctlislist"` — on which none of the `strcmp` arms below matches, so
none of the writes happens. The correlation is between a string and
which branch of a chain of string comparisons is taken, and no
path-sensitive analysis is going to carry that.

`cctl_islist_end_element` (14), `cctl_end_pelement` (13),
`cctl_end_element` (6) and `cctl_nvlist_end_element` (3) are all this,
with `ctlislist`, `ctlportlist`, `ctllunlist` and `ctlnvmflist` as their
respective exempt element names.

### `pfsync_status()`: two of five nvlist-optional locals with no default

`sbin/ifconfig/ifpfsync.c` reads the pfsync configuration out of an
nvlist the `SIOCGETPFSYNCNV` ioctl returns, and every field is optional
— each is copied out only if `nvlist_exists_*()` says the key is there:

```c
	struct sockaddr_storage syncpeer;
	int maxupdates = 0;
	int flags = 0;
	int version;
	...
	memset((char *)&syncdev, 0, IFNAMSIZ);
	if (nvlist_exists_string(nvl, "syncdev"))
		strlcpy(syncdev, nvlist_get_string(nvl, "syncdev"), IFNAMSIZ);
	if (nvlist_exists_number(nvl, "maxupdates"))
		maxupdates = nvlist_get_number(nvl, "maxupdates");
```

`syncdev` gets an explicit `memset`, `maxupdates` and `flags` get `= 0`
— the function already knows the keys are optional and defaults for
that reason.  `syncpeer` and `version` were the two that did not get
one.

Both are used unconditionally afterwards.  `version` is printed at the
bottom of the function whatever the ioctl returned.  `syncpeer` is
worse: the code switches on `syncpeer.ss_family` and, on `AF_INET6`,
calls

```c
	getnameinfo(syncpeer_sa, syncpeer_sa->sa_len, ...)
```

so a stack word that happens to hold `AF_INET6` hands `getnameinfo()`
a length taken from the same uninitialised storage.  A kernel that
does not put a `syncpeer` key in the nvlist — an older kernel, or a
pfsync interface with no peer configured — reaches this.

Both now start at zero: `int version = 0;` and a
`memset(&syncpeer, 0, sizeof(syncpeer))` next to the `syncdev` one.
`ss_family == 0` is `AF_UNSPEC`, which the existing `switch` already
falls through without printing a peer.

```
                sbin/ifconfig
                before  after
OK                 23      23
ERROR               0       0
findings           14      11
```

Read and not a defect in the same file set: `ifgif.c:64` reports `opts`
as uninitialised where the kernel writes it through `ifr.ifr_data` in
another translation unit — the out-parameter class.

## makefs: a timing macro that reads what it may not have written

`usr.sbin/makefs` reported eight `core.UndefinedBinaryOperatorResult`
findings, all "The right operand of '-' is a garbage value", spread
across `makefs.c` (4), `ffs.c` (2), `msdos.c` (2) and `walk.c` (1).
Every one of them is the same pair of macros in `makefs.h`:

```c
#define	TIMER_START(x)				\
	if (debug & DEBUG_TIME)			\
		gettimeofday(&(x), NULL)

#define	TIMER_RESULTS(x,d)				\
	if (debug & DEBUG_TIME) {			\
		struct timeval end, td;			\
		gettimeofday(&end, NULL);		\
		timersub(&end, &(x), &td);		\
		...
```

`timersub` is the `-`.  Every caller declares a bare
`struct timeval start;` and writes

```c
	TIMER_START(start);
	root = walk_dir(subtree, ".", NULL, NULL);
	TIMER_RESULTS(start, "walk_dir");
```

so `start` is written under one test of the global `debug` and read
under a second test of it, with the work being timed in between.  The
pair is sound only as long as nothing in that gap changes `debug`.

Nothing does — `debug` is written twice, both times while parsing the
command line, before any timer starts.  But the macro should not depend
on that, and it is worth noticing *where* the analyser put the eight
findings: exactly at the `TIMER_RESULTS` whose preceding call it cannot
see into.  `ffs.c:274` follows `ffs_validate()`, static in the same
file, and is not reported; `ffs.c:283` follows `ffs_create_image()`,
which reaches `open`/`mmap`, and is.  The finding tracks the opacity of
the gap, which is the correct reading of the premise.

`TIMER_START` now clears `x` first, so it is defined on every path:

```c
#define	TIMER_START(x)					\
	do {						\
		timerclear(&(x));			\
		if (debug & DEBUG_TIME)			\
			gettimeofday(&(x), NULL);	\
	} while (/* CONSTCOND */ 0)
```

The `do`/`while` is needed for the two statements and incidentally stops
the bare `if` from swallowing a following `else`.

### `dsl_dir_alloc()`: the loop that finds the parent can find none

`usr.sbin/makefs/zfs/dsl.c` walks a dataset name to its parent:

```c
	parent = NULL;
	for (lp = &l;; lp = &parent->children) {
		dirname = strsep(&nextdir, "/");
		if (nextdir == NULL)
			break;
		STAILQ_FOREACH(parent, lp, next) { ... }
		if (parent == NULL)
			errx(1, "no parent at `%s' for filesystem `%s'", ...);
	}
	...
	zap_add_uint64(parent->childzap, dir->name, dir->dirid);
	dir->parent = parent;
	dir->phys->dd_parent_obj = parent->dirid;
```

The `break` is taken on the *first* iteration when the name holds no
`/` at all, before the `STAILQ_FOREACH` has ever run — so `parent` is
still the `NULL` it was initialised to, and the `errx` inside the loop,
which exists for exactly this failure, is never reached.  The three
lines below then dereference it.

Both callers do guarantee a separator: `dsl_metadir_alloc()` builds
`"<pool>/<name>"` with `easprintf`, and the dataset loop rejects a name
that is not a child of the pool (`strchr(dsname, '/') == NULL` is one of
its three tests).  The root DSL directory takes the `name == NULL`
branch much earlier.  So the NULL path is unreachable today; it is the
function's precondition that is unstated.  A second `parent == NULL`
test after the loop, wording the same failure the one inside it words,
turns a crash into a diagnostic.

### `detrunc()`: `chaintofree` is written on one path and read on both

`msdos/msdosfs_denode.c` — and, identically, the kernel's
`sys/fs/msdosfs/msdosfs_denode.c` — declares

```c
	u_long eofentry;
	u_long chaintofree;
```

and then:

```c
	if (length == 0) {
		chaintofree = dep->de_StartCluster;
		dep->de_StartCluster = 0;
		eofentry = ~0ul;
	} else {
		error = pcbmap(dep, de_clcount(pmp, length) - 1, 0, &eofentry, 0);
		...
	}
	...
	if (eofentry != ~0ul) {
		error = fatentry(FAT_GET_AND_SET, pmp, eofentry,
				 &chaintofree, CLUST_EOFE);
		...
	}
	...
	if (chaintofree != 0 && !MSDOSFSEOF(pmp, chaintofree))
		freeclusterchain(pmp, chaintofree);
```

On the `length == 0` path `chaintofree` is written directly and
`eofentry` is set to the sentinel, so `fatentry()` is skipped.  On the
other path `chaintofree` is written *only* by `fatentry()`, which runs
only if `pcbmap()` left `eofentry` something other than `~0ul`.  The
read at the bottom is unconditional.

Reading every `return (0)` in `pcbmap()` settles it: all of them set
`*cnp`, and to either `MSDOSFSROOT` or a `cn` already masked with
`pm_fatmask`, so `~0ul` cannot come back on success and `fatentry()`
always runs on that path.  The correlation holds — but it is between
two variables three screens apart, nothing in the function states it,
and in the kernel copy the consequence of it not holding is
`freeclusterchain()` walking and freeing a FAT chain from a stack word.
`chaintofree = 0` at the declaration; zero is "nothing to free", which
is what the guard already tests for.

```
                usr.sbin/makefs         sys/fs/msdosfs
                before  after           before  after
OK                  31      31               7       7
ERROR                0       0               0       0
findings            23      12              13      12
```

The eleven that went are exactly the eight timer sites, `dsl.c:455`,
the makefs `detrunc`, and — the eighth timer site — `walk.c:329`;
nothing new appeared.

### The other twelve, read and not defects

- `msdosfs_fat.c` ×5 (`pcbmap`, `freeclusterchain`): `bp->b_data` after
  `bread(..., &bp)`.  makefs's own `bread()` in `ffs/buf.c` always
  writes `*bpp` (from `getblk`, which `err()`s rather than returning
  NULL) and either `err()`s or returns 0 — it has no failing return at
  all, so the `if (error) { brelse(bp); return (error); }` in every
  caller is dead.  The out-parameter-in-another-TU class.
- `ffs.c:1145` (`ffs_make_dirbuf`), `ffs.c:1219` (`ffs_write_inode`,
  a divide by `fs->fs_ipg`), `cd9660_debug.c` via `iso.h:347`
  (`isonum_731` on an unconstrained `buf`): the unconstrained-parameter
  class — each is a `static` function analysed as its own entry point,
  with a `dirbuf_t`/`fsinfo_t`/`unsigned char *` the caller has always
  initialised.
- `makefs.c:523` (`usage`): `fsoptions->fs_options` NULL.  All four
  `*_prep_opts` set it with `copy_opts()`, and `fstypes[]` is built by a
  macro that gives every entry a `prepare_options`, so the `if
  (fstype->prepare_options)` guard at line 123 is itself dead — but it
  is what tells the analyser the pointer can be NULL.
- `cd9660.c:1517`, `walk.c:376`: queue-macro use-after-free.
  `cd9660_generate_path_table()` `TAILQ_REMOVE`s before its `free`;
  `apply_specdir()` saves `next` before `free_fsnodes(curfsnode)`, and
  `free_fsnodes` unlinks its argument (`node->next = NULL`) before
  walking, so it frees only that node and its children.  That unlink is
  itself conditional on finding the node in its sibling list, which the
  analyser cannot prove.

## `ul(1)`: obuf grows, lbuf does not

`usr.bin/ul/ul.c` keeps the current line in `obuf`, and `obuf` grows:

```c
	if (col == buflen) {
		...
		obuf = realloc(obuf, sizeof(*obuf) * 2 * buflen);
		...
		buflen *= 2;
	}
```

starting from `MAXBUF` 512 and doubling for as long as the line runs.
`maxcol` tracks the high-water column and so is bounded only by the
length of the line.

`overstrike()` and `iattr()` each walk `obuf[0 .. maxcol-1]` and write
one `wchar_t` per column into

```c
	wchar_t lbuf[256];
	wchar_t *cp = lbuf;
```

plus one more, a sentinel at `lbuf[maxcol]`.  256 is not 512 and it is
certainly not "as long as the line".  A line past 256 columns that
carries any mode change — `hadmodes` in `flushln()` — runs off the end
of a 1KB stack buffer.  Both callers are reachable from ordinary use:
`iattr()` on `ul -i`, `overstrike()` whenever the terminal cannot
underline and `must_overstrike` is set.  The input is a text file.

`lbuf` is now one buffer shared by both functions, grown on demand the
way `obuf` itself is grown.

### The same two functions walk off the front

Both end the same way:

```c
	for (*cp=' '; *cp==' '; cp--)
		*cp = 0;
```

which zeroes the trailing run of blanks by walking back from the
sentinel — and tests `*cp` before checking `cp` is still inside `lbuf`.
When every column comes out blank the walk passes `lbuf[0]` and reads
`lbuf[-1]`; if the word before the buffer happens to hold a blank it
writes a zero there and keeps going.  `overstrike()` reaches that: it
maps `ALTSET`, `SUPERSC` and `SUBSC` to a space through its `default:`
arm, while `flushln()`'s `hadmodes` counts a change to any of them, so a
line entirely in shift-out mode arrives with `lbuf` all blanks.

The loop now stops at `lbuf`.  A differential test of the old and new
loops over thirteen line shapes, run with a guard cell in front so the
old one's underrun is legal to observe, gives no difference inside the
buffer and four writes to the guard cell from the old loop:

```
cases compared, mismatches=0, old-loop guard-cell writes=4
```

## `usbhidctl`: a 1000-byte stack buffer filled by the device

`parceargs()` builds the dotted path of HID collections enclosing the
current item:

```c
	char colls[1000];
	...
	cp = 0;
	for (d = hid_start_parse(...); hid_get_item(d, &h); ) {
		if (h.kind == hid_collection) {
			cp += sprintf(&colls[cp], "%s%s:%s",
			    cp != 0 ? "." : "",
			    hid_usage_page(HID_PAGE(h.usage)),
			    hid_usage_in_page(h.usage));
		} else if ...
```

`sprintf`, not `snprintf`, with `cp` advanced by the return and never
compared against `sizeof(colls)`.  The item stream comes from the report
descriptor the USB device supplies, so the nesting depth — and the
usage-page and usage strings at each level — are the device's to choose.
Enough nested collections write past the end of a 1000-byte stack
buffer.  The same loop appears twice in the function, once for `-a` and
once per named variable.

`colls` is also read before anything has written it.  Two lines below
the loop head:

```c
	asprintf(&var->name, "%s%s%s:%s",
	    colls, colls[0] != 0 ? "." : "", ...);
```

`colls[0]` is tested for every item, and `colls` is passed as a `%s`,
but only a `hid_collection` item ever writes either.  A descriptor whose
first item is an input, output or feature item prints uninitialised
stack into the variable's name — which `usbhidctl` then prints — and
reads past `colls` if none of the 1000 bytes is a NUL.

Both loops now start with `colls[0] = '\0'`, and the append goes through
a bounded helper that clamps `cp` to the buffer.

## `StrToPortRange`: the accessor pair reads before it writes

`sbin/ipfw/nat.c` and `sbin/natd/natd.c` carry the same function and the
same macros:

```c
#define SETLOPORT(x,y)   ((x) = ((x) & 0x0000ffff) | ((y) << 0x10))
#define SETNUMPORTS(x,y) ((x) = ((x) & 0xffff0000) | (y))
```

Each preserves the half of `x` it does not write.  Every path through
`StrToPortRange()` calls `SETLOPORT(*portRange, ...)` first, so the
first of the two reads the caller's uninitialised `port_range` — and
`SETNUMPORTS` then overwrites exactly the half that was read.  The value
is always discarded, so nothing observable comes of it on any real
target; it is still an indeterminate read, and `*portRange = 0` at the
top costs nothing.  Six findings, three in each copy.

```
                usr.bin/ul + usr.bin/usbhidctl + sbin/ipfw + sbin/natd
                before  after
OK                  15      15
ERROR                0       0
findings            29      19
```

Thirteen findings went and three appeared: `nat.c:1049`/`1054` and
`natd.c:2016` are the same three findings ten lines further down, which
is the length of the comment and statement inserted above them.  Ten
real.

Read and not defects in the same scope: `nat_show_data()` at
`nat.c:1059`/`1064` reports `oh + 1` on a garbage `oh`, where
`nat_get_cmd()` sets `*ooh` on its only `return (0)` — but does it
inside a `for (;;)` with a `continue`, which the analyser will not
unroll.

## `rpcbind`: a malformed reply frees somebody else's forwarding slot

`handle_reply()` in `usr.sbin/rpcbind/rpcb_svc_com.c` reads a reply
datagram off the forwarding socket and, at the bottom, does:

```c
done:
	free(buffer);

	if (reply_msg.rm_xid == 0) {
		/* "NULL xid on exit!" under SVC_RUN_DEBUG */
	} else
		(void)free_slot_by_xid(reply_msg.rm_xid);
```

`struct rpc_msg reply_msg;` is a bare local.  Three fields of it are
assigned after the `recvfrom`, and `rm_xid` is written by
`xdr_replymsg()` — but three paths reach `done:` before any of that:

```c
	buffer = malloc(RPC_BUF_MAX);
	if (buffer == NULL)
		goto done;
	...
	if (inlen < 0) { ... goto done; }
	...
	if (!xdr_replymsg(&reply_xdrs, &reply_msg)) { ... goto done; }
```

The third is the one that matters: it is reached by sending rpcbind a
datagram its XDR decoder rejects.  `free_slot_by_xid()` is

```c
	entry = xid % (u_int32_t)NFORWARD;
	return (free_slot_by_index(entry));
```

so the index stays in range, but `free_slot_by_index()` on an active
slot calls `netbuffree(fi->caller_addr)`, `free(fi->uaddr)`, decrements
`svc_maxfd` and `rpcb_rmtcalls`, and clears `FINFO_ACTIVE`.  A malformed
reply therefore tears down an unrelated in-flight forwarded rmtcall
chosen by a stack word: the legitimate reply for that call arrives later
and `forward_find()` no longer knows it, and `svc_maxfd` — the bound the
`select()` loop uses — is decremented against the wrong slot.

`reply_msg.rm_xid = 0;` before the first `goto`.  Zero is exactly what
the arm below it exists for.

## `pkg`: the signature path never sets the key it asserts about

`ecc_verify_data()` in `usr.sbin/pkg/ecc.c` fills a `cbdata` two ways:

```c
	if (sigfile != NULL) {
		cbdata.keyfp = fopen(sigfile, "r");
		if (cbdata.keyfp == NULL) { ... return (false); }
	} else {
		cbdata.keyfp = NULL;
		cbdata.key = key;
		cbdata.keylen = keylen;
	}
```

The `sigfile` arm sets only `keyfp`.  `ecc_verify_internal()` then calls

```c
	ecc_extract_pubkey(cbdata->keyfp, cbdata->key, cbdata->keylen, ...)
```

on both arms, and that function opens with

```c
	assert((keyfp != NULL) ^ (key != NULL));
```

— which reads the uninitialised `key`.  Whenever the stack word under it
happens to be non-NULL, `1 ^ 1` is 0 and `pkg` aborts on the assertion
in its signature-verification path.  When it reads as NULL the assertion
passes and nothing downstream uses `key` on that arm, so the failure is
intermittent in exactly the way an uninitialised read is.  The assertion
states the contract; `cbdata.key = NULL; cbdata.keylen = 0;` is that
contract being met.

This one was not in the before set.  It became visible only when a dead
store two lines up was removed:

```c
	keysz = MIN(sizeof(keybuf), cbdata->keylen / 2);

	keysz = sizeof(keybuf);
```

The first line computes a value the second discards — and it would have
under-reported `keybuf`'s capacity to `ecc_extract_pubkey()`, which
takes `&keysz` as the buffer size to write into, had it survived.  While
it was there the analyser reported the garbage read at the dead `/` and
stopped; with it gone the report moved to the live call, where the
uninitialised field actually is.  A dead store hiding the finding on the
statement after it is worth remembering as a shape.

The same function has one failure arm that returns instead of unwinding:

```c
	if (oidsz != sizeof(oid_ecpubkey) ||
	    memcmp(oidp, oid_ecpubkey, oidsz) != 0)
		return (1);
```

leaking `root` and the libder context on a key whose algorithm OID is
not `id-ecPublicKey`.  Every other failure in `ecc_extract_pubkey()`
goes to `out:`; this one now does too.

## `vidcontrol -p`: a trim with no lower bound

`dump_screen()` builds each text line and then:

```c
	do {
		line[x--] = '\0';
	} while (line[x] == ' ' && x != 0);
```

`x` is `shot.xsize` on entry.  The test reads `line[x]` before it checks
`x`, so an `xsize` of zero reads `line[-1]` and, if that byte happens to
be a blank, writes a NUL there and keeps walking backwards.  `xsize` is
`info.mv_csz` straight out of a `CONS_GETINFO` ioctl.  Swapping the two
tests — `while (x > 0 && line[x] == ' ')` — leaves every `xsize >= 1`
behaving exactly as before, the retained blank at `line[0]` included; a
differential test over twelve line shapes reports no difference and the
byte before the buffer untouched at `xsize == 0`.

Noted and not changed: both buffers here come from `alloca`, and the
`if (... == NULL)` after each is dead, since `alloca` does not return
NULL — it returns a pointer into a stack that may already be exhausted.
Converting them to `malloc` means unwinding on the two `err()` paths as
well, which is a larger change than this pass is making.

## `tabs ""`

`gettabs()` sets `*nstops = 0` and then walks `strtok(arg, ",")`.  For
`""` — and for `,` — `strtok` returns NULL immediately, so the loop
never runs and `*nstops` stays 0.  `main` tests `if (nstops >= 0)`,
where -1 means "no list was given", and prints

```c
	printf("%*s", (int)stops[0] - 1, "");
```

with `stops[0]` never written: a field width out of a stack word.  An
empty list is malformed like every other case `gettabs()` rejects, so it
now says so.

`tabs.c:167` is still reported afterwards, and that report is now about
the predefined-format path (`for (j = nstops = 0; ... formats[i].stops[j]
!= 0; j++)`), where every entry of a `const` table begins with 1 — the
analyser will not fold a const array of structs indexed by a loop
variable.

## `nscd`: one predicate, two spellings, a struct copy between them

`group_marshal_func()` counts `mem_size` under `grp->gr_mem != NULL` and
uses it under `new_grp.gr_mem != NULL`, where `new_grp` is a `memcpy` of
`*grp`.  Same predicate, but not one the analyser can carry across the
copy.  `mem_size = 0` at the declaration; zero makes the `memcpy` below
copy nothing, which is the right answer for a group with no members.

```
                rpcbind + tabs + vidcontrol + pkg + nscd
                before  after
OK                  37      37
ERROR                0       0
findings            13       9
```

## bhyve: a TRIM whose "nothing left" test is not the loop's exit test

`ahci_handle_next_trim()` walks the DSM ranges the guest queued:

```c
	while (done < len) {
		entry = &buf[done];
		elba = ...;			/* six bytes */
		elen = (uint16_t)entry[7] << 8 | entry[6];
		done += 8;
		if (elen != 0)
			break;
	}

	/* All remaining ranges were empty. */
	if (done == len) {
		...
		return;
	}
	...
	breq->br_offset = elba * blockif_sectsz(p->bctx);
	breq->br_resid = elen * blockif_sectsz(p->bctx);
```

The loop exits on `done < len` being false; the test below it is
`done == len`.  Those are the same condition only while `done` lands
exactly on `len`, and everything after it reads an `elba`/`elen` pair
the loop was supposed to have set — scaled into a block offset and a
length, and handed to the backing store as a discard.

It does land exactly today: `ahci_handle_dsm_trim()` computes `len` as a
sector count times 512, the continuation at the completion handler
passes `aior->len`/`aior->done` straight back, and `done` only ever
advances by 8.  So the pair is always written.  But the guard should be
the loop's own exit condition rather than a special case of it, and
`done >= len` costs nothing.

### The other three bhyve findings, read and not defects

- `pci_e82545.c:1346` — `MIN(left, iov->iov_len)` in the writable-header
  copy.  The walk is bounded by `if (pktlen < hdrlen + vlen) goto done;`
  and `pktlen` is incremented in the very block that fills
  `iov[iovcnt].iov_len`, so it is exactly the sum of the iovec.  The
  analyser will not carry a sum invariant across the descriptor loop.
- `pci_passthru.c:330` — `msixcap.pba_info` under
  `if (sc->psc_msix.capoff != 0)`.  `msixcap` is filled in the `cap ==
  PCIY_MSIX` arm, which is the same arm that sets `capoff`: one
  predicate, two spellings, a capability-list walk in between.
- `bootrom.c:282` — `ptr + i * PAGE_SIZE` after
  `bootrom_alloc(..., &ptr, NULL) != 0` is checked.  `bootrom_alloc()`
  has one `return (0)` and it writes `*region_out`; the other seven
  returns are all failures.

```
                usr.sbin/bhyve
                before  after
OK                  90      90
ERROR                2       2
findings            20      19
```

Both ERROR translation units are on the record (`snapshot.c` and one
more, missing `ucl.h`), unchanged across the pair.

### The progs shard's `core.UndefinedBinaryOperatorResult` set is now read

All 54 of them.  Fixed: `mlxcontrol` (4), `ifpfsync` (2), `makefs` (8),
`msdosfs` (1 here and 1 in the kernel copy), `ul` (2), `usbhidctl` (2),
`ipfw`/`natd` `StrToPortRange` (6), `rpcbind` (1), `tabs` (1),
`vidcontrol` (1), `nscd` (1), `pkg` (1, plus one that only became
visible once a dead store was removed), `bhyve` (1).  The rest are the
classes already on the record here — an out-parameter written in another
translation unit, an unconstrained parameter of a function analysed as
its own entry point, one predicate tested twice across an intervening
call, a loop the analyser will not unroll, and a macro that assigns
through its argument.

## Two declarations, twenty-one findings

`route6d` and `ppp` each have a helper that ends in `exit()` and a
declaration that does not say so.

`route6d`'s is `fatal()`, which ends in `rtdexit()`, which ends in
`exit(1)`.  Its seven findings are all the same sentence:

```c
	iffp = malloc(sizeof(*iffp));
	if (iffp == NULL) {
		fatal("malloc of iff");
		/*NOTREACHED*/
	}
	memcpy(iffp, &iff, sizeof(*iffp));
```

The comment is correct and the analyser could not know it, so it read on
into the `memcpy` — and into the `realloc` in `setindex2ifc()`, the
`malloc` in `allocopy()`, the `sysctl` buffer in `getifmtu()`, and the
`localtime()` return in `hms()`.

`ppp`'s is `AbortProgram()`, declared `extern void AbortProgram(int);` in
`main.h` and ending in `exit(excode)`.  Its callers are written on the
assumption:

```c
	if ((iov[*niov].iov_base = malloc(sz)) == NULL) {
		log_Printf(LogALERT, "physical2iov: Out of memory (%d bytes)\n", sz);
		AbortProgram(EX_OSERR);
	}
	if (h)
		memcpy(iov[*niov].iov_base, h, sizeof *h);
```

Fourteen findings across `exec.c`, `ether.c`, `netgraph.c`, `tty.c`,
`physical.c` and `udp.c` are that shape, in six copies of the same
device-serialisation pair.

```
                usr.sbin/route6d + usr.sbin/ppp
                before  after
OK                  59      59
ERROR                1       1
findings            66      45
```

Twenty-one gone, none new.  The one ERROR (`route6d/misc/cksum.c`, a
K&R-era declaration) is on the record and unchanged.

This is worth stating plainly, because it cuts both ways.  None of the
twenty-one was a defect: every one was the analyser correctly refusing
to believe a comment.  But an undeclared-noreturn helper is not free
either — the compiler cannot warn about code that really is unreachable
after it, it does warn about variables it thinks may be used
uninitialised past it, and every static analysis run over the program
pays for it in noise that has to be read by a person.  `patch(1)`'s
`fatal()` and `pfatal()` were the same finding earlier in this work.
Three programs is a shape, not a coincidence.

### A lint for it, and a negative result worth keeping

`tools/verify/noreturn_check.py` finds the shape: a function whose
body's last top-level statement is a call to something that does not
return — `exit()`, `abort()`, `err()` and friends, or another function
in the same file the pass has already concluded does not return — and
which contains no `return` anywhere, and whose declaration carries no
noreturn attribute.

Three things it got wrong while it was being written, each now a test
fixture:

- `\bnoreturn\b` does not match inside `__noreturn__`, because an
  underscore is a word character.  Every function already marked that
  way was reported.
- Taking the *last line* rather than the last top-level statement made
  `ifconfig`'s `set80211()` — which ends `if (ioctl(...) < 0) err(1,
  ...)` — look noreturn, and the propagation then called eighty of its
  callers noreturn too.
- A body containing a `return` anywhere means the function comes back,
  whatever its last statement is.  `mapfreq()` scans a table, returns on
  a hit, and `errx()`s at the bottom when there is none; marking it
  would have been wrong, not merely noisy.

With those fixed it reports **673 functions across 11,284 files**, and
reproduces both fixes above when the tree is reverted to before them.

Then the negative result.  The obvious next step was to pick the ones
worth marking, and the obvious predicate was the file: this file has an
undeclared-noreturn function, and this file has findings.  Twenty-four
functions across nineteen files were marked `__dead2` on that basis and
measured:

```
                the nineteen files
                before  after
OK                  76      76
ERROR                0       0
findings           116     116
```

Exactly nothing.  The batch was reverted.

What separates `route6d` and `ppp`, where two declarations closed
twenty-one findings, is not the file — it is that the call sits on the
path the finding walks:

```c
	if ((p = malloc(n)) == NULL) {
		fatal("malloc");
	}
	memcpy(p, &x, n);        /* the finding, reachable only because
				    the analyser thinks fatal() returns */
```

A `usage()` called from `getopt` and followed by nothing costs nothing,
however undeclared it is.

Two ways of predicting which is which were then tried and both dropped.
Ranking by findings within 40 lines after a call site scored the
known-zero batch 18.  Ranking by the guard shape itself — the call alone
in an `if` body, a name from the condition used after the block — scored
it 34 sites across 11 functions, because `if (argc < 2) usage();` is
that shape and an `int argc` produces no finding.  Neither beat the only
accurate oracle, which is a before/after sweep at the same scope on the
same tree, and which is cheap.

So the lint reports and does not rank, and it runs in `run_all.sh` as a
report rather than a gate.  673 items is not a list to apply wholesale;
it is a list to read against a measurement.

## `diff -i -N`: a name compared with itself

`usr.bin/diff/diffdir.c`, in `diffit()`:

```c
	/*
	 * If we are ignoring file case, use dent2s name here if both names are
	 * the same apart from case.
	 */
	if (ignore_file_case && strcasecmp(dp2->d_name, dp2->d_name) == 0)
		strlcpy(path2 + plen2, dp2->d_name, PATH_MAX - plen2);
	else
		strlcpy(path2 + plen2, dp->d_name, PATH_MAX - plen2);
```

`dp2->d_name` against `dp2->d_name`.  The comment says what was meant;
the code compares one name with itself and is therefore always 0, so
under `-i` the first branch is always taken.

Two things follow, and the caller is what makes them reachable.
`diffdir()` walks the two sorted directory listings together:

```c
	dent1 = dp1 != edp1 ? *dp1 : NULL;
	dent2 = dp2 != edp2 ? *dp2 : NULL;

	pos = dent1 == NULL ? 1 : dent2 == NULL ? -1 :
	    ignore_file_case ? strcasecmp(dent1->d_name, dent2->d_name) :
	    strcmp(dent1->d_name, dent2->d_name) ;
```

so it passes **NULL** for the side an entry is missing from, and relies
on `-N` or `-P` to diff the present file against nothing:

```c
	} else if (pos < 0) {
		if (Nflag)
			diffit(dent1, path1, dirlen1, dent2, path2, dirlen2, flags);
	...
	} else {
		if (Nflag || Pflag)
			diffit(dent2, path1, dirlen1, dent1, path2, dirlen2, flags);
```

`diff -i -N` over two directories that are not identical therefore
dereferences NULL.  And when both entries do exist but the names
genuinely differ — the `-N` case where `dent1` sorts first — `path2` was
built from `dent2`'s name, so diff compared `dir1/a` against `dir2/b`
instead of reporting `a` as absent from `dir2`.

Comparing the two names, which is what the comment says, does both jobs,
and the `else` arm already builds the right path when `dp2` is NULL.

A model of the block driven with the four `(dp, dp2)` pairs `diffdir()`'s
loop produces — `tools/verify/probes/diffit_path2.c`, a model of the code
and not the program — separates them:

```
case                                    old         new         wanted
both present, names differ by case      readme      readme      readme
both present, different names (-N)      zulu        README      README
only in dir1 (-N)                       SIGSEGV     README      README
only in dir2 (-N or -P)                 SIGSEGV     zulu        zulu

old block wrong or crashed in 3 of 4 cases
```

The one case the comment was written about is the one case the old block
got right.

## `tftp`: realloc into the only pointer to the buffer

`command()` in `usr.bin/tftp/main.c`:

```c
	static char *line;
	static size_t sz;
	...
	if ((size_t)len >= sz)
		line = realloc(line, sz = len + 1);
	strlcpy(line, bp, sz);
```

Three mistakes in one statement.  The result goes back over `line`,
which is the only pointer to the old buffer, so a failure leaks it.  The
result is not checked, so a failure hands `strlcpy()` a NULL
destination.  And `sz` is updated *inside* the call, so after a failure
it describes a buffer that does not exist.

The `line == NULL` half of the new test is the other path: the
non-interactive branch below uses `getline(&line, &sz, stdin)`, which
leaves the pair unspecified when it fails, so `sz` can outlive the
buffer `line` pointed at.

```
                usr.bin/diff + usr.bin/tftp
                before  after
OK                   8       8
ERROR                0       0
findings             5       3
```

The three that remain: `diffdir.c:60` is the `RB_INSERT_COLOR` macro
expansion already on the record; `diffdir.c:255` reports `dp` where all
three call sites pass a non-NULL first argument — `pos` is 1 exactly
when `dent1` is NULL and -1 exactly when `dent2` is, so the argument in
first position is never the NULL one, which is a ternary chain the
analyser will not carry; `tftp/main.c:323` is `res->ai_addr` after
`getaddrinfo`, the out-parameter class.

## `lpr`: seven more that end in exit(), and this time it paid

The lint's list is not to be applied wholesale — but `lpc`'s `quit()` is
called in exactly the shape that costs findings, and it turned up while
reading the `unix.cstring.NullArg` set rather than by picking off the
list:

```c
	if ((bp = el_gets(el, &num)) == NULL || num == 0)
		quit(0, NULL);

	len = MIN(MAX_CMDLINE - 1, num);
	memcpy(cmdline, bp, len);
```

`quit()` is declared `void quit(int, char *[]);` in `lpc/extern.h` and
ends in `exit()` in `cmds.c`.  So the analyser walks out of the guard
with `bp` still NULL and into the `memcpy`.

`lpr` has fourteen such functions across its six programs.  Seven were
marked — `quit`, `fatal`, `mcleanup`, `fhosterr`, `frecverr`, `abortpr`,
`intr` and `cleanup` — and the four `usage()`s were left, on the
evidence of the batch that measured nothing.

```
                usr.sbin/lpr
                before  after
OK                  28      28
ERROR                0       0
findings             6       3
```

Three closed, none new: `lpc.c:174` (the `memcpy` above),
`common_source/rmjob.c:331` in `rmremote()`, and `lprm/lprm.c:100` in
`main()`.

Half the remaining set of six, gone, from declarations in a header —
which is the case for reading the lint's list *against* the findings
rather than down it.

### `lpc.c:312`, read and not a defect

The one `NullArg` left in `lpr` is `help()`:

```c
		for (j = 0; j < columns; j++) {
			c = cmdtab + j * lines + i;
			if (c->c_name)
				printf("%s", c->c_name);
			if (c + lines >= &cmdtab[NCMDS]) {
				printf("\n");
				break;
			}
			w = strlen(c->c_name);
```

`c->c_name` is tested two lines above and not here, which reads like the
bug — and `NCMDS` is `sizeof(cmdtab)/sizeof(cmdtab[0])`, so
`cmdtab[NCMDS-1]` really is the `{0, 0, 0, 0, 0}` sentinel and
`c->c_name` really can be NULL at the `printf`.

It cannot be NULL at the `strlen`.  The test at the end of each
iteration is on `c + lines`, which is the *next* iteration's `c`, so `c`
at the top of the loop is always below `&cmdtab[NCMDS]` — the loop never
indexes past the array either.  And when `c` is the sentinel,
`c + lines >= &cmdtab[NCMDS]` holds for any `lines >= 1`, which the
`columns == 0` guard above ensures, so the `break` always fires before
the `strlen`.  The relation between `c` and `c + lines` across
iterations is what the analyser will not carry.

## Twelve more, and a gap in the lint that hid them

`dump`'s `blkread()`:

```c
	if (tmpbuf == NULL && (tmpbuf = malloc(secsize)) == NULL)
		quit("buffer malloc failed\n");
	...
	memcpy(buf, &tmpbuf[base], xfer);
```

and `pfctl`'s `pfctl_table()`:

```c
	if (command == NULL)
		usage();
	...
	if (!strcmp(command, "-F")) {
```

Both are the paying shape.  Neither was in the lint's 673, and the
reason `dump` was missed is worth recording: `quit()` does not end in
`exit()`, it ends in `dumpabort()` — which `dump.h` **already** declares
`__dead2`, in another file.  The lint's propagation ran within a single
translation unit and seeded only the C library's own names, so the chain
broke at exactly the link that was already correct.

It now also seeds from the noreturn declarations in the directory's own
headers, and immediately reports `quit()` and `tape.c`'s `tperror()`,
which ends in `Exit()`.

`pfctl` was missed for a duller reason: `usage()` is declared
`extern void usage(void);` inside `pfctl_table.c` itself, and defined in
`pfctl.c`, so the definition and the declaration the caller sees are in
different files.  Marking both.

```
                sbin/dump + sbin/pfctl
                before  after
OK                  19      19
ERROR                0       0
findings            34      22
```

Twelve from three declarations, none new — and only two of the twelve
were the `unix.cstring.NullArg` findings that started this.  The other
ten were `core.NullDereference` in `query()`, `allocfsent()`,
`dump_getfstab()`, `rollforward()`, `mapfiles()` twice, `searchdir()`
and `getino()` twice, all of them past a `quit()` the analyser thought
returned.

## What separates the noreturn markings that pay, and how far that gets you

Five programs' declarations closed forty findings.  Two batches chosen
by plausible heuristics closed none.  The difference is narrow enough to
write down, and `--guards` in `noreturn_check.py` is that predicate:

> the helper is called as the whole body of an `if` that tests a
> **pointer** against NULL, and that pointer is used as a pointer after
> the block closes.

Every payer fits:

```
route6d  if ((iffp = malloc(...)) == NULL) fatal(...);      memcpy(iffp, ...)
ppp      if ((iov[n].iov_base = malloc(sz)) == NULL) ... AbortProgram();
                                                         memcpy(iov[n].iov_base, ...)
lpc      if ((bp = el_gets(...)) == NULL || num == 0) quit(0, NULL);
                                                         memcpy(cmdline, bp, len)
dump     if (tmpbuf == NULL && (tmpbuf = malloc(...)) == NULL) quit(...);
                                                         memcpy(buf, &tmpbuf[base], ...)
pfctl    if (command == NULL) usage();                     strcmp(command, "-F")
```

Two earlier attempts at the same idea failed for reasons worth keeping:

- The word **pointer** was missing.  `if (argc < 2) usage();` is the
  same shape with an `int`, and produces no finding — which is why the
  shape-only version scored a known-zero batch 34.
- The search looked only in the file that **defines** the helper.  `ppp`
  defines `AbortProgram()` in `main.c` and guards with it in
  `physical.c`, `udp.c` and four more; `lpc` defines `quit()` in
  `cmds.c` and guards in `lpc.c`; `pfctl` defines `usage()` in
  `pfctl.c` and guards in `pfctl_table.c`.  Only `route6d` and `dump`
  keep both in one file — which is exactly the two a same-file search
  found, and the tell that the search was wrong.

With both fixed it separates the two known sets cleanly: all five
payers, and nothing at all in the fifteen directories whose measured
answer was zero.  Across `bin`, `sbin`, `usr.bin` and `usr.sbin` it cuts
477 candidates to **26**.

### And then the honest number

Those 26 were run through the same measure-and-keep loop, one directory
at a time, each kept only on a real drop with `--check-errors` matching
on both sides.  **One of twenty-five paid**: `usr.sbin/yppush`, 2 → 0,
where `yp_push()` writes

```c
	if ((job = (struct jobs *)malloc(sizeof (struct jobs))) == NULL) {
		yp_error("malloc failed");
		yppush_exit (1);
	}
	...
	job->stat = 0;
```

The other twenty-four are the same shape and closed nothing, because the
analyser stops at the first defect on each path: a guard can be exactly
this and still have no finding reported at it, either because nothing
downstream is checked or because an earlier finding on the same path
shadowed it.

So the predicate is a good filter and a poor oracle.  477 → 26 is worth
having; 26 → 1 is the reminder that the sweep is still the only thing
that answers the question.  All five of the cases that mattered were
found by reading a finding and walking back to its cause, not by
scanning a list — and that remains the method.

## `ctladm delay -t 5`, and an fsck error handler that is only sometimes fatal

`cctl_delay()` in `usr.sbin/ctladm/ctladm.c` collects three options:

```c
	char *delayloc = NULL;
	char *delaytype = NULL;
	int delaytime = -1;
	...
		case 'l': delayloc = strdup(optarg); break;
		case 't': delaytime = strtoul(optarg, NULL, 0); break;
	...
	if (delaytime == -1) {
		warnx("%s: you must specify the delaytime with -t", __func__);
		retval = 1;
		goto bailout;
	}

	if (strcasecmp(delayloc, "datamove") == 0)
```

One of the two required options is checked.  `ctladm delay -t 5` with no
`-l` reaches `strcasecmp(NULL, "datamove")`.  The missing test is now
there, worded like the one above it.

`checkfs()` in `sbin/fsck/fsck.c`:

```c
	vfstype = strdup(pvfstype);
	if (vfstype == NULL)
		perr("strdup(pvfstype)");
	for (i = 0; i < (int)strlen(vfstype); i++) {
```

This reads as a fatal error handler and is not one.  `perr()` calls
`vmsg(1, fmt, ap)`, and `vmsg()` exits **only when `preen` is set**:

```c
	if (fatal && preen) {
		(void) printf("%s: UNEXPECTED INCONSISTENCY; RUN %s MANUALLY.\n", ...);
		exit(8);
	}
```

Returning is the contract the rest of the program relies on —
`devcheck()` calls `perr()` three times and returns `origname` after
each.  So without `-p`, a failed `strdup` printed a message and walked
into `strlen(NULL)`.  `fsutil.c` already exports `estrdup()`, which is
`strdup` plus `err(1, "strdup failed")`, and that is what this line
wanted.

Worth noting for the lint: `perr()` is a noreturn-shaped function that
`noreturn_check.py` will never report, and correctly — its last
statement is `va_end(ap)`, and it is not noreturn anyway, only
conditionally so.  A helper that exits on some paths and returns on
others is the shape neither the lint nor a reader is well served by.

```
                usr.sbin/ctladm + sbin/fsck
                before  after
OK                   5       5
ERROR                0       0
findings            62      60
```

## Diffing sweeps without being fooled by line numbers

The two-line `ctladm` fix carries a thirteen-line comment, and every one
of the forty-eight findings below it in the file moved by thirteen
lines.  Diffed by the finding's `where`, that reads as 48 closed and 48
new around a real delta of 2.  This has been hand-checked three times in
this work — the `ul` batch's "three new", the `tftp` batch's "one new",
and now this.

`tools/verify/sweep_diff.py` identifies a finding by
`(file, checker, function, message)` instead, which is stable under
insertion, and warns if the OK/ERROR counts moved — because two sides
with different translation-unit counts are not the same measurement.

### Read and not defects in the same round

- `usr.sbin/pkg/config.c:247` — `strcmp(buf, c[i].key)` where `buf`
  starts NULL.  `buf` is `open_memstream(&buf, &bufsz)`'s target, and
  the `fflush(buffp)` three lines above always runs; POSIX has the
  pointer and size updated at that point, for an empty stream too.  The
  `if (buf != NULL)` guard above is for the first iteration, before
  anything has been flushed.
- `sbin/ifconfig/ifconfig.c:1002` — `strcmp(name, p->c_name)` reports
  `name`, an unconstrained parameter of `cmd_lookup()`.
- `bin/ed/glbl.c:118` and `bin/ed/main.c:1076` — both are `REALLOC(b, n,
  i, err)` followed by a `memcpy` into `b`.  The macro's whole body is
  under `if ((i) > (n))` and returns `err` when the allocation fails, so
  `b` is non-NULL afterwards on any path where `i > n` — and `i` is
  `n + 1` or a length plus one at both sites, with `n` starting at zero.
  The analyser will not relate the macro's guard to the use after it.
- `usr.bin/units/units.c:619` — `strcmp(*one, *two)` in
  `compareproducts()`.  `NULLUNIT` is `static char NULLUNIT[] = ""`, not
  NULL, and the two tests above the `strcmp` return 1 for a NULL `*one`
  with a non-`NULLUNIT` `*two` and vice versa, while a `NULLUNIT` on
  either side advances that side instead.  Both are non-NULL by the time
  the `strcmp` runs.

## `jail(8)`: a ternary whose condition is a constant

`rdtun_params()` checks whether a read-only-after-creation jail
parameter has changed:

```c
	if (rtjp->jp_valuelen != jp_valuelen ||
	    (CTLTYPE_STRING ? strncmp(rtjp->jp_value, jp_value, jp_valuelen)
			    : memcmp(rtjp->jp_value, jp_value, jp_valuelen))) {
		if (dofail) {
			jail_warnx(j, "%s cannot be changed after creation", ...);
```

`CTLTYPE_STRING` is `3`.  It is the constant, not a test — so the
condition is always true, `memcmp` is unreachable, and **every**
parameter is compared with `strncmp`, which stops at the first NUL.

`ip4.addr` is an array of `struct in_addr`.  10.0.0.1 is `0a 00 00 01`
and 10.0.5.9 is `0a 00 05 09`: `strncmp` compares `0a` against `0a`,
reaches a NUL in both at byte two and returns 0.  Two different
addresses compare equal, so a change to a tunable that cannot be changed
after creation is not reported and `jail -m` accepts it silently.

The test the line wanted is the one twelve lines above it, which the
string default already uses:

```c
		} else if ((jp->jp_ctltype & CTLTYPE) == CTLTYPE_STRING)
			jp_value = "";
```

The sweep is indifferent to this fix and that is worth stating: the
`unix.cstring.NullArg` reported at that line is about `jp_value` being
NULL, not about which comparison runs, and it survives.  A wrong
comparison against the right pointers is not a checker's business.  This
one came out of reading the line the finding pointed at.

## `chat(1)`, and two more gaps in the lint

`dup_mem()`:

```c
	void *ans = malloc (c);
	if (!ans)
	    fatal(2, "memory error!");

	memcpy (ans, b, c);
```

`fatal()` ends in `terminate()`, `terminate()` ends in `exit(status)`,
and neither declaration said so — while `usage()` three lines above them
in the same header block already carries `__dead2`.

`noreturn_check.py` reported nothing here, for two reasons both now
fixed and both now fixtures:

- The definition pattern required the **name at column 0**, which is KNF
  but not universal: `chat.c` writes `void terminate(int status)` on one
  line, and every helper in the file was invisible.  The pattern now
  allows the return type on the same line, with the repetition greedy so
  the capture lands on the last identifier before the parenthesis.
- `terminate()` ends `#endif` and then `exit(status);`.  Folding the
  preprocessor line into the statement accumulator made the last
  statement read as `"#endif exit(status);"`, which matches no call — so
  `terminate()` looked like it returned, and `fatal()`, which ends in
  `terminate()`, was never reached by the propagation either.

With both fixed the file reports five, three of them the chain above.

```
                usr.bin/chat + usr.sbin/jail
                before  after
OK                   5       5
ERROR                0       0
findings            10       9
```

One closed — `dup_mem`'s `memcpy` — and none new.

## The progs shard's `unix.cstring.NullArg` set is read

All 49.  **Sixteen were real and are fixed**; the other thirty-three are
characterised below rather than suppressed.

Fixed:

| where | what |
|---|---|
| `route6d` ×4 | `fatal()` ends in `rtdexit()` ends in `exit(1)`, undeclared |
| `ppp` `physical.c`, `udp.c` | `AbortProgram()` ends in `exit()`, undeclared |
| `lpr` `lpc.c` | `quit()` ends in `exit()`, undeclared |
| `dump` `traverse.c` ×2 | `quit()` ends in `dumpabort()`, which was already `__dead2` |
| `pfctl` `pfctl_table.c` | `usage()` ends in `exit()`, undeclared |
| `chat` `chat.c` | `fatal()` → `terminate()` → `exit()`, both undeclared |
| `diff` `diffdir.c` | a name compared with itself |
| `tftp` `main.c` | `realloc` into the only pointer to the buffer |
| `fsck` `fsck.c` | `perr()` is only fatal under `-p` |
| `ctladm` `ctladm.c` | one of two required options checked |
| `ndp` `ndp_netlink.c` | an optional netlink attribute walked with `strlen` |

Nine of the sixteen are one shape — a helper that exits, and a
declaration that does not say so — which is why it has its own lint and
its own section above.

The thirty-three that remain, by class:

- **An unconstrained parameter of a function analysed as its own entry
  point** (12): `natd`'s `SetAliasAddressFromIfName(ifn)`;
  `bsdinstall/partedit`'s `provider_for_name(..., name)` twice; `ppp`'s
  `SetVariable`, where `argp` is `arg->argv[arg->argn]` or `""` and
  never NULL; `sesutil`'s `devnames`; `ifconfig`'s `cmd_lookup(name)`;
  `ipf`'s `expand_string`; `jail`'s `dep_setup`; `crunchide`;
  `calendar`; `ruptime`; and `zfsbootcfg`'s `add_pair(type, ...)`,
  whose caller writes `if (type == NULL) type = "DATA_TYPE_STRING";`
  four lines before the call.
- **A switch that is exhaustive over a validated range** (2): `ppp`'s
  `chap_Input`.  `chapcodes[]` has five entries so `MAXCHAPCODE` is 4,
  the header check rejects `code == 0 || code > MAXCHAPCODE`, and the
  first switch covers all four remaining codes — allocating `ans` on
  three of them, and the second switch only reads `ans` under the two
  where it did.
- **A macro whose guard the analyser will not relate to the use after
  it** (2): `ed`'s `REALLOC(b, n, i, err)`, whose entire body is under
  `if ((i) > (n))` and returns `err` when the allocation fails.
- **A value returned across a translation unit** (1): `ppp`'s
  `nat_cmd.c` uses `m_get()`, which calls `AbortProgram(EX_OSERR)` on
  both of its failure paths — but it lives in `mbuf.c`.
- **A sentinel that is not NULL** (2): `units`' `compareproducts`, where
  `NULLUNIT` is `static char NULLUNIT[] = ""`.
- **A ternary chain the analyser will not carry** (1): `diff`'s
  `diffit(dp, ...)`, where `pos` is 1 exactly when `dent1` is NULL and
  -1 exactly when `dent2` is, so the first argument is never the NULL
  one.
- **A loop invariant across iterations** (1): `lpc`'s `help()`, where
  the bound tested at the end of each iteration is the next iteration's
  index.
- **A stream flushed before it is read** (1): `pkg`'s `config_parse`,
  where `open_memstream`'s buffer pointer is updated by the `fflush`
  three lines above the `strcmp`.
- The rest are `ppp`'s `command.c` and `ipv6cp.c`, all arrays reached
  through an unconstrained struct pointer.

## `gprof`: two loops that free the node they just stepped to

`usr.bin/gprof/arcs.c` walks the cycle list twice, and both walks make
the same mistake.  `cycleanalyze()`:

```c
	for ( clp = cyclehead ; clp ; ) {
	    endlist = &clp -> list[ clp -> size ];
	    for ( arcpp = clp -> list ; arcpp < endlist ; arcpp++ )
		(*arcpp) -> arc_cyclecnt--;
	    cyclecnt--;
	    clp = clp -> next;
	    free( clp );
	}
```

Advance, then free.  So `cyclehead` itself is never released, the node
*behind* it is, and the top of the next iteration reads `clp -> list`
and `clp -> size` through the pointer that was just freed.

`compresslist()` is the same and one worse, because it also unlinks:

```c
	*prev = clp -> next;
	clp = clp -> next;
	free( clp );
```

The node it unlinked leaks; the node it freed is still on the list
through `*prev`; and the loop walks the freed one.  That is also why
`cycleanalyze()` reports at its *own* loop head — it runs
`compresslist()` first and then walks a list containing a freed node,
so the two findings are one defect seen from both ends.

Both now take the successor first, free the node the loop is on, and
then step.

```
                usr.bin/gprof
                before  after
OK                   9       9
ERROR                0       0
findings             2       0
```

## `ipsend -R` frees a stack address

`ip_resend()` allocates its Ethernet header once and frees it at the
bottom:

```c
	eh = (ether_header_t *)malloc(sizeof(*eh));
	...
	while ((i = (*r->r_readip)(&mb, NULL, NULL)) > 0) {
		if (!(opts & OPT_RAW)) {
			eh = (ether_header_t *)realloc((char *)eh, sizeof(*eh) + len);
			...
		} else {
			eh = (ether_header_t *)mb.mb_buf;
			len = i;
		}
		if (sendip(wfd, (char *)eh, len) == -1)
			...
	}
	(*r->r_close)();
	free(eh);
```

`mb` is `mb_t mb;` — a local.  So under `-R`, `eh` is assigned the
address of a member of this function's own stack frame, the allocation
it was holding is leaked, and the `free(eh)` at the bottom is handed a
stack address on every run.

The send now goes through a separate `pkt`, leaving `eh` owning what it
allocated.  The `realloc` on the other arm also went straight back over
`eh`, so a failure lost the only pointer to the old header and then
wrote `ether_type` through NULL; that is checked now too.

```
                sbin/ipf/ipsend
                before  after
OK                   3       3
ERROR               12      12
findings             2       0
```

The twelve ERROR are `NOT_BUILT` translation units already on the
record, unchanged across the pair.

## The queue-macro use-after-free class

Eleven of the progs shard's thirteen `Use of memory after it is freed`
findings are one shape, and none of them is a defect:

```c
	while ((p = TAILQ_FIRST(&head)) != NULL) {
		...
		TAILQ_REMOVE(&head, p, entries);
		free(p);
	}
```

`TAILQ_REMOVE` unlinks before the `free`, so the next `TAILQ_FIRST`
returns a different node — but the analyser does not model the macro as
unlinking, so it believes the head can still be the pointer just freed.

The eleven: `makefs` `cd9660.c` and `walk.c` (already above), `fsck` and
`quotacheck`'s shared `preen.c`, `jail`'s `config.c` four times — where
`free_param()` frees the members, `TAILQ_REMOVE`s and then frees the
node, and `free_param_strings()` does the same one level down —
`rtadvd`'s `rm_rainfo()`, `hastd`'s `hastd_reload()`, `systat`'s
`dsmatchselect()` (which sets `matches = NULL` immediately after the
`free`), and `tail`'s `r_buf()`.

`tail`'s is worth a second look and survives it: the out-of-memory loop
reads `first = TAILQ_FIRST(&head)` *before* testing `TAILQ_EMPTY`, and
uses `first->len` after — which is safe only because the `err(1, ...)`
between them does not return.  It does not return; `err` is
`__dead2` in `<err.h>`.

Also read and not a defect: `lpr`'s `rmremote()`, reported as
`free()` of a global.  `iov[0..3]` and the `2 * users` entries after
them are string literals and borrowed pointers, `firstreq` is set to
`4 + 2 * users`, and the `asprintf` results fill exactly
`[firstreq, firstreq + requests)` — which is exactly what the free loop
covers.  The analyser will not track which slots of an array hold
literals across two loops it does not unroll.

## `Use of memory allocated with size zero` — one real class, one false one

Thirteen findings in the progs shard, and reading all thirteen splits
them cleanly in two.

**The false class — count, allocate, fill.** Nine of the thirteen are
this shape:

```c
	n = 0;
	TAILQ_FOREACH(x, &list, e)
		n++;
	arr = malloc(n * sizeof(*arr));
	i = 0;
	TAILQ_FOREACH(x, &list, e)
		arr[i++] = x;
```

The analyser sees that `n` can be zero, so the allocation can be
zero-sized, and that `arr[i++]` writes through it.  It will not carry
the fact that the same emptiness that made `n` zero also makes the
second loop body unreachable.  `bin/ps`'s `descendant_sort()` (three
findings — the guard there is `if ((lvl = ki[src].ki_d.level) == 0)
continue;`, and reaching `path[n / 8]` requires `lvl > 0`, which forces
`maxlvl >= 1`), `sbin/dhclient`'s `dispatch()`, `sbin/restore`'s
`printlist()`, `usr.sbin/efibootmgr`'s `make_next_boot_var_name()`,
`usr.sbin/jls`'s `add_param()`, `usr.sbin/kbdmap`'s `menu_read()`,
`usr.sbin/bsdinstall`'s `apply_changes()` and `usr.bin/mkimg`'s
`qcow_write()` are all it.

**The real class — a size the kernel decides, used without a floor.**
The other four are `sbin/ipfw`, and there the size is not counted
locally, it is read back out of a structure the kernel filled in:

```c
	sz = req.size;
	if ((olh = calloc(1, sz)) == NULL)
		return;
	olh->size = sz;
```

`req` is an `ipfw_obj_lheader` that was `memset` to zero and handed to
`getsockopt`.  Nothing between the `memset` and the `calloc` constrains
what comes back.  If `req.size` is zero — or anything short of
`sizeof(ipfw_obj_lheader)` — `calloc(1, sz)` returns a minimum-bucket
allocation and `olh->size = sz` writes four bytes through it.  The
present kernel never reports less (`dump_srvobjects()` sets
`hdr->size = sizeof(ipfw_obj_lheader) + count * sizeof(ipfw_obj_ntlv)`
before the `ENOMEM` return; `list_ifaces()` likewise), but userland is
on the other side of a trust boundary from it, and the check is one
line.  Both `ipfw_list_objects()` and `ipfw_get_tracked_ifaces()` now
refuse a `req.size` below `sizeof(req)`.

`sbin/ipfw`'s `table_do_get_list()` is the same defect written
differently, and this one does not need a hostile kernel to reach:

```c
	sz = 0;
	oh = NULL;
	for (c = 0; c < 8; c++) {
		if (sz < i->size)
			sz = i->size + 44;
		...
		if ((oh = calloc(1, sz)) == NULL)
			continue;
		table_fill_objheader(oh, i);
```

`sz` starts at zero and is only *raised*.  `0 < i->size` is false when
`i->size` is zero, so `sz` stays zero for all eight attempts, and
`table_fill_objheader()` writes a whole `ipfw_obj_header` — index, TLV
type, length, set and a `strlcpy`'d table name — through a zero-sized
allocation.  `sz` is now seeded at `sizeof(*oh)`, which is the header
that fill always writes.

Measured over `bin/ps`, `sbin/fsck`, `sbin/ipfw`, `sbin/quotacheck`,
`usr.sbin/bsdinstall` and `usr.sbin/jail`: 52 → 49, with the
translation-unit counts unchanged at 39 OK and no ERROR on either side.
The three that closed are exactly the three `ipfw` sites.  `usr.sbin/kbdmap`
was measured separately: 3 → 3, no change, as expected — a NULL check
does not tell the analyser the size is non-zero.

### Four allocations used without a check, and two frees that were missing

Read out of the same set and fixed because they are defects even where
the analyser was wrong about why:

* `bin/ps`'s `descendant_sort()` — the `calloc` for the sibling bitmap
  was the only unchecked allocation in the function; the `malloc` two
  lines below it is checked with `xo_errx`.
* `usr.sbin/kbdmap`'s `menu_read()` — `km_sorted` is indexed in the
  statement after the `malloc`.  (`<err.h>` was not included; adding the
  check without it turned the file into an ERROR, which the
  `--check-errors` gate caught before it could be mistaken for a clean
  sweep.)
* `usr.sbin/bsdinstall`'s `apply_changes()` — same shape, `tobesorted`.
* `sbin/fsck`'s `preen.c` — `p_devname`, `p_mntpt` and `p_type` are all
  `estrdup()`ed when a partition is added; only two of the three were
  freed.  (`quotacheck`'s near-copy is *not* affected: its `p_mntpt` is
  a `const char *` borrowed from `quota_fsname()`.)
* `usr.sbin/jail`'s `load_config()` — each wildcard jail's name and
  parameters are freed and the record is `TAILQ_REMOVE`d, and then the
  record itself is dropped on the floor.

## nfsuserd: an unknown NFSv4 id crashes the daemon, if the kernel then says no

`nfsuserdsrv()` answers four RPCs — uid to name, gid to name, name to
uid, name to gid — and all four are written the same way:

```c
	pwd = getpwuid((uid_t)info.id);
	info.retval = 0;
	if (pwd != NULL) {
		nid.nid_usertimeout = defusertimeout;
		nid.nid_uid = pwd->pw_uid;
		nid.nid_name = pwd->pw_name;
		...
	} else {
		nid.nid_usertimeout = 5;
		nid.nid_uid = (uid_t)info.id;
		nid.nid_name = defaultuser;
		...
	}
	nid.nid_namelen = strlen(nid.nid_name);
	nid.nid_flag = NFSID_ADDUID;
	error = nfssvc(NFSSVC_IDNAME | NFSSVC_NEWSTRUCT, &nid);
	if (error) {
		info.retval = error;
		syslog(LOG_ERR, "Can't add user %s\n", pwd->pw_name);
	}
```

The `else` arm is the ordinary case for an id the server does not know:
it maps to `defaultuser` with a short timeout and answers the client.
`pwd` is NULL on it.  The error arm below then reads `pwd->pw_name`.

So the crash needs two things at once: a client naming an id or name
the server cannot resolve, and an `nfssvc(2)` that fails for that entry.
The first is entirely under a remote client's control.  The second is
not remote, but it is reachable — the id map is a fixed-size kernel
table, and `nfsrv_setupidhash()`/`nfssvc_idname()` return `ENOMEM` and
`EPERM` rather than succeeding unconditionally.  A daemon that dies
takes NFSv4 name mapping down for the whole machine until it is
restarted.

The fix is the value the function already computed: `nid.nid_name` is
set on both arms, holds exactly what the log line wants to say, and is
never NULL.  All four sites now use it.

Measured over `usr.bin/gencat` and `usr.sbin/nfsuserd`: 14 → 10, two
translation units OK and no ERROR either side.  The four that closed are
the four `nfsuserd` sites.

### gencat: `error()` ends in `exit()`, and the ten findings do not rest on it

`usr.bin/gencat` has ten `core.NullDereference` findings, all in
`getmsg()`, all on `*tptr++`.  `tptr` comes from `msg`, and `msg` comes
from `xmalloc()`/`xrealloc()`, which return the pointer they only reach
when it is not NULL:

```c
	static void *xrealloc(void *ptr, size_t size) {
		if ((ptr = realloc(ptr, size)) == NULL)
			NOMEM();		/* error("out of memory") */
		return (ptr);
	}
```

`error()` ends in `exit(1)` and was declared `static void
error(const char *);` — the shape that has accounted for most of this
sweep's real NULL findings.  It is now `__dead2`, and so is `usage()`.

It moved nothing: 10 before, 10 after.  The findings are not downstream
of `error()` at all.  They rest on `getmsg()`'s two file-static locals:

```c
	static char *msg = NULL;
	static long msglen = 0;
	...
	clen = strlen(cptr) + 1;
	if (clen > msglen) { ... msg = xrealloc(msg, clen); msglen = clen; }
	tptr = msg;
```

Analysed as its own entry point, `getmsg()` may be a second call, so
`msglen` is unknown; `clen > msglen` can be false; and `msg` is then
whatever it was — which, for the analyser, includes NULL.  The invariant
is real (`msglen` is only ever raised in the same statement that
assigns `msg`) and it is one the analyser cannot carry across calls.
The declarations stay because they are true, not because they paid.

## pkg(8): the bootstrap signature is checked against a pointer that is NULL

`verify_pubsignature()` takes the repository it is verifying for, and
starts by working out which public key to use:

```c
	const char *pubkey;
	...
	if (r != NULL) {
		if (r->pubkey == NULL) {
			warnx("No CONFIG_PUBKEY defined for %s", r->name);
			goto cleanup;
		}
		pubkey = r->pubkey;
	} else {
		if (config_string(PUBKEY, &pubkey) != 0) {
			warnx("No CONFIG_PUBKEY defined");
			goto cleanup;
		}
	}
```

The whole point of the `else` arm is that `r` may be NULL — and it is:
the bootstrap path calls `verify_pubsignature(fd_pkg, fd_sig, NULL)`.
Two hundred lines of the function later, the verification itself did
not use the local it had just gone to that trouble to compute:

```c
	printf("Verifying signature with public key %s.a.. ", r->pubkey);
	if (pkgsign_verify_data(sctx, data, datasz, r->pubkey, NULL, 0,
	    pk->sig, pk->siglen) == false) {
```

So `pkg bootstrap` against a repository configured with
`SIGNATURE_TYPE: PUBKEY` dereferences NULL where it means to check a
signature.  `pubkey` was written and never read, which is why no
warning fired.  Both sites now use it.

## tftp(1): the loop's exhaustion is tested through a variable the loop need not set

`setpeer0()` walks `getaddrinfo()`'s result list and breaks on the first
address it can bind:

```c
	for (res = res0; res; res = res->ai_next) {
		if (res->ai_addrlen > sizeof(peeraddr))
			continue;
		peer = socket(res->ai_family, res->ai_socktype,
		    res->ai_protocol);
		if (peer < 0) { cause = "socket"; continue; }
		...
		break;
	}

	if (peer < 0)
		warn("%s", cause);
	else {
		memcpy(&peer_sock, res->ai_addr, res->ai_addrlen);
```

The `else` arm reads `res`, so it needs the loop to have `break`ed —
but what it tests is `peer`.  `peer` is `static int peer;`, which is
**zero**, not `-1`, before the first connection, and the first arm of
the loop `continue`s without touching it.  Run off the end of `res0`
without ever reaching the `socket()` call and `peer` is still `>= 0`:
the `else` arm then reads `res->ai_addr` through a NULL `res` and hands
`memcpy` a length from the same place.

Reaching it needs every address in the list to have an `ai_addrlen`
larger than a `sockaddr_storage`, which today's `getaddrinfo()` will not
produce.  The defect is the test, not the arithmetic: `res == NULL ||
peer < 0` is what the `else` arm actually requires, and it costs
nothing.

## efivar(8): two more helpers that end in `err()`, and a gap in the lint

`breakdown_name()` splits a `guid-name` string from right to left:

```c
	cp = strrchr(name, '-');
	if (cp == NULL) {
		if (ocp != NULL)
			*ocp = '-';
		rep_errx(1, "Invalid guid in: %s", name);
	}
	if (ocp != NULL)
		*ocp = '-';
	*vname = cp + 1;
	*cp = '\0';
```

`rep_errx()` does not return — it is `if (quiet) exit(eval);` and then
`verrx()` — but it was declared `static void`, so the analyser walked
out of the `cp == NULL` arm and into `*cp = '\0'`.  `rep_err()`,
`rep_errx()` and `usage()` are all `__dead2` now.

`tools/verify/noreturn_check.py` had not reported them, and the reason
is worth keeping.  Both wrappers are varargs:

```c
	va_start(ap, fmt);
	verr(eval, fmt, ap);
	va_end(ap);
```

The lint looks at the body's last top-level statement, and that is
`va_end(ap)` — a call that does return.  `va_end()` after a call that
does not return is unreachable, so the lint now drops trailing
`va_end()` statements before deciding.  Two fixtures went in with it:
`report_va_end` (the shape above) and `quiet_va_end`, where the call
before the `va_end()` is `vwarn()` and does come back.

### And a correction to the ipfw floor

The `sz < sizeof(req)` check committed with the previous batch was
written as a refusal (`return;` / `return (EINVAL);`).  That is wrong
for `ipfw -n`: `do_get3()` answers `test_only` by returning 0 without
touching `req` at all, so `req.size` is legitimately zero there, and
refusing turned `ipfw -n internal iflist` from "print an empty list"
into `err(EX_OSERR, ...)`.  Both sites now clamp instead —
`if (sz < sizeof(req)) sz = sizeof(req);` — which removes the
zero-sized allocation and leaves `-n` printing exactly what it printed
before.

Measured over `sbin/ipfw`, `usr.bin/tftp`, `usr.sbin/pkg` and
`usr.sbin/efivar`: 17 → 14, twenty-one translation units OK and no
ERROR either side.  The three that closed are `tftp`'s `setpeer0()`,
`pkg`'s `verify_pubsignature()` and `efivar`'s `breakdown_name()`.

## column(1): a line of nothing but separators has no columns, and one is printed anyway

`input()` drops a line that is entirely whitespace:

```c
	for (p = buf; *p && iswspace(*p); ++p);
	if (!*p)
		continue;
```

`maketbl()` splits on `separator`, which `-s` sets and which need not be
whitespace at all:

```c
	for (p = *lp; wcschr(separator, *p); ++p)
		/* nothing */ ;
	for (coloff = 0; *p;) {
		...
	}
	if ((t->list = calloc(coloff, sizeof(*t->list))) == NULL)
		err(1, NULL);
	...
	for (t->cols = coloff; --coloff >= 0;)
```

So a line of nothing but separators survives `input()` — `:::` is not
whitespace — and then `maketbl()`'s skip loop walks it to the NUL before
the column loop starts.  `coloff` is zero, `t->list` is a `calloc(0)`,
and `t->cols` is zero.  The printing loop is

```c
	for (coloff = 0; coloff < t->cols - 1; ++coloff)
		(void)wprintf(L"%ls%*ls", t->list[coloff], ...);
	(void)wprintf(L"%ls\n", t->list[coloff]);
```

`t->cols - 1` is `-1`, so the loop does not run and `coloff` is still
zero — and the trailing print reads `t->list[0]` out of a zero-sized
allocation and hands `%ls` whatever it found, as a `wchar_t *`.

`printf 'a:b\n:::\n' | column -t -s:` is the whole reproducer.  A row
with no columns now prints an empty line.

## ipf(8): an expression made only of separators

`parseipfexpr()` has two defects three lines apart.

```c
	if (temp[strlen(temp) - 1] != ';') {
```

For an empty expression `strlen(temp)` is zero and this reads
`temp[-1]` — one byte before the `strdup()`ed buffer.  If that byte
happens to be `;`, parsing continues.

```c
	for (ops = strtok(temp, ";"); ops != NULL; ops = strtok(NULL, ";")) {
		...
		if (oplist == NULL)
			oplist = calloc(asize + 2, sizeof(int));
		...
	}
	free(temp);
	...
	for (i = asize; i > 0; i--)
		oplist[i] = oplist[i - 1];
	oplist[0] = asize + 2;
```

`oplist` is allocated by the *first operand*.  `expr ";"` passes the
last-character test and then yields no operands at all — `strtok` on a
string of only delimiters returns NULL immediately — so the loop never
runs, `oplist` is still NULL, and `oplist[0] = asize + 2` writes through
it.  The check goes in before `free(temp)`, so `parseerror` does not
free `temp` twice.

## dump(8): the block cache reads a failed mmap as a successful one

```c
	base = calloc(sizeof(Block), NBlocks);
	BlockHash = calloc(sizeof(Block *), HSize);
	DataBase = mmap(NULL, NBlocks * BlockSize,
			PROT_READ|PROT_WRITE, MAP_ANON, -1, 0);
	for (i = 0; i < NBlocks; ++i) {
		base[i].b_Data = DataBase + i * BlockSize;
```

None of the three was checked, and the loop that follows writes through
all three unconditionally — `NBlocks` is at least sixteen whenever the
cache is enabled at all, so this is not the count-allocate-fill shape
where the zero case saves it.

The `mmap` is the interesting one.  `cread()` decides whether the cache
has been initialised with

```c
	if (DataBase == NULL)
		cinit();
```

and `mmap` reports failure as `MAP_FAILED`, which is `(void *)-1`, not
NULL.  A failed mapping therefore reads back as a *successful* one: the
cache is never re-initialised, and every block is written through
`(char *)-1 + i * BlockSize`.  Both checks are in now, reporting through
`quit()`, which `dump.h` already declares `__dead2`.

`cread()`'s own finding — `blk = *ppblk` where `ppblk` is NULL if the
hash bucket is empty — survives, and reading it out says why it should:
`cinit()` fills bucket `i / HFACTOR` for every `i` in `[0, NBlocks)`, so
with `NBlocks == HSize * HFACTOR` every bucket in `[0, HSize)` holds
exactly `HFACTOR` blocks and none is empty.  That equality is not
checked anywhere; it holds because `cachesize` is a whole number of
megabytes and `BlockSize` is a power of two no larger than `MAXBSIZE`,
which makes `NBlocks` a multiple of sixteen.  Change any of those three
and both an empty bucket and a one-past-the-end write to
`BlockHash[HSize]` become reachable.

Measured over `usr.bin/column`, `sbin/dump` and `sbin/ipf`: 15 → 13,
164 translation units OK and the same 16 ERROR on both sides, all of
them already on the record.  The two that closed are `column`'s
`maketbl()` and `ipf`'s `parseipfexpr()`.

## A negative result: "the finding's function calls this helper" is not an oracle either

The `usage()`-ends-in-`exit()` class has paid repeatedly this sweep —
`ppp`'s `AbortProgram`, `dump`'s `quit`, `lpr`'s seven, `route6d`,
`chat`, `patch`, `efivar`'s `rep_err`/`rep_errx`.  Every one of those
came from reading a *finding* and walking back to the helper on its
path.

So the obvious next step was to automate exactly that walk: take every
function that carries a pointer finding, run `noreturn_check.py` over
its directory, and keep the pairs where the finding's function textually
calls an undeclared-noreturn helper defined in the same file.  Twelve
pairs came out; three were self-matches from the crude body extraction,
leaving nine real ones across `nvmecontrol`, `hexdump` (five helpers),
`m4`, `login`, `ministat`, `rtsold`, `traceroute`, `watchdogd` and
`makefs`.

All nine declarations went in.  Measured over those nine directories:
**28 → 28, no change at all**, ninety translation units and the same
single known ERROR on both sides.  Every one was reverted.

The lesson is sharper than the earlier file-level one.  It is not that
the lint is noisy — each of the nine statements is *true*, and the
compiler would accept them.  It is that "`f()` mentions `g()`" says
nothing about whether the analyser's path to the finding in `f()` runs
through `g()`.  In all the cases that paid, the guard and the use were
adjacent and the helper sat between them; here the helper is a `usage()`
called from an argument-parsing arm that the finding's path never
takes.  Reading the finding is not a step that can be skipped.

## bsdinstall: five searches whose result is used whether or not they found it

`gpart_ops.c` looks up a geom's partition scheme the same way six times:

```c
	LIST_FOREACH(gc, &pp->lg_geom->lg_config, lg_config) {
		if (strcmp(gc->lg_name, "scheme") == 0) {
			scheme = gc->lg_val;
			break;
		}
	}
```

`gpart_create()` and one arm of `gpart_edit()` write it correctly —
`scheme = NULL;` first, and then a check afterwards.  The other four
declared `const char *errstr, *scheme;` and used the result directly:

| function | what it feeds the uninitialised pointer to |
|---|---|
| `gpart_activate()` | `strcmp(scheme, "MBR")` |
| `gpart_bootcode()` | `bootcode_path(scheme)` |
| `gpart_partcode()` | `partcode_path(scheme, fstype)`, and `indexstr` into the `gpart bootcode` command line |
| `gpart_edit()` | `scheme_supports_labels(scheme)`, and `oldtype` into three `strcmp()`s after the dialog |

The loop finds nothing when the geom is not a `PART` geom, or is a
zombie — the case `gpart_edit()`'s other arm explicitly calls out
("Check for zombie geoms, treating them as blank").  Seven findings, one
shape, and the file already contained the correct idiom twice.

## nscd(8): a read error hands the same block a NULL function pointer

`process_socket_event()` handles the split-buffer path and then runs the
query state machine:

```c
		if (qstate->use_alternate_io != 0) {
			switch (qstate->io_buffer_filter) {
			case EVFILT_READ:
				io_res = query_socket_read(qstate, ...);
				if (io_res < 0) {
					qstate->use_alternate_io = 0;
					qstate->process_func = NULL;
				} else {
					...
				}
			break;
			...
			}
		}

		if (qstate->use_alternate_io == 0) {
			do {
				res = qstate->process_func(qstate);
			} while ((qstate->kevent_watermark == 0) &&
					(qstate->process_func != NULL) &&
					(res == 0));
```

The error arm sets *both* `use_alternate_io = 0` and
`process_func = NULL` — which is exactly the state the block below calls
`process_func` in.  The `do`/`while` tests `process_func != NULL` only
as a continuation condition, so the first call has already gone through
NULL by the time it is checked.  `query_socket_read()` returns negative
on a short read or a peer that went away, so a local client that closes
its socket part-way through a large answer crashes the daemon.

The guard belongs on the `if`, and the body already ends by setting
`process_func = NULL` when `res != 0`, so skipping the block when it is
already NULL is exactly the intent.

Measured over `usr.sbin/bsdinstall` and `usr.sbin/nscd`: 25 → 17,
thirty-one translation units OK and no ERROR either side.  All eight
that closed are the eight fixed here.

## mail(1): `set append` and a failed write close a `FILE *` that was never opened

`quit()` writes the saved messages back to the mailbox one of two ways:

```c
	if (value("append") == NULL) {
		... obuf = Fdopen(fd, "w") ...
		... ibuf = Fopen(tempname, "r") ...
		... obuf = Fopen(mbox, "r+") ...
	}
	if (value("append") != NULL) {
		if ((obuf = Fopen(mbox, "a")) == NULL) { ... }
	}
	for (mp = &message[0]; mp < &message[msgCount]; mp++)
		if (mp->m_flag & MBOX)
			if (sendmessage(mp, obuf, saveignore, NULL) < 0) {
				warnx("%s", mbox);
				(void)Fclose(ibuf);
```

`ibuf` is the temporary holding the old mailbox contents, and it is
opened only on the first path.  The two later uses of it are correctly
wrapped in `if (value("append") == NULL)`.  This one is not: with
`set append` in `~/.mailrc` and a `sendmessage()` that fails — a full
disk, a quota, an I/O error — `Fclose()` is handed an indeterminate
`FILE *`.

`ibuf` starts at NULL now and the close asks the pointer.  The copy-back
below asked `value()` the same question a second time; it asks `ibuf`
instead, which is what it actually depends on and does not rest on an
opaque call answering the same way twice.

## pfctl(8): five `ENTRY`s passed to `hsearch_r()` with half of them unwritten

`hsearch_r()` takes its `ENTRY` **by value**, so a `FIND` copies both
`key` and `data`.  Five lookups fill in only `key`:

```c
	ENTRY	 item;
	ENTRY	*ret_item;

	item.key = ifname;
	if (hsearch_r(item, FIND, &ret_item, &if_map) == 0)
		return (NULL);
```

`pfaltq_lookup()`, `qname_to_pfaltq()`, `qname_to_qid()`,
`ifa_add_groups_to_map()` and `is_a_group()`.  The implementation does
not look at `data` for a `FIND`, so nothing observable goes wrong — but
copying an indeterminate member is a read of an indeterminate object,
and the fix is `= { NULL, NULL }` in five declarations.

Measured over `usr.bin/mail` and `sbin/pfctl`: 27 → 21, thirty-five
translation units OK and no ERROR either side.  Seven closed; one
appeared — `host_if(): Potential memory leak` in `pfctl_parser.c`, in a
function this change does not touch.  Reading it: `ps` is `strdup`ed and
freed on every path through the `error:` label, and `h` is the return
value.  It is the analyser's per-translation-unit path budget being
spent differently once five paths stop being explored, not a new defect
— the kind of thing a raw before/after count hides and a
`sweep_diff.py` keyed on `(file, checker, function, message)` shows.

## ktrdump(1) and devmatch(8): three varargs that were never written

`ktrdump` parses each record's format string to work out how many
parameters it names, filling `parms[0..parm)` as it goes, and then
prints with

```c
	fprintf(out, desc, parms[0], parms[1], parms[2], parms[3],
	    parms[4], parms[5]);
```

`KTR_PARMS` is six.  A record whose format names fewer than six
conversions leaves the tail of `parms` unwritten, and passes it as
varargs anyway.  `fprintf` reads only as many as `desc` names, so
nothing is printed from them — but they are still read to be passed.
`parms` is declared once, outside the record loop, so the values are
also whatever the previous record left there.  `= { 0 }` on the
declaration.

`devmatch -v` prints a line for every table entry it skips, and the
integer branch's version of it is

```c
			if (cp[2] == '#') {
				if (verbose_flag) {
					printf("Ignoring %s (%c) table=%#x "
					    "tomatch=%#x\n", cp + 2, *cp, v,
					    ival);
				}
				break;
			}
			v = pnpval_as_int(cp + 2, pnpinfo);
```

`v` is fetched on the line *after* the message.  The string branch has
a copy of the same message, and there `v` is the integer branch's
variable, which that path never assigns at all.  Both messages now print
what they actually have: the integer branch's `ival`, and the string
branch's `val1`.

Measured over `usr.bin/ktrdump` and `sbin/devmatch`: 6 → 2, two
translation units OK and no ERROR either side.  All four that closed are
the four fixed here.

## mptutil(8): eleven more error tests that can never fire

The `mpt_lookup_standalone_disk()` half of this was found and fixed
earlier in the sweep; the same convention runs through the rest of the
program and the same mistake is made everywhere it does.

`mpt_raid_action()` returns `0` or a **positive** `errno`:

```c
	if (ioctl(fd, MPTIO_RAID_ACTION, &raid_act) < 0)
		return (errno);
	...
		return (EIO);
```

and `mpt_lock_volume()`, `mpt_lock_physdisk()`, `mpt_create_physdisk()`,
`mpt_delete_physdisk()` and `mpt_lookup_drive()` all pass that
convention on.  Every call site tested `< 0`:

```c
	if (mpt_create_physdisk(fd, dinfo->sdisk, &PhysDiskNum) < 0) {
		error = errno;
		warn("Failed to create physical disk page for %s", ...);
		return (error);
	}
	...
	dinfo->info = mpt_pd_info(fd, PhysDiskNum, NULL);
```

Nothing in the file can return a negative value, so none of those arms
is reachable.  A physdisk page that the controller refused to create is
reported as created, `PhysDiskNum` is never written, and the
uninitialised value goes straight to `mpt_pd_info()` — which is what
the three `core.CallAndMessage` findings are.  The arms themselves are
wrong twice over: had they fired they would have read `errno`, which
nothing on that path sets, and thrown away the value the callee
returned.

Eleven sites in `mpt_config.c` and one in `mpt_drive.c` now capture the
return and test `!= 0`, and report with `warnc(error, ...)` — which
`mpt_lock_volume()` in the same file already uses.

Making `drive_set_state()`'s arm reachable immediately exposed a leak in
it: the error path returns without `mpt_free_pd_list(list)`, which the
success path three lines below does.  Fixed with it, and it is worth
noticing how it surfaced — a dead branch hides everything downstream of
it from the analyser too.

Measured over `usr.sbin/mptutil`: 9 → 6, eight translation units OK and
no ERROR either side.

## The leak class, surveyed rather than closed

176 of the shard's `unix.Malloc` findings are leaks.  They are spread
across 60-odd directories with no dense cluster, and the shape that
dominates them is **ownership leaving the function** by a route the
analyser does not follow.  Three read in full, one from each route:

* `rtadvd`'s `nd6_options()` — `nol` is `malloc`ed and immediately
  `TAILQ_INSERT_TAIL`ed into `ndopts->opt_list`.  The `bad:` label calls
  `free_ndopts()`, and both callers `goto done` to a `free_ndopts()` of
  their own on every exit.  Nothing leaks.
* `dhclient`'s `parse_option_buffer()` — `t` is `calloc`ed and stored as
  `packet->options[code].data`, replacing what was there.  The packet is
  the caller's.
* `ppp`'s `bundle_ReceiveDatalink()` — and this one **is** a leak, which
  is why it is fixed here:

```c
  for (f = expect = 0; f < niov; f++) {
    if ((iov[f].iov_base = malloc(iov[f].iov_len)) == NULL) {
      log_Printf(LogERROR, "Cannot allocate space to receive link\n");
      return;
    }
```

  The segments already allocated, `iov[0..f)`, are dropped.  This runs on
  every link handover, and the one case that reaches it is the one where
  holding onto them hurts most.  `usr.sbin/ppp`: 45 → 44.

A fourth is worth recording as *not* a defect for a reason particular to
its program.  `autofs`'s `parse_map_yyin()` drops `key` and `options` at
two points without freeing them — but `node_delete()` frees only the
node, never `n_key` or `n_options`, so `automountd` does not free those
strings anywhere.  The two drops are consistent with the file's design;
fixing them alone would not be.

A textual classifier was written to sort the 176 by which route the
pointer leaves through, and it is not reported here: its function-body
extraction is too crude for the numbers to be worth anything.  What the
reading supports is the shape, not a count.

## The sweep tooling filled the disk

`includes.py`'s `incs_shim()` builds `/usr/include` as a tree of
symlinks and returns the path, which goes on `-I`.  It has to outlive
the call, so it cannot be a `with tempfile.TemporaryDirectory()` — and
it was not removed at all.  One directory per sweep process
accumulated: **1,756 of them, 16GB**, which is how this container ran
out of disk four times in one session.  A sweep that dies on `ENOSPC`
reports nothing, which is the same failure mode `--check-errors` exists
to catch one file at a time.

Three changes, in order of how much they do:

1. `analyze.py` builds the shim in the **parent**, once per
   architecture the job list needs, and `incs_shim()` hands the path
   down through the environment.  Workers inherit it instead of each
   building — and leaving behind — its own.  It is strictly less work
   than before: the same set, built once.
2. `atexit` (and a `multiprocessing.util.Finalize`, for workers that
   leave through `util._exit_function` rather than `atexit`) removes
   what this process owns.
3. Anything that still escapes — a killed process runs neither — is
   reaped on the next run, at 24 hours old, which is far longer than
   any sweep.

Measured: a `usr.sbin/ppp` sweep left four directories before and zero
after, with the finding, OK and ERROR counts unchanged.

## chat(8): a static cursor into a stack buffer that is about to go away

`do_file()` reads the chat script a line at a time into `char buf[STR_LEN]`
and hands pointers into it to `chat_expect()` and `chat_send()`.  Both
tokenise through

```c
	char *
	expect_strtok (char *s, const char *term)
	{
	    static  char *str   = blank;
	    ...
	    if (s)
		str = s;
```

— a `strtok`-alike with a **static** cursor into whatever string it was
last handed.  When `do_file()` returns, `str` points into a dead frame.

Nothing resumes it today: every `expect_strtok(NULL, ...)` continuation
happens inside the call that started the sequence, and `main()` runs
either `do_file()` or the command-line script, never both.  The storage
is what makes that not matter, so `buf` is `static` now — one word,
`do_file()` is neither recursive nor threaded, and the cursor cannot
dangle.

`gstat(8)` is a smaller version of the same shape.  `int ... max_flen,
head_printed;` is uninitialised, and written only by the `-C` arm of
`getopt`; it is read only under `flag_C && !head_printed`, which is the
same condition, so nothing goes wrong.  The relation is stated nowhere
and costs one initialiser to remove.

Measured over `usr.bin/chat` and `usr.sbin/gstat`: 4 → 2, two
translation units OK and no ERROR either side.  Both singletons closed.

## virtual_oss(8): eleven divisions and one invariant, established two functions away

All eleven of the shard's `core.DivideZero` findings in
`virtual_oss/main.c` divide by the same product:

```c
	mod = pvc->channels * vclient_sample_bytes(pvc);
```

and `vclient_sample_bytes()` returns **0** for a format that names no
bit width — which is what a zeroed `vclient_t` holds.

Read all the way through, the program is safe, and it is worth being
precise about why rather than claiming a crash it does not have.
`vclient_open()` allocates the client, calls `vclient_setup_buffers()`
with the profile's channel count and `vclient_get_default_fmt()` — which
returns a real format on every arm — and **fails the open** if setup
returns an error.  Setup rejects a zero format and a channel count
outside `[1, profile->channels]`.  So a client that exists at all has a
non-zero `mod`, and none of the eleven divisions can be reached with a
zero divisor.

Two things are still wrong with how that is written, and both are fixed:

* `vclient_setup_buffers()` does its `size % mod` arithmetic **before**
  the sanity checks that reject a zero `mod`.  The checks are twelve
  lines further down, in a block the file labels `/* sanity checks */`.
  An ordering like that is safe only by accident of which callers exist;
  the check now precedes the arithmetic it protects.
* `SNDCTL_DSP_CURRENT_IPTR` and `_OPTR` divide by the product with no
  check at all, resting on the open-path invariant from two functions
  away.  Both compute it once into `temp`, refuse zero, and divide by
  that.

Measured over `usr.sbin/virtual_oss`: 11 → 7, twelve translation units
OK and the same one known ERROR on both sides.  The seven that remain —
`vclient_output_delay()`, `vclient_export_read_locked()` twice,
`vclient_import_write_locked()` three times and `vclient_scale()` — are
the same invariant seen from the data path, and guarding those would put
a branch per sample on the audio path to state something the
configuration boundary already guarantees.  They stay, characterised.

### mptutil's CAM helpers: six allocations written through unchecked

`fetch_path_id()`, `mpt_query_disk()` and `mpt_fetch_disks()` each set up
an `XPT_DEV_MATCH` the same way:

```c
	ccb.cdm.matches = calloc(1, bufsize);
	...
	ccb.cdm.patterns = calloc(1, bufsize);

	ccb.cdm.patterns[0].type = DEV_MATCH_BUS;
```

`patterns[0]` is written on the next line and `matches[]` is read after
the ioctl.  None of the six was checked.  Not the count-allocate-fill
shape, where a zero count also skips the writes: these are fixed sizes
and the writes are unconditional.

It moved nothing — `usr.sbin/mptutil` measured 6 → 6, eight translation
units OK and no ERROR either side, and it was not expected to: the
findings in that file are `path_id`, an out-parameter the analyser will
not follow into `fetch_path_id()`.  Recorded here for the same reason
the `ps`, `kbdmap` and `bsdinstall` checks earlier in the sweep were: an
allocation dereferenced in the next statement is a defect whether or not
a checker says so.

## The four uninitialised-ioctl-buffer findings that are not defects

`ifconfig`'s `ifgif.c` and `ifgre.c` account for four
`core.uninitialized.Assign` findings between them, all of this shape:

```c
	int opts;
	struct ifreq ifr = { .ifr_data = (caddr_t)&opts };

	if (ioctl_ctx_ifr(ctx, GIFGOPTS, &ifr) == -1)
		return;
	if (opts == 0)
		return;
```

The kernel fills `opts` **through a pointer stored in a structure
member**, which is one indirection further than the out-parameter cases
the analyser already loses.  Initialising `opts` would silence it and
would also hide a genuinely unfilled ioctl, which is the opposite of
what this sweep is for.  Characterised, not changed.

`mptutil`'s `mpt_cam.c` findings are the same class one level up:
`path_id` is written by `fetch_path_id()` on its success path, and all
three callers test `error` correctly.

## iwmbtfw(8): two hardware variants fall through both arms

`iwmbt_init_firmware()` works out how much of the `.sfi` file is header
before handing the rest to the adapter:

```c
	int header_len, ret = -1;
	...
	if (hw_variant <= 0x14) {
		header_len = RSA_HEADER_LEN;
		...
	} else if (hw_variant >= 0x17) {
		header_len = ECDSA_OFFSET + ECDSA_HEADER_LEN;
		...
	}

	/* Load in the CSS header */
	if (sbe_type == 0x00)
		ret = iwmbt_load_rsa_header(hdl, &fw);
	else if (sbe_type == 0x01)
		ret = iwmbt_load_ecdsa_header(hdl, &fw);
	if (ret < 0)
		goto exit;

	/* Load in the Command Buffer */
	ret = iwmbt_load_fwfile(hdl, &fw, boot_param, header_len);
```

`<= 0x14` and `>= 0x17` leave **0x15 and 0x16** falling through both
arms.  `header_len` is then never written.  `ret`'s initial `-1` does
not save it either: the `sbe_type` block below overwrites `ret` on
success, so a device reporting one of those two variants with a valid
`sbe_type` reaches `iwmbt_load_fwfile()` with a garbage header length —
which is how many bytes of the firmware image that function skips before
sending the rest to the adapter.

The `hw_variant` comes from the device's own version reply.  The caller
does validate `sbe_type` (`if (vt.sbe_type > 0x01)`); it does not
validate `hw_variant`.  There is an explicit `else` now, and it refuses.

Measured over `usr.sbin/bluetooth`: 10 → 9, seventy-three translation
units OK and no ERROR either side.

## crunchide(1): a section header size the file gets to choose

```c
	shnum = xe16toh(ehdr.e_shnum);

	shdrsize = shnum * xe16toh(ehdr.e_shentsize);
	if ((shdrp = xmalloc(shdrsize, fn, "section header table")) == NULL)
		goto bad;
	if (xreadatoff(fd, shdrp, xewtoh(ehdr.e_shoff), shdrsize, fn) !=
	    shdrsize)
		goto bad;
	...
	for (i = 0; i < shnum; i++) {
		switch (xe32toh(shdrp[i].sh_type)) {
```

The allocation is `shnum * e_shentsize` **bytes**; the indexing is
`shdrp[i]` as an `Elf_Shdr[]`, which needs `shnum * sizeof(Elf_Shdr)`.
Both numbers come out of the object file, and nothing requires them to
agree.  A file declaring an `e_shentsize` smaller than the header for
its class makes every `shdrp[i]` past the first read past the
allocation — and `crunchide` then writes the file back based on what it
read.

The product is also computed in `int`: `e_shnum` and `e_shentsize` are
both `Elf_Half`, and at their maximum the product is 4,294,836,225,
which overflows a signed 32-bit `int`.  A negative `shdrsize` then goes
to `xmalloc()`.

The ELF specification fixes `e_shentsize` at the size of the section
header for the file's class, and `crunchide` is compiled once per
class, so requiring equality is exactly right.  The multiplication is
done in `ssize_t` against `sizeof(Elf_Shdr)`.

It moved nothing: `usr.sbin/crunch` measured 7 → 7, four translation
units OK and the same one known ERROR.  The analyser's finding on
`shdrp[i]` is the generic "buffer filled by a call I cannot follow"
shape, not this path — the defect is visible by reading, not by the
checker, which is the second time in this sweep that has been true of
something worth fixing.

## The progs shard, closed

Opening and closing sweeps over `bin`, `sbin`, `usr.bin` and `usr.sbin`,
same tree layout, same scope, `--check-errors` on both sides:

```
before   595 finding(s)  OK 1822  ERROR 40
after    557 finding(s)  OK 1822  ERROR 40
```

Thirty-nine closed, one appeared — `pfctl_parser.c`'s `host_if()`, read
and characterised above as the per-translation-unit path budget being
spent differently.  The translation-unit counts are identical on both
sides, and all forty ERRORs are the ones already on the record, so the
two numbers are the same measurement.

By class:

| checker | before | after |
|---|---|---|
| `unix.Malloc` | 199 | 199 |
| `core.NullDereference` | 191 | 182 |
| `core.CallAndMessage` | 72 | 49 |
| `core.UndefinedBinaryOperatorResult` | 40 | 40 |
| `unix.cstring.NullArg` | 33 | 33 |
| `core.uninitialized.Assign` | 22 | 22 |
| `core.DivideZero` | 21 | 17 |
| `unix.MallocSizeof` | 12 | 12 |
| `core.uninitialized.Branch` | 2 | 1 |
| `core.StackAddressEscape` | 1 | 0 |

`unix.Malloc` is flat because the one leak fixed in it (`ppp`) is offset
by the one that appeared, and because the earlier `gprof`, `ipsend` and
`ipfw` fixes landed before this pair of sweeps.

What the whole shard came to, over its full reading: the classes that
paid were **a helper that exits and a declaration that does not say so**,
**a variable written in some arms of a chain and read after it**, **an
error test that cannot fire**, and **a check that runs after the
arithmetic it protects**.  The classes that did not were every attempt
to find those shapes by a predicate over the source instead of by
reading a finding — twice, measured, and reverted both times.

## fusebmc at scale: the reach number, and the two crashes that were not

The third engine was built to fuzz what the other two can only reason
about.  Driving it over the whole tree — 114,207 (translation unit,
function) pairs from the port ledger — produced two results worth
keeping, and one of them is a correction to the tool.

### The false CRASH

The first run of it at scale returned four CRASHes.  Two of them were
`_Exit` and `quick_exit`.

Neither is a defect.  Both do exactly what their names say, and the
reason the fuzzer said otherwise is structural: AFL detects a crash by
the child failing to come back to the fork server, and a child that
called `_Exit(1)` does not come back either.  A tool whose whole claim
is *a crash is a certainty* cannot carry a fifty per cent false rate on
the one status that matters.

The fix is a new status rather than a filter, because "this function
ends the process" is an answer, not a suppression:

```
  NORETURN    the function ends the process rather than returning.
```

The predicate is `noreturn_check.py`'s, reused rather than rewritten —
the seed set of libc names that do not return, propagated through
functions whose last top-level statement calls one of them, plus
anything a header declares `__dead2`.  The seed replay was tightened at
the same time, from "the process exited non-zero" to `returncode < 0`,
which is a signal; UBSan exits 1 by default, and the two were being
conflated.

The other two CRASHes survived it, and both are the same real defect:
`abs(INT_MIN)` and `imaxabs(INTMAX_MIN)`, where negating the most
negative value of the type is undefined.  One was found by the CBMC
seed and one by AFL from that seed, which is the method working as
advertised — the model checker produced the witness, the fuzzer
confirmed it by running it.

### The reach number

`--dry-run` classifies without spending a budget: `parse_params()` is
the whole gate and it costs a regex, so the reach of the engine over
114,207 pairs is a minute rather than a year.

```
ERROR=101357  NOFUNC=1200  RUNNABLE=11650
```

`RUNNABLE` is a **ceiling**, not a result.  It says the harness would
build, not that anything was run, and not that anything was proved.

The ERROR histogram is the actual finding:

| why | pairs |
|---|---|
| a pointer, array or varargs parameter | 83,894 |
| a pointer parameter, without inventing a size | 12,944 |
| `SYSCTL_HANDLER_ARGS` — a macro standing for a parameter list | 1,140 |
| `PFS_FILL_ARGS` — likewise | 96 |
| a long tail of driver-specific handles (`ofw_t`, `bus_space_tag_t`, `KBDC`, …) | 3,283 |

96,838 of 101,357 — **95.5%** — are the pointer answer, and it is not a
gap to be closed by trying harder.  `cbmc_driver.py` runs pointer
parameters under an explicit stated precondition (`--min-null-tree-depth`)
and records the assumption in the result; there is no honest fuzzing
equivalent, because a fuzzer given a pointer either passes `NULL` — and
reports the absence of a caller's contract as a crash — or passes a
buffer of an invented size, and reports the invention.  Both are
findings about the harness.

`load_tasks()` therefore does **not** intersect the ledger with a goto
model the way `cbmc_driver.py` does.  A function this engine cannot
synthesise parameters for is an answer with a reason attached, not a
task to drop before counting.  Drop them silently and the coverage
number stops meaning anything.

### What the histogram bought

Two of its buckets were the tool being wrong about itself, and both were
only visible because the histogram is bucketed on the *reason*:

* **148 parameters read as a type named `int signo`.**  `int signo
  __unused` — taking the last token for the name makes the rest the
  type.  The reason printed, "parameter type `int signo` is not a known
  scalar", was a true statement about the table and a false one about
  the code.
* **9,559 pairs reported as "not a known scalar" for `device_t`,
  `if_t`, `node_p` and friends.**  All of them are pointer typedefs.
  Saying "a pointer" says why they cannot be reached; saying "not in my
  table" says nothing.

Widening the scalar table over the types the histogram named — the BSD
spellings, the POSIX typedefs, the address-sized integers, the
Linux-compat aliases, `_Bool` and the floating types — took RUNNABLE
from 10,723 to 11,650.  `_Bool` is normalised rather than `memcpy`d into
(`(buf[0] & 1) != 0`): an arbitrary byte in a `_Bool` can be a trap
representation, and reading it back would make the harness the bug.

`tools/verify/test_fusebmc.py` holds all of it — 25 cases, every one a
mistake this engine actually made.

### Why it is not a gate

`.github/workflows/pbsd-fusebmc.yml` runs nightly and is report-only:
nothing `needs:` it, every job is `continue-on-error`, and the output is
an artifact and a step summary a person reads.

A gate wants a signal that only moves when the code moves.  AFL's does
not.  The same function, the same budget and a different runner is a
different number of executions, and a crash found on the ninth run of an
unchanged tree is a true finding that would have failed the eight runs
before it for no reason anybody could act on.  The asymmetry is what
makes it worth running anyway: `CLEAN` is worth nothing and is never
reported as if it were, and `CRASH` is a concrete input and a signal.

## libcalendar: `weekday()` overflows for the bottom 729,652 of its domain

`weekday(int nd)` caches the day number of a known Monday — 729,652 —
and reduces its argument against it:

```c
	nd = (nd - nmonday) % 7;
```

For every `nd` below `INT_MIN + 729652` that subtraction overflows,
which is undefined and not merely wrong.  It is a public `libcalendar`
entry point with no stated domain, so that is its whole `int` range; the
only in-tree caller, `firstday()`, passes an `ndaysgi()` result and stays
well inside it, which is why nothing had noticed.

Widened, exactly:

```c
	nd = (int)(((long long)nd - nmonday) % 7);
```

`long long` holds every `int` difference and the result still fits an
`int`, so nothing in range changes.

Found by the fuzzing engine, not by the analyser or the model checker —
AFL produced `nd = 0x8000550b`, the replay came back
`UndefinedBehaviorSanitizer: undefined-behavior calendar.c:283:11`, and
the same function is `CLEAN` on the same budget after the fix.

## The first fusebmc run at scale: three of five crashes were the instrument

4,390 (translation unit, function) pairs over `lib/libc` and `lib/msun`,
`--jobs 4 --budget 20`.  The opening run reported five CRASHes.  Read one
at a time, they were:

| function | verdict |
|---|---|
| `abs` | real — `abs(INT_MIN)`, documented C UB |
| `imaxabs` | real — the same at `intmax_t` |
| `weekday` | real — fixed above |
| `valloc` | the sanitizer's policy, not the code |
| `memalign` | the sanitizer's own internal assertion |

Two were the instrument, and neither is a rounding error: a tool whose
entire claim is *a crash is a certainty* was running at a 40% false rate
on the one status that carries the claim.

**`valloc(0x8700000000)`.**  ASan's allocator *aborts* on a request it
considers absurd, and "absurd" is exactly what a fuzzer feeds a function
whose only parameter is a size.  The C library returns `NULL` with
`ENOMEM` there.  `allocator_may_return_null=1` restores the library's
own answer, and a function that then *uses* that `NULL` still crashes —
which is the finding worth having.  `valloc` is `CLEAN` with it set.

**`memalign(1 << 63, 0)`.**  Not a report at all:

```
AddressSanitizer: CHECK failed: asan_allocator.cpp:601
"((user_end)) <= ((alloc_end))" (0x8000000000000001, 0x502000000020)
```

That is ASan's own arithmetic overflowing, and it says nothing about
the code beneath it.  It gets its own status, `SANFAIL`, rather than
being dropped: it is a real limit on what this engine can see, and a
limit that is not counted is a limit nobody knows about.

Classifying it needed one more correction.  `_tail()` is right for a
compiler, which puts its verdict last, and exactly wrong for a
sanitizer, which puts the verdict *first* and forty stack frames after
it — so the first attempt kept 600 characters that contained only frame
numbers, and `memalign` came back CRASH a second time with the line that
classifies it three screens above the cut.  `evidence()` takes the head
when there is a sanitizer verdict in it and falls back to the tail
otherwise.

A fuzzer-found CRASH also recorded only AFL's saved input and an empty
`detail`, so every one of them had to be re-run by hand to find out what
it was.  It is now replayed under the sanitizers and the report kept —
one execution, and the difference between a finding you can read and a
hex string.

## The include path the two halves of the hybrid do not share

`cbmc_seed()` was being run with no `-I` at all.  Over the same 4,390
pairs, 854 came back `NOSEED` — CBMC could not be run, so the fuzzer got
no smart seed and was never started — and the top reasons were
`namespace.h: No such file or directory` (201), `sys/_types.h` (137) and
`fpmath.h` (112).  **All three headers are in this tree.**  Nothing was
wrong with the code; the instrument was reading it through a Linux
host's include path.

`classify.py` has computed the right answer per file since the
beginning, for `goto-cc`.  It is the same answer for `cbmc`, so it is
asked for the same way rather than approximated a third time.  Two
details made it work:

* **`cbmc` is not a compiler driver.**  It takes `-I` and `-D` and
  rejects everything else outright — `Unknown option: -U__linux__`,
  then a page of usage, then exit.  The engine read that page as the
  reason for `NOSEED`, which is a tool reporting its own misuse as a
  property of the code.
* **`-include foo.h` is two argv elements.**  A filter written as a
  comprehension over the first character keeps the `-include` and drops
  the header, which is worse than dropping both: the next flag silently
  becomes its argument.

`NOSEED` fell from 854 to 240.

### What that did NOT do

`ERROR` rose from 3,412 to 4,026 by almost exactly the same amount, and
`CLEAN` went from 19 to 21.  The 614 pairs CBMC can now read fail one
stage later instead, at the harness compile, on the same headers.

That is a better diagnosis, not more fuzzing, and it is worth being
plain about which.  The harness compile deliberately keeps the **host**
include path, and the reason is not oversight:

* CBMC **reads** the code, so the tree's headers are strictly better for
  it.
* The fuzzer **runs** the code, on Linux, linked against glibc.

Compiling the unit with the build's own flags was tried.  Those flags
carry `--target=x86_64-unknown-freebsd15.0`; the object comes out with a
FreeBSD ABI, and linking it into a Linux binary produced three CRASHes
that were all harness — `valloc(0)` and `weekday(729652)` both SEGV'd
before executing a line of their own code.  Adding only the tree's `-I`
without the target is no better: those paths shadow the host's headers,
so `abs` and `weekday`, which build and run today, stop linking.

So the fuzzing half's reach stays bounded by what compiles and links
against the host's C library, and that bound is reported as `ERROR` in
the linker's own words rather than papered over:

| why ERROR | pairs |
|---|---|
| a pointer, array or varargs parameter | 2,910 |
| a type not in the scalar table | 484 |
| harness compile: a tree header the host path lacks | 482 |
| harness compile: other | 142 |
| harness link: needs the rest of the library | 5 |

### A widening that loses a finding is not a widening

The include path is a large net win and not a pure one.  It also hands
CBMC's C front end headers CBMC cannot parse, and `imaxabs.cpp` went
from a real counterexample to `parse error before '__char16_t'` the
moment `sys/_types.h` became reachable — a genuine defect lost to an
improvement.  Two things fix it:

* For a landed `.cpp` port read as C, the libc++ shim is dropped.
  `include_flags()` leads every `.cpp` with it, which is right for a C++
  compile and fatal for a C one: `<__config>` `#error`s by name.
* Where the flagged run cannot even parse, the bare run it used to do is
  tried instead, and whichever answers is kept.

`imaxabs` is back to CRASH on the real `imaxabs(INTMAX_MIN)`.

### The pair, side by side

Same tree layout, same scope, same budget:

```
before   CLEAN 19  CRASH 5  SANFAIL 0  NOSEED 854  ERROR 3412  NOFUNC 89  NORETURN 11
after    CLEAN 21  CRASH 2  SANFAIL 1  NOSEED 240  ERROR 4026  NOFUNC 89  NORETURN 11
```

Both CRASHes that remain are real and both are the same documented UB:
negating the most negative value of the type.  Of the three that went,
one was fixed in the tree and two were the instrument.

## systat(1): the index check that guards one end of a range and says it guards both

`get_tbl_ptr()` in `usr.bin/systat/convtbl.c` opens with a comment
stating exactly the right invariant, and a test that implements half of
it:

```c
	/* If our index is out of range, default to auto-scaling. */
	idx = scale < SC_AUTO ? scale : SC_AUTO;
	...
	return (&convtbl[idx]);
```

`SC_AUTO` is the last member of `enum scale` and `convtbl[]` is indexed
by that enum, so the upper bound is right.  There is no lower one.  A
negative `scale` passes the test unchanged and `&convtbl[idx]` is then a
wild pointer that both callers dereference at once — `convert()` reads
`tp->mul` and `tp->scale`, `get_string()` reads `tp->str` and returns it.

The one in-tree caller does check.  `ifcmds.c` has

```c
	if ((scale = get_scale(args)) != -1)
```

and `get_scale()`'s `-1` is the only negative that arises, so nothing
reaches the table out of range today.  But `convert()` and
`get_string()` are declared in `convtbl.h` taking a plain `int`, and
this line is the place that range is meant to be enforced — the comment
says so.  Bounded at both ends:

```c
	idx = scale >= SC_BYTE && scale < SC_AUTO ? scale : SC_AUTO;
```

Found by CBMC's counterexample, replayed: `convert(0, -32758)` and
`get_string(0, INT_MIN + 9)`.  **The analyser has no finding here** —
`usr.bin/systat` measures 5 → 5 across 26 translation units, no `ERROR`
either side — and that is the point of running a third engine.  Both
functions are `CLEAN` on the same budget after.

## bsdinstall: a function whose job is to die by a signal

`reproduce_signal_death()` sets the handler back to `SIG_DFL`, turns off
core dumps, and calls `kill(getpid(), sig)`.  It was reported `CRASH` on
`SIGILL`, which is the function doing precisely what its name says.

`terminates_process()` already covered the `exit()` family through
`noreturn_check.py`, and could not cover this one: neither `raise()` nor
`kill()` is `__dead2`, because both return when the signal is blocked or
ignored.  So nothing in the C declarations says what the function does,
and the answer has to come from the last statement of the body — a call
to `raise()` or `abort()`, or a `kill()` whose target is `getpid()`.
`kill(child, sig)` is deliberately not the same answer.

It is `NORETURN` now, which is a status this engine already had for
exactly this reason.

## The progs scopes, first pass

17,838 (translation unit, function) pairs over `bin`, `sbin`, `usr.bin`
and `usr.sbin`, `--jobs 4 --budget 20`:

```
CLEAN 14  CRASH 3  ERROR 16385  NOFUNC 132  NORETURN 623  NOSEED 681
```

Three CRASHes, all read: two are the `convtbl` bound above and the third
is `reproduce_signal_death`.  Fourteen `CLEAN`, which is worth nothing
and is never reported as if it were.

The shape of the 16,385:

| why ERROR | pairs |
|---|---|
| a pointer, array or varargs parameter | 13,394 |
| harness compile: a header the host path lacks | 1,920 |
| harness compile: other | 450 |
| a type not in the scalar table | 448 |
| harness link: needs the rest of the program | 160 |

`623 NORETURN` is the number that says most about these scopes as
opposed to `lib/libc`: a program is full of `usage()`, `errx()` and
their local wrappers, and every one of them is a function a fuzzer
cannot distinguish from a crash.  Eleven in `lib/libc` and `lib/msun`;
623 here.

The 160 link failures are the honest bound on this engine in a program
directory.  A function in `sbin/ipfw/tables.c` is compiled and linked
*with the rest of ipfw*; on its own it names symbols that live in the
other twenty translation units, and the harness gets the linker's own
words rather than a number that pretends otherwise.

### The progs scopes, closed

Same tree layout, same scope, same budget, after the two fixes and the
self-kill recognition:

```
before   CLEAN 14  CRASH 3  SANFAIL 0  NOSEED 681  ERROR 16385  NOFUNC 132  NORETURN 623
after    CLEAN 15  CRASH 0  SANFAIL 0  NOSEED 684  ERROR 16379  NOFUNC 132  NORETURN 628
```

Zero CRASH: two closed in the tree, one reclassified as what it always
was.  `NORETURN` 623 → 628 is the five functions the `kill(getpid(), …)`
rule now recognises.

## What the ERROR histogram named next

Bucketing the reason rather than the text is what made the last three
corrections visible, so the tail was read again.  932 pairs across the
libs and progs runs came back "parameter type `X` is not a known
scalar", over **244 distinct type names** — and the shape of that tail
is three answers, not 244:

* **An `enum` is an integer type in C**, whatever its tag is.  23 of them
  (`enum ev_type` and its neighbours) were reported as an unknown type,
  which is true of the table and false of the language.
* **A `struct` or `union` passed by value** is not an unknown type — it
  is a known one this cannot make.  `struct in_addr` is 21 of them.
  Inventing an aggregate is the same dishonesty as inventing a buffer
  behind a pointer, so it gets its own reason rather than being filed
  under ignorance.
* **A complex is two floats end to end**, and every bit pattern in one
  is a value — possibly a NaN, which is a value the function has to
  cope with rather than a trap.  66 of them.  The harness now includes
  `<complex.h>`, without which a `cpow()` harness does not compile and a
  type this *can* synthesise would be reported as one it cannot.

The named integer typedefs went in from the histogram, each read out of
the tree: `quad_t`, `u_quad_t`, `lba_t`, `ufs1_daddr_t`, `ufs2_daddr_t`,
`ufs_time_t`, the `rpc*_t` family, and softfloat's `float32` and
`float64` — which are `unsigned int` and `unsigned long long`, not
floating types at all.

Two names the histogram offered are deliberately **not** there.  `acl_t`
is `struct acl_info` in one header and a pointer in another; `iconv_t`
is `int` in a contrib copy and `void *` in the real one.  A table that
is right about one of those is wrong about the other, which is precisely
the failure the histogram exists to catch.

### And the evidence, again

The two real `convtbl` crashes were first recorded as their *stack
frames*.  `_tail()` was doing what it is for — a compiler puts its
verdict last — and a sanitizer buries its verdict among forty frames and
build IDs.  `evidence()` now keeps the lines that carry one:

```
==1==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000000
SUMMARY: AddressSanitizer: SEGV convtbl.c:99 in convert
```

That is the third time in this engine's short life that the *reporting*
was the defect rather than the finding, and all three were the same
mistake in different clothes: a true statement about the tool presented
as a statement about the code.

### What the three answers were worth, measured

Whole tree, `--dry-run`, same 114,207 pairs:

```
before   ERROR 101357  NOFUNC 1200  RUNNABLE 11650
after    ERROR 100953  NOFUNC 1200  RUNNABLE 12054
```

404 pairs moved, all of them `ERROR` → `RUNNABLE`, and no pair moved in
any other direction.

One number in the histogram moved in a way worth explaining: the
pointer bucket *rose*, 83,894 → 84,037.  Nothing became a pointer.  A
function whose first parameter was an `enum` used to stop there with
"not a known scalar"; now the `enum` is accepted, the parser reaches a
*later* parameter that really is a pointer, and the reason it reports is
the one that was always the actual blocker.  The old answer was true and
useless — it named the wrong parameter.

The two real scopes, same tree, same budget:

```
lib/libc + lib/msun
  before  CLEAN 21  CRASH 2  SANFAIL 1  NOSEED 240  ERROR 4026  NORETURN 11
  after   CLEAN 29  CRASH 2  SANFAIL 1  NOSEED 329  ERROR 3928  NORETURN 12

bin + sbin + usr.bin + usr.sbin
  before  CLEAN 15  CRASH 0  NOSEED 684  ERROR 16379  NORETURN 628
  after   CLEAN 15  CRASH 0  NOSEED 699  ERROR 16364  NORETURN 628
```

Eight more functions actually fuzzed in the libraries, none in the
programs — where the 15 that moved went to `NOSEED` instead, because
CBMC cannot model a complex or a 64-bit `quad_t` counterexample any
better for having been handed one.  No new crashes, and the two that
stand are still `abs` and `imaxabs`.

`abs`'s recorded evidence, which is what all of this was for:

```
lib/libc/stdlib/abs.c:37:17: runtime error: negation of -2147483648
cannot be represented in type 'int'
```

`imaxabs`'s was still four stack frames on that run, because the fix had
been applied to one of the two call sites.  `evidence()` is used on both
now.

## The model checker, re-read: five closed out of 209

The CBMC half had not been re-read in a long time.  Rebuilt over
`lib/libc` and `lib/msun`: **1,177 of 1,295 translation units modelled**,
118 TU-ERROR, and of the functions the ledger and the goto model agree
on, 770 checkable unguarded (`SCALAR` and `VOID`, no stated
precondition):

```
BOUNDED 14  ERROR 60  FAILED 209  PROVED 425  TIMEOUT 62
```

209 `FAILED`.  Read as a taxonomy first, because most of it is one of
four shapes:

| shape | count | verdict |
|---|---|---|
| division by zero | 96 | 30 are libm's deliberate IEEE idioms |
| dereference failure | 54 | a global or a callee's out-parameter CBMC cannot see |
| arithmetic overflow / shift | 33 | mixed — this is where the defects were |
| `__CPROVER__start.memory-leak` | 16 | a function that allocates and returns the pointer |

**The libm division-by-zero family is not a defect and not a
near-miss.**  `vzero / vzero`, `(double)-1 / zero`, `(x - x) / (x - x)`
are idioms libm writes *on purpose* to raise the IEEE invalid or
divide-by-zero exception and produce the NaN or infinity the function is
required to return.  C11 6.5.5p5 makes division by zero undefined for
every type, and Annex F — which FreeBSD's libm assumes throughout —
defines exactly these.  CBMC's `--div-by-zero-check` does not know which
of the two documents is in force.

**The dereference family is the model, not the code.**
`__getCurrentRuneLocale()` returning NULL, `l->components` on a locale,
`__libc_interposing[]` entries, `_elf_aux_info()`'s out-parameter in
`arc4random`'s `_rs_initialize_fxrng`: each is established by
initialisation or by a callee in another translation unit, and CBMC
models an unseen callee's writes as unconstrained.  This is the
characterised out-parameter family, one engine over.

That left the arithmetic, and five of it were real.

### killpg(INT32_MIN, sig)

```c
	return (kill(-pgid, sig));
```

`pid_t` is `__int32_t` on every architecture, so `-pgid` at the most
negative value is undefined — and there is no answer to give `kill(2)`
either, since `+2147483648` is not a `pid_t`.  No process group has that
id, so it is `ESRCH` for the same reason `pgid == 1` already is, three
lines above.

### nice(INT_MAX)

```c
	if (setpriority(PRIO_PROCESS, 0, prio + incr) == -1) {
```

`prio` is in `[PRIO_MIN, PRIO_MAX]`; `incr` is whatever the caller
passed.  Computed in `long` and saturated to the `int` range —
`setpriority(2)` clamps to the priority range anyway, so no result that
did not already overflow changes.

### timezone(INT_MIN, dst)

```c
	if (zone < 0) {
		zone = -zone;
```

`timezone(3)` names no domain for `zone`.  Done in `long`, where every
`int` has an exact negation.

### fpsetmask: a left shift of a negative value, on every call

```c
	_newcw |= (~_m << FP_MSKS_OFF) & FP_MSKS_FLD;
	_mxcsr |= (~_m << SSE_MSKS_OFF) & SSE_MSKS_FLD;
```

`fp_except_t` is `int` on x86 — a `#define` in `x86_ieeefp.h` — so `~_m`
is negative for every mask with the top bit clear, which is every mask
anyone passes, `fpsetmask(0)` included.  C11 6.5.7p4 makes a left shift
of a negative value undefined *whatever the distance*, so this is not an
edge case: it is the ordinary path.  The complement is done in
`unsigned` now; the `&` immediately below discards everything the change
could affect, so the result is identical for every input.  The i386 copy
of the same line went with it.

### The measurement

Same tree layout, same scope, same 770 pairs:

```
before   BOUNDED 14  ERROR 60  FAILED 209  PROVED 425  TIMEOUT 62
after    BOUNDED 15  ERROR 61  FAILED 205  PROVED 429  TIMEOUT 60
```

Pair by pair, six verdicts moved and every one is accounted for:

| function | before | after |
|---|---|---|
| `fpsetmask` | FAILED | PROVED |
| `killpg` | FAILED | PROVED |
| `nice` | FAILED | PROVED |
| `_tztab` | FAILED | PROVED |
| `__timezone_compat` | FAILED | BOUNDED |
| `arc4random` | TIMEOUT | FAILED |

The last is not a regression: `arc4random` timed out on the first run
and the solver got further on the second, reaching the
`_rs_initialize_fxrng` dereference described above.  A `TIMEOUT` that
becomes a verdict is the instrument finishing, not the tree changing —
and the same jitter is the whole of the `ERROR` and `TIMEOUT` ±1.

## sqrt(0x1p-1043): a shift by the whole width of the operand

`e_sqrt.c`'s subnormal normalisation:

```c
	while(ix0==0) {
	    m -= 21;
	    ix0 |= (ix1>>11); ix1 <<= 21;
	}
	for(i=0;(ix0&0x00100000)==0;i++) ix0<<=1;
	m -= i-1;
	ix0 |= (ix1>>(32-i));
	ix1 <<= i;
```

`ix1` is `u_int32_t`, so `ix1>>32` is a shift by the operand's whole
width — undefined, C11 6.5.7p3.  The reachability is not exotic.  The
`while` loop exits as soon as `ix0` is nonzero and it fills `ix0` from
`ix1>>11`; an `ix1` with bit 31 set therefore puts **bit 20** of `ix0`
in place on the very first pass, the `for` loop's condition is already
false, and `i` stays 0.

`x = 0x0000000080000000` — the subnormal whose mantissa is exactly
2<sup>31</sup> — does that:

```
after while: ix0=0x100000 ix1=0  i=0  -> shift distance 32
```

A 32-bit value shifted right by 32 means "everything out", so the
guarded form is what the unguarded one was trying to say — `ix0 |= 0`
and `ix1 <<= 0` are both no-ops:

```c
	if(i!=0) {
	    ix0 |= (ix1>>(32-i));
	    ix1 <<= i;
	}
```

The whole routine was then run against the host's `sqrt` over every
power-of-two bit pattern and the first 4,000 subnormals — 4,051 inputs,
**0 mismatches**, bit for bit.

On x86 the hardware masks the shift count to 5 bits, so `ix1 >> 32` has
been quietly computing `ix1 >> 0` and, with `ix1` zero on that path,
giving the right answer by accident.  That is what undefined behaviour
looks like right up until a compiler or an architecture disagrees.

## libcalendar: `y % 19` is negative for every year before 1

```c
	int     mc[] = {5, 25, 13, 2, 22, 10, 30, 18, 7, 27, 15, 4,
		    24, 12, 1, 21, 9, 29, 17};
	...
	dt.d = mc[y % 19];
```

C's `%` keeps the sign of the dividend, so `easterodn(-5)` reads
`mc[-5]`.  `easterog()` and `easteroj()` are public entry points taking
a plain `int` year with no stated domain, and the rest of the library
does handle years before 1.  The metonic cycle is periodic mod 19, so
the Euclidean remainder is also the mathematically right index and
nothing at or above zero changes:

```c
	dt.d = mc[((y % 19) + 19) % 19];
```

### A FAILED that stays FAILED can still have lost a property

Neither verdict moves — both functions are still `FAILED` — and the
per-property list is the measurement, not the verdict:

```
sqrt        before  line 116 division by zero in (x - x) / (x - x)
                    line 127 shift distance too large in ix1 >> 32 - i
            after   line 116 division by zero in (x - x) / (x - x)

easterodn   before  line  89 array 'mc' lower bound in mc[y % 19]
                    line 100 array 'ns' lower bound in ns[weekday]
                    line 100 array 'ns' upper bound in ns[weekday]
                    line 100 arithmetic overflow in dn + ns[weekday]
            after   line 111 array 'ns' lower bound in ns[weekday]
                    line 111 array 'ns' upper bound in ns[weekday]
                    line 111 arithmetic overflow in dn + ns[weekday]
```

Exactly the two properties closed and nothing else moved.  The `ns[]`
bounds that remain are the unseen-callee family again: `weekday()` lives
in `calendar.c` and `easterodn()` in `easter.c`, so CBMC models the
return as any `int` rather than the 0–6 it actually is.  Reporting the
verdict alone would have said "no change" about a fix that closed a real
out-of-bounds read.

## querylocale(0, loc): the index check that guards one end, again

```c
	int type = ffs(mask & ~LC_VERSION_MASK) - 1;
	FIX_LOCALE(loc);
	if (type >= XLC_LAST)
		return (NULL);
	...
		if (loc->components[type])
			return (loc->components[type]->locale);
```

`ffs()` answers 0 when no bit is set.  A mask naming no component —
`querylocale(0, loc)`, or `LC_VERSION_MASK` on its own — makes `type`
**-1**, passes the `>= XLC_LAST` test unchanged, reads
`components[-1]` out of bounds, and returns a `const char *` taken from
whatever was there.  `querylocale()` is a public entry point taking a
plain `int` mask.

The upper bound was checked and the lower was not.  That is the third
time today: `systat`'s `get_tbl_ptr()`, `easterodn()`'s `mc[y % 19]`,
and this.  A bound written as one comparison when the index has two
sides is a shape worth naming.

```c
	if (type < 0 || type >= XLC_LAST)
		return (NULL);
```

`NULL` is the answer the function already gives for a mask naming a
component it does not know.

### The finding that fired was not the defect

This one is worth being precise about, because the honest account is
less flattering than the fix.  CBMC reported exactly one property on
`querylocale`:

```
line 353 arithmetic overflow on signed - in return_value_ffs - 1
```

That is a **false positive**: `ffs()` lives in another translation unit,
so CBMC models its return as any `int`, and `INT_MIN - 1` overflows.
The real `ffs()` returns 0–32 and the subtraction is fine.

The verdict does not move — `querylocale` is still `FAILED` on that same
property, before and after — because the fix has nothing to do with it.
What the finding did was put a line number in front of a person, and the
defect was one line further down.

That is a use for a checker that no count of true positives measures,
and it is the reason "read every finding" is the rule rather than "fix
every finding".

## A lint for the shape, and the fourth one it found

Three defects in one day were the same shape:

* `systat`'s `get_tbl_ptr()` — `idx = scale < SC_AUTO ? scale : SC_AUTO`
* libc's `querylocale()` — `if (type >= XLC_LAST) return (NULL);`
* `libcalendar`'s `easterodn()` — `mc[y % 19]`

The first two write a bound for a two-sided index as one comparison, and
the first says so out loud in its own comment — *"if our index is out of
range"* — while the code checks one end.  The third subscripts with `%`,
which in C keeps the sign of the dividend.

`tools/verify/onesided_index.py` looks for both:

| rule | what |
|---|---|
| `MOD` | `arr[V % C]` where V is a **signed** integer whose value comes from outside the function |
| `ONESIDED` | `arr[V]` where V is signed, the function bounds it above, and nowhere below |

### The fourth: strsignal(INT_MIN)

```c
	signum = num;
	if (num < 0)
		signum = -signum;
	...
	do {
		*t++ = "0123456789"[signum % 10];
	} while (signum /= 10);
```

`strsignal(3)` takes a plain `int` and that `else` arm is the one that
handles *every* value outside the signal range, so `strsignal(INT_MIN)`
reaches it.  Negating the most negative `int` is undefined — and it does
not even come out positive: `signum` stays negative, and the subscript
reads off the front of the string literal, into the buffer
`strsignal()` returns to its caller.

The file had carried

```c
/* XXX: negative 'num' ? (REGR) */
```

above the definition.  Somebody knew.

`strerror()`'s `errstr()` had the same line — `uerr = (num >= 0) ? num :
-num;` — and although the destination was already `unsigned`, the `-num`
is computed in `int` before it gets there.  Same fix, negate in
`unsigned`, which is exact and modular.  Rendered over the full range
afterwards, `INT_MIN` included:

```
 -2147483648 -> -2147483648
  2147483647 -> 2147483647
```

**The analyser has nothing here**: `lib/libc/string` measures 0 findings
across 86 translation units.  Neither does the model checker —
`strsignal` is not in its reachable set.  This one came from the lint
alone.

### Only one of the two rules gates

`MOD` reports **8** sites tree-wide, every one read and written into
`EXPECTED` with the reason: a fraction-digit count from `strlen`, `pr`'s
own line counter, two `pid_t`s from `fork(2)`, a queue rank, a driver
ring index counted from zero, a `compat_strtoul()` result, and an
unsigned buffer offset.  That is a gate.

`ONESIDED` reports **264** — down from 1,109 over four rounds of
tightening — and they are dominated by internal contracts: a static
helper whose two callers both pass 0 or 1, a driver's ring index, a
parameter something three functions away pins.  Each is a reading, not a
defect, and a gate that demands 264 readings before the next commit is a
gate somebody turns off.  So it prints and does not fail, which is the
same call `noreturn_check.py` makes about its 673.

The four rounds are worth listing, because each was the checker being
wrong about the tree rather than the tree being wrong:

| what it learned | 1,109 → |
|---|---|
| a `for (V = …)` initialiser is a loop counter whatever the initialiser is | 656 |
| `assert(0 < i && …)` — the author stating the bound is a bound | 569 |
| an index whose value never leaves the function is not this defect | 264 |
| one report per (function, variable), not per use | 264 |

And two rounds before that were the checker failing to find the defects
it was written for — the systat shape puts its ceiling on the *source*
expression rather than on the index, and `if ((scale = get_scale(args))
!= -1)` puts the floor and the name on either side of a call.  Both are
now cases in `test_onesided_index.py`, along with the three originals in
the form they had before they were fixed.  A lint that cannot find the
bugs it was written for is worth nothing, and the only way to know is to
keep them.

## The libs model-check shard, accounted for in full

All 205 `FAILED` verdicts over `lib/libc` and `lib/msun`, every one in a
family that was read rather than assumed:

| family | count |
|---|---|
| libm's deliberate IEEE idiom (`vzero / vzero`, `(x - x) / (x - x)`) | 92 |
| an unseen callee or an unconstrained global | 52 |
| arithmetic on an unconstrained parameter | 28 |
| CBMC's entry-point leak check on a function that allocates and returns | 21 |
| division by zero that is the caller's contract (`div`, `ldiv`, `lldiv`, `gcd`) | 4 |
| an assertion on an unconstrained argument | 3 |
| an array bound | 3 |
| a CBMC limitation, stated in its own words | 2 |
| **other** | **0** |

Nine of the 28 in the arithmetic row were real and are fixed above.  The
three array bounds were read one at a time and none is a defect:
`__libc_interposing_slot(int s)` is called with an enum constant from
two places, `readpassphrase`'s `handler(int s)` is a signal handler and
the kernel supplies the number, and `srandomdev`'s is a pointer bound on
a file-scope `struct __random_state` whose fields CBMC models
unconstrained.

Two verifications that could have gone the other way and did not:

* **`gcd(a, b)` in `getopt_long.c`** does `a % b` and would be undefined
  at `b == 0`.  `permute_args()` computes `nopts = opt_end -
  panonopt_end`, and `nonopt_end` is assigned at an `optind` *before*
  the option there is consumed, so by the time any of the three call
  sites is reached `optind` has advanced past at least one option and
  `nopts >= 1`.  `nnonopts` is likewise at least one, so `gcd` returns
  at least one and the `/ ncycle` below it is safe too.
* **`significand(x)`** is `scalb(x, (double) -ilogb(x))`, and `ilogb(0)`
  looks like it should be `INT_MIN`.  It is not: FreeBSD defines
  `FP_ILOGB0` as `-__INT_MAX` and `FP_ILOGBNAN` as `__INT_MAX`,
  precisely so the negation is defined.  CBMC flags it because `ilogb`
  is in another translation unit.

The 92 libm idioms deserve their own sentence, because they are the
largest single family and the least like a defect.  `(x - x) / (x - x)`
is how `acosf` returns a signalling NaN for an out-of-domain argument;
`(double)-1 / zero` is how `logl` returns −∞ for zero.  C11 6.5.5p5
makes division by zero undefined for every type; Annex F, which
FreeBSD's libm assumes on every line, defines exactly these.  CBMC's
`--div-by-zero-check` does not know which of the two documents is in
force, and there is no flag that tells it.

## The programs, model-checked for the first time

The CBMC half had only ever been pointed at `lib/libc`, `lib/msun` and
`sys`.  `bin`, `sbin`, `usr.bin` and `usr.sbin` had never been
model-checked at all — even though the analyser sweep and the fuzzer had
both found real defects there.

**1,017 of 1,830 translation units modelled, 813 TU-ERROR.**  That ratio
is much worse than the libraries' 1,177 of 1,295, and for a reason worth
stating: a program's translation unit expects the rest of its own
program's headers, several of which the build generates.  Of the
functions the ledger and the goto model agree on, 895 were checkable
unguarded:

```
BOUNDED 70  ERROR 47  FAILED 187  PROVED 532  TIMEOUT 59
```

All 187 accounted for:

| family | count |
|---|---|
| an unseen callee or an unconstrained global | 76 |
| CBMC's entry-point leak check | 48 |
| arithmetic on an unconstrained parameter or global | 24 |
| an assertion on an unconstrained argument | 23 |
| division by zero | 15 |
| an array bound | 1 |
| **other** | **0** |

Four of the 24 were real.

### rtadvd(8): `1 << 63` into the sign bit, at every start-up

```c
	tm_limit.tv_sec = (-1) & ~((time_t)1 << ((sizeof(tm_max.tv_sec) * 8) - 1));
```

`time_t` is a signed 64-bit type, so this shifts a one *into* its sign
bit.  C11 6.5.7p4 requires the result to be representable in the result
type and 2<sup>63</sup> is not.  `rtadvd_timer_init()` runs
unconditionally at start-up: this is not an edge case, it is every
`rtadvd(8)`.  Shifted in `uintmax_t` and narrowed back; the value is
unchanged.

### ifconfig(8): a /1 prefix

```c
	a.s_addr = htonl(plen ? ~((1 << (32 - plen)) - 1) : 0);
```

A `/1` prefix shifts by 31, and `1 << 31` does not fit an `int` — so the
most ordinary width this function can be handed is the one that breaks
it.  `1U` is defined for every distance 0 through 31 and gives the same
bits.

The other end needed a clamp.  `plen` is `ifa->ifa_prefixlen` widened
from a `uint8_t` out of a netlink message — 0 to 255, not 0 to 32 — and
at `plen` 200 the shift distance is −168, undefined again in the other
direction.  The kernel should never send a wider one, and this function
cannot know that it did not.  32 is the widest mask there is.

### apm(8): a guard on one end, twice, and an infinite loop

```c
	int2bcd(int i) { if (i >= 10000) return -1; ... (i % 10) << base ... }
	bcd2int(int bcd) { if (bcd > 0x9999) return -1; ... bcd >>= 4 ... }
```

Both guard only the top.  In `int2bcd`, a negative `i` makes `i % 10`
negative and shifting a negative value left is undefined; a large `|i|`
also drives `base` past 31, so the distance exceeds the width as well.

`bcd2int` is worse than undefined — it does not terminate.  `bcd >>= 4`
on a negative `int` is an arithmetic shift, so `-1` stays `-1` forever.
And it is reachable: `args.edi` is a `uint32_t` straight out of the APM
BIOS reply and `bcd2int()` takes an `int`, so a reply with the top bit
set hangs `apm(8)`.

That is the one-sided bound again — the fifth and sixth of the day, and
the first two that are not array subscripts.

All four: `FAILED` → `PROVED`.

### Three that were checked and are not defects

* **`ppp`'s `bits2mask4(int bits)`** loops `while (bits) { …; bits--; }`,
  which for a negative `bits` runs to `INT_MIN` and then overflows.
  `ncprange_aton()` rejects `bits < 0 || bits > 128` at parse time
  before any of the three call sites.
* **`ed`'s `sigflags &= ~(1 << (signo - 1))`** would be undefined at
  `signo` 32.  `ed` installs handlers for SIGHUP, SIGINT and SIGWINCH.
* **`quota`'s `prthumanval(int len, …)`** declares `char buf[len + 1]`,
  a VLA sized from its parameter.  All three call sites pass the literal
  7.

## The kern shard, model-checked: nothing, and the number that says why

`sys/kern`, `sys/vm`, `sys/net`, `sys/netinet` and `sys/netinet6`:

```
197 of 662 translation units modelled, 465 TU-ERROR
158 (file, function) pairs checkable unguarded

BOUNDED 5  FAILED 18  PROVED 91  TIMEOUT 44
```

**Nothing in the 18 was a defect**, and the honest headline is the first
line rather than the last.  158 of the shard's 905 classified `SCALAR`
and `VOID` functions reach the checker at all — the rest have no goto
model because their translation unit does not compile standalone on a
Linux host, which is the same 465 TU-ERROR seen from the other side.
That ratio is why the kernel's findings in this project have come from
the analyser and not from here.

All 18:

| family | count |
|---|---|
| an unseen callee or an unconstrained global | 10 |
| CBMC's entry-point leak check | 5 |
| arithmetic on an unconstrained parameter or global | 2 |
| a `static inline` analysed as its own entry point | 1 |

The last one is worth a paragraph because it is the family at its most
convincing, and because the author had already answered it.
`subr_blist.c`'s

```c
static inline u_daddr_t
bitrange(int n, int count)
{
	return (((u_daddr_t)-1 << n) &
	    ((u_daddr_t)-1 >> (BLIST_RADIX - (n + count))));
}
```

is reported for a negative shift distance, a distance past the width,
and `n + count` overflowing — and at `count == 0` the right shift really
would be by `BLIST_RADIX`, which is undefined.  Every one of the ten
call sites was read.  Three of them could have passed zero and all three
are guarded, explicitly, right at the call:

```c
	if (maxcount % BLIST_RADIX != 0)
		scan->bm_bitmap &= ~bitrange(0, maxcount % BLIST_RADIX);
	else
		scan->bm_bitmap = 0;
...
	if ((blk & BLIST_MASK) != 0) {
		if ((~mask & bitrange(0, blk & BLIST_MASK)) != 0) {
```

The other seven pass a literal `1`, or a `*count` the surrounding
arithmetic pins between one and `BLIST_RADIX - lo`.  A guard written
three times at three call sites is somebody who knew.

The two arithmetic reports are `run_interrupt_driven_config_hooks_
warning`'s `warned * 60`, where `warned` counts one per sixty seconds
and needs sixty-eight years to overflow, and `blist_create(daddr_t
blocks)`'s `blocks - 1` at `LONG_MIN`, where `blocks` is a swap device's
size.

## The fs and dev shards, and the note number nobody bounded

`sys/fs`, `sys/ufs`, `sys/geom`, `sys/cam`, `sys/security`, `sys/cddl`:

```
161 of 311 translation units modelled, 150 TU-ERROR
BOUNDED 8  ERROR 1  FAILED 25  PROVED 87  TIMEOUT 1
```

`sys/dev`:

```
1,247 of 2,509 translation units modelled, 1,262 TU-ERROR
BOUNDED 12  ERROR 57  FAILED 220  PROVED 194  TIMEOUT 122
```

Accounted for:

| family | fs | dev |
|---|---|---|
| an unseen callee or an unconstrained global | 5 | 166 |
| arithmetic on an unconstrained parameter or global | 1 | 42 |
| CBMC's entry-point leak check | 17 | 8 |
| an array bound | 2 | 3 |
| division by zero | — | 1 |
| **other** | **0** | **0** |

### geom_flashmap: an exported interface with no bound at all

```c
void flash_register_slicer(flash_slicer_t slicer, u_int type, bool force)
{
	g_topology_lock();
	if (g_flashmap_slicers[type].slicer == NULL || force == TRUE)
		g_flashmap_slicers[type].slicer = slicer;
```

Nothing checked `type`.  `slicer.h` declares this function for any
driver, in tree or out, and what it writes at the index is a **function
pointer** the taste path later calls.  Every in-tree caller passes a
`FLASH_SLICES_TYPE_*` constant — so the bound changes nothing that works
today, and it is the bound the declaration implies and the definition
did not have.  `FAILED` → `PROVED`.

### spkr(4): five GETNUMs range-check their result and the sixth does not

`playstring()` parses a play string with

```c
#define GETNUM(cp, v)	for(v=0; isdigit(cp[1]) && slen > 0; ) \
				{v = v * 10 + (*++cp - '0'); slen--;}
```

and uses it six times.  Five are followed immediately by a range check
in the author's own idiom:

```c
	GETNUM(cp, timeval);  if (timeval <= 0 || timeval > MIN_VALUE) …
	GETNUM(cp, octave);   if (octave >= nitems(pitchtab) / OCTAVE_NOTES) …
	GETNUM(cp, value);    if (value <= 0 || value > MIN_VALUE) …
	GETNUM(cp, tempo);    if (tempo < MIN_TEMPO || tempo > MAX_TEMPO) …
```

The sixth is `N`:

```c
	case 'N':
		GETNUM(cp, pitch);
		…
		playtone(pitch - 1, value, sustain);
```

and `playtone()` does `tone(pitchtab[pitch], sound)`.  **`N500` reads
past the end of `pitchtab[]` and hands what it finds to `tone()` as a
frequency.**  The check the other five have is added.

Two more in the same file, both the same shape — a value the play string
supplies without limit:

* **The accumulator itself.**  `v = v * 10 + digit` overflows an `int`
  at ten digits, which is undefined *before* any of the six range checks
  gets to look at the result.  It saturates at `GETNUM_MAX`, which is
  past every bound any of the six applies, so every string that parsed
  to something meaningful still parses to the same thing — verified over
  the range, `N99999999999999999999` included, with UBSan clean.
* **The dot count.**  `playtone()` multiplies `snum` by 3 and `sdenom`
  by 2 once per `.`; at twenty dots `snum` overflows and at thirty-one
  `sdenom` wraps to zero.  The function already contains

  ```c
	if (value == 0 || sdenom == 0)
		return;
  ```

  which is that wrap noticed and answered at the symptom rather than at
  the cause.  Clamped at the last count for which the multipliers stay
  exact.

CBMC's verdict on `playtone` does not move, and that is worth stating:
the `sustain - 1` property is gone, but the rest are on `value` and
`pitch` as unconstrained parameters, and the fix for the note number
lives in `playstring()` — a `char *` function, `POINTER` class, which
the unguarded tier never checks.  The finding put a line number in front
of a person; the defect was one call up.

## The rest shard, and the matrix closed

`sys/arm64`, `sys/amd64`, `sys/arm`, `sys/i386`, `sys/riscv`,
`sys/powerpc`, `sys/x86`, `sys/compat`, `sys/crypto`, `sys/netpfil`,
`sys/net80211`, `sys/netgraph`, `sys/netlink` and the rest:

```
807 of 1,486 translation units modelled, 679 TU-ERROR
BOUNDED 24  ERROR 13  FAILED 151  PROVED 304  TIMEOUT 82
```

| family | count |
|---|---|
| an unseen callee or an unconstrained global | 101 |
| arithmetic on an unconstrained parameter or global | 37 |
| an array bound | 7 |
| CBMC's entry-point leak check | 4 |
| division by zero | 2 |
| **other** | **0** |

Nothing was fixed here, and one of the seven array bounds is worth
writing down anyway because of *why* it is safe:

```c
int
bsd_to_linux_errno(int error)
{
	KASSERT(error >= 0 && error <= ELAST,
	    ("%s: bad error %d", __func__, error));

	return (linux_errtbl[error]);
}
```

A `KASSERT` compiles to nothing without `INVARIANTS`, and `GENERIC` does
not have it — so on the kernel anyone actually runs, this indexes
`linux_errtbl[]` with no check.  What makes it safe is not the assertion
but the four call sites: every one passes a value the kernel itself set
as an errno.  The `SO_ERROR` path is the closest to a user: it returns
whatever `kern_getsockopt()` read out of `so_error`, and every in-kernel
assignment to that field is an errno.

That is the honest reading, and it is a different sentence from "the
assertion checks it".

### The whole matrix

Five shards, every `FAILED` verdict in a family that was read:

| shard | TUs modelled | checkable | PROVED | FAILED | real |
|---|---|---|---|---|---|
| `lib/libc` + `lib/msun` | 1,177 / 1,295 | 770 | 425 | 209 | **9** |
| `bin` `sbin` `usr.bin` `usr.sbin` | 1,017 / 1,830 | 895 | 532 | 187 | **4** |
| `sys/kern` `vm` `net` `netinet` `netinet6` | 197 / 662 | 158 | 91 | 18 | 0 |
| `sys/fs` `ufs` `geom` `cam` `security` `cddl` | 161 / 311 | 121 | 87 | 25 | **1** |
| `sys/dev` | 1,247 / 2,509 | 605 | 194 | 220 | **1** (+2 read out of it) |
| `sys/*` rest | 807 / 1,486 | 574 | 304 | 151 | 0 |

**4,606 of 8,093 translation units modelled** — the other 3,487 do not
compile standalone on a Linux host, and that number is the single
largest fact about what this engine can and cannot say about this tree.
Of the 3,123 (file, function) pairs it could check, 1,633 came back
`PROVED` — a real proof over all inputs for the checked properties,
within the unwinding bound — and 810 `FAILED`, of which **15 were
defects**.

The ratio of `FAILED` to defect is about 54 to 1, and every one of the
795 was put in a named family rather than waved at.  Four families
account for almost all of them: an unseen callee or unconstrained
global, CBMC's entry-point leak check, arithmetic on a parameter the
callers bound, and libm's deliberate IEEE idioms.

## Sixteen edits, re-swept: nothing moved that should not have

Sixteen vendor-tree fixes landed in one day, across `lib/libc`,
`lib/msun`, `lib/libcalendar`, `usr.bin/systat`, `sbin/ifconfig`,
`usr.sbin/rtadvd`, `usr.sbin/apm`, `sys/geom`, `sys/dev/speaker` and the
x86 `ieeefp.h` headers.  Every one was measured by the engine that found
it — but a fix measured only by its own engine is a fix nobody checked
against the others, so the analyser was run over every scope they
touched.

```
bin sbin usr.bin usr.sbin   557 finding(s)  OK 1822  ERROR 40   --check-errors ok
lib/libc lib/msun libexec   238 finding(s)  OK 1600  ERROR 30
lib/libcalendar sys/geom sys/dev/speaker
                             32 finding(s)  OK   81  ERROR  0
```

**557, OK 1822, ERROR 40** for the program scopes is the number to the
digit that the progs shard closed on before any of today's work.  The
four edits in `systat`, `ifconfig`, `rtadvd` and `apm` introduced no
finding and no translation-unit error.

### The one thing that did fail was the inventory, not the tree

`--check-errors` failed on the libs scope, and it took reading the
message to see it was not a regression:

```
FAIL  libexec/atf/atf-pytest-wrapper/atf_pytest_wrapper.cpp compiles now;
      its EXPECTED entry is stale (#include <format>, which needs a
      C++20 standard library)
```

A **stale exemption** — the gate firing in the direction it is less
often thanked for.  The file still `#include <format>` and still
compiles at `-std=gnu++17`, because the in-tree libc++ that went on the
sweep's include path guards nearly all of `<format>` behind
`_LIBCPP_STD_VER >= 20`; at C++17 the header resolves to almost nothing,
and the file never calls `std::format`.  So the reason the exemption
gave stopped being true the day the C++ standard headers were put on the
path, and nothing said so until something asked.

The entry is gone.  `libexec` now reports four ERROR translation units,
all four on the record.

The first sweep of this pass also failed, on `--scope lib` rather than
the shard's `lib/libc --scope lib/msun --scope libexec` — 37 files under
`libsecureboot`, `libnv`, `libfetch` and their neighbours that no shard
has ever analysed and `expected_errors.py` was never reconciled against.
That is a scope this project has not opened, not a hole in one it has:
worth its own pass, and not this one.

## The libraries no shard has ever analysed

The re-sweep above failed once more before it was pointed at the right
scopes, and that failure is worth its own section, because it is the
biggest remaining hole in this project's coverage.

`--scope lib` — all of `lib/`, rather than the shard's `lib/libc`,
`lib/msun` and `libexec` — reports:

```
436 finding(s) across 2169 translation units   ERROR 72
```

against the shard's

```
238 finding(s) across 1630 translation units   ERROR 30
```

**203 findings and 46 ERROR translation units** in libraries that no
analyse shard has ever covered.  `check_shards.py` has said so all along
— `lib` is on its excused list with the note *"lib/libc and lib/msun are
sharded; the rest is not yet"* — which is the bookkeeping being honest
about a hole rather than the hole being hidden.  What was missing was
the number.

Where the 203 are:

| library | findings |
|---|---|
| `lib/libdevstat` | 54 |
| `lib/clang` | 45 |
| `lib/libpmc` | 21 |
| `lib/libutil` | 11 |
| `lib/libcasper` | 10 |
| `lib/libpfctl` | 6 |
| `lib/libefivar` | 5 |
| the rest | 51 |

### Two of the 46 are not a hole at all

Read from the build rather than guessed at, twenty of the forty-six are
components a default build **never enters**, and `src.opts.mk` says so
in one list:

```
__DEFAULT_NO_OPTIONS = \
    ASAN \
    BEARSSL \
    ...
    DIALOG \
```

* `lib/libsecureboot/` — 17 files.  `lib/Makefile:164` is
  `SUBDIR.${MK_BEARSSL}+= libbearssl libsecureboot`.  `bmake` refuses
  the directory from the other side for the same reason:
  `local.trust.mk:90` stops with *"Need TRUST_ANCHORS see README.rst"*.
* `lib/libdpv/` — 3 files.  `lib/Makefile:179` is
  `SUBDIR.${MK_DIALOG}+= libdpv libfigpar`, and `<dialog.h>` is
  `contrib/dialog`'s, installed only when that option is on.

Both are on the record now — the honest verdict is *not built*, not
*cannot compile*.

They went down as `NOT_SUBDIR` first, and that was the wrong claim:
`lib/Makefile` **does** name both directories, `SUBDIR.${MK_BEARSSL}+=`
and `SUBDIR.${MK_DIALOG}+=` being SUBDIR assignments like any other.
`test_expected_errors.py` said so on the next run — *"lib/Makefile lists
libsecureboot in SUBDIR, so the build DOES descend into it"* — which is
the check doing exactly what it is for.  The right claim is
`DEFAULT_OFF:<OPTION>`, the shape `usr.bin/dpv/dpv.c` already used, and
it is stronger: it names the option, so both halves get checked — that
the option really is in `__DEFAULT_NO_OPTIONS`, and that the parent
Makefile really gates *that* directory on *that* option.  That claim had
only ever been made by file entries; `NOT_BUILT` prefixes can make it
now too, and the test's loop runs over both.

### And one negative result, measured

`lib/libsecureboot/Makefile.inc` sets `libsecureboot_src:= ${.PARSEDIR}`
and then `CFLAGS+= -I${libsecureboot_src}/h`, which `makefile_flags()`
drops because it has no `.PARSEDIR` in its variable table.  Adding one
looked obviously right and was written.

It moves nothing.  Exactly **one** Makefile in `lib`, `libexec`, `bin`,
`sbin`, `usr.bin` and `usr.sbin` uses `.PARSEDIR` in a `CFLAGS`
assignment — that one — and it sits inside a `.if !target(…)` block the
textual walk skips by design, in a component the build does not build.
Reverted, and written down here so the next reader does not spend the
same twenty minutes.

The remaining 26 are a real backlog and not this pass's: generated
headers (`ftperr.h`, `httperr.h`, `tables.h`, `tables_linux.h`,
`yppasswd_private.h`), krb5 and sndio dependencies, a `.tpl.c` template
that is not a translation unit, `lib/libmd/mdXhl.c` which is compiled
once per algorithm with `-DmdX`, and a handful where the include path
still has an answer to give.

## `PHDRS` is the same rule as `LHDRS`, one list over

`_lhdrs()` in `tools/verify/includes.py` exists because FreeBSD does not
keep `<errno.h>` in `include/`.  `include/Makefile` names a set of
top-level headers that actually live in `sys/sys`, and the install step
symlinks them into place:

```
.for i in ${LHDRS}
INCSLINKS+= sys/$i ${INCLUDEDIR}/$i
.endfor
```

The shim parses that list rather than copying it, so it tracks the
Makefile.  What it did not do was read the list **beside** it.
`include/Makefile:344` and `:350` are byte-for-byte the same rule under
two names:

```
.for i in ${LHDRS}          .for i in ${PHDRS}
INCSLINKS+= sys/$i ...      INCSLINKS+= sys/$i ...
.endfor                     .endfor
```

`PHDRS` is `_semaphore.h stdarg.h`.  `lib/libthr/thread/thr_sem.c:41` is
`#include <_semaphore.h>`, and the sweep answered

```
'_semaphore.h' file not found, did you mean 'semaphore.h'?
```

which is the failure mode this whole file is about: **a translation unit
that does not compile reports zero findings and is indistinguishable
from a clean one.**  `thr_sem.c` is 300 lines of `sem_t` handling that
nothing had ever looked at.

Measured over `lib/libthr`, before and after, same scope, same tree:

| | TUs | OK | ERROR | findings |
|---|---|---|---|---|
| before | 64 | 62 | 2 | 1 |
| after  | 64 | 63 | 1 | 1 |

`thr_sem.c` compiles and is clean.  The remaining `ERROR` is not one,
and the remaining finding is fixed below — `lib/libthr` closes at **64
translation units, 63 analysed, 0 findings, 1 not built and on the
record.**

### `thr_autoinit.c` is dead source, and now says so

`lib/libthr/thread/thr_autoinit.c:51` calls `_thread_init()`, a name
libthr does not declare — `thr_private.h:800` declares
`_libpthread_init(struct pthread *)`.  `_thread_init` is *libc's* stub
name (`lib/libc/gen/_thread_init.c:33` weak-references
`_thread_init_stub` to it), which libthr overrides rather than calls.

The file cannot compile, and nothing asks it to.
`lib/libthr/thread/Makefile.inc:26` lists `thr_init.c` and names
`thr_autoinit.c` nowhere, and `thr_init.c:285-291` carries the same
comment and the same `extern int _thread_autoinit_dummy_decl` the file
was written to hold.  The two were folded together and the original was
never deleted; they have since diverged, `thr_init.c:294` defining
`_thread_init_hack()` as `_libpthread_init(NULL)`.

`NOT_NAMED` in `tools/verify/expected_errors.py` — checked, not asserted: `sweep_report.names_it()` reads the build's own
answer, and it says `thr_init.c` yes, `thr_sem.c` yes,
`thr_autoinit.c` no.  `lib/libthr` is now
fully accounted for: 64 translation units, 63 analysed, one not built.

## `__thr_sigaction`: `sigaction(sig, NULL, NULL)` reads the stack

The one finding `lib/libthr` has, newly visible now that the scope is
open at all:

```
lib/libthr/thread/thr_sig.c:638  [core.UndefinedBinaryOperatorResult]
    The left operand of '!=' is a garbage value
```

`__thr_sigaction()` declares

```c
	struct sigaction newact, oldact, oldact2;
```

and fills `oldact` only by handing `&oldact` to `__sys_sigaction()`,
which it calls in two places:

```c
	if (act != NULL) {
		...
		ret = __sys_sigaction(sig, &newact, &oldact);
	} else if (oact != NULL) {
		ret = __sys_sigaction(sig, NULL, &oldact);
	}

	if (oldact.sa_handler != SIG_DFL && oldact.sa_handler != SIG_IGN) {
```

`sigaction(sig, NULL, NULL)` takes **neither** branch.  It is a legal
call — POSIX says if `act` is null the disposition is not changed, and
if `oact` is null nothing is reported — and it returns 0.  On that path
`oldact` is never written by anyone, and the line below reads
`oldact.sa_handler` regardless.

Nothing escapes: the read only decides an assignment whose result is
copied out solely when `oact != NULL` *and* `ret == 0`.  But it is a
read of an object that was never written, on a call any program is
entitled to make, and it is the kind of thing that stops being harmless
the moment someone adds a use below it.

Zeroing it first is both the fix and the statement of intent:

```c
	bzero(&oldact, sizeof(oldact));
```

`SIG_DFL` is `(void *)0`, so a zeroed `sa_handler` *is* `SIG_DFL` — the
fixup declines, which is exactly what "nothing was retrieved" should
mean.  Every other path is unchanged: on the two paths where the call
was made and succeeded the kernel overwrites it, and on the paths where
it failed `ret != 0` and `*oact` is not written either way.  The same
decision on every path, now for a stated reason rather than by accident
of what was on the stack.

## The libraries no shard had analysed, part two: five build-system readers

Twenty of the forty-six `lib` translation units that would not compile
were components a default build never enters, and went on the record as
that.  Of the twenty-six left, twenty-three turned out to be five
distinct things the readers in `tools/verify/` were not doing, and each
one is now done rather than excused.

### `.PATH` is where make finds SOURCES, and `sys/sys` is not a source directory

`makefile_flags()` puts every `.PATH` directory on the include path, for
a good reason it states: a source's private header sits beside it there,
which is how `lib/libc/stdtime` reaches `contrib/tzcode`'s `private.h`.

`sys/sys` is not that directory.  It is the **kernel's** header
namespace, whose every file is installed as `<sys/name.h>` and never as
`<name.h>` — and it holds a `unistd.h`, a `signal.h`, a `time.h`, a
`stat.h`, an `errno.h` and a `fcntl.h`, each of which shadows the
userland header of that name the moment the directory precedes
`include/` on the path.

`lib/libnv/Makefile:12` is the tree's **only** userland
`.PATH: … ${SRCTOP}/sys/sys`, and it cost eight translation units:

```
lib/libnv/msgio.c:384: call to undeclared function 'close'
```

`msgio.c` includes `<unistd.h>` at line 43.  It got
`sys/sys/unistd.h`, which defines the `_SC_*` constants and declares not
one function — so `close(3)` was undeclared, and seven `lib/libnv/tests`
programs inherited the same `-I` through the ancestor walk and lost
`STDERR_FILENO` and `PATH_MAX` with it.  The real build never had the
problem: `.PATH` is a *source* path, and libnv's five sources all come
from `sys/contrib/libnv`.

`lib/libnv` now: **8 translation units, 8 analysed, 0 findings.**

### A directory can name everything it builds in `OBJS`

`ask_module()` learned this on the kernel side when `sys/modules/blake2`
reached its ten SIMD implementations through `OBJS+= ${SRCS_IN:S/.c/.o/g}`
and no `SRCS` line.  The userland reader asked only `SRCS`, `PROGS` and
`SRCS.<prog>`.

`lib/csu` is the directory that shows it.  `lib/csu/Makefile.inc`:

```
OBJS+=  Scrt1.o crt1.o gcrt1.o
OBJS+=  crtbegin.o crtbeginS.o crtbeginT.o
OBJS+=  crtend.o crtendS.o
OBJS+=  crti.o crtn.o
```

and `lib/csu/amd64/Makefile` is `.PATH: ${.CURDIR:H}/common` plus
`CFLAGS+= -I${.CURDIR}`.  So `crtbegin.c` and `crtend.c` live in
`common/` and are compiled once per architecture **from** the
architecture directory, where that architecture's own `crt.h` is.
Asking only `SRCS` meant nothing named them, no builder directory was
recorded, and both came back `'crt.h' file not found`.

**Every C program on the system starts in that code**, and no static
analysis had ever seen it.

Each `.o` is now mapped back through every suffix a source can wear, by
stem rather than by name so the eighty-odd dead candidates a big
directory would otherwise generate never cost a stat, and `resolve()`
drops the ones that hit no file — `Scrt1.o`, `gcrt1.o` and `crtbeginS.o`
name no source of their own and simply fall away.

**And then the gate caught the widening.**  On the next run of
`test_expected_errors.py`:

```
FAIL lib/libc/db/test/ is named by nothing
     the build names ['lib/libc/db/test/dbtest.c'], so this prefix is
     absorbing ERRORs from code that IS compiled
FAIL lib/libc/regex/grot/ is named by nothing
     the build names 3 files, ...
```

Both of those prefixes are *true*.  `lib/libc/db/test/Makefile` is
`PROG= dbtest` plus `OBJS= dbtest.o strerror.o`, and
`lib/libc/regex/grot/Makefile` is Spencer's own regex harness — 4.4BSD-era
standalone Makefiles that `.include` **nothing**, name paths through an
undefined `${PORTDIR}`, and are run by hand.  `SRCS` and `PROGS` never
saw them because they use neither; reading `OBJS` without asking whether
the directory is part of this build called their five sources built.

bmake says which it is, and it says so about itself: `.MAKE.MAKEFILES`
is the list it read.  A directory the build enters reaches `bsd.lib.mk`
or `bsd.prog.mk` through its own `.include`; these two reach only
`sys.mk`'s unconditional `bsd.mkopt.mk` and `bsd.suffixes.mk`.  `OBJS`
is read only when the first is true — one more thing asked of the build
rather than decided about it.

### ...and then WHICH of the seven directories that name it

`lib/csu/common/crtbegin.c` is named by all seven of
`lib/csu/{aarch64,amd64,arm,i386,powerpc,powerpc64,riscv}`, each with
`-I${.CURDIR}`, each compiling it against its **own** `crt.h`.  Taking
the first in sorted order gave it aarch64's, which is one comment line
(*"Empty so we can include this unconditionally"*), while amd64's
defines `HAVE_CTORS` and `INIT_CALL_SEQ`.

And it **compiles** either way.  That is the dangerous half: not an
ERROR that says something is wrong, but a clean check of a program no
architecture builds.  `_component_dir()` now prefers the builder
directory whose name is this file's own architecture.

### `RPCSRC` is not the only spelling

`lib/libypclnt/Makefile` has three:

```
RPCSRC=      ${SRCTOP}/include/rpcsvc/yp.x
RPCSRC_PW=   ${SRCTOP}/include/rpcsvc/yppasswd.x
RPCSRC_PRIV= ${SRCTOP}/usr.sbin/rpc.yppasswdd/yppasswd_private.x
```

The pattern insisted on `RPCSRC` exactly, so the third — the only place
in the tree that names `yppasswd_private.x` — was invisible, and
`ypclnt_passwd.c` came back `'yppasswd_private.h' file not found` while
the two headers reached through the plain name were generated fine.  Any
`RPCSRC<suffix>` counts now; the basename test that follows is what
keeps a Makefile with several from being matched to the wrong one, and
it was already doing that job.

### Two more generators, and one that would have been worse than the error

`lib/libfetch`'s `ftperr.h` and `httperr.h` are a shell loop over
`ftp.errors` and `http.errors` in its own Makefile, run rather than
reimplemented.  Two of libfetch's eight translation units, and the two
that carry every protocol error string the library reports.

`lib/libsysdecode`'s `tables.h` and `tables_linux.h` are its own
`mktables` and `mklinuxtables` scripts — and this one took two tries
worth recording.  `mktables` takes an **installed** `/usr/include`: it
greps `<incdir>/sys/mman.h`, `<incdir>/netinet/in.h` and thirty more by
their installed paths.  `incs_shim()` is close but is not that: the
compile path reaches `<sys/*.h>` through `-I${SRC}/sys` rather than by
staging, so the shim's own `sys/` holds three headers.  mktables found
none of the thirty it greps and wrote **every table empty**:

```
TABLE_START(accessmode)
TABLE_END
```

That file compiles.  It would have turned an ERROR into a clean check of
a libsysdecode that decodes nothing — and this file's own generator
table says in as many words that a stub is worse than the error it
replaces.  An empty table *is* the stub.  Given the installed layout the
build gives it, `tables.h` comes out with 961 entries.

A guard was added rather than only the fix: a table set with no
`TABLE_ENTRY(` in it is deleted instead of shipped, so the next time
this breaks it breaks visibly.

### What the five readers bought

`lib/libfetch`, `lib/libsysdecode` and `lib/libypclnt`: **19 translation
units, 18 analysed, 10 findings** in code no sweep had ever compiled.

## `http_next_header`: `p = realloc(p, n)`, twice

The one defect in those ten.

```c
	if (hbuf->bufsize < conn->buflen + 1) {
		if ((hbuf->buf = realloc(hbuf->buf, conn->buflen + 1)) == NULL)
			return (hdr_syserror);
		hbuf->bufsize = conn->buflen + 1;
	}
	strcpy(hbuf->buf, conn->buf);
```

`realloc()` returning NULL leaves the old block allocated, and assigning
that NULL over the only pointer to it loses it.  Two things then go
wrong at once: the block is unreachable, and `clean_http_headerbuf()`
frees NULL — so it leaks for the life of the process; and `hbuf->bufsize`
goes on describing a buffer that no longer exists, so a later call with a
shorter line takes the `bufsize` branch, skips the allocation entirely,
and `strcpy`s into NULL.  That second half is what clang reported:

```
lib/libfetch/http.c:547  [unix.cstring.NullArg]
    Null pointer passed as 1st argument to string copy function
```

The same line appears twice in the function — once for the first header
line and once for each continuation line — and **every header line of
every HTTP fetch goes through the first of the two**.  Both now go
through a temporary, which keeps the block and the invariant together.

### And three that are not defects

* `fetch_read()` and `fetch_writev()` in `common.c` read a `struct
  timeval timeout` that is filled only under `if (fetchTimeout > 0)`
  and read only under the same test — but `fetchTimeout` is a global
  `int` the library exports, and `poll(2)` and `fetch_socket_read()`
  sit between the two tests.  The unconstrained-global family.
* `ftp_cwd()`'s `pwd[i]` after

  ```c
  for (i = 0; i <= len && i <= end - file; ++i)
      if (pwd[i] != file[i])
          break;
  if (pwd[i] == '\0' && (file[i - 1] == '/' || file[i] == '/'))
  ```

  can only be one past the NUL if the loop ran to `i == len` without
  breaking, which needs `pwd[len] == file[len]`, i.e. `file[len] ==
  '\0'` — while `len <= end - file` and `end = strrchr(file, '/')` put
  a `/` at or after index `len`.  The two cannot both hold.  A
  postcondition of `strrchr` the analyser cannot see.

### Three sources that have never been built

* `lib/csu/i386/reloc.c` and `lib/csu/powerpc64/reloc.c` — the old
  copies of libc's start-up relocation handler, left behind when it
  moved.  `lib/libc/csu/libc_start1.c:51` is `#include "reloc.c"` and
  `lib/libc/csu/Makefile.inc:9` is
  `CFLAGS+= -I${LIBC_SRCTOP}/csu/${LIBC_ARCH}`, so that include resolves
  to `lib/libc/csu/<arch>/reloc.c` — never these.  The two differ: the
  live copy defines `ifunc_init()`, this one still defines
  `crt1_handle_rel()`.  Only i386 and powerpc64 were left behind; amd64,
  aarch64, arm and riscv have no `lib/csu/<arch>/reloc.c` at all, which
  is what a half-finished move looks like.
* `lib/libypclnt/ypclnt_get.c` — not in `SRCS`, though `ypclnt.h`
  declares `ypclnt_get()`.  It could not compile if it were asked to:
  its only `#include` is `"ypclnt.h"`, which pulls in no `<stddef.h>`,
  so `NULL` and `strlen` are both undeclared in a 22-line file.
* `lib/libmd/mdXhl.c` — a template.  `lib/libmd/Makefile:163` turns it
  into `md4hl.c`, `md5hl.c`, `sha0hl.c` and the rest with a `sed` that
  rewrites `mdX` to each algorithm's name, so `#include "mdX.h"` becomes
  `#include "md5.h"` in the file that is actually compiled.  `mdX.h`
  does not exist and is not meant to.

### And krb5, whose headers the build makes in an object directory

`lib/libpam/modules/pam_ksu/Makefile`, under `.if ${MK_MITKRB5} != "no"`
— and `MITKRB5` is in `__DEFAULT_YES_OPTIONS`, so this is the ordinary
case:

```
CFLAGS+= -I${SRCTOP}/krb5/include
CFLAGS+= -I${OBJTOP}/krb5/util/profile
CFLAGS+= -include ${SRCTOP}/crypto/krb5/src/include/k5-int.h
```

`krb5/include` has a Makefile and no `krb5.h`; `krb5/util/profile` has a
Makefile and no `profile.h`.  Both are generated, into an `${OBJTOP}`
path this analyser has no answer for:

```
profile.h: profile.hin prof_err.h          (util/profile/Makefile:63)
        cat ${.ALLSRC} > ${.TARGET}

krb5.h: krb5.hin ${GENI_ET}                (include/krb5/Makefile:40)
        echo the include guard, cat ${.ALLSRC}, close it
```

`GENI_ET` is six error tables — `krb5_err`, `k5e1_err`, `kdb5_err`,
`kv5m_err`, `krb524_err`, `asn1_err` — each produced by `compile_et`,
which is MIT krb5's own shell script with `@AWK@` and `@DIR@`
substituted at configure time.  That is the whole of its configuration,
so it is run rather than reimplemented, like everything else in that
table.  `krb5.h` comes out at 350KB.

One detail cost a round: written as `krb5.h` at the top of the shim it
was on the include path and still not found, because
`crypto/krb5/src/include/krb5.h` is eight lines saying *"as of the 1.5
release … they're all moving to a `krb5/` subdirectory"* and then
`#include <krb5/krb5.h>`.  It goes where it is installed.

## `get_su_principal`: returns 0 for failure, and the caller frees the stack

The finding that came back once `pam_ksu.c` compiled:

```
lib/libpam/modules/pam_ksu/pam_ksu.c:120  [core.CallAndMessage]
    4th function call argument is an uninitialized value
```

The function says what it does, directly above itself:

> Returns 0 for success, or a com_err error code on failure.

And one arm returns 0 for failure.  The su-to-`root` path:

```c
	rv = krb5_unparse_name(context, default_principal, &principal_name);
	krb5_free_principal(context, default_principal);
	if (rv != 0) {
		...
		return (rv);
	}
	PAM_LOG("Default principal name: %s", principal_name);
	if (strcmp(target_user, superuser) == 0) {
		p = strrchr(principal_name, '@');
		if (p == NULL) {
			PAM_LOG("malformed principal name `%s'", principal_name);
			free(principal_name);
			return (rv);            /* <-- rv is provably 0 */
		}
```

`rv` there is `krb5_unparse_name()`'s return, which the four lines above
tested non-zero and passed.  So the function reports **success** having
written neither `*su_principal_name` nor a principal, and the caller
does:

```c
	rv = get_su_principal(context, user, ruser, &su_principal_name, &su_principal);
	if (rv != 0)
		return (PAM_AUTH_ERR);
	PAM_LOG("kuserok: %s -> %s", su_principal_name, user);
	...
	free(su_principal_name);
```

— prints an uninitialised stack pointer and hands it to `free()`, on the
su-to-root path of `pam_ksu(8)`.  `KRB5_PARSE_MALFORMED` is what
`krb5_err.et:181` calls a principal name with no realm, and is what the
arm returns now.

The report does not go away, and that is worth stating rather than
hiding: the path clang takes after the fix is

```
pam_ksu.c:238  Assuming 'rv' is not equal to 0
pam_ksu.c:239  Returning without writing to '*'
```

which is `rv = seteuid(ruid); if (rv != 0) return (errno);` — one of
three sites in the function that return `errno`.  A zero `errno` there
would be success again, and the analyser cannot know a failed
`seteuid(2)` sets it.  That is the unconstrained-global family, not this
defect; the defect was the arm that could not fail to return zero.

## The libs shard now covers all of lib

Every one of the fifty-five `ERROR` translation units under `lib` is on
the record, so the analyse shard is widened from

    --scope lib/libc --scope lib/msun --scope libexec

to

    --scope lib --scope libexec

and `lib` comes off `check_shards.py`'s `UNANALYSED` list, where its note
had read *"lib/libc and lib/msun are sharded; the rest is not yet"* for
as long as that list has existed.  It moves to `UNCHECKED`, which is the
model-check job's list and where the same sentence is still true: CBMC
over 539 more translation units is a different order of cost from clang
over them.

| | TUs | OK | ERROR | findings |
|---|---|---|---|---|
| the old shard, `lib/libc` + `lib/msun` + `libexec` | 1,630 | 1,600 | 30 | 238 |
| all of `lib` | 2,169 | 2,114 | 55 | 441 |
| ...plus `libexec` | 103 | 99 | 4 | — |

`ok all 55 ERROR translation unit(s) are on the record`, and the
libraries that had never been analysed at all — libdevstat, lib/clang,
libpmc, libutil, libcasper, libfetch, libsysdecode, libnv, libthr,
libypclnt, libpam and the rest — are analysed on every push from here.

## The 209 findings the widened shard newly reports

`--scope lib` is 441 findings against the old shard's 238.  209 of them
are outside `lib/libc` and `lib/msun`, in libraries nothing had ever
analysed:

| library | findings | | checker | findings |
|---|---|---|---|---|
| libdevstat | 54 | | `unix.Malloc` | 75 |
| lib/clang | 45 | | `core.NullDereference` | 63 |
| libpmc | 21 | | `core.CallAndMessage` | 26 |
| libutil | 11 | | `core.UndefinedBinaryOperatorResult` | 13 |
| libcasper | 10 | | `unix.cstring.NullArg` | 11 |
| libfetch | 6 | | `core.uninitialized.Assign` | 10 |
| a tail of 24 more | 62 | | five more | 11 |

Two of the top three are each **one thing**.

### All 45 of `lib/clang` are one generated file

`lib/clang/liblldb/LLDBWrapLua.cpp`, 2.8MB, whose second line reads

> This file was automatically generated by SWIG (https://www.swig.org).
> Do not make changes to this file unless you know what you are doing —
> modify the SWIG interface file instead.

Every one is *"Called C++ object pointer is null"* on a `self->` inside
a `%extend` body — `lldb_SBWatchpoint___repr__(lldb::SBWatchpoint *self)`
and its forty-four siblings.  The null is SWIG's own
`arg1 = (lldb::SBWatchpoint *)0;` in the wrapper, which the wrapper then
guards with `if (!SWIG_IsOK(res1)) SWIG_fail;` — and the `%extend` body
is a `SWIGINTERN` function the analyser also enters as its own entry
point, with `self` unconstrained.  The characterised
static-function-as-entry-point family, in code no one in this tree
writes: the interface file it is generated from is
`contrib/llvm-project`'s.

### And 53 of libdevstat's 54 are one function and one unwritten invariant

`devstat_compute_statistics()` takes a variadic list of
(metric, pointer) pairs.  For each metric it decides which type to pull
off the `va_list` **from a table**:

```c
	switch (devstat_arg_list[metric].argtype) {
	case DEVSTAT_ARG_UINT64:
		destu64 = (u_int64_t *)va_arg(ap, u_int64_t *);
		break;
	case DEVSTAT_ARG_LD:
		destld = (long double *)va_arg(ap, long double *);
```

and then, one screen further down, which of those two pointers to write
through, from a `switch (metric)` with one case per metric:

```c
	case DSM_TOTAL_BYTES:
		*destu64 = totalbytes;
	...
	case DSM_KB_PER_TRANSFER:
		*destld = ...;
```

The two switches agree **only while row `i` of the table belongs to
metric `i`**.  The enum is in `devstat.h`; the table is in `devstat.c`;
and nothing at all connected them.  Add a metric to the middle of the
enum without adding its row and every metric after it reads the wrong
`argtype`: `va_arg()` takes a `long double *` where the caller passed a
`u_int64_t *`, and a 16-byte write lands in eight bytes of the caller's
storage — or the reverse, and the caller's `long double` is left holding
half a counter.

The invariant holds today — checked, not assumed: 45 metrics, 45 rows,
`devstat_arg_list[i].metric == i` for every `i`.  That is *why* the 53
findings are false: clang cannot enumerate forty-five table rows to
correlate the two switches, so on its modelled path the `argtype` comes
from one row and the `case` from another.

**A file that reports fifty-three findings resting on an unwritten
invariant is a file where the invariant is worth writing down.**  Two
halves, because they need different tools:

```c
_Static_assert(nitems(devstat_arg_list) == DSM_MAX,
    "devstat_arg_list is indexed by devstat_metric and must have a row "
    "for every metric, in order");
```

is the **length**, which a compiler can check on every build, for
everyone.  A compiler cannot check the **order** — a table with the
right number of rows in the wrong order compiles fine — so
`tools/verify/devstat_metric_table.py` reads both files and compares
them position by position, and is a gate.  Its cases include a metric
added to the enum and not the table, a row added to the table and not
the enum, and the same rows in the wrong order, which is the case the
`_Static_assert` cannot see.

That is the third invariant this project has written down because a
cluster of findings rested on it, after the allwinner zero-based clock
factor and the linuxulator's `lxs_args_cnt[]`.

## `net_getaddrinfo`: a sandboxed caller's errno was uninitialised stack

Casper's `cap_net` service. `net_getaddrinfo()` declares

```c
	int error, serrno, family, n;
```

and assigns `serrno` in exactly one place — `serrno = errno;` after
`getaddrinfo(3)`.  Three early exits jump over it.  The first sets it
itself:

```c
	if (!net_allowed_mode(limits, CAPNET_NAME2ADDR)) {
		serrno = ENOTCAPABLE;
		error = EAI_SYSTEM;
		goto out;
	}
```

The other two — the family limit and the host limit — do not:

```c
	if (!net_allowed_family(funclimit, family)) {
		errno = ENOTCAPABLE;        /* the GLOBAL, one letter away */
		error = EAI_SYSTEM;
		goto out;
	}
	if (!net_allowed_hosts(funclimit, hostname, servname)) {
		errno = ENOTCAPABLE;
		error = EAI_SYSTEM;
		goto out;
	}
```

and `out:` is

```c
	if (error == EAI_SYSTEM)
		nvlist_add_number(nvlout, "errno", serrno);
```

`serrno` is never written on either path.  **That number crosses the
sandbox boundary**: the client half of this service, `cap_net.c:316`,
does

```c
	errno = (error == EAI_SYSTEM) ? serrno : 0;
```

so a Capsicum-sandboxed process denied by a Casper limit received an
uninitialised stack word *from the service process* as its `errno` —
a wrong error for every such denial, and a word of another process's
stack with it.

What was meant is not in doubt, because the twin function in the same
file does it correctly at all three of its early exits:
`net_getnameinfo()` sets `serrno = ENOTCAPABLE` at `:870`, `:906` and
`:923`.  `net_getaddrinfo()` got it right once out of three.

`lib/libcasper/services/cap_net`: **2 translation units, 0 findings**
after.

### A note on the marker, because it nearly did not bite

The first marker written for this fix was `("serrno = ENOTCAPABLE;", 4)`
— and the *unfixed* file already has four, three in `net_getnameinfo`
and one in `net_getaddrinfo`.  Reverting the fix left the marker
satisfied, and `check_pbsd_marks.py` exited 0 when it should have
exited 1.  The revert-verification is the only thing that says so;
a count that matches before the fix is a marker that guards nothing.
Six.

## Three families, one of them new and measured

### The cleared flag, which the analyser cannot follow through a mask

`setusercontext()` in `lib/libutil/login_class.c` says exactly the right
thing and does exactly the right thing:

```c
	/* we need a passwd entry to set these */
	if (pwd == NULL)
		flags &= ~(LOGIN_SETGROUP | LOGIN_SETLOGIN);
	...
	if (flags & LOGIN_SETGROUP) {
		if (setgid(pwd->pw_gid) != 0) {          /* reported */
	...
	if ((flags & LOGIN_SETLOGIN) && setlogin(pwd->pw_name) != 0) {
```

and `setclasscontext()`, the caller on the reported path, is belt *and*
braces: it masks `flags` down to `RESOURCES|PRIORITY|UMASK|PATH` before
passing `pwd = NULL`.

clang reports both dereferences anyway, and the path says why:
*"Assuming the condition is true"* on `flags & LOGIN_SETGROUP`, after
the line that cleared that very bit.  Its constraint manager tracks
ranges, not bits, and cannot carry `flags &= ~F` into `flags & F`.

Nine lines settle it rather than an argument:

```c
struct s { int x; };
int
f(const struct s *p, unsigned int flags)
{
	if (p == 0)
		flags &= ~(1u | 2u);
	if (flags & 1u)
		return (p->x);
	return (0);
}
```

```
maskprobe.c:8:10: warning: Access to field 'x' results in a dereference
                  of a null pointer [core.NullDereference]
maskprobe.c:5:6: note: Assuming 'p' is equal to null
maskprobe.c:7:6: note: Assuming the condition is true
```

**The cleared flag** joins the characterised families.  It is worth
naming because it is the shape of a correct guard, not a missing one:
every finding in it is a place where somebody already did the right
thing.

### Two traversals of the same array

`gr_util.c`'s `grcopy()` indexes `newgr->gr_mem[i]`, which is `NULL`
when its `ndx` argument is zero.  `ndx` comes from `grmemlen()`, which
counted `gr->gr_mem` two statements earlier in `gr_add()`, so `ndx == 0`
means the loop `for (; gr->gr_mem[i] != NULL; i++)` runs zero times and
the index never happens.  The agreement is between two separate walks of
the same array — and the function's own comment at `:545` shows the
author reasoning about exactly this for the `name` case one line down,
and not for the loop.

### `return (errno)`, which is zero as far as the analyser knows

`pidfile_signal()` in `lib/libutil` reads `pid` after
`errno = pidfile_read(...); if (errno != 0) return (errno);`.  The path
clang takes runs through `pidfile_read_impl`'s
`fd = openat(...); if (fd == -1) return (errno);` — a failed `openat(2)`
whose `errno` it cannot bound away from zero, so the caller's `!= 0`
test is "assumed false" and `pid` is read unwritten.  The same shape
that survives in `pam_ksu` after the real defect there was fixed.

### And what is left

Of the 209, 45 are the SWIG file, 53 the devstat table, 2 the cleared
flag, 1 the gr_util pair, 1 the `errno` return, and 1 was the cap_net
defect.  The remaining ~106 are a read backlog, honestly labelled: 20 in
`lib/libpmc/pmu-events/jevents.c` (a build-time generator imported from
Linux perf, leaking on its own error paths), 9 in
`lib/libcasper/services/cap_fileargs/tests` (one shape across nine ATF
test bodies), 7 in `lib/libutil/mntopts.c`, and a long tail across
twenty-four libraries that had never been compiled by anything until
this pass.

## `execv_script`: a verified-execution library that returns success when it refused

`lib/libveriexec/exec_script.c`.  HardenedBSD's own library for running
a script under veriexec.

```c
int
execv_script(const char *interpreter, char * const *argv)
{
	const char *script;
	int rc;

	script = argv[0];
	if (veriexec_check_path(script) == 0) {
		rc = execv(script, argv);
	}
	/* still here? we might be allowed to run via interpreter */
	if (gbl_check_pid(0) & GBL_VERIEXEC) {
		if (!interpreter)
			interpreter = find_interpreter(script);
		if (interpreter) {
			...
			rc = execv(interpreter, argv);
		}
	}
	return (rc);
}
```

`rc` is assigned in exactly two places, both inside conditions.  Two
paths reach `return (rc)` with it never written:

* the script is **refused** by veriexec and `GBL_VERIEXEC` is not set;
* the script is refused, `GBL_VERIEXEC` is set, and `find_interpreter()`
  finds no `#!` line.

The function's own comment says *"@return error on failure usually EPERM
or EAUTH"*, and the caller reads it to decide whether the script ran.
A garbage non-zero reads as some errno.  **A garbage zero reads as
success** — "the script was executed" — when verification refused it and
nothing ran.  That is the wrong direction for a verified-execution
library to fail in.

`veriexec_check_path()` has already computed the reason (`EAUTH` for an
unverified path, `veriexec_check.c:54`), so `rc` holds it now:

```c
	rc = veriexec_check_path(script);
	if (rc == 0) {
		rc = execv(script, argv);
	}
```

`lib/libveriexec`: 4 translation units, **0 findings**.

## `build_iovec`: the same `realloc` idiom, in every `mount_*`

```c
	*iov = realloc(*iov, sizeof **iov * (i + 2));
	if (*iov == NULL) {
		*iovlen = -1;
		return;
	}
```

The libfetch shape again, found the same day, and worse here in one
respect: the caller **cannot** free the lost block, because `*iov` is
now NULL and `*iovlen` is -1 — so every option name `strdup()`ed into
the array so far goes with it, and the `free_iovec()` on the way out
finds nothing to free.  Every `mount_*` program builds its `nmount(2)`
arguments through this function.

Through a temporary.  `lib/libutil`: **11 findings → 5**, the six
`unix.Malloc` reports in `free_iovec()` gone with it.

The five that remain are each in a family already named above: two in
`login_class.c` are the cleared flag, `gr_util.c:541` is the two
traversals, `pidfile.c:369` is `return (errno)`, and `mntopts.c:318` is
an interface-level ownership question rather than a bug —
`free_iovec()` deliberately frees only the even (name) slots, because
callers pass string literals and stack buffers as values, and
`build_iovec_argf()` is the one caller that `strdup()`s one.

## A fifth family: the intrusive queue(3) macros

Six `unix.Malloc` *"Use of memory after it is freed"* across
`libmemstat`, `libopenbsd`'s `imsg`, `libmixer` and `libusb` are one
thing.  Every one is the ordinary BSD drain loop:

```c
	while (!TAILQ_EMPTY(&m->devs)) {
		dp = TAILQ_FIRST(&m->devs);
		TAILQ_REMOVE(&m->devs, dp, devs);
		...
		free(dp);
	}
```

The analyser's path, read out of the plist rather than guessed:

```
mixer.c:150  Assuming field 'tqh_first' is not equal to null
mixer.c:150  Entering loop body
mixer.c:152  Assuming field 'tqe_next' is equal to null
mixer.c:153  Assuming field 'tqh_first' is not equal to null
```

It is assuming values for `tqh_first` and `tqe_next` **that
`TAILQ_REMOVE` has just written**.  The list is intrusive — the links
live in the element being freed — and clang models those fields as
independent symbols, so on the next turn of the loop `TAILQ_FIRST` still
yields the element that was unlinked and freed.  A use-after-free in a
drain loop is exactly what this checker is for; it cannot see the
unlink that makes this one safe.

## Two more read to a verdict, and the shape of what is left

### `cgialloc`: a division the superblock validator makes safe

```
lib/libufs/cgroup.c:164  [core.DivideZero]  Division by zero
```

The path is unambiguous about which divisor:

```
cgroup.c:160  Assuming 'i' is >= field 'fs_inopb'
cgroup.c:160  Loop body executed 0 times
cgroup.c:164  Division by zero
```

`i` starts at zero, so `fs_inopb == 0` — and `ino_to_fsba()` divides by
`INOPB(fs)`.  `fs_inopb` is a **superblock field read from disk**, which
is to say from a filesystem image, which is to say from anywhere; and
`libufs` is what `fsck_ffs(8)`, `newfs(8)`, `tunefs(8)` and `dumpfs(8)`
read superblocks with.  Worth chasing.

It is safe, and the guarantee is exact.  `disk->d_fs` is filled only by
`sbread()`/`sbget()`, which is `ffs_sbget()` in
`sys/ufs/ffs/ffs_subr.c` — the *same* code the kernel uses — and
`validate_sblock()` there rejects any superblock where

```c
	FCHK(fs->fs_inopb, !=, fs->fs_bsize / sizeof(struct ufs2_dinode), %jd);
```

(and the UFS1 equivalent at `:629`), with `fs_bsize` itself bounded
below by `MINBSIZE`.  So `fs_inopb` is never zero in a superblock
`libufs` accepted.  The unseen-callee family, with the callee named: a
validator in another translation unit that the analyser does not open.

### `jailparam_get`: a contract the library does not enforce

Three `unix.cstring.NullArg` in `lib/libjail/jail.c` are one shape:

```c
	if (jp[j].jp_value == NULL &&
	    !(jp[j].jp_flags & JP_RAWVALUE)) {
		jp[j].jp_value = malloc(jp[j].jp_valuelen);
		...
	}
	jiov[i].iov_base = jp[j].jp_value;
	jiov[i].iov_len = jp[j].jp_valuelen;
	memset(jiov[i].iov_base, 0, jiov[i].iov_len);
```

The guard contemplates `jp_value == NULL` and then memsets it anyway
when `JP_RAWVALUE` is set — the flag that means *the caller supplied the
buffer*.  A caller that sets `JP_RAWVALUE` with a null value is in
breach of the interface, and hardening the `memset` alone would only
move the failure one line: `iov_base` still goes to `jail_get(2)` as
NULL with a non-zero `iov_len`.  Left as it is, on the record as an API
contract rather than a defect.

### The shape of the rest

Of the 209, after this pass:

| | |
|---|---|
| the SWIG-generated `LLDBWrapLua.cpp` | 45 |
| the devstat metric table | 53 |
| fixed defects (`cap_net`, `build_iovec`, `execv_script`) | 8 |
| the intrusive queue(3) macros | 6 |
| `jevents.c`, a build-time generator imported from Linux perf | 20 |
| `cap_fileargs/tests`, one shape across nine ATF bodies | 9 |
| the cleared flag | 2 |
| named singly above (`gr_util`, `pidfile`, `cgialloc`, `jailparam_get` ×3, `mntopts`) | 8 |
| **read and accounted for** | **151** |
| a tail across twenty libraries, unread | 58 |

The 58 are not hidden and not excused — they are in a scope that is now
analysed on every push, which is the difference this pass made.  Before
it, all 209 were invisible.

### The shard as CI now runs it

`--scope lib --scope libexec --check-errors`, on the tree with this
pass's six fixes in it:

```
438 finding(s) across 2272 translation units
  ERROR    59
  OK       2213

ok    all 59 ERROR translation unit(s) are on the record
```

against the shard it replaces — `lib/libc` + `lib/msun` + `libexec`,
238 findings across 1,630 translation units with 30 ERROR.  **642 more
translation units are compiled and analysed on every push**, and the
200 findings in them are visible for the first time.

## Three more from the tail, all counters and restores

### `bsde_parse_rule_string`: `argc` counted tokens it never stored

`lib/libugidfw` is what turns a `mac_bsdextended(4)` rule string into a
`struct mac_bsdextended_rule`.  `ugidfw(8)` calls it, and so does
anything applying rules out of `rc.conf`.

```c
	char *stringdup, *stringp, *argv[100], **ap;
	...
	argc = 0;
	for (ap = argv; (*ap = strsep(&stringp, " \t")) != NULL;) {
		argc++;
		if (**ap != '\0')
			if (++ap >= &argv[100])
				break;
	}

	error = bsde_parse_rule(argc, argv, rule, buflen, errstr);
```

`ap` advances only for a **non-empty** token — that is deliberate, it is
how a run of separators is skipped — but `argc` counts every `strsep()`
result, empty ones included.  So two spaces anywhere in the rule make
`argc` larger than the number of slots written, and `bsde_parse_rule()`
walks `argv[0..argc-1]`: it reads past the last token into an
uninitialised `char *argv[100]` and `strcmp()`s whatever is on the
stack.  That is what the two `unix.cstring.NullArg` reports at
`ugidfw.c:1023` and `:1034` were.

`argc` now counts the tokens actually stored, which is the invariant
`bsde_parse_rule()` needs.  Three findings → one.

The `strdup()` feeding that loop was unchecked too —
`stringp = stringdup = strdup(string);` followed immediately by
`while (*stringp == ' ')`.

### `sbput`: one of two restore sites had the test

```c
	if (fs->fs_si != NULL) {
		savedcsp = fs->fs_csp;
		fs->fs_csp = NULL;
	}
	for (i = 0; i < numaltwrite; i++) {
		if ((error = ffs_sbput(...)) != 0) {
			fs->fs_sblockactualloc = savedactualloc;
			fs->fs_csp = savedcsp;        /* unguarded */
			return (error);
		}
	}
	fs->fs_sblockactualloc = savedactualloc;
	if (fs->fs_si != NULL)                /* guarded */
		fs->fs_csp = savedcsp;
```

`savedcsp` is set only inside the first `if`.  With no summary
information, a failed alternate-superblock write wrote an
**uninitialised stack pointer** into the caller's `struct fs` as
`fs_csp` — the in-core cylinder-group summary that `newfs(8)`,
`fsck_ffs(8)` and `tunefs(8)` go on to use.  The same fingerprint as
`cap_net`: two sites, one right.

The two reports that remain after guarding it are the unseen-callee
family, and the callee is known: `ffs_sbput()`
(`sys/ufs/ffs/ffs_subr.c`) saves `fs->fs_si`, clears it for the write
and restores it before returning, so the two `fs_si != NULL` tests do
agree — across a translation-unit boundary the analyser does not cross.

### `libpfctl`: three counters `pfctl(8)` prints to the operator

```c
	uint32_t added;
	...
	while ((hdr = snl_read_reply_multi(&h->ss, seq_id, &e)) != NULL) {
		if (! snl_parse_nlmsg(&h->ss, hdr, &table_add_addr_parser, &added))
			continue;
	}

	if (nadd)
		*nadd = added;
```

`table_add_addr_parser` has one attribute, `PF_TA_NBR_ADDED`, and
`snl_parse_nlmsg()` succeeds whether or not the reply carries it — and a
reply loop that runs zero times writes nothing at all.  `*nadd` then
hands the caller a stack word, and `pfctl(8)` prints it as *"N/M
addresses added"*.

Three times: `_pfctl_table_add_addrs_h`, `_pfctl_table_del_addrs_h`, and
`pfctl_clear_addrs` — pf's table add, delete and flush counts, all to
the same operator.  Zero is what *"the kernel reported no count"* means,
and matches the `struct snl_errmsg_data e = {}` two lines above each of
them.  `lib/libpfctl`: 5 findings → 3, and the two that remain are the
`return (errno)` family via `_pfctl_get_limit()`.

## Three parsers, and a round trip of garbage back to its owner

### `next_field`: reading the caller's OUTPUT parameter as an input

`lib/libcam/scsi_cmdparse.c` is `camcontrol(8)`'s SCSI command format
string engine.  `next_field()` parses one field and hands the result
back through `*value_p`.  Four of its arms — `v`, `i`, `t` and the `v`
after a seek — began with

```c
	value = *value_p;
```

reading the caller's *output* parameter as if it were an input.
`do_encode()` declares it as a bare `int value` and never writes it
before the call, so that was a read of an indeterminate object.

And a **dead** one: every one of those four arms sets `something = 2`,
the function returns it, and `if (ret == 2)` in `do_encode()` replaces
`value` with the `va_arg` or with zero.  A round trip of garbage back to
its owner.  `next_field`'s own `int value = 0` already had the right
answer.  `lib/libcam`: **4 findings → 0**.

### `parse_config`: one character of input crashed it

`lib/libfigpar` parses `rc.conf`-shaped configuration files.

```c
		/* Find the length of the directive */
		for (n = 0; r != 0; n++) {
			if (isspace(*p))
				break;
			if (bequals && *p == '=') {
				have_equals = 1;
				break;
			}
			...
		}
		if (n == 0 && r == 0) { ... return (0); }
		...
		if (n > dsize) {
			if ((directive = realloc(directive, n + 1)) == NULL) {
```

The length loop breaks at `n == 0` when the first character is `=` — the
`have_equals` arm stops it before its first `read(2)`.  `n == 0` with
`r != 0` falls past the EOF test.  And with `dsize` also zero, `n >
dsize` is false, so the buffer is **never allocated** — and four lines
down,

```c
		directive[n] = '\0';
```

writes through NULL.  A configuration file whose first directive line
begins with `=` crashed the parser.  The value buffer has the identical
shape at `n > vsize`.

### `strexpand`: an escape that eats a character it was not given

Same library, the function that resolves `\n`, `\xNN` and `\0NNN` in a
parsed value.

```c
		case '0': /* octal value (0 to 3 digits)(\0NNN) */
			d[3] = '\0'; /* pre-terminate the string */

			d[0] = (isdigit(*(chr+1)) && *(chr+1) < '8') ? *++chr : '\0';
			if (d[0] != '\0')
				d[1] = (isdigit(*(chr+1)) && *(chr+1) < '8') ? *++chr : '\0';
			if (d[1] != '\0')
				d[2] = (isdigit(*(chr+1)) && *(chr+1) < '8') ? *++chr : '\0';
```

`d[1]` is written only inside `if (d[0] != '\0')`, and the next test
reads it either way.  With no octal digit after the backslash-zero,
`if (d[1] != '\0')` reads an **indeterminate byte** — and when it
happens to be non-zero, the line after it does `*++chr`, eating a
character of the string that is not part of the escape.  The result of
`strtoul()` is zero either way; what changes is how much input the
expander consumed, non-deterministically.

The hex case four lines above has the same shape and is safe by
accident: it never *tests* `d[1]`, and `strtoul()` stops at `d[0]`.

All four bytes are pre-terminated now, which is what the author's own
*"pre-terminate the string"* comment was reaching for.  `lib/libfigpar`:
**4 findings → 1**.

The one that remains is not a spot fix and is recorded as what it is:
`parse_config()` never frees `directive` or `value` on **any** of its
twenty-odd `return` paths, and `return (-1)` at `:413` does not even
`close(fd)`.  That is a single-exit refactor of a function in a
component `MK_DIALOG` gates off by default, and it is its own change.
