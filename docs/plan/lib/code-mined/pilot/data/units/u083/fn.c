static int md8800_set_high_voltage2(struct dvb_frontend *fe, long arg)
{
	pr_warn("%s: sorry can't set high LNB supply voltage from here\n",
		__func__);
	return -EIO;
}
