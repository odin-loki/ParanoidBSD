static int
nge_list_rx_init(struct nge_softc *sc)
{
	struct nge_ring_data *rd;
	bus_addr_t addr;
	int i;

	sc->nge_cdata.nge_rx_cons = 0;
	sc->nge_head = sc->nge_tail = NULL;

	rd = &sc->nge_rdata;
	bzero(rd->nge_rx_ring, sizeof(struct nge_desc) * NGE_RX_RING_CNT);
	for (i = 0; i < NGE_RX_RING_CNT; i++) {
		if (nge_newbuf(sc, i) != 0)
			return (ENOBUFS);
		if (i == NGE_RX_RING_CNT - 1)
			addr = NGE_RX_RING_ADDR(sc, 0);
		else
			addr = NGE_RX_RING_ADDR(sc, i + 1);
		rd->nge_rx_ring[i].nge_next = htole32(NGE_ADDR_LO(addr));
	}

	bus_dmamap_sync(sc->nge_cdata.nge_rx_ring_tag,
	    sc->nge_cdata.nge_rx_ring_map,
	    BUS_DMASYNC_PREREAD | BUS_DMASYNC_PREWRITE);

	return (0);
}
