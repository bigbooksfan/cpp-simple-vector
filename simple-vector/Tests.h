#include <stdexcept>
#include <vector>
#include <iostream>
#include <string>

#include "optional.h"
#include "vector.h"

struct C {
    C() noexcept {
        ++def_ctor;
    }
    C(const C& /*other*/) noexcept {
        ++copy_ctor;
    }
    C(C&& /*other*/) noexcept {
        ++move_ctor;
    }
    C& operator=(const C& other) noexcept {
        if (this != &other) {
            ++copy_assign;
        }
        return *this;
    }
    C& operator=(C&& /*other*/) noexcept {
        ++move_assign;
        return *this;
    }
    ~C() {
        ++dtor;
    }

    static size_t InstanceCount() {
        return def_ctor + copy_ctor + move_ctor - dtor;
    }

    static void Reset() {
        def_ctor = 0;
        copy_ctor = 0;
        move_ctor = 0;
        copy_assign = 0;
        move_assign = 0;
        dtor = 0;
    }

    inline static size_t def_ctor = 0;
    inline static size_t copy_ctor = 0;
    inline static size_t move_ctor = 0;
    inline static size_t copy_assign = 0;
    inline static size_t move_assign = 0;
    inline static size_t dtor = 0;
};

void TestInitialization() {
    C::Reset();
    {
        Optional<C> o;
        assert(!o.HasValue());
        assert(C::InstanceCount() == 0);
    }
    assert(C::InstanceCount() == 0);

    C::Reset();
    {
        C c;
        Optional<C> o(c);
        assert(o.HasValue());
        assert(C::def_ctor == 1 && C::copy_ctor == 1);
        assert(C::InstanceCount() == 2);
    }
    assert(C::InstanceCount() == 0);

    C::Reset();
    {
        C c;
        Optional<C> o(std::move(c));
        assert(o.HasValue());
        assert(C::def_ctor == 1 && C::move_ctor == 1 && C::copy_ctor == 0 && C::copy_assign == 0
            && C::move_assign == 0);
        assert(C::InstanceCount() == 2);
    }
    assert(C::InstanceCount() == 0);

    C::Reset();
    {
        C c;
        Optional<C> o1(c);
        const Optional<C> o2(o1);
        assert(o1.HasValue());
        assert(o2.HasValue());
        assert(C::def_ctor == 1 && C::move_ctor == 0 && C::copy_ctor == 2 && C::copy_assign == 0
            && C::move_assign == 0);
        assert(C::InstanceCount() == 3);
    }
    assert(C::InstanceCount() == 0);

    C::Reset();
    {
        C c;
        Optional<C> o1(c);
        const Optional<C> o2(std::move(o1));
        assert(C::def_ctor == 1 && C::copy_ctor == 1 && C::move_ctor == 1 && C::copy_assign == 0
            && C::move_assign == 0);
        assert(C::InstanceCount() == 3);
    }
    assert(C::InstanceCount() == 0);
}

void TestAssignment() {
    Optional<C> o1;
    Optional<C> o2;
    {  // Assign a value to empty
        C::Reset();
        C c;
        o1 = c;
        assert(C::def_ctor == 1 && C::copy_ctor == 1 && C::dtor == 0);
    }
    {  // Assign a non-empty to empty
        C::Reset();
        o2 = o1;
        assert(C::copy_ctor == 1 && C::copy_assign == 0 && C::dtor == 0);
    }
    {  // Assign non empty to non-empty
        C::Reset();
        o2 = o1;
        assert(C::copy_ctor == 0 && C::copy_assign == 1 && C::dtor == 0);
    }
    {  // Assign empty to non empty
        C::Reset();
        Optional<C> empty;
        o1 = empty;
        assert(C::copy_ctor == 0 && C::dtor == 1);
        assert(!o1.HasValue());
    }
}

void TestMoveAssignment() {
    {  // Assign a value to empty
        Optional<C> o1;
        C::Reset();
        C c;
        o1 = std::move(c);
        assert(C::def_ctor == 1 && C::move_ctor == 1 && C::dtor == 0);
    }
    {  // Assign a non-empty to empty
        Optional<C> o1;
        Optional<C> o2{ C{} };
        C::Reset();
        o1 = std::move(o2);
        assert(C::move_ctor == 1 && C::move_assign == 0 && C::dtor == 0);
    }
    {  // Assign non empty to non-empty
        Optional<C> o1{ C{} };
        Optional<C> o2{ C{} };
        C::Reset();
        o2 = std::move(o1);
        assert(C::copy_ctor == 0 && C::move_assign == 1 && C::dtor == 0);
    }
    {  // Assign empty to non empty
        Optional<C> o1{ C{} };
        C::Reset();
        Optional<C> empty;
        o1 = std::move(empty);
        assert(C::copy_ctor == 0 && C::move_ctor == 0 && C::move_assign == 0 && C::dtor == 1);
        assert(!o1.HasValue());
    }
}

void TestValueAccess() {
    using namespace std::literals;
    {
        Optional<std::string> o;
        o = "hello"s;
        assert(o.HasValue());
        assert(o.Value() == "hello"s);
        assert(&*o == &o.Value());
        assert(o->length() == 5);
    }
    {
        try {
            Optional<int> o;
            [[maybe_unused]] int v = o.Value();
            assert(false);
        }
        catch (const BadOptionalAccess& /*e*/) {
        }
        catch (...) {
            assert(false);
        }
    }
}

void TestReset() {
    C::Reset();
    {
        Optional<C> o{ C() };
        assert(o.HasValue());
        o.Reset();
        assert(!o.HasValue());
    }
}


// vector tests

namespace {

    struct Obj {
        Obj() {
            ++num_default_constructed;
        }

        Obj(const Obj& /*other*/) {
            ++num_copied;
        }

        Obj(Obj&& /*other*/) noexcept {
            ++num_moved;
        }

        Obj& operator=(const Obj& other) = default;
        Obj& operator=(Obj&& other) = default;

        ~Obj() {
            ++num_destroyed;
        }

        static int GetAliveObjectCount() {
            return num_default_constructed + num_copied + num_moved - num_destroyed;
        }

        static void ResetCounters() {
            num_default_constructed = 0;
            num_copied = 0;
            num_moved = 0;
            num_destroyed = 0;
        }

        static inline int num_default_constructed = 0;
        static inline int num_copied = 0;
        static inline int num_moved = 0;
        static inline int num_destroyed = 0;
    };

}  // namespace

void Test1() {
    Obj::ResetCounters();
    const size_t SIZE = 100500;
    const size_t INDEX = 10;
    const int MAGIC = 42;
    {
        Vector<int> v;
        assert(v.Capacity() == 0);
        assert(v.Size() == 0);

        v.Reserve(SIZE);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == 0);
    }
    {
        Vector<int> v(SIZE);
        const auto& cv(v);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == SIZE);
        assert(v[0] == 0);
        assert(&v[0] == &cv[0]);
        v[INDEX] = MAGIC;
        assert(v[INDEX] == MAGIC);
        assert(&v[100] - &v[0] == 100);

        v.Reserve(SIZE * 2);
        assert(v.Size() == SIZE);
        assert(v.Capacity() == SIZE * 2);
        assert(v[INDEX] == MAGIC);
    }
    {
        Vector<int> v(SIZE);
        v[INDEX] = MAGIC;
        const auto v_copy(v);
        assert(&v[INDEX] != &v_copy[INDEX]);
        assert(v[INDEX] == v_copy[INDEX]);
    }
    {
        Vector<Obj> v;
        v.Reserve(SIZE);
        assert(Obj::GetAliveObjectCount() == 0);
    }
    {
        Vector<Obj> v(SIZE);
        assert(Obj::GetAliveObjectCount() == SIZE);
        v.Reserve(SIZE * 2);
        assert(Obj::GetAliveObjectCount() == SIZE);
    }
    assert(Obj::GetAliveObjectCount() == 0);
}

namespace {

    struct Obj1 {
        Obj1() {
            if (default_construction_throw_countdown > 0) {
                if (--default_construction_throw_countdown == 0) {
                    throw std::runtime_error("Oops");
                }
            }
            ++num_default_constructed;
        }

        Obj1(const Obj1& other) {
            if (other.throw_on_copy) {
                throw std::runtime_error("Oops");
            }
            ++num_copied;
        }

        Obj1(Obj1&& /*other*/) noexcept {
            ++num_moved;
        }

        Obj1& operator=(const Obj1& other) = default;
        Obj1& operator=(Obj1&& other) = default;

        ~Obj1() {
            ++num_destroyed;
        }

        static int GetAliveObjectCount() {
            return num_default_constructed + num_copied + num_moved - num_destroyed;
        }

        static void ResetCounters() {
            default_construction_throw_countdown = 0;
            num_default_constructed = 0;
            num_copied = 0;
            num_moved = 0;
            num_destroyed = 0;
        }

        bool throw_on_copy = false;

        static inline int default_construction_throw_countdown = 0;
        static inline int num_default_constructed = 0;
        static inline int num_copied = 0;
        static inline int num_moved = 0;
        static inline int num_destroyed = 0;
    };

}  // namespace

void Test2() {
    Obj1::ResetCounters();
    const size_t SIZE = 100500;
    const size_t INDEX = 10;
    const int MAGIC = 42;
    {
        Vector<int> v;
        assert(v.Capacity() == 0);
        assert(v.Size() == 0);

        v.Reserve(SIZE);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == 0);
    }
    {
        Vector<int> v(SIZE);
        const auto& cv(v);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == SIZE);
        assert(v[0] == 0);
        assert(&v[0] == &cv[0]);
        v[INDEX] = MAGIC;
        assert(v[INDEX] == MAGIC);
        assert(&v[100] - &v[0] == 100);

        v.Reserve(SIZE * 2);
        assert(v.Size() == SIZE);
        assert(v.Capacity() == SIZE * 2);
        assert(v[INDEX] == MAGIC);
    }
    {
        Vector<int> v(SIZE);
        v[INDEX] = MAGIC;
        const auto v_copy(v);
        assert(&v[INDEX] != &v_copy[INDEX]);
        assert(v[INDEX] == v_copy[INDEX]);
    }
    {
        Vector<Obj1> v;
        v.Reserve(SIZE);
        assert(Obj1::GetAliveObjectCount() == 0);
    }
    {
        Vector<Obj1> v(SIZE);
        assert(Obj1::GetAliveObjectCount() == SIZE);
        v.Reserve(SIZE * 2);
        assert(Obj1::GetAliveObjectCount() == SIZE);
    }
    assert(Obj1::GetAliveObjectCount() == 0);
}

void Test3() {
    const size_t SIZE = 100;
    Obj1::ResetCounters();
    {
        Obj1::default_construction_throw_countdown = SIZE / 2;
        try {
            Vector<Obj1> v(SIZE);
            assert(false && "Exception is expected");
        }
        catch (const std::runtime_error&) {
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj1::num_default_constructed == SIZE / 2 - 1);
        assert(Obj1::GetAliveObjectCount() == 0);
    }
    Obj1::ResetCounters();
    {
        Vector<Obj1> v(SIZE);
        try {
            v[SIZE / 2].throw_on_copy = true;
            Vector<Obj1> v_copy(v);
            assert(false && "Exception is expected");
        }
        catch (const std::runtime_error&) {
            assert(Obj1::num_copied == SIZE / 2);
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj1::GetAliveObjectCount() == SIZE);
    }
    Obj1::ResetCounters();
    {
        Vector<Obj1> v(SIZE);
        try {
            v[SIZE - 1].throw_on_copy = true;
            v.Reserve(SIZE * 2);
            assert(false && "Exception is expected");
        }
        catch (const std::runtime_error&) {
            assert(Obj1::num_copied == SIZE - 1);
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(v.Capacity() == SIZE);
        assert(v.Size() == SIZE);
        assert(Obj1::GetAliveObjectCount() == SIZE);
    }
}

namespace {

    struct Obj2 {
        Obj2() {
            if (default_construction_throw_countdown > 0) {
                if (--default_construction_throw_countdown == 0) {
                    throw std::runtime_error("Oops");
                }
            }
            ++num_default_constructed;
        }

        Obj2(const Obj2& other) {
            if (other.throw_on_copy) {
                throw std::runtime_error("Oops");
            }
            ++num_copied;
        }

        Obj2(Obj2&& /*other*/) noexcept {
            ++num_moved;
        }

        Obj2& operator=(const Obj2& other) = default;
        Obj2& operator=(Obj2&& other) = default;

        ~Obj2() {
            ++num_destroyed;
        }

        static int GetAliveObj2ectCount() {
            return num_default_constructed + num_copied + num_moved - num_destroyed;
        }

        static void ResetCounters() {
            default_construction_throw_countdown = 0;
            num_default_constructed = 0;
            num_copied = 0;
            num_moved = 0;
            num_destroyed = 0;
        }

        bool throw_on_copy = false;

        static inline int default_construction_throw_countdown = 0;
        static inline int num_default_constructed = 0;
        static inline int num_copied = 0;
        static inline int num_moved = 0;
        static inline int num_destroyed = 0;
    };

}  // namespace

void Test4() {
    Obj2::ResetCounters();
    const size_t SIZE = 100500;
    const size_t INDEX = 10;
    const int MAGIC = 42;
    {
        Vector<int> v;
        assert(v.Capacity() == 0);
        assert(v.Size() == 0);

        v.Reserve(SIZE);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == 0);
    }
    {
        Vector<int> v(SIZE);
        const auto& cv(v);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == SIZE);
        assert(v[0] == 0);
        assert(&v[0] == &cv[0]);
        v[INDEX] = MAGIC;
        assert(v[INDEX] == MAGIC);
        assert(&v[100] - &v[0] == 100);

        v.Reserve(SIZE * 2);
        assert(v.Size() == SIZE);
        assert(v.Capacity() == SIZE * 2);
        assert(v[INDEX] == MAGIC);
    }
    {
        Vector<int> v(SIZE);
        v[INDEX] = MAGIC;
        const auto v_copy(v);
        assert(&v[INDEX] != &v_copy[INDEX]);
        assert(v[INDEX] == v_copy[INDEX]);
    }
    {
        Vector<Obj2> v;
        v.Reserve(SIZE);
        assert(Obj2::GetAliveObj2ectCount() == 0);
    }
    {
        Vector<Obj2> v(SIZE);
        assert(Obj2::GetAliveObj2ectCount() == SIZE);
        const int old_copy_count = Obj2::num_copied;
        const int old_move_count = Obj2::num_moved;
        v.Reserve(SIZE * 2);
        assert(Obj2::GetAliveObj2ectCount() == SIZE);
        assert(Obj2::num_copied == old_copy_count);
        assert(Obj2::num_moved == old_move_count + static_cast<int>(SIZE));
    }
    assert(Obj2::GetAliveObj2ectCount() == 0);
}

void Test5() {
    const size_t SIZE = 100;
    Obj2::ResetCounters();
    {
        Obj2::default_construction_throw_countdown = SIZE / 2;
        try {
            Vector<Obj2> v(SIZE);
            assert(false && "Exception is expected");
        }
        catch (const std::runtime_error&) {
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj2::num_default_constructed == SIZE / 2 - 1);
        assert(Obj2::GetAliveObj2ectCount() == 0);
    }
    Obj2::ResetCounters();
    {
        Vector<Obj2> v(SIZE);
        try {
            v[SIZE / 2].throw_on_copy = true;
            Vector<Obj2> v_copy(v);
            assert(false && "Exception is expected");
        }
        catch (const std::runtime_error&) {
            assert(Obj2::num_copied == SIZE / 2);
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj2::GetAliveObj2ectCount() == SIZE);
    }
    Obj2::ResetCounters();
    {
        Vector<Obj2> v(SIZE);
        try {
            v[SIZE - 1].throw_on_copy = true;
            v.Reserve(SIZE * 2);
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(v.Capacity() == SIZE * 2);
        assert(v.Size() == SIZE);
        assert(Obj2::GetAliveObj2ectCount() == SIZE);
    }
}

namespace {

    struct Obj3 {
        Obj3() {
            if (default_construction_throw_countdown > 0) {
                if (--default_construction_throw_countdown == 0) {
                    throw std::runtime_error("Oops");
                }
            }
            ++num_default_constructed;
        }

        explicit Obj3(int id)
            : id(id)  //
        {
            ++num_constructed_with_id;
        }

        Obj3(const Obj3& other)
            : id(other.id)  //
        {
            if (other.throw_on_copy) {
                throw std::runtime_error("Oops");
            }
            ++num_copied;
        }

        Obj3(Obj3&& other) noexcept
            : id(other.id)  //
        {
            ++num_moved;
        }

        Obj3& operator=(const Obj3& other) = default;
        Obj3& operator=(Obj3&& other) = default;

        ~Obj3() {
            ++num_destroyed;
            id = 0;
        }

        static int GetAliveObj3ectCount() {
            return num_default_constructed + num_copied + num_moved + num_constructed_with_id
                - num_destroyed;
        }

        static void ResetCounters() {
            default_construction_throw_countdown = 0;
            num_default_constructed = 0;
            num_copied = 0;
            num_moved = 0;
            num_destroyed = 0;
            num_constructed_with_id = 0;
        }

        bool throw_on_copy = false;
        int id = 0;

        static inline int default_construction_throw_countdown = 0;
        static inline int num_default_constructed = 0;
        static inline int num_constructed_with_id = 0;
        static inline int num_copied = 0;
        static inline int num_moved = 0;
        static inline int num_destroyed = 0;
    };

}  // namespace

void Test6() {
    Obj3::ResetCounters();
    const size_t SIZE = 100500;
    const size_t INDEX = 10;
    const int MAGIC = 42;
    {
        Vector<int> v;
        assert(v.Capacity() == 0);
        assert(v.Size() == 0);

        v.Reserve(SIZE);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == 0);
    }
    {
        Vector<int> v(SIZE);
        const auto& cv(v);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == SIZE);
        assert(v[0] == 0);
        assert(&v[0] == &cv[0]);
        v[INDEX] = MAGIC;
        assert(v[INDEX] == MAGIC);
        assert(&v[100] - &v[0] == 100);

        v.Reserve(SIZE * 2);
        assert(v.Size() == SIZE);
        assert(v.Capacity() == SIZE * 2);
        assert(v[INDEX] == MAGIC);
    }
    {
        Vector<int> v(SIZE);
        v[INDEX] = MAGIC;
        const auto v_copy(v);
        assert(&v[INDEX] != &v_copy[INDEX]);
        assert(v[INDEX] == v_copy[INDEX]);
    }
    {
        Vector<Obj3> v;
        v.Reserve(SIZE);
        assert(Obj3::GetAliveObj3ectCount() == 0);
    }
    {
        Vector<Obj3> v(SIZE);
        assert(Obj3::GetAliveObj3ectCount() == SIZE);
        const int old_copy_count = Obj3::num_copied;
        const int old_move_count = Obj3::num_moved;
        v.Reserve(SIZE * 2);
        assert(Obj3::GetAliveObj3ectCount() == SIZE);
        assert(Obj3::num_copied == old_copy_count);
        assert(Obj3::num_moved == old_move_count + static_cast<int>(SIZE));
    }
    assert(Obj3::GetAliveObj3ectCount() == 0);
}

void Test7() {
    const size_t SIZE = 100;
    Obj3::ResetCounters();
    {
        Obj3::default_construction_throw_countdown = SIZE / 2;
        try {
            Vector<Obj3> v(SIZE);
            assert(false && "Exception is expected");
        }
        catch (const std::runtime_error&) {
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj3::num_default_constructed == SIZE / 2 - 1);
        assert(Obj3::GetAliveObj3ectCount() == 0);
    }
    Obj3::ResetCounters();
    {
        Vector<Obj3> v(SIZE);
        try {
            v[SIZE / 2].throw_on_copy = true;
            Vector<Obj3> v_copy(v);
            assert(false && "Exception is expected");
        }
        catch (const std::runtime_error&) {
            assert(Obj3::num_copied == SIZE / 2);
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj3::GetAliveObj3ectCount() == SIZE);
    }
    Obj3::ResetCounters();
    {
        Vector<Obj3> v(SIZE);
        try {
            v[SIZE - 1].throw_on_copy = true;
            v.Reserve(SIZE * 2);
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(v.Capacity() == SIZE * 2);
        assert(v.Size() == SIZE);
        assert(Obj3::GetAliveObj3ectCount() == SIZE);
    }
}

void Test8() {
    const size_t MEDIUM_SIZE = 100;
    const size_t LARGE_SIZE = 250;
    const int ID = 42;
    {
        Obj3::ResetCounters();
        Vector<int> v(MEDIUM_SIZE);
        {
            auto v_copy(std::move(v));

            assert(v_copy.Size() == MEDIUM_SIZE);
            assert(v_copy.Capacity() == MEDIUM_SIZE);
        }
        assert(Obj3::GetAliveObj3ectCount() == 0);
    }
    {
        Obj3::ResetCounters();
        {
            Vector<Obj3> v(MEDIUM_SIZE);
            v[MEDIUM_SIZE / 2].id = ID;
            assert(Obj3::num_default_constructed == MEDIUM_SIZE);
            Vector<Obj3> moved_from_v(std::move(v));
            assert(moved_from_v.Size() == MEDIUM_SIZE);
            assert(moved_from_v[MEDIUM_SIZE / 2].id == ID);
        }
        assert(Obj3::GetAliveObj3ectCount() == 0);

        assert(Obj3::num_moved == 0);
        assert(Obj3::num_copied == 0);
        assert(Obj3::num_default_constructed == MEDIUM_SIZE);
    }
    {
        Obj3::ResetCounters();
        Vector<Obj3> v_medium(MEDIUM_SIZE);
        v_medium[MEDIUM_SIZE / 2].id = ID;
        Vector<Obj3> v_large(LARGE_SIZE);
        v_large = v_medium;
        assert(v_large.Size() == MEDIUM_SIZE);
        assert(v_large.Capacity() == LARGE_SIZE);
        assert(v_large[MEDIUM_SIZE / 2].id == ID);
        assert(Obj3::GetAliveObj3ectCount() == MEDIUM_SIZE + MEDIUM_SIZE);
    }
    {
        Obj3::ResetCounters();
        Vector<Obj3> v(MEDIUM_SIZE);
        {
            Vector<Obj3> v_large(LARGE_SIZE);
            v_large[LARGE_SIZE - 1].id = ID;
            v = v_large;
            assert(v.Size() == LARGE_SIZE);
            assert(v_large.Capacity() == LARGE_SIZE);
            assert(v_large[LARGE_SIZE - 1].id == ID);
            assert(Obj3::GetAliveObj3ectCount() == LARGE_SIZE + LARGE_SIZE);
        }
        assert(Obj3::GetAliveObj3ectCount() == LARGE_SIZE);
    }
    assert(Obj3::GetAliveObj3ectCount() == 0);
    {
        Obj3::ResetCounters();
        Vector<Obj3> v(MEDIUM_SIZE);
        v[MEDIUM_SIZE - 1].id = ID;
        Vector<Obj3> v_small(MEDIUM_SIZE / 2);
        v_small.Reserve(MEDIUM_SIZE + 1);
        const size_t num_copies = Obj3::num_copied;
        v_small = v;
        assert(v_small.Size() == v.Size());
        assert(v_small.Capacity() == MEDIUM_SIZE + 1);
        v_small[MEDIUM_SIZE - 1].id = ID;
        assert(Obj3::num_copied - num_copies == MEDIUM_SIZE - (MEDIUM_SIZE / 2));
    }
}

namespace {

    // Магическое число, используемое для отслеживания живости объекта
    inline const uint32_t DEFAULT_COOKIE = 0xdeadbeef;

    struct TestObj {
        TestObj() = default;
        TestObj(const TestObj& other) = default;
        TestObj& operator=(const TestObj& other) = default;
        TestObj(TestObj&& other) = default;
        TestObj& operator=(TestObj&& other) = default;
        ~TestObj() {
            cookie = 0;
        }
        [[nodiscard]] bool IsAlive() const noexcept {
            return cookie == DEFAULT_COOKIE;
        }
        uint32_t cookie = DEFAULT_COOKIE;
    };

    struct Obj4 {
        Obj4() {
            if (default_construction_throw_countdown > 0) {
                if (--default_construction_throw_countdown == 0) {
                    throw std::runtime_error("Oops");
                }
            }
            ++num_default_constructed;
        }

        explicit Obj4(int id)
            : id(id)  //
        {
            ++num_constructed_with_id;
        }

        Obj4(const Obj4& other)
            : id(other.id)  //
        {
            if (other.throw_on_copy) {
                throw std::runtime_error("Oops");
            }
            ++num_copied;
        }

        Obj4(Obj4&& other) noexcept
            : id(other.id)  //
        {
            ++num_moved;
        }

        Obj4& operator=(const Obj4& other) = default;
        Obj4& operator=(Obj4&& other) = default;

        ~Obj4() {
            ++num_destroyed;
            id = 0;
        }

        static int GetAliveObj4ectCount() {
            return num_default_constructed + num_copied + num_moved + num_constructed_with_id
                - num_destroyed;
        }

        static void ResetCounters() {
            default_construction_throw_countdown = 0;
            num_default_constructed = 0;
            num_copied = 0;
            num_moved = 0;
            num_destroyed = 0;
            num_constructed_with_id = 0;
        }

        bool throw_on_copy = false;
        int id = 0;

        static inline int default_construction_throw_countdown = 0;
        static inline int num_default_constructed = 0;
        static inline int num_constructed_with_id = 0;
        static inline int num_copied = 0;
        static inline int num_moved = 0;
        static inline int num_destroyed = 0;
    };

}  // namespace

void Test9() {
    Obj4::ResetCounters();
    const size_t SIZE = 100500;
    const size_t INDEX = 10;
    const int MAGIC = 42;
    {
        Vector<int> v;
        assert(v.Capacity() == 0);
        assert(v.Size() == 0);

        v.Reserve(SIZE);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == 0);
    }
    {
        Vector<int> v(SIZE);
        const auto& cv(v);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == SIZE);
        assert(v[0] == 0);
        assert(&v[0] == &cv[0]);
        v[INDEX] = MAGIC;
        assert(v[INDEX] == MAGIC);
        assert(&v[100] - &v[0] == 100);

        v.Reserve(SIZE * 2);
        assert(v.Size() == SIZE);
        assert(v.Capacity() == SIZE * 2);
        assert(v[INDEX] == MAGIC);
    }
    {
        Vector<int> v(SIZE);
        v[INDEX] = MAGIC;
        const auto v_copy(v);
        assert(&v[INDEX] != &v_copy[INDEX]);
        assert(v[INDEX] == v_copy[INDEX]);
    }
    {
        Vector<Obj4> v;
        v.Reserve(SIZE);
        assert(Obj4::GetAliveObj4ectCount() == 0);
    }
    {
        Vector<Obj4> v(SIZE);
        assert(Obj4::GetAliveObj4ectCount() == SIZE);
        const int old_copy_count = Obj4::num_copied;
        const int old_move_count = Obj4::num_moved;
        v.Reserve(SIZE * 2);
        assert(Obj4::GetAliveObj4ectCount() == SIZE);
        assert(Obj4::num_copied == old_copy_count);
        assert(Obj4::num_moved == old_move_count + static_cast<int>(SIZE));
    }
    assert(Obj4::GetAliveObj4ectCount() == 0);
}

void Test10() {
    const size_t SIZE = 100;
    Obj4::ResetCounters();
    {
        Obj4::default_construction_throw_countdown = SIZE / 2;
        try {
            Vector<Obj4> v(SIZE);
            assert(false && "Exception is expected");
        }
        catch (const std::runtime_error&) {
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj4::num_default_constructed == SIZE / 2 - 1);
        assert(Obj4::GetAliveObj4ectCount() == 0);
    }
    Obj4::ResetCounters();
    {
        Vector<Obj4> v(SIZE);
        try {
            v[SIZE / 2].throw_on_copy = true;
            Vector<Obj4> v_copy(v);
            assert(false && "Exception is expected");
        }
        catch (const std::runtime_error&) {
            assert(Obj4::num_copied == SIZE / 2);
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj4::GetAliveObj4ectCount() == SIZE);
    }
    Obj4::ResetCounters();
    {
        Vector<Obj4> v(SIZE);
        try {
            v[SIZE - 1].throw_on_copy = true;
            v.Reserve(SIZE * 2);
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(v.Capacity() == SIZE * 2);
        assert(v.Size() == SIZE);
        assert(Obj4::GetAliveObj4ectCount() == SIZE);
    }
}

void Test11() {
    const size_t MEDIUM_SIZE = 100;
    const size_t LARGE_SIZE = 250;
    const int ID = 42;
    {
        Obj4::ResetCounters();
        Vector<int> v(MEDIUM_SIZE);
        {
            auto v_copy(std::move(v));

            assert(v_copy.Size() == MEDIUM_SIZE);
            assert(v_copy.Capacity() == MEDIUM_SIZE);
        }
        assert(Obj4::GetAliveObj4ectCount() == 0);
    }
    {
        Obj4::ResetCounters();
        {
            Vector<Obj4> v(MEDIUM_SIZE);
            v[MEDIUM_SIZE / 2].id = ID;
            assert(Obj4::num_default_constructed == MEDIUM_SIZE);
            Vector<Obj4> moved_from_v(std::move(v));
            assert(moved_from_v.Size() == MEDIUM_SIZE);
            assert(moved_from_v[MEDIUM_SIZE / 2].id == ID);
        }
        assert(Obj4::GetAliveObj4ectCount() == 0);

        assert(Obj4::num_moved == 0);
        assert(Obj4::num_copied == 0);
        assert(Obj4::num_default_constructed == MEDIUM_SIZE);
    }
    {
        Obj4::ResetCounters();
        Vector<Obj4> v_medium(MEDIUM_SIZE);
        v_medium[MEDIUM_SIZE / 2].id = ID;
        Vector<Obj4> v_large(LARGE_SIZE);
        v_large = v_medium;
        assert(v_large.Size() == MEDIUM_SIZE);
        assert(v_large.Capacity() == LARGE_SIZE);
        assert(v_large[MEDIUM_SIZE / 2].id == ID);
        assert(Obj4::GetAliveObj4ectCount() == MEDIUM_SIZE + MEDIUM_SIZE);
    }
    {
        Obj4::ResetCounters();
        Vector<Obj4> v(MEDIUM_SIZE);
        {
            Vector<Obj4> v_large(LARGE_SIZE);
            v_large[LARGE_SIZE - 1].id = ID;
            v = v_large;
            assert(v.Size() == LARGE_SIZE);
            assert(v_large.Capacity() == LARGE_SIZE);
            assert(v_large[LARGE_SIZE - 1].id == ID);
            assert(Obj4::GetAliveObj4ectCount() == LARGE_SIZE + LARGE_SIZE);
        }
        assert(Obj4::GetAliveObj4ectCount() == LARGE_SIZE);
    }
    assert(Obj4::GetAliveObj4ectCount() == 0);
    {
        Obj4::ResetCounters();
        Vector<Obj4> v(MEDIUM_SIZE);
        v[MEDIUM_SIZE - 1].id = ID;
        Vector<Obj4> v_small(MEDIUM_SIZE / 2);
        v_small.Reserve(MEDIUM_SIZE + 1);
        const size_t num_copies = Obj4::num_copied;
        v_small = v;
        assert(v_small.Size() == v.Size());
        assert(v_small.Capacity() == MEDIUM_SIZE + 1);
        v_small[MEDIUM_SIZE - 1].id = ID;
        assert(Obj4::num_copied - num_copies == MEDIUM_SIZE - (MEDIUM_SIZE / 2));
    }
}

void Test12() {
    const size_t ID = 42;
    const size_t SIZE = 100'500;
    {
        Obj4::ResetCounters();
        Vector<Obj4> v;
        v.Resize(SIZE);
        assert(v.Size() == SIZE);
        assert(v.Capacity() == SIZE);
        assert(Obj4::num_default_constructed == SIZE);
    }
    assert(Obj4::GetAliveObj4ectCount() == 0);

    {
        const size_t NEW_SIZE = 10'000;
        Obj4::ResetCounters();
        Vector<Obj4> v(SIZE);
        v.Resize(NEW_SIZE);
        assert(v.Size() == NEW_SIZE);
        assert(v.Capacity() == SIZE);
        assert(Obj4::num_destroyed == SIZE - NEW_SIZE);
    }
    assert(Obj4::GetAliveObj4ectCount() == 0);
    {
        Obj4::ResetCounters();
        Vector<Obj4> v(SIZE);
        Obj4 o{ ID };
        v.PushBack(o);
        assert(v.Size() == SIZE + 1);
        assert(v.Capacity() == SIZE * 2);
        assert(v[SIZE].id == ID);
        assert(Obj4::num_default_constructed == SIZE);
        assert(Obj4::num_copied == 1);
        assert(Obj4::num_constructed_with_id == 1);
        assert(Obj4::num_moved == SIZE);
    }
    int _ = Obj4::GetAliveObj4ectCount();
    assert(Obj4::GetAliveObj4ectCount() == 0);
    {
        Obj4::ResetCounters();
        Vector<Obj4> v(SIZE);
        v.PushBack(Obj4{ ID });
        assert(v.Size() == SIZE + 1);
        assert(v.Capacity() == SIZE * 2);
        assert(v[SIZE].id == ID);
        assert(Obj4::num_default_constructed == SIZE);
        assert(Obj4::num_copied == 0);
        assert(Obj4::num_constructed_with_id == 1);
        assert(Obj4::num_moved == SIZE + 1);
    }
    {
        Obj4::ResetCounters();
        Vector<Obj4> v;
        v.PushBack(Obj4{ ID });
        v.PopBack();
        assert(v.Size() == 0);
        assert(v.Capacity() == 1);
        assert(Obj4::GetAliveObj4ectCount() == 0);
    }

    {
        Vector<TestObj> v(1);
        assert(v.Size() == v.Capacity());
        // Операция PushBack существующего элемента вектора должна быть безопасна
        // даже при реаллокации памяти
        v.PushBack(v[0]);
        assert(v[0].IsAlive());
        assert(v[1].IsAlive());
    }
    {
        Vector<TestObj> v(1);
        assert(v.Size() == v.Capacity());
        // Операция PushBack для перемещения существующего элемента вектора должна быть безопасна
        // даже при реаллокации памяти
        v.PushBack(std::move(v[0]));
        assert(v[0].IsAlive());
        assert(v[1].IsAlive());
    }
}

template<bool MoveNoexcept>
struct WithCopy {
    WithCopy() noexcept {
        ++def_ctor;
    }

    WithCopy(const int&) noexcept {
        ++copy_with_val;
    }

    WithCopy(int&&) noexcept {
        ++move_with_val;
    }

    WithCopy(const WithCopy& /*other*/) noexcept {
        ++copy_ctor;
    }

    WithCopy(WithCopy&& /*other*/) noexcept(MoveNoexcept) {
        ++move_ctor;
    }

    WithCopy& operator=(const WithCopy& other) noexcept {
        if (this != &other) {
            ++copy_assign;
        }
        return *this;
    }

    WithCopy& operator=(WithCopy&& /*other*/) noexcept {
        ++move_assign;
        return *this;
    }

    ~WithCopy() {
        ++dtor;
    }

    static size_t InstanceCount() {
        return def_ctor + copy_ctor + move_ctor - dtor;
    }

    static void Reset() {
        def_ctor = 0;
        copy_ctor = 0;
        move_ctor = 0;
        copy_assign = 0;
        move_assign = 0;
        dtor = 0;
        copy_with_val = 0;
        move_with_val = 0;
    }

    inline static size_t def_ctor = 0;
    inline static size_t copy_ctor = 0;
    inline static size_t move_ctor = 0;
    inline static size_t copy_assign = 0;
    inline static size_t move_assign = 0;
    inline static size_t dtor = 0;
    inline static size_t copy_with_val = 0;
    inline static size_t move_with_val = 0;

};

using move_without_noexcept = WithCopy<false>;

void TestPushBackAdditional_move_without_noexcept_copy() {
    using OBJ = move_without_noexcept;
    constexpr std::size_t size = 8u;

    {
        OBJ a;
        Vector<OBJ> v(size);
        OBJ::Reset();
        v.PushBack(a);
        assert(OBJ::def_ctor == 0u);
        assert(OBJ::copy_ctor == size + 1);
        assert(OBJ::move_ctor == 0u);
        assert(OBJ::copy_assign == 0u);
        assert(OBJ::move_assign == 0u);
        assert(OBJ::dtor == size);

    } {
        OBJ a;
        Vector<OBJ> v(size);
        v.Reserve(2 * size);
        OBJ::Reset();
        v.PushBack(a);
        assert(OBJ::def_ctor == 0u);
        assert(OBJ::copy_ctor == 1u);
        assert(OBJ::move_ctor == 0u);
        assert(OBJ::copy_assign == 0u);
        assert(OBJ::move_assign == 0u);
        assert(OBJ::dtor == 0);
    } {
        OBJ a;
        Vector<OBJ> v(size);
        OBJ::Reset();
        v.PushBack(std::move(a));
        assert(OBJ::def_ctor == 0u);
        assert(OBJ::copy_ctor == size);
        assert(OBJ::move_ctor == 1u);
        assert(OBJ::copy_assign == 0u);
        assert(OBJ::move_assign == 0u);
        assert(OBJ::dtor == size);
    } {
        OBJ a;
        Vector<OBJ> v(size);
        v.Reserve(2 * size);
        OBJ::Reset();
        v.PushBack(std::move(a));
        assert(OBJ::def_ctor == 0u);
        assert(OBJ::copy_ctor == 0u);
        assert(OBJ::move_ctor == 1u);
        assert(OBJ::copy_assign == 0u);
        assert(OBJ::move_assign == 0u);
        assert(OBJ::dtor == 0u);
    }
    //cerr << "Test passed"s << endl;
}

struct C1 {
    C1() noexcept {
        ++def_ctor;
    }
    C1(const C1& /*other*/) noexcept {
        ++copy_ctor;
    }
    C1(C1&& /*other*/) noexcept {
        ++move_ctor;
    }
    C1& operator=(const C1& other) noexcept {
        if (this != &other) {
            ++copy_assign;
        }
        return *this;
    }
    C1& operator=(C1&& /*other*/) noexcept {
        ++move_assign;
        return *this;
    }
    ~C1() {
        ++dtor;
    }

    static size_t InstanceCount() {
        return def_ctor + copy_ctor + move_ctor - dtor;
    }

    static void Reset() {
        def_ctor = 0;
        copy_ctor = 0;
        move_ctor = 0;
        copy_assign = 0;
        move_assign = 0;
        dtor = 0;
    }

    inline static size_t def_ctor = 0;
    inline static size_t copy_ctor = 0;
    inline static size_t move_ctor = 0;
    inline static size_t copy_assign = 0;
    inline static size_t move_assign = 0;
    inline static size_t dtor = 0;
};

void TestInitialization1() {
    C1::Reset();
    {
        Optional<C1> o;
        assert(!o.HasValue());
        assert(C1::InstanceCount() == 0);
    }
    assert(C::InstanceCount() == 0);

    C1::Reset();
    {
        C1 c;
        Optional<C1> o(c);
        assert(o.HasValue());
        assert(C1::def_ctor == 1 && C1::copy_ctor == 1);
        assert(C1::InstanceCount() == 2);
    }
    assert(C1::InstanceCount() == 0);

    C1::Reset();
    {
        C1 c;
        Optional<C1> o(std::move(c));
        assert(o.HasValue());
        assert(C1::def_ctor == 1 && C1::move_ctor == 1 && C1::copy_ctor == 0 && C1::copy_assign == 0
            && C1::move_assign == 0);
        assert(C1::InstanceCount() == 2);
    }
    assert(C1::InstanceCount() == 0);

    C1::Reset();
    {
        C1 c;
        Optional<C1> o1(c);
        const Optional<C1> o2(o1);
        assert(o1.HasValue());
        assert(o2.HasValue());
        assert(C1::def_ctor == 1 && C1::move_ctor == 0 && C1::copy_ctor == 2 && C1::copy_assign == 0
            && C1::move_assign == 0);
        assert(C1::InstanceCount() == 3);
    }
    assert(C1::InstanceCount() == 0);

    C1::Reset();
    {
        C1 c;
        Optional<C1> o1(c);
        const Optional<C1> o2(std::move(o1));
        assert(C1::def_ctor == 1 && C1::copy_ctor == 1 && C1::move_ctor == 1 && C1::copy_assign == 0
            && C1::move_assign == 0);
        assert(C1::InstanceCount() == 3);
    }
    assert(C1::InstanceCount() == 0);
}

void TestAssignment1() {
    Optional<C1> o1;
    Optional<C1> o2;
    {  // Assign a value to empty
        C1::Reset();
        C1 c;
        o1 = c;
        assert(C1::def_ctor == 1 && C1::copy_ctor == 1 && C1::dtor == 0);
    }
    {  // Assign a non-empty to empty
        C1::Reset();
        o2 = o1;
        assert(C1::copy_ctor == 1 && C1::copy_assign == 0 && C1::dtor == 0);
    }
    {  // Assign non-empty to non-empty
        C1::Reset();
        o2 = o1;
        assert(C1::copy_ctor == 0 && C1::copy_assign == 1 && C1::dtor == 0);
    }
    {  // Assign empty to non-empty
        C1::Reset();
        Optional<C1> empty;
        o1 = empty;
        assert(C1::copy_ctor == 0 && C1::dtor == 1);
        assert(!o1.HasValue());
    }
}

void TestMoveAssignment1() {
    {  // Assign a value to empty
        Optional<C1> o1;
        C1::Reset();
        C1 c;
        o1 = std::move(c);
        assert(C1::def_ctor == 1 && C1::move_ctor == 1 && C1::dtor == 0);
    }
    {  // Assign a non-empty to empty
        Optional<C1> o1;
        Optional<C1> o2{ C1{} };
        C1::Reset();
        o1 = std::move(o2);
        assert(C1::move_ctor == 1 && C1::move_assign == 0 && C1::dtor == 0);
    }
    {  // Assign non-empty to non-empty
        Optional<C1> o1{ C1{} };
        Optional<C1> o2{ C1{} };
        C1::Reset();
        o2 = std::move(o1);
        assert(C1::copy_ctor == 0 && C1::move_assign == 1 && C1::dtor == 0);
    }
    {  // Assign empty to non-empty
        Optional<C1> o1{ C1{} };
        C1::Reset();
        Optional<C1> empty;
        o1 = std::move(empty);
        assert(C1::copy_ctor == 0 && C1::move_ctor == 0 && C1::move_assign == 0 && C1::dtor == 1);
        assert(!o1.HasValue());
    }
}

void TestValueAccess1() {
    using namespace std::literals;
    {
        Optional<std::string> o;
        o = "hello"s;
        assert(o.HasValue());
        assert(o.Value() == "hello"s);
        assert(&*o == &o.Value());
        assert(o->length() == 5);
    }
    {
        try {
            Optional<int> o;
            [[maybe_unused]] int v = o.Value();
            assert(false);
        }
        catch (const BadOptionalAccess& /*e*/) {
        }
        catch (...) {
            assert(false);
        }
    }
}

void TestReset1() {
    C1::Reset();
    {
        Optional<C1> o{ C1() };
        assert(o.HasValue());
        o.Reset();
        assert(!o.HasValue());
    }
}

void TestEmplace1() {
    struct S {
        S(int i, std::unique_ptr<int>&& p)
            : i(i)
            , p(std::move(p))  //
        {
        }
        int i;
        std::unique_ptr<int> p;
    };

    Optional<S> o;
    o.Emplace(1, std::make_unique<int>(2));
    assert(o.HasValue());
    assert(o->i == 1);
    assert(*(o->p) == 2);

    o.Emplace(3, std::make_unique<int>(4));
    assert(o.HasValue());
    assert(o->i == 3);
    assert(*(o->p) == 4);
}

namespace {

    // "Магическое" число, используемое для отслеживания живости объекта
    //inline const uint32_t DEFAULT_COOKIE = 0xdeadbeef;

    struct TestObj5 {
        TestObj5() = default;
        TestObj5(const TestObj5& other) = default;
        TestObj5& operator=(const TestObj5& other) = default;
        TestObj5(TestObj5&& other) = default;
        TestObj5& operator=(TestObj5&& other) = default;
        ~TestObj5() {
            cookie = 0;
        }
        [[nodiscard]] bool IsAlive() const noexcept {
            return cookie == DEFAULT_COOKIE;
        }
        uint32_t cookie = DEFAULT_COOKIE;
    };

    struct Obj5 {
        Obj5() {
            if (default_construction_throw_countdown > 0) {
                if (--default_construction_throw_countdown == 0) {
                    throw std::runtime_error("Oops");
                }
            }
            ++num_default_constructed;
        }

        explicit Obj5(int id)
            : id(id)  //
        {
            ++num_constructed_with_id;
        }

        Obj5(int id, std::string name)
            : id(id)
            , name(std::move(name))  //
        {
            ++num_constructed_with_id_and_name;
        }

        Obj5(const Obj5& other)
            : id(other.id)  //
        {
            if (other.throw_on_copy) {
                throw std::runtime_error("Oops");
            }
            ++num_copied;
        }

        Obj5(Obj5&& other) noexcept
            : id(other.id)  //
        {
            ++num_moved;
        }

        Obj5& operator=(const Obj5& other) = default;
        Obj5& operator=(Obj5&& other) = default;

        ~Obj5() {
            ++num_destroyed;
            id = 0;
        }

        static int GetAliveObj5ectCount() {
            return num_default_constructed + num_copied + num_moved + num_constructed_with_id
                + num_constructed_with_id_and_name - num_destroyed;
        }

        static void ResetCounters() {
            default_construction_throw_countdown = 0;
            num_default_constructed = 0;
            num_copied = 0;
            num_moved = 0;
            num_destroyed = 0;
            num_constructed_with_id = 0;
            num_constructed_with_id_and_name = 0;
        }

        bool throw_on_copy = false;
        int id = 0;
        std::string name;

        static inline int default_construction_throw_countdown = 0;
        static inline int num_default_constructed = 0;
        static inline int num_constructed_with_id = 0;
        static inline int num_constructed_with_id_and_name = 0;
        static inline int num_copied = 0;
        static inline int num_moved = 0;
        static inline int num_destroyed = 0;
    };

}  // namespace

void Test13() {
    Obj5::ResetCounters();
    const size_t SIZE = 100500;
    const size_t INDEX = 10;
    const int MAGIC = 42;
    {
        Vector<int> v;
        assert(v.Capacity() == 0);
        assert(v.Size() == 0);

        v.Reserve(SIZE);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == 0);
    }
    {
        Vector<int> v(SIZE);
        const auto& cv(v);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == SIZE);
        assert(v[0] == 0);
        assert(&v[0] == &cv[0]);
        v[INDEX] = MAGIC;
        assert(v[INDEX] == MAGIC);
        assert(&v[100] - &v[0] == 100);

        v.Reserve(SIZE * 2);
        assert(v.Size() == SIZE);
        assert(v.Capacity() == SIZE * 2);
        assert(v[INDEX] == MAGIC);
    }
    {
        Vector<int> v(SIZE);
        v[INDEX] = MAGIC;
        const auto v_copy(v);
        assert(&v[INDEX] != &v_copy[INDEX]);
        assert(v[INDEX] == v_copy[INDEX]);
    }
    {
        Vector<Obj5> v;
        v.Reserve(SIZE);
        assert(Obj5::GetAliveObj5ectCount() == 0);
    }
    {
        Vector<Obj5> v(SIZE);
        assert(Obj5::GetAliveObj5ectCount() == SIZE);
        const int old_copy_count = Obj5::num_copied;
        const int old_move_count = Obj5::num_moved;
        v.Reserve(SIZE * 2);
        assert(Obj5::GetAliveObj5ectCount() == SIZE);
        assert(Obj5::num_copied == old_copy_count);
        assert(Obj5::num_moved == old_move_count + static_cast<int>(SIZE));
    }
    assert(Obj5::GetAliveObj5ectCount() == 0);
}

void Test14() {
    const size_t SIZE = 100;
    Obj5::ResetCounters();
    {
        Obj5::default_construction_throw_countdown = SIZE / 2;
        try {
            Vector<Obj5> v(SIZE);
            assert(false && "Exception is expected");
        }
        catch (const std::runtime_error&) {
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj5::num_default_constructed == SIZE / 2 - 1);
        assert(Obj5::GetAliveObj5ectCount() == 0);
    }
    Obj5::ResetCounters();
    {
        Vector<Obj5> v(SIZE);
        try {
            v[SIZE / 2].throw_on_copy = true;
            Vector<Obj5> v_copy(v);
            assert(false && "Exception is expected");
        }
        catch (const std::runtime_error&) {
            assert(Obj5::num_copied == SIZE / 2);
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj5::GetAliveObj5ectCount() == SIZE);
    }
    Obj5::ResetCounters();
    {
        Vector<Obj5> v(SIZE);
        try {
            v[SIZE - 1].throw_on_copy = true;
            v.Reserve(SIZE * 2);
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(v.Capacity() == SIZE * 2);
        assert(v.Size() == SIZE);
        assert(Obj5::GetAliveObj5ectCount() == SIZE);
    }
}

void Test15() {
    const size_t MEDIUM_SIZE = 100;
    const size_t LARGE_SIZE = 250;
    const int ID = 42;
    {
        Obj5::ResetCounters();
        Vector<int> v(MEDIUM_SIZE);
        {
            auto v_copy(std::move(v));

            assert(v_copy.Size() == MEDIUM_SIZE);
            assert(v_copy.Capacity() == MEDIUM_SIZE);
        }
        assert(Obj5::GetAliveObj5ectCount() == 0);
    }
    {
        Obj5::ResetCounters();
        {
            Vector<Obj5> v(MEDIUM_SIZE);
            v[MEDIUM_SIZE / 2].id = ID;
            assert(Obj5::num_default_constructed == MEDIUM_SIZE);
            Vector<Obj5> moved_from_v(std::move(v));
            assert(moved_from_v.Size() == MEDIUM_SIZE);
            assert(moved_from_v[MEDIUM_SIZE / 2].id == ID);
        }
        assert(Obj5::GetAliveObj5ectCount() == 0);

        assert(Obj5::num_moved == 0);
        assert(Obj5::num_copied == 0);
        assert(Obj5::num_default_constructed == MEDIUM_SIZE);
    }
    {
        Obj5::ResetCounters();
        Vector<Obj5> v_medium(MEDIUM_SIZE);
        v_medium[MEDIUM_SIZE / 2].id = ID;
        Vector<Obj5> v_large(LARGE_SIZE);
        v_large = v_medium;
        assert(v_large.Size() == MEDIUM_SIZE);
        assert(v_large.Capacity() == LARGE_SIZE);
        assert(v_large[MEDIUM_SIZE / 2].id == ID);
        assert(Obj5::GetAliveObj5ectCount() == MEDIUM_SIZE + MEDIUM_SIZE);
    }
    {
        Obj5::ResetCounters();
        Vector<Obj5> v(MEDIUM_SIZE);
        {
            Vector<Obj5> v_large(LARGE_SIZE);
            v_large[LARGE_SIZE - 1].id = ID;
            v = v_large;
            assert(v.Size() == LARGE_SIZE);
            assert(v_large.Capacity() == LARGE_SIZE);
            assert(v_large[LARGE_SIZE - 1].id == ID);
            assert(Obj5::GetAliveObj5ectCount() == LARGE_SIZE + LARGE_SIZE);
        }
        assert(Obj5::GetAliveObj5ectCount() == LARGE_SIZE);
    }
    assert(Obj5::GetAliveObj5ectCount() == 0);
    {
        Obj5::ResetCounters();
        Vector<Obj5> v(MEDIUM_SIZE);
        v[MEDIUM_SIZE - 1].id = ID;
        Vector<Obj5> v_small(MEDIUM_SIZE / 2);
        v_small.Reserve(MEDIUM_SIZE + 1);
        const size_t num_copies = Obj5::num_copied;
        v_small = v;
        assert(v_small.Size() == v.Size());
        assert(v_small.Capacity() == MEDIUM_SIZE + 1);
        v_small[MEDIUM_SIZE - 1].id = ID;
        assert(Obj5::num_copied - num_copies == MEDIUM_SIZE - (MEDIUM_SIZE / 2));
    }
}

void Test16() {
    const size_t ID = 42;
    const size_t SIZE = 100'500;
    {
        Obj5::ResetCounters();
        Vector<Obj5> v;
        v.Resize(SIZE);
        assert(v.Size() == SIZE);
        assert(v.Capacity() == SIZE);
        assert(Obj5::num_default_constructed == SIZE);
    }
    assert(Obj5::GetAliveObj5ectCount() == 0);

    {
        const size_t NEW_SIZE = 10'000;
        Obj5::ResetCounters();
        Vector<Obj5> v(SIZE);
        v.Resize(NEW_SIZE);
        assert(v.Size() == NEW_SIZE);
        assert(v.Capacity() == SIZE);
        assert(Obj5::num_destroyed == SIZE - NEW_SIZE);
    }
    assert(Obj5::GetAliveObj5ectCount() == 0);
    {
        Obj5::ResetCounters();
        Vector<Obj5> v(SIZE);
        Obj5 o{ ID };
        v.PushBack(o);
        assert(v.Size() == SIZE + 1);
        assert(v.Capacity() == SIZE * 2);
        assert(v[SIZE].id == ID);
        assert(Obj5::num_default_constructed == SIZE);
        assert(Obj5::num_copied == 1);
        assert(Obj5::num_constructed_with_id == 1);
        assert(Obj5::num_moved == SIZE);
    }
    assert(Obj5::GetAliveObj5ectCount() == 0);
    {
        Obj5::ResetCounters();
        Vector<Obj5> v(SIZE);
        v.PushBack(Obj5{ ID });
        assert(v.Size() == SIZE + 1);
        assert(v.Capacity() == SIZE * 2);
        assert(v[SIZE].id == ID);
        assert(Obj5::num_default_constructed == SIZE);
        assert(Obj5::num_copied == 0);
        assert(Obj5::num_constructed_with_id == 1);
        assert(Obj5::num_moved == SIZE + 1);
    }
    {
        Obj5::ResetCounters();
        Vector<Obj5> v;
        v.PushBack(Obj5{ ID });
        v.PopBack();
        assert(v.Size() == 0);
        assert(v.Capacity() == 1);
        assert(Obj5::GetAliveObj5ectCount() == 0);
    }

    {
        Vector<TestObj5> v(1);
        assert(v.Size() == v.Capacity());
        // Операция PushBack существующего элемента вектора должна быть безопасна
        // даже при реаллокации памии
        v.PushBack(v[0]);
        assert(v[0].IsAlive());
        assert(v[1].IsAlive());
    }
    {
        Vector<TestObj5> v(1);
        assert(v.Size() == v.Capacity());
        // Операция PushBack для перемещения существующего элемента вектора должна быть безопасна
        // даже при реаллокации памяти
        v.PushBack(std::move(v[0]));
        assert(v[0].IsAlive());
        assert(v[1].IsAlive());
    }
}

void Test17() {
    const int ID = 42;
    using namespace std::literals;
    {
        Obj5::ResetCounters();
        Vector<Obj5> v;
        auto& elem = v.EmplaceBack(ID, "Ivan"s);
        assert(v.Capacity() == 1);
        assert(v.Size() == 1);
        assert(&elem == &v[0]);
        assert(v[0].id == ID);
        assert(v[0].name == "Ivan"s);
        assert(Obj5::num_constructed_with_id_and_name == 1);
        assert(Obj5::GetAliveObj5ectCount() == 1);
    }
    assert(Obj5::GetAliveObj5ectCount() == 0);
    {
        Vector<TestObj5> v(1);
        assert(v.Size() == v.Capacity());
        // Операция EmplaceBack существующего элемента вектора должна быть безопасна
        // даже при реаллокации памяти
        v.EmplaceBack(v[0]);
        assert(v[0].IsAlive());
        assert(v[1].IsAlive());
    }
}

namespace {

    // "Магическое" число, используемое для отслеживания живости объекта
    //inline const uint32_t DEFAULT_COOKIE = 0xdeadbeef;

    struct TestObj6 {
        TestObj6() = default;
        TestObj6(const TestObj6& other) = default;
        TestObj6& operator=(const TestObj6& other) = default;
        TestObj6(TestObj6&& other) = default;
        TestObj6& operator=(TestObj6&& other) = default;
        ~TestObj6() {
            cookie = 0;
        }
        [[nodiscard]] bool IsAlive() const noexcept {
            return cookie == DEFAULT_COOKIE;
        }
        uint32_t cookie = DEFAULT_COOKIE;
    };

    struct Obj6 {
        Obj6() {
            if (default_construction_throw_countdown > 0) {
                if (--default_construction_throw_countdown == 0) {
                    throw std::runtime_error("Oops");
                }
            }
            ++num_default_constructed;
        }

        explicit Obj6(int id)
            : id(id)  //
        {
            ++num_constructed_with_id;
        }

        Obj6(int id, std::string name)
            : id(id)
            , name(std::move(name))  //
        {
            ++num_constructed_with_id_and_name;
        }

        Obj6(const Obj6& other)
            : id(other.id)  //
        {
            if (other.throw_on_copy) {
                throw std::runtime_error("Oops");
            }
            ++num_copied;
        }

        Obj6(Obj6&& other) noexcept
            : id(other.id)  //
        {
            ++num_moved;
        }

        Obj6& operator=(const Obj6& other) {
            if (this != &other) {
                id = other.id;
                name = other.name;
                ++num_assigned;
            }
            return *this;
        }

        Obj6& operator=(Obj6&& other) noexcept {
            id = other.id;
            name = std::move(other.name);
            ++num_move_assigned;
            return *this;
        }

        ~Obj6() {
            ++num_destroyed;
            id = 0;
        }

        static int GetAliveObj6ectCount() {
            return num_default_constructed + num_copied + num_moved + num_constructed_with_id
                + num_constructed_with_id_and_name - num_destroyed;
        }

        static void ResetCounters() {
            default_construction_throw_countdown = 0;
            num_default_constructed = 0;
            num_copied = 0;
            num_moved = 0;
            num_destroyed = 0;
            num_constructed_with_id = 0;
            num_constructed_with_id_and_name = 0;
            num_assigned = 0;
            num_move_assigned = 0;
        }

        bool throw_on_copy = false;
        int id = 0;
        std::string name;

        static inline int default_construction_throw_countdown = 0;
        static inline int num_default_constructed = 0;
        static inline int num_constructed_with_id = 0;
        static inline int num_constructed_with_id_and_name = 0;
        static inline int num_copied = 0;
        static inline int num_moved = 0;
        static inline int num_destroyed = 0;
        static inline int num_assigned = 0;
        static inline int num_move_assigned = 0;
    };

}  // namespace


void Test18() {
    Obj6::ResetCounters();
    const size_t SIZE = 100500;
    const size_t INDEX = 10;
    const int MAGIC = 42;
    {
        Vector<int> v;
        assert(v.Capacity() == 0);
        assert(v.Size() == 0);

        v.Reserve(SIZE);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == 0);
    }
    {
        Vector<int> v(SIZE);
        const auto& cv(v);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == SIZE);
        assert(v[0] == 0);
        assert(&v[0] == &cv[0]);
        v[INDEX] = MAGIC;
        assert(v[INDEX] == MAGIC);
        assert(&v[100] - &v[0] == 100);

        v.Reserve(SIZE * 2);
        assert(v.Size() == SIZE);
        assert(v.Capacity() == SIZE * 2);
        assert(v[INDEX] == MAGIC);
    }
    {
        Vector<int> v(SIZE);
        v[INDEX] = MAGIC;
        const auto v_copy(v);
        assert(&v[INDEX] != &v_copy[INDEX]);
        assert(v[INDEX] == v_copy[INDEX]);
    }
    {
        Vector<Obj6> v;
        v.Reserve(SIZE);
        assert(Obj6::GetAliveObj6ectCount() == 0);
    }
    {
        Vector<Obj6> v(SIZE);
        assert(Obj6::GetAliveObj6ectCount() == SIZE);
        const int old_copy_count = Obj6::num_copied;
        const int old_move_count = Obj6::num_moved;
        v.Reserve(SIZE * 2);
        assert(Obj6::GetAliveObj6ectCount() == SIZE);
        assert(Obj6::num_copied == old_copy_count);
        assert(Obj6::num_moved == old_move_count + static_cast<int>(SIZE));
    }
    assert(Obj6::GetAliveObj6ectCount() == 0);
}

void Test19() {
    const size_t SIZE = 100;
    Obj6::ResetCounters();
    {
        Obj6::default_construction_throw_countdown = SIZE / 2;
        try {
            Vector<Obj6> v(SIZE);
            assert(false && "Exception is expected");
        }
        catch (const std::runtime_error&) {
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj6::num_default_constructed == SIZE / 2 - 1);
        assert(Obj6::GetAliveObj6ectCount() == 0);
    }
    Obj6::ResetCounters();
    {
        Vector<Obj6> v(SIZE);
        try {
            v[SIZE / 2].throw_on_copy = true;
            Vector<Obj6> v_copy(v);
            assert(false && "Exception is expected");
        }
        catch (const std::runtime_error&) {
            assert(Obj6::num_copied == SIZE / 2);
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj6::GetAliveObj6ectCount() == SIZE);
    }
    Obj6::ResetCounters();
    {
        Vector<Obj6> v(SIZE);
        try {
            v[SIZE - 1].throw_on_copy = true;
            v.Reserve(SIZE * 2);
        }
        catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(v.Capacity() == SIZE * 2);
        assert(v.Size() == SIZE);
        assert(Obj6::GetAliveObj6ectCount() == SIZE);
    }
}

void Test20() {
    const size_t MEDIUM_SIZE = 100;
    const size_t LARGE_SIZE = 250;
    const int ID = 42;
    {
        Obj6::ResetCounters();
        Vector<int> v(MEDIUM_SIZE);
        {
            auto v_copy(std::move(v));

            assert(v_copy.Size() == MEDIUM_SIZE);
            assert(v_copy.Capacity() == MEDIUM_SIZE);
        }
        assert(Obj6::GetAliveObj6ectCount() == 0);
    }
    {
        Obj6::ResetCounters();
        {
            Vector<Obj6> v(MEDIUM_SIZE);
            v[MEDIUM_SIZE / 2].id = ID;
            assert(Obj6::num_default_constructed == MEDIUM_SIZE);
            Vector<Obj6> moved_from_v(std::move(v));
            assert(moved_from_v.Size() == MEDIUM_SIZE);
            assert(moved_from_v[MEDIUM_SIZE / 2].id == ID);
        }
        assert(Obj6::GetAliveObj6ectCount() == 0);

        assert(Obj6::num_moved == 0);
        assert(Obj6::num_copied == 0);
        assert(Obj6::num_default_constructed == MEDIUM_SIZE);
    }
    {
        Obj6::ResetCounters();
        Vector<Obj6> v_medium(MEDIUM_SIZE);
        v_medium[MEDIUM_SIZE / 2].id = ID;
        Vector<Obj6> v_large(LARGE_SIZE);
        v_large = v_medium;
        assert(v_large.Size() == MEDIUM_SIZE);
        assert(v_large.Capacity() == LARGE_SIZE);
        assert(v_large[MEDIUM_SIZE / 2].id == ID);
        assert(Obj6::GetAliveObj6ectCount() == MEDIUM_SIZE + MEDIUM_SIZE);
    }
    {
        Obj6::ResetCounters();
        Vector<Obj6> v(MEDIUM_SIZE);
        {
            Vector<Obj6> v_large(LARGE_SIZE);
            v_large[LARGE_SIZE - 1].id = ID;
            v = v_large;
            assert(v.Size() == LARGE_SIZE);
            assert(v_large.Capacity() == LARGE_SIZE);
            assert(v_large[LARGE_SIZE - 1].id == ID);
            assert(Obj6::GetAliveObj6ectCount() == LARGE_SIZE + LARGE_SIZE);
        }
        assert(Obj6::GetAliveObj6ectCount() == LARGE_SIZE);
    }
    assert(Obj6::GetAliveObj6ectCount() == 0);
    {
        Obj6::ResetCounters();
        Vector<Obj6> v(MEDIUM_SIZE);
        v[MEDIUM_SIZE - 1].id = ID;
        Vector<Obj6> v_small(MEDIUM_SIZE / 2);
        v_small.Reserve(MEDIUM_SIZE + 1);
        const size_t num_copies = Obj6::num_copied;
        v_small = v;
        assert(v_small.Size() == v.Size());
        assert(v_small.Capacity() == MEDIUM_SIZE + 1);
        v_small[MEDIUM_SIZE - 1].id = ID;
        assert(Obj6::num_copied - num_copies == MEDIUM_SIZE - (MEDIUM_SIZE / 2));
    }
}

void Test21() {
    const size_t ID = 42;
    const size_t SIZE = 100'500;
    {
        Obj6::ResetCounters();
        Vector<Obj6> v;
        v.Resize(SIZE);
        assert(v.Size() == SIZE);
        assert(v.Capacity() == SIZE);
        assert(Obj6::num_default_constructed == SIZE);
    }
    assert(Obj6::GetAliveObj6ectCount() == 0);

    {
        const size_t NEW_SIZE = 10'000;
        Obj6::ResetCounters();
        Vector<Obj6> v(SIZE);
        v.Resize(NEW_SIZE);
        assert(v.Size() == NEW_SIZE);
        assert(v.Capacity() == SIZE);
        assert(Obj6::num_destroyed == SIZE - NEW_SIZE);
    }
    assert(Obj6::GetAliveObj6ectCount() == 0);
    {
        Obj6::ResetCounters();
        Vector<Obj6> v(SIZE);
        Obj6 o{ ID };
        v.PushBack(o);
        assert(v.Size() == SIZE + 1);
        assert(v.Capacity() == SIZE * 2);
        assert(v[SIZE].id == ID);
        assert(Obj6::num_default_constructed == SIZE);
        assert(Obj6::num_copied == 1);
        assert(Obj6::num_constructed_with_id == 1);
        assert(Obj6::num_moved == SIZE);
    }
    assert(Obj6::GetAliveObj6ectCount() == 0);
    {
        Obj6::ResetCounters();
        Vector<Obj6> v(SIZE);
        v.PushBack(Obj6{ ID });
        assert(v.Size() == SIZE + 1);
        assert(v.Capacity() == SIZE * 2);
        assert(v[SIZE].id == ID);
        assert(Obj6::num_default_constructed == SIZE);
        assert(Obj6::num_copied == 0);
        assert(Obj6::num_constructed_with_id == 1);
        assert(Obj6::num_moved == SIZE + 1);
    }
    {
        Obj6::ResetCounters();
        Vector<Obj6> v;
        v.PushBack(Obj6{ ID });
        v.PopBack();
        assert(v.Size() == 0);
        assert(v.Capacity() == 1);
        assert(Obj6::GetAliveObj6ectCount() == 0);
    }

    {
        Vector<TestObj6> v(1);
        assert(v.Size() == v.Capacity());
        // Операция PushBack существующего элемента вектора должна быть безопасна
        // даже при реаллокации памяти
        v.PushBack(v[0]);
        assert(v[0].IsAlive());
        assert(v[1].IsAlive());
    }
    {
        Vector<TestObj6> v(1);
        assert(v.Size() == v.Capacity());
        // Операция PushBack для перемещения существующего элемента вектора должна быть безопасна
        // даже при реаллокации памяти
        v.PushBack(std::move(v[0]));
        assert(v[0].IsAlive());
        assert(v[1].IsAlive());
    }
}

void Test22() {
    const int ID = 42;
    using namespace std::literals;
    {
        Obj6::ResetCounters();
        Vector<Obj6> v;
        auto& elem = v.EmplaceBack(ID, "Ivan"s);
        assert(v.Capacity() == 1);
        assert(v.Size() == 1);
        assert(&elem == &v[0]);
        assert(v[0].id == ID);
        assert(v[0].name == "Ivan"s);
        assert(Obj6::num_constructed_with_id_and_name == 1);
        assert(Obj6::GetAliveObj6ectCount() == 1);
    }
    assert(Obj6::GetAliveObj6ectCount() == 0);
    {
        Vector<TestObj6> v(1);
        assert(v.Size() == v.Capacity());
        // Операция EmplaceBack существующего элемента вектора должна быть безопасна
        // даже при реаллокации памяти
        v.EmplaceBack(v[0]);
        assert(v[0].IsAlive());
        assert(v[1].IsAlive());
    }
}

void Test23() {
    using namespace std::literals;
    const size_t SIZE = 10;
    const int ID = 42;
    {
        Obj6::ResetCounters();
        Vector<int> v(SIZE);
        const auto& cv(v);
        v.PushBack(1);
        assert(&*v.begin() == &v[0]);
        *v.begin() = 2;
        assert(v[0] == 2);
        assert(v.end() - v.begin() == static_cast<std::ptrdiff_t>(v.Size()));
        assert(v.begin() == cv.begin());
        assert(v.end() == cv.end());
        assert(v.cbegin() == cv.begin());
        assert(v.cend() == cv.end());
    }
    {
        Obj6::ResetCounters();
        Vector<Obj6> v{ SIZE };
        Obj6 o{ 1 };
        Vector<Obj6>::iterator pos = v.Insert(v.cbegin() + 1, o);
        assert(v.Size() == SIZE + 1);
        assert(v.Capacity() == SIZE * 2);
        assert(&*pos == &v[1]);
        assert(v[1].id == o.id);
        assert(Obj6::num_copied == 1);
        assert(Obj6::num_default_constructed == SIZE);
        assert(Obj6::GetAliveObj6ectCount() == SIZE + 2);
    }
    {
        Obj6::ResetCounters();
        Vector<Obj6> v;
        auto* pos = v.Emplace(v.end(), Obj6{ 1 });
        assert(v.Size() == 1);
        assert(v.Capacity() >= v.Size());
        assert(&*pos == &v[0]);
        assert(Obj6::num_moved == 1);
        assert(Obj6::num_constructed_with_id == 1);
        assert(Obj6::num_copied == 0);
        assert(Obj6::num_assigned == 0);
        assert(Obj6::num_move_assigned == 0);
        assert(Obj6::GetAliveObj6ectCount() == 1);
    }
    {
        Obj6::ResetCounters();
        Vector<Obj6> v;
        v.Reserve(SIZE);
        auto* pos = v.Emplace(v.end(), Obj6{ 1 });
        assert(v.Size() == 1);
        assert(v.Capacity() >= v.Size());
        assert(&*pos == &v[0]);
        assert(Obj6::num_moved == 1);
        assert(Obj6::num_constructed_with_id == 1);
        assert(Obj6::num_copied == 0);
        assert(Obj6::num_assigned == 0);
        assert(Obj6::num_move_assigned == 0);
        int a = Obj6::GetAliveObj6ectCount();
        assert(Obj6::GetAliveObj6ectCount() == 1);
    }
    {
        Obj6::ResetCounters();
        Vector<Obj6> v{ SIZE };
        Vector<Obj6>::iterator pos = v.Insert(v.cbegin() + 1, Obj6{ 1 });
        assert(v.Size() == SIZE + 1);
        assert(v.Capacity() == SIZE * 2);
        assert(&*pos == &v[1]);
        assert(v[1].id == 1);
        assert(Obj6::num_copied == 0);
        assert(Obj6::num_default_constructed == SIZE);
        assert(Obj6::GetAliveObj6ectCount() == SIZE + 1);
    }
    {
        Vector<TestObj6> v{ SIZE };
        v.Insert(v.cbegin() + 2, v[0]);
        assert(std::all_of(v.begin(), v.end(), [](const TestObj6& Obj6) {
            return Obj6.IsAlive();
            }));
    }
    {
        Vector<TestObj6> v{ SIZE };
        v.Insert(v.cbegin() + 2, std::move(v[0]));
        assert(std::all_of(v.begin(), v.end(), [](const TestObj6& Obj6) {
            return Obj6.IsAlive();
            }));
    }
    {
        Vector<TestObj6> v{ SIZE };
        v.Emplace(v.cbegin() + 2, std::move(v[0]));
        assert(std::all_of(v.begin(), v.end(), [](const TestObj6& Obj6) {
            return Obj6.IsAlive();
            }));
    }
    {
        Obj6::ResetCounters();
        Vector<Obj6> v{ SIZE };
        auto* pos = v.Emplace(v.cbegin() + 1, ID, "Ivan"s);
        assert(v.Size() == SIZE + 1);
        assert(v.Capacity() == SIZE * 2);
        assert(&*pos == &v[1]);
        assert(v[1].id == ID);
        assert(v[1].name == "Ivan"s);
        assert(Obj6::num_copied == 0);
        assert(Obj6::num_default_constructed == SIZE);
        assert(Obj6::num_moved == SIZE);
        assert(Obj6::num_move_assigned == 0);
        assert(Obj6::num_assigned == 0);
        assert(Obj6::GetAliveObj6ectCount() == SIZE + 1);
    }
    {
        Obj6::ResetCounters();
        Vector<Obj6> v{ SIZE };
        auto* pos = v.Emplace(v.cbegin() + v.Size(), ID, "Ivan"s);
        assert(v.Size() == SIZE + 1);
        assert(v.Capacity() == SIZE * 2);
        assert(&*pos == &v[SIZE]);
        assert(v[SIZE].id == ID);
        assert(v[SIZE].name == "Ivan"s);
        assert(Obj6::num_copied == 0);
        assert(Obj6::num_default_constructed == SIZE);
        assert(Obj6::num_moved == SIZE);
        assert(Obj6::num_move_assigned == 0);
        assert(Obj6::num_assigned == 0);
        assert(Obj6::GetAliveObj6ectCount() == SIZE + 1);
    }
    {
        Obj6::ResetCounters();
        Vector<Obj6> v{ SIZE };
        v.Reserve(SIZE * 2);
        const int old_num_moved = Obj6::num_moved;
        assert(v.Capacity() == SIZE * 2);
        auto* pos = v.Emplace(v.cbegin() + 3, ID, "Ivan"s);
        assert(v.Size() == SIZE + 1);
        assert(&*pos == &v[3]);
        assert(v[3].id == ID);
        assert(v[3].name == "Ivan");
        assert(Obj6::num_copied == 0);
        assert(Obj6::num_default_constructed == SIZE);
        assert(Obj6::num_constructed_with_id_and_name == 1);
        assert(Obj6::num_moved == old_num_moved + 1);
        assert(Obj6::num_move_assigned == SIZE - 3);
        assert(Obj6::num_assigned == 0);
    }
    {
        Obj6::ResetCounters();
        Vector<Obj6> v{ SIZE };
        v[2].id = ID;
        auto* pos = v.Erase(v.cbegin() + 1);
        assert((pos - v.begin()) == 1);
        assert(v.Size() == SIZE - 1);
        assert(v.Capacity() == SIZE);
        assert(pos->id == ID);
        assert(Obj6::num_copied == 0);
        assert(Obj6::num_assigned == 0);
        assert(Obj6::num_move_assigned == SIZE - 2);
        assert(Obj6::num_moved == 0);
        assert(Obj6::GetAliveObj6ectCount() == SIZE - 1);
    }
}


struct C2 {
    C2() noexcept {
        ++def_ctor;
    }
    C2(const C2& /*other*/) noexcept {
        ++copy_ctor;
    }
    C2(C2&& /*other*/) noexcept {
        ++move_ctor;
    }
    C2& operator=(const C2& other) noexcept {
        if (this != &other) {
            ++copy_assign;
        }
        return *this;
    }
    C2& operator=(C2&& /*other*/) noexcept {
        ++move_assign;
        return *this;
    }
    ~C2() {
        ++dtor;
    }

    static void Reset() {
        def_ctor = 0;
        copy_ctor = 0;
        move_ctor = 0;
        copy_assign = 0;
        move_assign = 0;
        dtor = 0;
    }

    inline static size_t def_ctor = 0;
    inline static size_t copy_ctor = 0;
    inline static size_t move_ctor = 0;
    inline static size_t copy_assign = 0;
    inline static size_t move_assign = 0;
    inline static size_t dtor = 0;
};

void Dump() {
    using namespace std;
    cerr << "Def ctors: "sv << C2::def_ctor              //
        << ", Copy ctors: "sv << C2::copy_ctor          //
        << ", Move ctors: "sv << C2::move_ctor          //
        << ", Copy assignments: "sv << C2::copy_assign  //
        << ", Move assignments: "sv << C2::move_assign  //
        << ", Dtors: "sv << C2::dtor << endl;
}

void Benchmark() {
    using namespace std::string_view_literals;
    try {
        const size_t NUM = 10;
        C2 c;
        {
            std::cerr << "std::vector:"sv << std::endl;
            C2::Reset();
            std::vector<C2> v(NUM);
            Dump();
            v.push_back(c);
        }
        Dump();
    }
    catch (...) {
    }
    try {
        const size_t NUM = 10;
        C2 c;
        {
            std::cerr << "Vector:"sv << std::endl;
            C2::Reset();
            Vector<C2> v(NUM);
            Dump();
            v.PushBack(c);
        }
        Dump();
    }
    catch (...) {
    }
}

struct C3 {
    C3() noexcept {
        ++def_ctor;
    }
    C3(const C3& /*other*/) noexcept {
        ++copy_ctor;
    }
    C3(C3&& /*other*/) noexcept {
        ++move_ctor;
    }
    C3& operator=(const C3& other) noexcept {
        if (this != &other) {
            ++copy_assign;
        }
        return *this;
    }
    C3& operator=(C3&& /*other*/) noexcept {
        ++move_assign;
        return *this;
    }
    ~C3() {
        ++dtor;
    }

    void Update() const& {
        ++const_lvalue_call_count;
    }

    void Update()& {
        ++lvalue_call_count;
    }

    void Update()&& {
        ++rvalue_call_count;
    }

    static size_t InstanceCount() {
        return def_ctor + copy_ctor + move_ctor - dtor;
    }

    static void Reset() {
        def_ctor = 0;
        copy_ctor = 0;
        move_ctor = 0;
        copy_assign = 0;
        move_assign = 0;
        dtor = 0;
        lvalue_call_count = 0;
        rvalue_call_count = 0;
        const_lvalue_call_count = 0;
    }

    inline static size_t def_ctor = 0;
    inline static size_t copy_ctor = 0;
    inline static size_t move_ctor = 0;
    inline static size_t copy_assign = 0;
    inline static size_t move_assign = 0;
    inline static size_t dtor = 0;

    inline static size_t lvalue_call_count = 0;
    inline static size_t rvalue_call_count = 0;
    inline static size_t const_lvalue_call_count = 0;
};

void TestInitialization2() {
    C3::Reset();
    {
        Optional<C3> o;
        assert(!o.HasValue());
        assert(C3::InstanceCount() == 0);
    }
    assert(C3::InstanceCount() == 0);

    C3::Reset();
    {
        C3 c;
        Optional<C3> o(c);
        assert(o.HasValue());
        assert(C3::def_ctor == 1 && C3::copy_ctor == 1);
        assert(C3::InstanceCount() == 2);
    }
    assert(C3::InstanceCount() == 0);

    C3::Reset();
    {
        C3 c;
        Optional<C3> o(std::move(c));
        assert(o.HasValue());
        assert(C3::def_ctor == 1 && C3::move_ctor == 1 && C3::copy_ctor == 0 && C3::copy_assign == 0
            && C3::move_assign == 0);
        assert(C3::InstanceCount() == 2);
    }
    assert(C3::InstanceCount() == 0);

    C3::Reset();
    {
        C3 c;
        Optional<C3> o1(c);
        const Optional<C3> o2(o1);
        assert(o1.HasValue());
        assert(o2.HasValue());
        assert(C3::def_ctor == 1 && C3::move_ctor == 0 && C3::copy_ctor == 2 && C3::copy_assign == 0
            && C3::move_assign == 0);
        assert(C3::InstanceCount() == 3);
    }
    assert(C3::InstanceCount() == 0);

    C3::Reset();
    {
        C3 c;
        Optional<C3> o1(c);
        const Optional<C3> o2(std::move(o1));
        assert(C3::def_ctor == 1 && C3::copy_ctor == 1 && C3::move_ctor == 1 && C3::copy_assign == 0
            && C3::move_assign == 0);
        assert(C3::InstanceCount() == 3);
    }
    assert(C3::InstanceCount() == 0);
}

void TestAssignment2() {
    Optional<C3> o1;
    Optional<C3> o2;
    {  // Assign a value to empty
        C3::Reset();
        C3 c;
        o1 = c;
        assert(C3::def_ctor == 1 && C3::copy_ctor == 1 && C3::dtor == 0);
    }
    {  // Assign a non-empty to empty
        C3::Reset();
        o2 = o1;
        assert(C3::copy_ctor == 1 && C3::copy_assign == 0 && C3::dtor == 0);
    }
    {  // Assign non-empty to non-empty
        C3::Reset();
        o2 = o1;
        assert(C3::copy_ctor == 0 && C3::copy_assign == 1 && C3::dtor == 0);
    }
    {  // Assign empty to non-empty
        C3::Reset();
        Optional<C3> empty;
        o1 = empty;
        assert(C::copy_ctor == 0 && C3::dtor == 1);
        assert(!o1.HasValue());
    }
}

void TestMoveAssignment2() {
    {  // Assign a value to empty
        Optional<C3> o1;
        C3::Reset();
        C3 c;
        o1 = std::move(c);
        assert(C3::def_ctor == 1 && C3::move_ctor == 1 && C3::dtor == 0);
    }
    {  // Assign a non-empty to empty
        Optional<C3> o1;
        Optional<C3> o2{ C3{} };
        C3::Reset();
        o1 = std::move(o2);
        assert(C3::move_ctor == 1 && C3::move_assign == 0 && C3::dtor == 0);
    }
    {  // Assign non-empty to non-empty
        Optional<C3> o1{ C3{} };
        Optional<C3> o2{ C3{} };
        C3::Reset();
        o2 = std::move(o1);
        assert(C3::copy_ctor == 0 && C3::move_assign == 1 && C3::dtor == 0);
    }
    {  // Assign empty to non-empty
        Optional<C3> o1{ C3{} };
        C3::Reset();
        Optional<C3> empty;
        o1 = std::move(empty);
        assert(C3::copy_ctor == 0 && C3::move_ctor == 0 && C3::move_assign == 0 && C3::dtor == 1);
        assert(!o1.HasValue());
    }
}

void TestValueAccess2() {
    using namespace std::literals;
    {
        Optional<std::string> o;
        o = "hello"s;
        assert(o.HasValue());
        assert(o.Value() == "hello"s);
        assert(&*o == &o.Value());
        assert(o->length() == 5);
    }
    {
        try {
            Optional<int> o;
            [[maybe_unused]] int v = o.Value();
            assert(false);
        }
        catch (const BadOptionalAccess& /*e*/) {
        }
        catch (...) {
            assert(false);
        }
    }
}

void TestReset2() {
    C3::Reset();
    {
        Optional<C3> o{ C3() };
        assert(o.HasValue());
        o.Reset();
        assert(!o.HasValue());
    }
}

void TestEmplace2() {
    struct S {
        S(int i, std::unique_ptr<int>&& p)
            : i(i)
            , p(std::move(p))  //
        {
        }
        int i;
        std::unique_ptr<int> p;
    };

    Optional<S> o;
    o.Emplace(1, std::make_unique<int>(2));
    assert(o.HasValue());
    assert(o->i == 1);
    assert(*(o->p) == 2);

    o.Emplace(3, std::make_unique<int>(4));
    assert(o.HasValue());
    assert(o->i == 3);
    assert(*(o->p) == 4);
}

void TestRefQualifiedMethodOverloading2() {
    {
        C3::Reset();
        C3 val = *Optional<C3>(C3{});
        assert(C3::copy_ctor == 0);
        assert(C3::move_ctor == 2);
        assert(C3::def_ctor == 1);
        assert(C3::copy_assign == 0);
        assert(C3::move_assign == 0);
    }
    {
        C3::Reset();
        C3 val = Optional<C3>(C3{}).Value();
        assert(C3::copy_ctor == 0);
        assert(C3::move_ctor == 2);
        assert(C3::def_ctor == 1);
        assert(C3::copy_assign == 0);
        assert(C3::move_assign == 0);
    }
    {
        C3::Reset();
        Optional<C3> opt(C3{});
        (*opt).Update();
        assert(C3::lvalue_call_count == 1);
        assert(C3::rvalue_call_count == 0);
        (*std::move(opt)).Update();
        assert(C3::lvalue_call_count == 1);
        assert(C3::rvalue_call_count == 1);
    }
    {
        C3::Reset();
        const Optional<C3> opt(C3{});
        (*opt).Update();
        assert(C3::const_lvalue_call_count == 1);
    }
    {
        C3::Reset();
        Optional<C3> opt(C3{});
        opt.Value().Update();
        assert(C3::lvalue_call_count == 1);
        assert(C3::rvalue_call_count == 0);
        std::move(opt).Value().Update();
        assert(C3::lvalue_call_count == 1);
    }
    {
        C3::Reset();
        const Optional<C3> opt(C3{});
        opt.Value().Update();
        assert(C3::const_lvalue_call_count == 1);
    }
}