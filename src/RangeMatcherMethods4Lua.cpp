#include "Any.hpp"
#include "Console.hpp"
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

// collections
#include <map>
#include <set>
#include <sstream> // ostringstream
#include <vector>

#include <stdexcept> // runtime_error

#include <limits> // std::numeric_limits

#include <iostream>

#define TYPE std::vector<char>

namespace
{

 struct RangeMatcherLuaRuntime
 {
  RangeMatcherLuaRuntime(RangeMatcherMethods4Lua & rangeMatcherMethods4Lua)
  : rangeMatcherMethods4Lua{rangeMatcherMethods4Lua} {}

  RangeMatcherMethods4Lua & rangeMatcherMethods4Lua;
  bool debug{false};
  std::vector<std::unique_ptr<AnyBase>> vectorOfObjects{};
  std::string lastErrorMessage{};
  std::vector<std::string> vectorOfRegisteredLuaFunctions{};
  std::map<std::string,std::string> mapOfHelpOfRegisteredLuaFunctions{};
 };

 std::set<RangeMatcherMethods4Lua *> rangeMatcherMethods4LuaInstances{}; //!< always valid pointers

 // std::map<lua_State*,RangeMatcherMethods4Lua *> lua2RangeMatcherLuaRuntime{}; //!< always valid pointers since the RangeMatcherMethods4Lua::m_MapLua2LuaBase has valid weak pointer
 std::map<lua_State*,RangeMatcherLuaRuntime> lua2RangeMatcherLuaRuntime{}; //!< always valid pointers since the RangeMatcherMethods4Lua::m_MapLua2LuaBase has valid weak pointer

 void collectGarbage() // TODO : bind to LuaBase destructors
 {
  for( auto rangeMatcherMethods4Lua : rangeMatcherMethods4LuaInstances)
  {
   auto luaInstances( rangeMatcherMethods4Lua -> collectGarbage());
   for( const auto & luaInstance : luaInstances)
   {
    lua2RangeMatcherLuaRuntime.erase(luaInstance);
   }
  }
 }

 void raiseLuaError( lua_State * L, std::string msg)
 {
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = msg;
  lua_pushstring(L, rt.lastErrorMessage.c_str());
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
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  chkArguments( L, 0, __func__);
 
  lua_pushinteger(L, rt.vectorOfObjects.size());
  
  return 1;
 }

 template <class T, class ... Tadditional> int rmT(lua_State * L, std::function<T(std::string)> creator) 
 {
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments = chkArguments( L, 1, std::numeric_limits<int>::max(), __func__);
 
  for( int i= 0; i< numberOfArguments; ++i)
  {
   std::string stringOfInterest{lua_tostring(L, i + 1)};

   if( rt.debug){ std::cout << "stringOfInterest " << stringOfInterest << std::endl;}

   std::unique_ptr<Any<T>> pattern(std::make_unique<Any<T>>(creator(stringOfInterest)));

   pattern -> template addTypes<Tadditional...>();

   rt.vectorOfObjects.push_back( std::move(pattern));
  }

  lua_pop(L, numberOfArguments);
  
  for( int i= 0; i< numberOfArguments; ++i)
  {
   lua_pushinteger(L, rt.vectorOfObjects.size() - numberOfArguments + i);
  }

  return numberOfArguments;
 }

 int rmPatternString(lua_State * L)
 {
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  using T = PatternString;
  using ST = std::shared_ptr<T>;

  auto creator = [](std::string stringOfInterest){ return std::make_shared<T>(stringOfInterest);};
 
  return rmT<ST,std::shared_ptr<Pattern>>(L, creator);
 }

 int rmPatternRegex(lua_State * L)
 {
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  using T = PatternRegex;
  using ST = std::shared_ptr<T>;

  auto creator = [](std::string stringOfInterest){ return std::make_shared<T>(stringOfInterest);};
 
  return rmT<ST,std::shared_ptr<Pattern>>(L, creator);
 }

 int rmNamedPatternRange(lua_State * L)
 {
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments{3};

  chkArguments( L, numberOfArguments, __func__);
 
  std::string nameString{lua_tostring(L, 1)};
  long patternVonInt{lua_tointeger(L, 2)};
  long patternBisInt{lua_tointeger(L, 3)};

  lua_pop(L, numberOfArguments);

  if( rt.debug){ std::cout << "nameString " << nameString << std::endl;}

  auto * patternVon(rt.vectorOfObjects.at(patternVonInt)->get<std::shared_ptr<Pattern>>());
  auto * patternBis(rt.vectorOfObjects.at(patternBisInt)->get<std::shared_ptr<Pattern>>());
  
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

  rt.vectorOfObjects.push_back( std::make_unique<Any<std::shared_ptr<CNPT>>>(std::move(cnpt)));

  lua_pushinteger(L, rt.vectorOfObjects.size() - 1);

  return 1;
 }

 int rmNamedPatternRangeVector(lua_State * L)
 {
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments = chkArguments( L, 1, std::numeric_limits<int>::max(), __func__);
 
  using CNPT = const NamedPatternRange;

  auto patternRangeVector( std::make_shared<std::vector<std::shared_ptr<CNPT>>>());
 
  for( int i= 0; i< numberOfArguments; ++i)
  {
   long patternRangeInt{lua_tointeger(L, i + 1)};

   auto * patternRange(rt.vectorOfObjects.at(patternRangeInt)->get<std::shared_ptr<CNPT>>());

   if( !patternRange)
   {
    raiseLuaError( L, "wrong type at index " + std::to_string(patternRangeInt) + " argument : " + std::to_string( i + 1));
   }

   patternRangeVector->push_back(*patternRange); // TODO : move
  }

  lua_pop(L, numberOfArguments);
  
  rt.vectorOfObjects.push_back( std::make_unique<Any<std::shared_ptr<std::vector<std::shared_ptr<CNPT>>>>>(std::move(patternRangeVector)));

  lua_pushinteger(L, rt.vectorOfObjects.size() - 1);

  return 1;
 }

 int rmNonOverlappingMatcher(lua_State * L)
 {
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments = chkArguments( L, 1, std::numeric_limits<int>::max(), __func__);
 
  using CNPT = const NamedPatternRange;

  auto patternRangeVector( std::make_shared<std::vector<std::shared_ptr<CNPT>>>());
 
  for( int i= 0; i< numberOfArguments; ++i)
  {
   long patternRangeInt{lua_tointeger(L, i + 1)};

   auto * patternRange(rt.vectorOfObjects.at(patternRangeInt)->get<std::shared_ptr<CNPT>>());

   if( !patternRange)
   {
    raiseLuaError( L, "wrong type at index " + std::to_string(patternRangeInt) + " argument : " + std::to_string( i + 1));
   }

   patternRangeVector->push_back(*patternRange); // TODO : move
  }

  lua_pop(L, numberOfArguments);
  
  auto nonOverlappingMatcher(std::make_shared<NonOverlappingMatcher>(*patternRangeVector));

  rt.vectorOfObjects.push_back( std::make_unique<Any<std::shared_ptr<NonOverlappingMatcher>>>(std::move(nonOverlappingMatcher)));

  lua_pushinteger(L, rt.vectorOfObjects.size() - 1);

  return 1;
 }

 int rmFunctions(lua_State * L) // TODO : optional regexp parameter 
 {
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments{0};
  chkArguments( L, numberOfArguments, __func__);
 
  for( const auto & functionName : rt.vectorOfRegisteredLuaFunctions)
  {
   lua_pushstring(L, functionName.c_str());
  }

  return rt.vectorOfRegisteredLuaFunctions.size();
 }

 int rmHelp(lua_State * L) // TODO : optional regexp parameter or help menu
 {
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments{0};
  chkArguments( L, numberOfArguments, __func__);
 
  for( const auto & key : rt.vectorOfRegisteredLuaFunctions)
  {
   const auto & value ( rt.mapOfHelpOfRegisteredLuaFunctions.at(key));
   std::cout << key << " ... " << value << std::endl;
  }

  return 0;
 }

 int rmToggleDebug(lua_State * L) 
 {
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments{0};
  chkArguments( L, numberOfArguments, __func__);
 
  rt.debug = !rt.debug;

  std::cout << "lua debug is " << ( rt.debug ? "on" : "off") << std::endl;

  return 0;
 }

 int rmClear(lua_State * L)
 {
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments{0};
  chkArguments( L, numberOfArguments, __func__);
 
  rt.vectorOfObjects.clear();

  return 0;
 }

 int rmFileRead(lua_State * L)
 {
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments = chkArguments( L, 1, std::numeric_limits<int>::max(), __func__);

  for( int i= 0; i< numberOfArguments; ++i)
  {
   std::string stringOfInterest{lua_tostring(L, i + 1)};

   if( rt.debug){ std::cout << "stringOfInterest " << stringOfInterest << std::endl;}

   std::shared_ptr<std::vector<char>> fileVector( FileToVector(stringOfInterest).get());

   rt.vectorOfObjects.push_back( std::make_unique<Any<decltype(fileVector)>>(std::move(fileVector)));
  }

  lua_pop(L, numberOfArguments);

  for( int i= 0; i< numberOfArguments; ++i)
  {
   lua_pushinteger(L, rt.vectorOfObjects.size() - numberOfArguments + i);
  }

  return numberOfArguments;
 }

 int rmMatchRanges(lua_State * L)
 {
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments = 2;
  chkArguments( L, numberOfArguments, __func__); // nonOverlappingMatcher, fileVector

  long nonOverlappingMatcherInt{lua_tointeger(L, 1)};
  long fileVectorInt{lua_tointeger(L, 2)};

  lua_pop(L, numberOfArguments);

  auto * nonOverlappingMatcher(rt.vectorOfObjects.at(nonOverlappingMatcherInt)->get<std::shared_ptr<NonOverlappingMatcher>>());

  if( !nonOverlappingMatcher)
  {
   raiseLuaError( L, "wrong type at index " + std::to_string(nonOverlappingMatcherInt) + " argument : " + std::to_string( 1));
  }

  auto * fileVector(rt.vectorOfObjects.at(fileVectorInt)->get<std::shared_ptr<std::vector<char>>>());

  if( !fileVector)
  {
   raiseLuaError( L, "wrong type at index " + std::to_string(fileVectorInt) + " argument : " + std::to_string( 2));
  }

  std::shared_ptr<std::vector<MatchRange<TYPE>>> matchedRanges ( (*nonOverlappingMatcher)->matchAll(Range<TYPE>{(*fileVector)->begin(), (*fileVector)->end()}));
 
  std::shared_ptr<std::vector<MatchRange<long>>> matchedRangesRelative{std::make_shared<std::vector<MatchRange<long>>>()};

  for( const auto & value : *matchedRanges)
  {
   const MatchRange<TYPE>::I & iRoot(matchedRanges->at(0).i());
   const MatchRange<TYPE>::I & i(value.i());
   MatchRange<long>::I i2;

   i2.namingWeakOrdered = i.namingWeakOrdered;
   i2.d.begin = Range<long>( iRoot.d.begin.begin, i.d.begin);
   i2.d.complete = i.d.complete;
   i2.d.end = Range<long>( iRoot.d.begin.begin, i.d.end);

   // matchedRangesRelative->push_back(MatchRange<long>{ });
   matchedRangesRelative->push_back(i2);
  }
 
/* // TODO :
  std::transform(matchedRanges->begin(), matchedRanges->end(), std::back_inserter(matchedRangesRelative->end()), []
   (MatchRange<TYPE> & matchRange){
   MatchRange<TYPE>::I i{matchRange.i()};
   return MatchRangeRelative{i.d().begin - fileVector.begin()};
  });
*/

  rt.vectorOfObjects.push_back( std::make_unique<Any<decltype(matchedRangesRelative)>>(std::move(matchedRangesRelative)));

  lua_pushinteger(L, rt.vectorOfObjects.size() - 1);

  return 1;
 }

 int rmMatchRanges2Lua(lua_State * L)
 {
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments = 1;
  chkArguments( L, numberOfArguments, __func__); // std::vector<MatchRange<long>>

  long matchRangesIdx{lua_tointeger(L, 1)};

  lua_pop(L, numberOfArguments);

  auto * matchedRangesRelative(rt.vectorOfObjects.at(matchRangesIdx)->get<std::shared_ptr<std::vector<MatchRange<long>>>>());

  if( !matchedRangesRelative)
  {
   raiseLuaError( L, "wrong type at index " + std::to_string(matchRangesIdx) + " argument : " + std::to_string( 1));
  }

  for( const auto & matchRange : **matchedRangesRelative)
  {
   const MatchRange<long>::I & i (matchRange.i());

   lua_createtable(L, 0, 5);
   int table = lua_gettop(L);

   lua_pushstring( L, "name");
   lua_pushstring( L, i.namingWeakOrdered.name.c_str());
   lua_settable( L, table); 

   lua_pushstring( L, "begin.begin");
   lua_pushinteger( L, i.d.begin.begin);
   lua_settable( L, table); 

   lua_pushstring( L, "begin.end");
   lua_pushinteger( L, i.d.begin.end);
   lua_settable( L, table); 

   lua_pushstring( L, "end.begin");
   lua_pushinteger( L, i.d.end.begin);
   lua_settable( L, table); 

   lua_pushstring( L, "end.end");
   lua_pushinteger( L, i.d.end.end);
   lua_settable( L, table); 
   
  }

  return (*matchedRangesRelative)->size();
 }

 int rmConsole(lua_State * L) 
 {
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments{0};
  chkArguments( L, numberOfArguments, __func__);
 
  Console(StreamPair{std::cin, std::cout}, rt.rangeMatcherMethods4Lua.m_MapLua2LuaBase.at(L).lock(), rt.rangeMatcherMethods4Lua);

  return 0;
 }

 int rmConsoleNew(lua_State * L) 
 {
  auto & rt = lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments{0};
  chkArguments( L, numberOfArguments, __func__);
 
  Console(StreamPair{std::cin, std::cout});

  return 0;
 }

}

RangeMatcherMethods4Lua::RangeMatcherMethods4Lua()
{
 auto itThis = rangeMatcherMethods4LuaInstances.find(this);
 if( itThis != rangeMatcherMethods4LuaInstances.end())
 {
  throw std::runtime_error{std::string(__func__) + "RangeMatcherMethods4Lua is already registered"};
 }
 rangeMatcherMethods4LuaInstances.emplace( this);
}

RangeMatcherMethods4Lua::~RangeMatcherMethods4Lua()
{
 auto itThis = rangeMatcherMethods4LuaInstances.find(this);
 if( itThis == rangeMatcherMethods4LuaInstances.end())
 {
  throw std::runtime_error{std::string(__func__) + "RangeMatcherMethods4Lua was not registered"};
 }
 rangeMatcherMethods4LuaInstances.erase( itThis);
}


void RangeMatcherMethods4Lua::registerMethods2LuaBase(std::weak_ptr<LuaBase> luaBaseWeak)
{

 std::shared_ptr<LuaBase> luaBase = luaBaseWeak.lock();

 if(!luaBase) { return;}

 auto * L( luaBase->getLua());
 
 if( !m_MapLua2LuaBase.emplace( L, luaBaseWeak).second)
 {
  throw std::runtime_error{std::string(__func__) + "lua_State* to LuaBase already registered"};
 }

 if( !lua2RangeMatcherLuaRuntime.emplace( L, *this).second)
 {
  throw std::runtime_error{std::string(__func__) + "lua_State* to RangeMatcherMethods4Lua already registered"};
 }

 ::collectGarbage();

 auto & rt = lua2RangeMatcherLuaRuntime.at(L); // TODO : take earlier instance

 decltype(rt.vectorOfRegisteredLuaFunctions) * vorlf = &rt.vectorOfRegisteredLuaFunctions;
 decltype(rt.mapOfHelpOfRegisteredLuaFunctions) * mohorlf = &rt.mapOfHelpOfRegisteredLuaFunctions;

 std::function<void(const char * functionName, lua_CFunction function, std::string helpString)>

  registerFunction{ [L, vorlf, mohorlf](const char * functionName, lua_CFunction functionCall, std::string helpString) {

  lua_register( L, functionName, functionCall);
  vorlf->emplace_back(functionName);
  mohorlf->emplace(functionName, helpString);
 }};

#define REGISTER(X, Y) registerFunction( #X, X, Y);

 REGISTER( rmClear, "clears the variables vector");
 REGISTER( rmNextObjectIndex, " outputs the next index of the variables vector");

 REGISTER( rmConsole, "starts a console with current settings");
 REGISTER( rmConsoleNew, "starts a console with new settings");

 REGISTER( rmFileRead, "reads a file");
 REGISTER( rmFunctions, "returns all function names");
 REGISTER( rmHelp, "outputs help");
 REGISTER( rmMatchRanges, "creates a vector of match ranges from a non overlapping matcher");
 REGISTER( rmMatchRanges2Lua, "converts the match ranges to lua datatypes");
 REGISTER( rmNamedPatternRange, "a named pattern range, consists of a name and 2 pattern");
 REGISTER( rmNamedPatternRangeVector, "a vector of named pattern ranges");
 REGISTER( rmNonOverlappingMatcher, "creates a non overlapping matcher from all given pattern ranges ");
 REGISTER( rmPatternRegex, "initializes a regular expression object");
 REGISTER( rmPatternString, "initializes a pattern string object");
 REGISTER( rmToggleDebug, "toggles the debug flag and outputs its state");
#undef REGISTER

}

const std::string RangeMatcherMethods4Lua::getLastErrorMessage() const
{
 std::ostringstream oss;

 for( const auto & kvp : m_MapLua2LuaBase)
 {
  std::shared_ptr<LuaBase> luaBase = kvp.second.lock();
  if( luaBase) // INFO : garbage collection information is available
  {
   oss << lua2RangeMatcherLuaRuntime.at( luaBase->getLua()).lastErrorMessage << std::endl;
  }
 }
 
 return oss.str();
}

