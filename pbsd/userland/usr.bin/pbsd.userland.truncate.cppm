module;
#include <cstdint>

export module pbsd.userland.truncate;

export import pbsd.core;
export import pbsd.userland.hosted;
export import pbsd.userland.util.numbers;

/// Port of hbsd/src/usr.bin/truncate/truncate.c — flag parse (logic-only).
export namespace pbsd::userland::usr_bin::truncate {

struct Options {
    bool no_create{false};
    bool dealloc{false};
    bool relative{false};
    bool round{false};
    bool refer{false};
    bool got_size{false};
    bool do_truncate{true};
    std::int64_t size{0};
    std::int64_t offset{0};
    std::int64_t length{-1};
    const char* refer_name{nullptr};
};

[[nodiscard]] inline Result<Options> parse_size_arg(const char* arg,
                                                      Options& opt) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return result_err<Options>(Status::Invalid);
    }
    const char* num = arg;
    if (*num == '+' || *num == '-' || *num == '%' || *num == '/') {
        if (*num == '+' || *num == '-') {
            opt.relative = true;
        } else {
            opt.round = true;
        }
        ++num;
    }
    long long usz = 0;
    if (util::expand_number(num, &usz) != 0 || usz < 0) {
        return result_err<Options>(Status::Invalid);
    }
    opt.size = (arg[0] == '-' || arg[0] == '/') ? -usz : usz;
    opt.got_size = true;
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
        for (int j = 1; flag[j] != '\0'; ++j) {
            switch (flag[j]) {
            case 'c':
                opt.no_create = true;
                break;
            case 'd':
                opt.dealloc = true;
                opt.do_truncate = false;
                break;
            case 'r':
                if (flag[j + 1] == '\0') {
                    if (i + 1 >= argc) {
                        return result_err<Options>(Status::Invalid);
                    }
                    opt.refer = true;
                    opt.refer_name = argv[++i];
                    goto next_flag;
                }
                opt.refer = true;
                opt.refer_name = flag + j + 1;
                goto next_flag;
            case 's':
                if (flag[j + 1] == '\0') {
                    if (i + 1 >= argc) {
                        return result_err<Options>(Status::Invalid);
                    }
                    if (!parse_size_arg(argv[++i], opt).has_value()) {
                        return result_err<Options>(Status::Invalid);
                    }
                    goto next_flag;
                }
                if (!parse_size_arg(flag + j + 1, opt).has_value()) {
                    return result_err<Options>(Status::Invalid);
                }
                goto next_flag;
            case 'o':
            case 'l':
                return result_err<Options>(Status::Invalid);
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    next_flag:;
    }
    optind_out = i;
    if (i >= argc) {
        return result_err<Options>(Status::Invalid);
    }
    if (!opt.got_size && !opt.refer) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::truncate
