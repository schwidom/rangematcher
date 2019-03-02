#pragma once

#include "C2LuaParameter.hpp"

#include <tools.hpp>

#include <tuple>

template <class T> struct C2LuaParameters;

template <class T, class ... Tp> struct C2LuaParameters<std::tuple<T, Tp...>>
{
 using convertedType = typename tuple_type_cat<
  std::tuple<typename C2LuaParameter<T>::convertedType>, 
  typename C2LuaParameters<std::tuple<Tp...>>::convertedType
  >::type;

 // static std::tuple<T, Tp...> doIt(lua_State *L, int offset)
 static int doIt(lua_State *L, int offset, convertedType parameters)
 {
   return 0 
    + C2LuaParameter<T>::doIt(L, offset, std::get<0>(parameters))
    + C2LuaParameters<std::tuple<Tp...>>::doIt(L, 1 + offset, tail(parameters));
 }
};

template <> struct C2LuaParameters<std::tuple<>>
{
 using convertedType = std::tuple<>;

 static int doIt(lua_State *L, int offset, std::tuple<>)
 {
  return 0;
 }
};

