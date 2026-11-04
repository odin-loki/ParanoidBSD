module;
#include <cstddef>

export module pbsd.userland.pax;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pax/pax.h — archive operation constants (logic-only).
export namespace pbsd::userland::bin::pax {

inline constexpr int kMaxBlk = 64512;
inline constexpr int kMaxBlkPosix = 32256;
inline constexpr int kBlkMult = 512;
inline constexpr int kDevBlk = 8192;
inline constexpr int kFileBlk = 10240;
inline constexpr int kPaxPathLen = 3072;

inline constexpr int kOpList = 0;
inline constexpr int kOpExtract = 1;
inline constexpr int kOpArchive = 2;
inline constexpr int kOpAppend = 3;
inline constexpr int kOpCopy = 4;
inline constexpr int kOpDefault = kOpList;

inline constexpr int kDevReg = 0;
inline constexpr int kDevTypeChr = 1;
inline constexpr int kDevTypeBlk = 2;
inline constexpr int kDevTape = 3;
inline constexpr int kDevPipe = 4;

struct Options {
    int operation{kOpDefault};
    bool read_write{false};
    bool verbose{false};
    const char* archive_path{nullptr};
};

[[nodiscard]] inline bool valid_blocksize(int size) noexcept {
    if (size <= 0 || size > kMaxBlk) {
        return false;
    }
    return (size % kBlkMult) == 0;
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* arg = argv[i];
        if (arg[1] == 'f' && arg[2] != '\0') {
            opt.archive_path = arg + 2;
            continue;
        }
        if (hosted::cstrcmp(arg, "-f") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.archive_path = argv[++i];
            continue;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'v':
                opt.verbose = true;
                break;
            case 'r':
                opt.read_write = true;
                opt.operation = kOpExtract;
                break;
            case 'w':
                opt.read_write = true;
                opt.operation = kOpArchive;
                break;
            case 'a':
                opt.operation = kOpAppend;
                break;
            default:
                break;
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::bin::pax
