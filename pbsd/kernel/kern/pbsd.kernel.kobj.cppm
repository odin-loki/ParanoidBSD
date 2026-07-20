module;

export module pbsd.kernel.kobj;

export import pbsd.core;

/// Wave 4/5 — kobj method dispatch stubs from sys/kobj.h.
export namespace pbsd::kernel::kobj {

inline constexpr unsigned kMethodTableSize = 64;

struct KobjClass {
    const char* name{};
    unsigned    method_count{};
};

[[nodiscard]] constexpr Status validate_class(const KobjClass& c) noexcept {
    if (c.name == nullptr || c.method_count == 0 || c.method_count > kMethodTableSize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kobj
