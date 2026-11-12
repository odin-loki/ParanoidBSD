CCRETVAL
e_last_item(Char c)
{				/* insert the last element of the prev. cmd */
    struct Hist *hp;
    struct wordent *wp, *firstp;
    int i;
    Char *expanded;

    USE(c);
    if (Argument <= 0)
	return(CC_ERROR);

    hp = Histlist.Hnext;
    if (hp == NULL) {	/* this is only if no history */
	return(CC_ERROR);
    }

    wp = (hp->Hlex).prev;

    if (wp->prev == (struct wordent *) NULL)
	return(CC_ERROR);	/* an empty history entry */

    firstp = (hp->Hlex).next;

    /* back up arg words in lex */
    for (i = 0; i < Argument && wp != firstp; i++) {
	wp = wp->prev;
    }

    expanded = expand_lex(wp->prev, 0, i - 1);
    if (InsertStr(expanded)) {
	xfree(expanded);
	return(CC_ERROR);
    }

    xfree(expanded);
    return(CC_REFRESH);
}
