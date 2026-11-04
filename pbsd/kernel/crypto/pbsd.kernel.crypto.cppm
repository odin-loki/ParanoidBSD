export module pbsd.kernel.crypto;

export import pbsd.core;

/// Wave 4 — opencrypto KPI stubs (opencrypto/cryptodev.h).
export namespace pbsd::kernel::crypto {

inline constexpr unsigned kDriversInitial = 4;
inline constexpr unsigned kSha256HashLen = 32;
inline constexpr unsigned kSha512HashLen = 64;
inline constexpr unsigned kAesBlockLen = 16;
inline constexpr unsigned kMaxKeyLen = 64;

enum class Op : unsigned {
    Encrypt = 1,
    Decrypt = 2,
    Compress = 3,
    Decompress = 4,
    Mac = 5,
    Digest = 6,
};

enum class Alg : unsigned {
    Null = 0,
    Des3Cbc = 1,
    AesCbc = 2,
    Sha1Hmac = 3,
    Sha256Hmac = 4,
    AesGcm = 5,
    Chacha20Poly1305 = 6,
};

struct SessionStub {
    Op op{};
    Alg crid{};
    unsigned key_len{};
    unsigned iv_len{};
};

[[nodiscard]] constexpr Status validate_session(const SessionStub& s) noexcept {
    if (s.key_len == 0 || s.key_len > kMaxKeyLen) {
        return Status::Invalid;
    }
    if (static_cast<unsigned>(s.op) == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr unsigned hash_len(Alg a) noexcept {
    switch (a) {
    case Alg::Sha1Hmac:
        return 20;
    case Alg::Sha256Hmac:
        return kSha256HashLen;
    default:
        return 0;
    }
}

[[nodiscard]] inline unsigned alg_table_size() noexcept {
    return 7;
}

} // namespace pbsd::kernel::crypto
