#ifndef ELYSIAN_LUA_FORWARD_DECLARATIONS_HPP
#define ELYSIAN_LUA_FORWARD_DECLARATIONS_HPP

namespace elysian::lua {


template<typename RefType>
class StatefulReference;

class StatelessStackReference;
class StatelessRegistryReference;
class StatelessGlobalsTablePsuedoReference;

class StackReference;
using RegistryReference = StatefulReference<StatelessRegistryReference>;
using GlobalsTablePsuedoReference = StatefulReference<StatelessGlobalsTablePsuedoReference>;


template<typename RefType>
class Object;


template<typename RefType, bool GlobalsTable=false>
class TableBase;

using GlobalsTable = TableBase<GlobalsTablePsuedoReference, true>;

template<typename T, typename Key>
class TableProxy;



}



#endif // ELYSIAN_LUA_FORWARD_DECLARATIONS_HPP
