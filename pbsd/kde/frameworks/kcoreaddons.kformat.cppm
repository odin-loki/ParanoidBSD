module;

#include <cmath>

export module pbsd.kde.frameworks.kcoreaddons.kformat;

import pbsd.core;

/// Wave 3 — byte size dialect + IEC formatting (from KFormat::formatByteSize).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/util/kformat.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kformat {

enum class BinaryUnitDialect : unsigned char { IEC, JEDEC, MetricBinary };

struct ByteSizeResult {
    double value{0.0};
    char unit[8]{};
};

[[nodiscard]] inline ByteSizeResult format_byte_size(double bytes, int precision,
                                                     BinaryUnitDialect dialect) noexcept {
    ByteSizeResult r{};
    if (bytes < 0.0) {
        bytes = -bytes;
    }
    double base = dialect == BinaryUnitDialect::MetricBinary ? 1000.0 : 1024.0;
    double v = bytes;
    unsigned idx = 0;
    while (v >= base && idx < 4) {
        v /= base;
        ++idx;
    }
    const double scale = std::pow(10.0, -precision);
    r.value = std::floor(v / scale + 0.5) * scale;
    const char suffixes[] = {'B', 'K', 'M', 'G', 'T'};
    r.unit[0] = suffixes[idx];
    r.unit[1] = (dialect == BinaryUnitDialect::IEC && idx > 0) ? 'i' : '\0';
    r.unit[2] = 'B';
    r.unit[3] = '\0';
    return r;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/util/kformat.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kformat
