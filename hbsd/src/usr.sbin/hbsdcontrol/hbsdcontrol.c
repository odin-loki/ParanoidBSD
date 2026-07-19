/*-
 * Copyright (c) 2024 Shawn Webb <shawn.webb@hardenedbsd.org>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>

#include <sys/capsicum.h>

#include <libhbsdcontrol.h>

static int verbose = 0;
static uint64_t open_flags = O_PATH | O_CLOEXEC | O_NOFOLLOW;
static const char *prog;
static const char *admin_privs_required = "[-] Administrative privileges are required for the \"system\" namespace\n";

static void
usage(const char *feature_name)
{
	hbsdctrl_ctx_t *ctx;
	hbsdctrl_feature_t *feature, *tfeature;
	unsigned short int i;

	ctx = hbsdctrl_ctx_new(0, LIBHBSDCONTROL_DEFAULT_NAMESPACE);
	i = 0;
	if (ctx == NULL) {
		goto end;
	} else if (feature_name == NULL) {
		fprintf(stderr, "USAGE\n\n");
		fprintf(stderr, "    %s [-Hdnh] pax enable|disable|sysdef <feature> <file>\n", prog);
		fprintf(stderr, "    %s [-Hdnh] pax list <file>\n\n", prog);
		fprintf(stderr, "OPTIONS\n\n");
		fprintf(stderr, "    -H: follow symlinks\n");
		fprintf(stderr, "    -d: enable verbose output\n");
		fprintf(stderr, "    -n namespace: set extended attribute namespace\n");
		fprintf(stderr, "    -h [feature]: show general help or feature-specific help\n\n");
		fprintf(stderr, "FEATURES\n");
		LIST_FOREACH_SAFE(feature, &(ctx->hc_features), hf_entry, tfeature) {
			if (i % 3 == 0) {
				fprintf(stderr, "\n");
				fprintf(stderr, "    ");
			}
			fprintf(stderr, "%-20s\t", feature->hf_name);
			i++;
		}
		fprintf(stderr, "\n");
		goto end;
	} else {
		LIST_FOREACH_SAFE(feature, &(ctx->hc_features), hf_entry, tfeature) {
			if (!strcmp(feature->hf_name, feature_name)) {
				hbsdctrl_feature_call_cb(feature, "help", "", stderr);
				goto end;
			}
		}
		fprintf(stderr, "[-] Unknown feature: %s\n", feature_name);
		goto end;
	}
end:
	hbsdctrl_ctx_free(&ctx);
	exit(0);
}

static int
open_file(const char *path)
{
	cap_rights_t rights;
	int fd;

	fd = open(path, open_flags);
	if (fd == -1) {
		return (-1);
	}

	memset(&rights, 0, sizeof(rights));
	cap_rights_init(&rights, CAP_EXTATTR_DELETE, CAP_EXTATTR_GET,
	    CAP_EXTATTR_LIST, CAP_EXTATTR_SET, CAP_FSTAT, CAP_LOOKUP);
	cap_rights_limit(fd, &rights);

	return (fd);
}

static int
do_list(hbsdctrl_ctx_t *ctx, const char *path)
{
	hbsdctrl_file_states_t *fstate, *tfstate;
	hbsdctrl_file_states_head_t *fstates;
	hbsdctrl_feature_state_t *state;
	hbsdctrl_feature_t *feature;
	int fd, res;

	if (path == NULL) {
		usage(false);
	}

	res = 0;
	fd = open_file(path);

#if 0
	cap_enter();
#endif

	fstates = hbsdctrl_get_file_states(ctx, fd);
	if (fstates == NULL) {
		fprintf(stderr, "[-] Unable to get feature states.\n");
		res = 1;
		goto end;
	}

	LIST_FOREACH_SAFE(fstate, &(fstates->hfsh_states), hfs_entry,
	    tfstate) {
		feature = hbsdctrl_file_states_get_feature(fstate);
		if (feature == NULL) {
			fprintf(stderr, "[-] feature is NULL. This shouldn't happen.\n");
			res = 1;
			goto end;
		}
		state = hbsdctrl_file_states_get_feature_state(fstate);
		if (state == NULL) {
			fprintf(stderr, "[-] state is NULL. This shouldn't happen.\n");
			res = 1;
			goto end;
		}
		printf("%s:\t%s\n",
		    hbsdctrl_feature_get_name(feature),
		    hbsdctrl_feature_state_to_string(state));
	}

end:
	hbsdctrl_free_file_states(&fstates);
	close(fd);
	return (res);
}

static int
set_state(hbsdctrl_ctx_t *ctx, const char *feature_name,
    hbsdctrl_feature_state_value_t state_value, const char *path)
{
	hbsdctrl_feature_state_t *state;
	hbsdctrl_feature_cb_res_t res;
	hbsdctrl_feature_t *feature;
	int fd, ret;

	if (feature_name == NULL || path == NULL) {
		usage(NULL);
	}

	if (!hbsdctrl_feature_state_value_valid(state_value)) {
		usage(NULL);
	}

	feature = hbsdctrl_ctx_find_feature_by_name(ctx, feature_name);
	if (feature == NULL) {
		usage(NULL);
	}

	ret = 0;
	fd = open_file(path);
	if (fd == -1) {
		perror("open");
		return (1);
	}

#if 0
	cap_enter();
#endif

	state = hbsdctrl_feature_state_new(fd, HBSDCTRL_FEATURE_STATE_FLAG_NONE);
	if (state == NULL) {
		fprintf(stderr, "[-] Could not create new feature state object\n");
		res = 1;
		goto end;
	}

	if (!hbsdctrl_feature_state_set_value(state, state_value)) {
		fprintf(stderr, "[-] Could not set state value\n");
		res = 1;
		goto end;
	}

	res = hbsdctrl_feature_call_cb(feature, "apply", &fd, state);
	switch (res) {
	case RES_SUCCESS:
		ret = 0;
		break;
	default:
		ret = 2;
	}

end:
	hbsdctrl_feature_state_free(&state);
	close(fd);
	return (ret);
}

static int
reset(hbsdctrl_ctx_t *ctx, const char *feature_name, const char *path)
{
	hbsdctrl_feature_cb_res_t res;
	hbsdctrl_feature_t *feature;
	int fd, ret;

	if (feature_name == NULL || path == NULL) {
		usage(NULL);
	}

	feature = hbsdctrl_ctx_find_feature_by_name(ctx, feature_name);
	if (feature == NULL) {
		return (1);
	}

	fd = open_file(path);
	if (fd == -1) {
		perror("open");
		return (1);
	}

#if 0
	cap_enter();
#endif

	res = hbsdctrl_feature_call_cb(feature, "unapply", &fd, NULL);
	switch (res) {
	case RES_SUCCESS:
		ret = 0;
		break;
	default:
		ret = 2;
	}

	close(fd);
	return (ret);
}

int
main(int argc, char *argv[])
{
	hbsdctrl_ctx_t *ctx;
	int ch, res;
	const char *ns, *verb, *feature_name;

	res = 0;
	prog = argv[0];
	ns = LIBHBSDCONTROL_DEFAULT_NAMESPACE;
	while ((ch = getopt(argc, argv, "Hhdn:")) != -1) {
		switch (ch) {
		case 'H':
			open_flags ^= O_NOFOLLOW;
			break;
		case 'd':
			verbose++;
			break;
		case 'n':
			ns = optarg;
			break;
		case 'h':
			feature_name = argv[optind];
			usage(feature_name);
			break;
		default:
			usage(NULL);
		}
	}

	if (optind == 0 || argc - optind < 3) {
		usage(NULL);
	}

	if (strcmp(argv[optind], "pax")) {
		usage(false);
	}

	ctx = hbsdctrl_ctx_new(0, ns);
	if (ctx == NULL) {
		fprintf(stderr, "[-] Could not create new hbsdctrl context\n");
		exit(1);
	}

	verb = argv[optind + 1];

	if (!strcmp(verb, "list")) {
		if (geteuid() && !strcmp(ns, "system")) {
			res = RES_FAIL;
			fprintf(stderr, "%s", admin_privs_required);
		} else {
			res = do_list(ctx, argv[optind + 2]);
		}
		goto end;
	}
	if (!strcmp(verb, "enable")) {
		if (argc - optind < 4) {
			usage(NULL);
		} else if (geteuid() && !strcmp(ns, "system")) {
			res = RES_FAIL;
			fprintf(stderr, "%s", admin_privs_required);
		} else {
			errno = 0;
			res = set_state(ctx, argv[optind + 2], HBSDCTRL_STATE_ENABLED,
					argv[optind + 3]);
			if (res) {
				fprintf(stderr, "[-] Failed to enable feature (%s)\n", strerror(errno));
			}
		}
		goto end;
	}
	if (!strcmp(verb, "disable")) {
		if (argc - optind < 4) {
			usage(NULL);
		} else if (geteuid() && !strcmp(ns, "system")) {
			res = RES_FAIL;
			fprintf(stderr, "%s", admin_privs_required);
		} else {
			errno = 0;
			res = set_state(ctx, argv[optind + 2], HBSDCTRL_STATE_DISABLED,
					argv[optind + 3]);
			if (res) {
				fprintf(stderr, "[-] Failed to disable feature (%s)\n", strerror(errno));
			}
		}
		goto end;
	}
	if (!strcmp(verb, "reset") || !strcmp(verb, "sysdef")) {
		if (argc - optind < 4) {
			usage(NULL);
		} else if (geteuid() && !strcmp(ns, "system")) {
			res = RES_FAIL;
			fprintf(stderr, "%s", admin_privs_required);
		} else {
			errno = 0;
			res = reset(ctx, argv[optind + 2], argv[optind + 3]);
			if (res) {
				fprintf(stderr, "[-] Failed to %s feature (%s)\n", verb, strerror(errno));
			}
		}
		goto end;
	}

	res = RES_FAIL;
	fprintf(stderr, "[-] Unknown verb: %s - did you mean enable|disable|list|sysdef ?\n", verb);
	goto end;

 end:
	hbsdctrl_ctx_free(&ctx);
	return (res);
}
