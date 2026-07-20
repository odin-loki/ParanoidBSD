module;
export module pbsd.userland.libc.msun.nextafterf;
import pbsd.userland.msun.nextafterf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float nextafterf(float x, float y) noexcept {
    return pbsd::userland::msun::nextafterf_val(x, y);
}
}
