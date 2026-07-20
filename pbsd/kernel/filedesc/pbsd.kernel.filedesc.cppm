module;
#include <cstdint>

export module pbsd.kernel.filedesc;

export import pbsd.core;

/// Wave 4 — file descriptor table layout and per-fd flags (sys/filedesc.h).
export namespace pbsd::kernel::filedesc {

inline constexpr unsigned char kUfExclose         = 0x01;
inline constexpr unsigned char kUfResolveBeneath  = 0x02;
inline constexpr unsigned char kUfFoclose         = 0x04;

inline constexpr int kDtypeVnode = 1;
inline constexpr int kDtypeSocket = 2;
inline constexpr int kDtypePipe  = 3;
inline constexpr int kDtypeKqueue = 4;
inline constexpr int kDtypeProc   = 5;
inline constexpr int kDtypeDev    = 6;

struct FileCapsStub {
    std::uint64_t rights[2]{};
    std::uint32_t fcntls{};
    std::int16_t  nioctls{};
};

struct FdescentStub {
    void*         fde_file{};
    FileCapsStub  fde_caps{};
    unsigned char fde_flags{};
};

struct FdescentTblStub {
    int              fdt_nfiles{};
    FdescentStub*    fdt_ofiles{};
};

struct FiledescStub {
    FdescentTblStub* fd_files{};
    int              fd_freefile{};
    int              fd_refcnt{};
    int              fd_holdcnt{};
    int              fd_holdleaderscount{};
};

struct DtypeEntry {
    int         dtype;
    const char* name;
};

inline constexpr DtypeEntry kDtypeTable[] = {
    {kDtypeVnode,  "DTYPE_VNODE"},
    {kDtypeSocket, "DTYPE_SOCKET"},
    {kDtypePipe,   "DTYPE_PIPE"},
    {kDtypeKqueue, "DTYPE_KQUEUE"},
    {kDtypeProc,   "DTYPE_PROC"},
    {kDtypeDev,    "DTYPE_DEV"},
};

[[nodiscard]] inline unsigned dtype_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kDtypeTable) / sizeof(kDtypeTable[0]));
}

[[nodiscard]] constexpr bool uf_has(unsigned char flags, unsigned char bit) noexcept {
    return (flags & bit) != 0;
}

[[nodiscard]] constexpr Status validate_fd(int fd, int nfiles) noexcept {
    if (fd < 0 || fd >= nfiles) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_filedesc(const FiledescStub& fd) noexcept {
    if (fd.fd_files == nullptr || fd.fd_files->fdt_nfiles <= 0) {
        return Status::Invalid;
    }
    if (fd.fd_refcnt < 0 || fd.fd_holdcnt < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool auto_close_on_exec(unsigned char uf) noexcept {
    return uf_has(uf, kUfExclose);
}

[[nodiscard]] constexpr bool resolve_beneath(unsigned char uf) noexcept {
    return uf_has(uf, kUfResolveBeneath);
}

[[nodiscard]] inline Result<const char*> dtype_name(int dtype) noexcept {
    for (const auto& e : kDtypeTable) {
        if (e.dtype == dtype) {
            return result_ok(e.name);
        }
    }
    return result_err<const char*>(Status::NotFound);
}

[[nodiscard]] constexpr int next_free_hint(const FiledescStub& fd) noexcept {
    return fd.fd_freefile >= 0 ? fd.fd_freefile : 0;
}

} // namespace pbsd::kernel::filedesc
