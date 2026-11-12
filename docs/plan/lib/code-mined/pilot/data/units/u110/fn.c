static ssize_t
allegro_dec_create_channel(struct mcu_msg_create_channel_response *msg,
			   u32 *src)
{
	enum mcu_msg_version version = msg->header.version;
	unsigned int i = 0;

	msg->channel_id = src[i++];
	msg->user_id = src[i++];
	/*
	 * Version >= MCU_MSG_VERSION_2019_2 is handled in
	 * allegro_decode_config_blob().
	 */
	if (version < MCU_MSG_VERSION_2019_2) {
		msg->options = src[i++];
		msg->num_core = src[i++];
		msg->num_ref_idx_l0 = FIELD_GET(GENMASK(7, 4), src[i]);
		msg->num_ref_idx_l1 = FIELD_GET(GENMASK(11, 8), src[i++]);
	}
	msg->int_buffers_count = src[i++];
	msg->int_buffers_size = src[i++];
	msg->rec_buffers_count = src[i++];
	msg->rec_buffers_size = src[i++];
	msg->reserved = src[i++];
	msg->error_code = src[i++];

	return i * sizeof(*src);
}
