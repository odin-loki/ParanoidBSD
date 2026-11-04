module;
#include <cstddef>

export module pbsd.userland.freebsd_version;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/freebsd-version/freebsd-version.sh.in — version flags (logic-only).
export namespace pbsd::userland::bin::freebsd_version {

struct Options {
    bool kernel_installed{false};
    bool kernel_running{false};
    bool userland{false};
    bool jail{false};
    const char* jail_name{nullptr};
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
        if (hosted::cstrcmp(arg, "-k") == 0) {
            opt.kernel_installed = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-r") == 0) {
            opt.kernel_running = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-u") == 0) {
            opt.userland = true;
            continue;
        }
        if (arg[1] == 'j' && arg[2] != '\0') {
            opt.jail = true;
            opt.jail_name = arg + 2;
            continue;
        }
        if (hosted::cstrcmp(arg, "-j") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.jail = true;
            opt.jail_name = argv[++i];
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (i < argc) {
        return result_err<Options>(Status::Invalid);
    }
    const int flag_count = (opt.kernel_installed ? 1 : 0) + (opt.kernel_running ? 1 : 0) +
                           (opt.userland ? 1 : 0) + (opt.jail ? 1 : 0);
    if (flag_count == 0) {
        opt.userland = true;
    }
    return result_ok(opt);
}

[[nodiscard]] inline bool valid_version_token(const char* token) noexcept {
    if (token == nullptr || token[0] == '\0') {
        return false;
    }
    for (const char* p = token; *p != '\0'; ++p) {
        const char c = *p;
        if (!((c >= '0' && c <= '9') || c == '.' || c == '-' ||
              (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')) {
            return false;
        }
    }
    return true;
}

} // namespace pbsd::userland::bin::freebsd_version
