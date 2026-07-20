export module pbsd.kde.frameworks.kcoreaddons.filesystemtype;

import pbsd.core;

/// Wave 3 — filesystem type name map (from KFileSystemType s_fsMap).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/io/kfilesystemtype.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::filesystemtype {

enum class Type : unsigned {
    Unknown = 0,
    Nfs,
    Smb,
    Fat,
    Ramfs,
    Ntfs,
    Exfat,
    Fuse,
    Other,
};

struct FsEntry {
    Type type;
    const char* name;
};

inline constexpr FsEntry kFsMap[] = {
    {Type::Nfs, "nfs"},
    {Type::Nfs, "nfs4"},
    {Type::Smb, "smb"},
    {Type::Fat, "fat"},
    {Type::Ramfs, "ramfs"},
    {Type::Other, "other"},
    {Type::Ntfs, "ntfs"},
    {Type::Ntfs, "ntfs3"},
    {Type::Exfat, "exfat"},
    {Type::Unknown, "unknown"},
    {Type::Nfs, "autofs"},
    {Type::Nfs, "cachefs"},
    {Type::Nfs, "fuse.sshfs"},
    {Type::Nfs, "xtreemfs@"},
    {Type::Smb, "smbfs"},
    {Type::Smb, "cifs"},
    {Type::Fat, "vfat"},
    {Type::Fat, "msdos"},
    {Type::Fuse, "fuseblk"},
};

inline constexpr unsigned kFsMapCount = sizeof(kFsMap) / sizeof(kFsMap[0]);

[[nodiscard]] inline Type type_from_name(const char* name) noexcept {
    if (name == nullptr) {
        return Type::Other;
    }
    for (unsigned i = 0; i < kFsMapCount; ++i) {
        const char* entry = kFsMap[i].name;
        unsigned j = 0;
        while (name[j] != '\0' && entry[j] != '\0' && name[j] == entry[j]) {
            ++j;
        }
        if (name[j] == '\0' && entry[j] == '\0') {
            return kFsMap[i].type;
        }
    }
    return Type::Other;
}

[[nodiscard]] inline bool is_network_type(Type t) noexcept {
    return t == Type::Nfs || t == Type::Smb;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/io/kfilesystemtype.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::filesystemtype
