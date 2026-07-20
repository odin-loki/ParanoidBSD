export module pbsd.fs.tmpfs;

export import pbsd.core;

/// Wave 6 — tmpfs mount options from sys/fs/tmpfs/tmpfs.h.
export namespace pbsd::fs::tmpfs {

inline constexpr unsigned long long kDefaultSizeBytes = 0; // 0 = RAM-backed unbounded stub
inline constexpr unsigned kDefaultMaxNodes = 0;

struct MountArgs {
    unsigned long long max_size_bytes{kDefaultSizeBytes};
    unsigned max_nodes{kDefaultMaxNodes};
    unsigned mode{0755};
    int uid{};
    int gid{};
};

[[nodiscard]] constexpr Status validate(const MountArgs& a) noexcept {
    if ((a.mode & ~07777u) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::tmpfs
