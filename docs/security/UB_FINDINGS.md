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
