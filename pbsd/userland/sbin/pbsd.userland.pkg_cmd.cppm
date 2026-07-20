module;
#include <cstddef>

export module pbsd.userland.pkg_cmd;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Stub helpers from hbsd/src/usr.sbin/pkg — subcommand/flag parse scaffold (logic-only).
/// Module name pkg_cmd avoids clash with pbsd.pkg nucleus.
export namespace pbsd::userland::usr_sbin::pkg {

enum class Subcommand : unsigned char {
    Add,
    Delete,
    Fetch,
    Info,
    Install,
    Query,
    Remove,
    Rquery,
    Search,
    Update,
    Upgrade,
    Version,
    Unknown,
};

struct Options {
    Subcommand cmd{Subcommand::Unknown};
    bool force{false};
    bool yes{false};
    bool dry_run{false};
    bool quiet{false};
    const char* root{nullptr};
    const char* chroot{nullptr};
};

[[nodiscard]] inline Result<Subcommand> subcommand_from_name(const char* name) noexcept {
    if (name == nullptr) {
        return result_err<Subcommand>(Status::Invalid);
    }
    static const struct {
        const char* name;
        Subcommand cmd;
    } kTable[] = {
        {"add", Subcommand::Add},       {"delete", Subcommand::Delete},
        {"fetch", Subcommand::Fetch},   {"info", Subcommand::Info},
        {"install", Subcommand::Install}, {"query", Subcommand::Query},
        {"remove", Subcommand::Remove}, {"rquery", Subcommand::Rquery},
        {"search", Subcommand::Search}, {"update", Subcommand::Update},
        {"upgrade", Subcommand::Upgrade}, {"version", Subcommand::Version},
        {nullptr, Subcommand::Unknown},
    };
    for (int i = 0; kTable[i].name != nullptr; ++i) {
        if (hosted::cstrcmp(name, kTable[i].name) == 0) {
            return result_ok(kTable[i].cmd);
        }
    }
    return result_err<Subcommand>(Status::Invalid);
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr || argc < 2) {
        return result_err<Options>(Status::Invalid);
    }
    const auto cmd = subcommand_from_name(argv[1]);
    if (!cmd.has_value()) {
        return result_err<Options>(Status::Invalid);
    }
    opt.cmd = cmd.value;
    int i = 2;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* arg = argv[i];
        if (hosted::cstrcmp(arg, "-y") == 0) {
            opt.yes = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-n") == 0) {
            opt.dry_run = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-q") == 0) {
            opt.quiet = true;
            continue;
        }
        if (arg[1] == 'r' && arg[2] != '\0') {
            opt.root = arg + 2;
            continue;
        }
        if (arg[1] == 'r' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.root = argv[++i];
            continue;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            if (arg[j] == 'f') {
                opt.force = true;
            } else {
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_sbin::pkg
