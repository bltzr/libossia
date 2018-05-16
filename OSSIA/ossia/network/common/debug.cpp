// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <ossia/network/dataspace/dataspace_visitors.hpp>
#include <ossia/network/value/value.hpp>
#include <ossia/network/base/parameter.hpp>
#include <ossia/network/base/node.hpp>
#include <ossia/network/common/debug.hpp>
#include <ossia/network/domain/domain.hpp>
#include <thread>
namespace ossia
{
namespace net
{

fmt::BasicWriter<char>&
operator<<(fmt::BasicWriter<char>& w, const node_base& n)
{
  auto parent = n.get_parent();
  while (parent != nullptr)
  {
    w << '\t';
    parent = parent->get_parent();
  }
  w << n.get_name();
  if (auto addr = n.get_parameter())
  {
    w << " : " << value_to_pretty_string(addr->value()) << ", AccessMode("
      << addr->get_access() << ")"
      << ", BoundingMode(" << addr->get_bounding() << ")"
      << ", Domain(" << addr->get_domain() << ")"
      << ", Unit(" << ossia::get_pretty_unit_text(addr->get_unit()) << ")";
  }

  return w;
}

void debug_recursively(fmt::MemoryWriter& w, const node_base& n)
{
  w << n << "\n";
  for (auto& cld : n.children())
    debug_recursively(w, *cld);
}

fmt::BasicWriter<char>& operator<<(fmt::BasicWriter<char>& w, access_mode a)
{
  switch (a)
  {
    case access_mode::SET:
      w << "set";
      break;
    case access_mode::GET:
      w << "get";
      break;
    case access_mode::BI:
      w << "bi";
      break;
    default:
      break;
  }
  return w;
}

fmt::BasicWriter<char>&
operator<<(fmt::BasicWriter<char>& w, ossia::bounding_mode b)
{
  switch (b)
  {
    case bounding_mode::FREE:
    {
      w << "free";
      break;
    }
    case bounding_mode::CLIP:
    {
      w << "clip";
      break;
    }
    case bounding_mode::WRAP:
    {
      w << "wrap";
      break;
    }
    case bounding_mode::FOLD:
    {
      w << "fold";
      break;
    }
    case bounding_mode::LOW:
    {
      w << "low";
      break;
    }
    case bounding_mode::HIGH:
    {
      w << "high";
      break;
    }
    default:
      break;
  }
  return w;
}
fmt::BasicWriter<char>&
operator<<(fmt::BasicWriter<char>& w, const ossia::domain& d)
{
  w << d.to_pretty_string();
  return w;
}
}
}
