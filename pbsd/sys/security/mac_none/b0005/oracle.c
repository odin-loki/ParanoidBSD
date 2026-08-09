/*-
 * Copyright (c) 1999-2002, 2007 Robert N. M. Watson
 * Copyright (c) 2001-2003 Networks Associates Technology, Inc.
 * All rights reserved.
 *
 * This software was developed by Robert Watson for the TrustedBSD Project.
 *
 * This software was developed for the FreeBSD Project in part by Network
 * Associates Laboratories, the Security Research Division of Network
 * Associates, Inc. under DARPA/SPAWAR contract N66001-01-C-8035 ("CBOSS"),
 * as part of the DARPA CHATS research program.
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
 * Developed by the TrustedBSD Project.
 *
 * Sample policy implementing no entry points; for performance measurement
 * purposes only.  If you're looking for a stub policy to base new policies
 * on, try mac_stub.
 */

/*
 * Oracle for PBSD batch b0005.
 *
 * The batch's only source file, sys/security/mac_none/mac_none.c, defines no
 * functions, so there is no function body to reproduce under a ref_ prefix.
 * Its two statements -- `static struct mac_policy_ops none_ops = {};' and
 * `MAC_POLICY_SET(&none_ops, mac_none, "TrustedBSD MAC/None",
 * MPC_LOADTIME_FLAG_UNLOADOK, NULL);' -- both need the struct layout and macro
 * expansion from <security/mac/mac_policy.h>, which is not part of this batch;
 * reproducing them here would mean inventing that header rather than
 * concatenating an original source.  See skipped.txt.
 *
 * The typedef below exists only so this is not an empty translation unit,
 * which ISO C does not permit.
 */
typedef int pbsd_b0005_oracle_nonempty_translation_unit;
