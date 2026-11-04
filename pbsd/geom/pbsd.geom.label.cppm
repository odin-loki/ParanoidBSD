module;
#include <cstdint>

export module pbsd.geom.label;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/label/g_label.h — GEOM label providers.
export namespace pbsd::geom::label {

enum class Provider : unsigned int {
    Msdosfs = 0,
    Gpt     = 1,
    Iso9660 = 2,
    Ufs     = 3,
    Ext2fs  = 4,
    Reiserfs = 5,
    Ntfs    = 6,
    Zfs     = 7,
};

struct LabelEntry {
    Provider    provider{};
    const char* prefix{};
};

inline constexpr LabelEntry kProviderTable[] = {
    {Provider::Msdosfs, "msdosfs"},
    {Provider::Gpt, "gpt"},
    {Provider::Iso9660, "cd9660"},
    {Provider::Ufs, "ufs"},
    {Provider::Ext2fs, "ext2fs"},
    {Provider::Ntfs, "ntfs"},
    {Provider::Zfs, "zfs"},
};

[[nodiscard]] inline constexpr std::size_t provider_table_size() noexcept {
    return sizeof(kProviderTable) / sizeof(kProviderTable[0]);
}

} // namespace pbsd::geom::label
