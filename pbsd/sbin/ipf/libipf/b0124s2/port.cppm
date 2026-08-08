module;

export module pbsd.sbin.ipf.libipf.b0124s2;

export namespace pbsd::sbin_ipf_libipf::b0124s2 {

/*
 * Copyright (C) 2012 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id$
 */

long	string_start = -1;
long	string_end = -1;
char	*string_val = NULL;
long	pos = 0;


void resetlexer(void)
{
	string_start = -1;
	string_end = -1;
	string_val = NULL;
	pos = 0;
}

} // namespace pbsd::sbin_ipf_libipf::b0124s2
