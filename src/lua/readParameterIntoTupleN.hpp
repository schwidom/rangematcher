#pragma once

#include "Lua2CParameters.hpp"

template <class T> typename Lua2CParameters<T>::convertedType readParameterIntoTupleN(lua_State *L, int offset)
{
 return Lua2CParameters<T>::doIt(L, offset);
}

