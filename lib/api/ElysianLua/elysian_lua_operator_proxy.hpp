#ifndef ELYSIAN_LUA_OPERATOR_PROXY_HPP
#define ELYSIAN_LUA_OPERATOR_PROXY_HPP

#include "elysian_lua_proxy.hpp"

namespace elysian::lua {

enum class OperatorType: uint8_t {
    Add = LUA_OPADD,
    Sub = LUA_OPSUB,
    Mul = LUA_OPMUL,
    Mod = LUA_OPMOD,
    Pow = LUA_OPPOW,
    Div = LUA_OPDIV,
    IDiv = LUA_OPIDIV,
    BAnd = LUA_OPBAND,
    BOr = LUA_OPBOR,
    BXOr = LUA_OPBXOR,
    Shl = LUA_OPSHL,
    Shr = LUA_OPSHR,
    Unm = LUA_OPUNM,
    BNot = LUA_OPBNOT,
    Count = BNot+1,
    Invalid = Count+1
};

// NEED AN OPTION TO RUN THIS IN A PROTECTED CONTEXT!!!

template<typename LType, typename RType>
class OperatorProxy:
        public Proxy<OperatorProxy<LType, RType>>
        //,public Callable<FunctionResult, FunctionResult>
{
public:
    OperatorProxy(LType& lhs, RType& rhs, OperatorType op);

    bool isValid(void) const;

    LType& getLeftOperand(void) const;
    RType& getRightOperand(void) const;
    OperatorType getOperator(void) const;

    const ThreadViewBase* getThread(void) const;
    int push(const ThreadViewBase* pThread) const;

    template<typename V>
    auto get(void) const;

private:

    bool stackEval(const ThreadViewBase* pThread) const;
    bool isRightOperandValid(void) const;


    LType& m_lOperand;
    RType& m_rOperand;

    OperatorType m_operator = OperatorType::Invalid;
};

template<typename LType, typename RType>
inline OperatorProxy<LType, RType>::OperatorProxy(LType& lhs, RType& rhs, OperatorType op):
    m_lOperand(lhs),
    m_rOperand(rhs),
    m_operator(op)
{
    assert(op != OperatorType::Invalid);
    assert(m_lOperand.getThread());

    if constexpr(!std::is_fundamental<RType>::value) {
        assert(m_rOperand.getThread());
        assert(m_lOperand.getThread() == m_rOperand.getThread());
    }
}

template<typename LType, typename RType>
inline bool OperatorProxy<LType, RType>::isRightOperandValid(void) const {
    if constexpr(std::is_fundamental<RType>::value) {
        return true;
    } else {
        return getRightOperand().isValid();
    }
}

template<typename LType, typename RType>
inline bool OperatorProxy<LType, RType>::isValid(void) const {
    return getLeftOperand().isValid() && isRightOperandValid() && getOperator() != OperatorType::Invalid;
}

template<typename LType, typename RType>
inline const ThreadViewBase* OperatorProxy<LType, RType>::getThread(void) const { return getLeftOperand().getThread(); }

template<typename LType, typename RType>
inline LType& OperatorProxy<LType, RType>::getLeftOperand(void) const { return m_lOperand; }

template<typename LType, typename RType>
inline RType& OperatorProxy<LType, RType>::getRightOperand(void) const { return m_rOperand; }

template<typename LType, typename RType>
inline OperatorType OperatorProxy<LType, RType>::getOperator(void) const { return m_operator; }

template<typename LType, typename RType>
inline bool OperatorProxy<LType, RType>::stackEval(const ThreadViewBase* pThread) const {
    bool success = false;
    if(pThread->push(getLeftOperand())) {
        if(pThread->push(getRightOperand())) {
            pThread->arith(static_cast<int>(getOperator()));
            //Check whether we even pushed anything or not!
            success = true;
        } else {
            pThread->pop();
        }
    }
    return success;
}

template<typename LType, typename RType>
inline int OperatorProxy<LType, RType>::push(const ThreadViewBase* pThread) const {
    if(!stackEval(pThread)) {
        pThread->pushNil();
    }
    return 1;
}

template<typename LType, typename RType>
template<typename V>
inline auto OperatorProxy<LType, RType>::get(void) const {
    V value {};

    //Push void if this is reested?
    if(stackEval(getThread())) {
        getThread()->pull(value);
    }

    return value;
}

namespace stack_impl {

template<typename LType, typename RType>
struct stack_pusher<OperatorProxy<LType, RType>>:
        public proxy_stack_pusher<OperatorProxy<LType, RType>>{};

}

}

#endif // ELYSIAN_LUA_OPERATOR_PROXY_HPP
