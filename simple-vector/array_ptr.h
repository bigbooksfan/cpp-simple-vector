#pragma once

#include <cstdlib>
#include <algorithm>

template <typename Type>
class ArrayPtr {
public:
	// �������������� ArrayPtr ������� ����������
	ArrayPtr() = default;

	// ������ � ���� ������ �� size ��������� ���� Type.
	// ���� size == 0, ���� raw_ptr_ ������ ���� ����� nullptr
	explicit ArrayPtr(size_t size) {
		size_ = size;
		if (size == 0) {
			raw_ptr_ = nullptr;
			return;
		}
		raw_ptr_ = new Type[size]{};
	}

	// ����������� �� ������ ���������, ��������� ����� ������� � ���� ���� nullptr
	explicit ArrayPtr(Type* raw_ptr) noexcept {
		raw_ptr_ = raw_ptr;
	}

	// ��������� �����������
	ArrayPtr(const ArrayPtr&) = delete;

	ArrayPtr(ArrayPtr&& other) {
		raw_ptr_ = std::move(other.raw_ptr_);
	}

	~ArrayPtr() {
		delete[] raw_ptr_;
		size_ = 0;
	}

	Type* begin() {
		return raw_ptr_;
	}

	Type* end() {
		return raw_ptr_ + size_;
	}

	// ��������� ������������
	ArrayPtr& operator=(const ArrayPtr&) = delete;

	ArrayPtr& operator=(ArrayPtr&& other) {
		delete[] raw_ptr_;
		raw_ptr_ = std::move(other.raw_ptr_);
		return *this;
	}

	// ���������� ��������� �������� � ������, ���������� �������� ������ �������
	// ����� ������ ������ ��������� �� ������ ������ ����������
	[[nodiscard]] Type* Release() noexcept {
		Type* tmp = &raw_ptr_[0];
		//delete[] raw_ptr_;
		raw_ptr_ = nullptr;
		size_ = 0;
		return tmp;
	}

	// ���������� ������ �� ������� ������� � �������� index
	Type& operator[](size_t index) noexcept {
		Type& ret = raw_ptr_[index];
		return ret;
	}

	// ���������� ����������� ������ �� ������� ������� � �������� index
	const Type& operator[](size_t index) const noexcept {
		const Type& ret = raw_ptr_[index];
		return ret;
	}

	// ���������� true, ���� ��������� ���������, � false � ��������� ������
	explicit operator bool() const {
		return raw_ptr_ != nullptr;
	}

	// ���������� �������� ������ ���������, ��������� ����� ������ �������
	Type* Get() const noexcept {
		return raw_ptr_;
	}

	// ������������ ��������� ��������� �� ������ � �������� other
	void swap(ArrayPtr& other) noexcept {
		std::swap(raw_ptr_, other.raw_ptr_);
		std::swap(size_, other.size_);
	}

	size_t GetSize() {
		return size_;
	}

private:
	Type* raw_ptr_ = nullptr;
	size_t size_ = 0;
};
