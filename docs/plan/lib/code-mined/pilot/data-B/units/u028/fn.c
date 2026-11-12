static int
lapic_calibrate_initcount_cpuid_vm(void)
{
	u_int regs[4];
	uint64_t freq;

	/* Get value from CPUID leaf if possible. */
	if (vm_guest == VM_GUEST_NO)
		return (false);
	if (hv_high < 0x40000010)
		return (false);
	do_cpuid(0x40000010, regs);
	freq = (uint64_t)(regs[1]) * 1000;

	/* Pick timer divisor. */
	lapic_timer_divisor = 2;
	do {
		if (freq / lapic_timer_divisor < APIC_TIMER_MAX_COUNT)
			break;
		lapic_timer_divisor <<= 1;
	} while (lapic_timer_divisor <= 128);
	if (lapic_timer_divisor > 128)
		return (false);

	/* Record divided frequency. */
	count_freq = freq / lapic_timer_divisor;
	return (count_freq != 0);
}
