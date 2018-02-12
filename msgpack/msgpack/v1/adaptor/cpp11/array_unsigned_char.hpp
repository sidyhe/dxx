//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2014-2015 KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_CPP11_ARRAY_UNSIGNED_CHAR_HPP
#define MSGPACK_V1_TYPE_CPP11_ARRAY_UNSIGNED_CHAR_HPP

#include "msgpack/versioning.hpp"
#include "msgpack/adaptor/adaptor_base.hpp"
#include "msgpack/adaptor/check_container_size.hpp"

#include <eastl/array.h>
#include <cstring>

namespace msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

template <std::size_t N>
struct convert<eastl::array<unsigned char, N>> {
    msgpack::object const& operator()(msgpack::object const& o, eastl::array<unsigned char, N>& v) const {
        switch (o.type) {
        case msgpack::type::BIN:
            if(o.via.bin.size > N) { ExRaiseStatus(EMSGPACK_TYPE_ERROR); }
            std::memcpy(v.data(), o.via.bin.ptr, o.via.bin.size);
            break;
        case msgpack::type::STR:
            if(o.via.str.size > N) { ExRaiseStatus(EMSGPACK_TYPE_ERROR); }
            std::memcpy(v.data(), o.via.str.ptr, N);
            break;
        default:
            ExRaiseStatus(EMSGPACK_TYPE_ERROR);
            break;
        }
        return o;
    }
};

template <>
struct convert<eastl::array<unsigned char, 0>> {
    msgpack::object const& operator()(msgpack::object const& o, eastl::array<unsigned char, 0>&) const {
        return o;
    }
};

template <std::size_t N>
struct pack<eastl::array<unsigned char, N>> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, const eastl::array<unsigned char, N>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_bin(size);
        o.pack_bin_body(reinterpret_cast<char const*>(v.data()), size);

        return o;
    }
};

template <std::size_t N>
struct object<eastl::array<unsigned char, N>> {
    void operator()(msgpack::object& o, const eastl::array<unsigned char, N>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.type = msgpack::type::BIN;
        o.via.bin.ptr = reinterpret_cast<char const*>(v.data());
        o.via.bin.size = size;
    }
};

template <std::size_t N>
struct object_with_zone<eastl::array<unsigned char, N>> {
    void operator()(msgpack::object::with_zone& o, const eastl::array<unsigned char, N>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.type = msgpack::type::BIN;
        char* ptr = static_cast<char*>(o.zone.allocate_align(size, MSGPACK_ZONE_ALIGNOF(char)));
        o.via.bin.ptr = ptr;
        o.via.bin.size = size;
        std::memcpy(ptr, v.data(), size);
    }
};

} // namespace adaptor

/// @cond
} // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

} // namespace msgpack

#endif // MSGPACK_V1_TYPE_CPP11_ARRAY_UNSIGNED_CHAR_HPP
