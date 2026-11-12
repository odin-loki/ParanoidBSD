static int map_tboot_pages(unsigned long vaddr, unsigned long start_pfn,
			   unsigned long nr)
{
	/* Reuse the original kernel mapping */
	tboot_pg_dir = pgd_alloc(&tboot_mm);
	if (!tboot_pg_dir)
		return -1;

	for (; nr > 0; nr--, vaddr += PAGE_SIZE, start_pfn++) {
		if (map_tboot_page(vaddr, start_pfn, PAGE_KERNEL_EXEC))
			return -1;
	}

	return 0;
}
