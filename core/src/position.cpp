#include <mehlissa/core/position.hpp>

#include <cmath>

namespace mehlissa::core {

double distance_meters(const Position3D& first, const Position3D& second) noexcept {
    const auto delta_x = second.x_meters - first.x_meters;
    const auto delta_y = second.y_meters - first.y_meters;
    const auto delta_z = second.z_meters - first.z_meters;
    return std::hypot(delta_x, delta_y, delta_z);
}

} // namespace mehlissa::core
