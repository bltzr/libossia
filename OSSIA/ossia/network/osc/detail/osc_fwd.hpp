#pragma once
#include <ossia/network/value/value.hpp>
#include <ossia/detail/string_view.hpp>
#include <ossia/network/value/value.hpp>
#include <ossia/network/base/parameter.hpp>
#include <ossia/network/domain/domain.hpp>
#include <oscpack/osc/OscOutboundPacketStream.h>
#include <oscpack/osc/OscReceivedElements.h>

// TODO better way would be to use Boost.Spirit :
// see
// http://stackoverflow.com/questions/23437778/comparing-3-modern-c-ways-to-convert-integral-values-to-strings
#include <boost/lexical_cast.hpp>
namespace oscpack
{
class OutboundPacketStream;
}
namespace ossia
{
namespace net
{

struct osc_outbound_visitor
{
  oscpack::OutboundPacketStream& p;

  template<typename T>
  void operator()(ossia::impulse, const T&) const
  {
  }

  template<typename T>
  void operator()(int32_t i, const T&) const
  {
    p << int32_t(i);
  }


  template<typename T>
  void operator()(float f, const T&) const
  {
    p << float(f);
  }

  template<typename T>
  void operator()(bool b, const T&) const
  {
    p << int32_t(b);
  }

  template<typename T>
  void operator()(char c, const T&) const
  {
    p << int32_t(c);
  }

  template<typename T>
  void operator()(const std::string& str, const T&) const
  {
    p << (ossia::string_view)str;
  }

  template<typename T>
  void operator()(ossia::vec2f vec, const T&) const
  {
    p << vec[0] << vec[1];
  }

  template<typename T>
  void operator()(ossia::vec3f vec, const T&) const
  {
    p << vec[0] << vec[1] << vec[2];
  }

  template<typename T>
  void operator()(ossia::vec4f vec, const T&) const
  {
    p << vec[0] << vec[1] << vec[2] << vec[3];
  }

  template<typename T>
  void operator()(const std::vector<ossia::value>& t, const T&) const
  {
    for (const auto& val : t)
    {
      ossia::apply(*this, val, const T&);
    }
  }

  void operator()(const std::array<float, 4>& value, ossia::rgba8_u) {
    uint32_t v = 0;
    v += ((uint32_t) value[0]) << 24;
    v += ((uint32_t) value[1]) << 16;
    v += ((uint32_t) value[2]) << 8;
    v += ((uint32_t) value[3]);

    p << int32_t(v);
    // etc... not sure about the formula / endinanness but you get it
  }

  void operator()(const std::vector<ossia::value>& value, ossia::rgba8_u) {
    if(value.size() == 4) {
      if(value[0].getType() == ossia::val_type::INT) // ????? && ... )
      {
        uint32_t v = 0;
        v += (uint32_t(value[0].get<int>())) << 24;
        v += (uint32_t(value[1].get<int>())) << 16;
        v += (uint32_t(value[2].get<int>())) << 8;
        v += (uint32_t(value[3].get<int>()));

        p << int32_t(v);
      }
      else if(value[0].getType() == ossia::val_type::FLOAT) // && ... )
      {
          uint32_t v = 0;
          v += (uint32_t(value[0].get<float>())) << 24;
          v += (uint32_t(value[1].get<float>())) << 16;
          v += (uint32_t(value[2].get<float>())) << 8;
          v += (uint32_t(value[3].get<float>()));

          p << int32_t(v);
      }
      else
      { /* just send the values ? there must have been an user error somewhere... */ }
    }
  }

  template<typename T>
  void operator()(const T&) const
  {
  }


};
}
}
