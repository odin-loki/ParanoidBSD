static int mm_put_stat(struct sk_buff *skb, u64 val, u16 attrtype)
{
	if (val == ETHTOOL_STAT_NOT_SET)
		return 0;
	if (nla_put_u64_64bit(skb, attrtype, val, ETHTOOL_A_MM_STAT_PAD))
		return -EMSGSIZE;
	return 0;
}
