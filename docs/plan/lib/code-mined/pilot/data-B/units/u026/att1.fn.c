int
dlg_calc_listw(int item_no, char **items, int group)
{
    int i, len1 = 0, len2 = 0;

    /*@ loop invariant 0 <= i;
        loop invariant 0 <= len1;
        loop invariant 0 <= len2;
        loop assigns i, len1, len2;
        loop variant item_no * group - i; */
    for (i = 0; i < (item_no * group); i += group) {
	int n;

	if ((n = dlg_count_columns(items[i])) > len1)
	    len1 = n;
	if ((n = dlg_count_columns(items[i + 1])) > len2)
	    len2 = n;
    }
    return len1 + len2;
}
