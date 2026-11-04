module;
#include <cstddef>

export module pbsd.userland.mount;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/sbin/mount/mount.c — flag parse stubs (logic-only).
export namespace pbsd::userland::sbin::mount {

inline constexpr int kMntForce = 0x00080000;
inline constexpr int kMntUpdate = 0x00010000;

struct Options {
    bool all{false};
    bool debug{false};
    bool late{false};
    bool onlylate{false};
    bool verbose{false};
    bool fstab_style{false};
    bool read_only{false};
    int init_flags{0};
    const char* fstab{nullptr};
    const char* options{nullptr};
    const char* vfstype{"ufs"};
};

[[nodiscard]] inline bool specified_ro(const char* opt) noexcept {
    if (opt == nullptr) {
        return false;
    }
    const char* p = opt;
    while (*p != '\0') {
        if (p[0] == 'r' && p[1] == 'o' && (p[2] == ',' || p[2] == '\0')) {
            return true;
        }
        while (*p != '\0' && *p != ',') {
            ++p;
        }
        if (*p == ',') {
            ++p;
        }
    }
    return false;
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* flag = argv[i];
        for (int j = 1; flag[j] != '\0'; ++j) {
            switch (flag[j]) {
            case 'a':
                opt.all = true;
                break;
            case 'd':
                opt.debug = true;
                break;
            case 'F':
                if (flag[j + 1] == '\0') {
                    if (i + 1 >= argc) {
                        return result_err<Options>(Status::Invalid);
                    }
                    opt.fstab = argv[++i];
                    goto next_flag;
                }
                opt.fstab = flag + j + 1;
                goto next_flag;
            case 'f':
                opt.init_flags |= kMntForce;
                break;
            case 'L':
                opt.onlylate = true;
                opt.late = true;
                break;
            case 'l':
                opt.late = true;
                break;
            case 'n':
                break;
            case 'o':
                if (flag[j + 1] == '\0') {
                    if (i + 1 >= argc) {
                        return result_err<Options>(Status::Invalid);
                    }
                    const char* o = argv[++i];
                    opt.options = o;
                    if (specified_ro(o)) {
                        opt.read_only = true;
                    }
                    goto next_flag;
                }
                opt.options = flag + j + 1;
                if (specified_ro(opt.options)) {
                    opt.read_only = true;
                }
                goto next_flag;
            case 'p':
                opt.fstab_style = true;
                opt.verbose = true;
                break;
            case 'r':
                opt.read_only = true;
                break;
            case 't':
                if (flag[j + 1] == '\0') {
                    if (i + 1 >= argc) {
                        return result_err<Options>(Status::Invalid);
                    }
                    opt.vfstype = argv[++i];
                    goto next_flag;
                }
                opt.vfstype = flag + j + 1;
                goto next_flag;
            case 'u':
                opt.init_flags |= kMntUpdate;
                break;
            case 'v':
                opt.verbose = true;
                break;
            case 'w':
                opt.read_only = false;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    next_flag:;
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool use_mountprog(const char* vfstype) noexcept {
    if (vfstype == nullptr) {
        return false;
    }
    static const char* const kExternal[] = {
        "cd9660", "mfs", "msdosfs", "nfs", "nullfs", "smbfs", "udf", "unionfs", nullptr};
    for (const char* const* p = kExternal; *p != nullptr; ++p) {
        if (hosted::cstrcmp(vfstype, *p) == 0) {
            return true;
        }
    }
    return false;
}

} // namespace pbsd::userland::sbin::mount
