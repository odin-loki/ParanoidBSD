module;
export module pbsd.userland.libc.msun.acosf;
import pbsd.userland.msun.acosf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float acosf(float x) noexcept {
    return pbsd::userland::msun::acosf_val(x);
}
}