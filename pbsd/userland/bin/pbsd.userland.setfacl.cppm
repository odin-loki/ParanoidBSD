module;
#include <cstddef>

export module pbsd.userland.setfacl;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/setfacl/{setfacl.c,util.c} — ACL op/brand helpers (logic-only).
export namespace pbsd::userland::bin::setfacl {

enum class AclBrand : int {
    Unknown = 0,
    Nfs4 = 1,
    Posix = 2,
};

enum class AclType : unsigned char {
    Access,
    Default,
};

enum class Operation : unsigned char {
    MergeAcl,
    RemoveDefault,
    RemoveExtended,
    RemoveAcl,
    RemoveByNumber,
    AddAcl,
};

enum class FollowMode : unsigned char {
    Logical,
    Physical,
    PhysicalFollowArgDir,
};

struct Options {
    FollowMode follow{FollowMode::Logical};
    bool recursive{false};
    bool no_dereference{false};
    bool numeric{false};
    AclType acl_type{AclType::Access};
    unsigned op_count{0};
    bool read_stdin{false};
};

[[nodiscard]] inline const char* brand_name(AclBrand brand) noexcept {
    switch (brand) {
    case AclBrand::Nfs4:
        return "NFSv4";
    case AclBrand::Posix:
        return "POSIX.1e";
    default:
        return "unknown";
    }
}

[[nodiscard]] inline bool branding_mismatch(AclBrand a, AclBrand b) noexcept {
    if (a == AclBrand::Unknown || b == AclBrand::Unknown) {
        return false;
    }
    return a != b;
}

[[nodiscard]] inline Result<unsigned> fts_options_from(const Options& opt) noexcept {
    if (opt.recursive && opt.no_dereference) {
        return result_err<unsigned>(Status::Invalid);
    }
    unsigned fts = 0;
    if (opt.recursive) {
        if (opt.follow == FollowMode::Logical) {
            fts = 1;
        } else {
            fts = 2;
            if (opt.follow == FollowMode::PhysicalFollowArgDir) {
                fts |= 4;
            }
        }
    } else if (opt.no_dereference) {
        fts = 2;
    } else {
        fts = 1;
    }
    return result_ok(fts);
}

[[nodiscard]] inline Result<int> parse_entry_number(const char* s) noexcept {
    if (s == nullptr || s[0] == '\0') {
        return result_err<int>(Status::Invalid);
    }
    int val = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return result_err<int>(Status::Invalid);
        }
        val = val * 10 + (*p - '0');
    }
    if (val < 0) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(val);
}

[[nodiscard]] inline Result<Operation> classify_remove_operand(const char* s,
                                                               int* entry_number) noexcept {
    if (s == nullptr || entry_number == nullptr) {
        return result_err<Operation>(Status::Invalid);
    }
    const auto num = parse_entry_number(s);
    if (num.has_value()) {
        *entry_number = num.value;
        return result_ok(Operation::RemoveByNumber);
    }
    return result_ok(Operation::RemoveAcl);
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }

    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        if (flag[1] == '\0') {
            return result_err<Options>(Status::Invalid);
        }
        for (int j = 1; flag[j] != '\0'; ++j) {
            switch (flag[j]) {
            case 'H':
                opt.follow = FollowMode::PhysicalFollowArgDir;
                break;
            case 'L':
                opt.follow = FollowMode::Logical;
                break;
            case 'P':
                opt.follow = FollowMode::Physical;
                break;
            case 'R':
                opt.recursive = true;
                break;
            case 'b':
            case 'k':
            case 'm':
            case 'M':
            case 'x':
            case 'X':
            case 'a':
                ++opt.op_count;
                break;
            case 'd':
                opt.acl_type = AclType::Default;
                break;
            case 'h':
                opt.no_dereference = true;
                break;
            case 'n':
                opt.numeric = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
        ++i;
    }

    if (i >= argc) {
        opt.read_stdin = true;
    } else if (argv[i] != nullptr && hosted::cstrcmp(argv[i], "-") == 0) {
        opt.read_stdin = true;
    }

    if (!opt.numeric && opt.op_count == 0) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline bool default_acl_requires_directory(AclType type,
                                                         bool is_directory) noexcept {
    return type == AclType::Default && !is_directory;
}

[[nodiscard]] inline bool follow_symlink_for_entry(const Options& opt,
                                                   int fts_level) noexcept {
    if (!opt.recursive && !opt.no_dereference) {
        return true;
    }
    if (opt.recursive && opt.follow == FollowMode::Logical) {
        return true;
    }
    if (opt.recursive && opt.follow == FollowMode::PhysicalFollowArgDir &&
        fts_level == 0) {
        return true;
    }
    return false;
}

} // namespace pbsd::userland::bin::setfacl
