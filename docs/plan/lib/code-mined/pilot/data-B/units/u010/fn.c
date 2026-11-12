static u_int
close_node(u_int n, u_int level)
{
	while (n--) {
		pindent(level);
		level--;
		if (level >= cut)
			printf(")\n");
	}
	return (level);
}
