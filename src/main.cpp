
#include "FileToVector.hpp"
#include "NamedPatternRange.hpp"
#include "NamingWeakOrdered.hpp"
#include "PatternRange.hpp"
#include "PatternRegex.hpp"
#include "PatternString.hpp"
#include "PatternVector.hpp"
#include "Range.hpp"

#include <algorithm> // sort
#include <iostream>
#include <iterator> // distance
#include <utility>
#include <vector>

void test()
{

 std::cout << __func__ << std::endl;

 std::vector<char> v{'1','a','b','c','2'};

 std::vector<char> v2{'b','c'};

 PatternString ps1{"abc"};

 PatternString ps2{"def"};

 PatternVector pv1{v2};

 std::unique_ptr<Match> m1{ ps1.match(Range{v.begin(), v.end()})}; // auto geht hier nicht
 std::unique_ptr<Match> m2{ ps2.match(Range{v.begin(), v.end()})}; // auto geht hier nicht

 // std::shared_ptr<MatchGot> mg2 = m2->get(); // !matched exception
 
 std::cout << ! m2->matched() << std::endl;
 std::cout << m1->matched() << std::endl;

 std::shared_ptr<const MatchGot> mg1 = m1->get();

 std::cout << ( mg1->m_Range.begin == v.begin() + 1 ) << std::endl;
 std::cout << ( mg1->m_Range.end == v.begin() + 4 ) << std::endl;

 std::unique_ptr<Match> m3{ pv1.match(Range{v.begin(), v.end()})};

 std::cout << m3->matched() << std::endl;

 std::shared_ptr<const MatchGot> mg2 = m3->get();

 std::cout << ( mg2->m_Range.begin == v.begin() + 2 ) << std::endl;
 std::cout << ( mg2->m_Range.end == v.begin() + 4 ) << std::endl;

 PatternRegex pr1{"a.*c"};
 PatternRegex pr2{"c.*a"};

 std::unique_ptr<Match> m4{ pr1.match(Range{v.begin(), v.end()})};
 std::unique_ptr<Match> m5{ pr2.match(Range{v.begin(), v.end()})};

 std::cout << ! m5->matched() << std::endl;
 std::cout << m4->matched() << std::endl;

 std::shared_ptr<const MatchGot> mg3 = m4->get();

 std::cout << ( mg3->m_Range.begin == v.begin() + 1 ) << std::endl;
 std::cout << ( mg3->m_Range.end == v.begin() + 4 ) << std::endl;

 std::unique_ptr<std::vector<char>> v3{static_cast<std::unique_ptr<std::vector<char>>>(FileToVector("testfiles/001.txt"))};

 std::unique_ptr<Match> m6{ ps2.match(Range{v3->begin(), v3->end()})}; 

 std::cout << m6->matched() << std::endl;

 std::shared_ptr<const MatchGot> mg4 = m6->get();

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
 
 std::unique_ptr<std::vector<char>> vectorTf002 { static_cast<std::unique_ptr<std::vector<char>>>(FileToVector("testfiles/002.txt"))};

 struct Status {
  std::string name;
  std::unique_ptr<Match> match;
 };

 using P = std::pair<PatternRange * , Status>;

 std::vector<P> v;

 v.push_back(P(&prComment1, Status{"prComment1"}));
 v.push_back(P(&prComment2, Status{"prComment2"}));
 v.push_back(P(&prString, Status{"prString"}));

 for( auto & value : v) {
  std::unique_ptr<Match> match { value.first->i().von->match(Range{vectorTf002->begin(), vectorTf002->end()})};
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

 
 std::unique_ptr<std::vector<char>> vectorTf002 { static_cast<std::unique_ptr<std::vector<char>>>(FileToVector("testfiles/002.txt"))};

 struct Status {
  std::unique_ptr<Match> match;
 };

 using P = std::pair<NamedPatternRange * , Status>;

 std::vector<P> v;

 v.push_back(P(&prComment1, Status{}));
 v.push_back(P(&prComment2, Status{}));
 v.push_back(P(&prString, Status{}));

 for( auto & value : v) {
  std::unique_ptr<Match> match { value.first->i().von->match(Range{vectorTf002->begin(), vectorTf002->end()})};
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

int main( int argc, char** argv)
{

 test();

 test2();

 test3();
 
 return 0;
}

