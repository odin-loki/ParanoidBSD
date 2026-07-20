module;
#include <cstddef>

export module pbsd.userland.getfacl;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/getfacl/getfacl.c — option parsing (logic-only).
export namespace pbsd::userland::bin::getfacl {

enum class AclType : unsigned char {
    Access,
    Default,
};

struct Options {
    AclType type{AclType::Access};
    bool follow_symlink{false};
    bool append_id{false};
    bool numeric{false};
    bool omit_header{false};
    bool skip_trivial{false};
    bool verbose{false};
    bool read_stdin{false};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }

    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        if (flag[1] == '-' && flag[2] == '\0') {
            ++i;
            break;
        }
        for (int j = 1; flag[j] != '\0'; ++j) {
            switch (flag[j]) {
            case 'd':
                opt.type = AclType::Default;
                break;
            case 'h':
                opt.follow_symlink = true;
                break;
            case 'i':
                opt.append_id = true;
                break;
            case 'n':
                opt.numeric = true;
                break;
            case 'q':
                opt.omit_header = true;
                break;
            case 's':
                opt.skip_trivial = true;
                break;
            case 'v':
                opt.verbose = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
        ++i;
    }

    if (i >= argc) {
        opt.read_stdin = true;
        return result_ok(opt);
    }
    if (argv[i] != nullptr && hosted::cstrcmp(argv[i], "-") == 0) {
        opt.read_stdin = true;
    }
    return result_ok(opt);
}

[[nodiscard]] inline bool wants_default_acl(const Options& opt) noexcept {
    return opt.type == AclType::Default;
}

[[nodiscard]] inline unsigned acl_text_flags(const Options& opt) noexcept {
    unsigned flags = 0;
    if (opt.append_id) {
        flags |= 1u;
    }
    if (opt.numeric) {
        flags |= 2u;
    }
    if (opt.verbose) {
        flags |= 4u;
    }
    return flags;
}

[[nodiscard]] inline bool strip_stdin_line(char* line) noexcept {
    if (line == nullptr) {
        return false;
    }
    for (char* p = line; *p != '\0'; ++p) {
        if (*p == '\n') {
            *p = '\0';
            return true;
        }
    }
    return true;
}

} // namespace pbsd::userland::bin::getfacl
