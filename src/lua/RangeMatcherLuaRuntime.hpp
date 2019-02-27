#pragma once

#include <LuaBase.hpp>

#include <RangeMatcherMethods4Lua.hpp>

#include <map>
#include <vector>

struct RangeMatcherLuaRuntime
{
 RangeMatcherLuaRuntime(RangeMatcherMethods4Lua & rangeMatcherMethods4Lua)
 : rangeMatcherMethods4Lua{rangeMatcherMethods4Lua} {}

 RangeMatcherMethods4Lua & rangeMatcherMethods4Lua;
 bool debug{false};
 std::vector<std::unique_ptr<AnyBase>> vectorOfObjects{};
 std::string lastErrorMessage{};
 std::vector<std::string> vectorOfRegisteredLuaFunctions{};
 std::map<std::string,std::string> mapOfHelpOfRegisteredLuaFunctions{};
 std::map<lua_State*,std::weak_ptr<LuaBase>> mapLua2LuaBase; 
};

