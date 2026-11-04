module;
export module pbsd.userland.libc.msun.rintf;
import pbsd.userland.msun.rintf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float rintf(float x) noexcept {
    return pbsd::userland::msun::rintf(x);
}
}