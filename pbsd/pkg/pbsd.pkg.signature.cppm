module;
#include <cstdint>

export module pbsd.pkg.signature;

import pbsd.core;

/// Wave 9 — package signature verification stubs.
export namespace pbsd::pkg::signature {

enum class Algo : unsigned char {
    None    = 0,
    Sha256  = 1,
    Ed25519 = 2,
};

struct SignatureBlock {
    Algo algo{Algo::None};
    char key_id[32]{};
    char sig_hex[129]{};
};

[[nodiscard]] inline Status validate_block(SignatureBlock const& b) noexcept {
    if (b.algo == Algo::None) {
        return Status::Invalid;
    }
    if (b.sig_hex[0] == '\0') {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status verify_digest(char const* digest_hex,
                                          SignatureBlock const& b) noexcept {
    if (digest_hex == nullptr || digest_hex[0] == '\0') {
        return Status::Invalid;
    }
    return validate_block(b);
}

} // namespace pbsd::pkg::signature
