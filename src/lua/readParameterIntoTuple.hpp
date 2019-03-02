#pragma once

#include "Lua2CParameters.hpp"

template <class T> typename Lua2CParameters<T>::convertedType readParameterIntoTuple(lua_State *L)
{
 LuaInstance li { currentLuaInstance(L)};
 int numberOfLuaArguments{Lua2CParameters<T>::numberOfLuaArguments};
 li.chkArguments( numberOfLuaArguments, __func__);

 auto ret = Lua2CParameters<T>::doIt(L, 1);

 lua_pop(L, numberOfLuaArguments);

 return ret;
}

