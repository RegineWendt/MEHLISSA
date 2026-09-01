// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_CORE_QUANTITY_HPP
#define MEHLISSA_CORE_QUANTITY_HPP

#include <compare>
#include <type_traits>

namespace mehlissa::core {

template <int LengthExponent, int TimeExponent, int AmountExponent, int MassExponent = 0>
struct Dimension final {};

template <typename DimensionType> class Quantity final {
  public:
    constexpr Quantity() noexcept = default;

    [[nodiscard]] static constexpr Quantity from_si(const double value) noexcept {
        return Quantity{value};
    }

    [[nodiscard]] constexpr double si_value() const noexcept { return value_; }

    constexpr Quantity& operator+=(const Quantity other) noexcept {
        value_ += other.value_;
        return *this;
    }

    constexpr Quantity& operator-=(const Quantity other) noexcept {
        value_ -= other.value_;
        return *this;
    }

    [[nodiscard]] constexpr Quantity operator-() const noexcept { return Quantity{-value_}; }

    [[nodiscard]] constexpr auto operator<=>(const Quantity&) const noexcept = default;

  private:
    explicit constexpr Quantity(const double value) noexcept : value_{value} {}

    double value_{};
};

template <typename DimensionType>
[[nodiscard]] constexpr Quantity<DimensionType>
operator+(Quantity<DimensionType> left, const Quantity<DimensionType> right) noexcept {
    left += right;
    return left;
}

template <typename DimensionType>
[[nodiscard]] constexpr Quantity<DimensionType>
operator-(Quantity<DimensionType> left, const Quantity<DimensionType> right) noexcept {
    left -= right;
    return left;
}

template <typename LeftDimension, typename RightDimension> struct DimensionProduct;

template <int LeftLength, int LeftTime, int LeftAmount, int LeftMass, int RightLength,
          int RightTime, int RightAmount, int RightMass>
struct DimensionProduct<Dimension<LeftLength, LeftTime, LeftAmount, LeftMass>,
                        Dimension<RightLength, RightTime, RightAmount, RightMass>>
    final {
    using type = Dimension<LeftLength + RightLength, LeftTime + RightTime, LeftAmount + RightAmount,
                           LeftMass + RightMass>;
};

template <typename LeftDimension, typename RightDimension>
using DimensionProductType = typename DimensionProduct<LeftDimension, RightDimension>::type;

template <typename LeftDimension, typename RightDimension> struct DimensionQuotient;

template <int LeftLength, int LeftTime, int LeftAmount, int LeftMass, int RightLength,
          int RightTime, int RightAmount, int RightMass>
struct DimensionQuotient<Dimension<LeftLength, LeftTime, LeftAmount, LeftMass>,
                         Dimension<RightLength, RightTime, RightAmount, RightMass>>
    final {
    using type = Dimension<LeftLength - RightLength, LeftTime - RightTime, LeftAmount - RightAmount,
                           LeftMass - RightMass>;
};

template <typename LeftDimension, typename RightDimension>
using DimensionQuotientType = typename DimensionQuotient<LeftDimension, RightDimension>::type;

template <typename LeftDimension, typename RightDimension>
[[nodiscard]] constexpr Quantity<DimensionProductType<LeftDimension, RightDimension>>
operator*(const Quantity<LeftDimension> left, const Quantity<RightDimension> right) noexcept {
    return Quantity<DimensionProductType<LeftDimension, RightDimension>>::from_si(left.si_value() *
                                                                                  right.si_value());
}

template <typename LeftDimension, typename RightDimension>
[[nodiscard]] constexpr Quantity<DimensionQuotientType<LeftDimension, RightDimension>>
operator/(const Quantity<LeftDimension> left, const Quantity<RightDimension> right) noexcept {
    return Quantity<DimensionQuotientType<LeftDimension, RightDimension>>::from_si(
        left.si_value() / right.si_value());
}

template <typename DimensionType, typename Scalar>
    requires std::is_arithmetic_v<Scalar>
[[nodiscard]] constexpr Quantity<DimensionType> operator*(const Quantity<DimensionType> quantity,
                                                          const Scalar scalar) noexcept {
    return Quantity<DimensionType>::from_si(quantity.si_value() * static_cast<double>(scalar));
}

template <typename Scalar, typename DimensionType>
    requires std::is_arithmetic_v<Scalar>
[[nodiscard]] constexpr Quantity<DimensionType>
operator*(const Scalar scalar, const Quantity<DimensionType> quantity) noexcept {
    return quantity * scalar;
}

template <typename DimensionType, typename Scalar>
    requires std::is_arithmetic_v<Scalar>
[[nodiscard]] constexpr Quantity<DimensionType> operator/(const Quantity<DimensionType> quantity,
                                                          const Scalar scalar) noexcept {
    return Quantity<DimensionType>::from_si(quantity.si_value() / static_cast<double>(scalar));
}

using Dimensionless = Quantity<Dimension<0, 0, 0>>;
using Length = Quantity<Dimension<1, 0, 0>>;
using Time = Quantity<Dimension<0, 1, 0>>;
using Area = Quantity<Dimension<2, 0, 0>>;
using Volume = Quantity<Dimension<3, 0, 0>>;
using Speed = Quantity<Dimension<1, -1, 0>>;
using Diffusivity = Quantity<Dimension<2, -1, 0>>;
using FlowRate = Quantity<Dimension<3, -1, 0>>;
using FirstOrderRate = Quantity<Dimension<0, -1, 0>>;
using SecondOrderAssociationRate = Quantity<Dimension<3, -1, -1>>;
using Amount = Quantity<Dimension<0, 0, 1>>;
using Concentration = Quantity<Dimension<-3, 0, 1>>;
using Mass = Quantity<Dimension<0, 0, 0, 1>>;
using Energy = Quantity<Dimension<2, -2, 0, 1>>;
using Pressure = Quantity<Dimension<-1, -2, 0, 1>>;
using InversePressure = Quantity<Dimension<1, 2, 0, -1>>;
using VascularResistance = Quantity<Dimension<-4, -1, 0, 1>>;
using VascularCompliance = Quantity<Dimension<4, 2, 0, -1>>;

[[nodiscard]] constexpr Length meters(const double value) noexcept {
    return Length::from_si(value);
}

[[nodiscard]] constexpr Length millimeters(const double value) noexcept {
    return meters(value * 1.0e-3);
}

[[nodiscard]] constexpr Length micrometers(const double value) noexcept {
    return meters(value * 1.0e-6);
}

[[nodiscard]] constexpr Time seconds(const double value) noexcept { return Time::from_si(value); }

[[nodiscard]] constexpr Time milliseconds(const double value) noexcept {
    return seconds(value * 1.0e-3);
}

[[nodiscard]] constexpr Time minutes(const double value) noexcept { return seconds(value * 60.0); }

[[nodiscard]] constexpr Area square_meters(const double value) noexcept {
    return Area::from_si(value);
}

[[nodiscard]] constexpr Volume cubic_meters(const double value) noexcept {
    return Volume::from_si(value);
}

[[nodiscard]] constexpr Volume liters(const double value) noexcept {
    return cubic_meters(value * 1.0e-3);
}

[[nodiscard]] constexpr Volume milliliters(const double value) noexcept {
    return liters(value * 1.0e-3);
}

[[nodiscard]] constexpr Speed meters_per_second(const double value) noexcept {
    return Speed::from_si(value);
}

[[nodiscard]] constexpr Speed millimeters_per_second(const double value) noexcept {
    return meters_per_second(value * 1.0e-3);
}

[[nodiscard]] constexpr Diffusivity square_meters_per_second(const double value) noexcept {
    return Diffusivity::from_si(value);
}

[[nodiscard]] constexpr FlowRate cubic_meters_per_second(const double value) noexcept {
    return FlowRate::from_si(value);
}

[[nodiscard]] constexpr FlowRate liters_per_minute(const double value) noexcept {
    return cubic_meters_per_second(value * 1.0e-3 / 60.0);
}

[[nodiscard]] constexpr FirstOrderRate per_second(const double value) noexcept {
    return FirstOrderRate::from_si(value);
}

[[nodiscard]] constexpr SecondOrderAssociationRate
cubic_meters_per_mole_second(const double value) noexcept {
    return SecondOrderAssociationRate::from_si(value);
}

[[nodiscard]] constexpr Amount moles(const double value) noexcept { return Amount::from_si(value); }

[[nodiscard]] constexpr Amount millimoles(const double value) noexcept {
    return moles(value * 1.0e-3);
}

[[nodiscard]] constexpr Amount micromoles(const double value) noexcept {
    return moles(value * 1.0e-6);
}

[[nodiscard]] constexpr Energy joules(const double value) noexcept {
    return Energy::from_si(value);
}

[[nodiscard]] constexpr Energy microjoules(const double value) noexcept {
    return joules(value * 1.0e-6);
}

[[nodiscard]] constexpr Concentration moles_per_cubic_meter(const double value) noexcept {
    return Concentration::from_si(value);
}

[[nodiscard]] constexpr Concentration millimoles_per_liter(const double value) noexcept {
    return moles_per_cubic_meter(value);
}

[[nodiscard]] constexpr Pressure pascals(const double value) noexcept {
    return Pressure::from_si(value);
}

[[nodiscard]] constexpr Pressure millimeters_of_mercury(const double value) noexcept {
    return pascals(value * 133.322387415);
}

[[nodiscard]] constexpr InversePressure per_pascal(const double value) noexcept {
    return InversePressure::from_si(value);
}

[[nodiscard]] constexpr InversePressure per_millimeter_of_mercury(const double value) noexcept {
    return per_pascal(value / 133.322387415);
}

[[nodiscard]] constexpr VascularResistance
pascal_seconds_per_cubic_meter(const double value) noexcept {
    return VascularResistance::from_si(value);
}

[[nodiscard]] constexpr VascularResistance wood_units(const double value) noexcept {
    return pascal_seconds_per_cubic_meter(value * 133.322387415 * 60.0 / 1.0e-3);
}

[[nodiscard]] constexpr VascularCompliance cubic_meters_per_pascal(const double value) noexcept {
    return VascularCompliance::from_si(value);
}

[[nodiscard]] constexpr VascularCompliance
milliliters_per_millimeter_of_mercury(const double value) noexcept {
    return cubic_meters_per_pascal(value * 1.0e-6 / 133.322387415);
}

[[nodiscard]] constexpr double in_meters(const Length value) noexcept { return value.si_value(); }

[[nodiscard]] constexpr double in_seconds(const Time value) noexcept { return value.si_value(); }

[[nodiscard]] constexpr double in_square_meters(const Area value) noexcept {
    return value.si_value();
}

[[nodiscard]] constexpr double in_cubic_meters(const Volume value) noexcept {
    return value.si_value();
}

[[nodiscard]] constexpr double in_meters_per_second(const Speed value) noexcept {
    return value.si_value();
}

[[nodiscard]] constexpr double in_square_meters_per_second(const Diffusivity value) noexcept {
    return value.si_value();
}

[[nodiscard]] constexpr double in_cubic_meters_per_second(const FlowRate value) noexcept {
    return value.si_value();
}

[[nodiscard]] constexpr double in_liters_per_minute(const FlowRate value) noexcept {
    return value.si_value() * 60.0 / 1.0e-3;
}

[[nodiscard]] constexpr double in_per_second(const FirstOrderRate value) noexcept {
    return value.si_value();
}

[[nodiscard]] constexpr double
in_cubic_meters_per_mole_second(const SecondOrderAssociationRate value) noexcept {
    return value.si_value();
}

[[nodiscard]] constexpr double in_moles(const Amount value) noexcept { return value.si_value(); }

[[nodiscard]] constexpr double in_joules(const Energy value) noexcept { return value.si_value(); }

[[nodiscard]] constexpr double in_moles_per_cubic_meter(const Concentration value) noexcept {
    return value.si_value();
}

[[nodiscard]] constexpr double in_pascals(const Pressure value) noexcept {
    return value.si_value();
}

[[nodiscard]] constexpr double in_millimeters_of_mercury(const Pressure value) noexcept {
    return value.si_value() / 133.322387415;
}

[[nodiscard]] constexpr double in_per_pascal(const InversePressure value) noexcept {
    return value.si_value();
}

[[nodiscard]] constexpr double in_per_millimeter_of_mercury(const InversePressure value) noexcept {
    return value.si_value() * 133.322387415;
}

[[nodiscard]] constexpr double
in_pascal_seconds_per_cubic_meter(const VascularResistance value) noexcept {
    return value.si_value();
}

[[nodiscard]] constexpr double in_wood_units(const VascularResistance value) noexcept {
    return value.si_value() / (133.322387415 * 60.0 / 1.0e-3);
}

[[nodiscard]] constexpr double in_cubic_meters_per_pascal(const VascularCompliance value) noexcept {
    return value.si_value();
}

[[nodiscard]] constexpr double
in_milliliters_per_millimeter_of_mercury(const VascularCompliance value) noexcept {
    return value.si_value() * 133.322387415 / 1.0e-6;
}

} // namespace mehlissa::core

#endif // MEHLISSA_CORE_QUANTITY_HPP
