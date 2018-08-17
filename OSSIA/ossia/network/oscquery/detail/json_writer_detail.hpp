#pragma once
#include <ossia/detail/json.hpp>
#include <ossia/network/base/node_attributes.hpp>
#include <ossia/network/dataspace/dataspace_visitors.hpp>

namespace ossia
{
namespace oscquery
{
namespace detail
{
//! Implementation of the JSON serialisation mechanism for oscquery
struct json_writer_impl
{
  using writer_t = rapidjson::Writer<rapidjson::StringBuffer>;
  writer_t& writer;

  void writeKey(ossia::string_view k) const;

  void writeValue(const ossia::value& val) const;
  void writeValue(ossia::bounding_mode b) const;
  void writeValue(ossia::access_mode b) const;
  void writeValue(const ossia::domain& d) const;
  void writeValue(const ossia::net::tags& tags) const;
  void writeValue(int32_t i) const;
  void writeValue(float i) const;
  void writeValue(double i) const;
  void writeValue(bool i) const;
  void writeValue(const std::string& i) const;
  void writeValue(const ossia::repetition_filter& i) const;
  void writeValue(const ossia::net::instance_bounds& i) const;
  void writeValue(const ossia::unit_t& d) const;

  template <typename T>
  void writeValue(const optional<T>& t) const
  {
    if (t)
    {
      writeValue(*t);
    }
    else
    {
      writer.Null();
    }
  }

  //! Writes a single attribute
  void writeAttribute(
      const ossia::net::node_base& n, ossia::string_view method) const;

  //! Writes only the attributes
  void writeNodeAttributes(const ossia::net::node_base& n) const;

  //! Writes a node recursively. Creates a new object.
  void writeNode(const ossia::net::node_base& n);

  template <typename Unit, typename = void>
  struct internal_units
  {
      std::vector<std::string> operator()(unit_t u) {
        return {get_pretty_unit_text(u)};
      }
  };

  template <typename Unit>
  using enable_if_multidimensional
      = std::enable_if_t<Unit::is_multidimensional::value>;

  template <typename Unit>
  struct internal_units<Unit, enable_if_multidimensional<Unit>>
  {

    std::vector<std::string> operator()(cartesian_2d_u u) {
      return {"distance.m", "distance.m"};
    }
    std::vector<std::string> operator()(cartesian_3d_u u) {
      return {"distance.m", "distance.m", "distance.m"};
    }
    std::vector<std::string> operator()(spherical_u u) {
      return {"angle.degree", "angle.degree", "distance.m"};
    }
    std::vector<std::string> operator()(polar_u u) {
      return {"angle.degree", "distance.m"};
    }
    std::vector<std::string> operator()(opengl_u u) {
      return {"distance.m", "distance.m", "distance.m"};
    }
    std::vector<std::string> operator()(cylindrical_u u) {
      return {"angle.degree", "distance.m", "distance.m"};
    }
    std::vector<std::string> operator()(quaternion_u u) {
      return {"distance.m", "distance.m", "distance.m", "distance.m"};
    }
    std::vector<std::string> operator()(euler_u u) {
      return {"angle.degree", "angle.degree", "angle.degree"};
    }
    std::vector<std::string> operator()(axis_u u) {
      return {"distance.m", "distance.m", "distance.m", "angle.degree"};
    }
    std::vector<std::string> operator()(rgba_u u) {
      return {"none", "none", "none", "none"};
    }
    std::vector<std::string> operator()(argb_u u) {
      return {"none", "none", "none", "none"};
    }
    std::vector<std::string> operator()(argb8_u u) {
      return {"none", "none", "none", "none"};
    }
    std::vector<std::string> operator()(rgb_u u) {
      return {"none", "none", "none"};
    }
    std::vector<std::string> operator()(bgr_u u) {
      return {"none", "none", "none"};
    }
    std::vector<std::string> operator()(cmy8_u u) {
      return {"none", "none", "none"};
    }
    std::vector<std::string> operator()(xyz_u u) {
      return {"none", "none", "none"};
    }
    std::vector<std::string> operator()(rgba8_u u) {
      return {"none"};
    }
    std::vector<std::string> operator()() {
      return {"none"};
    }
  };

  struct match_dataspace
  {
    template<typename Dataspace_T>
    std::vector<std::string> operator()(Dataspace_T d) {
      return ossia::apply_nonnull(internal_units<decltype(d)>{}, d);
    }
    std::vector<std::string> operator()() {
      return {};
    }
  };

  std::vector<std::string> get_oscquery_internal_units(const unit_t& u) const
  {
    return ossia::apply(match_dataspace{}, u.v);
  }


};
}
}
}
