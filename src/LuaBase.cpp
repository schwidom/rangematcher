#include "LuaBase.hpp"

#include <stdexcept>

LuaBase::LuaBase()
// :  m_LuaState = lua_open(); // 5.?
: m_LuaState(luaL_newstate())
{
 if( !m_LuaState)
 {
  throw std::runtime_error{"!m_LuaState"};
 }
}

LuaBase::~LuaBase()
{
 if( m_LuaState)
 {
  lua_close(m_LuaState);
 }
 m_LuaState = nullptr;
}

lua_State* LuaBase::getLua()
{
 return m_LuaState;
}


