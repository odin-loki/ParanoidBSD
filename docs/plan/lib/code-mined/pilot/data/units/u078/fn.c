static int sk_psock_skb_ingress_self(struct sk_psock *psock, struct sk_buff *skb,
				     u32 off, u32 len, bool take_ref)
{
	struct sk_msg *msg = alloc_sk_msg(GFP_ATOMIC);
	struct sock *sk = psock->sk;
	int err;

	if (unlikely(!msg))
		return -EAGAIN;
	skb_set_owner_r(skb, sk);

	/* This is used in tcp_bpf_recvmsg_parser() to determine whether the
	 * data originates from the socket's own protocol stack. No need to
	 * refcount sk because msg's lifetime is bound to sk via the ingress_msg.
	 */
	msg->sk = sk;
	err = sk_psock_skb_ingress_enqueue(skb, off, len, psock, sk, msg, take_ref);
	if (err < 0)
		kfree(msg);
	return err;
}
