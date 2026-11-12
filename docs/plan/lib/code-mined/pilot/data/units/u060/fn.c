static int __init amiserial_console_init(void)
{
	if (!MACH_IS_AMIGA)
		return -ENODEV;

	register_console(&sercons);
	return 0;
}
