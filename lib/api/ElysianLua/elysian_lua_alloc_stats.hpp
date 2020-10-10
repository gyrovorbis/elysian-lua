#ifndef ELYSIAN_LUA_ALLOC_STATS_HPP
#define ELYSIAN_LUA_ALLOC_STATS_HPP

#include <cstdint>

extern "C" {
    #include <lua/lua.h>
}

namespace elysian::lua {

class AllocStats {
public:
  enum class CounterType : uint16_t {
    String = LUA_TSTRING,
    Table = LUA_TTABLE,
    Function = LUA_TFUNCTION,
    Userdata = LUA_TUSERDATA,
    Thread = LUA_TTHREAD,
    Other = LUA_TTHREAD + 1,
    Count = Other + 1
  };

  struct Counters {
    uint64_t activeAllocs = 0;
    uint64_t maxAllocs = 0;
    size_t totalBytes = 0;

    void allocEvent(size_t bytes);
    void freeEvent(size_t bytes);
    void reallocEvent(size_t oldBytes, size_t newBytes);
  };

  const Counters &getTotalCounters(void) const;
  const Counters &getTypeCounters(CounterType type) const;

  void allocEvent(int luaType, size_t size);
  void reallocEvent(int luaType, size_t oldSize, size_t newSize);
  void freeEvent(int luaType, size_t size);

  static constexpr CounterType luaTypeToCounterType(int type);

private:
  Counters &_getTypeCounters(CounterType type);
  Counters &_getTotalCounters(void);

  // AllocStats *_parent = nullptr;
  Counters _totalCounters;
  Counters _typeCounters[static_cast<int>(CounterType::Count)];
};

inline constexpr auto AllocStats::luaTypeToCounterType(int type) -> CounterType {
  switch (type) {
  case LUA_TSTRING:
    return CounterType::String;
  case LUA_TTABLE:
    return CounterType::Table;
  case LUA_TFUNCTION:
    return CounterType::Function;
  case LUA_TUSERDATA:
    return CounterType::Userdata;
  case LUA_TTHREAD:
    return CounterType::Thread;
  default:
    return CounterType::Other;
  }
}

inline auto AllocStats::getTotalCounters(void) const -> const Counters & {
  return _totalCounters;
}
inline auto AllocStats::getTypeCounters(CounterType type) const
    -> const Counters & {
  return _typeCounters[static_cast<int>(type)];
}

inline auto AllocStats::_getTypeCounters(CounterType type) -> Counters & {
  return _typeCounters[static_cast<int>(type)];
}
inline auto AllocStats::_getTotalCounters(void) -> Counters & {
  return _totalCounters;
}

inline void AllocStats::Counters::allocEvent(size_t bytes) {
  if (++activeAllocs >= maxAllocs)
    maxAllocs = activeAllocs;
  totalBytes += bytes;
}
inline void AllocStats::Counters::freeEvent(size_t bytes) {
  --activeAllocs;
  totalBytes -= bytes;
}
inline void AllocStats::Counters::reallocEvent(size_t oldBytes,
                                                  size_t newBytes) {
  totalBytes -= oldBytes;
  totalBytes += newBytes;
}

inline void AllocStats::allocEvent(int luaType, size_t size) {
  Counters &counters = _getTypeCounters(luaTypeToCounterType(luaType));
  counters.allocEvent(size);
  _getTotalCounters().allocEvent(size);
}

inline void AllocStats::reallocEvent(int luaType, size_t oldSize,
                                        size_t newSize) {
  Counters &counters = _getTypeCounters(luaTypeToCounterType(luaType));
  counters.reallocEvent(oldSize, newSize);
  _getTotalCounters().reallocEvent(oldSize, newSize);
}

inline void AllocStats::freeEvent(int luaType, size_t size) {
  Counters &counters = _getTypeCounters(luaTypeToCounterType(luaType));
  counters.freeEvent(size);
  _getTotalCounters().freeEvent(size);
}

}

#endif // ELYSIAN_LUA_ALLOC_STATS_HPP
