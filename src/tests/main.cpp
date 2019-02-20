
#include "Any.hpp"
#include "FileToVector.hpp"
#include "InterpreterLua.hpp"
#include "LuaBase.hpp"
#include "LuaTest.hpp"
#include "MatchRange.hpp"
#include "NamedPatternRange.hpp"
#include "NamingWeakOrdered.hpp"
#include "NonOverlappingMatcher.hpp"
#include "PatternRange.hpp"
#include "PatternRegex.hpp"
#include "PatternString.hpp"
#include "PatternVector.hpp"
#include "Range.hpp"
#include "RangeMatcherMethods4Lua.hpp"

#include <algorithm> // sort
#include <iostream>
#include <iterator> // distance
#include <utility>
#include <vector>

#define TYPE std::vector<char>

void test()
{

 std::cout << __func__ << std::endl;

 std::vector<char> v{'1','a','b','c','2'};

 std::vector<char> v2{'b','c'};

 PatternString ps1{"abc"};

 PatternString ps2{"def"};

 PatternVector pv1{v2};

 std::unique_ptr<Match<TYPE>> m1{ ps1.match(Range<TYPE>{v.begin(), v.end()})}; 
 std::unique_ptr<Match<TYPE>> m2{ ps2.match(Range<TYPE>{v.begin(), v.end()})}; 

 // std::shared_ptr<MatchGot<TYPE>> mg2 = m2->get(); // !matched exception
 
 std::cout << ! m2->matched() << std::endl;
 std::cout << m1->matched() << std::endl;

 std::shared_ptr<const MatchGot<TYPE>> mg1 = m1->get();

 std::cout << ( mg1->m_Range.begin == v.begin() + 1 ) << std::endl;
 std::cout << ( mg1->m_Range.end == v.begin() + 4 ) << std::endl;

 std::unique_ptr<Match<TYPE>> m3{ pv1.match(Range<TYPE>{v.begin(), v.end()})};

 std::cout << m3->matched() << std::endl;

 std::shared_ptr<const MatchGot<TYPE>> mg2 = m3->get();

 std::cout << ( mg2->m_Range.begin == v.begin() + 2 ) << std::endl;
 std::cout << ( mg2->m_Range.end == v.begin() + 4 ) << std::endl;

 PatternRegex pr1{"a.*c"};
 PatternRegex pr2{"c.*a"};

 std::unique_ptr<Match<TYPE>> m4{ pr1.match(Range<TYPE>{v.begin(), v.end()})};
 std::unique_ptr<Match<TYPE>> m5{ pr2.match(Range<TYPE>{v.begin(), v.end()})};

 std::cout << ! m5->matched() << std::endl;
 std::cout << m4->matched() << std::endl;

 std::shared_ptr<const MatchGot<TYPE>> mg3 = m4->get();

 std::cout << ( mg3->m_Range.begin == v.begin() + 1 ) << std::endl;
 std::cout << ( mg3->m_Range.end == v.begin() + 4 ) << std::endl;

 auto v3( FileToVector("testfiles/001.txt").get());

 std::unique_ptr<Match<TYPE>> m6{ ps2.match(Range<TYPE>{v3->begin(), v3->end()})}; 

 std::cout << m6->matched() << std::endl;

 std::shared_ptr<const MatchGot<TYPE>> mg4 = m6->get();

 std::cout << ( mg4->m_Range.begin == v3->begin() + 1 ) << std::endl;
 std::cout << ( mg4->m_Range.end == v3->begin() + 4 ) << std::endl;
 
}

void test2()
{

 std::cout << __func__ << std::endl;

 PatternRange prComment1{ PatternRange::I{std::make_shared<PatternString>( "/*"), std::make_shared<PatternString>( "*/")}};
 PatternRange prComment2{ PatternRange::I{std::make_shared<PatternString>( "//"), std::make_shared<PatternString>( "\n")}};
 std::shared_ptr<Pattern> patternDoubleQuote{std::make_shared<PatternString>("\"")};
 PatternRange prString{ PatternRange::I{patternDoubleQuote, patternDoubleQuote}};
 
 auto vectorTf002(FileToVector("testfiles/002.txt").get());

 struct Status {
  std::string name;
  std::unique_ptr<Match<TYPE>> match;
 };

 using P = std::pair<PatternRange * , Status>;

 std::vector<P> v;

 v.push_back(P(&prComment1, Status{"prComment1"}));
 v.push_back(P(&prComment2, Status{"prComment2"}));
 v.push_back(P(&prString, Status{"prString"}));

 for( auto & value : v) {
  std::unique_ptr<Match<TYPE>> match { value.first->i().von->match(Range<TYPE>{vectorTf002->begin(), vectorTf002->end()})};
  value.second.match = std::move(match);
 }

 std::sort(v.begin(), v.end(), [](P& p1, P& p2) -> bool {
  if( p1.second.match->matched())
  {
   if( !p2.second.match->matched())
   {
    return true;
   }
   else
   {
    return p1.second.match->get()->m_Range.begin < p2.second.match->get()->m_Range.begin;
   }
  }

  return false;
 });

 std::cout << ("prComment1" == v.at(0).second.name) << std::endl;
 std::cout << ("prString" == v.at(1).second.name) << std::endl;
 std::cout << ("prComment2" == v.at(2).second.name) << std::endl;

 std::cout << (v.at(0).second.match->matched()) << std::endl;
 std::cout << (v.at(1).second.match->matched()) << std::endl;
 std::cout << (!v.at(2).second.match->matched()) << std::endl;

 std::cout << ( 0 == std::distance( vectorTf002->begin() , v.at(0).second.match->get()->m_Range.begin)) << std::endl;
 std::cout << ( 3 == std::distance( vectorTf002->begin() , v.at(1).second.match->get()->m_Range.begin)) << std::endl;

}

void test3()
{

 std::cout << __func__ << std::endl;

 NamedPatternRange prComment1{{"prComment1", std::make_shared<PatternString>( "/*"), std::make_shared<PatternString>( "*/")}};
 NamedPatternRange prComment2{{"prComment2", std::make_shared<PatternString>( "//"), std::make_shared<PatternString>( "\n")}};
 std::shared_ptr<Pattern> patternDoubleQuote{std::make_shared<PatternString>("\"")};
 NamedPatternRange prString{{"prString", patternDoubleQuote, patternDoubleQuote}};

 
 auto vectorTf002(FileToVector("testfiles/002.txt").get());

 struct Status {
  std::unique_ptr<Match<TYPE>> match;
 };

 using P = std::pair<NamedPatternRange * , Status>;

 std::vector<P> v;

 v.push_back(P(&prComment1, Status{}));
 v.push_back(P(&prComment2, Status{}));
 v.push_back(P(&prString, Status{}));

 for( auto & value : v) {
  std::unique_ptr<Match<TYPE>> match { value.first->i().von->match(Range<TYPE>{vectorTf002->begin(), vectorTf002->end()})};
  value.second.match = std::move(match);
 }

 std::sort(v.begin(), v.end(), [](P& p1, P& p2) -> bool {
  if( p1.second.match->matched())
  {
   if( !p2.second.match->matched())
   {
    return true;
   }
   else
   {
    return p1.second.match->get()->m_Range.begin < p2.second.match->get()->m_Range.begin;
   }
  }

  return false;
 });

 std::cout << ("prComment1" == v.at(0).first->getName()) << std::endl;
 std::cout << ("prString" == v.at(1).first->getName()) << std::endl;
 std::cout << ("prComment2" == v.at(2).first->getName()) << std::endl;

 std::cout << (v.at(0).second.match->matched()) << std::endl;
 std::cout << (v.at(1).second.match->matched()) << std::endl;
 std::cout << (!v.at(2).second.match->matched()) << std::endl;

 std::cout << ( 0 == std::distance( vectorTf002->begin() , v.at(0).second.match->get()->m_Range.begin)) << std::endl;
 std::cout << ( 3 == std::distance( vectorTf002->begin() , v.at(1).second.match->get()->m_Range.begin)) << std::endl;
}

void test4()
{
 std::cout << __func__ << std::endl;

 using CNPT = const NamedPatternRange;

 auto prComment1( std::make_shared<CNPT>(CNPT::I{"prComment1", std::make_shared<PatternString>( "/*"), std::make_shared<PatternString>( "*/")}));

 auto prComment2( std::make_shared<CNPT>(CNPT::I{"prComment2", std::make_shared<PatternString>( "//"), std::make_shared<PatternString>( "\n")}));
 auto patternDoubleQuote(std::make_shared<PatternString>("\""));
 auto prString(std::make_shared<CNPT>(CNPT::I{"prString", patternDoubleQuote, patternDoubleQuote}));

 std::vector<std::shared_ptr<CNPT>> patternRangeVector{ prComment1, prComment2, prString};

 NonOverlappingMatcher nonOverlappingMatcher{patternRangeVector};

 auto vectorTf( FileToVector("testfiles/002.txt").get());

 auto matchedRanges ( nonOverlappingMatcher.matchAll(Range<TYPE>{vectorTf->begin(), vectorTf->end()}));

 std::cout << ( 3 == matchedRanges->size()) << std::endl;

 {
  auto & mr( matchedRanges->at(0));
  auto data( mr.i().d);
  std::cout << ( "initial-element-jc3jvchtrz" == mr.getName()) << std::endl;
  std::cout << data.complete << std::endl;
  std::cout << ( vectorTf->size() == static_cast<size_t>(std::distance(data.begin.begin, data.end.end))) << std::endl; 
 }

 {
  auto & mr( matchedRanges->at(1));
  auto data( mr.i().d);
  std::cout << ( "prComment1" == mr.getName()) << std::endl;
  std::cout << data.complete << std::endl;
  std::cout << ( "/* \" */" == std::string(data.begin.begin, data.end.end)) << std::endl;
 }

 {
  auto & mr( matchedRanges->at(2));
  auto data( mr.i().d);
  std::cout << ( "prString" == mr.getName()) << std::endl;
  std::cout << data.complete << std::endl;
  std::cout << ( "\" /* \"" == std::string(data.begin.begin, data.end.end)) << std::endl;
 }
}

void test5()
{
 std::cout << __func__ << std::endl;

 using CNPT = const NamedPatternRange;

 auto prComment1( std::make_shared<CNPT>(CNPT::I{"prComment1", std::make_shared<PatternString>( "/*"), std::make_shared<PatternString>( "*/")}));

 auto prComment2( std::make_shared<CNPT>(CNPT::I{"prComment2", std::make_shared<PatternString>( "//"), std::make_shared<PatternString>( "\n")}));
 auto patternDoubleQuote(std::make_shared<PatternString>("\""));
 auto prString(std::make_shared<CNPT>(CNPT::I{"prString", patternDoubleQuote, patternDoubleQuote}));

 std::vector<std::shared_ptr<CNPT>> patternRangeVector{ prComment1, prComment2, prString};

 NonOverlappingMatcher nonOverlappingMatcher{patternRangeVector};

 auto vectorTf( FileToVector("testfiles/003.txt").get());

 auto matchedRanges ( nonOverlappingMatcher.matchAll(Range<TYPE>{vectorTf->begin(), vectorTf->end()}));

 std::cout << ( 3 == matchedRanges->size()) << std::endl;

 {
  auto & mr( matchedRanges->at(0));
  auto data( mr.i().d);
  std::cout << ( "initial-element-jc3jvchtrz" == mr.getName()) << std::endl;
  std::cout << data.complete << std::endl;
  std::cout << ( vectorTf->size() == static_cast<size_t>(std::distance(data.begin.begin, data.end.end))) << std::endl; 
 }

 {
  auto & mr( matchedRanges->at(1));
  auto data( mr.i().d);
  std::cout << ( "prComment1" == mr.getName()) << std::endl;
  std::cout << data.complete << std::endl;
  std::cout << ( "/* \" */" == std::string(data.begin.begin, data.end.end)) << std::endl;
 }

 {
  auto & mr( matchedRanges->at(2));
  auto data( mr.i().d);
  std::cout << ( "prString" == mr.getName()) << std::endl;
  std::cout << !data.complete << std::endl;
  std::cout << ( "\" /* X\n" == std::string(data.begin.begin, data.end.end)) << std::endl;
 }
}

void test6( std::string fname)
{
 std::cout << __func__ << " " << fname << std::endl;

 using CNPT = const NamedPatternRange;

 auto prComment1( std::make_shared<CNPT>(CNPT::I{"prComment1", std::make_shared<PatternString>( "/*"), std::make_shared<PatternString>( "*/")}));

 auto prComment2( std::make_shared<CNPT>(CNPT::I{"prComment2", std::make_shared<PatternString>( "//"), std::make_shared<PatternString>( "\n")}));

 auto patternDoubleQuoteStart(std::make_shared<PatternString>("\""));
 auto patternDoubleQuoteEnd(std::make_shared<PatternRegex>("([^\\\\]|^)\"")); 

 auto prString1(std::make_shared<CNPT>(CNPT::I{"prString1", patternDoubleQuoteStart, patternDoubleQuoteEnd}));

 auto patternApostrophStart(std::make_shared<PatternString>("'"));
 auto patternApostrophEnd(std::make_shared<PatternRegex>("([^\\\\]|^)'")); 

 auto prString2(std::make_shared<CNPT>(CNPT::I{"prString2", patternApostrophStart, patternApostrophEnd}));

 std::vector<std::shared_ptr<CNPT>> patternRangeVector{ prComment1, prComment2, prString1, prString2};

 NonOverlappingMatcher nonOverlappingMatcher{patternRangeVector};

 auto vectorTf( FileToVector(fname).get());

 auto matchedRanges ( nonOverlappingMatcher.matchAll(Range<TYPE>{vectorTf->begin(), vectorTf->end()}));

 std::cout << ( 8 == matchedRanges->size()) << std::endl;

 {
  auto & mr( matchedRanges->at(0));
  auto data( mr.i().d);
  std::cout << ( "initial-element-jc3jvchtrz" == mr.getName()) << std::endl;
  std::cout << data.complete << std::endl;
  std::cout << ( vectorTf->size() == static_cast<size_t>(std::distance(data.begin.begin, data.end.end))) << std::endl; 
 }

 {
  auto & mr( matchedRanges->at(1));
  auto data( mr.i().d);
  std::cout << ( "prComment2" == mr.getName()) << std::endl;
  std::cout << data.complete << std::endl;
  std::cout << ( "// /* \" \'\n" == std::string(data.begin.begin, data.end.end)) << std::endl;
 }

 {
  auto & mr( matchedRanges->at(2));
  auto data( mr.i().d);
  std::cout << ( "prString2" == mr.getName()) << std::endl;
  std::cout << data.complete << std::endl;
  std::cout << ( "\'\\\'\'" == std::string(data.begin.begin, data.end.end)) << std::endl;
 }

 {
  auto & mr( matchedRanges->at(3));
  auto data( mr.i().d);
  std::cout << ( "prString1" == mr.getName()) << std::endl;
  std::cout << data.complete << std::endl;
  std::cout << ( "\" \' /* // \\\" \"" == std::string(data.begin.begin, data.end.end)) << std::endl;
 }

 {
  auto & mr( matchedRanges->at(4));
  auto data( mr.i().d);
  std::cout << ( "prComment1" == mr.getName()) << std::endl;
  std::cout << data.complete << std::endl;
  std::cout << ( "/* \" \' // */" == std::string(data.begin.begin, data.end.end)) << std::endl;
 }

 {
  auto & mr( matchedRanges->at(5));
  auto data( mr.i().d);
  std::cout << ( "prString1" == mr.getName()) << std::endl;
  std::cout << data.complete << std::endl;
  std::cout << ( "\"\"" == std::string(data.begin.begin, data.end.end)) << std::endl;
 }

 {
  auto & mr( matchedRanges->at(6));
  auto data( mr.i().d);
  std::cout << ( "prString2" == mr.getName()) << std::endl;
  std::cout << data.complete << std::endl;
  std::cout << ( "''" == std::string(data.begin.begin, data.end.end)) << std::endl;
 }

 {
  auto & mr( matchedRanges->at(7));
  auto data( mr.i().d);
  std::cout << ( "prString1" == mr.getName()) << std::endl;
  std::cout << !data.complete << std::endl;
  std::cout << ( "\" /* X\n" == std::string(data.begin.begin, data.end.end)) << std::endl;
 }

#if false
#endif

}

void test7()
{
 std::cout << __func__ << std::endl;

 InterpreterLua il;
 LuaBase lb;
 
 LuaTest().allTests();
}

void test8()
{
 std::cout << __func__ << std::endl;

 std::vector<AnyBase*> ab;
 ab.push_back(new Any<std::string>("123"));
 ab.push_back(new Any<int>(199));
 std::cout << ( *ab.at(0)->get<std::string>() == "123") << std::endl;
 std::cout << ( !ab.at(1)->get<std::string>()) << std::endl;
 std::cout << ( !ab.at(0)->get<int>()) << std::endl;
 std::cout << ( *ab.at(1)->get<int>() == 199) << std::endl;

 struct B { int i{99};};
 struct D : B {};
 struct E : D {};

 ab.push_back(new Any<D>());

 std::cout << !!ab.at(2)->get<D>() << std::endl;
 std::cout << !ab.at(2)->get<B>() << std::endl;

 std::cout << ( 99 == ab.at(2)->get<D>() ->i ) << std::endl;

 {

  Any<D*> * ad = new Any<D*>(new D());
  ad -> addType<B*>();

  ab.push_back(ad);
 
  int idx = ab.size() - 1;

  std::cout << !!ab.at(idx)->get<D*>() << std::endl;
  std::cout << !!ab.at(idx)->get<B*>() << std::endl;

  std::cout << ( 99 == (*ab.at(idx)->get<D*>())->i) << std::endl;
  std::cout << ( 99 == (*ab.at(idx)->get<B*>())->i) << std::endl;

  delete *ad->get<D*>();
 }

 {

  auto * ad = new Any<std::shared_ptr<E>>(std::make_shared<E>());
  ad -> addTypes<std::shared_ptr<B>,std::shared_ptr<D>>();

  ab.push_back(ad);
 
  int idx = ab.size() - 1;

  std::cout << !!ab.at(idx)->get<std::shared_ptr<E>>() << std::endl;
  std::cout << !!ab.at(idx)->get<std::shared_ptr<D>>() << std::endl;
  std::cout << !!ab.at(idx)->get<std::shared_ptr<B>>() << std::endl;

  std::cout << ( 99 == ab.at(idx)->get<std::shared_ptr<E>>()->get()->i) << std::endl;
  std::cout << ( 99 == ab.at(idx)->get<std::shared_ptr<D>>()->get()->i) << std::endl;
  std::cout << ( 99 == ab.at(idx)->get<std::shared_ptr<B>>()->get()->i) << std::endl;

  delete ad->get<E*>();
 
 }

 for(auto value : ab)
 {
  delete value;
 }
}

void test9()
{
 std::cout << __func__ << std::endl;

 std::shared_ptr<LuaBase> lb{std::make_shared<LuaBase>()};

 RangeMatcherMethods4Lua rangeMatcherMethods4Lua{};

 rangeMatcherMethods4Lua.registerMethods2LuaBase(lb);

 int res = luaL_dofile( lb->getLua(), "testfiles/009_script.lua");

 std::cout << (0 == res) << std::endl;

}

void test10()
{
 std::cout << __func__ << std::endl;

 std::shared_ptr<LuaBase> lb1{std::make_shared<LuaBase>()};
 std::shared_ptr<LuaBase> lb2{std::make_shared<LuaBase>()};
 std::shared_ptr<LuaBase> lb3{std::make_shared<LuaBase>()};

 RangeMatcherMethods4Lua rangeMatcherMethods4Lua{};
 RangeMatcherMethods4Lua rangeMatcherMethods4Lua2{};

 rangeMatcherMethods4Lua.registerMethods2LuaBase(lb1);
 rangeMatcherMethods4Lua.registerMethods2LuaBase(lb2);
 rangeMatcherMethods4Lua2.registerMethods2LuaBase(lb3);

 {
  int res = luaL_dostring( lb1->getLua(), "print( 0 == rmNextObjectIndex())"); // bplsg11e6p
  std::cout << (0 == res) << std::endl;
 }

 {
  int res = luaL_dostring( lb1->getLua(), "rmDebugSetInt(99)");
  std::cout << (0 == res) << std::endl;
 }

 {
  int res = luaL_dostring( lb1->getLua(), "print( 1 == rmNextObjectIndex())");
  std::cout << (0 == res) << std::endl;
 }

 {
  int res = luaL_dostring( lb1->getLua(), "print( 99 == rmDebugGetInt(0))"); // bplsg11e6p 
  std::cout << (0 == res) << std::endl;
 }

 {
  int res = luaL_dostring( lb2->getLua(), "print( 1 == rmNextObjectIndex())");
  std::cout << (0 == res) << std::endl;
 }

 {
  int res = luaL_dostring( lb2->getLua(), "print( 99 == rmDebugGetInt(0))"); // bplsg11e6p 
  std::cout << (0 == res) << std::endl;
 }

 {
  int res = luaL_dostring( lb3->getLua(), "print( 0 == rmNextObjectIndex())");
  std::cout << (0 == res) << std::endl;
 }

}

void test11()
{
 std::cout << __func__ << std::endl;

 std::shared_ptr<LuaBase> lb1{std::make_shared<LuaBase>()};
 std::shared_ptr<LuaBase> lb2{std::make_shared<LuaBase>()};
 std::shared_ptr<LuaBase> lb3{std::make_shared<LuaBase>()};

 RangeMatcherMethods4Lua rangeMatcherMethods4Lua{};
 RangeMatcherMethods4Lua rangeMatcherMethods4Lua2{};

 rangeMatcherMethods4Lua.registerMethods2LuaBase(lb1);
 rangeMatcherMethods4Lua.registerMethods2LuaBase(lb2);
 rangeMatcherMethods4Lua2.registerMethods2LuaBase(lb3);

 {
  int res = luaL_dostring( lb1->getLua(), "print( 0 == rmNextObjectIndex())"); // bplsg11e6p
  std::cout << (0 == res) << std::endl;
 }

 {
  int res = luaL_dostring( lb1->getLua(), "print( 0 == rmDebugSetInt2(99))");
  std::cout << (0 == res) << std::endl;
 }

 {
  int res = luaL_dostring( lb1->getLua(), "print( 1 == rmNextObjectIndex())");
  std::cout << (0 == res) << std::endl;
 }

 {
  int res = luaL_dostring( lb1->getLua(), "print( 99 == rmDebugGetInt2(0))"); // bplsg11e6p 
  std::cout << (0 == res) << std::endl;
 }

 {
  int res = luaL_dostring( lb2->getLua(), "print( 1 == rmNextObjectIndex())");
  std::cout << (0 == res) << std::endl;
 }

 {
  int res = luaL_dostring( lb2->getLua(), "print( 99 == rmDebugGetInt2(0))"); // bplsg11e6p 
  std::cout << (0 == res) << std::endl;
 }

 {
  int res = luaL_dostring( lb3->getLua(), "print( 0 == rmNextObjectIndex())");
  std::cout << (0 == res) << std::endl;
 }

}

int main( int argc, char** argv)
{

 test();

 test2();

 test3();
 
 test4();

 test5();

 test6("testfiles/004.txt");

 test6("testfiles/005.txt");

 test6("testfiles/006.txt");

 test7();

 test8();

 test9();

 test10();

 test11();

 return 0;
}

