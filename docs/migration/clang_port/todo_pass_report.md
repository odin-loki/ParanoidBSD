# PBSD todo.md Pass Report

Generated: `2026-07-27T11:07:44+00:00`

- Files processed: **300**
- Edits applied: **13718**
- Refusals (model queue only): **0**
- compile_commands coverage: **100.0%**
- Corpus OK: **True**
- IR equal: **0** / ran **0**
- Diff equal: **0**

## Reason histogram (true refusals — not successful edits)

| Reason | Count |
|---|---:|

## Top edited files

| Edits | Refusals | File |
|---:|---:|---|
| 777 | 0 | `hbsd/src/sys/cam/ctl/ctl.c` |
| 515 | 0 | `hbsd/src/sys/cam/scsi/scsi_all.c` |
| 466 | 0 | `hbsd/src/sys/cam/cam_xpt.c` |
| 437 | 0 | `hbsd/src/sys/amd64/amd64/pmap.c` |
| 437 | 0 | `hbsd/src/sys/arm64/arm64/pmap.c` |
| 355 | 0 | `hbsd/src/sys/cam/ctl/ctl_cmd_table.c` |
| 291 | 0 | `hbsd/src/sys/cam/scsi/scsi_enc_ses.c` |
| 264 | 0 | `hbsd/src/sys/cam/scsi/scsi_sa.c` |
| 226 | 0 | `hbsd/src/sys/arm64/vmm/io/vgic_v3.c` |
| 218 | 0 | `hbsd/src/sys/cam/ctl/ctl_backend_block.c` |
| 196 | 0 | `hbsd/src/sys/cam/scsi/scsi_da.c` |
| 176 | 0 | `hbsd/src/sys/cam/ctl/ctl_frontend_iscsi.c` |
| 160 | 0 | `hbsd/src/sys/cam/scsi/scsi_xpt.c` |
| 158 | 0 | `hbsd/src/sys/amd64/vmm/intel/vmx.c` |
| 153 | 0 | `hbsd/src/sys/cam/ctl/ctl_tpc.c` |
| 151 | 0 | `hbsd/src/sys/cam/mmc/mmc_da.c` |
| 147 | 0 | `hbsd/src/sys/cam/cam_iosched.c` |
| 143 | 0 | `hbsd/src/sys/amd64/vmm/vmm.c` |
| 137 | 0 | `hbsd/src/sys/arm64/nvidia/tegra210/tegra210_clk_per.c` |
| 136 | 0 | `hbsd/src/sys/amd64/vmm/amd/svm.c` |
| 133 | 0 | `hbsd/src/sys/arm64/nvidia/tegra210/tegra210_xusbpadctl.c` |
| 132 | 0 | `hbsd/src/sys/amd64/vmm/vmm_instruction_emul.c` |
| 132 | 0 | `hbsd/src/sys/cam/scsi/scsi_cd.c` |
| 127 | 0 | `hbsd/src/sys/cam/ata/ata_da.c` |
| 125 | 0 | `hbsd/src/sys/arm64/arm64/gicv3_its.c` |
| 125 | 0 | `hbsd/src/sys/cam/cam_periph.c` |
| 123 | 0 | `hbsd/src/sys/cam/ata/ata_xpt.c` |
| 119 | 0 | `hbsd/src/sys/cam/scsi/scsi_pass.c` |
| 117 | 0 | `hbsd/src/sys/arm64/iommu/smmu.c` |
| 108 | 0 | `hbsd/src/sys/arm64/broadcom/genet/if_genet.c` |

## Outputs

- Staged C++: `/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/docs/migration/clang_port/staged`
- Refusals: `/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/docs/migration/clang_port/refusals.jsonl`
- Full JSON: `docs/migration/clang_port/pass_report.json`

## Proposal histogram (`proposals.jsonl`, 37527)

| Kind | Count |
|---|---:|
| `MACRO_OBJECT` | 9990 |
| `STR_FORMAT_CANDIDATE` | 3915 |
| `MALLOC_ESCAPE` | 3080 |
| `SPAN_CANDIDATE` | 2825 |
| `NULLABILITY` | 2583 |
| `LIFETIME_OWN` | 1802 |
| `LIFETIME_FAIL` | 1780 |
| `ANDERSEN_OWNED` | 1769 |
| `MACRO_CONSTEXPR` | 1691 |
| `GLOBAL_CLUSTER` | 1518 |
| `POINTER_KIND` | 1271 |
| `ANDERSEN_ESCAPE` | 1121 |
| `MACRO_DIVERGENT` | 924 |
| `DEAD_STORE` | 747 |
| `SYSCALL_TRACE` | 657 |
| `PURITY` | 505 |
| `GOTO_CLEANUP_CANDIDATE` | 257 |
| `VLA` | 247 |
| `QUEUE_H_SITE` | 178 |
| `TYPEDEF_COMPLEX` | 162 |
| `LOCK_DISCIPLINE` | 149 |
| `CALL_SITE_SPAN` | 97 |
| `RANGE_FOR_CANDIDATE` | 96 |
| `CALLBACK_CTX` | 58 |
| `GOTO_CROSS_INIT` | 30 |
| `FLEXIBLE_ARRAY` | 23 |
| `BIT_CAST_CANDIDATE` | 16 |
| `KR_DEFINITION` | 12 |
| `FN_PTR_STRUCT` | 11 |
| `ENUM_ARITHMETIC` | 7 |
| `NESTED_STRUCT_TAG` | 5 |
| `GENERIC` | 1 |

