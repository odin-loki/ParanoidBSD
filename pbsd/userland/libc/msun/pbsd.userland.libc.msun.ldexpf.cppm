module;
export module pbsd.userland.libc.msun.ldexpf;
import pbsd.userland.msun.ldexpf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float ldexpf(float x, int exp) noexcept {
    return pbsd::userland::msun::ldexpf_val(x, exp);
}
}
