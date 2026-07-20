module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.chflags;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/chflags/chflags.c — option/FTS/flag parsing (logic-only).
export namespace pbsd::userland::bin::chflags {

enum class FollowMode : unsigned char {
    Logical,
    Physical,
    PhysicalFollowArgDir,
};

struct Options {
    FollowMode follow{FollowMode::Logical};
    bool recursive{false};
    bool force{false};
    bool no_dereference{false};
    int verbose{0};
    bool cross_dev{false};
    const char* flags{nullptr};
};

struct FlagMasks {
    std::uint64_t set{0};
    std::uint64_t clear{~0ull};
    bool octal{false};
};

inline constexpr std::uint64_t kUfHidden = 0x00080000ull;
inline constexpr std::uint64_t kUfArchive = 0x00020000ull;
inline constexpr std::uint64_t kUfImmutable = 0x00000002ull;
inline constexpr std::uint64_t kUfAppend = 0x00000004ull;
inline constexpr std::uint64_t kUfNodump = 0x00000001ull;

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }

    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        if (flag[1] == '\0') {
            return result_err<Options>(Status::Invalid);
        }
        for (int j = 1; flag[j] != '\0'; ++j) {
            switch (flag[j]) {
            case 'H':
                opt.follow = FollowMode::PhysicalFollowArgDir;
                break;
            case 'L':
                opt.follow = FollowMode::Logical;
                break;
            case 'P':
                opt.follow = FollowMode::Physical;
                break;
            case 'R':
                opt.recursive = true;
                break;
            case 'f':
                opt.force = true;
                break;
            case 'h':
                opt.no_dereference = true;
                break;
            case 'v':
                ++opt.verbose;
                break;
            case 'x':
                opt.cross_dev = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
        ++i;
    }

    if (i >= argc - 1) {
        return result_err<Options>(Status::Invalid);
    }
    opt.flags = argv[i];
    ++i;
    if (i >= argc) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline Result<unsigned> fts_options_from(const Options& opt) noexcept {
    if (opt.recursive && opt.no_dereference) {
        return result_err<unsigned>(Status::Invalid);
    }
    unsigned fts = 0;
    if (opt.recursive) {
        if (opt.follow == FollowMode::Logical) {
            fts = 1; // FTS_LOGICAL
        } else {
            fts = 2; // FTS_PHYSICAL
            if (opt.follow == FollowMode::PhysicalFollowArgDir) {
                fts |= 4; // FTS_COMFOLLOW
            }
        }
    } else if (opt.no_dereference) {
        fts = 2;
    } else {
        fts = 1;
    }
    if (opt.cross_dev) {
        fts |= 8; // FTS_XDEV
    }
    return result_ok(fts);
}

[[nodiscard]] inline Result<std::uint64_t> parse_octal_flags(const char* s) noexcept {
    if (s == nullptr || s[0] < '0' || s[0] > '7') {
        return result_err<std::uint64_t>(Status::Invalid);
    }
    std::uint64_t val = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        if (*p < '0' || *p > '7') {
            return result_err<std::uint64_t>(Status::Invalid);
        }
        val = (val << 3) | static_cast<std::uint64_t>(*p - '0');
    }
    return result_ok(val);
}

[[nodiscard]] inline Result<std::uint64_t> flag_bit_from_name(const char* name) noexcept {
    if (name == nullptr) {
        return result_err<std::uint64_t>(Status::Invalid);
    }
    if (hosted::cstrcmp(name, "hidden") == 0 || hosted::cstrcmp(name, "h") == 0) {
        return result_ok(kUfHidden);
    }
    if (hosted::cstrcmp(name, "arch") == 0 || hosted::cstrcmp(name, "a") == 0) {
        return result_ok(kUfArchive);
    }
    if (hosted::cstrcmp(name, "immutable") == 0 || hosted::cstrcmp(name, "u") == 0) {
        return result_ok(kUfImmutable);
    }
    if (hosted::cstrcmp(name, "append") == 0 || hosted::cstrcmp(name, "p") == 0) {
        return result_ok(kUfAppend);
    }
    if (hosted::cstrcmp(name, "nodump") == 0 || hosted::cstrcmp(name, "d") == 0) {
        return result_ok(kUfNodump);
    }
    return result_err<std::uint64_t>(Status::NotFound);
}

[[nodiscard]] inline Result<FlagMasks> parse_flags_arg(const char* flags) noexcept {
    if (flags == nullptr || flags[0] == '\0') {
        return result_err<FlagMasks>(Status::Invalid);
    }
    FlagMasks masks{};
    if (flags[0] >= '0' && flags[0] <= '7') {
        const auto oct = parse_octal_flags(flags);
        if (!oct.has_value()) {
            return result_err<FlagMasks>(oct.status);
        }
        masks.set = oct.value;
        masks.octal = true;
        return result_ok(masks);
    }

    masks.set = 0;
    masks.clear = ~0ull;
    const char* p = flags;
    while (*p != '\0') {
        bool clear = false;
        if (*p == 'n' && p[1] == 'o') {
            p += 2;
            clear = true;
        } else if (*p == '-') {
            ++p;
            clear = true;
        }
        const char* start = p;
        while (*p != '\0' && *p != ',' && *p != '+' && *p != '-') {
            ++p;
        }
        char token[32];
        const std::size_t len = static_cast<std::size_t>(p - start);
        if (len == 0 || len >= sizeof(token)) {
            return result_err<FlagMasks>(Status::Invalid);
        }
        for (std::size_t i = 0; i < len; ++i) {
            token[i] = start[i];
        }
        token[len] = '\0';
        const auto bit = flag_bit_from_name(token);
        if (!bit.has_value()) {
            return result_err<FlagMasks>(bit.status);
        }
        if (clear) {
            masks.clear &= ~bit.value;
        } else {
            masks.set |= bit.value;
        }
        if (*p == ',') {
            ++p;
        }
    }
    return result_ok(masks);
}

[[nodiscard]] inline std::uint64_t compute_new_flags(std::uint64_t old_flags,
                                                     const FlagMasks& masks) noexcept {
    if (masks.octal) {
        return masks.set;
    }
    return (old_flags | masks.set) & masks.clear;
}

[[nodiscard]] inline bool flags_unchanged(std::uint64_t old_flags,
                                          std::uint64_t new_flags) noexcept {
    return old_flags == new_flags;
}

} // namespace pbsd::userland::bin::chflags
