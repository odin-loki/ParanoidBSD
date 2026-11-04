module;
#include <cstddef>

export module pbsd.userland.kenv;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/kenv/kenv.c — mode/argument parsing (logic-only).
export namespace pbsd::userland::bin::kenv {

enum class DumpMode : unsigned char {
    All,
    Loader,
    Static,
};

enum class Action : unsigned char {
    Dump,
    Get,
    Set,
    Unset,
};

struct Options {
    Action action{Action::Dump};
    DumpMode dump{DumpMode::All};
    bool hash_names{false};
    bool no_name{false};
    bool quiet{false};
    bool verbose{false};
    const char* variable{nullptr};
    const char* value{nullptr};
};

[[nodiscard]] inline bool list_mode_conflict(bool loader, bool stat) noexcept {
    return loader && stat;
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }

    int i = 1;
    bool loader = false;
    bool stat = false;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        for (int j = 1; flag[j] != '\0'; ++j) {
            switch (flag[j]) {
            case 'h':
                opt.hash_names = true;
                break;
            case 'l':
                loader = true;
                opt.dump = DumpMode::Loader;
                break;
            case 'N':
                opt.no_name = true;
                break;
            case 'q':
                opt.quiet = true;
                break;
            case 's':
                stat = true;
                opt.dump = DumpMode::Static;
                break;
            case 'u':
                opt.action = Action::Unset;
                break;
            case 'v':
                opt.verbose = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
        ++i;
    }

    if (opt.hash_names && i < argc) {
        return result_err<Options>(Status::Invalid);
    }
    if (list_mode_conflict(loader, stat)) {
        return result_err<Options>(Status::Invalid);
    }

    if (i < argc && argv[i] != nullptr) {
        const char* env = argv[i];
        const char* eq = nullptr;
        for (const char* p = env; *p != '\0'; ++p) {
            if (*p == '=') {
                eq = p;
                break;
            }
        }
        if (eq != nullptr) {
            opt.value = eq + 1;
            opt.action = Action::Set;
        } else if (opt.action == Action::Unset) {
            opt.action = Action::Unset;
        } else if (opt.verbose) {
            opt.action = Action::Get;
        } else if (opt.action != Action::Unset) {
            opt.action = Action::Get;
        }
        opt.variable = env;
        ++i;
    } else {
        opt.action = Action::Dump;
    }

    if ((opt.action == Action::Unset || opt.verbose) && opt.variable == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    if (i < argc) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::bin::kenv
