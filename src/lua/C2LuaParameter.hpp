#pragma once

#include "LuaParameterTraits.hpp"

#include "LuaInstance.hpp"

#include <MatchRange.hpp>

#include <lua.hpp>

#include <memory>
#include <vector>

extern LuaInstance currentLuaInstance( lua_State * L);

template <class T> struct C2LuaParameter;

template <> struct C2LuaParameter<P<bool>>
{
 using convertedType = bool;
 static int doIt(lua_State *L, int offset, convertedType value) 
 {
  lua_pushboolean(L, value);
  return 1;
 }
};

template <> struct C2LuaParameter<P<long>>
{
 using convertedType = long;
 static int doIt(lua_State *L, int offset, convertedType value) 
 {
  lua_pushinteger(L, value);
  return 1;
 }
};

template <> struct C2LuaParameter<P<std::shared_ptr<std::vector<std::string>>>>
{
 using convertedType = std::shared_ptr<std::vector<std::string>>;
 static int doIt(lua_State *L, int offset, convertedType values) 
 {
  for( const auto & value : *values)
  {
   lua_pushstring(L, value.c_str());
  }

  return values->size();
 }
};

template <> struct C2LuaParameter<P<std::shared_ptr<std::vector<MatchRange<long>>>>>
{
 using convertedType = std::shared_ptr<std::vector<MatchRange<long>>>;

 static int doIt(lua_State *L, int offset, convertedType values) 
 {
  for( const auto & matchRange : *values)
  {
   const MatchRange<long>::I & i (matchRange.i());

   lua_createtable(L, 0, 5);
   int table = lua_gettop(L);

   lua_pushstring( L, "name");
   lua_pushstring( L, i.namingWeakOrdered.name.c_str());
   lua_settable( L, table); 

   lua_pushstring( L, "begin.begin");
   lua_pushinteger( L, i.d.begin.begin);
   lua_settable( L, table); 

   lua_pushstring( L, "begin.end");
   lua_pushinteger( L, i.d.begin.end);
   lua_settable( L, table); 

   lua_pushstring( L, "end.begin");
   lua_pushinteger( L, i.d.end.begin);
   lua_settable( L, table); 

   lua_pushstring( L, "end.end");
   lua_pushinteger( L, i.d.end.end);
   lua_settable( L, table); 
  }

  return values->size();
 }
};

template <class T > struct C2LuaParameter<V<T>> 
{
 using convertedType = T;
 static int doIt(lua_State *L, int offset, convertedType value) 
 {
  LuaInstance li { currentLuaInstance(L) };
  li.set(std::make_unique<Any<T>>(value));
  return 1;
 }
};

template <class T, class ... Tadditional > struct C2LuaParameter<V<T, Tadditional...>>
{
 using convertedType = T;
 static int doIt(lua_State *L, int offset, convertedType value)
 {
  LuaInstance li { currentLuaInstance(L) };

  std::unique_ptr<Any<T>> returnValue(std::make_unique<Any<T>>(value));

  returnValue -> template addTypes<Tadditional...>();

  li.set(std::move(returnValue));

  return 1;
 }
};

