module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.uuidgen;

export import pbsd.core;
export import pbsd.userland.capsicum.helpers;

/// Port of hbsd/src/bin/uuidgen/uuidgen.c — Capsicum-first option parsing.
export namespace pbsd::userland::bin::uuidgen {

enum class UuidVersion : unsigned char {
    V1 = 1,
    V4 = 4,
};

struct Options {
    bool iterate{false};
    bool compact{false};
    UuidVersion version{UuidVersion::V4};
    int count{1};
    const char* output_file{nullptr};
};

struct UuidFields {
    std::uint32_t time_low{0};
    std::uint16_t time_mid{0};
    std::uint16_t time_hi_and_version{0};
    std::uint8_t clock_seq_hi_and_reserved{0};
    std::uint8_t clock_seq_low{0};
    std::uint8_t node[6]{};
};

[[nodiscard]] inline Result<int> parse_count(const char* s) noexcept {
    if (s == nullptr || s[0] == '\0') {
        return result_err<int>(Status::Invalid);
    }
    int val = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return result_err<int>(Status::Invalid);
        }
        val = val * 10 + (*p - '0');
        if (val < 1) {
            return result_err<int>(Status::Invalid);
        }
    }
    return result_ok(val);
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }

    bool count_set = false;
    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        if (flag[1] == '1' && flag[2] == '\0') {
            opt.iterate = true;
        } else if (flag[1] == 'R' && flag[2] == '\0') {
            opt.version = UuidVersion::V1;
        } else if (flag[1] == 'c' && flag[2] == '\0') {
            opt.compact = true;
        } else if (flag[1] == 'r' && flag[2] == '\0') {
            if (opt.version == UuidVersion::V1) {
                return result_err<Options>(Status::Invalid);
            }
            opt.version = UuidVersion::V4;
        } else if (flag[1] == 'n' && flag[2] == '\0') {
            if (i + 1 >= argc || argv[i + 1] == nullptr) {
                return result_err<Options>(Status::Invalid);
            }
            if (count_set) {
                return result_err<Options>(Status::Invalid);
            }
            const auto c = parse_count(argv[i + 1]);
            if (!c.has_value()) {
                return result_err<Options>(c.status);
            }
            opt.count = c.value;
            count_set = true;
            ++i;
        } else if (flag[1] == 'o' && flag[2] == '\0') {
            if (i + 1 >= argc || argv[i + 1] == nullptr) {
                return result_err<Options>(Status::Invalid);
            }
            if (opt.output_file != nullptr) {
                return result_err<Options>(Status::Invalid);
            }
            opt.output_file = argv[i + 1];
            ++i;
        } else {
            return result_err<Options>(Status::Invalid);
        }
        ++i;
    }

    if (i < argc) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline void apply_uuid_v4_version(UuidFields& u) noexcept {
    u.clock_seq_hi_and_reserved &= static_cast<std::uint8_t>(~(3 << 6));
    u.clock_seq_hi_and_reserved =
        static_cast<std::uint8_t>(u.clock_seq_hi_and_reserved | (2 << 6));
    u.time_hi_and_version =
        static_cast<std::uint16_t>((u.time_hi_and_version & ~(15 << 12)) | (4 << 12));
}

[[nodiscard]] inline StatusOnly enter_capsicum_sandbox() noexcept {
    if (capsicum::limit_stdio() != Status::Ok) {
        return status_err(Status::Denied);
    }
    if (capsicum::enter_sandbox() != Status::Ok) {
        return status_err(Status::Denied);
    }
    return status_ok();
}

} // namespace pbsd::userland::bin::uuidgen
