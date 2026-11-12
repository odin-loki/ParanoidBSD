int
aes_mod_fini(void)
{
	/* Unregister from KCF if module is registered */
	if (aes_prov_handle != 0) {
		if (crypto_unregister_provider(aes_prov_handle))
			return (EBUSY);

		aes_prov_handle = 0;
	}

	return (0);
}
