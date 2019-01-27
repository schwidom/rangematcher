#pragma once

#include <lua.hpp>

class LuaBase
{
public:
 LuaBase();

 virtual ~LuaBase();

 lua_State* getLua();

private:
 lua_State* m_LuaState{nullptr};

};
