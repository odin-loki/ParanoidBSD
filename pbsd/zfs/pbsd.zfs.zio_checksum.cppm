module;
#include <cstdint>

export module pbsd.zfs.zio_checksum;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/zio_checksum.h — checksum algorithms.
export namespace pbsd::zfs::zio_checksum {

enum class Algorithm : unsigned char {
    Inherit = 0,
    On,
    Off,
    Label,
    GangHeader,
    Zilog,
    Fletcher2,
    Fletcher4,
    Sha256,
    Zilog2,
    NoParity,
    Sha512,
    Skein,
    Edonr,
    Blake3,
    Functions,
};

enum class Flag : unsigned int {
    Metadata = 1u << 1,
    Embedded = 1u << 2,
    Dedup    = 1u << 3,
    Salted   = 1u << 4,
    Nopwrite = 1u << 5,
};

[[nodiscard]] constexpr Flag operator|(Flag a, Flag b) noexcept {
    return static_cast<Flag>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] constexpr Flag flags_meta_salted() noexcept {
    return Flag::Metadata | Flag::Salted;
}

struct Info {
    Algorithm algo{};
    Flag flags{};
    const char* name{};
};

inline constexpr Info kTable[] = {
    {Algorithm::Fletcher4, Flag::Metadata, "fletcher4"},
    {Algorithm::Sha256, flags_meta_salted(), "sha256"},
    {Algorithm::Sha512, flags_meta_salted(), "sha512"},
    {Algorithm::Skein, flags_meta_salted(), "skein"},
    {Algorithm::Blake3, flags_meta_salted(), "blake3"},
    {Algorithm::Off, static_cast<Flag>(0), "off"},
    {Algorithm::On, Flag::Metadata, "on"},
};

[[nodiscard]] inline constexpr std::size_t table_size() noexcept {
    return sizeof(kTable) / sizeof(kTable[0]);
}

[[nodiscard]] inline Status validate_algo(Algorithm a) noexcept {
    if (a >= Algorithm::Functions) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool flag_has(Flag f, Flag bit) noexcept {
    return (static_cast<unsigned>(f) & static_cast<unsigned>(bit)) != 0;
}

} // namespace pbsd::zfs::zio_checksum
