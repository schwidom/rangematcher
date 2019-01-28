#include "NamedPatternRange.hpp"
#include "PatternRegex.hpp"
#include "PatternString.hpp"
#include "RangeMatcherMethods4Lua.hpp"

#include <functional> // bind

#include <stdexcept> // runtime_error

#include <limits> // std::numeric_limits

#include <iostream>


namespace
{
 RangeMatcherMethods4Lua *singleton{nullptr};
 std::vector<std::unique_ptr<AnyBase>> vectorOfObjects{};
 std::string lastErrorMessage{};

 void raiseLuaError( lua_State * L, std::string msg)
 {
  lastErrorMessage = msg;
  lua_pushstring(L, lastErrorMessage.c_str());
  lua_error(L);
 }

 void chkArguments( lua_State * L, int n, std::string functionName)
 {
  int numberOfArguments = lua_gettop(L);

  if( n != numberOfArguments)
  {
   raiseLuaError( L, "Incorrect number of arguments to '"+ functionName +"'");
  }
 }

 int chkArguments( lua_State * L, int nMin, int nMax, std::string functionName)
 {
  int numberOfArguments = lua_gettop(L);

  if( !( nMin <= numberOfArguments && numberOfArguments <= nMax))
  {
   raiseLuaError( L, "Incorrect number of arguments to '"+ functionName +"'");
  }

  return numberOfArguments;
 }

 int rmNextObjectIndex(lua_State * L)
 {
  lastErrorMessage = "";

  chkArguments( L, 0, __func__);
 
  lua_pushinteger(L, vectorOfObjects.size());
  
  return 1;
 }

 int rmPatternString(lua_State * L)
 {
  lastErrorMessage = "";

  int numberOfArguments = chkArguments( L, 1, std::numeric_limits<int>::max(), __func__);
 
  for( int i= 0; i< numberOfArguments; ++i)
  {
   std::string stringOfInterest{lua_tostring(L, i + 1)};

   std::cout << "stringOfInterest " << stringOfInterest << std::endl;

   auto pattern(std::make_unique<Any<std::shared_ptr<PatternString>>>(std::make_shared<PatternString>(stringOfInterest)));

   pattern->addType<std::shared_ptr<Pattern>>();

   vectorOfObjects.push_back( std::move(pattern));
  }

  lua_pop(L, numberOfArguments);
  
  for( int i= 0; i< numberOfArguments; ++i)
  {
   lua_pushinteger(L, vectorOfObjects.size() - numberOfArguments + i);
  }

  return numberOfArguments;
 }

 int rmPatternRegex(lua_State * L)
 {
  lastErrorMessage = "";

  int numberOfArguments = chkArguments( L, 1, std::numeric_limits<int>::max(), __func__);
 
  for( int i= 0; i< numberOfArguments; ++i)
  {
   std::string stringOfInterest{lua_tostring(L, i + 1)};

   std::cout << "stringOfInterest " << stringOfInterest << std::endl;

   auto pattern(std::make_unique<Any<std::shared_ptr<PatternRegex>>>(std::make_shared<PatternRegex>(stringOfInterest)));

   pattern->addType<std::shared_ptr<Pattern>>();

   vectorOfObjects.push_back( std::move(pattern));
  }

  lua_pop(L, numberOfArguments);
  
  for( int i= 0; i< numberOfArguments; ++i)
  {
   lua_pushinteger(L, vectorOfObjects.size() - numberOfArguments + i);
  }

  return numberOfArguments;
 }

 int rmNamedPatternRange(lua_State * L)
 {
  lastErrorMessage = "";

  int numberOfArguments{3};

  chkArguments( L, numberOfArguments, __func__);
 
  std::string nameString{lua_tostring(L, 1)};
  long patternVonInt{lua_tointeger(L, 2)};
  long patternBisInt{lua_tointeger(L, 3)};

  lua_pop(L, numberOfArguments);

  std::cout << "nameString " << nameString << std::endl;

  auto * patternVon(vectorOfObjects.at(patternVonInt)->get<std::shared_ptr<Pattern>>());
  auto * patternBis(vectorOfObjects.at(patternBisInt)->get<std::shared_ptr<Pattern>>());
  
  if( !patternVon)
  {
   raiseLuaError( L, "wrong type at index "+ std::to_string(patternVonInt) + ", expected pattern, argument 2");
  }

  if( !patternBis)
  {
   raiseLuaError( L, "wrong type at index "+ std::to_string(patternBisInt) + ", expected pattern, argument 3");
  }

  using CNPT = const NamedPatternRange;

  auto cnpt( std::make_shared<CNPT>(CNPT::I{nameString, *patternVon, *patternBis}));

  vectorOfObjects.push_back( std::make_unique<Any<std::shared_ptr<CNPT>>>(std::move(cnpt)));

  lua_pushinteger(L, vectorOfObjects.size() - 1);

  return 1;
 }

 int rmNamedPatternRangeVector(lua_State * L)
 {
  lastErrorMessage = "";

  int numberOfArguments = chkArguments( L, 1, std::numeric_limits<int>::max(), __func__);
 
  using CNPT = const NamedPatternRange;

  auto patternRangeVector( std::make_shared<std::vector<std::shared_ptr<CNPT>>>());
 
  for( int i= 0; i< numberOfArguments; ++i)
  {
   long patternRangeInt{lua_tointeger(L, i + 1)};

   auto * patternRange(vectorOfObjects.at(patternRangeInt)->get<std::shared_ptr<CNPT>>());

   if( !patternRange)
   {
    raiseLuaError( L, "wrong type at index " + std::to_string(patternRangeInt) + " argument : " + std::to_string( i + 1));
   }

   patternRangeVector->push_back(*patternRange); // TODO : move
  }

  lua_pop(L, numberOfArguments);
  
  vectorOfObjects.push_back( std::make_unique<Any<std::shared_ptr<std::vector<std::shared_ptr<CNPT>>>>>(std::move(patternRangeVector)));

  lua_pushinteger(L, vectorOfObjects.size() - 1);

  return 1;
 }
}

RangeMatcherMethods4Lua::RangeMatcherMethods4Lua()
{
 if(singleton)
 {
  throw std::runtime_error{std::string(__func__) + "singleton already set"};
 }
 singleton = this;
}

void RangeMatcherMethods4Lua::propagate(LuaBase & luaBase)
{
 auto * L( luaBase.getLua());

#define REGISTER(X) lua_register( L, #X, X);
 REGISTER( rmNextObjectIndex);
 REGISTER( rmPatternString);
 REGISTER( rmPatternRegex);
 REGISTER( rmNamedPatternRange);
 REGISTER( rmNamedPatternRangeVector);
#undef REGISTER
}

const std::string & RangeMatcherMethods4Lua::getLastErrorMessage() const
{
 return lastErrorMessage;
}

