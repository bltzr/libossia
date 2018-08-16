#pragma once
#include <ossia/network/dataspace/dataspace_base.hpp>
namespace ossia
{
struct cartesian_3d_u;
struct position_dataspace;
template <typename Impl>
struct position_unit
{
  using is_unit = std::true_type;
  using is_multidimensional = std::true_type;
  using dataspace_type = position_dataspace;
  using neutral_unit = cartesian_3d_u;
  using concrete_type = Impl;
};

struct OSSIA_EXPORT cartesian_3d_u : public position_unit<cartesian_3d_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("cart3D", "xyz"));
  }
  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("xyz"));
  }
  static constexpr std::array<ossia::string_view, 3> internal_units()
  {
    constexpr_return(ossia::make_string_array("distance.m", "distance.m", "distance.m"));
  }
  using value_type = vec3f;
  static constexpr strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self)
  {
    return self;
  }

  static constexpr value_type
  from_neutral(strong_value<neutral_unit> self)
  {
    return self.dataspace_value;
  }
};

struct OSSIA_EXPORT cartesian_2d_u : public position_unit<cartesian_2d_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("cart2D", "xy"));
  }
  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("xy"));
  }
  static constexpr std::array<ossia::string_view, 2> internal_units()
  {
    constexpr_return(ossia::make_string_array("distance.m", "distance.m"));
  }
  using value_type = vec2f;
  static strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self)
  {
    return {self.dataspace_value[0], self.dataspace_value[1], 0.f};
  }

  static value_type from_neutral(strong_value<neutral_unit> self)
  {
    return {self.dataspace_value[0], self.dataspace_value[1]};
  }
};

// aed
struct OSSIA_EXPORT spherical_u : public position_unit<spherical_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("spherical", "aed"));
  }
  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("aed"));
  }
  static constexpr std::array<ossia::string_view, 3> internal_units()
  {
    constexpr_return(ossia::make_string_array("angle.degree", "angle.degree", "distance.m"));
  }
  using value_type = vec3f;
  static strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self);

  static value_type from_neutral(strong_value<neutral_unit> self);
};

// ad
struct OSSIA_EXPORT polar_u : public position_unit<polar_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("polar", "ad"));
  }
  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("ad"));
  }
  static constexpr std::array<ossia::string_view, 2> internal_units()
  {
    constexpr_return(ossia::make_string_array("angle.degree", "distance.m"));
  }
  using value_type = vec2f;
  static strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self)
  {
    const auto a = self.dataspace_value[0] * deg_to_rad;
    const auto d = self.dataspace_value[1];

    return {(float)(std::sin(a) * d), (float)(std::cos(a) * d), 0.};
  }

  static value_type from_neutral(strong_value<neutral_unit> self)
  {
    const auto x = self.dataspace_value[0];
    const auto y = self.dataspace_value[1];

    return {(float)(std::atan2(y, x) * rad_to_deg),
            (float)(ossia::norm(x, y))};
  }
};

struct OSSIA_EXPORT opengl_u : public position_unit<opengl_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("openGL"));
  }
  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("xyz"));
  }
  static constexpr std::array<ossia::string_view, 3> internal_units()
  {
    constexpr_return(ossia::make_string_array("distance.m", "distance.m", "distance.m"));
  }
  using value_type = vec3f;
  static strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self)
  {
    return {self.dataspace_value[0], -self.dataspace_value[2],
            self.dataspace_value[1]};
  }

  static value_type from_neutral(strong_value<neutral_unit> self)
  {
    return {self.dataspace_value[0], self.dataspace_value[2],
            -self.dataspace_value[1]};
  }
};

struct OSSIA_EXPORT cylindrical_u : public position_unit<cylindrical_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("cylindrical", "daz"));
  }
  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("daz"));
  }
  static constexpr std::array<ossia::string_view, 3> internal_units()
  {
    constexpr_return(ossia::make_string_array("angle.degree", "distance.m", "distance.m"));
  }
  using value_type = vec3f;
  static strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self);

  static value_type from_neutral(strong_value<neutral_unit> self);
};
}
