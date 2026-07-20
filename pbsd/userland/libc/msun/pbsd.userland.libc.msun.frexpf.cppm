module;
export module pbsd.userland.libc.msun.frexpf;
import pbsd.userland.msun.frexpf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float frexpf(float x, int* exp) noexcept {
    return pbsd::userland::msun::frexpf_val(x, exp);
}
}
