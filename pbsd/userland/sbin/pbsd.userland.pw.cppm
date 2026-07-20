module;
#include <cstddef>

export module pbsd.userland.pw;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.sbin/pw — subcommand/field parse scaffold (logic-only).
export namespace pbsd::userland::usr_sbin::pw {

enum class Subcommand : unsigned char {
    Useradd,
    Userdel,
    Usermod,
    Usershow,
    Groupadd,
    Groupdel,
    Groupmod,
    Lock,
    Unlock,
    Unknown,
};

struct Options {
    Subcommand cmd{Subcommand::Unknown};
    const char* name{nullptr};
    const char* uid{nullptr};
    const char* gid{nullptr};
    const char* home{nullptr};
    const char* shell{nullptr};
    const char* comment{nullptr};
    bool system{false};
    bool yes{false};
};

[[nodiscard]] inline Result<Subcommand> subcommand_from_name(const char* name) noexcept {
    if (name == nullptr) {
        return result_err<Subcommand>(Status::Invalid);
    }
    static const struct {
        const char* name;
        Subcommand cmd;
    } kTable[] = {
        {"useradd", Subcommand::Useradd}, {"userdel", Subcommand::Userdel},
        {"usermod", Subcommand::Usermod}, {"usershow", Subcommand::Usershow},
        {"groupadd", Subcommand::Groupadd}, {"groupdel", Subcommand::Groupdel},
        {"groupmod", Subcommand::Groupmod}, {"lock", Subcommand::Lock},
        {"unlock", Subcommand::Unlock},   {nullptr, Subcommand::Unknown},
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
    for (; i < argc && argv[i] != nullptr; ++i) {
        const char* arg = argv[i];
        if (arg[0] != '-') {
            if (opt.name == nullptr) {
                opt.name = arg;
            }
            continue;
        }
        if (hosted::cstrcmp(arg, "-y") == 0) {
            opt.yes = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-s") == 0) {
            opt.system = true;
            continue;
        }
        if (arg[1] == 'u' && arg[2] != '\0') {
            opt.uid = arg + 2;
            continue;
        }
        if (arg[1] == 'u' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.uid = argv[++i];
            continue;
        }
        if (arg[1] == 'g' && arg[2] != '\0') {
            opt.gid = arg + 2;
            continue;
        }
        if (arg[1] == 'd' && arg[2] != '\0') {
            opt.home = arg + 2;
            continue;
        }
        if (arg[1] == 'c' && arg[2] != '\0') {
            opt.comment = arg + 2;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_sbin::pw
