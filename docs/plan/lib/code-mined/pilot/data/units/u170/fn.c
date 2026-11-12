static inline u16 qed_attn_update_idx(struct qed_hwfn *p_hwfn,
				      struct qed_sb_attn_info *p_sb_desc)
{
	u16 rc = 0, index;

	index = le16_to_cpu(p_sb_desc->sb_attn->sb_index);
	if (p_sb_desc->index != index) {
		p_sb_desc->index	= index;
		rc		      = QED_SB_ATT_IDX;
	}

	return rc;
}
