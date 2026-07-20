module;
#include <cstdint>

export module pbsd.kernel.bsm_token;

import pbsd.core;

/// Freestanding port of `security/audit/bsm_token.c` — BSM token type constants.
export namespace pbsd::kernel::bsm_token {

inline constexpr unsigned char kFile = 1;
inline constexpr unsigned char kPath = 11;
inline constexpr unsigned char kSubject32 = 36;
inline constexpr unsigned char kReturn32 = 39;
inline constexpr unsigned char kText = 58;

[[nodiscard]] inline Status validate_type(unsigned char token_type) noexcept {
    switch (token_type) {
    case kFile:
    case kPath:
    case kSubject32:
    case kReturn32:
    case kText:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::kernel::bsm_token
