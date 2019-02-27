#pragma once

#include "LuaInstance.hpp"
#include "LuaParameterTraits.hpp"

#include <lua.hpp>

extern LuaInstance currentLuaInstance( lua_State * L);

template <class T> struct Lua2CParameter;

template <> struct Lua2CParameter<P<lua_State>> // TODO : possibly L<lua_State> (must not get parameter) + returning parameter count as constexpr
{
 using convertedType = lua_State *;

 static constexpr int numberOfLuaArguments = 0;

 static convertedType doIt(lua_State *L, int offset) // length check is already done, pop is later expected
 {
  return L;
 }
};

template <> struct Lua2CParameter<P<long>>
{
 using convertedType = long;

 static constexpr int numberOfLuaArguments = 1;

 static convertedType doIt(lua_State *L, int offset) // length check is already done, pop is later expected
 {
  return lua_tointeger(L, offset); // 1
 }
};

template <> struct Lua2CParameter<P<std::string>>
{
 using convertedType = std::string;

 static constexpr int numberOfLuaArguments = 1;

 static convertedType doIt(lua_State *L, int offset) // length check is already done, pop is later expected
 {
  return lua_tostring(L, offset); // 1
 }
};

template <class T> struct Lua2CParameter<V<T>> 
{
 using convertedType = T;

 static constexpr int numberOfLuaArguments = 1;

 static T doIt(lua_State *L, int offset) 
 {
  LuaInstance li { currentLuaInstance(L)};

  long intIdx{lua_tointeger(L, offset)}; // 1

  auto savedValue(li.get<T>(intIdx));

  if( !savedValue)
  {
   li.raiseLuaError( "wrong type at index " + std::to_string(intIdx) + " argument : " + std::to_string( offset));
  }

  return *savedValue;
 }
};
