#pragma once
#include <ossia/network/dataspace/dataspace_base.hpp>

namespace ossia
{

struct quaternion_u;
struct orientation_dataspace;
template <typename Impl>
struct orientation_unit
{
  using is_unit = std::true_type;
  using is_multidimensional = std::true_type;
  using dataspace_type = orientation_dataspace;
  using neutral_unit = quaternion_u;
  using concrete_type = Impl;
};

struct OSSIA_EXPORT quaternion_u : public orientation_unit<quaternion_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("quaternion", "quat"));
  }
  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("abcd"));
  } // TODO find something better than 1 ?

  using value_type = vec4f;
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

struct OSSIA_EXPORT euler_u : public orientation_unit<euler_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("euler", "ypr"));
  }
  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("ypr"));
  }

  using value_type = vec4f;

  static strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self);

  static value_type from_neutral(strong_value<neutral_unit> self);
};

struct OSSIA_EXPORT axis_u : public orientation_unit<axis_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("axis", "xyzw"));
  }
  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("xyzw"));
  }
  using value_type = vec4f;

  static strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self);

  static value_type from_neutral(strong_value<neutral_unit> self);
};
}
