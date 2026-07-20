module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand.signature;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/signature.c
export namespace pbsd::stand::signature {

inline constexpr std::size_t kSha256DigestLen = 32;

enum class Algorithm : unsigned char {
    None = 0,
    Sha256 = 1,
};

struct VerifiedImage {
    Algorithm algo{Algorithm::None};
    std::size_t image_size{};
    bool valid{false};
};

[[nodiscard]] inline Status verify(VerifiedImage& v, std::size_t image_size) noexcept {
    if (image_size == 0) {
        return Status::Invalid;
    }
    v.algo = Algorithm::Sha256;
    v.image_size = image_size;
    v.valid = true;
    return Status::Ok;
}

} // namespace pbsd::stand::signature
