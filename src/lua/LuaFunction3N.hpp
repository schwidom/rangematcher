#pragma once

#include "FunctionType3.hpp"
#include "LuaInstance.hpp"
#include "readParameterIntoTupleN.hpp"
#include "writeParameterFromTuple.hpp"

extern LuaInstance currentLuaInstance( lua_State * L);

#include <list>
#include <tuple> // tuple_size
#include <type_traits> // enable_if

#include <limits> // numeric_limits

template <class T_CallingParameter, class T_ReturningParameter, 
 typename FunctionType3<T_CallingParameter, T_ReturningParameter>::type FunctionOfInterest,
 typename std::enable_if<Lua2CParameters<typename T_CallingParameter::type>::numberOfLuaArguments >= 1,int>::type = 0>
struct LuaFunction3N 
{
 // typedef int (*lua_CFunction) (lua_State *L);
 static int call(lua_State *L)
 {

  LuaInstance li { currentLuaInstance(L)};
  
  using FunctionType3Instance = FunctionType3<T_CallingParameter, T_ReturningParameter>;

  int ret = 0;

  int numberOfArguments = li.chkArguments( 0, std::numeric_limits<int>::max(), __func__);

  std::list< typename Lua2CParameters<typename T_CallingParameter::type>::convertedType > queue;

  int inputTupleSize{std::tuple_size<typename T_CallingParameter::type>::value}; 

  if( 0 == inputTupleSize && 0 != numberOfArguments)
  { 
   li.raiseLuaError( "function with 0 parameters must get exactly 0 parameters");
  }

  for( int argumentNr = 0 ; argumentNr < numberOfArguments; argumentNr += inputTupleSize)
  {
   auto readin = readParameterIntoTupleN<typename T_CallingParameter::type>(L, 1 + argumentNr);
   queue.push_back(readin);
  }

  lua_pop(L, numberOfArguments); 

  for( auto & readin : queue)
  {
   typename FunctionType3Instance::returnType res = FunctionType3Instance::call(FunctionOfInterest, readin);

   ret += writeParameterFromTuple<typename T_ReturningParameter::type>(L, res);
  }
  
  return ret;
 }
};
