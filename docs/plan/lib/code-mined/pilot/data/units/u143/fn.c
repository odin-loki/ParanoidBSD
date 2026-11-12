static int vntb_epf_db_clear(struct ntb_dev *ndev, u64 db_bits)
{
	struct epf_ntb *ntb = ntb_ndev(ndev);

	ntb->db &= ~db_bits;
	return 0;
}
