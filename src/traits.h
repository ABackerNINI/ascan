#ifndef __ASCAN_TRAITS_H__
#define __ASCAN_TRAITS_H__

#include <type_traits>

#if __cplusplus < 202002L
namespace std {
// C++20 std::remove_cvref_t
template <typename _Tp> using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<_Tp>>;
} // namespace std
#endif

// Check if a type is a container.

template <typename Container> struct is_container : std::false_type {};

template <template <typename...> class Container, typename... Args>
struct is_container<Container<Args...>> : std::true_type {};

template <typename T> inline constexpr bool is_container_v = is_container<T>::value;

// Check if a type has a member function empty().

template <typename T, typename = void> struct has_empty : std::false_type {};

template <typename T> struct has_empty<T, std::void_t<decltype(std::declval<T>().empty())>> : std::true_type {};

template <typename T> inline constexpr bool has_empty_v = has_empty<T>::value;

#ifdef DEBUG
    #include <map>
    #include <string>
    #include <vector>
static_assert(is_container_v<int> == false, "int is not a container");
static_assert(is_container_v<std::string> == true, "string is a container");
static_assert(is_container_v<std::vector<int>> == true, "vector is a container");
static_assert(is_container_v<std::map<int, std::string>> == true, "map is a container");
static_assert(has_empty_v<int> == false, "int has no empty()");
static_assert(has_empty_v<std::string> == true, "string has empty()");
static_assert(has_empty_v<std::vector<int>> == true, "vector has empty()");
static_assert(has_empty_v<std::map<int, std::string>> == true, "map has empty()");
#endif

#endif // __ASCAN_TRAITS_H__
