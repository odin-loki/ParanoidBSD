module;
#include <cstddef>

export module pbsd.userland.cp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/cp/cp.c — option parsing and operation classification.
export namespace pbsd::userland::bin::cp {

enum class FollowMode : unsigned char {
    Physical,
    Logical,
    PhysicalFollowArgDir,
};

enum class Operation : unsigned char {
    FileToFile,
    FileToDir,
    DirToDne,
};

struct Options {
    FollowMode follow{FollowMode::Physical};
    bool recursive{false};
    bool archive{false};
    bool force{false};
    bool interactive{false};
    bool link{false};
    bool no_clobber{false};
    bool preserve{false};
    bool symbolic_link{false};
    bool verbose{false};
    bool one_file_system{false};
};

[[nodiscard]] inline Result<Options> parse_short_flag(Options& opt, char ch) noexcept {
    switch (ch) {
    case 'H':
        opt.follow = FollowMode::PhysicalFollowArgDir;
        return result_ok(opt);
    case 'L':
        opt.follow = FollowMode::Logical;
        return result_ok(opt);
    case 'P':
        opt.follow = FollowMode::Physical;
        return result_ok(opt);
    case 'R':
    case 'r':
        opt.recursive = true;
        return result_ok(opt);
    case 'a':
        opt.archive = true;
        opt.recursive = true;
        opt.preserve = true;
        opt.follow = FollowMode::Physical;
        return result_ok(opt);
    case 'f':
        opt.force = true;
        opt.interactive = false;
        opt.no_clobber = false;
        return result_ok(opt);
    case 'i':
        opt.interactive = true;
        opt.force = false;
        opt.no_clobber = false;
        return result_ok(opt);
    case 'l':
        opt.link = true;
        return result_ok(opt);
    case 'n':
        opt.no_clobber = true;
        opt.force = false;
        opt.interactive = false;
        return result_ok(opt);
    case 'p':
        opt.preserve = true;
        return result_ok(opt);
    case 's':
        opt.symbolic_link = true;
        return result_ok(opt);
    case 'v':
        opt.verbose = true;
        return result_ok(opt);
    case 'x':
        opt.one_file_system = true;
        return result_ok(opt);
    default:
        return result_err<Options>(Status::Invalid);
    }
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }

    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        if (hosted::cstrcmp(flag, "--") == 0) {
            ++i;
            break;
        }
        if (flag[1] == '\0') {
            return result_err<Options>(Status::Invalid);
        }
        if (flag[1] == '-' && flag[2] != '\0') {
            if (hosted::cstrcmp(flag, "--archive") == 0) {
                (void)parse_short_flag(opt, 'a');
            } else if (hosted::cstrcmp(flag, "--recursive") == 0) {
                (void)parse_short_flag(opt, 'R');
            } else {
                return result_err<Options>(Status::Invalid);
            }
            ++i;
            continue;
        }
        for (int j = 1; flag[j] != '\0'; ++j) {
            const auto st = parse_short_flag(opt, flag[j]);
            if (!st.has_value()) {
                return result_err<Options>(st.status);
            }
            opt = st.value;
        }
        ++i;
    }
    if (argc - i < 2) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline Operation classify_operation(int src_count,
                                                  bool target_exists,
                                                  bool target_is_dir) noexcept {
    if (src_count == 1 && !target_exists) {
        return Operation::DirToDne;
    }
    if (target_is_dir) {
        return Operation::FileToDir;
    }
    return Operation::FileToFile;
}

[[nodiscard]] inline unsigned fts_options_from_follow(FollowMode mode) noexcept {
    switch (mode) {
    case FollowMode::Logical:
        return 0x01u; // FTS_LOGICAL
    case FollowMode::PhysicalFollowArgDir:
        return 0x02u | 0x04u; // FTS_PHYSICAL | FTS_COMFOLLOW
    case FollowMode::Physical:
    default:
        return 0x02u;
    }
}

[[nodiscard]] inline bool append_relpath(char* to, std::size_t to_len,
                                         const char* relpath) noexcept {
    if (to == nullptr || relpath == nullptr || to_len == 0) {
        return false;
    }
    std::size_t base = hosted::cstrlen(to);
    std::size_t rlen = hosted::cstrlen(relpath);
    if (base > 0 && to[base - 1] != '/' && base + 1 + rlen + 1 > to_len) {
        return false;
    }
    if (base > 0 && to[base - 1] != '/') {
        to[base++] = '/';
    }
    if (base + rlen + 1 > to_len) {
        return false;
    }
    for (std::size_t i = 0; i < rlen; ++i) {
        to[base + i] = relpath[i];
    }
    to[base + rlen] = '\0';
    return true;
}

} // namespace pbsd::userland::bin::cp
