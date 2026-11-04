module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.script;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/script/script.c — flag parse helpers (logic-only).
export namespace pbsd::userland::usr_bin::script {

struct Options {
    const char* output_file{nullptr};
    const char* filemon_path{nullptr};
    bool flush{false};
    bool quiet{false};
    bool raw{false};
    bool show_exit{false};
    bool usesleep{false};
};

struct Stamp {
    std::uint64_t length{0};
    std::uint64_t sec{0};
    std::uint32_t usec{0};
    char direction{'i'};
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
        if (hosted::cstrcmp(arg, "-F") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.filemon_path = argv[++i];
            continue;
        }
        if (hosted::cstrcmp(arg, "-f") == 0) {
            opt.flush = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-q") == 0) {
            opt.quiet = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-r") == 0) {
            opt.raw = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-e") == 0) {
            opt.show_exit = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-T") == 0) {
            opt.usesleep = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-a") == 0) {
            opt.flush = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (i < argc) {
        opt.output_file = argv[i++];
    }
    return result_ok(opt);
}

[[nodiscard]] inline bool stamp_is_input(char dir) noexcept {
    return dir == 'i' || dir == 'I';
}

} // namespace pbsd::userland::usr_bin::script
