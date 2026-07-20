module;
#include <cstddef>

export module pbsd.userland.ldd;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/ldd/ldd.c — Shared lib dependency list stub
export namespace pbsd::userland::usr_bin::ldd {

struct Options {
    bool verbose{false};
    bool list_all{false};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        if (hosted::cstrcmp(argv[i], "-v") == 0) {
            opt.verbose = true;
            continue;
        }
        if (hosted::cstrcmp(argv[i], "-a") == 0) {
            opt.list_all = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool looks_shared_object(const char* path) noexcept {
    if (path == nullptr) {
        return false;
    }
    const char* dot = nullptr;
    for (const char* p = path; *p; ++p) {
        if (*p == '.') {
            dot = p;
        }
    }
    return dot != nullptr && hosted::cstrcmp(dot, ".so") == 0;
}

} // namespace pbsd::userland::usr_bin::ldd
