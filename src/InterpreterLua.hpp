#pragma once

#include <lua.hpp>

class InterpreterLua
{
public:
 InterpreterLua();

 ~InterpreterLua();

private:
 lua_State* m_LuaState{nullptr};

};
