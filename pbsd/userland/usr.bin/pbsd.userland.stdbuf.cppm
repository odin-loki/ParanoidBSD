module;
#include <cstddef>

export module pbsd.userland.stdbuf;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/stdbuf/stdbuf.c — buffer spec helpers (logic-only).
export namespace pbsd::userland::usr_bin::stdbuf {

inline constexpr const char* kLibStdbuf = "/usr/lib/libstdbuf.so";
inline constexpr const char* kLibStdbuf32 = "/usr/lib32/libstdbuf.so";
inline constexpr const char* kEnvInput = "_STDBUF_I";
inline constexpr const char* kEnvOutput = "_STDBUF_O";
inline constexpr const char* kEnvError = "_STDBUF_E";

struct Options {
    const char* input{nullptr};
    const char* output{nullptr};
    const char* error{nullptr};
};

[[nodiscard]] inline bool is_valid_buffer_spec(const char* spec) noexcept {
    if (spec == nullptr || *spec == '\0') {
        return false;
    }
    if (hosted::cstrcmp(spec, "0") == 0) {
        return true;
    }
    if (hosted::cstrcmp(spec, "L") == 0 || hosted::cstrcmp(spec, "B") == 0) {
        return true;
    }
    for (const char* p = spec; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* arg = argv[i];
        if (arg[1] == 'e' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.error = argv[++i];
            if (!is_valid_buffer_spec(opt.error)) {
                return result_err<Options>(Status::Invalid);
            }
            continue;
        }
        if (arg[1] == 'i' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.input = argv[++i];
            if (!is_valid_buffer_spec(opt.input)) {
                return result_err<Options>(Status::Invalid);
            }
            continue;
        }
        if (arg[1] == 'o' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.output = argv[++i];
            if (!is_valid_buffer_spec(opt.output)) {
                return result_err<Options>(Status::Invalid);
            }
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool has_command(int argc, int optind) noexcept {
    return argc - optind >= 1;
}

} // namespace pbsd::userland::usr_bin::stdbuf
