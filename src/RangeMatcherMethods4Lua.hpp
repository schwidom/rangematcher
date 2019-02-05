#pragma once

#include "Any.hpp"
#include "LuaBase.hpp"

#include <map>
#include <memory>
#include <vector>

class RangeMatcherMethods4Lua
{
public:

 RangeMatcherMethods4Lua(); // TODO : StreamPair

 void registerMethods2LuaBase(std::weak_ptr<LuaBase> luaBaseWeak);

 const std::string & getLastErrorMessage() const;

private:

 std::map<lua_State*,std::weak_ptr<LuaBase>> m_MapLua2LuaBase;
};

