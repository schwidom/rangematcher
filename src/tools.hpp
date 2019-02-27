#pragma once

#include <tuple> // make_tuple
#include <utility> // make_index_sequence

#include <cstddef> // size_t


////

template < std::size_t... Ns , typename... Ts >
auto tail_impl( std::index_sequence<Ns...> , std::tuple<Ts...> t )
{
 return  std::make_tuple( std::get<Ns+1u>(t)... );
}

template < typename... Ts >
auto tail( std::tuple<Ts...> t )
{
 return  tail_impl( std::make_index_sequence<sizeof...(Ts) - 1u>() , t );
}

////

template<class ... T>struct tuple_type_cat;

template<class T, class ... Tp> struct tuple_type_cat<std::tuple<T>,std::tuple<Tp...>>
{
 using type = std::tuple<T,Tp...>;
};

////

template<class TUPLE, int IDX> struct type_get;

template <class T, class ... Tp> struct type_get<std::tuple<T, Tp...>, 0> 
{
 using type = T;
};

template <class T, class ... Tp, int IDX> struct type_get<std::tuple<T, Tp...>, IDX> 
{
 using type = typename type_get<std::tuple<Tp...>, 1 + IDX>::type;
};

