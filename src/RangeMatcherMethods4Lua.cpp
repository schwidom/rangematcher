
// TODO : remove outdated TODOs
// - extract tools
// - LuaFunction3[N] for creating and expanding vectors

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

 template <class T> struct P; 
 template <class ... T> struct V; 

 // NOTE: P<T> represents lua parameters of type T which are to be used inside the functions
 // NOTE: V<T> represents parameters of type T which are represented by descriptors referring to "vectorOfObjects"
 // NOTE: V<T> ... in case of returning the parameter gets saved and the descriptor is returned

 template <class T> struct ReadParameter;

 template <> struct ReadParameter<P<lua_State>> // TODO : possibly L<lua_State> (must not get parameter) + returning parameter count as constexpr
 {
  using convertedType = lua_State *;

  static constexpr int numberOfLuaArguments = 0;

  static convertedType doIt(lua_State *L, int offset) // length check is already done, pop is later expected
  {
   return L;
  }
 };

 template <> struct ReadParameter<P<long>>
 {
  using convertedType = long;

  static constexpr int numberOfLuaArguments = 1;

  static convertedType doIt(lua_State *L, int offset) // length check is already done, pop is later expected
  {
   return lua_tointeger(L, offset); // 1
  }
 };

 template <> struct ReadParameter<P<std::string>>
 {
  using convertedType = std::string;

  static constexpr int numberOfLuaArguments = 1;

  static convertedType doIt(lua_State *L, int offset) // length check is already done, pop is later expected
  {
   return lua_tostring(L, offset); // 1
  }
 };

 void raiseLuaError( lua_State * L, std::string msg); // TODO : function ordering

 template <class T> struct ReadParameter<V<T>> // TODO : T was long, currently untested
 {
  using convertedType = T;

  static constexpr int numberOfLuaArguments = 1;

  static T doIt(lua_State *L, int offset) // length check is already done, pop is later expected
  {
   auto & rt = *lua2RangeMatcherLuaRuntime.at(L); // TODO : provide already existing rt

   long intIdx{lua_tointeger(L, offset)}; // 1

   auto savedValue(rt.vectorOfObjects.at(intIdx)->get<T>());

   if( !savedValue)
   {
    raiseLuaError( L, "wrong type at index " + std::to_string(intIdx) + " argument : " + std::to_string( offset));
   }

   return *savedValue;
  }
 };

 template<class ... T>struct tuple_type_cat;

 template<class T, class ... Tp> struct tuple_type_cat<std::tuple<T>,std::tuple<Tp...>>
 {
  using type = std::tuple<T,Tp...>;
 };

 template <class T> struct ReadParameters;

 template <class T, class ... Tp> struct ReadParameters<std::tuple<T, Tp...>>
 {
  using convertedType = typename tuple_type_cat<
   std::tuple<typename ReadParameter<T>::convertedType>, 
   typename ReadParameters<std::tuple<Tp...>>::convertedType
   >::type;

  static constexpr int numberOfLuaArguments = 0 
   + ReadParameter<T>::numberOfLuaArguments
   + ReadParameters<std::tuple<Tp...>>::numberOfLuaArguments;

  // static std::tuple<T, Tp...> doIt(lua_State *L, int offset)
  static decltype(auto) doIt(lua_State *L, int offset)
  {
   return std::tuple_cat(
    std::make_tuple(ReadParameter<T>::doIt(L, offset)), 
    ReadParameters<std::tuple<Tp...>>::doIt(L, 1 + offset)
   );
  }
 };

 template <> struct ReadParameters<std::tuple<>>
 {
  using convertedType = std::tuple<>;

  static constexpr int numberOfLuaArguments = 0;

  static std::tuple<> doIt(lua_State *L, int offset)
  {
   return std::tuple<>();
  }
 };

 template <class T> struct WriteParameter;

 template <> struct WriteParameter<P<long>>
 {
  using convertedType = long;
  static int doIt(lua_State *L, int offset, convertedType value) 
  {
   // TODO : prove stack size
   lua_pushinteger(L, value);
   return 1;
  }
 };

 template <> struct WriteParameter<P<std::shared_ptr<std::vector<std::string>>>>
 {
  using convertedType = std::shared_ptr<std::vector<std::string>>;
  static int doIt(lua_State *L, int offset, convertedType values) 
  {
   // TODO : prove stack size
   for( const auto & value : *values)
   {
    lua_pushstring(L, value.c_str());
   }

   return values->size();
  }
 };

 template <class T > struct WriteParameter<V<T>> // TODO : addTypes variant
 {
  using convertedType = T;
  static int doIt(lua_State *L, int offset, convertedType value) 
  {
   auto & rt = *lua2RangeMatcherLuaRuntime.at(L); // TODO : provide already existing rt

   std::unique_ptr<Any<T>> returnValue(std::make_unique<Any<T>>(value));
   rt.vectorOfObjects.push_back( std::move(returnValue));

   // TODO : prove stack size
   lua_pushinteger(L, rt.vectorOfObjects.size() - 1);
   return 1;
  }
 };

 template <class T, class ... Tadditional > struct WriteParameter<V<T, Tadditional...>> // TODO : addTypes variant
 {
  using convertedType = T;
  static int doIt(lua_State *L, int offset, convertedType value) 
  {
   auto & rt = *lua2RangeMatcherLuaRuntime.at(L); // TODO : provide already existing rt

   std::unique_ptr<Any<T>> returnValue(std::make_unique<Any<T>>(value));
   returnValue -> template addTypes<Tadditional...>();
   rt.vectorOfObjects.push_back( std::move(returnValue));

   // TODO : prove stack size
   lua_pushinteger(L, rt.vectorOfObjects.size() - 1);
   return 1;
  }
 };

 template < std::size_t... Ns , typename... Ts >
 auto tail_impl( std::index_sequence<Ns...> , std::tuple<Ts...> t )
 {
  return  std::make_tuple( std::get<Ns+1u>(t)... );
 }
 
 template < typename... Ts >
 auto tail( std::tuple<Ts...> t )
 {
  return  tail_impl( std::make_index_sequence<sizeof...(Ts) - 1u>() , t );
 }

 template <class T> struct WriteParameters;

 template <class T, class ... Tp> struct WriteParameters<std::tuple<T, Tp...>>
 {
  using convertedType = typename tuple_type_cat<
   std::tuple<typename WriteParameter<T>::convertedType>, 
   typename WriteParameters<std::tuple<Tp...>>::convertedType
   >::type;

  // static std::tuple<T, Tp...> doIt(lua_State *L, int offset)
  static int doIt(lua_State *L, int offset, convertedType parameters)
  {
    return 0 
     + WriteParameter<T>::doIt(L, offset, std::get<0>(parameters))
     + WriteParameters<std::tuple<Tp...>>::doIt(L, 1 + offset, tail(parameters));
  }
 };

 template <> struct WriteParameters<std::tuple<>>
 {
  using convertedType = std::tuple<>;

  static int doIt(lua_State *L, int offset, std::tuple<>)
  {
   return 0;
  }
 };

 void chkArguments( lua_State * L, int n, std::string functionName); // TODO : order functions
 int chkArguments( lua_State * L, int nMin, int nMax, std::string functionName); // TODO : order functions

 template <class T> typename ReadParameters<T>::convertedType readParameterIntoTuple(lua_State *L)
 {
  int numberOfLuaArguments{ReadParameters<T>::numberOfLuaArguments};
  chkArguments( L, numberOfLuaArguments, __func__);

  auto ret = ReadParameters<T>::doIt(L, 1);

  lua_pop(L, numberOfLuaArguments);

  return ret;
 }

 template <class T> typename ReadParameters<T>::convertedType readParameterIntoTupleN(lua_State *L, int offset)
 {
  return ReadParameters<T>::doIt(L, offset);
 }

 template <class T> int writeParameterFromTuple(lua_State *L, typename WriteParameters<T>::convertedType parameters)
 {
  // TODO prove possible needed stack extension
  return WriteParameters<T>::doIt(L, 1, parameters);
  // return std::tuple_size<T>::value;
 }

 template<class T_CallingParameter, class T_ReturningParameter, 
  class T_CallingReturnType = typename ReadParameters<typename T_CallingParameter::type>::convertedType,
  class T_ReturningReturnType = typename WriteParameters<typename T_ReturningParameter::type>::convertedType> struct FunctionType3;

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

 template <class T_CallingParameter, class T_ReturningParameter, 
  typename FunctionType3<T_CallingParameter, T_ReturningParameter>::type FunctionOfInterest>
 struct LuaFunction3 // WEITERBEI // TODO
 {
  // typedef int (*lua_CFunction) (lua_State *L);
  static int call(lua_State *L)
  {
   auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

   rt.lastErrorMessage = "";
   
   auto readin = readParameterIntoTuple<typename T_CallingParameter::type>(L); 

   using FunctionType3Instance = FunctionType3<T_CallingParameter, T_ReturningParameter>;

   typename FunctionType3Instance::returnType ret = FunctionType3Instance::call(FunctionOfInterest, readin);

   return writeParameterFromTuple<typename T_ReturningParameter::type>(L, ret);
  }
 };

 template <class T_CallingParameter, class T_ReturningParameter, 
  typename FunctionType3<T_CallingParameter, T_ReturningParameter>::type FunctionOfInterest,
  // typename std::enable_if<true,int>::type = 0>
  // typename std::enable_if<std::tuple_size<typename T_CallingParameter::type>::value >= 1,int>::type = 0>
  typename std::enable_if<ReadParameters<typename T_CallingParameter::type>::numberOfLuaArguments >= 1,int>::type = 0>
 struct LuaFunction3N // WEITERBEI // TODO : tests overdue
 {
  // typedef int (*lua_CFunction) (lua_State *L);
  static int call(lua_State *L)
  {
   auto & rt = *lua2RangeMatcherLuaRuntime.at(L); // TODO : implement own Alloc scheme and extend with own control data

   rt.lastErrorMessage = "";
   
   using FunctionType3Instance = FunctionType3<T_CallingParameter, T_ReturningParameter>;

   int ret = 0;

   int numberOfArguments = chkArguments( L, 0, std::numeric_limits<int>::max(), __func__);

   std::list< typename ReadParameters<typename T_CallingParameter::type>::convertedType > queue;

   int inputTupleSize{std::tuple_size<typename T_CallingParameter::type>::value}; 

   if( 0 == inputTupleSize && 0 != numberOfArguments)
   { 
    raiseLuaError( L, "function with 0 parameters must get exactly 0 parameters");
   }

   for( int argumentNr = 0 ; argumentNr < numberOfArguments; argumentNr += inputTupleSize)
   {

    std::cout << "chkArguments " << chkArguments( L, 0, std::numeric_limits<int>::max(), __func__) << std::endl;
    std::cout << "argumentNr " << argumentNr <<  std::endl;

    auto readin = readParameterIntoTupleN<typename T_CallingParameter::type>(L, 1 + argumentNr);

    queue.push_back(readin);
   }

   lua_pop(L, numberOfArguments); 

   for( auto & readin : queue)
   {
    typename FunctionType3Instance::returnType res = FunctionType3Instance::call(FunctionOfInterest, readin);

    ret += writeParameterFromTuple<typename T_ReturningParameter::type>(L, res);
   }
   
   return ret;
  }
 };

/*
<< l rmLuaFunctionTest3(1)
executing lua:  rmLuaFunctionTest3(1)
callback2_3 1
success
*/

 std::tuple<> callback2_3(long t1)
 { 
  std::cout << __func__ << " " << t1 << std::endl;
  return std::tuple<>();
 }

 void toDelete() // examplecalls // TODO
 {
  ReadParameter<P<long>>::doIt(nullptr, 1);
  ReadParameters<std::tuple<P<long>>>::doIt(nullptr, 1);
  ReadParameters<std::tuple<P<long>,P<long>>>::doIt(nullptr, 1);
  readParameterIntoTuple<std::tuple<P<long>>>(nullptr);
  readParameterIntoTuple<std::tuple<P<long>,P<long>>>(nullptr);
  std::tuple<> t0;
  std::tuple_cat<>(t0, t0);
  ReadParameters<std::tuple<>> rp1;
  WriteParameters<std::tuple<>> wp1;
  WriteParameters<std::tuple<>>::doIt(nullptr, 1, std::tuple<>());
  readParameterIntoTuple<std::tuple<>>(nullptr);
  writeParameterFromTuple<std::tuple<>>(nullptr, std::tuple<>());
  WriteParameter<P<long>>::doIt(nullptr, 1, 1L);
  WriteParameters<std::tuple<P<long>>> wp2;
  WriteParameters<std::tuple<P<long>>>::doIt(nullptr, 1, std::tuple<long>(1L));
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

  std::cout << "cb_rmNamedPatternRange" << std::endl;
  std::cout << nameString << std::endl;

  return std::make_tuple(cnpt);
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

 int rmClear(lua_State * L)
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

  rt.lastErrorMessage = "";

  int numberOfArguments{0};
  chkArguments( L, numberOfArguments, __func__);
 
  rt.vectorOfObjects.clear();

  return 0;
 }

 std::tuple<std::shared_ptr<std::vector<char>>>
  cb_rmFileRead( std::string stringOfInterest) // see cb_rmDebugSetInt2
 {
   // if( rt.debug){ std::cout << "stringOfInterest " << stringOfInterest << std::endl;}
   std::cout << "stringOfInterest " << stringOfInterest << std::endl;
   return std::make_tuple(std::shared_ptr<std::vector<char>>(FileToVector(stringOfInterest).get()));
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

 std::tuple<> cb_rmConsole(lua_State * L) 
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

  Console(StreamPair{std::cin, std::cout}, rt.mapLua2LuaBase.at(L).lock(), rt.rangeMatcherMethods4Lua);

  return std::tuple<>();
 }

 std::tuple<> cb_rmConsoleNew(lua_State * L) 
 {
  auto & rt = *lua2RangeMatcherLuaRuntime.at(L);

  Console(StreamPair{std::cin, std::cout});

  return std::tuple<>();
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

 std::tuple<long> cb_rmDebugSetInt2(long l)
 {
  std::cout << __func__ << std::endl;
  return std::tuple<long>( l);
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

 std::tuple<long> cb_rmDebugGetInt2(long l)
 {
  std::cout << __func__ << std::endl;
  return std::tuple<long>(l);
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

 registerFunction( "rmLuaFunctionTest3", LuaFunction3<CallingParameter<P<long>>,ReturningParameter<>,callback2_3>::call, "test3");

 REGISTER( rmClear, "clears the variables vector");

 registerFunction( "rmNextObjectIndex", LuaFunction3<
  CallingParameter<P<lua_State>>,
  ReturningParameter<P<long>>,
  cb_rmNextObjectIndex>::call, 
  "outputs the next index of the variables vector");

 registerFunction( "rmConsole", LuaFunction3<
  CallingParameter<P<lua_State>>,
  ReturningParameter<>,
  cb_rmConsole>::call,
  "starts a console with current settings");

 registerFunction( "rmConsoleNew", LuaFunction3<
  CallingParameter<P<lua_State>>,
  ReturningParameter<>,
  cb_rmConsoleNew>::call,
  "starts a console with new settings");

 REGISTER( rmDebugSetInt, "saves the given integer value to the variables vector");
 REGISTER( rmDebugGetInt, "outputs the integer value from the variables vector at the given position");

 registerFunction( "rmDebugSetInt2", LuaFunction3<CallingParameter<P<long>>,ReturningParameter<V<long>>,cb_rmDebugSetInt2>::call, "saves the given integer value to the variables vector v2");
 registerFunction( "rmDebugGetInt2", LuaFunction3<CallingParameter<V<long>>,ReturningParameter<P<long>>,cb_rmDebugGetInt2>::call, "outputs the integer value from the variables vector at the given position v2");

 registerFunction( "rmFileRead", LuaFunction3N<
  CallingParameter<P<std::string>>,
  ReturningParameter<V<std::shared_ptr<std::vector<char>>>>,cb_rmFileRead>::call, 
  "reads one file per parameter");

 registerFunction( "rmFunctions", LuaFunction3<
  CallingParameter<P<lua_State>>,
  ReturningParameter<P<std::shared_ptr<std::vector<std::string>>>>,cb_rmFunctions>::call, 
  "returns all function names");

 registerFunction( "rmHelp", LuaFunction3<
  CallingParameter<P<lua_State>>,
  ReturningParameter<>,cb_rmHelp>::call, 
  "outputs help");

 REGISTER( rmMatchRanges, "creates a vector of match ranges from a non overlapping matcher");

 REGISTER( rmMatchRanges2Lua, "converts the match ranges to lua datatypes");

 registerFunction( "rmNamedPatternRange", LuaFunction3<
  CallingParameter<P<std::string>, V<std::shared_ptr<const Pattern>>, V<std::shared_ptr<const Pattern>>>,
  ReturningParameter<V<std::shared_ptr<const NamedPatternRange>>>, cb_rmNamedPatternRange>::call,
  "a named pattern range, consists of a name and 2 pattern");

 REGISTER( rmNamedPatternRangeVector, "a vector of named pattern ranges");
 REGISTER( rmNonOverlappingMatcher, "creates a non overlapping matcher from all given pattern ranges ");

 registerFunction( "rmPatternRegex", LuaFunction3N<
  CallingParameter<P<std::string>>,
  ReturningParameter<V<std::shared_ptr<PatternRegex>, std::shared_ptr<const Pattern>>>,cb_rmPatternRegex>::call,
  "reads one file per parameter");

 registerFunction( "rmPatternString", LuaFunction3N<
  CallingParameter<P<std::string>>,
  ReturningParameter<V<std::shared_ptr<PatternString>, std::shared_ptr<const Pattern>>>,cb_rmPatternString>::call,
  "reads one file per parameter");

 registerFunction( "rmToggleDebug", LuaFunction3<
  CallingParameter<P<lua_State>>,
  ReturningParameter<>,cb_rmToggleDebug>::call,
  "toggles the debug flag and outputs its state");
#undef REGISTER

}

const std::string RangeMatcherMethods4Lua::getLastErrorMessage() const
{
 std::ostringstream oss;

 const auto & thisRmlr = rangeMatcherMethods4LuaInstances.at(this);

 return thisRmlr.lastErrorMessage;
 
 return oss.str();
}

