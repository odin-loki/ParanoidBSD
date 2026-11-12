static int alloc_fw_msg_bufs(struct ipu7_isys *isys, int amount)
{
	struct ipu7_bus_device *adev = isys->adev;
	struct isys_fw_msgs *addr;
	dma_addr_t dma_addr;
	unsigned long flags;
	unsigned int i;

	for (i = 0; i < amount; i++) {
		addr = ipu7_dma_alloc(adev, sizeof(struct isys_fw_msgs),
				      &dma_addr, GFP_KERNEL, 0);
		if (!addr)
			break;
		addr->dma_addr = dma_addr;

		spin_lock_irqsave(&isys->listlock, flags);
		list_add(&addr->head, &isys->framebuflist);
		spin_unlock_irqrestore(&isys->listlock, flags);
	}

	if (i == amount)
		return 0;

	spin_lock_irqsave(&isys->listlock, flags);
	while (!list_empty(&isys->framebuflist)) {
		addr = list_first_entry(&isys->framebuflist,
					struct isys_fw_msgs, head);
		list_del(&addr->head);
		spin_unlock_irqrestore(&isys->listlock, flags);
		ipu7_dma_free(adev, sizeof(struct isys_fw_msgs),
			      addr, addr->dma_addr, 0);
		spin_lock_irqsave(&isys->listlock, flags);
	}
	spin_unlock_irqrestore(&isys->listlock, flags);

	return -ENOMEM;
}
