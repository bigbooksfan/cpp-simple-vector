#pragma once

#include <stdexcept>
#include <utility>

// Empty instance call throw this exception
class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
private:        // fields
    // alignas - for correct memory align
    alignas(T) char data_[sizeof(T)];
    bool is_initialized_ = false;
    T* value_ = nullptr;

public:         // constructors
    Optional() = default;
    Optional(const T& value);
    Optional(T&& value);
    Optional(const Optional& other);
    Optional(Optional&& other);
    ~Optional();

public:         // methods
    bool HasValue() const;
    // Value() throws BadOptionalAccess, if instance is empty
    T& Value() &;
    const T& Value() const &;
    T&& Value() &&;
    void Reset();
    template <typename... S>
    T& Emplace(S&&... value);

public:         // operators
    Optional& operator=(const T& value);
    Optional& operator=(T&& rhs);
    Optional& operator=(const Optional& rhs);
    Optional& operator=(Optional&& rhs);
    
    T& operator*()&;
    const T& operator*() const&;
    T&& operator*()&&;

    T* operator->();
    const T* operator->() const;
};

template <typename T>
template <typename... S>
T& Optional<T>::Emplace(S&&... value) {
    if (is_initialized_) {
        Reset();
    }
    value_ = new (data_) T(std::forward<S>(value)...);
    is_initialized_ = true;
    return *value_;
}

template<typename T>
inline Optional<T>::Optional(const T& value) {
    value_ = new (data_) T(value);
    is_initialized_ = true;    
}

template<typename T>
inline Optional<T>::Optional(T&& value) {
    value_ = new (data_) T(std::move(value));
    is_initialized_ = true;
}

template<typename T>
inline Optional<T>::Optional(const Optional& other) {
    if (other.is_initialized_) {
        value_ = new (data_) T(*(other.value_));
        is_initialized_ = true;
    }
}

template<typename T>
inline Optional<T>::Optional(Optional&& other) {
    if (other.is_initialized_) {
        value_ = new (data_) T(std::move(*(other.value_)));
        is_initialized_ = true;
    }
}

template<typename T>
inline Optional<T>::~Optional() {
    Reset();
}

template<typename T>
inline bool Optional<T>::HasValue() const {
    return is_initialized_;
}

template<typename T>
inline T& Optional<T>::Value() & {
    if (is_initialized_) {
        return *value_;
    }
    else {
        throw BadOptionalAccess();
    }
}

template<typename T>
inline const T& Optional<T>::Value() const & {
    if (is_initialized_) {
        return *value_;
    }
    else {
        throw BadOptionalAccess();
    }
}

template <typename T>
inline T&& Optional<T>::Value()&& {
    if (is_initialized_) {
        return std::move(*value_);
    }
    else {
        throw BadOptionalAccess();
    }
}

template<typename T>
inline void Optional<T>::Reset() {
    if (is_initialized_) {
        value_->~T();
    }
    is_initialized_ = false;
}

template<typename T>
inline Optional<T>& Optional<T>::operator=(const T& value) {
    if (is_initialized_) {
        *value_ = value;
    }
    else {
        value_ = new (data_) T(value);
        is_initialized_ = true;
    }
    return *this;
}

template<typename T>
inline Optional<T>& Optional<T>::operator=(T&& rhs) {
    if (is_initialized_) {
        *value_ = std::move(rhs);
    }
    else {
        value_ = new (data_) T(std::move(rhs));
        is_initialized_ = true;
    }

    return *this;
}

template<typename T>
inline Optional<T>& Optional<T>::operator=(const Optional& rhs) {
    if (rhs.is_initialized_) {
        if (is_initialized_) {
            *value_ = *rhs.value_;
        }
        else {
            value_ = new (data_) T(*rhs.value_);
            is_initialized_ = true;
        }
    }
    else {
        Reset();
    }
    return *this;
}

template<typename T>
inline Optional<T>& Optional<T>::operator=(Optional&& rhs) {
    if (rhs.is_initialized_) {
        if (is_initialized_) {
            *value_ = std::move(*rhs.value_);
        }
        else {
            value_ = new (data_) T(std::move(*rhs.value_));
            is_initialized_ = true;
        }        
    }
    else {
        Reset();
    }
    return *this;
}

template<typename T>
inline T& Optional<T>::operator*() & {
    if (is_initialized_) {
        return *value_;
    }
    else {
        throw BadOptionalAccess();
    }
}

template<typename T>
inline const T& Optional<T>::operator*() const & {
    if (is_initialized_) {
        return *value_;
    }
    else {
        throw BadOptionalAccess();
    }
}

template<typename T>
inline T&& Optional<T>::operator*() && {
    if (is_initialized_) {
        return std::move(*value_);
    }
    else {
        throw BadOptionalAccess();
    }
}

template<typename T>
inline T* Optional<T>::operator->() {
    if (is_initialized_) {
        return value_;
    }
    else {
        throw BadOptionalAccess();
    }
}

template<typename T>
inline const T* Optional<T>::operator->() const {
    if (is_initialized_) {
        return value_;
    }
    else {
        throw BadOptionalAccess();
    }
}