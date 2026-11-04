module;
#include <cstddef>

export module pbsd.userland.bectl;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/sbin/bectl/bectl.c — subcommand registry (logic-only).
export namespace pbsd::userland::sbin::bectl {

enum class Subcommand : unsigned char {
    Activate,

    Check,

    Create,

    Destroy,

    Export,

    Import,

    Jail,

    List,

    Mount,

    Rename,

    Ujail,

    Unjail,

    Umount,

    Unmount,
    Unknown,
};

struct Options {
    Subcommand cmd{Subcommand::Unknown};
    bool recursive{false};
    bool force{false};
    bool verbose{false};
};

[[nodiscard]] inline Result<Subcommand> subcommand_from_name(const char* name) noexcept {
    if (name == nullptr) {
        return result_err<Subcommand>(Status::Invalid);
    }
    static const struct {
        const char* name;
        Subcommand cmd;
    } kTable[] = {
        {"activate", Subcommand::Activate},
        {"check", Subcommand::Check},
        {"create", Subcommand::Create},
        {"destroy", Subcommand::Destroy},
        {"export", Subcommand::Export},
        {"import", Subcommand::Import},
        {"jail", Subcommand::Jail},
        {"list", Subcommand::List},
        {"mount", Subcommand::Mount},
        {"rename", Subcommand::Rename},
        {"ujail", Subcommand::Ujail},
        {"unjail", Subcommand::Unjail},
        {"umount", Subcommand::Umount},
        {"unmount", Subcommand::Unmount},
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
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* arg = argv[i];
        if (hosted::cstrcmp(arg, "-r") == 0) {
            opt.recursive = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-f") == 0) {
            opt.force = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-v") == 0) {
            opt.verbose = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    if (i >= argc || argv[i] == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    const auto cmd = subcommand_from_name(argv[i]);
    if (!cmd.has_value()) {
        return result_err<Options>(Status::Invalid);
    }
    opt.cmd = cmd.value;
    optind_out = i + 1;
    return result_ok(opt);
}

} // namespace pbsd::userland::sbin::bectl
