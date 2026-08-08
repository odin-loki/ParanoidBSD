/* Differential harness for mac_stub b0046 */
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.sys.security.mac.stub.b0046;
namespace port = pbsd::sys_security_mac_stub::b0046;

extern "C" {
extern void ref_stub_destroy(struct mac_policy_conf *conf);
extern void ref_stub_init(struct mac_policy_conf *conf);
extern int ref_stub_syscall(struct thread *td, int call, void *arg);
extern void ref_stub_init_label(struct label *label);
extern int ref_stub_init_label_waitcheck(struct label *label, int flag);
extern void ref_stub_destroy_label(struct label *label);
extern void ref_stub_copy_label(struct label *src, struct label *dest);
extern int ref_stub_externalize_label(struct label *label, char *element_name,     struct sbuf *sb, int *claimed);
extern int ref_stub_internalize_label(struct label *label, char *element_name,     char *element_data, int *claimed);
extern int ref_stub_bpfdesc_check_receive(struct bpf_d *d, struct label *dlabel,     struct ifnet *ifp, struct label *ifplabel);
extern void ref_stub_bpfdesc_create(struct ucred *cred, struct bpf_d *d,     struct label *dlabel);
extern void ref_stub_bpfdesc_create_mbuf(struct bpf_d *d, struct label *dlabel,     struct mbuf *m, struct label *mlabel);
extern void ref_stub_cred_associate_nfsd(struct ucred *cred);
extern int ref_stub_cred_check_relabel(struct ucred *cred, struct label *newlabel);
extern int ref_stub_cred_check_setaudit(struct ucred *cred, struct auditinfo *ai);
extern int ref_stub_cred_check_setaudit_addr(struct ucred *cred, struct auditinfo_addr *aia);
extern int ref_stub_cred_check_setauid(struct ucred *cred, uid_t auid);
extern void ref_stub_cred_setcred_enter(void);
extern int ref_stub_cred_check_setcred(u_int flags, const struct ucred *old_cred,     struct ucred *new_cred);
extern void ref_stub_cred_setcred_exit(void);
extern int ref_stub_cred_check_setegid(struct ucred *cred, gid_t egid);
extern int ref_stub_cred_check_seteuid(struct ucred *cred, uid_t euid);
extern int ref_stub_cred_check_setgid(struct ucred *cred, gid_t gid);
extern int ref_stub_cred_check_setgroups(struct ucred *cred, int ngroups,  gid_t *gidset);
extern int ref_stub_cred_check_setregid(struct ucred *cred, gid_t rgid, gid_t egid);
extern int ref_stub_cred_check_setresgid(struct ucred *cred, gid_t rgid, gid_t egid,  gid_t sgid);
extern int ref_stub_cred_check_setresuid(struct ucred *cred, uid_t ruid, uid_t euid,  uid_t suid);
extern int ref_stub_cred_check_setreuid(struct ucred *cred, uid_t ruid, uid_t euid);
extern int ref_stub_cred_check_setuid(struct ucred *cred, uid_t uid);
extern int ref_stub_cred_check_visible(struct ucred *cr1, struct ucred *cr2);
extern void ref_stub_cred_create_init(struct ucred *cred);
extern void ref_stub_cred_create_swapper(struct ucred *cred);
extern void ref_stub_cred_relabel(struct ucred *cred, struct label *newlabel);
extern int ref_stub_ddb_command_exec(struct db_command *cmd, db_expr_t addr, bool have_addr,     db_expr_t count, char *modif);
extern int ref_stub_ddb_command_register(struct db_command_table *table,     struct db_command *cmd);
extern void ref_stub_devfs_create_device(struct ucred *cred, struct mount *mp,     struct cdev *dev, struct devfs_dirent *de, struct label *delabel);
extern void ref_stub_devfs_create_directory(struct mount *mp, char *dirname,     int dirnamelen, struct devfs_dirent *de, struct label *delabel);
extern void ref_stub_devfs_create_symlink(struct ucred *cred, struct mount *mp,     struct devfs_dirent *dd, struct label *ddlabel, struct devfs_dirent *de,     struct label *delabel);
extern void ref_stub_devfs_update(struct mount *mp, struct devfs_dirent *de,     struct label *delabel, struct vnode *vp, struct label *vplabel);
extern void ref_stub_devfs_vnode_associate(struct mount *mp, struct label *mplabel,     struct devfs_dirent *de, struct label *delabel, struct vnode *vp,     struct label *vplabel);
extern int ref_stub_ifnet_check_relabel(struct ucred *cred, struct ifnet *ifp,     struct label *ifplabel, struct label *newlabel);
extern int ref_stub_ifnet_check_transmit(struct ifnet *ifp, struct label *ifplabel,     struct mbuf *m, struct label *mlabel);
extern void ref_stub_ifnet_create(struct ifnet *ifp, struct label *ifplabel);
extern void ref_stub_ifnet_create_mbuf(struct ifnet *ifp, struct label *ifplabel,     struct mbuf *m, struct label *mlabel);
extern void ref_stub_ifnet_relabel(struct ucred *cred, struct ifnet *ifp,     struct label *ifplabel, struct label *newlabel);
extern int ref_stub_inpcb_check_deliver(struct inpcb *inp, struct label *inplabel,     struct mbuf *m, struct label *mlabel);
extern void ref_stub_inpcb_create(struct socket *so, struct label *solabel,     struct inpcb *inp, struct label *inplabel);
extern void ref_stub_inpcb_create_mbuf(struct inpcb *inp, struct label *inplabel,     struct mbuf *m, struct label *mlabel);
extern void ref_stub_inpcb_sosetlabel(struct socket *so, struct label *solabel,     struct inpcb *inp, struct label *inplabel);
extern void ref_stub_ip6q_create(struct mbuf *m, struct label *mlabel, struct ip6q *q6,     struct label *q6label);
extern int ref_stub_ip6q_match(struct mbuf *m, struct label *mlabel, struct ip6q *q6,     struct label *q6label);
extern void ref_stub_ip6q_reassemble(struct ip6q *q6, struct label *q6label, struct mbuf *m,     struct label *mlabel);
extern void ref_stub_ip6q_update(struct mbuf *m, struct label *mlabel, struct ip6q *q6,     struct label *q6label);
extern void ref_stub_ipq_create(struct mbuf *m, struct label *mlabel, struct ipq *q,     struct label *qlabel);
extern int ref_stub_ipq_match(struct mbuf *m, struct label *mlabel, struct ipq *q,     struct label *qlabel);
extern void ref_stub_ipq_reassemble(struct ipq *q, struct label *qlabel, struct mbuf *m,     struct label *mlabel);
extern void ref_stub_ipq_update(struct mbuf *m, struct label *mlabel, struct ipq *q,     struct label *qlabel);
extern int ref_stub_kdb_check_backend(struct kdb_dbbe *be);
extern int ref_stub_kenv_check_dump(struct ucred *cred);
extern int ref_stub_kenv_check_get(struct ucred *cred, char *name);
extern int ref_stub_kenv_check_set(struct ucred *cred, char *name, char *value);
extern int ref_stub_kenv_check_unset(struct ucred *cred, char *name);
extern int ref_stub_kld_check_load(struct ucred *cred, struct vnode *vp,     struct label *vplabel);
extern int ref_stub_kld_check_stat(struct ucred *cred);
extern int ref_stub_mount_check_stat(struct ucred *cred, struct mount *mp,     struct label *mplabel);
extern void ref_stub_mount_create(struct ucred *cred, struct mount *mp,     struct label *mplabel);
extern void ref_stub_netinet_arp_send(struct ifnet *ifp, struct label *iflpabel,     struct mbuf *m, struct label *mlabel);
extern void ref_stub_netinet_firewall_reply(struct mbuf *mrecv, struct label *mrecvlabel,     struct mbuf *msend, struct label *msendlabel);
extern void ref_stub_netinet_firewall_send(struct mbuf *m, struct label *mlabel);
extern void ref_stub_netinet_fragment(struct mbuf *m, struct label *mlabel, struct mbuf *frag,     struct label *fraglabel);
extern void ref_stub_netinet_icmp_reply(struct mbuf *mrecv, struct label *mrecvlabel,     struct mbuf *msend, struct label *msendlabel);
extern void ref_stub_netinet_icmp_replyinplace(struct mbuf *m, struct label *mlabel);
extern void ref_stub_netinet_igmp_send(struct ifnet *ifp, struct label *iflpabel,     struct mbuf *m, struct label *mlabel);
extern void ref_stub_netinet_tcp_reply(struct mbuf *m, struct label *mlabel);
extern void ref_stub_netinet6_nd6_send(struct ifnet *ifp, struct label *iflpabel,     struct mbuf *m, struct label *mlabel);
extern int ref_stub_pipe_check_ioctl(struct ucred *cred, struct pipepair *pp,     struct label *pplabel, unsigned long cmd, void /* caddr_t */ *data);
extern int ref_stub_pipe_check_poll(struct ucred *cred, struct pipepair *pp,     struct label *pplabel);
extern int ref_stub_pipe_check_read(struct ucred *cred, struct pipepair *pp,     struct label *pplabel);
extern int ref_stub_pipe_check_relabel(struct ucred *cred, struct pipepair *pp,     struct label *pplabel, struct label *newlabel);
extern int ref_stub_pipe_check_stat(struct ucred *cred, struct pipepair *pp,     struct label *pplabel);
extern int ref_stub_pipe_check_write(struct ucred *cred, struct pipepair *pp,     struct label *pplabel);
extern void ref_stub_pipe_create(struct ucred *cred, struct pipepair *pp,     struct label *pplabel);
extern void ref_stub_pipe_relabel(struct ucred *cred, struct pipepair *pp,     struct label *pplabel, struct label *newlabel);
extern int ref_stub_posixsem_check_getvalue(struct ucred *active_cred, struct ucred *file_cred,     struct ksem *ks, struct label *kslabel);
extern int ref_stub_posixsem_check_open(struct ucred *cred, struct ksem *ks,     struct label *kslabel);
extern int ref_stub_posixsem_check_post(struct ucred *active_cred, struct ucred *file_cred,     struct ksem *ks, struct label *kslabel);
extern int ref_stub_posixsem_check_setmode(struct ucred *cred, struct ksem *ks,     struct label *kslabel, mode_t mode);
extern int ref_stub_posixsem_check_setowner(struct ucred *cred, struct ksem *ks,     struct label *kslabel, uid_t uid, gid_t gid);
extern int ref_stub_posixsem_check_stat(struct ucred *active_cred, struct ucred *file_cred,     struct ksem *ks, struct label *kslabel);
extern int ref_stub_posixsem_check_unlink(struct ucred *cred, struct ksem *ks,     struct label *kslabel);
extern int ref_stub_posixsem_check_wait(struct ucred *active_cred, struct ucred *file_cred,     struct ksem *ks, struct label *kslabel);
extern void ref_stub_posixsem_create(struct ucred *cred, struct ksem *ks,     struct label *kslabel);
extern int ref_stub_posixshm_check_create(struct ucred *cred, const char *path);
extern int ref_stub_posixshm_check_mmap(struct ucred *cred, struct shmfd *shmfd,     struct label *shmlabel, int prot, int flags);
extern int ref_stub_posixshm_check_open(struct ucred *cred, struct shmfd *shmfd,     struct label *shmlabel, accmode_t accmode);
extern int ref_stub_posixshm_check_read(struct ucred *active_cred, struct ucred *file_cred,     struct shmfd *shm, struct label *shmlabel);
extern int ref_stub_posixshm_check_setmode(struct ucred *cred, struct shmfd *shmfd,     struct label *shmlabel, mode_t mode);
extern int ref_stub_posixshm_check_setowner(struct ucred *cred, struct shmfd *shmfd,     struct label *shmlabel, uid_t uid, gid_t gid);
extern int ref_stub_posixshm_check_stat(struct ucred *active_cred, struct ucred *file_cred,     struct shmfd *shmfd, struct label *shmlabel);
extern int ref_stub_posixshm_check_truncate(struct ucred *active_cred,     struct ucred *file_cred, struct shmfd *shmfd, struct label *shmlabel);
extern int ref_stub_posixshm_check_unlink(struct ucred *cred, struct shmfd *shmfd,     struct label *shmlabel);
extern int ref_stub_posixshm_check_write(struct ucred *active_cred, struct ucred *file_cred,     struct shmfd *shm, struct label *shmlabel);
extern void ref_stub_posixshm_create(struct ucred *cred, struct shmfd *shmfd,     struct label *shmlabel);
extern int ref_stub_priv_check(struct ucred *cred, int priv);
extern int ref_stub_priv_grant(struct ucred *cred, int priv);
extern int ref_stub_proc_check_debug(struct ucred *cred, struct proc *p);
extern int ref_stub_proc_check_sched(struct ucred *cred, struct proc *p);
extern int ref_stub_proc_check_signal(struct ucred *cred, struct proc *p, int signum);
extern int ref_stub_proc_check_wait(struct ucred *cred, struct proc *p);
extern int ref_stub_socket_check_accept(struct ucred *cred, struct socket *so,     struct label *solabel);
extern int ref_stub_socket_check_bind(struct ucred *cred, struct socket *so,     struct label *solabel, struct sockaddr *sa);
extern int ref_stub_socket_check_connect(struct ucred *cred, struct socket *so,     struct label *solabel, struct sockaddr *sa);
extern int ref_stub_socket_check_create(struct ucred *cred, int domain, int type, int proto);
extern int ref_stub_socket_check_deliver(struct socket *so, struct label *solabel,     struct mbuf *m, struct label *mlabel);
extern int ref_stub_socket_check_listen(struct ucred *cred, struct socket *so,     struct label *solabel);
extern int ref_stub_socket_check_poll(struct ucred *cred, struct socket *so,     struct label *solabel);
extern int ref_stub_socket_check_receive(struct ucred *cred, struct socket *so,     struct label *solabel);
extern int ref_stub_socket_check_relabel(struct ucred *cred, struct socket *so,     struct label *solabel, struct label *newlabel);
extern int ref_stub_socket_check_send(struct ucred *cred, struct socket *so,     struct label *solabel);
extern int ref_stub_socket_check_stat(struct ucred *cred, struct socket *so,     struct label *solabel);
extern int ref_stub_inpcb_check_visible(struct ucred *cred, struct inpcb *inp,    struct label *inplabel);
extern int ref_stub_socket_check_visible(struct ucred *cred, struct socket *so,    struct label *solabel);
extern void ref_stub_socket_create(struct ucred *cred, struct socket *so,     struct label *solabel);
extern void ref_stub_socket_create_mbuf(struct socket *so, struct label *solabel,     struct mbuf *m, struct label *mlabel);
extern void ref_stub_socket_newconn(struct socket *oldso, struct label *oldsolabel,     struct socket *newso, struct label *newsolabel);
extern void ref_stub_socket_relabel(struct ucred *cred, struct socket *so,     struct label *solabel, struct label *newlabel);
extern void ref_stub_socketpeer_set_from_mbuf(struct mbuf *m, struct label *mlabel,     struct socket *so, struct label *sopeerlabel);
extern void ref_stub_socketpeer_set_from_socket(struct socket *oldso,     struct label *oldsolabel, struct socket *newso,     struct label *newsopeerlabel);
extern void ref_stub_syncache_create(struct label *label, struct inpcb *inp);
extern void ref_stub_syncache_create_mbuf(struct label *sc_label, struct mbuf *m,     struct label *mlabel);
extern int ref_stub_system_check_acct(struct ucred *cred, struct vnode *vp,     struct label *vplabel);
extern int ref_stub_system_check_audit(struct ucred *cred, void *record, int length);
extern int ref_stub_system_check_auditctl(struct ucred *cred, struct vnode *vp,     struct label *vplabel);
extern int ref_stub_system_check_auditon(struct ucred *cred, int cmd);
extern int ref_stub_system_check_reboot(struct ucred *cred, int how);
extern int ref_stub_system_check_swapoff(struct ucred *cred, struct vnode *vp,     struct label *vplabel);
extern int ref_stub_system_check_swapon(struct ucred *cred, struct vnode *vp,     struct label *vplabel);
extern int ref_stub_system_check_sysctl(struct ucred *cred, struct sysctl_oid *oidp,     void *arg1, int arg2, struct sysctl_req *req);
extern void ref_stub_sysvmsg_cleanup(struct label *msglabel);
extern void ref_stub_sysvmsg_create(struct ucred *cred, struct msqid_kernel *msqkptr,     struct label *msqlabel, struct msg *msgptr, struct label *msglabel);
extern int ref_stub_sysvmsq_check_msgmsq(struct ucred *cred, struct msg *msgptr,     struct label *msglabel, struct msqid_kernel *msqkptr,     struct label *msqklabel);
extern int ref_stub_sysvmsq_check_msgrcv(struct ucred *cred, struct msg *msgptr,     struct label *msglabel);
extern int ref_stub_sysvmsq_check_msgrmid(struct ucred *cred, struct msg *msgptr,     struct label *msglabel);
extern int ref_stub_sysvmsq_check_msqget(struct ucred *cred, struct msqid_kernel *msqkptr,     struct label *msqklabel);
extern int ref_stub_sysvmsq_check_msqsnd(struct ucred *cred, struct msqid_kernel *msqkptr,     struct label *msqklabel);
extern int ref_stub_sysvmsq_check_msqrcv(struct ucred *cred, struct msqid_kernel *msqkptr,     struct label *msqklabel);
extern int ref_stub_sysvmsq_check_msqctl(struct ucred *cred, struct msqid_kernel *msqkptr,     struct label *msqklabel, int cmd);
extern void ref_stub_sysvmsq_cleanup(struct label *msqlabel);
extern void ref_stub_sysvmsq_create(struct ucred *cred, struct msqid_kernel *msqkptr,     struct label *msqlabel);
extern int ref_stub_sysvsem_check_semctl(struct ucred *cred, struct semid_kernel *semakptr,     struct label *semaklabel, int cmd);
extern int ref_stub_sysvsem_check_semget(struct ucred *cred, struct semid_kernel *semakptr,     struct label *semaklabel);
extern int ref_stub_sysvsem_check_semop(struct ucred *cred, struct semid_kernel *semakptr,     struct label *semaklabel, size_t accesstype);
extern void ref_stub_sysvsem_cleanup(struct label *semalabel);
extern void ref_stub_sysvsem_create(struct ucred *cred, struct semid_kernel *semakptr,     struct label *semalabel);
extern int ref_stub_sysvshm_check_shmat(struct ucred *cred, struct shmid_kernel *shmsegptr,     struct label *shmseglabel, int shmflg);
extern int ref_stub_sysvshm_check_shmctl(struct ucred *cred, struct shmid_kernel *shmsegptr,     struct label *shmseglabel, int cmd);
extern int ref_stub_sysvshm_check_shmdt(struct ucred *cred, struct shmid_kernel *shmsegptr,     struct label *shmseglabel);
extern int ref_stub_sysvshm_check_shmget(struct ucred *cred, struct shmid_kernel *shmsegptr,     struct label *shmseglabel, int shmflg);
extern void ref_stub_sysvshm_cleanup(struct label *shmlabel);
extern void ref_stub_sysvshm_create(struct ucred *cred, struct shmid_kernel *shmsegptr,     struct label *shmalabel);
extern void ref_stub_thread_userret(struct thread *td);
extern int ref_stub_vnode_associate_extattr(struct mount *mp, struct label *mplabel,     struct vnode *vp, struct label *vplabel);
extern void ref_stub_vnode_associate_singlelabel(struct mount *mp, struct label *mplabel,     struct vnode *vp, struct label *vplabel);
extern int ref_stub_vnode_check_access(struct ucred *cred, struct vnode *vp,     struct label *vplabel, accmode_t accmode);
extern int ref_stub_vnode_check_chdir(struct ucred *cred, struct vnode *dvp,     struct label *dvplabel);
extern int ref_stub_vnode_check_chroot(struct ucred *cred, struct vnode *dvp,     struct label *dvplabel);
extern int ref_stub_vnode_check_create(struct ucred *cred, struct vnode *dvp,     struct label *dvplabel, struct componentname *cnp, struct vattr *vap);
extern int ref_stub_vnode_check_deleteacl(struct ucred *cred, struct vnode *vp,     struct label *vplabel, acl_type_t type);
extern int ref_stub_vnode_check_deleteextattr(struct ucred *cred, struct vnode *vp,     struct label *vplabel, int attrnamespace, const char *name);
extern int ref_stub_vnode_check_exec(struct ucred *cred, struct vnode *vp,     struct label *vplabel, struct image_params *imgp,     struct label *execlabel);
extern int ref_stub_vnode_check_getacl(struct ucred *cred, struct vnode *vp,     struct label *vplabel, acl_type_t type);
extern int ref_stub_vnode_check_getextattr(struct ucred *cred, struct vnode *vp,     struct label *vplabel, int attrnamespace, const char *name);
extern int ref_stub_vnode_check_link(struct ucred *cred, struct vnode *dvp,     struct label *dvplabel, struct vnode *vp, struct label *vplabel,     struct componentname *cnp);
extern int ref_stub_vnode_check_listextattr(struct ucred *cred, struct vnode *vp,     struct label *vplabel, int attrnamespace);
extern int ref_stub_vnode_check_lookup(struct ucred *cred, struct vnode *dvp,     struct label *dvplabel, struct componentname *cnp);
extern int ref_stub_vnode_check_mmap(struct ucred *cred, struct vnode *vp,     struct label *vplabel, int prot, int flags);
extern void ref_stub_vnode_check_mmap_downgrade(struct ucred *cred, struct vnode *vp,     struct label *vplabel, int *prot);
extern int ref_stub_vnode_check_mprotect(struct ucred *cred, struct vnode *vp,     struct label *vplabel, int prot);
extern int ref_stub_vnode_check_open(struct ucred *cred, struct vnode *vp,     struct label *vplabel, accmode_t accmode);
extern int ref_stub_vnode_check_poll(struct ucred *active_cred, struct ucred *file_cred,     struct vnode *vp, struct label *vplabel);
extern int ref_stub_vnode_check_read(struct ucred *active_cred, struct ucred *file_cred,     struct vnode *vp, struct label *vplabel);
extern int ref_stub_vnode_check_readdir(struct ucred *cred, struct vnode *vp,     struct label *dvplabel);
extern int ref_stub_vnode_check_readlink(struct ucred *cred, struct vnode *vp,     struct label *vplabel);
extern int ref_stub_vnode_check_relabel(struct ucred *cred, struct vnode *vp,     struct label *vplabel, struct label *newlabel);
extern int ref_stub_vnode_check_rename_from(struct ucred *cred, struct vnode *dvp,     struct label *dvplabel, struct vnode *vp, struct label *vplabel,     struct componentname *cnp);
extern int ref_stub_vnode_check_rename_to(struct ucred *cred, struct vnode *dvp,     struct label *dvplabel, struct vnode *vp, struct label *vplabel,     int samedir, struct componentname *cnp);
extern int ref_stub_vnode_check_revoke(struct ucred *cred, struct vnode *vp,     struct label *vplabel);
extern int ref_stub_vnode_check_setacl(struct ucred *cred, struct vnode *vp,     struct label *vplabel, acl_type_t type, struct acl *acl);
extern int ref_stub_vnode_check_setextattr(struct ucred *cred, struct vnode *vp,     struct label *vplabel, int attrnamespace, const char *name);
extern int ref_stub_vnode_check_setflags(struct ucred *cred, struct vnode *vp,     struct label *vplabel, u_long flags);
extern int ref_stub_vnode_check_setmode(struct ucred *cred, struct vnode *vp,     struct label *vplabel, mode_t mode);
extern int ref_stub_vnode_check_setowner(struct ucred *cred, struct vnode *vp,     struct label *vplabel, uid_t uid, gid_t gid);
extern int ref_stub_vnode_check_setutimes(struct ucred *cred, struct vnode *vp,     struct label *vplabel, struct timespec atime, struct timespec mtime);
extern int ref_stub_vnode_check_stat(struct ucred *active_cred, struct ucred *file_cred,     struct vnode *vp, struct label *vplabel);
extern int ref_stub_vnode_check_unlink(struct ucred *cred, struct vnode *dvp,     struct label *dvplabel, struct vnode *vp, struct label *vplabel,     struct componentname *cnp);
extern int ref_stub_vnode_check_write(struct ucred *active_cred, struct ucred *file_cred,     struct vnode *vp, struct label *vplabel);
extern int ref_stub_vnode_create_extattr(struct ucred *cred, struct mount *mp,     struct label *mntlabel, struct vnode *dvp, struct label *dvplabel,     struct vnode *vp, struct label *vplabel, struct componentname *cnp);
extern void ref_stub_vnode_execve_transition(struct ucred *old, struct ucred *new,     struct vnode *vp, struct label *vplabel, struct label *interpvplabel,     struct image_params *imgp, struct label *execlabel);
extern int ref_stub_vnode_execve_will_transition(struct ucred *old, struct vnode *vp,     struct label *vplabel, struct label *interpvplabel,     struct image_params *imgp, struct label *execlabel);
extern void ref_stub_vnode_relabel(struct ucred *cred, struct vnode *vp,     struct label *vplabel, struct label *label);
extern int ref_stub_vnode_setlabel_extattr(struct ucred *cred, struct vnode *vp,     struct label *vplabel, struct label *intlabel);

}

struct FnStats { const char *name; long cases; long failures; };

static uint32_t rng_state = 0xb0046u;

static uint32_t xorshift32() {
    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state = x;
    return x;
}

static int ri(int lo, int hi) {
    return lo + (int)(xorshift32() % (uint32_t)(hi - lo + 1));
}

static char test_bufs[16][64];
static char *strs[8];
static int prot_bufs[8];
static int claimed_bufs[8];
static gid_t gidset[8];

static void init_test_data() {
    static const char *edge_strs[] = {
        "", "\0", "a", "\x80\xff", "\xff", "hello", "\x7f\x80\x00\xff", "x"
    };
    for (int i = 0; i < 8; ++i) {
        strs[i] = test_bufs[i];
        memset(test_bufs[i], 0x7f, sizeof(test_bufs[i]));
        size_t n = strlen(edge_strs[i]);
        memcpy(test_bufs[i], edge_strs[i], n + 1);
    }
    for (int i = 0; i < 8; ++i) {
        prot_bufs[i] = 0x7f7f7f7f;
        claimed_bufs[i] = 0x7f7f7f7f;
        gidset[i] = (gid_t)ri(0, 65535);
    }
}

static bool buf_unchanged(const void *before, const void *after, size_t n) {
    return memcmp(before, after, n) == 0;
}


static void test_stub_destroy(FnStats &st) {
    st.name = "stub_destroy";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_destroy(nullptr);
        port::stub_destroy(nullptr);
    }
    ++st.cases;
    {
        ref_stub_destroy(reinterpret_cast<mac_policy_conf*>((uintptr_t)0xdeadbeefUL));
        port::stub_destroy(reinterpret_cast<mac_policy_conf*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_destroy(reinterpret_cast<mac_policy_conf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_destroy(reinterpret_cast<mac_policy_conf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_init(FnStats &st) {
    st.name = "stub_init";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_init(nullptr);
        port::stub_init(nullptr);
    }
    ++st.cases;
    {
        ref_stub_init(reinterpret_cast<mac_policy_conf*>((uintptr_t)0xdeadbeefUL));
        port::stub_init(reinterpret_cast<mac_policy_conf*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_init(reinterpret_cast<mac_policy_conf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_init(reinterpret_cast<mac_policy_conf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_syscall(FnStats &st) {
    st.name = "stub_syscall";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_syscall(nullptr, 0, nullptr);
        int r_port = port::stub_syscall(nullptr, 0, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_syscall(reinterpret_cast<thread*>((uintptr_t)0xdeadbeefUL), -1, reinterpret_cast<void*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_syscall(reinterpret_cast<thread*>((uintptr_t)0xdeadbeefUL), -1, reinterpret_cast<void*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_syscall(reinterpret_cast<thread*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), reinterpret_cast<void*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_syscall(reinterpret_cast<thread*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), reinterpret_cast<void*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_init_label(FnStats &st) {
    st.name = "stub_init_label";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_init_label(nullptr);
        port::stub_init_label(nullptr);
    }
    ++st.cases;
    {
        ref_stub_init_label(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_init_label(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_init_label(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_init_label(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_init_label_waitcheck(FnStats &st) {
    st.name = "stub_init_label_waitcheck";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_init_label_waitcheck(nullptr, 0);
        int r_port = port::stub_init_label_waitcheck(nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_init_label_waitcheck(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        int r_port = port::stub_init_label_waitcheck(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_init_label_waitcheck(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_init_label_waitcheck(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_destroy_label(FnStats &st) {
    st.name = "stub_destroy_label";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_destroy_label(nullptr);
        port::stub_destroy_label(nullptr);
    }
    ++st.cases;
    {
        ref_stub_destroy_label(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_destroy_label(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_destroy_label(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_destroy_label(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_copy_label(FnStats &st) {
    st.name = "stub_copy_label";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_copy_label(nullptr, nullptr);
        port::stub_copy_label(nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_copy_label(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_copy_label(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_copy_label(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_copy_label(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_externalize_label(FnStats &st) {
    st.name = "stub_externalize_label";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_externalize_label(nullptr, strs[0], nullptr, &prot_bufs[0]);
        int r_port = port::stub_externalize_label(nullptr, strs[0], nullptr, &prot_bufs[0]);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_externalize_label(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), strs[3], reinterpret_cast<sbuf*>((uintptr_t)0xdeadbeefUL), &prot_bufs[1]);
        int r_port = port::stub_externalize_label(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), strs[3], reinterpret_cast<sbuf*>((uintptr_t)0xdeadbeefUL), &prot_bufs[1]);
        if (r_ref != r_port) ++st.failures;
    }
    {
        int pb_ref[4], pb_port[4], snap_ref[4], snap_port[4];
        memset(pb_ref, 0x7f, sizeof(pb_ref));
        memset(pb_port, 0x7f, sizeof(pb_port));
        memcpy(snap_ref, pb_ref, sizeof(pb_ref));
        memcpy(snap_port, pb_port, sizeof(pb_port));
        ++st.cases;
        int r_ref = ref_stub_externalize_label(nullptr, nullptr, nullptr, &pb_ref[0]);
        int r_port = port::stub_externalize_label(nullptr, nullptr, nullptr, &pb_port[0]);
        if (r_ref != r_port) ++st.failures;
        if (!buf_unchanged(snap_ref, pb_ref, sizeof(pb_ref))) ++st.failures;
        if (!buf_unchanged(snap_port, pb_port, sizeof(pb_port))) ++st.failures;
        if (memcmp(pb_ref, pb_port, sizeof(pb_ref))) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_externalize_label(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), strs[ri(0,7)], reinterpret_cast<sbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), &prot_bufs[ri(0,7)]);
        int r_port = port::stub_externalize_label(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), strs[ri(0,7)], reinterpret_cast<sbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), &prot_bufs[ri(0,7)]);
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_internalize_label(FnStats &st) {
    st.name = "stub_internalize_label";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_internalize_label(nullptr, strs[0], strs[0], &prot_bufs[0]);
        int r_port = port::stub_internalize_label(nullptr, strs[0], strs[0], &prot_bufs[0]);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_internalize_label(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), strs[3], strs[3], &prot_bufs[1]);
        int r_port = port::stub_internalize_label(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), strs[3], strs[3], &prot_bufs[1]);
        if (r_ref != r_port) ++st.failures;
    }
    {
        int pb_ref[4], pb_port[4], snap_ref[4], snap_port[4];
        memset(pb_ref, 0x7f, sizeof(pb_ref));
        memset(pb_port, 0x7f, sizeof(pb_port));
        memcpy(snap_ref, pb_ref, sizeof(pb_ref));
        memcpy(snap_port, pb_port, sizeof(pb_port));
        ++st.cases;
        int r_ref = ref_stub_internalize_label(nullptr, nullptr, nullptr, &pb_ref[0]);
        int r_port = port::stub_internalize_label(nullptr, nullptr, nullptr, &pb_port[0]);
        if (r_ref != r_port) ++st.failures;
        if (!buf_unchanged(snap_ref, pb_ref, sizeof(pb_ref))) ++st.failures;
        if (!buf_unchanged(snap_port, pb_port, sizeof(pb_port))) ++st.failures;
        if (memcmp(pb_ref, pb_port, sizeof(pb_ref))) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_internalize_label(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), strs[ri(0,7)], strs[ri(0,7)], &prot_bufs[ri(0,7)]);
        int r_port = port::stub_internalize_label(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), strs[ri(0,7)], strs[ri(0,7)], &prot_bufs[ri(0,7)]);
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_bpfdesc_check_receive(FnStats &st) {
    st.name = "stub_bpfdesc_check_receive";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_bpfdesc_check_receive(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_bpfdesc_check_receive(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_bpfdesc_check_receive(reinterpret_cast<bpf_d*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_bpfdesc_check_receive(reinterpret_cast<bpf_d*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_bpfdesc_check_receive(reinterpret_cast<bpf_d*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_bpfdesc_check_receive(reinterpret_cast<bpf_d*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_bpfdesc_create(FnStats &st) {
    st.name = "stub_bpfdesc_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_bpfdesc_create(nullptr, nullptr, nullptr);
        port::stub_bpfdesc_create(nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_bpfdesc_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<bpf_d*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_bpfdesc_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<bpf_d*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_bpfdesc_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<bpf_d*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_bpfdesc_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<bpf_d*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_bpfdesc_create_mbuf(FnStats &st) {
    st.name = "stub_bpfdesc_create_mbuf";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_bpfdesc_create_mbuf(nullptr, nullptr, nullptr, nullptr);
        port::stub_bpfdesc_create_mbuf(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_bpfdesc_create_mbuf(reinterpret_cast<bpf_d*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_bpfdesc_create_mbuf(reinterpret_cast<bpf_d*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_bpfdesc_create_mbuf(reinterpret_cast<bpf_d*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_bpfdesc_create_mbuf(reinterpret_cast<bpf_d*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_cred_associate_nfsd(FnStats &st) {
    st.name = "stub_cred_associate_nfsd";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_cred_associate_nfsd(nullptr);
        port::stub_cred_associate_nfsd(nullptr);
    }
    ++st.cases;
    {
        ref_stub_cred_associate_nfsd(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL));
        port::stub_cred_associate_nfsd(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_cred_associate_nfsd(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_cred_associate_nfsd(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_cred_check_relabel(FnStats &st) {
    st.name = "stub_cred_check_relabel";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_relabel(nullptr, nullptr);
        int r_port = port::stub_cred_check_relabel(nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_cred_check_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_cred_check_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_cred_check_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_cred_check_setaudit(FnStats &st) {
    st.name = "stub_cred_check_setaudit";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setaudit(nullptr, nullptr);
        int r_port = port::stub_cred_check_setaudit(nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setaudit(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<auditinfo*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_cred_check_setaudit(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<auditinfo*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_cred_check_setaudit(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<auditinfo*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_cred_check_setaudit(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<auditinfo*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_cred_check_setaudit_addr(FnStats &st) {
    st.name = "stub_cred_check_setaudit_addr";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setaudit_addr(nullptr, nullptr);
        int r_port = port::stub_cred_check_setaudit_addr(nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setaudit_addr(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<auditinfo_addr*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_cred_check_setaudit_addr(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<auditinfo_addr*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_cred_check_setaudit_addr(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<auditinfo_addr*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_cred_check_setaudit_addr(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<auditinfo_addr*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_cred_check_setauid(FnStats &st) {
    st.name = "stub_cred_check_setauid";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setauid(nullptr, 0);
        int r_port = port::stub_cred_check_setauid(nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setauid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff);
        int r_port = port::stub_cred_check_setauid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_cred_check_setauid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_cred_check_setauid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_cred_setcred_enter(FnStats &st) {
    st.name = "stub_cred_setcred_enter";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_cred_setcred_enter();
        port::stub_cred_setcred_enter();
    }
    ++st.cases;
    {
        ref_stub_cred_setcred_enter();
        port::stub_cred_setcred_enter();
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_cred_setcred_enter();
        port::stub_cred_setcred_enter();
    }
}

static void test_stub_cred_check_setcred(FnStats &st) {
    st.name = "stub_cred_check_setcred";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setcred(0, nullptr, nullptr);
        int r_port = port::stub_cred_check_setcred(0, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setcred(-1, reinterpret_cast<const struct ucred *>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_cred_check_setcred(-1, reinterpret_cast<const struct ucred *>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_cred_check_setcred(ri(-0x7fffffff, 0x7fffffff), reinterpret_cast<const struct ucred *>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_cred_check_setcred(ri(-0x7fffffff, 0x7fffffff), reinterpret_cast<const struct ucred *>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_cred_setcred_exit(FnStats &st) {
    st.name = "stub_cred_setcred_exit";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_cred_setcred_exit();
        port::stub_cred_setcred_exit();
    }
    ++st.cases;
    {
        ref_stub_cred_setcred_exit();
        port::stub_cred_setcred_exit();
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_cred_setcred_exit();
        port::stub_cred_setcred_exit();
    }
}

static void test_stub_cred_check_setegid(FnStats &st) {
    st.name = "stub_cred_check_setegid";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setegid(nullptr, 0);
        int r_port = port::stub_cred_check_setegid(nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setegid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff);
        int r_port = port::stub_cred_check_setegid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_cred_check_setegid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_cred_check_setegid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_cred_check_seteuid(FnStats &st) {
    st.name = "stub_cred_check_seteuid";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_seteuid(nullptr, 0);
        int r_port = port::stub_cred_check_seteuid(nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_seteuid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff);
        int r_port = port::stub_cred_check_seteuid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_cred_check_seteuid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_cred_check_seteuid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_cred_check_setgid(FnStats &st) {
    st.name = "stub_cred_check_setgid";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setgid(nullptr, 0);
        int r_port = port::stub_cred_check_setgid(nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setgid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff);
        int r_port = port::stub_cred_check_setgid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_cred_check_setgid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_cred_check_setgid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_cred_check_setgroups(FnStats &st) {
    st.name = "stub_cred_check_setgroups";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setgroups(nullptr, 0, gidset);
        int r_port = port::stub_cred_check_setgroups(nullptr, 0, gidset);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setgroups(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), -1, gidset);
        int r_port = port::stub_cred_check_setgroups(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), -1, gidset);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_cred_check_setgroups(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), gidset);
        int r_port = port::stub_cred_check_setgroups(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), gidset);
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_cred_check_setregid(FnStats &st) {
    st.name = "stub_cred_check_setregid";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setregid(nullptr, 0, 0);
        int r_port = port::stub_cred_check_setregid(nullptr, 0, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setregid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff, 0xff);
        int r_port = port::stub_cred_check_setregid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff, 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_cred_check_setregid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_cred_check_setregid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_cred_check_setresgid(FnStats &st) {
    st.name = "stub_cred_check_setresgid";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setresgid(nullptr, 0, 0, 0);
        int r_port = port::stub_cred_check_setresgid(nullptr, 0, 0, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setresgid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff, 0xff, 0xff);
        int r_port = port::stub_cred_check_setresgid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff, 0xff, 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_cred_check_setresgid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_cred_check_setresgid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_cred_check_setresuid(FnStats &st) {
    st.name = "stub_cred_check_setresuid";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setresuid(nullptr, 0, 0, 0);
        int r_port = port::stub_cred_check_setresuid(nullptr, 0, 0, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setresuid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff, 0xff, 0xff);
        int r_port = port::stub_cred_check_setresuid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff, 0xff, 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_cred_check_setresuid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_cred_check_setresuid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_cred_check_setreuid(FnStats &st) {
    st.name = "stub_cred_check_setreuid";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setreuid(nullptr, 0, 0);
        int r_port = port::stub_cred_check_setreuid(nullptr, 0, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setreuid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff, 0xff);
        int r_port = port::stub_cred_check_setreuid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff, 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_cred_check_setreuid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_cred_check_setreuid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_cred_check_setuid(FnStats &st) {
    st.name = "stub_cred_check_setuid";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setuid(nullptr, 0);
        int r_port = port::stub_cred_check_setuid(nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_setuid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff);
        int r_port = port::stub_cred_check_setuid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_cred_check_setuid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_cred_check_setuid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_cred_check_visible(FnStats &st) {
    st.name = "stub_cred_check_visible";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_visible(nullptr, nullptr);
        int r_port = port::stub_cred_check_visible(nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_cred_check_visible(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_cred_check_visible(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_cred_check_visible(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_cred_check_visible(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_cred_create_init(FnStats &st) {
    st.name = "stub_cred_create_init";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_cred_create_init(nullptr);
        port::stub_cred_create_init(nullptr);
    }
    ++st.cases;
    {
        ref_stub_cred_create_init(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL));
        port::stub_cred_create_init(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_cred_create_init(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_cred_create_init(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_cred_create_swapper(FnStats &st) {
    st.name = "stub_cred_create_swapper";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_cred_create_swapper(nullptr);
        port::stub_cred_create_swapper(nullptr);
    }
    ++st.cases;
    {
        ref_stub_cred_create_swapper(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL));
        port::stub_cred_create_swapper(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_cred_create_swapper(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_cred_create_swapper(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_cred_relabel(FnStats &st) {
    st.name = "stub_cred_relabel";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_cred_relabel(nullptr, nullptr);
        port::stub_cred_relabel(nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_cred_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_cred_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_cred_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_cred_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_ddb_command_exec(FnStats &st) {
    st.name = "stub_ddb_command_exec";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_ddb_command_exec(nullptr, 0, false, 0, strs[0]);
        int r_port = port::stub_ddb_command_exec(nullptr, 0, false, 0, strs[0]);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_ddb_command_exec(reinterpret_cast<db_command*>((uintptr_t)0xdeadbeefUL), -1, true, -1, strs[3]);
        int r_port = port::stub_ddb_command_exec(reinterpret_cast<db_command*>((uintptr_t)0xdeadbeefUL), -1, true, -1, strs[3]);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_ddb_command_exec(reinterpret_cast<db_command*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), ri(0,1)!=0, ri(-0x7fffffff, 0x7fffffff), strs[ri(0,7)]);
        int r_port = port::stub_ddb_command_exec(reinterpret_cast<db_command*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), ri(0,1)!=0, ri(-0x7fffffff, 0x7fffffff), strs[ri(0,7)]);
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_ddb_command_register(FnStats &st) {
    st.name = "stub_ddb_command_register";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_ddb_command_register(nullptr, nullptr);
        int r_port = port::stub_ddb_command_register(nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_ddb_command_register(reinterpret_cast<db_command_table*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<db_command*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_ddb_command_register(reinterpret_cast<db_command_table*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<db_command*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_ddb_command_register(reinterpret_cast<db_command_table*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<db_command*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_ddb_command_register(reinterpret_cast<db_command_table*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<db_command*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_devfs_create_device(FnStats &st) {
    st.name = "stub_devfs_create_device";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_devfs_create_device(nullptr, nullptr, nullptr, nullptr, nullptr);
        port::stub_devfs_create_device(nullptr, nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_devfs_create_device(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<cdev*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<devfs_dirent*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_devfs_create_device(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<cdev*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<devfs_dirent*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_devfs_create_device(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<cdev*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<devfs_dirent*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_devfs_create_device(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<cdev*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<devfs_dirent*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_devfs_create_directory(FnStats &st) {
    st.name = "stub_devfs_create_directory";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_devfs_create_directory(nullptr, strs[0], 0, nullptr, nullptr);
        port::stub_devfs_create_directory(nullptr, strs[0], 0, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_devfs_create_directory(reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), strs[3], -1, reinterpret_cast<devfs_dirent*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_devfs_create_directory(reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), strs[3], -1, reinterpret_cast<devfs_dirent*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_devfs_create_directory(reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), strs[ri(0,7)], ri(-0x7fffffff, 0x7fffffff), reinterpret_cast<devfs_dirent*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_devfs_create_directory(reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), strs[ri(0,7)], ri(-0x7fffffff, 0x7fffffff), reinterpret_cast<devfs_dirent*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_devfs_create_symlink(FnStats &st) {
    st.name = "stub_devfs_create_symlink";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_devfs_create_symlink(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        port::stub_devfs_create_symlink(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_devfs_create_symlink(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<devfs_dirent*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<devfs_dirent*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_devfs_create_symlink(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<devfs_dirent*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<devfs_dirent*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_devfs_create_symlink(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<devfs_dirent*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<devfs_dirent*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_devfs_create_symlink(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<devfs_dirent*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<devfs_dirent*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_devfs_update(FnStats &st) {
    st.name = "stub_devfs_update";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_devfs_update(nullptr, nullptr, nullptr, nullptr, nullptr);
        port::stub_devfs_update(nullptr, nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_devfs_update(reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<devfs_dirent*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_devfs_update(reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<devfs_dirent*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_devfs_update(reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<devfs_dirent*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_devfs_update(reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<devfs_dirent*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_devfs_vnode_associate(FnStats &st) {
    st.name = "stub_devfs_vnode_associate";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_devfs_vnode_associate(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        port::stub_devfs_vnode_associate(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_devfs_vnode_associate(reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<devfs_dirent*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_devfs_vnode_associate(reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<devfs_dirent*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_devfs_vnode_associate(reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<devfs_dirent*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_devfs_vnode_associate(reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<devfs_dirent*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_ifnet_check_relabel(FnStats &st) {
    st.name = "stub_ifnet_check_relabel";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_ifnet_check_relabel(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_ifnet_check_relabel(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_ifnet_check_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_ifnet_check_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_ifnet_check_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_ifnet_check_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_ifnet_check_transmit(FnStats &st) {
    st.name = "stub_ifnet_check_transmit";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_ifnet_check_transmit(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_ifnet_check_transmit(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_ifnet_check_transmit(reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_ifnet_check_transmit(reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_ifnet_check_transmit(reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_ifnet_check_transmit(reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_ifnet_create(FnStats &st) {
    st.name = "stub_ifnet_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_ifnet_create(nullptr, nullptr);
        port::stub_ifnet_create(nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_ifnet_create(reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_ifnet_create(reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_ifnet_create(reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_ifnet_create(reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_ifnet_create_mbuf(FnStats &st) {
    st.name = "stub_ifnet_create_mbuf";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_ifnet_create_mbuf(nullptr, nullptr, nullptr, nullptr);
        port::stub_ifnet_create_mbuf(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_ifnet_create_mbuf(reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_ifnet_create_mbuf(reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_ifnet_create_mbuf(reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_ifnet_create_mbuf(reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_ifnet_relabel(FnStats &st) {
    st.name = "stub_ifnet_relabel";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_ifnet_relabel(nullptr, nullptr, nullptr, nullptr);
        port::stub_ifnet_relabel(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_ifnet_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_ifnet_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_ifnet_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_ifnet_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_inpcb_check_deliver(FnStats &st) {
    st.name = "stub_inpcb_check_deliver";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_inpcb_check_deliver(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_inpcb_check_deliver(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_inpcb_check_deliver(reinterpret_cast<inpcb*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_inpcb_check_deliver(reinterpret_cast<inpcb*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_inpcb_check_deliver(reinterpret_cast<inpcb*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_inpcb_check_deliver(reinterpret_cast<inpcb*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_inpcb_create(FnStats &st) {
    st.name = "stub_inpcb_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_inpcb_create(nullptr, nullptr, nullptr, nullptr);
        port::stub_inpcb_create(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_inpcb_create(reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<inpcb*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_inpcb_create(reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<inpcb*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_inpcb_create(reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<inpcb*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_inpcb_create(reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<inpcb*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_inpcb_create_mbuf(FnStats &st) {
    st.name = "stub_inpcb_create_mbuf";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_inpcb_create_mbuf(nullptr, nullptr, nullptr, nullptr);
        port::stub_inpcb_create_mbuf(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_inpcb_create_mbuf(reinterpret_cast<inpcb*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_inpcb_create_mbuf(reinterpret_cast<inpcb*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_inpcb_create_mbuf(reinterpret_cast<inpcb*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_inpcb_create_mbuf(reinterpret_cast<inpcb*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_inpcb_sosetlabel(FnStats &st) {
    st.name = "stub_inpcb_sosetlabel";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_inpcb_sosetlabel(nullptr, nullptr, nullptr, nullptr);
        port::stub_inpcb_sosetlabel(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_inpcb_sosetlabel(reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<inpcb*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_inpcb_sosetlabel(reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<inpcb*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_inpcb_sosetlabel(reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<inpcb*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_inpcb_sosetlabel(reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<inpcb*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_ip6q_create(FnStats &st) {
    st.name = "stub_ip6q_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_ip6q_create(nullptr, nullptr, nullptr, nullptr);
        port::stub_ip6q_create(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_ip6q_create(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ip6q*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_ip6q_create(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ip6q*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_ip6q_create(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ip6q*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_ip6q_create(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ip6q*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_ip6q_match(FnStats &st) {
    st.name = "stub_ip6q_match";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_ip6q_match(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_ip6q_match(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_ip6q_match(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ip6q*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_ip6q_match(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ip6q*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_ip6q_match(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ip6q*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_ip6q_match(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ip6q*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_ip6q_reassemble(FnStats &st) {
    st.name = "stub_ip6q_reassemble";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_ip6q_reassemble(nullptr, nullptr, nullptr, nullptr);
        port::stub_ip6q_reassemble(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_ip6q_reassemble(reinterpret_cast<ip6q*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_ip6q_reassemble(reinterpret_cast<ip6q*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_ip6q_reassemble(reinterpret_cast<ip6q*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_ip6q_reassemble(reinterpret_cast<ip6q*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_ip6q_update(FnStats &st) {
    st.name = "stub_ip6q_update";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_ip6q_update(nullptr, nullptr, nullptr, nullptr);
        port::stub_ip6q_update(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_ip6q_update(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ip6q*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_ip6q_update(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ip6q*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_ip6q_update(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ip6q*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_ip6q_update(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ip6q*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_ipq_create(FnStats &st) {
    st.name = "stub_ipq_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_ipq_create(nullptr, nullptr, nullptr, nullptr);
        port::stub_ipq_create(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_ipq_create(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ipq*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_ipq_create(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ipq*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_ipq_create(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ipq*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_ipq_create(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ipq*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_ipq_match(FnStats &st) {
    st.name = "stub_ipq_match";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_ipq_match(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_ipq_match(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_ipq_match(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ipq*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_ipq_match(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ipq*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_ipq_match(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ipq*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_ipq_match(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ipq*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_ipq_reassemble(FnStats &st) {
    st.name = "stub_ipq_reassemble";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_ipq_reassemble(nullptr, nullptr, nullptr, nullptr);
        port::stub_ipq_reassemble(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_ipq_reassemble(reinterpret_cast<ipq*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_ipq_reassemble(reinterpret_cast<ipq*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_ipq_reassemble(reinterpret_cast<ipq*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_ipq_reassemble(reinterpret_cast<ipq*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_ipq_update(FnStats &st) {
    st.name = "stub_ipq_update";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_ipq_update(nullptr, nullptr, nullptr, nullptr);
        port::stub_ipq_update(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_ipq_update(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ipq*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_ipq_update(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ipq*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_ipq_update(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ipq*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_ipq_update(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ipq*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_kdb_check_backend(FnStats &st) {
    st.name = "stub_kdb_check_backend";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_kdb_check_backend(nullptr);
        int r_port = port::stub_kdb_check_backend(nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_kdb_check_backend(reinterpret_cast<kdb_dbbe*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_kdb_check_backend(reinterpret_cast<kdb_dbbe*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_kdb_check_backend(reinterpret_cast<kdb_dbbe*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_kdb_check_backend(reinterpret_cast<kdb_dbbe*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_kenv_check_dump(FnStats &st) {
    st.name = "stub_kenv_check_dump";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_kenv_check_dump(nullptr);
        int r_port = port::stub_kenv_check_dump(nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_kenv_check_dump(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_kenv_check_dump(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_kenv_check_dump(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_kenv_check_dump(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_kenv_check_get(FnStats &st) {
    st.name = "stub_kenv_check_get";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_kenv_check_get(nullptr, strs[0]);
        int r_port = port::stub_kenv_check_get(nullptr, strs[0]);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_kenv_check_get(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), strs[3]);
        int r_port = port::stub_kenv_check_get(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), strs[3]);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_kenv_check_get(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), strs[ri(0,7)]);
        int r_port = port::stub_kenv_check_get(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), strs[ri(0,7)]);
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_kenv_check_set(FnStats &st) {
    st.name = "stub_kenv_check_set";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_kenv_check_set(nullptr, strs[0], strs[0]);
        int r_port = port::stub_kenv_check_set(nullptr, strs[0], strs[0]);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_kenv_check_set(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), strs[3], strs[3]);
        int r_port = port::stub_kenv_check_set(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), strs[3], strs[3]);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_kenv_check_set(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), strs[ri(0,7)], strs[ri(0,7)]);
        int r_port = port::stub_kenv_check_set(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), strs[ri(0,7)], strs[ri(0,7)]);
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_kenv_check_unset(FnStats &st) {
    st.name = "stub_kenv_check_unset";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_kenv_check_unset(nullptr, strs[0]);
        int r_port = port::stub_kenv_check_unset(nullptr, strs[0]);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_kenv_check_unset(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), strs[3]);
        int r_port = port::stub_kenv_check_unset(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), strs[3]);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_kenv_check_unset(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), strs[ri(0,7)]);
        int r_port = port::stub_kenv_check_unset(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), strs[ri(0,7)]);
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_kld_check_load(FnStats &st) {
    st.name = "stub_kld_check_load";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_kld_check_load(nullptr, nullptr, nullptr);
        int r_port = port::stub_kld_check_load(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_kld_check_load(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_kld_check_load(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_kld_check_load(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_kld_check_load(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_kld_check_stat(FnStats &st) {
    st.name = "stub_kld_check_stat";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_kld_check_stat(nullptr);
        int r_port = port::stub_kld_check_stat(nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_kld_check_stat(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_kld_check_stat(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_kld_check_stat(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_kld_check_stat(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_mount_check_stat(FnStats &st) {
    st.name = "stub_mount_check_stat";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_mount_check_stat(nullptr, nullptr, nullptr);
        int r_port = port::stub_mount_check_stat(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_mount_check_stat(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_mount_check_stat(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_mount_check_stat(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_mount_check_stat(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_mount_create(FnStats &st) {
    st.name = "stub_mount_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_mount_create(nullptr, nullptr, nullptr);
        port::stub_mount_create(nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_mount_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_mount_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_mount_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_mount_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_netinet_arp_send(FnStats &st) {
    st.name = "stub_netinet_arp_send";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_netinet_arp_send(nullptr, nullptr, nullptr, nullptr);
        port::stub_netinet_arp_send(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_netinet_arp_send(reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_netinet_arp_send(reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_netinet_arp_send(reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_netinet_arp_send(reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_netinet_firewall_reply(FnStats &st) {
    st.name = "stub_netinet_firewall_reply";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_netinet_firewall_reply(nullptr, nullptr, nullptr, nullptr);
        port::stub_netinet_firewall_reply(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_netinet_firewall_reply(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_netinet_firewall_reply(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_netinet_firewall_reply(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_netinet_firewall_reply(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_netinet_firewall_send(FnStats &st) {
    st.name = "stub_netinet_firewall_send";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_netinet_firewall_send(nullptr, nullptr);
        port::stub_netinet_firewall_send(nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_netinet_firewall_send(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_netinet_firewall_send(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_netinet_firewall_send(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_netinet_firewall_send(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_netinet_fragment(FnStats &st) {
    st.name = "stub_netinet_fragment";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_netinet_fragment(nullptr, nullptr, nullptr, nullptr);
        port::stub_netinet_fragment(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_netinet_fragment(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_netinet_fragment(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_netinet_fragment(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_netinet_fragment(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_netinet_icmp_reply(FnStats &st) {
    st.name = "stub_netinet_icmp_reply";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_netinet_icmp_reply(nullptr, nullptr, nullptr, nullptr);
        port::stub_netinet_icmp_reply(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_netinet_icmp_reply(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_netinet_icmp_reply(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_netinet_icmp_reply(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_netinet_icmp_reply(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_netinet_icmp_replyinplace(FnStats &st) {
    st.name = "stub_netinet_icmp_replyinplace";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_netinet_icmp_replyinplace(nullptr, nullptr);
        port::stub_netinet_icmp_replyinplace(nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_netinet_icmp_replyinplace(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_netinet_icmp_replyinplace(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_netinet_icmp_replyinplace(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_netinet_icmp_replyinplace(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_netinet_igmp_send(FnStats &st) {
    st.name = "stub_netinet_igmp_send";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_netinet_igmp_send(nullptr, nullptr, nullptr, nullptr);
        port::stub_netinet_igmp_send(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_netinet_igmp_send(reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_netinet_igmp_send(reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_netinet_igmp_send(reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_netinet_igmp_send(reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_netinet_tcp_reply(FnStats &st) {
    st.name = "stub_netinet_tcp_reply";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_netinet_tcp_reply(nullptr, nullptr);
        port::stub_netinet_tcp_reply(nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_netinet_tcp_reply(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_netinet_tcp_reply(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_netinet_tcp_reply(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_netinet_tcp_reply(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_netinet6_nd6_send(FnStats &st) {
    st.name = "stub_netinet6_nd6_send";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_netinet6_nd6_send(nullptr, nullptr, nullptr, nullptr);
        port::stub_netinet6_nd6_send(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_netinet6_nd6_send(reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_netinet6_nd6_send(reinterpret_cast<ifnet*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_netinet6_nd6_send(reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_netinet6_nd6_send(reinterpret_cast<ifnet*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_pipe_check_ioctl(FnStats &st) {
    st.name = "stub_pipe_check_ioctl";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_pipe_check_ioctl(nullptr, nullptr, nullptr, 0UL, nullptr);
        int r_port = port::stub_pipe_check_ioctl(nullptr, nullptr, nullptr, 0UL, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_pipe_check_ioctl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<pipepair*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xffUL, reinterpret_cast<void*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_pipe_check_ioctl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<pipepair*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xffUL, reinterpret_cast<void*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_pipe_check_ioctl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<pipepair*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (unsigned long)ri(0, 0xffffff), reinterpret_cast<void*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_pipe_check_ioctl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<pipepair*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (unsigned long)ri(0, 0xffffff), reinterpret_cast<void*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_pipe_check_poll(FnStats &st) {
    st.name = "stub_pipe_check_poll";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_pipe_check_poll(nullptr, nullptr, nullptr);
        int r_port = port::stub_pipe_check_poll(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_pipe_check_poll(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<pipepair*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_pipe_check_poll(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<pipepair*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_pipe_check_poll(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<pipepair*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_pipe_check_poll(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<pipepair*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_pipe_check_read(FnStats &st) {
    st.name = "stub_pipe_check_read";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_pipe_check_read(nullptr, nullptr, nullptr);
        int r_port = port::stub_pipe_check_read(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_pipe_check_read(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<pipepair*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_pipe_check_read(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<pipepair*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_pipe_check_read(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<pipepair*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_pipe_check_read(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<pipepair*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_pipe_check_relabel(FnStats &st) {
    st.name = "stub_pipe_check_relabel";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_pipe_check_relabel(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_pipe_check_relabel(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_pipe_check_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<pipepair*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_pipe_check_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<pipepair*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_pipe_check_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<pipepair*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_pipe_check_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<pipepair*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_pipe_check_stat(FnStats &st) {
    st.name = "stub_pipe_check_stat";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_pipe_check_stat(nullptr, nullptr, nullptr);
        int r_port = port::stub_pipe_check_stat(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_pipe_check_stat(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<pipepair*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_pipe_check_stat(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<pipepair*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_pipe_check_stat(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<pipepair*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_pipe_check_stat(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<pipepair*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_pipe_check_write(FnStats &st) {
    st.name = "stub_pipe_check_write";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_pipe_check_write(nullptr, nullptr, nullptr);
        int r_port = port::stub_pipe_check_write(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_pipe_check_write(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<pipepair*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_pipe_check_write(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<pipepair*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_pipe_check_write(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<pipepair*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_pipe_check_write(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<pipepair*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_pipe_create(FnStats &st) {
    st.name = "stub_pipe_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_pipe_create(nullptr, nullptr, nullptr);
        port::stub_pipe_create(nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_pipe_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<pipepair*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_pipe_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<pipepair*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_pipe_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<pipepair*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_pipe_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<pipepair*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_pipe_relabel(FnStats &st) {
    st.name = "stub_pipe_relabel";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_pipe_relabel(nullptr, nullptr, nullptr, nullptr);
        port::stub_pipe_relabel(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_pipe_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<pipepair*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_pipe_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<pipepair*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_pipe_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<pipepair*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_pipe_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<pipepair*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_posixsem_check_getvalue(FnStats &st) {
    st.name = "stub_posixsem_check_getvalue";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixsem_check_getvalue(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_posixsem_check_getvalue(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixsem_check_getvalue(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_posixsem_check_getvalue(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixsem_check_getvalue(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_posixsem_check_getvalue(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixsem_check_open(FnStats &st) {
    st.name = "stub_posixsem_check_open";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixsem_check_open(nullptr, nullptr, nullptr);
        int r_port = port::stub_posixsem_check_open(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixsem_check_open(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_posixsem_check_open(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixsem_check_open(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_posixsem_check_open(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixsem_check_post(FnStats &st) {
    st.name = "stub_posixsem_check_post";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixsem_check_post(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_posixsem_check_post(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixsem_check_post(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_posixsem_check_post(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixsem_check_post(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_posixsem_check_post(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixsem_check_setmode(FnStats &st) {
    st.name = "stub_posixsem_check_setmode";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixsem_check_setmode(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_posixsem_check_setmode(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixsem_check_setmode(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff);
        int r_port = port::stub_posixsem_check_setmode(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixsem_check_setmode(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_posixsem_check_setmode(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixsem_check_setowner(FnStats &st) {
    st.name = "stub_posixsem_check_setowner";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixsem_check_setowner(nullptr, nullptr, nullptr, 0, 0);
        int r_port = port::stub_posixsem_check_setowner(nullptr, nullptr, nullptr, 0, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixsem_check_setowner(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff, 0xff);
        int r_port = port::stub_posixsem_check_setowner(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff, 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixsem_check_setowner(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_posixsem_check_setowner(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixsem_check_stat(FnStats &st) {
    st.name = "stub_posixsem_check_stat";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixsem_check_stat(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_posixsem_check_stat(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixsem_check_stat(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_posixsem_check_stat(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixsem_check_stat(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_posixsem_check_stat(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixsem_check_unlink(FnStats &st) {
    st.name = "stub_posixsem_check_unlink";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixsem_check_unlink(nullptr, nullptr, nullptr);
        int r_port = port::stub_posixsem_check_unlink(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixsem_check_unlink(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_posixsem_check_unlink(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixsem_check_unlink(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_posixsem_check_unlink(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixsem_check_wait(FnStats &st) {
    st.name = "stub_posixsem_check_wait";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixsem_check_wait(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_posixsem_check_wait(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixsem_check_wait(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_posixsem_check_wait(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixsem_check_wait(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_posixsem_check_wait(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixsem_create(FnStats &st) {
    st.name = "stub_posixsem_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_posixsem_create(nullptr, nullptr, nullptr);
        port::stub_posixsem_create(nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_posixsem_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_posixsem_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ksem*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_posixsem_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_posixsem_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ksem*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_posixshm_check_create(FnStats &st) {
    st.name = "stub_posixshm_check_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_create(nullptr, strs[0]);
        int r_port = port::stub_posixshm_check_create(nullptr, strs[0]);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), strs[3]);
        int r_port = port::stub_posixshm_check_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), strs[3]);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixshm_check_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), strs[ri(0,7)]);
        int r_port = port::stub_posixshm_check_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), strs[ri(0,7)]);
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixshm_check_mmap(FnStats &st) {
    st.name = "stub_posixshm_check_mmap";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_mmap(nullptr, nullptr, nullptr, 0, 0);
        int r_port = port::stub_posixshm_check_mmap(nullptr, nullptr, nullptr, 0, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_mmap(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1, -1);
        int r_port = port::stub_posixshm_check_mmap(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1, -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixshm_check_mmap(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_posixshm_check_mmap(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixshm_check_open(FnStats &st) {
    st.name = "stub_posixshm_check_open";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_open(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_posixshm_check_open(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_open(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff);
        int r_port = port::stub_posixshm_check_open(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixshm_check_open(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_posixshm_check_open(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixshm_check_read(FnStats &st) {
    st.name = "stub_posixshm_check_read";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_read(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_posixshm_check_read(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_read(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_posixshm_check_read(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixshm_check_read(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_posixshm_check_read(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixshm_check_setmode(FnStats &st) {
    st.name = "stub_posixshm_check_setmode";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_setmode(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_posixshm_check_setmode(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_setmode(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff);
        int r_port = port::stub_posixshm_check_setmode(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixshm_check_setmode(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_posixshm_check_setmode(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixshm_check_setowner(FnStats &st) {
    st.name = "stub_posixshm_check_setowner";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_setowner(nullptr, nullptr, nullptr, 0, 0);
        int r_port = port::stub_posixshm_check_setowner(nullptr, nullptr, nullptr, 0, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_setowner(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff, 0xff);
        int r_port = port::stub_posixshm_check_setowner(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff, 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixshm_check_setowner(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_posixshm_check_setowner(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixshm_check_stat(FnStats &st) {
    st.name = "stub_posixshm_check_stat";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_stat(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_posixshm_check_stat(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_stat(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_posixshm_check_stat(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixshm_check_stat(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_posixshm_check_stat(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixshm_check_truncate(FnStats &st) {
    st.name = "stub_posixshm_check_truncate";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_truncate(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_posixshm_check_truncate(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_truncate(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_posixshm_check_truncate(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixshm_check_truncate(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_posixshm_check_truncate(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixshm_check_unlink(FnStats &st) {
    st.name = "stub_posixshm_check_unlink";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_unlink(nullptr, nullptr, nullptr);
        int r_port = port::stub_posixshm_check_unlink(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_unlink(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_posixshm_check_unlink(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixshm_check_unlink(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_posixshm_check_unlink(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixshm_check_write(FnStats &st) {
    st.name = "stub_posixshm_check_write";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_write(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_posixshm_check_write(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_posixshm_check_write(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_posixshm_check_write(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_posixshm_check_write(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_posixshm_check_write(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_posixshm_create(FnStats &st) {
    st.name = "stub_posixshm_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_posixshm_create(nullptr, nullptr, nullptr);
        port::stub_posixshm_create(nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_posixshm_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_posixshm_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmfd*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_posixshm_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_posixshm_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmfd*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_priv_check(FnStats &st) {
    st.name = "stub_priv_check";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_priv_check(nullptr, 0);
        int r_port = port::stub_priv_check(nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_priv_check(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), -1);
        int r_port = port::stub_priv_check(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_priv_check(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_priv_check(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_priv_grant(FnStats &st) {
    st.name = "stub_priv_grant";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_priv_grant(nullptr, 0);
        int r_port = port::stub_priv_grant(nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_priv_grant(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), -1);
        int r_port = port::stub_priv_grant(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_priv_grant(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_priv_grant(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_proc_check_debug(FnStats &st) {
    st.name = "stub_proc_check_debug";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_proc_check_debug(nullptr, nullptr);
        int r_port = port::stub_proc_check_debug(nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_proc_check_debug(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<proc*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_proc_check_debug(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<proc*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_proc_check_debug(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<proc*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_proc_check_debug(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<proc*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_proc_check_sched(FnStats &st) {
    st.name = "stub_proc_check_sched";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_proc_check_sched(nullptr, nullptr);
        int r_port = port::stub_proc_check_sched(nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_proc_check_sched(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<proc*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_proc_check_sched(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<proc*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_proc_check_sched(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<proc*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_proc_check_sched(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<proc*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_proc_check_signal(FnStats &st) {
    st.name = "stub_proc_check_signal";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_proc_check_signal(nullptr, nullptr, 0);
        int r_port = port::stub_proc_check_signal(nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_proc_check_signal(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<proc*>((uintptr_t)0xdeadbeefUL), -1);
        int r_port = port::stub_proc_check_signal(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<proc*>((uintptr_t)0xdeadbeefUL), -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_proc_check_signal(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<proc*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_proc_check_signal(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<proc*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_proc_check_wait(FnStats &st) {
    st.name = "stub_proc_check_wait";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_proc_check_wait(nullptr, nullptr);
        int r_port = port::stub_proc_check_wait(nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_proc_check_wait(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<proc*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_proc_check_wait(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<proc*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_proc_check_wait(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<proc*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_proc_check_wait(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<proc*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_socket_check_accept(FnStats &st) {
    st.name = "stub_socket_check_accept";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_accept(nullptr, nullptr, nullptr);
        int r_port = port::stub_socket_check_accept(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_accept(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_socket_check_accept(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_socket_check_accept(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_socket_check_accept(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_socket_check_bind(FnStats &st) {
    st.name = "stub_socket_check_bind";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_bind(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_socket_check_bind(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_bind(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<sockaddr*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_socket_check_bind(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<sockaddr*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_socket_check_bind(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<sockaddr*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_socket_check_bind(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<sockaddr*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_socket_check_connect(FnStats &st) {
    st.name = "stub_socket_check_connect";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_connect(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_socket_check_connect(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_connect(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<sockaddr*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_socket_check_connect(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<sockaddr*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_socket_check_connect(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<sockaddr*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_socket_check_connect(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<sockaddr*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_socket_check_create(FnStats &st) {
    st.name = "stub_socket_check_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_create(nullptr, 0, 0, 0);
        int r_port = port::stub_socket_check_create(nullptr, 0, 0, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), -1, -1, -1);
        int r_port = port::stub_socket_check_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), -1, -1, -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_socket_check_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), ri(-0x7fffffff, 0x7fffffff), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_socket_check_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), ri(-0x7fffffff, 0x7fffffff), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_socket_check_deliver(FnStats &st) {
    st.name = "stub_socket_check_deliver";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_deliver(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_socket_check_deliver(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_deliver(reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_socket_check_deliver(reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_socket_check_deliver(reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_socket_check_deliver(reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_socket_check_listen(FnStats &st) {
    st.name = "stub_socket_check_listen";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_listen(nullptr, nullptr, nullptr);
        int r_port = port::stub_socket_check_listen(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_listen(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_socket_check_listen(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_socket_check_listen(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_socket_check_listen(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_socket_check_poll(FnStats &st) {
    st.name = "stub_socket_check_poll";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_poll(nullptr, nullptr, nullptr);
        int r_port = port::stub_socket_check_poll(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_poll(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_socket_check_poll(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_socket_check_poll(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_socket_check_poll(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_socket_check_receive(FnStats &st) {
    st.name = "stub_socket_check_receive";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_receive(nullptr, nullptr, nullptr);
        int r_port = port::stub_socket_check_receive(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_receive(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_socket_check_receive(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_socket_check_receive(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_socket_check_receive(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_socket_check_relabel(FnStats &st) {
    st.name = "stub_socket_check_relabel";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_relabel(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_socket_check_relabel(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_socket_check_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_socket_check_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_socket_check_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_socket_check_send(FnStats &st) {
    st.name = "stub_socket_check_send";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_send(nullptr, nullptr, nullptr);
        int r_port = port::stub_socket_check_send(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_send(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_socket_check_send(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_socket_check_send(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_socket_check_send(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_socket_check_stat(FnStats &st) {
    st.name = "stub_socket_check_stat";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_stat(nullptr, nullptr, nullptr);
        int r_port = port::stub_socket_check_stat(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_stat(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_socket_check_stat(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_socket_check_stat(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_socket_check_stat(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_inpcb_check_visible(FnStats &st) {
    st.name = "stub_inpcb_check_visible";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_inpcb_check_visible(nullptr, nullptr, nullptr);
        int r_port = port::stub_inpcb_check_visible(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_inpcb_check_visible(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<inpcb*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_inpcb_check_visible(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<inpcb*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_inpcb_check_visible(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<inpcb*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_inpcb_check_visible(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<inpcb*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_socket_check_visible(FnStats &st) {
    st.name = "stub_socket_check_visible";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_visible(nullptr, nullptr, nullptr);
        int r_port = port::stub_socket_check_visible(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_socket_check_visible(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_socket_check_visible(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_socket_check_visible(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_socket_check_visible(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_socket_create(FnStats &st) {
    st.name = "stub_socket_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_socket_create(nullptr, nullptr, nullptr);
        port::stub_socket_create(nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_socket_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_socket_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_socket_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_socket_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_socket_create_mbuf(FnStats &st) {
    st.name = "stub_socket_create_mbuf";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_socket_create_mbuf(nullptr, nullptr, nullptr, nullptr);
        port::stub_socket_create_mbuf(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_socket_create_mbuf(reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_socket_create_mbuf(reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_socket_create_mbuf(reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_socket_create_mbuf(reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_socket_newconn(FnStats &st) {
    st.name = "stub_socket_newconn";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_socket_newconn(nullptr, nullptr, nullptr, nullptr);
        port::stub_socket_newconn(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_socket_newconn(reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_socket_newconn(reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_socket_newconn(reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_socket_newconn(reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_socket_relabel(FnStats &st) {
    st.name = "stub_socket_relabel";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_socket_relabel(nullptr, nullptr, nullptr, nullptr);
        port::stub_socket_relabel(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_socket_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_socket_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_socket_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_socket_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_socketpeer_set_from_mbuf(FnStats &st) {
    st.name = "stub_socketpeer_set_from_mbuf";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_socketpeer_set_from_mbuf(nullptr, nullptr, nullptr, nullptr);
        port::stub_socketpeer_set_from_mbuf(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_socketpeer_set_from_mbuf(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_socketpeer_set_from_mbuf(reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_socketpeer_set_from_mbuf(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_socketpeer_set_from_mbuf(reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_socketpeer_set_from_socket(FnStats &st) {
    st.name = "stub_socketpeer_set_from_socket";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_socketpeer_set_from_socket(nullptr, nullptr, nullptr, nullptr);
        port::stub_socketpeer_set_from_socket(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_socketpeer_set_from_socket(reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_socketpeer_set_from_socket(reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<socket*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_socketpeer_set_from_socket(reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_socketpeer_set_from_socket(reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<socket*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_syncache_create(FnStats &st) {
    st.name = "stub_syncache_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_syncache_create(nullptr, nullptr);
        port::stub_syncache_create(nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_syncache_create(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<inpcb*>((uintptr_t)0xdeadbeefUL));
        port::stub_syncache_create(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<inpcb*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_syncache_create(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<inpcb*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_syncache_create(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<inpcb*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_syncache_create_mbuf(FnStats &st) {
    st.name = "stub_syncache_create_mbuf";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_syncache_create_mbuf(nullptr, nullptr, nullptr);
        port::stub_syncache_create_mbuf(nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_syncache_create_mbuf(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_syncache_create_mbuf(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mbuf*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_syncache_create_mbuf(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_syncache_create_mbuf(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mbuf*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_system_check_acct(FnStats &st) {
    st.name = "stub_system_check_acct";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_system_check_acct(nullptr, nullptr, nullptr);
        int r_port = port::stub_system_check_acct(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_system_check_acct(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_system_check_acct(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_system_check_acct(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_system_check_acct(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_system_check_audit(FnStats &st) {
    st.name = "stub_system_check_audit";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_system_check_audit(nullptr, nullptr, 0);
        int r_port = port::stub_system_check_audit(nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_system_check_audit(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<void*>((uintptr_t)0xdeadbeefUL), -1);
        int r_port = port::stub_system_check_audit(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<void*>((uintptr_t)0xdeadbeefUL), -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_system_check_audit(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<void*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_system_check_audit(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<void*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_system_check_auditctl(FnStats &st) {
    st.name = "stub_system_check_auditctl";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_system_check_auditctl(nullptr, nullptr, nullptr);
        int r_port = port::stub_system_check_auditctl(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_system_check_auditctl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_system_check_auditctl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_system_check_auditctl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_system_check_auditctl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_system_check_auditon(FnStats &st) {
    st.name = "stub_system_check_auditon";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_system_check_auditon(nullptr, 0);
        int r_port = port::stub_system_check_auditon(nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_system_check_auditon(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), -1);
        int r_port = port::stub_system_check_auditon(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_system_check_auditon(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_system_check_auditon(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_system_check_reboot(FnStats &st) {
    st.name = "stub_system_check_reboot";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_system_check_reboot(nullptr, 0);
        int r_port = port::stub_system_check_reboot(nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_system_check_reboot(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), -1);
        int r_port = port::stub_system_check_reboot(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_system_check_reboot(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_system_check_reboot(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_system_check_swapoff(FnStats &st) {
    st.name = "stub_system_check_swapoff";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_system_check_swapoff(nullptr, nullptr, nullptr);
        int r_port = port::stub_system_check_swapoff(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_system_check_swapoff(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_system_check_swapoff(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_system_check_swapoff(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_system_check_swapoff(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_system_check_swapon(FnStats &st) {
    st.name = "stub_system_check_swapon";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_system_check_swapon(nullptr, nullptr, nullptr);
        int r_port = port::stub_system_check_swapon(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_system_check_swapon(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_system_check_swapon(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_system_check_swapon(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_system_check_swapon(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_system_check_sysctl(FnStats &st) {
    st.name = "stub_system_check_sysctl";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_system_check_sysctl(nullptr, nullptr, nullptr, 0, nullptr);
        int r_port = port::stub_system_check_sysctl(nullptr, nullptr, nullptr, 0, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_system_check_sysctl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<sysctl_oid*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<void*>((uintptr_t)0xdeadbeefUL), -1, reinterpret_cast<sysctl_req*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_system_check_sysctl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<sysctl_oid*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<void*>((uintptr_t)0xdeadbeefUL), -1, reinterpret_cast<sysctl_req*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_system_check_sysctl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<sysctl_oid*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<void*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), reinterpret_cast<sysctl_req*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_system_check_sysctl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<sysctl_oid*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<void*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), reinterpret_cast<sysctl_req*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_sysvmsg_cleanup(FnStats &st) {
    st.name = "stub_sysvmsg_cleanup";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_sysvmsg_cleanup(nullptr);
        port::stub_sysvmsg_cleanup(nullptr);
    }
    ++st.cases;
    {
        ref_stub_sysvmsg_cleanup(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_sysvmsg_cleanup(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_sysvmsg_cleanup(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_sysvmsg_cleanup(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_sysvmsg_create(FnStats &st) {
    st.name = "stub_sysvmsg_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_sysvmsg_create(nullptr, nullptr, nullptr, nullptr, nullptr);
        port::stub_sysvmsg_create(nullptr, nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_sysvmsg_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msqid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msg*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_sysvmsg_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msqid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msg*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_sysvmsg_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msqid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msg*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_sysvmsg_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msqid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msg*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_sysvmsq_check_msgmsq(FnStats &st) {
    st.name = "stub_sysvmsq_check_msgmsq";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_sysvmsq_check_msgmsq(nullptr, nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_sysvmsq_check_msgmsq(nullptr, nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_sysvmsq_check_msgmsq(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msg*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msqid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_sysvmsq_check_msgmsq(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msg*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msqid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_sysvmsq_check_msgmsq(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msg*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msqid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_sysvmsq_check_msgmsq(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msg*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msqid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_sysvmsq_check_msgrcv(FnStats &st) {
    st.name = "stub_sysvmsq_check_msgrcv";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_sysvmsq_check_msgrcv(nullptr, nullptr, nullptr);
        int r_port = port::stub_sysvmsq_check_msgrcv(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_sysvmsq_check_msgrcv(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msg*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_sysvmsq_check_msgrcv(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msg*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_sysvmsq_check_msgrcv(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msg*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_sysvmsq_check_msgrcv(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msg*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_sysvmsq_check_msgrmid(FnStats &st) {
    st.name = "stub_sysvmsq_check_msgrmid";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_sysvmsq_check_msgrmid(nullptr, nullptr, nullptr);
        int r_port = port::stub_sysvmsq_check_msgrmid(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_sysvmsq_check_msgrmid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msg*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_sysvmsq_check_msgrmid(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msg*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_sysvmsq_check_msgrmid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msg*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_sysvmsq_check_msgrmid(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msg*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_sysvmsq_check_msqget(FnStats &st) {
    st.name = "stub_sysvmsq_check_msqget";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_sysvmsq_check_msqget(nullptr, nullptr, nullptr);
        int r_port = port::stub_sysvmsq_check_msqget(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_sysvmsq_check_msqget(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msqid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_sysvmsq_check_msqget(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msqid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_sysvmsq_check_msqget(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msqid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_sysvmsq_check_msqget(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msqid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_sysvmsq_check_msqsnd(FnStats &st) {
    st.name = "stub_sysvmsq_check_msqsnd";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_sysvmsq_check_msqsnd(nullptr, nullptr, nullptr);
        int r_port = port::stub_sysvmsq_check_msqsnd(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_sysvmsq_check_msqsnd(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msqid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_sysvmsq_check_msqsnd(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msqid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_sysvmsq_check_msqsnd(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msqid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_sysvmsq_check_msqsnd(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msqid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_sysvmsq_check_msqrcv(FnStats &st) {
    st.name = "stub_sysvmsq_check_msqrcv";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_sysvmsq_check_msqrcv(nullptr, nullptr, nullptr);
        int r_port = port::stub_sysvmsq_check_msqrcv(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_sysvmsq_check_msqrcv(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msqid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_sysvmsq_check_msqrcv(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msqid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_sysvmsq_check_msqrcv(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msqid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_sysvmsq_check_msqrcv(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msqid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_sysvmsq_check_msqctl(FnStats &st) {
    st.name = "stub_sysvmsq_check_msqctl";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_sysvmsq_check_msqctl(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_sysvmsq_check_msqctl(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_sysvmsq_check_msqctl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msqid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        int r_port = port::stub_sysvmsq_check_msqctl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msqid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_sysvmsq_check_msqctl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msqid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_sysvmsq_check_msqctl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msqid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_sysvmsq_cleanup(FnStats &st) {
    st.name = "stub_sysvmsq_cleanup";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_sysvmsq_cleanup(nullptr);
        port::stub_sysvmsq_cleanup(nullptr);
    }
    ++st.cases;
    {
        ref_stub_sysvmsq_cleanup(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_sysvmsq_cleanup(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_sysvmsq_cleanup(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_sysvmsq_cleanup(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_sysvmsq_create(FnStats &st) {
    st.name = "stub_sysvmsq_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_sysvmsq_create(nullptr, nullptr, nullptr);
        port::stub_sysvmsq_create(nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_sysvmsq_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msqid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_sysvmsq_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<msqid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_sysvmsq_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msqid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_sysvmsq_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<msqid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_sysvsem_check_semctl(FnStats &st) {
    st.name = "stub_sysvsem_check_semctl";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_sysvsem_check_semctl(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_sysvsem_check_semctl(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_sysvsem_check_semctl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<semid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        int r_port = port::stub_sysvsem_check_semctl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<semid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_sysvsem_check_semctl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<semid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_sysvsem_check_semctl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<semid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_sysvsem_check_semget(FnStats &st) {
    st.name = "stub_sysvsem_check_semget";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_sysvsem_check_semget(nullptr, nullptr, nullptr);
        int r_port = port::stub_sysvsem_check_semget(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_sysvsem_check_semget(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<semid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_sysvsem_check_semget(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<semid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_sysvsem_check_semget(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<semid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_sysvsem_check_semget(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<semid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_sysvsem_check_semop(FnStats &st) {
    st.name = "stub_sysvsem_check_semop";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_sysvsem_check_semop(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_sysvsem_check_semop(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_sysvsem_check_semop(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<semid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 255);
        int r_port = port::stub_sysvsem_check_semop(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<semid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 255);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_sysvsem_check_semop(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<semid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (size_t)ri(0, 4096));
        int r_port = port::stub_sysvsem_check_semop(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<semid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (size_t)ri(0, 4096));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_sysvsem_cleanup(FnStats &st) {
    st.name = "stub_sysvsem_cleanup";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_sysvsem_cleanup(nullptr);
        port::stub_sysvsem_cleanup(nullptr);
    }
    ++st.cases;
    {
        ref_stub_sysvsem_cleanup(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_sysvsem_cleanup(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_sysvsem_cleanup(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_sysvsem_cleanup(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_sysvsem_create(FnStats &st) {
    st.name = "stub_sysvsem_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_sysvsem_create(nullptr, nullptr, nullptr);
        port::stub_sysvsem_create(nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_sysvsem_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<semid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_sysvsem_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<semid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_sysvsem_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<semid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_sysvsem_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<semid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_sysvshm_check_shmat(FnStats &st) {
    st.name = "stub_sysvshm_check_shmat";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_sysvshm_check_shmat(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_sysvshm_check_shmat(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_sysvshm_check_shmat(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        int r_port = port::stub_sysvshm_check_shmat(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_sysvshm_check_shmat(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_sysvshm_check_shmat(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_sysvshm_check_shmctl(FnStats &st) {
    st.name = "stub_sysvshm_check_shmctl";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_sysvshm_check_shmctl(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_sysvshm_check_shmctl(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_sysvshm_check_shmctl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        int r_port = port::stub_sysvshm_check_shmctl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_sysvshm_check_shmctl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_sysvshm_check_shmctl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_sysvshm_check_shmdt(FnStats &st) {
    st.name = "stub_sysvshm_check_shmdt";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_sysvshm_check_shmdt(nullptr, nullptr, nullptr);
        int r_port = port::stub_sysvshm_check_shmdt(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_sysvshm_check_shmdt(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_sysvshm_check_shmdt(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_sysvshm_check_shmdt(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_sysvshm_check_shmdt(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_sysvshm_check_shmget(FnStats &st) {
    st.name = "stub_sysvshm_check_shmget";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_sysvshm_check_shmget(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_sysvshm_check_shmget(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_sysvshm_check_shmget(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        int r_port = port::stub_sysvshm_check_shmget(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_sysvshm_check_shmget(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_sysvshm_check_shmget(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_sysvshm_cleanup(FnStats &st) {
    st.name = "stub_sysvshm_cleanup";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_sysvshm_cleanup(nullptr);
        port::stub_sysvshm_cleanup(nullptr);
    }
    ++st.cases;
    {
        ref_stub_sysvshm_cleanup(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_sysvshm_cleanup(reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_sysvshm_cleanup(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_sysvshm_cleanup(reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_sysvshm_create(FnStats &st) {
    st.name = "stub_sysvshm_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_sysvshm_create(nullptr, nullptr, nullptr);
        port::stub_sysvshm_create(nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_sysvshm_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_sysvshm_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<shmid_kernel*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_sysvshm_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_sysvshm_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<shmid_kernel*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_thread_userret(FnStats &st) {
    st.name = "stub_thread_userret";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_thread_userret(nullptr);
        port::stub_thread_userret(nullptr);
    }
    ++st.cases;
    {
        ref_stub_thread_userret(reinterpret_cast<thread*>((uintptr_t)0xdeadbeefUL));
        port::stub_thread_userret(reinterpret_cast<thread*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_thread_userret(reinterpret_cast<thread*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_thread_userret(reinterpret_cast<thread*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_vnode_associate_extattr(FnStats &st) {
    st.name = "stub_vnode_associate_extattr";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_associate_extattr(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_associate_extattr(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_associate_extattr(reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_associate_extattr(reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_associate_extattr(reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_associate_extattr(reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_associate_singlelabel(FnStats &st) {
    st.name = "stub_vnode_associate_singlelabel";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_vnode_associate_singlelabel(nullptr, nullptr, nullptr, nullptr);
        port::stub_vnode_associate_singlelabel(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_vnode_associate_singlelabel(reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_vnode_associate_singlelabel(reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_vnode_associate_singlelabel(reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_vnode_associate_singlelabel(reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_vnode_check_access(FnStats &st) {
    st.name = "stub_vnode_check_access";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_access(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_vnode_check_access(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_access(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff);
        int r_port = port::stub_vnode_check_access(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_access(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_vnode_check_access(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_chdir(FnStats &st) {
    st.name = "stub_vnode_check_chdir";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_chdir(nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_check_chdir(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_chdir(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_chdir(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_chdir(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_chdir(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_chroot(FnStats &st) {
    st.name = "stub_vnode_check_chroot";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_chroot(nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_check_chroot(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_chroot(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_chroot(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_chroot(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_chroot(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_create(FnStats &st) {
    st.name = "stub_vnode_check_create";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_create(nullptr, nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_check_create(nullptr, nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<componentname*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vattr*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_create(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<componentname*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vattr*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<componentname*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vattr*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_create(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<componentname*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vattr*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_deleteacl(FnStats &st) {
    st.name = "stub_vnode_check_deleteacl";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_deleteacl(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_vnode_check_deleteacl(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_deleteacl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        int r_port = port::stub_vnode_check_deleteacl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_deleteacl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_vnode_check_deleteacl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_deleteextattr(FnStats &st) {
    st.name = "stub_vnode_check_deleteextattr";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_deleteextattr(nullptr, nullptr, nullptr, 0, strs[0]);
        int r_port = port::stub_vnode_check_deleteextattr(nullptr, nullptr, nullptr, 0, strs[0]);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_deleteextattr(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1, strs[3]);
        int r_port = port::stub_vnode_check_deleteextattr(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1, strs[3]);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_deleteextattr(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), strs[ri(0,7)]);
        int r_port = port::stub_vnode_check_deleteextattr(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), strs[ri(0,7)]);
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_exec(FnStats &st) {
    st.name = "stub_vnode_check_exec";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_exec(nullptr, nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_check_exec(nullptr, nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_exec(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<image_params*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_exec(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<image_params*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_exec(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<image_params*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_exec(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<image_params*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_getacl(FnStats &st) {
    st.name = "stub_vnode_check_getacl";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_getacl(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_vnode_check_getacl(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_getacl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        int r_port = port::stub_vnode_check_getacl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_getacl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_vnode_check_getacl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_getextattr(FnStats &st) {
    st.name = "stub_vnode_check_getextattr";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_getextattr(nullptr, nullptr, nullptr, 0, strs[0]);
        int r_port = port::stub_vnode_check_getextattr(nullptr, nullptr, nullptr, 0, strs[0]);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_getextattr(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1, strs[3]);
        int r_port = port::stub_vnode_check_getextattr(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1, strs[3]);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_getextattr(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), strs[ri(0,7)]);
        int r_port = port::stub_vnode_check_getextattr(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), strs[ri(0,7)]);
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_link(FnStats &st) {
    st.name = "stub_vnode_check_link";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_link(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_check_link(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_link(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<componentname*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_link(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<componentname*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_link(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<componentname*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_link(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<componentname*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_listextattr(FnStats &st) {
    st.name = "stub_vnode_check_listextattr";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_listextattr(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_vnode_check_listextattr(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_listextattr(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        int r_port = port::stub_vnode_check_listextattr(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_listextattr(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_vnode_check_listextattr(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_lookup(FnStats &st) {
    st.name = "stub_vnode_check_lookup";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_lookup(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_check_lookup(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_lookup(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<componentname*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_lookup(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<componentname*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_lookup(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<componentname*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_lookup(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<componentname*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_mmap(FnStats &st) {
    st.name = "stub_vnode_check_mmap";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_mmap(nullptr, nullptr, nullptr, 0, 0);
        int r_port = port::stub_vnode_check_mmap(nullptr, nullptr, nullptr, 0, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_mmap(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1, -1);
        int r_port = port::stub_vnode_check_mmap(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1, -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_mmap(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_vnode_check_mmap(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_mmap_downgrade(FnStats &st) {
    st.name = "stub_vnode_check_mmap_downgrade";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_vnode_check_mmap_downgrade(nullptr, nullptr, nullptr, &prot_bufs[0]);
        port::stub_vnode_check_mmap_downgrade(nullptr, nullptr, nullptr, &prot_bufs[0]);
    }
    ++st.cases;
    {
        ref_stub_vnode_check_mmap_downgrade(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), &prot_bufs[1]);
        port::stub_vnode_check_mmap_downgrade(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), &prot_bufs[1]);
    }
    {
        int pb_ref[4], pb_port[4], snap_ref[4], snap_port[4];
        memset(pb_ref, 0x7f, sizeof(pb_ref));
        memset(pb_port, 0x7f, sizeof(pb_port));
        memcpy(snap_ref, pb_ref, sizeof(pb_ref));
        memcpy(snap_port, pb_port, sizeof(pb_port));
        ++st.cases;
        ref_stub_vnode_check_mmap_downgrade(nullptr, nullptr, nullptr, &pb_ref[0]);
        port::stub_vnode_check_mmap_downgrade(nullptr, nullptr, nullptr, &pb_port[0]);
        if (!buf_unchanged(snap_ref, pb_ref, sizeof(pb_ref))) ++st.failures;
        if (!buf_unchanged(snap_port, pb_port, sizeof(pb_port))) ++st.failures;
        if (memcmp(pb_ref, pb_port, sizeof(pb_ref))) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_vnode_check_mmap_downgrade(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), &prot_bufs[ri(0,7)]);
        port::stub_vnode_check_mmap_downgrade(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), &prot_bufs[ri(0,7)]);
    }
}

static void test_stub_vnode_check_mprotect(FnStats &st) {
    st.name = "stub_vnode_check_mprotect";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_mprotect(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_vnode_check_mprotect(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_mprotect(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        int r_port = port::stub_vnode_check_mprotect(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_mprotect(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        int r_port = port::stub_vnode_check_mprotect(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_open(FnStats &st) {
    st.name = "stub_vnode_check_open";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_open(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_vnode_check_open(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_open(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff);
        int r_port = port::stub_vnode_check_open(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_open(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_vnode_check_open(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_poll(FnStats &st) {
    st.name = "stub_vnode_check_poll";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_poll(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_check_poll(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_poll(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_poll(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_poll(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_poll(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_read(FnStats &st) {
    st.name = "stub_vnode_check_read";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_read(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_check_read(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_read(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_read(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_read(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_read(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_readdir(FnStats &st) {
    st.name = "stub_vnode_check_readdir";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_readdir(nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_check_readdir(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_readdir(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_readdir(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_readdir(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_readdir(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_readlink(FnStats &st) {
    st.name = "stub_vnode_check_readlink";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_readlink(nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_check_readlink(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_readlink(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_readlink(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_readlink(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_readlink(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_relabel(FnStats &st) {
    st.name = "stub_vnode_check_relabel";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_relabel(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_check_relabel(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_rename_from(FnStats &st) {
    st.name = "stub_vnode_check_rename_from";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_rename_from(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_check_rename_from(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_rename_from(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<componentname*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_rename_from(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<componentname*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_rename_from(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<componentname*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_rename_from(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<componentname*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_rename_to(FnStats &st) {
    st.name = "stub_vnode_check_rename_to";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_rename_to(nullptr, nullptr, nullptr, nullptr, nullptr, 0, nullptr);
        int r_port = port::stub_vnode_check_rename_to(nullptr, nullptr, nullptr, nullptr, nullptr, 0, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_rename_to(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1, reinterpret_cast<componentname*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_rename_to(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1, reinterpret_cast<componentname*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_rename_to(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), reinterpret_cast<componentname*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_rename_to(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), reinterpret_cast<componentname*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_revoke(FnStats &st) {
    st.name = "stub_vnode_check_revoke";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_revoke(nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_check_revoke(nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_revoke(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_revoke(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_revoke(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_revoke(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_setacl(FnStats &st) {
    st.name = "stub_vnode_check_setacl";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_setacl(nullptr, nullptr, nullptr, 0, nullptr);
        int r_port = port::stub_vnode_check_setacl(nullptr, nullptr, nullptr, 0, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_setacl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1, reinterpret_cast<acl*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_setacl(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1, reinterpret_cast<acl*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_setacl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), reinterpret_cast<acl*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_setacl(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), reinterpret_cast<acl*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_setextattr(FnStats &st) {
    st.name = "stub_vnode_check_setextattr";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_setextattr(nullptr, nullptr, nullptr, 0, strs[0]);
        int r_port = port::stub_vnode_check_setextattr(nullptr, nullptr, nullptr, 0, strs[0]);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_setextattr(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1, strs[3]);
        int r_port = port::stub_vnode_check_setextattr(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), -1, strs[3]);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_setextattr(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), strs[ri(0,7)]);
        int r_port = port::stub_vnode_check_setextattr(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), ri(-0x7fffffff, 0x7fffffff), strs[ri(0,7)]);
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_setflags(FnStats &st) {
    st.name = "stub_vnode_check_setflags";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_setflags(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_vnode_check_setflags(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_setflags(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0);
        int r_port = port::stub_vnode_check_setflags(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_setflags(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), 0);
        int r_port = port::stub_vnode_check_setflags(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), 0);
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_setmode(FnStats &st) {
    st.name = "stub_vnode_check_setmode";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_setmode(nullptr, nullptr, nullptr, 0);
        int r_port = port::stub_vnode_check_setmode(nullptr, nullptr, nullptr, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_setmode(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff);
        int r_port = port::stub_vnode_check_setmode(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_setmode(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_vnode_check_setmode(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_setowner(FnStats &st) {
    st.name = "stub_vnode_check_setowner";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_setowner(nullptr, nullptr, nullptr, 0, 0);
        int r_port = port::stub_vnode_check_setowner(nullptr, nullptr, nullptr, 0, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_setowner(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff, 0xff);
        int r_port = port::stub_vnode_check_setowner(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0xff, 0xff);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_setowner(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535));
        int r_port = port::stub_vnode_check_setowner(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), (decltype((gid_t)0))ri(0, 65535), (decltype((gid_t)0))ri(0, 65535));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_setutimes(FnStats &st) {
    st.name = "stub_vnode_check_setutimes";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_setutimes(nullptr, nullptr, nullptr, 0, 0);
        int r_port = port::stub_vnode_check_setutimes(nullptr, nullptr, nullptr, 0, 0);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_setutimes(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0, 0);
        int r_port = port::stub_vnode_check_setutimes(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), 0, 0);
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_setutimes(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), 0, 0);
        int r_port = port::stub_vnode_check_setutimes(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), 0, 0);
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_stat(FnStats &st) {
    st.name = "stub_vnode_check_stat";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_stat(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_check_stat(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_stat(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_stat(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_stat(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_stat(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_unlink(FnStats &st) {
    st.name = "stub_vnode_check_unlink";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_unlink(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_check_unlink(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_unlink(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<componentname*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_unlink(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<componentname*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_unlink(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<componentname*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_unlink(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<componentname*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_check_write(FnStats &st) {
    st.name = "stub_vnode_check_write";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_write(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_check_write(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_check_write(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_check_write(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_check_write(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_check_write(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_create_extattr(FnStats &st) {
    st.name = "stub_vnode_create_extattr";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_create_extattr(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_create_extattr(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_create_extattr(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<componentname*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_create_extattr(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<mount*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<componentname*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_create_extattr(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<componentname*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_create_extattr(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<mount*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<componentname*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_execve_transition(FnStats &st) {
    st.name = "stub_vnode_execve_transition";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_vnode_execve_transition(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        port::stub_vnode_execve_transition(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_vnode_execve_transition(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<image_params*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_vnode_execve_transition(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<image_params*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_vnode_execve_transition(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<image_params*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_vnode_execve_transition(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<image_params*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_vnode_execve_will_transition(FnStats &st) {
    st.name = "stub_vnode_execve_will_transition";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_execve_will_transition(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_execve_will_transition(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_execve_will_transition(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<image_params*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_execve_will_transition(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<image_params*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_execve_will_transition(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<image_params*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_execve_will_transition(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<image_params*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

static void test_stub_vnode_relabel(FnStats &st) {
    st.name = "stub_vnode_relabel";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        ref_stub_vnode_relabel(nullptr, nullptr, nullptr, nullptr);
        port::stub_vnode_relabel(nullptr, nullptr, nullptr, nullptr);
    }
    ++st.cases;
    {
        ref_stub_vnode_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        port::stub_vnode_relabel(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        ref_stub_vnode_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        port::stub_vnode_relabel(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
    }
}

static void test_stub_vnode_setlabel_extattr(FnStats &st) {
    st.name = "stub_vnode_setlabel_extattr";
    st.cases = 0;
    st.failures = 0;
    init_test_data();
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_setlabel_extattr(nullptr, nullptr, nullptr, nullptr);
        int r_port = port::stub_vnode_setlabel_extattr(nullptr, nullptr, nullptr, nullptr);
        if (r_ref != r_port) ++st.failures;
    }
    ++st.cases;
    {
        int r_ref = ref_stub_vnode_setlabel_extattr(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        int r_port = port::stub_vnode_setlabel_extattr(reinterpret_cast<ucred*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<vnode*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL), reinterpret_cast<label*>((uintptr_t)0xdeadbeefUL));
        if (r_ref != r_port) ++st.failures;
    }
    for (int iter = 0; iter < 1000; ++iter) {
        init_test_data(); ++st.cases;
        int r_ref = ref_stub_vnode_setlabel_extattr(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        int r_port = port::stub_vnode_setlabel_extattr(reinterpret_cast<ucred*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<vnode*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))), reinterpret_cast<label*>((uintptr_t)(0x1000u * (1u + (xorshift32() & 0xffu)))));
        if (r_ref != r_port) ++st.failures;
    }
}

int main() {
    init_test_data();
    FnStats stats[] = {
        {"stub_destroy", 0, 0},
        {"stub_init", 0, 0},
        {"stub_syscall", 0, 0},
        {"stub_init_label", 0, 0},
        {"stub_init_label_waitcheck", 0, 0},
        {"stub_destroy_label", 0, 0},
        {"stub_copy_label", 0, 0},
        {"stub_externalize_label", 0, 0},
        {"stub_internalize_label", 0, 0},
        {"stub_bpfdesc_check_receive", 0, 0},
        {"stub_bpfdesc_create", 0, 0},
        {"stub_bpfdesc_create_mbuf", 0, 0},
        {"stub_cred_associate_nfsd", 0, 0},
        {"stub_cred_check_relabel", 0, 0},
        {"stub_cred_check_setaudit", 0, 0},
        {"stub_cred_check_setaudit_addr", 0, 0},
        {"stub_cred_check_setauid", 0, 0},
        {"stub_cred_setcred_enter", 0, 0},
        {"stub_cred_check_setcred", 0, 0},
        {"stub_cred_setcred_exit", 0, 0},
        {"stub_cred_check_setegid", 0, 0},
        {"stub_cred_check_seteuid", 0, 0},
        {"stub_cred_check_setgid", 0, 0},
        {"stub_cred_check_setgroups", 0, 0},
        {"stub_cred_check_setregid", 0, 0},
        {"stub_cred_check_setresgid", 0, 0},
        {"stub_cred_check_setresuid", 0, 0},
        {"stub_cred_check_setreuid", 0, 0},
        {"stub_cred_check_setuid", 0, 0},
        {"stub_cred_check_visible", 0, 0},
        {"stub_cred_create_init", 0, 0},
        {"stub_cred_create_swapper", 0, 0},
        {"stub_cred_relabel", 0, 0},
        {"stub_ddb_command_exec", 0, 0},
        {"stub_ddb_command_register", 0, 0},
        {"stub_devfs_create_device", 0, 0},
        {"stub_devfs_create_directory", 0, 0},
        {"stub_devfs_create_symlink", 0, 0},
        {"stub_devfs_update", 0, 0},
        {"stub_devfs_vnode_associate", 0, 0},
        {"stub_ifnet_check_relabel", 0, 0},
        {"stub_ifnet_check_transmit", 0, 0},
        {"stub_ifnet_create", 0, 0},
        {"stub_ifnet_create_mbuf", 0, 0},
        {"stub_ifnet_relabel", 0, 0},
        {"stub_inpcb_check_deliver", 0, 0},
        {"stub_inpcb_create", 0, 0},
        {"stub_inpcb_create_mbuf", 0, 0},
        {"stub_inpcb_sosetlabel", 0, 0},
        {"stub_ip6q_create", 0, 0},
        {"stub_ip6q_match", 0, 0},
        {"stub_ip6q_reassemble", 0, 0},
        {"stub_ip6q_update", 0, 0},
        {"stub_ipq_create", 0, 0},
        {"stub_ipq_match", 0, 0},
        {"stub_ipq_reassemble", 0, 0},
        {"stub_ipq_update", 0, 0},
        {"stub_kdb_check_backend", 0, 0},
        {"stub_kenv_check_dump", 0, 0},
        {"stub_kenv_check_get", 0, 0},
        {"stub_kenv_check_set", 0, 0},
        {"stub_kenv_check_unset", 0, 0},
        {"stub_kld_check_load", 0, 0},
        {"stub_kld_check_stat", 0, 0},
        {"stub_mount_check_stat", 0, 0},
        {"stub_mount_create", 0, 0},
        {"stub_netinet_arp_send", 0, 0},
        {"stub_netinet_firewall_reply", 0, 0},
        {"stub_netinet_firewall_send", 0, 0},
        {"stub_netinet_fragment", 0, 0},
        {"stub_netinet_icmp_reply", 0, 0},
        {"stub_netinet_icmp_replyinplace", 0, 0},
        {"stub_netinet_igmp_send", 0, 0},
        {"stub_netinet_tcp_reply", 0, 0},
        {"stub_netinet6_nd6_send", 0, 0},
        {"stub_pipe_check_ioctl", 0, 0},
        {"stub_pipe_check_poll", 0, 0},
        {"stub_pipe_check_read", 0, 0},
        {"stub_pipe_check_relabel", 0, 0},
        {"stub_pipe_check_stat", 0, 0},
        {"stub_pipe_check_write", 0, 0},
        {"stub_pipe_create", 0, 0},
        {"stub_pipe_relabel", 0, 0},
        {"stub_posixsem_check_getvalue", 0, 0},
        {"stub_posixsem_check_open", 0, 0},
        {"stub_posixsem_check_post", 0, 0},
        {"stub_posixsem_check_setmode", 0, 0},
        {"stub_posixsem_check_setowner", 0, 0},
        {"stub_posixsem_check_stat", 0, 0},
        {"stub_posixsem_check_unlink", 0, 0},
        {"stub_posixsem_check_wait", 0, 0},
        {"stub_posixsem_create", 0, 0},
        {"stub_posixshm_check_create", 0, 0},
        {"stub_posixshm_check_mmap", 0, 0},
        {"stub_posixshm_check_open", 0, 0},
        {"stub_posixshm_check_read", 0, 0},
        {"stub_posixshm_check_setmode", 0, 0},
        {"stub_posixshm_check_setowner", 0, 0},
        {"stub_posixshm_check_stat", 0, 0},
        {"stub_posixshm_check_truncate", 0, 0},
        {"stub_posixshm_check_unlink", 0, 0},
        {"stub_posixshm_check_write", 0, 0},
        {"stub_posixshm_create", 0, 0},
        {"stub_priv_check", 0, 0},
        {"stub_priv_grant", 0, 0},
        {"stub_proc_check_debug", 0, 0},
        {"stub_proc_check_sched", 0, 0},
        {"stub_proc_check_signal", 0, 0},
        {"stub_proc_check_wait", 0, 0},
        {"stub_socket_check_accept", 0, 0},
        {"stub_socket_check_bind", 0, 0},
        {"stub_socket_check_connect", 0, 0},
        {"stub_socket_check_create", 0, 0},
        {"stub_socket_check_deliver", 0, 0},
        {"stub_socket_check_listen", 0, 0},
        {"stub_socket_check_poll", 0, 0},
        {"stub_socket_check_receive", 0, 0},
        {"stub_socket_check_relabel", 0, 0},
        {"stub_socket_check_send", 0, 0},
        {"stub_socket_check_stat", 0, 0},
        {"stub_inpcb_check_visible", 0, 0},
        {"stub_socket_check_visible", 0, 0},
        {"stub_socket_create", 0, 0},
        {"stub_socket_create_mbuf", 0, 0},
        {"stub_socket_newconn", 0, 0},
        {"stub_socket_relabel", 0, 0},
        {"stub_socketpeer_set_from_mbuf", 0, 0},
        {"stub_socketpeer_set_from_socket", 0, 0},
        {"stub_syncache_create", 0, 0},
        {"stub_syncache_create_mbuf", 0, 0},
        {"stub_system_check_acct", 0, 0},
        {"stub_system_check_audit", 0, 0},
        {"stub_system_check_auditctl", 0, 0},
        {"stub_system_check_auditon", 0, 0},
        {"stub_system_check_reboot", 0, 0},
        {"stub_system_check_swapoff", 0, 0},
        {"stub_system_check_swapon", 0, 0},
        {"stub_system_check_sysctl", 0, 0},
        {"stub_sysvmsg_cleanup", 0, 0},
        {"stub_sysvmsg_create", 0, 0},
        {"stub_sysvmsq_check_msgmsq", 0, 0},
        {"stub_sysvmsq_check_msgrcv", 0, 0},
        {"stub_sysvmsq_check_msgrmid", 0, 0},
        {"stub_sysvmsq_check_msqget", 0, 0},
        {"stub_sysvmsq_check_msqsnd", 0, 0},
        {"stub_sysvmsq_check_msqrcv", 0, 0},
        {"stub_sysvmsq_check_msqctl", 0, 0},
        {"stub_sysvmsq_cleanup", 0, 0},
        {"stub_sysvmsq_create", 0, 0},
        {"stub_sysvsem_check_semctl", 0, 0},
        {"stub_sysvsem_check_semget", 0, 0},
        {"stub_sysvsem_check_semop", 0, 0},
        {"stub_sysvsem_cleanup", 0, 0},
        {"stub_sysvsem_create", 0, 0},
        {"stub_sysvshm_check_shmat", 0, 0},
        {"stub_sysvshm_check_shmctl", 0, 0},
        {"stub_sysvshm_check_shmdt", 0, 0},
        {"stub_sysvshm_check_shmget", 0, 0},
        {"stub_sysvshm_cleanup", 0, 0},
        {"stub_sysvshm_create", 0, 0},
        {"stub_thread_userret", 0, 0},
        {"stub_vnode_associate_extattr", 0, 0},
        {"stub_vnode_associate_singlelabel", 0, 0},
        {"stub_vnode_check_access", 0, 0},
        {"stub_vnode_check_chdir", 0, 0},
        {"stub_vnode_check_chroot", 0, 0},
        {"stub_vnode_check_create", 0, 0},
        {"stub_vnode_check_deleteacl", 0, 0},
        {"stub_vnode_check_deleteextattr", 0, 0},
        {"stub_vnode_check_exec", 0, 0},
        {"stub_vnode_check_getacl", 0, 0},
        {"stub_vnode_check_getextattr", 0, 0},
        {"stub_vnode_check_link", 0, 0},
        {"stub_vnode_check_listextattr", 0, 0},
        {"stub_vnode_check_lookup", 0, 0},
        {"stub_vnode_check_mmap", 0, 0},
        {"stub_vnode_check_mmap_downgrade", 0, 0},
        {"stub_vnode_check_mprotect", 0, 0},
        {"stub_vnode_check_open", 0, 0},
        {"stub_vnode_check_poll", 0, 0},
        {"stub_vnode_check_read", 0, 0},
        {"stub_vnode_check_readdir", 0, 0},
        {"stub_vnode_check_readlink", 0, 0},
        {"stub_vnode_check_relabel", 0, 0},
        {"stub_vnode_check_rename_from", 0, 0},
        {"stub_vnode_check_rename_to", 0, 0},
        {"stub_vnode_check_revoke", 0, 0},
        {"stub_vnode_check_setacl", 0, 0},
        {"stub_vnode_check_setextattr", 0, 0},
        {"stub_vnode_check_setflags", 0, 0},
        {"stub_vnode_check_setmode", 0, 0},
        {"stub_vnode_check_setowner", 0, 0},
        {"stub_vnode_check_setutimes", 0, 0},
        {"stub_vnode_check_stat", 0, 0},
        {"stub_vnode_check_unlink", 0, 0},
        {"stub_vnode_check_write", 0, 0},
        {"stub_vnode_create_extattr", 0, 0},
        {"stub_vnode_execve_transition", 0, 0},
        {"stub_vnode_execve_will_transition", 0, 0},
        {"stub_vnode_relabel", 0, 0},
        {"stub_vnode_setlabel_extattr", 0, 0},
    };
    const int nfn = sizeof(stats)/sizeof(stats[0]);
    test_stub_destroy(stats[0]);
    test_stub_init(stats[1]);
    test_stub_syscall(stats[2]);
    test_stub_init_label(stats[3]);
    test_stub_init_label_waitcheck(stats[4]);
    test_stub_destroy_label(stats[5]);
    test_stub_copy_label(stats[6]);
    test_stub_externalize_label(stats[7]);
    test_stub_internalize_label(stats[8]);
    test_stub_bpfdesc_check_receive(stats[9]);
    test_stub_bpfdesc_create(stats[10]);
    test_stub_bpfdesc_create_mbuf(stats[11]);
    test_stub_cred_associate_nfsd(stats[12]);
    test_stub_cred_check_relabel(stats[13]);
    test_stub_cred_check_setaudit(stats[14]);
    test_stub_cred_check_setaudit_addr(stats[15]);
    test_stub_cred_check_setauid(stats[16]);
    test_stub_cred_setcred_enter(stats[17]);
    test_stub_cred_check_setcred(stats[18]);
    test_stub_cred_setcred_exit(stats[19]);
    test_stub_cred_check_setegid(stats[20]);
    test_stub_cred_check_seteuid(stats[21]);
    test_stub_cred_check_setgid(stats[22]);
    test_stub_cred_check_setgroups(stats[23]);
    test_stub_cred_check_setregid(stats[24]);
    test_stub_cred_check_setresgid(stats[25]);
    test_stub_cred_check_setresuid(stats[26]);
    test_stub_cred_check_setreuid(stats[27]);
    test_stub_cred_check_setuid(stats[28]);
    test_stub_cred_check_visible(stats[29]);
    test_stub_cred_create_init(stats[30]);
    test_stub_cred_create_swapper(stats[31]);
    test_stub_cred_relabel(stats[32]);
    test_stub_ddb_command_exec(stats[33]);
    test_stub_ddb_command_register(stats[34]);
    test_stub_devfs_create_device(stats[35]);
    test_stub_devfs_create_directory(stats[36]);
    test_stub_devfs_create_symlink(stats[37]);
    test_stub_devfs_update(stats[38]);
    test_stub_devfs_vnode_associate(stats[39]);
    test_stub_ifnet_check_relabel(stats[40]);
    test_stub_ifnet_check_transmit(stats[41]);
    test_stub_ifnet_create(stats[42]);
    test_stub_ifnet_create_mbuf(stats[43]);
    test_stub_ifnet_relabel(stats[44]);
    test_stub_inpcb_check_deliver(stats[45]);
    test_stub_inpcb_create(stats[46]);
    test_stub_inpcb_create_mbuf(stats[47]);
    test_stub_inpcb_sosetlabel(stats[48]);
    test_stub_ip6q_create(stats[49]);
    test_stub_ip6q_match(stats[50]);
    test_stub_ip6q_reassemble(stats[51]);
    test_stub_ip6q_update(stats[52]);
    test_stub_ipq_create(stats[53]);
    test_stub_ipq_match(stats[54]);
    test_stub_ipq_reassemble(stats[55]);
    test_stub_ipq_update(stats[56]);
    test_stub_kdb_check_backend(stats[57]);
    test_stub_kenv_check_dump(stats[58]);
    test_stub_kenv_check_get(stats[59]);
    test_stub_kenv_check_set(stats[60]);
    test_stub_kenv_check_unset(stats[61]);
    test_stub_kld_check_load(stats[62]);
    test_stub_kld_check_stat(stats[63]);
    test_stub_mount_check_stat(stats[64]);
    test_stub_mount_create(stats[65]);
    test_stub_netinet_arp_send(stats[66]);
    test_stub_netinet_firewall_reply(stats[67]);
    test_stub_netinet_firewall_send(stats[68]);
    test_stub_netinet_fragment(stats[69]);
    test_stub_netinet_icmp_reply(stats[70]);
    test_stub_netinet_icmp_replyinplace(stats[71]);
    test_stub_netinet_igmp_send(stats[72]);
    test_stub_netinet_tcp_reply(stats[73]);
    test_stub_netinet6_nd6_send(stats[74]);
    test_stub_pipe_check_ioctl(stats[75]);
    test_stub_pipe_check_poll(stats[76]);
    test_stub_pipe_check_read(stats[77]);
    test_stub_pipe_check_relabel(stats[78]);
    test_stub_pipe_check_stat(stats[79]);
    test_stub_pipe_check_write(stats[80]);
    test_stub_pipe_create(stats[81]);
    test_stub_pipe_relabel(stats[82]);
    test_stub_posixsem_check_getvalue(stats[83]);
    test_stub_posixsem_check_open(stats[84]);
    test_stub_posixsem_check_post(stats[85]);
    test_stub_posixsem_check_setmode(stats[86]);
    test_stub_posixsem_check_setowner(stats[87]);
    test_stub_posixsem_check_stat(stats[88]);
    test_stub_posixsem_check_unlink(stats[89]);
    test_stub_posixsem_check_wait(stats[90]);
    test_stub_posixsem_create(stats[91]);
    test_stub_posixshm_check_create(stats[92]);
    test_stub_posixshm_check_mmap(stats[93]);
    test_stub_posixshm_check_open(stats[94]);
    test_stub_posixshm_check_read(stats[95]);
    test_stub_posixshm_check_setmode(stats[96]);
    test_stub_posixshm_check_setowner(stats[97]);
    test_stub_posixshm_check_stat(stats[98]);
    test_stub_posixshm_check_truncate(stats[99]);
    test_stub_posixshm_check_unlink(stats[100]);
    test_stub_posixshm_check_write(stats[101]);
    test_stub_posixshm_create(stats[102]);
    test_stub_priv_check(stats[103]);
    test_stub_priv_grant(stats[104]);
    test_stub_proc_check_debug(stats[105]);
    test_stub_proc_check_sched(stats[106]);
    test_stub_proc_check_signal(stats[107]);
    test_stub_proc_check_wait(stats[108]);
    test_stub_socket_check_accept(stats[109]);
    test_stub_socket_check_bind(stats[110]);
    test_stub_socket_check_connect(stats[111]);
    test_stub_socket_check_create(stats[112]);
    test_stub_socket_check_deliver(stats[113]);
    test_stub_socket_check_listen(stats[114]);
    test_stub_socket_check_poll(stats[115]);
    test_stub_socket_check_receive(stats[116]);
    test_stub_socket_check_relabel(stats[117]);
    test_stub_socket_check_send(stats[118]);
    test_stub_socket_check_stat(stats[119]);
    test_stub_inpcb_check_visible(stats[120]);
    test_stub_socket_check_visible(stats[121]);
    test_stub_socket_create(stats[122]);
    test_stub_socket_create_mbuf(stats[123]);
    test_stub_socket_newconn(stats[124]);
    test_stub_socket_relabel(stats[125]);
    test_stub_socketpeer_set_from_mbuf(stats[126]);
    test_stub_socketpeer_set_from_socket(stats[127]);
    test_stub_syncache_create(stats[128]);
    test_stub_syncache_create_mbuf(stats[129]);
    test_stub_system_check_acct(stats[130]);
    test_stub_system_check_audit(stats[131]);
    test_stub_system_check_auditctl(stats[132]);
    test_stub_system_check_auditon(stats[133]);
    test_stub_system_check_reboot(stats[134]);
    test_stub_system_check_swapoff(stats[135]);
    test_stub_system_check_swapon(stats[136]);
    test_stub_system_check_sysctl(stats[137]);
    test_stub_sysvmsg_cleanup(stats[138]);
    test_stub_sysvmsg_create(stats[139]);
    test_stub_sysvmsq_check_msgmsq(stats[140]);
    test_stub_sysvmsq_check_msgrcv(stats[141]);
    test_stub_sysvmsq_check_msgrmid(stats[142]);
    test_stub_sysvmsq_check_msqget(stats[143]);
    test_stub_sysvmsq_check_msqsnd(stats[144]);
    test_stub_sysvmsq_check_msqrcv(stats[145]);
    test_stub_sysvmsq_check_msqctl(stats[146]);
    test_stub_sysvmsq_cleanup(stats[147]);
    test_stub_sysvmsq_create(stats[148]);
    test_stub_sysvsem_check_semctl(stats[149]);
    test_stub_sysvsem_check_semget(stats[150]);
    test_stub_sysvsem_check_semop(stats[151]);
    test_stub_sysvsem_cleanup(stats[152]);
    test_stub_sysvsem_create(stats[153]);
    test_stub_sysvshm_check_shmat(stats[154]);
    test_stub_sysvshm_check_shmctl(stats[155]);
    test_stub_sysvshm_check_shmdt(stats[156]);
    test_stub_sysvshm_check_shmget(stats[157]);
    test_stub_sysvshm_cleanup(stats[158]);
    test_stub_sysvshm_create(stats[159]);
    test_stub_thread_userret(stats[160]);
    test_stub_vnode_associate_extattr(stats[161]);
    test_stub_vnode_associate_singlelabel(stats[162]);
    test_stub_vnode_check_access(stats[163]);
    test_stub_vnode_check_chdir(stats[164]);
    test_stub_vnode_check_chroot(stats[165]);
    test_stub_vnode_check_create(stats[166]);
    test_stub_vnode_check_deleteacl(stats[167]);
    test_stub_vnode_check_deleteextattr(stats[168]);
    test_stub_vnode_check_exec(stats[169]);
    test_stub_vnode_check_getacl(stats[170]);
    test_stub_vnode_check_getextattr(stats[171]);
    test_stub_vnode_check_link(stats[172]);
    test_stub_vnode_check_listextattr(stats[173]);
    test_stub_vnode_check_lookup(stats[174]);
    test_stub_vnode_check_mmap(stats[175]);
    test_stub_vnode_check_mmap_downgrade(stats[176]);
    test_stub_vnode_check_mprotect(stats[177]);
    test_stub_vnode_check_open(stats[178]);
    test_stub_vnode_check_poll(stats[179]);
    test_stub_vnode_check_read(stats[180]);
    test_stub_vnode_check_readdir(stats[181]);
    test_stub_vnode_check_readlink(stats[182]);
    test_stub_vnode_check_relabel(stats[183]);
    test_stub_vnode_check_rename_from(stats[184]);
    test_stub_vnode_check_rename_to(stats[185]);
    test_stub_vnode_check_revoke(stats[186]);
    test_stub_vnode_check_setacl(stats[187]);
    test_stub_vnode_check_setextattr(stats[188]);
    test_stub_vnode_check_setflags(stats[189]);
    test_stub_vnode_check_setmode(stats[190]);
    test_stub_vnode_check_setowner(stats[191]);
    test_stub_vnode_check_setutimes(stats[192]);
    test_stub_vnode_check_stat(stats[193]);
    test_stub_vnode_check_unlink(stats[194]);
    test_stub_vnode_check_write(stats[195]);
    test_stub_vnode_create_extattr(stats[196]);
    test_stub_vnode_execve_transition(stats[197]);
    test_stub_vnode_execve_will_transition(stats[198]);
    test_stub_vnode_relabel(stats[199]);
    test_stub_vnode_setlabel_extattr(stats[200]);
    long total_cases = 0, total_fail = 0;
    printf("%-45s %10s %10s\n", "function", "cases", "failures");
    printf("%-45s %10s %10s\n", "--------", "-----", "--------");
    for (int i = 0; i < nfn; ++i) {
        printf("%-45s %10ld %10ld\n", stats[i].name, stats[i].cases, stats[i].failures);
        total_cases += stats[i].cases;
        total_fail += stats[i].failures;
    }
    printf("%-45s %10ld %10ld\n", "TOTAL", total_cases, total_fail);
    return total_fail == 0 ? 0 : 1;
}
