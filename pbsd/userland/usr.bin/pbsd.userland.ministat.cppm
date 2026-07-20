module;

export module pbsd.userland.ministat;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/ministat/ministat.c — Student-t confidence table.
export namespace pbsd::userland::usr_bin::ministat {

inline constexpr int kNStudent = 100;
inline constexpr int kNConf = 6;
inline constexpr double kStudentPct[kNConf] = {80, 90, 95, 98, 99, 99.5};

[[nodiscard]] inline double student_lookup(int df, int conf_idx) noexcept {
    if (df < 0 || df > kNStudent || conf_idx < 0 || conf_idx >= kNConf) {
        return 0.0;
    }
    static constexpr double kInfRow[kNConf] = {1.282, 1.645, 1.960, 2.326, 2.576, 3.090};
    if (df == 0) {
        return kInfRow[conf_idx];
    }
    static constexpr double kRow1[kNConf] = {3.078, 6.314, 12.706, 31.821, 63.657, 318.313};
    if (df == 1) {
        return kRow1[conf_idx];
    }
    return kInfRow[conf_idx];
}

[[nodiscard]] inline double mean(const double* vals, int n) noexcept {
    if (vals == nullptr || n <= 0) {
        return 0.0;
    }
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        sum += vals[i];
    }
    return sum / static_cast<double>(n);
}

} // namespace pbsd::userland::usr_bin::ministat
