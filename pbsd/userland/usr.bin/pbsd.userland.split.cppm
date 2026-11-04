module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.split;

export import pbsd.core;
export import pbsd.userland.hosted;
export import pbsd.userland.util.numbers;

/// Port of hbsd/src/usr.bin/split/split.c — flag parse (logic-only).
export namespace pbsd::userland::usr_bin::split {

inline constexpr long kDefLines = 1000;

struct Options {
    long line_count{kDefLines};
    std::int64_t byte_count{0};
    long chunk_count{0};
    bool clobber{true};
    const char* prefix{"x"};
    const char* suffix{nullptr};
    bool numeric_suffix{false};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr; ++i) {
        const char* arg = argv[i];
        if (arg[0] != '-') {
            break;
        }
        if (hosted::cstrcmp(arg, "-a") == 0) {
            opt.clobber = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-n") == 0) {
            opt.numeric_suffix = true;
            continue;
        }
        if (arg[1] == 'l' && arg[2] != '\0') {
            long long n = 0;
            if (util::expand_number(arg + 2, &n) != 0 || n <= 0) {
                return result_err<Options>(Status::Invalid);
            }
            opt.line_count = static_cast<long>(n);
            continue;
        }
        if (arg[1] == 'b' && arg[2] != '\0') {
            long long n = 0;
            if (util::expand_number(arg + 2, &n) != 0 || n <= 0) {
                return result_err<Options>(Status::Invalid);
            }
            opt.byte_count = static_cast<std::int64_t>(n);
            continue;
        }
        if (arg[1] == 'C' && arg[2] != '\0') {
            long long n = 0;
            if (util::expand_number(arg + 2, &n) != 0 || n <= 0) {
                return result_err<Options>(Status::Invalid);
            }
            opt.chunk_count = static_cast<long>(n);
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (i < argc) {
        opt.prefix = argv[i++];
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::split
