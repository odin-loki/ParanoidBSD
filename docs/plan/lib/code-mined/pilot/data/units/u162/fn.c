static bool
gve_tx_have_enough_qpl_bufs(struct gve_tx_ring *tx, int num_bufs)
{
	uint32_t available = tx->dqo.qpl_bufs_produced_cached -
	    tx->dqo.qpl_bufs_consumed;

	if (__predict_true(available >= num_bufs))
		return (true);

	tx->dqo.qpl_bufs_produced_cached = atomic_load_acq_32(
	    &tx->dqo.qpl_bufs_produced);
	available = tx->dqo.qpl_bufs_produced_cached -
	    tx->dqo.qpl_bufs_consumed;

	if (__predict_true(available >= num_bufs))
		return (true);
	return (false);
}
