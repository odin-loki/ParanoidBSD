static bool check_attr_len(unsigned int attr_len, unsigned int expected_len)
{
	return expected_len == attr_len ||
	       expected_len == OVS_ATTR_NESTED ||
	       expected_len == OVS_ATTR_VARIABLE;
}
