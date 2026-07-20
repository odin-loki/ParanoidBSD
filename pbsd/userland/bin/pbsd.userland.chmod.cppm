module;
#include <cstddef>

export module pbsd.userland.chmod;

export import pbsd.core;
export import pbsd.userland.hosted;
export import pbsd.userland.util.filemode;

/// Port of hbsd/src/bin/chmod/chmod.c — option and FTS mode logic.
export namespace pbsd::userland::bin::chmod {

enum class FollowMode : unsigned char {
    Logical,
    Physical,
    PhysicalFollowArgDir, // -H with -R
};

enum class FtsMode : unsigned char {
    Logical,
    Physical,
    PhysicalComfollow,
};

struct Options {
    FollowMode follow{FollowMode::Logical};
    bool recursive{false};
    bool force{false};
    bool no_dereference{false}; // -h
    int verbose{0};
    const char* mode{nullptr};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }

    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        if (flag[1] == '\0') {
            return result_err<Options>(Status::Invalid);
        }
        if (flag[1] == 'r' || flag[1] == 'w' || flag[1] == 'x' || flag[1] == 'g' ||
            flag[1] == 'o' || flag[1] == 'u' || flag[1] == 's' || flag[1] == 't' ||
            flag[1] == 'X') {
            if (flag[2] == '\0') {
                goto done_flags;
            }
        }
        for (int j = 1; flag[j] != '\0'; ++j) {
            switch (flag[j]) {
            case 'H':
                opt.follow = FollowMode::PhysicalFollowArgDir;
                break;
            case 'L':
                opt.follow = FollowMode::Logical;
                break;
            case 'P':
                opt.follow = FollowMode::Physical;
                break;
            case 'R':
                opt.recursive = true;
                break;
            case 'f':
                opt.force = true;
                break;
            case 'h':
                opt.no_dereference = true;
                break;
            case 'v':
                ++opt.verbose;
                break;
            case 'g':
            case 'o':
            case 'r':
            case 's':
            case 't':
            case 'u':
            case 'w':
            case 'X':
            case 'x':
                goto done_flags;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
        ++i;
    }
done_flags:
    if (i >= argc - 1) {
        return result_err<Options>(Status::Invalid);
    }
    opt.mode = argv[i];
    ++i;
    if (i >= argc) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline Result<FtsMode> fts_mode_from_options(const Options& opt) noexcept {
    if (opt.recursive && opt.no_dereference) {
        return result_err<FtsMode>(Status::Invalid);
    }
    if (opt.recursive) {
        if (opt.follow == FollowMode::Logical) {
            return result_ok(FtsMode::Logical);
        }
        if (opt.follow == FollowMode::PhysicalFollowArgDir) {
            return result_ok(FtsMode::PhysicalComfollow);
        }
        return result_ok(FtsMode::Physical);
    }
    if (opt.no_dereference) {
        return result_ok(FtsMode::Physical);
    }
    return result_ok(FtsMode::Logical);
}

[[nodiscard]] inline bool should_skip_dir_descent(bool recursive) noexcept {
    return !recursive;
}

[[nodiscard]] inline bool mode_unchanged(unsigned old_mode, unsigned new_mode,
                                         bool may_have_nfs4acl) noexcept {
    constexpr unsigned kAllPerms = 07777u;
    if (may_have_nfs4acl) {
        return false;
    }
    return (new_mode & kAllPerms) == (old_mode & kAllPerms);
}

[[nodiscard]] inline Result<unsigned> parse_mode_arg(const char* mode) noexcept {
    return util::filemode::parse_octal_mode(mode);
}

[[nodiscard]] inline unsigned compute_new_mode(unsigned st_mode,
                                               unsigned template_mode) noexcept {
    return util::filemode::apply_mode_template(st_mode, template_mode);
}

} // namespace pbsd::userland::bin::chmod
