# PBSD todo.md Pass Report

Generated: `2026-07-28T07:35:07+00:00`

- Files processed: **4027**
- Edits applied: **100172**
- Refusals (model queue only): **0**
- compile_commands coverage: **100.0%**
- Corpus OK: **True**
- Corpus IR equal: **2** / ran **4**
- IR equal: **1** / ran **25**
- Diff equal: **3**

## Reason histogram (true refusals — not successful edits)

| Reason | Count |
|---|---:|

## Top edited files

| Edits | Refusals | File |
|---:|---:|---|
| 750 | 0 | `hbsd/src/libexec/rtld-elf/rtld.c` |
| 597 | 0 | `hbsd/src/sbin/camcontrol/camcontrol.c` |
| 508 | 0 | `hbsd/src/lib/libprocstat/libprocstat.c` |
| 481 | 0 | `hbsd/src/usr.sbin/cxgbetool/tcbinfot6.c` |
| 464 | 0 | `hbsd/src/usr.sbin/bsnmpd/modules/snmp_wlan/wlan_snmp.c` |
| 460 | 0 | `hbsd/src/sbin/ifconfig/ifieee80211.c` |
| 459 | 0 | `hbsd/src/usr.sbin/cxgbetool/tcbinfot5.c` |
| 457 | 0 | `hbsd/src/usr.sbin/ppp/command.c` |
| 442 | 0 | `hbsd/src/usr.sbin/cxgbetool/tcbinfot4.c` |
| 433 | 0 | `hbsd/src/sbin/ipfw/ipfw2.c` |
| 424 | 0 | `hbsd/src/usr.sbin/mountd/mountd.c` |
| 392 | 0 | `hbsd/src/usr.sbin/ctladm/ctladm.c` |
| 376 | 0 | `hbsd/src/usr.bin/find/function.c` |
| 353 | 0 | `hbsd/src/sbin/pfctl/pfctl.c` |
| 328 | 0 | `hbsd/src/lib/libpfctl/libpfctl.c` |
| 318 | 0 | `hbsd/src/lib/libc/gen/getpwent.c` |
| 288 | 0 | `hbsd/src/usr.sbin/syslogd/syslogd.c` |
| 275 | 0 | `hbsd/src/lib/libcasper/services/cap_net/tests/net_test.c` |
| 274 | 0 | `hbsd/src/usr.sbin/route6d/route6d.c` |
| 267 | 0 | `hbsd/src/usr.sbin/bsnmpd/tools/libbsnmptools/bsnmptools.c` |
| 259 | 0 | `hbsd/src/lib/libc/net/getaddrinfo.c` |
| 239 | 0 | `hbsd/src/lib/libc/gen/getgrent.c` |
| 237 | 0 | `hbsd/src/bin/ps/keyword.c` |
| 237 | 0 | `hbsd/src/usr.sbin/camdd/camdd.c` |
| 236 | 0 | `hbsd/src/usr.sbin/bhyve/pci_nvme.c` |
| 233 | 0 | `hbsd/src/usr.sbin/pw/pw_user.c` |
| 232 | 0 | `hbsd/src/sbin/dhclient/dhclient.c` |
| 227 | 0 | `hbsd/src/lib/libefivar/efivar-dp-parse.c` |
| 223 | 0 | `hbsd/src/lib/libcasper/services/cap_net/cap_net.c` |
| 218 | 0 | `hbsd/src/usr.sbin/bhyve/pci_emul.c` |

## Outputs

- Staged C++: `/home/odin/pbsd/docs/migration/clang_port/staged`
- Refusals: `/home/odin/pbsd/docs/migration/clang_port/refusals.jsonl`
- Full JSON: `docs/migration/clang_port/pass_report.json`

## Proposal histogram (`proposals.jsonl`, 25954)

| Kind | Count |
|---|---:|
| `MACRO_OBJECT` | 3221 |
| `STR_FORMAT_CANDIDATE` | 3066 |
| `MALLOC_ESCAPE` | 2577 |
| `SPAN_CANDIDATE` | 2334 |
| `NULLABILITY` | 2124 |
| `LIFETIME_OWN` | 1608 |
| `ANDERSEN_OWNED` | 1600 |
| `LIFETIME_FAIL` | 1435 |
| `GLOBAL_CLUSTER` | 1227 |
| `POINTER_KIND` | 1160 |
| `MACRO_CONSTEXPR` | 999 |
| `ANDERSEN_ESCAPE` | 964 |
| `DEAD_STORE` | 634 |
| `MACRO_DIVERGENT` | 631 |
| `SYSCALL_TRACE` | 610 |
| `PURITY` | 454 |
| `KR_DEFINITION` | 247 |
| `VLA` | 234 |
| `GOTO_CLEANUP_CANDIDATE` | 233 |
| `QUEUE_H_SITE` | 163 |
| `TYPEDEF_COMPLEX` | 128 |
| `CALL_SITE_SPAN` | 83 |
| `RANGE_FOR_CANDIDATE` | 82 |
| `CALLBACK_CTX` | 47 |
| `GOTO_CROSS_INIT` | 29 |
| `LOCK_DISCIPLINE` | 18 |
| `BIT_CAST_CANDIDATE` | 16 |
| `FN_PTR_STRUCT` | 9 |
| `FLEXIBLE_ARRAY` | 8 |
| `ENUM_ARITHMETIC` | 7 |
| `NESTED_STRUCT_TAG` | 5 |
| `GENERIC` | 1 |

