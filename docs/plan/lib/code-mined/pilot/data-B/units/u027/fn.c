static int snd_vortex_new_pcm(vortex_t *chip, int idx, int nr)
{
	struct snd_pcm *pcm;
	struct snd_kcontrol *kctl;
	int i;
	int err, nr_capt;

	if (!chip || idx < 0 || idx >= VORTEX_PCM_LAST)
		return -ENODEV;

	/* idx indicates which kind of PCM device. ADB, SPDIF, I2S and A3D share the 
	 * same dma engine. WT uses it own separate dma engine which can't capture. */
	if (idx == VORTEX_PCM_ADB)
		nr_capt = nr;
	else
		nr_capt = 0;
	err = snd_pcm_new(chip->card, vortex_pcm_prettyname[idx], idx, nr,
			  nr_capt, &pcm);
	if (err < 0)
		return err;
	snprintf(pcm->name, sizeof(pcm->name),
		"%s %s", CARD_NAME_SHORT, vortex_pcm_name[idx]);
	chip->pcm[idx] = pcm;
	// This is an evil hack, but it saves a lot of duplicated code.
	VORTEX_PCM_TYPE(pcm) = idx;
	pcm->private_data = chip;
	/* set operators */
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK,
			&snd_vortex_playback_ops);
	if (idx == VORTEX_PCM_ADB)
		snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE,
				&snd_vortex_playback_ops);
	
	/* pre-allocation of Scatter-Gather buffers */
	
	snd_pcm_set_managed_buffer_all(pcm, SNDRV_DMA_TYPE_DEV_SG,
				       &chip->pci_dev->dev, 0x10000, 0x10000);

	switch (VORTEX_PCM_TYPE(pcm)) {
	case VORTEX_PCM_ADB:
		err = snd_pcm_add_chmap_ctls(pcm, SNDRV_PCM_STREAM_PLAYBACK,
					     snd_pcm_std_chmaps,
					     VORTEX_IS_QUAD(chip) ? 4 : 2,
					     0, NULL);
		if (err < 0)
			return err;
		err = snd_pcm_add_chmap_ctls(pcm, SNDRV_PCM_STREAM_CAPTURE,
					     snd_pcm_std_chmaps, 2, 0, NULL);
		if (err < 0)
			return err;
		break;
#ifdef CHIP_AU8830
	case VORTEX_PCM_A3D:
		err = snd_pcm_add_chmap_ctls(pcm, SNDRV_PCM_STREAM_PLAYBACK,
					     snd_pcm_std_chmaps, 1, 0, NULL);
		if (err < 0)
			return err;
		break;
#endif
	}

	if (VORTEX_PCM_TYPE(pcm) == VORTEX_PCM_SPDIF) {
		for (i = 0; i < ARRAY_SIZE(snd_vortex_mixer_spdif); i++) {
			kctl = snd_ctl_new1(&snd_vortex_mixer_spdif[i], chip);
			if (!kctl)
				return -ENOMEM;
			err = snd_ctl_add(chip->card, kctl);
			if (err < 0)
				return err;
		}
	}
	if (VORTEX_PCM_TYPE(pcm) == VORTEX_PCM_ADB) {
		for (i = 0; i < NR_PCM; i++) {
			chip->pcm_vol[i].active = 0;
			chip->pcm_vol[i].dma = -1;
			kctl = snd_ctl_new1(&snd_vortex_pcm_vol, chip);
			if (!kctl)
				return -ENOMEM;
			chip->pcm_vol[i].kctl = kctl;
			kctl->id.device = 0;
			kctl->id.subdevice = i;
			err = snd_ctl_add(chip->card, kctl);
			if (err < 0)
				return err;
		}
	}
	return 0;
}
