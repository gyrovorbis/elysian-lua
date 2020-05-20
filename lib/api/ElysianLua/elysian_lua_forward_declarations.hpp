#ifndef ELYSIAN_LUA_FORWARD_DECLARATIONS_HPP
#define ELYSIAN_LUA_FORWARD_DECLARATIONS_HPP

#include <tuple>


#define ELYSIAN_LUA_GET_VARIADIC_MACRO_2(_1, _2, NAME, ...) NAME

#define ELYSIAN_LUA_GET_VARIADIC_MACRO_1(_1, NAME, ...) NAME

#define ELYSIAN_LUA_CPP_CONTEXT_0() \
    (CppExecutionContext(__FILE__, __FUNCTION__, __LINE__))

#define ELYSIAN_LUA_CPP_CONTEXT_1(BLOCK) \
    (CppExecutionContext(__FILE__, __FUNCTION__, __LINE__, BLOCK))

#define ELYSIAN_LUA_CPP_CONTEXT(...) \
    ELYSIAN_LUA_GET_VARIADIC_MACRO_1(__VA_ARGS__, \
                                      ELYSIAN_LUA_CPP_CONTEXT_1, \
                                      ELYSIAN_LUA_CPP_CONTEXT_0)(__VA_ARGS__)


#define ELYSIAN_LUA_PROTECTED_BLOCK(S) \
    ProtectedBlock block(S, ELYSIAN_LUA_CPP_CONTEXT("Protected Block")); block = [&]()

#define ELYSIAN_LUA_CONTEXTUAL_CALL(F, ...) \
    F.contextualCall(ELYSIAN_LUA_CPP_CONTEXT("Contextual Call"), __VA_ARGS__)

#define ELYSIAN_LUA_STACK_GUARD2(T, O) \
    StackGuard guard(T, O, ELYSIAN_LUA_CPP_CONTEXT("Stack Guard")); (void)guard

#define ELYSIAN_LUA_STACK_GUARD1(T) \
    ELYSIAN_LUA_STACK_GUARD2(T, 0)

#define ELYSIAN_LUA_STACK_GUARD(...) \
    ELYSIAN_LUA_GET_VARIADIC_MACRO_2(__VA_ARGS__, \
                                      ELYSIAN_LUA_STACK_GUARD2, \
                                      ELYSIAN_LUA_STACK_GUARD1)(__VA_ARGS__)

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

using GlobalsTablePsuedoRef = StatefulRefBase<StatelessGlobalsTablePsuedoReference, ExplicitRefState>;
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

using GlobalsTable = TableBase<GlobalsTablePsuedoRef, true>;

template<typename Ref>
class FunctionBase;

using Function = FunctionBase<RegistryRef>;
using StackFunction = FunctionBase<StackRef>;


template<typename T, typename Key>
class TableProxy;


template<typename Key>
using GlobalsTableProxy = TableProxy<GlobalsTable, std::tuple<Key>>;

template<typename LType, typename RType>
class OperatorProxy;

class FunctionResult;
class ProtectedFunctionResult;

class StaticMessageHandlerState;

template<typename RefType=Function>
class ExplicitMessageHandlerState;

class FunctionCaller;

template<typename MsgHandlerState>
class ProtectedFunctionCaller;

template<typename CRTP,
         typename Caller =
         ProtectedFunctionCaller<StaticMessageHandlerState>>

class Callable;

struct StackRecord;

class CppExecutionContext;
class StackMonitor;
template<bool RAII=true>
class StackGuard;
class ProtectedBlock;


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

}




#endif // ELYSIAN_LUA_FORWARD_DECLARATIONS_HPP
