krb5_error_code
k5_populate_gic_opt(krb5_context context, krb5_get_init_creds_opt **out,
                    krb5_flags options, krb5_address *const *addrs,
                    krb5_enctype *ktypes, krb5_preauthtype *pre_auth_types,
                    krb5_creds *creds)
{
    int i;
    krb5_timestamp starttime;
    krb5_deltat lifetime;
    krb5_get_init_creds_opt *opt;
    krb5_error_code retval;

    *out = NULL;
    retval = krb5_get_init_creds_opt_alloc(context, &opt);
    if (retval)
        return(retval);

    if (addrs)
        krb5_get_init_creds_opt_set_address_list(opt, (krb5_address **) addrs);
    if (ktypes) {
        i = k5_count_etypes(ktypes);
        if (i)
            krb5_get_init_creds_opt_set_etype_list(opt, ktypes, i);
    }
    if (pre_auth_types) {
        for (i=0; pre_auth_types[i]; i++);
        if (i)
            krb5_get_init_creds_opt_set_preauth_list(opt, pre_auth_types, i);
    }
    if (options&KDC_OPT_FORWARDABLE)
        krb5_get_init_creds_opt_set_forwardable(opt, 1);
    else krb5_get_init_creds_opt_set_forwardable(opt, 0);
    if (options&KDC_OPT_PROXIABLE)
        krb5_get_init_creds_opt_set_proxiable(opt, 1);
    else krb5_get_init_creds_opt_set_proxiable(opt, 0);
    if (creds && creds->times.endtime) {
        retval = krb5_timeofday(context, &starttime);
        if (retval)
            goto cleanup;
        if (creds->times.starttime) starttime = creds->times.starttime;
        lifetime = ts_delta(creds->times.endtime, starttime);
        krb5_get_init_creds_opt_set_tkt_life(opt, lifetime);
    }
    *out = opt;
    return 0;

cleanup:
    krb5_get_init_creds_opt_free(context, opt);
    return retval;
}
