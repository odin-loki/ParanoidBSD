int t1_tp_reset(struct petp *tp, struct tp_params *p, unsigned int tp_clk)
{
	adapter_t *adapter = tp->adapter;

	tp_init(adapter, p, tp_clk);
	writel(F_TP_RESET, adapter->regs +  A_TP_RESET);
	return 0;
}
