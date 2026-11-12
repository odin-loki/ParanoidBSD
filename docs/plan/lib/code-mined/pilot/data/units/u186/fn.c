static int
irdma_ieq_create_pbufl(struct irdma_pfpdu *pfpdu,
		       struct list_head *rxlist,
		       struct list_head *pbufl,
		       struct irdma_puda_buf *buf, u16 fpdu_len)
{
	int status = 0;
	struct irdma_puda_buf *nextbuf;
	u32 nextseqnum;
	u16 plen = fpdu_len - buf->datalen;
	bool done = false;

	nextseqnum = buf->seqnum + buf->datalen;
	do {
		nextbuf = irdma_puda_get_listbuf(rxlist);
		if (!nextbuf) {
			status = -ENOBUFS;
			break;
		}
		list_add_tail(&nextbuf->list, pbufl);
		if (nextbuf->seqnum != nextseqnum) {
			pfpdu->bad_seq_num++;
			status = -ERANGE;
			break;
		}
		if (nextbuf->datalen >= plen) {
			done = true;
		} else {
			plen -= nextbuf->datalen;
			nextseqnum = nextbuf->seqnum + nextbuf->datalen;
		}

	} while (!done);

	return status;
}
