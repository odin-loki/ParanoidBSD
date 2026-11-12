int __mlx4_qp_reserve_range(struct mlx4_dev *dev, int cnt, int align,
			    int *base, u8 flags)
{
	u32 uid;
	int bf_qp = !!(flags & (u8)MLX4_RESERVE_ETH_BF_QP);

	struct mlx4_priv *priv = mlx4_priv(dev);
	struct mlx4_qp_table *qp_table = &priv->qp_table;

	if (cnt > MLX4_MAX_BF_QP_RANGE && bf_qp)
		return -ENOMEM;

	uid = MLX4_QP_TABLE_ZONE_GENERAL;
	if (flags & (u8)MLX4_RESERVE_A0_QP) {
		if (bf_qp)
			uid = MLX4_QP_TABLE_ZONE_RAW_ETH;
		else
			uid = MLX4_QP_TABLE_ZONE_RSS;
	}

	*base = mlx4_zone_alloc_entries(qp_table->zones, uid, cnt, align,
					bf_qp ? MLX4_BF_QP_SKIP_MASK : 0, NULL);
	if (*base == -1)
		return -ENOMEM;

	return 0;
}
