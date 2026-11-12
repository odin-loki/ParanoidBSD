u64 gen8_ggtt_pte_encode(dma_addr_t addr,
			 unsigned int pat_index,
			 u32 flags)
{
	gen8_pte_t pte = addr | GEN8_PAGE_PRESENT;

	if (flags & PTE_LM)
		pte |= GEN12_GGTT_PTE_LM;

	return pte;
}
