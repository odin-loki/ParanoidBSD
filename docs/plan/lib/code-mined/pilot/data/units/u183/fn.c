static int first_slot_num(u8 slot_num, u8 first_slot, u8 var)
{
	struct opt_rio *opt_vg_ptr = NULL;
	struct opt_rio_lo *opt_lo_ptr = NULL;
	int rc = 0;

	if (!var) {
		list_for_each_entry(opt_vg_ptr, &opt_vg_head, opt_rio_list) {
			if ((first_slot < opt_vg_ptr->first_slot_num) && (slot_num >= opt_vg_ptr->first_slot_num)) {
				rc = -ENODEV;
				break;
			}
		}
	} else {
		list_for_each_entry(opt_lo_ptr, &opt_lo_head, opt_rio_lo_list) {
			if ((first_slot < opt_lo_ptr->first_slot_num) && (slot_num >= opt_lo_ptr->first_slot_num)) {
				rc = -ENODEV;
				break;
			}
		}
	}
	return rc;
}
