/*-
 * Copyright (c) 1999-2002, 2007-2011 Robert N. M. Watson
 * Copyright (c) 2001-2005 McAfee, Inc.
 * Copyright (c) 2005-2006 SPARTA, Inc.
 * Copyright (c) 2008 Apple Inc.
 * All rights reserved.
 *
 * This software was developed by Robert Watson for the TrustedBSD Project.
 *
 * This software was developed for the FreeBSD Project in part by McAfee
 * Research, the Security Research Division of McAfee, Inc. under
 * DARPA/SPAWAR contract N66001-01-C-8035 ("CBOSS"), as part of the DARPA
 * CHATS research program.
 *
 * This software was enhanced by SPARTA ISSO under SPAWAR contract
 * N66001-04-C-6019 ("SEFOS").
 *
 * This software was developed at the University of Cambridge Computer
 * Laboratory with support from a grant from Google, Inc.
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
 * Stub module that implements a NOOP for most (if not all) MAC Framework
 * policy entry points.
 */
module;

#include <cstddef>
#include <cstdint>
#include <sys/types.h>

export module pbsd.sys.security.mac.stub.b0046;

namespace pbsd::sys_security_mac_stub::b0046 {

#ifndef EPERM
constexpr int EPERM = 1;
#endif

#define SOCK_LOCK(so) ((void)0)
#define SOCK_UNLOCK(so) ((void)0)
#define SOCK_LOCK_ASSERT(so) ((void)0)

typedef unsigned int u_int;
typedef unsigned long u_long;
typedef unsigned int accmode_t;
typedef unsigned long acl_type_t;
typedef long db_expr_t;

struct timespec {
	long tv_sec;
	long tv_nsec;
};

struct mac_policy_conf;
struct thread;
struct label;
struct sbuf;
struct bpf_d;
struct ifnet;
struct mbuf;
struct ucred;
struct auditinfo;
struct auditinfo_addr;
struct db_command;
struct db_command_table;
struct mount;
struct cdev;
struct devfs_dirent;
struct vnode;
struct inpcb;
struct socket;
struct ip6q;
struct ipq;
struct kdb_dbbe;
struct pipepair;
struct ksem;
struct shmfd;
struct proc;
struct sockaddr;
struct sysctl_oid;
struct sysctl_req;
struct msqid_kernel;
struct msg;
struct semid_kernel;
struct shmid_kernel;
struct componentname;
struct vattr;
struct image_params;
struct acl;

export void
stub_destroy(struct mac_policy_conf *conf)
{

}

export void
stub_init(struct mac_policy_conf *conf)
{

}

export int
stub_syscall(struct thread *td, int call, void *arg)
{

	return (0);
}

export void
stub_init_label(struct label *label)
{

}

export int
stub_init_label_waitcheck(struct label *label, int flag)
{

	return (0);
}

export void
stub_destroy_label(struct label *label)
{

}

export void
stub_copy_label(struct label *src, struct label *dest)
{

}

export int
stub_externalize_label(struct label *label, char *element_name,
    struct sbuf *sb, int *claimed)
{

	return (0);
}

export int
stub_internalize_label(struct label *label, char *element_name,
    char *element_data, int *claimed)
{

	return (0);
}

export int
stub_bpfdesc_check_receive(struct bpf_d *d, struct label *dlabel,
    struct ifnet *ifp, struct label *ifplabel)
{

        return (0);
}

export void
stub_bpfdesc_create(struct ucred *cred, struct bpf_d *d,
    struct label *dlabel)
{

}

export void
stub_bpfdesc_create_mbuf(struct bpf_d *d, struct label *dlabel,
    struct mbuf *m, struct label *mlabel)
{

}

export void
stub_cred_associate_nfsd(struct ucred *cred)
{

}

export int
stub_cred_check_relabel(struct ucred *cred, struct label *newlabel)
{

	return (0);
}

export int
stub_cred_check_setaudit(struct ucred *cred, struct auditinfo *ai)
{

	return (0);
}

export int
stub_cred_check_setaudit_addr(struct ucred *cred, struct auditinfo_addr *aia)
{

	return (0);
}

export int
stub_cred_check_setauid(struct ucred *cred, uid_t auid)
{

	return (0);
}

export void
stub_cred_setcred_enter(void)
{
}

export int
stub_cred_check_setcred(u_int flags, const struct ucred *old_cred,
    struct ucred *new_cred)
{
	return (0);
}

export void
stub_cred_setcred_exit(void)
{
}

export int
stub_cred_check_setegid(struct ucred *cred, gid_t egid)
{

	return (0);
}

export int
stub_cred_check_seteuid(struct ucred *cred, uid_t euid)
{

	return (0);
}

export int
stub_cred_check_setgid(struct ucred *cred, gid_t gid)
{

	return (0);
}

export int
stub_cred_check_setgroups(struct ucred *cred, int ngroups,
	gid_t *gidset)
{

	return (0);
}

export int
stub_cred_check_setregid(struct ucred *cred, gid_t rgid, gid_t egid)
{

	return (0);
}

export int
stub_cred_check_setresgid(struct ucred *cred, gid_t rgid, gid_t egid,
	gid_t sgid)
{

	return (0);
}

export int
stub_cred_check_setresuid(struct ucred *cred, uid_t ruid, uid_t euid,
	uid_t suid)
{

	return (0);
}

export int
stub_cred_check_setreuid(struct ucred *cred, uid_t ruid, uid_t euid)
{

	return (0);
}

export int
stub_cred_check_setuid(struct ucred *cred, uid_t uid)
{

	return (0);
}

export int
stub_cred_check_visible(struct ucred *cr1, struct ucred *cr2)
{

	return (0);
}

export void
stub_cred_create_init(struct ucred *cred)
{

}

export void
stub_cred_create_swapper(struct ucred *cred)
{

}

export void
stub_cred_relabel(struct ucred *cred, struct label *newlabel)
{

}

export int
stub_ddb_command_exec(struct db_command *cmd, db_expr_t addr, bool have_addr,
    db_expr_t count, char *modif)
{

	return (0);
}

export int
stub_ddb_command_register(struct db_command_table *table,
    struct db_command *cmd)
{

	return (0);
}

export void
stub_devfs_create_device(struct ucred *cred, struct mount *mp,
    struct cdev *dev, struct devfs_dirent *de, struct label *delabel)
{

}

export void
stub_devfs_create_directory(struct mount *mp, char *dirname,
    int dirnamelen, struct devfs_dirent *de, struct label *delabel)
{

}

export void
stub_devfs_create_symlink(struct ucred *cred, struct mount *mp,
    struct devfs_dirent *dd, struct label *ddlabel, struct devfs_dirent *de,
    struct label *delabel)
{

}

export void
stub_devfs_update(struct mount *mp, struct devfs_dirent *de,
    struct label *delabel, struct vnode *vp, struct label *vplabel)
{

}

export void
stub_devfs_vnode_associate(struct mount *mp, struct label *mplabel,
    struct devfs_dirent *de, struct label *delabel, struct vnode *vp,
    struct label *vplabel)
{

}

export int
stub_ifnet_check_relabel(struct ucred *cred, struct ifnet *ifp,
    struct label *ifplabel, struct label *newlabel)
{

	return (0);
}

export int
stub_ifnet_check_transmit(struct ifnet *ifp, struct label *ifplabel,
    struct mbuf *m, struct label *mlabel)
{

	return (0);
}

export void
stub_ifnet_create(struct ifnet *ifp, struct label *ifplabel)
{

}

export void
stub_ifnet_create_mbuf(struct ifnet *ifp, struct label *ifplabel,
    struct mbuf *m, struct label *mlabel)
{

}

export void
stub_ifnet_relabel(struct ucred *cred, struct ifnet *ifp,
    struct label *ifplabel, struct label *newlabel)
{

}

export int
stub_inpcb_check_deliver(struct inpcb *inp, struct label *inplabel,
    struct mbuf *m, struct label *mlabel)
{

	return (0);
}

export void
stub_inpcb_create(struct socket *so, struct label *solabel,
    struct inpcb *inp, struct label *inplabel)
{

}

export void
stub_inpcb_create_mbuf(struct inpcb *inp, struct label *inplabel,
    struct mbuf *m, struct label *mlabel)
{

}

export void
stub_inpcb_sosetlabel(struct socket *so, struct label *solabel,
    struct inpcb *inp, struct label *inplabel)
{

	SOCK_LOCK_ASSERT(so);

}

export void
stub_ip6q_create(struct mbuf *m, struct label *mlabel, struct ip6q *q6,
    struct label *q6label)
{

}

export int
stub_ip6q_match(struct mbuf *m, struct label *mlabel, struct ip6q *q6,
    struct label *q6label)
{

	return (1);
}

export void
stub_ip6q_reassemble(struct ip6q *q6, struct label *q6label, struct mbuf *m,
    struct label *mlabel)
{

}

export void
stub_ip6q_update(struct mbuf *m, struct label *mlabel, struct ip6q *q6,
    struct label *q6label)
{

}

export void
stub_ipq_create(struct mbuf *m, struct label *mlabel, struct ipq *q,
    struct label *qlabel)
{

}

export int
stub_ipq_match(struct mbuf *m, struct label *mlabel, struct ipq *q,
    struct label *qlabel)
{

	return (1);
}

export void
stub_ipq_reassemble(struct ipq *q, struct label *qlabel, struct mbuf *m,
    struct label *mlabel)
{

}

export void
stub_ipq_update(struct mbuf *m, struct label *mlabel, struct ipq *q,
    struct label *qlabel)
{

}

export int
stub_kdb_check_backend(struct kdb_dbbe *be)
{

	return (0);
}

export int
stub_kenv_check_dump(struct ucred *cred)
{

	return (0);
}

export int
stub_kenv_check_get(struct ucred *cred, char *name)
{

	return (0);
}

export int
stub_kenv_check_set(struct ucred *cred, char *name, char *value)
{

	return (0);
}

export int
stub_kenv_check_unset(struct ucred *cred, char *name)
{

	return (0);
}

export int
stub_kld_check_load(struct ucred *cred, struct vnode *vp,
    struct label *vplabel)
{

	return (0);
}

export int
stub_kld_check_stat(struct ucred *cred)
{

	return (0);
}

export int
stub_mount_check_stat(struct ucred *cred, struct mount *mp,
    struct label *mplabel)
{

	return (0);
}

export void
stub_mount_create(struct ucred *cred, struct mount *mp,
    struct label *mplabel)
{

}

export void
stub_netinet_arp_send(struct ifnet *ifp, struct label *iflpabel,
    struct mbuf *m, struct label *mlabel)
{

}

export void
stub_netinet_firewall_reply(struct mbuf *mrecv, struct label *mrecvlabel,
    struct mbuf *msend, struct label *msendlabel)
{

}

export void
stub_netinet_firewall_send(struct mbuf *m, struct label *mlabel)
{

}

export void
stub_netinet_fragment(struct mbuf *m, struct label *mlabel, struct mbuf *frag,
    struct label *fraglabel)
{

}

export void
stub_netinet_icmp_reply(struct mbuf *mrecv, struct label *mrecvlabel,
    struct mbuf *msend, struct label *msendlabel)
{

}

export void
stub_netinet_icmp_replyinplace(struct mbuf *m, struct label *mlabel)
{

}

export void
stub_netinet_igmp_send(struct ifnet *ifp, struct label *iflpabel,
    struct mbuf *m, struct label *mlabel)
{

}

export void
stub_netinet_tcp_reply(struct mbuf *m, struct label *mlabel)
{

}

export void
stub_netinet6_nd6_send(struct ifnet *ifp, struct label *iflpabel,
    struct mbuf *m, struct label *mlabel)
{

}

export int
stub_pipe_check_ioctl(struct ucred *cred, struct pipepair *pp,
    struct label *pplabel, unsigned long cmd, void /* caddr_t */ *data)
{

	return (0);
}

export int
stub_pipe_check_poll(struct ucred *cred, struct pipepair *pp,
    struct label *pplabel)
{

	return (0);
}

export int
stub_pipe_check_read(struct ucred *cred, struct pipepair *pp,
    struct label *pplabel)
{

	return (0);
}

export int
stub_pipe_check_relabel(struct ucred *cred, struct pipepair *pp,
    struct label *pplabel, struct label *newlabel)
{

	return (0);
}

export int
stub_pipe_check_stat(struct ucred *cred, struct pipepair *pp,
    struct label *pplabel)
{

	return (0);
}

export int
stub_pipe_check_write(struct ucred *cred, struct pipepair *pp,
    struct label *pplabel)
{

	return (0);
}

export void
stub_pipe_create(struct ucred *cred, struct pipepair *pp,
    struct label *pplabel)
{

}

export void
stub_pipe_relabel(struct ucred *cred, struct pipepair *pp,
    struct label *pplabel, struct label *newlabel)
{

}

export int
stub_posixsem_check_getvalue(struct ucred *active_cred, struct ucred *file_cred,
    struct ksem *ks, struct label *kslabel)
{

	return (0);
}

export int
stub_posixsem_check_open(struct ucred *cred, struct ksem *ks,
    struct label *kslabel)
{

	return (0);
}

export int
stub_posixsem_check_post(struct ucred *active_cred, struct ucred *file_cred,
    struct ksem *ks, struct label *kslabel)
{

	return (0);
}

export int
stub_posixsem_check_setmode(struct ucred *cred, struct ksem *ks,
    struct label *kslabel, mode_t mode)
{

	return (0);
}

export int
stub_posixsem_check_setowner(struct ucred *cred, struct ksem *ks,
    struct label *kslabel, uid_t uid, gid_t gid)
{

	return (0);
}

export int
stub_posixsem_check_stat(struct ucred *active_cred, struct ucred *file_cred,
    struct ksem *ks, struct label *kslabel)
{

	return (0);
}

export int
stub_posixsem_check_unlink(struct ucred *cred, struct ksem *ks,
    struct label *kslabel)
{

	return (0);
}

export int
stub_posixsem_check_wait(struct ucred *active_cred, struct ucred *file_cred,
    struct ksem *ks, struct label *kslabel)
{

	return (0);
}

export void
stub_posixsem_create(struct ucred *cred, struct ksem *ks,
    struct label *kslabel)
{

}

export int
stub_posixshm_check_create(struct ucred *cred, const char *path)
{

	return (0);
}

export int
stub_posixshm_check_mmap(struct ucred *cred, struct shmfd *shmfd,
    struct label *shmlabel, int prot, int flags)
{

	return (0);
}

export int
stub_posixshm_check_open(struct ucred *cred, struct shmfd *shmfd,
    struct label *shmlabel, accmode_t accmode)
{

	return (0);
}

export int
stub_posixshm_check_read(struct ucred *active_cred, struct ucred *file_cred,
    struct shmfd *shm, struct label *shmlabel)
{

	return (0);
}

export int
stub_posixshm_check_setmode(struct ucred *cred, struct shmfd *shmfd,
    struct label *shmlabel, mode_t mode)
{

	return (0);
}

export int
stub_posixshm_check_setowner(struct ucred *cred, struct shmfd *shmfd,
    struct label *shmlabel, uid_t uid, gid_t gid)
{

	return (0);
}

export int
stub_posixshm_check_stat(struct ucred *active_cred, struct ucred *file_cred,
    struct shmfd *shmfd, struct label *shmlabel)
{

	return (0);
}

export int
stub_posixshm_check_truncate(struct ucred *active_cred,
    struct ucred *file_cred, struct shmfd *shmfd, struct label *shmlabel)
{

	return (0);
}

export int
stub_posixshm_check_unlink(struct ucred *cred, struct shmfd *shmfd,
    struct label *shmlabel)
{

	return (0);
}

export int
stub_posixshm_check_write(struct ucred *active_cred, struct ucred *file_cred,
    struct shmfd *shm, struct label *shmlabel)
{

	return (0);
}

export void
stub_posixshm_create(struct ucred *cred, struct shmfd *shmfd,
    struct label *shmlabel)
{

}

export int
stub_priv_check(struct ucred *cred, int priv)
{

	return (0);
}

export int
stub_priv_grant(struct ucred *cred, int priv)
{

	return (EPERM);
}

export int
stub_proc_check_debug(struct ucred *cred, struct proc *p)
{

	return (0);
}

export int
stub_proc_check_sched(struct ucred *cred, struct proc *p)
{

	return (0);
}

export int
stub_proc_check_signal(struct ucred *cred, struct proc *p, int signum)
{

	return (0);
}

export int
stub_proc_check_wait(struct ucred *cred, struct proc *p)
{

	return (0);
}

export int
stub_socket_check_accept(struct ucred *cred, struct socket *so,
    struct label *solabel)
{

#if 0
	SOCK_LOCK(so);
	SOCK_UNLOCK(so);
#endif

	return (0);
}

export int
stub_socket_check_bind(struct ucred *cred, struct socket *so,
    struct label *solabel, struct sockaddr *sa)
{

#if 0
	SOCK_LOCK(so);
	SOCK_UNLOCK(so);
#endif

	return (0);
}

export int
stub_socket_check_connect(struct ucred *cred, struct socket *so,
    struct label *solabel, struct sockaddr *sa)
{

#if 0
	SOCK_LOCK(so);
	SOCK_UNLOCK(so);
#endif

	return (0);
}

export int
stub_socket_check_create(struct ucred *cred, int domain, int type, int proto)
{

	return (0);
}

export int
stub_socket_check_deliver(struct socket *so, struct label *solabel,
    struct mbuf *m, struct label *mlabel)
{

#if 0
	SOCK_LOCK(so);
	SOCK_UNLOCK(so);
#endif

	return (0);
}

export int
stub_socket_check_listen(struct ucred *cred, struct socket *so,
    struct label *solabel)
{

#if 0
	SOCK_LOCK(so);
	SOCK_UNLOCK(so);
#endif

	return (0);
}

export int
stub_socket_check_poll(struct ucred *cred, struct socket *so,
    struct label *solabel)
{

#if 0
	SOCK_LOCK(so);
	SOCK_UNLOCK(so);
#endif

	return (0);
}

export int
stub_socket_check_receive(struct ucred *cred, struct socket *so,
    struct label *solabel)
{

#if 0
	SOCK_LOCK(so);
	SOCK_UNLOCK(so);
#endif

	return (0);
}

export int
stub_socket_check_relabel(struct ucred *cred, struct socket *so,
    struct label *solabel, struct label *newlabel)
{

	SOCK_LOCK_ASSERT(so);

	return (0);
}

export int
stub_socket_check_send(struct ucred *cred, struct socket *so,
    struct label *solabel)
{

#if 0
	SOCK_LOCK(so);
	SOCK_UNLOCK(so);
#endif

	return (0);
}

export int
stub_socket_check_stat(struct ucred *cred, struct socket *so,
    struct label *solabel)
{

#if 0
	SOCK_LOCK(so);
	SOCK_UNLOCK(so);
#endif

	return (0);
}

export int
stub_inpcb_check_visible(struct ucred *cred, struct inpcb *inp,
   struct label *inplabel)
{

	return (0);
}

export int
stub_socket_check_visible(struct ucred *cred, struct socket *so,
   struct label *solabel)
{

#if 0
	SOCK_LOCK(so);
	SOCK_UNLOCK(so);
#endif

	return (0);
}

export void
stub_socket_create(struct ucred *cred, struct socket *so,
    struct label *solabel)
{

}

export void
stub_socket_create_mbuf(struct socket *so, struct label *solabel,
    struct mbuf *m, struct label *mlabel)
{

#if 0
	SOCK_LOCK(so);
	SOCK_UNLOCK(so);
#endif
}

export void
stub_socket_newconn(struct socket *oldso, struct label *oldsolabel,
    struct socket *newso, struct label *newsolabel)
{

#if 0
	SOCK_LOCK(oldso);
	SOCK_UNLOCK(oldso);
#endif
#if 0
	SOCK_LOCK(newso);
	SOCK_UNLOCK(newso);
#endif
}

export void
stub_socket_relabel(struct ucred *cred, struct socket *so,
    struct label *solabel, struct label *newlabel)
{

	SOCK_LOCK_ASSERT(so);
}

export void
stub_socketpeer_set_from_mbuf(struct mbuf *m, struct label *mlabel,
    struct socket *so, struct label *sopeerlabel)
{

#if 0
	SOCK_LOCK(so);
	SOCK_UNLOCK(so);
#endif
}

export void
stub_socketpeer_set_from_socket(struct socket *oldso,
    struct label *oldsolabel, struct socket *newso,
    struct label *newsopeerlabel)
{

#if 0
	SOCK_LOCK(oldso);
	SOCK_UNLOCK(oldso);
#endif
#if 0
	SOCK_LOCK(newso);
	SOCK_UNLOCK(newso);
#endif
}

export void
stub_syncache_create(struct label *label, struct inpcb *inp)
{

}

export void
stub_syncache_create_mbuf(struct label *sc_label, struct mbuf *m,
    struct label *mlabel)
{

}

export int
stub_system_check_acct(struct ucred *cred, struct vnode *vp,
    struct label *vplabel)
{

	return (0);
}

export int
stub_system_check_audit(struct ucred *cred, void *record, int length)
{

	return (0);
}

export int
stub_system_check_auditctl(struct ucred *cred, struct vnode *vp,
    struct label *vplabel)
{

	return (0);
}

export int
stub_system_check_auditon(struct ucred *cred, int cmd)
{

	return (0);
}

export int
stub_system_check_reboot(struct ucred *cred, int how)
{

	return (0);
}

export int
stub_system_check_swapoff(struct ucred *cred, struct vnode *vp,
    struct label *vplabel)
{

	return (0);
}

export int
stub_system_check_swapon(struct ucred *cred, struct vnode *vp,
    struct label *vplabel)
{

	return (0);
}

export int
stub_system_check_sysctl(struct ucred *cred, struct sysctl_oid *oidp,
    void *arg1, int arg2, struct sysctl_req *req)
{

	return (0);
}

export void
stub_sysvmsg_cleanup(struct label *msglabel)
{

}

export void
stub_sysvmsg_create(struct ucred *cred, struct msqid_kernel *msqkptr,
    struct label *msqlabel, struct msg *msgptr, struct label *msglabel)
{

}

export int
stub_sysvmsq_check_msgmsq(struct ucred *cred, struct msg *msgptr,
    struct label *msglabel, struct msqid_kernel *msqkptr,
    struct label *msqklabel)
{

	return (0);
}

export int
stub_sysvmsq_check_msgrcv(struct ucred *cred, struct msg *msgptr,
    struct label *msglabel)
{

	return (0);
}

export int
stub_sysvmsq_check_msgrmid(struct ucred *cred, struct msg *msgptr,
    struct label *msglabel)
{

	return (0);
}

export int
stub_sysvmsq_check_msqget(struct ucred *cred, struct msqid_kernel *msqkptr,
    struct label *msqklabel)
{

	return (0);
}

export int
stub_sysvmsq_check_msqsnd(struct ucred *cred, struct msqid_kernel *msqkptr,
    struct label *msqklabel)
{

	return (0);
}

export int
stub_sysvmsq_check_msqrcv(struct ucred *cred, struct msqid_kernel *msqkptr,
    struct label *msqklabel)
{

	return (0);
}

export int
stub_sysvmsq_check_msqctl(struct ucred *cred, struct msqid_kernel *msqkptr,
    struct label *msqklabel, int cmd)
{

	return (0);
}

export void
stub_sysvmsq_cleanup(struct label *msqlabel)
{

}

export void
stub_sysvmsq_create(struct ucred *cred, struct msqid_kernel *msqkptr,
    struct label *msqlabel)
{

}

export int
stub_sysvsem_check_semctl(struct ucred *cred, struct semid_kernel *semakptr,
    struct label *semaklabel, int cmd)
{

	return (0);
}

export int
stub_sysvsem_check_semget(struct ucred *cred, struct semid_kernel *semakptr,
    struct label *semaklabel)
{

	return (0);
}

export int
stub_sysvsem_check_semop(struct ucred *cred, struct semid_kernel *semakptr,
    struct label *semaklabel, size_t accesstype)
{

	return (0);
}

export void
stub_sysvsem_cleanup(struct label *semalabel)
{

}

export void
stub_sysvsem_create(struct ucred *cred, struct semid_kernel *semakptr,
    struct label *semalabel)
{

}

export int
stub_sysvshm_check_shmat(struct ucred *cred, struct shmid_kernel *shmsegptr,
    struct label *shmseglabel, int shmflg)
{

	return (0);
}

export int
stub_sysvshm_check_shmctl(struct ucred *cred, struct shmid_kernel *shmsegptr,
    struct label *shmseglabel, int cmd)
{

	return (0);
}

export int
stub_sysvshm_check_shmdt(struct ucred *cred, struct shmid_kernel *shmsegptr,
    struct label *shmseglabel)
{

	return (0);
}

export int
stub_sysvshm_check_shmget(struct ucred *cred, struct shmid_kernel *shmsegptr,
    struct label *shmseglabel, int shmflg)
{

	return (0);
}

export void
stub_sysvshm_cleanup(struct label *shmlabel)
{

}

export void
stub_sysvshm_create(struct ucred *cred, struct shmid_kernel *shmsegptr,
    struct label *shmalabel)
{

}

export void
stub_thread_userret(struct thread *td)
{

}

export int
stub_vnode_associate_extattr(struct mount *mp, struct label *mplabel,
    struct vnode *vp, struct label *vplabel)
{

	return (0);
}

export void
stub_vnode_associate_singlelabel(struct mount *mp, struct label *mplabel,
    struct vnode *vp, struct label *vplabel)
{

}

export int
stub_vnode_check_access(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, accmode_t accmode)
{

	return (0);
}

export int
stub_vnode_check_chdir(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel)
{

	return (0);
}

export int
stub_vnode_check_chroot(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel)
{

	return (0);
}

export int
stub_vnode_check_create(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel, struct componentname *cnp, struct vattr *vap)
{

	return (0);
}

export int
stub_vnode_check_deleteacl(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, acl_type_t type)
{

	return (0);
}

export int
stub_vnode_check_deleteextattr(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, int attrnamespace, const char *name)
{

	return (0);
}

export int
stub_vnode_check_exec(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, struct image_params *imgp,
    struct label *execlabel)
{

	return (0);
}

export int
stub_vnode_check_getacl(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, acl_type_t type)
{

	return (0);
}

export int
stub_vnode_check_getextattr(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, int attrnamespace, const char *name)
{

	return (0);
}

export int
stub_vnode_check_link(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel, struct vnode *vp, struct label *vplabel,
    struct componentname *cnp)
{

	return (0);
}

export int
stub_vnode_check_listextattr(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, int attrnamespace)
{

	return (0);
}

export int
stub_vnode_check_lookup(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel, struct componentname *cnp)
{

	return (0);
}

export int
stub_vnode_check_mmap(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, int prot, int flags)
{

	return (0);
}

export void
stub_vnode_check_mmap_downgrade(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, int *prot)
{

}

export int
stub_vnode_check_mprotect(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, int prot)
{

	return (0);
}

export int
stub_vnode_check_open(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, accmode_t accmode)
{

	return (0);
}

export int
stub_vnode_check_poll(struct ucred *active_cred, struct ucred *file_cred,
    struct vnode *vp, struct label *vplabel)
{

	return (0);
}

export int
stub_vnode_check_read(struct ucred *active_cred, struct ucred *file_cred,
    struct vnode *vp, struct label *vplabel)
{

	return (0);
}

export int
stub_vnode_check_readdir(struct ucred *cred, struct vnode *vp,
    struct label *dvplabel)
{

	return (0);
}

export int
stub_vnode_check_readlink(struct ucred *cred, struct vnode *vp,
    struct label *vplabel)
{

	return (0);
}

export int
stub_vnode_check_relabel(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, struct label *newlabel)
{

	return (0);
}

export int
stub_vnode_check_rename_from(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel, struct vnode *vp, struct label *vplabel,
    struct componentname *cnp)
{

	return (0);
}

export int
stub_vnode_check_rename_to(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel, struct vnode *vp, struct label *vplabel,
    int samedir, struct componentname *cnp)
{

	return (0);
}

export int
stub_vnode_check_revoke(struct ucred *cred, struct vnode *vp,
    struct label *vplabel)
{

	return (0);
}

export int
stub_vnode_check_setacl(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, acl_type_t type, struct acl *acl)
{

	return (0);
}

export int
stub_vnode_check_setextattr(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, int attrnamespace, const char *name)
{

	return (0);
}

export int
stub_vnode_check_setflags(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, u_long flags)
{

	return (0);
}

export int
stub_vnode_check_setmode(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, mode_t mode)
{

	return (0);
}

export int
stub_vnode_check_setowner(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, uid_t uid, gid_t gid)
{

	return (0);
}

export int
stub_vnode_check_setutimes(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, struct timespec atime, struct timespec mtime)
{

	return (0);
}

export int
stub_vnode_check_stat(struct ucred *active_cred, struct ucred *file_cred,
    struct vnode *vp, struct label *vplabel)
{

	return (0);
}

export int
stub_vnode_check_unlink(struct ucred *cred, struct vnode *dvp,
    struct label *dvplabel, struct vnode *vp, struct label *vplabel,
    struct componentname *cnp)
{

	return (0);
}

export int
stub_vnode_check_write(struct ucred *active_cred, struct ucred *file_cred,
    struct vnode *vp, struct label *vplabel)
{

	return (0);
}

export int
stub_vnode_create_extattr(struct ucred *cred, struct mount *mp,
    struct label *mntlabel, struct vnode *dvp, struct label *dvplabel,
    struct vnode *vp, struct label *vplabel, struct componentname *cnp)
{

	return (0);
}

export void
stub_vnode_execve_transition(struct ucred *old, struct ucred *new,
    struct vnode *vp, struct label *vplabel, struct label *interpvplabel,
    struct image_params *imgp, struct label *execlabel)
{

}

export int
stub_vnode_execve_will_transition(struct ucred *old, struct vnode *vp,
    struct label *vplabel, struct label *interpvplabel,
    struct image_params *imgp, struct label *execlabel)
{

	return (0);
}

export void
stub_vnode_relabel(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, struct label *label)
{

}

export int
stub_vnode_setlabel_extattr(struct ucred *cred, struct vnode *vp,
    struct label *vplabel, struct label *intlabel)
{

	return (0);
}


} // namespace pbsd::sys_security_mac_stub::b0046
