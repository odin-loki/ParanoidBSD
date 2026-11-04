module;

export module pbsd.userland.periodic;

export import pbsd.core;
export import pbsd.userland.hosted;

export namespace pbsd::userland::usr_sbin::periodic {

enum class RunMode : unsigned char { Daily, Weekly, Monthly, Security, Unknown };

[[nodiscard]] inline Result<RunMode> mode_from_name(const char* name) noexcept {
    if (name == nullptr) {
        return result_err<RunMode>(Status::Invalid);
    }
    if (hosted::cstrcmp(name, "daily") == 0) {
        return result_ok(RunMode::Daily);
    }
    if (hosted::cstrcmp(name, "weekly") == 0) {
        return result_ok(RunMode::Weekly);
    }
    return result_err<RunMode>(Status::Invalid);
}

}
