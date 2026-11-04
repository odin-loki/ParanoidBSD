module;
export module pbsd.userland.libc.msun.nearbyintf;
import pbsd.userland.msun.nearbyintf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float nearbyintf(float x) noexcept {
    return pbsd::userland::msun::nearbyintf_val(x);
}
}