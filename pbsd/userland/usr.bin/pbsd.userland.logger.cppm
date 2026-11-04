module;
#include <cstddef>

export module pbsd.userland.logger;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/logger/logger.c — priority/tag parse (logic-only).
export namespace pbsd::userland::usr_bin::logger {

struct Options {
    int priority{0x0020}; // LOG_INFO default
    const char* tag{nullptr};
    const char* host{nullptr};
    const char* file{nullptr};
    bool stderr_mode{false};
    bool inet6{true};
    bool send_all{false};
};

[[nodiscard]] inline int priority_from_name(const char* name) noexcept {
    if (name == nullptr) {
        return -1;
    }
    static const struct {
        const char* name;
        int pri;
    } kTable[] = {
        {"emerg", 0},   {"alert", 1},   {"crit", 2},    {"err", 3},
        {"warning", 4}, {"notice", 5},  {"info", 6},    {"debug", 7},
        {"panic", 0},   {"error", 3},   {"warn", 4},    {nullptr, -1},
    };
    for (int i = 0; kTable[i].name != nullptr; ++i) {
        if (hosted::cstrcmp(name, kTable[i].name) == 0) {
            return kTable[i].pri;
        }
    }
    return -1;
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
        if (arg[1] == 'p' && arg[2] != '\0') {
            const int pri = priority_from_name(arg + 2);
            if (pri < 0) {
                return result_err<Options>(Status::Invalid);
            }
            opt.priority = pri;
            continue;
        }
        if (arg[1] == 'p' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            const int pri = priority_from_name(argv[++i]);
            if (pri < 0) {
                return result_err<Options>(Status::Invalid);
            }
            opt.priority = pri;
            continue;
        }
        if (arg[1] == 't' && arg[2] != '\0') {
            opt.tag = arg + 2;
            continue;
        }
        if (arg[1] == 't' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.tag = argv[++i];
            continue;
        }
        if (arg[1] == 'h' && arg[2] != '\0') {
            opt.host = arg + 2;
            continue;
        }
        if (arg[1] == 'h' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.host = argv[++i];
            continue;
        }
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
        if (hosted::cstrcmp(arg, "-i") == 0) {
            opt.inet6 = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-4") == 0) {
            opt.inet6 = false;
            continue;
        }
        if (hosted::cstrcmp(arg, "-P") == 0) {
            opt.send_all = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-s") == 0) {
            opt.stderr_mode = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::logger
