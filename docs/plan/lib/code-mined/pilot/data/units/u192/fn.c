static int rds_still_queued(struct rds_sock *rs, struct rds_incoming *inc,
			    int drop)
{
	struct sock *sk = rds_rs_to_sk(rs);
	int ret = 0;
	unsigned long flags;
	struct rds_incoming *to_drop = NULL;

	write_lock_irqsave(&rs->rs_recv_lock, flags);
	if (!list_empty(&inc->i_item)) {
		ret = 1;
		if (drop) {
			/* XXX make sure this i_conn is reliable */
			rds_recv_rcvbuf_delta(rs, sk, inc->i_conn->c_lcong,
					      -be32_to_cpu(inc->i_hdr.h_len),
					      inc->i_hdr.h_dport);
			list_del_init(&inc->i_item);
			to_drop = inc;
		}
	}
	write_unlock_irqrestore(&rs->rs_recv_lock, flags);

	if (to_drop)
		rds_inc_put(to_drop);

	rdsdebug("inc %p rs %p still %d dropped %d\n", inc, rs, ret, drop);
	return ret;
}
