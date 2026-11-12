static int
mmcspi_acquire_host(device_t brdev, device_t reqdev)
{
	struct mmcspi_slot *slot;
	int err;

	TRACE_ENTER(brdev);
	err = 0;

	slot = device_get_ivars(reqdev);

	MMCSPI_LOCK_SLOT(slot);
	while (slot->bus_busy)
		mtx_sleep(slot, &slot->mtx, 0, "mmcspiah", 0);
	slot->bus_busy++;
	MMCSPI_UNLOCK_SLOT(slot);

	TRACE_EXIT(brdev);

	return (err);
}
