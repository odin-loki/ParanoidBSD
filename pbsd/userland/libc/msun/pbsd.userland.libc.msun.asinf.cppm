module;
export module pbsd.userland.libc.msun.asinf;
import pbsd.userland.msun.asinf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float asinf(float x) noexcept {
    return pbsd::userland::msun::asinf_val(x);
}
}