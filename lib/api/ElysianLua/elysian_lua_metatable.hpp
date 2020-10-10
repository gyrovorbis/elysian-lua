#ifndef ELYSIAN_LUA_METATABLE_HPP
#define ELYSIAN_LUA_METATABLE_HPP

#include "elysian_lua_object.hpp"
#include "elysian_lua_function.hpp"
#include <cassert>

namespace elysian::lua {

enum class Metamethod: uint8_t {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Pow,
    Unm,
    IDiv,
    BAnd,
    BOr,
    BXOr,
    BNot,
    Shl,
    Shr,
    Concat,
    Len,
    Eq,
    Lt,
    Le,
    Index,
    NewIndex,
    Call,
    Gc,
    Close,
    Mode,
    Name,
    Count,
    ToString
};


struct MetatableCommon {
   static consteval const char* getMetamethodName(Metamethod method);
  /*
   * Some have to be functions, some can be tables, some can be callable objects
   bool validateMetamethod()
   bool validateArgs() - make sure arguments are valid

   */
};


template<typename RefType, bool GlobalsTable=false>
class Metatable:
        public TableBase<RefType, GlobalsTable>,
        public MetatableCommon
{
public:
    //all accesses should be using RAW FUNCTIONS


     //this can be fucking anything, not just methods, can be a table!

    auto getMetamethod(Metamethod method) const -> TableProxy<Metatable<RefType, GlobalsTable>, const char*>;

     void setMetamethod(Metamethod method, auto&& func) const;
     bool callMetamethod(Metamethod method, auto&&... args) const;

};


template<typename RefType, bool GlobalsTable>
inline auto Metatable<RefType, GlobalsTable>::getMetamethod(Metamethod method) const ->
    TableProxy<Metatable<RefType, GlobalsTable>, const char*>
{

}



inline consteval const char* MetatableCommon::getMetamethodName(Metamethod method) {

    switch(method) {
    case Metamethod::Add: return "__add";
    case Metamethod::Sub: return "__sub";
    case Metamethod::Mul: return "__mul";
    case Metamethod::Div: return "__div";
    case Metamethod::Mod: return "__mod";
    case Metamethod::Pow: return "__pow";
    case Metamethod::Unm: return "__unm";
    case Metamethod::IDiv: return "__idiv";
    case Metamethod::BAnd: return "__band";
    case Metamethod::BOr: return "__bor";
    case Metamethod::BXOr: return "__bxor";
    case Metamethod::BNot: return "__bnot";
    case Metamethod::Shl: return "__shl";
    case Metamethod::Shr: return "__shr";
    case Metamethod::Concat: return "__concat";
    case Metamethod::Len: return "__len";
    case Metamethod::Eq: return "__eq";
    case Metamethod::Lt: return "__lt";
    case Metamethod::Le: return "__le";
    case Metamethod::Index: return "__index";
    case Metamethod::NewIndex: return "__newindex";
    case Metamethod::Call: return "__call";
    case Metamethod::Gc: return "__gc";
    case Metamethod::Close: return "__close";
    case Metamethod::Mode: return "__mode";
    case Metamethod::Name: return "__name";
    case Metamethod::ToString: return "__tostring";
    default:
        assert(false);
        return nullptr;
    }
}


#endif // ELYSIAN_LUA_METATABLE_HPP
