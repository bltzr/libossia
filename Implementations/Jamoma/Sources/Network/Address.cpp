#include "Network/JamomaAddress.h"
#include "Editor/Value.h"
#include "Network/NetworkLogger.h"

#include <iostream> //! \todo to remove. only here for debug purpose

# pragma mark -
# pragma mark Life cycle

JamomaAddress::JamomaAddress(weak_ptr<Node> node, TTObject aData) :
mNode(node),
mObject(aData),
mObjectValueCallback("callback"),
mValue(new Impulse()),
mValueType(Type::IMPULSE),
mAccessMode(AccessMode::BI),
mBoundingMode(BoundingMode::FREE),
mRepetitionFilter(false)
{
  if (mObject.valid())
  {
    // prepare callback for value observation
    TTValue args(TTPtr(this), mObject);
    mObjectValueCallback.set("baton", args);
    mObjectValueCallback.set("function", TTPtr(&JamomaAddress::TTValueCallback));
  }

  // To set-up the protocol cache.
  // Note : the cache works because the nodes cannot change parents.
  getProtocol();
  mTextualAddress = getAddressFromNode(*mNode.lock());
}

JamomaAddress::~JamomaAddress()
{
  std::lock_guard<std::mutex> lock(mValueMutex);
  if (mValue)
    delete mValue;
}

Address::~Address()
{}

# pragma mark -
# pragma mark Network

const shared_ptr<Node> JamomaAddress::getNode() const
{
  return mNode.lock();
}

void JamomaAddress::pullValue()
{
  // use the device protocol to pull address value
  getProtocol().pullAddressValue(*this);
}

Address & JamomaAddress::pushValue(const Value& value)
{
  setValue(value);

  // use the device protocol to push address value
  getProtocol().pushAddressValue(*this);

  return *this;
}

Address & JamomaAddress::pushValue()
{
  // use the device protocol to push address value
  getProtocol().pushAddressValue(*this);

  return *this;
}

# pragma mark -
# pragma mark Accessors

const Value * JamomaAddress::getValue() const
{
  return mValue;
}

std::unique_ptr<OSSIA::Value> JamomaAddress::cloneValue(std::vector<char> index) const
{
  std::lock_guard<std::mutex> lock(mValueMutex);

  if (mValue == nullptr)
    throw runtime_error("cloning null value");

  if (index.empty() || mValueType != Type::TUPLE)
  {
    return std::unique_ptr<OSSIA::Value>{mValue->clone()};
  }
  else if (index.size() == 1)
  {
    // clone value from tuple element at index
    auto tuple = static_cast<const Tuple*>(mValue);
    return std::unique_ptr<OSSIA::Value>{tuple->value[index[0]]->clone()};
  }
  else
  {
    // create a new tuple from tuple's values at index
    auto tuple = static_cast<const Tuple*>(mValue);
    vector<const Value*> values;

    for (char i : index)
    {
      values.push_back(tuple->value[i]->clone());
    }

    return std::make_unique<Tuple>(values);
  }
}

Address & JamomaAddress::setValue(const Value& value)
{
  std::lock_guard<std::mutex> lock(mValueMutex);

  // clear former value
  delete mValue;
  mValue = nullptr;

  // set value querying the value from another address
  if (value.getType() == Type::DESTINATION &&
      mValueType != Type::DESTINATION)
  {
    auto& destination = static_cast<const Destination&>(value);
    auto address = destination.value->getAddress();

    if (address)
    {
      if (address->getValueType() == mValueType)
      {
        address->pullValue();
        mValue = address->cloneValue().release();
      }
      else
        throw runtime_error("setting an address value using a destination with a bad type address");
    }
    else
    {
      throw runtime_error("setting an address value using a destination without address");
    }
  }

  // copy the new value
  else
  {
    if(mValueType != value.getType())
    {
        mValueType = value.getType();

        if (mObject.name() != kTTSym_Mirror)
        {
          if (mValueType == Type::IMPULSE)
            mObject.set("type", kTTSym_none);
          else if (mValueType == Type::BOOL)
            mObject.set("type", kTTSym_boolean);
          else if (mValueType == Type::INT)
            mObject.set("type", kTTSym_integer);
          else if (mValueType == Type::FLOAT)
            mObject.set("type", kTTSym_decimal);
          else if (mValueType == Type::CHAR)
            mObject.set("type", kTTSym_string);
          else if (mValueType == Type::STRING)
            mObject.set("type", kTTSym_string);
          else if (mValueType == Type::TUPLE)
            mObject.set("type", kTTSym_array);
          else if (mValueType == Type::DESTINATION)
            mObject.set("type", kTTSym_string);
        }

    }
    mValue = value.clone();

  }

  return *this;
}

Type JamomaAddress::getValueType() const
{
  return mValueType;
}

Address & JamomaAddress::setValueType(Type type)
{
  mValueType = type;

  if (mObject.name() != kTTSym_Mirror)
  {
    if (mValueType == Type::IMPULSE)
      mObject.set("type", kTTSym_none);
    else if (mValueType == Type::BOOL)
      mObject.set("type", kTTSym_boolean);
    else if (mValueType == Type::INT)
      mObject.set("type", kTTSym_integer);
    else if (mValueType == Type::FLOAT)
      mObject.set("type", kTTSym_decimal);
    else if (mValueType == Type::CHAR)
      mObject.set("type", kTTSym_string);
    else if (mValueType == Type::STRING)
      mObject.set("type", kTTSym_string);
    else if (mValueType == Type::TUPLE)
      mObject.set("type", kTTSym_array);
    else if (mValueType == Type::DESTINATION)
      mObject.set("type", kTTSym_string);
  }

  // initialize the value member
  initValue();

  return *this;
}

AccessMode JamomaAddress::getAccessMode() const
{
  return mAccessMode;
}

Address & JamomaAddress::setAccessMode(AccessMode accessMode)
{
  mAccessMode = accessMode;

  if (mObject.name() != kTTSym_Mirror)
  {
    if (mAccessMode == AccessMode::BI)
      mObject.set("service", kTTSym_parameter);
    else if (mAccessMode == AccessMode::SET)
      mObject.set("service", kTTSym_message);
    else if (mAccessMode == AccessMode::GET)
      mObject.set("service", kTTSym_return);
  }

  return *this;
}

const shared_ptr<Domain> & JamomaAddress::getDomain() const
{
  return mDomain;
}

Address & JamomaAddress::setDomain(shared_ptr<Domain> domain)
{
  mDomain = domain;

  TTValue range, v;

  if(mDomain)
  {
      if (mDomain->getValues().empty())
      {
          convertValueIntoTTValue(*mDomain->getMin(), v);
          range.append(v);

          convertValueIntoTTValue(*mDomain->getMax(), v);
          range.append(v);
      }
      else
      {
          for (const auto & e : mDomain->getValues())
          {
              convertValueIntoTTValue(*e, v);
              range.append(v);
          }
      }
  }

  if (mObject.name() != kTTSym_Mirror)
  {
    mObject.set("rangeBounds", range);
  }

  return *this;
}

BoundingMode JamomaAddress::getBoundingMode() const
{
  return mBoundingMode;
}

Address & JamomaAddress::setBoundingMode(BoundingMode boundingMode)
{
  mBoundingMode = boundingMode;

  if (mObject.name() != kTTSym_Mirror)
  {
    if (mBoundingMode == BoundingMode::FREE)
      mObject.set("rangeClipmode", kTTSym_none);
    else if (mBoundingMode == BoundingMode::CLIP)
      mObject.set("rangeClipmode", kTTSym_both);
    else if (mBoundingMode == BoundingMode::WRAP)
      mObject.set("rangeClipmode", kTTSym_wrap);
    else if (mBoundingMode == BoundingMode::FOLD)
      mObject.set("rangeClipmode", kTTSym_fold);
  }

  return *this;
}

bool JamomaAddress::getRepetitionFilter() const
{
  return mRepetitionFilter;
}

Address & JamomaAddress::setRepetitionFilter(bool repetitionFilter)
{
  mRepetitionFilter = repetitionFilter;

  if (mObject.name() != kTTSym_Mirror)
  {
    mObject.set("repetitionsFilter", repetitionFilter);
  }

  return *this;
}

# pragma mark -
# pragma mark Callback

Address::iterator JamomaAddress::addCallback(ValueCallback callback)
{
  auto it = CallbackContainer::addCallback(std::move(callback));

  if (callbacks().size() == 1)
  {
    // use the device protocol to start address value observation
    getProtocol().observeAddressValue(shared_from_this(), true);

    //! \debug
    //cout << "opening listening on " << buildNodePath(mNode.lock()) << endl;
  }

  return it;
}

void JamomaAddress::removeCallback(Address::iterator callback)
{
  CallbackContainer::removeCallback(callback);

  if (callbacks().size() == 0)
  {
    // use the device protocol to stop address value observation
    getProtocol().observeAddressValue(shared_from_this(), false);

    //! \debug
    //cout << "closing listening on " << buildNodePath(mNode.lock()) << endl;
  }
}

# pragma mark -
# pragma mark Implementation specific

TTErr JamomaAddress::TTValueCallback(const TTValue& baton, const TTValue& value)
{
  JamomaAddress * self = static_cast<JamomaAddress*>(TTPtr(baton[0]));
  TTObject aData = baton[1];

  // check data object
  if (aData.instance() == self->mObject.instance())
  {
    self->setValue(value);

    // We clone the value to prevent crashes
    // if it is rewritten afterwards.
    try {
        auto val = self->cloneValue();
        if(auto logger = self->getProtocol().getLogger())
        {
            if(auto& log_callback = logger->getInboundLogCallback())
                log_callback(self->getTextualAddress() + " <= " + getValueAsString(*self->getValue()));
        }
        self->send(*val);
    }
    catch(...) {
        return kTTErrGeneric;
    }

    return kTTErrNone;
  }

  return kTTErrGeneric;
}

void JamomaAddress::initValue()
{
  if(mValue)
  {
      delete mValue;
      mValue = nullptr;
  }

  switch(mValueType)
  {
      case Type::IMPULSE:
          mValue = new Impulse;
          break;
      case Type::BOOL:
          mValue = new Bool;
          break;
      case Type::INT:
          mValue = new Int;
          break;
      case Type::FLOAT:
          mValue = new Float;
          break;
      case Type::CHAR:
          mValue = new Char;
          break;
      case Type::STRING:
          mValue = new String;
          break;
      case Type::TUPLE:
          mValue = new Tuple;
          break;
      case Type::VEC2F:
          mValue = new Vec2f;
          break;
      case Type::VEC3F:
          mValue = new Vec3f;
          break;
      case Type::VEC4F:
          mValue = new Vec4f;
          break;
      case Type::DESTINATION:
          mValue = new Destination;
          break;
  }
}

bool JamomaAddress::pullValue(TTValue& value) const
{
  return !mObject.get("value", value);
}

bool JamomaAddress::pushValue(const TTValue& value) const
{
  TTErr err;

  if (mObject.name() == "Data")
    err = mObject.send("Command", value);
  else
    err = mObject.set("value", value);

  return !err;
}

void JamomaAddress::getValue(TTValue& value) const
{
  std::lock_guard<std::mutex> lock(mValueMutex);

  // convert current value
  convertValueIntoTTValue(*mValue, value);
}

void JamomaAddress::setValue(const TTValue& value)
{
  std::lock_guard<std::mutex> lock(mValueMutex);

  // clear former value
  delete mValue;

  // store new value
  mValue = convertTTValueIntoValue(value, mValueType);
}

void JamomaAddress::observeValue(bool enable)
{
  TTAttributePtr attribute;
  mObject.instance()->findAttribute("value", &attribute);

  if (enable)
  {
    attribute->registerObserverForNotifications(mObjectValueCallback);
  }
  else
  {
    attribute->unregisterObserverForNotifications(mObjectValueCallback);
  }

  // for Mirror object : enable listening
  if (mObject.name() == kTTSym_Mirror)
  {
    TTMirrorPtr(mObject.instance())->enableListening(*attribute, enable);
  }
}

Value * JamomaAddress::convertTTValueIntoValue(const TTValue& v, Type valueType) const
{
  switch (valueType)
  {
    case Type::IMPULSE :
    {
      return new OSSIA::Impulse();
    }

    case Type::BOOL :
    {
      if (v.size() == 1)
        return new OSSIA::Bool(v[0]);

      return new OSSIA::Bool();
    }

    case Type::INT :
    {
      if (v.size() == 1)
        return new OSSIA::Int(v[0]);

      return new OSSIA::Int();
    }

    case Type::FLOAT :
    {
      if (v.size() == 1)
        return new OSSIA::Float(v[0]);

      return new OSSIA::Float();
    }

    case Type::CHAR :
    {
      if (v.size() == 1)
      {
        if (v[0].type() == kTypeString)
        {
          char* c_value = TTString(v[0]).data();
          return new OSSIA::Char(c_value[0]);
        }
      }

      return new OSSIA::Char();
    }

    case Type::STRING :
    {
      if (v.size() == 1)
      {
        if (v[0].type() == kTypeSymbol)
        {
          TTSymbol s_value = v[0];
          return new OSSIA::String(s_value.c_str());
        }
      }

      return new OSSIA::String();
    }

    case Type::DESTINATION :
    {
      throw runtime_error("convertion to destination value is not handled");
      /*
       if (v.size() == 1)
       {
       if (v[0].type() == kTypeSymbol)
       {
       //! \todo retreive the Address from the symbol
       //TTAddress s_value = v[0];

       return new OSSIA::Destination();
       }
       }

       return new OSSIA::Destination();
       */
    }

    case Type::BEHAVIOR :
    {
      break;
    }

    case Type::VEC2F :
    {
      const constexpr int n = 2;
      auto vec = new Vec2f;

      if(v.size() != n)
        return vec;

      std::array<TTValue, n> vals{v[0], v[1]};
      std::array<TTDataType, n> types{v[0].type(), v[1].type()};
      if(std::any_of(types.begin(), types.end(),
                     [] (auto t) { return t != kTypeFloat32 || t != kTypeFloat64; }))
        return vec;

      for(int i = 0; i < n; i++)
        vec->value[i] = vals[i];

      return vec;
    }

    case Type::VEC3F :
    {
      const constexpr int n = 3;
      auto vec = new Vec3f;

      if(v.size() != n)
        return vec;

      std::array<TTValue, n> vals{v[0], v[1], v[2]};
      std::array<TTDataType, n> types{v[0].type(), v[1].type(), v[2].type()};
      if(std::any_of(types.begin(), types.end(),
                     [] (auto t) { return t != kTypeFloat32 || t != kTypeFloat64; }))
        return vec;

      for(int i = 0; i < n; i++)
        vec->value[i] = vals[i];

      return vec;
    }

    case Type::VEC4F :
    {
      const constexpr int n = 4;
      auto vec = new Vec4f;

      if(v.size() != n)
        return vec;

      std::array<TTValue, n> vals{v[0], v[1], v[2], v[3]};
      std::array<TTDataType, n> types{v[0].type(), v[1].type(), v[2].type(), v[3].type()};
      if(std::any_of(types.begin(), types.end(),
                     [] (auto t) { return t != kTypeFloat32 || t != kTypeFloat64; }))
        return vec;

      for(int i = 0; i < n; i++)
        vec->value[i] = vals[i];

      return vec;
    }


    case Type::TUPLE :
    {
      vector<Value*> t_value;

      for (const auto & e : v)
      {
        Type type;

        if (e.type() == kTypeBoolean)
        {
          type = Type::BOOL;
        }
        else if (e.type() == kTypeInt8 || e.type() == kTypeUInt8 ||
                 e.type() == kTypeInt16 || e.type() == kTypeUInt16 ||
                 e.type() == kTypeInt32 || e.type() == kTypeUInt32 ||
                 e.type() == kTypeInt64 || e.type() == kTypeUInt64)
        {
          type = Type::INT;
        }
        else if (e.type() == kTypeFloat32 || e.type() == kTypeFloat64)
        {
          type = Type::FLOAT;
        }
        else if (e.type() == kTypeSymbol)
        {
          type = Type::STRING;
        }
        else
        {
          continue;
        }

        TTValue t(e);
        t_value.push_back(convertTTValueIntoValue(t, type));
      }

      return new OSSIA::Tuple(std::move(t_value));
    }
  }

  return nullptr;
}

void JamomaAddress::convertValueIntoTTValue(const Value& value, TTValue & v) const
{
  switch (value.getType())
  {
    case Type::IMPULSE :
    {
      break;
    }

    case Type::BOOL :
    {
      auto& b = static_cast<const Bool&>(value);
      v = TTBoolean(b.value);
      break;
    }

    case Type::INT :
    {
      auto& i = static_cast<const Int&>(value);
      v = TTInt32(i.value);
      break;
    }

    case Type::FLOAT :
    {
      auto& f = static_cast<const Float&>(value);
      v = TTFloat64(f.value);
      break;
    }

    case Type::CHAR :
    {
      auto& c = static_cast<const Char&>(value);
      v = TTSymbol(c.value);
      break;
    }

    case Type::STRING :
    {
      auto& s = static_cast<const String&>(value);
      v = TTSymbol(s.value);
      break;
    }

    case Type::DESTINATION :
    {
      auto& d = static_cast<const Destination&>(value);
      v = TTAddress(buildNodePath(d.value).data());
      break;
    }

    case Type::BEHAVIOR :
    {
      break;
    }

    case Type::VEC2F :
    {
      auto& vec = static_cast<const Vec2f&>(value);
      v = TTValue{vec.value[0], vec.value[1]};
      break;
    }

    case Type::VEC3F :
    {
      auto& vec = static_cast<const Vec3f&>(value);
      v = TTValue{vec.value[0], vec.value[1], vec.value[2]};
      break;
    }

    case Type::VEC4F :
    {
      auto& vec = static_cast<const Vec4f&>(value);
      v = TTValue{vec.value[0], vec.value[1], vec.value[2], vec.value[3]};
      break;
    }

    case Type::TUPLE :
    {
      auto& t = static_cast<const Tuple&>(value);

      for (const auto & e : t.value)
      {
        TTValue n;
        convertValueIntoTTValue(*e, n);

        if (n.size())
          v.append(n[0]);
      }

      break;
    }
  }
}

string JamomaAddress::buildNodePath(shared_ptr<Node> node) const
{
  string path;
  string name = node->getName();
  shared_ptr<Node> parent = node->getParent();

  if (parent != nullptr)
  {
    path += buildNodePath(parent);
    if (path != "/")
      path += "/";
    path += name;
  }
  else
  {
    //! \todo use device name
    path = name;
  }

  return path;
}

Protocol& JamomaAddress::getProtocol() const
{
    if(auto cached_proto = mProtocolCache.lock())
    {
        return *cached_proto.get();
    }
    else
    {
        auto node = mNode.lock();
        if(!node)
            return getDummyProtocol();
        auto dev = node->getDevice();
        if(!dev)
            return getDummyProtocol();
        auto proto = dev->getProtocol();
        if(!proto)
            return getDummyProtocol();

        // Save in the cache
        mProtocolCache = proto;
        return *proto.get();
    }
}

Protocol& getDummyProtocol()
{
    struct DummyProtocol : public Protocol
    {
            Type getType() const override { return Type::OSC; }
            bool pullAddressValue(Address&) const override { return true; }
            bool pushAddressValue(const Address&) const override { return true; }
            bool observeAddressValue(std::shared_ptr<Address>, bool) const override { return false; }
            bool updateChildren(Node&) const override { return false; }
            void setLogger(std::shared_ptr<NetworkLogger>) override { }
            std::shared_ptr<NetworkLogger> getLogger() const override { return{}; }
    };

    static DummyProtocol proto;
    return proto;
}



static void getAddressFromNode_rec(
        const OSSIA::Node& node,
        std::vector<std::string>& str)
{
    if(auto p = node.getParent())
        getAddressFromNode_rec(*p, str);

    str.push_back(node.getName());
}
std::string getAddressFromNode(const OSSIA::Node& node)
{
    std::vector<std::string> vec;
    getAddressFromNode_rec(node, vec);

    // vec cannot be empty.

    int i = 0;

    std::string str;
    str.reserve(vec.size() * 5);
    str.append(vec.at(i++));
    str.append(":/");

    int n = vec.size();
    for(; i < n - 1; i++)
    {
        str.append(vec.at(i));
        str.append("/");
    }
    if((n - 1) > 0)
        str.append(vec.at(n-1));

    return str;
}

std::string getTupleAsString(const OSSIA::Tuple& tuple)
{
    std::stringstream s;

    int n = tuple.value.size();

    s << "[";
    for(int i = 0; i < n; i++)
    {
        const OSSIA::Value& val = *tuple.value.at(i);
        switch(val.getType())
        {
            case OSSIA::Type::INT:
                s << "int: " << static_cast<const OSSIA::Int&>(val).value;
                break;
            case OSSIA::Type::FLOAT:
                s << "float: " << static_cast<const OSSIA::Float&>(val).value;
                break;
            case OSSIA::Type::BOOL:
                s << "bool: " << (static_cast<const OSSIA::Bool&>(val).value ? "true" : "false");
                break;
            case OSSIA::Type::STRING:
                s << "string: " << static_cast<const OSSIA::String&>(val).value;
                break;
            case OSSIA::Type::CHAR:
                s << "char: " << static_cast<const OSSIA::Char&>(val).value;
                break;
            case OSSIA::Type::IMPULSE:
                s << "impulse";
                break;
            case OSSIA::Type::TUPLE:
                s << "tuple: " << getTupleAsString(static_cast<const OSSIA::Tuple&>(val));
                break;
            case OSSIA::Type::DESTINATION:
                s << "destination";
                break;
        }

        if(i < n - 1)
            s << ", ";
    }
    s << "]";

    return s.str();
}

std::string getValueAsString(const OSSIA::Value& val)
{
    std::stringstream s;
    switch(val.getType())
    {
        case OSSIA::Type::INT:
            s << "int: " << static_cast<const OSSIA::Int&>(val).value;
            break;
        case OSSIA::Type::FLOAT:
            s << "float: " << static_cast<const OSSIA::Float&>(val).value;
            break;
        case OSSIA::Type::BOOL:
            s << "bool: " << (static_cast<const OSSIA::Bool&>(val).value ? "true" : "false");
            break;
        case OSSIA::Type::STRING:
            s << "string: " << static_cast<const OSSIA::String&>(val).value;
            break;
        case OSSIA::Type::CHAR:
            s << "char: " << static_cast<const OSSIA::Char&>(val).value;
            break;
        case OSSIA::Type::IMPULSE:
            s << "impulse";
            break;
        case OSSIA::Type::TUPLE:
            s << "tuple: " << getTupleAsString(static_cast<const OSSIA::Tuple&>(val));
            break;
        case OSSIA::Type::DESTINATION:
            s << "destination";
            break;
    }

    return s.str();
}
