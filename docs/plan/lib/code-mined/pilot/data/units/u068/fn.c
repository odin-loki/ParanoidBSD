static void ipw_prom_free(struct ipw_priv *priv)
{
	if (!priv->prom_net_dev)
		return;

	unregister_netdev(priv->prom_net_dev);
	free_libipw(priv->prom_net_dev, 1);

	priv->prom_net_dev = NULL;
}
