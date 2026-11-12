static int
ixl_isc_rxd_available(void *arg, uint16_t rxqid, qidx_t idx, qidx_t budget)
{
	struct ixl_vsi *vsi = arg;
	struct rx_ring *rxr = &vsi->rx_queues[rxqid].rxr;
	union i40e_rx_desc *rxd;
	u64 qword;
	uint32_t status;
	int cnt, i, nrxd;

	nrxd = vsi->shared->isc_nrxd[0];

	for (cnt = 0, i = idx; cnt < nrxd - 1 && cnt <= budget;) {
		rxd = &rxr->rx_base[i];
		qword = le64toh(rxd->wb.qword1.status_error_len);
		status = (qword & I40E_RXD_QW1_STATUS_MASK)
			>> I40E_RXD_QW1_STATUS_SHIFT;

		if ((status & (1 << I40E_RX_DESC_STATUS_DD_SHIFT)) == 0)
			break;
		if (++i == nrxd)
			i = 0;
		if (status & (1 << I40E_RX_DESC_STATUS_EOF_SHIFT))
			cnt++;
	}

	return (cnt);
}
