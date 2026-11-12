int
cfumass_shutdown(void)
{
	int error;

	if (cfumass_refcount > 0) {
		if (debug > 1) {
			printf("%s: still have %u attachments; "
			    "returning EBUSY\n", __func__, cfumass_refcount);
		}
		return (EBUSY);
	}

	error = ctl_port_deregister(&cfumass_port);
	if (error != 0) {
		printf("%s: ctl_port_deregister() failed "
		    "with error %d\n", __func__, error);
	}

	return (error);
}
