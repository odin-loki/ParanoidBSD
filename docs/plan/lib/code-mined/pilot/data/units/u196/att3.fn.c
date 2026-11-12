static bool __is_raid10_far(int layout)
{
	/*@ assert __is_raid10_offset(layout) == (!(layout & (1 << 16))); */
	/*@ assert __raid10_far_copies(layout) == ((layout >> 8) & 0xFF); */
	return !__is_raid10_offset(layout) && __raid10_far_copies(layout) > 1;
}
