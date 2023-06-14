/*

// old version

#pragma once
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <numeric>

using namespace std;

// Ó ôóíêöèè, îáúÿâëåííîé ñî ñïåöèôèêàòîðîì inline, ìîæåò áûòü íåñêîëüêî
// èäåíòè÷íûõ îïðåäåëåíèé â ðàçíûõ åäèíèöàõ òðàíñëÿöèè.
// Îáû÷íî inline ïîìå÷àþò ôóíêöèè, ÷ü¸ òåëî íàõîäèòñÿ â çàãîëîâî÷íîì ôàéëå,
// ÷òîáû ïðè ïîäêëþ÷åíèè ýòîãî ôàéëà èç ðàçíûõ åäèíèö òðàíñëÿöèè íå âîçíèêàëî îøèáîê êîìïîíîâêè
inline void Test1() {
    // Èíèöèàëèçàöèÿ êîíñòðóêòîðîì ïî óìîë÷àíèþ
    {
        SimpleVector<int> v;
        assert(v.GetSize() == 0u);
        assert(v.IsEmpty());
        assert(v.GetCapacity() == 0u);
    }
    
    // Èíèöèàëèçàöèÿ âåêòîðà óêàçàííîãî ðàçìåðà
    {
        SimpleVector<int> v(5);
        assert(v.GetSize() == 5u);
        assert(v.GetCapacity() == 5u);
        assert(!v.IsEmpty());
        for (size_t i = 0; i < v.GetSize(); ++i) {
            assert(v[i] == 0);
        }
    }
    
    // Èíèöèàëèçàöèÿ âåêòîðà, çàïîëíåííîãî çàäàííûì çíà÷åíèåì
    {
        SimpleVector<int> v(3, 42);
        assert(v.GetSize() == 3);
        assert(v.GetCapacity() == 3);
        for (size_t i = 0; i < v.GetSize(); ++i) {
            assert(v[i] == 42);
        }
    }
    
    // Èíèöèàëèçàöèÿ âåêòîðà ïðè ïîìîùè initializer_list
    {
        SimpleVector<int> v{ 1, 2, 3 };
        assert(v.GetSize() == 3);
        assert(v.GetCapacity() == 3);
        assert(v[2] == 3);
    }
    
    // Äîñòóï ê ýëåìåíòàì ïðè ïîìîùè At
    {
        SimpleVector<int> v(3);
        assert(&v.At(2) == &v[2]);
        try {
            v.At(3);
            assert(false);  // Îæèäàåòñÿ âûáðàñûâàíèå èñêëþ÷åíèÿ
        }
        catch (const std::out_of_range&) {
        }
        catch (...) {
            assert(false);  // Íå îæèäàåòñÿ èñêëþ÷åíèå, îòëè÷íîå îò out_of_range
        }
    }
    
    // Î÷èñòêà âåêòîðà
    {
        SimpleVector<int> v(10);
        const size_t old_capacity = v.GetCapacity();
        v.Clear();
        assert(v.GetSize() == 0);
        assert(v.GetCapacity() == old_capacity);
    }
    
    // Èçìåíåíèå ðàçìåðà
    {
        SimpleVector<int> v(3);
        v[2] = 17;
        v.Resize(7);
        assert(v.GetSize() == 7);
        assert(v.GetCapacity() >= v.GetSize());
        assert(v[2] == 17);
        assert(v[3] == 0);
    }
    {
        SimpleVector<int> v(3);
        v[0] = 42;
        v[1] = 55;
        const size_t old_capacity = v.GetCapacity();
        v.Resize(2);
        assert(v.GetSize() == 2);
        assert(v.GetCapacity() == old_capacity);
        assert(v[0] == 42);
        assert(v[1] == 55);
    }
    {
        const size_t old_size = 3;
        SimpleVector<int> v(3);
        v.Resize(old_size + 5);
        v[3] = 42;
        v.Resize(old_size);
        v.Resize(old_size + 2);
        assert(v[3] == 0);
    }
    
    // Èòåðèðîâàíèå ïî SimpleVector
    {
        // Ïóñòîé âåêòîð
        {
            SimpleVector<int> v;
            assert(v.begin() == nullptr);
            assert(v.end() == nullptr);
        }

        // Íåïóñòîé âåêòîð
        {
            SimpleVector<int> v(10, 42);
            assert(v.begin());
            assert(*v.begin() == 42);
            assert(v.end() == v.begin() + v.GetSize());
        }
    }
}

inline void Test2() {
    // PushBack
     {
        SimpleVector<int> v(1);
        v.PushBack(42);
        assert(v.GetSize() == 2);
        assert(v.GetCapacity() >= v.GetSize());
        assert(v[0] == 0);
        assert(v[1] == 42);
    }

    // Åñëè õâàòàåò ìåñòà, PushBack íå óâåëè÷èâàåò Capacity
    {
        SimpleVector<int> v(2);
        v.Resize(1);
        const size_t old_capacity = v.GetCapacity();
        v.PushBack(123);
        assert(v.GetSize() == 2);
        assert(v.GetCapacity() == old_capacity);
    }
    
    // PopBack
    {
        SimpleVector<int> v{ 0, 1, 2, 3 };
        const size_t old_capacity = v.GetCapacity();
        const auto old_begin = v.begin();
        v.PopBack();
        assert(v.GetCapacity() == old_capacity);
        assert(v.begin() == old_begin);
        assert((v == SimpleVector<int>{0, 1, 2}));
    }
    
    // Êîíñòðóêòîð êîïèðîâàíèÿ
    {
        SimpleVector<int> numbers{ 1, 2 };
        auto numbers_copy(numbers);
        assert(&numbers_copy[0] != &numbers[0]);
        assert(numbers_copy.GetSize() == numbers.GetSize());
        for (size_t i = 0; i < numbers.GetSize(); ++i) {
            assert(numbers_copy[i] == numbers[i]);
            assert(&numbers_copy[i] != &numbers[i]);
        }
    }
    
    // Ñðàâíåíèå
    {
        assert((SimpleVector{ 1, 2, 3 } == SimpleVector{ 1, 2, 3 }));
        assert((SimpleVector{ 1, 2, 3 } != SimpleVector{ 1, 2, 2 }));

        assert((SimpleVector{ 1, 2, 3 } < SimpleVector{ 1, 2, 3, 1 }));
        assert((SimpleVector{ 1, 2, 3 } > SimpleVector{ 1, 2, 2, 1 }));

        assert((SimpleVector{ 1, 2, 3 } >= SimpleVector{ 1, 2, 3 }));
        assert((SimpleVector{ 1, 2, 4 } >= SimpleVector{ 1, 2, 3 }));
        assert((SimpleVector{ 1, 2, 3 } <= SimpleVector{ 1, 2, 3 }));
        assert((SimpleVector{ 1, 2, 3 } <= SimpleVector{ 1, 2, 4 }));
    }
    
    // Îáìåí çíà÷åíèé âåêòîðîâ
    {
        SimpleVector<int> v1{ 42, 666 };
        SimpleVector<int> v2;
        v2.PushBack(0);
        v2.PushBack(1);
        v2.PushBack(2);
        const int* const begin1 = &v1[0];
        const int* const begin2 = &v2[0];

        const size_t capacity1 = v1.GetCapacity();
        const size_t capacity2 = v2.GetCapacity();

        const size_t size1 = v1.GetSize();
        const size_t size2 = v2.GetSize();

        static_assert(noexcept(v1.swap(v2)));
        v1.swap(v2);
        assert(&v2[0] == begin1);
        assert(&v1[0] == begin2);
        assert(v1.GetSize() == size2);
        assert(v2.GetSize() == size1);
        assert(v1.GetCapacity() == capacity2);
        assert(v2.GetCapacity() == capacity1);
    }
    
    // Ïðèñâàèâàíèå
    {
        SimpleVector<int> src_vector{ 1, 2, 3, 4 };
        SimpleVector<int> dst_vector{ 1, 2, 3, 4, 5, 6 };
        dst_vector = src_vector;
        assert(dst_vector == src_vector);
    }
    
    // Âñòàâêà ýëåìåíòîâ
    {
        SimpleVector<int> v{ 1, 2, 3, 4 };
        v.Insert(v.begin() + 2, 42);
        assert((v == SimpleVector<int>{1, 2, 42, 3, 4}));
    }
    
    // Óäàëåíèå ýëåìåíòîâ
    {
        SimpleVector<int> v{ 1, 2, 3, 4 };
        v.Erase(v.cbegin() + 2);
        assert((v == SimpleVector<int>{1, 2, 4}));
    }
    {
        SimpleVector<int> v;
        assert(v.Insert(v.end(), -10) == v.end() - 1);
        v.Clear();
        v.Insert(v.end(), -1);
        assert(v.Insert(v.end(), -1) == v.end() - 1);
    }
}

void TestReserveConstructor() {
    cout << "TestReserveConstructor"s << endl;
    SimpleVector<int> v(Reserve(5));
    assert(v.GetCapacity() == 5);
    assert(v.IsEmpty());
    cout << "Done!"s << endl;
}

void TestReserveMethod() {
    cout << "TestReserveMethod"s << endl;
    SimpleVector<int> v;
    // çàðåçåðâèðóåì 5 ìåñò â âåêòîðå
    v.Reserve(5);
    assert(v.GetCapacity() == 5);
    assert(v.IsEmpty());

    // ïîïûòàåìñÿ óìåíüøèòü capacity äî 1
    v.Reserve(1);
    // capacity äîëæíî îñòàòüñÿ ïðåæíèì
    assert(v.GetCapacity() == 5);
    // ïîìåñòèì 10 ýëåìåíòîâ â âåêòîð
    for (int i = 0; i < 10; ++i) {
        v.PushBack(i);
    }
    assert(v.GetSize() == 10);
    // óâåëè÷èì capacity äî 100
    v.Reserve(100);
    // ïðîâåðèì, ÷òî ðàçìåð íå ïîìåíÿëñÿ
    assert(v.GetSize() == 10);
    assert(v.GetCapacity() == 100);
    // ïðîâåðèì, ÷òî ýëåìåíòû íà ìåñòå
    for (int i = 0; i < 10; ++i) {
        assert(v[i] == i);
    }
    cout << "Done!"s << endl;
}

struct NoncopyableInt {
    int value;

    NoncopyableInt(const NoncopyableInt&) = delete;
    NoncopyableInt& operator=(const NoncopyableInt&) = delete;

    NoncopyableInt(NoncopyableInt&&) = default;
    NoncopyableInt& operator=(NoncopyableInt&&) = default;
};

void TestUncopyables() {
    cout << "TestUncopyables"s << endl;
    {
        SimpleVector<NoncopyableInt> a;
    }
    {
        SimpleVector<NoncopyableInt> a(5);
    }
    {
        NoncopyableInt i{ 1 };
        //SimpleVector<NoncopyableInt> a(5, i);                 // ERROR: vector of five non-copyable objects
    }
    cout << "Done!"s << endl;
}

class X {
public:
    X()
        : X(5) {
    }
    X(size_t num)
        : x_(num) {
    }
    X(const X& other) = delete;
    X& operator=(const X& other) = delete;
    X(X&& other) {
        x_ = exchange(other.x_, 0);
    }
    X& operator=(X&& other) {
        x_ = exchange(other.x_, 0);
        return *this;
    }
    size_t GetX() const {
        return x_;
    }

private:
    size_t x_;
};

SimpleVector<int> GenerateVector(size_t size) {
    SimpleVector<int> v(size);
    iota(v.begin(), v.end(), 1);
    return v;
}

void TestTemporaryObjConstructor() {
    const size_t size = 1000000;
    cout << "Test with temporary object, copy elision" << endl;
    SimpleVector<int> moved_vector(GenerateVector(size));
    assert(moved_vector.GetSize() == size);
    cout << "Done!" << endl << endl;
}

void TestTemporaryObjOperator() {
    const size_t size = 1000000;
    cout << "Test with temporary object, operator=" << endl;
    SimpleVector<int> moved_vector;
    assert(moved_vector.GetSize() == 0);
    moved_vector = GenerateVector(size);
    assert(moved_vector.GetSize() == size);
    cout << "Done!" << endl << endl;
}

void TestNamedMoveConstructor() {
    const size_t size = 1000000;
    cout << "Test with named object, move constructor" << endl;
    SimpleVector<int> vector_to_move(GenerateVector(size));
    assert(vector_to_move.GetSize() == size);

    SimpleVector<int> moved_vector(move(vector_to_move));
    assert(moved_vector.GetSize() == size);
    assert(vector_to_move.GetSize() == 0);
    cout << "Done!" << endl << endl;
}

void TestNamedMoveOperator() {
    const size_t size = 1000000;
    cout << "Test with named object, operator=" << endl;
    SimpleVector<int> vector_to_move(GenerateVector(size));
    assert(vector_to_move.GetSize() == size);

    SimpleVector<int> moved_vector = move(vector_to_move);
    assert(moved_vector.GetSize() == size);
    assert(vector_to_move.GetSize() == 0);
    cout << "Done!" << endl << endl;
}

void TestNoncopiableMoveConstructor() {
    const size_t size = 5;
    cout << "Test noncopiable object, move constructor" << endl;
    SimpleVector<X> vector_to_move;
    for (size_t i = 0; i < size; ++i) {
        vector_to_move.PushBack(X(i));
    }

    SimpleVector<X> moved_vector = move(vector_to_move);
    assert(moved_vector.GetSize() == size);
    assert(vector_to_move.GetSize() == 0);

    for (size_t i = 0; i < size; ++i) {
        assert(moved_vector[i].GetX() == i);
    }
    cout << "Done!" << endl << endl;
}

void TestNoncopiablePushBack() {
    const size_t size = 5;
    cout << "Test noncopiable push back" << endl;
    SimpleVector<X> v;
    for (size_t i = 0; i < size; ++i) {
        v.PushBack(X(i));
    }

    assert(v.GetSize() == size);

    for (size_t i = 0; i < size; ++i) {
        assert(v[i].GetX() == i);
    }
    cout << "Done!" << endl << endl;
}

void TestNoncopiableInsert() {
    const size_t size = 5;
    cout << "Test noncopiable insert" << endl;
    SimpleVector<X> v;
    for (size_t i = 0; i < size; ++i) {
        v.PushBack(X(i));
    }

    // â íà÷àëî
    v.Insert(v.begin(), X(size + 1));
    assert(v.GetSize() == size + 1);
    assert(v.begin()->GetX() == size + 1);
    // â êîíåö
    v.Insert(v.end(), X(size + 2));
    assert(v.GetSize() == size + 2);
    assert((v.end() - 1)->GetX() == size + 2);
    // â ñåðåäèíó
    v.Insert(v.begin() + 3, X(size + 3));
    assert(v.GetSize() == size + 3);
    assert((v.begin() + 3)->GetX() == size + 3);
    cout << "Done!" << endl << endl;
}

void TestNoncopiableErase() {
    const size_t size = 3;
    cout << "Test noncopiable erase" << endl;
    SimpleVector<X> v;
    for (size_t i = 0; i < size; ++i) {
        v.PushBack(X(i));
    }

    auto it = v.Erase(v.begin());
    assert(it->GetX() == 1);
    cout << "Done!" << endl << endl;
}
*/
