#include "InterpreterLua.hpp"

InterpreterLua::InterpreterLua()
{
 // m_LuaState = lua_open(); // 5.?
 m_LuaState = luaL_newstate();
}

InterpreterLua::~InterpreterLua()
{
 if( m_LuaState)
 {
  lua_close(m_LuaState);
 }
 m_LuaState = nullptr;
}

