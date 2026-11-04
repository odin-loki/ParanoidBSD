module;
#include <cstdint>

export module pbsd.userland.tail;

export import pbsd.core;
export import pbsd.userland.hosted;
export import pbsd.userland.util.numbers;

/// Port of hbsd/src/usr.bin/tail/tail.c — flag parse (logic-only).
export namespace pbsd::userland::usr_bin::tail {

enum class ReadStyle : unsigned char {
    NotSet,
    Forward,
    Backward,
};

struct Options {
    ReadStyle style{ReadStyle::NotSet};
    std::int64_t offset{0};
    bool follow{false};
    bool follow_create{false};
    bool follow_name{false};
    bool quiet{false};
    bool verbose{false};
    bool reverse{false};
};

[[nodiscard]] inline Result<Options> parse_number_arg(const char* optarg,
                                                      std::int64_t units,
                                                      Options& opt) noexcept {
    if (optarg == nullptr || optarg[0] == '\0') {
        return result_err<Options>(Status::Invalid);
    }
    long long num = 0;
    const char* digits = optarg;
    if (*digits == '+' || *digits == '-') {
        ++digits;
    }
    if (util::expand_number(digits, &num) != 0) {
        return result_err<Options>(Status::Invalid);
    }
    std::int64_t off = static_cast<std::int64_t>(num) * units;
    switch (optarg[0]) {
    case '+':
        if (off != 0) {
            off -= units;
        }
        opt.style = ReadStyle::Forward;
        break;
    case '-':
        off = -off;
        opt.style = ReadStyle::Backward;
        break;
    default:
        opt.style = ReadStyle::Backward;
        break;
    }
    opt.offset = off;
    return result_ok(opt);
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* flag = argv[i];
        if (hosted::cstrcmp(flag, "-F") == 0) {
            opt.follow_create = true;
            opt.follow = true;
            continue;
        }
        if (flag[1] == 'b' && flag[2] != '\0') {
            if (!parse_number_arg(flag + 2, 512, opt).has_value()) {
                return result_err<Options>(Status::Invalid);
            }
            continue;
        }
        if (flag[1] == 'c' && flag[2] != '\0') {
            if (!parse_number_arg(flag + 2, 1, opt).has_value()) {
                return result_err<Options>(Status::Invalid);
            }
            continue;
        }
        if (hosted::cstrcmp(flag, "-f") == 0) {
            opt.follow = true;
            continue;
        }
        if (flag[1] == 'n' && flag[2] != '\0') {
            if (!parse_number_arg(flag + 2, 1, opt).has_value()) {
                return result_err<Options>(Status::Invalid);
            }
            continue;
        }
        if (hosted::cstrcmp(flag, "-q") == 0) {
            opt.quiet = true;
            opt.verbose = false;
            continue;
        }
        if (hosted::cstrcmp(flag, "-r") == 0) {
            opt.reverse = true;
            continue;
        }
        if (hosted::cstrcmp(flag, "-v") == 0) {
            opt.verbose = true;
            opt.quiet = false;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (opt.style == ReadStyle::NotSet) {
        opt.style = ReadStyle::Backward;
        opt.offset = -10;
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::tail
