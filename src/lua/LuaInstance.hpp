#pragma once

#include "RangeMatcherLuaRuntime.hpp"

#include <Any.hpp> // TODO : separate any and anybase

#include <lua.hpp>

#include <memory> 

#include <cstddef> // size_t

class LuaInstance
{
public:
 LuaInstance( lua_State* L, RangeMatcherLuaRuntime & rangeMatcherLuaRuntime);

 void raiseLuaError( std::string msg);

 void chkArguments( int n, std::string functionName);

 int chkArguments( int nMin, int nMax, std::string functionName);

 template <class T> T * get(size_t idx)
 {
  return m_RangeMatcherLuaRuntime.vectorOfObjects.at(idx)->get<T>();
 }

 void set(std::unique_ptr<AnyBase> anyBase) 
 {
  m_RangeMatcherLuaRuntime.vectorOfObjects.push_back( std::move(anyBase));
  lua_pushinteger(m_L, m_RangeMatcherLuaRuntime.vectorOfObjects.size() - 1);
 }

private:
 
 lua_State * m_L;
 RangeMatcherLuaRuntime & m_RangeMatcherLuaRuntime;
};
