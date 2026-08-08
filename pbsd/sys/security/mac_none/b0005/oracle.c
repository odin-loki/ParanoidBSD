/*
 * oracle.c -- reference implementation for PBSD batch b0005.
 *
 * hbsd/src/sys/security/mac_none/mac_none.c concatenated with scaffolding
 * so the original module-registration data compiles in userspace.  There are
 * no functions in the source; MAC_POLICY_SET is reproduced verbatim.
 */

#include <stddef.h>
#include <stdint.h>

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * 8)
#endif

#define LIST_ENTRY(type) \
	struct { \
		struct type *le_next; \
		struct type *le_prev; \
	}

struct mac_policy_ops {
	void *mpo_destroy;
	void *mpo_init;
	void *mpo_syscall;
	void *mpo_bpfdesc_check_receive;
	void *mpo_bpfdesc_create;
	void *mpo_bpfdesc_create_mbuf;
	void *mpo_bpfdesc_destroy_label;
	void *mpo_bpfdesc_init_label;
	void *mpo_cred_associate_nfsd;
	void *mpo_cred_check_relabel;
	void *mpo_cred_check_setaudit;
	void *mpo_cred_check_setaudit_addr;
	void *mpo_cred_check_setauid;
	void *mpo_cred_setcred_enter;
	void *mpo_cred_check_setcred;
	void *mpo_cred_setcred_exit;
	void *mpo_cred_check_setuid;
	void *mpo_cred_check_seteuid;
	void *mpo_cred_check_setgid;
	void *mpo_cred_check_setegid;
	void *mpo_cred_check_setgroups;
	void *mpo_cred_check_setreuid;
	void *mpo_cred_check_setregid;
	void *mpo_cred_check_setresuid;
	void *mpo_cred_check_setresgid;
	void *mpo_cred_check_visible;
	void *mpo_cred_copy_label;
	void *mpo_cred_create_swapper;
	void *mpo_cred_create_init;
	void *mpo_cred_destroy_label;
	void *mpo_cred_externalize_label;
	void *mpo_cred_init_label;
	void *mpo_cred_internalize_label;
	void *mpo_cred_relabel;
	void *mpo_ddb_command_register;
	void *mpo_ddb_command_exec;
	void *mpo_devfs_create_device;
	void *mpo_devfs_create_directory;
	void *mpo_devfs_create_symlink;
	void *mpo_devfs_destroy_label;
	void *mpo_devfs_init_label;
	void *mpo_devfs_update;
	void *mpo_devfs_vnode_associate;
	void *mpo_ifnet_check_relabel;
	void *mpo_ifnet_check_transmit;
	void *mpo_ifnet_copy_label;
	void *mpo_ifnet_create;
	void *mpo_ifnet_create_mbuf;
	void *mpo_ifnet_destroy_label;
	void *mpo_ifnet_externalize_label;
	void *mpo_ifnet_init_label;
	void *mpo_ifnet_internalize_label;
	void *mpo_ifnet_relabel;
	void *mpo_inpcb_check_deliver;
	void *mpo_inpcb_check_visible;
	void *mpo_inpcb_create;
	void *mpo_inpcb_create_mbuf;
	void *mpo_inpcb_destroy_label;
	void *mpo_inpcb_init_label;
	void *mpo_inpcb_sosetlabel;
	void *mpo_ip4_check_jail;
	void *mpo_ip6_check_jail;
	void *mpo_ip6q_create;
	void *mpo_ip6q_destroy_label;
	void *mpo_ip6q_init_label;
	void *mpo_ip6q_match;
	void *mpo_ip6q_reassemble;
	void *mpo_ip6q_update;
	void *mpo_ipq_create;
	void *mpo_ipq_destroy_label;
	void *mpo_ipq_init_label;
	void *mpo_ipq_match;
	void *mpo_ipq_reassemble;
	void *mpo_ipq_update;
	void *mpo_kdb_check_backend;
	void *mpo_kenv_check_dump;
	void *mpo_kenv_check_get;
	void *mpo_kenv_check_set;
	void *mpo_kenv_check_unset;
	void *mpo_kld_check_load;
	void *mpo_kld_check_stat;
	void *mpo_mbuf_copy_label;
	void *mpo_mbuf_destroy_label;
	void *mpo_mbuf_init_label;
	void *mpo_mount_check_stat;
	void *mpo_mount_create;
	void *mpo_mount_destroy_label;
	void *mpo_mount_init_label;
	void *mpo_netinet_arp_send;
	void *mpo_netinet_firewall_reply;
	void *mpo_netinet_firewall_send;
	void *mpo_netinet_fragment;
	void *mpo_netinet_icmp_reply;
	void *mpo_netinet_icmp_replyinplace;
	void *mpo_netinet_igmp_send;
	void *mpo_netinet_tcp_reply;
	void *mpo_netinet6_nd6_send;
	void *mpo_pipe_check_ioctl;
	void *mpo_pipe_check_poll;
	void *mpo_pipe_check_read;
	void *mpo_pipe_check_relabel;
	void *mpo_pipe_check_stat;
	void *mpo_pipe_check_write;
	void *mpo_pipe_copy_label;
	void *mpo_pipe_create;
	void *mpo_pipe_destroy_label;
	void *mpo_pipe_externalize_label;
	void *mpo_pipe_init_label;
	void *mpo_pipe_internalize_label;
	void *mpo_pipe_relabel;
	void *mpo_posixsem_check_getvalue;
	void *mpo_posixsem_check_open;
	void *mpo_posixsem_check_post;
	void *mpo_posixsem_check_setmode;
	void *mpo_posixsem_check_setowner;
	void *mpo_posixsem_check_stat;
	void *mpo_posixsem_check_unlink;
	void *mpo_posixsem_check_wait;
	void *mpo_posixsem_create;
	void *mpo_posixsem_destroy_label;
	void *mpo_posixsem_init_label;
	void *mpo_posixshm_check_create;
	void *mpo_posixshm_check_mmap;
	void *mpo_posixshm_check_open;
	void *mpo_posixshm_check_read;
	void *mpo_posixshm_check_setmode;
	void *mpo_posixshm_check_setowner;
	void *mpo_posixshm_check_stat;
	void *mpo_posixshm_check_truncate;
	void *mpo_posixshm_check_unlink;
	void *mpo_posixshm_check_write;
	void *mpo_posixshm_create;
	void *mpo_posixshm_destroy_label;
	void *mpo_posixshm_init_label;
	void *mpo_priv_check;
	void *mpo_priv_grant;
	void *mpo_proc_check_debug;
	void *mpo_proc_check_sched;
	void *mpo_proc_check_signal;
	void *mpo_proc_check_wait;
	void *mpo_proc_destroy_label;
	void *mpo_proc_init_label;
	void *mpo_socket_check_accept;
	void *mpo_socket_check_bind;
	void *mpo_socket_check_connect;
	void *mpo_socket_check_create;
	void *mpo_socket_check_deliver;
	void *mpo_socket_check_listen;
	void *mpo_socket_check_poll;
	void *mpo_socket_check_receive;
	void *mpo_socket_check_relabel;
	void *mpo_socket_check_send;
	void *mpo_socket_check_stat;
	void *mpo_socket_check_visible;
	void *mpo_socket_copy_label;
	void *mpo_socket_create;
	void *mpo_socket_create_mbuf;
	void *mpo_socket_destroy_label;
	void *mpo_socket_externalize_label;
	void *mpo_socket_init_label;
	void *mpo_socket_internalize_label;
	void *mpo_socket_newconn;
	void *mpo_socket_relabel;
	void *mpo_socketpeer_destroy_label;
	void *mpo_socketpeer_externalize_label;
	void *mpo_socketpeer_init_label;
	void *mpo_socketpeer_set_from_mbuf;
	void *mpo_socketpeer_set_from_socket;
	void *mpo_syncache_init_label;
	void *mpo_syncache_destroy_label;
	void *mpo_syncache_create;
	void *mpo_syncache_create_mbuf;
	void *mpo_system_check_acct;
	void *mpo_system_check_audit;
	void *mpo_system_check_auditctl;
	void *mpo_system_check_auditon;
	void *mpo_system_check_reboot;
	void *mpo_system_check_swapon;
	void *mpo_system_check_swapoff;
	void *mpo_system_check_sysctl;
	void *mpo_sysvmsg_cleanup;
	void *mpo_sysvmsg_create;
	void *mpo_sysvmsg_destroy_label;
	void *mpo_sysvmsg_init_label;
	void *mpo_sysvmsq_check_msgmsq;
	void *mpo_sysvmsq_check_msgrcv;
	void *mpo_sysvmsq_check_msgrmid;
	void *mpo_sysvmsq_check_msqctl;
	void *mpo_sysvmsq_check_msqget;
	void *mpo_sysvmsq_check_msqrcv;
	void *mpo_sysvmsq_check_msqsnd;
	void *mpo_sysvmsq_cleanup;
	void *mpo_sysvmsq_create;
	void *mpo_sysvmsq_destroy_label;
	void *mpo_sysvmsq_init_label;
	void *mpo_sysvsem_check_semctl;
	void *mpo_sysvsem_check_semget;
	void *mpo_sysvsem_check_semop;
	void *mpo_sysvsem_cleanup;
	void *mpo_sysvsem_create;
	void *mpo_sysvsem_destroy_label;
	void *mpo_sysvsem_init_label;
	void *mpo_sysvshm_check_shmat;
	void *mpo_sysvshm_check_shmctl;
	void *mpo_sysvshm_check_shmdt;
	void *mpo_sysvshm_check_shmget;
	void *mpo_sysvshm_cleanup;
	void *mpo_sysvshm_create;
	void *mpo_sysvshm_destroy_label;
	void *mpo_sysvshm_init_label;
	void *mpo_thread_userret;
	void *mpo_vnode_check_access;
	void *mpo_vnode_check_chdir;
	void *mpo_vnode_check_chroot;
	void *mpo_vnode_check_create;
	void *mpo_vnode_check_deleteacl;
	void *mpo_vnode_check_deleteextattr;
	void *mpo_vnode_check_exec;
	void *mpo_vnode_check_getacl;
	void *mpo_vnode_check_getextattr;
	void *mpo_vnode_check_link;
	void *mpo_vnode_check_listextattr;
	void *mpo_vnode_check_lookup;
	void *mpo_vnode_check_mmap;
	void *mpo_vnode_check_mmap_downgrade;
	void *mpo_vnode_check_mprotect;
	void *mpo_vnode_check_open;
	void *mpo_vnode_check_poll;
	void *mpo_vnode_check_read;
	void *mpo_vnode_check_readdir;
	void *mpo_vnode_check_readlink;
	void *mpo_vnode_check_relabel;
	void *mpo_vnode_check_rename_from;
	void *mpo_vnode_check_rename_to;
	void *mpo_vnode_check_revoke;
	void *mpo_vnode_check_setacl;
	void *mpo_vnode_check_setextattr;
	void *mpo_vnode_check_setflags;
	void *mpo_vnode_check_setmode;
	void *mpo_vnode_check_setowner;
	void *mpo_vnode_check_setutimes;
	void *mpo_vnode_check_stat;
	void *mpo_vnode_check_unlink;
	void *mpo_vnode_check_write;
	void *mpo_vnode_associate_extattr;
	void *mpo_vnode_associate_singlelabel;
	void *mpo_vnode_destroy_label;
	void *mpo_vnode_copy_label;
	void *mpo_vnode_create_extattr;
	void *mpo_vnode_execve_transition;
	void *mpo_vnode_execve_will_transition;
	void *mpo_vnode_externalize_label;
	void *mpo_vnode_init_label;
	void *mpo_vnode_internalize_label;
	void *mpo_vnode_relabel;
	void *mpo_vnode_setlabel_extattr;
	void *mpo_prison_destroy;
};

struct mac_policy_conf {
	char				*mpc_name;
	char				*mpc_fullname;
	struct mac_policy_ops	*mpc_ops;
	int				 mpc_loadtime_flags;
	int				*mpc_field_off;
	int				 mpc_runtime_flags;
	int				 _mpc_spare1;
	uint64_t		 _mpc_spare2;
	uint64_t		 _mpc_spare3;
	void				*_mpc_spare4;
	LIST_ENTRY(mac_policy_conf)	 mpc_list;
};

#define	MPC_LOADTIME_FLAG_NOTLATE	0x00000001
#define	MPC_LOADTIME_FLAG_UNLOADOK	0x00000002

#define	MAC_VERSION	6

typedef struct module *module_t;

typedef struct moduledata {
	const char *name;
	int (*modevent)(module_t, int, void *);
	void *priv;
} moduledata_t;

#define	DECLARE_MODULE(name, data, sub, order)
#define	MODULE_DEPEND(mod, what, ver, pref, max)

int mac_policy_modevent(module_t mod, int type, void *data);

#define	MAC_POLICY_SET(mpops, mpname, mpfullname, mpflags, privdata_wanted) \
	static struct mac_policy_conf mpname##_mac_policy_conf = { \
		.mpc_name = #mpname, \
		.mpc_fullname = mpfullname, \
		.mpc_ops = mpops, \
		.mpc_loadtime_flags = mpflags, \
		.mpc_field_off = privdata_wanted, \
	}; \
	static moduledata_t mpname##_mod = { \
		#mpname, \
		mac_policy_modevent, \
		&mpname##_mac_policy_conf \
	}; \
	MODULE_DEPEND(mpname, kernel_mac_support, MAC_VERSION, \
	    MAC_VERSION, MAC_VERSION); \
	DECLARE_MODULE(mpname, mpname##_mod, SI_SUB_MAC_POLICY, \
	    SI_ORDER_MIDDLE)

int
mac_policy_modevent(module_t mod, int type, void *data)
{

	(void)mod;
	(void)type;
	(void)data;
	return (0);
}

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



static struct mac_policy_ops none_ops =
{
};

MAC_POLICY_SET(&none_ops, mac_none, "TrustedBSD MAC/None",
    MPC_LOADTIME_FLAG_UNLOADOK, NULL);

/* harness accessors */
const struct mac_policy_ops *
ref_none_ops(void)
{

	return (&none_ops);
}

const struct mac_policy_conf *
ref_mac_none_mac_policy_conf(void)
{

	return (&mac_none_mac_policy_conf);
}

const moduledata_t *
ref_mac_none_mod(void)
{

	return (&mac_none_mod);
}

int
ref_mac_policy_modevent(module_t mod, int type, void *data)
{

	return (mac_policy_modevent(mod, type, data));
}
