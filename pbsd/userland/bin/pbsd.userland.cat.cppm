module;
#include <cstddef>

export module pbsd.userland.cat;

export import pbsd.core;
export import pbsd.userland.hosted;
export import pbsd.userland.capsicum.helpers;

/// Port helpers from hbsd/src/bin/cat/cat.c — flag parse (Capsicum/casper at run site).
export namespace pbsd::userland::bin::cat {

struct Options {
    bool number_nonblank{false}; // -b
    bool show_ends{false};       // -e (implies -v)
    bool number_all{false};      // -n
    bool squeeze_blank{false};   // -s
    bool show_tabs{false};       // -t (implies -v)
    bool show_nonprinting{false};// -v
};

[[nodiscard]] inline Result<Options> parse_options(int argc, char* const* argv,
                                                   int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-' && argv[i][1] != '\0'; ++i) {
        if (argv[i][1] == '-' && argv[i][2] == '\0') {
            ++i;
            break;
        }
        // lone "-" means stdin — stop option parse
        if (argv[i][1] == '\0') {
            break;
        }
        for (const char* p = argv[i] + 1; *p; ++p) {
            switch (*p) {
            case 'b':
                opt.number_nonblank = true;
                break;
            case 'e':
                opt.show_ends = true;
                opt.show_nonprinting = true;
                break;
            case 'n':
                opt.number_all = true;
                break;
            case 's':
                opt.squeeze_blank = true;
                break;
            case 't':
                opt.show_tabs = true;
                opt.show_nonprinting = true;
                break;
            case 'u':
                break; // historical no-op
            case 'v':
                opt.show_nonprinting = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    if (opt.number_nonblank) {
        opt.number_all = false; // -b overrides -n numbering of blanks
    }
    optind_out = i;
    return result_ok(opt);
}

/// Raw copy loop helper: write buffer to fd; SI-2 requires explicit out_fd.
[[nodiscard]] inline StatusOnly raw_write(int out_fd, const char* buf, std::size_t len) noexcept {
    if (out_fd < 0 || buf == nullptr) {
        return status_err(Status::Invalid);
    }
    if (len == 0) {
        return status_ok();
    }
    return hosted::write_all(out_fd, buf, len);
}

[[nodiscard]] inline bool is_stdin_path(const char* path) noexcept {
    return path == nullptr || hosted::cstrcmp(path, "-") == 0;
}

[[nodiscard]] inline bool wants_cooked_mode(const Options& opt) noexcept {
    return opt.number_nonblank || opt.number_all || opt.show_ends ||
           opt.squeeze_blank || opt.show_tabs || opt.show_nonprinting;
}

[[nodiscard]] inline StatusOnly run(int argc, char* const* argv, int stdout_fd = 1) noexcept {
    if (capsicum::limit_stdio() != Status::Ok ||
        capsicum::enter_sandbox() != Status::Ok) {
        return status_err(Status::Denied);
    }
    int optind = 1;
    const auto parsed = parse_options(argc, argv, optind);
    if (!parsed.has_value()) {
        return status_err(parsed.status);
    }
    (void)stdout_fd;
    (void)parsed.value;
    if (argv == nullptr || argv[optind] == nullptr) {
        return status_ok();
    }
    return status_ok();
}

} // namespace pbsd::userland::bin::cat
