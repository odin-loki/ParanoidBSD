# PBSD todo.md Pass Report

Generated: `2026-09-05T01:32:16+00:00`

- Files processed: **4162**
- Edits applied: **106728**
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
| 750 | 0 | `hbsd/src/libexec/rtld-elf/rtld.c` |
| 597 | 0 | `hbsd/src/sbin/camcontrol/camcontrol.c` |
| 508 | 0 | `hbsd/src/lib/libprocstat/libprocstat.c` |
| 481 | 0 | `hbsd/src/usr.sbin/cxgbetool/tcbinfot6.c` |
| 464 | 0 | `hbsd/src/usr.sbin/bsnmpd/modules/snmp_wlan/wlan_snmp.c` |
| 460 | 0 | `hbsd/src/sbin/ifconfig/ifieee80211.c` |
| 459 | 0 | `hbsd/src/usr.sbin/cxgbetool/tcbinfot5.c` |
| 458 | 0 | `hbsd/src/usr.sbin/ppp/command.c` |
| 442 | 0 | `hbsd/src/usr.sbin/cxgbetool/tcbinfot4.c` |
| 433 | 0 | `hbsd/src/sbin/ipfw/ipfw2.c` |
| 424 | 0 | `hbsd/src/usr.sbin/mountd/mountd.c` |
| 392 | 0 | `hbsd/src/usr.sbin/ctladm/ctladm.c` |
| 385 | 0 | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_parser.c` |
| 376 | 0 | `hbsd/src/usr.bin/find/function.c` |
| 352 | 0 | `hbsd/src/sbin/pfctl/pfctl.c` |
| 339 | 0 | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_cc.c` |
| 328 | 0 | `hbsd/src/lib/libpfctl/libpfctl.c` |
| 319 | 0 | `hbsd/src/lib/libc/gen/getpwent.c` |
| 319 | 0 | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_consume.c` |
| 288 | 0 | `hbsd/src/usr.sbin/syslogd/syslogd.c` |
| 274 | 0 | `hbsd/src/usr.sbin/route6d/route6d.c` |
| 274 | 0 | `hbsd/src/lib/libcasper/services/cap_net/tests/net_test.c` |
| 267 | 0 | `hbsd/src/usr.sbin/bsnmpd/tools/libbsnmptools/bsnmptools.c` |
| 263 | 0 | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_printf.c` |
| 259 | 0 | `hbsd/src/lib/libc/net/getaddrinfo.c` |
| 256 | 0 | `hbsd/src/cddl/contrib/opensolaris/tools/ctf/cvt/dwarf.c` |
| 241 | 0 | `hbsd/src/lib/libc/gen/getgrent.c` |
| 239 | 0 | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_module.c` |
| 237 | 0 | `hbsd/src/bin/ps/keyword.c` |
| 237 | 0 | `hbsd/src/usr.sbin/camdd/camdd.c` |

## Outputs

- Staged C++: `/home/user/paranoidbsd/docs/migration/clang_port/staged`
- Refusals: `/home/user/paranoidbsd/docs/migration/clang_port/refusals.jsonl`
- Full JSON: `docs/migration/clang_port/pass_report.json`

## Proposal histogram (`proposals.jsonl`, 26642)

| Kind | Count |
|---|---:|
| `MACRO_OBJECT` | 3272 |
| `STR_FORMAT_CANDIDATE` | 3169 |
| `MALLOC_ESCAPE` | 2647 |
| `SPAN_CANDIDATE` | 2449 |
| `NULLABILITY` | 2164 |
| `LIFETIME_OWN` | 1646 |
| `ANDERSEN_OWNED` | 1637 |
| `LIFETIME_FAIL` | 1485 |
| `GLOBAL_CLUSTER` | 1252 |
| `POINTER_KIND` | 1187 |
| `MACRO_CONSTEXPR` | 1014 |
| `ANDERSEN_ESCAPE` | 986 |
| `DEAD_STORE` | 642 |
| `MACRO_DIVERGENT` | 635 |
| `SYSCALL_TRACE` | 626 |
| `PURITY` | 487 |
| `KR_DEFINITION` | 247 |
| `VLA` | 239 |
| `GOTO_CLEANUP_CANDIDATE` | 235 |
| `QUEUE_H_SITE` | 163 |
| `TYPEDEF_COMPLEX` | 138 |
| `CALL_SITE_SPAN` | 83 |
| `RANGE_FOR_CANDIDATE` | 82 |
| `CALLBACK_CTX` | 60 |
| `GOTO_CROSS_INIT` | 29 |
| `LOCK_DISCIPLINE` | 18 |
| `BIT_CAST_CANDIDATE` | 16 |
| `FN_PTR_STRUCT` | 12 |
| `FLEXIBLE_ARRAY` | 8 |
| `ENUM_ARITHMETIC` | 7 |
| `NESTED_STRUCT_TAG` | 6 |
| `GENERIC` | 1 |

