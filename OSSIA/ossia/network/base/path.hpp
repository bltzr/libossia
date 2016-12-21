#pragma once
#include <string>
#include <regex>
#include <ossia/network/base/name_validation.hpp>

namespace ossia
{
/**
 * \namespace regex_path
 * \brief Utilities to construct regexes to validate paths.
 *
 * Paths can be constructed like this :
 *
 * \code
 * {
 *   using namespace ossia::regex_path;
 *   auto path = device("foo") / "bar" / any_instance("baz");
 *   std::regex_match("foo:/bar/baz.2", path.regex()); // shall return true.
 * }
 * \endcode
 */
namespace regex_path
{
/**
 * @brief Base class for our paths
 */
struct path_element
{
  std::string address;
  path_element(std::string s)
    : address{std::move(s)}
  {

  }

  template<int N>
  path_element(const char (&s)[N])
    : address(s, N - 1)
  {

  }

  friend std::ostream& operator<<(std::ostream& s, const path_element& p)
  {
    return s << p.address;
  }

  operator std::string() const { return address; }
  std::regex regex() const { return std::regex(address); }
};

//! Represents a device in a path, e.g. "foo:"
struct device : public path_element
{
  explicit device(std::string s)
    : path_element{std::move(s) + ":"}
  {

  }
};

//! Can match nodes that are instances : foo:/bar, foo:/bar.1, etc.
struct any_instance : public path_element
{
  explicit any_instance(std::string s)
    : path_element{std::move(s) + "(\\.[0-9]+)?"}
  {

  }
};

//! Can match any node : foo:/bar, foo:/baz.1234, etc.
struct any_node
{
};

//! Can match any subpath : foo:/bar/baz, foo:/bar/bo.12/baz, etc.
struct any_path
{
};

//! Matches the end of an address
struct stop
{
};

inline path_element operator/(const path_element& lhs, const path_element& rhs)
{
  return path_element{lhs.address + "\\/" + rhs.address};
}

inline path_element operator/(const path_element& lhs, const any_instance& rhs)
{
  return path_element{lhs.address + "\\/" + rhs.address};
}

inline path_element operator/(const path_element& lhs, const any_node&)
{
  return path_element{lhs.address +
        "\\/[" + ossia::net::name_characters().to_string() + "]*"};
}

inline path_element operator/(const path_element& lhs, const any_path&)
{
  return path_element{lhs.address +
        "(\\/[" + ossia::net::name_characters().to_string() + "]*)+"};
}

inline path_element operator/(const path_element& lhs, const stop& rhs)
{
  return path_element{lhs.address + "$"};
}

}
}