KRB5_LIB_FUNCTION size_t KRB5_LIB_CALL
krb5_max_sockaddr_size (void)
{
    if (max_sockaddr_size == 0) {
	struct addr_operations *a;

	for(a = at; a < at + num_addrs; ++a)
	    max_sockaddr_size = max(max_sockaddr_size, a->max_sockaddr_size);
    }
    return max_sockaddr_size;
}
