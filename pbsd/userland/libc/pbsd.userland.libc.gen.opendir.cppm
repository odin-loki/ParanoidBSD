module;

export module pbsd.userland.libc.gen.opendir;

/// opendir/closedir concepts from hbsd/src/lib/libc/gen/opendir.c
export namespace pbsd::userland::libc {

struct DirEntry {
    const char* name{nullptr};
    int fd{-1};
};

struct Dir {
    int fd{-1};
    unsigned index{0};
};

[[nodiscard]] inline bool dir_valid(const Dir& d) noexcept { return d.fd >= 0; }

inline void closedir(Dir& d) noexcept {
    d.fd = -1;
    d.index = 0;
}

} // namespace pbsd::userland::libc
