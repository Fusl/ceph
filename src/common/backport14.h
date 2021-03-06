// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*- 
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2004-2006 Sage Weil <sage@newdream.net>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 *
 */

#include <memory>
#include <type_traits>

#ifndef CEPH_COMMON_BACKPORT14_H
#define CEPH_COMMON_BACKPORT14_H

// Library code from C++14 that can be implemented in C++11.

namespace ceph {
template<typename T>
using remove_extent_t = typename std::remove_extent<T>::type;
template<typename T>
using remove_reference_t = typename std::remove_reference<T>::type;
template<typename T>
using result_of_t = typename std::result_of<T>::type;

template<typename T>
using decay_t = typename std::decay<T>::type;

template<bool T, typename F = void>
using enable_if_t = typename std::enable_if<T,F>::type;

namespace _backport14 {
template<typename T>
struct uniquity {
  using datum = std::unique_ptr<T>;
};

template<typename T>
struct uniquity<T[]> {
  using array = std::unique_ptr<T[]>;
};

template<typename T, std::size_t N>
struct uniquity<T[N]> {
  using verboten = void;
};

template<typename T, typename... Args>
inline typename uniquity<T>::datum make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename T>
inline typename uniquity<T>::array make_unique(std::size_t n) {
  return std::unique_ptr<T>(new remove_extent_t<T>[n]());
}

template<typename T, class... Args>
typename uniquity<T>::verboten
make_unique(Args&&...) = delete;

// The constexpr variant of std::max().
template<class T>
constexpr const T& max(const T& a, const T& b) {
  return a < b ? b : a;
}
} // namespace _backport14

namespace _backport17 {
template <class C>
constexpr auto size(const C& c) -> decltype(c.size()) {
  return c.size();
}

template <typename T, std::size_t N>
constexpr std::size_t size(const T (&array)[N]) noexcept {
  return N;
}

/// http://en.cppreference.com/w/cpp/utility/functional/not_fn
// this implementation uses c++14's result_of_t (above) instead of the c++17
// invoke_result_t, and so may not behave correctly when SFINAE is required
template <typename F>
class not_fn_result {
  using DecayF = decay_t<F>;
  DecayF fn;
 public:
  explicit not_fn_result(F&& f) : fn(std::forward<F>(f)) {}
  not_fn_result(not_fn_result&& f) = default;
  not_fn_result(const not_fn_result& f) = default;

  template<class... Args>
  auto operator()(Args&&... args) &
  -> decltype(!std::declval<result_of_t<DecayF&(Args...)>>()) {
    return !fn(std::forward<Args>(args)...);
  }
  template<class... Args>
  auto operator()(Args&&... args) const&
  -> decltype(!std::declval<result_of_t<DecayF const&(Args...)>>()) {
    return !fn(std::forward<Args>(args)...);
  }

  template<class... Args>
  auto operator()(Args&&... args) &&
  -> decltype(!std::declval<result_of_t<DecayF(Args...)>>()) {
    return !std::move(fn)(std::forward<Args>(args)...);
  }
  template<class... Args>
  auto operator()(Args&&... args) const&&
  -> decltype(!std::declval<result_of_t<DecayF const(Args...)>>()) {
    return !std::move(fn)(std::forward<Args>(args)...);
  }
};

template <typename F>
not_fn_result<F> not_fn(F&& fn) {
  return not_fn_result<F>(std::forward<F>(fn));
}

struct in_place_t {};
constexpr in_place_t in_place{};

template<typename T>
struct in_place_type_t {};

#ifdef __cpp_variable_templates
template<typename T>
constexpr in_place_type_t<T> in_place_type{};
#endif // __cpp_variable_templates
} // namespace _backport17

namespace _backport_ts {
template <class DelimT,
          class CharT = char,
          class Traits = std::char_traits<CharT>>
class ostream_joiner {
public:
  using char_type = CharT;
  using traits_type = Traits;
  using ostream_type = std::basic_ostream<CharT, Traits>;
  using iterator_category = std::output_iterator_tag;
  using value_type = void;
  using difference_type = void;
  using pointer = void;
  using reference = void;

  ostream_joiner(ostream_type& s, const DelimT& delimiter)
    : out_stream(std::addressof(out_stream)),
      delim(delimiter),
      first_element(true)
  {}
  ostream_joiner(ostream_type& s, DelimT&& delimiter)
    : out_stream(std::addressof(s)),
      delim(std::move(delimiter)),
      first_element(true)
  {}

  template<typename T>
  ostream_joiner& operator=(const T& value) {
    if (!first_element)
      *out_stream << delim;
    first_element = false;
    *out_stream << value;
    return *this;
  }

  ostream_joiner& operator*() noexcept {
    return *this;
  }
  ostream_joiner& operator++() noexcept {
    return *this;
  }
  ostream_joiner& operator++(int) noexcept {
    return this;
  }

private:
  ostream_type* out_stream;
  DelimT delim;
  bool first_element;
};

template <class CharT, class Traits, class DelimT>
ostream_joiner<decay_t<DelimT>, CharT, Traits>
make_ostream_joiner(std::basic_ostream<CharT, Traits>& os,
                    DelimT&& delimiter) {
    return ostream_joiner<decay_t<DelimT>,
                          CharT, Traits>(os, std::forward<DelimT>(delimiter));
}

} // namespace _backport_ts

using _backport14::make_unique;
using _backport17::size;
using _backport14::max;
using _backport17::not_fn;
using _backport17::in_place_t;
using _backport17::in_place;
using _backport17::in_place_type_t;
#ifdef __cpp_variable_templates
using _backport17::in_place_type;
#endif // __cpp_variable_templates
using _backport_ts::ostream_joiner;
using _backport_ts::make_ostream_joiner;
} // namespace ceph

#endif // CEPH_COMMON_BACKPORT14_H
