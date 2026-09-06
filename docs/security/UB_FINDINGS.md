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
