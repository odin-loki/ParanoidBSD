module;
#include <cstddef>

export module pbsd.userland.mktemp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mktemp/mktemp.c — flag parse (logic-only).
export namespace pbsd::userland::usr_bin::mktemp {

struct Options {
    bool directory{false};
    bool quiet{false};
    bool template_mode{false};
    bool dry_run{false};
    const char* tmpdir{nullptr};
    const char* prefix{"mktemp"};
    bool prefer_tmpdir{true};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* flag = argv[i];
        if (hosted::cstrcmp(flag, "-d") == 0 || hosted::cstrcmp(flag, "--directory") == 0) {
            opt.directory = true;
            continue;
        }
        if (hosted::cstrcmp(flag, "-q") == 0 || hosted::cstrcmp(flag, "--quiet") == 0) {
            opt.quiet = true;
            continue;
        }
        if (hosted::cstrcmp(flag, "-u") == 0 || hosted::cstrcmp(flag, "--dry-run") == 0) {
            opt.dry_run = true;
            continue;
        }
        if (flag[1] == 'p' && flag[2] != '\0') {
            opt.tmpdir = flag + 2;
            opt.prefer_tmpdir = false;
            continue;
        }
        if (flag[1] == 't' && flag[2] != '\0') {
            opt.prefix = flag + 2;
            opt.template_mode = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (!opt.template_mode && i >= argc) {
        opt.template_mode = true;
        opt.prefix = "tmp";
        opt.prefer_tmpdir = false;
    }
    return result_ok(opt);
}

[[nodiscard]] inline bool has_trailing_x(const char* tmpl) noexcept {
    if (tmpl == nullptr) {
        return false;
    }
    std::size_t len = hosted::cstrlen(tmpl);
    if (len < 6) {
        return false;
    }
    for (std::size_t i = len - 6; i < len; ++i) {
        if (tmpl[i] != 'X') {
            return false;
        }
    }
    return true;
}

} // namespace pbsd::userland::usr_bin::mktemp
