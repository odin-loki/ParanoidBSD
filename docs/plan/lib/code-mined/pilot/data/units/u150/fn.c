static uint32_t
wlan_peerstate_to_snmp(uint32_t pstate)
{
	uint32_t sstate = 0;

	if ((pstate & IEEE80211_NODE_AUTH) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_authorizedForData);
	if ((pstate & IEEE80211_NODE_QOS) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_qosEnabled);
	if ((pstate & IEEE80211_NODE_ERP) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_erpEnabled);
	if ((pstate & IEEE80211_NODE_PWR_MGT) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_powerSaveMode);
	if ((pstate & IEEE80211_NODE_AREF) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_authRefHeld);
	if ((pstate & IEEE80211_NODE_HT) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_htEnabled);
	if ((pstate & IEEE80211_NODE_HTCOMPAT) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_htCompat);
	if ((pstate & IEEE80211_NODE_WPS) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_wpsAssoc);
	if ((pstate & IEEE80211_NODE_TSN) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_tsnAssoc);
	if ((pstate & IEEE80211_NODE_AMPDU_RX) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_ampduRx);
	if ((pstate & IEEE80211_NODE_AMPDU_TX) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_ampduTx);
	if ((pstate & IEEE80211_NODE_MIMO_PS) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_mimoPowerSave);
	if ((pstate & IEEE80211_NODE_MIMO_RTS) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_sendRts);
	if ((pstate & IEEE80211_NODE_RIFS) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_rifs);
	if ((pstate & IEEE80211_NODE_SGI20) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_shortGiHT20);
	if ((pstate & IEEE80211_NODE_SGI40) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_shortGiHT40);
	if ((pstate & IEEE80211_NODE_AMSDU_RX) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_amsduRx);
	if ((pstate & IEEE80211_NODE_AMSDU_TX) != 0)
		sstate |= (0x1 << WlanIfacePeerFlagsType_amsduTx);

	return (sstate);
}
