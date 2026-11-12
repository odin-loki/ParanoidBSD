static int __init parse_amd_iommu_intr(char *str)
{
	for (; *str; ++str) {
		if (strncmp(str, "legacy", 6) == 0) {
			amd_iommu_guest_ir = AMD_IOMMU_GUEST_IR_LEGACY_GA;
			break;
		}
		if (strncmp(str, "vapic", 5) == 0) {
			amd_iommu_guest_ir = AMD_IOMMU_GUEST_IR_VAPIC;
			break;
		}
	}
	return 1;
}
