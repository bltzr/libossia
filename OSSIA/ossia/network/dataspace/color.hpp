#pragma once
#include <ossia/network/dataspace/dataspace_base.hpp>
// Algorithms taken from http://www.easyrgb.com
namespace ossia
{

struct rgba8_u;
struct color_dataspace;
template <typename Impl>
struct color_unit
{
  using is_unit = std::true_type;
  using is_multidimensional
      = std::true_type; // number of dimensions -> decltype(value)::size_value
  using neutral_unit = rgba8_u;
  using concrete_type = Impl;
  using dataspace_type = color_dataspace;
};

struct OSSIA_EXPORT rgba8_u : public color_unit<rgba8_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("rgba8"));
  }

  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("rgba"));
  }

  static constexpr std::array<ossia::string_view, 4> internal_units()
  {
    constexpr_return(ossia::make_string_array("none", "none", "none", "none"));
  }


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



struct OSSIA_EXPORT rgba_u : public color_unit<rgba_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("rgba"));
  }

  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("rgba"));
  }

  static constexpr std::array<ossia::string_view, 4> internal_units()
  {
    constexpr_return(ossia::make_string_array("none", "none", "none", "none"));
  }

  using value_type = vec4f;

  static strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self)
  {
    return make_vec(
            self.dataspace_value[0] * 255., self.dataspace_value[1] * 255.,
            self.dataspace_value[2] * 255., self.dataspace_value[3] * 255.);
  }

  static value_type from_neutral(strong_value<neutral_unit> self)
  {
    return make_vec(
             self.dataspace_value[0] / 255., self.dataspace_value[1] / 255.,
             self.dataspace_value[2] / 255., self.dataspace_value[3] / 255.);
  }
};

struct OSSIA_EXPORT rgb_u : public color_unit<rgb_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("rgb"));
  }

  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("rgb"));
  }

  static constexpr std::array<ossia::string_view, 3> internal_units()
  {
    constexpr_return(ossia::make_string_array("none", "none", "none"));
  }
  using value_type = vec3f;

  static strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self)
  {
    return make_vec(
             self.dataspace_value[0] * 255, self.dataspace_value[1] * 255,
             self.dataspace_value[2] * 255, 255);
  }

  static value_type from_neutral(strong_value<neutral_unit> self)
  {
    return make_vec(
             self.dataspace_value[0] / 255., self.dataspace_value[1] / 255.,
             self.dataspace_value[2] / 255.);
  }
};

struct OSSIA_EXPORT bgr_u : public color_unit<bgr_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("bgr"));
  }

  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("bgr"));
  }

  static constexpr std::array<ossia::string_view, 3> internal_units()
  {
    constexpr_return(ossia::make_string_array("none", "none", "none"));
  }

  using value_type = vec3f;

  static strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self)
  {
    return make_vec(
             self.dataspace_value[2] * 255, self.dataspace_value[1] * 255,
             self.dataspace_value[0] * 255, 255);
  }

  static value_type from_neutral(strong_value<neutral_unit> self)
  {
    return make_vec(
             self.dataspace_value[2] / 255., self.dataspace_value[1] / 255.,
             self.dataspace_value[0] / 255.);
  }
};

struct OSSIA_EXPORT argb_u : public color_unit<argb_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("argb"));
  }

  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("argb"));
  }

  static constexpr std::array<ossia::string_view, 4> internal_units()
  {
    constexpr_return(ossia::make_string_array("none", "none", "none", "none"));
  }

  using value_type = vec4f;

  static strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self)
  {
    return make_vec(
        self.dataspace_value[3] * 255., self.dataspace_value[0] * 255.,
        self.dataspace_value[1] * 255., self.dataspace_value[2] * 255.);
  }

  static value_type from_neutral(strong_value<neutral_unit> self)
  {
    return make_vec(
        self.dataspace_value[3] / 255., self.dataspace_value[0] / 255.,
        self.dataspace_value[1] / 255., self.dataspace_value[2] / 255.);
  }
};

struct OSSIA_EXPORT argb8_u : public color_unit<argb8_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("argb8"));
  }

  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("argb"));
  }

  static constexpr std::array<ossia::string_view, 4> internal_units()
  {
    constexpr_return(ossia::make_string_array("none", "none", "none", "none"));
  }

  using value_type = vec4f;

  static strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self)
  {
    return {self.dataspace_value[1], self.dataspace_value[2],
            self.dataspace_value[3], self.dataspace_value[0]};
  }

  static value_type from_neutral(strong_value<neutral_unit> self)
  {
    return {self.dataspace_value[3], self.dataspace_value[0],
            self.dataspace_value[1], self.dataspace_value[2]};
  }
};

struct OSSIA_EXPORT hsv_u : public color_unit<hsv_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("hsv"));
  }

  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("hsv"));
  }
  static constexpr std::array<ossia::string_view, 3> internal_units()
  {
    constexpr_return(ossia::make_string_array("none", "none", "none"));
  }


  using value_type = vec3f;
  static strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self);

  static value_type from_neutral(strong_value<neutral_unit> self);
};

struct OSSIA_EXPORT hsl_u : public color_unit<hsl_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("hsl"));
  }

  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("hsl"));
  }

  static constexpr std::array<ossia::string_view, 3> internal_units()
  {
    constexpr_return(ossia::make_string_array("none", "none", "none"));
  }

  using value_type = vec3f;
};

struct OSSIA_EXPORT cmy8_u : public color_unit<cmy8_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("cmy8"));
  }

  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("cmy"));
  }

  static constexpr std::array<ossia::string_view, 3> internal_units()
  {
    constexpr_return(ossia::make_string_array("none", "none", "none"));
  }

  using value_type = vec3f;

  static strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self)
  {
    return make_vec(
        (255. - self.dataspace_value[0]),
        (255. - self.dataspace_value[1]),
        (255. - self.dataspace_value[2]),
         255);
  }

  static value_type from_neutral(strong_value<neutral_unit> self)
  {
    return make_vec(
        (255. - self.dataspace_value[0]),
        (255. - self.dataspace_value[1]),
        (255. - self.dataspace_value[2]));
  }
};

struct OSSIA_EXPORT cmyk8_u : public color_unit<cmyk8_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("cmyk8"));
  }

  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("cmyk"));
  }

  static constexpr std::array<ossia::string_view, 4> internal_units()
  {
    constexpr_return(ossia::make_string_array("none", "none", "none", "none"));
  }

  using value_type = vec4f;
};

struct OSSIA_EXPORT xyz_u : public color_unit<xyz_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("cie_xyz"));
  }

  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("xyz"));
  }

  static constexpr std::array<ossia::string_view, 3> internal_units()
  {
    constexpr_return(ossia::make_string_array("none", "none", "none"));
  }

  using value_type = vec3f;

  static strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self);

  static value_type from_neutral(strong_value<neutral_unit> self);
};

struct OSSIA_EXPORT yxy_u : public color_unit<yxy_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("cie_Yxy"));
  }

  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("Yxy"));
  }

  static constexpr std::array<ossia::string_view, 3> internal_units()
  {
    constexpr_return(ossia::make_string_array("none", "none", "none"));
  }

  using value_type = vec3f;
};

struct OSSIA_EXPORT hunter_lab_u : public color_unit<hunter_lab_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("hunter_lab"));
  }
  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("lab"));
  }
  static constexpr std::array<ossia::string_view, 3> internal_units()
  {
    constexpr_return(ossia::make_string_array("none", "none", "none"));
  }

  using value_type = vec3f;

  static strong_value<neutral_unit>
  to_neutral(strong_value<concrete_type> self);
  static value_type from_neutral(strong_value<neutral_unit> self);
};

struct OSSIA_EXPORT cie_lab_u : public color_unit<cie_lab_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("cie_lab"));
  }
  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("lab"));
  }
  static constexpr std::array<ossia::string_view, 3> internal_units()
  {
    constexpr_return(ossia::make_string_array("none", "none", "none"));
  }
  using value_type = vec3f;
};

struct OSSIA_EXPORT cie_luv_u : public color_unit<cie_luv_u>
{
  static constexpr auto text()
  {
    constexpr_return(ossia::make_string_array("cie_luv"));
  }
  static constexpr auto array_parameters()
  {
    constexpr_return(ossia::make_string_view("luv"));
  }
  static constexpr std::array<ossia::string_view, 3> internal_units()
  {
    constexpr_return(ossia::make_string_array("none", "none", "none"));
  }
  using value_type = vec3f;
};
}
