int tpg_alloc(struct tpg_data *tpg, unsigned max_w)
{
	unsigned pat;
	unsigned plane;
	int ret = 0;

	tpg->max_line_width = max_w;
	for (pat = 0; pat < TPG_MAX_PAT_LINES; pat++) {
		for (plane = 0; plane < TPG_MAX_PLANES; plane++) {
			unsigned pixelsz = plane ? 2 : 4;

			tpg->lines[pat][plane] =
				vzalloc(array3_size(max_w, 2, pixelsz));
			if (!tpg->lines[pat][plane]) {
				ret = -ENOMEM;
				goto free_lines;
			}
			if (plane == 0)
				continue;
			tpg->downsampled_lines[pat][plane] =
				vzalloc(array3_size(max_w, 2, pixelsz));
			if (!tpg->downsampled_lines[pat][plane]) {
				ret = -ENOMEM;
				goto free_lines;
			}
		}
	}
	for (plane = 0; plane < TPG_MAX_PLANES; plane++) {
		unsigned pixelsz = plane ? 2 : 4;

		tpg->contrast_line[plane] =
			vzalloc(array_size(pixelsz, max_w));
		if (!tpg->contrast_line[plane]) {
			ret = -ENOMEM;
			goto free_contrast_line;
		}
		tpg->black_line[plane] =
			vzalloc(array_size(pixelsz, max_w));
		if (!tpg->black_line[plane]) {
			ret = -ENOMEM;
			goto free_contrast_line;
		}
		tpg->random_line[plane] =
			vzalloc(array3_size(max_w, 2, pixelsz));
		if (!tpg->random_line[plane]) {
			ret = -ENOMEM;
			goto free_contrast_line;
		}
	}
	return 0;

free_contrast_line:
	for (plane = 0; plane < TPG_MAX_PLANES; plane++) {
		vfree(tpg->contrast_line[plane]);
		vfree(tpg->black_line[plane]);
		vfree(tpg->random_line[plane]);
		tpg->contrast_line[plane] = NULL;
		tpg->black_line[plane] = NULL;
		tpg->random_line[plane] = NULL;
	}
free_lines:
	for (pat = 0; pat < TPG_MAX_PAT_LINES; pat++)
		for (plane = 0; plane < TPG_MAX_PLANES; plane++) {
			vfree(tpg->lines[pat][plane]);
			tpg->lines[pat][plane] = NULL;
			if (plane == 0)
				continue;
			vfree(tpg->downsampled_lines[pat][plane]);
			tpg->downsampled_lines[pat][plane] = NULL;
		}
	return ret;
}
