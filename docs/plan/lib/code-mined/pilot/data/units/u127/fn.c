vm_offset_t
kva_alloc(vm_size_t size)
{
	vm_offset_t addr;

	TSENTER();
	size = round_page(size);
	if (vmem_xalloc(kernel_arena, size, 0, 0, 0, VMEM_ADDR_MIN,
	    VMEM_ADDR_MAX, M_BESTFIT | M_NOWAIT, &addr))
		return (0);
	TSEXIT();

	return (addr);
}
