/*-
 * Copyright (c) 2006-2016 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2004 Max Khon
 * Copyright (c) 2014 Juniper Networks, Inc.
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

/*
 * Oracle for PBSD batch b0144.
 *
 * The four source files of this batch are kernel GEOM uzip backends that
 * depend on <sys/mutex.h>, <sys/kthread.h>, <sys/bio.h>, kmem malloc,
 * contrib/zlib, contrib/xz-embedded, contrib/zstd, and <geom/uzip/g_uzip_dapi.h>
 * — none of which are part of this batch.  Reproducing their function bodies
 * here would require inventing those headers and library entry points rather
 * than concatenating faithful originals.  See skipped.txt.
 *
 * The typedef below exists only so this is not an empty translation unit,
 * which ISO C does not permit.
 */
typedef int pbsd_b0144_oracle_nonempty_translation_unit;
