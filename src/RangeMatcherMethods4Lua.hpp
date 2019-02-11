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
 RangeMatcherMethods4Lua(RangeMatcherMethods4Lua&) = delete; 
 RangeMatcherMethods4Lua(RangeMatcherMethods4Lua&&) = delete; 
 ~RangeMatcherMethods4Lua();

 void registerMethods2LuaBase(std::weak_ptr<LuaBase> luaBaseWeak);

 const std::string getLastErrorMessage() const;

};

