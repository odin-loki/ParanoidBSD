static int parse_mode(char *val, u32 *mode)
{
	*mode = 0;

	if (!strcmp(val, "idle"))
		*mode = IDLE_WRITEBACK;
	if (!strcmp(val, "huge"))
		*mode = HUGE_WRITEBACK;
	if (!strcmp(val, "huge_idle"))
		*mode = IDLE_WRITEBACK | HUGE_WRITEBACK;
	if (!strcmp(val, "incompressible"))
		*mode = INCOMPRESSIBLE_WRITEBACK;

	/*@ assert *mode == 0 || *mode == IDLE_WRITEBACK || *mode == HUGE_WRITEBACK
	    || *mode == (IDLE_WRITEBACK | HUGE_WRITEBACK)
	    || *mode == INCOMPRESSIBLE_WRITEBACK; */

	if (*mode == 0) {
		/*@ assert *mode == 0; */
		return -EINVAL;
	}

	/*@ assert *mode != 0; */
	return 0;
}
