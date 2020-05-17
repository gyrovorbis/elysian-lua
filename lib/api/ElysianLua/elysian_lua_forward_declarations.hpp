#ifndef ELYSIAN_LUA_FORWARD_DECLARATIONS_HPP
#define ELYSIAN_LUA_FORWARD_DECLARATIONS_HPP

#include <tuple>

namespace elysian::lua {

template<typename RefType, typename StateType>
class StatefulRefBase;

template<typename RefType>
class StatefulReference;

class StatelessStackReference;
class StatelessRegistryReference;
class StatelessGlobalsTablePsuedoReference;

class ExplicitRefState;
class StaticRefState;

using StackReference = StatefulReference<StatelessStackReference>;
using RegistryReference = StatefulReference<StatelessRegistryReference>;
using GlobalsTablePsuedoReference = StatefulReference<StatelessGlobalsTablePsuedoReference>;

using RegistryRef = StatefulRefBase<StatelessRegistryReference, ExplicitRefState>;
using StaticRegistryRef = StatefulRefBase<StatelessRegistryReference, StaticRefState>;
using StackRef = StatefulRefBase<StatelessStackReference, ExplicitRefState>;
using StaticStackRef = StatefulRefBase<StatelessStackReference, StaticRefState>;

template<typename RefType>
class Object;


template<typename RefType, bool GlobalsTable=false>
class TableBase;

//using Table = TableBase<RegistryReference, false>;
using Table = TableBase<RegistryRef, false>;
using StaticTable = TableBase<StaticRegistryRef, false>;
using StackTable = TableBase<StackRef, false>;
using StaticStackTable = TableBase<StaticStackRef, false>;

using GlobalsTable = TableBase<GlobalsTablePsuedoReference, true>;

template<typename Ref>
class FunctionBase;

using Function = FunctionBase<RegistryReference>;
using StackFunction = FunctionBase<StackReference>;


template<typename T, typename Key>
class TableProxy;


template<typename Key>
using GlobalsTableProxy = TableProxy<GlobalsTable, std::tuple<Key>>;


class FunctionResult;

template<typename LType, typename RType>
class OperatorProxy;


namespace stack_impl {

template<typename T, typename=void>
struct stack_checker;

template<typename T, typename=void>
struct stack_getter;

template<typename T, typename=void>
struct stack_pusher;

template<typename T, typename=void>
constexpr const static bool stack_table_type = false;

template<typename T, typename=void>
constexpr const static int stack_count = 1;

template<typename T, typename SFINAE=void>
constexpr const static int stack_pull_pop_count = stack_count<T, SFINAE>;

}

struct StackRecord;

}



#endif // ELYSIAN_LUA_FORWARD_DECLARATIONS_HPP
