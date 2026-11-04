module;
#include <cstddef>

export module pbsd.userland.mkdir;

export import pbsd.core;
export import pbsd.userland.hosted;
export import pbsd.userland.util.filemode;

/// Port of hbsd/src/bin/mkdir/mkdir.c — option parsing and -p build helpers.
export namespace pbsd::userland::bin::mkdir {

[[nodiscard]] inline int cstrncmp(const char* a, const char* b, std::size_t n) noexcept {
    if (a == nullptr || b == nullptr) {
        return (a == b) ? 0 : 1;
    }
    for (std::size_t i = 0; i < n; ++i) {
        if (a[i] != b[i]) {
            return static_cast<unsigned char>(a[i]) - static_cast<unsigned char>(b[i]);
        }
        if (a[i] == '\0') {
            return 0;
        }
    }
    return 0;
}

struct Options {
    bool parents{false};
    bool verbose{false};
    const char* mode_str{nullptr};
    unsigned mode{util::filemode::kDefaultDirMode};
};

enum class BuildOutcome : unsigned char {
    Created,
    AlreadyExists,
    Failed,
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }

    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        if (flag[1] == '\0' || flag[2] != '\0') {
            if (hosted::cstrcmp(flag, "-p") == 0) {
                opt.parents = true;
                ++i;
                continue;
            }
            if (hosted::cstrcmp(flag, "-v") == 0) {
                opt.verbose = true;
                ++i;
                continue;
            }
            if (cstrncmp(flag, "-m", 2) == 0 && flag[2] != '\0') {
                opt.mode_str = flag + 2;
                ++i;
                continue;
            }
            return result_err<Options>(Status::Invalid);
        }
        for (int j = 1; flag[j] != '\0'; ++j) {
            switch (flag[j]) {
            case 'p':
                opt.parents = true;
                break;
            case 'v':
                opt.verbose = true;
                break;
            case 'm':
                if (flag[j + 1] == '\0') {
                    ++i;
                    if (i >= argc || argv[i] == nullptr) {
                        return result_err<Options>(Status::Invalid);
                    }
                    opt.mode_str = argv[i];
                } else {
                    opt.mode_str = flag + j + 1;
                }
                goto done_short;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
        ++i;
    }
done_short:
    if (i >= argc) {
        return result_err<Options>(Status::Invalid);
    }

    if (opt.mode_str != nullptr) {
        const auto mode = util::filemode::parse_octal_mode(opt.mode_str);
        if (!mode.has_value()) {
            return result_err<Options>(mode.status);
        }
        opt.mode = mode.value;
    }
    return result_ok(opt);
}

[[nodiscard]] inline bool needs_sticky_mode_fixup(const Options& opt) noexcept {
    return opt.mode_str != nullptr;
}

/// Walk path segments for mkdir -p (string-only planning pass).
[[nodiscard]] inline Result<unsigned> count_path_segments(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return result_err<unsigned>(Status::Invalid);
    }
    unsigned count = 0;
    const char* p = path;
    if (*p == '/') {
        ++p;
    }
    while (*p != '\0') {
        while (*p == '/') {
            ++p;
        }
        if (*p == '\0') {
            break;
        }
        ++count;
        while (*p != '\0' && *p != '/') {
            ++p;
        }
    }
    if (count == 0) {
        return result_err<unsigned>(Status::Invalid);
    }
    return result_ok(count);
}

[[nodiscard]] inline BuildOutcome classify_existing(bool is_dir, bool is_last) noexcept {
    if (!is_dir) {
        return BuildOutcome::Failed;
    }
    return is_last ? BuildOutcome::AlreadyExists : BuildOutcome::Created;
}

} // namespace pbsd::userland::bin::mkdir
