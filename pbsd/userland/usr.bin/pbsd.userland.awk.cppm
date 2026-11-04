module;
#include <cstddef>

export module pbsd.userland.awk;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Stub helpers from hbsd/src/usr.bin/awk — program/field parse scaffold (logic-only).
export namespace pbsd::userland::usr_bin::awk {

struct Options {
    const char* program{nullptr};
    const char* field_sep{nullptr};
    const char* record_sep{nullptr};
    const char* var_assign{nullptr};
    bool safe{false};
    bool sandbox{false};
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
        if (arg[1] == 'F' && arg[2] != '\0') {
            opt.field_sep = arg + 2;
            continue;
        }
        if (arg[1] == 'F' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.field_sep = argv[++i];
            continue;
        }
        if (arg[1] == 'f' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            ++i;
            continue;
        }
        if (arg[1] == 'v' && arg[2] != '\0') {
            opt.var_assign = arg + 2;
            continue;
        }
        if (arg[1] == 'v' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.var_assign = argv[++i];
            continue;
        }
        if (hosted::cstrcmp(arg, "-S") == 0) {
            opt.safe = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-s") == 0) {
            opt.sandbox = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (i < argc && opt.program == nullptr) {
        opt.program = argv[i++];
    }
    return result_ok(opt);
}

[[nodiscard]] inline std::size_t split_fields(const char* line, char sep,
                                              char* fields[], std::size_t max_fields) noexcept {
    if (line == nullptr || fields == nullptr || max_fields == 0) {
        return 0;
    }
    std::size_t count = 0;
    fields[count++] = const_cast<char*>(line);
    for (const char* p = line; *p && count < max_fields; ++p) {
        if (*p == sep) {
            fields[count++] = const_cast<char*>(p + 1);
        }
    }
    return count;
}

} // namespace pbsd::userland::usr_bin::awk
