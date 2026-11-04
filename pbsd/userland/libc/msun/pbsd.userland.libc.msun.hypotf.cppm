module;
export module pbsd.userland.libc.msun.hypotf;
import pbsd.userland.msun.hypotf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float hypotf(float x, float y) noexcept {
    return pbsd::userland::msun::hypotf_val(x, y);
}
}