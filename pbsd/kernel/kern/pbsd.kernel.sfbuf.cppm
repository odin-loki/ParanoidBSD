module;
#include <cstdint>

export module pbsd.kernel.sfbuf;

export import pbsd.core;

/// Freestanding port of `sys/sfbuf.h` / `kern/subr_sfbuf.c`.
export namespace pbsd::kernel::sfbuf {

inline constexpr unsigned kFlagRead = 0x0001;
inline constexpr unsigned kFlagWrite = 0x0002;
inline constexpr unsigned kFlagUnmapped = 0x0004;

struct Softc {
    void*     va{nullptr};
    std::size_t len{};
    unsigned  flags{};
};

[[nodiscard]] inline bool is_read(unsigned flags) noexcept {
    return (flags & kFlagRead) != 0;
}

[[nodiscard]] inline Status validate(Softc& s) noexcept {
    if (s.va == nullptr || s.len == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::sfbuf
