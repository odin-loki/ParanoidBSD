static int idt_ntb_db_clear(struct ntb_dev *ntb, u64 db_bits)
{
	struct idt_ntb_dev *ndev = to_ndev_ntb(ntb);

	idt_nt_write(ndev, IDT_NT_INDBELLSTS, (u32)db_bits);

	return 0;
}
