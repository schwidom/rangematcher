#include "LuaTest.hpp"

#include "LuaBase.hpp"

#include <iostream>
#include <string>

namespace {

bool wasCalledFromLua{false};

int calledFromLua(lua_State *L)
{
 wasCalledFromLua = true;
 return 0;
}

void test001()
{
 std::cout << __FILE__ << " " << __func__ <<  std::endl;

 LuaBase lb;

 lua_register( lb.getLua(), "callCFunction", calledFromLua);

 wasCalledFromLua = false;

 int res = luaL_dofile( lb.getLua(), "testfiles/007_callCFunction.lua");

 std::cout << (0 == res) <<  std::endl;
 std::cout << wasCalledFromLua <<  std::endl;
};

void test002()
{
 std::cout << __FILE__ << " " << __func__ <<  std::endl;

 LuaBase lb;

 lua_register( lb.getLua(), "callCFunction", calledFromLua);

 wasCalledFromLua = false;

 int res = luaL_dostring( lb.getLua(), "callCFunction()");

 std::cout << (0 == res) <<  std::endl;
 std::cout << wasCalledFromLua <<  std::endl;
};

void test003()
{
 std::cout << __FILE__ << " " << __func__ <<  std::endl;

 LuaBase lb;

 luaL_openlibs( lb.getLua());

 int res = luaL_dostring( lb.getLua(), "print(1)");

 std::cout << (0 == res) <<  std::endl;
 
};

void test004()
{
 std::cout << __FILE__ << " " << __func__ <<  std::endl;

 LuaBase lb;

 luaL_openlibs( lb.getLua());

 std::string code{
 R"lua(
  print(1)
  print(1)
 )lua" 
 };

 int res = luaL_dostring( lb.getLua(), code.c_str() );

 std::cout << (0 == res) <<  std::endl;
 
};

std::string handledString{};

int handleLuaString(lua_State *L)
{
 int numberOfArguments = lua_gettop(L);

 if( 1 != numberOfArguments)
 {
  lua_pushstring(L, "Incorrect number of arguments to 'handleLuaString'");
  lua_error(L);
 }

 size_t strLen{};
 const char * strPtr = lua_tolstring(L, 1, &strLen);
 handledString = {strPtr, strLen};

 return 0;
}

void test005()
{
 std::cout << __FILE__ << " " << __func__ <<  std::endl;

 LuaBase lb;

 luaL_openlibs( lb.getLua());

 lua_register( lb.getLua(), "handleLuaString", handleLuaString);
 
 {
  handledString = "";

  int res = luaL_dostring( lb.getLua(), "handleLuaString('abc')" );

  std::cout << ("abc" == handledString) <<  std::endl;
  std::cout << (0 == res) <<  std::endl;
 }

 {
  handledString = "";

  int res = luaL_dostring( lb.getLua(), "handleLuaString('abc\\0def')" );

  std::cout << ( 7 == handledString.size()) <<  std::endl;
  std::cout << ((std::string("abc") + '\0' + std::string("def")) == handledString) << std::endl;
  std::cout << (0 == res) <<  std::endl;
 }

}

}


void LuaTest::allTests()
{
 test001();
 test002();
 test003();
 test004();
 test005();
};

