export module pbsd.userland.od;

export import pbsd.userland.hosted;

/// Wave 2 — od(1) dump formats from usr.bin/od/od.h (subset).
export namespace pbsd::userland::od {

enum class Format : unsigned char {
    Octal = 0,
    Decimal = 1,
    Hex = 2,
    Char = 3,
    Float = 4,
};

struct Options {
    Format format{Format::Octal};
    unsigned bytes_per_line{16};
    unsigned skip{};
    unsigned count{}; // 0 = unlimited
    bool traditional{};
};

[[nodiscard]] constexpr unsigned radix(Format f) noexcept {
    switch (f) {
    case Format::Octal:
        return 8;
    case Format::Decimal:
        return 10;
    case Format::Hex:
        return 16;
    default:
        return 10;
    }
}

[[nodiscard]] constexpr Status validate(const Options& o) noexcept {
    if (o.bytes_per_line == 0 || o.bytes_per_line > 64) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::od
