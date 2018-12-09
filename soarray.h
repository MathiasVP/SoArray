#ifndef SOARRAY_H
#define SOARRAY_H
#include <tuple>

template<typename T, typename U> constexpr size_t offsetOf(U T::*member) {
  return (char*)&((T*)nullptr->*member) - (char*)nullptr;
}

template <std::size_t ... Is>
constexpr auto index_sequence_reverse (std::index_sequence<Is...> const &)
   -> decltype( std::index_sequence<sizeof...(Is)-1U-Is...>{} );

template <std::size_t N>
using make_index_sequence_reverse
   = decltype(index_sequence_reverse(std::make_index_sequence<N>{}));

template <class A, class B, A B::*ptr>
struct FieldBase {
  using type = A;
  const static std::size_t offset = offsetOf(ptr);
};

template<typename T>
struct FieldCount {};

template<typename T, int N>
struct Field {};

template<template<typename...> typename C, typename T, typename Idx>
struct SoArray_impl {};

template<template<typename...> typename C, typename T, int... Is>
struct SoArray_impl<C, T, std::index_sequence<Is...>> {
  std::tuple<C<typename Field<T, Is>::type>...> data;
  using type = std::tuple<typename Field<T, Is>::type...>;
};

template<template<typename...> typename C, typename T>
struct SoArray_ : SoArray_impl<C, T, std::make_index_sequence<FieldCount<T>::value>> {};

template<template<typename...> typename C, typename T, typename Idx>
struct PushBacker {};

template<template<typename...> typename C, typename T, int I, int... Is>
struct PushBacker<C, T, std::index_sequence<I, Is...>> {
  static void push_back_(SoArray_<C, T>& a, T t) {
    std::get<I>(a.data).push_back(*(typename Field<T, I>::type*)((unsigned char*)(&t) + Field<T, I>::offset));
    PushBacker<C, T, std::index_sequence<Is...>>::push_back_(a, t);
  }
};

template<template<typename...> typename C, typename T>
struct PushBacker<C, T, std::index_sequence<>> {
  static void push_back_(SoArray_<C, T>& a, T t) {}
};

template<std::size_t... Is>
struct Call {
  template<typename F, template<typename...> typename C, typename... T>
  static void call(F f, std::size_t i, std::tuple<C<T>...> arr) {
    f(std::get<Is>(arr)[i]...);
  }
};

template<template<typename...> typename C, typename T>
class SoArray {
public:
  SoArray_<C, T> arr;
public:
  void push_back(T t) {
    PushBacker<C, T, make_index_sequence_reverse<FieldCount<T>::value>>::push_back_(arr, t);
  }

  template<std::size_t... Is, typename F>
  void iterate(F f) {
    auto n = std::get<0>(arr.data).size();
    for(int i = 0; i < n; ++i) {
      Call<Is...>::call(f, i, arr.data);
    }
  }
};

#endif
