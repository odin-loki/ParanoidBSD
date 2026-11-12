static bool
lkpi_pci_bar_id_valid(int bar)
{
	if (bar < 0 || bar > PCIR_MAX_BAR_0)
		return (false);

	return (true);
}
