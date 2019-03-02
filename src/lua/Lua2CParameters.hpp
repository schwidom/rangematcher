#pragma once

#include "Lua2CParameter.hpp"

#include <tools.hpp>

#include <tuple>

template <class T> struct Lua2CParameters;

template <class T, class ... Tp> struct Lua2CParameters<std::tuple<T, Tp...>>
{
 using convertedType = typename tuple_type_cat<
  std::tuple<typename Lua2CParameter<T>::convertedType>, 
  typename Lua2CParameters<std::tuple<Tp...>>::convertedType
  >::type;

 static constexpr int numberOfLuaArguments = 0 
  + Lua2CParameter<T>::numberOfLuaArguments
  + Lua2CParameters<std::tuple<Tp...>>::numberOfLuaArguments;

 // static std::tuple<T, Tp...> doIt(lua_State *L, int offset)
 static decltype(auto) doIt(lua_State *L, int offset)
 {
  return std::tuple_cat(
   std::make_tuple(Lua2CParameter<T>::doIt(L, offset)), 
   Lua2CParameters<std::tuple<Tp...>>::doIt(L, 1 + offset)
  );
 }
};

template <> struct Lua2CParameters<std::tuple<>>
{
 using convertedType = std::tuple<>;

 static constexpr int numberOfLuaArguments = 0;

 static std::tuple<> doIt(lua_State *L, int offset)
 {
  return std::tuple<>();
 }
};

