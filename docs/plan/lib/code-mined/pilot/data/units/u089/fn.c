OM_uint32 GSSAPI_CALLCONV
_gss_ntlm_inquire_cred_by_mech (
            OM_uint32 * minor_status,
            const gss_cred_id_t cred_handle,
            const gss_OID mech_type,
            gss_name_t * name,
            OM_uint32 * initiator_lifetime,
            OM_uint32 * acceptor_lifetime,
            gss_cred_usage_t * cred_usage
    )
{
    if (minor_status)
	*minor_status = 0;
    if (name)
	*name = GSS_C_NO_NAME;
    if (initiator_lifetime)
	*initiator_lifetime = 0;
    if (acceptor_lifetime)
	*acceptor_lifetime = 0;
    if (cred_usage)
	*cred_usage = 0;
    return GSS_S_UNAVAILABLE;
}
