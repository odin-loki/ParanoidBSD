static inline u32 host1x_sync_cbread_r(unsigned int channel)
{
	return 0x720 + channel * REGISTER_STRIDE;
}
