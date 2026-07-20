module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.chown;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.sbin/chown/chown.c — owner/group parse + FTS helpers.
export namespace pbsd::userland::sbin::chown {

enum class FollowMode : unsigned char {
    Logical,
    Physical,
    PhysicalFollowArgDir,
};

enum class Mode : unsigned char {
    Chown,
    Chgrp,
};

struct OwnerSpec {
    std::int64_t uid{-1};
    std::int64_t gid{-1};
    bool uid_set{false};
    bool gid_set{false};
};

struct Options {
    Mode mode{Mode::Chown};
    FollowMode follow{FollowMode::Logical};
    bool recursive{false};
    bool force{false};
    bool no_dereference{false};
    int verbose{0};
    bool cross_dev{false};
    OwnerSpec owner{};
};

[[nodiscard]] inline Result<std::int64_t> parse_id(const char* name) noexcept {
    if (name == nullptr || name[0] == '\0') {
        return result_err<std::int64_t>(Status::Invalid);
    }
    std::int64_t val = 0;
    for (const char* p = name; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return result_err<std::int64_t>(Status::Invalid);
        }
        val = val * 10 + (*p - '0');
    }
    if (val > 4294967295LL) {
        return result_err<std::int64_t>(Status::Invalid);
    }
    return result_ok(val);
}

[[nodiscard]] inline Result<OwnerSpec> parse_owner_spec(const char* spec,
                                                      Mode mode) noexcept {
    if (spec == nullptr || spec[0] == '\0') {
        return result_err<OwnerSpec>(Status::Invalid);
    }
    OwnerSpec out{};

    if (mode == Mode::Chgrp) {
        const auto gid = parse_id(spec);
        if (!gid.has_value()) {
            return result_err<OwnerSpec>(gid.status);
        }
        out.gid = gid.value;
        out.gid_set = true;
        return result_ok(out);
    }

    char buf[256];
    std::size_t len = hosted::cstrlen(spec);
    if (len >= sizeof(buf)) {
        return result_err<OwnerSpec>(Status::Invalid);
    }
    for (std::size_t i = 0; i <= len; ++i) {
        buf[i] = spec[i];
    }

    char* colon = nullptr;
    for (char* p = buf; *p != '\0'; ++p) {
        if (*p == ':') {
            colon = p;
            break;
        }
    }

    if (colon != nullptr) {
        *colon++ = '\0';
        if (*colon != '\0') {
            const auto gid = parse_id(colon);
            if (!gid.has_value()) {
                return result_err<OwnerSpec>(gid.status);
            }
            out.gid = gid.value;
            out.gid_set = true;
        }
    }

    if (buf[0] != '\0') {
        const auto uid = parse_id(buf);
        if (!uid.has_value()) {
            return result_err<OwnerSpec>(uid.status);
        }
        out.uid = uid.value;
        out.uid_set = true;
    } else if (!out.gid_set) {
        return result_err<OwnerSpec>(Status::Invalid);
    }

    return result_ok(out);
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                const char* progname) noexcept {
    Options opt{};
    if (argv == nullptr || progname == nullptr) {
        return result_err<Options>(Status::Invalid);
    }

    const char* base = progname;
    for (const char* p = progname; *p != '\0'; ++p) {
        if (*p == '/' || *p == '\\') {
            base = p + 1;
        }
    }
    if (hosted::cstrcmp(base, "chgrp") == 0) {
        opt.mode = Mode::Chgrp;
    }

    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        if (flag[1] == '\0') {
            return result_err<Options>(Status::Invalid);
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
            case 'x':
                opt.cross_dev = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
        ++i;
    }

    if (i >= argc - 1) {
        return result_err<Options>(Status::Invalid);
    }

    const auto owner = parse_owner_spec(argv[i], opt.mode);
    if (!owner.has_value()) {
        return result_err<Options>(owner.status);
    }
    opt.owner = owner.value;
    ++i;
    if (i >= argc) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline Result<unsigned> fts_options_from(const Options& opt) noexcept {
    if (opt.recursive && opt.no_dereference &&
        (opt.follow == FollowMode::Logical ||
         opt.follow == FollowMode::PhysicalFollowArgDir)) {
        return result_err<unsigned>(Status::Invalid);
    }
    unsigned fts = 0;
    if (opt.recursive) {
        if (opt.follow == FollowMode::Logical) {
            fts = 1;
        } else {
            fts = 2;
            if (opt.follow == FollowMode::PhysicalFollowArgDir) {
                fts |= 4;
            }
        }
    } else if (opt.no_dereference) {
        fts = 2;
    } else {
        fts = 1;
    }
    if (opt.cross_dev) {
        fts |= 8;
    }
    return result_ok(fts);
}

[[nodiscard]] inline bool ownership_unchanged(std::int64_t cur_uid, std::int64_t cur_gid,
                                            const OwnerSpec& spec) noexcept {
    const bool uid_ok = !spec.uid_set || spec.uid == cur_uid;
    const bool gid_ok = !spec.gid_set || spec.gid == cur_gid;
    return uid_ok && gid_ok;
}

[[nodiscard]] inline bool should_skip_dir_descent(bool recursive) noexcept {
    return !recursive;
}

} // namespace pbsd::userland::sbin::chown
