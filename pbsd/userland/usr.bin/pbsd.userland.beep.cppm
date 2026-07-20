module;

export module pbsd.userland.beep;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/beep/beep.c — tone parameter validation.
export namespace pbsd::userland::usr_bin::beep {

inline constexpr int kSampleRateDef = 48000;
inline constexpr int kSampleRateMax = 48000;
inline constexpr int kSampleRateMin = 8000;
inline constexpr int kDurationDef = 150;
inline constexpr int kDurationMax = 2000;
inline constexpr int kDurationMin = 50;
inline constexpr int kGainDef = 75;
inline constexpr int kGainMax = 100;
inline constexpr int kGainMin = 0;
inline constexpr int kDefaultHz = 440;

[[nodiscard]] inline bool in_range(int v, int lo, int hi) noexcept {
    return v >= lo && v <= hi;
}

[[nodiscard]] inline Result<int> clamp_frequency(int hz) noexcept {
    if (hz <= 0) {
        return result_ok(kDefaultHz);
    }
    if (hz > 20000) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(hz);
}

[[nodiscard]] inline Result<int> clamp_duration(int ms) noexcept {
    if (!in_range(ms, kDurationMin, kDurationMax)) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(ms);
}

[[nodiscard]] inline Result<int> clamp_gain(int gain) noexcept {
    if (!in_range(gain, kGainMin, kGainMax)) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(gain);
}

} // namespace pbsd::userland::usr_bin::beep
