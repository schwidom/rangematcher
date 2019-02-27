#pragma once

#include "RangeMatcherLuaRuntime.hpp"

#include <lua.hpp>

class LuaInstance
{
public:
 LuaInstance( lua_State* L, RangeMatcherLuaRuntime & rangeMatcherLuaRuntime);

 void raiseLuaError( std::string msg);

 void chkArguments( int n, std::string functionName);

 int chkArguments( int nMin, int nMax, std::string functionName);

private:
 
 lua_State * m_L;
 RangeMatcherLuaRuntime & m_RangeMatcherLuaRuntime;
};
