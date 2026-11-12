static inline u32 gve_get_rx_pages_per_qpl_dqo(u32 rx_desc_cnt)
{
	/* For DQO, page count should be more than ring size for
	 * out-of-order completions. Set it to two times of ring size.
	 */
	return 2 * rx_desc_cnt;
}
