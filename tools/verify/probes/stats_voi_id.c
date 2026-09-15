/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * sys/kern/subr_stats.c: three exported entry points take an int32_t
 * voi_id from their caller and two of them bounded it at one end.
 *
 *   cbmc -DOLD --unwind 4 --bounds-check --signed-overflow-check \
 *       tools/verify/probes/stats_voi_id.c   -> 2 of 2 failed, FAILED
 *   cbmc       --unwind 4 --bounds-check --signed-overflow-check \
 *       tools/verify/probes/stats_voi_id.c   -> 0 of 2, SUCCESSFUL
 *
 * NVOIS(sb) casts to int32_t:
 *
 *     #define NVOIS(sb) ((int32_t)((((struct statsblobv1 *)(sb))->stats_off
 *                        - sizeof(struct statsblobv1)) / sizeof(struct voi)))
 *
 * so `voi_id >= NVOIS(sb)' is a SIGNED comparison and a negative voi_id
 * passes it.  &sb->vois[voi_id] then addresses before the array.
 *
 *   stats_v1_voistat_fetch_dptr() reads v->voistatmaxid and v->stats_off
 *   from there and returns a pointer built from them to the caller.
 *
 *   stats_v1_voi_update() reads v->dtype, v->id and v->flags and, if
 *   they happen to agree, WRITES a statistic through
 *   BLOB_OFFSET(sb, v->stats_off) -- an offset itself read out of
 *   bounds.
 *
 * Both are declared in <sys/stats.h> and built into lib/libstats, whose
 * Makefile has `SRCS= subr_stats.c tcp_stats.c', so they are a shared
 * library's exported interface as well as a kernel one.  The header's
 * inline family -- stats_voistat_fetch_s64, _u64, _s32, _u32 and
 * stats_voi_update_abs_s32 and its relatives -- funnels into exactly
 * these two with the application's own int32_t.
 *
 * stats_v1_tpl_add_voistats(), the third, opens with
 *
 *     if (voi_id < 0 || voi_dtype == 0 || ...)
 *             return (EINVAL);
 *
 * which is what makes this a slip rather than a convention.
 */

int nondet_int(void);

#define	NVOIS_MODEL	8

int
main(void)
{
	int voi_id = nondet_int();	/* the caller's int32_t */
	int nvois = NVOIS_MODEL;
	int idx;

	/* stats_v1_voistat_fetch_dptr(): the reading path. */
#ifdef OLD
	if (voi_id >= nvois)
		return (0);
#else
	if (voi_id < 0 || voi_id >= nvois)
		return (0);
#endif
	idx = voi_id;
	__CPROVER_assert(idx >= 0 && idx < nvois,
	    "fetch_dptr subscripts sb->vois[] inside the array");

	/* stats_v1_voi_update(): the writing path, same bound. */
#ifdef OLD
	if (voi_id >= nvois)
		return (0);
#else
	if (voi_id < 0 || voi_id >= nvois)
		return (0);
#endif
	__CPROVER_assert(voi_id >= 0 && voi_id < nvois,
	    "voi_update subscripts sb->vois[] inside the array");

	return (0);
}
