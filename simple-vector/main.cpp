#include <cassert>

#include "Tests.h"

#include <optional>

int main() {
    try {
        TestInitialization();
        TestAssignment();
        TestMoveAssignment();
        TestValueAccess();
        TestReset();
    }
    catch (...) {
        assert(false);
    }

    // vector tests
    Test1();
    Test2();
    Test4();
    Test5();
    Test6();
    Test7();
    Test8();
    Test9();
    Test10();
    Test11();
    Test12();
    TestPushBackAdditional_move_without_noexcept_copy();

    try {
        TestInitialization1();
        TestAssignment1();
        TestMoveAssignment1();
        TestValueAccess1();
        TestReset1();
        TestEmplace1();
    }
    catch (...) {
        assert(false);
    }
    try {
        Test13();
        Test14();
        Test15();
        Test16();
        Test17();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    try {
        Test18();
        Test19();
        Test20();
        Test21();
        Test22();
        Test23();
        Benchmark();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    try {
        TestInitialization2();
        TestAssignment2();
        TestMoveAssignment2();
        TestValueAccess2();
        TestReset2();
        TestEmplace2();
        TestRefQualifiedMethodOverloading2();
    }
    catch (...) {
        assert(false);
    }
}