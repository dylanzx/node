// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_VECTOR_H_
#define V8_VECTOR_H_

#include <string.h>
#include <algorithm>

#include "src/allocation.h"
#include "src/checks.h"
#include "src/globals.h"

namespace v8 {
namespace internal {


template <typename T>
class Vector {
 public:
  Vector() : start_(NULL), length_(0) {}
  Vector(T* data, int length) : start_(data), length_(length) {
    DCHECK(length == 0 || (length > 0 && data != NULL));
  }

  template <int N>
  explicit Vector(T (&arr)[N]) : start_(arr), length_(N) {}

  static Vector<T> New(int length) {
    return Vector<T>(NewArray<T>(length), length);
  }

  // Returns a vector using the same backing storage as this one,
  // spanning from and including 'from', to but not including 'to'.
  Vector<T> SubVector(int from, int to) const {
    DCHECK(0 <= from);
    SLOW_DCHECK(from < to);
    SLOW_DCHECK(static_cast<unsigned>(to) <= static_cast<unsigned>(length_));
    return Vector<T>(start() + from, to - from);
  }

  // Returns the length of the vector.
  int length() const { return length_; }

  // Returns whether or not the vector is empty.
  bool is_empty() const { return length_ == 0; }

  // Returns the pointer to the start of the data in the vector.
  T* start() const { return start_; }

  // Access individual vector elements - checks bounds in debug mode.
  T& operator[](int index) const {
    DCHECK_LE(0, index);
    DCHECK_LT(index, length_);
    return start_[index];
  }

  const T& at(int index) const { return operator[](index); }

  T& first() { return start_[0]; }

  T& last() { return start_[length_ - 1]; }

  typedef T* iterator;
  inline iterator begin() const { return &start_[0]; }
  inline iterator end() const { return &start_[length_]; }

  // Returns a clone of this vector with a new backing store.
  Vector<T> Clone() const {
    T* result = NewArray<T>(length_);
    for (int i = 0; i < length_; i++) result[i] = start_[i];
    return Vector<T>(result, length_);
  }

  template <typename CompareFunction>
  void Sort(CompareFunction cmp, size_t s, size_t l) {
    std::sort(start() + s, start() + s + l, RawComparer<CompareFunction>(cmp));
  }

  template <typename CompareFunction>
  void Sort(CompareFunction cmp) {
    std::sort(start(), start() + length(), RawComparer<CompareFunction>(cmp));
  }

  void Sort() {
    std::sort(start(), start() + length());
  }

  template <typename CompareFunction>
  void StableSort(CompareFunction cmp, size_t s, size_t l) {
    std::stable_sort(start() + s, start() + s + l,
                     RawComparer<CompareFunction>(cmp));
  }

  template <typename CompareFunction>
  void StableSort(CompareFunction cmp) {
    std::stable_sort(start(), start() + length(),
                     RawComparer<CompareFunction>(cmp));
  }

  void StableSort() { std::stable_sort(start(), start() + length()); }

  void Truncate(int length) {
    DCHECK(length <= length_);
    length_ = length;
  }

  // Releases the array underlying this vector. Once disposed the
  // vector is empty.
  void Dispose() {
    DeleteArray(start_);
    start_ = NULL;
    length_ = 0;
  }

  inline Vector<T> operator+(int offset) {
    DCHECK(offset < length_);
    return Vector<T>(start_ + offset, length_ - offset);
  }

  // Implicit conversion from Vector<T> to Vector<const T>.
  inline operator Vector<const T>() { return Vector<const T>::cast(*this); }

  // Factory method for creating empty vectors.
  static Vector<T> empty() { return Vector<T>(NULL, 0); }

  template<typename S>
  static Vector<T> cast(Vector<S> input) {
    return Vector<T>(reinterpret_cast<T*>(input.start()),
                     input.length() * sizeof(S) / sizeof(T));
  }

  bool operator==(const Vector<T>& other) const {
    if (length_ != other.length_) return false;
    if (start_ == other.start_) return true;
    for (int i = 0; i < length_; ++i) {
      if (start_[i] != other.start_[i]) {
        return false;
      }
    }
    return true;
  }

 protected:
  void set_start(T* start) { start_ = start; }

 private:
  T* start_;
  int length_;

  template <typename CookedComparer>
  class RawComparer {
   public:
    explicit RawComparer(CookedComparer cmp) : cmp_(cmp) {}
    bool operator()(const T& a, const T& b) {
      return cmp_(&a, &b) < 0;
    }

   private:
    CookedComparer cmp_;
  };
};


template <typename T>
class ScopedVector : public Vector<T> {
 public:
  explicit ScopedVector(int length) : Vector<T>(NewArray<T>(length), length) { }
  ~ScopedVector() {
    DeleteArray(this->start());
  }

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(ScopedVector);
};


inline int StrLength(const char* string) {
  size_t length = strlen(string);
  DCHECK(length == static_cast<size_t>(static_cast<int>(length)));
  return static_cast<int>(length);
}


#define STATIC_CHAR_VECTOR(x)                                              \
  v8::internal::Vector<const uint8_t>(reinterpret_cast<const uint8_t*>(x), \
                                      arraysize(x) - 1)

inline Vector<const char> CStrVector(const char* data) {
  return Vector<const char>(data, StrLength(data));
}

inline Vector<const uint8_t> OneByteVector(const char* data, int length) {
  return Vector<const uint8_t>(reinterpret_cast<const uint8_t*>(data), length);
}

inline Vector<const uint8_t> OneByteVector(const char* data) {
  return OneByteVector(data, StrLength(data));
}

inline Vector<char> MutableCStrVector(char* data) {
  return Vector<char>(data, StrLength(data));
}

inline Vector<char> MutableCStrVector(char* data, int max) {
  int length = StrLength(data);
  return Vector<char>(data, (length < max) ? length : max);
}

template <typename T, int N>
inline Vector<T> ArrayVector(T (&arr)[N]) {
  return Vector<T>(arr);
}

}  // namespace internal
}  // namespace v8

#endif  // V8_VECTOR_H_
