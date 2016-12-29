/// The MIT License (MIT)
/// Copyright (c) 2016 Peter Goldsborough and Markus Engel
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.

#ifndef BASE_ORDERED_ITERATOR_HPP
#define BASE_ORDERED_ITERATOR_HPP

#include <functional>
#include <type_traits>

#include "lru/error.hpp"
#include "lru/internal/base-unordered-iterator.hpp"
#include "lru/internal/definitions.hpp"
#include "lru/internal/optional.hpp"
#include "lru/pair.hpp"

namespace LRU {
namespace Internal {
template <typename Key, typename Value, typename Cache>
class BaseOrderedIterator {
 public:
  using Tag = std::true_type;
  using UnderlyingIterator = typename Queue<Key>::const_iterator;
  using Pair = LRU::Internal::Pair<Key, Value>;

  explicit BaseOrderedIterator(Cache& cache, UnderlyingIterator iterator)
  : _iterator(iterator), _cache(cache) {
  }

  template <typename UnderlyingIterator>
  BaseOrderedIterator(const BaseUnorderedIterator<Cache, UnderlyingIterator>&
                          unordered_iterator)
  : _cache(unordered_iterator._cache) {
    if (unordered_iterator == _cache.unordered_end()) {
      throw LRU::Error::InvalidIteratorConversion();
    } else {
      _iterator = unordered_iterator._iterator->second.order;
    }
  }

  bool operator==(const BaseOrderedIterator& other) const noexcept {
    return this->_iterator == other._iterator;
  }

  bool operator!=(const BaseOrderedIterator& other) const noexcept {
    return !(*this == other);
  }

  template <typename OtherCache, typename OtherUnderlyingIterator>
  friend bool
  operator==(const BaseOrderedIterator& first,
             const BaseUnorderedIterator<OtherCache, OtherUnderlyingIterator>&
                 second) noexcept {
    if (&first._cache != &second._cache) return false;

    // The past-the-end iterators of the same cache should compare equal
    // This is an exceptional guarantee we make. This is also the reason
    // why we can't rely on the conversion from unordered to ordered iterators
    // because construction of an ordered iterator from the past-the-end
    // unordered iterator will fail (with an InvalidIteratorConversion error)
    if (second == second._cache.unordered_end()) {
      return first == first._cache.ordered_end();
    }

    // Will call the other overload
    return first == static_cast<BaseOrderedIterator>(second);
  }

  template <typename OtherCache, typename OtherUnderlyingIterator>
  friend bool operator==(
      const BaseUnorderedIterator<OtherCache, OtherUnderlyingIterator>& first,
      const BaseOrderedIterator& second) noexcept {
    return second == first;
  }

  template <typename OtherCache, typename OtherUnderlyingIterator>
  friend bool
  operator!=(const BaseOrderedIterator& first,
             const BaseUnorderedIterator<OtherCache, OtherUnderlyingIterator>&
                 second) noexcept {
    return !(first == second);
  }

  template <typename OtherCache, typename OtherUnderlyingIterator>
  friend bool operator!=(
      const BaseUnorderedIterator<OtherCache, OtherUnderlyingIterator>& first,
      const BaseOrderedIterator& second) noexcept {
    return second != first;
  }

  BaseOrderedIterator& operator++() {
    ++_iterator;
    _pair.reset();
    return *this;
  }

  BaseOrderedIterator operator++(int) {
    auto previous = *this;
    ++*this;
    return previous;
  }

  BaseOrderedIterator& operator--() {
    --_iterator;
    _pair.reset();
    return *this;
  }

  BaseOrderedIterator operator--(int) {
    auto previous = *this;
    --*this;
    return previous;
  }

  Pair& operator*() noexcept {
    return pair();
  }

  Pair* operator->() noexcept {
    return &(**this);
  }

  Pair& pair() noexcept {
    return _maybe_lookup();
  }

  Value& value() noexcept {
    return _maybe_lookup().value();
  }

  const Key& key() noexcept {
    return *_iterator;
  }

 protected:
  Pair& _maybe_lookup() {
    if (!_pair.has_value()) {
      _lookup();
    }

    return *_pair;
  }

  void _lookup() {
    Value& value = _cache.lookup(key());
    _pair.emplace(key(), value);
  }

  UnderlyingIterator _iterator;
  Optional<Pair> _pair;
  Cache& _cache;
};

}  // namespace Internal
}  // namespace LRU

#endif  // BASE_ORDERED_ITERATOR_HPP