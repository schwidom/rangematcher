#include "Any.hpp"
#include "FileToVector.hpp"
#include "MatchRange.hpp"
#include "NamedPatternRange.hpp"
#include "NonOverlappingMatcher.hpp"
#include "PatternRegex.hpp"
#include "PatternString.hpp"
#include "Range.hpp"
#include "RangeMatcherMethods4Lua.hpp"

#include <algorithm> // transform
#include <functional> // bind
#include <iterator> // back_inserter

#include <stdexcept> // runtime_error

#include <limits> // std::numeric_limits

#include <iostream>


namespace
{
 RangeMatcherMethods4Lua *singleton{nullptr};
 std::vector<std::unique_ptr<AnyBase>> vectorOfObjects{};
 std::string lastErrorMessage{};
 std::vector<std::string> vectorOfRegisteredLuaFunctions{};

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

 template <class T, class ... Tadditional> int rmT(lua_State * L, std::function<T(std::string)> creator) 
 {
  lastErrorMessage = "";

  int numberOfArguments = chkArguments( L, 1, std::numeric_limits<int>::max(), __func__);
 
  for( int i= 0; i< numberOfArguments; ++i)
  {
   std::string stringOfInterest{lua_tostring(L, i + 1)};

   std::cout << "stringOfInterest " << stringOfInterest << std::endl;

   std::unique_ptr<Any<T>> pattern(std::make_unique<Any<T>>(creator(stringOfInterest)));

   pattern -> template addTypes<Tadditional...>();

   vectorOfObjects.push_back( std::move(pattern));
  }

  lua_pop(L, numberOfArguments);
  
  for( int i= 0; i< numberOfArguments; ++i)
  {
   lua_pushinteger(L, vectorOfObjects.size() - numberOfArguments + i);
  }

  return numberOfArguments;
 }

 int rmPatternString(lua_State * L)
 {
  using T = PatternString;
  using ST = std::shared_ptr<T>;

  auto creator = [](std::string stringOfInterest){ return std::make_shared<T>(stringOfInterest);};
 
  return rmT<ST,std::shared_ptr<Pattern>>(L, creator);
 }

 int rmPatternRegex(lua_State * L)
 {
  using T = PatternRegex;
  using ST = std::shared_ptr<T>;

  auto creator = [](std::string stringOfInterest){ return std::make_shared<T>(stringOfInterest);};
 
  return rmT<ST,std::shared_ptr<Pattern>>(L, creator);
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

 int rmNonOverlappingMatcher(lua_State * L)
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
  
  auto nonOverlappingMatcher(std::make_shared<NonOverlappingMatcher>(*patternRangeVector));

  vectorOfObjects.push_back( std::make_unique<Any<std::shared_ptr<NonOverlappingMatcher>>>(std::move(nonOverlappingMatcher)));

  lua_pushinteger(L, vectorOfObjects.size() - 1);

  return 1;
 }

 int rmFunctions(lua_State * L)
 {
  lastErrorMessage = "";

  int numberOfArguments{0};
  chkArguments( L, numberOfArguments, __func__);
 
  for( const auto & functionName : vectorOfRegisteredLuaFunctions)
  {
   lua_pushstring(L, functionName.c_str());
  }

  return vectorOfRegisteredLuaFunctions.size();
 }

 int rmClear(lua_State * L)
 {
  lastErrorMessage = "";

  int numberOfArguments{0};
  chkArguments( L, numberOfArguments, __func__);
 
  vectorOfObjects.clear();

  return 0;
 }

 int rmFileRead(lua_State * L)
 {
  lastErrorMessage = "";

  int numberOfArguments = chkArguments( L, 1, std::numeric_limits<int>::max(), __func__);

  for( int i= 0; i< numberOfArguments; ++i)
  {
   std::string stringOfInterest{lua_tostring(L, i + 1)};

   std::cout << "stringOfInterest " << stringOfInterest << std::endl;

   std::shared_ptr<std::vector<char>> fileVector( FileToVector(stringOfInterest).get());

   vectorOfObjects.push_back( std::make_unique<Any<decltype(fileVector)>>(std::move(fileVector)));
  }

  lua_pop(L, numberOfArguments);

  for( int i= 0; i< numberOfArguments; ++i)
  {
   lua_pushinteger(L, vectorOfObjects.size() - numberOfArguments + i);
  }

  return numberOfArguments;
 }

 struct MatchRangeRelative
 {
  std::string name;
  long bb;
  long be;
  long eb;
  long ee;
 };

 int rmMatchRanges(lua_State * L)
 {
  lastErrorMessage = "";

  int numberOfArguments = 2;
  chkArguments( L, numberOfArguments, __func__); // nonOverlappingMatcher, fileVector

  long nonOverlappingMatcherInt{lua_tointeger(L, 1)};
  long fileVectorInt{lua_tointeger(L, 2)};

  lua_pop(L, numberOfArguments);

  auto * nonOverlappingMatcher(vectorOfObjects.at(nonOverlappingMatcherInt)->get<std::shared_ptr<NonOverlappingMatcher>>());

  if( !nonOverlappingMatcher)
  {
   raiseLuaError( L, "wrong type at index " + std::to_string(nonOverlappingMatcherInt) + " argument : " + std::to_string( 1));
  }

  auto * fileVector(vectorOfObjects.at(fileVectorInt)->get<std::shared_ptr<std::vector<char>>>());

  if( !fileVector)
  {
   raiseLuaError( L, "wrong type at index " + std::to_string(fileVectorInt) + " argument : " + std::to_string( 2));
  }

  std::shared_ptr<std::vector<MatchRange>> matchedRanges ( (*nonOverlappingMatcher)->matchAll(Range{(*fileVector)->begin(), (*fileVector)->end()}));

  std::shared_ptr<std::vector<MatchRangeRelative>> matchedRangesRelative{std::make_shared<std::vector<MatchRangeRelative>>()};
 
  for( const auto & value : *matchedRanges)
  {
   // matchedRangesRelative->emplace_back({"ox"});
   const MatchRange::I & i(value.i());
   const MatchRange::D & d(i.d);
   // matchedRangesRelative->push_back(MatchRangeRelative{i.namingWeakOrdered.name});
   matchedRangesRelative->push_back(MatchRangeRelative{value.getName()
   , d.begin.begin - (*fileVector)->begin()
   , d.begin.end - (*fileVector)->begin()
   , d.end.begin - (*fileVector)->begin()
   , d.end.end - (*fileVector)->begin()
  });
  }
 
/* // TODO :
  std::transform(matchedRanges->begin(), matchedRanges->end(), std::back_inserter(matchedRangesRelative->end()), []
   (MatchRange & matchRange){
   MatchRange::I i{matchRange.i()};
   return MatchRangeRelative{i.d().begin - fileVector.begin()};
  });
*/

  vectorOfObjects.push_back( std::make_unique<Any<decltype(matchedRangesRelative)>>(std::move(matchedRangesRelative)));

  lua_pushinteger(L, vectorOfObjects.size() - 1);

  return 1;
 }

 int rmMatchRanges2Lua(lua_State * L)
 {
  lastErrorMessage = "";

  int numberOfArguments = 1;
  chkArguments( L, numberOfArguments, __func__); // std::vector<MatchRangeRelative>

  long matchRangesIdx{lua_tointeger(L, 1)};

  lua_pop(L, numberOfArguments);

  auto * matchedRangesRelative(vectorOfObjects.at(matchRangesIdx)->get<std::shared_ptr<std::vector<MatchRangeRelative>>>());

  if( !matchedRangesRelative)
  {
   raiseLuaError( L, "wrong type at index " + std::to_string(matchRangesIdx) + " argument : " + std::to_string( 1));
  }

  for( const auto & matchRange : **matchedRangesRelative)
  {
   lua_checkstack( L, 5); // TODO check
   lua_pushstring(L, matchRange.name.c_str());
   lua_pushinteger(L, matchRange.bb);
   lua_pushinteger(L, matchRange.be);
   lua_pushinteger(L, matchRange.eb);
   lua_pushinteger(L, matchRange.ee);
  }

  return (*matchedRangesRelative)->size() * 5;
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
 if( m_Propagated)
 {
  throw std::runtime_error{std::string(__func__) + "already propagated"};
 }

 auto * L( luaBase.getLua());

 decltype(vectorOfRegisteredLuaFunctions) * vorlf = &vectorOfRegisteredLuaFunctions;

 std::function<void(const char * functionName, lua_CFunction function)> registerFunction{ [L, vorlf](const char * functionName, lua_CFunction functionCall) {
  lua_register( L, functionName, functionCall);
  vorlf->emplace_back(functionName);
 }};

#define REGISTER(X) registerFunction( #X, X);
 REGISTER( rmClear);
 REGISTER( rmFileRead);
 REGISTER( rmFunctions);
 REGISTER( rmMatchRanges);
 REGISTER( rmMatchRanges2Lua);
 REGISTER( rmNamedPatternRange);
 REGISTER( rmNamedPatternRangeVector);
 REGISTER( rmNextObjectIndex);
 REGISTER( rmNonOverlappingMatcher);
 REGISTER( rmPatternRegex);
 REGISTER( rmPatternString);
#undef REGISTER

 m_Propagated = true;
}

const std::string & RangeMatcherMethods4Lua::getLastErrorMessage() const
{
 return lastErrorMessage;
}

