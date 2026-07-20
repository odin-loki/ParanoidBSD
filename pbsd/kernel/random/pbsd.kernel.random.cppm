export module pbsd.kernel.random;

export import pbsd.core;

/// Wave 4 — kernel random(4) entropy sources (sys/random.h).
export namespace pbsd::kernel::random {

enum class EntropySource : unsigned {
    Cached = 0,
    Attach = 1,
    Keyboard = 2,
    Mouse = 3,
    NetTun = 4,
    NetEther = 5,
    NetNg = 6,
    Interrupt = 7,
    Swi = 8,
    FsAtime = 9,
    Uma = 10,
    Callout = 11,
    RandomDev = 12,
    PureTpm = 13,
    PureRdrand = 14,
    PureRdseed = 15,
    PureVirtio = 18,
};

inline constexpr unsigned kMaxSources = 32;

struct HarvestStub {
    EntropySource source{EntropySource::Cached};
    unsigned length{};
};

[[nodiscard]] constexpr bool is_environmental(EntropySource src) noexcept {
    return static_cast<unsigned>(src) <= static_cast<unsigned>(EntropySource::RandomDev);
}

[[nodiscard]] constexpr bool is_pure(EntropySource src) noexcept {
    return static_cast<unsigned>(src) >= static_cast<unsigned>(EntropySource::PureTpm);
}

[[nodiscard]] constexpr Status validate_harvest(const HarvestStub& h) noexcept {
    if (h.length == 0 || h.length > 256) {
        return Status::Invalid;
    }
    if (static_cast<unsigned>(h.source) >= kMaxSources) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned source_table_size() noexcept {
    return 17;
}

} // namespace pbsd::kernel::random
