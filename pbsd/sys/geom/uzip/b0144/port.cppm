/*-
 * Copyright (c) 2004 Max Khon
 * Copyright (c) 2014 Juniper Networks, Inc.
 * Copyright (c) 2006-2016 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2010-2012 Aleksandr Rybalko
 * Copyright (c) 2019 Conrad Meyer <cem@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

export module pbsd.sys.geom.uzip.b0144;

/*
 * Batch b0144 covers four GEOM uzip kernel backend sources
 * (g_uzip_wrkthr.c, g_uzip_zlib.c, g_uzip_lzma.c, g_uzip_zstd.c).  Each
 * depends on kernel threading, mutexes, bio queues, kmem malloc, and/or
 * external compression libraries wired through g_uzip_dapi — none of which
 * are available in this isolated differential-test batch.  Every file is
 * recorded in skipped.txt rather than stubbed.  Nothing was faithfully
 * portable, so this namespace exports nothing.
 */
export namespace pbsd::sys_geom_uzip::b0144 {
}
