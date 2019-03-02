#pragma once

#include "FunctionType3.hpp"
#include "LuaInstance.hpp"
#include "readParameterIntoTuple.hpp"
#include "writeParameterFromTuple.hpp"

extern LuaInstance currentLuaInstance( lua_State * L);

template <class T_CallingParameter, class T_ReturningParameter, 
 typename FunctionType3<T_CallingParameter, T_ReturningParameter>::type FunctionOfInterest>
struct LuaFunction3 
{
 // typedef int (*lua_CFunction) (lua_State *L);
 static int call(lua_State *L)
 {
  currentLuaInstance(L);
  
  auto readin = readParameterIntoTuple<typename T_CallingParameter::type>(L); 

  using FunctionType3Instance = FunctionType3<T_CallingParameter, T_ReturningParameter>;

  typename FunctionType3Instance::returnType ret = FunctionType3Instance::call(FunctionOfInterest, readin);

  return writeParameterFromTuple<typename T_ReturningParameter::type>(L, ret);
 }
};

