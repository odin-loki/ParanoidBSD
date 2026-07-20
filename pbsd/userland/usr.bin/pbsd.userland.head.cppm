module;
#include <cstddef>

export module pbsd.userland.head;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/head/head.c.
export namespace pbsd::userland::usr_bin::head {

struct Options {
    std::size_t count{10}; // -n
    bool bytes{false};     // -c (byte mode)
};

[[nodiscard]] inline Result<Options> parse_options(int argc, char* const* argv,
                                                   int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* a = argv[i];
        if (a[1] == '-' && a[2] == '\0') {
            ++i;
            break;
        }
        if (a[1] >= '0' && a[1] <= '9') {
            // Historic: -N means -n N
            std::size_t n = 0;
            for (const char* p = a + 1; *p >= '0' && *p <= '9'; ++p) {
                n = n * 10 + static_cast<std::size_t>(*p - '0');
            }
            opt.count = n;
            continue;
        }
        for (const char* p = a + 1; *p; ++p) {
            switch (*p) {
            case 'n':
            case 'c': {
                opt.bytes = (*p == 'c');
                const char* arg = nullptr;
                if (p[1] != '\0') {
                    arg = p + 1;
                    while (p[1]) {
                        ++p;
                    }
                } else if (i + 1 < argc) {
                    arg = argv[++i];
                } else {
                    return result_err<Options>(Status::Invalid);
                }
                std::size_t n = 0;
                for (const char* q = arg; q && *q; ++q) {
                    if (*q < '0' || *q > '9') {
                        return result_err<Options>(Status::Invalid);
                    }
                    n = n * 10 + static_cast<std::size_t>(*q - '0');
                }
                opt.count = n;
                break;
            }
            case 'q':
            case 'v':
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

/// Count newlines in buffer; return byte offset after `lines` newlines (or len).
[[nodiscard]] inline std::size_t take_lines(const char* buf, std::size_t len,
                                           std::size_t lines) noexcept {
    if (buf == nullptr || lines == 0) {
        return 0;
    }
    std::size_t seen = 0;
    for (std::size_t i = 0; i < len; ++i) {
        if (buf[i] == '\n') {
            ++seen;
            if (seen >= lines) {
                return i + 1;
            }
        }
    }
    return len;
}

} // namespace pbsd::userland::usr_bin::head
