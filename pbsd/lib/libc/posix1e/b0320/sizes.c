#include <stdint.h>
#include <stdio.h>
typedef uint32_t acl_tag_t;
typedef uint32_t acl_perm_t;
typedef uint16_t acl_entry_type_t;
typedef uint16_t acl_flag_t;
struct acl_entry { acl_tag_t ae_tag; unsigned ae_id; acl_perm_t ae_perm; acl_entry_type_t ae_entry_type; acl_flag_t ae_flags; };
#define ACL_MAX_ENTRIES 254
struct acl { unsigned acl_maxcnt, acl_cnt; int acl_spare[4]; struct acl_entry acl_entry[ACL_MAX_ENTRIES]; };
struct acl_t_struct { struct acl ats_acl; int ats_cur_entry; int ats_brand; };
int main(void) { printf("c=%zu\n", sizeof(struct acl_t_struct)); return 0; }
