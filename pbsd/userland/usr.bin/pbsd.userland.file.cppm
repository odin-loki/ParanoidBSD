module;
#include <cstddef>

export module pbsd.userland.file;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/file/file.c — Magic sniff + -b/-i/-L
export namespace pbsd::userland::usr_bin::file {

struct Options {
    bool brief{false};
    bool mime{false};
    bool follow_links{false};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        for (const char* p = argv[i] + 1; *p; ++p) {
            switch (*p) {
            case 'b':
                opt.brief = true;
                break;
            case 'i':
                opt.mime = true;
                break;
            case 'L':
                opt.follow_links = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool looks_elf(const unsigned char* buf, std::size_t len) noexcept {
    return buf != nullptr && len >= 4 && buf[0] == 0x7f && buf[1] == 'E' && buf[2] == 'L'
           && buf[3] == 'F';
}

} // namespace pbsd::userland::usr_bin::file
