static boolean_t
split_kv(char *line, char **kp, char **vp)
{
	char *c = strstr(line, ":");
	if (c == NULL)
		return (B_FALSE);


	*c++ = '\0';
	while (*c == ' ')
		c++;

	char *v = c;
	c = strchr(v, '\n');
	if (c != NULL) {
		*c++ = '\0';
		if (*c != '\0')
			return (B_FALSE);
	}

	*kp = line;
	*vp = v;
	return (B_TRUE);
}
