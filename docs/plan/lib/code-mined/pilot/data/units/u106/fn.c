static int setup_iph(struct iphdr *iph, uint16_t payload_len)
{
	struct sockaddr_in *daddr = (void *) &cfg_dst_addr;
	struct sockaddr_in *saddr = (void *) &cfg_src_addr;

	memset(iph, 0, sizeof(*iph));

	iph->version	= 4;
	iph->tos	= 0;
	iph->ihl	= 5;
	iph->ttl	= 2;
	iph->saddr	= saddr->sin_addr.s_addr;
	iph->daddr	= daddr->sin_addr.s_addr;
	iph->protocol	= IPPROTO_EGP;
	iph->tot_len	= htons(sizeof(*iph) + payload_len);
	iph->check	= get_ip_csum((void *) iph, iph->ihl << 1);

	return sizeof(*iph);
}
