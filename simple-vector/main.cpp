#include "simple_vector.h"
#include "tests.h"

int main() {
    Test1();
    Test2();
    TestReserveConstructor();
    TestReserveMethod();
    TestUncopyables();
    TestTemporaryObjConstructor();
    TestTemporaryObjOperator();
    TestNamedMoveConstructor();
    TestNamedMoveOperator();
    TestNoncopiableMoveConstructor();
    TestNoncopiablePushBack();
    TestNoncopiableInsert();
    TestNoncopiableErase();
    return 0;
}