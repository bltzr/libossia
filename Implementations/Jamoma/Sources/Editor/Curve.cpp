#include "Editor/JamomaCurve.h"
#include <Editor/CurveSegment/Linear.h>
#include <Editor/Value/SafeValue.h>
#include <iostream> //! \todo to remove. only here for debug purpose

# pragma mark -
# pragma mark Life cycle

template <typename X, typename Y>
JamomaCurve<X,Y>::
JamomaCurve() :
mInitialPointAbscissaDestination(nullptr),
mInitialPointOrdinateDestination(nullptr)
{}

template <typename X, typename Y>
JamomaCurve<X,Y>::
JamomaCurve(const JamomaCurve * other)
{}

template <typename X, typename Y>
shared_ptr<Curve<X,Y>> JamomaCurve<X,Y>::
clone() const
{
  return make_shared<JamomaCurve>(this);
}

template <typename X, typename Y>
JamomaCurve<X,Y>::
~JamomaCurve()
{}

# pragma mark -
# pragma mark Edition

template <typename X, typename Y>
bool JamomaCurve<X,Y>::
addPoint(unique_ptr<CurveSegment<Y>> segment, X abscissa, Y value)
{
  mPointsMap.emplace(abscissa, std::make_pair(value, std::move(segment)));

  return true;
}

template <typename X, typename Y>
bool JamomaCurve<X,Y>::
removePoint(X abscissa)
{
  return mPointsMap.erase(abscissa) > 0;
}

# pragma mark -
# pragma mark Execution

template <typename X, typename Y>
Y JamomaCurve<X,Y>::
valueAt(X abscissa) const
{
  X lastAbscissa = getInitialPointAbscissa();
  Y lastValue = getInitialPointOrdinate();

  auto end = mPointsMap.end();
  for (auto it = mPointsMap.begin(); it != end; ++it)
  {
    if (abscissa > lastAbscissa &&
        abscissa <= it->first)
    {
      lastValue = it->second.second->valueAt(
            ((double)abscissa - (double)lastAbscissa) / ((double)it->first - (double)lastAbscissa),
            lastValue,
            it->second.first);
      break;
    }
    else if (abscissa > it->first)
    {
      lastAbscissa = it->first;
      lastValue = it->second.first;
    }
    else
      break;
  }

  return lastValue;
}

# pragma mark -
# pragma mark Accessors
template<typename T>
constexpr nullptr_t OssiaType{};
template<>
constexpr OSSIA::CurveSegmentType OssiaType<int> = OSSIA::CurveSegmentType::INT;
template<>
constexpr OSSIA::CurveSegmentType OssiaType<float> = OSSIA::CurveSegmentType::FLOAT;
template<>
constexpr OSSIA::CurveSegmentType OssiaType<double> = OSSIA::CurveSegmentType::DOUBLE;
template<>
constexpr OSSIA::CurveSegmentType OssiaType<bool> = OSSIA::CurveSegmentType::BOOL;

template<typename X, typename Y>
OSSIA::CurveType JamomaCurve<X, Y>::getType() const
{
    return std::make_pair(OssiaType<X>, OssiaType<Y>);
}

template <typename X, typename Y>
X JamomaCurve<X,Y>::
getInitialPointAbscissa() const
{
  auto& node = mInitialPointAbscissaDestination.value;
  if(!node)
      return mInitialPointAbscissa;

  auto address = node->getAddress();

  if (!address)
    throw runtime_error("getting an address value using from an abscissa destination without address");

  address->pullValue();
  auto val = address->cloneValue();
  auto res = convertToTemplateTypeValue(val, mInitialPointAbscissaDestination.index.begin());
  return res;
}

template <typename X, typename Y>
Y JamomaCurve<X,Y>::
getInitialPointOrdinate() const
{
  auto& node = mInitialPointOrdinateDestination.value;
  if(!node)
    return mInitialPointOrdinate;

  auto address = node->getAddress();

  if (!address)
    throw runtime_error("getting an address value using from an ordinate destination without address");

  address->pullValue();
  auto val = address->cloneValue();
  auto res = convertToTemplateTypeValue(val, mInitialPointOrdinateDestination.index.begin());
  return res;
}

template <typename X, typename Y>
void JamomaCurve<X,Y>::
setInitialPointAbscissa(X value)
{
  mInitialPointAbscissa = value;
}

template <typename X, typename Y>
void JamomaCurve<X,Y>::
setInitialPointOrdinate(Y value)
{
  mInitialPointOrdinate = value;
}

template <typename X, typename Y>
const Destination& JamomaCurve<X,Y>::
getInitialPointAbscissaDestination() const
{
  return mInitialPointAbscissaDestination;
}

template <typename X, typename Y>
const Destination& JamomaCurve<X,Y>::
getInitialPointOrdinateDestination() const
{
  return mInitialPointOrdinateDestination;
}

template <typename X, typename Y>
void JamomaCurve<X,Y>::
setInitialPointAbscissaDestination(const Destination& destination)
{
  mInitialPointAbscissaDestination = destination;
}

template <typename X, typename Y>
void JamomaCurve<X,Y>::
setInitialPointOrdinateDestination(const Destination& destination)
{
  mInitialPointOrdinateDestination = destination;
}

template <typename X, typename Y>
std::map<X, std::pair<Y, CurveSegment<Y>*>> JamomaCurve<X,Y>::
getPointsMap() const
{
  map<X, pair<Y, CurveSegment<Y>*>> m;
  for(const auto& pair : mPointsMap)
  {
    m.insert(std::make_pair(pair.first, std::make_pair(pair.second.first, pair.second.second.get())));
  }
  return m;
}

# pragma mark -
# pragma mark Implementation specific

template <typename X, typename Y>
Y JamomaCurve<X,Y>::
convertToTemplateTypeValue(
    const SafeValue& value,
    vector<char>::const_iterator idx)
{
  struct visitor {
    vector<char>::const_iterator index;
    Y operator()(Int i) const   { return i.value; }
    Y operator()(Float f) const { return f.value; }
    Y operator()(Bool b) const  { return b.value; }
    Y operator()(Char c) const  { return c.value; }
    Y operator()(Vec2f vec) const { return vec.value[*index]; }
    Y operator()(Vec3f vec) const { return vec.value[*index]; }
    Y operator()(Vec4f vec) const { return vec.value[*index]; }
    Y operator()(const Tuple& t) const {
      auto& val = t.value[*index];
      return convertToTemplateTypeValue(val, index + 1);
    }

    Y operator()(Impulse) const { throw runtime_error("Cannot convert to a numeric type"); }
    Y operator()(const String& str) const { throw runtime_error("Cannot convert to a numeric type"); }
    Y operator()(const Destination& d) const { throw runtime_error("Cannot convert to a numeric type");; }
    Y operator()(const Behavior&) const { throw runtime_error("Cannot convert to a numeric type"); }
  } vis{idx};

  if(value.valid())
    return eggs::variants::apply(vis, value.v);
  throw runtime_error("Invalid variant");
}



namespace OSSIA
{
    // Curve implementation
    template<class X, class Y>
    std::shared_ptr<Curve<X, Y>> Curve<X, Y>::create()
    {
        return make_shared<JamomaCurve<X, Y>>();
    }
    template<class X, class Y>
    Curve<X, Y>::~Curve()
    {
    }

}

// Explicit instantiation
template class OSSIA_EXPORT OSSIA::Curve<double, bool>;
template class OSSIA_EXPORT OSSIA::Curve<double, int>;
template class OSSIA_EXPORT OSSIA::Curve<double, float>;

template class OSSIA_EXPORT OSSIA::Curve<bool, bool>;
template class OSSIA_EXPORT OSSIA::Curve<bool, int>;
template class OSSIA_EXPORT OSSIA::Curve<bool, float>;

template class OSSIA_EXPORT OSSIA::Curve<int, bool>;
template class OSSIA_EXPORT OSSIA::Curve<int, int>;
template class OSSIA_EXPORT OSSIA::Curve<int, float>;

template class OSSIA_EXPORT OSSIA::Curve<float, bool>;
template class OSSIA_EXPORT OSSIA::Curve<float, int>;
template class OSSIA_EXPORT OSSIA::Curve<float, float>;


