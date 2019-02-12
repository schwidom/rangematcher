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
#include <utility> // pair

// collections
#include <map>
#include <set>
#include <sstream> // ostringstream
#include <vector>

#include <tuple>

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
  std::map<lua_State*,std::weak_ptr<LuaBase>> mapLua2LuaBase; 
 };

 // get this always as a reference because it contains unique ptrs
 std::map<const RangeMatcherMethods4Lua *, RangeMatcherLuaRuntime> rangeMatcherMethods4LuaInstances{}; //!< always valid pointers

 // RangeMatcherLuaRuntime * points to rangeMatcherMethods4LuaInstances (for speed reasons)
 std::map<const lua_State*,RangeMatcherLuaRuntime *> lua2RangeMatcherLuaRuntime{}; //!< always valid pointers 

 template <class ... T> struct CallingParameter
 {
  using type = std::tuple<T...>;
 };

 template <class ... T> struct ReturningParameter
 {
  using type = std::tuple<T...>;
 };

 template<class T_CallingParameter, class T_ReturningParameter>
 struct FunctionType
 {
  using type = void (*)(typename T_CallingParameter::type, typename T_ReturningParameter::type);
 };

 template <class T> struct ReadParameter;

 template <> struct ReadParameter<long>
 {
  static long doIt(lua_State *L, int offset) // length check is already done, pop is later expected
  {
   return lua_tointeger(L, offset); // 1
  }
 };

 template <class ... T> struct ReadParameters;

 template <class T, class ... Tp> struct ReadParameters<T, Tp...>
 {
  static std::tuple<T, Tp...> doIt(lua_State *L, int offset)
  {
/* // TODO: find correct type arguments
   return std::tuple_cat<T, Tp...>(
    std::tuple<T>(ReadParameter<T>::doIt(L, offset)), 
    ReadParameters<Tp...>::doIt(L, 1 + offset));
*/

   return std::tuple_cat(
    std::tuple<T>(ReadParameter<T>::doIt(L, offset)), 
    ReadParameters<Tp...>::doIt(L, 1 + offset));
  }
 };

 template <> struct ReadParameters<>
 {
  static std::tuple<> doIt(lua_State *L, int offset)
  {
   return std::tuple<>();
  }
 };

 void chkArguments( lua_State * L, int n, std::string functionName); // TODO : order functions

 template <class ... Tp> std::tuple<Tp...> readParameterIntoTuple(lua_State *L)
 {
  std::tuple<Tp...> ret;

  chkArguments( L, std::tuple_size<decltype(ret)>::value, __func__);

  ret = ReadParameters<Tp...>::doIt(L, 1);

  return ret;
 }

 template <class ... T> struct ReadParametersFromTuple;

 template <class ... Tp> struct ReadParametersFromTuple<std::tuple<Tp...>>
 {
  static std::tuple<Tp...> doIt(lua_State *L, int offset)
  {
   return  ReadParameters<Tp...>::doIt( L, offset);
  }
 };

 template <class T> T readParameterIntoTuple2(lua_State *L)
 {
  // std::tuple<Tp...> ret;
  T ret;

  chkArguments( L, std::tuple_size<decltype(ret)>::value, __func__);

  ret = ReadParametersFromTuple<T>::doIt(L, 1);

  return ret;
 }

 template <class T_CallingParameter, class T_ReturningParameter, 
  typename FunctionType<T_CallingParameter, T_ReturningParameter>::type FunctionOfInterest>
 struct LuaFunction // WEITERBEI // TODO
 {
  // typedef int (*lua_CFunction) (lua_State *L);
  static int call(lua_State *L)
  {
   auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

   rt.lastErrorMessage = "";
   
   auto readin = readParameterIntoTuple2<typename T_CallingParameter::type>(L); // TODO : Types...

   FunctionOfInterest(readin, std::tuple<>());

   return 0;
  }
 };

 void calllback1(std::tuple<>,std::tuple<>)
 {
 }

 void calllback2(std::tuple<long> t1,std::tuple<>)
 { 
  std::cout << __func__ << " " << std::get<0>(t1) << std::endl;
/*
+<< l rmLuaFunctionTest( 1)
executing lua:  rmLuaFunctionTest( 1)
calllback2 1
success
*/
 }

 void toDelete() // examplecalls // TODO
 {
  ReadParameter<long>::doIt(nullptr, 1);
  ReadParameters<long>::doIt(nullptr, 1);
  ReadParameters<long,long>::doIt(nullptr, 1);
  readParameterIntoTuple<long>(nullptr);
  readParameterIntoTuple<long,long>(nullptr);
  std::tuple<> t0;
  std::tuple_cat<>(t0, t0);
  FunctionType<CallingParameter<>, ReturningParameter<>> ft1;
  FunctionType<CallingParameter<long>, ReturningParameter<>> ft2;
  FunctionType<CallingParameter<long,long>, ReturningParameter<>> ft3;

  LuaFunction<CallingParameter<>,ReturningParameter<>,calllback1> lf1;

  LuaFunction<CallingParameter<long>,ReturningParameter<>,calllback2> lf2;
 }

 void collectGarbagePerRangeMatcherLuaRuntime(RangeMatcherLuaRuntime & rangeMatcherLuaRuntime)
 {
   std::vector<lua_State*> toDelete;
   // std::set<lua_State*> toDelete;

   decltype(rangeMatcherLuaRuntime.mapLua2LuaBase) & mapLua2LuaBase{rangeMatcherLuaRuntime.mapLua2LuaBase};

   for( auto kvp: mapLua2LuaBase) // TODO : bind to LuaBase destructors
   {
    if( !kvp.second.lock())
    {
     toDelete.push_back(kvp.first);
     // toDelete.emplace(kvp.first);
    }
   }

   for( const auto & luaInstance : toDelete)
   {
    mapLua2LuaBase.erase(luaInstance);
    lua2RangeMatcherLuaRuntime.erase(luaInstance);
   }
 }

 void collectGarbageAll() 
 {
  for( auto & kvp: rangeMatcherMethods4LuaInstances)
  {

   collectGarbagePerRangeMatcherLuaRuntime(kvp.second);
  }
 }


 void raiseLuaError( lua_State * L, std::string msg)
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

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
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  chkArguments( L, 0, __func__);
 
  lua_pushinteger(L, rt.vectorOfObjects.size());
  
  return 1;
 }

 template <class T, class ... Tadditional> int rmT(lua_State * L, std::function<T(std::string)> creator) 
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

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
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

  using T = PatternString;
  using ST = std::shared_ptr<T>;

  auto creator = [](std::string stringOfInterest){ return std::make_shared<T>(stringOfInterest);};
 
  return rmT<ST,std::shared_ptr<Pattern>>(L, creator);
 }

 int rmPatternRegex(lua_State * L)
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  using T = PatternRegex;
  using ST = std::shared_ptr<T>;

  auto creator = [](std::string stringOfInterest){ return std::make_shared<T>(stringOfInterest);};
 
  return rmT<ST,std::shared_ptr<Pattern>>(L, creator);
 }

 int rmNamedPatternRange(lua_State * L)
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

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
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

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
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

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
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

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
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

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
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments{0};
  chkArguments( L, numberOfArguments, __func__);
 
  rt.debug = !rt.debug;

  std::cout << "lua debug is " << ( rt.debug ? "on" : "off") << std::endl;

  return 0;
 }

 int rmClear(lua_State * L)
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments{0};
  chkArguments( L, numberOfArguments, __func__);
 
  rt.vectorOfObjects.clear();

  return 0;
 }

 int rmFileRead(lua_State * L)
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

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
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

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
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

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
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments{0};
  chkArguments( L, numberOfArguments, __func__);
 
  Console(StreamPair{std::cin, std::cout}, rt.mapLua2LuaBase.at(L).lock(), rt.rangeMatcherMethods4Lua);

  return 0;
 }

 int rmConsoleNew(lua_State * L) 
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments{0};
  chkArguments( L, numberOfArguments, __func__);
 
  Console(StreamPair{std::cin, std::cout});

  return 0;
 }

 int rmDebugSetInt(lua_State * L)
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments = 1;
  chkArguments( L, numberOfArguments, __func__); 

  long gotInt{lua_tointeger(L, 1)};

  lua_pop(L, numberOfArguments);

  rt.vectorOfObjects.push_back(std::make_unique<Any<long>>(gotInt));

  return 0;
 }

 int rmDebugGetInt(lua_State * L)
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments = 1;
  chkArguments( L, numberOfArguments, __func__); 

  long intIdx{lua_tointeger(L, 1)};

  lua_pop(L, numberOfArguments);

  auto savedInt(rt.vectorOfObjects.at(intIdx)->get<long>());

  if( !savedInt)
  {
   raiseLuaError( L, "wrong type at index " + std::to_string(intIdx) + " argument : " + std::to_string( 1));
  }
  
  lua_pushinteger(L, *savedInt);
  
  return 1;
 }
}

RangeMatcherMethods4Lua::RangeMatcherMethods4Lua()
{
 auto itThis = rangeMatcherMethods4LuaInstances.find(this);
 if( itThis != rangeMatcherMethods4LuaInstances.end())
 {
  throw std::runtime_error{std::string(__func__) + "RangeMatcherMethods4Lua is already registered"};
 }
 rangeMatcherMethods4LuaInstances.emplace( this, *this);
}

RangeMatcherMethods4Lua::~RangeMatcherMethods4Lua()
{
 auto itThis = rangeMatcherMethods4LuaInstances.find(this);
 if( itThis == rangeMatcherMethods4LuaInstances.end())
 {
  throw std::runtime_error{std::string(__func__) + "RangeMatcherMethods4Lua was not registered"};
 }

 // derived from : collectGarbagePerRangeMatcherLuaRuntime(RangeMatcherLuaRuntime & rangeMatcherLuaRuntime)
 {
   std::vector<lua_State*> toDelete;

   decltype(itThis->second.mapLua2LuaBase) & mapLua2LuaBase{itThis->second.mapLua2LuaBase};

   for( auto kvp: mapLua2LuaBase) // TODO : bind to LuaBase destructors
   {
    toDelete.push_back(kvp.first);
    if( !kvp.second.lock())
    {
     std::cout << "warning : a LuaBase istance is still active" << std::endl;
    }
   }

   for( const auto & luaInstance : toDelete)
   {
    mapLua2LuaBase.erase(luaInstance);
    lua2RangeMatcherLuaRuntime.erase(luaInstance);
   }
 }

 rangeMatcherMethods4LuaInstances.erase( itThis);
}


void RangeMatcherMethods4Lua::registerMethods2LuaBase(std::weak_ptr<LuaBase> luaBaseWeak)
{

 std::shared_ptr<LuaBase> luaBase = luaBaseWeak.lock();

 if(!luaBase) { return;}

 auto * L( luaBase->getLua());
 
 {

  RangeMatcherLuaRuntime& thisRmlr = rangeMatcherMethods4LuaInstances.at(this);

  if( !thisRmlr.mapLua2LuaBase.emplace( L, luaBaseWeak).second)
  {
   throw std::runtime_error{std::string(__func__) + "lua_State* to LuaBase already registered"};
  }
 
  if( !lua2RangeMatcherLuaRuntime.emplace( L, &thisRmlr).second)
  {
   throw std::runtime_error{std::string(__func__) + "lua_State* to RangeMatcherMethods4Lua already registered"};
  }

 }

 ::collectGarbageAll();

 auto & rt = *lua2RangeMatcherLuaRuntime.at(L); // TODO : take earlier instance

 decltype(rt.vectorOfRegisteredLuaFunctions) * vorlf = &rt.vectorOfRegisteredLuaFunctions;
 decltype(rt.mapOfHelpOfRegisteredLuaFunctions) * mohorlf = &rt.mapOfHelpOfRegisteredLuaFunctions;

 std::function<void(const char * functionName, lua_CFunction function, std::string helpString)>

  registerFunction{ [L, vorlf, mohorlf](const char * functionName, lua_CFunction functionCall, std::string helpString) {

  lua_register( L, functionName, functionCall);
  vorlf->emplace_back(functionName);
  mohorlf->emplace(functionName, helpString);
 }};

#define REGISTER(X, Y) registerFunction( #X, X, Y);

 registerFunction( "rmLuaFunctionTest", LuaFunction<CallingParameter<long>,ReturningParameter<>,calllback2>::call, "test");

 REGISTER( rmClear, "clears the variables vector");
 REGISTER( rmNextObjectIndex, " outputs the next index of the variables vector");

 REGISTER( rmConsole, "starts a console with current settings");
 REGISTER( rmConsoleNew, "starts a console with new settings");

 REGISTER( rmDebugSetInt, "saves the given integer value to the variables vector");
 REGISTER( rmDebugGetInt, "outputs the integer value from the variables vector at the given position");

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

 const auto & thisRmlr = rangeMatcherMethods4LuaInstances.at(this);

 return thisRmlr.lastErrorMessage;
 
 return oss.str();
}

