//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
// Copyright (c) 2022 Dmitry Arkhipov (grisumbras@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_VALUE_FROM_HPP
#define BOOST_JSON_DETAIL_VALUE_FROM_HPP

#include <boost/json/storage_ptr.hpp>
#include <boost/json/value.hpp>
#include <boost/json/detail/value_traits.hpp>
#include <boost/mp11/algorithm.hpp>

BOOST_JSON_NS_BEGIN

namespace detail {

template <class T>
struct append_tuple_element {
    array& arr;
    T&& t;

    template<std::size_t I>
    void
    operator()(mp11::mp_size_t<I>) const
    {
        using std::get;
        arr.emplace_back(value_from(
            get<I>(std::forward<T>(t)), arr.storage()));
    }
};

//----------------------------------------------------------
// User-provided conversion

template<class T>
void
value_from_helper(
    value& jv,
    T&& from,
    user_conversion_tag)
{
    tag_invoke(value_from_tag(), jv, std::forward<T>(from));
}


//----------------------------------------------------------
// Native conversion

template<class T>
void
value_from_helper(
    value& jv,
    T&& from,
    native_conversion_tag)
{
    jv = std::forward<T>(from);
}

template<class T>
void
value_from_helper(
    value& jv,
    T&&,
    nullptr_conversion_tag)
{
    // do nothing
    BOOST_ASSERT(jv.is_null());
    (void)jv;
}

// string-like types
template<class T>
void
value_from_helper(
    value& jv,
    T&& from,
    string_like_conversion_tag)
{
    jv.emplace_string().assign(
        from.data(), from.size());
}

// map-like types
template<class T>
void
value_from_helper(
    value& jv,
    T&& from,
    map_like_conversion_tag)
{
    using std::get;
    object& obj = jv.emplace_object();
    obj.reserve(detail::try_size(from, size_implementation<T>()));
    for (auto&& elem : from)
        obj.emplace(get<0>(elem), value_from(
            get<1>(elem), obj.storage()));
}

// ranges
template<class T>
void
value_from_helper(
    value& jv,
    T&& from,
    sequence_conversion_tag)
{
    array& result = jv.emplace_array();
    result.reserve(detail::try_size(from, size_implementation<T>()));
    for (auto&& elem : from)
        result.emplace_back(
            value_from(elem, result.storage()));
}

// tuple-like types
template<class T>
void
value_from_helper(
    value& jv,
    T&& from,
    tuple_conversion_tag)
{
    constexpr std::size_t n =
        std::tuple_size<remove_cvref<T>>::value;
    array& arr = jv.emplace_array();
    arr.reserve(n);
    mp11::mp_for_each<mp11::mp_iota_c<n>>(
        append_tuple_element<T>{arr, std::forward<T>(from)});
}

// no suitable conversion implementation
template<class T>
void
value_from_helper(
    value&,
    T&&,
    no_conversion_tag)
{
    static_assert(
        !std::is_same<T, T>::value,
        "No suitable tag_invoke overload found for the type");
}

} // detail
BOOST_JSON_NS_END

#endif
