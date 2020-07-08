#include "elysian_qtest.hpp"
#include "test_stack.hpp"
#include "test_globals_table.hpp"
#include "test_stateless_reference.hpp"
#include "test_reference.hpp"
#include "test_table.hpp"
#include "test_table_proxy.hpp"
#include "test_function.hpp"
#include "test_buffer.hpp"
#include "test_operators.hpp"
#include "test_thread_view.hpp"
#include "test_variant.hpp"

using namespace elysian;
using namespace elysian::lua::test;

int main(int argc, char* argv[]) {

    UnitTestSuite testSuite;
    testSuite.enqueueTestSet(new StackTestSet);
    testSuite.enqueueTestSet(new StatelessRegistryReferenceTestSet);
    testSuite.enqueueTestSet(new StatelessStackReferenceTestSet);
    testSuite.enqueueTestSet(new StatelessGlobalsTableFixedReferenceTestSet);
    testSuite.enqueueTestSet(new RegistryReferenceTestSet);
    testSuite.enqueueTestSet(new GlobalsTableTestSet);
    testSuite.enqueueTestSet(new TableTestSet);
    testSuite.enqueueTestSet(new StackTableTestSet);
    testSuite.enqueueTestSet(new StaticTableTestSet);
    testSuite.enqueueTestSet(new StaticStackTableTestSet);
    testSuite.enqueueTestSet(new TableProxyTestSet);
    testSuite.enqueueTestSet(new OperatorProxyTestSet);
    testSuite.enqueueTestSet(new FunctionTestSet);
    testSuite.enqueueTestSet(new BufferTestSet);
    testSuite.enqueueTestSet(new ThreadViewTestSet);
    testSuite.enqueueTestSet(new VariantTestSet);

    return !testSuite.exec(argc, argv);
}
