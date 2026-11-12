static int _qed_vf_pf_release(struct qed_hwfn *p_hwfn, bool b_final)
{
	struct qed_vf_iov *p_iov = p_hwfn->vf_iov_info;
	struct pfvf_def_resp_tlv *resp;
	struct vfpf_first_tlv *req;
	u32 size;
	int rc;

	/* clear mailbox and prep first tlv */
	req = qed_vf_pf_prep(p_hwfn, CHANNEL_TLV_RELEASE, sizeof(*req));

	/* add list termination tlv */
	qed_add_tlv(p_hwfn, &p_iov->offset,
		    CHANNEL_TLV_LIST_END, sizeof(struct channel_list_end_tlv));

	resp = &p_iov->pf2vf_reply->default_resp;
	rc = qed_send_msg2pf(p_hwfn, &resp->hdr.status);

	if (!rc && resp->hdr.status != PFVF_STATUS_SUCCESS)
		rc = -EAGAIN;

	qed_vf_pf_req_end(p_hwfn, rc);
	if (!b_final)
		return rc;

	p_hwfn->b_int_enabled = 0;

	if (p_iov->vf2pf_request)
		dma_free_coherent(&p_hwfn->cdev->pdev->dev,
				  sizeof(union vfpf_tlvs),
				  p_iov->vf2pf_request,
				  p_iov->vf2pf_request_phys);
	if (p_iov->pf2vf_reply)
		dma_free_coherent(&p_hwfn->cdev->pdev->dev,
				  sizeof(union pfvf_tlvs),
				  p_iov->pf2vf_reply, p_iov->pf2vf_reply_phys);

	if (p_iov->bulletin.p_virt) {
		size = sizeof(struct qed_bulletin_content);
		dma_free_coherent(&p_hwfn->cdev->pdev->dev,
				  size,
				  p_iov->bulletin.p_virt, p_iov->bulletin.phys);
	}

	kfree(p_hwfn->vf_iov_info);
	p_hwfn->vf_iov_info = NULL;

	return rc;
}
