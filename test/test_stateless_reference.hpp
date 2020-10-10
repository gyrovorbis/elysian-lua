#ifndef ELYSIAN_TEST_STATELESS_REFERENCE_HPP
#define ELYSIAN_TEST_STATELESS_REFERENCE_HPP

#include "test_base.hpp"
#include <ElysianLua/elysian_lua_vm.hpp>
#include <ElysianLua/elysian_lua_stack_monitor.hpp>
#include <ElysianLua/elysian_lua_variant.hpp>

namespace elysian::lua::test {

class StatelessReferenceTestSetBase: public TestSetBase {
protected:
    auto getVariantTestData(void) {
        auto initialValues = std::tuple(Variant::newTable(),
                                       reinterpret_cast<void*>(this),
                                       static_cast<lua_CFunction>([](lua_State*) -> int { return 0; }),
                                       nullptr,
                                       true,
                                       12,
                                       static_cast<lua_Number>(-33.33f),
                                       "trolo");


        const auto tupleSize = std::tuple_size_v<decltype(initialValues)>;
        const auto indexSequence = std::make_index_sequence<tupleSize>();

        return std::pair(initialValues, indexSequence);
    }

    template<typename F, typename V, std::size_t... Is>
    void executeTestSet(F&& testCase, const V& testTuple, std::index_sequence<Is...>) const;

    template<typename F>
    void executeVariantTestSet(F&& testCase) {
        const auto testData = getVariantTestData();
        executeTestSet(std::forward<F>(testCase), testData.first, testData.second);
    }

    template<typename F>
    auto createTestCase(int refDelta, F&& testFn) {
        return [=, this](Variant v) {
            StackGuard stackGuard(&this->thread());
            RegistryRefCountGuard refGuard;

            const bool wasRefVariant = v.isRefType();
            auto ref = testFn(v);
            const int delta = refGuard.getCurrentDelta();

            QVERIFY(ref.isValid());
            QVERIFY(delta == refDelta);
            QVERIFY(this->thread().push(v));
            QVERIFY(this->thread().push(ref));
            QVERIFY(this->thread().compare(-1, -2, LUA_OPEQ));
            QVERIFY(ref.release(&this->thread()));
            this->thread().pop(2);
        };
    }

    template<typename To, typename From>
    To toRef(From&& src) const {
        if constexpr(StackReferenceable<To>) {
            this->thread().push(std::forward<From>(src));
            return this->thread().toValue<To>(-1);
        } else {
            return std::forward<From>(src);
        }
    }

    template<typename R>
    bool softPull(R& ref) const {
        if constexpr(StackReferenceable<R>) {
            ref.setStackIndex(-1);
            return true;
        } else {
            return this->thread().pull(ref);
        }
    }

};

template<BasicReferenceable R, typename Base=StatelessReferenceTestSetBase>
class StatelessReferenceTestSet: public Base {
protected:
    void createEmpty();
    void pull();
    void push();
    void copyConstruct();
    void copyAssign();
    void moveConstruct();
    void moveAssign();
    void variantCopyConstruct();
    void variantCopyAssign();
    void variantMoveConstruct();
    void variantMoveAssign();

    template<typename From>
    R toRef(From&& src) const {
        return StatelessReferenceTestSet::toRef<R, From>(std::forward<From>(src));
    }

    bool softPop(int depth=1) const {
        if constexpr(StackReferenceable<R>) {
            this->thread().pop(depth);
        }
        return true;
    }

    template<Monitorable M>
    void softDeltaCheck(const M& monitor, int expected) {
        if constexpr(RegistryReferenceable<R>) {
            const auto delta = monitor.getCurrentDelta();
            QTRY_COMPARE(monitor.getCurrentDelta(), expected);
        }
    }
};

class StatelessRegistryReferenceTestSet:
        public StatelessReferenceTestSet<StatelessRegRef>
{
    using ParentTestSet = StatelessReferenceTestSet<StatelessRegRef>;

    Q_OBJECT
private slots:

    ELYSIAN_INHERIT_TEST_CASE(createEmpty)
    ELYSIAN_INHERIT_TEST_CASE(pull)
    ELYSIAN_INHERIT_TEST_CASE(push)
    ELYSIAN_INHERIT_TEST_CASE(copyConstruct)
    ELYSIAN_INHERIT_TEST_CASE(copyAssign)
    ELYSIAN_INHERIT_TEST_CASE(moveConstruct)
    ELYSIAN_INHERIT_TEST_CASE(moveAssign)
    ELYSIAN_INHERIT_TEST_CASE(variantCopyConstruct)
    ELYSIAN_INHERIT_TEST_CASE(variantCopyAssign)
    ELYSIAN_INHERIT_TEST_CASE(variantMoveConstruct)
    ELYSIAN_INHERIT_TEST_CASE(variantMoveAssign)
};


class StatelessStackReferenceTestSet:
        public StatelessReferenceTestSet<StatelessStackRef>
{
    using ParentTestSet = StatelessReferenceTestSet<StatelessStackRef>;
    Q_OBJECT
private slots:

    ELYSIAN_INHERIT_TEST_CASE(createEmpty)
    ELYSIAN_INHERIT_TEST_CASE(push)
    ELYSIAN_INHERIT_TEST_CASE(copyConstruct)
    ELYSIAN_INHERIT_TEST_CASE(copyAssign)
    ELYSIAN_INHERIT_TEST_CASE(moveConstruct)
    ELYSIAN_INHERIT_TEST_CASE(moveAssign)
};

class StatelessGlobalsTableFixedReferenceTestSet:
        public StatelessReferenceTestSet<StatelessGlobalsTableFixedRef>
{
    using ParentTestSet = StatelessReferenceTestSet<StatelessGlobalsTableFixedRef>;
    Q_OBJECT
private slots:
    ELYSIAN_INHERIT_TEST_CASE(createEmpty)
    ELYSIAN_INHERIT_TEST_CASE(push)
    ELYSIAN_INHERIT_TEST_CASE(copyConstruct)
    ELYSIAN_INHERIT_TEST_CASE(copyAssign)
    ELYSIAN_INHERIT_TEST_CASE(moveConstruct)
    ELYSIAN_INHERIT_TEST_CASE(moveAssign)
};

template<BasicReferenceable R, typename B>
inline void StatelessReferenceTestSet<R, B>::createEmpty() {
    StackGuard stackGuard(&this->thread());
    RegistryRefCountGuard refGuard;
    R ref;
    QVERIFY(ref.isValid() == FixedReferenceable<R>);
    QVERIFY(static_cast<bool>(ref) == FixedReferenceable<R>);
    QVERIFY(!ref.isNilRef());
}

template<BasicReferenceable R, typename B>
inline void StatelessReferenceTestSet<R, B>::pull() {
    StackGuard stackGuard(&this->thread());
    RegistryRefCountGuard refGuard;
    R ref;
    this->thread().pushNewTable();
    QVERIFY(this->thread().pull(ref));
    QVERIFY(refGuard.getCurrentDelta() == 1);
    QVERIFY(this->thread().getTop() == 0);
    QVERIFY(ref.isValid());
    QVERIFY(ref);
    QVERIFY(!ref.isNilRef());
    if constexpr(ModifiableThreadStateful<R>) {
        QVERIFY(ref.getThread() == &this->thread());
    }

    QVERIFY(ref.release(&this->thread()));
    QVERIFY(!ref);
    QVERIFY(!ref.isValid());
    QVERIFY(refGuard.isCurrentlyBalanced());

    this->thread().pushNil();
    QVERIFY(this->thread().pull(ref));
    QVERIFY(refGuard.getCurrentDelta() == 1);
    QVERIFY(this->thread().getTop() == 0);
    QVERIFY(ref.isValid());
    QVERIFY(!ref);
    QVERIFY(ref.isNilRef());
    QVERIFY(ref.release(&this->thread()));
}

template<BasicReferenceable R, typename B>
inline void StatelessReferenceTestSet<R, B>::push() {
    StackGuard stackGuard(&this->thread());
    RegistryRefCountGuard refGuard;
    R ref;

    QVERIFY(this->thread().push(ref));
    QVERIFY(FixedReferenceable<R> || this->thread().isNil(-1));
    this->thread().pop();
    QVERIFY(refGuard.isCurrentlyBalanced());

    if constexpr(FixedReferenceable<R>) {
        this->thread().pushGlobalsTable();
        QVERIFY(this->thread().push(this->thread().getGlobalsTable()));
    } else {
        this->thread().pushNewTable();
        this->thread().pushValue(-1);
        QVERIFY(B::softPull(ref));
        softDeltaCheck(refGuard, 1);
        QVERIFY(this->thread().push(ref));
    }

    QVERIFY(this->thread().isTable(-1));
    QVERIFY(this->thread().compare());
    this->thread().pop();
    softPop();
    this->thread().pop();
    QVERIFY(ref.release(&this->thread()));
}

template<BasicReferenceable R, typename B>
inline void StatelessReferenceTestSet<R, B>::copyConstruct() {
    StackGuard stackGuard(&this->thread());
    RegistryRefCountGuard refGuard;
    R ref1;
    R ref2 = ref1;

    QVERIFY(!ref2 == !ref1);
    QVERIFY(!ref2.isValid() == !ref1.isValid());
    QVERIFY(ref1.compare(nullptr, ref2));

    if constexpr(WritableReferenceable<R>) {
        this->thread().pushNewTable();
        QVERIFY(B::softPull(ref2));
        QVERIFY(ref2);
        QVERIFY(ref2.isValid());
        softDeltaCheck(refGuard, 1);
    }

    R ref3 = ref2;
    QVERIFY(ref3);
    QVERIFY(ref2);
    QVERIFY(ref3.isValid());
    QVERIFY(this->thread().push(ref3));
    QVERIFY(this->thread().push(ref2));
    QVERIFY(this->thread().isTable(-1));
    QVERIFY(this->thread().compare());
    this->thread().pop(2);
    softPop();
    QVERIFY(ref3.release(&this->thread()));
}

template<BasicReferenceable R, typename B>
inline void StatelessReferenceTestSet<R, B>::copyAssign() {
    StackGuard stackGuard(&this->thread());
    RegistryRefCountGuard refGuard;
    R ref1, ref2;

    if constexpr(WritableReferenceable<R>) {
        QVERIFY(this->thread().push(reinterpret_cast<void*>(this)));
        QVERIFY(B::softPull(ref1));
        softDeltaCheck(refGuard, 1);
    }

    ref2 = ref1;
    QVERIFY(ref2);
    QVERIFY(ref1);
    QVERIFY(this->thread().push(ref1));
    QVERIFY(this->thread().push(ref2));
    QVERIFY(this->thread().compare());

    if constexpr(WritableReferenceable<R>) {
        QVERIFY(this->thread().isLightUserdata(-1));
        QVERIFY(this->thread().template toValue<void*>(-1) == reinterpret_cast<void*>(this));
    }

    QVERIFY(ref1.release(&this->thread()));
    this->thread().pop(2);
    softPop();
}

template<BasicReferenceable R, typename B>
inline void StatelessReferenceTestSet<R, B>::moveConstruct() {
    StackGuard stackGuard(&this->thread());
    RegistryRefCountGuard refGuard;
    R ref1;
    R ref2 = std::move(ref1);

    QVERIFY(!ref2 == !ref1);
    QVERIFY(!ref2.isValid() == !ref1.isValid());

    if constexpr(WritableReferenceable<R>) {
        QVERIFY(this->thread().push("Fucker"));
        QVERIFY(B::softPull(ref2));
        QVERIFY(ref2);
        QVERIFY(ref2.isValid());
        softDeltaCheck(refGuard, 1);
    }

    QVERIFY(this->thread().push(ref2));

    R ref3(std::move(ref2));
    QVERIFY(ref3);
    QVERIFY(ref3.isValid());
    QVERIFY(this->thread().push(ref3));

    if constexpr(RegistryReferenceable<R>) QVERIFY(static_cast<bool>(ref2) == R().isValid());

    if constexpr(WritableReferenceable<R>) {
        QVERIFY(this->thread().isString(-1));
        QVERIFY(QString(this->thread().template toValue<const char*>(-1)) == "Fucker");
        softDeltaCheck(refGuard, 1);
    }

    QVERIFY(this->thread().compare());
    this->thread().pop(2);
    softPop();
    QVERIFY(ref3.release(&this->thread()));
}

template<BasicReferenceable R, typename B>
inline void StatelessReferenceTestSet<R, B>::moveAssign() {
    StackGuard stackGuard(&this->thread());
    RegistryRefCountGuard refGuard;
    R ref1, ref2;

    if constexpr(WritableReferenceable<R>) {
        QVERIFY(this->thread().push(reinterpret_cast<void*>(this)));
        QVERIFY(B::softPull(ref1));
    }
    QVERIFY(this->thread().push(ref1));

    ref2 = std::move(ref1);
    QVERIFY(ref2);
    if constexpr(RegistryReferenceable<R>) QVERIFY(static_cast<bool>(ref1) == R().isValid());
    QVERIFY(this->thread().push(ref2));

    if constexpr(WritableReferenceable<R>) {
        QVERIFY(this->thread().isLightUserdata(-1));
        QVERIFY(this->thread().template toValue<void*>(-1) == reinterpret_cast<void*>(this));
    } else {
        this->thread().isTable(-1);
    }

    QVERIFY(this->thread().compare());
    this->thread().pop(2);
    softPop();
    QVERIFY(ref2.release(&this->thread()));
}


template<typename F, typename V, std::size_t... Is>
inline void StatelessReferenceTestSetBase::executeTestSet(F&& testCase, const V& testTuple, std::index_sequence<Is...>) const {
    auto executeTestCase = [&](auto Idx) {
        testCase(std::get<Idx>(testTuple));
    };

    (executeTestCase(std::integral_constant<std::size_t, Is>()), ...);
}

template<BasicReferenceable R, typename B>
inline void StatelessReferenceTestSet<R, B>::variantCopyConstruct() {
    const auto testCopyConstruction = B::createTestCase(1, [&](Variant v) {
        R ref = v;
        return ref;
    });

    B::executeVariantTestSet(testCopyConstruction);
}

template<BasicReferenceable R, typename B>
inline void StatelessReferenceTestSet<R, B>::variantCopyAssign() {
    auto testCopyAssignment = B::createTestCase(1, [&](Variant v) {
        R ref{};
        ref = v;
        return ref;
    });

    B::executeVariantTestSet(testCopyAssignment);

}

template<BasicReferenceable R, typename B>
inline void StatelessReferenceTestSet<R, B>::variantMoveConstruct() {
    auto testMoveConstruction = B::createTestCase(1, [&](Variant v) {
        R ref = std::move(v);
        return ref;
    });

    B::executeVariantTestSet(testMoveConstruction);
}

template<BasicReferenceable R, typename B>
inline void StatelessReferenceTestSet<R, B>::variantMoveAssign() {
    auto testMoveAssignment = B::createTestCase(1, [&](Variant v) {
        R ref{};
        ref = std::move(v);
        return ref;
    });

    B::executeVariantTestSet(testMoveAssignment);
}

}

#endif
