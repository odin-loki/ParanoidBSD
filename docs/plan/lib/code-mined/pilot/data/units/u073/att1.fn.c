static bool check_attr_len(unsigned int attr_len, unsigned int expected_len)
{
	/*@ assert expected_len == attr_len || expected_len == OVS_ATTR_NESTED ||
	    expected_len == OVS_ATTR_VARIABLE ||
	    (expected_len != attr_len && expected_len != OVS_ATTR_NESTED &&
	     expected_len != OVS_ATTR_VARIABLE); */
	return expected_len == attr_len ||
	       expected_len == OVS_ATTR_NESTED ||
	       expected_len == OVS_ATTR_VARIABLE;
}
