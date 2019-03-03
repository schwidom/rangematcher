
// TODO : remove outdated TODOs
// - extract tools
// - LuaFunction3[N] for creating and expanding vectors

// RangeMatcherLuaRuntime : externalize
// implement own Alloc scheme and extend with own control data
// better error output for lua scripts
// prove stack size
// get rid of the TYPE macro

#include "tools.hpp"

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

#include "lua/C2LuaParameter.hpp"
#include "lua/C2LuaParameters.hpp"
#include "lua/FunctionType3.hpp"
#include "lua/Lua2CParameter.hpp"
#include "lua/Lua2CParameters.hpp"
#include "lua/LuaFunction3.hpp"
#include "lua/LuaFunction3N.hpp"
#include "lua/LuaFunction3NVector1.hpp"
#include "lua/LuaFunction3NVector.hpp"
#include "lua/LuaInstance.hpp"
#include "lua/LuaParameterTraits.hpp"
#include "lua/RangeMatcherLuaRuntime.hpp"
#include "lua/readParameterIntoTuple.hpp"
#include "lua/readParameterIntoTupleN.hpp"
#include "lua/writeParameterFromTuple.hpp"

#include <algorithm> // transform
#include <functional> // bind
#include <iterator> // back_inserter
#include <sstream> // ostringstream
#include <string> // to_string
#include <utility> // pair, make_index_sequence

// collections
#include <map>
#include <set>
#include <vector>
#include <list>

#include <type_traits> // enable_if

#include <tuple>

#include <stdexcept> // runtime_error

#include <limits> // std::numeric_limits

#include <iostream>

#define TYPE std::vector<char>

LuaInstance currentLuaInstance( lua_State * L);

namespace
{

 // get this always as a reference because it contains unique ptrs
 std::map<const RangeMatcherMethods4Lua *, RangeMatcherLuaRuntime> rangeMatcherMethods4LuaInstances{}; //!< always valid pointers

 // RangeMatcherLuaRuntime * points to rangeMatcherMethods4LuaInstances (for speed reasons)
 std::map<const lua_State*,RangeMatcherLuaRuntime *> lua2RangeMatcherLuaRuntime{}; //!< always valid pointers 

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

 std::tuple<long> cb_rmNextObjectIndex(lua_State * L)
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);
  return std::make_tuple(static_cast<long>(rt.vectorOfObjects.size()));
 }

 // TODO : template variant
 std::tuple<std::shared_ptr<PatternRegex>> cb_rmPatternRegex(std::string stringOfInterest)
 {
  return std::make_tuple(std::make_shared<PatternRegex>(stringOfInterest));
 }

 std::tuple<std::shared_ptr<PatternString>> cb_rmPatternString(std::string stringOfInterest)
 {
  return std::make_tuple(std::make_shared<PatternString>(stringOfInterest));
 }

 std::tuple<std::shared_ptr<const NamedPatternRange>> cb_rmNamedPatternRange(std::string nameString, std::shared_ptr<const Pattern> patternVon, std::shared_ptr<const Pattern> patternBis)
 {
  using CNPT = const NamedPatternRange;
  auto cnpt( std::make_shared<CNPT>(CNPT::I{nameString, patternVon, patternBis}));
  return std::make_tuple(cnpt);
 }

 std::tuple<std::shared_ptr<const NamedPatternRange>>
  cb_rmNamedPatternRangeVector(std::shared_ptr<const NamedPatternRange> patternRange)
 {
  return std::make_tuple(patternRange);
 }
 
 std::tuple<> cb_rmCheckNamedPatternRange(std::shared_ptr<std::vector<std::tuple<std::shared_ptr<const NamedPatternRange>>>> nprVector)
 {
  std::cout << "cb_rmCheckNamedPatternRange " << nprVector->size() << std::endl;
  return std::make_tuple();
 }

 std::tuple<> cb_rmCheckNamedPatternRange1(std::shared_ptr<std::vector<std::shared_ptr<const NamedPatternRange>>> nprVector)
 {
  std::cout << "cb_rmCheckNamedPatternRange1 " << nprVector->size() << std::endl;
  return std::make_tuple();
 }

 std::tuple<std::shared_ptr<NonOverlappingMatcher>> 
  cb_rmNonOverlappingMatcher1( std::shared_ptr<std::vector<std::shared_ptr< const NamedPatternRange>>> patternRangeVector1)
   // 1 stands for : without tuple, see LuaFunction3NVector1 vs. LuaFunction3NVector
 {
  std::cout << "cb_rmNonOverlappingMatcher1 " << patternRangeVector1->size() << std::endl;
  auto nonOverlappingMatcher(std::make_shared<NonOverlappingMatcher>(*patternRangeVector1));
  return std::make_tuple(nonOverlappingMatcher);
 }

 std::tuple<std::shared_ptr<std::vector<std::string>>> cb_rmFunctions(lua_State * L) // TODO : optional regexp parameter
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);
  return std::make_tuple(std::make_shared<std::vector<std::string>>(rt.vectorOfRegisteredLuaFunctions));
 }

 std::tuple<> cb_rmHelp(lua_State * L) // TODO : optional regexp parameter or help menu
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

  for( const auto & key : rt.vectorOfRegisteredLuaFunctions)
  {
   const auto & value ( rt.mapOfHelpOfRegisteredLuaFunctions.at(key));
   std::cout << key << " ... " << value << std::endl;
  }

  return std::tuple<>();
 }

 std::tuple<> cb_rmToggleDebug(lua_State * L) 
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

  rt.debug = !rt.debug;

  std::cout << "lua debug is " << ( rt.debug ? "on" : "off") << std::endl;

  return std::tuple<>();
 }

 std::tuple<> cb_rmClear(lua_State * L)
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  rt.vectorOfObjects.clear();

  return std::make_tuple();
 }

 std::tuple<std::shared_ptr<std::vector<char>>>
  cb_rmFileRead( std::string stringOfInterest) // see cb_rmDebugSetInt
 {
   // if( rt.debug){ std::cout << "stringOfInterest " << stringOfInterest << std::endl;}
   std::cout << "stringOfInterest " << stringOfInterest << std::endl;
   return std::make_tuple(std::shared_ptr<std::vector<char>>(FileToVector(stringOfInterest).get()));
 }

 std::tuple<bool, std::shared_ptr<std::vector<MatchRange<long>>>>
 cb_rmMatchRanges(std::shared_ptr<NonOverlappingMatcher> nonOverlappingMatcher, std::shared_ptr<std::vector<char>> fileVector)
 {

  bool complete;
  std::shared_ptr<std::vector<MatchRange<TYPE>>> matchedRanges;

  std::tie(complete, matchedRanges) = nonOverlappingMatcher->matchAll(Range<TYPE>{(fileVector)->begin(), (fileVector)->end()});
 
  std::shared_ptr<std::vector<MatchRange<long>>> matchedRangesRelative{std::make_shared<std::vector<MatchRange<long>>>()}; 

  for( const auto & value : *matchedRanges)
  {
   const MatchRange<TYPE>::I & iRoot(matchedRanges->at(0).i());
   const MatchRange<TYPE>::I & i(value.i());
   MatchRange<long>::I i2;

   i2.namingWeakOrdered = i.namingWeakOrdered;
   i2.d.begin = Range<long>( iRoot.d.begin.begin, i.d.begin);
   i2.d.end = Range<long>( iRoot.d.begin.begin, i.d.end);

   matchedRangesRelative->push_back(i2);
  }
 
  return std::make_tuple(complete, matchedRangesRelative);
 }

 std::tuple<std::shared_ptr<std::vector<MatchRange<long>>>> 
 cb_rmMatchRanges2Lua(std::shared_ptr<std::vector<MatchRange<long>>> matchedRangesRelative)
 {
  return std::make_tuple( matchedRangesRelative);
 }

 std::tuple<> cb_rmConsole(lua_State * L) 
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

  Console(StreamPair{std::cin, std::cout}, rt.mapLua2LuaBase.at(L).lock(), rt.rangeMatcherMethods4Lua);

  return std::tuple<>();
 }

 std::tuple<> cb_rmConsoleNew(lua_State * L) 
 {
  Console(StreamPair{std::cin, std::cout});

  return std::tuple<>();
 }

 std::tuple<long> cb_rmDebugSetInt(long l)
 {
  std::cout << __func__ << std::endl;
  return std::tuple<long>( l);
 }

 std::tuple<long> cb_rmDebugGetInt(long l)
 {
  std::cout << __func__ << std::endl;
  return std::tuple<long>(l);
 }
}

LuaInstance currentLuaInstance( lua_State * L)
{
 auto & rt = *lua2RangeMatcherLuaRuntime.at(L);
 rt.lastErrorMessage = ""; // TODO : clanup on reading
 return LuaInstance( L, rt);
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

 registerFunction( "rmClear", LuaFunction3<
  CallingParameter<S<lua_State>>,
  ReturningParameter<>, cb_rmClear>::call, 
  "clears the variables vector");

 registerFunction( "rmNextObjectIndex", LuaFunction3<
  CallingParameter<S<lua_State>>,
  ReturningParameter<P<long>>, cb_rmNextObjectIndex>::call, 
  "outputs the next index of the variables vector");

 registerFunction( "rmConsole", LuaFunction3<
  CallingParameter<S<lua_State>>,
  ReturningParameter<>, cb_rmConsole>::call,
  "starts a console with current settings");

 registerFunction( "rmConsoleNew", LuaFunction3<
  CallingParameter<S<lua_State>>,
  ReturningParameter<>, cb_rmConsoleNew>::call,
  "starts a console with new settings");

 registerFunction( "rmDebugSetInt", LuaFunction3<
  CallingParameter<P<long>>,
  ReturningParameter<V<long>>,cb_rmDebugSetInt>::call,
  "saves the given integer value to the variables vector v2");

 registerFunction( "rmDebugGetInt", 
  LuaFunction3<CallingParameter<V<long>>,
  ReturningParameter<P<long>>,cb_rmDebugGetInt>::call,
  "outputs the integer value from the variables vector at the given position v2");

 registerFunction( "rmFileRead", LuaFunction3N<
  CallingParameter<P<std::string>>,
  ReturningParameter<V<std::shared_ptr<std::vector<char>>>>,cb_rmFileRead>::call, 
  "reads one file per parameter");

 registerFunction( "rmFunctions", LuaFunction3<
  CallingParameter<S<lua_State>>,
  ReturningParameter<P<std::shared_ptr<std::vector<std::string>>>>,cb_rmFunctions>::call, 
  "returns all function names");

 registerFunction( "rmHelp", LuaFunction3<
  CallingParameter<S<lua_State>>,
  ReturningParameter<>,cb_rmHelp>::call, 
  "outputs help");

 registerFunction( "rmMatchRanges", LuaFunction3<
  CallingParameter<V<std::shared_ptr<NonOverlappingMatcher>>, V<std::shared_ptr<std::vector<char>>>>,
  ReturningParameter<P<bool>, V<std::shared_ptr<std::vector<MatchRange<long>>>>>, cb_rmMatchRanges>::call,
  "creates a vector of match ranges from a non overlapping matcher");
  
 registerFunction( "rmMatchRanges2Lua", LuaFunction3<
  CallingParameter<V<std::shared_ptr<std::vector<MatchRange<long>>>>>,
  ReturningParameter<P<std::shared_ptr<std::vector<MatchRange<long>>>>>,cb_rmMatchRanges2Lua>::call,
  "converts the match ranges to lua datatypes");

 registerFunction( "rmNamedPatternRange", LuaFunction3<
  CallingParameter<P<std::string>, V<std::shared_ptr<const Pattern>>, V<std::shared_ptr<const Pattern>>>,
  ReturningParameter<V<std::shared_ptr<const NamedPatternRange>>>, cb_rmNamedPatternRange>::call,
  "a named pattern range, consists of a name and 2 pattern");

 registerFunction( "rmNamedPatternRangeVector", LuaFunction3NVector<
  CallingParameter<V<std::shared_ptr<const NamedPatternRange>>>,
  ReturningParameter<V<std::shared_ptr<const NamedPatternRange>>>, cb_rmNamedPatternRangeVector>::call,
  "a vector of named pattern ranges");

 registerFunction( "rmNamedPatternRangeVector1", LuaFunction3NVector1<
  CallingParameter<V<std::shared_ptr<const NamedPatternRange>>>,
  ReturningParameter<V<std::shared_ptr<const NamedPatternRange>>>, cb_rmNamedPatternRangeVector>::call,
  "a vector of named pattern ranges");

 registerFunction( "rmCheckNamedPatternRange", LuaFunction3<
  CallingParameter<V<std::shared_ptr<std::vector<std::tuple<std::shared_ptr<const NamedPatternRange>>>>>>,
  ReturningParameter<>,cb_rmCheckNamedPatternRange>::call,
  "check for debugging purposes");

 registerFunction( "rmCheckNamedPatternRange1", LuaFunction3<
  CallingParameter<V<std::shared_ptr<std::vector<std::shared_ptr<const NamedPatternRange>>>>>,
  ReturningParameter<>,cb_rmCheckNamedPatternRange1>::call,
  "check for debugging purposes");

 registerFunction( "rmNonOverlappingMatcher1", LuaFunction3N<
  CallingParameter<V<std::shared_ptr<std::vector<std::shared_ptr<const NamedPatternRange>>>>>,
  ReturningParameter<V<std::shared_ptr<NonOverlappingMatcher>>>, cb_rmNonOverlappingMatcher1>::call,
  "creates a non overlapping matcher from all given pattern ranges");

 registerFunction( "rmPatternRegex", LuaFunction3N<
  CallingParameter<P<std::string>>,
  ReturningParameter<V<std::shared_ptr<PatternRegex>, std::shared_ptr<const Pattern>>>,cb_rmPatternRegex>::call,
  "reads one file per parameter");

 registerFunction( "rmPatternString", LuaFunction3N<
  CallingParameter<P<std::string>>,
  ReturningParameter<V<std::shared_ptr<PatternString>, std::shared_ptr<const Pattern>>>,cb_rmPatternString>::call,
  "reads one file per parameter");

 registerFunction( "rmToggleDebug", LuaFunction3<
  CallingParameter<S<lua_State>>,
  ReturningParameter<>,cb_rmToggleDebug>::call,
  "toggles the debug flag and outputs its state");

}

const std::string RangeMatcherMethods4Lua::getLastErrorMessage() const
{
 std::ostringstream oss;

 const auto & thisRmlr = rangeMatcherMethods4LuaInstances.at(this);

 return thisRmlr.lastErrorMessage;
 
 return oss.str();
}

