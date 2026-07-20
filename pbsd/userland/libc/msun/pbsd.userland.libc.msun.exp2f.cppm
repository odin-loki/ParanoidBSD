module;
export module pbsd.userland.libc.msun.exp2f;
import pbsd.userland.msun.exp2f;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float exp2f(float x) noexcept {
    return pbsd::userland::msun::exp2f_val(x);
}
}
