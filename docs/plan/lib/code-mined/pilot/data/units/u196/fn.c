static bool __is_raid10_far(int layout)
{
	return !__is_raid10_offset(layout) && __raid10_far_copies(layout) > 1;
}
