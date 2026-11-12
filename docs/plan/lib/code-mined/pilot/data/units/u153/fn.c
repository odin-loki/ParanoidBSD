static enum slb_index alloc_slb_index(bool kernel)
{
	enum slb_index index;

	/*
	 * The allocation bitmaps can become out of synch with the SLB
	 * when the _switch code does slbie when bolting a new stack
	 * segment and it must not be anywhere else in the SLB. This leaves
	 * a kernel allocated entry that is unused in the SLB. With very
	 * large systems or small segment sizes, the bitmaps could slowly
	 * fill with these entries. They will eventually be cleared out
	 * by the round robin allocator in that case, so it's probably not
	 * worth accounting for.
	 */

	/*
	 * SLBs beyond 32 entries are allocated with stab_rr only
	 * POWER7/8/9 have 32 SLB entries, this could be expanded if a
	 * future CPU has more.
	 */
	if (local_paca->slb_used_bitmap != U32_MAX) {
		index = ffz(local_paca->slb_used_bitmap);
		local_paca->slb_used_bitmap |= 1U << index;
		if (kernel)
			local_paca->slb_kern_bitmap |= 1U << index;
	} else {
		/* round-robin replacement of slb starting at SLB_NUM_BOLTED. */
		index = local_paca->stab_rr;
		if (index < (mmu_slb_size - 1))
			index++;
		else
			index = SLB_NUM_BOLTED;
		local_paca->stab_rr = index;
		if (index < 32) {
			if (kernel)
				local_paca->slb_kern_bitmap |= 1U << index;
			else
				local_paca->slb_kern_bitmap &= ~(1U << index);
		}
	}
	BUG_ON(index < SLB_NUM_BOLTED);

	return index;
}
