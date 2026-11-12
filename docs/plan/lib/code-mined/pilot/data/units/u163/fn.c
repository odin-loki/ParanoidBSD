static bool xennet_tx_buf_gc(struct netfront_queue *queue)
{
	RING_IDX cons, prod;
	unsigned short id;
	struct sk_buff *skb;
	bool more_to_do;
	bool work_done = false;
	const struct device *dev = &queue->info->netdev->dev;

	BUG_ON(!netif_carrier_ok(queue->info->netdev));

	do {
		prod = queue->tx.sring->rsp_prod;
		if (RING_RESPONSE_PROD_OVERFLOW(&queue->tx, prod)) {
			dev_alert(dev, "Illegal number of responses %u\n",
				  prod - queue->tx.rsp_cons);
			goto err;
		}
		rmb(); /* Ensure we see responses up to 'rp'. */

		for (cons = queue->tx.rsp_cons; cons != prod; cons++) {
			struct xen_netif_tx_response txrsp;

			work_done = true;

			RING_COPY_RESPONSE(&queue->tx, cons, &txrsp);
			if (txrsp.status == XEN_NETIF_RSP_NULL)
				continue;

			id = txrsp.id;
			if (id >= RING_SIZE(&queue->tx)) {
				dev_alert(dev,
					  "Response has incorrect id (%u)\n",
					  id);
				goto err;
			}
			if (queue->tx_link[id] != TX_PENDING) {
				dev_alert(dev,
					  "Response for inactive request\n");
				goto err;
			}

			queue->tx_link[id] = TX_LINK_NONE;
			skb = queue->tx_skbs[id];
			queue->tx_skbs[id] = NULL;
			if (unlikely(!gnttab_end_foreign_access_ref(
				queue->grant_tx_ref[id]))) {
				dev_alert(dev,
					  "Grant still in use by backend domain\n");
				goto err;
			}
			gnttab_release_grant_reference(
				&queue->gref_tx_head, queue->grant_tx_ref[id]);
			queue->grant_tx_ref[id] = INVALID_GRANT_REF;
			queue->grant_tx_page[id] = NULL;
			add_id_to_list(&queue->tx_skb_freelist, queue->tx_link, id);
			dev_kfree_skb_irq(skb);
		}

		queue->tx.rsp_cons = prod;

		RING_FINAL_CHECK_FOR_RESPONSES(&queue->tx, more_to_do);
	} while (more_to_do);

	xennet_maybe_wake_tx(queue);

	return work_done;

 err:
	queue->info->broken = true;
	dev_alert(dev, "Disabled for further use\n");

	return work_done;
}
