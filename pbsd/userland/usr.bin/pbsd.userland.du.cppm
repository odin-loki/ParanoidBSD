module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.du;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/du/du.c — flag/human-size helpers (logic-only).
export namespace pbsd::userland::usr_bin::du {

struct Options {
    bool all_files{false};
    bool human{false};
    bool si_units{false};
    bool block512{false};
    bool summarize{false};
    bool exclude{false};
    int depth{-1};
    const char* threshold{nullptr};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* arg = argv[i];
        if (hosted::cstrcmp(arg, "--help") == 0) {
            continue;
        }
        if (arg[1] == 'd' && arg[2] != '\0') {
            int d = 0;
            for (const char* p = arg + 2; *p >= '0' && *p <= '9'; ++p) {
                d = d * 10 + (*p - '0');
            }
            opt.depth = d;
            continue;
        }
        if (arg[1] == 'd' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            int d = 0;
            for (const char* p = argv[++i]; *p >= '0' && *p <= '9'; ++p) {
                d = d * 10 + (*p - '0');
            }
            opt.depth = d;
            continue;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'a':
                opt.all_files = true;
                break;
            case 'h':
                opt.human = true;
                break;
            case 'H':
                opt.si_units = true;
                opt.human = true;
                break;
            case 'k':
                opt.block512 = false;
                break;
            case 's':
                opt.summarize = true;
                break;
            case 'x':
                opt.exclude = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline const char* human_suffix(int scale) noexcept {
    static const char* kSuffix[] = {"B", "K", "M", "G", "T", "P", nullptr};
    if (scale < 0 || scale > 5) {
        return kSuffix[0];
    }
    return kSuffix[scale];
}

[[nodiscard]] inline std::uint64_t blocks_to_bytes(std::uint64_t blocks,
                                                   bool block512) noexcept {
    return block512 ? blocks * 512u : blocks * 1024u;
}

} // namespace pbsd::userland::usr_bin::du
