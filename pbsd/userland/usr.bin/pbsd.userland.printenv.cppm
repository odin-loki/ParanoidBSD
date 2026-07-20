module;
#include <cstddef>

export module pbsd.userland.printenv;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/printenv/printenv.c — env lookup (logic-only).
export namespace pbsd::userland::usr_bin::printenv {

struct Options {
    const char* name{nullptr}; // nullptr => dump all
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        if (argv[i][1] != '\0') {
            return result_err<Options>(Status::Invalid);
        }
        ++i;
    }
    optind_out = i;
    const int remaining = argc - i;
    if (remaining > 1) {
        return result_err<Options>(Status::Invalid);
    }
    Options opt{};
    if (remaining == 1) {
        opt.name = argv[i];
    }
    return result_ok(opt);
}

[[nodiscard]] inline const char* lookup_env(char* const* environ,
                                            const char* name) noexcept {
    if (environ == nullptr || name == nullptr) {
        return nullptr;
    }
    const std::size_t len = hosted::cstrlen(name);
    for (char* const* ep = environ; *ep != nullptr; ++ep) {
        const char* entry = *ep;
        bool match = true;
        for (std::size_t j = 0; j < len; ++j) {
            if (entry[j] != name[j]) {
                match = false;
                break;
            }
        }
        if (!match) {
            continue;
        }
        const char* cp = entry + len;
        if (*cp == '=') {
            return cp + 1;
        }
    }
    return nullptr;
}

} // namespace pbsd::userland::usr_bin::printenv
