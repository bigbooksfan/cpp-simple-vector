#pragma once
/*

// old version

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>

#include "array_ptr.h"

class ReserveProxyObj {
    size_t to_reserve_ = 0;

public:
    ReserveProxyObj(size_t size_to_reserve) {
        to_reserve_ = size_to_reserve;
    }
    size_t GetSize() {
        return to_reserve_;
    }
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {

    size_t size_ = 0;
    size_t capacity_ = 0;

    ArrayPtr<Type> content_;

public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) : size_(size), capacity_(size) {
        ArrayPtr<Type> tmp(size);
        content_.swap(tmp);
        //ArrayPtr<Type> content_(size);        // Don't work
    }

    SimpleVector(size_t size, const Type& value) {
        ArrayPtr<Type> tmp(size);
        //Type* It = tmp.begin();
        //for (size_t i = 0; i < size; ++i) {
        //    *It = value;
        //    ++It;
        //}
        std::fill(tmp.begin(), tmp.end(), value);
        content_.swap(tmp);
        size_ = size;
        capacity_ = size;
    }
    
    SimpleVector(SimpleVector&& other) noexcept {
        content_.swap(other.content_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
    
    SimpleVector(std::initializer_list<Type> init) {
        ArrayPtr<Type> tmp(init.size());
        std::copy(init.begin(), init.end(), tmp.begin());
        content_.swap(tmp);
        size_ = init.size();
        capacity_ = size_;
    }

    SimpleVector(const SimpleVector& other) {
        ArrayPtr<Type> tmp(other.size_);
        std::copy(other.cbegin(), other.cend(), tmp.begin());
        content_.swap(tmp);
        size_ = other.size_;
        capacity_ = other.capacity_;
    }

    SimpleVector(ReserveProxyObj to_reserve) {
        size_t to_res = to_reserve.GetSize();
        ArrayPtr<Type> tmp(to_res);
        content_.swap(tmp);
        size_ = 0;
        capacity_ = to_res;
    }

    ~SimpleVector() = default;
    
    SimpleVector& operator=(const SimpleVector& rhs) {
        //if (&content_[0] == &rhs.content_[0])
        if (this == &rhs)
            return *this;
        if (capacity_ >= rhs.size_) {
            std::copy(rhs.cbegin(), rhs.cend(), &content_[0]);
            size_ = rhs.size_;
            return *this;
        }
        ArrayPtr<Type> tmp(rhs.size_);
        std::copy(rhs.begin(), rhs.end(), tmp.begin());
        content_.swap(tmp);
        size_ = rhs.size_;
        capacity_ = rhs.capacity_;

        return *this;
    }

    void PushBack(const Type& item) {
        if (size_ < capacity_) {
            content_[size_] = item;
            ++size_;
            return;
        }
        size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
        ArrayPtr<Type> tmp(new_capacity);
        std::copy(begin(), end(), tmp.begin());
        tmp[size_] = item;
        content_.swap(tmp);     // here and everywhere - move?

        ++size_;
        capacity_ = new_capacity;
    }

    void PushBack(Type&& item) {
        if (size_ < capacity_) {
            content_[size_] = std::move(item);
            ++size_;
            return;
        }
        size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
        ArrayPtr<Type> tmp(new_capacity);
        std::move(begin(), end(), tmp.begin());
        tmp[size_] = std::move(item);
        content_.swap(tmp);

        ++size_;
        capacity_ = new_capacity;
    }
    
    Iterator Insert(ConstIterator pos, const Type& value) {
        if (pos < begin() || pos > end())
            throw std::out_of_range("Invalid iterator insert");

        if (capacity_ == 0) {
            // pos - ?? nullptr? first_?
            ArrayPtr<Type> tmp(1);
            tmp[0] = value;
            content_.swap(tmp);
            capacity_ = 1;
            size_ = 1;
            return begin();
        }
        auto dist = std::distance(cbegin(), pos);
        if (size_ < capacity_) {
            if (pos != cend())
                std::copy_backward(pos + 1, cend(), end() + 1);
            *(begin() + dist) = value;
            ++size_;
            return begin() + dist;
        }
        ArrayPtr<Type> tmp(capacity_ * 2);
        std::copy(cbegin(), cbegin() + dist, tmp.begin());
        std::copy_backward(cbegin() + dist, cend(), &tmp[size_ + 1]);
        tmp[dist] = value;

        content_.swap(tmp);
        ++size_;
        capacity_ *= 2;

        return begin() + dist;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        if (pos < begin() || pos > end())
            throw std::out_of_range("Invalid iterator insert"); 
        
        if (capacity_ == 0) {
            // pos - ?? nullptr? first_?
            ArrayPtr<Type> tmp(1);
            tmp[0] = std::move(value);
            content_.swap(tmp);
            capacity_ = 1;
            size_ = 1;
            return begin();
        }
        auto dist = std::distance(cbegin(), pos);
        if (size_ < capacity_) {
            if (pos != cend())
                std::move_backward(begin() + dist + 1, end(), end() + 1);
            *(begin() + dist) = std::move(value);
            ++size_;
            return begin() + dist;
        }
        ArrayPtr<Type> tmp(capacity_ * 2);
        std::move(begin(), begin() + dist, tmp.begin());
        std::move_backward(begin() + dist, end(), &tmp[size_ + 1]);
        tmp[dist] = std::move(value);

        content_.swap(tmp);
        ++size_;
        capacity_ *= 2;

        return begin() + dist;
    }
    
    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;
    }
    
    Iterator Erase(ConstIterator pos) {
        if (pos < begin() || pos > end())
            throw std::out_of_range("Invalid iterator erase"); 

        auto dist = std::distance(cbegin(), pos);
        ArrayPtr<Type> tmp(capacity_);
        std::move(begin(), begin() + dist, tmp.begin());
        std::move(begin() + dist + 1, end(), &tmp[dist]);
        content_.swap(tmp);
        --size_;
        return begin() + dist;
    }
    
    void swap(SimpleVector& other) noexcept {
        content_.swap(other.content_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
    
    size_t GetSize() const noexcept {
        return size_;
    }
    
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }
    
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return content_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return content_[index];
    }
    
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Invalid index in At method");
        }
        return content_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Invalid index in At method");
        }
        return content_[index];
    }
    
    void Clear() noexcept {
        size_ = 0;
    }
    
    void Resize(size_t new_size) {
        if (new_size < size_) {
            size_ = new_size;
            return;
        }

        if (new_size < capacity_) {
            Type* It = begin() + size_;
            for (size_t i = size_; i < new_size; ++i) {
                *It = Type{};
                ++It;
            }

            size_ = new_size;
            return;
        }

        ArrayPtr<Type> tmp(new_size);
        std::move(begin(), end(), tmp.begin());
        content_.swap(tmp);

        capacity_ = new_size;
        size_ = new_size;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity == capacity_)
            return;

        if (new_capacity < capacity_) {
            //ArrayPtr<Type> tmp(new_capacity);
            //std::copy(cbegin(), cbegin() + std::min(size_, new_capacity), tmp.begin());
            //content_.swap(tmp);
            //size_ = std::min(size_, new_capacity);
            //capacity_ = new_capacity;
            return;
        }

        ArrayPtr<Type> tmp(new_capacity);
        std::copy(cbegin(), cend(), tmp.begin());
        content_.swap(tmp);
        capacity_ = new_capacity;
    }
        
    Iterator begin() noexcept {
        //if (capacity_ == 0)
        //    return nullptr;
        return content_.Get();
    }

    Iterator end() noexcept {
        //if (capacity_ == 0)
        //    return nullptr;
        return content_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        //if (capacity_ == 0)
        //    return nullptr;
        return content_.Get();
    }

    ConstIterator end() const noexcept {
        //if (capacity_ == 0)
        //    return nullptr;
        return content_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return begin();
    }

    ConstIterator cend() const noexcept {
        return end();
    }
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) {
        return false;
    }
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs == lhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end()));
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs <= rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

*/
