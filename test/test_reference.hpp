#ifndef ELYSIAN_TEST_REFERENCE_HPP
#define ELYSIAN_TEST_REFERENCE_HPP

#include "test_stateless_reference.hpp"

namespace elysian::lua::test {

class ReferenceTestSetBase: public StatelessReferenceTestSetBase {


};

template<BasicReferenceable R>
    requires ThreadStateful<R>
class ReferenceTestSet:
        public StatelessReferenceTestSet<R, ReferenceTestSetBase>
{

};

class RegistryReferenceTestSet:
        public ReferenceTestSet<RegistryRef>
{
    using ParentTestSet = ReferenceTestSet<RegistryRef>;

    Q_OBJECT
private slots:

    ELYSIAN_INHERIT_TEST_CASE(createEmpty)
#if 0
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
#endif
};

}

#endif // TEST_REFERENCE_HPP
