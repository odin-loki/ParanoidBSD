int qlcnic_83xx_cam_lock(struct qlcnic_adapter *adapter)
{
	void __iomem *addr;
	u32 val, limit = 0;

	struct qlcnic_hardware_context *ahw = adapter->ahw;

	addr = ahw->pci_base0 + QLC_83XX_SEM_LOCK_FUNC(ahw->pci_func);
	do {
		val = readl(addr);
		if (val) {
			/* write the function number to register */
			QLC_SHARED_REG_WR32(adapter, QLCNIC_FLASH_LOCK_OWNER,
					    ahw->pci_func);
			return 0;
		}
		usleep_range(1000, 2000);
	} while (++limit <= QLCNIC_PCIE_SEM_TIMEOUT);

	return -EIO;
}
