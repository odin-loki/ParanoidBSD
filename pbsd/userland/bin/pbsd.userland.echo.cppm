module;
#include <cstddef>

export module pbsd.userland.echo;

export import pbsd.core;
export import pbsd.userland.hosted;
export import pbsd.userland.capsicum.helpers;

/// Port of hbsd/src/bin/echo/echo.c — Capsicum-first, Status errors.
export namespace pbsd::userland::bin::echo {

struct Options {
    bool no_newline{false};
};

/// Parse -n flag. This utility may NOT do getopt(3) option parsing (BSD echo).
[[nodiscard]] inline Result<Options> parse_options(char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr || argv[0] == nullptr) {
        return result_ok(opt);
    }
    if (hosted::cstrcmp(argv[0], "-n") == 0) {
        opt.no_newline = true;
    }
    return result_ok(opt);
}

[[nodiscard]] inline char* const* args_after_options(char* const* argv, const Options& opt) noexcept {
    if (argv == nullptr) {
        return nullptr;
    }
    if (opt.no_newline && argv[0] != nullptr && hosted::cstrcmp(argv[0], "-n") == 0) {
        return argv + 1;
    }
    return argv;
}

/// Build iovec sequence and write to stdout. Preserves echo.c spacing and \c semantics.
[[nodiscard]] inline StatusOnly write_args(int stdout_fd, char* const* argv,
                                           Options opt) noexcept {
    if (stdout_fd < 0) {
        return status_err(Status::Invalid);
    }

    hosted::Iovec stack_iov[256];
    hosted::Iovec* iov = stack_iov;
    int veclen = 0;
    const int max_stack = static_cast<int>(sizeof(stack_iov) / sizeof(stack_iov[0]));

    char space[] = " ";
    char newline[] = "\n";

    while (argv != nullptr && argv[0] != nullptr) {
        char* arg = argv[0];
        std::size_t len = hosted::cstrlen(arg);

        if (argv[1] == nullptr) {
            if (len >= 2 && arg[len - 2] == '\\' && arg[len - 1] == 'c') {
                len -= 2;
                opt.no_newline = true;
            }
        }

        if (veclen + 2 > max_stack) {
            return status_err(Status::Invalid);
        }
        iov[veclen].base = arg;
        iov[veclen].len = len;
        ++veclen;

        if (argv[1] != nullptr) {
            iov[veclen].base = space;
            iov[veclen].len = 1;
            ++veclen;
        }
        ++argv;
    }

    if (!opt.no_newline) {
        if (veclen + 1 > max_stack) {
            return status_err(Status::Invalid);
        }
        iov[veclen].base = newline;
        iov[veclen].len = 1;
        ++veclen;
    }

    return hosted::writev_all(stdout_fd, iov, veclen);
}

/// Full echo run: caph_limit_stdio + caph_enter + write.
[[nodiscard]] inline StatusOnly run(char* const* argv) noexcept {
    if (capsicum::limit_stdio() != Status::Ok) {
        return status_err(Status::Denied);
    }
    if (capsicum::enter_sandbox() != Status::Ok) {
        return status_err(Status::Denied);
    }

    const auto parsed = parse_options(argv);
    if (!parsed.has_value()) {
        return status_err(parsed.status);
    }
    Options opt = parsed.value;
    char* const* args = args_after_options(argv, opt);
    return write_args(1, args, opt);
}

} // namespace pbsd::userland::bin::echo
