static int xfrm_xlate64_attr(struct sk_buff *dst, const struct nlattr *src)
{
	switch (src->nla_type) {
	case XFRMA_PAD:
		/* Ignore */
		return 0;
	case XFRMA_UNSPEC:
	case XFRMA_ALG_AUTH:
	case XFRMA_ALG_CRYPT:
	case XFRMA_ALG_COMP:
	case XFRMA_ENCAP:
	case XFRMA_TMPL:
		return xfrm_nla_cpy(dst, src, nla_len(src));
	case XFRMA_SA:
		return xfrm_nla_cpy(dst, src, XMSGSIZE(compat_xfrm_usersa_info));
	case XFRMA_POLICY:
		return xfrm_nla_cpy(dst, src, XMSGSIZE(compat_xfrm_userpolicy_info));
	case XFRMA_SEC_CTX:
		return xfrm_nla_cpy(dst, src, nla_len(src));
	case XFRMA_LTIME_VAL:
		return nla_put_64bit(dst, src->nla_type, nla_len(src),
			nla_data(src), XFRMA_PAD);
	case XFRMA_REPLAY_VAL:
	case XFRMA_REPLAY_THRESH:
	case XFRMA_ETIMER_THRESH:
	case XFRMA_SRCADDR:
	case XFRMA_COADDR:
		return xfrm_nla_cpy(dst, src, nla_len(src));
	case XFRMA_LASTUSED:
		return nla_put_64bit(dst, src->nla_type, nla_len(src),
			nla_data(src), XFRMA_PAD);
	case XFRMA_POLICY_TYPE:
	case XFRMA_MIGRATE:
	case XFRMA_ALG_AEAD:
	case XFRMA_KMADDRESS:
	case XFRMA_ALG_AUTH_TRUNC:
	case XFRMA_MARK:
	case XFRMA_TFCPAD:
	case XFRMA_REPLAY_ESN_VAL:
	case XFRMA_SA_EXTRA_FLAGS:
	case XFRMA_PROTO:
	case XFRMA_ADDRESS_FILTER:
	case XFRMA_OFFLOAD_DEV:
	case XFRMA_SET_MARK:
	case XFRMA_SET_MARK_MASK:
	case XFRMA_IF_ID:
	case XFRMA_MTIMER_THRESH:
	case XFRMA_SA_DIR:
	case XFRMA_NAT_KEEPALIVE_INTERVAL:
	case XFRMA_SA_PCPU:
	case XFRMA_IPTFS_DROP_TIME:
	case XFRMA_IPTFS_REORDER_WINDOW:
	case XFRMA_IPTFS_DONT_FRAG:
	case XFRMA_IPTFS_INIT_DELAY:
	case XFRMA_IPTFS_MAX_QSIZE:
	case XFRMA_IPTFS_PKT_SIZE:
		return xfrm_nla_cpy(dst, src, nla_len(src));
	default:
		BUILD_BUG_ON(XFRMA_MAX != XFRMA_IPTFS_PKT_SIZE);
		pr_warn_once("unsupported nla_type %d\n", src->nla_type);
		return -EOPNOTSUPP;
	}
}
