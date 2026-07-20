module;

#include <cstddef>

export module pbsd.userland.libc.gen.getgrouplist;

/// getgrouplist from hbsd/src/lib/libc/gen/getgrouplist.c (logic-only merge)
export namespace pbsd::userland::libc {

[[nodiscard]] inline int merge_group_list(int* groups, int ngroups, int gid, int maxgroups) noexcept {
    if (maxgroups <= 0) {
        return -1;
    }
    for (int i = 0; i < ngroups; ++i) {
        if (groups[i] == gid) {
            return ngroups;
        }
    }
    if (ngroups >= maxgroups) {
        return -1;
    }
    groups[ngroups] = gid;
    return ngroups + 1;
}

} // namespace pbsd::userland::libc
