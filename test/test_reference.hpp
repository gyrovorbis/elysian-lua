#ifndef ELYSIAN_TEST_REFERENCE_HPP
#define ELYSIAN_TEST_REFERENCE_HPP

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
        return [=](Variant v) {
            StackGuard stackGuard(&thread());
            RegistryRefCountGuard refGuard;

            const bool wasRefVariant = v.isRefType();
            auto ref = testFn(v);
            const int delta = refGuard.getCurrentDelta();

            QVERIFY(ref.isValid());
            QVERIFY(delta == refDelta);
            QVERIFY(thread().push(v));
            QVERIFY(thread().push(ref));
            QVERIFY(thread().compare(-1, -2, LUA_OPEQ));
            QVERIFY(ref.release(&thread()));
            thread().pop(2);
        };
    }

    template<typename To, typename From>
    To toRef(From&& src) const {
        if constexpr(StackReferenceable<To>) {
            thread().push(std::forward<From>(src));
            return thread().toValue<To>(-1);
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
            return thread().pull(ref);
        }
    }
};

template<BasicReferenceable R>
class StatelessReferenceTestSet: public StatelessReferenceTestSetBase {
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

template<BasicReferenceable R>
inline void StatelessReferenceTestSet<R>::createEmpty() {
    StackGuard stackGuard(&thread());
    RegistryRefCountGuard refGuard;
    R ref;
    QVERIFY(ref.isValid() == FixedReferenceable<R>);
    QVERIFY(static_cast<bool>(ref) == FixedReferenceable<R>);
    QVERIFY(!ref.isNilRef());
}

template<BasicReferenceable R>
inline void StatelessReferenceTestSet<R>::pull() {
    StackGuard stackGuard(&thread());
    RegistryRefCountGuard refGuard;
    R ref;
    thread().pushNewTable();
    QVERIFY(thread().pull(ref));
    QVERIFY(refGuard.getCurrentDelta() == 1);
    QVERIFY(thread().getTop() == 0);
    QVERIFY(ref.isValid());
    QVERIFY(ref);
    QVERIFY(!ref.isNilRef());

    QVERIFY(ref.release(&thread()));
    QVERIFY(!ref);
    QVERIFY(!ref.isValid());
    QVERIFY(refGuard.isCurrentlyBalanced());

    thread().pushNil();
    QVERIFY(thread().pull(ref));
    QVERIFY(refGuard.getCurrentDelta() == 1);
    QVERIFY(thread().getTop() == 0);
    QVERIFY(ref.isValid());
    QVERIFY(!ref);
    QVERIFY(ref.isNilRef());
    QVERIFY(ref.release(&thread()));
}

template<BasicReferenceable R>
inline void StatelessReferenceTestSet<R>::push() {
    StackGuard stackGuard(&thread());
    RegistryRefCountGuard refGuard;
    R ref;

    QVERIFY(thread().push(ref));
    QVERIFY(thread().isNil(-1));
    thread().pop();
    QVERIFY(refGuard.isCurrentlyBalanced());

    if constexpr(FixedReferenceable<R>) {
        thread().pushGlobalsTable();
        QVERIFY(thread().push(thread().getGlobalsTable()));
    } else {
        thread().pushNewTable();
        thread().pushValue(-1);
        QVERIFY(softPull(ref));
        QVERIFY(refGuard.getCurrentDelta() == 1);
        QVERIFY(thread().push(ref));
    }

    QVERIFY(thread().isTable(-1));
    QVERIFY(thread().compare());
    thread().pop(2);
    QVERIFY(ref.release(&thread()));
}

template<BasicReferenceable R>
inline void StatelessReferenceTestSet<R>::copyConstruct() {
    StackGuard stackGuard(&thread());
    RegistryRefCountGuard refGuard;
    R ref1;
    R ref2 = ref1;

    QVERIFY(!ref2 == !ref1);
    QVERIFY(!ref2.isValid() == !ref1.isValid());
    QVERIFY(ref1.compare(nullptr, ref2));

    if constexpr(WritableReferenceable<R>) {
        thread().pushNewTable();
        QVERIFY(softPull(ref2));
        QVERIFY(ref2);
        QVERIFY(ref2.isValid());
        QVERIFY(refGuard.getCurrentDelta() == 1);
    }

    R ref3 = ref2;
    QVERIFY(ref3);
    QVERIFY(ref2);
    QVERIFY(ref3.isValid());
    QVERIFY(thread().push(ref3));
    QVERIFY(thread().push(ref2));
    QVERIFY(thread().isTable(-1));
    QVERIFY(thread().compare());
    thread().pop(2);
    QVERIFY(ref3.release(&thread()));
}

template<BasicReferenceable R>
inline void StatelessReferenceTestSet<R>::copyAssign() {
    StackGuard stackGuard(&thread());
    RegistryRefCountGuard refGuard;
    R ref1, ref2;

    if constexpr(WritableReferenceable<R>) {
        QVERIFY(thread().push(reinterpret_cast<void*>(this)));
        QVERIFY(softPull(ref1));
    }

    ref2 = ref1;
    QVERIFY(ref2);
    QVERIFY(ref1);
    QVERIFY(thread().push(ref1));
    QVERIFY(thread().push(ref2));
    QVERIFY(thread().compare());

    if constexpr(WritableReferenceable<R>) {
        QVERIFY(thread().isLightUserdata(-1));
        QVERIFY(thread().template toValue<void*>(-1) == reinterpret_cast<void*>(this));
    }

    thread().pop(2);
    QVERIFY(ref2.release(&thread()));
}

template<BasicReferenceable R>
inline void StatelessReferenceTestSet<R>::moveConstruct() {
    StackGuard stackGuard(&thread());
    RegistryRefCountGuard refGuard;
    R ref1;
    R ref2 = std::move(ref1);

    QVERIFY(!ref2 == !ref1);
    QVERIFY(!ref2.isValid() == !ref1.isValid());

    if constexpr(WritableReferenceable<R>) {
        QVERIFY(thread().push("Fucker"));
        QVERIFY(softPull(ref2));
        QVERIFY(ref2);
        QVERIFY(ref2.isValid());
        QVERIFY(refGuard.getCurrentDelta() == 1);
    }

    QVERIFY(thread().push(ref2));

    R ref3(std::move(ref2));
    QVERIFY(ref3);
    QVERIFY(ref3.isValid());
    QVERIFY(thread().push(ref3));

    QVERIFY(static_cast<bool>(ref2) == R().isValid());

    if constexpr(WritableReferenceable<R>) {
        QVERIFY(thread().isString(-1));
        QVERIFY(QString(thread().template toValue<const char*>(-1)) == "Fucker");
        QVERIFY(refGuard.getCurrentDelta() == 1);
    } else {

    }
    QVERIFY(thread().compare());
    thread().pop(2);
    QVERIFY(ref3.release(&thread()));
}

template<BasicReferenceable R>
inline void StatelessReferenceTestSet<R>::moveAssign() {
    StackGuard stackGuard(&thread());
    RegistryRefCountGuard refGuard;
    R ref1, ref2;

    if constexpr(WritableReferenceable<R>) {
        QVERIFY(thread().push(reinterpret_cast<void*>(this)));
        QVERIFY(softPull(ref1));
    }
    QVERIFY(thread().push(ref1));

    ref2 = std::move(ref1);
    QVERIFY(ref2);
    QVERIFY(static_cast<bool>(ref1) == R().isValid());
    QVERIFY(thread().push(ref2));

    if constexpr(WritableReferenceable<R>) {
        QVERIFY(thread().isLightUserdata(-1));
        QVERIFY(thread().template toValue<void*>(-1) == reinterpret_cast<void*>(this));
    } else {
        thread().isTable(-1);
    }

    QVERIFY(thread().compare());
    thread().pop(2);
    QVERIFY(ref2.release(&thread()));
}


template<typename F, typename V, std::size_t... Is>
inline void StatelessReferenceTestSetBase::executeTestSet(F&& testCase, const V& testTuple, std::index_sequence<Is...>) const {
    auto executeTestCase = [&](auto Idx) {
        testCase(std::get<Idx>(testTuple));
    };

    (executeTestCase(std::integral_constant<std::size_t, Is>()), ...);
}

template<BasicReferenceable R>
inline void StatelessReferenceTestSet<R>::variantCopyConstruct() {
    const auto testCopyConstruction = createTestCase(1, [&](Variant v) {
        R ref = v;
        return ref;
    });

    executeVariantTestSet(testCopyConstruction);
}

template<BasicReferenceable R>
inline void StatelessReferenceTestSet<R>::variantCopyAssign() {
    auto testCopyAssignment = createTestCase(1, [&](Variant v) {
        R ref{};
        ref = v;
        return ref;
    });

    executeVariantTestSet(testCopyAssignment);

}

template<BasicReferenceable R>
inline void StatelessReferenceTestSet<R>::variantMoveConstruct() {
    auto testMoveConstruction = createTestCase(1, [&](Variant v) {
        R ref = std::move(v);
        return ref;
    });

    executeVariantTestSet(testMoveConstruction);
}

template<BasicReferenceable R>
inline void StatelessReferenceTestSet<R>::variantMoveAssign() {
    auto testMoveAssignment = createTestCase(1, [&](Variant v) {
        R ref{};
        ref = std::move(v);
        return ref;
    });

    executeVariantTestSet(testMoveAssignment);
}

}

#endif
