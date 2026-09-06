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
