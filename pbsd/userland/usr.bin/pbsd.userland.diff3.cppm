module;
#include <cstddef>

export module pbsd.userland.diff3;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/diff3/diff3.c — three-way diff options (logic-only).
export namespace pbsd::userland::usr_bin::diff3 {

inline constexpr int kExitConflict = 1;
inline constexpr int kExitError = 2;

struct Options {
    bool easy_only{false};
    bool show_all{false};
    bool overlap_only{false};
    bool text_mode{false};
    bool ed_script{false};
    const char* label_mine{nullptr};
    const char* label_older{nullptr};
    const char* label_yours{nullptr};
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
        if (hosted::cstrcmp(arg, "-e") == 0) {
            opt.ed_script = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-E") == 0) {
            opt.easy_only = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-A") == 0) {
            opt.show_all = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-x") == 0) {
            opt.overlap_only = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-a") == 0) {
            opt.text_mode = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-m") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.label_mine = argv[++i];
            continue;
        }
        if (arg[1] == 'm' && arg[2] != '\0') {
            opt.label_mine = arg + 2;
            continue;
        }
        if (hosted::cstrcmp(arg, "-L") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.label_older = argv[++i];
            continue;
        }
        if (arg[1] == 'L' && arg[2] != '\0') {
            opt.label_older = arg + 2;
            continue;
        }
        if (hosted::cstrcmp(arg, "-X") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.label_yours = argv[++i];
            continue;
        }
        if (arg[1] == 'X' && arg[2] != '\0') {
            opt.label_yours = arg + 2;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (argc - i < 3) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::diff3
