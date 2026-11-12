static int mtip_quiesce_io(struct mtip_port *port, unsigned long timeout)
{
	unsigned long to;
	bool active = true;

	blk_mq_quiesce_queue(port->dd->queue);

	to = jiffies + msecs_to_jiffies(timeout);
	do {
		if (test_bit(MTIP_PF_SVC_THD_ACTIVE_BIT, &port->flags) &&
			test_bit(MTIP_PF_ISSUE_CMDS_BIT, &port->flags)) {
			msleep(20);
			continue; /* svc thd is actively issuing commands */
		}

		msleep(100);

		if (mtip_check_surprise_removal(port->dd))
			goto err_fault;

		active = mtip_commands_active(port);
		if (!active)
			break;
	} while (time_before(jiffies, to));

	blk_mq_unquiesce_queue(port->dd->queue);
	return active ? -EBUSY : 0;
err_fault:
	blk_mq_unquiesce_queue(port->dd->queue);
	return -EFAULT;
}
