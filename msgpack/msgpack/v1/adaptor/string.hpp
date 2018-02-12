//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2015 FURUHASHI Sadayuki
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_STRING_HPP
#define MSGPACK_V1_TYPE_STRING_HPP

#include "msgpack/versioning.hpp"
#include "msgpack/adaptor/adaptor_base.hpp"
#include "msgpack/adaptor/check_container_size.hpp"

#include <eastl/string.h>
#include <cstring>

namespace msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace adaptor {

template <>
struct convert<eastl::string> {
    msgpack::object const& operator()(msgpack::object const& o, eastl::string& v) const {
        switch (o.type) {
        case msgpack::type::BIN:
            v.assign(o.via.bin.ptr, o.via.bin.size);
            break;
        case msgpack::type::STR:
            v.assign(o.via.str.ptr, o.via.str.size);
            break;
        default:
            ExRaiseStatus(EMSGPACK_TYPE_ERROR);
            break;
        }
        return o;
    }
};

template <>
struct pack<eastl::string> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, const eastl::string& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_str(size);
        o.pack_str_body(v.data(), size);
        return o;
    }
};

template <>
struct object<eastl::string> {
    void operator()(msgpack::object& o, const eastl::string& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.type = msgpack::type::STR;
        o.via.str.ptr = v.data();
        o.via.str.size = size;
    }
};

template <>
struct object_with_zone<eastl::string> {
    void operator()(msgpack::object::with_zone& o, const eastl::string& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.type = msgpack::type::STR;
        char* ptr = static_cast<char*>(o.zone.allocate_align(size, MSGPACK_ZONE_ALIGNOF(char)));
        o.via.str.ptr = ptr;
        o.via.str.size = size;
        std::memcpy(ptr, v.data(), v.size());
    }
};

} // namespace adaptor

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace msgpack

#endif // MSGPACK_V1_TYPE_STRING_HPP
