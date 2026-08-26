#ifndef MEHLISSA_CORE_POSITION_HPP
#define MEHLISSA_CORE_POSITION_HPP

namespace mehlissa::core {

struct Position3D {
    double x_meters{};
    double y_meters{};
    double z_meters{};
};

[[nodiscard]] double distance_meters(const Position3D& first, const Position3D& second) noexcept;

} // namespace mehlissa::core

#endif // MEHLISSA_CORE_POSITION_HPP
