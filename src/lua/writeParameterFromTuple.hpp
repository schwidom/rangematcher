#pragma once

#include "C2LuaParameters.hpp"

template <class T> int writeParameterFromTuple(lua_State *L, typename C2LuaParameters<T>::convertedType parameters)
{
 return C2LuaParameters<T>::doIt(L, 1, parameters);
}

