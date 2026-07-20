module;
#include <cstddef>

export module pbsd.userland.id;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/id/id.c — id/whoami/groups parse (logic-only).
export namespace pbsd::userland::usr_bin::id {

enum class ProgMode : unsigned char {
    Id,
    Whoami,
    Groups,
};

struct Options {
    ProgMode mode{ProgMode::Id};
    bool audit_id{false};
    bool groups_only{false};
    bool mac_label{false};
    bool pretty{false};
    bool real_id{false};
    bool name_out{false};
    bool primary_group{false};
    bool user_id{false};
    bool dir{false};
    bool shell{false};
};

[[nodiscard]] inline ProgMode detect_mode(const char* progname) noexcept {
    if (progname == nullptr) {
        return ProgMode::Id;
    }
    if (hosted::cstrcmp(progname, "groups") == 0) {
        return ProgMode::Groups;
    }
    if (hosted::cstrcmp(progname, "whoami") == 0) {
        return ProgMode::Whoami;
    }
    return ProgMode::Id;
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                const char* progname,
                                                int& optind_out) noexcept {
    Options opt{};
    opt.mode = detect_mode(progname);
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }

    if (opt.mode == ProgMode::Groups) {
        opt.groups_only = true;
        opt.name_out = true;
        optind_out = 1;
        if (argc > 1) {
            return result_err<Options>(Status::Invalid);
        }
        return result_ok(opt);
    }
    if (opt.mode == ProgMode::Whoami) {
        opt.user_id = true;
        opt.name_out = true;
        optind_out = 1;
        if (argc > 1) {
            return result_err<Options>(Status::Invalid);
        }
        return result_ok(opt);
    }

    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        for (const char* p = argv[i] + 1; *p != '\0'; ++p) {
            switch (*p) {
            case 'A':
                opt.audit_id = true;
                break;
            case 'G':
                opt.groups_only = true;
                break;
            case 'M':
                opt.mac_label = true;
                break;
            case 'P':
                opt.pretty = true;
                break;
            case 'a':
                break;
            case 'c':
                break;
            case 'd':
                opt.dir = true;
                break;
            case 'g':
                opt.primary_group = true;
                break;
            case 'n':
                opt.name_out = true;
                break;
            case 'p':
                break;
            case 'r':
                opt.real_id = true;
                break;
            case 's':
                opt.shell = true;
                break;
            case 'u':
                opt.user_id = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;

    int combo = (opt.audit_id ? 1 : 0) + (opt.groups_only ? 1 : 0) +
                (opt.mac_label ? 1 : 0) + (opt.pretty ? 1 : 0) +
                (opt.primary_group ? 1 : 0) + (opt.user_id ? 1 : 0);
    if (combo + (opt.dir ? 1 : 0) + (opt.shell ? 1 : 0) > 1) {
        return result_err<Options>(Status::Invalid);
    }
    if (combo > 1) {
        return result_err<Options>(Status::Invalid);
    }
    if ((opt.audit_id || opt.mac_label) && i < argc) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::id
