/* Dual-link C ABI — kernel shims callable from hbsd KPI / kmod glue. */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long long pbsd_cap_rights_t;
typedef unsigned long long pbsd_lineage_id_t;

enum pbsd_status {
    PBSD_STATUS_OK       = 0,
    PBSD_STATUS_INVALID  = 1,
    PBSD_STATUS_DENIED   = 2,
    PBSD_STATUS_NOMEM    = 4,
};

int  pbsd_kernel_shim_init(void);
void pbsd_kernel_shim_fini(void);

pbsd_cap_rights_t pbsd_kernel_encode_rights(unsigned read, unsigned write,
                                            unsigned grant, unsigned exec);

int pbsd_kernel_cap_check(unsigned long long have_w0, unsigned long long have_w1,
                          unsigned long long need_w0, unsigned long long need_w1);
int pbsd_kernel_cap_rights_limit(unsigned long long have_w0, unsigned long long have_w1,
                                 unsigned long long limit_w0, unsigned long long limit_w1);
unsigned pbsd_kernel_cap_rights_to_vmprot(unsigned long long w0, unsigned long long w1);
int pbsd_kernel_revoke_lineage(pbsd_lineage_id_t id);
int pbsd_kernel_validate_wx(unsigned prot);
unsigned pbsd_kernel_merge_entry_offset(unsigned current_prot, unsigned max_prot);
unsigned pbsd_kernel_prot_max_extract(unsigned encoded);
int pbsd_kernel_pax_mprotect_enforce(unsigned old_prot, unsigned new_prot,
                                     int mprotect_active);
unsigned pbsd_kernel_pax_mprotect_apply(unsigned prot, unsigned maxprot);
unsigned pbsd_kernel_pax_pageexec_apply(unsigned prot);
unsigned long long pbsd_kernel_aslr_delta(unsigned long long seed, unsigned lsb,
                                        unsigned len);
unsigned long long pbsd_kernel_aslr_apply_mmap(unsigned long long addr,
                                             unsigned long long orig_addr,
                                             int mmap_flags,
                                             unsigned long long delta,
                                             int active);
int pbsd_kernel_priority_cap_check(int which, int who, int self_pid, int in_cap_mode);
int pbsd_kernel_p_cansee(unsigned viewer_ruid, unsigned subject_ruid,
                         unsigned viewer_jail, unsigned subject_jail,
                         int see_other_uids);
int pbsd_kernel_cap_rights_is_valid(unsigned long long w0, unsigned long long w1);
int pbsd_kernel_str2sig(const char* name);
int pbsd_kernel_errno_to_status(int errno_val);
int pbsd_kernel_sysctl_top_number(const char* name);
int pbsd_kernel_validate_rlimit_id(int which);
int pbsd_kernel_pager_status_ok(int pager_code);
int pbsd_kernel_validate_mib_depth(unsigned depth);

#ifdef __cplusplus
}
#endif
