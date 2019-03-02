#pragma once

#include "C2LuaParameters.hpp"
#include "Lua2CParameters.hpp"

#include <tuple>

template<class T_CallingParameter, class T_ReturningParameter, 
 class T_CallingReturnType = typename Lua2CParameters<typename T_CallingParameter::type>::convertedType,
 class T_ReturningReturnType = typename C2LuaParameters<typename T_ReturningParameter::type>::convertedType> struct FunctionType3;

template<class ... T_CallingParameterP, class ... T_ReturningParameterP, 
 class ... T_CallingConvertedTypeP, class ... T_ReturningConvertedTypeP> 
struct FunctionType3<CallingParameter<T_CallingParameterP...>, ReturningParameter<T_ReturningParameterP...>
 , std::tuple<T_CallingConvertedTypeP...>, std::tuple<T_ReturningConvertedTypeP...>>
{
 using type = std::tuple<T_ReturningConvertedTypeP...> (*)(T_CallingConvertedTypeP...); 

 using returnType = std::tuple<T_ReturningConvertedTypeP...>; 

 private:
 using callType = std::tuple<T_CallingConvertedTypeP...>;

 template <size_t ... I>
 static returnType call2(type function, std::index_sequence<I...> is, callType parametersTuple)
 {
  return function(std::get<I>(parametersTuple)...);
 }

 public:
 static returnType call(type function, callType parametersTuple)
 {
  return call2(function, std::make_index_sequence<sizeof...(T_CallingConvertedTypeP)>(), parametersTuple);
 }
};

