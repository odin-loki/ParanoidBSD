module;
#include <cstddef>

export module pbsd.userland.tee;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/tee/tee.c.
export namespace pbsd::userland::usr_bin::tee {

struct Options {
    bool append{false}; // -a
    bool ignore_intr{false}; // -i
};

[[nodiscard]] inline Result<Options> parse_options(int argc, char* const* argv,
                                                   int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-' && argv[i][1] != '\0'; ++i) {
        for (const char* p = argv[i] + 1; *p; ++p) {
            switch (*p) {
            case 'a':
                opt.append = true;
                break;
            case 'i':
                opt.ignore_intr = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

/// Fan-out write: copy buf to each fd in list. SI-2: explicit fds.
[[nodiscard]] inline StatusOnly write_all_fds(const int* fds, int nfd, const char* buf,
                                              std::size_t len) noexcept {
    if (fds == nullptr || nfd <= 0 || buf == nullptr) {
        return status_err(Status::Invalid);
    }
    for (int i = 0; i < nfd; ++i) {
        if (fds[i] < 0) {
            return status_err(Status::Invalid);
        }
        auto st = hosted::write_all(fds[i], buf, len);
        if (!st.has_value()) {
            return st;
        }
    }
    return status_ok();
}

} // namespace pbsd::userland::usr_bin::tee
