module;
#include <cstddef>

export module pbsd.userland.tar;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/bin/tar — archive flag parse scaffold (logic-only).
export namespace pbsd::userland::bin::tar {

enum class Operation : unsigned char { Create, Extract, List, Append, Unknown };

struct Options {
    Operation op{Operation::Unknown};
    bool verbose{false};
    bool gzip{false};
    bool bzip2{false};
    bool xz{false};
    bool preserve{false};
    const char* file{nullptr};
    const char* directory{nullptr};
};

[[nodiscard]] inline Operation op_from_letter(char c) noexcept {
    switch (c) {
    case 'c':
        return Operation::Create;
    case 'x':
        return Operation::Extract;
    case 't':
        return Operation::List;
    case 'r':
        return Operation::Append;
    default:
        return Operation::Unknown;
    }
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
            opt.file = arg + 2;
            continue;
        }
        if (arg[1] == 'f' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.file = argv[++i];
            continue;
        }
        if (arg[1] == 'C' && arg[2] != '\0') {
            opt.directory = arg + 2;
            continue;
        }
        if (arg[1] == 'C' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.directory = argv[++i];
            continue;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'c':
            case 'x':
            case 't':
            case 'r':
                opt.op = op_from_letter(arg[j]);
                break;
            case 'v':
                opt.verbose = true;
                break;
            case 'z':
                opt.gzip = true;
                break;
            case 'j':
                opt.bzip2 = true;
                break;
            case 'p':
                opt.preserve = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

inline constexpr std::size_t kTarBlockSize = 512;

[[nodiscard]] inline bool tar_header_magic(const char* hdr) noexcept {
    return hdr != nullptr && hdr[257] == 'u' && hdr[258] == 's' && hdr[259] == 't'
           && hdr[260] == 'a' && hdr[261] == 'r';
}

} // namespace pbsd::userland::bin::tar
