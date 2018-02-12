//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2016 FURUHASHI Sadayuki and KONDO Takatoshi
//
//    Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//    http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef MSGPACK_V1_TYPE_MAP_HPP
#define MSGPACK_V1_TYPE_MAP_HPP

#include "msgpack/v1/adaptor/map_decl.hpp"
#include "msgpack/adaptor/adaptor_base.hpp"

#include <eastl/map.h>
#include <eastl/vector.h>

namespace msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond

namespace type {

template <typename K, typename V, typename Compare, typename Alloc>
class assoc_vector : public eastl::vector< std::pair<K, V>, Alloc > {
#if !defined(MSGPACK_USE_CPP03)
    using eastl::vector<std::pair<K, V>, Alloc>::vector;
#endif // !defined(MSGPACK_USE_CPP03)
};

namespace detail {
    template <typename K, typename V, typename Compare, typename Alloc>
    struct pair_first_less {
        bool operator() (const std::pair<K, V>& x, const std::pair<K, V>& y) const
            { return Compare()(x.first, y.first); }
    };
}

}  //namespace type

namespace adaptor {

#if !defined(MSGPACK_USE_CPP03)

template <typename K, typename V, typename Compare, typename Alloc>
struct as<
    type::assoc_vector<K, V, Compare, Alloc>,
    typename std::enable_if<msgpack::has_as<K>::value || msgpack::has_as<V>::value>::type> {
    type::assoc_vector<K, V, Compare, Alloc> operator()(msgpack::object const& o) const {
        if (o.type != msgpack::type::MAP) { ExRaiseStatus(EMSGPACK_TYPE_ERROR); }
        type::assoc_vector<K, V, Compare, Alloc> v;
        v.reserve(o.via.map.size);
        msgpack::object_kv* p = o.via.map.ptr;
        msgpack::object_kv* const pend = o.via.map.ptr + o.via.map.size;
        for (; p < pend; ++p) {
            v.emplace_back(p->key.as<K>(), p->val.as<V>());
        }
        std::sort(v.begin(), v.end(), type::detail::pair_first_less<K, V, Compare, Alloc>());
        return v;
    }
};

#endif // !defined(MSGPACK_USE_CPP03)

template <typename K, typename V, typename Compare, typename Alloc>
struct convert<type::assoc_vector<K, V, Compare, Alloc> > {
    msgpack::object const& operator()(msgpack::object const& o, type::assoc_vector<K, V, Compare, Alloc>& v) const {
        if (o.type != msgpack::type::MAP) { ExRaiseStatus(EMSGPACK_TYPE_ERROR); }
        v.resize(o.via.map.size);
        if (o.via.map.size != 0) {
            msgpack::object_kv* p = o.via.map.ptr;
            msgpack::object_kv* const pend = o.via.map.ptr + o.via.map.size;
            std::pair<K, V>* it(&v.front());
            for (; p < pend; ++p, ++it) {
                p->key.convert(it->first);
                p->val.convert(it->second);
            }
            std::sort(v.begin(), v.end(), type::detail::pair_first_less<K, V, Compare, Alloc>());
        }
        return o;
    }
};

template <typename K, typename V, typename Compare, typename Alloc>
struct pack<type::assoc_vector<K, V, Compare, Alloc> > {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, const type::assoc_vector<K, V, Compare, Alloc>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_map(size);
        for (typename type::assoc_vector<K, V, Compare, Alloc>::const_iterator it(v.begin()), it_end(v.end());
            it != it_end; ++it) {
            o.pack(it->first);
            o.pack(it->second);
        }
        return o;
    }
};

template <typename K, typename V, typename Compare, typename Alloc>
struct object_with_zone<type::assoc_vector<K, V, Compare, Alloc> > {
    void operator()(msgpack::object::with_zone& o, const type::assoc_vector<K, V, Compare, Alloc>& v) const {
        o.type = msgpack::type::MAP;
        if (v.empty()) {
            o.via.map.ptr  = MSGPACK_NULLPTR;
            o.via.map.size = 0;
        }
        else {
            uint32_t size = checked_get_container_size(v.size());
            msgpack::object_kv* p = static_cast<msgpack::object_kv*>(o.zone.allocate_align(sizeof(msgpack::object_kv)*size, MSGPACK_ZONE_ALIGNOF(msgpack::object_kv)));
            msgpack::object_kv* const pend = p + size;
            o.via.map.ptr  = p;
            o.via.map.size = size;
            typename type::assoc_vector<K, V, Compare, Alloc>::const_iterator it(v.begin());
            do {
                p->key = msgpack::object(it->first, o.zone);
                p->val = msgpack::object(it->second, o.zone);
                ++p;
                ++it;
            } while(p < pend);
        }
    }
};

#if !defined(MSGPACK_USE_CPP03)

template <typename K, typename V, typename Compare, typename Alloc>
struct as<
    eastl::map<K, V, Compare, Alloc>,
    typename std::enable_if<msgpack::has_as<K>::value || msgpack::has_as<V>::value>::type> {
    eastl::map<K, V, Compare, Alloc> operator()(msgpack::object const& o) const {
        if (o.type != msgpack::type::MAP) { ExRaiseStatus(EMSGPACK_TYPE_ERROR); }
        msgpack::object_kv* p(o.via.map.ptr);
        msgpack::object_kv* const pend(o.via.map.ptr + o.via.map.size);
        eastl::map<K, V, Compare, Alloc> v;
        for (; p != pend; ++p) {
            v.emplace(p->key.as<K>(), p->val.as<V>());
        }
        return v;
    }
};

#endif // !defined(MSGPACK_USE_CPP03)

template <typename K, typename V, typename Compare, typename Alloc>
struct convert<eastl::map<K, V, Compare, Alloc> > {
    msgpack::object const& operator()(msgpack::object const& o, eastl::map<K, V, Compare, Alloc>& v) const {
        if (o.type != msgpack::type::MAP) { ExRaiseStatus(EMSGPACK_TYPE_ERROR); }
        msgpack::object_kv* p(o.via.map.ptr);
        msgpack::object_kv* const pend(o.via.map.ptr + o.via.map.size);
        eastl::map<K, V, Compare, Alloc> tmp;
        for (; p != pend; ++p) {
            K key;
            p->key.convert(key);
#if __cplusplus >= 201103L
            p->val.convert(tmp[std::move(key)]);
#else
            p->val.convert(tmp[key]);
#endif
        }
#if __cplusplus >= 201103L
        v = std::move(tmp);
#else
        tmp.swap(v);
#endif
        return o;
    }
};

template <typename K, typename V, typename Compare, typename Alloc>
struct pack<eastl::map<K, V, Compare, Alloc> > {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, const eastl::map<K, V, Compare, Alloc>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_map(size);
        for (typename eastl::map<K, V, Compare, Alloc>::const_iterator it(v.begin()), it_end(v.end());
            it != it_end; ++it) {
            o.pack(it->first);
            o.pack(it->second);
        }
        return o;
    }
};

template <typename K, typename V, typename Compare, typename Alloc>
struct object_with_zone<eastl::map<K, V, Compare, Alloc> > {
    void operator()(msgpack::object::with_zone& o, const eastl::map<K, V, Compare, Alloc>& v) const {
        o.type = msgpack::type::MAP;
        if (v.empty()) {
            o.via.map.ptr  = MSGPACK_NULLPTR;
            o.via.map.size = 0;
        }
        else {
            uint32_t size = checked_get_container_size(v.size());

            msgpack::object_kv* p = static_cast<msgpack::object_kv*>(o.zone.allocate_align(sizeof(msgpack::object_kv)*size, MSGPACK_ZONE_ALIGNOF(msgpack::object_kv)));
            msgpack::object_kv* const pend = p + size;
            o.via.map.ptr  = p;
            o.via.map.size = size;
            typename eastl::map<K, V, Compare, Alloc>::const_iterator it(v.begin());
            do {
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif // (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) && !defined(__clang__)
                p->key = msgpack::object(it->first, o.zone);
                p->val = msgpack::object(it->second, o.zone);
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif // (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) && !defined(__clang__)
                ++p;
                ++it;
            } while(p < pend);
        }
    }
};

#if !defined(MSGPACK_USE_CPP03)

template <typename K, typename V, typename Compare, typename Alloc>
struct as<
    eastl::multimap<K, V, Compare, Alloc>,
    typename std::enable_if<msgpack::has_as<K>::value || msgpack::has_as<V>::value>::type> {
    eastl::multimap<K, V, Compare, Alloc> operator()(msgpack::object const& o) const {
        if (o.type != msgpack::type::MAP) { ExRaiseStatus(EMSGPACK_TYPE_ERROR); }
        msgpack::object_kv* p(o.via.map.ptr);
        msgpack::object_kv* const pend(o.via.map.ptr + o.via.map.size);
        eastl::multimap<K, V, Compare, Alloc> v;
        for (; p != pend; ++p) {
            v.emplace(p->key.as<K>(), p->val.as<V>());
        }
        return v;
    }
};

#endif // !defined(MSGPACK_USE_CPP03)

template <typename K, typename V, typename Compare, typename Alloc>
struct convert<eastl::multimap<K, V, Compare, Alloc> > {
    msgpack::object const& operator()(msgpack::object const& o, eastl::multimap<K, V, Compare, Alloc>& v) const {
        if (o.type != msgpack::type::MAP) { ExRaiseStatus(EMSGPACK_TYPE_ERROR); }
        msgpack::object_kv* p(o.via.map.ptr);
        msgpack::object_kv* const pend(o.via.map.ptr + o.via.map.size);
        eastl::multimap<K, V, Compare, Alloc> tmp;
        for (; p != pend; ++p) {
            std::pair<K, V> value;
            p->key.convert(value.first);
            p->val.convert(value.second);
#if __cplusplus >= 201103L
            tmp.insert(std::move(value));
#else
            tmp.insert(value);
#endif
        }
#if __cplusplus >= 201103L
        v = std::move(tmp);
#else
        tmp.swap(v);
#endif
        return o;
    }
};

template <typename K, typename V, typename Compare, typename Alloc>
struct pack<eastl::multimap<K, V, Compare, Alloc> > {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, const eastl::multimap<K, V, Compare, Alloc>& v) const {
        uint32_t size = checked_get_container_size(v.size());
        o.pack_map(size);
        for (typename eastl::multimap<K, V, Compare, Alloc>::const_iterator it(v.begin()), it_end(v.end());
            it != it_end; ++it) {
            o.pack(it->first);
            o.pack(it->second);
        }
        return o;
    }
};

template <typename K, typename V, typename Compare, typename Alloc>
struct object_with_zone<eastl::multimap<K, V, Compare, Alloc> > {
    void operator()(msgpack::object::with_zone& o, const eastl::multimap<K, V, Compare, Alloc>& v) const {
        o.type = msgpack::type::MAP;
        if (v.empty()) {
            o.via.map.ptr  = MSGPACK_NULLPTR;
            o.via.map.size = 0;
        }
        else {
            uint32_t size = checked_get_container_size(v.size());
            msgpack::object_kv* p = static_cast<msgpack::object_kv*>(o.zone.allocate_align(sizeof(msgpack::object_kv)*size, MSGPACK_ZONE_ALIGNOF(msgpack::object_kv)));
            msgpack::object_kv* const pend = p + size;
            o.via.map.ptr  = p;
            o.via.map.size = size;
            typename eastl::multimap<K, V, Compare, Alloc>::const_iterator it(v.begin());
            do {
                p->key = msgpack::object(it->first, o.zone);
                p->val = msgpack::object(it->second, o.zone);
                ++p;
                ++it;
            } while(p < pend);
        }
    }
};

} // namespace adaptor

/// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace msgpack

#endif // MSGPACK_V1_TYPE_MAP_HPP
