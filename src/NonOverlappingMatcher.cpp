#include "NonOverlappingMatcher.hpp"

#include "Match.hpp"

#include <algorithm> // sort
#include <functional> // function

#include <stdexcept> // logic_error

#include <iostream> // TODO remove

NonOverlappingMatcher::NonOverlappingMatcher(std::vector<std::shared_ptr<NamedPatternRange>> & patternRange)
: m_PatternRange{patternRange}
{
}

namespace {

 struct Status {
  std::unique_ptr<Match> match;
 };

 using P = std::pair<NamedPatternRange const * , Status>; // s56plqg7n2 

 std::unique_ptr<Match> findOpeningMatch( std::vector<P> & v, Range range) 
 {
  for( auto & value : v) {
   std::unique_ptr<Match> match { value.first->i().von->match(range)};
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

  return std::move( v.front().second.match);
 }

 std::unique_ptr<Match> findClosingMatch( const std::vector<P> & v, Range range)
 {
  return v.at(0).first->i().bis->match(range);
 }
}

void NonOverlappingMatcher::matchAll(Range range) const
{
 
 std::vector<P> v;

 if( 0==m_PatternRange.size()){
  return; 
 }

 for( auto & namedPatternRange : m_PatternRange)
 {
  v.push_back(P(namedPatternRange.get(), Status{}));
 }

 Range currentRange(range);

 while( true)
 {

  // find first match

  std::unique_ptr<Match> matchOpen{ findOpeningMatch( v, currentRange)};

  std::cout << ("prComment1" == v.at(0).first->getName()) << std::endl;
  std::cout << ("prString" == v.at(1).first->getName()) << std::endl;
  std::cout << ("prComment2" == v.at(2).first->getName()) << std::endl;

  // std::cout << (v.at(0).second.match->matched()) << std::endl; // crash (moved away in findOpeningMatch)
  std::cout << (v.at(1).second.match->matched()) << std::endl;
  std::cout << (!v.at(2).second.match->matched()) << std::endl;

  // std::cout << ( 0 == std::distance( vectorTf002->begin() , v.at(0).second.match->get()->m_Range.begin)) << std::endl;
  // std::cout << ( 3 == std::distance( vectorTf002->begin() , v.at(1).second.match->get()->m_Range.begin)) << std::endl;


  if( !matchOpen->matched()){
   return; // no further matches
  }


  std::function<void()> currentRangeCheck{ [&currentRange, &range](){
   std::cout << (currentRange.begin - range.begin) << std::endl;

   if( currentRange.begin == currentRange.end)
   {
    return; // no further matches
   }

   if( currentRange.begin > currentRange.end)
   {
    throw std::logic_error("currentRange.begin > currentRange.end");
   }
  }};

  currentRange.begin = matchOpen->get()->m_Range.end; // MatchGot

  currentRangeCheck();

  // seek closing match

  std::unique_ptr<Match> matchClose{ findClosingMatch( v, currentRange)};
  
  if( !matchClose->matched()){
   return; // no further matches
  }

  currentRange.begin = matchClose->get()->m_Range.end; // MatchGot

  // TODO : matches sammeln, benennen und zurueckgeben

 }
 

}

