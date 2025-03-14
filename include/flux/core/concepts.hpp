
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_CORE_CONCEPTS_HPP_INCLUDED
#define FLUX_CORE_CONCEPTS_HPP_INCLUDED

#include <flux/core/utils.hpp>

#include <compare>
#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <tuple>
#include <type_traits>

#if defined(__cpp_lib_ranges_zip) && (__cpp_lib_ranges_zip >= 202110L)
#define FLUX_HAVE_CPP23_TUPLE_COMMON_REF
#endif

namespace flux {

/*
 * Cursor concepts
 */
FLUX_EXPORT
template <typename Cur>
concept cursor = std::movable<Cur>;

FLUX_EXPORT
template <typename Cur>
concept regular_cursor = cursor<Cur> && std::regular<Cur>;

FLUX_EXPORT
template <typename Cur>
concept ordered_cursor =
    regular_cursor<Cur> &&
    std::totally_ordered<Cur>;

/*
 * Sequence concepts and associated types
 */

FLUX_EXPORT
template <typename T>
struct sequence_traits;

namespace detail {

template <typename T>
using traits_t = sequence_traits<std::remove_cvref_t<T>>;

} // namespace detail

FLUX_EXPORT
template <typename Seq>
using cursor_t = decltype(detail::traits_t<Seq>::first(FLUX_DECLVAL(Seq&)));

FLUX_EXPORT
template <typename Seq>
using element_t = decltype(detail::traits_t<Seq>::read_at(FLUX_DECLVAL(Seq&), FLUX_DECLVAL(cursor_t<Seq> const&)));

namespace detail {

template <typename T>
concept has_element_type = requires { typename element_t<T>; };

template <has_element_type T>
struct value_type { using type = std::remove_cvref_t<element_t<T>>; };

template <has_element_type T>
    requires requires { typename traits_t<T>::value_type; }
struct value_type<T> { using type = typename traits_t<T>::value_type; };

template <has_element_type T>
    requires requires { traits_t<T>::using_primary_template; } &&
             requires { typename T::value_type; }
struct value_type<T> { using type = typename T::value_type; };

template <has_element_type T>
struct rvalue_element_type {
    using type = std::conditional_t<std::is_lvalue_reference_v<element_t<T>>,
                                    std::add_rvalue_reference_t<std::remove_reference_t<element_t<T>>>,
                                    element_t<T>>;
};

template <typename Seq>
concept has_move_at = requires (Seq& seq, cursor_t<Seq> const& cur) {
   { traits_t<Seq>::move_at(seq, cur) };
};

template <has_element_type T>
    requires has_move_at<T>
struct rvalue_element_type<T> {
    using type = decltype(traits_t<T>::move_at(FLUX_DECLVAL(T&), FLUX_DECLVAL(cursor_t<T> const&)));
};

} // namespace detail

FLUX_EXPORT
template <typename Seq>
using value_t = typename detail::value_type<Seq>::type;

FLUX_EXPORT
using distance_t = flux::config::int_type;

FLUX_EXPORT
using index_t = flux::config::int_type;

FLUX_EXPORT
template <typename Seq>
using rvalue_element_t = typename detail::rvalue_element_type<Seq>::type;

FLUX_EXPORT
template <typename Seq>
using common_element_t = std::common_reference_t<element_t<Seq>, value_t<Seq>&>;

FLUX_EXPORT
template <typename Seq>
using const_element_t = std::common_reference_t<value_t<Seq> const&&, element_t<Seq>>;

namespace detail {

template <typename B>
concept boolean_testable =
    std::convertible_to<B, bool> &&
    requires (B&& b) {
        { !FLUX_FWD(b) } -> std::convertible_to<bool>;
    };

template <typename T>
using with_ref = T&;

template <typename T>
concept can_reference = requires { typename with_ref<T>; };

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept sequence_concept =
    requires (Seq& seq) {
        { Traits::first(seq) } -> cursor;
    } &&
    requires (Seq& seq, cursor_t<Seq> const& cur) {
        { Traits::is_last(seq, cur) } -> boolean_testable;
        { Traits::read_at(seq, cur) } -> can_reference;
    } &&
    requires (Seq& seq, cursor_t<Seq>& cur) {
        { Traits::inc(seq, cur) };
    } &&
#ifdef FLUX_HAVE_CPP23_TUPLE_COMMON_REF
    std::common_reference_with<element_t<Seq>&&, value_t<Seq>&> &&
    std::common_reference_with<rvalue_element_t<Seq>&&, value_t<Seq> const&> &&
#endif
    std::common_reference_with<element_t<Seq>&&, rvalue_element_t<Seq>&&>;

} // namespace detail

FLUX_EXPORT
template <typename Seq>
concept sequence = detail::sequence_concept<Seq>;

namespace detail {

template <typename>
inline constexpr bool disable_multipass = false;

template <typename T>
    requires requires { T::disable_multipass; } &&
             decays_to<decltype(T::disable_multipass), bool>
inline constexpr bool disable_multipass<T> = T::disable_multipass;

} // namespace detail

FLUX_EXPORT
template <typename Seq>
concept multipass_sequence =
    sequence<Seq> && regular_cursor<cursor_t<Seq>> &&
    !detail::disable_multipass<detail::traits_t<Seq>>;

namespace detail {

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept bidirectional_sequence_concept =
    multipass_sequence<Seq> &&
    requires (Seq& seq, cursor_t<Seq>& cur) {
        { Traits::dec(seq, cur) };
    };

} // namespace detail

FLUX_EXPORT
template <typename Seq>
concept bidirectional_sequence = detail::bidirectional_sequence_concept<Seq>;

namespace detail {

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept random_access_sequence_concept =
    bidirectional_sequence<Seq> && ordered_cursor<cursor_t<Seq>> &&
    requires (Seq& seq, cursor_t<Seq>& cur, distance_t offset) {
        { Traits::inc(seq, cur, offset) };
    } &&
    requires (Seq& seq, cursor_t<Seq> const& cur) {
        { Traits::distance(seq, cur, cur) } -> std::convertible_to<distance_t>;
    };

} // namespace detail

FLUX_EXPORT
template <typename Seq>
concept random_access_sequence = detail::random_access_sequence_concept<Seq>;

namespace detail {

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept bounded_sequence_concept =
    sequence<Seq> &&
    requires (Seq& seq) {
        { Traits::last(seq) } -> std::same_as<cursor_t<Seq>>;
    };

} // namespace detail

FLUX_EXPORT
template <typename Seq>
concept bounded_sequence = detail::bounded_sequence_concept<Seq>;

namespace detail {

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept contiguous_sequence_concept =
    random_access_sequence<Seq> &&
    bounded_sequence<Seq> &&
    std::is_lvalue_reference_v<element_t<Seq>> &&
    std::same_as<value_t<Seq>, std::remove_cvref_t<element_t<Seq>>> &&
    requires (Seq& seq) {
        { Traits::data(seq) } -> std::same_as<std::add_pointer_t<element_t<Seq>>>;
    };

} // namespace detail

FLUX_EXPORT
template <typename Seq>
concept contiguous_sequence = detail::contiguous_sequence_concept<Seq>;

namespace detail {

template <typename Seq, typename Traits = sequence_traits<std::remove_cvref_t<Seq>>>
concept sized_sequence_concept =
    sequence<Seq> &&
    (requires (Seq& seq) {
        { Traits::size(seq) } -> std::convertible_to<distance_t>;
    } || (
        random_access_sequence<Seq> && bounded_sequence<Seq>
    ));

} // namespace detail

FLUX_EXPORT
template <typename Seq>
concept sized_sequence = detail::sized_sequence_concept<Seq>;

FLUX_EXPORT
template <typename Seq, typename T>
concept writable_sequence_of =
    sequence<Seq> &&
    requires (element_t<Seq> elem, T&& item) {
        { elem = FLUX_FWD(item) } -> std::same_as<element_t<Seq>&>;
    };

namespace detail {

template <typename>
inline constexpr bool is_infinite_seq = false;

template <typename T>
    requires requires { T::is_infinite; } &&
                 decays_to<decltype(T::is_infinite), bool>
inline constexpr bool is_infinite_seq<T> = T::is_infinite;

}

FLUX_EXPORT
template <typename Seq>
concept infinite_sequence =
    sequence<Seq> &&
    detail::is_infinite_seq<detail::traits_t<Seq>>;

FLUX_EXPORT
template <typename Seq>
concept read_only_sequence =
    sequence<Seq> &&
    std::same_as<element_t<Seq>, const_element_t<Seq>>;

namespace detail {

template <typename T, typename R = std::remove_cvref_t<T>>
constexpr bool is_ilist = false;

template <typename T, typename E>
constexpr bool is_ilist<T, std::initializer_list<E>> = true;

template <typename Seq>
concept rvalue_sequence =
    std::is_object_v<Seq> &&
    std::move_constructible<Seq> &&
    sequence<Seq>;

template <typename Seq>
concept trivially_copyable_sequence =
    std::copyable<Seq> &&
    std::is_trivially_copyable_v<Seq> &&
    sequence<Seq>;

}

FLUX_EXPORT
template <typename Seq>
concept adaptable_sequence =
    (detail::rvalue_sequence<Seq>
         || (std::is_lvalue_reference_v<Seq> &&
             detail::trivially_copyable_sequence<std::decay_t<Seq>>)) &&
    !detail::is_ilist<Seq>;

FLUX_EXPORT
template <typename D>
struct inline_sequence_base;

namespace detail {

template <typename T, typename U>
    requires (!std::same_as<T, inline_sequence_base<U>>)
void derived_from_inline_sequence_base_test(T const&, inline_sequence_base<U> const&);

template <typename T>
concept derived_from_inline_sequence_base = requires(T t) {
    derived_from_inline_sequence_base_test(t, t);
};

} // namespace detail


/*
 * Default sequence_traits implementation
 */
namespace detail {

template <typename T>
concept has_nested_sequence_traits =
    requires { typename T::flux_sequence_traits; } &&
    std::is_class_v<typename T::flux_sequence_traits>;

}

template <typename T>
    requires detail::has_nested_sequence_traits<T>
struct sequence_traits<T> : T::flux_sequence_traits {};


} // namespace flux

#endif // FLUX_CORE_CONCEPTS_HPP_INCLUDED
