module;

#include <cstdint>

export module pbsd.userland.rtld.version;

/// rtld version symbols from hbsd/src/libexec/rtld-elf/rtld.c
export namespace pbsd::userland::rtld {

inline constexpr int kFreeBsdVersion = 1405000;
inline constexpr std::uintptr_t kLaddrOffsetVersion = 1;
inline constexpr std::uintptr_t kDlpiTlsDataVersion = 1;

struct RtldVersionInfo {
    int freebsd_version;
    std::uintptr_t laddr_offset;
    std::uintptr_t dlpi_tls_data;
};

[[nodiscard]] inline RtldVersionInfo current_version() noexcept {
    return {kFreeBsdVersion, kLaddrOffsetVersion, kDlpiTlsDataVersion};
}

} // namespace pbsd::userland::rtld
