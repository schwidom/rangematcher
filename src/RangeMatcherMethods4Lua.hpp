#pragma once

#include "Any.hpp"
#include "LuaBase.hpp"

#include <memory>
#include <vector>

class RangeMatcherMethods4Lua
{
public:

 RangeMatcherMethods4Lua(); // TODO : StreamPair

 void propagate(LuaBase & luaBase);

 const std::string & getLastErrorMessage() const;

private:

 bool m_Propagated{false};
};

