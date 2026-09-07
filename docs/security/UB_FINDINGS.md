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
| `:17384` `res = lentim / rate_wanted` | `:17352` does `if (((bw_est == 0) \|\| (rate_wanted == 0) \|\| ...)) goto old_method;`. The one thing between that and the division is `rack_rate_cap_bw()`, which can only lower it — and both of its assignments (`:2199`, `:2223`) are guarded on the new value being positive, which is two of the four rows in the table above. |
| `:2496` `bw_est = high_rate` | the same unguarded shape as `:17185`, but `bw_est` leaves the function as a bandwidth rather than a divisor, and its caller rejects zero at `:17352` before dividing. Defended downstream rather than at the site — worth knowing if that caller ever changes. |

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
| `sys/net/if.c:1757-1759` (three), and every `fail:` label after an `M_ZERO` allocation | `ifa_alloc()` does `malloc(size, M_IFADDR, M_ZERO | flags)`, so all four counter fields are NULL before any of them is assigned, and its `fail:` path says so — `/* free(NULL) is okay */`. The analyser does not model `M_ZERO`, so every field of a zeroed allocation is an uninitialised value to it. This is a large class in a kernel that zeroes most of what it allocates. |
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
| `kern_condvar.c` ×4, `kern_synch.c:234`, `kern_exit.c:1576`, `uipc_sockbuf.c:747` — "2nd function call argument is an uninitialized value" | one idiom, seven times. `WITNESS_SAVE_DECL(lock_witness)` declares it, `WITNESS_SAVE(lock, lock_witness)` fills it inside `if (lock != &Giant.lock_object)`, and `WITNESS_RESTORE(lock, lock_witness)` reads it inside the same test with `lock` a parameter nothing between them touches. (`lock_state`, the argument it looks like at a glance, is initialised to 0 unconditionally.) |
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
