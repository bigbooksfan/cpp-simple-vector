#pragma once

#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>
#include <memory>
#include <iostream>
#include <algorithm>

// raw memory wrapper
template <typename T>
class RawMemory {
private:        // fields
    T* buffer_ = nullptr;
    size_t capacity_ = 0;

public:         // constructors
    RawMemory() = default;
    explicit RawMemory(size_t capacity);

    RawMemory(const RawMemory&) = delete;
    RawMemory(RawMemory&& other) noexcept;

    ~RawMemory();

public:         // operators
    T* operator+(size_t offset) noexcept;
    const T* operator+(size_t offset) const noexcept;

    const T& operator[](size_t index) const noexcept;
    T& operator[](size_t index) noexcept;

    RawMemory& operator=(const RawMemory&) = delete;
    RawMemory& operator=(RawMemory&& other) noexcept;

public:         // methods
    void Swap(RawMemory& other) noexcept;
    const T* GetAddress() const noexcept;
    T* GetAddress() noexcept;
    size_t Capacity() const;

private:        // methods
    static T* Allocate(size_t n);
    static void Deallocate(T* buf) noexcept;
};

template <typename T>
class Vector {
private:        // fields
    RawMemory<T> data_;
    size_t size_ = 0;

public:         // constructors
    Vector() = default;
    explicit Vector(size_t size);
    Vector(const Vector& other);
    Vector(Vector&& other) noexcept;
    ~Vector();

public:         // iterators
    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() noexcept;
    iterator end() noexcept;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;

public:         // operators
    const T& operator[](size_t index) const noexcept;
    T& operator[](size_t index) noexcept;
    
    Vector& operator=(const Vector& other);
    Vector& operator=(Vector&& other) noexcept;

public:         // methods
    size_t Size() const noexcept;
    size_t Capacity() const noexcept;
    void Reserve(size_t new_capacity);
    void Swap(Vector& other) noexcept;
    void Resize(size_t new_size);
    void PopBack() /* noexcept */;
    void PushBack(const T& value);
    void PushBack(T&& value);
    iterator Erase(const_iterator pos);
    iterator Insert(const_iterator pos, const T& value);
    iterator Insert(const_iterator pos, T&& value);

    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args);
    template <typename... Args>
    T& EmplaceBack(Args&&... args);

private:        // methods
    static void DestroyN(T* buf, size_t n) noexcept;
    static void Destroy(T* buf) noexcept;
};

template<typename T>
inline RawMemory<T>::RawMemory(size_t capacity)
    : buffer_(Allocate(capacity))
    , capacity_(capacity) { }

template<typename T>
inline RawMemory<T>::RawMemory(RawMemory&& other) noexcept
    : buffer_(std::exchange(other.buffer_, nullptr))
    , capacity_(std::exchange(other.capacity_, 0)) {
}

template<typename T>
inline RawMemory<T>::~RawMemory() {
    Deallocate(buffer_);
}

template<typename T>
inline Vector<T>::Vector(size_t size) 
    : data_(size), size_(size) {
    std::uninitialized_value_construct_n(data_.GetAddress(), size);
}

template<typename T>
inline Vector<T>::Vector(const Vector& other) 
    : data_(other.size_), size_(other.size_) {
    std::uninitialized_copy_n(
        other.data_.GetAddress(), 
        other.size_, 
        data_.GetAddress());
}

template<typename T>
inline Vector<T>::Vector(Vector&& other) noexcept 
    : data_(std::move(other.data_))
    , size_(std::exchange(other.size_, 0)) { }

template <typename T>
Vector<T>::~Vector() {
    std::destroy_n(data_.GetAddress(), size_);
}

template<typename T>
inline T* Vector<T>::begin() noexcept {
    return data_.GetAddress();
}

template<typename T>
inline T* Vector<T>::end() noexcept {
    return data_.GetAddress() + size_;
}

template<typename T>
inline const T* Vector<T>::begin() const noexcept {
    return data_.GetAddress();
}

template<typename T>
inline const T* Vector<T>::end() const noexcept {
    return data_.GetAddress() + size_;
}

template<typename T>
inline const T* Vector<T>::cbegin() const noexcept {
    return data_.GetAddress();
}

template<typename T>
inline const T* Vector<T>::cend() const noexcept {
    return data_.GetAddress() + size_;
}

template <typename T>
size_t Vector<T>::Size() const noexcept {
    return size_;
}

template <typename T>
size_t Vector<T>::Capacity() const noexcept {
    return data_.Capacity();
}

template<typename T>
inline void Vector<T>::Reserve(size_t new_capacity) {
    if (new_capacity <= data_.Capacity()) {
        return;
    }
    RawMemory<T> tmp(new_capacity);

    if constexpr (
        std::is_nothrow_move_constructible_v<T> ||
        !std::is_copy_constructible_v<T>) 
    {
        std::uninitialized_move_n(data_.GetAddress(), size_, tmp.GetAddress());
    }
    else {
        std::uninitialized_copy_n(data_.GetAddress(), size_, tmp.GetAddress());
    }

    std::destroy_n(data_.GetAddress(), size_);
    data_.Swap(tmp);
}

template<typename T>
inline void Vector<T>::Swap(Vector& other) noexcept {
    data_.Swap(other.data_);
    std::swap(size_, other.size_);
}

template<typename T>
inline void Vector<T>::Resize(size_t new_size) {
    if (data_.Capacity() >= new_size) {
        if (new_size > size_) {
            std::uninitialized_value_construct_n(
                data_.GetAddress() + size_,
                new_size - size_);
        }
        else {
            std::destroy_n(data_.GetAddress() + new_size, size_ - new_size);            
        }
    }
    else {
        size_t new_capacity = new_size;
        Reserve(new_capacity);
        std::uninitialized_value_construct_n(
            data_.GetAddress() + size_,
            new_capacity - size_);
    }
    size_ = new_size;
}

template <typename T>
inline void Vector<T>::PushBack(const T& value) {
    EmplaceBack(value);
}

template <typename T>
inline void Vector<T>::PushBack(T&& value) {
    EmplaceBack(std::move(value));
}

template<typename T>
inline typename Vector<T>::iterator Vector<T>::Erase(typename Vector<T>::const_iterator pos) {
    assert(pos >= begin() && pos < end());
    size_t dist = pos - begin();
    std::move(begin() + dist + 1, end(), begin() + dist);
    PopBack();
    return begin() + dist;
}

template<typename T>
inline typename Vector<T>::iterator Vector<T>::Insert(typename Vector<T>::const_iterator pos, const T& value) {
    return Emplace(pos, value);
}

template<typename T>
inline typename Vector<T>::iterator Vector<T>::Insert(typename Vector<T>::const_iterator pos, T&& value) {
    return Emplace(pos, std::move(value));
}

template<typename T>
template<typename... Args>
inline T& Vector<T>::EmplaceBack(Args&&... args) {
    return *Emplace(
        end(),
        std::forward<Args>(args)...);
}

template<typename T>
template<typename... Args>
inline typename Vector<T>::iterator Vector<T>::Emplace(
    typename Vector<T>::const_iterator pos, 
    Args&& ...args) {
    assert(pos >= begin() && pos <= end());     // <= end() because could be EmplaceBack()
    T* It = const_cast<T*>(pos);
    size_t dist = It - begin();

    if (data_.Capacity() > size_) {
        if (dist < size_) {
            T tmp(std::forward<Args>(args)...);
            new (&data_[size_]) T(std::move(data_[size_ - 1]));
            std::move_backward(It, end() - 1, end());
            data_[dist] = std::move(tmp);
        }
        else {
            new (&data_[size_]) T(std::forward<Args>(args)...);
        }
    }
    else {
        size_t new_capacity = data_.Capacity() == 0 ? 1 : data_.Capacity() * 2;
        RawMemory<T> tmp(new_capacity);

        new (tmp + dist) T(std::forward<Args>(args)...);
        if constexpr (
            std::is_nothrow_move_constructible_v<T> ||
            !std::is_copy_constructible_v<T>)
        {
            std::uninitialized_move_n(
                data_.GetAddress(),
                dist,
                tmp.GetAddress());
            std::uninitialized_move_n(
                data_.GetAddress() + dist,
                size_ - dist,
                tmp.GetAddress() + dist + 1);
        }
        else {
            std::uninitialized_copy_n(
                data_.GetAddress(),
                dist,
                tmp.GetAddress());
            std::uninitialized_copy_n(
                data_.GetAddress() + dist,
                size_ - dist,
                tmp.GetAddress() + dist + 1);
        }        
        data_.Swap(tmp);
        std::destroy_n(tmp.GetAddress(), tmp.Capacity());
    }
    ++size_;
    return data_.GetAddress() + dist;
}

template<typename T>
inline void Vector<T>::PopBack() {
    if (size_ == 0) {
        return;
    }
    --size_;
    std::destroy_n(data_.GetAddress() + size_, 1);
}

template <typename T>
const T& Vector<T>::operator[](size_t index) const noexcept {
    return const_cast<Vector&>(*this)[index];
}

template <typename T>
T& Vector<T>::operator[](size_t index) noexcept {
    assert(index < size_);
    return data_[index];
}

template<typename T>
inline Vector<T>& Vector<T>::operator=(const Vector& other) {
    if (this != &other) {
        if (other.size_ > data_.Capacity()) {   // copy - swap
            Vector<T> other_copy(other);
            Swap(other_copy);
        }
        else {
            if (size_ > other.size_) {
                std::copy(other.data_.GetAddress(),
                    other.data_.GetAddress() + other.size_,
                    data_.GetAddress());
                std::destroy_n(data_.GetAddress() + other.size_,
                    size_ - other.size_);
            }
            else {
                std::copy(other.data_.GetAddress(),
                    other.data_.GetAddress() + size_,
                    data_.GetAddress());
                std::uninitialized_copy_n(other.data_.GetAddress() + size_,
                    other.size_ - size_,
                    data_.GetAddress() + size_);
            }
            size_ = other.size_;
        }
    }
    return *this;
}

template<typename T>
inline Vector<T>& Vector<T>::operator=(Vector&& other) noexcept {
    Swap(other);
    return *this;
}

template<typename T>
inline void Vector<T>::DestroyN(T* buf, size_t n) noexcept {
    for (size_t i = 0; i < n; ++i) {
        Destroy(buf + i);
    }
}

template<typename T>
inline void Vector<T>::Destroy(T* buf) noexcept {
    buf->~T();
}

template<typename T>
inline void RawMemory<T>::Swap(RawMemory& other) noexcept {
    std::swap(buffer_, other.buffer_);
    std::swap(capacity_, other.capacity_);
}

template<typename T>
inline const T* RawMemory<T>::GetAddress() const noexcept {
    return buffer_;
}

template<typename T>
inline T* RawMemory<T>::GetAddress() noexcept {
    return buffer_;
}

template<typename T>
inline size_t RawMemory<T>::Capacity() const {
    return capacity_;
}

template<typename T>
inline T* RawMemory<T>::Allocate(size_t n) {
    return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
}

template<typename T>
inline void RawMemory<T>::Deallocate(T* buf) noexcept {
    operator delete(buf);
}

template<typename T>
inline T* RawMemory<T>::operator+(size_t offset) noexcept {
    // <= (not <) because .end() is out of allocated memory
    assert(offset <= capacity_);
    return buffer_ + offset;
}

template<typename T>
inline const T* RawMemory<T>::operator+(size_t offset) const noexcept {
    return const_cast<RawMemory&>(*this) + offset;
}

template<typename T>
inline const T& RawMemory<T>::operator[](size_t index) const noexcept {
    return const_cast<RawMemory&>(*this)[index];
}

template<typename T>
inline T& RawMemory<T>::operator[](size_t index) noexcept {
    assert(index < capacity_);
    return buffer_[index];
}

template<typename T>
inline RawMemory<T>& RawMemory<T>::operator=(RawMemory&& other) noexcept {
    if (&other != this) {
        buffer_ = std::move(other.buffer_);
        capacity_ = std::move(other.capacity_);
        other.buffer_ = nullptr;
        other.capacity_ = 0;
    }
    return *this;
}