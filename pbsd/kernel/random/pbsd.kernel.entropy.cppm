export module pbsd.kernel.entropy;

export import pbsd.core;
export import pbsd.kernel.random;

/// Wave 4 — entropy processor KPI stubs (dev/random/randomdev.h).
export namespace pbsd::kernel::entropy {

inline constexpr unsigned kDefaultReadLen = 32;
inline constexpr unsigned kMaxReadLen = 4096;

struct AlgorithmStub {
    const char* ident{"fortuna"};
    bool seeded{};
    unsigned read_count{};
};

[[nodiscard]] constexpr Status validate_read_len(unsigned len) noexcept {
    if (len == 0 || len > kMaxReadLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status pre_read(AlgorithmStub& alg) noexcept {
    if (!alg.seeded) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status record_read(AlgorithmStub& alg, unsigned len) noexcept {
    if (validate_read_len(len) != Status::Ok) {
        return Status::Invalid;
    }
    if (pre_read(alg) != Status::Ok) {
        return Status::Protocol;
    }
    alg.read_count += len;
    return Status::Ok;
}

[[nodiscard]] constexpr Status harvest_from(random::EntropySource src,
                                            unsigned len) noexcept {
    random::HarvestStub h{.source = src, .length = len};
    return random::validate_harvest(h);
}

} // namespace pbsd::kernel::entropy
