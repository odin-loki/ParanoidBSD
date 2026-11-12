void
SetKillRing(int max)
{
    CStr *new;
    int count, i, j;

    if (max < 1)
	max = 1;		/* no ring, but always one buffer */
    if (max == KillRingMax)
	return;
    new = xcalloc(max, sizeof(CStr));
    if (KillRing != NULL) {
	if (KillRingLen != 0) {
	    if (max >= KillRingLen) {
		count = KillRingLen;
		j = KillPos;
	    } else {
		count = max;
		j = (KillPos - count + KillRingLen) % KillRingLen;
	    }
	    for (i = 0; i < KillRingLen; i++) {
		if (i < count)	/* copy latest */
		    new[i] = KillRing[j];
		else		/* free the others */
		    xfree(KillRing[j].buf);
		j = (j + 1) % KillRingLen;
	    }
	    KillRingLen = count;
	    KillPos = count % max;
	    YankPos = count - 1;
	}
	xfree(KillRing);
    }
    KillRing = new;
    KillRingMax = max;
}
