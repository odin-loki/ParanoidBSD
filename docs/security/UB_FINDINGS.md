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
