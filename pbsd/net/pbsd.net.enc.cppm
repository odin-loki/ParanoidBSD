module;
#include <cstdint>

export module pbsd.net.enc;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_enc.h — IPsec encap context flags.
export namespace pbsd::net::enc {

inline constexpr unsigned char kEncBefore = 0x01;
inline constexpr unsigned char kEncAfter  = 0x02;

enum class EncPhase : unsigned char {
    None   = 0,
    Before = kEncBefore,
    After  = kEncAfter,
};

struct IpsecCtx {
    void*          mbuf_ptr{};
    void*          secasvar{};
    void*          inpcb{};
    unsigned char  af{};
    unsigned char  enc{};
};

[[nodiscard]] inline Status validate_af(unsigned char af) noexcept {
    if (af != 2 && af != 28) { // AF_INET, AF_INET6
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_enc_flags(unsigned char enc) noexcept {
    if (enc == 0) {
        return Status::Invalid;
    }
    if ((enc & ~(kEncBefore | kEncAfter)) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_ctx(IpsecCtx const& ctx) noexcept {
    if (validate_af(ctx.af) != Status::Ok) {
        return Status::Invalid;
    }
    return validate_enc_flags(ctx.enc);
}

[[nodiscard]] inline bool enc_before(unsigned char enc) noexcept {
    return (enc & kEncBefore) != 0;
}

[[nodiscard]] inline bool enc_after(unsigned char enc) noexcept {
    return (enc & kEncAfter) != 0;
}

} // namespace pbsd::net::enc
