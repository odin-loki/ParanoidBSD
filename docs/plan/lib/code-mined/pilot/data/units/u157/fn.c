bool
pvr_kccb_is_idle(struct pvr_device *pvr_dev)
{
	struct rogue_fwif_ccb_ctl *ctrl = pvr_dev->kccb.ccb.ctrl;
	bool idle;

	mutex_lock(&pvr_dev->kccb.ccb.lock);

	idle = (READ_ONCE(ctrl->write_offset) == READ_ONCE(ctrl->read_offset));

	mutex_unlock(&pvr_dev->kccb.ccb.lock);

	return idle;
}
