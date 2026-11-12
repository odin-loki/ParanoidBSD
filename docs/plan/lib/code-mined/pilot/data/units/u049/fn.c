int npc_get_nixlf_mcam_index(struct npc_mcam *mcam,
			     u16 pcifunc, int nixlf, int type)
{
	struct rvu_hwinfo *hw = container_of(mcam, struct rvu_hwinfo, mcam);
	struct rvu *rvu = hw->rvu;
	int pf = rvu_get_pf(rvu->pdev, pcifunc);
	int index;

	/* Check if this is for a PF */
	if (pf && !(pcifunc & RVU_PFVF_FUNC_MASK)) {
		/* Reserved entries exclude PF0 */
		pf--;
		index = mcam->pf_offset + (pf * RSVD_MCAM_ENTRIES_PER_PF);
		/* Broadcast address matching entry should be first so
		 * that the packet can be replicated to all VFs.
		 */
		if (type == NIXLF_BCAST_ENTRY)
			return index;
		else if (type == NIXLF_ALLMULTI_ENTRY)
			return index + 1;
		else if (type == NIXLF_PROMISC_ENTRY)
			return index + 2;
	}

	return npc_get_ucast_mcam_index(mcam, pcifunc, nixlf);
}
