int hfi1_netdev_rx_destroy(struct hfi1_devdata *dd)
{
	struct hfi1_netdev_rx *rx = dd->netdev_rx;

	/* destroy the RX queues only if it is the last netdev going away */
	if (atomic_fetch_add_unless(&rx->netdevs, -1, 0) == 1) {
		mutex_lock(&hfi1_mutex);
		hfi1_netdev_rxq_deinit(rx);
		mutex_unlock(&hfi1_mutex);
	}

	return 0;
}
