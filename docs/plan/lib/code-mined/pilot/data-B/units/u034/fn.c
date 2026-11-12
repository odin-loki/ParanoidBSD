static int wpas_p2p_select_go_freq(struct wpa_supplicant *wpa_s, int freq)
{
	unsigned int r;

	if (!wpa_s->conf->num_p2p_pref_chan && !freq) {
		unsigned int i, size = P2P_MAX_PREF_CHANNELS;
		struct weighted_pcl pref_freq_list[P2P_MAX_PREF_CHANNELS];
		int res;

		res = wpa_drv_get_pref_freq_list(wpa_s, WPA_IF_P2P_GO,
						 &size, pref_freq_list);
		if (!res && size > 0 && !is_p2p_allow_6ghz(wpa_s->global->p2p))
			size = p2p_remove_6ghz_channels(pref_freq_list, size);

		if (!res && size > 0) {
			i = 0;
			while (i < size &&
			       (!p2p_supported_freq(wpa_s->global->p2p,
						    pref_freq_list[i].freq) ||
				wpas_p2p_disallowed_freq(
					wpa_s->global,
					pref_freq_list[i].freq) ||
				!p2p_pref_freq_allowed(&pref_freq_list[i],
						       true))) {
				wpa_printf(MSG_DEBUG,
					   "P2P: preferred_freq_list[%d]=%d is disallowed",
					   i, pref_freq_list[i].freq);
				i++;
			}
			if (i != size) {
				freq = pref_freq_list[i].freq;
				wpa_printf(MSG_DEBUG,
					   "P2P: Using preferred_freq_list[%d]=%d",
					   i, freq);
			} else {
				wpa_printf(MSG_DEBUG,
					   "P2P: All driver preferred frequencies are disallowed for P2P use");
			}
		} else {
			wpa_printf(MSG_DEBUG,
				   "P2P: No preferred frequency list available");
		}
	}

	if (freq == 2) {
		wpa_printf(MSG_DEBUG, "P2P: Request to start GO on 2.4 GHz "
			   "band");
		if (wpa_s->best_24_freq > 0 &&
		    p2p_supported_freq_go(wpa_s->global->p2p,
					  wpa_s->best_24_freq)) {
			freq = wpa_s->best_24_freq;
			wpa_printf(MSG_DEBUG, "P2P: Use best 2.4 GHz band "
				   "channel: %d MHz", freq);
		} else {
			if (os_get_random((u8 *) &r, sizeof(r)) < 0)
				return -1;
			freq = 2412 + (r % 3) * 25;
			wpa_printf(MSG_DEBUG, "P2P: Use random 2.4 GHz band "
				   "channel: %d MHz", freq);
		}
	}

	if (freq == 5) {
		wpa_printf(MSG_DEBUG, "P2P: Request to start GO on 5 GHz "
			   "band");
		if (wpa_s->best_5_freq > 0 &&
		    p2p_supported_freq_go(wpa_s->global->p2p,
				       wpa_s->best_5_freq)) {
			freq = wpa_s->best_5_freq;
			wpa_printf(MSG_DEBUG, "P2P: Use best 5 GHz band "
				   "channel: %d MHz", freq);
		} else {
			const int freqs[] = {
				/* operating class 115 */
				5180, 5200, 5220, 5240,
				/* operating class 124 */
				5745, 5765, 5785, 5805,
			};
			unsigned int i, num_freqs = ARRAY_SIZE(freqs);

			if (os_get_random((u8 *) &r, sizeof(r)) < 0)
				return -1;

			/*
			 * Most of the 5 GHz channels require DFS. Only
			 * operating classes 115 and 124 are available possibly
			 * without that requirement. Check these for
			 * availability starting from a randomly picked
			 * position.
			 */
			for (i = 0; i < num_freqs; i++, r++) {
				freq = freqs[r % num_freqs];
				if (p2p_supported_freq_go(wpa_s->global->p2p,
							  freq))
					break;
			}

			if (i >= num_freqs) {
				wpa_printf(MSG_DEBUG, "P2P: Could not select "
					   "5 GHz channel for P2P group");
				return -1;
			}
			wpa_printf(MSG_DEBUG, "P2P: Use random 5 GHz band "
				   "channel: %d MHz", freq);
		}
	}

	if (freq > 0 && !p2p_supported_freq_go(wpa_s->global->p2p, freq)) {
		if ((wpa_s->drv_flags & WPA_DRIVER_FLAGS_DFS_OFFLOAD) &&
		    ieee80211_is_dfs(freq, wpa_s->hw.modes,
				     wpa_s->hw.num_modes)) {
			/*
			 * If freq is a DFS channel and DFS is offloaded to the
			 * driver, allow P2P GO to use it.
			 */
			wpa_printf(MSG_DEBUG, "P2P: "
				   "%s: The forced channel for GO (%u MHz) is DFS, and DFS is offloaded",
				   __func__, freq);
			return freq;
		}
		wpa_printf(MSG_DEBUG, "P2P: The forced channel for GO "
			   "(%u MHz) is not supported for P2P uses",
			   freq);
		return -1;
	}

	return freq;
}
