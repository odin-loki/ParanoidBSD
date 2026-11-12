static int
emu_dspmixer_init(struct snd_mixer *m)
{
	struct emu_pcm_info	*sc;
	int i;
	int p, r;

	p = 0;
	r = 0;

	sc = mix_getdevinfo(m);

	if (sc->route == RT_FRONT) {
		/* create submixer for AC97 codec */
		if ((sc->ac97_mixerclass != NULL) && (sc->codec != NULL)) {
			sc->sm = mixer_create(sc->dev, sc->ac97_mixerclass, sc->codec, "ac97");
			if (sc->sm != NULL) {
				p = mix_getdevs(sc->sm);
				r = mix_getrecdevs(sc->sm);
			}
		}

		sc->ac97_playdevs = p;
		sc->ac97_recdevs = r;
	}

	/* This two are always here */
	p |= (1 << SOUND_MIXER_PCM);
	p |= (1 << SOUND_MIXER_VOLUME);

	if (sc->route == RT_FRONT) {
		if (sc->is_emu10k1) {
			for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
				if (dspmix_k1[i].present)
					p |= (1 << i);
				if (dspmix_k1[i].recdev)
					r |= (1 << i);
			}
		} else {
			for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
				if (dspmix_k2[i].present)
					p |= (1 << i);
				if (dspmix_k2[i].recdev)
					r |= (1 << i);
			}
		}
	}

	mix_setdevs(m, p);
	mix_setrecdevs(m, r);

	return (0);
}
