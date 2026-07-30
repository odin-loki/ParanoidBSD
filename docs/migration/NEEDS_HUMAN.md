# PBSD — files needing human decisions

Generated 2026-07-30 14:41 UTC. 4204 files could not be converted automatically.

These are grouped by blocking idiom. Each group is usually **one design decision** that then unblocks every file in it — that is the highest-leverage work left.

## AUTO_FAILED — 3761 files

- `hbsd/src/bin/cat/cat.c` — agent failed: exit 1 after 10s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/chflags/chflags.c` — agent failed: exit 1 after 10s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/chio/chio.c` — agent failed: exit 1 after 9s: ActionRequiredError: Rate limit exceeded. You've reached the rate limit. Please
- `hbsd/src/bin/chmod/chmod.c` — agent failed: exit 1 after 10s: ActionRequiredError: Rate limit exceeded. You've reached the rate limit. Pleas
- `hbsd/src/bin/cp/cp.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/cp/utils.c` — agent failed: exit 1 after 12s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/cpuset/cpuset.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/csh/iconv_stub.c` — agent failed: exit 1 after 20s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/date/date.c` — agent failed: exit 1 after 11s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/date/vary.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/dd/args.c` — agent failed: exit 1 after 10s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/dd/conv_tab.c` — agent failed: exit 1 after 10s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/dd/dd.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/dd/gen.c` — agent failed: exit 1 after 13s: ActionRequiredError: Rate limit exceeded. You've reached the rate limit. Pleas
- `hbsd/src/bin/dd/misc.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/df/df.c` — agent failed: exit 1 after 7s: ActionRequiredError: Rate limit exceeded. You've reached the rate limit. Please
- `hbsd/src/bin/echo/echo.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/ed/buf.c` — agent failed: exit 1 after 10s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/ed/glbl.c` — agent failed: exit 1 after 9s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/ed/io.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/ed/main.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/ed/re.c` — agent failed: exit 1 after 10s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/ed/sub.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/ed/undo.c` — agent failed: exit 1 after 9s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/getfacl/getfacl.c` — agent failed: exit 1 after 10s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/kenv/kenv.c` — agent failed: exit 1 after 9s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/kill/kill.c` — agent failed: exit 1 after 12s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/ln/ln.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/ls/cmp.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/ls/ls.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/ls/print.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/ls/util.c` — agent failed: exit 1 after 10s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/mkdir/mkdir.c` — agent failed: exit 1 after 10s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/mv/mv.c` — agent failed: exit 1 after 9s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/nproc/nproc.c` — agent failed: exit 1 after 9s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/pax/ar_io.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/pax/ar_subs.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/pax/buf_subs.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/bin/pax/cache.c` — agent failed: exit 1 after 9s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/pax/cpio.c` — agent failed: exit 1 after 12s: ActionRequiredError: Rate limit exceeded. You've reached the rate limit. Pleas
- …and 3721 more

## QUEUE_MACRO — 336 files

- `hbsd/src/bin/pkill/pkill.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/ps/ps.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/bin/pwait/pwait.c` — agent failed: exit 1 after 15s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/libc/nls/msgcat.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/libcuse/cuse_lib.c` — agent failed: exit 1 after 16s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/libthr/thread/thr_mutex.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/lib/libthread_db/thread_db.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sbin/bsdlabel/bsdlabel.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sbin/fsck_msdosfs/fat.c` — agent failed: exit 1 after 10s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sbin/geom/core/geom.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sbin/ggate/ggated/ggated.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sbin/ggate/shared/ggate.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sbin/hastctl/hastctl.c` — agent failed: exit 1 after 15s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sbin/hastd/hastd.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sbin/hastd/primary.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sbin/kldconfig/kldconfig.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sbin/mdconfig/mdconfig.c` — agent failed: exit 1 after 6s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sbin/pfctl/pfctl.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sbin/pfctl/pfctl_parser.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sbin/recoverdisk/recoverdisk.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sbin/routed/if.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sbin/routed/input.c` — agent failed: exit 1 after 9s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sbin/routed/parms.c` — agent failed: exit 1 after 9s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sbin/routed/rdisc.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sbin/routed/table.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/amd64/pt/pt.c` — agent failed: exit 1 after 6s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/cam/cam_iosched.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/cam/cam_periph.c` — agent failed: exit 1 after 12s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/cam/cam_xpt.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/cam/ctl/ctl.c` — agent failed: exit 1 after 6s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/cam/ctl/ctl_backend_block.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/cam/ctl/ctl_backend_ramdisk.c` — agent failed: exit 1 after 7s: ActionRequiredError: Rate limit exceeded. You've reached the rate limit. Please
- `hbsd/src/sys/cam/ctl/ctl_frontend_iscsi.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/cam/ctl/ctl_tpc.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/cam/ctl/scsi_ctl.c` — agent failed: exit 1 after 7s: ActionRequiredError: Rate limit exceeded. You've reached the rate limit. Please
- `hbsd/src/sys/cam/scsi/scsi_cd.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/cam/scsi/scsi_ch.c` — agent failed: exit 1 after 12s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/cam/scsi/scsi_da.c` — agent failed: exit 1 after 6s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/cam/scsi/scsi_enc.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/cam/scsi/scsi_enc_safte.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- …and 296 more

## LINKER_SET — 146 files

- `hbsd/src/lib/libthread_db/libpthread_db.c` — agent failed: exit 1 after 15s: ActionRequiredError: Rate limit exceeded. You've reached the rate limit. Pleas
- `hbsd/src/lib/libthread_db/libthr_db.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/amd64/pt/pt.c` — agent failed: exit 1 after 6s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/cam/cam_xpt.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/cam/ctl/ctl.c` — agent failed: exit 1 after 6s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/cam/ctl/ctl_frontend_iscsi.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/cam/scsi/scsi_all.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/compat/lindebugfs/lindebugfs.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/compat/linprocfs/linprocfs.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/compat/linsysfs/linsysfs.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/compat/linuxkpi/common/src/linux_80211.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/compat/linuxkpi/common/src/linux_compat.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/compat/linuxkpi/common/src/linux_rcu.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/compat/x86bios/x86bios.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/fs/cuse/cuse.c` — agent failed: exit 1 after 6s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/fs/fuse/fuse_main.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/geom/eli/g_eli.c` — agent failed: exit 1 after 12s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/geom/gate/g_gate.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/geom/geom_event.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/geom/journal/g_journal_ufs.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/geom/raid/g_raid.c` — agent failed: exit 1 after 11s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/geom/uzip/g_uzip.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/init_main.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_descrip.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_devctl.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_environment.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_event.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_exec.c` — agent failed: exit 1 after 12s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/kern/kern_exit.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_intr.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_jailmeta.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/kern/kern_kcov.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_linker.c` — agent failed: exit 1 after 6s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_lockf.c` — agent failed: exit 1 after 6s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_module.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/kern/kern_pmc.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_poll.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_racct.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_rctl.c` — agent failed: exit 1 after 12s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/kern/kern_sig.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- …and 106 more

## HEAVY_CPP — 48 files

- `hbsd/src/sbin/ipf/ipfstat/ipfstat.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/cam/ctl/ctl.c` — agent failed: exit 1 after 6s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/cam/ctl/ctl_frontend_iscsi.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/cam/scsi/scsi_all.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/compat/linuxkpi/common/src/linux_80211.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/kern/kern_descrip.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_exec.c` — agent failed: exit 1 after 12s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/kern/kern_jail.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_prot.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_rwlock.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_sig.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/kern_sx.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/link_elf.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/kern/sched_ule.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/uipc_socket.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/vfs_syscalls.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/net/altq/altq_rmclass.c` — agent failed: exit 1 after 12s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/net/bpf.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/net/if.c` — agent failed: exit 1 after 9s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/net/if_ipsec.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/net/if_vlan.c` — agent failed: exit 1 after 11s: [31m✗ Failed to reach the Cursor API. If you are behind a corporate proxy, se
- `hbsd/src/sys/net/iflib.c` — agent failed: exit 1 after 21s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/net/rtsock.c` — agent failed: exit 1 after 22s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/netinet/in_pcb.c` — agent failed: exit 1 after 12s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/netinet/ip_carp.c` — agent failed: exit 1 after 24s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/netinet/sctp_asconf.c` — agent failed: exit 1 after 7s: ActionRequiredError: Rate limit exceeded. You've reached the rate limit. Please
- `hbsd/src/sys/netinet/sctp_indata.c` — agent failed: exit 1 after 12s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/netinet/sctp_output.c` — agent failed: exit 1 after 16s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/netinet/sctp_pcb.c` — agent failed: exit 1 after 10s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/netinet/sctp_usrreq.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/netinet/sctputil.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/netinet/tcp_input.c` — agent failed: exit 1 after 11s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/netinet/tcp_stacks/bbr.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/netinet/tcp_stacks/rack.c` — agent failed: exit 1 after 12s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/netinet/tcp_subr.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/netinet/tcp_syncache.c` — agent failed: exit 1 after 16s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/netinet/tcp_usrreq.c` — agent failed: exit 1 after 22s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/netpfil/ipfw/ip_fw2.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/netpfil/ipfw/ip_fw_dynamic.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/netpfil/ipfw/ip_fw_table_algo.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- …and 8 more

## NO_FUNCTIONS — 43 files

- `hbsd/src/lib/libc/aarch64/string/memset_resolver.c` — agent failed: exit 1 after 12s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/libc/amd64/gen/fpgetmask.c` — agent failed: exit 1 after 9s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/lib/libc/amd64/gen/fpgetprec.c` — agent failed: exit 1 after 16s: ActionRequiredError: Rate limit exceeded. You've reached the rate limit. Pleas
- `hbsd/src/lib/libc/amd64/gen/fpgetround.c` — agent failed: exit 1 after 11s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/libc/amd64/gen/fpgetsticky.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/lib/libc/amd64/gen/fpsetmask.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/libc/amd64/gen/fpsetprec.c` — agent failed: exit 1 after 14s: ActionRequiredError: Rate limit exceeded. You've reached the rate limit. Pleas
- `hbsd/src/lib/libc/amd64/gen/fpsetround.c` — agent failed: exit 1 after 16s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/libc/amd64/gen/infinity.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/lib/libc/gen/_rand48.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/libc/locale/c16rtomb_iconv.c` — agent failed: exit 1 after 6s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/lib/libc/locale/c32rtomb_iconv.c` — agent failed: exit 1 after 9s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/lib/libc/locale/mbrtoc16_iconv.c` — agent failed: exit 1 after 11s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/libc/locale/mbrtoc32_iconv.c` — agent failed: exit 1 after 15s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/libc/net/sockatmark.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/libc/softfloat/bits32/softfloat.c` — agent failed: exit 1 after 6s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/lib/libc/softfloat/bits64/softfloat.c` — agent failed: exit 1 after 15s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/libc_nonshared/__stub.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/lib/msun/src/s_signgam.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sbin/ipf/ipfs/ipfs.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sbin/natd/icmp.c` — agent failed: exit 1 after 14s: ActionRequiredError: Rate limit exceeded. You've reached the rate limit. Pleas
- `hbsd/src/sbin/umbctl/umbctl.c` — agent failed: exit 1 after 15s: ActionRequiredError: Rate limit exceeded. You've reached the rate limit. Pleas
- `hbsd/src/sys/cam/ctl/ctl_cmd_table.c` — agent failed: exit 1 after 7s: ActionRequiredError: Rate limit exceeded. You've reached the rate limit. Please
- `hbsd/src/sys/compat/linuxkpi/common/src/linux_aperture.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/compat/linuxkpi/common/src/linux_siphash.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/genoffset.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/kern/imgact_elf32.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/kern/imgact_elf64.c` — agent failed: exit 1 after 15s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/kern/init_sysent.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/security/mac_none/mac_none.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/security/mac_veriexec/mac_veriexec_sha256.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/sys/security/mac_veriexec/mac_veriexec_sha384.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/security/mac_veriexec/mac_veriexec_sha512.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/usr.bin/lex/initparse.c` — agent failed: exit 1 after 11s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/usr.bin/lex/initskel.c` — agent failed: exit 1 after 13s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/usr.sbin/cxgbetool/reg_defs_t4vf.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/usr.sbin/cxgbetool/tcbshowt4.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/usr.sbin/cxgbetool/tcbshowt5.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/usr.sbin/cxgbetool/tcbshowt6.c` — agent failed: exit 1 after 15s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/usr.sbin/lpr/filters.ru/koi2855/koi2855.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- …and 3 more

## INCLUDE_SHIM — 7 files

- `hbsd/src/lib/msun/src/s_llrint.c` — agent failed: exit 1 after 15s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/msun/src/s_llrintf.c` — agent failed: exit 1 after 15s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/msun/src/s_llrintl.c` — agent failed: exit 1 after 6s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/lib/msun/src/s_lrintf.c` — agent failed: exit 1 after 10s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/msun/src/s_lrintl.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/msun/src/s_lroundf.c` — agent failed: exit 1 after 14s: ActionRequiredError: Increase limits for faster responses You're out of usage.
- `hbsd/src/lib/msun/src/s_lroundl.c` — agent failed: exit 1 after 6s: ActionRequiredError: Increase limits for faster responses You're out of usage. 

## VLA — 5 files

- `hbsd/src/sys/cam/scsi/scsi_enc_ses.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/subr_stats.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/vfs_cache.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/netinet/libalias/alias_sctp.c` — agent failed: exit 1 after 6s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/vm/vm_pageout.c` — agent failed: exit 1 after 7s: ActionRequiredError: Rate limit exceeded. You've reached the rate limit. Please

## FLEX_ARRAY — 4 files

- `hbsd/src/sys/kern/kern_jail.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/subr_stats.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/sys/kern/vfs_cache.c` — agent failed: exit 1 after 8s: ActionRequiredError: Increase limits for faster responses You're out of usage. 
- `hbsd/src/usr.bin/script/script.c` — agent failed: exit 1 after 7s: ActionRequiredError: Increase limits for faster responses You're out of usage. 

## GENERIC — 1 files

- `hbsd/src/sys/kern/kern_tc.c` — agent failed: exit 1 after 11s: ActionRequiredError: Increase limits for faster responses You're out of usage.
