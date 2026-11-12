int amdtee_close_session(struct tee_context *ctx, u32 session)
{
	struct amdtee_context_data *ctxdata = ctx->data;
	u32 i, ta_handle, session_info;
	struct amdtee_session *sess;

	pr_debug("%s: sid = 0x%x\n", __func__, session);

	/*
	 * Check that the session is valid and clear the session
	 * usage bit
	 */
	mutex_lock(&session_list_mutex);
	sess = find_session(ctxdata, session);
	if (sess) {
		ta_handle = get_ta_handle(session);
		i = get_session_index(session);
		session_info = sess->session_info[i];
		spin_lock(&sess->lock);
		clear_bit(i, sess->sess_mask);
		spin_unlock(&sess->lock);
	}
	mutex_unlock(&session_list_mutex);

	if (!sess)
		return -EINVAL;

	/* Close the session */
	handle_close_session(ta_handle, session_info);
	handle_unload_ta(ta_handle);

	kref_put_mutex(&sess->refcount, destroy_session, &session_list_mutex);

	return 0;
}
