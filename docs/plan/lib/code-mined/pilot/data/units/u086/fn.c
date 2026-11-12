static OM_uint32
val_acq_cred_impersonate_name_args(
    OM_uint32 *minor_status,
    const gss_cred_id_t impersonator_cred_handle,
    const gss_name_t desired_name,
    OM_uint32 time_req,
    gss_OID_set desired_mechs,
    gss_cred_usage_t cred_usage,
    gss_cred_id_t *output_cred_handle,
    gss_OID_set *actual_mechs,
    OM_uint32 *time_rec)
{

    /* Initialize outputs. */

    if (minor_status != NULL)
	*minor_status = 0;

    if (output_cred_handle != NULL)
	*output_cred_handle = GSS_C_NO_CREDENTIAL;

    if (actual_mechs != NULL)
	*actual_mechs = GSS_C_NULL_OID_SET;

    if (time_rec != NULL)
	*time_rec = 0;

    /* Validate arguments. */

    if (minor_status == NULL)
	return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (impersonator_cred_handle == GSS_C_NO_CREDENTIAL)
	return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_NO_CRED);

    if (desired_name == GSS_C_NO_NAME)
	return (GSS_S_CALL_INACCESSIBLE_READ | GSS_S_BAD_NAME);

    if (output_cred_handle == NULL)
	return (GSS_S_CALL_INACCESSIBLE_WRITE);

    if (cred_usage != GSS_C_ACCEPT
	&& cred_usage != GSS_C_INITIATE
	&& cred_usage != GSS_C_BOTH) {
	if (minor_status) {
	    *minor_status = EINVAL;
	    map_errcode(minor_status);
	}
	return GSS_S_FAILURE;
    }

    return (GSS_S_COMPLETE);
}
