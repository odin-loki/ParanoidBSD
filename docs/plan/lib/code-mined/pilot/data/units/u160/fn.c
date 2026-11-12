static int ish_send(struct ishtp_cl_data *client_data,
		    u8 *out_msg, size_t out_size,
		    u8 *in_msg, size_t in_size)
{
	static u8 next_token;
	int rv;
	struct header *out_hdr = (struct header *)out_msg;
	struct ishtp_cl *cros_ish_cl = client_data->cros_ish_cl;

	dev_dbg(cl_data_to_dev(client_data),
		"%s: channel=%02u status=%02u\n",
		__func__, out_hdr->channel, out_hdr->status);

	/* Setup for incoming response */
	client_data->response.data = in_msg;
	client_data->response.max_size = in_size;
	client_data->response.error = 0;
	client_data->response.token = next_token++;
	client_data->response.received = false;

	out_hdr->token = client_data->response.token;

	rv = ishtp_cl_send(cros_ish_cl, out_msg, out_size);
	if (rv) {
		dev_err(cl_data_to_dev(client_data),
			"ishtp_cl_send error %d\n", rv);
		return rv;
	}

	wait_event_interruptible_timeout(client_data->response.wait_queue,
					 client_data->response.received,
					 ISHTP_SEND_TIMEOUT);
	if (!client_data->response.received) {
		dev_err(cl_data_to_dev(client_data),
			"Timed out for response to host message\n");
		return -ETIMEDOUT;
	}

	if (client_data->response.error < 0)
		return client_data->response.error;

	return client_data->response.size;
}
