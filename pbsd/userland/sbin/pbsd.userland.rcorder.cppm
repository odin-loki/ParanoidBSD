module;

export module pbsd.userland.rcorder;

export import pbsd.core;
export import pbsd.userland.hosted;

export namespace pbsd::userland::sbin::rcorder {

[[nodiscard]] inline bool keyword_is_provide(const char* kw) noexcept {
    return kw != nullptr && hosted::cstrcmp(kw, "PROVIDE:") == 0;
}

}
