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

 std::vector<lua_State*> collectGarbage() // TODO : to cpp
 {

  std::vector<lua_State*> ret;

  for( const auto & kvp : m_MapLua2LuaBase)
  {
   if( !kvp.second.lock())
   {
    ret.push_back(kvp.first);
   }
  }

  for( const auto & key : ret) // TODO : use algorithm
  {
   m_MapLua2LuaBase.erase(key);
  }

  return ret;
 }

 std::map<lua_State*,std::weak_ptr<LuaBase>> m_MapLua2LuaBase; // TODO : private or move away

private:

};

