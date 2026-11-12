static inline uint16_t get_pcie_gen_support(uint32_t pcie_link_speed_cap,
					    uint16_t ns_pcie_gen)
{
	uint32_t asic_pcie_link_speed_cap = (pcie_link_speed_cap &
		CAIL_ASIC_PCIE_LINK_SPEED_SUPPORT_MASK);
	uint32_t sys_pcie_link_speed_cap  = (pcie_link_speed_cap &
		CAIL_PCIE_LINK_SPEED_SUPPORT_MASK);

	switch (asic_pcie_link_speed_cap) {
	case CAIL_ASIC_PCIE_LINK_SPEED_SUPPORT_GEN1:
		return PP_PCIEGen1;

	case CAIL_ASIC_PCIE_LINK_SPEED_SUPPORT_GEN2:
		return PP_PCIEGen2;

	case CAIL_ASIC_PCIE_LINK_SPEED_SUPPORT_GEN3:
		return PP_PCIEGen3;

	default:
		if (is_pcie_gen3_supported(sys_pcie_link_speed_cap) &&
			(ns_pcie_gen == PP_PCIEGen3)) {
			return PP_PCIEGen3;
		} else if (is_pcie_gen2_supported(sys_pcie_link_speed_cap) &&
			((ns_pcie_gen == PP_PCIEGen3) || (ns_pcie_gen == PP_PCIEGen2))) {
			return PP_PCIEGen2;
		}
	}

	return PP_PCIEGen1;
}
