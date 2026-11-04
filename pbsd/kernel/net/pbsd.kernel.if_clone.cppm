export module pbsd.kernel.if_clone;

export import pbsd.core;

/// Wave 4 — cloned interface factory (net/if_clone.h).
export namespace pbsd::kernel::if_clone {

inline constexpr unsigned kNameSize = 16;

enum class Flag : unsigned {
    NeedGiant = 0x0001,
    NeedIfDetach = 0x0002,
};

struct CloneReq {
    char name[kNameSize]{};
    unsigned unit{};
};

[[nodiscard]] constexpr Status validate_name(const char* name) noexcept {
    if (name == nullptr || name[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_unit(unsigned unit) noexcept {
    if (unit > 65535) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_request(const CloneReq& req) noexcept {
    if (validate_name(req.name) != Status::Ok) {
        return Status::Invalid;
    }
    return validate_unit(req.unit);
}

[[nodiscard]] inline unsigned flag_table_size() noexcept {
    return 2;
}

} // namespace pbsd::kernel::if_clone
