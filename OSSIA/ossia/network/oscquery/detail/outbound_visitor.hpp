#pragma once
#include <ossia/network/value/value.hpp>
#include <oscpack/osc/OscOutboundPacketStream.h>
namespace oscpack
{
class OutboundPacketStream;
}
namespace ossia
{
namespace oscquery
{
struct OSSIA_EXPORT osc_outbound_visitor
{
public:
  oscpack::OutboundPacketStream& p;
  void operator()(impulse) const
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
    p << bool(b);
  }

  template<typename T>
  void operator()(char c, const T&) const
  {
    p << char(c);
  }

  template<typename T>
  void operator()(const std::string& str, const T&) const
  {
    p << (ossia::string_view)str;
  }

  template<typename T>
  void operator()(vec2f vec, const T&) const
  {
    p << vec[0] << vec[1];
  }

  template<typename T>
  void operator()(vec3f vec, const T&) const
  {
    p << vec[0] << vec[1] << vec[2];
  }

  void operator()(vec4f vec, ossia::rgba8_u) {
      uint32_t v = 0;
      v += ((uint32_t) vec[0]) << 24;
      v += ((uint32_t) vec[1]) << 16;
      v += ((uint32_t) vec[2]) << 8;
      v += ((uint32_t) vec[3]);

      p << int32_t(v);
      // etc... not sure about the formula / endinanness but you get it
    }

  void operator()(vec4f vec, ossia::rgba8_u) {
      if(value.size() == 4) {
        if(value[0].getType() == ossia::val_type::INT) // ????? && ... )
        {
          uint32_t v = 0;
          v += (uint32_t(vec[0].get<int>())) << 24;
          v += (uint32_t(vec[1].get<int>())) << 16;
          v += (uint32_t(vec[2].get<int>())) << 8;
          v += (uint32_t(vec[3].get<int>()));

          p << int32_t(v);
        }
        else if(value[0].getType() == ossia::val_type::FLOAT) // && ... )
        {
            uint32_t v = 0;
            v += (uint32_t(vec[0].get<float>())) << 24;
            v += (uint32_t(vec[1].get<float>())) << 16;
            v += (uint32_t(vec[2].get<float>())) << 8;
            v += (uint32_t(vec[3].get<float>()));

            p << int32_t(v);
        }
        else
        { /* just send the values ? there must have been an user error somewhere... */ }
      }
    }

  template<typename T>
  void operator()(vec4f vec, const T&) const
  {
    p << vec[0] << vec[1] << vec[2] << vec[3];
  }

  template<typename T>
  void operator()(const std::vector<value>& t, const T&) const
  {
    for (const auto& val : t)
    {
      val.apply(*this);
    }
  }

  template<typename T>
  void operator()() const
  {
  }
};
}
}
