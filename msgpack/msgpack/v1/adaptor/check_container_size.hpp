//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2015 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_CHECK_CONTAINER_SIZE_HPP
#define MSGPACK_V1_CHECK_CONTAINER_SIZE_HPP

#include "msgpack/v1/adaptor/check_container_size_decl.hpp"

namespace msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond
#if 0
struct container_size_overflow : public std::runtime_error {
    explicit container_size_overflow(const eastl::string& msg)
        :std::runtime_error(msg) {}
#if !defined(MSGPACK_USE_CPP03)
    explicit container_size_overflow(const char* msg):
        std::runtime_error(msg) {}
#endif // !defined(MSGPACK_USE_CPP03)
};
#endif

namespace detail {

template <std::size_t N>
inline void check_container_size(std::size_t size) {
    if (size > 0xffffffff) ExRaiseStatus(EMSGPACK_CONTAINER_SIZE_OVERFLOW);
}

template <>
inline void check_container_size<4>(std::size_t /*size*/) {
}

template <std::size_t N>
inline void check_container_size_for_ext(std::size_t size) {
    if (size > 0xffffffff) ExRaiseStatus(EMSGPACK_CONTAINER_SIZE_OVERFLOW);
}

template <>
inline void check_container_size_for_ext<4>(std::size_t size) {
    if (size > 0xfffffffe) ExRaiseStatus(EMSGPACK_CONTAINER_SIZE_OVERFLOW);
}

} // namespace detail

template <typename T>
inline uint32_t checked_get_container_size(T size) {
    detail::check_container_size<sizeof(T)>(size);
    return static_cast<uint32_t>(size);
}


/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace msgpack

#endif // MSGPACK_V1_CHECK_CONTAINER_SIZE_HPP
