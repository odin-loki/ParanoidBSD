static int tse_tx_complete(struct altera_tse_private *priv)
{
	unsigned int txsize = priv->tx_ring_size;
	struct tse_buffer *tx_buff;
	unsigned int entry;
	int txcomplete = 0;
	u32 ready;

	spin_lock(&priv->tx_lock);

	ready = priv->dmaops->tx_completions(priv);

	/* Free sent buffers */
	while (ready && (priv->tx_cons != priv->tx_prod)) {
		entry = priv->tx_cons % txsize;
		tx_buff = &priv->tx_ring[entry];

		if (netif_msg_tx_done(priv))
			netdev_dbg(priv->dev, "%s: curr %d, dirty %d\n",
				   __func__, priv->tx_prod, priv->tx_cons);

		if (likely(tx_buff->skb))
			priv->dev->stats.tx_packets++;

		tse_free_tx_buffer(priv, tx_buff);
		priv->tx_cons++;

		txcomplete++;
		ready--;
	}

	if (unlikely(netif_queue_stopped(priv->dev) &&
		     tse_tx_avail(priv) > TSE_TX_THRESH(priv))) {
		if (netif_queue_stopped(priv->dev) &&
		    tse_tx_avail(priv) > TSE_TX_THRESH(priv)) {
			if (netif_msg_tx_done(priv))
				netdev_dbg(priv->dev, "%s: restart transmit\n",
					   __func__);
			netif_wake_queue(priv->dev);
		}
	}

	spin_unlock(&priv->tx_lock);
	return txcomplete;
}
