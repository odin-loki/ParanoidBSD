module;
#include <cstddef>

export module pbsd.userland.ssh;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Stub helpers from hbsd/src/usr.bin/ssh — client flag parse scaffold (logic-only).
export namespace pbsd::userland::usr_bin::ssh {

struct Options {
    const char* user{nullptr};
    const char* identity{nullptr};
    const char* port{nullptr};
    const char* bind_addr{nullptr};
    const char* command{nullptr};
    bool batch{false};
    bool quiet{false};
    bool no_host_check{false};
    bool agent_forward{false};
    bool x11_forward{false};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* arg = argv[i];
        if (arg[1] == 'l' && arg[2] != '\0') {
            opt.user = arg + 2;
            continue;
        }
        if (arg[1] == 'l' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.user = argv[++i];
            continue;
        }
        if (arg[1] == 'i' && arg[2] != '\0') {
            opt.identity = arg + 2;
            continue;
        }
        if (arg[1] == 'i' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.identity = argv[++i];
            continue;
        }
        if (arg[1] == 'p' && arg[2] != '\0') {
            opt.port = arg + 2;
            continue;
        }
        if (arg[1] == 'p' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.port = argv[++i];
            continue;
        }
        if (hosted::cstrcmp(arg, "-A") == 0) {
            opt.agent_forward = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-X") == 0) {
            opt.x11_forward = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-q") == 0) {
            opt.quiet = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-o") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            const char* val = argv[++i];
            if (hosted::cstrcmp(val, "BatchMode=yes") == 0) {
                opt.batch = true;
            }
            if (hosted::cstrcmp(val, "StrictHostKeyChecking=no") == 0) {
                opt.no_host_check = true;
            }
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (i < argc) {
        ++i;
        if (i < argc) {
            opt.command = argv[i];
        }
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::ssh
