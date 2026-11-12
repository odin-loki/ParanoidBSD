int ocelot_vcap_policer_add(struct ocelot *ocelot, u32 pol_ix,
			    struct ocelot_policer *pol)
{
	struct qos_policer_conf pp = { 0 };
	struct vcap_policer_entry *tmp;
	int ret;

	if (!pol)
		return -EINVAL;

	pp.mode = MSCC_QOS_RATE_MODE_DATA;
	pp.pir = pol->rate;
	pp.pbs = pol->burst;

	list_for_each_entry(tmp, &ocelot->vcap_pol.pol_list, list)
		if (tmp->pol_ix == pol_ix) {
			refcount_inc(&tmp->refcount);
			return 0;
		}

	tmp = kzalloc(sizeof(*tmp), GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;

	ret = qos_policer_conf_set(ocelot, pol_ix, &pp);
	if (ret) {
		kfree(tmp);
		return ret;
	}

	tmp->pol_ix = pol_ix;
	refcount_set(&tmp->refcount, 1);
	list_add_tail(&tmp->list, &ocelot->vcap_pol.pol_list);

	return 0;
}
