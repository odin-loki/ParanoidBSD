module;

#if defined(__FreeBSD__) || defined(__linux__) || defined(__APPLE__)
#include <sys/stat.h>
#define PBSD_PWD_STAT 1
#endif

export module pbsd.userland.pwd;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pwd/pwd.c
export namespace pbsd::userland::bin::pwd {

enum class PathMode : unsigned char {
    Logical,  // -L (default)
    Physical, // -P
};

struct Options {
    PathMode mode{PathMode::Logical};
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
            return result_err<Options>(Status::Invalid);
        }
        switch (flag[1]) {
        case 'L':
            opt.mode = PathMode::Logical;
            break;
        case 'P':
            opt.mode = PathMode::Physical;
            break;
        default:
            return result_err<Options>(Status::Invalid);
        }
        ++i;
    }

    if (argc - i != 0) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

#if defined(PBSD_PWD_STAT)

[[nodiscard]] inline bool pwd_has_dot_segments(const char* pwd) noexcept {
    const char* p = pwd;
    while (*p != '\0') {
        const char* q = ++p;
        while (*q != '\0' && *q != '/') {
            ++q;
        }
        if ((*p == '.' && q == p + 1) ||
            (*p == '.' && q == p + 2 && p[1] == '.')) {
            return true;
        }
        p = q;
    }
    return false;
}

[[nodiscard]] inline bool pwd_refers_to_dot(const char* pwd) noexcept {
    struct stat log {};
    struct stat phy {};
    if (stat(pwd, &log) != 0 || stat(".", &phy) != 0) {
        return false;
    }
    return log.st_dev == phy.st_dev && log.st_ino == phy.st_ino;
}

#endif

[[nodiscard]] inline const char* getcwd_logical() noexcept {
#if defined(PBSD_PWD_STAT)
    const char* pwd = hosted::getenv_cstr("PWD");
    if (pwd == nullptr || pwd[0] != '/') {
        return nullptr;
    }
    if (pwd_has_dot_segments(pwd)) {
        return nullptr;
    }
    if (!pwd_refers_to_dot(pwd)) {
        return nullptr;
    }
    return pwd;
#else
    return nullptr;
#endif
}

struct PathOutput {
    char buf[hosted::kMaxPathLen]{};
    const char* path{nullptr};
};

[[nodiscard]] inline Result<PathOutput> get_path(PathMode mode) noexcept {
    PathOutput out{};
    if (mode == PathMode::Logical) {
        const char* logical = getcwd_logical();
        if (logical != nullptr) {
            out.path = logical;
            return result_ok(out);
        }
    }
    const auto st = hosted::get_cwd(out.buf, sizeof(out.buf));
    if (!ok(st.status)) {
        return result_err<PathOutput>(st.status);
    }
    out.path = out.buf;
    return result_ok(out);
}

[[nodiscard]] inline StatusOnly run(int argc, char* const* argv, int stdout_fd = 1) noexcept {
    const auto parsed = parse_args(argc, argv);
    if (!parsed.has_value()) {
        return status_err(parsed.status);
    }

    const auto path = get_path(parsed.value.mode);
    if (!path.has_value()) {
        return status_err(path.status);
    }
    return hosted::write_line(stdout_fd, path.value.path);
}

} // namespace pbsd::userland::bin::pwd
