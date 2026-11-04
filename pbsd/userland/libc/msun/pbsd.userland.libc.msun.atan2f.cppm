module;
export module pbsd.userland.libc.msun.atan2f;
import pbsd.userland.msun.atan2f;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float atan2f(float x, float y) noexcept {
    return pbsd::userland::msun::atan2f_val(x, y);
}
}