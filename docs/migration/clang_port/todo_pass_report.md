# PBSD todo.md Pass Report

Generated: `2026-09-05T02:13:24+00:00`

- Files processed: **10464**
- Edits applied: **467960**
- Refusals (model queue only): **0**
- compile_commands coverage: **100.0%**
- Corpus OK: **True**
- Corpus IR equal: **4** / ran **4**
- IR equal: **0** / ran **0**
- Diff equal: **0**

## Reason histogram (true refusals — not successful edits)

| Reason | Count |
|---|---:|

## Top edited files

| Edits | Refusals | File |
|---:|---:|---|
| 1274 | 0 | `hbsd/src/sys/fs/nfsclient/nfs_clrpcops.c` |
| 1193 | 0 | `hbsd/src/sys/cddl/contrib/opensolaris/uts/common/dtrace/dtrace.c` |
| 1098 | 0 | `hbsd/src/sys/ufs/ffs/ffs_softdep.c` |
| 1073 | 0 | `hbsd/src/sys/dev/bxe/bxe_elink.c` |
| 1061 | 0 | `hbsd/src/sys/fs/nfsserver/nfs_nfsdport.c` |
| 1030 | 0 | `hbsd/src/sys/netinet/sctp_output.c` |
| 1002 | 0 | `hbsd/src/sys/netinet/tcp_stacks/rack.c` |
| 926 | 0 | `hbsd/src/sys/contrib/openzfs/cmd/zpool/zpool_main.c` |
| 912 | 0 | `hbsd/src/sys/dev/ocs_fc/ocs_hw.c` |
| 853 | 0 | `hbsd/src/sys/dev/aic7xxx/aic79xx_reg_print.c` |
| 825 | 0 | `hbsd/src/sys/contrib/openzfs/cmd/zdb/zdb.c` |
| 824 | 0 | `hbsd/src/sys/netinet/sctp_usrreq.c` |
| 822 | 0 | `hbsd/src/sys/dev/bxe/bxe.c` |
| 814 | 0 | `hbsd/src/sys/netpfil/pf/pf.c` |
| 809 | 0 | `hbsd/src/sys/netinet/sctputil.c` |
| 803 | 0 | `hbsd/src/sys/fs/nfsserver/nfs_nfsdstate.c` |
| 784 | 0 | `hbsd/src/sys/fs/nfsclient/nfs_clstate.c` |
| 777 | 0 | `hbsd/src/sys/cam/ctl/ctl.c` |
| 768 | 0 | `hbsd/src/sys/contrib/openzfs/module/zfs/spa.c` |
| 764 | 0 | `hbsd/src/sys/netpfil/ipfilter/netinet/fil.c` |
| 752 | 0 | `hbsd/src/sys/contrib/openzfs/cmd/zfs/zfs_main.c` |
| 750 | 0 | `hbsd/src/libexec/rtld-elf/rtld.c` |
| 734 | 0 | `hbsd/src/sys/security/mac_test/mac_test.c` |
| 694 | 0 | `hbsd/src/sys/netipsec/key.c` |
| 675 | 0 | `hbsd/src/sys/contrib/dev/athk/ath12k/mac.c` |
| 670 | 0 | `hbsd/src/sys/contrib/openzfs/module/zfs/zfs_ioctl.c` |
| 658 | 0 | `hbsd/src/sys/dev/cxgbe/t4_main.c` |
| 657 | 0 | `hbsd/src/sys/compat/linuxkpi/common/src/linux_80211.c` |
| 635 | 0 | `hbsd/src/sys/security/mac_stub/mac_stub.c` |
| 633 | 0 | `hbsd/src/sys/security/mac_biba/mac_biba.c` |

## Outputs

- Staged C++: `/home/user/paranoidbsd/docs/migration/clang_port/staged`
- Refusals: `/home/user/paranoidbsd/docs/migration/clang_port/refusals.jsonl`
- Full JSON: `docs/migration/clang_port/pass_report.json`

## Proposal histogram (`proposals.jsonl`, 93800)

| Kind | Count |
|---|---:|
| `NULLABILITY` | 14105 |
| `SPAN_CANDIDATE` | 12836 |
| `MACRO_OBJECT` | 11508 |
| `LIFETIME_FAIL` | 7587 |
| `MALLOC_ESCAPE` | 7048 |
| `STR_FORMAT_CANDIDATE` | 5064 |
| `MACRO_CONSTEXPR` | 4449 |
| `POINTER_KIND` | 3933 |
| `GLOBAL_CLUSTER` | 3601 |
| `LOCK_DISCIPLINE` | 3366 |
| `LIFETIME_OWN` | 2956 |
| `MACRO_DIVERGENT` | 2716 |
| `ANDERSEN_OWNED` | 2667 |
| `ANDERSEN_ESCAPE` | 2284 |
| `PURITY` | 2188 |
| `KR_DEFINITION` | 1852 |
| `DEAD_STORE` | 1846 |
| `SYSCALL_TRACE` | 765 |
| `GOTO_CLEANUP_CANDIDATE` | 646 |
| `VLA` | 493 |
| `RANGE_FOR_CANDIDATE` | 448 |
| `TYPEDEF_COMPLEX` | 414 |
| `QUEUE_H_SITE` | 302 |
| `CALLBACK_CTX` | 266 |
| `CALL_SITE_SPAN` | 262 |
| `FLEXIBLE_ARRAY` | 53 |
| `FN_PTR_STRUCT` | 46 |
| `BIT_CAST_CANDIDATE` | 32 |
| `GOTO_CROSS_INIT` | 29 |
| `NESTED_STRUCT_TAG` | 21 |
| `ENUM_ARITHMETIC` | 13 |
| `GENERIC` | 4 |

