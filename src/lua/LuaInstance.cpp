
#include "LuaInstance.hpp"

LuaInstance::LuaInstance( lua_State* L, RangeMatcherLuaRuntime & rangeMatcherLuaRuntime)
 : m_L{L}
 , m_RangeMatcherLuaRuntime{rangeMatcherLuaRuntime}
{
}

void LuaInstance::raiseLuaError( std::string msg)
{
 m_RangeMatcherLuaRuntime.lastErrorMessage = msg;
 lua_pushstring(m_L, m_RangeMatcherLuaRuntime.lastErrorMessage.c_str());
 lua_error(m_L);
}

void LuaInstance::chkArguments( int n, std::string functionName)
{
 int numberOfArguments = lua_gettop(m_L);

 if( n != numberOfArguments)
 {
  raiseLuaError( "Incorrect number of arguments to '"+ functionName +"'");
 }
}

int LuaInstance::chkArguments( int nMin, int nMax, std::string functionName)
{
 int numberOfArguments = lua_gettop(m_L);

 if( !( nMin <= numberOfArguments && numberOfArguments <= nMax))
 {
  raiseLuaError( "Incorrect number of arguments to '"+ functionName +"'");
 }

 return numberOfArguments;
}

