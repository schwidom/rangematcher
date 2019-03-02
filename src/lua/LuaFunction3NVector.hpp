#pragma once

#include "FunctionType3.hpp"
#include "LuaInstance.hpp"
#include "readParameterIntoTupleN.hpp"
#include "writeParameterFromTuple.hpp"

extern LuaInstance currentLuaInstance( lua_State * L);

#include <list>
#include <tuple> // tuple_size
#include <type_traits> // enable_if
#include <vector>

#include <limits> // numeric_limits

template <class T_CallingParameter, class T_ReturningParameter, 
 typename FunctionType3<T_CallingParameter, T_ReturningParameter>::type FunctionOfInterest,
 typename std::enable_if<Lua2CParameters<typename T_CallingParameter::type>::numberOfLuaArguments >= 1,int>::type = 0>
struct LuaFunction3NVector
{
 // typedef int (*lua_CFunction) (lua_State *L);
 static int call(lua_State *L)
 {

  LuaInstance li { currentLuaInstance(L)};
  
  using FunctionType3Instance = FunctionType3<T_CallingParameter, T_ReturningParameter>;

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

  using NewConvertedReturnType = std::shared_ptr<std::vector<typename FunctionType3Instance::returnType>>;
  
  NewConvertedReturnType returnVector{};
  returnVector = std::make_shared<typename NewConvertedReturnType::element_type>();

  for( auto & readin : queue)
  {
   typename FunctionType3Instance::returnType res = FunctionType3Instance::call(FunctionOfInterest, readin);
   returnVector->push_back(res);
  }
  
  writeParameterFromTuple<std::tuple<V<NewConvertedReturnType>>>(L, std::make_tuple(returnVector));

  return 1;
 }
};
