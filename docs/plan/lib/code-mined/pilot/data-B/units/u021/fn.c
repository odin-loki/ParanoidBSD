static int
bnad_txrx_irq_alloc(struct bnad *bnad, enum bnad_intr_source src,
		    u32 txrx_id, struct bna_intr_info *intr_info)
{
	int i, vector_start = 0;
	u32 cfg_flags;
	unsigned long flags;

	spin_lock_irqsave(&bnad->bna_lock, flags);
	cfg_flags = bnad->cfg_flags;
	spin_unlock_irqrestore(&bnad->bna_lock, flags);

	if (cfg_flags & BNAD_CF_MSIX) {
		intr_info->intr_type = BNA_INTR_T_MSIX;
		intr_info->idl = kcalloc(intr_info->num,
					sizeof(struct bna_intr_descr),
					GFP_KERNEL);
		if (!intr_info->idl)
			return -ENOMEM;

		switch (src) {
		case BNAD_INTR_TX:
			vector_start = BNAD_MAILBOX_MSIX_VECTORS + txrx_id;
			break;

		case BNAD_INTR_RX:
			vector_start = BNAD_MAILBOX_MSIX_VECTORS +
					(bnad->num_tx * bnad->num_txq_per_tx) +
					txrx_id;
			break;

		default:
			BUG();
		}

		for (i = 0; i < intr_info->num; i++)
			intr_info->idl[i].vector = vector_start + i;
	} else {
		intr_info->intr_type = BNA_INTR_T_INTX;
		intr_info->num = 1;
		intr_info->idl = kcalloc(intr_info->num,
					sizeof(struct bna_intr_descr),
					GFP_KERNEL);
		if (!intr_info->idl)
			return -ENOMEM;

		switch (src) {
		case BNAD_INTR_TX:
			intr_info->idl[0].vector = BNAD_INTX_TX_IB_BITMASK;
			break;

		case BNAD_INTR_RX:
			intr_info->idl[0].vector = BNAD_INTX_RX_IB_BITMASK;
			break;
		}
	}
	return 0;
}
