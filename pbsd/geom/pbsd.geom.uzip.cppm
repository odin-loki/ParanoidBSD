module;
#include <cstdint>

export module pbsd.geom.uzip;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/uzip/g_uzip_softc.h, g_uzip_cloop.h — GEOM UZIP.
export namespace pbsd::geom::uzip {

inline constexpr unsigned kShutdown = 0x1;
inline constexpr unsigned kExiting  = 0x2;

enum class Compression : unsigned char {
    LibZ     = static_cast<unsigned char>('V'),
    LibZDdp  = static_cast<unsigned char>('v'),
    Lzma     = static_cast<unsigned char>('L'),
    LzmaDdp  = static_cast<unsigned char>('l'),
    Zstd     = static_cast<unsigned char>('Z'),
    ZstdDdp  = static_cast<unsigned char>('z'),
};

struct Softc {
    unsigned block_size{};
    unsigned num_blocks{};
    unsigned last_block{};
    int req_total{};
    int req_cached{};
    unsigned wrkthr_flags{};
};

[[nodiscard]] inline Status validate_block_size(unsigned blksz) noexcept {
    if (blksz == 0 || (blksz & (blksz - 1)) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_compression(Compression c) noexcept {
    switch (c) {
    case Compression::LibZ:
    case Compression::LibZDdp:
    case Compression::Lzma:
    case Compression::LzmaDdp:
    case Compression::Zstd:
    case Compression::ZstdDdp:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline bool is_shutting_down(unsigned flags) noexcept {
    return (flags & (kShutdown | kExiting)) != 0;
}

[[nodiscard]] inline Status account_request(Softc& sc, bool cache_hit) noexcept {
    if (is_shutting_down(sc.wrkthr_flags)) {
        return Status::Denied;
    }
    ++sc.req_total;
    if (cache_hit) {
        ++sc.req_cached;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned cache_hit_ratio(Softc const& sc) noexcept {
    if (sc.req_total == 0) {
        return 0;
    }
    return static_cast<unsigned>((100ull * sc.req_cached) / static_cast<unsigned>(sc.req_total));
}

} // namespace pbsd::geom::uzip
