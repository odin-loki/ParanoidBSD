static u32 snp_cpuid_calc_xsave_size(u64 xfeatures_en, bool compacted)
{
	const struct snp_cpuid_table *cpuid_table = snp_cpuid_get_table();
	u64 xfeatures_found = 0;
	u32 xsave_size = 0x240;
	int i;

	for (i = 0; i < cpuid_table->count; i++) {
		const struct snp_cpuid_fn *e = &cpuid_table->fn[i];

		if (!(e->eax_in == 0xD && e->ecx_in > 1 && e->ecx_in < 64))
			continue;
		if (!(xfeatures_en & (BIT_ULL(e->ecx_in))))
			continue;
		if (xfeatures_found & (BIT_ULL(e->ecx_in)))
			continue;

		xfeatures_found |= (BIT_ULL(e->ecx_in));

		if (compacted)
			xsave_size += e->eax;
		else
			xsave_size = max(xsave_size, e->eax + e->ebx);
	}

	/*
	 * Either the guest set unsupported XCR0/XSS bits, or the corresponding
	 * entries in the CPUID table were not present. This is not a valid
	 * state to be in.
	 */
	if (xfeatures_found != (xfeatures_en & GENMASK_ULL(63, 2)))
		return 0;

	return xsave_size;
}
