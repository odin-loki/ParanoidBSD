module;
#include <cstdint>

export module pbsd.geom.nop;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/nop/g_nop.h — GEOM NOP test provider.
export namespace pbsd::geom::nop {

inline constexpr char kClassName[] = "NOP";
inline constexpr unsigned kVersion = 4;
inline constexpr char kSuffix[] = ".nop";
inline constexpr char kPhyspathPassthrough[] = "\255";

enum class ErrorMode : int {
    Passthrough = -1,
    None = 0,
};

struct Config {
    int error{static_cast<int>(ErrorMode::Passthrough)};
    unsigned long long offset{};
    unsigned long long explicit_size{};
    unsigned long long stripe_size{};
    unsigned long long stripe_offset{};
    unsigned read_fail_prob{};
    unsigned write_fail_prob{};
    unsigned delay_msec{};
    unsigned read_delay_prob{};
    unsigned write_delay_prob{};
    int count_until_fail{-1};
};

[[nodiscard]] inline Status validate_prob(unsigned prob) noexcept {
    if (prob > 100) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_config(Config const& cfg) noexcept {
    if (validate_prob(cfg.read_fail_prob) != Status::Ok
        || validate_prob(cfg.write_fail_prob) != Status::Ok
        || validate_prob(cfg.read_delay_prob) != Status::Ok
        || validate_prob(cfg.write_delay_prob) != Status::Ok) {
        return Status::Invalid;
    }
    if (cfg.stripe_size != 0 && (cfg.stripe_size % 512) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

struct Stats {
    unsigned long long reads{};
    unsigned long long writes{};
    unsigned long long deletes{};
    unsigned long long read_bytes{};
    unsigned long long wrote_bytes{};
};

} // namespace pbsd::geom::nop
