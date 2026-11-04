module;
#include <cstddef>

export module pbsd.stand.geli;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/geli/geliboot.h
export namespace pbsd::stand::geli {

inline constexpr unsigned kMaxKeys = 64;
inline constexpr unsigned kPwMaxLen = 256;
inline constexpr unsigned kDevBsize = 512;
inline constexpr unsigned kGeliBootBsize = 4096;

enum class KeyState : unsigned char {
    None = 0,
    Loaded = 1,
    Active = 2,
};

struct BootHint {
    unsigned sector_size{kGeliBootBsize};
    unsigned key_count{0};
    KeyState state{KeyState::None};
};

[[nodiscard]] inline Status validate_sector_size(unsigned size) noexcept {
    if (size != kDevBsize && size != kGeliBootBsize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline std::size_t keybuf_size(unsigned key_count) noexcept {
    if (key_count > kMaxKeys) {
        return 0;
    }
    return static_cast<std::size_t>(key_count) * 64u + 32u;
}

} // namespace pbsd::stand::geli
