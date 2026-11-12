int
smb_ntencrypt(const u_char *apwd, u_char *C8, u_char *RN)
{
	u_char S21[21];
	u_int16_t *unipwd;
	MD4_CTX *ctxp;
	u_int len;

	len = strlen(apwd);
	unipwd = malloc((len + 1) * sizeof(u_int16_t), M_SMBTEMP, M_WAITOK);
	/*
	 * S21 = concat(MD4(U(apwd)), zeros(5));
	 */
	smb_strtouni(unipwd, apwd);
	ctxp = malloc(sizeof(MD4_CTX), M_SMBTEMP, M_WAITOK);
	MD4Init(ctxp);
	MD4Update(ctxp, (u_char*)unipwd, len * sizeof(u_int16_t));
	free(unipwd, M_SMBTEMP);
	bzero(S21, 21);
	MD4Final(S21, ctxp);
	free(ctxp, M_SMBTEMP);

	smb_E(S21, C8, RN);
	smb_E(S21 + 7, C8, RN + 8);
	smb_E(S21 + 14, C8, RN + 16);
	return 0;
}
