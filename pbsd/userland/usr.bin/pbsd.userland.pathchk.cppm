module;
#include <cstddef>

export module pbsd.userland.pathchk;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/pathchk/pathchk.c — pathname validation (logic-only).
export namespace pbsd::userland::usr_bin::pathchk {

inline constexpr long kPosixNameMax = 14;
inline constexpr long kPosixPathMax = 256;

struct Options {
    bool portable{false};
    bool check_empty{false};
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
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'p':
                opt.portable = true;
                break;
            case 'P':
                opt.check_empty = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    if (argc - i < 1) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline bool is_empty_path(const char* path) noexcept {
    return path != nullptr && path[0] == '\0';
}

[[nodiscard]] inline bool has_dash_component(const char* path) noexcept {
    if (path == nullptr) {
        return false;
    }
    if (path[0] == '-') {
        return true;
    }
    for (const char* p = path; *p != '\0'; ++p) {
        if (p[0] == '/' && p[1] == '-') {
            return true;
        }
    }
    return false;
}

[[nodiscard]] inline bool is_portable_component(const char* component) noexcept {
    static constexpr const char charset[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789._-";
    if (component == nullptr) {
        return false;
    }
    for (const char* p = component; *p != '\0'; ++p) {
        bool found = false;
        for (const char* c = charset; *c != '\0'; ++c) {
            if (*p == *c) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline int component_length(const char* path) noexcept {
    if (path == nullptr) {
        return 0;
    }
    int len = 0;
    while (path[len] != '\0' && path[len] != '/') {
        ++len;
    }
    return len;
}

[[nodiscard]] inline bool component_too_long(const char* component, long limit) noexcept {
    if (limit < 0) {
        return false;
    }
    return component_length(component) > limit;
}

} // namespace pbsd::userland::usr_bin::pathchk
