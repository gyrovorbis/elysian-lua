#ifndef ELYSIAN_LUA_VARIANT_BASE_HPP
#define ELYSIAN_LUA_VARIANT_BASE_HPP

#include "elysian_lua_reference.hpp"
#include <cstring>

#define ELYSIAN_LUA_PROXY_TYPE_METAFIELD "__typename"

#define ELYSIAN_LUA_VARIANT_TYPE_BIT_POS 0
#define ELYSIAN_LUA_VARIANT_TYPE_BIT_MASK 0x0f
#define ELYSIAN_LUA_VARIANT_INTEGER_BIT_POS 4
#define ELYSIAN_LUA_VARIANT_INTEGER_BIT_MASK 0x10
#define ELYSIAN_LUA_VARIANT_WEAK_REF_BIT_POS 31

namespace elysian::lua {
template <bool is_const_iterator> class const_noconst_iterator;

class Variant;

using VariantKVPair = std::pair<Variant, Variant>;

class Variant : public StaticRefState {
  using iterator = const_noconst_iterator<false>;
  using const_iterator = const_noconst_iterator<true>;

  friend const_iterator;

protected:
  uint32_t _type = LUA_TNIL; // only least significant 4 bits represent
                             // LUA_TYPE, rest is metadata
  union Union {
    bool boolean;
    lua_Number number;
    lua_Integer integer;
    int ref = LUA_REFNIL;
    const char *string;
    void *luserdata;
  } _value;

  template <typename L> Variant _convertType(const L converter) const;
  template <typename L> Variant &_convertType(const L converter);

  static bool _lightConvertToBool(const Variant &source);
  static Variant &_convertToBool(const Variant &source,
                                        Variant &dest);

  template<typename NumberType=lua_Number>
  static NumberType _lightConvertToNumber(const Variant &source);
  template<typename NumberType=lua_Number>
  static Variant &_convertToNumber(const Variant &source,
                                          Variant &dest);

  void _setWeakRef(const bool val);
  void _setType(const int luaType, const bool isInteger = false);
  void _setRefDirect(const int luaRef);

public:
  Variant(void) = default;
  Variant(const std::initializer_list<VariantKVPair> &pairs);
  template <typename T> Variant(T *const userdata);
  Variant(std::nullptr_t null);
  Variant(const Variant &rhs);
  Variant(Variant &&rhs);
  Variant(const lua_Integer integer);
  Variant(const lua_Number number);
  Variant(const bool val);
  Variant(const char *const string);
  Variant(void *const usd);
  Variant(const lua_CFunction funcPtr);
  template <typename Ret, typename... Args>
  Variant(const std::function<Ret(Args...)> &closure);
  //template <typename T> Variant(const T fun);

  ~Variant(void);

  Variant &operator=(const std::initializer_list<VariantKVPair> &pairs);
  template <typename T> Variant &operator=(const T *const ud);
  template <typename T> Variant &operator=(T *const ud);
  Variant &operator=(const Variant &rhs);
  Variant &operator=(Variant &&rhs);
  Variant &operator=(const std::nullptr_t);
  Variant &operator=(const bool val);
  Variant &operator=(const lua_Integer num);
  Variant &operator=(const lua_Number num);
  Variant &operator=(const char *const string);
  Variant &operator=(void *const ptr);
  Variant &operator=(const lua_CFunction funcPtr);
  template <typename Ret, typename... Args>
  Variant &operator=(const std::function<Ret(Args...)> &closure);
  //template <typename T> Variant &operator=(const T func);

  template <typename T> bool operator==(const T *const ud) const;
  bool operator==(const Variant &rhs) const;
  bool operator==(const std::nullptr_t) const;
  bool operator==(const bool val) const;
  bool operator==(const lua_Integer num) const;
  bool operator==(const lua_Number num) const;
  bool operator==(void *const ptr) const;
  bool operator==(const char *const str) const;

  template <typename T> bool operator!=(const T *const ud) const;
  bool operator!=(const Variant &rhs) const;
  bool operator!=(const std::nullptr_t) const;
  bool operator!=(const bool val) const;
  bool operator!=(const lua_Integer num) const;
  bool operator!=(const lua_Number num) const;
  bool operator!=(void *const ptr) const;
  bool operator!=(const char *const str) const;

  template <typename... Args> Variant operator()(Args &&... args) const;

  // returns new Variant with the desired type
  Variant asBool(void) const;
  Variant asNumber(void) const;
  Variant asInteger(void) const;
  Variant asString(void) const;
  Variant asUserdata(void) const;

  // doesn't do conversions, so type better be known statically
  template <typename T> T getValue(void) const;

  int getRef(void) const;
  void setRef(const int ref);
  void setNil(void);

  template <typename T> void setValue(T *const ud);
  void setValue(const std::nullptr_t null);
  void setValue(const lua_Integer number);
  void setValue(const lua_Number number);
  void setValue(const char *const str);
  void setValue(const bool val);
  void setValue(void *const ptr);
  void setValue(const lua_CFunction funcPtr);
  template <typename Ret, typename... Args>
  void setValue(const std::function<Ret(Args...)> &closure);
  template <typename T> void setValue(const T func);
  void setValue(const std::initializer_list<VariantKVPair> &pairs);

  bool isWeakRef(void) const;

  bool isNil(void) const;
  bool isNilRef(void) const;
  bool isRefType(void) const;
  bool isIndexable(void) const;
  bool isObject(void) const;
  bool isValueType(void) const;
  bool isInteger(void) const;
  bool isUserdataType(void) const;
  bool isCFunction(void) const;
  int getType(void) const;
  const char *getTypeString(void) const;
  void setMetatable(const Variant &mt) const;
  //Variant getMetatable(void) const;
  int getLength(void) const;
  const void *getPointer(void) const;

  bool push(void) const;
  bool pull(void);

  Variant getValue(const Variant &key) const;
  //LuaFieldRef operator[](const Variant &key) const;

  // C++11-style iterators
  iterator begin(void);
  const_iterator begin(void) const;
  const_iterator cbegin(void) const;
  iterator end(void);
  const_iterator end(void) const;
  const_iterator cend(void) const;

  //bool iteratePairs(std::function<void(LuaVariant, LuaVariant)> callback);

  // static constructors
  static Variant fromStack(const int index = -1,
                                  const bool remove = true);
  static Variant fromRef(const int luaRef);
  static Variant fromWeakRef(const int ref);
  static Variant fromArray(const std::initializer_list<Variant> &array);
  template <typename C>
  static Variant fromSequenceContainer(const C &cont);
  static Variant makeWeakRef(const Variant &variant);
  static Variant newTable(int arraySize=0, int hashSize=0);
};

namespace stack_impl {

template<>
struct stack_checker<Variant> {
    static bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return true;
    }
};

template<>
struct stack_getter<Variant> {
    static Variant get(const ThreadViewBase* pBase, StackRecord& record, int index) {
        Variant v;
        v.pull();
        return v;
    }
};

template<>
struct stack_pusher<Variant> {
    static int push(const ThreadViewBase* pBase, StackRecord&, const Variant& v) {
        v.push();
        return 1;
    }
};

template<>
struct stack_pusher<std::initializer_list<VariantKVPair>> {
    static int push(const ThreadViewBase* pBase, StackRecord&, const std::initializer_list<VariantKVPair>& pairs) {
        pBase->pushNewTable(0, pairs.size());
        for (auto it : pairs) {
            pBase->setTableRaw(-1, it.first, it.second);
        }
        return 1;
    }
};
}

#if 0
class LuaFieldRef : public LuaManager {
  friend class Variant;
  friend class const_noconst_iterator<false>;
  friend class const_noconst_iterator<true>;

private:
  Variant _tableRef;
  Variant _key;
  mutable Variant _cachedValue;

public:
  LuaFieldRef(Variant tableRef, Variant key,
              Variant val = Variant());
  LuaFieldRef(const LuaFieldRef &rhs);
  LuaFieldRef(LuaFieldRef &&rhs);

  const Variant &getVariant(void) const;
  const Variant &getCachedVariant(void) const;
  void setCachedVariant(const Variant &variant);
  bool setVariant(const Variant &variant);
#ifndef ELYSIAN_ENGINE
  QVariant getQVariant(void) const;
  void setQVariant(QVariant variant);
#endif
  const Variant &getKey(void) const;
  void setKey(const Variant &key);
  const Variant &getTableRef(void) const;
  void setTableRef(const Variant &ref);

  LuaFieldRef operator[](const Variant &key) const;

  LuaFieldRef &operator=(const LuaFieldRef &rhs);
  LuaFieldRef &operator=(LuaFieldRef &&rhs);
  LuaFieldRef &operator=(const Variant &rhs);
  const Variant *operator->(void)const;
  const Variant &operator*(void)const;

  bool operator==(const LuaFieldRef &rhs) const;
  operator Variant(void) const;
};


template <bool is_const_iterator = true>
class const_noconst_iterator
    : public std::iterator<std::forward_iterator_tag, Variant>,
      public LuaManager {
  friend class const_noconst_iterator<true>;
  friend class Variant;

  typedef typename std::conditional<is_const_iterator, const Variant *,
                                    Variant *>::type
      DataStructurePointerType;
  typedef typename std::conditional<is_const_iterator, const Variant &,
                                    Variant &>::type ValueReferenceType;

public:
  const_noconst_iterator(const LuaFieldRef &fieldRef, const LuaVariant &funcRef)
      : _fieldRef(fieldRef), _nextFuncRef(funcRef) {}

  /**
   * Copy constructor. Allows for implicit conversion from a regular iterator to
   * a const_iterator
   */
  const_noconst_iterator(const const_noconst_iterator<false> &other)
      : _fieldRef(other._fieldRef), _nextFuncRef(other._nextFuncRef) {}

  /**
   * Equals comparison operator
   */
  bool operator==(const const_noconst_iterator &other) const {
    return _fieldRef == other._fieldRef;
  }

  /**
   * Not-equals comparison operator
   * @see operator==(const const_noconst_iterator&) const
   */
  bool operator!=(const const_noconst_iterator &other) const {
    return !(*this == other);
  }

  /**
   * Dereference operator
   * @return the value of the element this iterator is currently pointing at
   */
  LuaFieldRef &operator*(void) { return _fieldRef; }

  LuaFieldRef *operator->(void) { return &_fieldRef; }

  Variant first(void) const { return _fieldRef.getKey(); }

  Variant second(void) const { return _fieldRef.getCachedVariant(); }

  const_noconst_iterator operator++(void);
  /**
   * Postfix increment operator (e.g., it++)
   */
  const_noconst_iterator operator++(int) {
    // Use operator++()
    const const_noconst_iterator old(*this);
    ++(*this);
    return old;
  }

private:
  LuaFieldRef _fieldRef;
  LuaVariant _nextFuncRef;
};

// ======== INLINEZ ==========

inline LuaFieldRef::LuaFieldRef(Variant tableRef, Variant key,
                                Variant value)
    : _tableRef(std::move(tableRef)), _key(std::move(key)),
      _cachedValue(std::move(value)) {}

inline LuaFieldRef::LuaFieldRef(const LuaFieldRef &rhs) { *this = rhs; }

inline LuaFieldRef::LuaFieldRef(LuaFieldRef &&rhs) { *this = rhs; }

inline LuaFieldRef &LuaFieldRef::operator=(const LuaFieldRef &rhs) {
  _tableRef = rhs._tableRef;
  _key = rhs._key;
  _cachedValue = rhs._cachedValue;
  return *this;
}

inline LuaFieldRef &LuaFieldRef::operator=(LuaFieldRef &&rhs) {
  _tableRef = std::move(rhs._tableRef);
  _key = std::move(rhs._key);
  _cachedValue = std::move(rhs._cachedValue);
  return *this;
}

inline LuaFieldRef &LuaFieldRef::operator=(const Variant &rhs) {
  setVariant(rhs);
  return *this;
}

inline const Variant *LuaFieldRef::operator->(void)const {
  getVariant();
  return &_cachedValue;
}

inline const Variant &LuaFieldRef::operator*(void)const {
  getVariant();
  return _cachedValue;
}

inline bool LuaFieldRef::operator==(const LuaFieldRef &rhs) const {
  return _tableRef == rhs._tableRef && _key == rhs._key;
}

inline const Variant &LuaFieldRef::getKey(void) const { return _key; }

inline void LuaFieldRef::setKey(const Variant &key) { _key = key; }

inline const Variant &LuaFieldRef::getTableRef(void) const {
  return _tableRef;
}

inline void LuaFieldRef::setTableRef(const Variant &ref) {
  _tableRef = ref;
}

inline const Variant &LuaFieldRef::getCachedVariant(void) const {
  return _cachedValue;
}

inline void LuaFieldRef::setCachedVariant(const Variant &variant) {
  _cachedValue = variant;
}

inline const Variant &LuaFieldRef::getVariant(void) const {
  bool success = true;
  if (!_tableRef.isNil()) {
    success &= _tableRef.push();
    success &= _key.push();
    GY_ASSERT(success);
    lua_gettable(*_mainState, -2);
    lua_remove(*_mainState, -2);
    success &= _cachedValue.pull();
  }
  return _cachedValue;
}

inline bool LuaFieldRef::setVariant(const Variant &variant) {
  bool success = true;
  success &= _tableRef.push();
  success &= _key.push();
  success &= variant.push();
  GY_ASSERT(success);
  lua_settable(*_mainState, -3);
  lua_pop(*_mainState, 1);
  return success;
}

#ifndef ELYSIAN_ENGINE
inline QVariant LuaFieldRef::getQVariant(void) const {
  QVariant v;
  push(getVariant());
  OOLUA::pull(*_mainState, v);
  return v;
}
inline void LuaFieldRef::setQVariant(QVariant variant) {
  if (OOLUA::push(*_mainState, variant)) {
    setVariant(LuaVariant::fromStack());
  }
}

#endif

inline LuaFieldRef LuaFieldRef::operator[](const Variant &key) const {
  // forward through the variant's index operator
  return getVariant()[key];
}
#endif

template <typename T> inline Variant::Variant(T *const ref) {
  if (ref) {
    getThread()->push(ref);
    pull();
  } else {
    _setType(LUA_TNIL);
  }
}

inline Variant::Variant(
    const std::initializer_list<VariantKVPair> &pairs) {
  *this = pairs;
}

template <typename Ret, typename... Args>
Variant::Variant(const std::function<Ret(Args...)> &closure) {
  setValue<Ret, Args...>(closure);
}

inline Variant::Variant(const Variant &rhs) {
  *this = rhs;
}
inline Variant::Variant(std::nullptr_t /*null*/)
    : Variant() {}

inline Variant::Variant(Variant &&rhs) { *this = rhs; }

inline Variant &Variant::operator=(const Variant &rhs) {
  if (!rhs.isWeakRef()) { // Gracefully obtain copy of data for strong
                          // references
    switch (rhs.getType()) {
    case LUA_TNIL:
      setNil();
      break;
    case LUA_TBOOLEAN:
      setValue(rhs._value.boolean);
      break;
    case LUA_TNUMBER:
      if (rhs.isInteger())
        setValue(rhs._value.integer);
      else
        setValue(rhs._value.number);
      break;
    case LUA_TSTRING:
      setValue(rhs._value.string);
      break;
    case LUA_TLIGHTUSERDATA:
      setValue(rhs._value.luserdata);
      break;
    default:
      setRef(rhs.getRef());
    }
  } else { // Simply assume raw values, otherwise
    _setType(LUA_TNIL);
    _type = rhs._type;
    _value = rhs._value;
  }
  return *this;
}

inline Variant &Variant::operator=(Variant &&rhs) {
  _setType(LUA_TNIL); // clear out old shit
  _type = rhs._type;
  _value = rhs._value;
  if (!rhs.isWeakRef()) { // only steal references if this was a strong
                          // reference
    rhs._type = LUA_TNIL;
    rhs._value.ref = LUA_REFNIL;
  }
  return *this;
}

inline Variant::Variant(const char *const string) {
  setValue(string);
}
inline Variant::Variant(const bool val) { setValue(val); }
inline Variant::Variant(const lua_Number number) { setValue(number); }
inline Variant::Variant(const lua_Integer num) { setValue(num); }
inline Variant::Variant(void *const ptr) { setValue(ptr); }

inline Variant::Variant(const lua_CFunction funcPtr) {
  setValue(funcPtr);
}
//template <typename T> inline Variant::Variant(const T fun) {
//  setValue(fun);
//}

inline Variant::~Variant(void) { setNil(); }

template <typename T>
inline Variant &Variant::operator=(const T *const rhs) {
  setValue(rhs);
  return *this;
}

template <typename T>
inline Variant &Variant::operator=(T *const rhs) {
  setValue(rhs);
  return *this;
}

inline Variant &Variant::operator=(const bool val) {
  setValue(val);
  return *this;
}
inline Variant &Variant::operator=(const std::nullptr_t) {
  setValue(nullptr);
  return *this;
}
inline Variant &Variant::operator=(const lua_Integer num) {
  setValue(num);
  return *this;
}
inline Variant &Variant::operator=(const lua_Number num) {
  setValue(num);
  return *this;
}
inline Variant &Variant::operator=(const char *const string) {
  setValue(string);
  return *this;
}
inline Variant &Variant::operator=(void *const ptr) {
  setValue(ptr);
  return *this;
}
inline Variant &Variant::operator=(const lua_CFunction funcPtr) {
  setValue(funcPtr);
  return *this;
}
template <typename Ret, typename... Args>
Variant &
Variant::operator=(const std::function<Ret(Args...)> &closure) {
  setValue(closure);
  return *this;
}
#if 0
template <typename T> Variant &Variant::operator=(const T func) {
  setValue(make_function(func));
  return *this;
}
#endif

inline bool Variant::operator==(const Variant &rhs) const {
  const auto type = getType();
  if (type != rhs.getType())
    return false;
  switch (type) {
  case LUA_TNIL:
    return true;
  case LUA_TBOOLEAN:
    return _value.boolean == rhs._value.boolean;
  case LUA_TSTRING:
    return *this == rhs._value.string;
  case LUA_TLIGHTUSERDATA:
    return _value.luserdata == rhs._value.luserdata;
  case LUA_TNUMBER: // obey Lua's int vs float comparisons
  default:          // let Lua handle the comparison for complex types
    push();
    rhs.push();
    const bool equality = getThread()->compare(-1, -2, LUA_OPEQ);
    getThread()->pop(2);
    return equality;
  }
}

template <typename T>
inline bool Variant::operator==(const T *const ud) const {
  // Purposely defer comparison to Lua, to honor overloaded comparison
  // metamethods
  bool equal = false;
  if (push()) {
    if(getThread()->push(ud)) {
      equal = getThread()->compare(-1, -2, LUA_OPEQ);
      getThread()->pop();
    }
    getThread()->pop();
  }
  return equal;
}

inline bool Variant::operator==(const std::nullptr_t) const {
  return isNil() ||
         (getType() == LUA_TLIGHTUSERDATA && _value.luserdata == nullptr);
}
inline bool Variant::operator==(const bool val) const {
  return getType() == LUA_TBOOLEAN && _value.boolean == val;
}
inline bool Variant::operator==(const lua_Integer num) const {
    bool equal = false;
    if(getType() == LUA_TNUMBER) {
        if(isInteger()) equal = _value.integer == num;
        else equal = _value.number == num;
    }
    return equal;
}
inline bool Variant::operator==(const lua_Number num) const {
    bool equal = false;
    if(getType() == LUA_TNUMBER) {
        if(isInteger()) equal = _value.integer == num;
        else equal = _value.number == num;
    }
    return equal;
}
inline bool Variant::operator==(void *const ptr) const {
  return ptr ? isUserdataType() && _value.luserdata == ptr : isNil();
}
inline bool Variant::operator==(const char *const str) const {
  return (getType() == LUA_TSTRING && (strcmp(_value.string, str) == 0));
};

template <typename T>
inline bool Variant::operator!=(const T *const ud) const {
  return !(*this == ud);
}
inline bool Variant::operator!=(const Variant &rhs) const {
  return !(*this == rhs);
}
inline bool Variant::operator!=(const std::nullptr_t) const {
  return !(*this == nullptr);
}
inline bool Variant::operator!=(const bool val) const {
  return !(*this == val);
}
inline bool Variant::operator!=(const lua_Integer num) const {
  return !(*this == num);
}
inline bool Variant::operator!=(const lua_Number num) const {
  return !(*this == num);
}
inline bool Variant::operator!=(void *const ptr) const {
  return !(*this == ptr);
}
inline bool Variant::operator!=(const char *const str) const {
  return !(*this == str);
}

inline bool Variant::isNil(void) const { return getType() == LUA_TNIL; }
inline bool Variant::isNilRef(void) const {
  return getType() == LUA_TNIL || (isRefType() && _value.ref == LUA_REFNIL);
}
inline bool Variant::isRefType(void) const {
  return !(getType() == LUA_TBOOLEAN || getType() == LUA_TNUMBER ||
           getType() == LUA_TNIL || getType() == LUA_TLIGHTUSERDATA ||
           getType() == LUA_TSTRING);
}

inline bool Variant::isObject(void) const { return isRefType(); }
inline bool Variant::isIndexable(void) const {
  return (getType() == LUA_TUSERDATA || getType() == LUA_TTABLE);
}
inline bool Variant::isValueType(void) const {
  return getType() == LUA_TNIL || !isRefType();
}
inline bool Variant::isInteger(void) const {
  return ((_type >> ELYSIAN_LUA_VARIANT_INTEGER_BIT_POS) & 0x1);
}
inline bool Variant::isUserdataType(void) const {
  return getType() == LUA_TUSERDATA || getType() == LUA_TLIGHTUSERDATA;
}
inline bool Variant::isCFunction(void) const {
  bool result = false;
  if (getType() == LUA_TFUNCTION) {
    if (push()) {
      if(getThread()->isCFunction(-1)) {
        result = true;
      }
      getThread()->pop(1);
    }
  }
  return result;
}

inline bool Variant::isWeakRef(void) const {
  return ((_type >> ELYSIAN_LUA_VARIANT_WEAK_REF_BIT_POS) & 0x1);
}

inline void Variant::_setWeakRef(const bool value) {
  _type &= ~(0x1u << ELYSIAN_LUA_VARIANT_WEAK_REF_BIT_POS);
  _type |= ((uint32_t)(value) << ELYSIAN_LUA_VARIANT_WEAK_REF_BIT_POS);
}

inline void Variant::_setType(const int luaType, const bool isInteger) {
  assert(!isInteger || luaType == LUA_TNUMBER);
  if (!isWeakRef()) { // only delete shit if we have ownership of it (are a
                      // strong reference)
    if (isRefType()) {
      getThread()->unref(LUA_REGISTRYINDEX, _value.ref);
    } else if (getType() == LUA_TSTRING) {
      free((void *)_value.string);
    } else if (getType() == LUA_TNIL) {
      _value.ref = LUA_REFNIL;
    }
  }
  // does this not retain whether it's a weak ref or not?
  _type &= ~(ELYSIAN_LUA_VARIANT_TYPE_BIT_MASK |
             ELYSIAN_LUA_VARIANT_INTEGER_BIT_MASK);
  _type |= ((uint32_t)luaType);
  _type |= (uint32_t)(isInteger << ELYSIAN_LUA_VARIANT_INTEGER_BIT_POS);
}

inline void Variant::setNil(void) {
  _setType(LUA_TNIL);
  _value.ref = LUA_REFNIL;
}

template <typename T> inline void Variant::setValue(T *const ud) {
    getThread()->push(ud);
  pull();
}

inline void
Variant::setValue(const std::initializer_list<VariantKVPair> &pairs) {
    getThread()->push(pairs);
  pull();
}

inline void Variant::setValue(std::nullptr_t /*null*/) { setNil(); }

inline void Variant::setValue(const lua_Integer number) {
  _setType(LUA_TNUMBER, true);
  _value.integer = (lua_Integer)number;
}

inline void Variant::setValue(const lua_Number number) {
  _setType(LUA_TNUMBER);
  _value.number = number;
}

inline void Variant::setValue(const char *const str) {
  if (!str)
    setValue(nullptr);

  const int size = std::strlen(str);

  if (getType() != LUA_TSTRING) {
    _setType(LUA_TSTRING);
    _value.string = (const char *)malloc(size * sizeof(char) + 1);
  } else {
    const int curSize =
        std::strlen(_value.string); // only realloc if we actually need more
                                    // space... fuck fragmentation
    if (curSize < size)
      _value.string =
          (const char *)realloc((void *)_value.string, size * sizeof(char) + 1);
  }

  std::strcpy((char *)_value.string, str);
}

inline void Variant::setValue(const bool val) {
  _setType(LUA_TBOOLEAN);
  _value.boolean = val;
}

inline void Variant::setValue(void *const ptr) {
  if (ptr) {
    _setType(LUA_TLIGHTUSERDATA);
    _value.luserdata = ptr;
  } else
    _setType(LUA_TNIL);
}

inline void Variant::setValue(const lua_CFunction funcPtr) {
  getThread()->push(funcPtr);
  pull();
}

template <typename T> inline T Variant::getValue(void) const {
  T ud = nullptr;
  push();
  getThread()->pull(ud);
  return ud;
}

template <> inline bool Variant::getValue<bool>(void) const {
  return _value.boolean;
}

template <> inline lua_Number Variant::getValue<lua_Number>(void) const {
  return isInteger() ? static_cast<lua_Number>(_value.integer) : _value.number;
}

template <>
inline const char *Variant::getValue<const char *>(void) const {
  return _value.string;
}

template <> inline lua_Integer Variant::getValue<lua_Integer>(void) const {
  return isInteger() ? _value.integer : static_cast<lua_Integer>(_value.number);
}

template <> inline void *Variant::getValue<void *>(void) const {
  return _value.luserdata;
}

inline bool Variant::push(void) const {
    switch (getType()) {
    case LUA_TNIL:
      getThread()->pushNil();
      break;
    case LUA_TBOOLEAN:
      getThread()->push(getValue<bool>());
      break;
    case LUA_TNUMBER:
      if (isInteger())
        getThread()->push(getValue<lua_Integer>());
      else
        getThread()->push(getValue<lua_Number>());
      break;
    case LUA_TSTRING:
      getThread()->push(getValue<const char *>());
      break;
    case LUA_TLIGHTUSERDATA:
      getThread()->push(getValue<void *>());
      break;
    case LUA_TTABLE:
    case LUA_TUSERDATA:
    case LUA_TFUNCTION:
    case LUA_TTHREAD:
      getThread()->getTableRaw(LUA_REGISTRYINDEX, getRef());
      break;
    }
    return true;
}

inline bool Variant::pull(void) {
  if (!getThread()->getTop()) {
    setNil();
    return false;
  }

  const int type = getThread()->getType(-1);

  switch (type) {
  case LUA_TNIL:
    setNil();
    break;
  case LUA_TBOOLEAN:
    setValue(getThread()->toValue<bool>(-1));
    break;
  case LUA_TNUMBER:
    if (getThread()->isInteger(-1))
        setValue(getThread()->toValue<lua_Integer>(-1));
    else
        setValue(getThread()->toValue<lua_Number>(-1));
    break;
  case LUA_TSTRING:
    setValue(getThread()->toValue<const char*>(-1));
    break;
  case LUA_TLIGHTUSERDATA:
    setValue(getThread()->toValue<void*>(-1));
    break;
  case LUA_TTABLE:
  case LUA_TUSERDATA:
  case LUA_TFUNCTION:
  case LUA_TTHREAD:
    _setRefDirect(getThread()->ref(LUA_REGISTRYINDEX));
    return true;
  }

  getThread()->pop();
  return true;
}

inline int Variant::getType(void) const {
  return (_type & ELYSIAN_LUA_VARIANT_TYPE_BIT_MASK);
}

inline const char *Variant::getTypeString(void) const {
  const int luaType = getType();
  if (luaType == LUA_TNUMBER) {
    return isInteger() ? "integer" : "number";
  } else if (luaType != LUA_TUSERDATA && luaType != LUA_TLIGHTUSERDATA) {
    return getThread()->getTypeName(luaType);
  } else {
    getThread()->getTableRaw(LUA_REGISTRYINDEX, _value.ref);
    getThread()->getTable(-1, ELYSIAN_LUA_PROXY_TYPE_METAFIELD);
    if (!getThread()->isNil(-1)) {
      const char *name = getThread()->toString(-1);
      getThread()->pop(2);
      return name;
    } else {
      getThread()->pop(2);
      return lua_typename(getThread()->getState(), luaType);
    }
  }
}

inline int Variant::getRef(void) const {
  return isRefType() ? _value.ref : LUA_NOREF;
}

inline void Variant::setRef(const int ref) {
  getThread()->getTableRaw(LUA_REGISTRYINDEX, ref);
  _setType(getThread()->getType(-1));
  _value.ref = getThread()->ref(LUA_REGISTRYINDEX);
}

template <typename L>
Variant Variant::_convertType(const L converter) const {
  push();
  converter();
  return fromStack(-1);
}

template <typename L>
Variant &Variant::_convertType(const L converter) {
  push();
  converter();
  pull();
  return *this;
}

inline bool Variant::_lightConvertToBool(const Variant &source) {
  switch (source.getType()) {
  case LUA_TBOOLEAN:
    return source._value.boolean;
  case LUA_TNIL:
    return false;
  case LUA_TSTRING:
    return (source == "true") ? true : false;
  case LUA_TNUMBER:
    if (source.isInteger())
      return (source._value.integer >= 1) ? true : false;
    else
      return (source._value.number >= 1) ? true : false;
  case LUA_TLIGHTUSERDATA:
    return (source._value.luserdata) ? true : false;
  default:
    return false;
  }
}

inline Variant &
Variant::_convertToBool(const Variant &source,
                               Variant &dest) {
  switch (source.getType()) {
  case LUA_TBOOLEAN:
  case LUA_TNIL:
  case LUA_TSTRING:
  case LUA_TNUMBER:
  case LUA_TLIGHTUSERDATA:
    dest = _lightConvertToBool(source);
    break;
  default:
    dest = source._convertType([&](void) { source.getThread()->toBoolean(-1); });
  }
  return dest;
}

template<typename NumberType>
inline NumberType
Variant::_lightConvertToNumber(const Variant &source) {
  switch (source.getType()) {
  case LUA_TNUMBER:
    if (source.isInteger())
      return source._value.integer;
    else
      return source._value.number;
  case LUA_TBOOLEAN:
    return source._value.boolean ? 1 : 0;
  case LUA_TNIL:
    return 0;
  case LUA_TLIGHTUSERDATA:
    return source._value.luserdata ? 1 : 0;
  default:
    return 0;
  }
}

template<typename NumberType>
inline Variant &
Variant::_convertToNumber(const Variant &source,
                                 Variant &dest) {
  switch (source.getType()) {
  case LUA_TNUMBER:
  case LUA_TBOOLEAN:
  case LUA_TNIL:
  case LUA_TLIGHTUSERDATA:
    dest = _lightConvertToNumber<NumberType>(source);
    break;
  case LUA_TSTRING:
  default:
    dest = source
               ._convertType([&](void) {
                 source.getThread()->push(source.getThread()->toValue<NumberType>(-1));
                 source.getThread()->remove(-2);
               })
               .template getValue<NumberType>();
  }
  return dest;
}

// same as above, doesn't change actual value
inline Variant Variant::asBool(void) const {
  Variant variant;
  return (getType() == LUA_TBOOLEAN) ? *this : _convertToBool(*this, variant);
}

inline Variant Variant::asNumber(void) const {
  Variant variant;
  return (getType() == LUA_TNUMBER) ? *this : _convertToNumber(*this, variant);
}

inline Variant Variant::asInteger(void) const {
  Variant variant;
  return (isInteger()) ? *this : _convertToNumber<lua_Integer>(*this, variant);
}

inline Variant Variant::asString(void) const {
  return (getType() == LUA_TSTRING) ? *this : _convertType([&](void) {
    getThread()->convertToString(-1, nullptr);
    getThread()->remove(-2);
  });
}

inline Variant Variant::asUserdata(void) const {
  if (getType() == LUA_TLIGHTUSERDATA) {
    return *this;
  } else {
    Variant variant;
    variant._setType(LUA_TLIGHTUSERDATA);
    variant._value.luserdata = nullptr;
    return variant;
  }
}

inline int Variant::getLength(void) const {
  int len = 0;
  if (push()) {
    len = getThread()->length(-1);
    getThread()->pop();
  }
  return len;
}

inline const void *Variant::getPointer(void) const {
  const void *ptr = nullptr;
  if (push()) {
    ptr = getThread()->toValue<const void*>(-1);
    getThread()->pop();
  }
  return ptr;
}

#if 0
inline void Variant::setMetatable(const Variant &mt) const {
  GY_ASSERT(isRefType() && mt.isRefType());
  if (push()) {
    if (mt.push()) {
      lua_setmetatable(*_mainState, -2);
    }
    lua_pop(*_mainState, 1);
  }
}

inline LuaVariant Variant::getMetatable(void) const {
  GY_ASSERT(isRefType());
  push();
  lua_getmetatable(*_mainState, -1);
  lua_remove(*_mainState, -2);
  return LuaVariant::fromStack(-1);
}
#endif

inline Variant Variant::newTable(int arraySize, int hashSize) {
  staticThread()->pushNewTable(arraySize, hashSize);
  return Variant::fromStack();
}

inline Variant Variant::fromStack(const int index,
                                const bool remove) {
  Variant variant;
  staticThread()->pushValue(index);
  if (remove)
   staticThread()->remove((index > 0) ? index : index - 1);
  variant.pull();
  return variant;
}

inline Variant Variant::fromRef(const int luaRef) {
  Variant variant;
  variant.setRef(luaRef);
  return variant;
}

inline Variant Variant::fromWeakRef(const int luaRef) {
  Variant v;
#if 0 // WRONG AS FUCK! NOT WEAK AT ALL!

        lua_rawgeti(*_mainState, LUA_REGISTRYINDEX, luaRef);
        v.pull();
#else
  v._setRefDirect(luaRef);
#endif
  v._setWeakRef(true);
  return v;
}

inline Variant
Variant::fromArray(const std::initializer_list<Variant> &array) {
  int index = 1;
  staticThread()->pushNewTable(array.size(), 0);
  for (const auto &it : array) {
    staticThread()->push(index++);
    it.push();
    staticThread()->setTableRaw(-3);
  }
  return fromStack();
}

template <typename C>
inline Variant Variant::fromSequenceContainer(const C &cont) {
  Variant array;
#if 0
  unsigned index = 1;
  for (auto it : cont) {
    array[index++] = it;
  }
#endif
  return array;
}

inline Variant Variant::makeWeakRef(const Variant &src) {
  Variant dest;
  dest._type = src._type;
  dest._value = src._value;
  dest._setWeakRef(true);
  return dest;
}

#if 0
template <typename... Args>
inline Variant Variant::operator()(Args &&... args) const {
  push();
  return LuaManager::_callFunction<Variant>(nullptr,
                                                   std::forward<Args>(args)...);
}

inline Variant
Variant::getValue(const Variant &key) const {
  if (!isIndexable()) { // gracefully return nil variant
    return Variant();
  }
  lua_rawgeti(*_mainState, LUA_REGISTRYINDEX, _value.ref);
  key.push();
  lua_gettable(*_mainState, -2);
  lua_remove(*_mainState, -2);
  return Variant::fromStack();
}

inline LuaFieldRef Variant::operator[](const Variant &key) const {
  return LuaFieldRef(*this, key);
}
#endif

// Assumes ownership of reference rather than duplicating!
inline void Variant::_setRefDirect(const int luaRef) {
  getThread()->getTableRaw(LUA_REGISTRYINDEX, luaRef);
  _setType(getThread()->getType(-1));
  getThread()->pop(1);
  _value.ref = luaRef;
}

#if 0
inline bool Variant::iteratePairs(
    std::function<void(LuaVariant, LuaVariant)> callback) {
  bool success = true;
  assert(isIndexable());

  success &= push();
  if (success) {

    LuaManagerBase::genericIterate(-1, [&](void) {
      LuaVariant value = LuaVariant::fromStack(-1, false);
      LuaVariant key = LuaVariant::fromStack(-2, false);
      success &= !key.isNil();
      callback(key, value);
    });
  }

  lua_pop(*_mainState, 1);

  return success;
}
#endif



inline Variant &
Variant::operator=(const std::initializer_list<VariantKVPair> &pairs) {
#if 0
        lua_newtable(*_mainState);
        for(auto it : pairs) {
            it.first.push();
            it.second.push();
            lua_settable(*_mainState, -3);
        }
        pull();
#else
  setValue(pairs);
  return *this;
#endif
}

#if 0
inline Variant::iterator Variant::begin(void) {
  const auto &it = cbegin();
  return iterator(it._fieldRef, it._nextFuncRef);
}

inline Variant::const_iterator Variant::begin(void) const {
  LuaVariant nextFunc;
  VariantKVPair pair;

  pushGlobalField("pairs");
  if (push()) {
    // call pairs(this)
    if (ELYSIAN_LUA_MANAGER_PROTECTED_CALL(1, 3)) {
      // store next() function
      lua_pushvalue(*_mainState, -3);
      if (nextFunc.pull()) {
        GY_ASSERT(nextFunc.getType() == LUA_TFUNCTION);
        // call next(this, k)
        if (ELYSIAN_LUA_MANAGER_PROTECTED_CALL(2, 2, nullptr)) {
          OOLUA::pull(*_mainState, pair);
        } else
          GY_ASSERT(false); // next(this, k) failed
      } else
        GY_ASSERT(false); // no next function found
    } else
      GY_ASSERT(false); // pairs(this) failed
  }
  return const_iterator(
      LuaFieldRef(LuaVariant::makeWeakRef(*this), pair.first, pair.second),
      nextFunc);
}

inline Variant::const_iterator Variant::cbegin(void) const {
  return begin();
}

inline Variant::iterator Variant::end(void) {
  return iterator(LuaFieldRef(LuaVariant::makeWeakRef(*this), LuaVariant()),
                  LuaVariant());
}

inline Variant::const_iterator Variant::end(void) const {
  return const_iterator(
      LuaFieldRef(LuaVariant::makeWeakRef(*this), LuaVariant()), LuaVariant());
}

inline Variant::const_iterator Variant::cend(void) const {
  return const_iterator(
      LuaFieldRef(LuaVariant::makeWeakRef(*this), LuaVariant()), LuaVariant());
}

template <bool is_const_iterator>
inline const_noconst_iterator<is_const_iterator>
const_noconst_iterator<is_const_iterator>::operator++(void) {
  bool success = true;

  success = _nextFuncRef.push();
  GY_ASSERT(success);
  success = _fieldRef._tableRef.push();
  GY_ASSERT(success);
  success = _fieldRef._key.push();
  GY_ASSERT(success);

  if (ELYSIAN_LUA_MANAGER_PROTECTED_CALL(2, 2, nullptr)) {

    success &= _fieldRef._cachedValue.pull();
    GY_ASSERT(success);
    success &= _fieldRef._key.pull();
    GY_ASSERT(success);
  } else
    GY_ASSERT(false);

  return *this;
}
#endif

template <typename Ret, typename... Args>
inline void
Variant::setValue(const std::function<Ret(Args...)> &closure) {
  //getThread()->push(closure);
  pull();
}

template <typename T> inline void Variant::setValue(const T lambda) {
  //setValue(make_function(lambda));
  // LuaManager::push(lambda);
  // pull();
}

} // namespace elysian

#endif // ELYSIAN_LUA_TABLE_VARIANT_HPP
