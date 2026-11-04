module;
#include <cstddef>

export module pbsd.userland.umount;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/sbin/umount/umount.c — flag parse (logic-only).
export namespace pbsd::userland::sbin::umount {

inline constexpr int kMntForce = 0x00080000;
inline constexpr int kMntNonBusy = 0x00200000;

struct Options {
    int all{0};
    bool debug{false};
    bool verbose{false};
    bool nfs_force{false};
    int force_flags{0};
    const char* fstab{nullptr};
    const char* nfs_host{nullptr};
    char** typelist{nullptr};
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
        for (int j = 1; flag[j] != '\0'; ++j) {
            switch (flag[j]) {
            case 'A':
                opt.all = 2;
                break;
            case 'a':
                opt.all = 1;
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
                opt.force_flags |= kMntForce;
                break;
            case 'h':
                opt.all = 2;
                if (flag[j + 1] == '\0') {
                    if (i + 1 >= argc) {
                        return result_err<Options>(Status::Invalid);
                    }
                    opt.nfs_host = argv[++i];
                    goto next_flag;
                }
                opt.nfs_host = flag + j + 1;
                goto next_flag;
            case 'N':
                opt.nfs_force = true;
                break;
            case 'n':
                opt.force_flags |= kMntNonBusy;
                break;
            case 't':
                return result_err<Options>(Status::Invalid);
            case 'v':
                opt.verbose = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    next_flag:;
    }
    optind_out = i;
    const int nargs = argc - i;
    if ((nargs == 0 && opt.all == 0) || (nargs != 0 && opt.all != 0)) {
        return result_err<Options>(Status::Invalid);
    }
    if ((opt.force_flags & kMntForce) != 0 && (opt.force_flags & kMntNonBusy) != 0) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::sbin::umount
