static inline u8 qm_eqcr_get_avail(struct qm_portal *portal)
{
	struct qm_eqcr *eqcr = &portal->eqcr;

	return eqcr->available;
}
